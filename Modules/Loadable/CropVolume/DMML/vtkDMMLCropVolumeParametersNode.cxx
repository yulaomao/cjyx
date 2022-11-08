/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

// VTK includes
#include <vtkCommand.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// DMML includes
#include "vtkDMMLVolumeNode.h"
#include "vtkDMMLTransformNode.h"

// CropModuleDMML includes
#include "vtkDMMLCropVolumeParametersNode.h"

// AnnotationModuleDMML includes
#include "vtkDMMLDisplayableNode.h"

// STD includes

static const char* InputVolumeNodeReferenceRole = "inputVolume";
static const char* InputVolumeNodeReferenceDMMLAttributeName = "inputVolumeNodeID";
static const char* OutputVolumeNodeReferenceRole = "outputVolume";
static const char* OutputVolumeNodeReferenceDMMLAttributeName = "outputVolumeNodeID";
static const char* ROINodeReferenceRole = "roi";
static const char* ROINodeReferenceDMMLAttributeName = "ROINodeID";
static const char* ROIAlignmentTransformNodeReferenceRole = "roiAlignmentTransform";

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLCropVolumeParametersNode);

//----------------------------------------------------------------------------
vtkDMMLCropVolumeParametersNode::vtkDMMLCropVolumeParametersNode()
{
  this->HideFromEditors = 1;

  vtkNew<vtkIntArray> inputVolumeEvents;
  inputVolumeEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  inputVolumeEvents->InsertNextValue(vtkDMMLVolumeNode::ImageDataModifiedEvent);
  this->AddNodeReferenceRole(InputVolumeNodeReferenceRole,
    InputVolumeNodeReferenceDMMLAttributeName,
    inputVolumeEvents.GetPointer());

  vtkNew<vtkIntArray> roiEvents;
  roiEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  this->AddNodeReferenceRole(ROINodeReferenceRole,
    ROINodeReferenceDMMLAttributeName,
    roiEvents.GetPointer());

  this->AddNodeReferenceRole(OutputVolumeNodeReferenceRole,
    OutputVolumeNodeReferenceDMMLAttributeName);

  this->VoxelBased = false;
  this->InterpolationMode = vtkDMMLCropVolumeParametersNode::InterpolationLinear;
  this->IsotropicResampling = false;
  this->SpacingScalingConst = 1.;
  this->FillValue = 0.;
}

//----------------------------------------------------------------------------
vtkDMMLCropVolumeParametersNode::~vtkDMMLCropVolumeParametersNode() = default;

//----------------------------------------------------------------------------
void vtkDMMLCropVolumeParametersNode::ReadXMLAttributes(const char** atts)
{
  // Read all DMML node attributes from two arrays of names and values
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLBooleanMacro(voxelBased, VoxelBased);
  vtkDMMLReadXMLIntMacro(interpolationMode, InterpolationMode);
  vtkDMMLReadXMLBooleanMacro(isotropicResampling, IsotropicResampling);
  vtkDMMLReadXMLFloatMacro(spaceScalingConst, SpacingScalingConst);
  vtkDMMLReadXMLFloatMacro(fillValue, FillValue);
  vtkDMMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLCropVolumeParametersNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLBooleanMacro(voxelBased, VoxelBased);
  vtkDMMLWriteXMLIntMacro(interpolationMode, InterpolationMode);
  vtkDMMLWriteXMLBooleanMacro(isotropicResampling, IsotropicResampling);
  vtkDMMLWriteXMLFloatMacro(spaceScalingConst, SpacingScalingConst);
  vtkDMMLWriteXMLFloatMacro(fillValue, FillValue);
  vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLCropVolumeParametersNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkDMMLCopyBeginMacro(anode);
  vtkDMMLCopyBooleanMacro(VoxelBased);
  vtkDMMLCopyIntMacro(InterpolationMode);
  vtkDMMLCopyBooleanMacro(IsotropicResampling);
  vtkDMMLCopyFloatMacro(SpacingScalingConst);
  vtkDMMLCopyFloatMacro(FillValue);
  vtkDMMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLCropVolumeParametersNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  vtkDMMLPrintBeginMacro(os, indent);
  vtkDMMLPrintBooleanMacro(VoxelBased);
  vtkDMMLPrintIntMacro(InterpolationMode);
  vtkDMMLPrintBooleanMacro(IsotropicResampling);
  vtkDMMLPrintFloatMacro(SpacingScalingConst);
  vtkDMMLPrintFloatMacro(FillValue);
  vtkDMMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLCropVolumeParametersNode::SetInputVolumeNodeID(const char *nodeID)
{
  this->SetNodeReferenceID(InputVolumeNodeReferenceRole, nodeID);
}

//----------------------------------------------------------------------------
const char * vtkDMMLCropVolumeParametersNode::GetInputVolumeNodeID()
{
  return this->GetNodeReferenceID(InputVolumeNodeReferenceRole);
}

//----------------------------------------------------------------------------
vtkDMMLVolumeNode* vtkDMMLCropVolumeParametersNode::GetInputVolumeNode()
{
  return vtkDMMLVolumeNode::SafeDownCast(this->GetNodeReference(InputVolumeNodeReferenceRole));
}

//----------------------------------------------------------------------------
void vtkDMMLCropVolumeParametersNode::SetOutputVolumeNodeID(const char *nodeID)
{
  this->SetNodeReferenceID(OutputVolumeNodeReferenceRole, nodeID);
}

//----------------------------------------------------------------------------
const char * vtkDMMLCropVolumeParametersNode::GetOutputVolumeNodeID()
{
  return this->GetNodeReferenceID(OutputVolumeNodeReferenceRole);
}

//----------------------------------------------------------------------------
vtkDMMLVolumeNode* vtkDMMLCropVolumeParametersNode::GetOutputVolumeNode()
{
  return vtkDMMLVolumeNode::SafeDownCast(this->GetNodeReference(OutputVolumeNodeReferenceRole));
}

//----------------------------------------------------------------------------
void vtkDMMLCropVolumeParametersNode::SetROINodeID(const char *nodeID)
{
  this->SetNodeReferenceID(ROINodeReferenceRole, nodeID);
}

//----------------------------------------------------------------------------
const char * vtkDMMLCropVolumeParametersNode::GetROINodeID()
{
  return this->GetNodeReferenceID(ROINodeReferenceRole);
}

//----------------------------------------------------------------------------
vtkDMMLDisplayableNode* vtkDMMLCropVolumeParametersNode::GetROINode()
{
  return vtkDMMLDisplayableNode::SafeDownCast(this->GetNodeReference(ROINodeReferenceRole));
}

//----------------------------------------------------------------------------
void vtkDMMLCropVolumeParametersNode::SetROIAlignmentTransformNodeID(const char *nodeID)
{
  this->SetNodeReferenceID(ROIAlignmentTransformNodeReferenceRole, nodeID);
}

//----------------------------------------------------------------------------
const char * vtkDMMLCropVolumeParametersNode::GetROIAlignmentTransformNodeID()
{
  return this->GetNodeReferenceID(ROIAlignmentTransformNodeReferenceRole);
}

//----------------------------------------------------------------------------
vtkDMMLTransformNode* vtkDMMLCropVolumeParametersNode::GetROIAlignmentTransformNode()
{
  return vtkDMMLTransformNode::SafeDownCast(this->GetNodeReference(ROIAlignmentTransformNodeReferenceRole));
}

//----------------------------------------------------------------------------
void vtkDMMLCropVolumeParametersNode::DeleteROIAlignmentTransformNode()
{
  vtkDMMLTransformNode* transformNode = this->GetROIAlignmentTransformNode();
  if (transformNode)
    {
    this->SetROIAlignmentTransformNodeID(nullptr);
    if (this->GetScene())
      {
      this->GetScene()->RemoveNode(transformNode);
      }
    }
}
