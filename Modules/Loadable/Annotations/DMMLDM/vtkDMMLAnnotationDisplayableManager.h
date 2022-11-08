/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Cjyx

 Module:    $RCSfile: vtkDMMLAnnotationDisplayableManager.h,v $
 Date:      $Date: 2010/07/26 04:48:05 $
 Version:   $Revision: 1.2 $

 =========================================================================auto=*/

#ifndef __vtkDMMLAnnotationDisplayableManager_h
#define __vtkDMMLAnnotationDisplayableManager_h

// Annotations includes
#include "vtkCjyxAnnotationsModuleDMMLDisplayableManagerExport.h"
class vtkDMMLAnnotationClickCounter;
class vtkDMMLAnnotationDisplayNode;
class vtkDMMLAnnotationDisplayableManagerHelper;
class vtkDMMLAnnotationLineDisplayNode;
class vtkDMMLAnnotationNode;
class vtkDMMLAnnotationPointDisplayNode;
class vtkCjyxViewerWidget;

// DMMLDisplayableManager includes
#include <vtkDMMLAbstractDisplayableManager.h>

// DMML includes
class vtkDMMLSliceNode;

// VTK includes
class vtkAbstractWidget;
class vtkHandleWidget;
class vtkSeedWidget;

/// \ingroup Cjyx_QtModules_Annotation
class VTK_CJYX_ANNOTATIONS_MODULE_DMMLDISPLAYABLEMANAGER_EXPORT
vtkDMMLAnnotationDisplayableManager
  : public vtkDMMLAbstractDisplayableManager
{
public:

  static vtkDMMLAnnotationDisplayableManager *New();
  vtkTypeMacro(vtkDMMLAnnotationDisplayableManager, vtkDMMLAbstractDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // the following functions must be public to be accessible by the callback
  /// Propagate properties of DMML node to widget.
  virtual void PropagateDMMLToWidget(vtkDMMLAnnotationNode* node, vtkAbstractWidget * widget);
  /// Propagate properties of widget to DMML node.
  virtual void PropagateWidgetToDMML(vtkAbstractWidget * widget, vtkDMMLAnnotationNode* node);
  /// Check if this is a 2d SliceView displayable manager, returns true if so,
  /// false otherwise. Checks return from GetSliceNode for non null, which means
  /// it's a 2d displayable manager
  virtual bool Is2DDisplayableManager();
  /// Get the sliceNode, if registered. This would mean it is a 2D SliceView displayableManager.
  vtkDMMLSliceNode * GetSliceNode();

  /// Check if the displayCoordinates are inside the viewport and if not, correct the displayCoordinates
  void RestrictDisplayCoordinatesToViewport(double* displayCoordinates);

  /// Check if there are real changes between two sets of displayCoordinates
  bool GetDisplayCoordinatesChanged(double * displayCoordinates1, double * displayCoordinates2);

  /// Check if there are real changes between two sets of worldCoordinates
  bool GetWorldCoordinatesChanged(double * worldCoordinates1, double * worldCoordinates2);

  /// Convert display to world coordinates
  void GetDisplayToWorldCoordinates(double x, double y, double * worldCoordinates);
  void GetDisplayToWorldCoordinates(double * displayCoordinates, double * worldCoordinates);

  /// Convert world coordinates to local using dmml parent transform
  virtual void GetWorldToLocalCoordinates(vtkDMMLAnnotationNode *node,
                                  double *worldCoordinates, double *localCoordinates);

  /// Set dmml parent transform to widgets
  virtual void SetParentTransformToWidget(vtkDMMLAnnotationNode *vtkNotUsed(node), vtkAbstractWidget *vtkNotUsed(widget)){}

  /// Set/Get the 2d scale factor to divide 3D scale by to show 2D elements appropriately (usually set to 300)
  vtkSetMacro(ScaleFactor2D, double);
  vtkGetMacro(ScaleFactor2D, double);

  /// Return true if in lightbox mode - there is a slice node that has layout
  /// grid columns or rows > 1
  bool IsInLightboxMode();

  /// Gets the world coordinate of the annotation node, transforms it to
  /// display coordinates.
  /// Defaults to returning the 0th control point's light box index. Returns
  /// -1 if not in lightbox mode.
  int GetLightboxIndex(vtkDMMLAnnotationNode *node, int controlPointIndex = 0);

  /// Set up data structures for an annotation node.  Returns false on failure
  /// or if it's already set up. Can be called to reinitialise a node's widgets
  /// after calling RemoveWidgetAndNode on the Helper
  /// \sa vtkDMMLAnnotationDisplayableManagerHelper::RemoveWidgetAndNode()
  bool AddAnnotation(vtkDMMLAnnotationNode *node);

  bool CanProcessInteractionEvent(vtkDMMLInteractionEventData* eventData, double &closestDistance2) override;
  bool ProcessInteractionEvent(vtkDMMLInteractionEventData* eventData) override;

protected:

  vtkDMMLAnnotationDisplayableManager();
  ~vtkDMMLAnnotationDisplayableManager() override;

  void ProcessDMMLNodesEvents(vtkObject *caller, unsigned long event, void *callData) override;

  void Create() override;

  /// wrap the superclass render request in a check for batch processing
  virtual void RequestRender();

  /// Remove DMML observers
  void RemoveDMMLObservers() override;

  /// Called from RequestRender method if UpdateFromDMMLRequested is true
  /// \sa RequestRender() SetUpdateFromDMMLRequested()
  void UpdateFromDMML() override;

  void SetDMMLSceneInternal(vtkDMMLScene* newScene) override;

  /// Called after the corresponding DMML event is triggered, from AbstractDisplayableManager
  /// \sa ProcessDMMLSceneEvents
  void UpdateFromDMMLScene() override;
  void OnDMMLSceneEndClose() override;
  void OnDMMLSceneNodeAdded(vtkDMMLNode* node) override;
  void OnDMMLSceneNodeRemoved(vtkDMMLNode* node) override;

  /// Called after the corresponding DMML View container was modified
  void OnDMMLDisplayableNodeModifiedEvent(vtkObject* caller) override;

  /// Handler for specific SliceView actions
  virtual void OnDMMLSliceNodeModifiedEvent(vtkDMMLSliceNode * sliceNode);

  /// Check, if the widget is displayable in the current slice geometry
  virtual bool IsWidgetDisplayable(vtkDMMLSliceNode * sliceNode, vtkDMMLAnnotationNode* node);

  /// Observe one node
  void SetAndObserveNode(vtkDMMLAnnotationNode *annotationNode);
  /// Observe all associated nodes.
  void SetAndObserveNodes();

  /// Observe the interaction node.
  void AddObserversToInteractionNode();
  void RemoveObserversFromInteractionNode();

  /// Observe the selection node for:
  ///    * vtkDMMLSelectionNode::UnitModifiedEvent
  /// events to update the unit of the annotation nodes.
  /// \sa RemoveObserversFromSelectionNode(), AddObserversToInteractionNode(),
  /// OnDMMLSelectionNodeUnitModifiedEvent()
  void AddObserversToSelectionNode();
  void RemoveObserversFromSelectionNode();

  /// Preset functions for certain events.
  virtual void OnDMMLAnnotationNodeModifiedEvent(vtkDMMLNode* node);
  virtual void OnDMMLAnnotationNodeTransformModifiedEvent(vtkDMMLNode* node);
  virtual void OnDMMLAnnotationNodeLockModifiedEvent(vtkDMMLNode* node);
  virtual void OnDMMLAnnotationDisplayNodeModifiedEvent(vtkDMMLNode *node);
  virtual void OnDMMLAnnotationControlPointModifiedEvent(vtkDMMLNode *node);
  virtual void OnDMMLSelectionNodeUnitModifiedEvent(vtkDMMLSelectionNode*) {}

  //
  // Handling of interaction within the RenderWindow
  //

  // Get the coordinates of a click in the RenderWindow
  void OnClickInRenderWindowGetCoordinates();
  /// Callback for click in RenderWindow
  virtual void OnClickInRenderWindow(double x, double y, const char *associatedNodeID = nullptr);
  /// Counter for clicks in Render Window
  vtkDMMLAnnotationClickCounter* m_ClickCounter;

  /// Update just the position for the widget, implemented by subclasses.
  virtual void UpdatePosition(vtkAbstractWidget *vtkNotUsed(widget), vtkDMMLNode *vtkNotUsed(node)) {}
  //
  // Seeds for widget placement
  //

  /// Place a seed for widgets
  virtual void PlaceSeed(double x, double y);
  /// Return the placed seeds
  vtkHandleWidget * GetSeed(int index);

  //
  // Coordinate Conversions
  //

  /// Convert display to world coordinates
//  void GetDisplayToWorldCoordinates(double x, double y, double * worldCoordinates);
//  void GetDisplayToWorldCoordinates(double * displayCoordinates, double * worldCoordinates);

  /// Convert display to world coordinates
  void GetWorldToDisplayCoordinates(double r, double a, double s, double * displayCoordinates);
  void GetWorldToDisplayCoordinates(double * worldCoordinates, double * displayCoordinates);

  /// Convert display to viewport coordinates
  void GetDisplayToViewportCoordinates(double x, double y, double * viewportCoordinates);
  void GetDisplayToViewportCoordinates(double *displayCoordinates, double * viewportCoordinates);

  //
  // Widget functionality
  //

  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkDMMLAnnotationNode* node);
  /// Gets called when widget was created
  virtual void OnWidgetCreated(vtkAbstractWidget * widget, vtkDMMLAnnotationNode * node);
  /// Get the widget of a node.
  vtkAbstractWidget * GetWidget(vtkDMMLAnnotationNode * node);

  /// Check if it is the right displayManager
  bool IsCorrectDisplayableManager();

  /// Return true if this displayable manager supports(can manage) that node,
  /// false otherwise.
  /// Can be reimplemented to add more conditions.
  /// \sa IsManageable(const char*), IsCorrectDisplayableManager()
  virtual bool IsManageable(vtkDMMLNode* node);
  /// Return true if this displayable manager supports(can manage) that node class,
  /// false otherwise.
  /// Can be reimplemented to add more conditions.
  /// \sa IsManageable(vtkDMMLNode*), IsCorrectDisplayableManager()
  virtual bool IsManageable(const char* nodeClassName);

  /// Focus of this displayableManager is set to a specific annotation type when inherited
  const char* m_Focus;

  /// Disable processing when updating is in progress.
  int m_Updating;

  /// Respond to interactor style events
  void OnInteractorStyleEvent(int eventid) override;

  /// Accessor for internal flag that disables interactor style event processing
  vtkGetMacro(DisableInteractorStyleEventsProcessing, int);

  vtkDMMLAnnotationDisplayableManagerHelper * Helper;

  double LastClickWorldCoordinates[4];

private:

  vtkDMMLAnnotationDisplayableManager(const vtkDMMLAnnotationDisplayableManager&) = delete;
  void operator=(const vtkDMMLAnnotationDisplayableManager&) = delete;


  int DisableInteractorStyleEventsProcessing;

  /// Scale factor for 2d windows
  double ScaleFactor2D;
};

#endif
