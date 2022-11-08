/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLVolumeNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/


#include "vtkObjectFactory.h"

#include "vtkDMMLDiffusionImageVolumeNode.h"
#include "vtkDMMLDiffusionWeightedVolumeNode.h"
#include "vtkDMMLScene.h"

vtkCxxSetReferenceStringMacro(vtkDMMLDiffusionImageVolumeNode, BaselineNodeID);
vtkCxxSetReferenceStringMacro(vtkDMMLDiffusionImageVolumeNode, MaskNodeID);
vtkCxxSetReferenceStringMacro(vtkDMMLDiffusionImageVolumeNode, DiffusionWeightedNodeID);

//------------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLDiffusionImageVolumeNode);

//----------------------------------------------------------------------------
vtkDMMLDiffusionImageVolumeNode::vtkDMMLDiffusionImageVolumeNode()
{
  this->BaselineNodeID = nullptr;
  this->MaskNodeID = nullptr;
  this->DiffusionWeightedNodeID = nullptr;
  //Pair of ID-pointer for observing the corresponding Display node.
  //this->DisplayNodeID = nullptr;
  this->Order = 2; //Second order Tensor
}

//----------------------------------------------------------------------------
vtkDMMLDiffusionImageVolumeNode::~vtkDMMLDiffusionImageVolumeNode()
{

  if (this->BaselineNodeID)
    {
    delete [] this->BaselineNodeID;
    this->BaselineNodeID = nullptr;
    }
  if (this->MaskNodeID)
    {
    delete [] this->MaskNodeID;
    this->MaskNodeID = nullptr;
    }

  if (this->DiffusionWeightedNodeID)
    {
    delete [] this->DiffusionWeightedNodeID;
    this->DiffusionWeightedNodeID = nullptr;
    }
   this->SetAndObserveDisplayNodeID(nullptr);
}

//----------------------------------------------------------------------------
void vtkDMMLDiffusionImageVolumeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  std::stringstream ss;
  if (this->BaselineNodeID != nullptr)
    {
    of << " baselineNodeRef=\"" << this->BaselineNodeID << "\"";
    }
  if (this->DiffusionWeightedNodeID != nullptr)
    {
    of << " diffusionWeightedNodeRef=\"" << this->DiffusionWeightedNodeID << "\"";
    }
  if (this->MaskNodeID != nullptr)
    {
    of << " maskNodeRef=\"" << this->MaskNodeID << "\"";
    }

}

//----------------------------------------------------------------------------
void vtkDMMLDiffusionImageVolumeNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    std::stringstream ss;
    ss<<attValue;

    if (!strcmp(attName, "baselineNodeRef"))
      {
      this->SetBaselineNodeID(attValue);
      }
    if (!strcmp(attName, "diffusionWeightedNodeRef"))
      {
      this->SetDiffusionWeightedNodeID(attValue);
      }
    if (!strcmp(attName, "maskNodeRef"))
      {
      this->SetMaskNodeID(attValue);
      }
  }

  this->EndModify(disabledModify);

}

//----------------------------------------------------------------------------
vtkDMMLVolumeNode* vtkDMMLDiffusionImageVolumeNode::GetBaselineNode()
{
  vtkDMMLVolumeNode* node = nullptr;
  if (this->GetScene() && this->GetBaselineNodeID() )
    {
    vtkDMMLNode* snode = this->GetScene()->GetNodeByID(this->BaselineNodeID);
    node = vtkDMMLVolumeNode::SafeDownCast(snode);
    }
  return node;
}

//----------------------------------------------------------------------------
vtkDMMLVolumeNode* vtkDMMLDiffusionImageVolumeNode::GetMaskNode()
{
  vtkDMMLVolumeNode* node = nullptr;
  if (this->GetScene() && this->GetMaskNodeID() )
    {
    vtkDMMLNode* snode = this->GetScene()->GetNodeByID(this->MaskNodeID);
    node = vtkDMMLVolumeNode::SafeDownCast(snode);
    }
  return node;
}


//----------------------------------------------------------------------------
vtkDMMLDiffusionWeightedVolumeNode* vtkDMMLDiffusionImageVolumeNode::GetDiffusionWeightedNode()
{
  vtkDMMLDiffusionWeightedVolumeNode* node = nullptr;
  if (this->GetScene() && this->GetDiffusionWeightedNodeID() )
    {
    vtkDMMLNode* snode = this->GetScene()->GetNodeByID(this->DiffusionWeightedNodeID);
    node = vtkDMMLDiffusionWeightedVolumeNode::SafeDownCast(snode);
    }
  return node;
}

//----------------------------------------------------------------------------
//vtkDMMLVolumeDisplayNode* vtkDMMLVolumeNode::GetDisplayNode()
//{
//  vtkDMMLDiffusionImageVolumeDisplayNode* node = nullptr;
//  if (this->GetScene() && this->GetDisplayNodeID() )
//    {
//    vtkDMMLNode* snode = this->GetScene()->GetNodeByID(this->DisplayNodeID);
//    node = vtkDMMLDiffusionImageVolumeDisplayNode::SafeDownCast(snode);
//    }
//  return node;
//}


//-----------------------------------------------------------
//void vtkDMMLDiffusionImageVolumeNode::UpdateScene(vtkDMMLScene *scene)
//{
//  Superclass::UpdateScene(scene);

//  if (this->GetDiffusionWeightedNodeID())
//    {
//    this->SetAndObserveDisplayNodeID(this->GetDiffusionWeightedNodeID());
//    }
//}

//----------------------------------------------------------------------------
void vtkDMMLDiffusionImageVolumeNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  this->Superclass::UpdateReferenceID(oldID,newID);
  if (this->BaselineNodeID && !strcmp(oldID, this->BaselineNodeID))
    {
    this->SetBaselineNodeID(newID);
    }
  if (this->MaskNodeID && !strcmp(oldID, this->MaskNodeID))
    {
    this->SetMaskNodeID(newID);
    }
  if (this->DiffusionWeightedNodeID && !strcmp(oldID, this->DiffusionWeightedNodeID))
    {
    this->SetDiffusionWeightedNodeID(newID);
    }
}

//-----------------------------------------------------------
void vtkDMMLDiffusionImageVolumeNode::UpdateReferences()
{
  Superclass::UpdateReferences();

if (this->BaselineNodeID != nullptr && this->Scene->GetNodeByID(this->BaselineNodeID) == nullptr)
    {
    this->SetBaselineNodeID(nullptr);
    }
if (this->MaskNodeID != nullptr && this->Scene->GetNodeByID(this->MaskNodeID) == nullptr)
    {
    this->SetMaskNodeID(nullptr);
    }
if (this->DiffusionWeightedNodeID != nullptr && this->Scene->GetNodeByID(this->DiffusionWeightedNodeID) == nullptr)
    {
    this->SetDiffusionWeightedNodeID(nullptr);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLDiffusionImageVolumeNode::ProcessDMMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessDMMLEvents(caller, event, callData);
}

//----------------------------------------------------------------------------
void vtkDMMLDiffusionImageVolumeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "DiffusionWeightedNodeID: " <<
    (this->DiffusionWeightedNodeID ? this->DiffusionWeightedNodeID : "(none)") << "\n";

  os << indent << "BaselineNodeID: " <<
    (this->BaselineNodeID ? this->BaselineNodeID : "(none)") << "\n";

  os << indent << "MaskNodeID: " <<
    (this->MaskNodeID ? this->MaskNodeID : "(none)") << "\n";

}



