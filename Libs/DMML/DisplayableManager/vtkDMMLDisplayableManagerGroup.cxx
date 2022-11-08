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
#include "vtkDMMLDisplayableManagerFactory.h"
#include <vtkDMMLLightBoxRendererManagerProxy.h>

#ifdef DMMLDisplayableManager_USE_PYTHON
#include "vtkDMMLScriptedDisplayableManager.h"
#endif

// DMML includes
#include <vtkDMMLNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkDebugLeaks.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

// STD includes
#include <algorithm>
#include <cassert>
#include <vector>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLDisplayableManagerGroup);

//----------------------------------------------------------------------------
class vtkDMMLDisplayableManagerGroup::vtkInternal
{
public:
  vtkInternal();

  // Collection of Displayable Managers
  std::vector<vtkDMMLAbstractDisplayableManager *> DisplayableManagers;

  // .. and its associated convenient typedef
  typedef std::vector<vtkDMMLAbstractDisplayableManager *>::iterator DisplayableManagersIt;

  // Map DisplayableManagerName -> DisplayableManagers*
  std::map<std::string, vtkDMMLAbstractDisplayableManager*> NameToDisplayableManagerMap;

  // .. and its associated convenient typedef
  typedef std::map<std::string, vtkDMMLAbstractDisplayableManager*>::iterator
      NameToDisplayableManagerMapIt;

  vtkSmartPointer<vtkCallbackCommand>   CallBackCommand;
  vtkDMMLDisplayableManagerFactory*     DisplayableManagerFactory;
  vtkDMMLNode*                          DMMLDisplayableNode;
  vtkRenderer*                          Renderer;
  vtkWeakPointer<vtkDMMLLightBoxRendererManagerProxy> LightBoxRendererManagerProxy;
};

//----------------------------------------------------------------------------
// vtkInternal methods

//----------------------------------------------------------------------------
vtkDMMLDisplayableManagerGroup::vtkInternal::vtkInternal()
{
  this->DMMLDisplayableNode = nullptr;
  this->Renderer = nullptr;
  this->CallBackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
  this->DisplayableManagerFactory = nullptr;
  this->LightBoxRendererManagerProxy = nullptr;
}

//----------------------------------------------------------------------------
// vtkDMMLDisplayableManagerGroup methods

//----------------------------------------------------------------------------
vtkDMMLDisplayableManagerGroup::vtkDMMLDisplayableManagerGroup()
{
  this->Internal = new vtkInternal;
  this->Internal->CallBackCommand->SetCallback(Self::DoCallback);
  this->Internal->CallBackCommand->SetClientData(this);
}

//----------------------------------------------------------------------------
vtkDMMLDisplayableManagerGroup::~vtkDMMLDisplayableManagerGroup()
{
  this->SetAndObserveDisplayableManagerFactory(nullptr);
  this->SetDMMLDisplayableNode(nullptr);

  for(size_t i=0; i < this->Internal->DisplayableManagers.size(); ++i)
    {
    this->Internal->DisplayableManagers[i]->Delete();
    }

  if (this->Internal->Renderer)
    {
    this->Internal->Renderer->UnRegister(this);
    }

  if (this->Internal->LightBoxRendererManagerProxy)
    {
    this->Internal->LightBoxRendererManagerProxy = nullptr;
    }

  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableManagerGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
bool vtkDMMLDisplayableManagerGroup
::IsADisplayableManager(const char* displayableManagerName)
{
  // Check if displayableManagerName is a valid displayable manager
  vtkSmartPointer<vtkObject> objectSmartPointer;
  objectSmartPointer.TakeReference(vtkObjectFactory::CreateInstance(displayableManagerName));
  if (objectSmartPointer.GetPointer() &&
      objectSmartPointer->IsA("vtkDMMLAbstractDisplayableManager"))
    {
    return true;
    }
#ifdef VTK_DEBUG_LEAKS
  vtkDebugLeaks::DestructClass(displayableManagerName);
#endif
#ifdef DMMLDisplayableManager_USE_PYTHON
  // Check if vtkClassOrScriptName is a python script
  if (std::string(displayableManagerName).find(".py") != std::string::npos)
    {
    // TODO Make sure the file exists ...
    return true;
    }
#endif
  return false;
}

//----------------------------------------------------------------------------
vtkDMMLAbstractDisplayableManager* vtkDMMLDisplayableManagerGroup
::InstantiateDisplayableManager(const char* displayableManagerName)
{
  vtkDMMLAbstractDisplayableManager* displayableManager = nullptr;
#ifdef DMMLDisplayableManager_USE_PYTHON
  // Are we dealing with a python scripted displayable manager
  if (std::string(displayableManagerName).find(".py") != std::string::npos)
    {
    // TODO Make sure the file exists ...
    vtkDMMLScriptedDisplayableManager* scriptedDisplayableManager =
      vtkDMMLScriptedDisplayableManager::New();
    scriptedDisplayableManager->SetPythonSource(displayableManagerName);
    displayableManager = scriptedDisplayableManager;
    }
  else
    {
#endif
    // Object will be unregistered when the SmartPointer will go out-of-scope
    displayableManager = vtkDMMLAbstractDisplayableManager::SafeDownCast(
      vtkObjectFactory::CreateInstance(displayableManagerName));
#ifdef DMMLDisplayableManager_USE_PYTHON
    }
#endif
  return displayableManager;
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableManagerGroup::Initialize(vtkDMMLDisplayableManagerFactory * factory,
                                                vtkRenderer * renderer)
{
  // Sanity checks
  if (!factory)
    {
    vtkWarningMacro(<<"Initialize - factory is NULL");
    return;
    }
  if (!renderer)
    {
    vtkWarningMacro(<<"Initialize - renderer is NULL");
    return;
    }

  // A Group observes the factory and eventually instantiates new DisplayableManager
  // when they are registered in the factory
  this->SetAndObserveDisplayableManagerFactory(factory);
  this->SetRenderer(renderer);

  for(int i=0; i < factory->GetRegisteredDisplayableManagerCount(); ++i)
    {
    std::string classOrScriptName = factory->GetRegisteredDisplayableManagerName(i);
    vtkSmartPointer<vtkDMMLAbstractDisplayableManager> displayableManager;
    displayableManager.TakeReference(
      vtkDMMLDisplayableManagerGroup::InstantiateDisplayableManager(classOrScriptName.c_str()));
    // Note that DisplayableManagerGroup will take ownership of the object
    this->AddDisplayableManager(displayableManager);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableManagerGroup::SetAndObserveDisplayableManagerFactory(
    vtkDMMLDisplayableManagerFactory * factory)
{
  // Remove observers
  if (this->Internal->DisplayableManagerFactory)
    {
    this->Internal->DisplayableManagerFactory->RemoveObserver(this->Internal->CallBackCommand);
    this->Internal->DisplayableManagerFactory->Delete();
    this->Internal->DisplayableManagerFactory = nullptr;
    }

  this->Internal->DisplayableManagerFactory = factory;

  // Add observers
  if (this->Internal->DisplayableManagerFactory)
    {

    this->Internal->DisplayableManagerFactory->Register(this);

    // DisplayableManagerFactoryRegisteredEvent
    this->Internal->DisplayableManagerFactory->AddObserver(
        vtkDMMLDisplayableManagerFactory::DisplayableManagerFactoryRegisteredEvent,
        this->Internal->CallBackCommand);

    // DisplayableManagerFactoryUnRegisteredEvent
    this->Internal->DisplayableManagerFactory->AddObserver(
        vtkDMMLDisplayableManagerFactory::DisplayableManagerFactoryUnRegisteredEvent,
        this->Internal->CallBackCommand);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableManagerGroup::AddDisplayableManager(
    vtkDMMLAbstractDisplayableManager * displayableManager)
{
  // Sanity checks
  if (!displayableManager)
    {
    vtkWarningMacro(<<"AddDisplayableManager - displayableManager is NULL");
    return;
    }

  // Make sure the displayableManager has NOT already been added
  const char * displayableManagerClassName = displayableManager->GetClassName();
  if (this->GetDisplayableManagerByClassName(displayableManagerClassName) != nullptr)
    {
    vtkWarningMacro(<<"AddDisplayableManager - "
                    << displayableManager->GetClassName()
                    << " (" << displayableManager << ") already added");
    return;
    }

  displayableManager->SetDMMLDisplayableManagerGroup(this);
  if (this->Internal->DisplayableManagerFactory)
    {
    displayableManager->SetDMMLApplicationLogic(
      this->Internal->DisplayableManagerFactory->GetDMMLApplicationLogic());
    }
  displayableManager->SetRenderer(this->Internal->Renderer);

  // pass the lightbox manager proxy to the new displayable manager
  displayableManager->SetLightBoxRendererManagerProxy(this->Internal->LightBoxRendererManagerProxy);

  displayableManager->SetAndObserveDMMLDisplayableNode(this->GetDMMLDisplayableNode());

  displayableManager->Register(this);
  this->Internal->DisplayableManagers.push_back(displayableManager);
  this->Internal->NameToDisplayableManagerMap[displayableManagerClassName] = displayableManager;

  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "registering DisplayableManager: " << displayableManager << "("
                << displayableManager->GetClassName() << ")");
}

//----------------------------------------------------------------------------
int vtkDMMLDisplayableManagerGroup::GetDisplayableManagerCount()
{
  return static_cast<int>(this->Internal->DisplayableManagers.size());
}

//----------------------------------------------------------------------------
vtkDMMLAbstractDisplayableManager * vtkDMMLDisplayableManagerGroup::GetNthDisplayableManager(int n)
{
  int numManagers = this->GetDisplayableManagerCount();
  if (n < 0 || n >= numManagers)
    {
    return nullptr;
    }
  return this->Internal->DisplayableManagers[n];
}
//----------------------------------------------------------------------------
void vtkDMMLDisplayableManagerGroup::SetRenderer(vtkRenderer* newRenderer)
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

  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "initializing DisplayableManagerGroup using Renderer: " << newRenderer);

  // Loop though DisplayableManager and initialize
  for(size_t i = 0; i < this->Internal->DisplayableManagers.size(); ++i)
    {
    vtkDMMLAbstractDisplayableManager * displayableManager = this->Internal->DisplayableManagers[i];
    displayableManager->SetRenderer(newRenderer);
    }

  this->Modified();
}

//----------------------------------------------------------------------------
vtkRenderWindowInteractor* vtkDMMLDisplayableManagerGroup::GetInteractor()
{
  if (!this->Internal->Renderer || !this->Internal->Renderer->GetRenderWindow())
    {
    vtkDebugMacro(<< this->GetClassName() << " (" << this << "): returning Interactor address 0");
    return nullptr;
    }
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "returning Internal->Renderer->GetRenderWindow()->GetInteractor() address "
                << this->Internal->Renderer->GetRenderWindow()->GetInteractor() );
  return this->Internal->Renderer->GetRenderWindow()->GetInteractor();
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableManagerGroup::RequestRender()
{
  this->InvokeEvent(vtkCommand::UpdateEvent);
}

//----------------------------------------------------------------------------
vtkRenderer* vtkDMMLDisplayableManagerGroup::GetRenderer()
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "returning Internal->Renderer address " << this->Internal->Renderer );
  return this->Internal->Renderer;
}

//----------------------------------------------------------------------------
vtkDMMLNode* vtkDMMLDisplayableManagerGroup::GetDMMLDisplayableNode()
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "returning Internal->DMMLDisplayableNode address "
                << this->Internal->DMMLDisplayableNode );
  return this->Internal->DMMLDisplayableNode;
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableManagerGroup::SetDMMLDisplayableNode(
    vtkDMMLNode* newDMMLDisplayableNode)
{
  for(std::size_t i=0; i < this->Internal->DisplayableManagers.size(); ++i)
    {
    vtkDMMLAbstractDisplayableManager * displayableManager = this->Internal->DisplayableManagers[i];

    displayableManager->SetAndObserveDMMLDisplayableNode(newDMMLDisplayableNode);
    }
  vtkSetObjectBodyMacro(Internal->DMMLDisplayableNode, vtkDMMLNode, newDMMLDisplayableNode);
}

//----------------------------------------------------------------------------
vtkDMMLAbstractDisplayableManager*
    vtkDMMLDisplayableManagerGroup::GetDisplayableManagerByClassName(const char* className)
{
  if (!className)
    {
    vtkWarningMacro(<< "GetDisplayableManagerByClassName - className is NULL");
    return nullptr;
    }
  vtkInternal::NameToDisplayableManagerMapIt it =
      this->Internal->NameToDisplayableManagerMap.find(className);

  if (it == this->Internal->NameToDisplayableManagerMap.end())
    {
    return nullptr;
    }

  return it->second;
}

//-----------------------------------------------------------------------------
void vtkDMMLDisplayableManagerGroup::DoCallback(vtkObject* vtk_obj, unsigned long event,
                                                void* client_data, void* call_data)
{
  vtkDMMLDisplayableManagerGroup* self =
      reinterpret_cast<vtkDMMLDisplayableManagerGroup*>(client_data);
  char* displayableManagerName = reinterpret_cast<char*>(call_data);
  assert(self);
  assert(reinterpret_cast<vtkDMMLDisplayableManagerFactory*>(vtk_obj));
#ifndef _DEBUG
  (void)vtk_obj;
#endif
  assert(displayableManagerName);

  switch(event)
    {
    case vtkDMMLDisplayableManagerFactory::DisplayableManagerFactoryRegisteredEvent:
      self->onDisplayableManagerFactoryRegisteredEvent(displayableManagerName);
      break;
    case vtkDMMLDisplayableManagerFactory::DisplayableManagerFactoryUnRegisteredEvent:
      self->onDisplayableManagerFactoryUnRegisteredEvent(displayableManagerName);
      break;
    }
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableManagerGroup::onDisplayableManagerFactoryRegisteredEvent(
    const char* displayableManagerName)
{
  assert(displayableManagerName);

  vtkSmartPointer<vtkDMMLAbstractDisplayableManager> newDisplayableManager;
  newDisplayableManager.TakeReference(
    vtkDMMLDisplayableManagerGroup::InstantiateDisplayableManager(
      displayableManagerName));
  this->AddDisplayableManager(newDisplayableManager);
  vtkDebugMacro(<< "group:" << this << ", onDisplayableManagerFactoryRegisteredEvent:"
                << displayableManagerName);
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableManagerGroup::onDisplayableManagerFactoryUnRegisteredEvent(
    const char* displayableManagerName)
{
  assert(displayableManagerName);

  // Find the associated object
  vtkInternal::NameToDisplayableManagerMapIt it =
      this->Internal->NameToDisplayableManagerMap.find(displayableManagerName);

  // The DisplayableManager is expected to be in the map
  assert(it != this->Internal->NameToDisplayableManagerMap.end());

  vtkDMMLAbstractDisplayableManager * displayableManager = it->second;
  assert(displayableManager);

  // Find DisplayableManager in the vector
  vtkInternal::DisplayableManagersIt it2 = std::find(this->Internal->DisplayableManagers.begin(),
                                                     this->Internal->DisplayableManagers.end(),
                                                     displayableManager);

  // The DisplayableManager is expected to be in the vector
  assert(it2 != this->Internal->DisplayableManagers.end());

  // Remove it from the vector
  this->Internal->DisplayableManagers.erase(it2);

  // Clean memory
  displayableManager->Delete();

  // Remove it from the map
  this->Internal->NameToDisplayableManagerMap.erase(it);

  vtkDebugMacro(<< "group:" << this << ", onDisplayableManagerFactoryUnRegisteredEvent:"
                << displayableManagerName);
}

//---------------------------------------------------------------------------
void vtkDMMLDisplayableManagerGroup::SetLightBoxRendererManagerProxy(vtkDMMLLightBoxRendererManagerProxy* mgr)
{
  this->Internal->LightBoxRendererManagerProxy = mgr;

  for(size_t i=0; i < this->Internal->DisplayableManagers.size(); ++i)
    {
    this->Internal->DisplayableManagers[i]->SetLightBoxRendererManagerProxy(this->Internal->LightBoxRendererManagerProxy);
    }
}

//---------------------------------------------------------------------------
vtkDMMLLightBoxRendererManagerProxy* vtkDMMLDisplayableManagerGroup::GetLightBoxRendererManagerProxy()
{
  return this->Internal->LightBoxRendererManagerProxy;
}
