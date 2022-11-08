/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLVolumeNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

#include "vtkDMMLScene.h"
#include "vtkDMMLVectorVolumeDisplayNode.h"
#include "vtkDMMLVectorVolumeNode.h"
#include "vtkDMMLVolumeArchetypeStorageNode.h"

#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkImageExtractComponents.h"


//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLVectorVolumeNode);

//----------------------------------------------------------------------------
vtkDMMLVectorVolumeNode::vtkDMMLVectorVolumeNode() = default;

//----------------------------------------------------------------------------
vtkDMMLVectorVolumeNode::~vtkDMMLVectorVolumeNode() = default;

//----------------------------------------------------------------------------
void vtkDMMLVectorVolumeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkDMMLVectorVolumeNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkDMMLVectorVolumeNode::Copy(vtkDMMLNode *anode)
{
  Superclass::Copy(anode);
  //vtkDMMLVectorVolumeNode *node = (vtkDMMLVectorVolumeNode *) anode;
}

//----------------------------------------------------------------------------
void vtkDMMLVectorVolumeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
vtkDMMLVectorVolumeDisplayNode* vtkDMMLVectorVolumeNode::GetVectorVolumeDisplayNode()
{
  return vtkDMMLVectorVolumeDisplayNode::SafeDownCast(this->GetDisplayNode());
}

//----------------------------------------------------------------------------
vtkDMMLStorageNode* vtkDMMLVectorVolumeNode::CreateDefaultStorageNode()
{
  vtkDMMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkDMMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkDMMLVolumeArchetypeStorageNode"));
}

//----------------------------------------------------------------------------
void vtkDMMLVectorVolumeNode::CreateDefaultDisplayNodes()
{
  if (vtkDMMLVectorVolumeDisplayNode::SafeDownCast(this->GetDisplayNode())!=nullptr)
    {
    // display node already exists
    return;
    }
  if (this->GetScene()==nullptr)
    {
    vtkErrorMacro("vtkDMMLVectorVolumeNode::CreateDefaultDisplayNodes failed: scene is invalid");
    return;
    }
  vtkDMMLVectorVolumeDisplayNode* dispNode = vtkDMMLVectorVolumeDisplayNode::SafeDownCast(
    this->GetScene()->AddNewNodeByClass("vtkDMMLVectorVolumeDisplayNode") );
  dispNode->SetDefaultColorMap();
  this->SetAndObserveDisplayNodeID(dispNode->GetID());
}
