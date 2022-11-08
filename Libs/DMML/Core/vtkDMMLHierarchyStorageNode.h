/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLHierarchyStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.8 $

=========================================================================auto=*/

#ifndef __vtkDMMLHierarchyStorageNode_h
#define __vtkDMMLHierarchyStorageNode_h

#include "vtkDMMLStorageNode.h"

/// \brief DMML node for representing a no-op hierarchy storage.
///
/// vtkDMMLHierarchyStorageNode nodes describe the storage that is a place
/// holder for hierarchy nodes that don't need to write anything to file,
/// subclasses should reimplement ReadData and WriteData
class VTK_DMML_EXPORT vtkDMMLHierarchyStorageNode : public vtkDMMLStorageNode
{
public:
  static vtkDMMLHierarchyStorageNode *New();
  vtkTypeMacro(vtkDMMLHierarchyStorageNode,vtkDMMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  // Description:
  // Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "HierarchyStorage";}

  /// Return true if reference node can be read in
  bool CanReadInReferenceNode(vtkDMMLNode *refNode) override;
protected:
  vtkDMMLHierarchyStorageNode();
  ~vtkDMMLHierarchyStorageNode() override;
  vtkDMMLHierarchyStorageNode(const vtkDMMLHierarchyStorageNode&);
  void operator=(const vtkDMMLHierarchyStorageNode&);

  // Initialize all the supported read file types
  void InitializeSupportedReadFileTypes() override;

  // Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  // Read data and set it in the referenced node
  int ReadDataInternal(vtkDMMLNode *refNode) override;

  // Write data from a  referenced node
  int WriteDataInternal(vtkDMMLNode *refNode) override;
};

#endif
