/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Cjyx
 Module:    $RCSfile: vtkDMMLAnnotationDisplayableManagerHelper,v $
 Date:      $Date: Aug 4, 2010 10:44:52 AM $
 Version:   $Revision: 1.0 $

 =========================================================================auto=*/

#ifndef VTKDMMLANNOTATIONDISPLAYABLEMANAGERHELPER_H_
#define VTKDMMLANNOTATIONDISPLAYABLEMANAGERHELPER_H_

// Annotations includes
#include "vtkCjyxAnnotationsModuleDMMLDisplayableManagerExport.h"

// Annotations DMML includes
class vtkDMMLAnnotationDisplayNode;
class vtkDMMLAnnotationNode;

// DMML includes
class vtkDMMLInteractionNode;

// VTK includes
#include <vtkHandleWidget.h>
#include <vtkLineWidget2.h>
#include <vtkSeedWidget.h>
#include <vtkSmartPointer.h>

// STD includes
#include <map>

/// \ingroup Cjyx_QtModules_Annotation
class VTK_CJYX_ANNOTATIONS_MODULE_DMMLDISPLAYABLEMANAGER_EXPORT
vtkDMMLAnnotationDisplayableManagerHelper
  : public vtkObject
{
public:

  static vtkDMMLAnnotationDisplayableManagerHelper *New();
  vtkTypeMacro(vtkDMMLAnnotationDisplayableManagerHelper, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Lock/Unlock all widgets based on the state of the nodes
  void UpdateLockedAllWidgetsFromNodes();
  /// Lock/Unlock all widgets from interaction node
  void UpdateLockedAllWidgetsFromInteractionNode(vtkDMMLInteractionNode* interactionNode);
  /// Lock/Unlock all widgets
  void UpdateLockedAllWidgets(bool locked);
  /// Lock/Unlock a widget
  void UpdateLocked(vtkDMMLAnnotationNode* node);
  /// Hide/Show a widget according to node's visible flag and if it can be
  /// displayed in this viewer
  void UpdateVisible(vtkDMMLAnnotationNode* node, bool displayableInViewer = true);
  /// Update lock and visibility of a widget
  void UpdateWidget(vtkDMMLAnnotationNode* node);

  /// Get a vtkAbstractWidget* given a node
  vtkAbstractWidget * GetWidget(vtkDMMLAnnotationNode * node);
  /// ...an its associated vtkAbstractWidget* for Slice intersection representation
  vtkAbstractWidget * GetIntersectionWidget(vtkDMMLAnnotationNode * node);
  /// ...an its associated vtkAbstractWidget* for Slice projection representation
  vtkAbstractWidget * GetOverLineProjectionWidget(vtkDMMLAnnotationNode * node);
  /// ...an its associated vtkAbstractWidget* for Slice projection representation
  vtkAbstractWidget * GetUnderLineProjectionWidget(vtkDMMLAnnotationNode * node);
  /// ...an its associated vtkAbstractWidget* for Slice projection representation
  vtkAbstractWidget * GetPointProjectionWidget(vtkDMMLAnnotationNode * node);
  /// Remove all widgets, intersection widgets, nodes
  void RemoveAllWidgetsAndNodes();
  /// Remove a node, its widget and its intersection widget
  void RemoveWidgetAndNode(vtkDMMLAnnotationNode *node);


  /// Search the annotation node list and return the annotation node that has this display node
  vtkDMMLAnnotationNode * GetAnnotationNodeFromDisplayNode(vtkDMMLAnnotationDisplayNode *displayNode);

  //----------------------------------------------------------------------------------
  // The Lists!!
  //
  // An annotation which is managed by a displayableManager consists of
  //   a) the Annotation DMML Node (AnnotationNodeList)
  //   b) the vtkWidget to show this annotation (Widgets)
  //   c) a vtkWidget to represent sliceIntersections in the slice viewers (WidgetIntersections)
  //

  /// List of Nodes managed by the DisplayableManager
  std::vector<vtkDMMLAnnotationNode*> AnnotationNodeList;

  /// .. and its associated convenient typedef
  typedef std::vector<vtkDMMLAnnotationNode*>::iterator AnnotationNodeListIt;

  /// Map of vtkWidget indexed using associated node ID
  std::map<vtkDMMLAnnotationNode*, vtkAbstractWidget*> Widgets;

  /// .. and its associated convenient typedef
  typedef std::map<vtkDMMLAnnotationNode*, vtkAbstractWidget*>::iterator WidgetsIt;

  /// Map of vtkWidgets to reflect the Slice intersections indexed using associated node ID
  std::map<vtkDMMLAnnotationNode*, vtkAbstractWidget*> WidgetIntersections;

  /// .. and its associated convenient typedef
  typedef std::map<vtkDMMLAnnotationNode*, vtkAbstractWidget*>::iterator WidgetIntersectionsIt;

  /// Map of vtkWidgets to reflect the Slice projection indexed using associated node ID
  std::map<vtkDMMLAnnotationNode*, vtkAbstractWidget*> WidgetOverLineProjections;

  /// .. and its associated convenient typedef
  typedef std::map<vtkDMMLAnnotationNode*, vtkAbstractWidget*>::iterator WidgetOverLineProjectionsIt;

  /// Map of vtkWidgets to reflect the Slice projection indexed using associated node ID
  std::map<vtkDMMLAnnotationNode*, vtkAbstractWidget*> WidgetUnderLineProjections;

  /// .. and its associated convenient typedef
  typedef std::map<vtkDMMLAnnotationNode*, vtkAbstractWidget*>::iterator WidgetUnderLineProjectionsIt;

  /// Map of vtkWidgets to reflect the Slice projection indexed using associated node ID
  std::map<vtkDMMLAnnotationNode*, vtkAbstractWidget*> WidgetPointProjections;

  /// .. and its associated convenient typedef
  typedef std::map<vtkDMMLAnnotationNode*, vtkAbstractWidget*>::iterator WidgetPointProjectionsIt;

  //
  // End of The Lists!!
  //
  //----------------------------------------------------------------------------------


  /// Placement of seeds for widget placement
  void PlaceSeed(double x, double y, vtkRenderWindowInteractor * interactor, vtkRenderer * renderer);

  /// Get a placed seed
  vtkHandleWidget * GetSeed(int index);

  /// Remove all placed seeds
  void RemoveSeeds();


protected:

  vtkDMMLAnnotationDisplayableManagerHelper();
  ~vtkDMMLAnnotationDisplayableManagerHelper() override;

private:

  vtkDMMLAnnotationDisplayableManagerHelper(const vtkDMMLAnnotationDisplayableManagerHelper&) = delete;
  void operator=(const vtkDMMLAnnotationDisplayableManagerHelper&) = delete;

  /// SeedWidget for point placement
  vtkSmartPointer<vtkSeedWidget> SeedWidget;
  /// List of Handles for the SeedWidget
  std::vector<vtkSmartPointer<vtkHandleWidget> > HandleWidgetList;
  /// .. and its associated convenient typedef
  typedef std::vector<vtkSmartPointer<vtkHandleWidget> >::iterator HandleWidgetListIt;
};

#endif /* VTKDMMLANNOTATIONDISPLAYABLEMANAGERHELPER_H_ */
