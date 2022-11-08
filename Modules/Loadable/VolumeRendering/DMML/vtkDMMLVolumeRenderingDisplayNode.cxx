/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLVolumeRenderingDisplayNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// Annotations includes
#include "vtkDMMLMarkupsROINode.h"

// DMML includes
#include "vtkDMMLAnnotationROINode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLShaderPropertyNode.h"
#include "vtkDMMLViewNode.h"
#include "vtkDMMLVolumeNode.h"
#include "vtkDMMLVolumePropertyNode.h"
#include "vtkDMMLVolumeRenderingDisplayNode.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
const char* vtkDMMLVolumeRenderingDisplayNode::VolumePropertyNodeReferenceRole = "volumeProperty";
const char* vtkDMMLVolumeRenderingDisplayNode::VolumePropertyNodeReferenceDMMLAttributeName = "volumePropertyNodeID";
const char* vtkDMMLVolumeRenderingDisplayNode::ROINodeReferenceRole = "roi";
const char* vtkDMMLVolumeRenderingDisplayNode::ROINodeReferenceDMMLAttributeName = "ROINodeID";
const char* vtkDMMLVolumeRenderingDisplayNode::ShaderPropertyNodeReferenceRole = "shaderProperty";
const char* vtkDMMLVolumeRenderingDisplayNode::ShaderPropertyNodeReferenceDMMLAttributeName = "shaderPropertyNodeId";

//----------------------------------------------------------------------------
vtkDMMLVolumeRenderingDisplayNode::vtkDMMLVolumeRenderingDisplayNode()
{
  vtkNew<vtkIntArray> volumePropertyEvents;
  volumePropertyEvents->InsertNextValue(vtkCommand::StartEvent);
  volumePropertyEvents->InsertNextValue(vtkCommand::EndEvent);
  volumePropertyEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  volumePropertyEvents->InsertNextValue(vtkCommand::StartInteractionEvent);
  volumePropertyEvents->InsertNextValue(vtkCommand::InteractionEvent);
  volumePropertyEvents->InsertNextValue(vtkCommand::EndInteractionEvent);
  this->AddNodeReferenceRole(VolumePropertyNodeReferenceRole,
                             VolumePropertyNodeReferenceDMMLAttributeName,
                             volumePropertyEvents.GetPointer());

  vtkNew<vtkIntArray> roiEvents;
  roiEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  this->AddNodeReferenceRole(ROINodeReferenceRole,
                             ROINodeReferenceDMMLAttributeName,
                             roiEvents.GetPointer());

  vtkNew<vtkIntArray> shaderPropertyEvents;
  shaderPropertyEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  this->AddNodeReferenceRole(ShaderPropertyNodeReferenceRole,
                             ShaderPropertyNodeReferenceDMMLAttributeName,
                             shaderPropertyEvents.GetPointer());

  this->CroppingEnabled = 0;//by default cropping is not enabled

  this->Threshold[0] = 0.0;
  this->Threshold[1] = 1.0;

  this->FollowVolumeDisplayNode = 0;// by default do not follow volume display node
  this->IgnoreVolumeDisplayNodeThreshold = 0;
  this->UseSingleVolumeProperty = 0;

  this->WindowLevel[0] = 0.0;
  this->WindowLevel[1] = 0.0;
}

//----------------------------------------------------------------------------
vtkDMMLVolumeRenderingDisplayNode::~vtkDMMLVolumeRenderingDisplayNode() = default;

//----------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);

  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLIntMacro(croppingEnabled, CroppingEnabled);
  vtkDMMLReadXMLVectorMacro(threshold, Threshold, double, 2);
  vtkDMMLReadXMLVectorMacro(windowLevel, WindowLevel, double, 2);
  vtkDMMLReadXMLIntMacro(followVolumeDisplayNode, FollowVolumeDisplayNode);
  vtkDMMLReadXMLIntMacro(ignoreVolumeDisplayNodeThreshold, IgnoreVolumeDisplayNodeThreshold);
  vtkDMMLReadXMLIntMacro(useSingleVolumeProperty, UseSingleVolumeProperty);
  vtkDMMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayNode::WriteXML(ostream& of, int nIndent)
{
  this->Superclass::WriteXML(of, nIndent);

  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLIntMacro(croppingEnabled, CroppingEnabled);
  vtkDMMLWriteXMLVectorMacro(threshold, Threshold, double, 2);
  vtkDMMLWriteXMLVectorMacro(windowLevel, WindowLevel, double, 2);
  vtkDMMLWriteXMLIntMacro(followVolumeDisplayNode, FollowVolumeDisplayNode);
  vtkDMMLWriteXMLIntMacro(ignoreVolumeDisplayNodeThreshold, IgnoreVolumeDisplayNodeThreshold);
  vtkDMMLWriteXMLIntMacro(useSingleVolumeProperty, UseSingleVolumeProperty);
  vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, SliceID
void vtkDMMLVolumeRenderingDisplayNode::Copy(vtkDMMLNode *anode)
{
  int wasModifying = this->StartModify();
  this->Superclass::Copy(anode);

  vtkDMMLCopyBeginMacro(anode);
  vtkDMMLCopyIntMacro(CroppingEnabled);
  vtkDMMLCopyVectorMacro(Threshold, double, 2);
  vtkDMMLCopyVectorMacro(WindowLevel, double, 2);
  vtkDMMLCopyIntMacro(FollowVolumeDisplayNode);
  vtkDMMLCopyIntMacro(IgnoreVolumeDisplayNodeThreshold);
  vtkDMMLCopyIntMacro(UseSingleVolumeProperty);
  vtkDMMLCopyEndMacro();

  this->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkDMMLPrintBeginMacro(os,indent);
  vtkDMMLPrintIntMacro(CroppingEnabled);
  vtkDMMLPrintVectorMacro(Threshold, double, 2);
  vtkDMMLPrintVectorMacro(WindowLevel, double, 2);
  vtkDMMLPrintIntMacro(FollowVolumeDisplayNode);
  vtkDMMLPrintIntMacro(IgnoreVolumeDisplayNodeThreshold);
  vtkDMMLPrintIntMacro(UseSingleVolumeProperty);
  vtkDMMLPrintEndMacro();
}

//----------------------------------------------------------------------------
const char* vtkDMMLVolumeRenderingDisplayNode::GetVolumeNodeID()
{
  vtkDMMLDisplayableNode* volumeNode = this->GetDisplayableNode();
  return (volumeNode ? volumeNode->GetID() : nullptr);
}

//----------------------------------------------------------------------------
vtkDMMLVolumeNode* vtkDMMLVolumeRenderingDisplayNode::GetVolumeNode()
{
  return vtkDMMLVolumeNode::SafeDownCast(this->GetDisplayableNode());
}

//----------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayNode::SetAndObserveVolumePropertyNodeID(
  const char* volumePropertyNodeID)
{
  this->SetAndObserveNodeReferenceID(VolumePropertyNodeReferenceRole, volumePropertyNodeID);
}

//----------------------------------------------------------------------------
const char* vtkDMMLVolumeRenderingDisplayNode::GetVolumePropertyNodeID()
{
  return this->GetNodeReferenceID(VolumePropertyNodeReferenceRole);
}

//----------------------------------------------------------------------------
vtkDMMLVolumePropertyNode* vtkDMMLVolumeRenderingDisplayNode::GetVolumePropertyNode()
{
  return vtkDMMLVolumePropertyNode::SafeDownCast(
    this->GetNodeReference(VolumePropertyNodeReferenceRole));
}

//----------------------------------------------------------------------------
const char* vtkDMMLVolumeRenderingDisplayNode::GetShaderPropertyNodeID()
{
    return this->GetNodeReferenceID(ShaderPropertyNodeReferenceRole);
}

//----------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayNode::SetAndObserveShaderPropertyNodeID(const char *shaderPropertyNodeID)
{
    this->SetAndObserveNodeReferenceID(ShaderPropertyNodeReferenceRole, shaderPropertyNodeID);
}

//----------------------------------------------------------------------------
vtkDMMLShaderPropertyNode* vtkDMMLVolumeRenderingDisplayNode::GetShaderPropertyNode()
{
    return vtkDMMLShaderPropertyNode::SafeDownCast(
      this->GetNodeReference(ShaderPropertyNodeReferenceRole));
}

//----------------------------------------------------------------------------
vtkDMMLShaderPropertyNode* vtkDMMLVolumeRenderingDisplayNode::GetOrCreateShaderPropertyNode( vtkDMMLScene * dmmlScene )
{
  vtkDMMLShaderPropertyNode * sp = this->GetShaderPropertyNode();
  if( sp == nullptr )
    {
    vtkNew<vtkDMMLShaderPropertyNode> shaderNode;
    dmmlScene->AddNode(shaderNode);
    this->SetAndObserveShaderPropertyNodeID(shaderNode->GetID());
    sp = shaderNode.GetPointer();
    }
  return sp;
}

//----------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayNode::SetAndObserveROINodeID(const char* roiNodeID)
{
  this->SetAndObserveNodeReferenceID(ROINodeReferenceRole, roiNodeID);
}

//----------------------------------------------------------------------------
const char* vtkDMMLVolumeRenderingDisplayNode::GetROINodeID()
{
  return this->GetNodeReferenceID(ROINodeReferenceRole);
}

//----------------------------------------------------------------------------
vtkDMMLDisplayableNode* vtkDMMLVolumeRenderingDisplayNode::GetROINode()
{
  return vtkDMMLDisplayableNode::SafeDownCast(this->GetNodeReference(ROINodeReferenceRole));
}

//----------------------------------------------------------------------------
vtkDMMLAnnotationROINode* vtkDMMLVolumeRenderingDisplayNode::GetAnnotationROINode()
{
  return vtkDMMLAnnotationROINode::SafeDownCast(this->GetNodeReference(ROINodeReferenceRole));
}

//----------------------------------------------------------------------------
vtkDMMLMarkupsROINode* vtkDMMLVolumeRenderingDisplayNode::GetMarkupsROINode()
{
  return vtkDMMLMarkupsROINode::SafeDownCast(this->GetNodeReference(ROINodeReferenceRole));
}

//-----------------------------------------------------------------------------
vtkDMMLViewNode* vtkDMMLVolumeRenderingDisplayNode::GetFirstViewNode()
{
  if (!this->GetScene())
    {
    return nullptr;
    }

  std::vector<vtkDMMLNode*> viewNodes;
  this->GetScene()->GetNodesByClass("vtkDMMLViewNode", viewNodes);
  for (std::vector<vtkDMMLNode*>::iterator it=viewNodes.begin(); it!=viewNodes.end(); ++it)
    {
    if (this->IsDisplayableInView((*it)->GetID()))
      {
      return vtkDMMLViewNode::SafeDownCast(*it);
      }
    }

  return nullptr;
}

//---------------------------------------------------------------------------
double vtkDMMLVolumeRenderingDisplayNode::GetSampleDistance()
{
  vtkDMMLViewNode* firstViewNode = this->GetFirstViewNode();
  if (!firstViewNode)
    {
    vtkErrorMacro("GetSampleDistance: Failed to access view node, returning 1mm");
    return 1.0;
    }
  vtkDMMLVolumeNode* volumeNode = this->GetVolumeNode();
  if (!volumeNode)
    {
    vtkErrorMacro("GetSampleDistance: Failed to access volume node, assuming 1mm voxel size");
    return 1.0 / firstViewNode->GetVolumeRenderingOversamplingFactor();
    }

  const double minSpacing = volumeNode->GetMinSpacing() > 0 ? volumeNode->GetMinSpacing() : 1.;
  double sampleDistance = minSpacing / firstViewNode->GetVolumeRenderingOversamplingFactor();
  if ( firstViewNode
    && firstViewNode->GetVolumeRenderingQuality() == vtkDMMLViewNode::Maximum)
    {
    sampleDistance = minSpacing / 10.; // =10x smaller than pixel is high quality
    }
  return sampleDistance;
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayNode::ProcessDMMLEvents(vtkObject *caller,
                                                          unsigned long event,
                                                          void *callData)
{
  this->Superclass::ProcessDMMLEvents(caller, event, callData);

  vtkDMMLVolumePropertyNode* volumePropertyNode = this->GetVolumePropertyNode();
  if (volumePropertyNode != nullptr &&
      volumePropertyNode == vtkDMMLVolumePropertyNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, nullptr);
    }
  vtkDMMLShaderPropertyNode* shaderPropertyNode = this->GetShaderPropertyNode();
  if (shaderPropertyNode != nullptr &&
      shaderPropertyNode == vtkDMMLShaderPropertyNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, nullptr);
    }
  vtkDMMLAnnotationROINode* roiNode = this->GetAnnotationROINode();
  if (roiNode != nullptr &&
      roiNode == vtkDMMLAnnotationROINode::SafeDownCast(caller) &&
      event == vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, nullptr);
    }
  vtkDMMLMarkupsROINode* markupRoiNode = this->GetMarkupsROINode();
  if (markupRoiNode != nullptr &&
      markupRoiNode == vtkDMMLMarkupsROINode::SafeDownCast(caller) &&
      event == vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, nullptr);
    }

  if (event == vtkCommand::StartEvent ||
      event == vtkCommand::EndEvent ||
      event == vtkCommand::StartInteractionEvent ||
      event == vtkCommand::InteractionEvent ||
      event == vtkCommand::EndInteractionEvent
      )
    {
    this->InvokeEvent(event);
    }
}
