/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLVolumePropertyStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
///  vtkDMMLVolumePropertyStorageNode - DMML node for transform storage on disk
///
/// Storage nodes has methods to read/write transforms to/from disk

#ifndef __vtkDMMLVolumePropertyStorageNode_h
#define __vtkDMMLVolumePropertyStorageNode_h

// VolumeRendering includes
#include "vtkCjyxVolumeRenderingModuleDMMLExport.h"

// DMML includes
#include "vtkDMMLStorageNode.h"

class vtkImageData;

class VTK_CJYX_VOLUMERENDERING_MODULE_DMML_EXPORT vtkDMMLVolumePropertyStorageNode
  : public vtkDMMLStorageNode
{
  public:
  static vtkDMMLVolumePropertyStorageNode *New();
  vtkTypeMacro(vtkDMMLVolumePropertyStorageNode,vtkDMMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  ///
  /// Get node XML tag name (like Storage, Transform)
  const char* GetNodeTagName() override {return "VolumePropertyStorage";}

  /// Return true if the node can be read in
  bool CanReadInReferenceNode(vtkDMMLNode *refNode) override;

protected:
  vtkDMMLVolumePropertyStorageNode();
  ~vtkDMMLVolumePropertyStorageNode() override;
  vtkDMMLVolumePropertyStorageNode(const vtkDMMLVolumePropertyStorageNode&);
  void operator=(const vtkDMMLVolumePropertyStorageNode&);

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
