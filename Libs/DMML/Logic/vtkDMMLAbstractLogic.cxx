/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLAbstractLogic.cxx,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/

// DMMLLogic includes
#include "vtkDMMLAbstractLogic.h"
//#include "vtkDMMLApplicationLogic.h"

// DMML includes
#include "vtkDMMLNode.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// STD includes
#include <cassert>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLAbstractLogic);

//----------------------------------------------------------------------------
class vtkDMMLAbstractLogic::vtkInternal
{
public:
  vtkInternal();
  ~vtkInternal();

  vtkDMMLScene *           DMMLScene;
  vtkDMMLApplicationLogic* DMMLApplicationLogic;

  vtkObserverManager * DMMLSceneObserverManager;

  int                  InDMMLSceneCallbackFlag;
  int                  ProcessingDMMLSceneEvent;

  vtkObserverManager * DMMLNodesObserverManager;
  int                  InDMMLNodesCallbackFlag;

  vtkObserverManager * DMMLLogicsObserverManager;
  int                  InDMMLLogicsCallbackFlag;

  bool                 DisableModifiedEvent;
  int                  ModifiedEventPending;
};

//----------------------------------------------------------------------------
// vtkInternal methods

//----------------------------------------------------------------------------
vtkDMMLAbstractLogic::vtkInternal::vtkInternal()
{
  this->DMMLApplicationLogic = nullptr;
  this->DMMLScene = nullptr;

  this->DMMLSceneObserverManager = vtkObserverManager::New();
  this->InDMMLSceneCallbackFlag = false;
  this->ProcessingDMMLSceneEvent = 0;

  this->DMMLNodesObserverManager = vtkObserverManager::New();
  this->InDMMLNodesCallbackFlag = false;

  this->DMMLLogicsObserverManager = vtkObserverManager::New();
  this->InDMMLLogicsCallbackFlag = false;

  this->DisableModifiedEvent = false;
  this->ModifiedEventPending = 0;
}

//----------------------------------------------------------------------------
vtkDMMLAbstractLogic::vtkInternal::~vtkInternal()
{
  this->DMMLSceneObserverManager->Delete();
  this->DMMLNodesObserverManager->Delete();
  this->DMMLLogicsObserverManager->Delete();
}

//----------------------------------------------------------------------------
// vtkDMMLAbstractLogic methods

//----------------------------------------------------------------------------
vtkDMMLAbstractLogic::vtkDMMLAbstractLogic()
{
  this->Internal = new vtkInternal;

  // Setup DMML scene callback
  vtkObserverManager * sceneObserverManager = this->Internal->DMMLSceneObserverManager;
  sceneObserverManager->AssignOwner(this);
  sceneObserverManager->GetCallbackCommand()->SetClientData(reinterpret_cast<void *>(this));
  sceneObserverManager->GetCallbackCommand()->SetCallback(vtkDMMLAbstractLogic::DMMLSceneCallback);

  // Setup DMML nodes callback
  vtkObserverManager * nodesObserverManager = this->Internal->DMMLNodesObserverManager;
  nodesObserverManager->AssignOwner(this);
  nodesObserverManager->GetCallbackCommand()->SetClientData(reinterpret_cast<void *> (this));
  nodesObserverManager->GetCallbackCommand()->SetCallback(vtkDMMLAbstractLogic::DMMLNodesCallback);

  // Setup DMML logics callback
  vtkObserverManager * logicsObserverManager = this->Internal->DMMLLogicsObserverManager;
  logicsObserverManager->AssignOwner(this);
  logicsObserverManager->GetCallbackCommand()->SetClientData(reinterpret_cast<void *> (this));
  logicsObserverManager->GetCallbackCommand()->SetCallback(vtkDMMLAbstractLogic::DMMLLogicsCallback);
}

//----------------------------------------------------------------------------
vtkDMMLAbstractLogic::~vtkDMMLAbstractLogic()
{
  this->SetDMMLScene(nullptr);
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "ClassName:   " << this->GetClassName() << "\n";
  os << indent << "DMMLScene:   " << this->GetDMMLScene() << "\n";
}


//----------------------------------------------------------------------------
vtkDMMLApplicationLogic* vtkDMMLAbstractLogic::GetDMMLApplicationLogic()const
{
  return this->Internal->DMMLApplicationLogic;
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractLogic::SetDMMLApplicationLogic(vtkDMMLApplicationLogic* logic)
{
  if (logic == this->Internal->DMMLApplicationLogic)
    {
    return;
    }
  this->Internal->DMMLApplicationLogic = logic;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractLogic::DMMLSceneCallback(vtkObject*caller, unsigned long eid,
                                             void* clientData, void* callData)
{
  vtkDMMLAbstractLogic *self = reinterpret_cast<vtkDMMLAbstractLogic *>(clientData);
  assert(vtkDMMLScene::SafeDownCast(caller));
  assert(caller == self->GetDMMLScene());

  if (self && !self->EnterDMMLSceneCallback())
    {
#ifdef _DEBUG
    vtkWarningWithObjectMacro(self, "vtkDMMLAbstractLogic ******* DMMLSceneCallback called recursively?");
#endif
    return;
    }

  vtkDebugWithObjectMacro(self, "In vtkDMMLAbstractLogic DMMLSceneCallback");

  self->SetInDMMLSceneCallbackFlag(self->GetInDMMLSceneCallbackFlag() + 1);
  int oldProcessingEvent = self->GetProcessingDMMLSceneEvent();
  self->SetProcessingDMMLSceneEvent(eid);
  self->ProcessDMMLSceneEvents(caller, eid, callData);
  self->SetProcessingDMMLSceneEvent(oldProcessingEvent);
  self->SetInDMMLSceneCallbackFlag(self->GetInDMMLSceneCallbackFlag() - 1);
}

//----------------------------------------------------------------------------
// Description:
// the DMMLNodesCallback is a static function to relay modified events from the
// observed dmml node back into the gui layer for further processing
//
void vtkDMMLAbstractLogic::DMMLNodesCallback(vtkObject* caller, unsigned long eid,
                                             void* clientData, void* callData)
{
  vtkDMMLAbstractLogic *self = reinterpret_cast<vtkDMMLAbstractLogic *>(clientData);
  assert("Observed object is not a node" && vtkDMMLNode::SafeDownCast(caller));

  if (self && !self->EnterDMMLNodesCallback())
    {
#ifdef _DEBUG
    vtkWarningWithObjectMacro(self, "vtkDMMLAbstractLogic ******* DMMLNodesCallback called recursively?");
#endif
    return;
    }
  vtkDebugWithObjectMacro(self, "In vtkDMMLAbstractLogic DMMLNodesCallback");

  self->SetInDMMLNodesCallbackFlag(self->GetInDMMLNodesCallbackFlag() + 1);
  self->ProcessDMMLNodesEvents(caller, eid, callData);
  self->SetInDMMLNodesCallbackFlag(self->GetInDMMLNodesCallbackFlag() - 1);
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractLogic
::DMMLLogicsCallback(vtkObject*caller, unsigned long eid,
                     void* clientData, void* callData)
{
  vtkDMMLAbstractLogic *self = reinterpret_cast<vtkDMMLAbstractLogic *>(clientData);
  assert("Observed object is not a logic" &&
         vtkDMMLAbstractLogic::SafeDownCast(caller));

  if (self && !self->EnterDMMLLogicsCallback())
    {
#ifdef _DEBUG
    vtkWarningWithObjectMacro(self,
      "vtkDMMLAbstractLogic ******* DMMLLogicCallback called recursively?");
#endif
    return;
    }
  vtkDebugWithObjectMacro(self, "In vtkDMMLAbstractLogic DMMLLogicsCallback");

  self->SetInDMMLLogicsCallbackFlag(self->GetInDMMLLogicsCallbackFlag() + 1);
  self->ProcessDMMLLogicsEvents(caller, eid, callData);
  self->SetInDMMLLogicsCallbackFlag(self->GetInDMMLLogicsCallbackFlag() - 1);
}

//----------------------------------------------------------------------------
vtkDMMLScene * vtkDMMLAbstractLogic::GetDMMLScene()const
{
  return this->Internal->DMMLScene;
}

//----------------------------------------------------------------------------
vtkObserverManager* vtkDMMLAbstractLogic::GetDMMLSceneObserverManager()const
{
  return this->Internal->DMMLSceneObserverManager;
}

//----------------------------------------------------------------------------
vtkCallbackCommand* vtkDMMLAbstractLogic::GetDMMLSceneCallbackCommand()
{
  return this->GetDMMLSceneObserverManager()->GetCallbackCommand();
}

//----------------------------------------------------------------------------
vtkObserverManager* vtkDMMLAbstractLogic::GetDMMLNodesObserverManager()const
{
  return this->Internal->DMMLNodesObserverManager;
}

//----------------------------------------------------------------------------
vtkCallbackCommand* vtkDMMLAbstractLogic::GetDMMLNodesCallbackCommand()
{
  return this->GetDMMLNodesObserverManager()->GetCallbackCommand();
}

//----------------------------------------------------------------------------
vtkObserverManager* vtkDMMLAbstractLogic::GetDMMLLogicsObserverManager()const
{
  return this->Internal->DMMLLogicsObserverManager;
}

//----------------------------------------------------------------------------
vtkCallbackCommand* vtkDMMLAbstractLogic::GetDMMLLogicsCallbackCommand()
{
  return this->GetDMMLLogicsObserverManager()->GetCallbackCommand();
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractLogic::SetDMMLScene(vtkDMMLScene * newScene)
{
  if (this->Internal->DMMLScene == newScene)
    {
    return;
    }

  if (this->Internal->DMMLScene != nullptr)
    {
    this->UnobserveDMMLScene();
    }

  this->SetDMMLSceneInternal(newScene);

  if (newScene)
    {
    this->RegisterNodes();
    this->ObserveDMMLScene();
    }
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractLogic::SetDMMLSceneInternal(vtkDMMLScene * newScene)
{
  this->GetDMMLSceneObserverManager()->SetObject(
    vtkObjectPointer(&this->Internal->DMMLScene), newScene);
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractLogic::SetAndObserveDMMLScene(vtkDMMLScene *newScene)
{
  if (this->Internal->DMMLScene == newScene)
    {
    return;
    }

  if (this->Internal->DMMLScene != nullptr)
    {
    this->UnobserveDMMLScene();
    }

  this->SetAndObserveDMMLSceneInternal(newScene);

  if (newScene)
    {
    this->ObserveDMMLScene();
    }

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractLogic::SetAndObserveDMMLSceneInternal(vtkDMMLScene *newScene)
{
  this->GetDMMLSceneObserverManager()->SetAndObserveObject(
    vtkObjectPointer(&this->Internal->DMMLScene), newScene);
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractLogic::SetAndObserveDMMLSceneEvents(vtkDMMLScene *newScene, vtkIntArray *events, vtkFloatArray *priorities)
{
  if (this->Internal->DMMLScene == newScene)
    {
    return;
    }

  if (this->Internal->DMMLScene)
    {
    this->UnobserveDMMLScene();
    }

  this->SetAndObserveDMMLSceneEventsInternal(newScene, events, priorities);

  if (newScene)
    {
    this->ObserveDMMLScene();
    }
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractLogic::SetAndObserveDMMLSceneEventsInternal(vtkDMMLScene *newScene,
                                                                vtkIntArray *events,
                                                                vtkFloatArray *priorities
)
{
  this->GetDMMLSceneObserverManager()->SetAndObserveObjectEvents(
    vtkObjectPointer(&this->Internal->DMMLScene), newScene, events, priorities);
}

//----------------------------------------------------------------------------
int vtkDMMLAbstractLogic::GetProcessingDMMLSceneEvent()const
{
  return this->Internal->ProcessingDMMLSceneEvent;
}

//----------------------------------------------------------------------------
// NOTE: Do *NOT* use the SetMacro or it call modified itself and generate even more events !
void vtkDMMLAbstractLogic::SetProcessingDMMLSceneEvent(int event)
{
  this->Internal->ProcessingDMMLSceneEvent = event;
}


//----------------------------------------------------------------------------
// NOTE: Do *NOT* use the SetMacro or it call modified itself and generate even more events !
void vtkDMMLAbstractLogic::SetInDMMLSceneCallbackFlag(int flag)
{
  this->Internal->InDMMLSceneCallbackFlag = flag;
}

//----------------------------------------------------------------------------
int vtkDMMLAbstractLogic::GetInDMMLSceneCallbackFlag()const
{
  return this->Internal->InDMMLSceneCallbackFlag;
}

//----------------------------------------------------------------------------
bool vtkDMMLAbstractLogic::EnterDMMLSceneCallback()const
{
  return true;
}

//----------------------------------------------------------------------------
// NOTE: Do *NOT* use the SetMacro or it call modified itself and generate even more events !
void vtkDMMLAbstractLogic::SetInDMMLNodesCallbackFlag(int flag)
{
  this->Internal->InDMMLNodesCallbackFlag = flag;
}

//----------------------------------------------------------------------------
int vtkDMMLAbstractLogic::GetInDMMLNodesCallbackFlag()const
{
  return this->Internal->InDMMLNodesCallbackFlag;
}

//----------------------------------------------------------------------------
bool vtkDMMLAbstractLogic::EnterDMMLNodesCallback()const
{
  return true;
}

//----------------------------------------------------------------------------
// NOTE: Do *NOT* use the SetMacro or it call modified itself and generate even more events !
void vtkDMMLAbstractLogic::SetInDMMLLogicsCallbackFlag(int flag)
{
  this->Internal->InDMMLLogicsCallbackFlag = flag;
}

//----------------------------------------------------------------------------
int vtkDMMLAbstractLogic::GetInDMMLLogicsCallbackFlag()const
{
  return this->Internal->InDMMLLogicsCallbackFlag;
}

//----------------------------------------------------------------------------
bool vtkDMMLAbstractLogic::EnterDMMLLogicsCallback()const
{
  return true;
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractLogic
::ProcessDMMLSceneEvents(vtkObject *caller, unsigned long event, void *callData)
{
  assert(!caller || vtkDMMLScene::SafeDownCast(caller));
  assert(caller == this->GetDMMLScene());
#ifndef _NDEBUG
  (void)caller;
#endif

  vtkDMMLNode * node = nullptr;

  switch(event)
    {
    case vtkDMMLScene::StartBatchProcessEvent:
      this->OnDMMLSceneStartBatchProcess();
      break;
    case vtkDMMLScene::EndBatchProcessEvent:
      this->OnDMMLSceneEndBatchProcess();
      break;
    case vtkDMMLScene::StartCloseEvent:
      this->OnDMMLSceneStartClose();
      break;
    case vtkDMMLScene::EndCloseEvent:
      this->OnDMMLSceneEndClose();
      break;
    case vtkDMMLScene::StartImportEvent:
      this->OnDMMLSceneStartImport();
      break;
    case vtkDMMLScene::EndImportEvent:
      this->OnDMMLSceneEndImport();
      break;
    case vtkDMMLScene::StartRestoreEvent:
      this->OnDMMLSceneStartRestore();
      break;
    case vtkDMMLScene::EndRestoreEvent:
      this->OnDMMLSceneEndRestore();
      break;
    case vtkDMMLScene::NewSceneEvent:
      this->OnDMMLSceneNew();
      break;
    case vtkDMMLScene::NodeAddedEvent:
      node = reinterpret_cast<vtkDMMLNode*>(callData);
      assert(node);
      this->OnDMMLSceneNodeAdded(node);
      break;
    case vtkDMMLScene::NodeRemovedEvent:
      node = reinterpret_cast<vtkDMMLNode*>(callData);
      assert(node);
      this->OnDMMLSceneNodeRemoved(node);
      break;
    default:
      break;
    }
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractLogic::UnobserveDMMLScene()
{
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractLogic::ObserveDMMLScene()
{
  this->UpdateFromDMMLScene();
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractLogic::UpdateFromDMMLScene()
{
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractLogic::OnDMMLSceneEndBatchProcess()
{
  this->UpdateFromDMMLScene();
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractLogic
::ProcessDMMLNodesEvents(vtkObject *caller, unsigned long event, void *vtkNotUsed(callData))
{
  vtkDMMLNode * node = vtkDMMLNode::SafeDownCast(caller);
  assert(node);
  switch(event)
    {
    case vtkCommand::ModifiedEvent:
      this->OnDMMLNodeModified(node);
      break;
    default:
      break;
    }
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractLogic
::ProcessDMMLLogicsEvents(vtkObject * caller,
                          unsigned long vtkNotUsed(event),
                          void *vtkNotUsed(callData))
{
#ifndef _NDEBUG
  (void)caller;
#else
  vtkDMMLAbstractLogic* logic = vtkDMMLAbstractLogic::SafeDownCast(caller);
  assert(logic);
#endif
}

//---------------------------------------------------------------------------
bool vtkDMMLAbstractLogic::GetDisableModifiedEvent()const
{
  return this->Internal->DisableModifiedEvent;
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractLogic::SetDisableModifiedEvent(bool onOff)
{
  this->Internal->DisableModifiedEvent = onOff;
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractLogic::Modified()
{
  if (this->GetDisableModifiedEvent())
    {
    ++this->Internal->ModifiedEventPending;
    return;
    }
  this->Superclass::Modified();
}

//---------------------------------------------------------------------------
int vtkDMMLAbstractLogic::InvokePendingModifiedEvent ()
{
  if ( this->Internal->ModifiedEventPending )
    {
    int oldModifiedEventPending = this->Internal->ModifiedEventPending;
    this->Internal->ModifiedEventPending = 0;
    this->Superclass::Modified();
    return oldModifiedEventPending;
    }
  return this->Internal->ModifiedEventPending;
}

//---------------------------------------------------------------------------
int vtkDMMLAbstractLogic::GetPendingModifiedEventCount()const
{
  return this->Internal->ModifiedEventPending;
}
