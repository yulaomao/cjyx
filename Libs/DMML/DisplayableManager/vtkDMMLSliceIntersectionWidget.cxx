/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkDMMLSliceIntersectionWidget.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or https://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkDMMLSliceIntersectionWidget.h"

#include "vtkDMMLAbstractSliceViewDisplayableManager.h"
#include "vtkDMMLApplicationLogic.h"
#include "vtkDMMLCrosshairDisplayableManager.h"
#include "vtkDMMLCrosshairNode.h"
#include "vtkDMMLInteractionEventData.h"
#include "vtkDMMLInteractionNode.h"
#include "vtkDMMLLayoutNode.h"
#include "vtkDMMLScalarVolumeDisplayNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSegmentationDisplayNode.h"
#include "vtkDMMLSliceCompositeNode.h"
#include "vtkDMMLSliceDisplayNode.h"
#include "vtkDMMLSliceIntersectionInteractionRepresentation.h"
#include "vtkDMMLSliceIntersectionRepresentation2D.h"
#include "vtkDMMLSliceLayerLogic.h"
#include "vtkDMMLVolumeNode.h"

#include "vtkCommand.h"
#include "vtkCallbackCommand.h"
#include "vtkEvent.h"
#include "vtkGeneralTransform.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkTransform.h"
#include "vtkWidgetEvent.h"

#include <deque>

vtkStandardNewMacro(vtkDMMLSliceIntersectionWidget);

//----------------------------------------------------------------------------------
vtkDMMLSliceIntersectionWidget::vtkDMMLSliceIntersectionWidget()
{
  this->StartEventPosition[0] = 0.0;
  this->StartEventPosition[1] = 0.0;

  this->PreviousRotationAngleRad = 0.0;

  this->PreviousEventPosition[0] = 0;
  this->PreviousEventPosition[1] = 0;

  this->StartRotationCenter[0] = 0.0;
  this->StartRotationCenter[1] = 0.0;

  this->StartRotationCenter_RAS[0] = 0.0;
  this->StartRotationCenter_RAS[1] = 0.0;
  this->StartRotationCenter_RAS[2] = 0.0;
  this->StartRotationCenter_RAS[3] = 1.0; // to allow easy homogeneous transformations

  // Interactive slice intersection widget

  this->StartTranslationPoint[0] = 0.0;
  this->StartTranslationPoint[1] = 0.0;

  this->StartTranslationPoint_RAS[0] = 0.0;
  this->StartTranslationPoint_RAS[1] = 0.0;
  this->StartTranslationPoint_RAS[2] = 0.0;

  this->CurrentTranslationPoint_RAS[0] = 0.0;
  this->CurrentTranslationPoint_RAS[1] = 0.0;
  this->CurrentTranslationPoint_RAS[2] = 0.0;

  this->StartActionFOV[0] = 0.;
  this->StartActionFOV[1] = 0.;
  this->StartActionFOV[2] = 0.;
  this->VolumeScalarRange[0] = 0;
  this->VolumeScalarRange[1] = 0;

  this->LastForegroundOpacity = 0.;
  this->LastLabelOpacity = 0.;
  this->StartActionSegmentationDisplayNode = nullptr;

  this->ModifierKeyPressedSinceLastClickAndDrag = true;

  this->TouchRotationThreshold = 10.0;
  this->TouchZoomThreshold = 0.1;
  this->TouchTranslationThreshold = 25.0;

  this->TotalTouchRotation = 0.0;
  this->TouchRotateEnabled = false;
  this->TotalTouchZoom = 0.0;
  this->TouchZoomEnabled = false;
  this->TotalTouchTranslation = 0.0;
  this->TouchTranslationEnabled = false;

  this->SliceModifiedCommand->SetClientData(this);
  this->SliceModifiedCommand->SetCallback(vtkDMMLSliceIntersectionWidget::SliceModifiedCallback);

  this->SliceLogicsModifiedCommand->SetClientData(this);
  this->SliceLogicsModifiedCommand->SetCallback(vtkDMMLSliceIntersectionWidget::SliceLogicsModifiedCallback);

  this->ActionsEnabled = ActionAll;
  this->UpdateInteractionEventMapping();
}

//----------------------------------------------------------------------------------
vtkDMMLSliceIntersectionWidget::~vtkDMMLSliceIntersectionWidget()
{
  this->SetSliceNode(nullptr);
  this->SetDMMLApplicationLogic(nullptr);
}

//----------------------------------------------------------------------
void vtkDMMLSliceIntersectionWidget::UpdateInteractionEventMapping()
{
  this->EventTranslators.clear();

  // Interactions without handles

  if (this->GetActionEnabled(ActionRotateSliceIntersection))
    {
    // Touch gesture slice rotate
    this->SetEventTranslation(WidgetStateIdle, vtkCommand::StartRotateEvent, vtkEvent::AnyModifier, WidgetEventTouchGestureStart);
    this->SetEventTranslation(WidgetStateTouchGesture, vtkCommand::RotateEvent, vtkEvent::AnyModifier, WidgetEventTouchRotateSliceIntersection);
    this->SetEventTranslation(WidgetStateTouchGesture, vtkCommand::EndRotateEvent, vtkEvent::AnyModifier, WidgetEventTouchGestureEnd);
    // Ctrl-alt-left-click-and-drag rotate
    this->SetEventTranslationClickAndDrag(WidgetStateIdle, vtkCommand::LeftButtonPressEvent, vtkEvent::AltModifier + vtkEvent::ControlModifier,
      WidgetStateRotateIntersectingSlices, WidgetEventRotateIntersectingSlicesStart, WidgetEventRotateIntersectingSlicesEnd);
    }
  if (this->GetActionEnabled(ActionTranslateSliceIntersection))
    {
    this->SetEventTranslationClickAndDrag(WidgetStateIdle, vtkCommand::LeftButtonPressEvent,
      vtkEvent::AltModifier + vtkEvent::ShiftModifier, WidgetStateTranslate, WidgetEventTranslateStart, WidgetEventTranslateEnd);
    }
  if (this->GetActionEnabled(ActionSetCrosshairPosition))
    {
    this->SetEventTranslation(WidgetStateIdle, vtkCommand::MouseMoveEvent, vtkEvent::ShiftModifier, WidgetEventSetCrosshairPositionBackground);
    }
  if (this->GetActionEnabled(ActionBlend))
    {
    this->SetEventTranslationClickAndDrag(WidgetStateIdle, vtkCommand::LeftButtonPressEvent,
      vtkEvent::ControlModifier, WidgetStateBlend, WidgetEventBlendStart, WidgetEventBlendEnd);
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "g", WidgetEventToggleLabelOpacity);
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "t", WidgetEventToggleForegroundOpacity);
    }
  if (this->GetActionEnabled(ActionBrowseSlice))
    {
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "Right", WidgetEventIncrementSlice);
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "Left", WidgetEventDecrementSlice);
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "Up", WidgetEventIncrementSlice);
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "Down", WidgetEventDecrementSlice);
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "f", WidgetEventIncrementSlice);
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "b", WidgetEventDecrementSlice);
    this->SetEventTranslation(WidgetStateIdle, vtkCommand::MouseWheelForwardEvent, vtkEvent::NoModifier, WidgetEventIncrementSlice);
    this->SetEventTranslation(WidgetStateIdle, vtkCommand::MouseWheelBackwardEvent, vtkEvent::NoModifier, WidgetEventDecrementSlice);
    }
  if (this->GetActionEnabled(ActionShowSlice))
    {
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "v", WidgetEventToggleSliceVisibility);
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::AnyModifier, 0, 0, "V", WidgetEventToggleAllSlicesVisibility);
    }
  if (this->GetActionEnabled(ActionSelectVolume))
    {
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::AnyModifier, 0, 0, "bracketleft", WidgetEventShowPreviousBackgroundVolume);
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::AnyModifier, 0, 0, "bracketright", WidgetEventShowNextBackgroundVolume);
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::AnyModifier, 0, 0, "braceleft", WidgetEventShowPreviousForegroundVolume);
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::AnyModifier, 0, 0, "braceright", WidgetEventShowNextForegroundVolume);
    }
  if (this->GetActionEnabled(ActionTranslate) && this->GetActionEnabled(ActionZoom))
    {
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "r", WidgetEventResetFieldOfView);
    }
  if (this->GetActionEnabled(ActionZoom))
    {
    this->SetEventTranslationClickAndDrag(WidgetStateIdle, vtkCommand::RightButtonPressEvent, vtkEvent::NoModifier,
      WidgetStateZoomSlice, WidgetEventZoomSliceStart, WidgetEventZoomSliceEnd);
    this->SetEventTranslation(WidgetStateIdle, vtkCommand::MouseWheelForwardEvent, vtkEvent::ControlModifier, WidgetEventZoomOutSlice);
    this->SetEventTranslation(WidgetStateIdle, vtkCommand::MouseWheelBackwardEvent, vtkEvent::ControlModifier, WidgetEventZoomInSlice);
    // Touch slice zoom
    this->SetEventTranslation(WidgetStateIdle, vtkCommand::StartPinchEvent, vtkEvent::AnyModifier, WidgetEventTouchGestureStart);
    this->SetEventTranslation(WidgetStateTouchGesture, vtkCommand::PinchEvent, vtkEvent::AnyModifier, WidgetEventTouchZoomSlice);
    this->SetEventTranslation(WidgetStateTouchGesture, vtkCommand::EndPinchEvent, vtkEvent::AnyModifier, WidgetEventTouchGestureEnd);
    }

  if (this->GetActionEnabled(ActionTranslate))
    {
    this->SetEventTranslationClickAndDrag(WidgetStateIdle, vtkCommand::LeftButtonPressEvent, vtkEvent::ShiftModifier,
      WidgetStateTranslateSlice, WidgetEventTranslateSliceStart, WidgetEventTranslateSliceEnd);
    this->SetEventTranslationClickAndDrag(WidgetStateIdle, vtkCommand::MiddleButtonPressEvent, vtkEvent::NoModifier,
      WidgetStateTranslateSlice, WidgetEventTranslateSliceStart, WidgetEventTranslateSliceEnd);
    // Touch slice translate
    this->SetEventTranslation(WidgetStateIdle, vtkCommand::StartPanEvent, vtkEvent::AnyModifier, WidgetEventTouchGestureStart);
    this->SetEventTranslation(WidgetStateTouchGesture, vtkCommand::PanEvent, vtkEvent::AnyModifier, WidgetEventTouchTranslateSlice);
    this->SetEventTranslation(WidgetStateTouchGesture, vtkCommand::EndPanEvent, vtkEvent::AnyModifier, WidgetEventTouchGestureEnd);
    }

  // Context menu
  this->SetEventTranslation(WidgetStateAny, vtkDMMLInteractionEventData::RightButtonClickEvent, vtkEvent::NoModifier, WidgetEventMenu);

  // Maximize/restore view
  this->SetEventTranslation(WidgetStateIdle, vtkCommand::LeftButtonDoubleClickEvent, vtkEvent::NoModifier, WidgetEventMaximizeView);

  // Interactions with slice intersection handles

  // Update active component
  this->SetEventTranslation(WidgetStateIdle, vtkCommand::MouseMoveEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateOnWidget, vtkCommand::MouseMoveEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateIdle, vtkCommand::Move3DEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateOnWidget, vtkCommand::Move3DEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  // Update active interaction handle component
  this->SetEventTranslation(WidgetStateOnTranslateIntersectingSlicesHandle, vtkCommand::MouseMoveEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateOnTranslateIntersectingSlicesHandle, vtkCommand::Move3DEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateOnRotateIntersectingSlicesHandle, vtkCommand::MouseMoveEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateOnRotateIntersectingSlicesHandle, vtkCommand::Move3DEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateOnTranslateSingleIntersectingSliceHandle, vtkCommand::MouseMoveEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateOnTranslateSingleIntersectingSliceHandle, vtkCommand::Move3DEvent, vtkEvent::NoModifier, WidgetEventMouseMove);

  if (this->GetActionEnabled(ActionRotateSliceIntersection))
    {
    this->SetEventTranslationClickAndDrag(WidgetStateOnRotateIntersectingSlicesHandle, vtkCommand::LeftButtonPressEvent, vtkEvent::NoModifier,
      WidgetStateRotateIntersectingSlicesHandle, WidgetEventRotateIntersectingSlicesHandleStart, WidgetEventRotateIntersectingSlicesHandleEnd);
    }
  if (this->GetActionEnabled(ActionTranslateSliceIntersection))
    {
    this->SetEventTranslationClickAndDrag(WidgetStateOnTranslateIntersectingSlicesHandle, vtkCommand::LeftButtonPressEvent, vtkEvent::NoModifier,
      WidgetStateTranslateIntersectingSlicesHandle, WidgetEventTranslateIntersectingSlicesHandleStart, WidgetEventTranslateIntersectingSlicesHandleEnd);
    }
  if (this->GetActionEnabled(ActionTranslate))
    {
    this->SetEventTranslationClickAndDrag(WidgetStateOnTranslateSingleIntersectingSliceHandle, vtkCommand::LeftButtonPressEvent, vtkEvent::NoModifier,
      WidgetStateTranslateSingleIntersectingSliceHandle,
      WidgetEventTranslateSingleIntersectingSliceHandleStart, WidgetEventTranslateSingleIntersectingSliceHandleEnd);
    }
}

//----------------------------------------------------------------------
void vtkDMMLSliceIntersectionWidget::CreateDefaultRepresentation()
{
  if (this->WidgetRep)
    {
    // already created
    return;
    }
  vtkDMMLApplicationLogic *dmmlAppLogic = this->GetDMMLApplicationLogic();
  if (!dmmlAppLogic)
    {
    vtkWarningMacro("vtkDMMLSliceIntersectionWidget::CreateDefaultRepresentation failed: application logic invalid");
    return;
    }

  if (this->IsSliceIntersectionInteractive())
    {
    vtkNew<vtkDMMLSliceIntersectionInteractionRepresentation> newRepInteraction;
    newRepInteraction->SetDMMLApplicationLogic(dmmlAppLogic);
    newRepInteraction->SetRenderer(this->GetRenderer());
    newRepInteraction->SetSliceNode(this->SliceNode);
    this->WidgetRep = newRepInteraction;
    }
  else
    {
    vtkNew<vtkDMMLSliceIntersectionRepresentation2D> newRep;
    newRep->SetDMMLApplicationLogic(dmmlAppLogic);
    newRep->SetRenderer(this->GetRenderer());
    newRep->SetSliceNode(this->SliceNode);
    this->WidgetRep = newRep;
    }

}

//-----------------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::CanProcessInteractionEvent(vtkDMMLInteractionEventData* eventData, double &distance2)
{
  if (!this->SliceLogic)
    {
    return false;
    }

  if (eventData->GetType() == vtkCommand::LeaveEvent)
    {
    // We cannot capture keypress events until the user clicks in the view
    // so when we are outside then we should assume that modifier
    // is not just "stuck".
    this->ModifierKeyPressedSinceLastClickAndDrag = true;

    if (this->GetSliceDisplayNode() && this->GetSliceDisplayNode()->HasActiveComponent())
      {
      // this widget has active component, therefore leave event is relevant
      distance2 = 0.0;
      return true;
      }
    }
  if (eventData->GetType() == vtkCommand::KeyPressEvent)
    {
    if (eventData->GetKeySym().find("Shift") != std::string::npos)
      {
      this->ModifierKeyPressedSinceLastClickAndDrag = true;
      }
    }

  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);
  if (widgetEvent == WidgetEventNone)
    {
    // If this event is not recognized then give a chance to process it as a click event.
    return this->CanProcessButtonClickEvent(eventData, distance2);
    }
  if (!this->GetRepresentation())
    {
    return false;
    }

  // If we are currently dragging a point then we interact everywhere
  // (we do not let any other widget take away the focus).
  if (this->WidgetState == WidgetStateMoveCrosshair
    || this->WidgetState == WidgetStateTranslate
    || this->WidgetState == WidgetStateRotateIntersectingSlices
    || this->WidgetState == WidgetStateBlend
    || this->WidgetState == WidgetStateTranslateSlice
    || this->WidgetState == WidgetStateZoomSlice
    || this->WidgetState == WidgetStateTranslateIntersectingSlicesHandle
    || this->WidgetState == WidgetStateRotateIntersectingSlicesHandle
    || this->WidgetState == WidgetStateTranslateSingleIntersectingSliceHandle
    )
    {
    distance2 = 0.0;
    return true;
    }

  // By processing the SetCrosshairPosition action at this point, rather than in ProcessInteractionEvent,
  // we allow other widgets to perform actions at the same time.
  // For example, this allows markup preview to remain visible in place mode while adjusting slice position
  // with shift + mouse-move.
  if (this->GetActionEnabled(ActionSetCrosshairPosition)
    && widgetEvent == WidgetEventSetCrosshairPositionBackground)
    {
    this->ProcessSetCrosshairBackground(eventData);
    }

  // Representation
  vtkDMMLSliceIntersectionInteractionRepresentation* rep = vtkDMMLSliceIntersectionInteractionRepresentation::SafeDownCast(this->GetRepresentation());
  if (rep)
    {
    // Currently interacting
    if (this->WidgetState == WidgetStateTranslate
      || this->WidgetState == WidgetStateRotateIntersectingSlicesHandle
      || this->WidgetState == WidgetStateTranslateSingleIntersectingSliceHandle)
      {
      distance2 = 0.0;
      return true;
      }

    // Interaction
    int foundComponentType = vtkDMMLSliceDisplayNode::ComponentNone;
    int foundComponentIndex = -1;
    double closestDistance2 = 0.0;
    double handleOpacity = 0.0;
    std::string intersectingSliceNodeID = rep->CanInteract(eventData, foundComponentType, foundComponentIndex, closestDistance2, handleOpacity);

    if (foundComponentType != vtkDMMLSliceDisplayNode::ComponentNone)
      {
      // Store closest squared distance
      distance2 = closestDistance2;

      // Store last intersecting slice node index
      this->LastIntersectingSliceNodeID = intersectingSliceNodeID;
      return true;
      }
    }

  distance2 = 1e10; // we can process this event but we let more specific widgets to claim it (if they are closer)
  return true;
}

//-----------------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::ProcessInteractionEvent(vtkDMMLInteractionEventData* eventData)
{
  if (!this->SliceLogic)
    {
    return false;
    }

  if (eventData->GetType() == vtkCommand::LeaveEvent)
    {
    // If a widget component was still active when the mouse left the view then this will deactivate it.
    this->Leave(eventData);
    return true;
    }

  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);

  bool processedEvent = true;

  switch (widgetEvent)
    {
    case WidgetEventMouseMove:
      // click-and-dragging the mouse cursor
      processedEvent = this->ProcessMouseMove(eventData);
      break;
    case  WidgetEventTouchGestureStart:
      this->ProcessTouchGestureStart(eventData);
      break;
    case WidgetEventTouchGestureEnd:
      this->ProcessTouchGestureEnd(eventData);
      break;
    case WidgetEventMoveCrosshairStart:
      this->SetWidgetState(WidgetStateMoveCrosshair);
      processedEvent = this->ProcessStartMouseDrag(eventData);
      break;
    case WidgetEventMoveCrosshairEnd:
      processedEvent = this->ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventTouchRotateSliceIntersection:
      this->ProcessTouchRotate(eventData);
      break;
    case WidgetEventTouchZoomSlice:
      this->ProcessTouchZoom(eventData);
      break;
    case WidgetEventTouchTranslateSlice:
      this->ProcessTouchTranslate(eventData);
      break;
    case WidgetEventTranslateStart:
      this->SetWidgetState(WidgetStateTranslate);
      this->SliceLogic->GetDMMLScene()->SaveStateForUndo();
      processedEvent = this->ProcessStartMouseDrag(eventData);
      break;
    case WidgetEventTranslateEnd:
      processedEvent = this->ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventRotateIntersectingSlicesStart:
      this->SliceLogic->GetDMMLScene()->SaveStateForUndo();
      // Indicate interaction in the slice node to make behavior similar to the 3D reformat widget
      this->SliceLogic->StartSliceNodeInteraction(vtkDMMLSliceNode::MultiplanarReformatFlag);
      processedEvent = this->ProcessRotateIntersectingSlicesStart(eventData);
      break;
    case WidgetEventRotateIntersectingSlicesEnd:
      processedEvent = this->ProcessEndMouseDrag(eventData);
      this->SliceLogic->EndSliceNodeInteraction();
      break;
    case WidgetEventTranslateSliceStart:
      this->SliceLogic->GetDMMLScene()->SaveStateForUndo();
      this->SliceLogic->StartSliceNodeInteraction(vtkDMMLSliceNode::XYZOriginFlag);
      this->SetWidgetState(WidgetStateTranslateSlice);
      processedEvent = this->ProcessStartMouseDrag(eventData);
      break;
    case WidgetEventTranslateSliceEnd:
      processedEvent = this->ProcessEndMouseDrag(eventData);
      this->SliceLogic->EndSliceNodeInteraction();
      break;
    case WidgetEventZoomSliceStart:
      this->SliceLogic->GetDMMLScene()->SaveStateForUndo();
      this->SetWidgetState(WidgetStateZoomSlice);
      this->SliceLogic->StartSliceNodeInteraction(vtkDMMLSliceNode::FieldOfViewFlag);
      this->SliceLogic->GetSliceNode()->GetFieldOfView(this->StartActionFOV);
      processedEvent = this->ProcessStartMouseDrag(eventData);
      break;
    case WidgetEventZoomSliceEnd:
      processedEvent = this->ProcessEndMouseDrag(eventData);
      this->SliceLogic->EndSliceNodeInteraction();
      break;
    case WidgetEventBlendStart:
      {
      this->SetWidgetState(WidgetStateBlend);
      vtkDMMLSliceCompositeNode *sliceCompositeNode = this->SliceLogic->GetSliceCompositeNode();
      this->SliceLogic->GetDMMLScene()->SaveStateForUndo();
      this->LastForegroundOpacity = sliceCompositeNode->GetForegroundOpacity();
      this->LastLabelOpacity = this->GetLabelOpacity();
      this->StartActionSegmentationDisplayNode = this->GetVisibleSegmentationDisplayNode();
      processedEvent = this->ProcessStartMouseDrag(eventData);
      // It would be nicer to call this->SliceLogic->StartSliceCompositeNodeInteraction(...) to
      // synchronize opacity value. However, since synchronization occurs via GUI widgets
      // (the slider in qDMMLSliceControllerWidget), there is no immediate need to change this.
      }
      break;
    case WidgetEventBlendEnd:
      processedEvent = this->ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventSetCrosshairPosition:
      this->ProcessSetCrosshair(eventData);
      break;
    case WidgetEventToggleLabelOpacity:
      {
      this->StartActionSegmentationDisplayNode = nullptr;
      this->SliceLogic->GetDMMLScene()->SaveStateForUndo();
      this->StartActionSegmentationDisplayNode = nullptr;
      double opacity = this->GetLabelOpacity();
      if (opacity != 0.0)
        {
        this->LastLabelOpacity = opacity;
        this->SetLabelOpacity(0.0);
        }
      else
        {
        this->SetLabelOpacity(this->LastLabelOpacity);
        }
      }
      break;
    case WidgetEventToggleForegroundOpacity:
      {
      vtkDMMLSliceCompositeNode *sliceCompositeNode = this->SliceLogic->GetSliceCompositeNode();
      this->SliceLogic->GetDMMLScene()->SaveStateForUndo();
      double opacity = sliceCompositeNode->GetForegroundOpacity();
      if (opacity != 0.0)
        {
        this->LastForegroundOpacity = opacity;
        sliceCompositeNode->SetForegroundOpacity(0.0);
        }
      else
        {
        sliceCompositeNode->SetForegroundOpacity(this->LastForegroundOpacity);
        }
      }
      break;
    case WidgetEventIncrementSlice:
      this->IncrementSlice();
      break;
    case WidgetEventDecrementSlice:
      this->DecrementSlice();
      break;
    case WidgetEventToggleSliceVisibility:
      this->SliceLogic->GetDMMLScene()->SaveStateForUndo();
      this->GetSliceNode()->SetSliceVisible(!this->GetSliceNode()->GetSliceVisible());
      break;
    case WidgetEventToggleAllSlicesVisibility:
      // TODO: need to set all slices visible
      this->SliceLogic->GetDMMLScene()->SaveStateForUndo();
      this->GetSliceNode()->SetSliceVisible(!this->GetSliceNode()->GetSliceVisible());
      break;
    case WidgetEventResetFieldOfView:
      {
      this->SliceLogic->GetDMMLScene()->SaveStateForUndo();
      this->SliceLogic->StartSliceNodeInteraction(vtkDMMLSliceNode::ResetFieldOfViewFlag);
      this->SliceLogic->FitSliceToAll();
      this->GetSliceNode()->UpdateMatrices();
      this->SliceLogic->EndSliceNodeInteraction();
      }
      break;
    case WidgetEventZoomInSlice:
      this->ScaleZoom(1.2, eventData);
      break;
    case WidgetEventZoomOutSlice:
      this->ScaleZoom(0.8, eventData);
      break;
    break;
    case WidgetEventShowPreviousBackgroundVolume:
      this->SliceLogic->GetDMMLScene()->SaveStateForUndo();
      this->CycleVolumeLayer(LayerBackground, -1);
      break;
    case WidgetEventShowNextBackgroundVolume:
      this->SliceLogic->GetDMMLScene()->SaveStateForUndo();
      this->CycleVolumeLayer(LayerBackground, 1);
      break;
    case WidgetEventShowPreviousForegroundVolume:
      this->SliceLogic->GetDMMLScene()->SaveStateForUndo();
      this->CycleVolumeLayer(LayerForeground, -1);
      break;
    case WidgetEventShowNextForegroundVolume:
      this->SliceLogic->GetDMMLScene()->SaveStateForUndo();
      this->CycleVolumeLayer(LayerForeground, 1);
      break;
    case WidgetEventMenu:
      processedEvent = this->ProcessWidgetMenu(eventData);
      break;
    case WidgetEventMaximizeView:
      processedEvent = this->ProcessMaximizeView(eventData);
      break;

    case WidgetEventRotateIntersectingSlicesHandleStart:
      this->SliceLogic->GetDMMLScene()->SaveStateForUndo();
      processedEvent = this->ProcessRotateIntersectingSlicesHandleStart(eventData);
      break;
    case WidgetEventRotateIntersectingSlicesHandleEnd:
      processedEvent = this->ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventTranslateIntersectingSlicesHandleStart:
      this->SliceLogic->GetDMMLScene()->SaveStateForUndo();
      processedEvent = this->ProcessTranslateIntersectingSlicesHandleStart(eventData);
      break;
    case WidgetEventTranslateIntersectingSlicesHandleEnd:
      processedEvent = this->ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventTranslateSingleIntersectingSliceHandleStart:
      this->SliceLogic->GetDMMLScene()->SaveStateForUndo();
      processedEvent = this->ProcessTranslateSingleIntersectingSliceHandleStart(eventData);
      break;
    case WidgetEventTranslateSingleIntersectingSliceHandleEnd:
      processedEvent = this->ProcessEndMouseDrag(eventData);
      break;

    default:
      processedEvent = false;
    }

  if (!processedEvent)
    {
    processedEvent = this->ProcessButtonClickEvent(eventData);
    }

  return processedEvent;
}

//-------------------------------------------------------------------------
void vtkDMMLSliceIntersectionWidget::Leave(vtkDMMLInteractionEventData* eventData)
{
  // Ensure that EndInteractionEvent is invoked, even if interrupted by an unexpected event
  if (this->WidgetState == WidgetStateTranslate
    || this->WidgetState == WidgetStateTranslateSlice
    || this->WidgetState == WidgetStateZoomSlice
    || this->WidgetState == WidgetStateBlend
    || this->WidgetState == WidgetStateRotateIntersectingSlices
    || this->WidgetState == WidgetStateTranslateIntersectingSlicesHandle
    || this->WidgetState == WidgetStateTranslateSingleIntersectingSliceHandle
    || this->WidgetState == WidgetStateRotateIntersectingSlicesHandle)
    {
    this->ProcessEndMouseDrag(eventData);
    }

  vtkDMMLSliceDisplayNode* sliceDisplayNode = this->GetSliceDisplayNode();
  if (sliceDisplayNode)
    {
    std::string interactionContext;
    if (eventData)
      {
      interactionContext = eventData->GetInteractionContextName();
      }
    sliceDisplayNode->SetActiveComponent(vtkDMMLSliceDisplayNode::ComponentNone, -1, interactionContext);
    }
  this->Superclass::Leave(eventData);
}

//-------------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::ProcessMouseMove(vtkDMMLInteractionEventData* eventData)
{
  vtkDMMLSliceIntersectionInteractionRepresentation* interactiveRep = vtkDMMLSliceIntersectionInteractionRepresentation::SafeDownCast(this->WidgetRep);
  if (!this->WidgetRep || !eventData)
    {
    return false;
    }

  switch (this->WidgetState)
    {
    case WidgetStateRotateIntersectingSlices:
      this->ProcessRotateIntersectingSlices(eventData);
      break;
    case WidgetStateTranslate:
      {
      const double* worldPos = eventData->GetWorldPosition();
      this->GetSliceNode()->JumpAllSlices(worldPos[0], worldPos[1], worldPos[2]);
      }
      break;
    case WidgetStateMoveCrosshair:
      this->ProcessSetCrosshair(eventData);
      break;
    case WidgetStateBlend:
      this->ProcessBlend(eventData);
      break;
    case WidgetStateTranslateSlice:
      this->ProcessTranslateSlice(eventData);
      break;
    case WidgetStateZoomSlice:
      this->ProcessZoomSlice(eventData);
      break;
    case WidgetStateIdle: // if moving over an intersection in idle mode then this will change the widget state to WidgetStateOn...
    case WidgetStateOnWidget:
    case WidgetStateOnTranslateIntersectingSlicesHandle:
    case WidgetStateOnRotateIntersectingSlicesHandle:
    case WidgetStateOnTranslateSingleIntersectingSliceHandle:
      if (interactiveRep)
        {
        // Update widget state according to distance to interaction handles
        int foundComponentType = vtkDMMLSliceDisplayNode::ComponentNone;
        int foundComponentIndex = -1;
        double closestDistance2 = 0.0;
        double handleOpacity = 0.0;
        std::string intersectingSliceNodeID = interactiveRep->CanInteract(eventData, foundComponentType, foundComponentIndex, closestDistance2, handleOpacity);
        if (foundComponentType == vtkDMMLSliceDisplayNode::ComponentNone)
          {
          if (this->WidgetState != WidgetStateIdle)
            {
            this->SetWidgetState(WidgetStateIdle);
            this->SliceLogic->EndSliceNodeInteraction();
            }
          }
        else if (foundComponentType == vtkDMMLSliceDisplayNode::ComponentTranslateIntersectingSlicesHandle)
          {
          this->SetWidgetState(WidgetStateOnTranslateIntersectingSlicesHandle);
          }
        else if (foundComponentType == vtkDMMLSliceDisplayNode::ComponentRotateIntersectingSlicesHandle)
          {
          this->SetWidgetState(WidgetStateOnRotateIntersectingSlicesHandle);
          }
        else if (foundComponentType == vtkDMMLSliceDisplayNode::ComponentTranslateSingleIntersectingSliceHandle)
          {
          this->SetWidgetState(WidgetStateOnTranslateSingleIntersectingSliceHandle);
          }
        else
          {
          this->SetWidgetState(WidgetStateOnWidget);
          }

        // Store last intersecting slice node index
        if (foundComponentType != vtkDMMLSliceDisplayNode::ComponentNone)
          {
          this->LastIntersectingSliceNodeID = intersectingSliceNodeID;
          }

        vtkDMMLSliceDisplayNode* sliceDisplayNode = this->GetSliceDisplayNode();
        if (sliceDisplayNode)
          {
          sliceDisplayNode->SetActiveComponent(foundComponentType, foundComponentIndex, eventData->GetInteractionContextName());
          }
        }
        break;
    case WidgetStateTranslateIntersectingSlicesHandle:
    case WidgetStateTranslateSingleIntersectingSliceHandle:
    case WidgetStateRotateIntersectingSlicesHandle:
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
      bool inSliceView = interactiveRep->IsMouseCursorInSliceView(eventPos);

      // Make handles disappear during interaction
      vtkDMMLSliceDisplayNode* sliceDisplayNode = this->GetSliceDisplayNode();
      if (sliceDisplayNode)
        {
        sliceDisplayNode->SetActiveComponent(vtkDMMLSliceDisplayNode::ComponentSliceIntersection, 0, eventData->GetInteractionContextName());
        }

      if (this->WidgetState == WidgetStateTranslateIntersectingSlicesHandle)
        {
        if (inSliceView) // Stop interaction if mouse cursor is outside of the slice view
          {
          const double* worldPos = eventData->GetWorldPosition();
          this->GetSliceNode()->JumpAllSlices(worldPos[0], worldPos[1], worldPos[2]);
          }
        }
      else if (this->WidgetState == WidgetStateTranslateSingleIntersectingSliceHandle)
        {
        if (inSliceView) // Stop interaction if mouse cursor is outside of the slice view
          {
          this->ProcessTranslateSingleIntersectingSliceHandle(eventData);
          }
        }
      else if (this->WidgetState == WidgetStateRotateIntersectingSlicesHandle)
        {
        this->ProcessRotateIntersectingSlicesHandle(eventData);
        }
      }
    }

  return true;
}

//-------------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::ProcessStartMouseDrag(vtkDMMLInteractionEventData* eventData)
{
  if (!this->WidgetRep)
    {
    return false;
    }

  const int* displayPos = eventData->GetDisplayPosition();

  this->StartEventPosition[0] = displayPos[0];
  this->StartEventPosition[1] = displayPos[1];

  this->PreviousEventPosition[0] = this->StartEventPosition[0];
  this->PreviousEventPosition[1] = this->StartEventPosition[1];

  this->ProcessMouseMove(eventData);

  return true;
}

//----------------------------------------------------------------------------
double vtkDMMLSliceIntersectionWidget::GetSliceRotationAngleRad(double eventPos[2])
{
  return atan2(eventPos[1] - this->StartRotationCenter[1],
    eventPos[0] - this->StartRotationCenter[0]);
}


//-------------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::ProcessEndMouseDrag(vtkDMMLInteractionEventData* eventData)
{
  if (!this->WidgetRep)
    {
    return false;
    }

  if (this->WidgetState == WidgetStateIdle)
    {
    return false;
    }
  this->SetWidgetState(WidgetStateIdle);

  // Prevent shift+mousemove events after click-and-drag (until shift is pressed again)
  this->ModifierKeyPressedSinceLastClickAndDrag = false;

  // only claim this as processed if the mouse was moved (this lets the event interpreted as button click)
  bool processedEvent = eventData->GetMouseMovedSinceButtonDown();

  // Simulate a mousemove event to update the widget state according to the current position.
  // Without this, handles would always disappear at the end of drag-and-drop, even if the mouse pointer is over the widget
  // (which would break Nearby handle visible mode).
  int originalEventType = eventData->GetType();
  eventData->SetType(vtkCommand::MouseMoveEvent);
  this->ProcessMouseMove(eventData);
  eventData->SetType(originalEventType);

  return processedEvent;
}

//-------------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::ProcessTouchGestureStart(vtkDMMLInteractionEventData* vtkNotUsed(eventData))
{
  this->SetWidgetState(WidgetStateTouchGesture);
  return true;
}

//-------------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::ProcessTouchGestureEnd(vtkDMMLInteractionEventData* vtkNotUsed(eventData))
{
  this->TotalTouchRotation = 0.0;
  this->TouchRotateEnabled = false;
  this->TotalTouchZoom = 0.0;
  this->TouchZoomEnabled = false;
  this->TotalTouchTranslation = 0.0;
  this->TouchTranslationEnabled = false;
  this->SetWidgetState(WidgetStateIdle);
  return true;
}

//-------------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::ProcessTouchRotate(vtkDMMLInteractionEventData* eventData)
{
  this->TotalTouchRotation += eventData->GetRotation() - eventData->GetLastRotation();
  if (this->TouchRotateEnabled || std::abs(this->TotalTouchRotation) >= this->TouchRotationThreshold)
    {
    this->Rotate(vtkMath::RadiansFromDegrees(eventData->GetRotation() - eventData->GetLastRotation()));
    this->TouchRotateEnabled = true;
    }
  return true;
}

//-------------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::ProcessTouchZoom(vtkDMMLInteractionEventData* eventData)
{
  this->TotalTouchZoom += eventData->GetLastScale() / eventData->GetScale();
  if (this->TouchZoomEnabled || std::abs(this->TotalTouchZoom - 1.0) > this->TouchZoomThreshold)
    {
    this->ScaleZoom(eventData->GetLastScale() / eventData->GetScale(), eventData);
    this->TouchZoomEnabled = true;
    }
  return true;
}

//-------------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::ProcessTouchTranslate(vtkDMMLInteractionEventData* eventData)
{
  vtkDMMLSliceNode* sliceNode = this->SliceLogic->GetSliceNode();

  vtkMatrix4x4* xyToSlice = sliceNode->GetXYToSlice();
  const double* translate = eventData->GetTranslation();
  double translation[2] = {
    xyToSlice->GetElement(0, 0) * translate[0],
    xyToSlice->GetElement(1, 1) * translate[1]
  };

  this->TotalTouchTranslation += vtkMath::Norm2D(translate);

  if (this->TouchTranslationEnabled || this->TotalTouchTranslation >= this->TouchTranslationThreshold)
    {
    double sliceOrigin[3];
    sliceNode->GetXYZOrigin(sliceOrigin);
    sliceNode->SetSliceOrigin(sliceOrigin[0] - translation[0], sliceOrigin[1] - translation[1], 0);
    this->TouchTranslationEnabled = true;
    }

  return true;
}

//----------------------------------------------------------------------------------
void vtkDMMLSliceIntersectionWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------------
void vtkDMMLSliceIntersectionWidget::SetDMMLApplicationLogic(vtkDMMLApplicationLogic* appLogic)
{
  if (appLogic == this->ApplicationLogic)
    {
    return;
    }
  this->Superclass::SetDMMLApplicationLogic(appLogic);
  vtkCollection* sliceLogics = nullptr;
  this->SliceLogic = nullptr;
  if (appLogic)
    {
    sliceLogics = appLogic->GetSliceLogics();
    }
  if (sliceLogics != this->SliceLogics)
    {
    if (this->SliceLogics)
      {
      this->SliceLogics->RemoveObserver(this->SliceLogicsModifiedCommand);
      }
    if (sliceLogics)
      {
      sliceLogics->AddObserver(vtkCommand::ModifiedEvent, this->SliceLogicsModifiedCommand);
      }
    this->SliceLogics = sliceLogics;
    if (this->GetSliceNode())
      {
      this->SliceLogic = this->GetDMMLApplicationLogic()->GetSliceLogic(this->GetSliceNode());
      }
    }
}

//----------------------------------------------------------------------------------
void vtkDMMLSliceIntersectionWidget::SliceLogicsModifiedCallback(vtkObject* vtkNotUsed(caller),
  unsigned long vtkNotUsed(eid), void* clientData, void* vtkNotUsed(callData))
{
  vtkDMMLSliceIntersectionWidget* self = vtkDMMLSliceIntersectionWidget::SafeDownCast((vtkObject*)clientData);
  if (!self)
    {
    return;
    }
  vtkDMMLSliceIntersectionRepresentation2D* rep = vtkDMMLSliceIntersectionRepresentation2D::SafeDownCast(self->WidgetRep);
  if (rep)
    {
    rep->UpdateIntersectingSliceNodes();
    }
  vtkDMMLSliceIntersectionInteractionRepresentation* repInteraction = vtkDMMLSliceIntersectionInteractionRepresentation::SafeDownCast(self->WidgetRep);
  if (repInteraction)
    {
    repInteraction->UpdateIntersectingSliceNodes();
    }
  self->SliceLogic = nullptr;
  if (self->GetDMMLApplicationLogic())
    {
    self->SliceLogic = self->GetDMMLApplicationLogic()->GetSliceLogic(self->GetSliceNode());
    }
}

//----------------------------------------------------------------------------------
void vtkDMMLSliceIntersectionWidget::SliceModifiedCallback(vtkObject* vtkNotUsed(caller),
  unsigned long vtkNotUsed(eid), void* clientData, void* vtkNotUsed(callData))
{
  vtkDMMLSliceIntersectionWidget* self = vtkDMMLSliceIntersectionWidget::SafeDownCast((vtkObject*)clientData);
  if (!self)
    {
    return;
    }

  vtkDMMLSliceNode* sliceNode = self->GetSliceNode();
  if (!sliceNode)
    {
    return;
    }

  bool needToUpdateRepresentation = false;


  if (self->IsSliceIntersectionInteractive())
    {
    needToUpdateRepresentation = !vtkDMMLSliceIntersectionInteractionRepresentation::SafeDownCast(self->WidgetRep);
    }
  else
    {
    needToUpdateRepresentation = !vtkDMMLSliceIntersectionRepresentation2D::SafeDownCast(self->WidgetRep);
    }
  if (needToUpdateRepresentation)
    {
    self->WidgetRep = nullptr;
    self->CreateDefaultRepresentation();
    }
}

//----------------------------------------------------------------------------------
vtkDMMLSliceDisplayNode* vtkDMMLSliceIntersectionWidget::GetSliceDisplayNode()
{
  if (!this->SliceLogic)
    {
    return nullptr;
    }
  return this->SliceLogic->GetSliceDisplayNode();
}

//----------------------------------------------------------------------------------
void vtkDMMLSliceIntersectionWidget::SetSliceNode(vtkDMMLSliceNode* sliceNode)
{
  if (this->SliceNode == sliceNode)
    {
    // no change
    return;
    }

  if (this->SliceNode)
    {
    this->SliceNode->RemoveObserver(this->SliceModifiedCommand);
    }
  if (sliceNode)
    {
    sliceNode->AddObserver(vtkCommand::ModifiedEvent, this->SliceModifiedCommand);
    }

  this->SliceNode = sliceNode;

  // Update slice logic
  this->SliceLogic = nullptr;
  if (this->GetDMMLApplicationLogic())
    {
    this->SliceLogic = this->GetDMMLApplicationLogic()->GetSliceLogic(this->SliceNode);
    }

  // Update representation
  vtkDMMLSliceIntersectionRepresentation2D* rep = vtkDMMLSliceIntersectionRepresentation2D::SafeDownCast(this->WidgetRep);
  if (rep)
    {
    rep->SetSliceNode(sliceNode);
    }
  vtkDMMLSliceIntersectionInteractionRepresentation* repInteraction = vtkDMMLSliceIntersectionInteractionRepresentation::SafeDownCast(this->WidgetRep);
  if (repInteraction)
    {
    repInteraction->SetSliceNode(sliceNode);
    }
}

//----------------------------------------------------------------------------------
vtkDMMLSliceNode* vtkDMMLSliceIntersectionWidget::GetSliceNode()
{
  return this->SliceNode;
}

//----------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::ProcessRotateIntersectingSlicesStart(vtkDMMLInteractionEventData* eventData)
{
  vtkDMMLSliceIntersectionRepresentation2D* rep = vtkDMMLSliceIntersectionRepresentation2D::SafeDownCast(this->WidgetRep);
  vtkDMMLSliceIntersectionInteractionRepresentation* repInteraction = vtkDMMLSliceIntersectionInteractionRepresentation::SafeDownCast(this->WidgetRep);
  if ((!rep) && (!repInteraction))
    {
    return false;
    }

  this->SetWidgetState(WidgetStateRotateIntersectingSlices);

  // Save rotation center
  double* sliceIntersectionPoint = {};
  if (rep)
    {
    sliceIntersectionPoint = rep->GetSliceIntersectionPoint();
    }
  else
    {
    sliceIntersectionPoint = repInteraction->GetSliceIntersectionPoint();
    }
  this->StartRotationCenter[0] = sliceIntersectionPoint[0];
  this->StartRotationCenter[1] = sliceIntersectionPoint[1];
  double startRotationCenterXY[4] = { sliceIntersectionPoint[0] , sliceIntersectionPoint[1], 0.0, 1.0 };
  this->GetSliceNode()->GetXYToRAS()->MultiplyPoint(startRotationCenterXY, this->StartRotationCenter_RAS);

  // Save initial rotation angle
  const int* displayPos = eventData->GetDisplayPosition();
  double displayPosDouble[2] = { static_cast<double>(displayPos[0]), static_cast<double>(displayPos[1]) };
  this->PreviousRotationAngleRad = this->GetSliceRotationAngleRad(displayPosDouble);

  return this->ProcessStartMouseDrag(eventData);
}

//----------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::ProcessRotateIntersectingSlices(vtkDMMLInteractionEventData* eventData)
{
  vtkDMMLSliceIntersectionRepresentation2D* rep = vtkDMMLSliceIntersectionRepresentation2D::SafeDownCast(this->WidgetRep);
  vtkDMMLSliceIntersectionInteractionRepresentation* repInteraction = vtkDMMLSliceIntersectionInteractionRepresentation::SafeDownCast(this->WidgetRep);
  if ((!rep) && (!repInteraction))
    {
    return false;
    }

  const int* displayPos = eventData->GetDisplayPosition();
  double displayPosDouble[2] = { static_cast<double>(displayPos[0]), static_cast<double>(displayPos[1]) };
  double sliceRotationAngleRad = this->GetSliceRotationAngleRad(displayPosDouble);

  vtkMatrix4x4* sliceToRAS = this->GetSliceNode()->GetSliceToRAS();
  vtkNew<vtkTransform> rotatedSliceToSliceTransform;

  rotatedSliceToSliceTransform->Translate(this->StartRotationCenter_RAS[0], this->StartRotationCenter_RAS[1], this->StartRotationCenter_RAS[2]);
  double rotationDirection = vtkMath::Determinant3x3(sliceToRAS->Element[0], sliceToRAS->Element[1], sliceToRAS->Element[2]) >= 0 ? 1.0 : -1.0;
  rotatedSliceToSliceTransform->RotateWXYZ(rotationDirection*vtkMath::DegreesFromRadians(sliceRotationAngleRad - this->PreviousRotationAngleRad),
    sliceToRAS->GetElement(0, 2), sliceToRAS->GetElement(1, 2), sliceToRAS->GetElement(2, 2));
  rotatedSliceToSliceTransform->Translate(-this->StartRotationCenter_RAS[0], -this->StartRotationCenter_RAS[1], -this->StartRotationCenter_RAS[2]);

  this->PreviousRotationAngleRad = sliceRotationAngleRad;
  this->PreviousEventPosition[0] = displayPos[0];
  this->PreviousEventPosition[1] = displayPos[1];

  if (rep)
    {
    rep->TransformIntersectingSlices(rotatedSliceToSliceTransform->GetMatrix());
    }
  else
    {
    repInteraction->TransformIntersectingSlices(rotatedSliceToSliceTransform->GetMatrix());
    }

  return true;
}

//----------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::Rotate(double sliceRotationAngleRad)
{
  vtkDMMLSliceIntersectionRepresentation2D* rep = vtkDMMLSliceIntersectionRepresentation2D::SafeDownCast(this->WidgetRep);
  if (!rep)
    {
    return false;
    }
  vtkMatrix4x4* sliceToRAS = this->GetSliceNode()->GetSliceToRAS();
  vtkNew<vtkTransform> rotatedSliceToSliceTransform;

  double* sliceIntersectionPoint = rep->GetSliceIntersectionPoint();
  double startRotationCenterXY[4] = { sliceIntersectionPoint[0] , sliceIntersectionPoint[1], 0.0, 1.0 };
  this->GetSliceNode()->GetXYToRAS()->MultiplyPoint(startRotationCenterXY, this->StartRotationCenter_RAS);

  rotatedSliceToSliceTransform->Translate(this->StartRotationCenter_RAS[0], this->StartRotationCenter_RAS[1], this->StartRotationCenter_RAS[2]);
  double rotationDirection = vtkMath::Determinant3x3(sliceToRAS->Element[0], sliceToRAS->Element[1], sliceToRAS->Element[2]) >= 0 ? 1.0 : -1.0;
  rotatedSliceToSliceTransform->RotateWXYZ(rotationDirection*vtkMath::DegreesFromRadians(sliceRotationAngleRad),
    sliceToRAS->GetElement(0, 2), sliceToRAS->GetElement(1, 2), sliceToRAS->GetElement(2, 2));
  rotatedSliceToSliceTransform->Translate(-this->StartRotationCenter_RAS[0], -this->StartRotationCenter_RAS[1], -this->StartRotationCenter_RAS[2]);

  this->PreviousRotationAngleRad = sliceRotationAngleRad;

  rep->TransformIntersectingSlices(rotatedSliceToSliceTransform->GetMatrix());
  return true;
}

//----------------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::ProcessBlend(vtkDMMLInteractionEventData* eventData)
{
  const int* eventPosition = eventData->GetDisplayPosition();

  const int* windowSize = this->Renderer->GetRenderWindow()->GetSize();
  double windowMinSize = std::min(windowSize[0], windowSize[1]);

  int deltaY = eventPosition[1] - this->PreviousEventPosition[1];
  double offsetY = (2.0 * deltaY) / windowMinSize;
  double newForegroundOpacity =
    this->LastForegroundOpacity + offsetY;
  newForegroundOpacity = std::min(std::max(newForegroundOpacity, 0.), 1.);
  vtkDMMLSliceCompositeNode *sliceCompositeNode = this->SliceLogic->GetSliceCompositeNode();
  if (sliceCompositeNode->GetForegroundVolumeID() != nullptr)
    {
    sliceCompositeNode->SetForegroundOpacity(newForegroundOpacity);
    this->LastForegroundOpacity = newForegroundOpacity;
    }
  int deltaX = eventPosition[0] - this->PreviousEventPosition[0];
  double offsetX = (2.0 * deltaX) / windowMinSize;
  double newLabelOpacity = this->LastLabelOpacity + offsetX;
  newLabelOpacity = std::min(std::max(newLabelOpacity, 0.), 1.);
  if (sliceCompositeNode->GetLabelVolumeID() != nullptr || this->StartActionSegmentationDisplayNode != nullptr)
    {
    this->SetLabelOpacity(newLabelOpacity);
    this->LastLabelOpacity = newLabelOpacity;
    }

  this->PreviousEventPosition[0] = eventPosition[0];
  this->PreviousEventPosition[1] = eventPosition[1];

  return true;
}


//----------------------------------------------------------------------------
void vtkDMMLSliceIntersectionWidget::SetLabelOpacity(double opacity)
{
  // If a labelmap node is selected then adjust opacity of that
  vtkDMMLSliceCompositeNode *sliceCompositeNode = this->SliceLogic->GetSliceCompositeNode();
  if (sliceCompositeNode->GetLabelVolumeID())
  {
    sliceCompositeNode->SetLabelOpacity(opacity);
    return;
  }
  // No labelmap node is selected, adjust segmentation node instead
  vtkDMMLSegmentationDisplayNode* displayNode = this->StartActionSegmentationDisplayNode;
  if (!displayNode)
  {
    displayNode = this->GetVisibleSegmentationDisplayNode();
  }
  if (!displayNode)
  {
    return;
  }
  displayNode->SetOpacity(opacity);
}

//----------------------------------------------------------------------------
double vtkDMMLSliceIntersectionWidget::GetLabelOpacity()
{
  // If a labelmap node is selected then get opacity of that
  vtkDMMLSliceCompositeNode *sliceCompositeNode = this->SliceLogic->GetSliceCompositeNode();
  if (sliceCompositeNode->GetLabelVolumeID())
  {
    return sliceCompositeNode->GetLabelOpacity();
  }
  // No labelmap node is selected, use segmentation node instead
  vtkDMMLSegmentationDisplayNode* displayNode = this->StartActionSegmentationDisplayNode;
  if (!displayNode)
  {
    displayNode = this->GetVisibleSegmentationDisplayNode();
  }
  if (!displayNode)
  {
    return 0;
  }
  return displayNode->GetOpacity();
}

//----------------------------------------------------------------------------
vtkDMMLSegmentationDisplayNode* vtkDMMLSliceIntersectionWidget::GetVisibleSegmentationDisplayNode()
{
  vtkDMMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();
  vtkDMMLScene* scene = this->SliceLogic->GetDMMLScene();
  std::vector<vtkDMMLNode*> displayNodes;
  int nnodes = scene ? scene->GetNodesByClass("vtkDMMLSegmentationDisplayNode", displayNodes) : 0;
  for (int i = 0; i < nnodes; i++)
  {
    vtkDMMLSegmentationDisplayNode* displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(displayNodes[i]);
    if (displayNode
      && (displayNode->GetVisibility2DOutline() || displayNode->GetVisibility2DFill())
      && displayNode->IsDisplayableInView(sliceNode->GetID()))
    {
      return displayNode;
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------
double vtkDMMLSliceIntersectionWidget::GetSliceSpacing()
{
  double spacing;
  vtkDMMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();

  if (sliceNode->GetSliceSpacingMode() == vtkDMMLSliceNode::PrescribedSliceSpacingMode)
    {
    spacing = sliceNode->GetPrescribedSliceSpacing()[2];
    }
  else
    {
    spacing = this->SliceLogic->GetLowestVolumeSliceSpacing()[2];
    }
  return spacing;
}

//----------------------------------------------------------------------------
void vtkDMMLSliceIntersectionWidget::IncrementSlice()
{
  this->MoveSlice(this->GetSliceSpacing());
}
//----------------------------------------------------------------------------
void vtkDMMLSliceIntersectionWidget::DecrementSlice()
{
  this->MoveSlice(-1. * this->GetSliceSpacing());
}
//----------------------------------------------------------------------------
void vtkDMMLSliceIntersectionWidget::MoveSlice(double delta)
{
  double offset = this->SliceLogic->GetSliceOffset();
  double newOffset = offset + delta;

  double sliceBounds[6] = {0, -1, 0, -1, 0, -1};
  this->SliceLogic->GetSliceBounds(sliceBounds);
  if (newOffset >= sliceBounds[4] && newOffset <= sliceBounds[5])
    {
    this->SliceLogic->StartSliceNodeInteraction(vtkDMMLSliceNode::SliceToRASFlag);
    this->SliceLogic->SetSliceOffset(newOffset);
    this->SliceLogic->EndSliceNodeInteraction();
    }
}

//----------------------------------------------------------------------------
void vtkDMMLSliceIntersectionWidget::CycleVolumeLayer(int layer, int direction)
{
  // first, find the current volume index for the given layer (can be nullptr)
  vtkDMMLScene *scene = this->SliceLogic->GetDMMLScene();
  vtkDMMLSliceCompositeNode *sliceCompositeNode = this->SliceLogic->GetSliceCompositeNode();
  const char *volumeID = nullptr;
  switch (layer)
    {
    case 0: { volumeID = sliceCompositeNode->GetBackgroundVolumeID(); } break;
    case 1: { volumeID = sliceCompositeNode->GetForegroundVolumeID(); } break;
    case 2: { volumeID = sliceCompositeNode->GetLabelVolumeID(); } break;
    }
  vtkDMMLVolumeNode *volumeNode = vtkDMMLVolumeNode::SafeDownCast(scene->GetNodeByID(volumeID));

  // now figure out which one it is in the list
  int volumeCount = scene->GetNumberOfNodesByClass("vtkDMMLVolumeNode");
  int volumeIndex;
  for (volumeIndex = 0; volumeIndex < volumeCount; volumeIndex++)
    {
    if (volumeNode == scene->GetNthNodeByClass(volumeIndex, "vtkDMMLVolumeNode"))
      {
      break;
      }
    }

  // now increment by direction, and clamp to number of nodes
  volumeIndex += direction;
  if (volumeIndex >= volumeCount)
    {
    volumeIndex = 0;
    }
  if (volumeIndex < 0)
    {
    volumeIndex = volumeCount - 1;
    }

  // if we found a node, set it in the given layer
  volumeNode = vtkDMMLVolumeNode::SafeDownCast(
                  scene->GetNthNodeByClass(volumeIndex, "vtkDMMLVolumeNode"));
  if (volumeNode)
    {
    switch (layer)
      {
      case 0: { sliceCompositeNode->SetBackgroundVolumeID(volumeNode->GetID()); } break;
      case 1: { sliceCompositeNode->SetForegroundVolumeID(volumeNode->GetID()); } break;
      case 2: { sliceCompositeNode->SetLabelVolumeID(volumeNode->GetID()); } break;
      }
    }
}

//----------------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::ProcessTranslateSlice(vtkDMMLInteractionEventData* eventData)
{
  vtkDMMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();

  double xyz[3];
  sliceNode->GetXYZOrigin(xyz);

  const int* eventPosition = eventData->GetDisplayPosition();

  // account for zoom using XYToSlice matrix
  vtkMatrix4x4* xyToSlice = sliceNode->GetXYToSlice();
  double deltaX = xyToSlice->GetElement(0, 0)*(this->PreviousEventPosition[0] - eventPosition[0]);
  double deltaY = xyToSlice->GetElement(1, 1)*(this->PreviousEventPosition[1] - eventPosition[1]);

  sliceNode->SetSliceOrigin(xyz[0] + deltaX, xyz[1] + deltaY, 0);

  this->PreviousEventPosition[0] = eventPosition[0];
  this->PreviousEventPosition[1] = eventPosition[1];
  return true;
}

//----------------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::ProcessZoomSlice(vtkDMMLInteractionEventData* eventData)
{
  const int* eventPosition = eventData->GetDisplayPosition();

  const int* windowSize = this->GetRenderer()->GetRenderWindow()->GetSize();

  int deltaY = eventPosition[1] - this->StartEventPosition[1];
  double percent = (windowSize[1] + deltaY) / (1.0 * windowSize[1]);

  // the factor operation is so 'z' isn't changed and the
  // slider can still move through the full range
  if (percent > 0.)
    {
    double newFOVx = this->StartActionFOV[0] * percent;
    double newFOVy = this->StartActionFOV[1] * percent;
    double newFOVz = this->StartActionFOV[2];
    vtkDMMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();
    sliceNode->SetFieldOfView(newFOVx, newFOVy, newFOVz);
    sliceNode->UpdateMatrices();
    }
  return true;
}

//----------------------------------------------------------------------------
void vtkDMMLSliceIntersectionWidget::ScaleZoom(double zoomScaleFactor, vtkDMMLInteractionEventData* eventData)
{
  // the factor operation is so 'z' isn't changed and the
  // slider can still move through the full range
  if (zoomScaleFactor <= 0)
    {
    vtkWarningMacro("vtkDMMLSliceViewInteractorStyle::ScaleZoom: invalid zoom scale factor (" << zoomScaleFactor);
    return;
    }
  this->SliceLogic->StartSliceNodeInteraction(vtkDMMLSliceNode::FieldOfViewFlag);
  vtkDMMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();
  int wasModifying = sliceNode->StartModify();

  // Get distance of event position from slice center
  const int* eventPosition = eventData->GetDisplayPosition();
  const int* windowSize = this->GetRenderer()->GetRenderWindow()->GetSize();
  vtkMatrix4x4* xyToSlice = sliceNode->GetXYToSlice();
  double evenPositionDistanceFromOrigin[2] =
    {
    (eventPosition[0] - windowSize[0] / 2) * xyToSlice->GetElement(0, 0),
    (eventPosition[1] - windowSize[1] / 2) * xyToSlice->GetElement(1, 1)
    };

  // Adjust field of view
  double fov[3] = { 1.0 };
  sliceNode->GetFieldOfView(fov);
  fov[0] *= zoomScaleFactor;
  fov[1] *= zoomScaleFactor;
  sliceNode->SetFieldOfView(fov[0], fov[1], fov[2]);

  // Keep the mouse position at the same place on screen
  double sliceOrigin[3] = { 0 };
  sliceNode->GetXYZOrigin(sliceOrigin);
  sliceNode->SetSliceOrigin(
    sliceOrigin[0] + evenPositionDistanceFromOrigin[0] * (1.0 - zoomScaleFactor),
    sliceOrigin[1] + evenPositionDistanceFromOrigin[1] * (1.0 - zoomScaleFactor),
    sliceOrigin[2]);

  sliceNode->UpdateMatrices();
  sliceNode->EndModify(wasModifying);
  this->SliceLogic->EndSliceNodeInteraction();
}

//----------------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::ProcessSetCrosshairBackground(vtkDMMLInteractionEventData* eventData)
{
  if (!this->ModifierKeyPressedSinceLastClickAndDrag)
    {
    // this event was caused by a "stuck" modifier key
    return false;
    }
  return this->ProcessSetCrosshair(eventData);
}

//----------------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::ProcessSetCrosshair(vtkDMMLInteractionEventData* eventData)
{
  vtkDMMLSliceNode* sliceNode = this->GetSliceNode();
  vtkDMMLCrosshairNode* crosshairNode = vtkDMMLCrosshairDisplayableManager::FindCrosshairNode(sliceNode->GetScene());
  if (!crosshairNode)
    {
    return false;
    }
  const double* worldPos = eventData->GetWorldPosition();
  crosshairNode->SetCrosshairRAS(const_cast<double*>(worldPos));
  if (crosshairNode->GetCrosshairBehavior() != vtkDMMLCrosshairNode::NoAction)
    {
    int viewJumpSliceMode = vtkDMMLSliceNode::OffsetJumpSlice;
    if (crosshairNode->GetCrosshairBehavior() == vtkDMMLCrosshairNode::CenteredJumpSlice)
      {
      viewJumpSliceMode = vtkDMMLSliceNode::CenteredJumpSlice;
      }
    sliceNode->JumpAllSlices(sliceNode->GetScene(),
      worldPos[0], worldPos[1], worldPos[2],
      viewJumpSliceMode, sliceNode->GetViewGroup(), sliceNode);
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::IsEventInsideVolume(bool background, vtkDMMLInteractionEventData* eventData)
{
  vtkDMMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();
  if (!sliceNode)
  {
    return false;
  }
  vtkDMMLSliceLayerLogic* layerLogic = background ?
    this->SliceLogic->GetBackgroundLayer() : this->SliceLogic->GetForegroundLayer();
  if (!layerLogic)
  {
    return false;
  }
  vtkDMMLVolumeNode* volumeNode = layerLogic->GetVolumeNode();
  if (!volumeNode || !volumeNode->GetImageData())
  {
    return false;
  }
  const int* eventPosition = eventData->GetDisplayPosition();
  double xyz[3] = { 0 };
  vtkDMMLAbstractSliceViewDisplayableManager::ConvertDeviceToXYZ(this->GetRenderer(), sliceNode, eventPosition[0], eventPosition[1], xyz);
  vtkGeneralTransform* xyToBackgroundIJK = layerLogic->GetXYToIJKTransform();
  double mousePositionIJK[3] = { 0 };
  xyToBackgroundIJK->TransformPoint(xyz, mousePositionIJK);
  int volumeExtent[6] = { 0 };
  volumeNode->GetImageData()->GetExtent(volumeExtent);
  for (int i = 0; i < 3; i++)
  {
    if (mousePositionIJK[i]<volumeExtent[i * 2] || mousePositionIJK[i]>volumeExtent[i * 2 + 1])
    {
      return false;
    }
  }
  return true;
}

//----------------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::GetActionEnabled(int actionsMask)
{
  return (this->ActionsEnabled & actionsMask) == actionsMask;
}

//----------------------------------------------------------------------------
void vtkDMMLSliceIntersectionWidget::SetActionEnabled(int actionsMask, bool enable /*=true*/)
{
  if (enable)
    {
    this->ActionsEnabled |= actionsMask;
    }
  else
    {
    this->ActionsEnabled &= (~actionsMask);
    }
  this->UpdateInteractionEventMapping();
}

//----------------------------------------------------------------------------
int vtkDMMLSliceIntersectionWidget::GetActionsEnabled()
{
  return this->ActionsEnabled;
}

//----------------------------------------------------------------------------
void vtkDMMLSliceIntersectionWidget::SetActionsEnabled(int actions)
{
  this->ActionsEnabled = actions;
  this->UpdateInteractionEventMapping();
}

//-------------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::ProcessWidgetMenu(vtkDMMLInteractionEventData* eventData)
{
  if (this->WidgetState != WidgetStateIdle)
    {
    return false;
    }
  if (!this->SliceNode)
    {
    return false;
    }
  vtkDMMLInteractionNode* interactionNode = this->SliceNode->GetInteractionNode();
  if (!interactionNode)
    {
    return false;
    }
  vtkNew<vtkDMMLInteractionEventData> pickEventData;
  pickEventData->SetType(vtkDMMLInteractionNode::ShowViewContextMenuEvent);
  pickEventData->SetViewNode(this->SliceNode);
  if (eventData->IsDisplayPositionValid())
    {
    pickEventData->SetDisplayPosition(eventData->GetDisplayPosition());
    }
  if (eventData->IsWorldPositionValid())
    {
    pickEventData->SetWorldPosition(eventData->GetWorldPosition(), eventData->IsWorldPositionAccurate());
    }
  interactionNode->ShowViewContextMenu(pickEventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::ProcessMaximizeView(vtkDMMLInteractionEventData* vtkNotUsed(eventData))
{
  if (this->WidgetState != WidgetStateIdle)
    {
    return false;
    }
  if (!this->SliceNode)
    {
    return false;
    }

  bool isMaximized = false;
  bool canBeMaximized = false;
  vtkDMMLLayoutNode* layoutNode = this->SliceNode->GetMaximizedState(isMaximized, canBeMaximized);
  if (!layoutNode || !canBeMaximized)
    {
    return false;
    }

  if (isMaximized)
    {
    layoutNode->SetMaximizedViewNode(nullptr);
    }
  else
    {
    layoutNode->SetMaximizedViewNode(this->SliceNode);
    }

  // Maximize/restore takes away the focus without resetting
  // this->ModifierKeyPressedSinceLastMouseButtonRelease
  // therefore we need to reset the flag here.
  this->ModifierKeyPressedSinceLastClickAndDrag = true;

  return true;
}

//-------------------------------------------------------------------------
int vtkDMMLSliceIntersectionWidget::GetMouseCursor()
{
  switch (this->WidgetState)
    {
    case WidgetStateOnTranslateIntersectingSlicesHandle:
      return VTK_CURSOR_SIZEALL;
    case WidgetStateOnTranslateSingleIntersectingSliceHandle:
    {
      vtkDMMLSliceIntersectionInteractionRepresentation* rep = vtkDMMLSliceIntersectionInteractionRepresentation::SafeDownCast(this->WidgetRep);
      if (!rep)
        {
        return VTK_CURSOR_DEFAULT;
        }
      return rep->GetTranslateArrowCursor(this->LastIntersectingSliceNodeID);
    }
    case WidgetStateOnRotateIntersectingSlicesHandle:
      return VTK_CURSOR_HAND;
    default:
      return VTK_CURSOR_DEFAULT;
    }
}

//----------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::ProcessTranslateIntersectingSlicesHandleStart(vtkDMMLInteractionEventData* eventData)
{
  vtkDMMLSliceIntersectionInteractionRepresentation* rep = vtkDMMLSliceIntersectionInteractionRepresentation::SafeDownCast(this->WidgetRep);
  if (!rep)
    {
    return false;
    }

  this->SetWidgetState(WidgetStateTranslateIntersectingSlicesHandle);

  // Save start translation point XY
  const int* displayPos = eventData->GetDisplayPosition();
  double displayPosDouble[2] = { static_cast<double>(displayPos[0]), static_cast<double>(displayPos[1]) };
  this->StartTranslationPoint[0] = displayPosDouble[0];
  this->StartTranslationPoint[1] = displayPosDouble[1];

  // Save start translation point RAS
  vtkMatrix4x4* xyToRAS = this->GetSliceNode()->GetXYToRAS();
  double displayPosDouble_XY[4] = { displayPosDouble[0], displayPosDouble[1], 0, 1 };
  double displayPosDouble_RAS[4] = { 0, 0, 0, 1 };
  xyToRAS->MultiplyPoint(displayPosDouble_XY, displayPosDouble_RAS);
  this->StartTranslationPoint_RAS[0] = displayPosDouble_RAS[0];
  this->StartTranslationPoint_RAS[1] = displayPosDouble_RAS[1];
  this->StartTranslationPoint_RAS[2] = displayPosDouble_RAS[2];

  return this->ProcessStartMouseDrag(eventData);
}


//----------------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::ProcessTranslateIntersectingSlicesHandle(vtkDMMLInteractionEventData* eventData)
{
  // Get current slice node and scene
  vtkDMMLSliceNode* sliceNode = this->SliceLogic->GetSliceNode();
  vtkDMMLScene* scene = sliceNode->GetScene();

  // Get event position
  const int* eventPosition = eventData->GetDisplayPosition();
  double displayPosDouble[2] = { static_cast<double>(eventPosition[0]), static_cast<double>(eventPosition[1]) };
  const double* worldPos = eventData->GetWorldPosition();

  // Get event position RAS
  vtkMatrix4x4* xyToRAS = this->GetSliceNode()->GetXYToRAS();
  double eventPos_XY[4] = { displayPosDouble[0], displayPosDouble[1], 0, 1 };
  double eventPos_RAS[4] = { 0, 0, 0, 1 };
  xyToRAS->MultiplyPoint(eventPos_XY, eventPos_RAS);
  this->CurrentTranslationPoint_RAS[0] = eventPos_RAS[0];
  this->CurrentTranslationPoint_RAS[1] = eventPos_RAS[1];
  this->CurrentTranslationPoint_RAS[2] = eventPos_RAS[2];

  // Get representation
  vtkDMMLSliceIntersectionInteractionRepresentation* rep = vtkDMMLSliceIntersectionInteractionRepresentation::SafeDownCast(this->GetRepresentation());
  if (!rep)
    {
    return false;
    }

  // Get intersecting slice node
  vtkDMMLSliceNode* intersectingSliceNode = vtkDMMLSliceNode::SafeDownCast(scene->GetNodeByID(this->LastIntersectingSliceNodeID));
  if (!intersectingSliceNode)
    {
    return false;
    }

  // Get translation offset
  double translationOffset[3] = { this->CurrentTranslationPoint_RAS[0] - this->StartTranslationPoint_RAS[0],
                                  this->CurrentTranslationPoint_RAS[1] - this->StartTranslationPoint_RAS[1],
                                  this->CurrentTranslationPoint_RAS[2] - this->StartTranslationPoint_RAS[2] };

  double translationOffsetNorm = vtkMath::Norm(translationOffset);
  if (translationOffsetNorm == 0) // to avoid translation when no mouse dragging was done
    {
    return false;
    }

  // Jump slice in intersecting slice node
  intersectingSliceNode->JumpSlice(worldPos[0], worldPos[1], worldPos[2]);

  // Store previous event position
  this->PreviousEventPosition[0] = eventPosition[0];
  this->PreviousEventPosition[1] = eventPosition[1];

  return true;
}


//----------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::ProcessTranslateSingleIntersectingSliceHandleStart(vtkDMMLInteractionEventData* eventData)
{
  vtkDMMLSliceIntersectionInteractionRepresentation* rep = vtkDMMLSliceIntersectionInteractionRepresentation::SafeDownCast(this->WidgetRep);
  if (!rep)
    {
    return false;
    }

  this->SetWidgetState(WidgetStateTranslateSingleIntersectingSliceHandle);

  // Save start translation point XY
  const int* displayPos = eventData->GetDisplayPosition();
  double displayPosDouble[2] = { static_cast<double>(displayPos[0]), static_cast<double>(displayPos[1]) };
  this->StartTranslationPoint[0] = displayPosDouble[0];
  this->StartTranslationPoint[1] = displayPosDouble[1];

  // Save start translation point RAS
  vtkMatrix4x4* xyToRAS = this->GetSliceNode()->GetXYToRAS();
  double displayPosDouble_XY[4] = { displayPosDouble[0], displayPosDouble[1], 0, 1 };
  double displayPosDouble_RAS[4] = { 0, 0, 0, 1 };
  xyToRAS->MultiplyPoint(displayPosDouble_XY, displayPosDouble_RAS);
  this->StartTranslationPoint_RAS[0] = displayPosDouble_RAS[0];
  this->StartTranslationPoint_RAS[1] = displayPosDouble_RAS[1];
  this->StartTranslationPoint_RAS[2] = displayPosDouble_RAS[2];

  return this->ProcessStartMouseDrag(eventData);
}

//----------------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::ProcessTranslateSingleIntersectingSliceHandle(vtkDMMLInteractionEventData* eventData)
{
  // Get current slice node and scene
  vtkDMMLSliceNode* sliceNode = this->SliceLogic->GetSliceNode();
  vtkDMMLScene* scene = sliceNode->GetScene();

  // Get event position
  const int* eventPosition = eventData->GetDisplayPosition();
  double displayPosDouble[2] = { static_cast<double>(eventPosition[0]), static_cast<double>(eventPosition[1]) };
  const double* worldPos = eventData->GetWorldPosition();

  // Get event position RAS
  vtkMatrix4x4* xyToRAS = this->GetSliceNode()->GetXYToRAS();
  double eventPos_XY[4] = { displayPosDouble[0], displayPosDouble[1], 0, 1 };
  double eventPos_RAS[4] = { 0, 0, 0, 1 };
  xyToRAS->MultiplyPoint(eventPos_XY, eventPos_RAS);
  this->CurrentTranslationPoint_RAS[0] = eventPos_RAS[0];
  this->CurrentTranslationPoint_RAS[1] = eventPos_RAS[1];
  this->CurrentTranslationPoint_RAS[2] = eventPos_RAS[2];

  // Get representation
  vtkDMMLSliceIntersectionInteractionRepresentation* rep = vtkDMMLSliceIntersectionInteractionRepresentation::SafeDownCast(this->GetRepresentation());
  if (!rep)
    {
    return false;
    }

  // Get intersecting slice node
  std::string intersectingSliceNodeID = this->LastIntersectingSliceNodeID;
  vtkDMMLSliceNode* intersectingSliceNode = vtkDMMLSliceNode::SafeDownCast(scene->GetNodeByID(intersectingSliceNodeID));
  if (!intersectingSliceNode)
    {
    return false;
    }

  this->SetWidgetState(WidgetStateTranslateSingleIntersectingSliceHandle);

  // Get translation offset
  double translationOffset[3] = { this->CurrentTranslationPoint_RAS[0] - this->StartTranslationPoint_RAS[0],
                                  this->CurrentTranslationPoint_RAS[1] - this->StartTranslationPoint_RAS[1],
                                  this->CurrentTranslationPoint_RAS[2] - this->StartTranslationPoint_RAS[2] };

  double translationOffsetNorm = vtkMath::Norm(translationOffset);
  if (translationOffsetNorm == 0) // to avoid translation when no mouse dragging was done
    {
    return false;
    }

  // Jump slice in intersecting slice node
  intersectingSliceNode->JumpSlice(worldPos[0], worldPos[1], worldPos[2]);

  // Store previous event position
  this->PreviousEventPosition[0] = eventPosition[0];
  this->PreviousEventPosition[1] = eventPosition[1];

  return true;
}

//----------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::ProcessRotateIntersectingSlicesHandleStart(vtkDMMLInteractionEventData* eventData)
{
  vtkDMMLSliceIntersectionInteractionRepresentation* rep = vtkDMMLSliceIntersectionInteractionRepresentation::SafeDownCast(this->WidgetRep);
  if (!rep)
    {
    return false;
    }

  this->SetWidgetState(WidgetStateRotateIntersectingSlicesHandle);

  // Save rotation center RAS
  double* sliceIntersectionPoint_XY = rep->GetSliceIntersectionPoint(); // in RAS coordinate system
  this->StartRotationCenter[0] = sliceIntersectionPoint_XY[0];
  this->StartRotationCenter[1] = sliceIntersectionPoint_XY[1];

  // Save rotation center XY
  vtkMatrix4x4* xyToRAS = this->GetSliceNode()->GetXYToRAS();
  double sliceIntersectionPoint_RAS[4] = { 0, 0, 0, 1 };
  xyToRAS->MultiplyPoint(sliceIntersectionPoint_XY, sliceIntersectionPoint_RAS);
  this->StartRotationCenter_RAS[0] = sliceIntersectionPoint_RAS[0];
  this->StartRotationCenter_RAS[1] = sliceIntersectionPoint_RAS[1];
  this->StartRotationCenter_RAS[2] = sliceIntersectionPoint_RAS[2];

  // Save initial rotation angle
  const int* displayPos = eventData->GetDisplayPosition();
  double displayPosDouble[2] = { static_cast<double>(displayPos[0]), static_cast<double>(displayPos[1]) };
  this->PreviousRotationAngleRad = this->GetSliceRotationAngleRad(displayPosDouble);

  return this->ProcessStartMouseDrag(eventData);
}

/*
//----------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::ProcessRotate(double sliceRotationAngleRad)
{
  vtkDMMLSliceIntersectionRepresentation2D* rep = vtkDMMLSliceIntersectionRepresentation2D::SafeDownCast(this->WidgetRep);
  if (!rep)
    {
    return false;
    }
  vtkMatrix4x4* sliceToRAS = this->GetSliceNode()->GetSliceToRAS();
  vtkNew<vtkTransform> rotatedSliceToSliceTransform;

  // Save rotation center RAS
  double* sliceIntersectionPoint_XY = rep->GetSliceIntersectionPoint(); // in RAS coordinate system
  this->StartRotationCenter[0] = sliceIntersectionPoint_XY[0];
  this->StartRotationCenter[1] = sliceIntersectionPoint_XY[1];

  // Save rotation center XY
  vtkMatrix4x4* xyToRAS = this->GetSliceNode()->GetXYToRAS();
  double sliceIntersectionPoint_RAS[4] = { 0, 0, 0, 1 };
  xyToRAS->MultiplyPoint(sliceIntersectionPoint_XY, sliceIntersectionPoint_RAS);
  this->StartRotationCenter_RAS[0] = sliceIntersectionPoint_RAS[0];
  this->StartRotationCenter_RAS[1] = sliceIntersectionPoint_RAS[1];
  this->StartRotationCenter_RAS[2] = sliceIntersectionPoint_RAS[2];

  rotatedSliceToSliceTransform->Translate(this->StartRotationCenter_RAS[0], this->StartRotationCenter_RAS[1], this->StartRotationCenter_RAS[2]);
  double rotationDirection = vtkMath::Determinant3x3(sliceToRAS->Element[0], sliceToRAS->Element[1], sliceToRAS->Element[2]) >= 0 ? 1.0 : -1.0;
  rotatedSliceToSliceTransform->RotateWXYZ(rotationDirection * vtkMath::DegreesFromRadians(sliceRotationAngleRad),
    sliceToRAS->GetElement(0, 2), sliceToRAS->GetElement(1, 2), sliceToRAS->GetElement(2, 2));
  rotatedSliceToSliceTransform->Translate(-this->StartRotationCenter_RAS[0], -this->StartRotationCenter_RAS[1], -this->StartRotationCenter_RAS[2]);

  this->PreviousRotationAngleRad = sliceRotationAngleRad;

  rep->TransformIntersectingSlices(rotatedSliceToSliceTransform->GetMatrix());
  return true;
}
*/

//----------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::ProcessRotateIntersectingSlicesHandle(vtkDMMLInteractionEventData* eventData)
{
  vtkDMMLSliceIntersectionInteractionRepresentation* rep = vtkDMMLSliceIntersectionInteractionRepresentation::SafeDownCast(this->WidgetRep);
  if (!rep)
    {
    return false;
    }

  const int* displayPos = eventData->GetDisplayPosition();
  double displayPosDouble[2] = { static_cast<double>(displayPos[0]), static_cast<double>(displayPos[1]) };
  double sliceRotationAngleRad = this->GetSliceRotationAngleRad(displayPosDouble);

  vtkMatrix4x4* sliceToRAS = this->GetSliceNode()->GetSliceToRAS();
  vtkNew<vtkTransform> rotatedSliceToSliceTransform;

  rotatedSliceToSliceTransform->Translate(this->StartRotationCenter_RAS[0], this->StartRotationCenter_RAS[1], this->StartRotationCenter_RAS[2]);
  double rotationDirection = vtkMath::Determinant3x3(sliceToRAS->Element[0], sliceToRAS->Element[1], sliceToRAS->Element[2]) >= 0 ? 1.0 : -1.0;
  rotatedSliceToSliceTransform->RotateWXYZ(rotationDirection * vtkMath::DegreesFromRadians(sliceRotationAngleRad - this->PreviousRotationAngleRad),
    sliceToRAS->GetElement(0, 2), sliceToRAS->GetElement(1, 2), sliceToRAS->GetElement(2, 2));
  rotatedSliceToSliceTransform->Translate(-this->StartRotationCenter_RAS[0], -this->StartRotationCenter_RAS[1], -this->StartRotationCenter_RAS[2]);

  this->PreviousRotationAngleRad = sliceRotationAngleRad;
  this->PreviousEventPosition[0] = displayPos[0];
  this->PreviousEventPosition[1] = displayPos[1];

  rep->TransformIntersectingSlices(rotatedSliceToSliceTransform->GetMatrix());

  return true;
}

//----------------------------------------------------------------------------
bool vtkDMMLSliceIntersectionWidget::IsSliceIntersectionInteractive()
{
  vtkDMMLApplicationLogic* dmmlAppLogic = this->GetDMMLApplicationLogic();
  if (!dmmlAppLogic)
    {
    return false;
    }
  return dmmlAppLogic->GetIntersectingSlicesEnabled(vtkDMMLApplicationLogic::IntersectingSlicesInteractive);
}
