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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// DMMLDisplayableManager includes
#include "vtkDMMLAbstractDisplayableManager.h"
#include "vtkDMMLDisplayableManagerGroup.h"
#include <vtkDMMLLightBoxRendererManagerProxy.h>

// DMMLLogic includes
#include <vtkDMMLApplicationLogic.h>

// DMML includes
#include <vtkDMMLAbstractViewNode.h>
#include <vtkDMMLInteractionNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSelectionNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkInteractorStyle.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

// STD includes
#include <cassert>
#include <algorithm>
#include <functional>

#if (_MSC_VER >= 1700 && _MSC_VER < 1800)
// Visual Studio 2012 moves bind1st to <functional>
#include <functional>
#endif

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLAbstractDisplayableManager);

//----------------------------------------------------------------------------
class vtkDMMLAbstractDisplayableManager::vtkInternal
{
public:
  vtkInternal(vtkDMMLAbstractDisplayableManager* external);
  ~vtkInternal();

  /// Called after vtkCommand::DeleteEvent is called on the DisplayableManager
  static void DoDeleteCallback(vtkObject* vtk_obj, unsigned long event,
                               void* client_data, void* call_data);

  /// Set and observe \a newInteractorStyle
  void SetAndObserveInteractor(vtkRenderWindowInteractor* newInteractor);

  /// Set and observe \a newInteractorStyle
  void SetAndObserveInteractorStyle(vtkInteractorObserver* newInteractorStyle);

  /// Called after one of the observable event is invoked
  static void DoInteractorCallback(vtkObject* vtk_obj, unsigned long event,
                                   void* client_data, void* call_data);

  /// Called after one of the observable event is invoked
  static void DoInteractorStyleCallback(vtkObject* vtk_obj, unsigned long event,
                                        void* client_data, void* call_data);

  /// Set and observe \a newDMMLInteractionNode
  void SetAndObserveDMMLInteractionNode(vtkDMMLInteractionNode* newDMMLInteractionNode);

  /// Called after vtkDMMLInteractionNode::InteractionModeChangedEvent is called on the
  /// current InteractionNode
  /// Allow to add/remove Interactor style observer in case the InteractionNode MouseMode
  /// is updated. InteractorStyleObserver are enabled if MouseMode
  /// is one of the ones matching ActiveInteractionModes
  /// \note Since we want to keep the virtual method ProcessDMMLEvent of the base class pure,
  /// the pattern DMMLObserverManager/ProcessDMMLEvent is not used here.
  static void DoDMMLInteractionNodeCallback(vtkObject* vtk_obj, unsigned long event,
                                            void* client_data, void* call_data);

  /// Called after DMML DisplayableNode is set, it will add/remove interactor style observer
  /// according to the state of the current DMML InteractionNode
  /// \sa DoDMMLInteractionNodeCallback
  void UpdateInteractorStyle(int eventIdToObserve = vtkCommand::NoEvent,
                             int eventIdToUnObserve = vtkCommand::NoEvent,
                             float priority=0.0);

  /// Called after DMML DisplayableNode is set, it will add/remove
  /// interactor observer
  /// according to the state of the current DMML InteractionNode
  /// \sa DoDMMLInteractionNodeCallback
  void UpdateInteractor(int eventIdToObserve = vtkCommand::NoEvent,
                        int eventIdToUnObserve = vtkCommand::NoEvent,
                        float priority=0.0);

  vtkDMMLAbstractDisplayableManager*        External;
  bool                                      Created;
  vtkObserverManager*                       WidgetsObserverManager;
  bool                                      UpdateFromDMMLRequested;
  vtkRenderer*                              Renderer;
  vtkDMMLNode*                              DMMLDisplayableNode;
  vtkSmartPointer<vtkIntArray>              DMMLDisplayableNodeObservableEvents;
  vtkDMMLInteractionNode*                   DMMLInteractionNode;
  vtkSmartPointer<vtkCallbackCommand>       DMMLInteractionNodeCallBackCommand;
  vtkDMMLDisplayableManagerGroup*           DisplayableManagerGroup;
  vtkSmartPointer<vtkCallbackCommand>       DeleteCallBackCommand;
  vtkRenderWindowInteractor*                Interactor;
  vtkSmartPointer<vtkCallbackCommand>       InteractorCallBackCommand;
  std::vector<std::pair<int,float> >        InteractorObservableEvents;
  vtkInteractorObserver*                    InteractorStyle;
  vtkSmartPointer<vtkCallbackCommand>       InteractorStyleCallBackCommand;
  std::vector<std::pair<int,float> >        InteractorStyleObservableEvents;
  vtkWeakPointer<vtkDMMLLightBoxRendererManagerProxy> LightBoxRendererManagerProxy;

};

//----------------------------------------------------------------------------
// vtkInternal methods

//----------------------------------------------------------------------------
vtkDMMLAbstractDisplayableManager::vtkInternal::vtkInternal(
    vtkDMMLAbstractDisplayableManager* external):External(external)
{
  this->Created = false;
  this->WidgetsObserverManager = vtkObserverManager::New();
  this->UpdateFromDMMLRequested = false;
  this->Renderer = nullptr;
  this->LightBoxRendererManagerProxy = nullptr;
  this->DMMLDisplayableNode = nullptr;
  this->DMMLDisplayableNodeObservableEvents = vtkSmartPointer<vtkIntArray>::New();
  this->DisplayableManagerGroup = nullptr;

  this->DeleteCallBackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
  this->DeleteCallBackCommand->SetCallback(
      vtkDMMLAbstractDisplayableManager::vtkInternal::DoDeleteCallback);

  this->Interactor = nullptr;
  this->InteractorCallBackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
  this->InteractorCallBackCommand->SetCallback(
      vtkDMMLAbstractDisplayableManager::vtkInternal::DoInteractorCallback);
  this->InteractorCallBackCommand->SetClientData(this->External);

  this->DMMLInteractionNode = nullptr;
  this->DMMLInteractionNodeCallBackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
  this->DMMLInteractionNodeCallBackCommand->SetCallback(
      vtkDMMLAbstractDisplayableManager::vtkInternal::DoDMMLInteractionNodeCallback);
  this->DMMLInteractionNodeCallBackCommand->SetClientData(this->External);

  this->InteractorStyle = nullptr;
  this->InteractorStyleCallBackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
  this->InteractorStyleCallBackCommand->SetCallback(
      vtkDMMLAbstractDisplayableManager::vtkInternal::DoInteractorStyleCallback);
  this->InteractorStyleCallBackCommand->SetClientData(this->External);

  // Default Interactor style events to observe
  this->InteractorStyleObservableEvents.emplace_back(vtkCommand::LeftButtonPressEvent,0.0);
  this->InteractorStyleObservableEvents.emplace_back(vtkCommand::LeftButtonReleaseEvent,0.0);
  this->InteractorStyleObservableEvents.emplace_back(vtkCommand::RightButtonPressEvent,0.0);
  this->InteractorStyleObservableEvents.emplace_back(vtkCommand::RightButtonReleaseEvent,0.0);
  this->InteractorStyleObservableEvents.emplace_back(vtkCommand::MiddleButtonPressEvent,0.0);
  this->InteractorStyleObservableEvents.emplace_back(vtkCommand::MiddleButtonReleaseEvent,0.0);
  this->InteractorStyleObservableEvents.emplace_back(vtkCommand::MouseWheelBackwardEvent,0.0);
  this->InteractorStyleObservableEvents.emplace_back(vtkCommand::MouseWheelForwardEvent,0.0);
  this->InteractorStyleObservableEvents.emplace_back(vtkCommand::EnterEvent,0.0);
  this->InteractorStyleObservableEvents.emplace_back(vtkCommand::LeaveEvent,0.0);
  this->InteractorStyleObservableEvents.emplace_back(vtkCommand::Button3DEvent,0.0);
  this->InteractorStyleObservableEvents.emplace_back(vtkCommand::Move3DEvent,0.0);
}

//-----------------------------------------------------------------------------
vtkDMMLAbstractDisplayableManager::vtkInternal::~vtkInternal()
{
  this->SetAndObserveInteractor(nullptr);
  this->SetAndObserveInteractorStyle(nullptr);
  this->SetAndObserveDMMLInteractionNode(nullptr);
  this->LightBoxRendererManagerProxy = nullptr;
  this->WidgetsObserverManager->Delete();
}

//-----------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::vtkInternal::DoDeleteCallback(vtkObject* vtk_obj,
                                                                      unsigned long event,
                                                                      void* vtkNotUsed(client_data),
                                                                      void* vtkNotUsed(call_data))
{
  vtkDMMLAbstractDisplayableManager* self =
      vtkDMMLAbstractDisplayableManager::SafeDownCast(vtk_obj);
  assert(self);
  assert(event == vtkCommand::DeleteEvent);
#ifndef _DEBUG
  (void)event;
#endif

  self->RemoveDMMLObservers();
}


//----------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::vtkInternal::
    SetAndObserveDMMLInteractionNode(vtkDMMLInteractionNode* newDMMLInteractionNode)
{
  if (this->DMMLInteractionNode == newDMMLInteractionNode)
    {
    return;
    }

  //std::cout << "SetAndObserveDMMLInteractionNode " << newDMMLInteractionNode << std::endl;

  // Remove existing interactionNode observer
  if (this->DMMLInteractionNode)
    {
    this->DMMLInteractionNode->RemoveObserver(this->DMMLInteractionNodeCallBackCommand);
    this->DMMLInteractionNode->UnRegister(this->External);
    }

  // Install observer
  if (newDMMLInteractionNode)
    {
    newDMMLInteractionNode->Register(this->External);
    newDMMLInteractionNode->AddObserver(
        vtkDMMLInteractionNode::InteractionModeChangedEvent,
        this->DMMLInteractionNodeCallBackCommand);
    }

  this->DMMLInteractionNode = newDMMLInteractionNode;
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::vtkInternal::DoDMMLInteractionNodeCallback(
    vtkObject* vtk_obj, unsigned long event, void* client_data, void* vtkNotUsed(call_data))
{
  //std::cout << "DoDMMLInteractionNodeCallback " << event << std::endl;

  // InteractionModeChangedEvent is expected
  assert(event == vtkDMMLInteractionNode::InteractionModeChangedEvent);
#ifndef _DEBUG
  (void)event;
#endif

  // vtkDMMLInteractionNode is expected to be source of the ModifiedEvent
  assert(vtkDMMLInteractionNode::SafeDownCast(vtk_obj));
#ifndef _DEBUG
  (void)vtk_obj;
#endif

  vtkDMMLAbstractDisplayableManager* self =
      reinterpret_cast<vtkDMMLAbstractDisplayableManager*>(client_data);
  assert(self);

  self->Internal->UpdateInteractorStyle();
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::vtkInternal::SetAndObserveInteractor(
    vtkRenderWindowInteractor* newInteractor)
{
  if (this->Interactor == newInteractor)
    {
    return;
    }

  // Remove existing interactor observer
  if (this->Interactor)
    {
    this->Interactor->RemoveObserver(this->InteractorCallBackCommand);
    this->Interactor->UnRegister(this->External);
    }

  // Install observers
  if (newInteractor)
    {
    newInteractor->Register(this->External);
    for(size_t i=0; i < this->InteractorObservableEvents.size(); ++i)
      {
      int eid = this->InteractorObservableEvents[i].first;
      float priority = this->InteractorObservableEvents[i].second;
      newInteractor->AddObserver(eid, this->InteractorCallBackCommand, priority);
      }
    }

  this->Interactor = newInteractor;
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::vtkInternal::SetAndObserveInteractorStyle(
    vtkInteractorObserver* newInteractorStyle)
{
  if (this->InteractorStyle == newInteractorStyle)
    {
    return;
    }

  // Remove existing interactor style observer
  if (this->InteractorStyle)
    {

    this->InteractorStyle->UnRegister(this->External);
    }

  // Install observers
  if (newInteractorStyle)
    {
    newInteractorStyle->Register(this->External);
    }

  this->InteractorStyle = newInteractorStyle;
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::vtkInternal::DoInteractorCallback(
    vtkObject* vtk_obj, unsigned long event, void* client_data, void* vtkNotUsed(call_data))
{
  // vtkInteractor is expected to be source of the event
  assert(vtkRenderWindowInteractor::SafeDownCast(vtk_obj));
#ifndef _DEBUG
  (void)vtk_obj;
#endif

  vtkDMMLAbstractDisplayableManager* self =
      reinterpret_cast<vtkDMMLAbstractDisplayableManager*>(client_data);
  assert(self);

  self->OnInteractorEvent(event);
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::vtkInternal::DoInteractorStyleCallback(
    vtkObject* vtk_obj, unsigned long event, void* client_data, void* vtkNotUsed(call_data))
{
  // vtkInteractorStyle is expected to be source of the event
  assert(vtkInteractorStyle::SafeDownCast(vtk_obj));
#ifndef _DEBUG
  (void)vtk_obj;
#endif

  vtkDMMLAbstractDisplayableManager* self =
      reinterpret_cast<vtkDMMLAbstractDisplayableManager*>(client_data);
  assert(self);

  self->OnInteractorStyleEvent(event);
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::vtkInternal::UpdateInteractorStyle(int eventIdToObserve, int eventIdToUnObserve, float priority)
{
  // TODO: The following code could be factorized in shorter and simpler functions,
  // or may be replaced by using vtkEventBroker.
  bool updateObserver = false;
  if (this->DMMLInteractionNode)
    {
    this->SetAndObserveInteractor( this->Renderer->GetRenderWindow()->GetInteractor());
    this->SetAndObserveInteractorStyle(
        this->Renderer->GetRenderWindow()->GetInteractor()->GetInteractorStyle());
    updateObserver = (this->InteractorStyle != nullptr);
    }

  // Update observe if it applies
  if (updateObserver)
    {
    if (eventIdToObserve != vtkCommand::NoEvent)
      {
      assert(!this->InteractorStyle->HasObserver(eventIdToObserve, this->InteractorStyleCallBackCommand));
      this->InteractorStyle->AddObserver(eventIdToObserve, this->InteractorStyleCallBackCommand, priority);
      }
    if (eventIdToUnObserve != vtkCommand::NoEvent)
      {
      assert(this->InteractorStyle->HasObserver(eventIdToUnObserve, this->InteractorStyleCallBackCommand));
      this->InteractorStyle->RemoveObservers(eventIdToUnObserve, this->InteractorStyleCallBackCommand);
      }
    }

  // Update InteractorStyleObservableEvents vector
  if (eventIdToObserve != vtkCommand::NoEvent)
    {
    // Check if the ObservableEvent has already been registered
    std::vector< std::pair<int, float> >::iterator it = std::find_if(
      this->InteractorStyleObservableEvents.begin(), this->InteractorStyleObservableEvents.end(),
      [&eventIdToObserve](const std::pair<int, float>& eventIdPriorityPtr) { return eventIdPriorityPtr.first == eventIdToObserve; });
    if (it != this->InteractorStyleObservableEvents.end())
      {
      vtkWarningWithObjectMacro(this->External, << "UpdateInteractorStyle - eventid:" << eventIdToObserve
                                << " has already been added to the list of observable events !");
      }
    else
      {
      this->InteractorStyleObservableEvents.emplace_back(eventIdToObserve,priority);
      }
    }

  if (eventIdToUnObserve != vtkCommand::NoEvent)
    {
    // Check if the ObservableEvent has already been registered
    std::vector< std::pair<int, float> >::iterator it = std::find_if(
      this->InteractorStyleObservableEvents.begin(), this->InteractorStyleObservableEvents.end(),
      [&eventIdToUnObserve](const std::pair<int, float>& eventIdPriorityPtr) { return eventIdPriorityPtr.first == eventIdToUnObserve; });
    if (it == this->InteractorStyleObservableEvents.end())
      {
      vtkWarningWithObjectMacro(this->External, << "UpdateInteractorStyle - eventid:" << eventIdToUnObserve
                                << " has already NOT been added to the list of observable events !");
      }
    else
      {
      this->InteractorStyleObservableEvents.erase(it);
      }
    }
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::vtkInternal::UpdateInteractor(int eventIdToObserve, int eventIdToUnObserve, float priority)
{
  // TODO: The following code could be factorized in shorter and simpler functions,
  // or may be replaced by using vtkEventBroker.
  bool updateObserver = false;
  if (this->DMMLInteractionNode)
    {
    int currentInteractionMode =
      this->DMMLInteractionNode->GetCurrentInteractionMode();
    if ( currentInteractionMode & this->External->ActiveInteractionModes() )
      {
      this->SetAndObserveInteractor(this->Renderer->GetRenderWindow()->GetInteractor());
      updateObserver = (this->Interactor != nullptr);
      }
    else
      {
      this->SetAndObserveInteractor(nullptr);
      }
    }

  // Update observe if it applies
  if (updateObserver)
    {
    if (eventIdToObserve != vtkCommand::NoEvent)
      {
      assert(!this->Interactor->HasObserver(eventIdToObserve, this->InteractorCallBackCommand));
      this->Interactor->AddObserver(eventIdToObserve, this->InteractorCallBackCommand, priority);
      }
    if (eventIdToUnObserve != vtkCommand::NoEvent)
      {
      assert(this->Interactor->HasObserver(eventIdToUnObserve, this->InteractorCallBackCommand));
      this->Interactor->RemoveObservers(eventIdToUnObserve, this->InteractorCallBackCommand);
      }
    }

  // Update InteractorObservableEvents vector
  if (eventIdToObserve != vtkCommand::NoEvent)
    {
    // Check if the ObservableEvent has already been registered
    std::vector< std::pair<int, float> >::iterator it = std::find_if(
      this->InteractorStyleObservableEvents.begin(), this->InteractorStyleObservableEvents.end(),
      [&eventIdToObserve](const std::pair<int, float>& eventIdPriorityPtr) { return eventIdPriorityPtr.first == eventIdToObserve; });
    if (it != this->InteractorObservableEvents.end())
      {
      vtkWarningWithObjectMacro(this->External, << "UpdateInteractor - eventid:" << eventIdToObserve
                                << " has already been added to the list of observable events !");
      }
    else
      {
      this->InteractorObservableEvents.emplace_back(eventIdToObserve,priority);
      }
    }

  if (eventIdToUnObserve != vtkCommand::NoEvent)
    {
    // Check if the ObservableEvent has already been registered
    std::vector< std::pair<int, float> >::iterator it = std::find_if(
      this->InteractorStyleObservableEvents.begin(), this->InteractorStyleObservableEvents.end(),
      [&eventIdToUnObserve](const std::pair<int, float>& eventIdPriorityPtr) { return eventIdPriorityPtr.first == eventIdToUnObserve; });
    if (it == this->InteractorObservableEvents.end())
      {
      vtkWarningWithObjectMacro(this->External, << "UpdateInteractor - eventid:" << eventIdToUnObserve
                                << " has already NOT been added to the list of observable events !");
      }
    else
      {
      this->InteractorObservableEvents.erase(it);
      }
    }
}

//----------------------------------------------------------------------------
// vtkDMMLAbstractDisplayableManager methods

//----------------------------------------------------------------------------
vtkDMMLAbstractDisplayableManager::vtkDMMLAbstractDisplayableManager()
{
  this->Internal = new vtkInternal(this);
  this->AddObserver(vtkCommand::DeleteEvent, this->Internal->DeleteCallBackCommand);
  // Default observable event associated with DisplayableNode
  this->AddDMMLDisplayableManagerEvent(vtkCommand::ModifiedEvent);
  this->AddDMMLDisplayableManagerEvent(vtkDMMLNode::ReferenceAddedEvent);
  this->AddDMMLDisplayableManagerEvent(vtkDMMLNode::ReferenceRemovedEvent);
  this->AddDMMLDisplayableManagerEvent(vtkDMMLNode::ReferenceModifiedEvent);

  // Setup widgets callback
  vtkObserverManager * widgetsObserver = this->Internal->WidgetsObserverManager;
  widgetsObserver->AssignOwner(this);
  widgetsObserver->GetCallbackCommand()->SetClientData(this);
  widgetsObserver->GetCallbackCommand()->SetCallback(
    vtkDMMLAbstractDisplayableManager::WidgetsCallback);
}

//----------------------------------------------------------------------------
vtkDMMLAbstractDisplayableManager::~vtkDMMLAbstractDisplayableManager()
{
  if (this->Internal->Renderer)
    {
    this->Internal->Renderer->UnRegister(this);
    }
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
vtkDMMLDisplayableManagerGroup * vtkDMMLAbstractDisplayableManager
::GetDMMLDisplayableManagerGroup()
{
  return this->Internal->DisplayableManagerGroup;
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::CreateIfPossible()
{
  if (!this->GetDMMLDisplayableNode())
    {
    return;
    }
  if (!this->IsCreated())
    {
    assert(this->GetDMMLScene());
    assert(this->GetDMMLDisplayableNode());

    // Look for InteractionNode
    if (!this->GetInteractionNode())
      {
      vtkWarningMacro( << "CreateIfPossible - DMMLScene does NOT contain any InteractionNode");
      }

    this->Create();
    this->Internal->Created = true;
    }
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::Create()
{
  this->ProcessDMMLNodesEvents(this->GetDMMLDisplayableNode(), vtkCommand::ModifiedEvent, nullptr);
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager
::SetDMMLDisplayableManagerGroup(vtkDMMLDisplayableManagerGroup * group)
{
  // Sanity checks
  if (this->Internal->DisplayableManagerGroup == group)
    {
    return;
    }
  this->Internal->DisplayableManagerGroup = group;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::SetRenderer(vtkRenderer* newRenderer)
{
  // Sanity checks
  if (this->Internal->Renderer == newRenderer)
    {
    return;
    }

  if (this->Internal->Renderer)
    {
    this->Internal->Renderer->Delete();
    }

  this->Internal->Renderer = newRenderer;

  if (this->Internal->Renderer)
    {
    this->Internal->Renderer->Register(this);
    }

  this->AdditionalInitializeStep();

  if (this->Internal->Renderer)
    {
    // Need to do this AFTER the call to AdditionalInitializeStep(), otherwise
    // the events registered in AfterInitializeStep() are not
    // observed.
    // Indeed, the method "AddInteractorObservableEvent" expects the DMMLInteractionNode
    // to be set to add the observer(s). DMMLInteractionNode being set after "SetRenderer" function
    // is completed, the method "SetAndObserveInteractor" that also setup observers has to be called
    // after the observable events are added.
    this->Internal->SetAndObserveInteractor(this->Internal->Renderer->GetRenderWindow()->GetInteractor());
    }

  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkDMMLAbstractDisplayableManager::IsCreated()
{
  return this->Internal->Created;
}

//---------------------------------------------------------------------------
vtkRenderer * vtkDMMLAbstractDisplayableManager::GetRenderer()
{
  return this->Internal->Renderer;
}

//---------------------------------------------------------------------------
vtkRenderWindowInteractor * vtkDMMLAbstractDisplayableManager::GetInteractor()
{

  if (!this->Internal->Interactor)
    {
    vtkDebugMacro(<< this->GetClassName() << " (" << this << "): returning Interactor address 0");
    return nullptr;
    }
  vtkDebugMacro("returning Internal->Interactor address "
                << this->Internal->Interactor );
  return this->Internal->Interactor;
}

//---------------------------------------------------------------------------
vtkDMMLInteractionNode* vtkDMMLAbstractDisplayableManager::GetInteractionNode()
{
  return this->Internal->DMMLInteractionNode;
}

//---------------------------------------------------------------------------
vtkDMMLSelectionNode* vtkDMMLAbstractDisplayableManager::GetSelectionNode()
{
  return this->GetDMMLScene() ? vtkDMMLSelectionNode::SafeDownCast(this->GetDMMLScene()->GetNodeByID("vtkDMMLSelectionNodeSingleton")): nullptr;
}

//---------------------------------------------------------------------------
vtkDMMLNode * vtkDMMLAbstractDisplayableManager::GetDMMLDisplayableNode()
{
  return this->Internal->DMMLDisplayableNode;
}

//---------------------------------------------------------------------------
int vtkDMMLAbstractDisplayableManager::ActiveInteractionModes() {
  return vtkDMMLInteractionNode::Place;
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::ProcessDMMLNodesEvents(
  vtkObject* caller, unsigned long event, void * callData)
{
  if (caller == this->GetDMMLDisplayableNode())
    {
      if(event == vtkCommand::ModifiedEvent)
        {
        this->OnDMMLDisplayableNodeModifiedEvent(caller);
        return;
        }
      else if(event == vtkDMMLNode::ReferenceAddedEvent ||
              event == vtkDMMLNode::ReferenceRemovedEvent ||
              event == vtkDMMLNode::ReferenceModifiedEvent)
        {
        // Update interaction node
        vtkDMMLAbstractViewNode* viewNode = vtkDMMLAbstractViewNode::SafeDownCast(this->GetDMMLDisplayableNode());
        if (viewNode)
          {
          this->Internal->SetAndObserveDMMLInteractionNode(viewNode->GetInteractionNode());
          }
        else
          {
          vtkErrorMacro(<< "ProcessDMMLNodesEvents failed: "
                        << "No viewNode is associated with the displayable manager: " << this->GetClassName());
          }
        }
    }
  this->Superclass::ProcessDMMLNodesEvents(caller, event, callData);
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager
::OnDMMLDisplayableNodeModifiedEvent(vtkObject* vtkNotUsed(caller))
{
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager
::ProcessWidgetsEvents(vtkObject *vtkNotUsed(caller),
                       unsigned long vtkNotUsed(event),
                       void *vtkNotUsed(callData))
{
}

//----------------------------------------------------------------------------
// Description:
// the WidgetCallback is a static function to relay modified events from the
// observed vtk widgets back into the dmml node for further processing
void vtkDMMLAbstractDisplayableManager::WidgetsCallback(vtkObject *caller,
                                                        unsigned long eid,
                                                        void *clientData,
                                                        void *callData)
{
  vtkDMMLAbstractDisplayableManager* self =
    reinterpret_cast<vtkDMMLAbstractDisplayableManager *>(clientData);
  assert(!caller->IsA("vtkDMMLNode"));
  self->ProcessWidgetsEvents(caller, eid, callData);
}

//----------------------------------------------------------------------------
vtkCallbackCommand* vtkDMMLAbstractDisplayableManager::GetWidgetsCallbackCommand()
{
  return this->GetWidgetsObserverManager()->GetCallbackCommand();
}

//----------------------------------------------------------------------------
vtkObserverManager* vtkDMMLAbstractDisplayableManager::GetWidgetsObserverManager()const
{
  return this->Internal->WidgetsObserverManager;
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::SetDMMLSceneInternal(vtkDMMLScene* newScene)
{
  assert(newScene != this->GetDMMLScene());

  vtkNew<vtkIntArray> sceneEvents;
  sceneEvents->InsertNextValue(vtkDMMLScene::StartBatchProcessEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::EndBatchProcessEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::StartCloseEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::EndCloseEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::StartImportEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::EndImportEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::StartRestoreEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::EndRestoreEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::NewSceneEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::NodeAddedEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::NodeRemovedEvent);

  this->SetAndObserveDMMLSceneEventsInternal(newScene, sceneEvents.GetPointer());
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::AddDMMLDisplayableManagerEvent(int eventId)
{
  for(int i = 0; i < this->Internal->DMMLDisplayableNodeObservableEvents->GetNumberOfValues(); ++i)
    {
    if (eventId == this->Internal->DMMLDisplayableNodeObservableEvents->GetValue(i))
      {
      vtkErrorMacro(<< "AddDMMLDisplayableManagerEvent - eventId:" << eventId
                    << " already added");
      return;
      }
    }
  this->Internal->DMMLDisplayableNodeObservableEvents->InsertNextValue(eventId);
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::SetAndObserveDMMLDisplayableNode(
    vtkDMMLNode * newDMMLDisplayableNode)
{
  // Observe scene associated with the DMML DisplayableNode
  vtkDMMLScene * sceneToObserve = nullptr;
  vtkDMMLAbstractViewNode* viewNode = vtkDMMLAbstractViewNode::SafeDownCast(newDMMLDisplayableNode);
  if (viewNode)
    {
    sceneToObserve = viewNode->GetScene();

    // Observe InteractionNode
    vtkDMMLInteractionNode *interactionNode = viewNode->GetInteractionNode();
    this->Internal->SetAndObserveDMMLInteractionNode(interactionNode);
    if (interactionNode)
      {
      this->Internal->UpdateInteractorStyle();
      }
    else
      {
      vtkWarningMacro(<< "SetAndObserveDMMLDisplayableNode - "
                      "DMMLScene does NOT contain any InteractionNode");
      }
    }
  vtkSetAndObserveDMMLNodeEventsMacro(this->Internal->DMMLDisplayableNode,
                                      viewNode,
                                      this->Internal->DMMLDisplayableNodeObservableEvents);
  this->SetDMMLScene(sceneToObserve);
  this->SetUpdateFromDMMLRequested(true);
  this->CreateIfPossible();
  if (viewNode != nullptr)
    {
    this->RequestRender();
    }
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::SetUpdateFromDMMLRequested(bool requested)
{
  if (this->Internal->UpdateFromDMMLRequested == requested)
    {
    return;
    }

  this->Internal->UpdateFromDMMLRequested = requested;

  this->Modified();
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::RequestRender()
{
  // TODO Add a mechanism to check if Rendering is disable

  if (this->Internal->UpdateFromDMMLRequested)
    {
    this->UpdateFromDMML();
    }

  this->InvokeEvent(vtkCommand::UpdateEvent);
  if (this->Internal->DisplayableManagerGroup)
    {
    this->Internal->DisplayableManagerGroup->RequestRender();
    }
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::RemoveDMMLObservers()
{
  this->SetAndObserveDMMLDisplayableNode(nullptr);
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::AddInteractorStyleObservableEvent(int eventid, float priority)
{
  this->Internal->UpdateInteractorStyle(eventid, vtkCommand::NoEvent, priority);
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::RemoveInteractorStyleObservableEvent(int eventid)
{
  this->Internal->UpdateInteractorStyle(vtkCommand::NoEvent, eventid);
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::AddInteractorObservableEvent(int eventid, float priority)
{
  this->Internal->UpdateInteractor(eventid, vtkCommand::NoEvent, priority);
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::RemoveInteractorObservableEvent(int eventid)
{
  this->Internal->UpdateInteractor(vtkCommand::NoEvent, eventid);
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::OnInteractorStyleEvent(int vtkNotUsed(eventid))
{
  // std::cout << "OnInteractorStyleEvent - eventid:" << eventid
  //           << ", eventname:" << vtkCommand::GetStringFromEventId(eventid) << std::endl;
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::OnInteractorEvent(int vtkNotUsed(eventid))
{
  // std::cout << "OnInteractorEvent - eventid:" << eventid
  //           << ", eventname:" << vtkCommand::GetStringFromEventId(eventid) << std::endl;
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::SetInteractorStyleAbortFlag(int f)
{
  this->Internal->InteractorStyleCallBackCommand->SetAbortFlag(f);
}

//---------------------------------------------------------------------------
int vtkDMMLAbstractDisplayableManager::GetInteractorStyleAbortFlag()
{
  return this->Internal->InteractorStyleCallBackCommand->GetAbortFlag();
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::InteractorStyleAbortFlagOn()
{
  this->Internal->InteractorStyleCallBackCommand->AbortFlagOn();
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::InteractorStyleAbortFlagOff()
{
  this->Internal->InteractorStyleCallBackCommand->AbortFlagOff();
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::SetInteractorAbortFlag(int f)
{
  this->Internal->InteractorCallBackCommand->SetAbortFlag(f);
}

//---------------------------------------------------------------------------
int vtkDMMLAbstractDisplayableManager::GetInteractorAbortFlag()
{
  return this->Internal->InteractorCallBackCommand->GetAbortFlag();
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::InteractorAbortFlagOn()
{
  this->Internal->InteractorCallBackCommand->AbortFlagOn();
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::InteractorAbortFlagOff()
{
  this->Internal->InteractorCallBackCommand->AbortFlagOff();
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::SetLightBoxRendererManagerProxy(vtkDMMLLightBoxRendererManagerProxy* mgr)
{
  this->Internal->LightBoxRendererManagerProxy = mgr;
}

//---------------------------------------------------------------------------
vtkDMMLLightBoxRendererManagerProxy* vtkDMMLAbstractDisplayableManager::GetLightBoxRendererManagerProxy()
{
  return this->Internal->LightBoxRendererManagerProxy;
}

//---------------------------------------------------------------------------
vtkRenderer* vtkDMMLAbstractDisplayableManager::GetRenderer(int idx)
{
  if (this->Internal->LightBoxRendererManagerProxy)
    {
    return this->Internal->LightBoxRendererManagerProxy->GetRenderer(idx);
    }
  else
    {
    return this->GetRenderer();
    }
}

//---------------------------------------------------------------------------
bool vtkDMMLAbstractDisplayableManager::CanProcessInteractionEvent(vtkDMMLInteractionEventData* vtkNotUsed(eventData), double &distance2)
{
  distance2 = VTK_DOUBLE_MAX;
  return false;
}

//---------------------------------------------------------------------------
bool vtkDMMLAbstractDisplayableManager::ProcessInteractionEvent(vtkDMMLInteractionEventData* vtkNotUsed(eventData))
{
  return false;
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::SetHasFocus(bool vtkNotUsed(hasFocus), vtkDMMLInteractionEventData* vtkNotUsed(eventData))
{
  return;
}

//---------------------------------------------------------------------------
bool vtkDMMLAbstractDisplayableManager::GetGrabFocus()
{
  return false;
}

//---------------------------------------------------------------------------
bool vtkDMMLAbstractDisplayableManager::GetInteractive()
{
  return false;
}

//---------------------------------------------------------------------------
int vtkDMMLAbstractDisplayableManager::GetMouseCursor()
{
  return VTK_CURSOR_DEFAULT;
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractDisplayableManager::SetMouseCursor(int cursor)
{
  if (this->GetRenderer() && this->GetRenderer()->GetRenderWindow())
  {
    this->GetRenderer()->GetRenderWindow()->SetCurrentCursor(cursor);
  }
}
