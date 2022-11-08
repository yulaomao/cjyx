/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Cjyx

 Module:    $RCSfile: vtkDMMLAnnotationROIDisplayableManager.h,v $
 Date:      $Date: 2010/07/26 04:48:05 $
 Version:   $Revision: 1.5 $

 =========================================================================auto=*/

#ifndef __vtkDMMLAnnotationROIDisplayableManager_h
#define __vtkDMMLAnnotationROIDisplayableManager_h

// Annotation includes
#include "vtkDMMLAnnotationDisplayableManager.h"

class vtkDMMLAnnotationROINode;
class vtkCjyxViewerWidget;
class vtkDMMLAnnotationROIDisplayNode;
class vtkDMMLAnnotationPointDisplayNode;
class vtkDMMLAnnotationLineDisplayNode;
class vtkTextWidget;

/// \ingroup Cjyx_QtModules_Annotation
class VTK_CJYX_ANNOTATIONS_MODULE_DMMLDISPLAYABLEMANAGER_EXPORT
vtkDMMLAnnotationROIDisplayableManager
  : public vtkDMMLAnnotationDisplayableManager
{
public:

  static vtkDMMLAnnotationROIDisplayableManager *New();
  vtkTypeMacro(vtkDMMLAnnotationROIDisplayableManager, vtkDMMLAnnotationDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:

  vtkDMMLAnnotationROIDisplayableManager(){this->m_Focus="vtkDMMLAnnotationROINode";}
  ~vtkDMMLAnnotationROIDisplayableManager() override;

  /// Callback for click in RenderWindow
  void OnClickInRenderWindow(double x, double y, const char *associatedNodeID) override;
  /// Create a widget.
  vtkAbstractWidget * CreateWidget(vtkDMMLAnnotationNode* node) override;

  /// Gets called when widget was created
  void OnWidgetCreated(vtkAbstractWidget * widget, vtkDMMLAnnotationNode * node) override;

  /// Propagate properties of DMML node to widget.
  void PropagateDMMLToWidget(vtkDMMLAnnotationNode* node, vtkAbstractWidget * widget) override;
  virtual void PropagateDMMLToWidget2D(vtkDMMLAnnotationNode* node, vtkAbstractWidget * widget);

  /// Propagate properties of widget to DMML node.
  void PropagateWidgetToDMML(vtkAbstractWidget * widget, vtkDMMLAnnotationNode* node) override;

  void OnDMMLSceneNodeRemoved(vtkDMMLNode* node) override;

  /// Handler for specific SliceView actions
  void OnDMMLSliceNodeModifiedEvent(vtkDMMLSliceNode * sliceNode) override;


  /// Update just the position for the widget, implemented by subclasses.
  void UpdatePosition(vtkAbstractWidget *widget, vtkDMMLNode *node) override;

  /// Check, if the widget is displayable in the current slice geometry
  bool IsWidgetDisplayable(vtkDMMLSliceNode *sliceNode, vtkDMMLAnnotationNode* node) override;

  /// Set dmml parent transform to widgets
  void SetParentTransformToWidget(vtkDMMLAnnotationNode *node, vtkAbstractWidget *widget) override;

private:

  vtkDMMLAnnotationROIDisplayableManager(const vtkDMMLAnnotationROIDisplayableManager&) = delete;
  void operator=(const vtkDMMLAnnotationROIDisplayableManager&) = delete;

};

#endif
