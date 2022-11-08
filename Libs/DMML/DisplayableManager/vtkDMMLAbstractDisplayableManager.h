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

#ifndef __vtkDMMLAbstractDisplayableManager_h
#define __vtkDMMLAbstractDisplayableManager_h

// DMMLLogic includes
#include "vtkDMMLAbstractLogic.h"

#include "vtkDMMLDisplayableManagerExport.h"

class vtkDMMLInteractionEventData;
class vtkDMMLInteractionNode;
class vtkDMMLSelectionNode;
class vtkDMMLDisplayableManagerGroup;
class vtkDMMLNode;
class vtkDMMLScene;
class vtkDMMLLightBoxRendererManagerProxy;

class vtkRenderer;
class vtkRenderWindowInteractor;

/// \brief Superclass for displayable manager classes.
///
/// A displayable manager is responsible for representing DMML display nodes
/// into a renderer (e.g. 2D view renderer). Displayable managers are the interface
/// between DMML display nodes and vtkRenderer/vtkActors. They are responsible
/// for creating and synchronizing vtkActor, vtkMapper and vtkProperties with
/// DMML display nodes.
class VTK_DMML_DISPLAYABLEMANAGER_EXPORT vtkDMMLAbstractDisplayableManager
  : public vtkDMMLAbstractLogic
{
public:
  static vtkDMMLAbstractDisplayableManager *New();
  void PrintSelf(ostream& os, vtkIndent indent) override;
  vtkTypeMacro(vtkDMMLAbstractDisplayableManager, vtkDMMLAbstractLogic);

  /// Return True if Create() method has been invoked
  /// \sa CreateIfPossible() Create()
  bool IsCreated();

  /// Set the LightBoxRendererManager proxy. This proxy provides a
  /// method GetRenderer(int) that returns the renderer for the Nth
  /// lightbox pane. The DisplayableManagers use this method to map
  /// coordinates to the proper lightbox pane, e.g. in placing
  /// crosshairs or annotations in the proper renderer.
  virtual void SetLightBoxRendererManagerProxy(vtkDMMLLightBoxRendererManagerProxy *);

  /// Get the LightBoxRendererManagerProxy if one has been provided
  /// \sa SetLightBoxRendererManagerProxy(vtkDMMLLightBoxRendererManagerProxy *)
  virtual vtkDMMLLightBoxRendererManagerProxy* GetLightBoxRendererManagerProxy();

  /// Get the default renderer for this displayable manager.
  vtkRenderer* GetRenderer();

  /// Get the renderer for the Nth lightbox pane. This method uses the
  /// LightBoxRendererManagerProxy if one has been configured for the
  /// DisplayableManager. If no LightBoxRendererManagerProxy has been
  /// set, this method returns the default renderer by deferring to
  /// GetRenderer(),
  vtkRenderer* GetRenderer(int idx);

  /// Convenient method to get the WindowInteractor associated with the Renderer
  vtkRenderWindowInteractor* GetInteractor();

  /// Convenient method to get the current InteractionNode
  vtkDMMLInteractionNode* GetInteractionNode();

  /// Convenient method to get the current SelectionNode
  vtkDMMLSelectionNode* GetSelectionNode();

  /// Assemble and return info string to display in Data probe for a given viewer XYZ position.
  /// \return Invalid string by default, meaning no information to display.
  virtual std::string GetDataProbeInfoStringForPosition(
      double vtkNotUsed(xyz)[3]) { return ""; }

  /// Return true if the displayable manager can process the event.
  /// Distance2 is the squared distance in display coordinates from the closest interaction position.
  /// The displayable manager with the closest distance will get the chance to process the interaction event.
  virtual bool CanProcessInteractionEvent(vtkDMMLInteractionEventData* eventData, double &distance2);

  /// Process an interaction event.
  /// Returns true if the event should be aborted (not processed any further by other event observers).
  virtual bool ProcessInteractionEvent(vtkDMMLInteractionEventData* eventData);

  /// Set if the widget gets/loses focus (interaction events are processed by this displayable manager).
  virtual void SetHasFocus(bool hasFocus, vtkDMMLInteractionEventData* eventData);

  /// Displayable manager can indicate that it would like to get all events (even when mouse pointer is outside the window).
  virtual bool GetGrabFocus();

  /// Displayable manager can indicate that the window is in interactive mode (faster updates).
  virtual bool GetInteractive();

  /// Displayable manager returns ID of the mouse cursor shape that should be displayed
  virtual int GetMouseCursor();

  void SetMouseCursor(int cursor);

protected:

  vtkDMMLAbstractDisplayableManager();
  ~vtkDMMLAbstractDisplayableManager() override;

  /// Get DMML Displayable Node
  vtkDMMLNode * GetDMMLDisplayableNode();

  /// Access to SetRenderer, SetDMMLDisplayableNode and CreateIfPossible methods
  friend class vtkDMMLDisplayableManagerGroup;

  virtual void SetDMMLDisplayableManagerGroup(vtkDMMLDisplayableManagerGroup* group);
  virtual void SetRenderer(vtkRenderer* newRenderer);

  /// Sub-class could overload that function and perform additional initialization steps
  /// \note Called by SetRenderer()
  /// \note Initialization occurs before the DMMLDisplayableNode is set and observed
  /// \warning That function should NOT be used directly !
  /// \sa SetRenderer
  virtual void AdditionalInitializeStep(){}

  /// Subclass can overload this method to specify under which InteractionNode modes
  /// this Displayable Manager InteractorStyle events.
  /// By default events only arrive when in Place mode (good for annotations)
  /// but if you want a continuous read out of, for example, slice positions while
  /// the mouse moves set this to include Place and ViewTransform
  virtual int ActiveInteractionModes();

  void ProcessDMMLNodesEvents(vtkObject* caller,
                                      unsigned long event,
                                      void * callData) override;

  /// Receives all the events fired by any graphical object interacted by the
  /// user (typically vtk widgets).
  /// A typical use case is to listen to dmml nodes (using
  /// GetDMMLNodesCallbackCommand()) and update the graphical
  /// objects like mappers, actors, widgets... in ProcessDMMLNodesEvent, and to
  /// listen to user interactions (using (using
  /// GetWidgetsCallbackCommand()) like widgets and update the dmml nodes in
  /// ProcessWidgetsEvents.
  /// To listen to a widget (or any vtk Object), you can add an observer using
  /// GetWidgetsCallbackCommand().
  /// ProcessWidgetsEvents doesn't do anything by default, you need to reimplement
  /// it.
  virtual void ProcessWidgetsEvents(vtkObject* caller,
                                    unsigned long event,
                                    void * callData);

  /// WidgetsCallback is a static function to relay modified events from the vtk widgets
  static void WidgetsCallback(vtkObject *caller, unsigned long eid,
                              void *clientData, void *callData);

  /// Get vtkWidget callbackCommand
  vtkCallbackCommand * GetWidgetsCallbackCommand();

  /// Get widget observerManager
  vtkObserverManager * GetWidgetsObserverManager()const;

  /// Called by SetDMMLScene - Used to initialize the Scene
  /// Observe all the events of the scene and call OnDMMLSceneEndClose()
  /// or OnDMMLSceneEndImport() if the new scene is valid
  void SetDMMLSceneInternal(vtkDMMLScene* newScene) override;

  /// ProcessDMMLNodesEvents calls OnDMMLDisplayableNodeModifiedEvent when the
  /// displayable node (e.g. vtkDMMLSliceNode, vtkDMMLViewNode) is Modified.
  /// Could be overloaded in DisplayableManager subclass.
  virtual void OnDMMLDisplayableNodeModifiedEvent(vtkObject* caller);

  /// \brief Allow to specify additional events that the DisplayableNode will observe
  /// \warning Should be called within AdditionalInitializeStep() method
  /// \sa AdditionalInitializeStep()
  void AddDMMLDisplayableManagerEvent(int eventId);

  /// Set DMML DisplayableNode
  /// Called by vtkDMMLDisplayableManagerFactory
  void SetAndObserveDMMLDisplayableNode(vtkDMMLNode * newDMMLDisplayableNode);

  /// Get associated DisplayableManager group
  vtkDMMLDisplayableManagerGroup * GetDMMLDisplayableManagerGroup();

  /// Invoke Create() and set Created flag to True
  /// A no-op if IsCreated() return True
  void CreateIfPossible();

  /// Called after a valid DMML DisplayableNode is set.
  /// By default it simulates a ModifiedEvent event on the displayable node
  /// so that ProcessDMMLNodesEvents(displayableNode, ModifiedEvent) is called.
  /// \note GetRenderer() and GetDMMLDisplayableNode() will return valid object
  virtual void Create();

  /// Remove DMML observers
  virtual void RemoveDMMLObservers();

  /// Specify if UodateFromDMML() should be called
  /// \sa UpdateFromDMML()
  void SetUpdateFromDMMLRequested(bool requested);

  /// Called from RequestRender method if UpdateFromDMMLRequested is true
  /// \sa RequestRender() SetUpdateFromDMMLRequested()
  virtual void UpdateFromDMML(){}

  /// Invoke vtkCommand::UpdateEvent and then call
  /// vtkDMMLThreeDViewDisplayableManagerFactory::RequestRender() which will also
  /// invoke vtkCommand::UpdateEvent.
  /// An observer can then listen for that event and "compress" the different Render requests
  /// to efficiently call RenderWindow->Render()
  void RequestRender();

  /// Usually used inside AdditionalInitializeStep()
  /// Allows to add observer to the current interactor style that will call the
  /// virtual method OnInteractorStyleEvent accordingly.
  /// \sa AdditionalInitializeStep RemoveInteractorStyleObservableEvent
  void AddInteractorStyleObservableEvent(int eventid, float priority=0.0);

  /// \sa AddInteractorStyleObservableEvent
  void RemoveInteractorStyleObservableEvent(int eventid);

  /// Usually used inside AdditionalInitializeStep()
  /// Allows to add observer to the current interactor that will call the
  /// virtual method OnInteractorEvent accordingly.
  /// \sa AdditionalInitializeStep RemoveInteractorObservableEvent
  void AddInteractorObservableEvent(int eventid, float priority=0.0);

  /// \sa AddInteractorObservableEvent
  void RemoveInteractorObservableEvent(int eventid);

  /// Called after interactor style event specified using AddInteractorStyleObservableEvent
  /// are invoked.
  /// \note The following events are observed by default:
  /// <ul>
  ///   <li>vtkCommand::LeftButtonPressEvent</li>
  ///   <li>vtkCommand::LeftButtonReleaseEvent</li>
  ///   <li>vtkCommand::RightButtonPressEvent</li>
  ///   <li>vtkCommand::RightButtonReleaseEvent</li>
  ///   <li>vtkCommand::MiddleButtonPressEvent</li>
  ///   <li>vtkCommand::MiddleButtonReleaseEvent</li>
  ///   <li>vtkCommand::MouseWheelBackwardEvent</li>
  ///   <li>vtkCommand::MouseWheelForwardEvent</li>
  ///   <li>vtkCommand::EnterEvent</li>
  ///   <li>vtkCommand::LeaveEvent</li>
  /// </ul>
  /// \sa AddInteractorStyleObservableEvent RemoveInteractorStyleObservableEvent
  virtual void OnInteractorStyleEvent(int eventid);

  /// Called after interactor event specified using
  /// AddInteractorObservableEvent are invoked
  /// \sa AddInteractorObservableEvent RemoveInteractorObservableEvent
  virtual void OnInteractorEvent(int eventid);

  /// Set the Abort flag on the InteractorStyle event callback
  void SetInteractorStyleAbortFlag(int f);
  int GetInteractorStyleAbortFlag();
  void InteractorStyleAbortFlagOn();
  void InteractorStyleAbortFlagOff();

  /// Set the Abort flag on the Interactor event callback
  void SetInteractorAbortFlag(int f);
  int GetInteractorAbortFlag();
  void InteractorAbortFlagOn();
  void InteractorAbortFlagOff();

private:

  vtkDMMLAbstractDisplayableManager(const vtkDMMLAbstractDisplayableManager&) = delete;
  void operator=(const vtkDMMLAbstractDisplayableManager&) = delete;

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal; // For access from the callback function
};

#endif
