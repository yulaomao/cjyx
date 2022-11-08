/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLGridTransformNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkDMMLGridTransformNode_h
#define __vtkDMMLGridTransformNode_h

#include "vtkDMMLTransformNode.h"

/// \brief DMML node for representing a nonlinear transformation to the parent node using a grid transform.
///
/// DMML node for representing a nonlinear transformation to the parent
/// node in the form of a vtkOrientedGridTransform.
class VTK_DMML_EXPORT vtkDMMLGridTransformNode : public vtkDMMLTransformNode
{
public:
  static vtkDMMLGridTransformNode *New();
  vtkTypeMacro(vtkDMMLGridTransformNode,vtkDMMLTransformNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  ///
  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkDMMLNode::CopyContent
  vtkDMMLCopyContentDefaultMacro(vtkDMMLGridTransformNode);

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "GridTransform";}

protected:
  vtkDMMLGridTransformNode();
  ~vtkDMMLGridTransformNode() override;
  vtkDMMLGridTransformNode(const vtkDMMLGridTransformNode&);
  void operator=(const vtkDMMLGridTransformNode&);
};

#endif
