/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLAnnotationSnapshotStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
///  vtkDMMLAnnotationSnapshotStorageNode - DMML node for model storage on disk
///
/// Storage nodes has methods to read/write vtkPolyData to/from disk

#ifndef __vtkDMMLAnnotationSnapshotStorageNode_h
#define __vtkDMMLAnnotationSnapshotStorageNode_h

#include "vtkCjyxAnnotationsModuleDMMLExport.h"
#include "vtkDMMLStorageNode.h"

class vtkImageData;
/// \ingroup Cjyx_QtModules_Annotation
class VTK_CJYX_ANNOTATIONS_MODULE_DMML_EXPORT vtkDMMLAnnotationSnapshotStorageNode
  : public vtkDMMLStorageNode
{
public:
  static vtkDMMLAnnotationSnapshotStorageNode *New();
  vtkTypeMacro(vtkDMMLAnnotationSnapshotStorageNode,vtkDMMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "AnnotationSnapshotStorage";}

  /// Return true if the node can be read in
  bool CanReadInReferenceNode(vtkDMMLNode* refNode) override;
protected:
  vtkDMMLAnnotationSnapshotStorageNode();
  ~vtkDMMLAnnotationSnapshotStorageNode() override;
  vtkDMMLAnnotationSnapshotStorageNode(const vtkDMMLAnnotationSnapshotStorageNode&);
  void operator=(const vtkDMMLAnnotationSnapshotStorageNode&);

  /// Initialize all the supported read file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  /// Read data and set it in the referenced node
  int ReadDataInternal(vtkDMMLNode *refNode) override;

  /// Write data from a  referenced node
  int WriteDataInternal(vtkDMMLNode *refNode) override;
};

#endif



