/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLDisplayableHierarchyNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkDMMLDisplayableHierarchyNode_h
#define __vtkDMMLDisplayableHierarchyNode_h

#include "vtkDMMLHierarchyNode.h"
class vtkDMMLDisplayableNode;
class vtkDMMLDisplayNode;

class vtkCallbackCommand;

/// \brief DMML node to represent a hierarchy of displayable nodes
class VTK_DMML_EXPORT vtkDMMLDisplayableHierarchyNode : public vtkDMMLHierarchyNode
{
public:
  static vtkDMMLDisplayableHierarchyNode *New();
  vtkTypeMacro(vtkDMMLDisplayableHierarchyNode,vtkDMMLHierarchyNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //--------------------------------------------------------------------------
  /// DMMLNode methods
  //--------------------------------------------------------------------------

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
  /// Get node XML tag name (like Volume, DisplayableHierarchy)
  const char* GetNodeTagName() override {return "DisplayableHierarchy";}

  /// Set the display node as reference into the scene
  void SetSceneReferences() override;

  ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  void UpdateReferences() override;

  ///
  /// Finds the Displayable node and read the data
  void UpdateScene(vtkDMMLScene *scene) override;

  ///
  /// Update the stored reference to another node in the scene
  void UpdateReferenceID(const char *oldID, const char *newID) override;

  ///
  /// String ID of the corresponding displayable DMML node
  virtual char* GetDisplayableNodeID()
  {
    return this->GetAssociatedNodeID();
  }

  virtual void SetDisplayableNodeID(const char* ref)
  {
    this->SetAssociatedNodeID(ref);
  }

  void SetAndObserveDisplayNodeID(const char *DisplayNodeID);
  vtkGetStringMacro(DisplayNodeID);


  ///
  /// Get associated displayable DMML node
  vtkDMMLDisplayableNode* GetDisplayableNode();

  ///
  /// Get associated display DMML node
  vtkDMMLDisplayNode* GetDisplayNode();


  ///
  /// Indicates if the node is expanded
  vtkBooleanMacro(Expanded, int);
  vtkGetMacro(Expanded, int);
  vtkSetMacro(Expanded, int);

  ///
  /// Get the top parent node in the hierarchy which is not expanded
  vtkDMMLDisplayableHierarchyNode* GetCollapsedParentNode();

  ///
  /// Find all child displayable nodes in the hierarchy
  void GetChildrenDisplayableNodes(vtkCollection *children);

  ///
  /// Get Hierarchy node for a given displayable node
  static vtkDMMLDisplayableHierarchyNode* GetDisplayableHierarchyNode(vtkDMMLScene *scene,
                                                                      const char *displayableNodeID);


  /// Removes immediate children nodes, both hierarchy and corresponding displayable/display nodes
  /// their children are reparented to this parent node.
  void RemoveChildrenNodes();

  /// Removes all children hierarchy nodes both hierarchy and corresponding displayable/display nodes,
  /// including children of children, etc.
  void RemoveAllChildrenNodes();


  ///
  /// alternative method to propagate events generated in Display nodes
  void ProcessDMMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;

  /// DisplayModifiedEvent is generated when display node parameters is changed
  enum
    {
      DisplayModifiedEvent = 17000
    };


protected:
  vtkDMMLDisplayableHierarchyNode();
  ~vtkDMMLDisplayableHierarchyNode() override;
  vtkDMMLDisplayableHierarchyNode(const vtkDMMLDisplayableHierarchyNode&);
  void operator=(const vtkDMMLDisplayableHierarchyNode&);

  void SetDisplayNodeID(const char *);

  char *DisplayNodeID;

  vtkDMMLDisplayNode *DisplayNode;

  int Expanded;


};

#endif
