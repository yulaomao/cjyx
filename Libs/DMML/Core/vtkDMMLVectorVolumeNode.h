/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLVectorVolumeNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkDMMLVectorVolumeNode_h
#define __vtkDMMLVectorVolumeNode_h

#include "vtkDMMLTensorVolumeNode.h"
class vtkDMMLVolumeArchetypeStorageNode;
class vtkDMMLVectorVolumeDisplayNode;

/// \brief DMML node for representing a vector volume (image stack).
///
/// Volume with vector pixel type.
class VTK_DMML_EXPORT vtkDMMLVectorVolumeNode : public vtkDMMLTensorVolumeNode
{
  public:
  static vtkDMMLVectorVolumeNode *New();
  vtkTypeMacro(vtkDMMLVectorVolumeNode,vtkDMMLTensorVolumeNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkDMMLNode::CopyContent
  vtkDMMLCopyContentDefaultMacro(vtkDMMLVectorVolumeNode);

  ///
  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Copy the node's attributes to this object
  void Copy(vtkDMMLNode *node) override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "VectorVolume";}

  ///
  /// Associated display DMML node
  virtual vtkDMMLVectorVolumeDisplayNode* GetVectorVolumeDisplayNode();

  ///
  /// Create default storage node or nullptr if does not have one
  vtkDMMLStorageNode* CreateDefaultStorageNode() override;

  ///
  /// Create and observe default display node
  void CreateDefaultDisplayNodes() override;

protected:
  vtkDMMLVectorVolumeNode();
  ~vtkDMMLVectorVolumeNode() override;
  vtkDMMLVectorVolumeNode(const vtkDMMLVectorVolumeNode&);
  void operator=(const vtkDMMLVectorVolumeNode&);

};

#endif
