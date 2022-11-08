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

#ifndef __vtkDMMLDisplayableManagerGroup_h
#define __vtkDMMLDisplayableManagerGroup_h

// VTK includes
#include <vtkObject.h>

#include "vtkDMMLDisplayableManagerExport.h"

class vtkDMMLDisplayableManagerFactory;
class vtkDMMLAbstractDisplayableManager;
class vtkDMMLLightBoxRendererManagerProxy;
class vtkDMMLNode;
class vtkRenderer;
class vtkRenderWindowInteractor;

/// \brief DisplayableManagerGroup is a collection of DisplayableManager.
///
/// It also provides method allowing to either call RenderRequest
/// or SetAndObserveDMMLDisplayableNode on all member of the group.
/// When the displayable managers in the group request the view to be
/// refreshed, the group fires a vtkCommand::UpdateEvent event.
/// This event can be observed and trigger a Render on the render window.
class VTK_DMML_DISPLAYABLEMANAGER_EXPORT vtkDMMLDisplayableManagerGroup : public vtkObject
{
public:

  static vtkDMMLDisplayableManagerGroup *New();
  vtkTypeMacro(vtkDMMLDisplayableManagerGroup,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Convenient method equivalent to call SetAndObserveDisplayableManagerFactory, SetRenderer,
  /// then instantiate and add all displayable managers registered within the \a factory.
  /// \sa SetAndObserveDisplayableManagerFactory SetRenderer
  /// \sa AddDisplayableManager InstantiateDisplayableManager
  void Initialize(vtkDMMLDisplayableManagerFactory * factory, vtkRenderer * renderer);

  /// Set and observe DisplayableManager factory
  void SetAndObserveDisplayableManagerFactory(vtkDMMLDisplayableManagerFactory * factory);

  /// Add a DisplayableManager and initialize it if required
  void AddDisplayableManager(vtkDMMLAbstractDisplayableManager * displayableManager);

  /// Return the number of DisplayableManager already added to the group
  int GetDisplayableManagerCount();

  vtkDMMLAbstractDisplayableManager *GetNthDisplayableManager(int n);

  /// Return a DisplayableManager given its class name
  vtkDMMLAbstractDisplayableManager*
      GetDisplayableManagerByClassName(const char* className);

  /// Set Renderer and Interactor
  /// No-op if already initialized.
  /// \sa IsInitialized
  void SetRenderer(vtkRenderer* newRenderer);

  /// Convenient method to get the WindowInteractor associated with the Renderer
  vtkRenderWindowInteractor* GetInteractor();

  /// Invoke vtkCommand::UpdateEvent
  /// An observer can then listen for that event and "compress" the different Render requests
  /// to efficiently call RenderWindow->Render()
  /// \sa vtkDMMLAbstractDisplayableManager::RequestRender()
  void RequestRender();

  /// Get Renderer
  vtkRenderer* GetRenderer();

  /// Set / Get DMML Displayable Node
  vtkDMMLNode* GetDMMLDisplayableNode();
  void SetDMMLDisplayableNode(vtkDMMLNode* newDMMLDisplayableNode);

  /// Returns true if the displayableManagerName corresponds to a valid
  /// displayable manager, false otherwise.
  /// To be valid, ad displayableManagerName must be the name of a VTK
  /// class that derives from vtkDMMLAbstractDisplayableManager or be a valid
  /// python script file name.
  static bool IsADisplayableManager(const char* displayableManagerName);

  /// Returns a new instance of a displayable manager defined by its name:
  /// VTK class name or python file name.
  /// You are responsible of the returned pointer.
  static vtkDMMLAbstractDisplayableManager* InstantiateDisplayableManager(
    const char* displayableManagerName);


  /// Set the LightBoxRendererManagerProxy on the
  /// DisplayableManagerGroup. This caches the proxy and broadcasts
  /// the proxy to all DisplayableManagers in the group. This proxy
  /// provides a method GetRenderer(int) that returns the renderer for
  /// the Nth lightbox pane. The DisplayableManagers use this method
  /// to map coordinates to the proper lightbox pane, e.g. in placing
  /// crosshairs or annotations in the proper renderer.
  virtual void SetLightBoxRendererManagerProxy(vtkDMMLLightBoxRendererManagerProxy *);

  /// Get the LightBoxRendererManagerProxy if one has been provided.
  /// \sa SetLightBoxRendererManagerProxy(vtkDMMLLightBoxRendererManagerProxy *)
  virtual vtkDMMLLightBoxRendererManagerProxy* GetLightBoxRendererManagerProxy();

protected:

  vtkDMMLDisplayableManagerGroup();
  ~vtkDMMLDisplayableManagerGroup() override;

  typedef vtkDMMLDisplayableManagerGroup Self;
  static void DoCallback(vtkObject* vtk_obj, unsigned long event,
                         void* client_data, void* call_data);
  /// Trigger upon a DisplayableManager is either registered or unregistered from
  /// the associated factory
  void onDisplayableManagerFactoryRegisteredEvent(const char* displayableManagerName);
  void onDisplayableManagerFactoryUnRegisteredEvent(const char* displayableManagerName);

  class vtkInternal;
  vtkInternal* Internal;

private:

  vtkDMMLDisplayableManagerGroup(const vtkDMMLDisplayableManagerGroup&) = delete;
  void operator=(const vtkDMMLDisplayableManagerGroup&) = delete;

};

#endif
