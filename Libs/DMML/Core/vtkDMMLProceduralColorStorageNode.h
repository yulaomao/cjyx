/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLProceduralColorStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.8 $

=========================================================================auto=*/

#ifndef __vtkDMMLProceduralColorStorageNode_h
#define __vtkDMMLProceduralColorStorageNode_h

#include "vtkDMMLStorageNode.h"

/// \brief DMML node for procedural color node storage
///
/// vtkDMMLProceduralColorStorageNode nodes describe the color storage
/// node that allows to read/write volume data from/to file using XML to describe
/// the points defined in the color transfer function
class VTK_DMML_EXPORT vtkDMMLProceduralColorStorageNode : public vtkDMMLStorageNode
{
  public:
  static vtkDMMLProceduralColorStorageNode *New();
  vtkTypeMacro(vtkDMMLProceduralColorStorageNode,vtkDMMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "ProceduralColorStorage";}

  /// Return true if the node can be read in
  bool CanReadInReferenceNode(vtkDMMLNode* refNode) override;

protected:
  vtkDMMLProceduralColorStorageNode();
  ~vtkDMMLProceduralColorStorageNode() override;
  vtkDMMLProceduralColorStorageNode(const vtkDMMLProceduralColorStorageNode&);
  void operator=(const vtkDMMLProceduralColorStorageNode&);

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
