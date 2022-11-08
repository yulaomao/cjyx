/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLColorTableStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.8 $

=========================================================================auto=*/

#ifndef __vtkDMMLColorTableStorageNode_h
#define __vtkDMMLColorTableStorageNode_h

#include "vtkDMMLStorageNode.h"

/// \brief DMML node for representing a volume storage.
///
/// vtkDMMLColorTableStorageNode nodes describe the archetybe based volume storage
/// node that allows to read/write volume data from/to file using generic ITK mechanism.
class VTK_DMML_EXPORT vtkDMMLColorTableStorageNode : public vtkDMMLStorageNode
{
  public:
  static vtkDMMLColorTableStorageNode *New();
  vtkTypeMacro(vtkDMMLColorTableStorageNode,vtkDMMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override  {return "ColorTableStorage";};

  /// Return true if the node can be read in
  bool CanReadInReferenceNode(vtkDMMLNode* refNode) override;

protected:
  vtkDMMLColorTableStorageNode();
  ~vtkDMMLColorTableStorageNode() override;
  vtkDMMLColorTableStorageNode(const vtkDMMLColorTableStorageNode&);
  void operator=(const vtkDMMLColorTableStorageNode&);

  /// Initialize all the supported read file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  /// Read data and set it in the referenced node
  int ReadDataInternal(vtkDMMLNode *refNode) override;

  /// Write data from a  referenced node
  int WriteDataInternal(vtkDMMLNode *refNode) override;

  /// maximum valid number of colors to read in
  int MaximumColorID;

};

#endif



