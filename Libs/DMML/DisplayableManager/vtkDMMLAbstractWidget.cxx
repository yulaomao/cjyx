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

#include "vtkDMMLAbstractWidget.h"

#include "vtkDMMLApplicationLogic.h"
#include "vtkDMMLInteractionEventData.h"
#include "vtkDMMLInteractionNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSliceCompositeNode.h"
#include "vtkDMMLSliceNode.h"
#include "vtkDMMLSliceLogic.h"
#include "vtkDMMLAbstractWidgetRepresentation.h"

// VTK includes
#include "vtkEvent.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkWidgetEventTranslator.h"

//----------------------------------------------------------------------
vtkDMMLAbstractWidget::vtkDMMLAbstractWidget()
{
  this->WidgetRep = nullptr;
  this->WidgetState = vtkDMMLAbstractWidget::WidgetStateIdle;

//  this->NeedToRender = false;
}

//----------------------------------------------------------------------
vtkDMMLAbstractWidget::~vtkDMMLAbstractWidget()
{
  this->SetRenderer(nullptr);
}

//----------------------------------------------------------------------
void vtkDMMLAbstractWidget::SetEventTranslation(int widgetState, unsigned long interactionEvent, int modifiers, unsigned long widgetEvent)
{
  if (widgetState >= static_cast<int>(this->EventTranslators.size()))
    {
    this->EventTranslators.resize(widgetState + 1);
    }
  vtkWidgetEventTranslator* translator = this->EventTranslators[widgetState];
  if (!translator)
    {
    this->EventTranslators[widgetState] = vtkSmartPointer<vtkWidgetEventTranslator>::New();
    translator = this->EventTranslators[widgetState];
    }
  vtkNew<vtkDMMLInteractionEventData> ed;
  ed->SetType(interactionEvent);
  ed->SetModifiers(modifiers);
  translator->SetTranslation(interactionEvent, ed, widgetEvent);
}

//----------------------------------------------------------------------
void vtkDMMLAbstractWidget::SetEventTranslation(unsigned long interactionEvent, int modifiers, unsigned long widgetEvent)
{
  this->SetEventTranslation(WidgetStateAny, interactionEvent, modifiers, widgetEvent);
}

//----------------------------------------------------------------------
void vtkDMMLAbstractWidget::SetEventTranslationClickAndDrag(int widgetState, unsigned long startInteractionEvent,
  int modifiers, int widgetStateDragging,
  unsigned long widgetStartEvent, unsigned long widgetEndEvent)
{
  unsigned long endInteractionEvent = WidgetEventNone;
  switch (startInteractionEvent)
    {
    case vtkCommand::LeftButtonPressEvent: endInteractionEvent = vtkCommand::LeftButtonReleaseEvent; break;
    case vtkCommand::MiddleButtonPressEvent: endInteractionEvent = vtkCommand::MiddleButtonReleaseEvent; break;
    case vtkCommand::RightButtonPressEvent: endInteractionEvent = vtkCommand::RightButtonReleaseEvent; break;
    }
  this->SetEventTranslation(widgetState, startInteractionEvent, modifiers, widgetStartEvent);
  this->SetEventTranslation(widgetStateDragging, vtkCommand::MouseMoveEvent, vtkEvent::AnyModifier, WidgetEventMouseMove);
  this->SetEventTranslation(widgetStateDragging, endInteractionEvent, vtkEvent::AnyModifier, widgetEndEvent);
}

//----------------------------------------------------------------------
void vtkDMMLAbstractWidget::SetKeyboardEventTranslation(
  int widgetState, int modifier, char keyCode,
  int repeatCount, const char* keySym, unsigned long widgetEvent)
{
  if (widgetState >= static_cast<int>(this->EventTranslators.size()))
    {
    this->EventTranslators.resize(widgetState +1);
    }
  vtkWidgetEventTranslator* translator = this->EventTranslators[widgetState];
  if (!translator)
    {
    this->EventTranslators[widgetState] = vtkSmartPointer<vtkWidgetEventTranslator>::New();
    translator = this->EventTranslators[widgetState];
    }
  translator->SetTranslation(vtkCommand::KeyPressEvent, modifier, keyCode,
    repeatCount, keySym, widgetEvent);
}

//----------------------------------------------------------------------
void vtkDMMLAbstractWidget::SetKeyboardEventTranslation(
  int modifier, char keyCode,
  int repeatCount, const char* keySym, unsigned long widgetEvent)
{
  this->SetKeyboardEventTranslation(WidgetStateAny, modifier, keyCode,
    repeatCount, keySym, widgetEvent);
}

//-------------------------------------------------------------------------
vtkWidgetEventTranslator* vtkDMMLAbstractWidget::GetEventTranslator(int widgetState)
{
  if (widgetState < 0 || widgetState >= static_cast<int>(this->EventTranslators.size()))
    {
    return nullptr;
    }
  return this->EventTranslators[widgetState];
}

//-------------------------------------------------------------------------
int vtkDMMLAbstractWidget::GetNumberOfEventTranslators()
{
  return this->EventTranslators.size();
}

//-------------------------------------------------------------------------
void vtkDMMLAbstractWidget::SetRepresentation(vtkDMMLAbstractWidgetRepresentation *rep)
{
  if (rep == this->WidgetRep)
    {
    // no change
    return;
    }

  if (this->WidgetRep)
    {
    if (this->Renderer)
      {
      this->WidgetRep->SetRenderer(nullptr);
      this->Renderer->RemoveViewProp(this->WidgetRep);
      }
    }

  this->WidgetRep = rep;

  if (this->Renderer != nullptr && this->WidgetRep != nullptr)
    {
    this->WidgetRep->SetRenderer(this->Renderer);
    this->Renderer->AddViewProp(this->WidgetRep);
    }
}

//-------------------------------------------------------------------------
vtkDMMLAbstractWidgetRepresentation* vtkDMMLAbstractWidget::GetRepresentation()
{
  return this->WidgetRep;
}

//-------------------------------------------------------------------------
void vtkDMMLAbstractWidget::UpdateFromDMML(vtkDMMLNode* caller, unsigned long event, void *callData/*=nullptr*/)
{
  if (!this->WidgetRep)
    {
    return;
    }

  this->WidgetRep->UpdateFromDMML(caller, event, callData);
}

//----------------------------------------------------------------------
void vtkDMMLAbstractWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "WidgetState: " << this->WidgetState << endl;
}

//-----------------------------------------------------------------------------
unsigned long vtkDMMLAbstractWidget::TranslateInteractionEventToWidgetEvent(
  vtkDMMLInteractionEventData* eventData)
{
  if (!eventData)
    {
    return WidgetEventNone;
    }

  // Try to process with a state-specific translator
  if (this->WidgetState < static_cast<int>(this->EventTranslators.size()))
    {
    vtkWidgetEventTranslator* translator = this->EventTranslators[this->WidgetState];
    if (translator)
      {
      unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(translator, eventData);
      if (widgetEvent != WidgetEventNone)
        {
        return widgetEvent;
        }
      }
    }

  // Try to process with the state-independent translator
  unsigned long widgetEvent = WidgetEventNone;
  if (WidgetStateAny < this->EventTranslators.size())
    {
    vtkWidgetEventTranslator* translator = this->EventTranslators[WidgetStateAny];
    if (translator)
      {
      // There is an event translator for this state
      widgetEvent = this->TranslateInteractionEventToWidgetEvent(translator, eventData);
      }
    }

  return widgetEvent;
}

//-----------------------------------------------------------------------------
unsigned long vtkDMMLAbstractWidget::TranslateInteractionEventToWidgetEvent(
  vtkWidgetEventTranslator* translator, vtkDMMLInteractionEventData* eventData)
{
  unsigned long widgetEvent = WidgetEventNone;

  if (!eventData)
    {
    return widgetEvent;
    }

  if (eventData->GetType() == vtkCommand::KeyPressEvent)
    {
    // We package keypress events information into event data,
    // unpack it for the event translator
    int modifier = eventData->GetModifiers();
    char keyCode = eventData->GetKeyCode();
    int repeatCount = eventData->GetKeyRepeatCount();
    const char* keySym = nullptr;
    if (!eventData->GetKeySym().empty())
      {
      keySym = eventData->GetKeySym().c_str();
      }

    // If neither the ctrl nor the shift keys are pressed, give
    // NoModifier a preference over AnyModifer.
    if (modifier == vtkEvent::AnyModifier)
      {
      widgetEvent = translator->GetTranslation(vtkCommand::KeyPressEvent,
        vtkEvent::NoModifier, keyCode, repeatCount, keySym);
      }
    if (widgetEvent == WidgetEventNone)
      {
      widgetEvent = translator->GetTranslation(vtkCommand::KeyPressEvent,
        modifier, keyCode, repeatCount, keySym);
      }
    }
  else
    {
    widgetEvent = translator->GetTranslation(eventData->GetType(), eventData);
    }

  return widgetEvent;
}

//-----------------------------------------------------------------------------
bool vtkDMMLAbstractWidget::CanProcessInteractionEvent(vtkDMMLInteractionEventData* vtkNotUsed(eventData), double &vtkNotUsed(distance2))
{
  return false;
}

//-----------------------------------------------------------------------------
bool vtkDMMLAbstractWidget::ProcessInteractionEvent(vtkDMMLInteractionEventData* vtkNotUsed(eventData))
{
  return false;
}

//-----------------------------------------------------------------------------
void vtkDMMLAbstractWidget::Leave(vtkDMMLInteractionEventData* vtkNotUsed(eventData))
{
  this->SetWidgetState(WidgetStateIdle);
}

//-------------------------------------------------------------------------
int vtkDMMLAbstractWidget::GetMouseCursor()
{
  if (this->WidgetState == WidgetStateIdle)
    {
    return VTK_CURSOR_DEFAULT;
    }
  else
    {
    return VTK_CURSOR_HAND;
    }
}

//-------------------------------------------------------------------------
bool vtkDMMLAbstractWidget::GetGrabFocus()
{
  // we need to grab focus when interactively dragging points
  return this->GetInteractive();
}

//-------------------------------------------------------------------------
bool vtkDMMLAbstractWidget::GetInteractive()
{
  switch (this->WidgetState)
    {
    case WidgetStateTranslate:
    case WidgetStateScale:
    case WidgetStateRotate:
      return true;
    default:
      return false;
    }
}

//-------------------------------------------------------------------------
bool vtkDMMLAbstractWidget::GetNeedToRender()
{
  if (!this->WidgetRep)
    {
    return false;
    }
  return this->WidgetRep->GetNeedToRender();
/*
  if (this->NeedToRender)
    {
    return true;
    }
  if (this->WidgetRep && this->WidgetRep->GetNeedToRender())
    {
    return true;
    }
  return false;*/
}

//-------------------------------------------------------------------------
void vtkDMMLAbstractWidget::NeedToRenderOff()
{
  if (!this->WidgetRep)
    {
    return;
    }
  this->WidgetRep->NeedToRenderOff();
/*
  this->NeedToRender = false;
  if (this->WidgetRep)
    {
    this->WidgetRep->NeedToRenderOff();
    }
*/
}

//----------------------------------------------------------------------
vtkRenderer* vtkDMMLAbstractWidget::GetRenderer()
{
  return this->Renderer;
}

//----------------------------------------------------------------------
void vtkDMMLAbstractWidget::SetRenderer(vtkRenderer* renderer)
{
  if (renderer == this->Renderer)
    {
    return;
    }

  if (this->Renderer != nullptr && this->WidgetRep != nullptr)
    {
    this->Renderer->RemoveViewProp(this->WidgetRep);
    }

  this->Renderer = renderer;

  if (this->WidgetRep != nullptr && this->Renderer != nullptr)
    {
    this->WidgetRep->SetRenderer(this->Renderer);
    this->Renderer->AddViewProp(this->WidgetRep);
    }
}

//---------------------------------------------------------------------------
const char* vtkDMMLAbstractWidget::GetAssociatedNodeID(vtkDMMLInteractionEventData* vtkNotUsed(eventData))
{
  if (!this->WidgetRep)
    {
    return nullptr;
    }
  // is there a volume in the background?
  vtkDMMLSliceNode* sliceNode = vtkDMMLSliceNode::SafeDownCast(this->WidgetRep->GetViewNode());
  if (!sliceNode)
    {
    // this only works for slice views for now
    return nullptr;
    }
  // find the slice composite node in the scene with the matching layout name
  vtkDMMLApplicationLogic *dmmlAppLogic = this->GetDMMLApplicationLogic();
  if (!dmmlAppLogic)
    {
    return nullptr;
    }
  vtkDMMLSliceLogic *sliceLogic = dmmlAppLogic->GetSliceLogic(sliceNode);
  if (!sliceLogic)
    {
    return nullptr;
    }
  vtkDMMLSliceCompositeNode* sliceCompositeNode = sliceLogic->GetSliceCompositeNode(sliceNode);
  if (!sliceCompositeNode)
    {
    return nullptr;
    }
  if (sliceCompositeNode->GetBackgroundVolumeID())
    {
    return sliceCompositeNode->GetBackgroundVolumeID();
    }
  else if (sliceCompositeNode->GetForegroundVolumeID())
    {
    return sliceCompositeNode->GetForegroundVolumeID();
    }
  else if (sliceCompositeNode->GetLabelVolumeID())
    {
    return sliceCompositeNode->GetLabelVolumeID();
    }
  return nullptr;
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractWidget::SetDMMLApplicationLogic(vtkDMMLApplicationLogic* applicationLogic)
{
  this->ApplicationLogic = applicationLogic;
}

//---------------------------------------------------------------------------
vtkDMMLApplicationLogic* vtkDMMLAbstractWidget::GetDMMLApplicationLogic()
{
  return this->ApplicationLogic;
}

//---------------------------------------------------------------------------
vtkDMMLInteractionNode* vtkDMMLAbstractWidget::GetInteractionNode()
{
  if (!this->WidgetRep)
    {
    return nullptr;
    }
  vtkDMMLAbstractViewNode* viewNode = this->WidgetRep->GetViewNode();
  if (!viewNode)
    {
    return nullptr;
    }
  return viewNode->GetInteractionNode();
}

//---------------------------------------------------------------------------
bool vtkDMMLAbstractWidget::CanProcessButtonClickEvent(vtkDMMLInteractionEventData* eventData, double& distance2)
{
  if (eventData->GetMouseMovedSinceButtonDown())
    {
    return false;
    }

  int clickEvent = 0;
  switch (eventData->GetType())
    {
    case vtkCommand::LeftButtonReleaseEvent:
      clickEvent = vtkDMMLInteractionEventData::LeftButtonClickEvent;
      break;
    case vtkCommand::MiddleButtonReleaseEvent:
      clickEvent = vtkDMMLInteractionEventData::MiddleButtonClickEvent;
      break;
    case vtkCommand::RightButtonReleaseEvent:
      clickEvent = vtkDMMLInteractionEventData::RightButtonClickEvent;
      break;
    default:
      return false;
    }

  // Temporarily change the event ID to click, and process the event
  int originalEventType = eventData->GetType();
  eventData->SetType(clickEvent);
  bool canProcessEvent = this->CanProcessInteractionEvent(eventData, distance2);
  eventData->SetType(originalEventType);
  return canProcessEvent;
}

//---------------------------------------------------------------------------
int vtkDMMLAbstractWidget::ProcessButtonClickEvent(vtkDMMLInteractionEventData* eventData)
{
  if (eventData->GetMouseMovedSinceButtonDown())
    {
    return false;
    }

  int clickEvent = 0;
  switch (eventData->GetType())
    {
    case vtkCommand::LeftButtonReleaseEvent:
      clickEvent = vtkDMMLInteractionEventData::LeftButtonClickEvent;
      break;
    case vtkCommand::MiddleButtonReleaseEvent:
      clickEvent = vtkDMMLInteractionEventData::MiddleButtonClickEvent;
      break;
    case vtkCommand::RightButtonReleaseEvent:
      clickEvent = vtkDMMLInteractionEventData::RightButtonClickEvent;
      break;
    default:
      return false;
    }

  // Temporarily change the event ID to click, and process the event
  int originalEventType = eventData->GetType();
  eventData->SetType(clickEvent);
  bool processedEvent = this->ProcessInteractionEvent(eventData);
  eventData->SetType(originalEventType);
  return processedEvent;
}
