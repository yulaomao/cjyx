/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

// DMML includes
#include "vtkDMMLInteractionEventData.h"
#include "vtkDMMLMarkupsROIDisplayNode.h"
#include "vtkDMMLMarkupsROINode.h"
#include "vtkDMMLSliceNode.h"

// Markups VTK widgets includes
#include "vtkCjyxROIWidget.h"
#include "vtkCjyxROIRepresentation2D.h"
#include "vtkCjyxROIRepresentation3D.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkEvent.h>
#include <vtkPointPlacer.h>
#include <vtkRenderer.h>
#include <vtkTransform.h>

vtkStandardNewMacro(vtkCjyxROIWidget);

//----------------------------------------------------------------------
vtkCjyxROIWidget::vtkCjyxROIWidget()
{
  this->SetEventTranslationClickAndDrag(WidgetStateOnScaleHandle, vtkCommand::LeftButtonPressEvent, vtkEvent::AltModifier,
    WidgetStateSymmetricScale, WidgetEventSymmetricScaleStart, WidgetEventSymmetricScaleEnd);
}

//----------------------------------------------------------------------
vtkCjyxROIWidget::~vtkCjyxROIWidget() = default;

//----------------------------------------------------------------------
void vtkCjyxROIWidget::CreateDefaultRepresentation(
  vtkDMMLMarkupsDisplayNode* markupsDisplayNode, vtkDMMLAbstractViewNode* viewNode, vtkRenderer* renderer)
{
  vtkSmartPointer<vtkCjyxMarkupsWidgetRepresentation> rep = nullptr;
  if (vtkDMMLSliceNode::SafeDownCast(viewNode))
    {
    rep = vtkSmartPointer<vtkCjyxROIRepresentation2D>::New();
    }
  else
    {
    rep = vtkSmartPointer<vtkCjyxROIRepresentation3D>::New();
    }
  this->SetRenderer(renderer);
  this->SetRepresentation(rep);
  rep->SetViewNode(viewNode);
  rep->SetMarkupsDisplayNode(markupsDisplayNode);
  rep->UpdateFromDMML(nullptr, 0); // full update
}

//-----------------------------------------------------------------------------
bool vtkCjyxROIWidget::CanProcessInteractionEvent(vtkDMMLInteractionEventData* eventData, double &distance2)
{
  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);
  if (widgetEvent == WidgetEventNone)
    {
    return false;
    }
  vtkCjyxMarkupsWidgetRepresentation* rep = this->GetMarkupsRepresentation();
  if (!rep)
    {
    return false;
    }

  // If we are placing markups or dragging the mouse then we interact everywhere
  if (this->WidgetState == WidgetStateSymmetricScale)
    {
    distance2 = 0.0;
    return true;
    }

  return Superclass::CanProcessInteractionEvent(eventData, distance2);
}

//-----------------------------------------------------------------------------
bool vtkCjyxROIWidget::ProcessInteractionEvent(vtkDMMLInteractionEventData* eventData)
{
  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);

  bool processedEvent = false;
  switch (widgetEvent)
    {
    case WidgetEventSymmetricScaleStart:
      processedEvent = ProcessWidgetSymmetricScaleStart(eventData);
      break;
    case WidgetEventSymmetricScaleEnd:
      processedEvent = ProcessEndMouseDrag(eventData);
      break;
    }

  if (!processedEvent)
    {
    processedEvent = Superclass::ProcessInteractionEvent(eventData);
    }
  return processedEvent;
}

//-------------------------------------------------------------------------
bool vtkCjyxROIWidget::ProcessWidgetSymmetricScaleStart(vtkDMMLInteractionEventData* eventData)
{
  if ((this->WidgetState != vtkCjyxMarkupsWidget::WidgetStateOnWidget && this->WidgetState != vtkCjyxMarkupsWidget::WidgetStateOnScaleHandle)
    || this->IsAnyControlPointLocked())
    {
    return false;
    }

  this->SetWidgetState(WidgetStateSymmetricScale);
  this->StartWidgetInteraction(eventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkCjyxROIWidget::ProcessEndMouseDrag(vtkDMMLInteractionEventData* eventData)
{
  if (!this->WidgetRep)
    {
    return false;
    }

  if (this->WidgetState != vtkCjyxROIWidget::WidgetStateSymmetricScale)
    {
    return Superclass::ProcessEndMouseDrag(eventData);
    }

  int activeComponentType = this->GetActiveComponentType();
  if (activeComponentType == vtkDMMLMarkupsDisplayNode::ComponentScaleHandle)
    {
    this->SetWidgetState(WidgetStateOnScaleHandle);
    }
  else
    {
    this->SetWidgetState(WidgetStateOnWidget);
    }

  this->EndWidgetInteraction();
  return true;
}


//-------------------------------------------------------------------------
bool vtkCjyxROIWidget::ProcessMouseMove(vtkDMMLInteractionEventData* eventData)
{
  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  vtkCjyxMarkupsWidgetRepresentation* rep = this->GetMarkupsRepresentation();
  if (!rep || !markupsNode || !eventData)
    {
    return false;
    }

  int state = this->WidgetState;
  if (state != WidgetStateSymmetricScale)
    {
    return Superclass::ProcessMouseMove(eventData);
    }

  // Process the motion
  // Based on the displacement vector (computed in display coordinates) and
  // the cursor state (which corresponds to which part of the widget has been
  // selected), the widget points are modified.
  // First construct a local coordinate system based on the display coordinates
  // of the widget.
  double eventPos[2]
    {
    static_cast<double>(eventData->GetDisplayPosition()[0]),
    static_cast<double>(eventData->GetDisplayPosition()[1]),
    };

  this->ScaleWidget(eventPos, true);

  this->LastEventPosition[0] = eventPos[0];
  this->LastEventPosition[1] = eventPos[1];

  return true;
}

//----------------------------------------------------------------------
void vtkCjyxROIWidget::ScaleWidget(double eventPos[2])
{
  this->ScaleWidget(eventPos, false);
}

//----------------------------------------------------------------------
void vtkCjyxROIWidget::ScaleWidget(double eventPos[2], bool symmetricScale)
{
  vtkDMMLMarkupsDisplayNode* displayNode = this->GetMarkupsDisplayNode();
  vtkDMMLMarkupsROINode* markupsNode = vtkDMMLMarkupsROINode::SafeDownCast(this->GetMarkupsNode());
  if (!markupsNode || !displayNode)
    {
    return;
    }

  DMMLNodeModifyBlocker blocker(markupsNode);

  double lastEventPos_World[3] = { 0.0 };
  double eventPos_World[3] = { 0.0 };
  double orientation_World[9] = { 0.0 };

  vtkCjyxROIRepresentation2D* rep2d = vtkCjyxROIRepresentation2D::SafeDownCast(this->WidgetRep);
  vtkCjyxROIRepresentation3D* rep3d = vtkCjyxROIRepresentation3D::SafeDownCast(this->WidgetRep);
  if (rep2d)
    {
    // 2D view
    double eventPos_Slice[3] = { 0. };
    eventPos_Slice[0] = this->LastEventPosition[0];
    eventPos_Slice[1] = this->LastEventPosition[1];
    rep2d->GetSliceToWorldCoordinates(eventPos_Slice, lastEventPos_World);

    eventPos_Slice[0] = eventPos[0];
    eventPos_Slice[1] = eventPos[1];
    rep2d->GetSliceToWorldCoordinates(eventPos_Slice, eventPos_World);
    }
  else if (rep3d)
    {
    // 3D view
    double eventPos_Display[2] = { 0. };
    eventPos_Display[0] = this->LastEventPosition[0];
    eventPos_Display[1] = this->LastEventPosition[1];

    if (!rep3d->GetPointPlacer()->ComputeWorldPosition(this->Renderer,
      eventPos_Display, lastEventPos_World, eventPos_World,
      orientation_World))
      {
      return;
      }
    lastEventPos_World[0] = eventPos_World[0];
    lastEventPos_World[1] = eventPos_World[1];
    lastEventPos_World[2] = eventPos_World[2];

    eventPos_Display[0] = eventPos[0];
    eventPos_Display[1] = eventPos[1];

    if (!rep3d->GetPointPlacer()->ComputeWorldPosition(this->Renderer,
      eventPos_Display, lastEventPos_World, eventPos_World,
      orientation_World))
      {
      return;
      }
    }

  if (this->GetActiveComponentType() == vtkDMMLMarkupsDisplayNode::ComponentScaleHandle)
    {
    vtkNew<vtkMatrix4x4> worldToObjectMatrix;
    worldToObjectMatrix->DeepCopy(markupsNode->GetObjectToWorldMatrix());
    worldToObjectMatrix->Invert();
    vtkNew<vtkTransform> worldToObjectTransform;
    worldToObjectTransform->SetMatrix(worldToObjectMatrix);

    int index = displayNode->GetActiveComponentIndex();
    if (index < 6 && rep3d)
      {
      this->GetClosestPointOnInteractionAxis(
        vtkDMMLMarkupsDisplayNode::ComponentScaleHandle, index, this->LastEventPosition, lastEventPos_World);
      this->GetClosestPointOnInteractionAxis(
        vtkDMMLMarkupsDisplayNode::ComponentScaleHandle, index, eventPos, eventPos_World);
      }

    double scaleVector_World[3] = { 0.0, 0.0, 0.0 };
    vtkMath::Subtract(eventPos_World, lastEventPos_World, scaleVector_World);

    double scaleVector_ROI[3] = { 0.0, 0.0, 0.0 };
    worldToObjectTransform->TransformVector(scaleVector_World, scaleVector_ROI);

    double radius_ROI[3] = { 0.0, 0.0, 0.0 };
    markupsNode->GetSize(radius_ROI);
    vtkMath::MultiplyScalar(radius_ROI, 0.5);
    double bounds_ROI[6] = { -radius_ROI[0], radius_ROI[0], -radius_ROI[1], radius_ROI[1], -radius_ROI[2], radius_ROI[2] };

    switch (index)
      {
      case vtkDMMLMarkupsROIDisplayNode::HandleLFace:
      case vtkDMMLMarkupsROIDisplayNode::HandleLPICorner:
      case vtkDMMLMarkupsROIDisplayNode::HandleLAICorner:
      case vtkDMMLMarkupsROIDisplayNode::HandleLPSCorner:
      case vtkDMMLMarkupsROIDisplayNode::HandleLASCorner:
      case vtkDMMLMarkupsROIDisplayNode::HandleLAEdge:
      case vtkDMMLMarkupsROIDisplayNode::HandleLPEdge:
      case vtkDMMLMarkupsROIDisplayNode::HandleLIEdge:
      case vtkDMMLMarkupsROIDisplayNode::HandleLSEdge:
        bounds_ROI[0] += scaleVector_ROI[0];
        if (symmetricScale)
          {
          bounds_ROI[1] -= scaleVector_ROI[0];
          }
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleRFace:
      case vtkDMMLMarkupsROIDisplayNode::HandleRPICorner:
      case vtkDMMLMarkupsROIDisplayNode::HandleRAICorner:
      case vtkDMMLMarkupsROIDisplayNode::HandleRPSCorner:
      case vtkDMMLMarkupsROIDisplayNode::HandleRASCorner:
      case vtkDMMLMarkupsROIDisplayNode::HandleRAEdge:
      case vtkDMMLMarkupsROIDisplayNode::HandleRPEdge:
      case vtkDMMLMarkupsROIDisplayNode::HandleRIEdge:
      case vtkDMMLMarkupsROIDisplayNode::HandleRSEdge:
        bounds_ROI[1] += scaleVector_ROI[0];
        if (symmetricScale)
          {
          bounds_ROI[0] -= scaleVector_ROI[0];
          }
        break;
      default:
        break;
      }

    switch (index)
      {
      case vtkDMMLMarkupsROIDisplayNode::HandlePFace:
      case vtkDMMLMarkupsROIDisplayNode::HandleLPICorner:
      case vtkDMMLMarkupsROIDisplayNode::HandleRPICorner:
      case vtkDMMLMarkupsROIDisplayNode::HandleLPSCorner:
      case vtkDMMLMarkupsROIDisplayNode::HandleRPSCorner:
      case vtkDMMLMarkupsROIDisplayNode::HandleLPEdge:
      case vtkDMMLMarkupsROIDisplayNode::HandleRPEdge:
      case vtkDMMLMarkupsROIDisplayNode::HandlePIEdge:
      case vtkDMMLMarkupsROIDisplayNode::HandlePSEdge:
        bounds_ROI[2] += scaleVector_ROI[1];
        if (symmetricScale)
          {
          bounds_ROI[3] -= scaleVector_ROI[1];
          }
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleAFace:
      case vtkDMMLMarkupsROIDisplayNode::HandleLAICorner:
      case vtkDMMLMarkupsROIDisplayNode::HandleRAICorner:
      case vtkDMMLMarkupsROIDisplayNode::HandleLASCorner:
      case vtkDMMLMarkupsROIDisplayNode::HandleRASCorner:
      case vtkDMMLMarkupsROIDisplayNode::HandleLAEdge:
      case vtkDMMLMarkupsROIDisplayNode::HandleRAEdge:
      case vtkDMMLMarkupsROIDisplayNode::HandleAIEdge:
      case vtkDMMLMarkupsROIDisplayNode::HandleASEdge:
        bounds_ROI[3] += scaleVector_ROI[1];
        if (symmetricScale)
          {
          bounds_ROI[2] -= scaleVector_ROI[1];
          }
        break;
      default:
        break;
      }

    switch (index)
      {
      case vtkDMMLMarkupsROIDisplayNode::HandleIFace:
      case vtkDMMLMarkupsROIDisplayNode::HandleLPICorner:
      case vtkDMMLMarkupsROIDisplayNode::HandleRPICorner:
      case vtkDMMLMarkupsROIDisplayNode::HandleLAICorner:
      case vtkDMMLMarkupsROIDisplayNode::HandleRAICorner:
      case vtkDMMLMarkupsROIDisplayNode::HandleLIEdge:
      case vtkDMMLMarkupsROIDisplayNode::HandleRIEdge:
      case vtkDMMLMarkupsROIDisplayNode::HandleAIEdge:
      case vtkDMMLMarkupsROIDisplayNode::HandlePIEdge:
        bounds_ROI[4] += scaleVector_ROI[2];
        if (symmetricScale)
          {
          bounds_ROI[5] -= scaleVector_ROI[2];
          }
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleSFace:
      case vtkDMMLMarkupsROIDisplayNode::HandleLPSCorner:
      case vtkDMMLMarkupsROIDisplayNode::HandleRPSCorner:
      case vtkDMMLMarkupsROIDisplayNode::HandleLASCorner:
      case vtkDMMLMarkupsROIDisplayNode::HandleRASCorner:
      case vtkDMMLMarkupsROIDisplayNode::HandleLSEdge:
      case vtkDMMLMarkupsROIDisplayNode::HandleRSEdge:
      case vtkDMMLMarkupsROIDisplayNode::HandleASEdge:
      case vtkDMMLMarkupsROIDisplayNode::HandlePSEdge:
        bounds_ROI[5] += scaleVector_ROI[2];
        if (symmetricScale)
          {
          bounds_ROI[4] -= scaleVector_ROI[2];
          }
        break;
      default:
        break;
      }

    double newSize[3] = { 0.0, 0.0, 0.0 };
    double newOrigin_ROI[3] = { 0.0, 0.0, 0.0 };
    for (int i = 0; i < 3; ++i)
      {
      newSize[i] = std::abs(bounds_ROI[2 * i + 1] - bounds_ROI[2 * i]);
      newOrigin_ROI[i] = (bounds_ROI[2 * i + 1] + bounds_ROI[2 * i]) / 2.0;
      }

    vtkNew<vtkTransform> objectToWorldTransform;
    objectToWorldTransform->SetMatrix(markupsNode->GetObjectToWorldMatrix());
    double newOrigin_World[3] = { 0.0, 0.0, 0.0 };
    objectToWorldTransform->TransformPoint(newOrigin_ROI, newOrigin_World);
    markupsNode->SetCenterWorld(newOrigin_World);
    markupsNode->SetSize(newSize);

    bool flipLRHandle = bounds_ROI[1] < bounds_ROI[0];
    bool flipPAHandle = bounds_ROI[3] < bounds_ROI[2];
    bool flipISHandle = bounds_ROI[5] < bounds_ROI[4];
    if (flipLRHandle || flipPAHandle || flipISHandle)
      {
      this->FlipROIHandles(flipLRHandle, flipPAHandle, flipISHandle);
      }
    }
}

//----------------------------------------------------------------------
void vtkCjyxROIWidget::FlipROIHandles(bool flipLRHandle, bool flipPAHandle, bool flipISHandle)
{
  vtkDMMLMarkupsDisplayNode* displayNode = this->GetMarkupsDisplayNode();
  vtkDMMLMarkupsROINode* markupsNode = vtkDMMLMarkupsROINode::SafeDownCast(this->GetMarkupsNode());
  if (!markupsNode || !displayNode)
    {
    return;
    }

  int index = displayNode->GetActiveComponentIndex();
  if (flipLRHandle)
    {
    switch (index)
      {
      case vtkDMMLMarkupsROIDisplayNode::HandleLFace:
        index = vtkDMMLMarkupsROIDisplayNode::HandleRFace;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleRFace:
        index = vtkDMMLMarkupsROIDisplayNode::HandleLFace;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleLAICorner:
        index = vtkDMMLMarkupsROIDisplayNode::HandleRAICorner;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleLPICorner:
        index = vtkDMMLMarkupsROIDisplayNode::HandleRPICorner;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleLASCorner:
        index = vtkDMMLMarkupsROIDisplayNode::HandleRASCorner;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleLPSCorner:
        index = vtkDMMLMarkupsROIDisplayNode::HandleRPSCorner;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleRAICorner:
        index = vtkDMMLMarkupsROIDisplayNode::HandleLAICorner;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleRPICorner:
        index = vtkDMMLMarkupsROIDisplayNode::HandleLPICorner;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleRASCorner:
        index = vtkDMMLMarkupsROIDisplayNode::HandleLASCorner;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleRPSCorner:
        index = vtkDMMLMarkupsROIDisplayNode::HandleLPSCorner;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleLAEdge:
        index = vtkDMMLMarkupsROIDisplayNode::HandleRAEdge;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleLPEdge:
        index = vtkDMMLMarkupsROIDisplayNode::HandleRPEdge;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleLIEdge:
        index = vtkDMMLMarkupsROIDisplayNode::HandleRIEdge;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleLSEdge:
        index = vtkDMMLMarkupsROIDisplayNode::HandleRSEdge;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleRAEdge:
        index = vtkDMMLMarkupsROIDisplayNode::HandleLAEdge;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleRPEdge:
        index = vtkDMMLMarkupsROIDisplayNode::HandleLPEdge;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleRIEdge:
        index = vtkDMMLMarkupsROIDisplayNode::HandleLIEdge;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleRSEdge:
        index = vtkDMMLMarkupsROIDisplayNode::HandleLSEdge;
        break;
      default:
        break;
      }
    }

  if (flipPAHandle)
    {
    switch (index)
      {
      case vtkDMMLMarkupsROIDisplayNode::HandleAFace:
        index = vtkDMMLMarkupsROIDisplayNode::HandlePFace;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandlePFace:
        index = vtkDMMLMarkupsROIDisplayNode::HandleAFace;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleLAICorner:
        index = vtkDMMLMarkupsROIDisplayNode::HandleLPICorner;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleLPICorner:
        index = vtkDMMLMarkupsROIDisplayNode::HandleLAICorner;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleLASCorner:
        index = vtkDMMLMarkupsROIDisplayNode::HandleLPSCorner;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleLPSCorner:
        index = vtkDMMLMarkupsROIDisplayNode::HandleLASCorner;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleRAICorner:
        index = vtkDMMLMarkupsROIDisplayNode::HandleRPICorner;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleRPICorner:
        index = vtkDMMLMarkupsROIDisplayNode::HandleRAICorner;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleRASCorner:
        index = vtkDMMLMarkupsROIDisplayNode::HandleRPSCorner;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleRPSCorner:
        index = vtkDMMLMarkupsROIDisplayNode::HandleRASCorner;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleLPEdge:
        index = vtkDMMLMarkupsROIDisplayNode::HandleLAEdge;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleRPEdge:
        index = vtkDMMLMarkupsROIDisplayNode::HandleRAEdge;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandlePIEdge:
        index = vtkDMMLMarkupsROIDisplayNode::HandleAIEdge;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandlePSEdge:
        index = vtkDMMLMarkupsROIDisplayNode::HandleASEdge;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleLAEdge:
        index = vtkDMMLMarkupsROIDisplayNode::HandleLPEdge;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleRAEdge:
        index = vtkDMMLMarkupsROIDisplayNode::HandleRPEdge;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleAIEdge:
        index = vtkDMMLMarkupsROIDisplayNode::HandlePIEdge;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleASEdge:
        index = vtkDMMLMarkupsROIDisplayNode::HandlePSEdge;
        break;
      default:
        break;
      }
    }

  if (flipISHandle)
    {
    switch (index)
      {
      case vtkDMMLMarkupsROIDisplayNode::HandleIFace:
        index = vtkDMMLMarkupsROIDisplayNode::HandleSFace;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleSFace:
        index = vtkDMMLMarkupsROIDisplayNode::HandleIFace;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleLAICorner:
        index = vtkDMMLMarkupsROIDisplayNode::HandleLASCorner;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleLPICorner:
        index = vtkDMMLMarkupsROIDisplayNode::HandleLPSCorner;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleLASCorner:
        index = vtkDMMLMarkupsROIDisplayNode::HandleLAICorner;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleLPSCorner:
        index = vtkDMMLMarkupsROIDisplayNode::HandleLPICorner;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleRAICorner:
        index = vtkDMMLMarkupsROIDisplayNode::HandleRASCorner;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleRPICorner:
        index = vtkDMMLMarkupsROIDisplayNode::HandleRPICorner;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleRASCorner:
        index = vtkDMMLMarkupsROIDisplayNode::HandleRAICorner;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleRPSCorner:
        index = vtkDMMLMarkupsROIDisplayNode::HandleRPICorner;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleLIEdge:
        index = vtkDMMLMarkupsROIDisplayNode::HandleLSEdge;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleRIEdge:
        index = vtkDMMLMarkupsROIDisplayNode::HandleRSEdge;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleAIEdge:
        index = vtkDMMLMarkupsROIDisplayNode::HandleASEdge;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandlePIEdge:
        index = vtkDMMLMarkupsROIDisplayNode::HandlePSEdge;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleLSEdge:
        index = vtkDMMLMarkupsROIDisplayNode::HandleLIEdge;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleRSEdge:
        index = vtkDMMLMarkupsROIDisplayNode::HandleRIEdge;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandleASEdge:
        index = vtkDMMLMarkupsROIDisplayNode::HandleAIEdge;
        break;
      case vtkDMMLMarkupsROIDisplayNode::HandlePSEdge:
        index = vtkDMMLMarkupsROIDisplayNode::HandlePIEdge;
        break;
      default:
        break;
      }
    }

  displayNode->SetActiveComponent(displayNode->GetActiveComponentType(), index);
}
