/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLVolumeNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/


// DMML includes
#include "vtkDMMLDiffusionTensorVolumeDisplayNode.h"
#include "vtkDMMLDiffusionTensorVolumeNode.h"
#include "vtkDMMLNRRDStorageNode.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

//------------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLDiffusionTensorVolumeNode);

//----------------------------------------------------------------------------
vtkDMMLDiffusionTensorVolumeNode::vtkDMMLDiffusionTensorVolumeNode()
{
  this->Order = 2; //Second order Tensor
}

//----------------------------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeNode::SetAndObserveDisplayNodeID(const char *displayNodeID)
{
  this->Superclass::SetAndObserveDisplayNodeID(displayNodeID);
  // Make sure the node added is a DiffusionTensorVolumeDisplayNode
  vtkDMMLNode* displayNode =  this->GetDisplayNode();
  if (displayNode && !vtkDMMLDiffusionTensorVolumeDisplayNode::SafeDownCast(displayNode))
    {
    vtkWarningMacro("SetAndObserveDisplayNodeID: The node to display "
                    << displayNodeID << " can NOT display diffusion tensors");
    }
}

//----------------------------------------------------------------------------
vtkDMMLDiffusionTensorVolumeNode::~vtkDMMLDiffusionTensorVolumeNode() = default;

//----------------------------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
vtkDMMLDiffusionTensorVolumeDisplayNode* vtkDMMLDiffusionTensorVolumeNode
::GetDiffusionTensorVolumeDisplayNode()
{
  return vtkDMMLDiffusionTensorVolumeDisplayNode::SafeDownCast(this->GetDisplayNode());
}

//----------------------------------------------------------------------------
vtkDMMLStorageNode* vtkDMMLDiffusionTensorVolumeNode::CreateDefaultStorageNode()
{
  vtkDMMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkDMMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkDMMLNRRDStorageNode"));
}

//----------------------------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeNode::CreateDefaultDisplayNodes()
{
  if (vtkDMMLDiffusionTensorVolumeDisplayNode::SafeDownCast(this->GetDisplayNode())!=nullptr)
    {
    // display node already exists
    return;
    }
  if (this->GetScene()==nullptr)
    {
    vtkErrorMacro("vtkDMMLDiffusionTensorVolumeNode::CreateDefaultDisplayNodes failed: scene is invalid");
    return;
    }
  vtkDMMLDiffusionTensorVolumeDisplayNode* dispNode = vtkDMMLDiffusionTensorVolumeDisplayNode::SafeDownCast(
    this->GetScene()->AddNewNodeByClass("vtkDMMLDiffusionTensorVolumeDisplayNode") );
  dispNode->SetDefaultColorMap();
  this->SetAndObserveDisplayNodeID(dispNode->GetID());
  // add slice display nodes
  dispNode->AddSliceGlyphDisplayNodes( this );
}
