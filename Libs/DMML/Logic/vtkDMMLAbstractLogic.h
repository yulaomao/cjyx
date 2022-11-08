/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#ifndef __vtkDMMLAbstractLogic_h
#define __vtkDMMLAbstractLogic_h

// DMMLLogic includes
class vtkDMMLApplicationLogic;

// DMML includes
#include <vtkObserverManager.h>
class vtkDMMLNode;
class vtkDMMLScene;

// VTK includes
#include <vtkCommand.h>
#include <vtkObject.h>
class vtkIntArray;
class vtkFloatArray;

#include "vtkDMMLLogicExport.h"


//----------------------------------------------------------------------------
// Convenient macros

//----------------------------------------------------------------------------
// It removes all the event observations associated with the old value.
#ifndef vtkSetDMMLNodeMacro
#define vtkSetDMMLNodeMacro(node,value)  {                                    \
  vtkObject *_oldNode = (node);                                               \
  this->GetDMMLNodesObserverManager()->SetObject(                             \
    vtkObjectPointer(&(node)), (value));                                      \
  vtkObject *_newNode = (node);                                               \
  if (_oldNode != _newNode)                                                   \
    {                                                                         \
    this->Modified();                                                         \
    }                                                                         \
};
#endif

//----------------------------------------------------------------------------
#ifndef vtkSetAndObserveDMMLNodeMacro
/// \brief Set and observe a DMML node.
/// Replace the existing value of \a node with \a value. Unobserve the old node
/// and observe the ModifiedEvent of the new. When the new node is modified,
/// vtkDMMLAbstractLogic::ProcessDMMLNodesEvents is called which propagate the
/// call to vtkDMMLAbstractLogic::OnDMMLNodeModified(vtkDMMLNode*)
/// automatically.
/// \note Can't be used with objects other than vtkDMMLNodes
/// \code
/// void vtkDMMLMyLogic::SetMyNode(vtkDMMLNode* newNode)
/// {
///   vtkSetAndObserveDMMLNodeMacro(this->MyNode, newNode);
///   this->OnDMMLNodeModified(this->MyNode);
/// }
/// \endcode
/// \sa vtkDMMLAbstractLogic::ProcessDMMLNodesEvents(),
/// vtkDMMLAbstractLogic::OnDMMLNodeModified()
#define vtkSetAndObserveDMMLNodeMacro(node,value) {                           \
  vtkObject *_oldNode = (node);                                               \
  this->GetDMMLNodesObserverManager()->SetAndObserveObject(                   \
    vtkObjectPointer(&(node)), (value));                                      \
  vtkObject *_newNode = (node);                                               \
  if (_oldNode != _newNode)                                                   \
    {                                                                         \
    this->Modified();                                                         \
    }                                                                         \
};
#endif

//----------------------------------------------------------------------------
#ifndef vtkSetAndObserveDMMLNodeEventsMacro
#define vtkSetAndObserveDMMLNodeEventsMacro(node,value,events) {              \
  vtkObject *_oldNode = (node);                                               \
  this->GetDMMLNodesObserverManager()->SetAndObserveObjectEvents(             \
     vtkObjectPointer(&(node)), (value), (events));                           \
  vtkObject *_newNode = (node);                                               \
  if (_oldNode != _newNode)                                                   \
    {                                                                         \
    this->Modified();                                                         \
    }                                                                         \
};
#endif

#ifndef vtkObserveDMMLNodeMacro
#define vtkObserveDMMLNodeMacro(node)                                         \
{                                                                             \
  this->GetDMMLNodesObserverManager()->ObserveObject( (node) );               \
};
#endif


#ifndef vtkObserveDMMLNodeEventsMacro
#define vtkObserveDMMLNodeEventsMacro(node, events)                           \
{                                                                             \
  this->GetDMMLNodesObserverManager()->AddObjectEvents ( (node), (events) );  \
};
#endif

#ifndef vtkUnObserveDMMLNodeMacro
#define vtkUnObserveDMMLNodeMacro(node)                                       \
{                                                                             \
  this->GetDMMLNodesObserverManager()->RemoveObjectEvents ( (node) );         \
};
#endif

#ifndef vtkIsObservedDMMLNodeEventMacro
#define vtkIsObservedDMMLNodeEventMacro(node, event)                          \
  (                                                                           \
  this->GetDMMLNodesObserverManager()->GetObservationsCount(node, event) != 0 \
  )
#endif

/// \brief Superclass for DMML logic classes.
///
/// Superclass for all DMML logic classes.
/// When a scene is set, SetDMMLScene(vtkDMMLScene*),
/// - UnobserveDMMLScene() is called if a scene was previously set,
/// - SetDMMLSceneInternal() is called to observe the scene events
/// (e.g. StartImportEvent, EndBatchProcessEvent...)
/// - ObserveDMMLScene() is called to initialize the scene from the logic
/// - UpdateDMMLScene() is called to initialize the logic from the scene
/// Later, when events are fired by the scene, corresponding methods
/// (e.g. OnDMMLSceneNodeAdded, OnDMMLEndBatchProcess...) are called in the
/// logic if the events have been previously observed in SetDMMLSceneInternal()
class VTK_DMML_LOGIC_EXPORT vtkDMMLAbstractLogic : public vtkObject
{
public:
  /// Typedef for member functions of DMMLLogic that can be used as
  /// scheduled tasks.
  typedef void (vtkDMMLAbstractLogic::*TaskFunctionPointer)(void *clientdata);

  static vtkDMMLAbstractLogic *New();
  void PrintSelf(ostream& os, vtkIndent indent) override;
  vtkTypeMacro(vtkDMMLAbstractLogic, vtkObject);

  /// Get access to overall application state
  virtual vtkDMMLApplicationLogic* GetDMMLApplicationLogic()const;
  virtual void SetDMMLApplicationLogic(vtkDMMLApplicationLogic* logic);

  /// Return a reference to the current DMML scene
  vtkDMMLScene * GetDMMLScene()const;

  /// Set and observe the DMMLScene
  void SetDMMLScene(vtkDMMLScene * newScene);

  /// @cond
  /// \deprecated Still here for EMSegment
  /// Set and observe DMML Scene. In order to provide a single method to set
  /// the scene, consider overloading SetDMMLSceneInternal().
  /// \note After each module are ported to Qt, these methods will be removed.
  ///  Use SetDMMLScene() instead.
  /// \sa SetDMMLSceneInternal()
  /// \sa SetAndObserveDMMLSceneInternal() SetAndObserveDMMLSceneEventsInternal()
  void SetAndObserveDMMLScene(vtkDMMLScene * newScene);
  void SetAndObserveDMMLSceneEvents(vtkDMMLScene * newScene, vtkIntArray * events, vtkFloatArray* priorities=nullptr);
  /// @endcond

protected:

  vtkDMMLAbstractLogic();
  ~vtkDMMLAbstractLogic() override;

  /// Receives all the events fired by the scene.
  /// By default, it calls OnDMMLScene*Event based on the event passed.
  virtual void ProcessDMMLSceneEvents(vtkObject* caller,
                                      unsigned long event,
                                      void * callData);

  /// Receives all the events fired by the nodes.
  /// To listen to a node, you can add an observer using
  /// GetDMMLNodesCallbackCommand() or use the utility macros
  /// vtk[SetAndObserve|Observe]DMMLNode[Event]Macro
  /// ProcessDMMLNodesEvents calls OnDMMLNodeModified when event is
  /// vtkCommand::ModifiedEvent.
  /// \sa ProcessDMMLSceneEvents, ProcessDMMLLogicsEvents,
  /// OnDMMLNodeModified(), vtkSetAndObserveDMMLNodeMacro,
  /// vtkObserveDMMLNodeMacro, vtkSetAndObserveDMMLNodeEventMacro
  virtual void ProcessDMMLNodesEvents(vtkObject* caller,
                                      unsigned long event,
                                      void * callData);

  /// Receives all the events fired by the logics.
  /// To listen to a logic, you can add an observer using
  /// GetDMMLLogicsCallbackCommand().
  /// To be reimplemented in subclasses if needed.
  /// \sa GetDMMLLogicsCallbackCommand() ,ProcessDMMLSceneEvents(),
  /// ProcessDMMLNodesEvents()
  virtual void ProcessDMMLLogicsEvents(vtkObject* caller,
                                      unsigned long event,
                                      void * callData);

  /// Get DMML scene callbackCommand.
  /// You shouldn't have to use it manually, reimplementing
  /// SetDMMLSceneInternal and setting the events to listen should be enough.
  /// \sa SetDMMLSceneInternal()
  vtkCallbackCommand * GetDMMLSceneCallbackCommand();

  /// Get the DMML nodes callbackCommand. The Execute function associated
  /// the the callback calls ProcessDMMLNodesEvents.
  /// Only vtkDMMLNodes can be listened to.
  /// \sa ProcessDMMLNodesEvents()
  vtkCallbackCommand * GetDMMLNodesCallbackCommand();

  /// Get the DMML Logic callback command.
  /// \sa GetDMMLSceneCallbackCommand(), GetDMMLNodesCallbackCommand()
  vtkCallbackCommand * GetDMMLLogicsCallbackCommand();

  /// Get DMML scene observerManager. It points to the scene callback.
  /// \sa GetDMMLSceneCallbackCommand()
  vtkObserverManager * GetDMMLSceneObserverManager()const;

  /// Get DMML nodes observerManager. It points to the nodes callback.
  /// \sa GetDMMLNodesCallbackCommand()
  vtkObserverManager * GetDMMLNodesObserverManager()const;

  /// Get DMML logics observerManager. It points to the logics callback.
  /// \sa GetDMMLLogicsCallbackCommand()
  vtkObserverManager * GetDMMLLogicsObserverManager()const;

  /// Return the event id currently processed or 0 if any.
  int GetProcessingDMMLSceneEvent()const;

  /// Called anytime a scene is not set to the logic anymore (e.g. a new or
  /// no scene is set)
  /// Reimplement the method to delete all the scene specific information
  /// such as a node IDs, pointers...
  /// \sa SetDMMLSceneInternal, ObserveDMMLScene, UpdateFromDMMLScene
  virtual void UnobserveDMMLScene();
  /// Called after a scene is set to the logic and nodes are registered
  /// (RegisterNodes()).
  /// The scene events to observe are already set in SetDMMLSceneInternal().
  /// By default, ObserveDMMLScene() calls UpdateFromDMMLScene().
  /// Override for a custom behavior.
  /// \sa SetDMMLSceneInternal, RegisterNodes, UnobserveDMMLScene
  /// \sa UpdateFromDMMLScene
  virtual void ObserveDMMLScene();
  /// Called every time the scene has been significantly changed.
  /// If the scene BatchProcessState events are observed (in
  /// SetDMMLSceneInternal() ), UpdateFromDMMLScene is called after each
  /// batch process (Close, Import, Restore...). It is also being called by
  /// default when a new scene is set (SetDMMLScene).
  /// \sa SetDMMLSceneInternal, UnobserveDMMLScene, ObserveDMMLScene
  virtual void UpdateFromDMMLScene();

  /// If vtkDMMLScene::StartBatchProcessEvent has been set to be observed in
  ///  SetDMMLSceneInternal, it is called when the scene fires the event
  /// \sa ProcessDMMLSceneEvents, SetDMMLSceneInternal
  /// \sa OnDMMLSceneEndBatchProcess
  virtual void OnDMMLSceneStartBatchProcess(){}
  /// If vtkDMMLScene::EndBatchProcessEvent has been set to be observed in
  ///  SetDMMLSceneInternal, it is called when the scene fires the event
  /// Internally calls UpdateFromDMMLScene.
  /// Can be reimplemented to change the default behavior.
  /// \sa ProcessDMMLSceneEvents, SetDMMLSceneInternal
  /// \sa OnDMMLSceneStartBatchProcess
  virtual void OnDMMLSceneEndBatchProcess();
  /// If vtkDMMLScene::StartCloseEvent has been set to be observed in
  ///  SetDMMLSceneInternal, it is called when the scene fires the event
  /// \sa ProcessDMMLSceneEvents, SetDMMLSceneInternal
  /// \sa OnDMMLSceneEndClose
  virtual void OnDMMLSceneStartClose(){}
  /// If vtkDMMLScene::EndCloseEvent has been set to be observed in
  ///  SetDMMLSceneInternal, it is called when the scene fires the event
  /// \sa ProcessDMMLSceneEvents, SetDMMLSceneInternal
  /// \sa OnDMMLSceneStartClose
  virtual void OnDMMLSceneEndClose(){}
  /// If vtkDMMLScene::StartImportEvent has been set to be observed in
  ///  SetDMMLSceneInternal, it is called when the scene fires the event
  /// \sa ProcessDMMLSceneEvents, SetDMMLSceneInternal
  /// \sa OnDMMLSceneEndImport, OnDMMLSceneNew
  virtual void OnDMMLSceneStartImport(){}
  /// If vtkDMMLScene::EndImportEvent has been set to be observed in
  ///  SetDMMLSceneInternal, it is called when the scene fires the event
  /// \sa ProcessDMMLSceneEvents, SetDMMLSceneInternal
  /// \sa OnDMMLSceneStartImport, OnDMMLSceneNew
  virtual void OnDMMLSceneEndImport(){}
  /// If vtkDMMLScene::StartRestoreEvent has been set to be observed in
  ///  SetDMMLSceneInternal, it is called when the scene fires the event
  /// \sa ProcessDMMLSceneEvents, SetDMMLSceneInternal
  /// \sa OnDMMLSceneEndRestore
  virtual void OnDMMLSceneStartRestore(){}
  /// If vtkDMMLScene::EndRestoreEvent has been set to be observed in
  ///  SetDMMLSceneInternal, it is called when the scene fires the event
  /// \sa ProcessDMMLSceneEvents, SetDMMLSceneInternal
  /// \sa OnDMMLSceneStartRestore
  virtual void OnDMMLSceneEndRestore(){}
  /// If vtkDMMLScene::SceneNewEvent has been set to be observed in
  ///  SetDMMLSceneInternal, it is called when the scene fires the event
  /// \sa ProcessDMMLSceneEvents, SetDMMLSceneInternal
  /// \sa OnDMMLSceneStartImport, OnDMMLSceneEndImport
  virtual void OnDMMLSceneNew(){}
  /// If vtkDMMLScene::NodeAddedEvent has been set to be observed in
  ///  SetDMMLSceneInternal, it is called when the scene fires the event
  /// \sa ProcessDMMLSceneEvents, SetDMMLSceneInternal
  /// \sa OnDMMLSceneNodeRemoved, vtkDMMLScene::NodeAboutToBeAdded
  virtual void OnDMMLSceneNodeAdded(vtkDMMLNode* /*node*/){}
  /// If vtkDMMLScene::NodeRemovedEvent has been set to be observed in
  ///  SetDMMLSceneInternal, it is called when the scene fires the event
  /// \sa ProcessDMMLSceneEvents, SetDMMLSceneInternal
  /// \sa OnDMMLSceneNodeAdded, vtkDMMLScene::NodeAboutToBeRemoved
  virtual void OnDMMLSceneNodeRemoved(vtkDMMLNode* /*node*/){}

  /// Called after the corresponding DMML event is triggered.
  /// \sa ProcessDMMLNodesEvents
  virtual void OnDMMLNodeModified(vtkDMMLNode* /*node*/){}

  /// Called each time a new scene is set. Can be reimplemented in derivated classes.
  /// Doesn't observe the scene by default, that means that
  /// UpdateFromDMMLScene() won't be called by default when a scene is imported,
  /// closed or restored, only when a new scene is set.
  /// \sa SetAndObserveDMMLSceneInternal() SetAndObserveDMMLSceneEventsInternal()
  /// \sa UpdateFromDMMLScene()
  virtual void SetDMMLSceneInternal(vtkDMMLScene* newScene);

  /// @cond
  /// Convenient method to set and observe the scene.
  /// \deprecated The ModifiedEvent on the scene is deprecated.
  void SetAndObserveDMMLSceneInternal(vtkDMMLScene *newScene);
  /// @endcond

  /// Typically called by a subclass in the derived SetDMMLSceneInternal to
  /// observe specific node events.
  /// \code
  /// void vtkDMMLMyLogic::SetDMMLSceneInternal(vtkDMMLScene* newScene)
  /// {
  ///   vtkNew<vtkIntArray> events;
  ///   events->InsertNextValue(vtkDMMLScene::NodeAddedEvent);
  ///   events->InsertNextValue(vtkDMMLScene::NodeRemovedEvent);
  ///   this->SetAndObserveDMMLSceneEventsInternal(newScene, events);
  /// }
  /// \endcode
  /// \sa SetDMMLSceneInternal()
  void SetAndObserveDMMLSceneEventsInternal(vtkDMMLScene *newScene,
                                            vtkIntArray *events,
                                            vtkFloatArray *priorities=nullptr);

  /// Register node classes into the DMML scene. Called each time a new scene
  /// is set. Do nothing by default. Can be reimplemented in derivated classes.
  virtual void RegisterNodes(){}

  /// Set DMMLSceneCallback flag
  /// True means ProcessDMMLEvent has already been called
  /// In DMMLSceneCallback, loop are avoided by checking the value of the flag
  /// \sa EnterDMMLSceneCallback()
  void SetInDMMLSceneCallbackFlag(int flag);

  /// Return 0 when not processing a DMML scene event, >0 otherwise.
  /// Values can be higher than 1 when receiving nested event:
  /// processing a DMML scene event fires other scene events.
  /// \sa SetInDMMLCallbackFlag()
  int GetInDMMLSceneCallbackFlag()const;

  /// Return true if the DMML callback must be executed, false otherwise.
  /// By default, it returns true, you can reimplement it in subclasses
  virtual bool EnterDMMLSceneCallback()const;

  /// Set event id currently processed or 0 if any.
  /// \sa EnterDMMLSceneCallback()
  void SetProcessingDMMLSceneEvent(int event);

  /// Set InDMMLNodesCallback flag.
  /// In InDMMLNodesCallback, loop are avoided by checking the value of the
  /// flag.
  /// \sa EnterDMMLNodesCallback()
  void SetInDMMLNodesCallbackFlag(int flag);

  /// Return 0 when not processing any DMML node event, >0 otherwise.
  /// Values can be higher than 1 when receiving nested events:
  /// processing a DMML node event fires other node events.
  /// \sa SetDMMLNodesCallbackFlag()
  int GetInDMMLNodesCallbackFlag()const;

  /// Return true if the DMML Nodes callback must be executed, false otherwise.
  /// By default, it returns true, you can reimplement it in subclasses.
  /// \sa SetInDMMLNodesCallbackFlag()
  virtual bool EnterDMMLNodesCallback()const;

  /// Set InDMMLLogicsCallback flag.
  /// In InDMMLLogicsCallback, loop are avoided by checking the value of the
  /// flag.
  /// \sa EnterDMMLLogicsCallback()
  void SetInDMMLLogicsCallbackFlag(int flag);

  /// Return 0 when not processing any DMML logic event, >0 otherwise.
  /// Values can be higher than 1 when receiving nested events:
  /// processing a DMML logic event fires other node events.
  /// \sa SetDMMLLogicsCallbackFlag()
  int GetInDMMLLogicsCallbackFlag()const;

  /// Return true if the Logics callback must be executed, false otherwise.
  /// By default, it returns true, you can reimplement it in subclasses
  virtual bool EnterDMMLLogicsCallback()const;

  /// DMMLSceneCallback is a static function to relay modified events from the DMML Scene
  /// In subclass, DMMLSceneCallback can also be used to relay event from observe DMML node(s)
  static void DMMLSceneCallback(vtkObject *caller, unsigned long eid, void *clientData, void *callData);

  /// DMMLNodesCallback is a static function to relay modified events from the nodes
  static void DMMLNodesCallback(vtkObject *caller, unsigned long eid, void *clientData, void *callData);

  /// DMMLLogicCallback is a static function to relay modified events from the logics
  static void DMMLLogicsCallback(vtkObject *caller, unsigned long eid, void *clientData, void *callData);

  /// Start modifying the logic. Disable Modify events.
  /// Returns the previous state of DisableModifiedEvent flag
  /// that should be passed to EndModify() method
  inline bool StartModify() ;

  /// End modifying the node. Enable Modify events if the
  /// previous state of DisableModifiedEvent flag is 0.
  /// Return the number of pending ModifiedEvent;
  inline int EndModify(bool wasModifying);

  bool GetDisableModifiedEvent()const;
  void SetDisableModifiedEvent(bool onOff);

  /// overrides the vtkObject method so that all changes to the node which would normally
  /// generate a ModifiedEvent can be grouped into an 'atomic' operation.  Typical usage
  /// would be to disable modified events, call a series of Set* operations, and then re-enable
  /// modified events and call InvokePendingModifiedEvent to invoke the event (if any of the Set*
  /// calls actually changed the values of the instance variables).
  void Modified() override;

  /// Invokes any modified events that are 'pending', meaning they were generated
  /// while the DisableModifiedEvent flag was nonzero.
  int InvokePendingModifiedEvent();

  int GetPendingModifiedEventCount()const;

private:

  vtkDMMLAbstractLogic(const vtkDMMLAbstractLogic&) = delete;
  void operator=(const vtkDMMLAbstractLogic&) = delete;

  class vtkInternal;
  vtkInternal * Internal;

};

//---------------------------------------------------------------------------
bool vtkDMMLAbstractLogic::StartModify()
{
  bool disabledModify = this->GetDisableModifiedEvent();
  this->SetDisableModifiedEvent(true);
  return disabledModify;
}

//---------------------------------------------------------------------------
int vtkDMMLAbstractLogic::EndModify(bool previousDisableModifiedEventState)
{
  this->SetDisableModifiedEvent(previousDisableModifiedEventState);
  if (!previousDisableModifiedEventState)
    {
    return this->InvokePendingModifiedEvent();
    }
  return this->GetPendingModifiedEventCount();
}

#endif
