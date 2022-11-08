/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLTransformableNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkDMMLTransformableNode_h
#define __vtkDMMLTransformableNode_h

// DMML includes
#include "vtkDMMLStorableNode.h"
#include "vtkVector.h"
class vtkDMMLTransformNode;

// VTK includes
class vtkAbstractTransform;
class vtkMatrix4x4;

/// \brief DMML node for representing a node with a transform.
///
/// A superclass for other nodes that can have a transform to parent node
/// like volume, model and transformation nodes.
class VTK_DMML_EXPORT vtkDMMLTransformableNode : public vtkDMMLStorableNode
{
public:
  vtkTypeMacro(vtkDMMLTransformableNode,vtkDMMLStorableNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override = 0;

  ///
  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override = 0;

  ///
  /// Set a reference to transform node
  /// Returns true on success.
  /// The method will fail if a child transform of a transform node is attempted
  /// to be set as parent to prevent circular reference.
  /// If current node or new parent transform node is not added to the scene yet
  /// then circular reference is not checked and it is the developer's responsibility
  /// no child transform is set as parent.
  bool SetAndObserveTransformNodeID(const char *transformNodeID);

  ///
  /// Associated transform DMML node
  vtkDMMLTransformNode* GetParentTransformNode();

  ///
  /// alternative method to propagate events generated in Transform nodes
  void ProcessDMMLEvents ( vtkObject * /*caller*/,
                                  unsigned long /*event*/,
                                  void * /*callData*/ ) override;

  /// TransformModifiedEvent is send when the parent transform is modidied
  enum
    {
      TransformModifiedEvent = 15000
    };

  /// Returns true if the transformable node can apply non-linear transforms.
  /// A transformable node is always expected to apply linear transforms.
  /// \sa ApplyTransformMatrix, ApplyTransform
  virtual bool CanApplyNonLinearTransforms()const;

  /// Convenience function to allow transforming a node by specifying a
  /// transformation matrix.
  /// \sa ApplyTransformMatrix, ApplyTransform
  virtual void ApplyTransformMatrix(vtkMatrix4x4* transformMatrix);

  /// Transforms the node with the provided non-linear transform.
  /// \sa SetAndObserveTransformNodeID, ApplyTransformMatrix, CanApplyNonLinearTransforms
  virtual void ApplyTransform(vtkAbstractTransform* transform);

  /// Utility function to convert a point position in the node's coordinate system to world coordinate system.
  /// Note for transform nodes: the node coordinate system is transformed by all the parent transforms, but not by the
  /// transform that is stored in the current node. To get all the transform, including that is stored in the current
  /// transform node, vtkDMMLTransformNode::GetTransformBetweenNodes() method can be used.
  /// \sa TransformPointFromWorld, SetAndObserveTransformNodeID
  virtual void TransformPointToWorld(const double inLocal[3], double outWorld[3]);

  /// Utility function to convert a point position in the node's coordinate system to world coordinate system.
  /// \sa TransformPointToWorld, SetAndObserveTransformNodeID
  virtual void TransformPointToWorld(const vtkVector3d &inLocal, vtkVector3d &outWorld);

  /// Utility function to convert a point position in world coordinate system to markup node's coordinate system
  /// \sa TransformPointToWorld, SetAndObserveTransformNodeID
  virtual void TransformPointFromWorld(const double inWorld[3], double outLocal[3]);

  /// Utility function to convert a point position in world coordinate system to markup node's coordinate system
  /// \sa TransformPointToWorld, SetAndObserveTransformNodeID
  virtual void TransformPointFromWorld(const vtkVector3d &inWorld, vtkVector3d &outLocal);

  /// Get referenced transform node id
  const char *GetTransformNodeID();

  /// Apply the associated transform to the transformable node. Return true
  /// on success, false otherwise.
  bool HardenTransform();

protected:
  vtkDMMLTransformableNode();
  ~vtkDMMLTransformableNode() override;
  vtkDMMLTransformableNode(const vtkDMMLTransformableNode&);
  void operator=(const vtkDMMLTransformableNode&);

  static const char* TransformNodeReferenceRole;
  static const char* TransformNodeReferenceDMMLAttributeName;

  virtual const char* GetTransformNodeReferenceRole();
  virtual const char* GetTransformNodeReferenceDMMLAttributeName();

  ///
  /// Called when a node reference ID is added (list size increased).
  void OnNodeReferenceAdded(vtkDMMLNodeReference *reference) override;

  ///
  /// Called when a node reference ID is modified.
  void OnNodeReferenceModified(vtkDMMLNodeReference *reference) override;

  ///
  /// Called after a node reference ID is removed (list size decreased).
  void OnNodeReferenceRemoved(vtkDMMLNodeReference *reference) override;

  /// Called when transform node reference added/modified/removed
  virtual void OnTransformNodeReferenceChanged(vtkDMMLTransformNode* transformNode);

private:
  char* TransformNodeIDInternal;
  vtkSetStringMacro(TransformNodeIDInternal);
  vtkGetStringMacro(TransformNodeIDInternal);

};

#endif
