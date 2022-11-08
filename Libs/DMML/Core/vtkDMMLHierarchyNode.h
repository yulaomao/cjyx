/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLHierarchyNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkDMMLHierarchyNode_h
#define __vtkDMMLHierarchyNode_h

// DMML includes
#include "vtkDMMLNode.h"

// VTK includes
class vtkCollection;

// STD includes
#include <vector>

/// \brief Abstract class representing a hierarchy member.
class VTK_DMML_EXPORT vtkDMMLHierarchyNode : public vtkDMMLNode
{
public:
  static vtkDMMLHierarchyNode *New();
  vtkTypeMacro(vtkDMMLHierarchyNode,vtkDMMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  ///
  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Copy the node's attributes to this object
  void Copy(vtkDMMLNode *node) override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "Hierarchy";}

  /// Set the reference node to current scene.
  void SetSceneReferences() override;

  ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  void UpdateReferences() override;

  ///
  /// Observe the reference transform node
  void UpdateScene(vtkDMMLScene *scene) override;

  ///
  /// Update the stored reference to another node in the scene
  void UpdateReferenceID(const char *oldID, const char *newID) override;

  ///
  /// Associated prent DMML node
  vtkDMMLHierarchyNode* GetParentNode();

  ///
  /// Get the top parent node in the hierarchy
  vtkDMMLHierarchyNode* GetTopParentNode();

  ///
  /// String ID of the parent hierarchy DMML node
  virtual char* GetParentNodeID()
  {
    return GetParentNodeIDReference();
  }

  virtual void SetParentNodeID(const char* ref);

  ///
  /// Given this hierarchy node returns all it's children recursively.
  void GetAllChildrenNodes(std::vector< vtkDMMLHierarchyNode *> &childrenNodes);

  ///
  /// Given this hierarchy node returns all it's 1st level children (not recursive).
  /// Note: Most compilers don't make a copy of the list if you call the function like that:
  /// std::vector< vtkDMMLHierarchyNode* > children = this->GetChildrenNodes();
  std::vector< vtkDMMLHierarchyNode *> GetChildrenNodes();

  /// Returns the number of immediate children in the hierarchy
  int GetNumberOfChildrenNodes()
  {
    return static_cast<int>(this->GetChildrenNodes().size());
  }

  /// Get n-th child node sorted in the order of their SortingValue
  vtkDMMLHierarchyNode *GetNthChildNode(int index);

  /// Get index of this node in it's parent based on the value of their SortingValue
  int GetIndexInParent();

  /// Set index of this node in it's parent based on the value of their SortingValue
  void SetIndexInParent(int index);

  /// Move this node in it's parent up (positive increment) or down (negative increment)
  /// by 'increment' number of places
  void MoveInParent(int increment);

  /// Removes immediate children nodes, their children are reparented to this parent node.
  void RemoveHierarchyChildrenNodes();

  /// Removes all children hierarchy nodes including children of children, etc.
  void RemoveAllHierarchyChildrenNodes();

  /// ChildNodeAddedEvent is send when a child node added to this parent
  enum
    {
      ChildNodeAddedEvent = 15550,
      ChildNodeRemovedEvent = 15551
    };

  //// Assocailted node methods ////////////////

  ///
  /// String ID of the corresponding displayable DMML node
  virtual char* GetAssociatedNodeID()
  {
    return GetAssociatedNodeIDReference();
  }

  virtual void SetAssociatedNodeID(const char* ref);


  /// Get node associated with this hierarchy node
  virtual vtkDMMLNode* GetAssociatedNode();


  /// Find all associated children nodes of a specified class in the hierarchy
  /// if childClass is nullptr returns all associated children nodes.
  virtual void GetAssociatedChildrenNodes(vtkCollection *children, const char* childClass=nullptr);

  ///
  /// Get Hierarchy node for a given associated node
  static vtkDMMLHierarchyNode* GetAssociatedHierarchyNode(vtkDMMLScene *scene,
                                                          const char *associatedNodeID);
  ///
  /// Node's Sorting Value
  //vtkSetMacro(SortingValue, double);
  /// Use a method for Set because it needs to call modified on any associated
  //nodes (since the order of that associated node could have changed as well)
  void SetSortingValue(double value);
  vtkGetMacro(SortingValue, double);


  /// turn off if only want to have one child associated with this hierarchy
  /// node, as with the leaf type nodes that are pointing to a single dmml
  /// node. Used first in checking drag and drop targets. Default to true.
  vtkGetMacro(AllowMultipleChildren, int);
  vtkSetMacro(AllowMultipleChildren, int);
  vtkBooleanMacro(AllowMultipleChildren, int);

protected:
  vtkDMMLHierarchyNode();
  ~vtkDMMLHierarchyNode() override;
  vtkDMMLHierarchyNode(const vtkDMMLHierarchyNode&);
  void operator=(const vtkDMMLHierarchyNode&);


  ///
  /// String ID of the parent hierarchy DMML node
  void SetParentNodeIDReference(const char* id);
  vtkGetStringMacro(ParentNodeIDReference);

  char *ParentNodeIDReference;

  /// Mark hierarchy as modified when you
  static void HierarchyIsModified(vtkDMMLScene *scene);


  ///////////////////////

  /// Mark hierarchy as modified
  static void AssociatedHierarchyIsModified(vtkDMMLScene *scene);
  ///
  /// String ID of the associated DMML node
  char *AssociatedNodeIDReference;

  void SetAssociatedNodeIDReference(const char*);
  vtkGetStringMacro(AssociatedNodeIDReference);

  typedef std::map<std::string, std::vector< vtkDMMLHierarchyNode *> > HierarchyChildrenNodesType;

  static std::map< vtkDMMLScene*, HierarchyChildrenNodesType> SceneHierarchyChildrenNodes;
  static std::map< vtkDMMLScene*, vtkMTimeType> SceneHierarchyChildrenNodesMTime;

  ////////////////////////////
  ///
  /// Create Associated to hierarchy map,
  /// return number of Associated hierarchy nodes
  static int UpdateAssociatedToHierarchyMap(vtkDMMLScene *scene);

  typedef std::map<std::string, vtkDMMLHierarchyNode *> AssociatedHierarchyNodesType;

  static std::map< vtkDMMLScene*, AssociatedHierarchyNodesType> SceneAssociatedHierarchyNodes;

  static std::map< vtkDMMLScene*, vtkMTimeType> SceneAssociatedHierarchyNodesMTime;

  double SortingValue;

  static double MaximumSortingValue;

  void UpdateChildrenMap();

  /// is this a node that's only supposed to have one child?
  int AllowMultipleChildren;

  /// Invoke hierarchy modified event(s)
  /// It should be fired on any node that see its location changed relative to
  /// it parent, and also on a parent that add/remove/move a child
  /// Invoke the event on the passed node if not null, otherwise on the
  /// associated node if not null.
  void InvokeHierarchyModifiedEvent(vtkDMMLNode *node = nullptr);
};

#endif




