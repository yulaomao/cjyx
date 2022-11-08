/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLDisplayableHierarchyLogic.h,v $
  Date:      $Date: 2010-02-15 16:35:35 -0500 (Mon, 15 Feb 2010) $
  Version:   $Revision: 12142 $

=========================================================================auto=*/

#ifndef __vtkDMMLDisplayableHierarchyLogic_h
#define __vtkDMMLDisplayableHierarchyLogic_h

// DMML includes
#include <vtkDMMLAbstractLogic.h>
class vtkDMMLDisplayableNode;
class vtkDMMLDisplayableHierarchyNode;

// STD includes
#include <cstdlib>
#include <vector>

typedef std::vector< vtkDMMLDisplayableHierarchyNode *> vtkDMMLDisplayableHierarchyNodeList;

/// \brief Cjyx logic class for hierarchy manipulation.
///
/// This class manages the logic associated with displayable hierarchy nodes.
class VTK_DMML_LOGIC_EXPORT vtkDMMLDisplayableHierarchyLogic : public vtkDMMLAbstractLogic
{
  public:

  /// The Usual vtk class functions
  static vtkDMMLDisplayableHierarchyLogic *New();
  vtkTypeMacro(vtkDMMLDisplayableHierarchyLogic,vtkDMMLAbstractLogic);

  /// Create a 1:1 displayable hierarchy node for this node, add it to the
  /// scene and return the id, null on failure
  char *AddDisplayableHierarchyNodeForNode(vtkDMMLDisplayableNode *node);

  /// Create displayable hierarchy nodes as needed to make the child node a
  /// child of the parent node (may need to add 1:1 hierarchy nodes for both
  /// parent and child). Return true on success, false on failure.
  bool AddChildToParent(vtkDMMLDisplayableNode *child, vtkDMMLDisplayableNode *parent);

  /// Delete the passed hierarchy node and all children hierarchy nodes and
  /// the associated nodes to which they point. Return true on success, false
  /// on failure. Gets the dmml scene from the node.
  bool DeleteHierarchyNodeAndChildren(vtkDMMLDisplayableHierarchyNode *hnode);

protected:
  vtkDMMLDisplayableHierarchyLogic();
  ~vtkDMMLDisplayableHierarchyLogic() override;
  vtkDMMLDisplayableHierarchyLogic(const vtkDMMLDisplayableHierarchyLogic&);
  void operator=(const vtkDMMLDisplayableHierarchyLogic&);

  /// Reimplemented to observe the scene
  void SetDMMLSceneInternal(vtkDMMLScene* newScene) override;

  /// Delete the hierarchy node when a node is removed from the scene
  void OnDMMLSceneNodeRemoved(vtkDMMLNode* removedNode) override;

};

#endif
