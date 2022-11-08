/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLLinearTransformNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkDMMLLinearTransformNode_h
#define __vtkDMMLLinearTransformNode_h

#define TRANSFORM_NODE_MATRIX_COPY_REQUIRED

#include "vtkDMMLTransformNode.h"

class vtkDMMLStorageNode;
class vtkTransform;
class InternalTransformToParentMatrix;

/// \brief DMML node for representing a linear transformation.
///
/// Internally, always the TransformToParent matrix is stored and TransformFromParent is computed by inverting
/// the matrix. It makes the code simpler and faster to hardcode this. ToParent is stored because this is what
/// we usually display to the user (it is more intuitive than the FromParent resampling transform).
class VTK_DMML_EXPORT vtkDMMLLinearTransformNode : public vtkDMMLTransformNode
{
  public:
  static vtkDMMLLinearTransformNode *New();
  vtkTypeMacro(vtkDMMLLinearTransformNode,vtkDMMLTransformNode);
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
  vtkDMMLCopyContentDefaultMacro(vtkDMMLLinearTransformNode);

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "LinearTransform";};

  ///
  /// Create default storage node or nullptr if does not have one
  vtkDMMLStorageNode* CreateDefaultStorageNode() override
    {
    return Superclass::CreateDefaultStorageNode();
    };

protected:
  vtkDMMLLinearTransformNode();
  ~vtkDMMLLinearTransformNode() override;
  vtkDMMLLinearTransformNode(const vtkDMMLLinearTransformNode&);
  void operator=(const vtkDMMLLinearTransformNode&);
};

#endif
