/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLTransformNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLHierarchyNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLHierarchyStorageNode.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkObjectFactory.h>

// STD includes
#include <algorithm>
#include <sstream>

vtkCxxSetReferenceStringMacro(vtkDMMLHierarchyNode, ParentNodeIDReference);
vtkCxxSetReferenceStringMacro(vtkDMMLHierarchyNode, AssociatedNodeIDReference);

typedef std::map<std::string, std::vector< vtkDMMLHierarchyNode *> > HierarchyChildrenNodesType;

std::map< vtkDMMLScene*, HierarchyChildrenNodesType> vtkDMMLHierarchyNode::SceneHierarchyChildrenNodes = std::map< vtkDMMLScene*, HierarchyChildrenNodesType>();
std::map< vtkDMMLScene*, vtkMTimeType> vtkDMMLHierarchyNode::SceneHierarchyChildrenNodesMTime = std::map< vtkDMMLScene*, vtkMTimeType>();

double vtkDMMLHierarchyNode::MaximumSortingValue = 0;

typedef std::map<std::string, vtkDMMLHierarchyNode *> AssociatedHierarchyNodesType;

std::map< vtkDMMLScene*, AssociatedHierarchyNodesType> vtkDMMLHierarchyNode::SceneAssociatedHierarchyNodes = std::map< vtkDMMLScene*, AssociatedHierarchyNodesType>();

std::map< vtkDMMLScene*, vtkMTimeType> vtkDMMLHierarchyNode::SceneAssociatedHierarchyNodesMTime = std::map< vtkDMMLScene*, vtkMTimeType>();;

typedef vtkDMMLHierarchyNode* const vtkDMMLHierarchyNodePointer;
bool vtkDMMLHierarchyNodeSortPredicate(vtkDMMLHierarchyNodePointer d1, vtkDMMLHierarchyNodePointer d2);
bool vtkDMMLHierarchyNodeSortPredicate(vtkDMMLHierarchyNodePointer d1, vtkDMMLHierarchyNodePointer d2)
{
  return d1->GetSortingValue() < d2->GetSortingValue();
}

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLHierarchyNode);

//----------------------------------------------------------------------------
vtkDMMLHierarchyNode::vtkDMMLHierarchyNode()
{
  this->HideFromEditors = 0;

  this->ParentNodeIDReference = nullptr;

  this->AssociatedNodeIDReference = nullptr;

  this->SortingValue = 0;

  this->AllowMultipleChildren = 1;
}

//----------------------------------------------------------------------------
vtkDMMLHierarchyNode::~vtkDMMLHierarchyNode()
{
  if (this->ParentNodeIDReference)
    {
    delete [] this->ParentNodeIDReference;
    this->ParentNodeIDReference = nullptr;
    }
  if (this->AssociatedNodeIDReference)
    {
    delete [] this->AssociatedNodeIDReference;
    this->AssociatedNodeIDReference = nullptr;
    }
}

//----------------------------------------------------------------------------
void vtkDMMLHierarchyNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  if (this->ParentNodeIDReference != nullptr)
    {
    of << " parentNodeRef=\"" << this->ParentNodeIDReference << "\"";
    }
  if (this->AssociatedNodeIDReference != nullptr)
    {
    of << " associatedNodeRef=\"" << this->AssociatedNodeIDReference << "\"";
    }
  of << " sortingValue=\"" << this->SortingValue << "\"";
  of << " allowMultipleChildren=\"" << (this->AllowMultipleChildren ? "true" : "false") << "\"";
}

//----------------------------------------------------------------------------
void vtkDMMLHierarchyNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID, newID);
  if (this->ParentNodeIDReference && !strcmp(oldID, this->ParentNodeIDReference))
    {
    this->SetParentNodeID(newID);
    }
  else if (this->AssociatedNodeIDReference && !strcmp(oldID, this->AssociatedNodeIDReference))
    {
    this->SetAssociatedNodeID(newID);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLHierarchyNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "parentNodeRef"))
      {
      // don't reset SortingValue
      double sortingValue = this->GetSortingValue();
      this->SetParentNodeID(attValue);
      this->SetSortingValue(sortingValue);
      //this->Scene->AddReferencedNodeID(this->ParentNodeIDReference, this);
      }
    if (!strcmp(attName, "associatedNodeRef"))
      {
      this->SetAssociatedNodeID(attValue);
      }
    else if (!strcmp(attName, "sortingValue"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> SortingValue;
      }
    else if (!strcmp(attName, "allowMultipleChildren"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->AllowMultipleChildren = 1;
        }
      else
        {
        this->AllowMultipleChildren = 0;
        }
      }
  }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkDMMLHierarchyNode::Copy(vtkDMMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkDMMLHierarchyNode *node = (vtkDMMLHierarchyNode *) anode;
  //this->SetParentNodeID(node->ParentNodeIDReference);
  this->SetParentNodeIDReference(node->ParentNodeIDReference);
  this->SetAssociatedNodeIDReference(node->AssociatedNodeIDReference);
  this->SortingValue = node->SortingValue;
  this->SetAllowMultipleChildren(node->AllowMultipleChildren);

  this->EndModify(disabledModify);
  this->InvokeHierarchyModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkDMMLHierarchyNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << indent << "AssociatedNodeID: " <<
    (this->AssociatedNodeIDReference ? this->AssociatedNodeIDReference : "(none)") << "\n";
  os << indent << "ParentNodeID: " <<
    (this->ParentNodeIDReference ? this->ParentNodeIDReference : "(none)") << "\n";
  os << indent << "SortingValue:     " << this->SortingValue << "\n";
  os << indent << "AllowMultipleChildren: " << (this->AllowMultipleChildren ? "true" : "false") << "\n";
}

//----------------------------------------------------------------------------
vtkDMMLHierarchyNode* vtkDMMLHierarchyNode::GetParentNode()
{
  vtkDMMLHierarchyNode* node = nullptr;
  if (this->GetScene() && this->ParentNodeIDReference != nullptr )
    {
    vtkDMMLNode* snode = this->GetScene()->GetNodeByID(this->ParentNodeIDReference);
    node = vtkDMMLHierarchyNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------
void vtkDMMLHierarchyNode::SetSceneReferences()
{
  this->Superclass::SetSceneReferences();
  this->Scene->AddReferencedNodeID(this->ParentNodeIDReference, this);
  this->Scene->AddReferencedNodeID(this->AssociatedNodeIDReference, this);
}

//-----------------------------------------------------------
void vtkDMMLHierarchyNode::UpdateScene(vtkDMMLScene *scene)
{
  Superclass::UpdateScene(scene);
}

//-----------------------------------------------------------
void vtkDMMLHierarchyNode::UpdateReferences()
{
  Superclass::UpdateReferences();

  if (this->ParentNodeIDReference != nullptr && this->Scene->GetNodeByID(this->ParentNodeIDReference) == nullptr)
    {
    this->SetParentNodeID(nullptr);
    }
  if (this->AssociatedNodeIDReference != nullptr && this->Scene->GetNodeByID(this->AssociatedNodeIDReference) == nullptr)
    {
    this->SetAssociatedNodeID(nullptr);
    }
}

//-----------------------------------------------------------
void vtkDMMLHierarchyNode::SetParentNodeID(const char* ref)
{
  // is it trying to set it's parent to be itself?
  if (this->GetID() && ref &&
      !strcmp(this->GetID(), ref))
    {
    vtkErrorMacro("SetParentNode: node "
                  << (this->GetName() ? this->GetName() : "(unnamed)")
                  << " with id " << this->GetID()
                  << " tried to set itself as parent! Returning...");
    return;
    }
  // is the parent node id already set to ref?
  if (this->ParentNodeIDReference && ref &&
      !strcmp(ref, this->ParentNodeIDReference))
    {
    return;
    }
  // or are both already null?
  if (!this->ParentNodeIDReference && !ref)
    {
    return;
    }

  vtkDMMLHierarchyNode *oldParentNode = this->GetParentNode();

  int disableModify = this->StartModify();

  this->SetParentNodeIDReference(ref);
  this->SetSortingValue(++MaximumSortingValue);

  this->HierarchyIsModified(this->GetScene());
  if (this->GetScene())
    {
    this->GetScene()->AddReferencedNodeID(ref, this);
    }
  this->EndModify(disableModify);

  vtkDMMLHierarchyNode *parentNode = this->GetParentNode();
  if (oldParentNode)
    {
    oldParentNode->InvokeEvent(vtkDMMLHierarchyNode::ChildNodeRemovedEvent, this);
    oldParentNode->Modified();
    }
  if (parentNode)
    {
    parentNode->InvokeEvent(vtkDMMLHierarchyNode::ChildNodeAddedEvent, this);
    parentNode->Modified();
    }
  if (this->GetScene())
    {
    this->GetScene()->Modified();
    }

  this->InvokeHierarchyModifiedEvent();
}

//----------------------------------------------------------------------------
vtkDMMLHierarchyNode* vtkDMMLHierarchyNode::GetTopParentNode()
{
  vtkDMMLHierarchyNode *node = nullptr;
  vtkDMMLHierarchyNode *parent = vtkDMMLHierarchyNode::SafeDownCast(this->GetParentNode());
  if (parent == nullptr)
    {
    node = this;
    }
  else
    {
    node =  parent->GetTopParentNode();
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkDMMLHierarchyNode::GetAllChildrenNodes(std::vector< vtkDMMLHierarchyNode *> &childrenNodes)
{
  if (this->GetScene() == nullptr)
    {
    return;
    }

  this->UpdateChildrenMap();

  std::map< vtkDMMLScene*, HierarchyChildrenNodesType>::iterator siter =
        SceneHierarchyChildrenNodes.find(this->GetScene());

  HierarchyChildrenNodesType::iterator iter =
    siter->second.find(std::string(this->GetID()));
  if (iter == siter->second.end())
    {
    return;
    }
  for (unsigned int i=0; i<iter->second.size(); i++)
    {
    childrenNodes.push_back(iter->second[i]);
    iter->second[i]->GetAllChildrenNodes(childrenNodes);
    }
}

//----------------------------------------------------------------------------
std::vector< vtkDMMLHierarchyNode *> vtkDMMLHierarchyNode::GetChildrenNodes()
{
  std::vector< vtkDMMLHierarchyNode *> childrenNodes;
  if (this->GetScene() == nullptr)
    {
    return childrenNodes;
    }

  this->UpdateChildrenMap();

  std::map< vtkDMMLScene*, HierarchyChildrenNodesType>::iterator siter =
        SceneHierarchyChildrenNodes.find(this->GetScene());

  HierarchyChildrenNodesType::iterator iter =
    siter->second.find(std::string(this->GetID()));
  if (iter == siter->second.end())
    {
    return childrenNodes;
    }
  for (unsigned int i=0; i<iter->second.size(); i++)
    {
    childrenNodes.push_back(iter->second[i]);
    }

  // Sort the vector using predicate and std::sort
  std::sort(childrenNodes.begin(), childrenNodes.end(), vtkDMMLHierarchyNodeSortPredicate);

  return childrenNodes;
}

//----------------------------------------------------------------------------
vtkDMMLHierarchyNode* vtkDMMLHierarchyNode::GetNthChildNode(int index)
{
  std::vector< vtkDMMLHierarchyNode *> childrenNodes = this->GetChildrenNodes();
  if (index < 0 || index > (int)(childrenNodes.size()-1))
    {
    vtkErrorMacro("vtkDMMLHierarchyNode::GetNthChildNode() index " << index << " outside the range 0-" << childrenNodes.size()-1 );
    return nullptr;
    }
  else
    {
    return childrenNodes[index];
    }
}

//----------------------------------------------------------------------------

int vtkDMMLHierarchyNode::GetIndexInParent()
{
  vtkDMMLHierarchyNode *pnode = this->GetParentNode();
  if (pnode == nullptr)
    {
    vtkErrorMacro("vtkDMMLHierarchyNode::GetIndexInParent() no parent");
    return -1;
    }
  else
    {
    std::vector< vtkDMMLHierarchyNode *> childrenNodes = pnode->GetChildrenNodes();
    for (unsigned int i=0; i<childrenNodes.size(); i++)
      {
      if (childrenNodes[i] == this)
        {
        return i;
        }
      }
    return -1;
    }
}

//----------------------------------------------------------------------------

void vtkDMMLHierarchyNode::SetIndexInParent(int index)
{
  vtkDMMLHierarchyNode *pnode = this->GetParentNode();
  if (pnode == nullptr)
    {
    vtkErrorMacro("vtkDMMLHierarchyNode::SetIndexInParent() no parent");
    return;
    }
  else
    {
    std::vector< vtkDMMLHierarchyNode *> childrenNodes = pnode->GetChildrenNodes();
    if (index < 0 || index >= (int)childrenNodes.size())
      {
      vtkErrorMacro("vtkDMMLHierarchyNode::SetIndexInParent() index " << index << ", outside the range 0-" << childrenNodes.size()-1);
      return;
      }
    double sortValue = childrenNodes[index]->GetSortingValue();
    int oldIndex = this->GetIndexInParent();
    if (index == 0)
      {
      sortValue -= 1;
      }
    else if (index == (int)childrenNodes.size()-1)
      {
      sortValue += 1;
      }
    else if (index > oldIndex)
      {
      sortValue = 0.5*(sortValue + childrenNodes[index+1]->GetSortingValue());
      }
    else if (index < oldIndex)
      {
      sortValue = 0.5*(sortValue + childrenNodes[index-1]->GetSortingValue());
      }
    this->SetSortingValue(sortValue);
    }

  return;
}

//----------------------------------------------------------------------------

void vtkDMMLHierarchyNode::MoveInParent(int increment)
{
  if (increment == 0)
    {
    return;
    }

  vtkDMMLHierarchyNode *pnode = this->GetParentNode();
  if (pnode == nullptr)
    {
    vtkErrorMacro("vtkDMMLHierarchyNode::MoveInParent() no parent");
    return;
    }
  else
    {
    std::vector< vtkDMMLHierarchyNode *> childrenNodes = pnode->GetChildrenNodes();
    int oldIndex = this->GetIndexInParent();
    if (oldIndex + increment < 0 || oldIndex + increment >= (int)childrenNodes.size())
      {
      vtkErrorMacro("vtkDMMLHierarchyNode::MoveInParent() index " << oldIndex << ", outside the range 0-" << childrenNodes.size()-1);
      return;
      }
    int incr1 = increment > 0 ? 1:-1;
    int index1 = oldIndex;
    int index2 = oldIndex + incr1;
    for (int i=0; i<incr1*increment; i++)
      {
      // swap pair of sort values
      childrenNodes = pnode->GetChildrenNodes();
      double sortValue1 = childrenNodes[index1]->GetSortingValue();
      double sortValue2 = childrenNodes[index2]->GetSortingValue();
      childrenNodes[index1]->SortingValue = sortValue2;
      childrenNodes[index2]->SortingValue = sortValue1;

      // update the cache
      pnode->UpdateChildrenMap();
      index1 += incr1;
      index2 += incr1;
      }
    // notify observers
    this->Modified();
    this->InvokeHierarchyModifiedEvent();
    }
}
//----------------------------------------------------------------------------

void vtkDMMLHierarchyNode::RemoveHierarchyChildrenNodes()
{
  if (this->GetScene() == nullptr)
    {
    return;
    }

  char *parentID = this->GetParentNodeID();
  vtkDMMLHierarchyNode *parentNode = this->GetParentNode();

  std::vector< vtkDMMLHierarchyNode *> children = this->GetChildrenNodes();
  for (unsigned int i=0; i<children.size(); i++)
    {
    vtkDMMLHierarchyNode *child = children[i];
    std::vector< vtkDMMLHierarchyNode *> childChildren = child->GetChildrenNodes();
    for (unsigned int j=0; j<childChildren.size(); j++)
      {
      childChildren[j]->SetParentNodeID(parentID);
      }
    if (parentNode)
      {
      parentNode->InvokeEvent(vtkDMMLHierarchyNode::ChildNodeRemovedEvent, child);
      }
    this->GetScene()->RemoveNode(child);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLHierarchyNode::RemoveAllHierarchyChildrenNodes()
{
  if (this->GetScene() == nullptr)
    {
    return;
    }

  std::vector< vtkDMMLHierarchyNode *> children = this->GetChildrenNodes();
  for (unsigned int i=0; i<children.size(); i++)
    {
    vtkDMMLHierarchyNode *child = children[i];
    std::vector< vtkDMMLHierarchyNode *> childChildren = child->GetChildrenNodes();
    for (unsigned int j=0; j<childChildren.size(); j++)
      {
      childChildren[j]->RemoveAllHierarchyChildrenNodes();
      }
    this->InvokeEvent(vtkDMMLHierarchyNode::ChildNodeRemovedEvent, child);
    this->GetScene()->RemoveNode(child);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLHierarchyNode::UpdateChildrenMap()
{
  if (this->GetScene() == nullptr)
    {
    this->SceneHierarchyChildrenNodes.clear();
    this->SceneHierarchyChildrenNodesMTime.clear();
    return;
    }

  std::map< vtkDMMLScene*, HierarchyChildrenNodesType>::iterator siter =
        SceneHierarchyChildrenNodes.find(this->GetScene());
  if (siter == SceneHierarchyChildrenNodes.end())
    {
    HierarchyChildrenNodesType h;
    SceneHierarchyChildrenNodes[this->GetScene()] = h;
    siter = SceneHierarchyChildrenNodes.find(this->GetScene());
    SceneHierarchyChildrenNodesMTime[this->GetScene()] = 0;
    }

  std::map< vtkDMMLScene*, vtkMTimeType>::iterator titer =
        SceneHierarchyChildrenNodesMTime.find(this->GetScene());

  std::map<std::string, std::vector< vtkDMMLHierarchyNode *> >::iterator iter;
  if (this->GetScene() == nullptr)
    {
    for (iter  = siter->second.begin();
         iter != siter->second.end();
         iter++)
      {
      iter->second.clear();
      }
    siter->second.clear();
    }

  if (this->GetScene()->GetNodes()->GetMTime() > titer->second)
  {
    for (iter  = siter->second.begin();
         iter != siter->second.end();
         iter++)
      {
      iter->second.clear();
      }
    siter->second.clear();

    std::vector<vtkDMMLNode *> nodes;
    int nnodes = this->GetScene()->GetNodesByClass("vtkDMMLHierarchyNode", nodes);

    double maxSortingValue = this->MaximumSortingValue;

    for (int i=0; i<nnodes; i++)
      {
      vtkDMMLHierarchyNode *node =  vtkDMMLHierarchyNode::SafeDownCast(nodes[i]);
      if (node)
        {
        vtkDMMLHierarchyNode *pnode = vtkDMMLHierarchyNode::SafeDownCast(node->GetParentNode());
        if (pnode)
          {
          if (pnode->GetSortingValue() > maxSortingValue)
            {
            maxSortingValue = pnode->GetSortingValue();
            }
          iter = siter->second.find(std::string(pnode->GetID()));
          if (iter == siter->second.end())
            {
            std::vector< vtkDMMLHierarchyNode *> children;
            children.push_back(node);
            siter->second[std::string(pnode->GetID())] = children;
            }
          else
            {
            iter->second.push_back(node);
            }
          }
        }
      }
    titer->second = this->GetScene()->GetNodes()->GetMTime();
    this->MaximumSortingValue = maxSortingValue;
  }
}

void vtkDMMLHierarchyNode::HierarchyIsModified(vtkDMMLScene *scene)
{
  if (scene == nullptr)
    {
    return;
    }

  SceneHierarchyChildrenNodesMTime[scene] = 0;
}

void vtkDMMLHierarchyNode::GetAssociatedChildrenNodes(vtkCollection *children,
                                                      const char* childClass)
{
  if (children == nullptr)
    {
    return;
    }
  vtkDMMLScene *scene = this->GetScene();
  if (scene == nullptr)
    {
    //vtkErrorMacro("GetChildrenAssociatedNodes: scene is null, cannot find children of this node");
    return;
    }
  vtkDMMLNode *mnode = nullptr;
  vtkDMMLHierarchyNode *hnode = nullptr;
  std::string nodeClass("vtkDMMLNode");
  if (childClass)
    {
    nodeClass = childClass;
    }

  int numNodes = scene->GetNumberOfNodesByClass(nodeClass.c_str());
  for (int n=0; n < numNodes; n++)
    {
    mnode = scene->GetNthNodeByClass(n, nodeClass.c_str());
    // check for a hierarchy node for this displayble node
    hnode = this->GetAssociatedHierarchyNode(this->GetScene(), mnode->GetID());
    while (hnode)
      {
      // hnode == this
      if (hnode->GetID() && this->GetID() &&
          strcmp(hnode->GetID(), this->GetID()) == 0)
        {
        children->AddItem(mnode);
        break;
        }
      // the hierarchy node for this node may not be the one we're checking
      // against, go up the tree
      hnode = vtkDMMLHierarchyNode::SafeDownCast(hnode->GetParentNode());
      }// end while
    }// end for
}

//---------------------------------------------------------------------------
vtkDMMLHierarchyNode* vtkDMMLHierarchyNode::GetAssociatedHierarchyNode(vtkDMMLScene *scene,
                                                                       const char *associatedNodeID)
{
  if (associatedNodeID == nullptr)
    {
    std::cerr << "GetAssociatedHierarchyNode: associated node id is null" << std::endl;
    return nullptr;
    }
  if (scene == nullptr)
    {
    std::cerr << "GetAssociatedHierarchyNode: scene is null" << std::endl;
    return nullptr;
    }

  vtkDMMLHierarchyNode::UpdateAssociatedToHierarchyMap(scene);

  std::map< vtkDMMLScene*, AssociatedHierarchyNodesType>::iterator siter =
        SceneAssociatedHierarchyNodes.find(scene);
  if (siter == SceneAssociatedHierarchyNodes.end())
    {
    std::cerr << "GetAssociatedHierarchyNode: didn't find an associated hierarchy node type associated with the scene" << std::endl;
    return nullptr;
    }

  std::map<std::string, vtkDMMLHierarchyNode *>::iterator iter;

  iter = siter->second.find(associatedNodeID);
  if (iter != siter->second.end())
    {
    return iter->second;
    }
  else
    {
    return nullptr;
    }
}

//----------------------------------------------------------------------------
int vtkDMMLHierarchyNode::UpdateAssociatedToHierarchyMap(vtkDMMLScene *scene)
{
  if (scene == nullptr)
    {
    SceneAssociatedHierarchyNodes.clear();
    SceneAssociatedHierarchyNodesMTime.clear();
    return 0;
    }

  std::map< vtkDMMLScene*, AssociatedHierarchyNodesType>::iterator siter =
        SceneAssociatedHierarchyNodes.find(scene);
  if (siter == SceneAssociatedHierarchyNodes.end())
    {
    AssociatedHierarchyNodesType h;
    SceneAssociatedHierarchyNodes[scene] = h;
    siter = SceneAssociatedHierarchyNodes.find(scene);
    SceneAssociatedHierarchyNodesMTime[scene] = 0;
    }

  std::map< vtkDMMLScene*, vtkMTimeType>::iterator titer =
        SceneAssociatedHierarchyNodesMTime.find(scene);

  if (scene->GetNodes()->GetMTime() > titer->second)
  {
    siter->second.clear();

    std::vector<vtkDMMLNode *> nodes;
    int nnodes = scene->GetNodesByClass("vtkDMMLHierarchyNode", nodes);

    for (int i=0; i<nnodes; i++)
      {
      vtkDMMLHierarchyNode *node =  vtkDMMLHierarchyNode::SafeDownCast(nodes[i]);
      if (node)
        {
        vtkDMMLNode *mnode = node->vtkDMMLHierarchyNode::GetAssociatedNode();
        if (mnode)
          {
          siter->second[std::string(mnode->GetID())] = node;
          }
        }
      }
    titer->second = scene->GetNodes()->GetMTime();
  }
  return static_cast<int>(siter->second.size());
}

//----------------------------------------------------------------------------
vtkDMMLNode* vtkDMMLHierarchyNode::GetAssociatedNode()
{
  vtkDMMLNode* node = nullptr;
  if (this->GetScene() && this->GetAssociatedNodeID() )
    {
    node = this->GetScene()->GetNodeByID(this->AssociatedNodeIDReference);
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkDMMLHierarchyNode::SetAssociatedNodeID(const char* ref)
{
  if ((this->AssociatedNodeIDReference && ref && strcmp(ref, this->AssociatedNodeIDReference)) ||
      (this->AssociatedNodeIDReference != ref))
    {
    this->SetAssociatedNodeIDReference(ref);
    this->AssociatedHierarchyIsModified(this->GetScene());
    if (this->Scene)
      {
      this->Scene->AddReferencedNodeID(ref, this);
      }
    vtkDMMLNode* node = this->GetAssociatedNode();
    if (node)
      {
//      node->Modified();
      this->InvokeHierarchyModifiedEvent(node);
      }
    }
}

//----------------------------------------------------------------------------
void vtkDMMLHierarchyNode::AssociatedHierarchyIsModified(vtkDMMLScene *scene)
{
  if (scene == nullptr)
    {
    return;
    }

  SceneAssociatedHierarchyNodesMTime[scene] = 0;
}

//----------------------------------------------------------------------------
void vtkDMMLHierarchyNode::SetSortingValue(double value)
{
  // reimplemented from vtkSet macro
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting SortingValue to " << value);
  if (this->SortingValue != value)
    {
    this->SortingValue = value;
    this->Modified();

    this->InvokeHierarchyModifiedEvent();
    }
}

//----------------------------------------------------------------------------
void vtkDMMLHierarchyNode::InvokeHierarchyModifiedEvent(vtkDMMLNode *node)
{
  if (node)
    {
    // invoke it on the passed node
    vtkDebugMacro("InvokeHierarchyModifiedEvent: passed node: " << node->GetID());
    node->InvokeEvent(vtkDMMLNode::HierarchyModifiedEvent);
    return;
    }
  // if there is an associated node, invoke a hierarchy modified on it to trigger updates in
  // q widgets
  vtkDMMLNode *associatedNode = this->GetAssociatedNode();
  if (associatedNode)
    {
    vtkDebugMacro("InvokeHierarchyModifiedEvent: associated node " << associatedNode->GetID());
    associatedNode->InvokeEvent(vtkDMMLNode::HierarchyModifiedEvent);
    }
  /*
  else
    {
    // there is no node associated with this hierarchy, are there children?
    if (this->GetNumberOfChildrenNodes())
      {
      // invoke it on the top level children
      std::vector< vtkDMMLHierarchyNode* > children = this->GetChildrenNodes();
      for (unsigned int i = 0; i < children.size(); ++i)
        {
        vtkDebugMacro("InvokeHierarchyModifiedEvent: child " << i << ", id = " << children[i]->GetID());
        children[i]->InvokeEvent(vtkDMMLNode::HierarchyModifiedEvent);
        }
      }
    }
  */
}
// End
