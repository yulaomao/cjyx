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

#include "vtkDMMLViewInteractorStyle.h"

// DMML includes
#include "vtkDMMLAbstractSliceViewDisplayableManager.h"
#include "vtkDMMLApplicationLogic.h"
#include "vtkDMMLDisplayableManagerGroup.h"
#include "vtkDMMLInteractionEventData.h"

// VTK includes
#include "vtkCallbackCommand.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkVersionMacros.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLViewInteractorStyle);

//----------------------------------------------------------------------------
vtkDMMLViewInteractorStyle::vtkDMMLViewInteractorStyle()
{
  this->EventCallbackCommand->SetCallback(vtkDMMLViewInteractorStyle::CustomProcessEvents);

  this->FocusedDisplayableManager = nullptr;
  this->MouseMovedSinceButtonDown = false;
  this->NumberOfClicks = 0;
  this->DoubleClickIntervalTimeSec = 0.5;
}

//----------------------------------------------------------------------------
vtkDMMLViewInteractorStyle::~vtkDMMLViewInteractorStyle() = default;

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::OnKeyPress()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::KeyPressEvent))
    {
    return;
    }
  this->Superclass::OnKeyPress();
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::OnKeyRelease()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::KeyReleaseEvent))
    {
    return;
    }
  this->Superclass::OnKeyRelease();
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::OnChar()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::CharEvent))
    {
    return;
    }
  // Do not call this->Superclass::OnChar(), because char OnChar events perform various
  // low-level operations on the actors (change their rendering style to wireframe, pick them,
  // change rendering mode to stereo, etc.), which would interfere with displayable managers.
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::OnMouseMove()
{
  this->MouseMovedSinceButtonDown = true;
  if (!this->DelegateInteractionEventToDisplayableManagers(vtkCommand::MouseMoveEvent))
    {
    return;
    }
  this->Superclass::OnMouseMove();
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::OnRightButtonDown()
{
  this->MouseMovedSinceButtonDown = false;
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::RightButtonPressEvent))
    {
    return;
    }
  this->InvokeEvent(vtkCommand::RightButtonPressEvent, nullptr);
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::OnRightButtonUp()
{
  if (!this->DelegateInteractionEventToDisplayableManagers(vtkCommand::RightButtonReleaseEvent))
    {
    this->InvokeEvent(vtkCommand::RightButtonReleaseEvent, nullptr);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::OnMiddleButtonDown()
{
  this->MouseMovedSinceButtonDown = false;
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::MiddleButtonPressEvent))
    {
    return;
    }
  this->InvokeEvent(vtkCommand::MiddleButtonPressEvent, nullptr);
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::OnMiddleButtonUp()
{
  if (!this->DelegateInteractionEventToDisplayableManagers(vtkCommand::MiddleButtonReleaseEvent))
    {
    this->InvokeEvent(vtkCommand::MiddleButtonReleaseEvent, nullptr);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::OnLeftButtonDown()
{
  this->MouseMovedSinceButtonDown = false;
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::LeftButtonPressEvent))
    {
    return;
    }
  this->InvokeEvent(vtkCommand::LeftButtonPressEvent, nullptr);
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::OnLeftButtonUp()
{
  if (!this->DelegateInteractionEventToDisplayableManagers(vtkCommand::LeftButtonReleaseEvent))
    {
    this->InvokeEvent(vtkCommand::LeftButtonReleaseEvent, nullptr);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::OnEnter()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::EnterEvent))
    {
    return;
    }
  this->Superclass::OnEnter();
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::OnLeave()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::LeaveEvent))
    {
    return;
    }
  this->Superclass::OnLeave();
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::OnMouseWheelForward()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::MouseWheelForwardEvent))
    {
    return;
    }
  this->Superclass::OnMouseWheelForward();
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::OnMouseWheelBackward()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::MouseWheelBackwardEvent))
    {
    return;
    }
  this->Superclass::OnMouseWheelBackward();
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::OnButton3D(vtkEventData* eventData)
{
  if (this->DelegateInteractionEventToDisplayableManagers(eventData))
    {
    return;
    }
  this->InvokeEvent(eventData->GetType(), eventData);
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::OnMove3D(vtkEventData* eventData)
{
  if (this->DelegateInteractionEventToDisplayableManagers(eventData))
    {
    return;
    }
  this->InvokeEvent(eventData->GetType(), eventData);
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::OnExpose()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::ExposeEvent))
    {
    return;
    }
  this->Superclass::OnExpose();
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::OnConfigure()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::ConfigureEvent))
    {
    return;
    }
  this->Superclass::OnConfigure();
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::OnPinch()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::PinchEvent))
    {
    return;
    }
  this->Superclass::OnPinch();
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::OnRotate()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::RotateEvent))
    {
    return;
    }
  this->Superclass::OnRotate();
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::OnPan()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::PanEvent))
    {
    return;
    }
  this->Superclass::OnPan();
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::OnTap()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::TapEvent))
    {
    return;
    }
  this->Superclass::OnTap();
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::OnLongTap()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::LongTapEvent))
    {
    return;
    }
  this->Superclass::OnLongTap();
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::SetDisplayableManagers(vtkDMMLDisplayableManagerGroup* displayableManagerGroup)
{
  this->DisplayableManagers = displayableManagerGroup;
}

//----------------------------------------------------------------------------
bool vtkDMMLViewInteractorStyle::DelegateInteractionEventToDisplayableManagers(unsigned long event)
{
  vtkNew<vtkDMMLInteractionEventData> ed;
  ed->SetType(event);

  return this->DelegateInteractionEventToDisplayableManagers(ed);
}

//----------------------------------------------------------------------------
bool vtkDMMLViewInteractorStyle::DelegateInteractionEventToDisplayableManagers(vtkEventData* inputEventData)
{
  int* displayPositionInt = this->GetInteractor()->GetEventPosition();
  vtkRenderer* pokedRenderer = this->GetInteractor()->FindPokedRenderer(displayPositionInt[0], displayPositionInt[1]);

  vtkNew<vtkDMMLInteractionEventData> ed;
  ed->SetType(inputEventData ? inputEventData->GetType() : vtkCommand::NoEvent);
  int displayPositionCorrected[2] = { displayPositionInt[0] - pokedRenderer->GetOrigin()[0], displayPositionInt[1] - pokedRenderer->GetOrigin()[1] };
  ed->SetDisplayPosition(displayPositionCorrected);
  ed->SetMouseMovedSinceButtonDown(this->MouseMovedSinceButtonDown);
  ed->SetAttributesFromInteractor(this->GetInteractor());
  vtkEventDataDevice3D* inputEventDataDevice3D = inputEventData->GetAsEventDataDevice3D();
  if (inputEventDataDevice3D)
    {
    ed->SetDevice(inputEventDataDevice3D->GetDevice());
    ed->SetWorldPosition(inputEventDataDevice3D->GetWorldPosition());
    ed->SetWorldOrientation(inputEventDataDevice3D->GetWorldOrientation());
    ed->SetWorldDirection(inputEventDataDevice3D->GetWorldDirection());
    ed->SetInput(inputEventDataDevice3D->GetInput());
    ed->SetAction(inputEventDataDevice3D->GetAction());
    }

  return this->DelegateInteractionEventDataToDisplayableManagers(ed);
}

//----------------------------------------------------------------------------
bool vtkDMMLViewInteractorStyle::DelegateInteractionEventDataToDisplayableManagers(vtkDMMLInteractionEventData* eventData)
{
  if (!this->DisplayableManagers)
    {
    //this->SetMouseCursor(VTK_CURSOR_DEFAULT);
    return false;
    }
  if (eventData->GetType() == vtkCommand::Button3DEvent || eventData->GetType() == vtkCommand::Move3DEvent)
    {
    // Invalidate display position if 3D event
    eventData->SetDisplayPositionInvalid();
    }

  bool canProcessEvent = false;
  double closestDistance2 = VTK_DOUBLE_MAX;
  vtkDMMLAbstractDisplayableManager* closestDisplayableManager = nullptr;
  int numberOfDisplayableManagers = this->DisplayableManagers->GetDisplayableManagerCount();

  // Find the most suitable displayable manager
  for (int displayableManagerIndex = 0; displayableManagerIndex < numberOfDisplayableManagers; ++displayableManagerIndex)
    {
    vtkDMMLAbstractDisplayableManager* displayableManager = vtkDMMLAbstractDisplayableManager::SafeDownCast(
      this->DisplayableManagers->GetNthDisplayableManager(displayableManagerIndex));
    if (!displayableManager)
      {
      continue;
      }
    double distance2 = VTK_DOUBLE_MAX;
    if (displayableManager->CanProcessInteractionEvent(eventData, distance2))
      {
      if (!canProcessEvent || (distance2 < closestDistance2))
        {
        canProcessEvent = true;
        closestDisplayableManager = displayableManager;
        closestDistance2 = distance2;
        }
      }
    }

  if (!canProcessEvent)
    {
    // None of the displayable managers can process the event, just ignore it.
    // If click events (non-keyboard events) cannot be processed here then
    // indicate this by setting the mouse cursor to default.
    if (eventData->GetType() != vtkCommand::KeyPressEvent && eventData->GetType() != vtkCommand::KeyReleaseEvent)
      {
      this->DisplayableManagers->GetNthDisplayableManager(0)->SetMouseCursor(VTK_CURSOR_DEFAULT);
      }
    return false;
    }

  // Notify displayable managers about focus change
  vtkDMMLAbstractDisplayableManager* oldFocusedDisplayableManager = this->FocusedDisplayableManager;
  if (oldFocusedDisplayableManager != closestDisplayableManager)
    {
    if (oldFocusedDisplayableManager != nullptr)
      {
      oldFocusedDisplayableManager->SetHasFocus(false, eventData);
      }
    this->FocusedDisplayableManager = closestDisplayableManager;
    if (closestDisplayableManager != nullptr)
      {
      closestDisplayableManager->SetHasFocus(true, eventData);
      }
    }

  // Process event with new displayable manager
  if (!this->FocusedDisplayableManager)
    {
    if (oldFocusedDisplayableManager)
      {
      oldFocusedDisplayableManager->SetMouseCursor(VTK_CURSOR_DEFAULT);
      }
    return false;
    }

  // This prevents desynchronized update of displayable managers during user interaction
  // (ie. slice intersection widget or segmentations lagging behind during slice translation)
  this->FocusedDisplayableManager->GetDMMLApplicationLogic()->PauseRender();
  bool processed = this->FocusedDisplayableManager->ProcessInteractionEvent(eventData);
  int cursor = VTK_CURSOR_DEFAULT;
  if (processed)
    {
    cursor = this->FocusedDisplayableManager->GetMouseCursor();
    }
  this->FocusedDisplayableManager->SetMouseCursor(cursor);
  this->FocusedDisplayableManager->GetDMMLApplicationLogic()->ResumeRender();
  return processed;
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::SetMouseCursor(int cursor)
{
  if (this->GetCurrentRenderer() && this->GetCurrentRenderer()->GetRenderWindow())
    {
    this->GetCurrentRenderer()->GetRenderWindow()->SetCurrentCursor(cursor);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::CustomProcessEvents(vtkObject* object,
  unsigned long event, void* clientdata, void* calldata)
{
  vtkDMMLViewInteractorStyle* self
    = reinterpret_cast<vtkDMMLViewInteractorStyle *>(clientdata);

  // Save info for button click detection
  if (event == vtkCommand::LeftButtonPressEvent
    || event == vtkCommand::RightButtonPressEvent
    || event == vtkCommand::MiddleButtonPressEvent)
    {
    self->MouseMovedSinceButtonDown = false;
    }
  if (event == vtkCommand::MouseMoveEvent)
    {
    self->MouseMovedSinceButtonDown = true;
    }

  // Displayable managers add interactor style observers and those observers
  // replace callback method calls. We make sure here that displayable managers
  // get the chance to process the events first (except when we are in an
  // interaction state - such as zooming, panning, etc).
  if (self->State != VTKIS_NONE || !self->DelegateInteractionEventToDisplayableManagers(event))
    {
    // Displayable managers did not processed it
    Superclass::ProcessEvents(object, event, clientdata, calldata);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLViewInteractorStyle::SetInteractor(vtkRenderWindowInteractor *interactor)
{
  this->Superclass::SetInteractor(interactor);
}
