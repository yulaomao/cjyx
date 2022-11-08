/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLModelHierarchyNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLModelDisplayNode.h"
#include "vtkDMMLModelHierarchyNode.h"
#include "vtkDMMLModelNode.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLModelHierarchyNode);


//----------------------------------------------------------------------------
vtkDMMLModelHierarchyNode::vtkDMMLModelHierarchyNode()
{
  this->ModelDisplayNode = nullptr;
  this->HideFromEditors = 0;
}

//----------------------------------------------------------------------------
vtkDMMLModelHierarchyNode::~vtkDMMLModelHierarchyNode() = default;

//----------------------------------------------------------------------------
void vtkDMMLModelHierarchyNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

}

//----------------------------------------------------------------------------
void vtkDMMLModelHierarchyNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID, newID);
}

//----------------------------------------------------------------------------
void vtkDMMLModelHierarchyNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "modelNodeRef") ||
        !strcmp(attName, "modelNodeID") )
      {
      this->SetDisplayableNodeID(attValue);
      //this->Scene->AddReferencedNodeID(this->ModelNodeID, this);
      }
    }

  this->EndModify(disabledModify);
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkDMMLModelHierarchyNode::Copy(vtkDMMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
//  vtkDMMLModelHierarchyNode *node = (vtkDMMLModelHierarchyNode *) anode;

  this->EndModify(disabledModify);

}

//----------------------------------------------------------------------------
void vtkDMMLModelHierarchyNode::PrintSelf(ostream& os, vtkIndent indent)
{

  Superclass::PrintSelf(os,indent);

  if (this->ModelDisplayNode)
    {
    os << indent << "ModelDisplayNode ID = " <<
      (this->ModelDisplayNode->GetID() ? this->ModelDisplayNode->GetID() : "(none)") << "\n";
    }
}

//-----------------------------------------------------------
void vtkDMMLModelHierarchyNode::UpdateScene(vtkDMMLScene *scene)
{
  Superclass::UpdateScene(scene);

}

//-----------------------------------------------------------
void vtkDMMLModelHierarchyNode::UpdateReferences()
{
  Superclass::UpdateReferences();
}

vtkDMMLModelNode* vtkDMMLModelHierarchyNode::GetModelNode()
{
  vtkDMMLModelNode* node = vtkDMMLModelNode::SafeDownCast(
    this->GetAssociatedNode());
  return node;
}

//----------------------------------------------------------------------------
vtkDMMLModelDisplayNode* vtkDMMLModelHierarchyNode::GetModelDisplayNode()
{
  vtkDMMLModelDisplayNode* node = nullptr;
  vtkDMMLNode* snode = Superclass::GetDisplayNode();
  if (snode)
    {
    node = vtkDMMLModelDisplayNode::SafeDownCast(snode);
    }
  return node;
}



//---------------------------------------------------------------------------
void vtkDMMLModelHierarchyNode::ProcessDMMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessDMMLEvents(caller, event, callData);

  vtkDMMLModelDisplayNode *dnode = this->GetModelDisplayNode();
  if (dnode != nullptr && dnode == vtkDMMLModelDisplayNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, nullptr);
    }
  return;
}

//----------------------------------------------------------------------------
vtkDMMLModelHierarchyNode* vtkDMMLModelHierarchyNode::GetCollapsedParentNode()
{
  vtkDMMLModelHierarchyNode *node = nullptr;
  vtkDMMLDisplayableHierarchyNode *dhnode = Superclass::GetCollapsedParentNode();
  if (dhnode != nullptr)
    {
    node = vtkDMMLModelHierarchyNode::SafeDownCast(dhnode);
    }
  return node;
}



//---------------------------------------------------------------------------
void vtkDMMLModelHierarchyNode:: GetChildrenModelNodes(vtkCollection *models)
{
  if (models == nullptr)
    {
    return;
    }
  vtkDMMLScene *scene = this->GetScene();
  vtkDMMLNode *mnode = nullptr;
  vtkDMMLModelHierarchyNode *hnode = nullptr;
  for (int n=0; n < scene->GetNumberOfNodes(); n++)
    {
    mnode = scene->GetNthNode(n);
    if (mnode->IsA("vtkDMMLModelNode"))
      {
      hnode = vtkDMMLModelHierarchyNode::SafeDownCast(
          vtkDMMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(scene, mnode->GetID()));
      while (hnode)
        {
        if (hnode == this)
          {
          models->AddItem(mnode);
          break;
          }
          hnode = vtkDMMLModelHierarchyNode::SafeDownCast(hnode->GetParentNode());
        }// end while
      }// end if
    }// end for
}


