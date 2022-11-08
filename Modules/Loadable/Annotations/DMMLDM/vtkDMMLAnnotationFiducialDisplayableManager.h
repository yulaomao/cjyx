/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Cjyx

 Module:    $RCSfile: vtkDMMLAnnotationFiducialDisplayableManager.h,v $
 Date:      $Date: 2010/07/26 04:48:05 $
 Version:   $Revision: 1.5 $

 =========================================================================auto=*/

#ifndef __vtkDMMLAnnotationFiducialDisplayableManager_h
#define __vtkDMMLAnnotationFiducialDisplayableManager_h

// Annotation includes
#include "vtkDMMLAnnotationDisplayableManager.h"
#include "vtkCjyxAnnotationsModuleDMMLDisplayableManagerExport.h"

class vtkDMMLAnnotationFiducialNode;
class vtkCjyxViewerWidget;
class vtkDMMLAnnotationTextDisplayNode;
class vtkDMMLAnnotationPointDisplayNode;
class vtkDMMLAnnotationLineDisplayNode;
class vtkTextWidget;

/// \ingroup Cjyx_QtModules_Annotation
class VTK_CJYX_ANNOTATIONS_MODULE_DMMLDISPLAYABLEMANAGER_EXPORT
vtkDMMLAnnotationFiducialDisplayableManager
  : public vtkDMMLAnnotationDisplayableManager
{
public:

  static vtkDMMLAnnotationFiducialDisplayableManager *New();
  vtkTypeMacro(vtkDMMLAnnotationFiducialDisplayableManager, vtkDMMLAnnotationDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:

  vtkDMMLAnnotationFiducialDisplayableManager(){this->m_Focus="vtkDMMLAnnotationFiducialNode";}
  ~vtkDMMLAnnotationFiducialDisplayableManager() override = default;

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

  /// set up an observer on the interactor style to watch for key press events
  virtual void AdditionnalInitializeStep();
  /// respond to the interactor style event
  void OnInteractorStyleEvent(int eventid) override;

  // respond to control point modified events
  void UpdatePosition(vtkAbstractWidget *widget, vtkDMMLNode *node) override;

  std::map<vtkDMMLNode*, int> NodeGlyphTypes;

  // clean up when scene closes
  void OnDMMLSceneEndClose() override;

private:

  vtkDMMLAnnotationFiducialDisplayableManager(const vtkDMMLAnnotationFiducialDisplayableManager&) = delete;
  void operator=(const vtkDMMLAnnotationFiducialDisplayableManager&) = delete;

};

#endif
