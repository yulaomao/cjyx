/*=========================================================================

 Copyright (c) ProxSim ltd., Kwun Tong, Hong Kong. All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 This file was originally developed by Davide Punzo, punzodavide@hotmail.it,
 and development was supported by ProxSim ltd.

=========================================================================*/

#include "vtkCjyxMarkupsWidget.h"

#include "vtkDMMLInteractionEventData.h"
#include "vtkDMMLInteractionNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSliceCompositeNode.h"
#include "vtkDMMLSliceLogic.h"
#include "vtkCjyxMarkupsWidgetRepresentation.h"
#include "vtkCjyxMarkupsWidgetRepresentation2D.h"
#include "vtkCjyxMarkupsWidgetRepresentation3D.h"

// VTK includes
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkEvent.h>
#include <vtkLine.h>
#include <vtkPlane.h>
#include <vtkPointPlacer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkTransform.h>
#include <vtkObjectFactory.h>

// DMML includes
#include "vtkDMMLTransformNode.h"
#include <vtkDMMLApplicationLogic.h>

//----------------------------------------------------------------------
vtkCjyxMarkupsWidget::vtkCjyxMarkupsWidget()
{
  this->LastEventPosition[0] = 0.0;
  this->LastEventPosition[1] = 0.0;
  this->StartEventOffsetPosition[0] = 0.0;
  this->StartEventOffsetPosition[1] = 0.0;

  this->PreviewPointIndex = -1;

  // Place
  this->SetEventTranslation(WidgetStateDefine, vtkCommand::LeftButtonPressEvent, vtkEvent::NoModifier, WidgetEventReserved);
  this->SetEventTranslation(WidgetStateDefine, vtkCommand::LeftButtonReleaseEvent, vtkEvent::NoModifier, WidgetEventControlPointPlace);
  this->SetEventTranslation(WidgetStateDefine, vtkCommand::RightButtonPressEvent, vtkEvent::NoModifier, WidgetEventReserved);
  this->SetEventTranslation(WidgetStateDefine, vtkCommand::RightButtonReleaseEvent, vtkEvent::NoModifier, WidgetEventStopPlace);
  this->SetEventTranslation(WidgetStateDefine, vtkCommand::LeftButtonDoubleClickEvent, vtkEvent::NoModifier, WidgetEventStopPlace);

  // Manipulate
  this->SetEventTranslationClickAndDrag(WidgetStateOnWidget, vtkCommand::LeftButtonPressEvent, vtkEvent::NoModifier,
    WidgetStateTranslateControlPoint, WidgetEventControlPointMoveStart, WidgetEventControlPointMoveEnd);
  this->SetEventTranslationClickAndDrag(WidgetStateOnWidget, vtkCommand::MiddleButtonPressEvent, vtkEvent::NoModifier,
    WidgetStateTranslate, WidgetEventTranslateStart, WidgetEventTranslateEnd);
  this->SetKeyboardEventTranslation(WidgetStateOnWidget, vtkEvent::NoModifier, 115, 1, "s", WidgetEventControlPointSnapToSlice);
  this->SetKeyboardEventTranslation(WidgetStateOnWidget, vtkEvent::ShiftModifier, 127, 1, "Delete", WidgetEventReset);
  this->SetKeyboardEventTranslation(WidgetStateOnWidget, vtkEvent::NoModifier, 127, 1, "Delete", WidgetEventControlPointDelete);
  this->SetKeyboardEventTranslation(WidgetStateOnWidget, vtkEvent::NoModifier, 8, 1, "BackSpace", WidgetEventControlPointDelete);

  this->SetEventTranslation(WidgetStateOnWidget, vtkDMMLInteractionEventData::LeftButtonClickEvent, vtkEvent::NoModifier, WidgetEventJumpCursor);
  this->SetEventTranslation(WidgetStateOnWidget, vtkCommand::LeftButtonDoubleClickEvent, vtkEvent::NoModifier, WidgetEventAction);

  unsigned int menuStates[] = { WidgetStateOnWidget, WidgetStateOnTranslationHandle, WidgetStateOnRotationHandle, WidgetStateOnScaleHandle };
  for (auto menuState : menuStates)
    {
    this->SetEventTranslation(menuState, vtkCommand::RightButtonPressEvent, vtkEvent::NoModifier, WidgetEventReserved);
    this->SetEventTranslation(menuState, vtkCommand::RightButtonReleaseEvent, vtkEvent::NoModifier, WidgetEventReserved);
    this->SetEventTranslation(menuState, vtkDMMLInteractionEventData::RightButtonClickEvent, vtkEvent::NoModifier, WidgetEventMenu);
    }

  // Update active component
  this->SetEventTranslation(WidgetStateIdle, vtkCommand::MouseMoveEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateOnWidget, vtkCommand::MouseMoveEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  // Allow AnyModifier when defining the markup position. This allows the markup preview to be continually updated, even
  // when using shift + mouse-move to change the slice positions.
  // We still do not allow shift+left click for placement however, so that the shift + left-click-and-drag interaction can
  // still be used to pan the slice.
  this->SetEventTranslation(WidgetStateDefine, vtkCommand::MouseMoveEvent, vtkEvent::AnyModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateIdle, vtkCommand::Move3DEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateOnWidget, vtkCommand::Move3DEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateDefine, vtkCommand::Move3DEvent, vtkEvent::NoModifier, WidgetEventMouseMove);

  // Handle interactions
  this->SetEventTranslationClickAndDrag(WidgetStateOnTranslationHandle, vtkCommand::LeftButtonPressEvent, vtkEvent::NoModifier,
    WidgetStateTranslate, WidgetEventTranslateStart, WidgetEventTranslateEnd);
  this->SetEventTranslation(WidgetStateOnTranslationHandle, vtkDMMLInteractionEventData::RightButtonClickEvent, vtkEvent::NoModifier, WidgetEventMenu);
  this->SetEventTranslation(WidgetStateOnTranslationHandle, vtkDMMLInteractionEventData::LeftButtonClickEvent, vtkEvent::NoModifier, WidgetEventJumpCursor);

  this->SetEventTranslationClickAndDrag(WidgetStateOnRotationHandle, vtkCommand::LeftButtonPressEvent, vtkEvent::NoModifier,
    WidgetStateRotate, WidgetEventRotateStart, WidgetEventRotateEnd);
  this->SetEventTranslation(WidgetStateOnRotationHandle, vtkDMMLInteractionEventData::RightButtonClickEvent, vtkEvent::NoModifier, WidgetEventMenu);
  this->SetEventTranslation(WidgetStateOnRotationHandle, vtkDMMLInteractionEventData::LeftButtonClickEvent, vtkEvent::NoModifier, WidgetEventJumpCursor);

  this->SetEventTranslationClickAndDrag(WidgetStateOnScaleHandle, vtkCommand::LeftButtonPressEvent, vtkEvent::NoModifier,
    WidgetStateScale, WidgetEventScaleStart, WidgetEventScaleEnd);
  this->SetEventTranslation(WidgetStateOnScaleHandle, vtkDMMLInteractionEventData::RightButtonClickEvent, vtkEvent::NoModifier, WidgetEventMenu);
  this->SetEventTranslation(WidgetStateOnScaleHandle, vtkDMMLInteractionEventData::LeftButtonClickEvent, vtkEvent::NoModifier, WidgetEventJumpCursor);


  // Update active interaction handle component
  this->SetEventTranslation(WidgetStateOnTranslationHandle, vtkCommand::MouseMoveEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateOnTranslationHandle, vtkCommand::Move3DEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateOnRotationHandle, vtkCommand::MouseMoveEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateOnRotationHandle, vtkCommand::Move3DEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateOnScaleHandle, vtkCommand::MouseMoveEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateOnScaleHandle, vtkCommand::Move3DEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
}

//----------------------------------------------------------------------
vtkCjyxMarkupsWidget::~vtkCjyxMarkupsWidget() = default;

//-------------------------------------------------------------------------
bool vtkCjyxMarkupsWidget::ProcessControlPointMoveStart(vtkDMMLInteractionEventData* eventData)
{
  if (this->WidgetState != vtkCjyxMarkupsWidget::WidgetStateOnWidget)
    {
    return false;
    }
  int activeControlPoint = this->GetActiveControlPoint();
  if (activeControlPoint < 0)
    {
    return false;
    }
  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return false;
    }
  // Do not reject this event if control point is locked
  // because then we would not receive the mouse release event
  // and so we could not process mouse clicks.
  this->SetWidgetState(WidgetStateTranslateControlPoint);
  this->StartWidgetInteraction(eventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkCjyxMarkupsWidget::ProcessControlPointInsert(vtkDMMLInteractionEventData* vtkNotUsed(eventData))
{
  // Can be implemented in derived classes
  return false;
}

//----------------------------------------------------------------------
bool vtkCjyxMarkupsWidget::ProcessWidgetRotateStart(vtkDMMLInteractionEventData* eventData)
{
  if ((this->WidgetState != vtkCjyxMarkupsWidget::WidgetStateOnWidget && this->WidgetState != vtkCjyxMarkupsWidget::WidgetStateOnRotationHandle)
    || this->IsAnyControlPointLocked())
    {
    return false;
    }

  this->SetWidgetState(WidgetStateRotate);
  this->StartWidgetInteraction(eventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkCjyxMarkupsWidget::ProcessWidgetScaleStart(vtkDMMLInteractionEventData* eventData)
{
  if ((this->WidgetState != vtkCjyxMarkupsWidget::WidgetStateOnWidget && this->WidgetState != vtkCjyxMarkupsWidget::WidgetStateOnScaleHandle)
    || this->IsAnyControlPointLocked())
    {
    return false;
    }

  this->SetWidgetState(WidgetStateScale);
  this->StartWidgetInteraction(eventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkCjyxMarkupsWidget::ProcessWidgetTranslateStart(vtkDMMLInteractionEventData* eventData)
{
  if ((this->WidgetState != vtkCjyxMarkupsWidget::WidgetStateOnWidget && this->WidgetState != vtkCjyxMarkupsWidget::WidgetStateOnTranslationHandle)
    || this->IsAnyControlPointLocked())
    {
    return false;
    }

  this->SetWidgetState(WidgetStateTranslate);
  this->StartWidgetInteraction(eventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkCjyxMarkupsWidget::ProcessMouseMove(vtkDMMLInteractionEventData* eventData)
{
  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  vtkCjyxMarkupsWidgetRepresentation* rep = this->GetMarkupsRepresentation();
  if (!rep || !markupsNode || !eventData)
    {
    return false;
    }

  int state = this->WidgetState;

  if (state == vtkCjyxMarkupsWidget::WidgetStateDefine)
    {
    const char* associatedNodeID = this->GetAssociatedNodeID(eventData);
    int positionPreviewState = vtkDMMLMarkupsNode::PositionPreview;
    this->UpdatePreviewPointIndex(eventData);
    if (PreviewPointIndex >= 0)
      {
      positionPreviewState = markupsNode->GetNthControlPointPositionStatus(PreviewPointIndex);
      }
    this->UpdatePreviewPoint(eventData, associatedNodeID, positionPreviewState);
    }
  else if (state == WidgetStateIdle
    || state == WidgetStateOnWidget
    || state == WidgetStateOnTranslationHandle
    || state == WidgetStateOnRotationHandle
    || state == WidgetStateOnScaleHandle)
    {
    // update state
    int foundComponentType = vtkDMMLMarkupsDisplayNode::ComponentNone;
    int foundComponentIndex = -1;
    double closestDistance2 = 0.0;
    rep->CanInteract(eventData, foundComponentType, foundComponentIndex, closestDistance2);
    if (foundComponentType == vtkDMMLMarkupsDisplayNode::ComponentNone)
      {
      this->SetWidgetState(WidgetStateIdle);
      }
    else if (foundComponentType == vtkDMMLMarkupsDisplayNode::ComponentTranslationHandle)
      {
      this->SetWidgetState(WidgetStateOnTranslationHandle);
      }
    else if (foundComponentType == vtkDMMLMarkupsDisplayNode::ComponentRotationHandle)
      {
      this->SetWidgetState(WidgetStateOnRotationHandle);
      }
    else if (foundComponentType == vtkDMMLMarkupsDisplayNode::ComponentScaleHandle)
      {
      this->SetWidgetState(WidgetStateOnScaleHandle);
      }
    else
      {
      this->SetWidgetState(WidgetStateOnWidget);
      }

    this->GetMarkupsDisplayNode()->SetActiveComponent(foundComponentType, foundComponentIndex, eventData->GetInteractionContextName());
    }
  else
    {
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
    if (state == WidgetStateTranslateControlPoint)
      {
      this->TranslatePoint(eventPos);
      }
    else if (state == WidgetStateTranslate)
      {
      this->TranslateWidget(eventPos);
      }
    else if (state == WidgetStateScale)
      {
      this->ScaleWidget(eventPos);
      }
    else if (state == WidgetStateRotate)
      {
      this->RotateWidget(eventPos);
      }

    if (markupsNode->GetCurveClosed())
      {
      rep->UpdateCenterOfRotation();
      }

    this->LastEventPosition[0] = eventPos[0];
    this->LastEventPosition[1] = eventPos[1];
    }

  return true;
}

//-------------------------------------------------------------------------
bool vtkCjyxMarkupsWidget::ProcessEndMouseDrag(vtkDMMLInteractionEventData* eventData)
{
  if (!this->WidgetRep)
    {
    return false;
    }

  if ((this->WidgetState != vtkCjyxMarkupsWidget::WidgetStateTranslateControlPoint
    && this->WidgetState != vtkCjyxMarkupsWidget::WidgetStateTranslate
    && this->WidgetState != vtkCjyxMarkupsWidget::WidgetStateScale
    && this->WidgetState != vtkCjyxMarkupsWidget::WidgetStateRotate
    ) || !this->WidgetRep)
    {
    return false;
    }

  int activeComponentType = this->GetActiveComponentType();
  if (activeComponentType == vtkDMMLMarkupsDisplayNode::ComponentTranslationHandle)
    {
    this->SetWidgetState(WidgetStateOnTranslationHandle);
    }
  else if (activeComponentType == vtkDMMLMarkupsDisplayNode::ComponentRotationHandle)
    {
    this->SetWidgetState(WidgetStateOnRotationHandle);
    }
  else if (activeComponentType == vtkDMMLMarkupsDisplayNode::ComponentScaleHandle)
    {
    this->SetWidgetState(WidgetStateOnScaleHandle);
    }
  else
    {
    this->SetWidgetState(WidgetStateOnWidget);
    }

  this->EndWidgetInteraction();

  // only claim this as processed if the mouse was moved (this lets the event interpreted as button click)
  bool processedEvent = eventData->GetMouseMovedSinceButtonDown();
  return processedEvent;
}

//-------------------------------------------------------------------------
bool vtkCjyxMarkupsWidget::ProcessWidgetReset(vtkDMMLInteractionEventData* vtkNotUsed(eventData))
{
  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return false;
    }
  markupsNode->GetScene()->SaveStateForUndo();
  markupsNode->RemoveAllControlPoints();
  return true;
}

//-------------------------------------------------------------------------
bool vtkCjyxMarkupsWidget::ProcessControlPointSnapToSlice(vtkDMMLInteractionEventData* eventData)
{
  vtkCjyxMarkupsWidgetRepresentation2D* rep2d = vtkCjyxMarkupsWidgetRepresentation2D::SafeDownCast(this->WidgetRep);
  if (!rep2d)
    {
    return false;
    }

  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return false;
    }

  markupsNode->GetScene()->SaveStateForUndo();
  double eventPos[2]
  {
    static_cast<double>(eventData->GetDisplayPosition()[0]),
    static_cast<double>(eventData->GetDisplayPosition()[1]),
  };
  this->StartWidgetInteraction(eventData);
  this->TranslatePoint(eventPos, true);
  return true;
}

//-------------------------------------------------------------------------
bool vtkCjyxMarkupsWidget::ProcessControlPointDelete(vtkDMMLInteractionEventData* vtkNotUsed(eventData))
{
  if (this->WidgetState != WidgetStateDefine && this->WidgetState != WidgetStateOnWidget)
    {
    return false;
    }

  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  vtkDMMLMarkupsDisplayNode* markupsDisplayNode = this->GetMarkupsDisplayNode();
  if (!markupsNode || !markupsDisplayNode)
    {
    return false;
    }
  std::vector<int> controlPointsToDelete;
  if (this->WidgetState == WidgetStateDefine)
    {
    controlPointsToDelete.push_back(markupsNode->GetNumberOfControlPoints() - 2);
    }
  else if (this->WidgetState == WidgetStateOnWidget)
    {
    markupsDisplayNode->GetActiveControlPoints(controlPointsToDelete);
    }
  if (controlPointsToDelete.empty())
    {
    return false;
    }

  markupsNode->GetScene()->SaveStateForUndo();

  for (std::vector<int>::iterator cpIt = controlPointsToDelete.begin(); cpIt != controlPointsToDelete.end(); ++cpIt)
    {
    int controlPointToDelete = (*cpIt);
    if (controlPointToDelete < 0 || controlPointToDelete >= markupsNode->GetNumberOfControlPoints())
      {
      continue;
      }
    markupsNode->RemoveNthControlPoint(controlPointToDelete);
    }

  return true;
}

//-------------------------------------------------------------------------
bool vtkCjyxMarkupsWidget::ProcessWidgetJumpCursor(vtkDMMLInteractionEventData* vtkNotUsed(eventData))
{
  if (this->WidgetState != WidgetStateOnWidget &&
    this->WidgetState != WidgetStateOnTranslationHandle &&
    this->WidgetState != WidgetStateOnRotationHandle &&
    this->WidgetState != WidgetStateOnScaleHandle)
    {
    return false;
    }

  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  vtkDMMLMarkupsDisplayNode* markupsDisplayNode = this->GetMarkupsDisplayNode();
  if (!markupsNode || !markupsDisplayNode)
    {
    return false;
    }

  int componentIndex = markupsDisplayNode->GetActiveComponentIndex();;
  int componentType = markupsDisplayNode->GetActiveComponentType();

  // Use first active control point for jumping //TODO: Have an 'even more active' point concept
  if (componentType == vtkDMMLMarkupsDisplayNode::ComponentControlPoint)
    {
    std::vector<int> activeControlPointIndices;
    markupsDisplayNode->GetActiveControlPoints(activeControlPointIndices);

    if (!activeControlPointIndices.empty())
      {
      componentIndex = activeControlPointIndices[0];
      }
    if (componentIndex < 0 || componentIndex >= markupsNode->GetNumberOfControlPoints())
      {
      return false;
      }
    }

  markupsNode->GetScene()->SaveStateForUndo();

  vtkNew<vtkDMMLInteractionEventData> jumpToPointEventData;
  jumpToPointEventData->SetType(vtkDMMLMarkupsDisplayNode::JumpToPointEvent);
  jumpToPointEventData->SetComponentType(componentType);
  jumpToPointEventData->SetComponentIndex(componentIndex);
  jumpToPointEventData->SetViewNode(this->WidgetRep->GetViewNode());

  if (componentType == vtkDMMLMarkupsDisplayNode::ComponentRotationHandle
    || componentType == vtkDMMLMarkupsDisplayNode::ComponentTranslationHandle
    || componentType == vtkDMMLMarkupsDisplayNode::ComponentScaleHandle)
    {
    // For interaction handle, send the position of the handle as well.
    // The position of the handle may be different in each view, so we need to get the position from the representation.
    vtkCjyxMarkupsWidgetRepresentation* rep = vtkCjyxMarkupsWidgetRepresentation::SafeDownCast(this->WidgetRep);
    if (rep)
      {
      double position_World[3] = { 0.0, 0.0, 0.0 };
      rep->GetInteractionHandlePositionWorld(componentType, markupsDisplayNode->GetActiveComponentIndex(), position_World);
      jumpToPointEventData->SetWorldPosition(position_World);
      }
    }

  markupsDisplayNode->InvokeEvent(vtkDMMLMarkupsDisplayNode::JumpToPointEvent, jumpToPointEventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkCjyxMarkupsWidget::ConvertDisplayPositionToWorld(const int displayPos[2],
  double worldPos[3], double worldOrientationMatrix[9], double* refWorldPos/*=nullptr*/)
{
  vtkCjyxMarkupsWidgetRepresentation2D* rep2d = vtkCjyxMarkupsWidgetRepresentation2D::SafeDownCast(this->WidgetRep);
  vtkCjyxMarkupsWidgetRepresentation3D* rep3d = vtkCjyxMarkupsWidgetRepresentation3D::SafeDownCast(this->WidgetRep);
  double doubleDisplayPos[3] = { static_cast<double>(displayPos[0]), static_cast<double>(displayPos[1]), 0.0 };
  if (rep2d)
    {
    // 2D view
    rep2d->GetSliceToWorldCoordinates(doubleDisplayPos, worldPos);
    return true;
    }
  else if (rep3d)
    {
    // 3D view
    bool preferPickOnSurface = true;
    if (refWorldPos != nullptr)
      {
      // If reference position is provided then we may use that instead of picking on visible surface.
      vtkDMMLMarkupsDisplayNode* markupsDisplayNode = this->GetMarkupsDisplayNode();
      if (markupsDisplayNode)
        {
        preferPickOnSurface = (markupsDisplayNode->GetSnapMode() == vtkDMMLMarkupsDisplayNode::SnapModeToVisibleSurface);
        }
      }
    if (preferPickOnSurface)
      {
      // SnapModeToVisibleSurface
      // Try to pick on surface and pick on camera plane if nothing is found.
      if (rep3d->AccuratePick(displayPos[0], displayPos[1], worldPos))
        {
        return true;
        }
      if (refWorldPos)
        {
        // Reference position is available (most likely, moving the point).
        return (rep3d->GetPointPlacer()->ComputeWorldPosition(this->Renderer,
          doubleDisplayPos, refWorldPos, worldPos, worldOrientationMatrix));
        }
      }
    else
      {
      // SnapModeUnconstrained
      // Move the point relative to reference position, not restricted to surfaces if possible.
      if (refWorldPos)
        {
        // Reference position is available (most likely, moving the point).
        return (rep3d->GetPointPlacer()->ComputeWorldPosition(this->Renderer,
          doubleDisplayPos, refWorldPos, worldPos, worldOrientationMatrix));
        }
      else
        {
        // Reference position is unavailable (e.g., not moving of an existing point but first placement)
        // Even if the constraining on the surface is no preferred, it is still better to
        // place it on a visible surface in 3D views rather on the .
        if (rep3d->AccuratePick(displayPos[0], displayPos[1], worldPos))
          {
          return true;
          }
        }
      }
    // Last resort: place a point on the camera plane
    // (no reference position is available and no surface is visible there)
    return (rep3d->GetPointPlacer()->ComputeWorldPosition(this->Renderer,
      doubleDisplayPos, worldPos, worldOrientationMatrix));
    }
  return false;
}

// -------------------------------------------------------------------------
void vtkCjyxMarkupsWidget::UpdatePreviewPointIndex(vtkDMMLInteractionEventData* vtkNotUsed(eventData))
{
    vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
    if (!markupsNode)
      {
      return;
      }
    int numberOfControlPoints = markupsNode->GetNumberOfControlPoints();
    for (int i = 0; i < numberOfControlPoints; i++)
      {
      int pointStatus = markupsNode->GetNthControlPointPositionStatus(i);
      if (pointStatus == vtkDMMLMarkupsNode::PositionPreview)
        {
        this->PreviewPointIndex = i;
        return;
        }
      }
    // if no preview points found, set to -1
    this->PreviewPointIndex = -1;
}

//-------------------------------------------------------------------------
void vtkCjyxMarkupsWidget::UpdatePreviewPoint(vtkDMMLInteractionEventData* eventData, const char* associatedNodeID, int positionStatus)
{
  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return;
    }

  // Get accurate world position
  double accurateWorldPos[3] = { 0.0 };
  double accurateWorldOrientationMatrix[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
  if (eventData->IsWorldPositionValid() && eventData->IsWorldPositionAccurate())
    {
    eventData->GetWorldPosition(accurateWorldPos);

    double worldOrientationQuaternion[4] = { 0.0 };
    eventData->GetWorldOrientation(worldOrientationQuaternion);
    vtkDMMLMarkupsNode::ConvertOrientationWXYZToMatrix(worldOrientationQuaternion, accurateWorldOrientationMatrix);
    }
  else if (eventData->IsDisplayPositionValid())
    {
    int displayPos[2] = { 0 };
    eventData->GetDisplayPosition(displayPos);
    if (!this->ConvertDisplayPositionToWorld(displayPos, accurateWorldPos, accurateWorldOrientationMatrix))
      {
      eventData->GetWorldPosition(accurateWorldPos);
      }
    }
  eventData->SetWorldPosition(accurateWorldPos);

  // Add/update control point position and orientation

  const char* viewNodeID = nullptr;
  if (this->WidgetRep && this->WidgetRep->GetViewNode())
    {
    viewNodeID = this->WidgetRep->GetViewNode()->GetID();
    }

  this->UpdatePreviewPointIndex(eventData);
  int updatePointID = this->PreviewPointIndex;
  if (updatePointID < 0)
    {
    updatePointID = markupsNode->GetControlPointPlacementStartIndex();
    }

  this->PreviewPointIndex = this->GetMarkupsDisplayNode()->UpdateActiveControlPointWorld(
    updatePointID, eventData, accurateWorldOrientationMatrix, viewNodeID,
    associatedNodeID, positionStatus);
}

//-------------------------------------------------------------------------
bool vtkCjyxMarkupsWidget::RemovePreviewPoint()
{
  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return false;
    }
  if (this->PreviewPointIndex < 0)
    {
    // no preview point
    return false;
    }
  int previewIndex = this->PreviewPointIndex;
  int status = markupsNode->GetNthControlPointPositionStatus(previewIndex);
  if (status == vtkDMMLMarkupsNode::PositionPreview)
    {
    if (markupsNode->GetNthControlPointAutoCreated(previewIndex))
      {
      markupsNode->RemoveNthControlPoint(previewIndex);
      markupsNode->GetMarkupsDisplayNode()->SetActiveControlPoint(-1);
      }
    else
      {
      markupsNode->UnsetNthControlPointPosition(previewIndex);
      }
    }
  this->PreviewPointIndex = -1;
  return true;
}

//----------------------------------------------------------------------
void vtkCjyxMarkupsWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "PreviewPointIndex: " << this->PreviewPointIndex << endl;
}

//-----------------------------------------------------------------------------
bool vtkCjyxMarkupsWidget::CanProcessInteractionEvent(vtkDMMLInteractionEventData* eventData, double &distance2)
{
  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);
  if (widgetEvent == WidgetEventNone)
    {
    // If this event is not recognized then give a chance to process it as a click event.
    return this->CanProcessButtonClickEvent(eventData, distance2);
    }
  vtkCjyxMarkupsWidgetRepresentation* rep = this->GetMarkupsRepresentation();
  if (!rep)
    {
    return false;
    }

  // If we are placing markups or dragging the mouse then we interact everywhere
  if (this->WidgetState == WidgetStateDefine
    || this->WidgetState == WidgetStateTranslateControlPoint
    || this->WidgetState == WidgetStateTranslate
    || this->WidgetState == WidgetStateRotate
    || this->WidgetState == WidgetStateScale)
    {
    distance2 = 0.0;
    return true;
    }

  int foundComponentType = vtkDMMLMarkupsDisplayNode::ComponentNone;
  int foundComponentIndex = -1;
  double closestDistance2 = 0.0;
  rep->CanInteract(eventData, foundComponentType, foundComponentIndex, closestDistance2);
  if (foundComponentType == vtkDMMLMarkupsDisplayNode::ComponentNone)
    {
    return false;
    }
  distance2 = closestDistance2;
  return true;
}

//-------------------------------------------------------------------------
bool vtkCjyxMarkupsWidget::ProcessWidgetMenu(vtkDMMLInteractionEventData* eventData)
{
  if (this->WidgetState != WidgetStateOnWidget &&
       this->WidgetState != WidgetStateOnTranslationHandle &&
       this->WidgetState != WidgetStateOnRotationHandle &&
       this->WidgetState != WidgetStateOnScaleHandle)
    {
    return false;
    }

  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  vtkDMMLMarkupsDisplayNode* markupsDisplayNode = this->GetMarkupsDisplayNode();
  if (!markupsNode || !markupsDisplayNode)
    {
    return false;
    }

  vtkNew<vtkDMMLInteractionEventData> menuEventData;
  menuEventData->SetType(vtkDMMLDisplayNode::MenuEvent);
  menuEventData->SetComponentType(markupsDisplayNode->GetActiveComponentType()); //TODO: This will always pass the active component for the mouse
  menuEventData->SetComponentIndex(markupsDisplayNode->GetActiveComponentIndex());
  menuEventData->SetViewNode(this->WidgetRep->GetViewNode());

  // Copy display position
  if (eventData->IsDisplayPositionValid())
    {
    menuEventData->SetDisplayPosition(eventData->GetDisplayPosition());
    }

  // Copy/compute world position
  double worldPos[3] = { 0.0 };
  if (eventData->IsWorldPositionValid())
    {
    eventData->GetWorldPosition(worldPos);
    menuEventData->SetWorldPosition(worldPos, eventData->IsWorldPositionAccurate());
    }
  else if (eventData->IsDisplayPositionValid())
    {
    double worldOrientationMatrix[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
    int displayPos[2] = { 0 };
    eventData->GetDisplayPosition(displayPos);
    if (this->ConvertDisplayPositionToWorld(displayPos, worldPos, worldOrientationMatrix))
      {
      menuEventData->SetWorldPosition(worldPos);
      }
    }

  markupsDisplayNode->InvokeEvent(vtkDMMLDisplayNode::MenuEvent, menuEventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkCjyxMarkupsWidget::ProcessWidgetAction(vtkDMMLInteractionEventData* eventData)
{
  if (this->WidgetState != WidgetStateOnWidget)
    {
    return false;
    }
  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  vtkDMMLMarkupsDisplayNode* markupsDisplayNode = this->GetMarkupsDisplayNode();
  if (!markupsNode || !markupsDisplayNode)
    {
    return false;
    }
  // Use first active control point for jumping //TODO: Have an 'even more active' point concept
  std::vector<int> activeControlPointIndices;
  markupsDisplayNode->GetActiveControlPoints(activeControlPointIndices);
  int controlPointIndex = -1;
  if (!activeControlPointIndices.empty())
    {
    controlPointIndex = activeControlPointIndices[0];
    }
  if (controlPointIndex < 0 || controlPointIndex >= markupsNode->GetNumberOfControlPoints())
    {
    return false;
    }
  markupsNode->GetScene()->SaveStateForUndo();

  // Convert widget action to display node event
  unsigned long displayNodeEvent = vtkDMMLMarkupsDisplayNode::ActionEvent;
  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);
  switch (widgetEvent)
  {
  case WidgetEventCustomAction1: displayNodeEvent = vtkDMMLMarkupsDisplayNode::CustomActionEvent1; break;
  case WidgetEventCustomAction2: displayNodeEvent = vtkDMMLMarkupsDisplayNode::CustomActionEvent2; break;
  case WidgetEventCustomAction3: displayNodeEvent = vtkDMMLMarkupsDisplayNode::CustomActionEvent3; break;
  case WidgetEventCustomAction4: displayNodeEvent = vtkDMMLMarkupsDisplayNode::CustomActionEvent4; break;
  case WidgetEventCustomAction5: displayNodeEvent = vtkDMMLMarkupsDisplayNode::CustomActionEvent5; break;
  case WidgetEventCustomAction6: displayNodeEvent = vtkDMMLMarkupsDisplayNode::CustomActionEvent6; break;
  case WidgetEventAction:
  default:
    displayNodeEvent = vtkDMMLMarkupsDisplayNode::ActionEvent;
    break;
  }

  vtkNew<vtkDMMLInteractionEventData> actionEventData;
  actionEventData->SetType(displayNodeEvent);
  actionEventData->SetComponentType(vtkDMMLMarkupsDisplayNode::ComponentControlPoint);
  actionEventData->SetComponentIndex(controlPointIndex);
  actionEventData->SetViewNode(this->WidgetRep->GetViewNode());
  markupsDisplayNode->InvokeEvent(displayNodeEvent, actionEventData);
  return true;
}

//-----------------------------------------------------------------------------
bool vtkCjyxMarkupsWidget::ProcessWidgetStopPlace(vtkDMMLInteractionEventData* vtkNotUsed(eventData))
{
    vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
    if (!markupsNode)
      {
      return false;
      }
    markupsNode->Modified();
    return true;
}
//-----------------------------------------------------------------------------

bool vtkCjyxMarkupsWidget::ProcessInteractionEvent(vtkDMMLInteractionEventData* eventData)
{
  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);

  if (this->ApplicationLogic)
    {
    this->ApplicationLogic->PauseRender();
    }


  bool processedEvent = false;
  switch (widgetEvent)
    {
    case WidgetEventControlPointPlace:
      processedEvent = this->PlacePoint(eventData);
      break;
    case WidgetEventStopPlace:
      // cancel point placement
      this->GetInteractionNode()->SwitchToViewTransformMode();
      processedEvent = ProcessWidgetStopPlace(eventData);
      break;
    case WidgetEventMouseMove:
      processedEvent = ProcessMouseMove(eventData);
      break;
    case WidgetEventMenu:
      processedEvent = ProcessWidgetMenu(eventData);
      break;
    case WidgetEventAction:
    case WidgetEventCustomAction1:
    case WidgetEventCustomAction2:
    case WidgetEventCustomAction3:
      processedEvent = ProcessWidgetAction(eventData);
      break;
    case WidgetEventControlPointSnapToSlice:
      processedEvent = ProcessControlPointSnapToSlice(eventData);
      break;
    case WidgetEventControlPointDelete:
      processedEvent = ProcessControlPointDelete(eventData);
      break;
    case WidgetEventControlPointMoveStart:
      processedEvent = ProcessControlPointMoveStart(eventData);
      break;
    case WidgetEventControlPointInsert:
      processedEvent = ProcessControlPointInsert(eventData);
      break;
    case WidgetEventControlPointMoveEnd:
      processedEvent = ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventTranslateStart:
      processedEvent = ProcessWidgetTranslateStart(eventData);
      break;
    case WidgetEventTranslateEnd:
      processedEvent = ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventRotateStart:
      processedEvent = ProcessWidgetRotateStart(eventData);
      break;
    case WidgetEventRotateEnd:
      processedEvent = ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventScaleStart:
      processedEvent = ProcessWidgetScaleStart(eventData);
      break;
    case WidgetEventScaleEnd:
      processedEvent = ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventReset:
      processedEvent = ProcessWidgetReset(eventData);
      break;
    case WidgetEventJumpCursor:
      processedEvent = ProcessWidgetJumpCursor(eventData);
      break;
    }

  if (!processedEvent)
    {
    processedEvent = this->ProcessButtonClickEvent(eventData);
    }

  if (this->ApplicationLogic)
    {
    this->ApplicationLogic->ResumeRender();
    }

  return processedEvent;
}

//-----------------------------------------------------------------------------
void vtkCjyxMarkupsWidget::Leave(vtkDMMLInteractionEventData* eventData)
{
  this->RemovePreviewPoint();

  // Ensure that EndInteractionEvent is invoked, even if interrupted by an unexpected event
  if (this->WidgetState == vtkCjyxMarkupsWidget::WidgetStateTranslateControlPoint
    || this->WidgetState == vtkCjyxMarkupsWidget::WidgetStateTranslate
    || this->WidgetState == vtkCjyxMarkupsWidget::WidgetStateScale
    || this->WidgetState == vtkCjyxMarkupsWidget::WidgetStateRotate)
    {
    this->EndWidgetInteraction();
    }

  vtkDMMLMarkupsDisplayNode* markupsDisplayNode = this->GetMarkupsDisplayNode();
  if (markupsDisplayNode)
    {
    std::string interactionContext("");
    if (eventData)
      {
      interactionContext = eventData->GetInteractionContextName();
      }
    markupsDisplayNode->SetActiveComponent(vtkDMMLMarkupsDisplayNode::ComponentNone, -1, interactionContext);
    }
  Superclass::Leave(eventData);
}

//----------------------------------------------------------------------
void vtkCjyxMarkupsWidget::StartWidgetInteraction(vtkDMMLInteractionEventData* eventData)
{
  vtkCjyxMarkupsWidgetRepresentation* rep = this->GetMarkupsRepresentation();
  if (!rep)
    {
    return;
    }
  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  vtkDMMLMarkupsDisplayNode* markupsDisplayNode = this->GetMarkupsDisplayNode();
  if (!markupsNode || !markupsDisplayNode)
    {
    return;
    }

  markupsNode->GetScene()->SaveStateForUndo();

  double startEventPos[2]
    {
    static_cast<double>(eventData->GetDisplayPosition()[0]),
    static_cast<double>(eventData->GetDisplayPosition()[1])
    };

  // save the cursor position
  this->LastEventPosition[0] = startEventPos[0];
  this->LastEventPosition[1] = startEventPos[1];

  // Use first active control point for jumping //TODO: Have an 'even more active' point concept
  std::vector<int> activeControlPointIndices;
  markupsDisplayNode->GetActiveControlPoints(activeControlPointIndices);
  int activeControlPointIndex = -1;
  if (!activeControlPointIndices.empty())
    {
    activeControlPointIndex = activeControlPointIndices[0];
    }
  if (activeControlPointIndex >= 0 || activeControlPointIndex < markupsNode->GetNumberOfControlPoints())
    {
    // How far is this in pixels from the position of this widget?
    // Maintain this during interaction such as translating (don't
    // force center of widget to snap to mouse position)
    double pos[2] = { 0.0 };
    if (rep->GetNthControlPointDisplayPosition(activeControlPointIndex, pos))
      {
      // save offset
      this->StartEventOffsetPosition[0] = startEventPos[0] - pos[0];
      this->StartEventOffsetPosition[1] = startEventPos[1] - pos[1];
      }
    // AddControlPoint will fire modified events anyway, so we temporarily disable events
    // to add a new point with a minimum number of events.
    bool wasDisabled = markupsNode->GetDisableModifiedEvent();
    markupsNode->DisableModifiedEventOn();
    const char* layoutName = nullptr;
    if (rep->GetViewNode() && rep->GetViewNode()->GetLayoutName())
      {
      layoutName = rep->GetViewNode()->GetLayoutName();
      }
    markupsNode->SetAttribute("Markups.MovingInSliceView", layoutName ? layoutName : "");
    std::ostringstream controlPointIndexStr;
    controlPointIndexStr << activeControlPointIndex;
    markupsNode->SetAttribute("Markups.MovingMarkupIndex", controlPointIndexStr.str().c_str());
    markupsNode->SetDisableModifiedEvent(wasDisabled);
    markupsNode->InvokeCustomModifiedEvent(vtkDMMLMarkupsNode::PointStartInteractionEvent);
    }
  else
    {
    // Picking line or something else - not handled in this base class
    this->StartEventOffsetPosition[0] = 0;
    this->StartEventOffsetPosition[1] = 0;
    }
}

//----------------------------------------------------------------------
void vtkCjyxMarkupsWidget::EndWidgetInteraction()
{
  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return;
    }
  markupsNode->InvokeCustomModifiedEvent(vtkDMMLMarkupsNode::PointEndInteractionEvent);
  // AddControlPoint will fire modified events anyway, so we temporarily disable events
  // to add a new point with a minimum number of events.
  bool wasDisabled = markupsNode->GetDisableModifiedEvent();
  markupsNode->DisableModifiedEventOn();
  markupsNode->SetAttribute("Markups.MovingInSliceView", "");
  markupsNode->SetAttribute("Markups.MovingMarkupIndex", "");
  markupsNode->SetDisableModifiedEvent(wasDisabled);
}

//----------------------------------------------------------------------
void vtkCjyxMarkupsWidget::TranslatePoint(double eventPos[2], bool snapToSlice /* = false*/)
{
  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  vtkDMMLMarkupsDisplayNode* markupsDisplayNode = this->GetMarkupsDisplayNode();
  if (!markupsNode || !markupsDisplayNode)
    {
    return;
    }

  // Use first active control point for jumping //TODO: Have an 'even more active' point concept
  std::vector<int> activeControlPointIndices;
  markupsDisplayNode->GetActiveControlPoints(activeControlPointIndices);
  int activeControlPointIndex = -1;
  if (!activeControlPointIndices.empty())
    {
    activeControlPointIndex = activeControlPointIndices[0];
    }
  if (activeControlPointIndex < 0 || activeControlPointIndex >= markupsNode->GetNumberOfControlPoints())
    {
    return;
    }

  if (markupsNode->GetNthControlPointLocked(activeControlPointIndex))
    {
    // point is locked, do not translate
    return;
    }

  eventPos[0] -= this->StartEventOffsetPosition[0];
  eventPos[1] -= this->StartEventOffsetPosition[1];

  double oldWorldPos[3] = { 0.0 };
  markupsNode->GetNthControlPointPositionWorld(activeControlPointIndex, oldWorldPos);

  // Get accurate world position
  int displayPos[2] = { int(eventPos[0] + 0.5), int(eventPos[1] + 0.5) };
  double worldPos[3] = { 0.0 };
  double worldOrientationMatrix[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
  if (!this->ConvertDisplayPositionToWorld(displayPos, worldPos, worldOrientationMatrix, oldWorldPos))
    {
    vtkCjyxMarkupsWidgetRepresentation* rep = this->GetMarkupsRepresentation();
    if (!rep || !rep->GetPointPlacer()->ComputeWorldPosition(this->Renderer, eventPos, oldWorldPos, worldPos, worldOrientationMatrix))
      {
      return;
      }
    }

  vtkCjyxMarkupsWidgetRepresentation2D* rep2d = vtkCjyxMarkupsWidgetRepresentation2D::SafeDownCast(this->WidgetRep);
  if (rep2d && markupsDisplayNode->GetSliceProjection() && !snapToSlice)
    {
    double doubleDisplayPos[2], oldWorldPos2[3];
    rep2d->GetWorldToSliceCoordinates(oldWorldPos, doubleDisplayPos);
    rep2d->GetSliceToWorldCoordinates(doubleDisplayPos, oldWorldPos2);
    double worldPosProjDiff[3] =
    {
      oldWorldPos[0] - oldWorldPos2[0],
      oldWorldPos[1] - oldWorldPos2[1],
      oldWorldPos[2] - oldWorldPos2[2],
    };

    worldPos[0] += worldPosProjDiff[0];
    worldPos[1] += worldPosProjDiff[1];
    worldPos[2] += worldPosProjDiff[2];
    }

  markupsNode->SetNthControlPointPositionWorld(activeControlPointIndex, worldPos);
}

//----------------------------------------------------------------------
void vtkCjyxMarkupsWidget::TranslateWidget(double eventPos[2])
{
  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return;
    }

  double lastEventPos_World[3] = { 0.0 };
  double eventPos_World[3] = { 0.0 };
  double orientation_World[9] = { 0.0 };

  vtkCjyxMarkupsWidgetRepresentation* rep = vtkCjyxMarkupsWidgetRepresentation::SafeDownCast(this->WidgetRep);
  vtkCjyxMarkupsWidgetRepresentation2D* rep2d = vtkCjyxMarkupsWidgetRepresentation2D::SafeDownCast(this->WidgetRep);
  vtkCjyxMarkupsWidgetRepresentation3D* rep3d = vtkCjyxMarkupsWidgetRepresentation3D::SafeDownCast(this->WidgetRep);
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

  double translationVector_World[3];
  translationVector_World[0] = eventPos_World[0] - lastEventPos_World[0];
  translationVector_World[1] = eventPos_World[1] - lastEventPos_World[1];
  translationVector_World[2] = eventPos_World[2] - lastEventPos_World[2];
  int type = this->GetActiveComponentType();
  if (type == vtkDMMLMarkupsDisplayNode::ComponentTranslationHandle && this->GetMarkupsDisplayNode())
    {
    int index = this->GetMarkupsDisplayNode()->GetActiveComponentIndex();

    double translationAxis_World[3] = { 0 };
    rep->GetInteractionHandleAxisWorld(type, index, translationAxis_World);

    // Only perform constrained translation if the length of the axis is non-zero.
    if (vtkMath::Norm(translationAxis_World) > 0)
      {
      double lastEventPositionOnAxis_World[3] = { 0.0, 0.0, 0.0 };
      this->GetClosestPointOnInteractionAxis(
        vtkDMMLMarkupsDisplayNode::ComponentTranslationHandle, index, this->LastEventPosition, lastEventPositionOnAxis_World);

      double eventPositionOnAxis_World[3] = { 0.0, 0.0, 0.0 };
      this->GetClosestPointOnInteractionAxis(
        vtkDMMLMarkupsDisplayNode::ComponentTranslationHandle, index, eventPos, eventPositionOnAxis_World);

      vtkMath::Subtract(eventPositionOnAxis_World, lastEventPositionOnAxis_World, translationVector_World);
      double distance = vtkMath::Norm(translationVector_World);
      if (vtkMath::Dot(translationVector_World, translationAxis_World) < 0)
        {
        distance *= -1.0;
        }
      translationVector_World[0] = distance * translationAxis_World[0];
      translationVector_World[1] = distance * translationAxis_World[1];
      translationVector_World[2] = distance * translationAxis_World[2];
      }
    }

  vtkNew<vtkTransform> translationTransform;
  translationTransform->Translate(translationVector_World);

  vtkNew<vtkPoints> transformedPoints_World;
  transformedPoints_World->SetNumberOfPoints(markupsNode->GetNumberOfControlPoints());
  for (int i = 0; i < markupsNode->GetNumberOfControlPoints(); i++)
    {
    double currentControlPointPosition_World[3] = { 0.0 };
    markupsNode->GetNthControlPointPositionWorld(i, currentControlPointPosition_World);

    double newControlPointPosition_World[3] = { 0.0 };
    translationTransform->TransformPoint(currentControlPointPosition_World, newControlPointPosition_World);
    transformedPoints_World->SetPoint(i, newControlPointPosition_World);
    }
  markupsNode->SetControlPointPositionsWorld(transformedPoints_World);

  if (transformedPoints_World->GetNumberOfPoints() == 0)
    {
    translationTransform->Concatenate(markupsNode->GetInteractionHandleToWorldMatrix());
    markupsNode->GetInteractionHandleToWorldMatrix()->DeepCopy(translationTransform->GetMatrix());
    }
}

//----------------------------------------------------------------------
void vtkCjyxMarkupsWidget::ScaleWidget(double eventPos[2])
{
  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return;
    }

  double center[3] = { 0. };
  double ref[3] = { 0. };
  double worldPos[3], worldOrient[9];

  vtkCjyxMarkupsWidgetRepresentation2D* rep2d = vtkCjyxMarkupsWidgetRepresentation2D::SafeDownCast(this->WidgetRep);
  vtkCjyxMarkupsWidgetRepresentation3D* rep3d = vtkCjyxMarkupsWidgetRepresentation3D::SafeDownCast(this->WidgetRep);
  if (rep2d)
    {
    double slicePos[3] = { 0. };
    slicePos[0] = this->LastEventPosition[0];
    slicePos[1] = this->LastEventPosition[1];
    rep2d->GetSliceToWorldCoordinates(slicePos, ref);

    slicePos[0] = eventPos[0];
    slicePos[1] = eventPos[1];
    rep2d->GetSliceToWorldCoordinates(slicePos, worldPos);

    rep2d->GetTransformationReferencePoint(center);
    }
  else if (rep3d)
    {
    double displayPos[2] = { 0. };
    displayPos[0] = this->LastEventPosition[0];
    displayPos[1] = this->LastEventPosition[1];
    if (rep3d->GetPointPlacer()->ComputeWorldPosition(this->Renderer,
      displayPos, ref, worldPos,
      worldOrient))
      {
      for (int i = 0; i < 3; i++)
        {
        ref[i] = worldPos[i];
        }
      }
    else
      {
      return;
      }
    displayPos[0] = eventPos[0];
    displayPos[1] = eventPos[1];

    if (!rep3d->GetPointPlacer()->ComputeWorldPosition(this->Renderer,
      displayPos, ref, worldPos,
      worldOrient))
      {
      return;
      }

    rep3d->GetTransformationReferencePoint(center);
    }

  double r2 = vtkMath::Distance2BetweenPoints(ref, center);
  double d2 = vtkMath::Distance2BetweenPoints(worldPos, center);
  if (d2 < 0.0000001)
    {
    return;
    }

  double ratio = sqrt(d2 / r2);

  vtkNew<vtkPoints> transformedPoints_World;
  transformedPoints_World->SetNumberOfPoints(markupsNode->GetNumberOfControlPoints());
  for (int i = 0; i < markupsNode->GetNumberOfControlPoints(); i++)
    {
    markupsNode->GetNthControlPointPositionWorld(i, ref);
    for (int j = 0; j < 3; j++)
      {
      worldPos[j] = center[j] + ratio * (ref[j] - center[j]);
      }
    transformedPoints_World->SetPoint(i, worldPos);
    }
  markupsNode->SetControlPointPositionsWorld(transformedPoints_World);
}

//----------------------------------------------------------------------
void vtkCjyxMarkupsWidget::RotateWidget(double eventPos[2])
{
  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return;
    }

  double eventPos_World[3] = { 0. };
  double lastEventPos_World[3] = { 0. };
  double orientation_World[9] = { 0. };
  double eventPos_Display[2] = { 0. };

  vtkCjyxMarkupsWidgetRepresentation* rep = vtkCjyxMarkupsWidgetRepresentation::SafeDownCast(this->WidgetRep);
  vtkCjyxMarkupsWidgetRepresentation2D* rep2d = vtkCjyxMarkupsWidgetRepresentation2D::SafeDownCast(this->WidgetRep);
  vtkCjyxMarkupsWidgetRepresentation3D* rep3d = vtkCjyxMarkupsWidgetRepresentation3D::SafeDownCast(this->WidgetRep);
  if (rep2d)
    {
    double eventPos_Slice[3] = { 0. };
    eventPos_Slice[0] = this->LastEventPosition[0];
    eventPos_Slice[1] = this->LastEventPosition[1];
    rep2d->GetSliceToWorldCoordinates(eventPos_Slice, lastEventPos_World);

    eventPos_Slice[0] = eventPos[0];
    eventPos_Slice[1] = eventPos[1];
    rep2d->GetSliceToWorldCoordinates(eventPos_Slice, eventPos_World);

    eventPos_Display[0] = eventPos_Slice[0];
    eventPos_Display[1] = eventPos_Slice[1];
    }
  else if (rep3d)
    {
    eventPos_Display[0] = this->LastEventPosition[0];
    eventPos_Display[1] = this->LastEventPosition[1];

    if (rep3d->GetPointPlacer()->ComputeWorldPosition(this->Renderer,
      eventPos_Display, eventPos_World, lastEventPos_World,
      orientation_World))
      {
      for (int i = 0; i < 3; i++)
        {
        eventPos_World[i] = lastEventPos_World[i];
        }
      }
    else
      {
      return;
      }
    eventPos_Display[0] = eventPos[0];
    eventPos_Display[1] = eventPos[1];

    if (!rep3d->GetPointPlacer()->ComputeWorldPosition(this->Renderer,
      eventPos_Display, eventPos_World, eventPos_World,
      orientation_World))
      {
      return;
      }
    }

  double origin_World[3] = { 0.0 };
  rep->GetInteractionHandleOriginWorld(origin_World);

  double epsilon = 1e-5;
  double d2 = vtkMath::Distance2BetweenPoints(eventPos_World, origin_World);
  if (d2 < epsilon)
    {
    return;
    }

  for (int i = 0; i < 3; i++)
    {
    lastEventPos_World[i] -= origin_World[i];
    eventPos_World[i] -= origin_World[i];
    }

  double angle = vtkMath::DegreesFromRadians(
    vtkMath::AngleBetweenVectors(lastEventPos_World, eventPos_World));
  double rotationNormal_World[3] = { 0.0 };
  vtkMath::Cross(lastEventPos_World, eventPos_World, rotationNormal_World);
  double rotationAxis_World[3] = { 0.0, 1.0, 0.0 };
  int type = this->GetActiveComponentType();
  if (type == vtkDMMLMarkupsDisplayNode::ComponentRotationHandle)
    {
    int index = this->GetMarkupsDisplayNode()->GetActiveComponentIndex();
    double eventPositionOnAxisPlane_World[3] = { 0.0, 0.0, 0.0 };
    if (!this->GetIntersectionOnAxisPlane(type, index, eventPos, eventPositionOnAxisPlane_World))
      {
      vtkWarningMacro("RotateWidget: Could not calculate intended orientation");
      return;
      }

    rep->GetInteractionHandleAxisWorld(type, index, rotationAxis_World); // Axis of rotation
    double origin_World[3] = { 0.0, 0.0, 0.0 };
    rep->GetInteractionHandleOriginWorld(origin_World);

    double lastEventPositionOnAxisPlane_World[3] = { 0.0, 0.0, 0.0 };
    if (!this->GetIntersectionOnAxisPlane(
      vtkDMMLMarkupsDisplayNode::ComponentRotationHandle, index, this->LastEventPosition,lastEventPositionOnAxisPlane_World))
      {
      vtkWarningMacro("RotateWidget: Could not calculate previous orientation");
      return;
      }

    double rotationHandleVector_World[3] = { 0.0, 0.0, 0.0 };
    vtkMath::Subtract(lastEventPositionOnAxisPlane_World, origin_World, rotationHandleVector_World);

    double destinationVector_World[3] = { 0.0, 0.0, 0.0 };
    vtkMath::Subtract(eventPositionOnAxisPlane_World, origin_World, destinationVector_World);

    angle = vtkMath::DegreesFromRadians(vtkMath::AngleBetweenVectors(rotationHandleVector_World, destinationVector_World));
    vtkMath::Cross(rotationHandleVector_World, destinationVector_World, rotationNormal_World);
    }
  else
    {
    rotationAxis_World[0] = rotationNormal_World[0];
    rotationAxis_World[1] = rotationNormal_World[1];
    rotationAxis_World[2] = rotationNormal_World[2];
    }

  if (vtkMath::Dot(rotationNormal_World, rotationAxis_World) < 0.0)
    {
    angle *= -1.0;
    }

  vtkNew<vtkTransform> rotateTransform;
  rotateTransform->Translate(origin_World);
  rotateTransform->RotateWXYZ(angle, rotationAxis_World);
  rotateTransform->Translate(-origin_World[0], -origin_World[1], -origin_World[2]);

  DMMLNodeModifyBlocker blocker(markupsNode);

  // The orientation of some markup types are not fully defined by their control points (line, etc.).
  // For these cases, we need to manually apply a rotation to the interaction handles.
  vtkNew<vtkTransform> handleToWorldTransform;
  handleToWorldTransform->PostMultiply();
  handleToWorldTransform->Concatenate(markupsNode->GetInteractionHandleToWorldMatrix());
  handleToWorldTransform->Translate(-origin_World[0], -origin_World[1], -origin_World[2]);
  handleToWorldTransform->RotateWXYZ(angle, rotationAxis_World);
  handleToWorldTransform->Translate(origin_World[0], origin_World[1], origin_World[2]);
  markupsNode->GetInteractionHandleToWorldMatrix()->DeepCopy(handleToWorldTransform->GetMatrix());

  vtkNew<vtkPoints> transformedPoints_World;
  transformedPoints_World->SetNumberOfPoints(markupsNode->GetNumberOfControlPoints());
  for (int i = 0; i < markupsNode->GetNumberOfControlPoints(); i++)
    {
    double currentControlPointPosition_World[3] = { 0.0 };
    markupsNode->GetNthControlPointPositionWorld(i, currentControlPointPosition_World);

    double newControlPointPosition_World[3] = { 0.0 };
    rotateTransform->TransformPoint(currentControlPointPosition_World, newControlPointPosition_World);
    transformedPoints_World->SetPoint(i, newControlPointPosition_World);
    }
  markupsNode->SetControlPointPositionsWorld(transformedPoints_World);
}

//----------------------------------------------------------------------
bool vtkCjyxMarkupsWidget::GetIntersectionOnAxisPlane(int type, int index, const double input_Display[2], double outputIntersection_World[3])
{
  vtkCjyxMarkupsWidgetRepresentation* rep = vtkCjyxMarkupsWidgetRepresentation::SafeDownCast(this->WidgetRep);
  vtkCjyxMarkupsWidgetRepresentation2D* rep2d = vtkCjyxMarkupsWidgetRepresentation2D::SafeDownCast(this->WidgetRep);
  vtkCjyxMarkupsWidgetRepresentation3D* rep3d = vtkCjyxMarkupsWidgetRepresentation3D::SafeDownCast(this->WidgetRep);

  double rotationAxis[3] = { 0 };
  rep->GetInteractionHandleAxisWorld(type, index, rotationAxis); // Axis of rotation
  double origin[3] = { 0, 0, 0 };
  rep->GetInteractionHandleOriginWorld(origin);

  vtkNew<vtkPlane> axisPlaneWorld;
  axisPlaneWorld->SetNormal(rotationAxis);
  axisPlaneWorld->SetOrigin(origin);

  double inputPoint0_World[3] = { 0.0, 0.0, 0.0 };
  double inputPoint1_World[3] = { 0.0, 0.0, 1.0 };
  double projectionVector_World[3] = { 0 };
  if (rep3d)
    {
    vtkRenderer* renderer = rep3d->GetRenderer();
    vtkCamera* camera = renderer->GetActiveCamera();

    // Focal point position
    double cameraFP_World[4] = { 0 };
    camera->GetFocalPoint(cameraFP_World);

    renderer->SetWorldPoint(cameraFP_World[0], cameraFP_World[1], cameraFP_World[2], cameraFP_World[3]);
    renderer->WorldToDisplay();
    double* cameraFP_Display = renderer->GetDisplayPoint();
    double selectionZ_Display = cameraFP_Display[2];

    renderer->SetDisplayPoint(input_Display[0], input_Display[1], selectionZ_Display);
    renderer->DisplayToWorld();
    double* input_World = renderer->GetWorldPoint();
    if (input_World[3] == 0.0)
      {
      vtkWarningMacro("Bad homogeneous coordinates");
      return false;
      }
    double pickPosition_World[3] = { 0.0 };
    for (int i = 0; i < 3; i++)
      {
      pickPosition_World[i] = input_World[i] / input_World[3];
      }
    if (camera->GetParallelProjection())
      {
      camera->GetDirectionOfProjection(projectionVector_World);
      for (int i = 0; i < 3; i++)
        {
        inputPoint0_World[i] = pickPosition_World[i];
        }
      }
    else
      {
      // Camera position
      double cameraPosition_World[4] = { 0.0 };
      camera->GetPosition(cameraPosition_World);

      //  Compute the ray endpoints.  The ray is along the line running from
      //  the camera position to the selection point, starting where this line
      //  intersects the front clipping plane, and terminating where this
      //  line intersects the back clipping plane.
      for (int i = 0; i < 3; i++)
        {
        projectionVector_World[i] = pickPosition_World[i] - cameraPosition_World[i];
        inputPoint0_World[i] = cameraPosition_World[i];
        }
      }
    vtkMath::Add(inputPoint0_World, projectionVector_World, inputPoint1_World);
    }
  else if (rep2d)
    {
    double inputPoint0_Display[3] = { input_Display[0], input_Display[1], 0.0 };
    double inputPoint1_Display[3] = { input_Display[0], input_Display[1], 1.0 };

    vtkNew<vtkTransform> displayToWorldTransform;
    vtkDMMLSliceNode* sliceNode = vtkDMMLSliceNode::SafeDownCast(rep2d->GetViewNode());
    vtkMatrix4x4* xyToRASMatrix = sliceNode->GetXYToRAS();
    displayToWorldTransform->SetMatrix(xyToRASMatrix);
    displayToWorldTransform->TransformPoint(inputPoint0_Display, inputPoint0_World);
    displayToWorldTransform->TransformPoint(inputPoint1_Display, inputPoint1_World);
    }

  double t = 0.0; // not used
  axisPlaneWorld->IntersectWithLine(inputPoint0_World, inputPoint1_World, t, outputIntersection_World);
  return true;
}

//----------------------------------------------------------------------
bool vtkCjyxMarkupsWidget::GetClosestPointOnInteractionAxis(int type, int index, const double input_Display[2], double outputClosestPoint_World[3])
{
  vtkCjyxMarkupsWidgetRepresentation* rep = vtkCjyxMarkupsWidgetRepresentation::SafeDownCast(this->WidgetRep);
  vtkCjyxMarkupsWidgetRepresentation2D* rep2d = vtkCjyxMarkupsWidgetRepresentation2D::SafeDownCast(this->WidgetRep);
  vtkCjyxMarkupsWidgetRepresentation3D* rep3d = vtkCjyxMarkupsWidgetRepresentation3D::SafeDownCast(this->WidgetRep);

  double translationAxis_World[3] = { 0 };
  rep->GetInteractionHandleAxisWorld(type, index, translationAxis_World); // Axis of rotation
  double origin_World[3] = { 0, 0, 0 };
  rep->GetInteractionHandleOriginWorld(origin_World);

  double inputPoint0_World[3] = { 0.0, 0.0, 0.0 };
  double inputPoint1_World[3] = { 0.0, 0.0, 1.0 };
  if (rep3d)
    {
    vtkRenderer* renderer = rep3d->GetRenderer();
    vtkCamera* camera = renderer->GetActiveCamera();

    // Focal point position
    double cameraFP_World[4] = { 0 };
    camera->GetFocalPoint(cameraFP_World);

    renderer->SetWorldPoint(cameraFP_World[0], cameraFP_World[1], cameraFP_World[2], cameraFP_World[3]);
    renderer->WorldToDisplay();
    double* displayCoords = renderer->GetDisplayPoint();
    double selectionZ = displayCoords[2];

    renderer->SetDisplayPoint(input_Display[0], input_Display[1], selectionZ);
    renderer->DisplayToWorld();
    double* input_World = renderer->GetWorldPoint();
    if (input_World[3] == 0.0)
      {
      vtkWarningMacro("Bad homogeneous coordinates");
      return false;
      }
    double pickPosition_World[3] = { 0 };
    for (int i = 0; i < 3; i++)
      {
      pickPosition_World[i] = input_World[i] / input_World[3];
      }

    double projectionVector_World[3] = { 0 };
    if (camera->GetParallelProjection())
      {
      camera->GetDirectionOfProjection(projectionVector_World);
      for (int i = 0; i < 3; i++)
        {
        inputPoint0_World[i] = pickPosition_World[i];
        }
      }
    else
      {
      // Camera position
      double cameraPosition_World[4] = { 0 };
      camera->GetPosition(cameraPosition_World);

      //  Compute the ray endpoints.  The ray is along the line running from
      //  the camera position to the selection point, starting where this line
      //  intersects the front clipping plane, and terminating where this
      //  line intersects the back clipping plane.
      for (int i = 0; i < 3; i++)
        {
        inputPoint0_World[i] = cameraPosition_World[i];
        projectionVector_World[i] = pickPosition_World[i] - cameraPosition_World[i];
        }
      }
    vtkMath::Add(inputPoint0_World, projectionVector_World, inputPoint1_World);
    }
  else if (rep2d)
    {
    double inputPoint0_Display[3] = { input_Display[0], input_Display[1], 0.0 };
    double inputPoint1_Display[3] = { input_Display[0], input_Display[1], 1.0 };

    vtkNew<vtkTransform> displayToWorldTransform;
    vtkDMMLSliceNode* sliceNode = vtkDMMLSliceNode::SafeDownCast(rep2d->GetViewNode());
    vtkMatrix4x4* xyToRASMatrix = sliceNode->GetXYToRAS();
    displayToWorldTransform->SetMatrix(xyToRASMatrix);
    displayToWorldTransform->TransformPoint(inputPoint0_Display, inputPoint0_World);
    displayToWorldTransform->TransformPoint(inputPoint1_Display, inputPoint1_World);
    }
  double t1; // not used
  double t2; // not used
  double closestPointNotUsed[3] = { 0 };
  double translationVectorPoint[3] = { 0 };
  vtkMath::Add(origin_World, translationAxis_World, translationVectorPoint);
  vtkLine::DistanceBetweenLines(origin_World, translationVectorPoint,
    inputPoint0_World, inputPoint1_World, outputClosestPoint_World, closestPointNotUsed, t1, t2);
  return true;
}

//----------------------------------------------------------------------
vtkDMMLMarkupsNode* vtkCjyxMarkupsWidget::GetMarkupsNode()
{
  vtkCjyxMarkupsWidgetRepresentation* widgetRep = vtkCjyxMarkupsWidgetRepresentation::SafeDownCast(this->WidgetRep);
  if (!widgetRep)
    {
    return nullptr;
    }
  return widgetRep->GetMarkupsNode();
}

//----------------------------------------------------------------------
vtkDMMLMarkupsDisplayNode* vtkCjyxMarkupsWidget::GetMarkupsDisplayNode()
{
  vtkCjyxMarkupsWidgetRepresentation* widgetRep = vtkCjyxMarkupsWidgetRepresentation::SafeDownCast(this->WidgetRep);
  if (!widgetRep)
    {
    return nullptr;
    }
  return widgetRep->GetMarkupsDisplayNode();
}

//----------------------------------------------------------------------
vtkCjyxMarkupsWidgetRepresentation* vtkCjyxMarkupsWidget::GetMarkupsRepresentation()
{
  return vtkCjyxMarkupsWidgetRepresentation::SafeDownCast(this->WidgetRep);
}

//----------------------------------------------------------------------
int vtkCjyxMarkupsWidget::GetActiveControlPoint()
{
  vtkCjyxMarkupsWidgetRepresentation* rep = this->GetMarkupsRepresentation();
  if (!rep)
    {
    return -1;
    }
  vtkDMMLMarkupsDisplayNode* markupsDisplayNode = rep->GetMarkupsDisplayNode();
  if (!markupsDisplayNode)
    {
    return -1;
    }

  // Return first active control point for jumping //TODO: Have an 'even more active' point concept
  std::vector<int> activeControlPointIndices;
  markupsDisplayNode->GetActiveControlPoints(activeControlPointIndices);
  int activeControlPointIndex = -1;
  if (!activeControlPointIndices.empty())
    {
    activeControlPointIndex = activeControlPointIndices[0];
    }
  return activeControlPointIndex;
}

//----------------------------------------------------------------------
bool vtkCjyxMarkupsWidget::IsAnyControlPointLocked()
{
  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return false;
    }
  // If any node is locked return
  for (int i = 0; i < markupsNode->GetNumberOfControlPoints(); i++)
    {
    if (markupsNode->GetNthControlPointLocked(i))
      {
      return true;
      }
    }
  return false;
}

//-------------------------------------------------------------------------
int vtkCjyxMarkupsWidget::AddPointFromWorldCoordinate(const double worldCoordinates[3])
{
  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return -1;
    }

  int addedControlPoint = -1;
  if (this->PreviewPointIndex)
    {
    // convert point preview to final point
    addedControlPoint = this->PreviewPointIndex;
    markupsNode->SetNthControlPointPositionWorld(addedControlPoint, worldCoordinates);
    this->PreviewPointIndex = -1;
    }
  else
    {
    addedControlPoint = this->GetMarkupsNode()->AddControlPointWorld(vtkVector3d(worldCoordinates));
    }

  if (addedControlPoint>=0 && this->GetMarkupsDisplayNode())
    {
    this->GetMarkupsDisplayNode()->SetActiveControlPoint(addedControlPoint);
    }

  return addedControlPoint;
}

//----------------------------------------------------------------------
int vtkCjyxMarkupsWidget::AddNodeOnWidget(const int displayPos[2])
{
  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return 0;
    }
  vtkCjyxMarkupsWidgetRepresentation* rep = this->GetMarkupsRepresentation();
  if (!rep)
    {
    return 0;
    }
  double displayPosDouble[3] = { static_cast<double>(displayPos[0]), static_cast<double>(displayPos[1]), 0.0 };
  double worldPos[3] = { 0.0 };
  double worldOrient[9] = { 0.0 };
  if (!rep->GetPointPlacer()->ComputeWorldPosition(this->Renderer, displayPosDouble, worldPos, worldOrient))
    {
    return 0;
    }

  int idx = -1;
  double closestPosWorld[3];
  if (!rep->FindClosestPointOnWidget(displayPos, closestPosWorld, &idx))
    {
    return 0;
    }

  if (!rep->GetPointPlacer()->ComputeWorldPosition(this->Renderer,
    displayPosDouble, closestPosWorld, worldPos, worldOrient))
    {
    return 0;
    }

  // Add a new point at this position
  vtkDMMLMarkupsNode::ControlPoint* controlPoint = new vtkDMMLMarkupsNode::ControlPoint;
  markupsNode->TransformPointFromWorld(worldPos, controlPoint->Position);

  markupsNode->InsertControlPoint(controlPoint, idx);
  markupsNode->SetNthControlPointOrientationMatrixWorld(idx, worldOrient);
  return 1;
}

//-------------------------------------------------------------------------
bool vtkCjyxMarkupsWidget::GetInteractive()
{
  switch (this->WidgetState)
    {
    case WidgetStateTranslateControlPoint:
    case WidgetStateTranslate:
    case WidgetStateScale:
    case WidgetStateRotate:
      return true;
    default:
      return false;
    }
}

//-------------------------------------------------------------------------
int vtkCjyxMarkupsWidget::GetMouseCursor()
{
  if (this->WidgetState == WidgetStateIdle ||
    this->WidgetState == WidgetStateDefine) // default cursor shape is the "place" cursor
    {
    return VTK_CURSOR_DEFAULT;
    }
  else
    {
    return VTK_CURSOR_HAND;
    }
}

//----------------------------------------------------------------------
bool vtkCjyxMarkupsWidget::IsPointPreviewed()
{
  return this->PreviewPointIndex >= 0;
}

//---------------------------------------------------------------------------
bool vtkCjyxMarkupsWidget::PlacePoint(vtkDMMLInteractionEventData* eventData)
{
  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return false;
    }
  // save for undo and add the node to the scene after any reset of the
  // interaction node so that don't end up back in place mode
  markupsNode->GetScene()->SaveStateForUndo();

  // Add/update preview point
  const char* associatedNodeID = this->GetAssociatedNodeID(eventData);
  this->UpdatePreviewPoint(eventData, associatedNodeID, vtkDMMLMarkupsNode::PositionDefined);
  int controlPointIndex = this->PreviewPointIndex;
  // Convert the preview point to a proper control point
  this->PreviewPointIndex = -1;

  // if this was a one time place, go back to view transform mode
  vtkDMMLInteractionNode *interactionNode = this->GetInteractionNode();

  if(interactionNode)
    {
    bool hasRequiredPoints = markupsNode->GetRequiredNumberOfControlPoints() > 0;
    bool hasRequiredPointNumber = markupsNode->GetNumberOfControlPoints() >= markupsNode->GetRequiredNumberOfControlPoints();
    bool requiredPointsReached = hasRequiredPoints && hasRequiredPointNumber && !bool(markupsNode->GetNumberOfUndefinedControlPoints() > 0);
    bool lockedPointsReached = markupsNode->GetFixedNumberOfControlPoints() && !bool(markupsNode->GetNumberOfUndefinedControlPoints() > 0);

    if((requiredPointsReached && !interactionNode->GetPlaceModePersistence())
      || (!hasRequiredPoints && !interactionNode->GetPlaceModePersistence()) || lockedPointsReached)
      {
      vtkDebugMacro("End of one time place, place mode persistence = " << interactionNode->GetPlaceModePersistence());
      interactionNode->SetCurrentInteractionMode(vtkDMMLInteractionNode::ViewTransform);

      // The mouse is over the control point and we are not in place mode anymore
      if (this->GetMarkupsDisplayNode())
        {
        this->GetMarkupsDisplayNode()->SetActiveControlPoint(controlPointIndex);
        }
      this->WidgetState = WidgetStateOnWidget;
      }
    }

  bool success = (controlPointIndex >= 0);
  return success;
}

//---------------------------------------------------------------------------
int vtkCjyxMarkupsWidget::GetActiveComponentType()
{
  vtkDMMLMarkupsDisplayNode* displayNode = this->GetMarkupsDisplayNode();
  if (!displayNode)
    {
    return vtkDMMLMarkupsDisplayNode::ComponentNone;
    }
  return displayNode->GetActiveComponentType();
}

//---------------------------------------------------------------------------
int vtkCjyxMarkupsWidget::GetActiveComponentIndex()
{
  vtkDMMLMarkupsDisplayNode* displayNode = this->GetMarkupsDisplayNode();
  if (!displayNode)
    {
    return -1;
    }
  return displayNode->GetActiveComponentIndex();
}

//-----------------------------------------------------------------------------
vtkDMMLSelectionNode* vtkCjyxMarkupsWidget::selectionNode()
  {
  return vtkDMMLSelectionNode::SafeDownCast(
    this->GetMarkupsNode()->GetScene()->GetNodeByID("vtkDMMLSelectionNodeSingleton"));

  }