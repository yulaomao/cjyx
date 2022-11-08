/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLDisplayableHierarchyLogic.cxx,v $
  Date:      $Date: 2010-02-15 16:35:35 -0500 (Mon, 15 Feb 2010) $
  Version:   $Revision: 12142 $

=========================================================================auto=*/

// DMMLLogic includes
#include "vtkDMMLDisplayableHierarchyLogic.h"

// DMML includes
#include "vtkDMMLDisplayableHierarchyNode.h"
#include "vtkDMMLDisplayableNode.h"
#include "vtkDMMLDisplayNode.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkDMMLDisplayableHierarchyLogic);

//----------------------------------------------------------------------------
vtkDMMLDisplayableHierarchyLogic::vtkDMMLDisplayableHierarchyLogic() = default;

//----------------------------------------------------------------------------
vtkDMMLDisplayableHierarchyLogic::~vtkDMMLDisplayableHierarchyLogic() = default;

//----------------------------------------------------------------------------
void vtkDMMLDisplayableHierarchyLogic::SetDMMLSceneInternal(vtkDMMLScene* newScene)
{
  vtkNew<vtkIntArray> sceneEvents;
  sceneEvents->InsertNextValue(vtkDMMLScene::NodeRemovedEvent);
  this->SetAndObserveDMMLSceneEventsInternal(newScene, sceneEvents.GetPointer());
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableHierarchyLogic::OnDMMLSceneNodeRemoved(vtkDMMLNode* node)
{
  vtkDMMLDisplayableNode* displayableNode = vtkDMMLDisplayableNode::SafeDownCast(node);
  if (!displayableNode || this->GetDMMLScene()->IsBatchProcessing())
    {
    return;
    }
  // A displayable hierarchy node without children as well as a displayable
  // node is useless node. Delete it.
  vtkDMMLDisplayableHierarchyNode* displayableHierarchyNode = vtkDMMLDisplayableHierarchyNode::SafeDownCast(
    vtkDMMLHierarchyNode::GetAssociatedHierarchyNode(this->GetDMMLScene(), node->GetID()) );
  if (displayableHierarchyNode &&
      displayableHierarchyNode->GetNumberOfChildrenNodes() == 0)
    {
    this->GetDMMLScene()->RemoveNode(displayableHierarchyNode);
    }
}


//----------------------------------------------------------------------------
char *vtkDMMLDisplayableHierarchyLogic::AddDisplayableHierarchyNodeForNode(vtkDMMLDisplayableNode *node)
{
  char *hierarchyNodeID = nullptr;

  if (!node)
    {
    vtkErrorMacro("AddDisplayableHierarchyNodeForNode: null node!");
    return hierarchyNodeID;
    }
  if (!node->GetScene())
    {
    vtkErrorMacro("AddDisplayableHierarchyNodeForNode: node isn't in a scene!");
    return hierarchyNodeID;
    }
  vtkDMMLDisplayableHierarchyNode *hierarchyNode = nullptr;
  hierarchyNode = vtkDMMLDisplayableHierarchyNode::New();
  // it's a stealth node:
  hierarchyNode->HideFromEditorsOn();

  // give it a unique name based on the node
  std::string hnodeName = std::string(node->GetName()) + std::string(" Hierarchy");
  hierarchyNode->SetName(node->GetScene()->GetUniqueNameByString(hnodeName.c_str()));

  node->GetScene()->AddNode(hierarchyNode);
  // with a parent node id of null, it's a child of the scene

  // now point to the  node, need disable modified event to avoid an assert in qDMMLSceneModel
  node->SetDisableModifiedEvent(1);
  hierarchyNode->SetDisplayableNodeID(node->GetID());
  node->SetDisableModifiedEvent(0);

  // save the id for return
  hierarchyNodeID = hierarchyNode->GetID();

  // clean up
  hierarchyNode->Delete();

  return hierarchyNodeID;
}

//----------------------------------------------------------------------------
bool vtkDMMLDisplayableHierarchyLogic::AddChildToParent(vtkDMMLDisplayableNode *child, vtkDMMLDisplayableNode *parent)
{
  if (!child)
    {
    vtkErrorMacro("AddChildToParent: null child node");
    return false;
    }
  if (!child->GetScene())
    {
    vtkErrorMacro("AddChildToParent: child is not in a scene");
    return false;
    }
  if (!parent)
    {
    vtkErrorMacro("AddChildToParent: null parent node");
    return false;
    }
  if (!parent->GetScene())
    {
    vtkErrorMacro("AddChildToParent: parent is not in a scene");
    return false;
    }

  // does the parent already have a hierarchy node associated with it?
  char *parentHierarchyNodeID = nullptr;
  vtkDMMLDisplayableHierarchyNode *hierarchyNode = vtkDMMLDisplayableHierarchyNode::SafeDownCast(
    vtkDMMLHierarchyNode::GetAssociatedHierarchyNode(parent->GetScene(), parent->GetID()) );
  if (!hierarchyNode)
    {
    // create one and add to the scene
    parentHierarchyNodeID = this->AddDisplayableHierarchyNodeForNode(parent);
    }
  else
    {
    parentHierarchyNodeID = hierarchyNode->GetID();
    }
  if (!parentHierarchyNodeID)
    {
    vtkWarningMacro("AddChildToParent: unable to add or find a hierarchy node for the parent node " << parent->GetID() << ", so unable to place the child in a hierarchy");
    return false;
    }

  // does the child already have a hierarchy node associated with it?
  vtkDMMLDisplayableHierarchyNode *childHierarchyNode = vtkDMMLDisplayableHierarchyNode::SafeDownCast(
    vtkDMMLHierarchyNode::GetAssociatedHierarchyNode(child->GetScene(), child->GetID()) );
  if (!childHierarchyNode)
    {
    char *childHierarchyNodeID = this->AddDisplayableHierarchyNodeForNode(child);
    if (childHierarchyNodeID)
      {
      vtkDMMLNode *dmmlNode = child->GetScene()->GetNodeByID(childHierarchyNodeID);
      if (dmmlNode)
        {
        childHierarchyNode = vtkDMMLDisplayableHierarchyNode::SafeDownCast(dmmlNode);
        }
      }
    }
  if (childHierarchyNode)
    {
    std::cout << "AddChildToParent: parentHierarchyID = " << parentHierarchyNodeID << ", childHierarchyNodeID = " << childHierarchyNode->GetID() << std::endl;
    // disable modified events on the parent
    vtkDMMLNode *parentNode = childHierarchyNode->GetScene()->GetNodeByID(parentHierarchyNodeID);
    parentNode->SetDisableModifiedEvent(1);
    childHierarchyNode->SetParentNodeID(parentHierarchyNodeID);
    parentNode->SetDisableModifiedEvent(0);

    return true;
    }
  else
    {
    vtkWarningMacro("AddChildToParent: unable to add or find a hierarchy node for the child node " << child->GetID() << ", so unable to place it in a hierarchy");
    return false;
    }
  return false;
}

//----------------------------------------------------------------------------
bool vtkDMMLDisplayableHierarchyLogic::DeleteHierarchyNodeAndChildren(vtkDMMLDisplayableHierarchyNode *hnode)
{
  if (!hnode)
    {
    vtkErrorMacro("DeleteHierarchyNodeAndChildren: no hierarchy node given");
    return false;
    }
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("DeleteHierarchyNodeAndChildren: no scene defined on this class");
    return false;
    }

  // first off, set up batch processing mode on the scene
  this->GetDMMLScene()->StartState(vtkDMMLScene::BatchProcessState);

  // get all the children nodes
  std::vector< vtkDMMLHierarchyNode *> allChildren;
  hnode->GetAllChildrenNodes(allChildren);

  // and loop over them
  for (unsigned int i = 0; i < allChildren.size(); ++i)
    {
    vtkDMMLDisplayableHierarchyNode *dispHierarchyNode = vtkDMMLDisplayableHierarchyNode::SafeDownCast(allChildren[i]);
    if (dispHierarchyNode)
      {
      // get any associated node
      vtkDMMLNode *associatedNode = dispHierarchyNode->GetAssociatedNode();
      if (associatedNode)
        {
        this->GetDMMLScene()->RemoveNode(associatedNode);
        }
      // remove the display node (hierarchy nodes aren't displayable so the
      // scene doesn't do the housekeeping automatically)
      vtkDMMLDisplayNode *dispDisplayNode = dispHierarchyNode->GetDisplayNode();
      if (dispDisplayNode)
        {
        this->GetDMMLScene()->RemoveNode(dispDisplayNode);
        }
      this->GetDMMLScene()->RemoveNode(dispHierarchyNode);
      }
    }
  // sanity check
  bool retval = true;
  if (hnode->GetNumberOfChildrenNodes() != 0)
    {
    vtkErrorMacro("Failed to delete all children hierarchy nodes! Still have " << hnode->GetNumberOfChildrenNodes() << " left");
    retval = false;
    }
  // delete it's display node
  vtkDMMLDisplayNode *dispNode = hnode->GetDisplayNode();
  if (dispNode)
    {
    this->GetDMMLScene()->RemoveNode(dispNode);
    }
  // and then delete itself
  this->GetDMMLScene()->RemoveNode(hnode);

  // end batch processing
  this->GetDMMLScene()->EndState(vtkDMMLScene::BatchProcessState);

  return retval;
}
