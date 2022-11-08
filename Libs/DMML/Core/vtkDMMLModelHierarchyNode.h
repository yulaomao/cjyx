/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLModelHierarchyNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkDMMLModelHierarchyNode_h
#define __vtkDMMLModelHierarchyNode_h

#include "vtkDMMLDisplayableHierarchyNode.h"
class vtkDMMLModelDisplayNode;
class vtkDMMLModelNode;

/// \brief DMML node to represent a hierarchyu of models.
class VTK_DMML_EXPORT vtkDMMLModelHierarchyNode : public vtkDMMLDisplayableHierarchyNode
{
public:
  static vtkDMMLModelHierarchyNode *New();
  vtkTypeMacro(vtkDMMLModelHierarchyNode,vtkDMMLDisplayableHierarchyNode);
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
  /// Get node XML tag name (like Volume, ModelHierarchy)
  const char* GetNodeTagName() override {return "ModelHierarchy";}

   ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  void UpdateReferences() override;

  ///
  /// Finds the model node and read the data
  void UpdateScene(vtkDMMLScene *scene) override;

  ///
  /// Update the stored reference to another node in the scene
  void UpdateReferenceID(const char *oldID, const char *newID) override;

  ///
  /// String ID of the model DMML node
  void SetModelNodeID(const char* id)
  {
    this->SetDisplayableNodeID(id);
  }

  char *GetModelNodeID()
  {
    return this->GetDisplayableNodeID();
  }

  /// Need this for tcl wrapping to call ReferenceStringMacro methods
  void SetModelNodeIDReference(const char* ref) {
    this->SetModelNodeID(ref);
  };


  ///
  /// Get associated model DMML node
  vtkDMMLModelNode* GetModelNode();

  ///
  /// Get associated display DMML node
  vtkDMMLModelDisplayNode* GetModelDisplayNode();


  ///
  /// Get the first parent node in hierarchy which is not expanded
  vtkDMMLModelHierarchyNode* GetCollapsedParentNode();

  ///
  /// Find all child model nodes in the hierarchy
  void GetChildrenModelNodes(vtkCollection *models);

  ///
  /// alternative method to propagate events generated in Display nodes
  void ProcessDMMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;


protected:
  vtkDMMLModelHierarchyNode();
  ~vtkDMMLModelHierarchyNode() override;
  vtkDMMLModelHierarchyNode(const vtkDMMLModelHierarchyNode&);
  void operator=(const vtkDMMLModelHierarchyNode&);


  /// Data

  vtkDMMLModelDisplayNode *ModelDisplayNode;

};

#endif
