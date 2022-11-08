/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLSceneViewStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkDMMLSceneViewStorageNode_h
#define __vtkDMMLSceneViewStorageNode_h

#include "vtkDMMLStorageNode.h"

/// \brief DMML node for model storage on disk.
///
/// Storage nodes has methods to read/write vtkPolyData to/from disk.
class VTK_DMML_EXPORT vtkDMMLSceneViewStorageNode : public vtkDMMLStorageNode
{
public:
  static vtkDMMLSceneViewStorageNode *New();
  vtkTypeMacro(vtkDMMLSceneViewStorageNode,vtkDMMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  ///
  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "SceneViewStorage";}

  /// Initialize all the supported read file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  /// Return true if the node can be read in
  bool CanReadInReferenceNode(vtkDMMLNode *refNode) override;

protected:
  vtkDMMLSceneViewStorageNode();
  ~vtkDMMLSceneViewStorageNode() override;
  vtkDMMLSceneViewStorageNode(const vtkDMMLSceneViewStorageNode&);
  void operator=(const vtkDMMLSceneViewStorageNode&);

  ///
  /// Read data and set it in the referenced node
  /// NOTE: Subclasses should implement this method
  int ReadDataInternal(vtkDMMLNode *refNode) override;

  ///
  /// Write data from a  referenced node
  /// NOTE: Subclasses should implement this method
  int WriteDataInternal(vtkDMMLNode *refNode) override;

};

#endif
