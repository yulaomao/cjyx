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
#include "vtkCodedEntry.h"
#include "vtkDMMLScalarVolumeDisplayNode.h"
#include "vtkDMMLScalarVolumeNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLVolumeArchetypeStorageNode.h"
#include "vtkDMMLVolumeSequenceStorageNode.h"

// VTK includes
#include <vtkDataArray.h>
#include <vtkObjectFactory.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkPointData.h>

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLScalarVolumeNode);
vtkCxxSetObjectMacro(vtkDMMLScalarVolumeNode, VoxelValueQuantity, vtkCodedEntry);
vtkCxxSetObjectMacro(vtkDMMLScalarVolumeNode, VoxelValueUnits, vtkCodedEntry);

//----------------------------------------------------------------------------
vtkDMMLScalarVolumeNode::vtkDMMLScalarVolumeNode()
{
  this->DefaultSequenceStorageNodeClassName = "vtkDMMLVolumeSequenceStorageNode";
}

//----------------------------------------------------------------------------
vtkDMMLScalarVolumeNode::~vtkDMMLScalarVolumeNode()
{
  this->SetVoxelValueQuantity(nullptr);
  this->SetVoxelValueUnits(nullptr);
}

//----------------------------------------------------------------------------
void vtkDMMLScalarVolumeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  if (this->GetVoxelValueQuantity())
    {
    of << " voxelValueQuantity=\"" << vtkDMMLNode::URLEncodeString(this->GetVoxelValueQuantity()->GetAsString().c_str()) << "\"";
    }
  if (this->GetVoxelValueUnits())
    {
    of << " voxelValueUnits=\"" << vtkDMMLNode::URLEncodeString(this->GetVoxelValueUnits()->GetAsString().c_str()) << "\"";
    }
}

//----------------------------------------------------------------------------
void vtkDMMLScalarVolumeNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  // For backward compatibility, we read the labelMap attribute and save it as a custom attribute.
  // This allows scene reader to detect that this node has to be converted to a segmentation node.
  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "labelMap"))
      {
      std::stringstream ss;
      int val;
      ss << attValue;
      ss >> val;
      if (val)
        {
        this->SetAttribute("LabelMap", "1");
        }
      }
    else if (!strcmp(attName, "voxelValueQuantity"))
      {
      vtkNew<vtkCodedEntry> entry;
      entry->SetFromString(vtkDMMLNode::URLDecodeString(attValue));
      this->SetVoxelValueQuantity(entry.GetPointer());
      }
    else if (!strcmp(attName, "voxelValueUnits"))
      {
      vtkNew<vtkCodedEntry> entry;
      entry->SetFromString(vtkDMMLNode::URLDecodeString(attValue));
      this->SetVoxelValueUnits(entry.GetPointer());
      }
    }

  this->EndModify(disabledModify);
}

//-----------------------------------------------------------
void vtkDMMLScalarVolumeNode::CreateNoneNode(vtkDMMLScene *scene)
{
  // Create a None volume RGBA of 0, 0, 0 so the filters won't complain
  // about missing input
  vtkNew<vtkImageData> id;
  id->SetDimensions(1, 1, 1);
  id->AllocateScalars(VTK_DOUBLE, 4);
  id->GetPointData()->GetScalars()->FillComponent(0, 0.0);
  id->GetPointData()->GetScalars()->FillComponent(1, 125.0);
  id->GetPointData()->GetScalars()->FillComponent(2, 0.0);
  id->GetPointData()->GetScalars()->FillComponent(3, 0.0);

  vtkNew<vtkDMMLScalarVolumeNode> n;
  n->SetName("None");
  // the scene will set the id
  n->SetAndObserveImageData(id.GetPointer());
  scene->AddNode(n.GetPointer());
}

//----------------------------------------------------------------------------
vtkDMMLScalarVolumeDisplayNode* vtkDMMLScalarVolumeNode::GetScalarVolumeDisplayNode()
{
  return vtkDMMLScalarVolumeDisplayNode::SafeDownCast(this->GetVolumeDisplayNode());
}

//----------------------------------------------------------------------------
void vtkDMMLScalarVolumeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  if (this->GetVoxelValueQuantity())
    {
    os << indent << "VoxelValueQuantity: " << this->GetVoxelValueQuantity()->GetAsPrintableString() << "\n";
    }
  if (this->GetVoxelValueUnits())
    {
    os << indent << "VoxelValueUnits: " << this->GetVoxelValueUnits()->GetAsPrintableString() << "\n";
    }
}

//---------------------------------------------------------------------------
vtkDMMLStorageNode* vtkDMMLScalarVolumeNode::CreateDefaultStorageNode()
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
void vtkDMMLScalarVolumeNode::CreateDefaultDisplayNodes()
{
  if (vtkDMMLScalarVolumeDisplayNode::SafeDownCast(this->GetDisplayNode())!=nullptr)
    {
    // display node already exists
    return;
    }
  if (this->GetScene()==nullptr)
    {
    vtkErrorMacro("vtkDMMLScalarVolumeNode::CreateDefaultDisplayNodes failed: scene is invalid");
    return;
    }
  vtkDMMLScalarVolumeDisplayNode* dispNode = vtkDMMLScalarVolumeDisplayNode::SafeDownCast(
    this->GetScene()->AddNewNodeByClass("vtkDMMLScalarVolumeDisplayNode") );
  // If color node is already specified (via default display node mechanism) then use that,
  // otherwise set the default color specified in this class.
  if (!dispNode->GetColorNodeID())
    {
    dispNode->SetDefaultColorMap();
    }
  this->SetAndObserveDisplayNodeID(dispNode->GetID());
}
