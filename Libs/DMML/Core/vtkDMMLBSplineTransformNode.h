/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLBSplineTransformNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkDMMLBSplineTransformNode_h
#define __vtkDMMLBSplineTransformNode_h

#include "vtkDMMLTransformNode.h"

class vtkDMMLStorageNode;

/// \brief DMML node for representing a nonlinear transformation to the parent node
/// using a bspline transform.
///
/// DMML node for representing a nonlinear transformation to the parent
/// node in the form of a vtkBSplineDeformableTransform.
class VTK_DMML_EXPORT vtkDMMLBSplineTransformNode : public vtkDMMLTransformNode
{
  public:
  static vtkDMMLBSplineTransformNode *New();
  vtkTypeMacro(vtkDMMLBSplineTransformNode,vtkDMMLTransformNode);
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
  vtkDMMLCopyContentDefaultMacro(vtkDMMLBSplineTransformNode);

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "BSplineTransform";};

  ///
  /// Create default storage node or nullptr if does not have one
  vtkDMMLStorageNode* CreateDefaultStorageNode() override
    {
    return Superclass::CreateDefaultStorageNode();
    };

protected:
  vtkDMMLBSplineTransformNode();
  ~vtkDMMLBSplineTransformNode() override;
  vtkDMMLBSplineTransformNode(const vtkDMMLBSplineTransformNode&);
  void operator=(const vtkDMMLBSplineTransformNode&);

};

#endif

