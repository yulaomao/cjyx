/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#include "vtkDMMLWindowLevelWidget.h"

#include "vtkDMMLAbstractSliceViewDisplayableManager.h"
#include "vtkDMMLApplicationLogic.h"
#include "vtkDMMLCrosshairDisplayableManager.h"
#include "vtkDMMLCrosshairNode.h"
#include "vtkDMMLInteractionEventData.h"
#include "vtkDMMLInteractionNode.h"
#include "vtkDMMLScalarVolumeDisplayNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSegmentationDisplayNode.h"
#include "vtkDMMLSliceCompositeNode.h"
#include "vtkDMMLSliceLayerLogic.h"
#include "vtkDMMLVolumeNode.h"
#include "vtkDMMLRubberBandWidgetRepresentation.h"

#include "vtkCommand.h"
#include "vtkCallbackCommand.h"
#include "vtkEvent.h"
#include "vtkGeneralTransform.h"
#include "vtkImageData.h"
#include "vtkImageClip.h"
#include "vtkImageHistogramStatistics.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkTransform.h"
#include "vtkWidgetEvent.h"

#include <deque>

vtkStandardNewMacro(vtkDMMLWindowLevelWidget);

//----------------------------------------------------------------------------------
vtkDMMLWindowLevelWidget::vtkDMMLWindowLevelWidget()
{
  this->StartEventPosition[0] = 0;
  this->StartEventPosition[1] = 0;

  this->PreviousEventPosition[0] = 0;
  this->PreviousEventPosition[1] = 0;

  this->VolumeScalarRange[0] = 0;
  this->VolumeScalarRange[1] = 0;

  this->IsStartVolumeAutoWindowLevel = false;
  this->StartVolumeWindowLevel[0] = 0;
  this->StartVolumeWindowLevel[1] = 0;
  this->LastVolumeWindowLevel[0] = 0;
  this->LastVolumeWindowLevel[1] = 0;
  this->WindowLevelAdjustedLayer = vtkDMMLSliceLogic::LayerBackground;

  this->BackgroundVolumeEditable = true;
  this->ForegroundVolumeEditable = true;

  this->CenteredRubberBand = true;

  this->AdjustMode = ModeAdjust;

  this->SetEventTranslationClickAndDrag(WidgetStateIdle, vtkCommand::LeftButtonPressEvent, vtkEvent::NoModifier,
    WidgetStateAdjustWindowLevel, WidgetEventAdjustWindowLevelStart, WidgetEventAdjustWindowLevelEnd);
  this->SetKeyboardEventTranslation(WidgetStateAdjustWindowLevel, vtkEvent::NoModifier, 0, 0, "Escape", WidgetEventAdjustWindowLevelCancel);
  this->SetEventTranslation(WidgetStateAdjustWindowLevel, vtkCommand::RightButtonPressEvent, vtkEvent::NoModifier, WidgetEventAdjustWindowLevelCancel);

  this->SetEventTranslationClickAndDrag(WidgetStateIdle, vtkCommand::LeftButtonPressEvent, vtkEvent::ControlModifier,
    WidgetStateAdjustWindowLevelAlternative, WidgetEventAdjustWindowLevelAlternativeStart, WidgetEventAdjustWindowLevelAlternativeEnd);
  this->SetKeyboardEventTranslation(WidgetStateAdjustWindowLevelAlternative,
    vtkEvent::AnyModifier, 0, 0, "Escape", WidgetEventAdjustWindowLevelAlternativeCancel);
  this->SetEventTranslation(WidgetStateAdjustWindowLevelAlternative, vtkCommand::RightButtonPressEvent, vtkEvent::AnyModifier,
    WidgetEventAdjustWindowLevelAlternativeCancel);

  this->SetEventTranslation(WidgetStateIdle, vtkCommand::LeftButtonDoubleClickEvent, vtkEvent::NoModifier, WidgetEventResetWindowLevel);
}

//----------------------------------------------------------------------------------
vtkDMMLWindowLevelWidget::~vtkDMMLWindowLevelWidget()
{
  this->SetDMMLApplicationLogic(nullptr);
}

//----------------------------------------------------------------------
void vtkDMMLWindowLevelWidget::CreateDefaultRepresentation()
{
  if (this->WidgetRep)
    {
    // already created
    return;
    }
  vtkNew<vtkDMMLRubberBandWidgetRepresentation> newRep;
  this->WidgetRep = newRep;
  this->WidgetRep->SetViewNode(this->GetSliceNode());
}

//-----------------------------------------------------------------------------
bool vtkDMMLWindowLevelWidget::CanProcessInteractionEvent(vtkDMMLInteractionEventData* eventData, double &distance2)
{
  vtkDMMLSliceLogic* sliceLogic = this->GetSliceLogic();
  if (!sliceLogic)
    {
    return false;
    }

  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);
  if (widgetEvent == WidgetEventNone)
    {
    return false;
    }
  if (!this->GetRepresentation())
    {
    return false;
    }

  // If we are currently dragging a point then we interact everywhere
  if (this->WidgetState == WidgetStateAdjustWindowLevel
    || this->WidgetState == WidgetStateAdjustWindowLevelAlternative)
    {
    distance2 = 0.0;
    return true;
    }

  if (this->GetInteractionNode()->GetCurrentInteractionMode() != vtkDMMLInteractionNode::AdjustWindowLevel
    && (widgetEvent < WidgetEventAlwaysOnResetWindowLevel || widgetEvent > WidgetEventAlwaysOnAdjustWindowLevelAlternativeCancel))
    {
    // if we are not in adjust window/level mouse mode then only always-on widget events are processed
    return false;
    }

  // We can process this event but we let more specific widgets to claim it (if they are closer).
  // View adjust actions are set at 1e10, set a lower value in order to override them.
  distance2 = 1e9;
  return true;
}

//-----------------------------------------------------------------------------
bool vtkDMMLWindowLevelWidget::ProcessInteractionEvent(vtkDMMLInteractionEventData* eventData)
{
  vtkDMMLSliceLogic* sliceLogic = this->GetSliceLogic();
  if (!sliceLogic)
    {
    return false;
    }

  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);

  bool processedEvent = true;

  switch (widgetEvent)
    {
    case WidgetEventMouseMove:
    // click-and-dragging the mouse cursor
      processedEvent = this->ProcessMouseMove(eventData);
      break;
    case WidgetEventAdjustWindowLevelStart:
    case WidgetEventAdjustWindowLevelAlternativeStart:
    case WidgetEventAlwaysOnAdjustWindowLevelStart:
    case WidgetEventAlwaysOnAdjustWindowLevelAlternativeStart:
    {
      vtkDMMLInteractionNode * interactionNode = this->GetInteractionNode();
      this->AdjustMode = -1;
      if (interactionNode)
        {
        this->AdjustMode = GetAdjustWindowLevelModeFromString(
          interactionNode->GetAttribute(GetInteractionNodeAdjustWindowLevelModeAttributeName()));
        }
      if (this->AdjustMode < 0)
        {
        // no valid mode is defined, use default
        this->AdjustMode = ModeAdjust;
        }
      // Control modifier indicates to use the alternative adjustment mode
      if (widgetEvent == WidgetEventAdjustWindowLevelAlternativeStart
        || widgetEvent == WidgetEventAlwaysOnAdjustWindowLevelAlternativeStart)
        {
        if (this->AdjustMode == ModeAdjust)
          {
          this->AdjustMode = ModeRectangleCentered;
          }
        else
          {
          this->AdjustMode = ModeAdjust;
          }
        }
      if (this->AdjustMode == ModeAdjust)
        {
        processedEvent = this->ProcessAdjustWindowLevelStart(eventData);
        }
      else
        {
        this->SetCenteredRubberBand(this->AdjustMode == ModeRectangleCentered);
        processedEvent = this->ProcessSetWindowLevelFromRegionStart(eventData);
        }
      }
      break;
    case WidgetEventAdjustWindowLevelEnd:
    case WidgetEventAdjustWindowLevelAlternativeEnd:
    case WidgetEventAlwaysOnAdjustWindowLevelEnd:
    case WidgetEventAlwaysOnAdjustWindowLevelAlternativeEnd:
      if (this->AdjustMode == ModeAdjust)
        {
        processedEvent = this->ProcessEndMouseDrag(eventData);
        }
      else
        {
        processedEvent = this->ProcessSetWindowLevelFromRegionEnd(eventData);
        }
      break;
    case WidgetEventAdjustWindowLevelCancel:
    case WidgetEventAlwaysOnAdjustWindowLevelCancel:
      processedEvent = this->ProcessEndMouseDrag(eventData);
      this->SetVolumeWindowLevel(this->StartVolumeWindowLevel[0], this->StartVolumeWindowLevel[1], this->IsStartVolumeAutoWindowLevel);
      break;
    case WidgetEventResetWindowLevel:
    case WidgetEventAlwaysOnResetWindowLevel:
      processedEvent = this->ProcessResetWindowLevel(eventData);
      break;
    case WidgetEventAdjustWindowLevelAlternativeCancel:
    case WidgetEventAlwaysOnAdjustWindowLevelAlternativeCancel:
      processedEvent = this->ProcessSetWindowLevelFromRegionEnd(eventData, false);
      break;
    default:
      processedEvent = false;
    }

  return processedEvent;
}

//-------------------------------------------------------------------------
void vtkDMMLWindowLevelWidget::Leave(vtkDMMLInteractionEventData* eventData)
{
  this->SetWidgetState(WidgetStateIdle);
  if (this->WidgetRep)
    {
    this->WidgetRep->SetVisibility(false);
    }
  this->Superclass::Leave(eventData);
}

//-------------------------------------------------------------------------
bool vtkDMMLWindowLevelWidget::ProcessMouseMove(vtkDMMLInteractionEventData* eventData)
{
  if (!this->WidgetRep || !eventData)
    {
    return false;
    }

  switch (this->WidgetState)
    {
    case WidgetStateAdjustWindowLevel:
      this->ProcessAdjustWindowLevel(eventData);
      break;
    case WidgetStateAdjustWindowLevelAlternative:
      this->ProcessSetWindowLevelFromRegion(eventData);
      break;
    }

  return true;
}

//-------------------------------------------------------------------------
bool vtkDMMLWindowLevelWidget::ProcessStartMouseDrag(vtkDMMLInteractionEventData* eventData)
{
  const int* displayPos = eventData->GetDisplayPosition();

  this->StartEventPosition[0] = displayPos[0];
  this->StartEventPosition[1] = displayPos[1];

  this->PreviousEventPosition[0] = this->StartEventPosition[0];
  this->PreviousEventPosition[1] = this->StartEventPosition[1];

  this->ProcessMouseMove(eventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkDMMLWindowLevelWidget::ProcessEndMouseDrag(vtkDMMLInteractionEventData* vtkNotUsed(eventData))
{
  /*
  if (this->WidgetState == WidgetStateIdle)
    {
    return false;
    }
  */
  this->SetWidgetState(WidgetStateIdle);
  return true;
}

//----------------------------------------------------------------------------------
void vtkDMMLWindowLevelWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------------
void vtkDMMLWindowLevelWidget::SetDMMLApplicationLogic(vtkDMMLApplicationLogic* appLogic)
{
  if (appLogic == this->ApplicationLogic)
    {
    return;
    }
  this->Superclass::SetDMMLApplicationLogic(appLogic);
}

//----------------------------------------------------------------------------------
vtkDMMLSliceLogic* vtkDMMLWindowLevelWidget::GetSliceLogic()
{
  if (!this->SliceLogic && this->GetDMMLApplicationLogic())
    {
    this->SliceLogic = this->GetDMMLApplicationLogic()->GetSliceLogic(this->SliceNode);
    }
  return this->SliceLogic;
}


//----------------------------------------------------------------------------------
void vtkDMMLWindowLevelWidget::SetSliceNode(vtkDMMLSliceNode* sliceNode)
{
  if (this->SliceNode == sliceNode)
    {
    // no change
    return;
    }
  this->SliceNode = sliceNode;
  if (this->WidgetRep)
    {
    this->WidgetRep->SetViewNode(sliceNode);
    }
  // Force update of slice logic
  this->SliceLogic = nullptr;
}

//----------------------------------------------------------------------------------
vtkDMMLSliceNode* vtkDMMLWindowLevelWidget::GetSliceNode()
{
  return this->SliceNode;
}

//----------------------------------------------------------------------------
void vtkDMMLWindowLevelWidget::ProcessAdjustWindowLevel(vtkDMMLInteractionEventData* eventData)
{
  const int* eventPosition = eventData->GetDisplayPosition();

  int deltaX = eventPosition[0] - this->PreviousEventPosition[0];
  int deltaY = eventPosition[1] - this->PreviousEventPosition[1];

  double rangeLow = this->VolumeScalarRange[0];
  double rangeHigh = this->VolumeScalarRange[1];

  const int* windowSize = this->GetRenderer()->GetRenderWindow()->GetSize();
  double windowMinSize = std::min(windowSize[0], windowSize[1]);

  double gain = (rangeHigh - rangeLow) / windowMinSize;
  double newWindow = this->LastVolumeWindowLevel[0] + (gain * deltaX);
  if (newWindow < 0)
    {
    newWindow = 0;
    }
  double newLevel = this->LastVolumeWindowLevel[1] + (gain * deltaY);
  if (newLevel < rangeLow - newWindow / 2)
    {
    newLevel = rangeLow - newWindow / 2;
    }
  if (newLevel > rangeHigh + newWindow / 2)
    {
    newLevel = rangeHigh + newWindow / 2;
    }
  this->SetVolumeWindowLevel(newWindow, newLevel, false);
  this->LastVolumeWindowLevel[0] = newWindow;
  this->LastVolumeWindowLevel[1] = newLevel;

  this->PreviousEventPosition[0] = eventPosition[0];
  this->PreviousEventPosition[1] = eventPosition[1];
  return;
}

//----------------------------------------------------------------------------
int vtkDMMLWindowLevelWidget::GetEditableLayerAtEventPosition(vtkDMMLInteractionEventData* eventData)
{
  double worldPos[3] = { 0.0, 0.0, 0.0 };
  eventData->GetWorldPosition(worldPos);
  if (!eventData->IsWorldPositionValid())
    {
    vtkErrorMacro("vtkDMMLWindowLevelWidget::GetEditableLayerAtEventPosition failed: invalid world position");
    return vtkDMMLSliceLogic::LayerNone;
    }
  if (!this->GetSliceLogic())
    {
    vtkErrorMacro("vtkDMMLWindowLevelWidget::GetEditableLayerAtEventPosition failed: invalid slice logic");
    return vtkDMMLSliceLogic::LayerNone;
    }
  return this->GetSliceLogic()->GetEditableLayerAtWorldPosition(worldPos, this->BackgroundVolumeEditable, this->ForegroundVolumeEditable);
}

//----------------------------------------------------------------------------
bool vtkDMMLWindowLevelWidget::ProcessAdjustWindowLevelStart(vtkDMMLInteractionEventData* eventData)
{
  vtkDMMLSliceLogic* sliceLogic = this->GetSliceLogic();
  if (!sliceLogic)
    {
    return false;
    }
  this->WindowLevelAdjustedLayer = vtkDMMLSliceLogic::LayerNone;
  vtkDMMLSliceCompositeNode *sliceCompositeNode = sliceLogic->GetSliceCompositeNode();
  if (!sliceCompositeNode)
    {
    return false;
    }
  int editedLayer = this->GetEditableLayerAtEventPosition(eventData);
  if (editedLayer != vtkDMMLSliceLogic::LayerForeground
    && editedLayer != vtkDMMLSliceLogic::LayerBackground)
    {
    return false;
    }
  sliceLogic->GetDMMLScene()->SaveStateForUndo();
  this->WindowLevelAdjustedLayer = editedLayer;
  if (editedLayer == vtkDMMLSliceLogic::LayerForeground)
    {
    sliceLogic->GetForegroundWindowLevelAndRange(
      this->LastVolumeWindowLevel[0], this->LastVolumeWindowLevel[1],
      this->VolumeScalarRange[0], this->VolumeScalarRange[1],
      this->IsStartVolumeAutoWindowLevel);
    }
  else if (editedLayer == vtkDMMLSliceLogic::LayerBackground)
    {
    sliceLogic->GetBackgroundWindowLevelAndRange(
      this->LastVolumeWindowLevel[0], this->LastVolumeWindowLevel[1],
      this->VolumeScalarRange[0], this->VolumeScalarRange[1],
      this->IsStartVolumeAutoWindowLevel);
    }
  this->StartVolumeWindowLevel[0] = this->LastVolumeWindowLevel[0];
  this->StartVolumeWindowLevel[1] = this->LastVolumeWindowLevel[1];
  this->SetWidgetState(WidgetStateAdjustWindowLevel);
  return this->ProcessStartMouseDrag(eventData);
}

//----------------------------------------------------------------------------
vtkDMMLVolumeNode* vtkDMMLWindowLevelWidget::GetVolumeNodeFromSliceLayer(int editedLayer)
{
  vtkDMMLSliceLogic* sliceLogic = this->GetSliceLogic();
  if (!sliceLogic)
    {
    return nullptr;
    }
  vtkDMMLSliceCompositeNode *sliceCompositeNode = sliceLogic->GetSliceCompositeNode();
  if (!sliceCompositeNode)
    {
    return nullptr;
    }
  vtkDMMLVolumeNode* volumeNode = nullptr;
  if (editedLayer == vtkDMMLSliceLogic::LayerForeground)
    {
    volumeNode = vtkDMMLVolumeNode::SafeDownCast(sliceLogic->GetDMMLScene()->GetNodeByID(sliceCompositeNode->GetForegroundVolumeID()));
    }
  else if (editedLayer == vtkDMMLSliceLogic::LayerBackground)
    {
    volumeNode = vtkDMMLVolumeNode::SafeDownCast(sliceLogic->GetDMMLScene()->GetNodeByID(sliceCompositeNode->GetBackgroundVolumeID()));
    }
  return volumeNode;
}

//----------------------------------------------------------------------------
bool vtkDMMLWindowLevelWidget::ProcessResetWindowLevel(vtkDMMLInteractionEventData* vtkNotUsed(eventData))
{
  vtkDMMLVolumeNode* volumeNode = this->GetVolumeNodeFromSliceLayer(this->WindowLevelAdjustedLayer);
  if (!volumeNode)
    {
    return false;
    }
  vtkDMMLScalarVolumeDisplayNode* displayNode = vtkDMMLScalarVolumeDisplayNode::SafeDownCast(volumeNode->GetVolumeDisplayNode());
  if (!displayNode)
    {
    return false;
    }
  if (displayNode->GetAutoWindowLevel())
    {
    // auto-window/level is already enabled
    return true;
    }
  vtkDMMLSliceLogic* sliceLogic = this->GetSliceLogic();
  if (sliceLogic)
    {
    sliceLogic->GetDMMLScene()->SaveStateForUndo();
    }
  displayNode->AutoWindowLevelOn();
  return true;
}

//----------------------------------------------------------------------------
bool vtkDMMLWindowLevelWidget::ProcessSetWindowLevelFromRegionStart(vtkDMMLInteractionEventData* eventData)
{
  this->WindowLevelAdjustedLayer = this->GetEditableLayerAtEventPosition(eventData);
  if (this->WindowLevelAdjustedLayer == vtkDMMLSliceLogic::LayerNone)
    {
    return false;
    }
  vtkDMMLRubberBandWidgetRepresentation* rubberBand = vtkDMMLRubberBandWidgetRepresentation::SafeDownCast(this->WidgetRep);
  if (rubberBand)
    {
    const int* displayPos = eventData->GetDisplayPosition();
    rubberBand->SetCornerPoint1((int*)displayPos);
    rubberBand->SetCornerPoint2((int*)displayPos);
    rubberBand->SetVisibility(true);
    rubberBand->NeedToRenderOn();
    }
  this->SetWidgetState(WidgetStateAdjustWindowLevelAlternative);
  return this->ProcessStartMouseDrag(eventData);
}

//----------------------------------------------------------------------------
void vtkDMMLWindowLevelWidget::ProcessSetWindowLevelFromRegion(vtkDMMLInteractionEventData* eventData)
{
  vtkDMMLRubberBandWidgetRepresentation* rubberBand = vtkDMMLRubberBandWidgetRepresentation::SafeDownCast(this->WidgetRep);
  if (!rubberBand)
    {
    return;
    }
  const int* displayPos = eventData->GetDisplayPosition();

  if (this->CenteredRubberBand)
    {
    int radius[2] =
      {
      abs(displayPos[0] - this->StartEventPosition[0]),
      abs(displayPos[1] - this->StartEventPosition[1])
      };
    rubberBand->SetCornerPoint1(this->StartEventPosition[0] - radius[0], this->StartEventPosition[1] - radius[1]);
    rubberBand->SetCornerPoint2(this->StartEventPosition[0] + radius[0], this->StartEventPosition[1] + radius[1]);
    }
  else
    {
    rubberBand->SetCornerPoint1(this->StartEventPosition);
    rubberBand->SetCornerPoint2((int*)displayPos);
    }

  rubberBand->NeedToRenderOn();
}

//----------------------------------------------------------------------------
bool vtkDMMLWindowLevelWidget::ProcessSetWindowLevelFromRegionEnd(vtkDMMLInteractionEventData* eventData, bool updateWindowLevel/*=true*/)
{
  if (!this->ProcessEndMouseDrag(eventData))
    {
    return false;
    }
  vtkDMMLRubberBandWidgetRepresentation* rubberBand = vtkDMMLRubberBandWidgetRepresentation::SafeDownCast(this->WidgetRep);
  if (rubberBand)
    {
    rubberBand->SetVisibility(false);
    rubberBand->NeedToRenderOn();
    }
  if (updateWindowLevel)
    {
    return this->UpdateWindowLevelFromRectangle(this->WindowLevelAdjustedLayer, rubberBand->GetCornerPoint1(), rubberBand->GetCornerPoint2());
    }
  else
    {
    // cancelled
    return true;
    }
}

//----------------------------------------------------------------------------
bool vtkDMMLWindowLevelWidget::UpdateWindowLevelFromRectangle(int layer, int cornerPoint1[2], int cornerPoint2[2])
{
  if (cornerPoint1[0] == cornerPoint2[0]
    || cornerPoint1[1] == cornerPoint2[1])
    {
    // empty box
    return false;
    }
  vtkDMMLSliceLogic* sliceLogic = this->GetSliceLogic();
  if (!sliceLogic)
    {
    return false;
    }
  vtkDMMLSliceNode *sliceNode = sliceLogic->GetSliceNode();
  if (!sliceNode)
    {
    return false;
    }
  vtkDMMLSliceLayerLogic* layerLogic = nullptr;
  if (layer == vtkDMMLSliceLogic::LayerBackground)
    {
    layerLogic = sliceLogic->GetBackgroundLayer();
    }
  else if (layer == vtkDMMLSliceLogic::LayerForeground)
    {
    layerLogic = sliceLogic->GetForegroundLayer();
    }
  if (!layerLogic)
    {
    return false;
    }

  // get the rubberband bounding box in ijk coordinates
  vtkGeneralTransform* xyToIJK = layerLogic->GetXYToIJKTransform();
  if (!xyToIJK)
    {
    return false;
    }
  vtkBoundingBox ijkBounds;
  ijkBounds.AddPoint(xyToIJK->TransformPoint(cornerPoint1[0], cornerPoint1[1], 0.0));
  ijkBounds.AddPoint(xyToIJK->TransformPoint(cornerPoint2[0], cornerPoint1[1], 0.0));
  ijkBounds.AddPoint(xyToIJK->TransformPoint(cornerPoint2[0], cornerPoint2[1], 0.0));
  ijkBounds.AddPoint(xyToIJK->TransformPoint(cornerPoint1[0], cornerPoint2[1], 0.0));

  // clamp the bounds to the dimensions of the label image
  vtkDMMLVolumeNode* volumeNode = layerLogic->GetVolumeNode();
  vtkImageData* imageData = (volumeNode ? volumeNode->GetImageData() : nullptr);
  if (!imageData || !imageData->GetPointData() || !imageData->GetPointData()->GetScalars())
    {
    // vtkImageHistogramStatistics crashes if there are no scalars
    return false;
    }
  double bounds[6] = { 0.0 };
  ijkBounds.GetBounds(bounds);
  int extent[6] = { 0 };
  for (int i = 0; i < 3; i++)
    {
    extent[i * 2] = std::max(static_cast<int>(std::floor(bounds[i * 2])), imageData->GetExtent()[i * 2]);
    extent[i * 2 + 1] = std::min(static_cast<int>(std::floor(bounds[i * 2 + 1])), imageData->GetExtent()[i * 2 + 1]);
    }

   // calculate the statistics for the selected region
  vtkNew<vtkImageClip> clip;
  clip->SetOutputWholeExtent(extent);
  clip->SetInputData(imageData);
  clip->ClipDataOn();
  vtkNew<vtkImageHistogramStatistics> stats;
  stats->SetInputConnection(clip->GetOutputPort());
  stats->Update();

  vtkDMMLScalarVolumeDisplayNode* displayNode = vtkDMMLScalarVolumeDisplayNode::SafeDownCast(volumeNode->GetVolumeDisplayNode());
  if (!displayNode)
    {
    return false;
    }
  displayNode->AutoWindowLevelOff();
  // Compute intensity range as 1th and 99th percentile, expanded by 1%.
  // It is more robust than taking the minimum and maximum - a few outlier voxels do not throw off the range.
  double* intensityRange = stats->GetAutoRange();
  displayNode->SetWindowLevelMinMax(intensityRange[0], intensityRange[1]);
  return true;
}

//----------------------------------------------------------------------------
bool vtkDMMLWindowLevelWidget::SetVolumeWindowLevel(double window, double level, bool isAutoWindowLevel)
{
  vtkDMMLScalarVolumeNode* volumeNode = vtkDMMLScalarVolumeNode::SafeDownCast(
    this->GetVolumeNodeFromSliceLayer(this->WindowLevelAdjustedLayer));
  if (!volumeNode)
    {
    return false;
    }
  vtkDMMLScalarVolumeDisplayNode* volumeDisplayNode = volumeNode->GetScalarVolumeDisplayNode();
  if (!volumeDisplayNode)
    {
    return false;
    }
  if (isAutoWindowLevel)
    {
    int disabledModify = volumeDisplayNode->StartModify();
    volumeDisplayNode->SetWindowLevel(window, level);
    volumeDisplayNode->SetAutoWindowLevel(1);
    volumeDisplayNode->EndModify(disabledModify);
    }
  else
    {
    int disabledModify = volumeDisplayNode->StartModify();
    volumeDisplayNode->SetAutoWindowLevel(0);
    volumeDisplayNode->SetWindowLevel(window, level);
    volumeDisplayNode->EndModify(disabledModify);
    }
  return true;
}

//-----------------------------------------------------------
const char* vtkDMMLWindowLevelWidget::GetAdjustWindowLevelModeAsString(int id)
{
  switch (id)
    {
    case ModeAdjust: return "Adjust";
    case ModeRectangle: return "Rectangle";
    case ModeRectangleCentered: return "RectangleCentered";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkDMMLWindowLevelWidget::GetAdjustWindowLevelModeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < Mode_Last; ii++)
    {
    if (strcmp(name, GetAdjustWindowLevelModeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}
