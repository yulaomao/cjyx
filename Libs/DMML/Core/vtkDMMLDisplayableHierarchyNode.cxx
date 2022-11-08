/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLDisplayableHierarchyNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLDisplayNode.h"
#include "vtkDMMLDisplayableNode.h"
#include "vtkDMMLDisplayableHierarchyNode.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkCxxSetReferenceStringMacro(vtkDMMLDisplayableHierarchyNode, DisplayNodeID);

//------------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLDisplayableHierarchyNode);

//----------------------------------------------------------------------------
vtkDMMLDisplayableHierarchyNode::vtkDMMLDisplayableHierarchyNode()
{
  this->DisplayNodeID = nullptr;
  this->DisplayNode = nullptr;
  this->HideFromEditors = 1;
  this->Expanded = 1;
}

//----------------------------------------------------------------------------
vtkDMMLDisplayableHierarchyNode::~vtkDMMLDisplayableHierarchyNode()
{
  this->SetAndObserveDisplayNodeID( nullptr);
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableHierarchyNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  if (this->DisplayNodeID != nullptr)
    {
    of << " displayNodeID=\"" << this->DisplayNodeID << "\"";
    }

  of << " expanded=\"" << (this->Expanded ? "true" : "false") << "\"";
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableHierarchyNode::SetSceneReferences()
{
  this->Superclass::SetSceneReferences();
  this->Scene->AddReferencedNodeID(this->DisplayNodeID, this);
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableHierarchyNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID, newID);
  if (this->DisplayNodeID == nullptr || !strcmp(oldID, this->DisplayNodeID))
    {
    this->SetDisplayNodeID(newID);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableHierarchyNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "displayableNodeID"))
      {
      this->SetDisplayableNodeID(attValue);
      }
    else if (!strcmp(attName, "displayNodeRef") ||
             !strcmp(attName, "displayNodeID"))
      {
      this->SetDisplayNodeID(attValue);
      }
    else if (!strcmp(attName, "expanded"))
        {
        if (!strcmp(attValue,"true"))
          {
          this->Expanded = 1;
          }
        else
          {
          this->Expanded = 0;
          }
        }
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkDMMLDisplayableHierarchyNode::Copy(vtkDMMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkDMMLDisplayableHierarchyNode *node = (vtkDMMLDisplayableHierarchyNode *) anode;

  this->SetDisplayNodeID(node->DisplayNodeID);
  this->SetExpanded(node->Expanded);
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableHierarchyNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "DisplayNodeID: " <<
    (this->DisplayNodeID ? this->DisplayNodeID : "(none)") << "\n";
  os << indent << "Expanded:        " << this->Expanded << "\n";

  vtkNew<vtkCollection> col;
  this->GetChildrenDisplayableNodes(col.GetPointer());
  unsigned int numChildren = col->GetNumberOfItems();
  os << indent << "Number of children displayable nodes = " << numChildren << "\n";
  for (unsigned int i = 0; i < numChildren; i++)
    {
    vtkDMMLDisplayableNode *child = vtkDMMLDisplayableNode::SafeDownCast(col->GetItemAsObject(i));
    if (child)
      {
      os << indent.GetNextIndent() << i << "th child id = " << (child->GetID() ? child->GetID() : "NULL") << "\n";
      }
    }
}

//-----------------------------------------------------------
void vtkDMMLDisplayableHierarchyNode::UpdateScene(vtkDMMLScene *scene)
{
  Superclass::UpdateScene(scene);
  this->SetAndObserveDisplayNodeID(this->GetDisplayNodeID());

}

//-----------------------------------------------------------
void vtkDMMLDisplayableHierarchyNode::UpdateReferences()
{
  Superclass::UpdateReferences();

  if (this->Scene == nullptr)
    {
    return;
    }
  if (this->DisplayNodeID != nullptr && this->Scene->GetNodeByID(this->DisplayNodeID) == nullptr)
    {
    this->SetAndObserveDisplayNodeID(nullptr);
    }
}

//----------------------------------------------------------------------------
vtkDMMLDisplayableNode* vtkDMMLDisplayableHierarchyNode::GetDisplayableNode()
{
  vtkDMMLDisplayableNode* node = vtkDMMLDisplayableNode::SafeDownCast(this->GetAssociatedNode());
  return node;
}

//----------------------------------------------------------------------------
vtkDMMLDisplayNode* vtkDMMLDisplayableHierarchyNode::GetDisplayNode()
{
  vtkDMMLDisplayNode* node = nullptr;
  if (this->GetScene() && this->GetDisplayNodeID() )
    {
    vtkDMMLNode* snode = this->GetScene()->GetNodeByID(this->DisplayNodeID);
    node = vtkDMMLDisplayNode::SafeDownCast(snode);
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableHierarchyNode::SetAndObserveDisplayNodeID(const char *displayNodeID)
{
  vtkSetAndObserveDMMLObjectMacro(this->DisplayNode, nullptr);

  this->SetDisplayNodeID(displayNodeID);

  vtkDMMLDisplayNode *dnode = this->GetDisplayNode();

  vtkSetAndObserveDMMLObjectMacro(this->DisplayNode, dnode);

  if (this->Scene)
    {
    this->Scene->AddReferencedNodeID(displayNodeID, this);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLDisplayableHierarchyNode::ProcessDMMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessDMMLEvents(caller, event, callData);

  vtkDMMLDisplayNode *dnode = this->GetDisplayNode();
  if (dnode != nullptr && dnode == vtkDMMLDisplayNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkDMMLDisplayableHierarchyNode::DisplayModifiedEvent, nullptr);
    }
  return;
}

//----------------------------------------------------------------------------
vtkDMMLDisplayableHierarchyNode* vtkDMMLDisplayableHierarchyNode::GetCollapsedParentNode()
{
  // initialize the return node to null, if there are no collapsed hierarchy
  // nodes, returns null
  vtkDMMLDisplayableHierarchyNode *node = nullptr;

  // build up a vector of collapsed parents
  std::vector< vtkDMMLDisplayableHierarchyNode * > collapsedParents;
  if (!this->GetExpanded())
    {
    collapsedParents.push_back(this);
    }
  vtkDMMLDisplayableHierarchyNode *parent = vtkDMMLDisplayableHierarchyNode::SafeDownCast(this->GetParentNode());
  while (parent)
    {
    if (!parent->GetExpanded())
      {
      collapsedParents.push_back(parent);
      }
    parent = vtkDMMLDisplayableHierarchyNode::SafeDownCast(parent->GetParentNode());
    }
  // return the last collapsed parent
  if (collapsedParents.size() != 0)
    {
    node = collapsedParents.back();
    }
  return node;
}

//---------------------------------------------------------------------------
void vtkDMMLDisplayableHierarchyNode::GetChildrenDisplayableNodes(vtkCollection *children)
{
  this->GetAssociatedChildrenNodes(children, "vtkDMMLDisplayableNode");
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableHierarchyNode::RemoveChildrenNodes()
{
  if (this->GetScene() == nullptr)
    {
    return;
    }

  std::vector< vtkDMMLHierarchyNode *> children = this->GetChildrenNodes();
  for (unsigned int i=0; i<children.size(); i++)
    {
    vtkDMMLDisplayableHierarchyNode *child = vtkDMMLDisplayableHierarchyNode::SafeDownCast(children[i]);
    if (child)
      {
      std::vector< vtkDMMLHierarchyNode *> childChildern = child->GetChildrenNodes();
      vtkDMMLDisplayableNode *dnode = child->GetDisplayableNode();
      if (dnode)
        {
        this->GetScene()->RemoveNode(dnode);
        }
      vtkDMMLDisplayNode *disnode = child->GetDisplayNode();
      if (disnode)
        {
        this->GetScene()->RemoveNode(disnode);
        }
      }
    }
  this->RemoveHierarchyChildrenNodes();
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableHierarchyNode::RemoveAllChildrenNodes()
{
  if (this->GetScene() == nullptr)
    {
    return;
    }

  std::vector< vtkDMMLHierarchyNode *> children = this->GetChildrenNodes();
  for (unsigned int i=0; i<children.size(); i++)
    {
    vtkDMMLDisplayableHierarchyNode *child = vtkDMMLDisplayableHierarchyNode::SafeDownCast(children[i]);
    if (child)
      {
      child->RemoveAllChildrenNodes();

      std::vector< vtkDMMLHierarchyNode *> childChildern = child->GetChildrenNodes();
      vtkDMMLDisplayableNode *dnode = child->GetDisplayableNode();
      if (dnode)
        {
        this->GetScene()->RemoveNode(dnode);
        }
      vtkDMMLDisplayNode *disnode = child->GetDisplayNode();
      if (disnode)
        {
        this->GetScene()->RemoveNode(disnode);
        }
      }
    }
  this->RemoveAllHierarchyChildrenNodes();

}

//----------------------------------------------------------------------------
vtkDMMLDisplayableHierarchyNode*
vtkDMMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(vtkDMMLScene *scene,
                                                             const char *displayableNodeID)
{
  return vtkDMMLDisplayableHierarchyNode::SafeDownCast(
    vtkDMMLHierarchyNode::GetAssociatedHierarchyNode(scene,displayableNodeID));
}
