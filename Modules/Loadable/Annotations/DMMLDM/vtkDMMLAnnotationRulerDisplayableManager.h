/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Cjyx

 Module:    $RCSfile: vtkDMMLAnnotationRulerDisplayableManager.h,v $
 Date:      $Date: 2010/07/26 04:48:05 $
 Version:   $Revision: 1.5 $

 =========================================================================auto=*/

#ifndef __vtkDMMLAnnotationRulerDisplayableManager_h
#define __vtkDMMLAnnotationRulerDisplayableManager_h

// Annotation includes
#include "vtkDMMLAnnotationDisplayableManager.h"
#include "vtkCjyxAnnotationsModuleDMMLDisplayableManagerExport.h"

class vtkDMMLAnnotationRulerNode;
class vtkCjyxViewerWidget;
class vtkDMMLAnnotationRulerDisplayNode;
class vtkDMMLAnnotationPointDisplayNode;
class vtkDMMLAnnotationLineDisplayNode;
class vtkDMMLSelectionNode;
class vtkTextWidget;

/// \ingroup Cjyx_QtModules_Annotation
class VTK_CJYX_ANNOTATIONS_MODULE_DMMLDISPLAYABLEMANAGER_EXPORT
vtkDMMLAnnotationRulerDisplayableManager
  : public vtkDMMLAnnotationDisplayableManager
{
public:

  static vtkDMMLAnnotationRulerDisplayableManager *New();
  vtkTypeMacro(vtkDMMLAnnotationRulerDisplayableManager, vtkDMMLAnnotationDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:

  vtkDMMLAnnotationRulerDisplayableManager(){this->m_Focus="vtkDMMLAnnotationRulerNode";}
  ~vtkDMMLAnnotationRulerDisplayableManager() override = default;

  /// Callback for click in RenderWindow
  void OnClickInRenderWindow(double x, double y, const char *associatedNodeID) override;
  /// Create a widget.
  vtkAbstractWidget * CreateWidget(vtkDMMLAnnotationNode* node) override;

  /// Gets called when widget was created
  void OnWidgetCreated(vtkAbstractWidget * widget, vtkDMMLAnnotationNode * node) override;

  /// Propagate properties of DMML node to widget.
  void PropagateDMMLToWidget(vtkDMMLAnnotationNode* node, vtkAbstractWidget * widget) override;
  /// Propagate properties of widget to DMML node.
  void PropagateWidgetToDMML(vtkAbstractWidget * widget, vtkDMMLAnnotationNode* node) override;

  // update the ruler end point positions from the DMML node
  void UpdatePosition(vtkAbstractWidget *widget, vtkDMMLNode *node) override;

  // Get the label from the node and unit node
  std::string GetLabelFormat(vtkDMMLAnnotationRulerNode* rulerNode);

  /// Compute the distance in mm between 2 world coordinates points
  /// \sa ApplyUnit()
  double GetDistance(const double* wc1, const double* wc2);
  /// Apply the current unit to a length in mm.
  /// \sa GetDistance()
  double ApplyUnit(double lengthInMM);

  /// When the unit has changed, modify the ruler nodes to refresh the label.
  /// \sa AddObserversToSelectionNode(), RemoveObserversFromSelectionNode()
  void OnDMMLSelectionNodeUnitModifiedEvent(vtkDMMLSelectionNode* selectionNode) override;

private:

  vtkDMMLAnnotationRulerDisplayableManager(const vtkDMMLAnnotationRulerDisplayableManager&) = delete;
  void operator=(const vtkDMMLAnnotationRulerDisplayableManager&) = delete;

};

#endif
