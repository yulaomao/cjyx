/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLSliceCompositeNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLSliceCompositeNode.h"
#include "vtkDMMLSliceDisplayNode.h"
#include "vtkDMMLModelNode.h"
#include "vtkDMMLScene.h"

// VTK includes
#include "vtkCollection.h"
#include "vtkCommand.h"
#include "vtkObjectFactory.h"

// STD includes
#include <sstream>

static const char* BackgroundVolumeNodeReferenceRole = "backgroundVolume";
static const char* BackgroundVolumeNodeReferenceDMMLAttributeName = "backgroundVolumeID";
static const char* ForegroundVolumeNodeReferenceRole = "foregroundVolume";
static const char* ForegroundVolumeNodeReferenceDMMLAttributeName = "foregroundVolumeID";
static const char* LabelVolumeNodeReferenceRole = "labelVolume";
static const char* LabelVolumeNodeReferenceDMMLAttributeName = "labelVolumeID";

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLSliceCompositeNode);

//----------------------------------------------------------------------------
vtkDMMLSliceCompositeNode::vtkDMMLSliceCompositeNode()
{
  this->HideFromEditors = 1;

  this->AddNodeReferenceRole(BackgroundVolumeNodeReferenceRole, BackgroundVolumeNodeReferenceDMMLAttributeName);
  this->AddNodeReferenceRole(ForegroundVolumeNodeReferenceRole, ForegroundVolumeNodeReferenceDMMLAttributeName);
  this->AddNodeReferenceRole(LabelVolumeNodeReferenceRole, LabelVolumeNodeReferenceDMMLAttributeName);
}

//----------------------------------------------------------------------------
vtkDMMLSliceCompositeNode::~vtkDMMLSliceCompositeNode()
{
}

//----------------------------------------------------------------------------
void vtkDMMLSliceCompositeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLIntMacro(compositing, Compositing);
  vtkDMMLWriteXMLFloatMacro(foregroundOpacity, ForegroundOpacity);
  vtkDMMLWriteXMLFloatMacro(labelOpacity, LabelOpacity);
  vtkDMMLWriteXMLIntMacro(linkedControl, LinkedControl);
  vtkDMMLWriteXMLIntMacro(hotLinkedControl, HotLinkedControl);
  vtkDMMLWriteXMLIntMacro(fiducialVisibility, FiducialVisibility);
  vtkDMMLWriteXMLIntMacro(fiducialLabelVisibility, FiducialLabelVisibility);
  vtkDMMLWriteXMLStringMacro(layoutName, LayoutName);
  vtkDMMLWriteXMLEnumMacro(annotationSpace, AnnotationSpace);
  vtkDMMLWriteXMLEnumMacro(annotationMode, AnnotationMode);
  vtkDMMLWriteXMLIntMacro(doPropagateVolumeSelection, DoPropagateVolumeSelection);
  vtkDMMLWriteXMLEndMacro();
}

//-----------------------------------------------------------
const char* vtkDMMLSliceCompositeNode::GetAnnotationSpaceAsString(int id)
    {
  switch (id)
  {
  case vtkDMMLSliceCompositeNode::XYZ: return "xyz";
  case vtkDMMLSliceCompositeNode::IJK: return "ijk";
  case vtkDMMLSliceCompositeNode::RAS: return "RAS";
  case vtkDMMLSliceCompositeNode::IJKAndRAS: return "IJKAndRAS";
  default:
    // invalid id
    return "";
  }
    }

//-----------------------------------------------------------
int vtkDMMLSliceCompositeNode::GetAnnotationSpaceFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int i = 0; i < AnnotationSpace_Last; i++)
    {
    if (strcmp(name, this->GetAnnotationSpaceAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
const char* vtkDMMLSliceCompositeNode::GetAnnotationModeAsString(int id)
{
  switch (id)
    {
    case vtkDMMLSliceCompositeNode::NoAnnotation: return "NoAnnotation";
    case vtkDMMLSliceCompositeNode::All: return "All";
    case vtkDMMLSliceCompositeNode::LabelValuesOnly: return "LabelValuesOnly";
    case vtkDMMLSliceCompositeNode::LabelAndVoxelValuesOnly: return "LabelAndVoxelValuesOnly";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkDMMLSliceCompositeNode::GetAnnotationModeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int i = 0; i < AnnotationMode_Last; i++)
    {
    if (strcmp(name, this->GetAnnotationModeAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
void vtkDMMLSliceCompositeNode::SetInteracting(int interacting)
{
  // Don't call Modified()
  this->Interacting = interacting;
}

//-----------------------------------------------------------
void vtkDMMLSliceCompositeNode::SetInteractionFlags(unsigned int flags)
{
  // Don't call Modified()
  this->InteractionFlags = flags;
}

//-----------------------------------------------------------
void vtkDMMLSliceCompositeNode::SetInteractionFlagsModifier(unsigned int flags)
{
  // Don't call Modified()
  this->InteractionFlagsModifier = flags;
}

//-----------------------------------------------------------
void vtkDMMLSliceCompositeNode::ResetInteractionFlagsModifier()
{
  // Don't call Modified()
  this->InteractionFlagsModifier = (unsigned int) -1;
}

//----------------------------------------------------------------------------
void vtkDMMLSliceCompositeNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLIntMacro(compositing, Compositing);
  vtkDMMLReadXMLFloatMacro(foregroundOpacity, ForegroundOpacity);
  vtkDMMLReadXMLFloatMacro(labelOpacity, LabelOpacity);
  vtkDMMLReadXMLIntMacro(linkedControl, LinkedControl);
  vtkDMMLReadXMLIntMacro(hotLinkedControl, HotLinkedControl);
  vtkDMMLReadXMLIntMacro(fiducialVisibility, FiducialVisibility);
  vtkDMMLReadXMLIntMacro(fiducialLabelVisibility, FiducialLabelVisibility);
  vtkDMMLReadXMLStringMacro(layoutName, LayoutName);
  vtkDMMLReadXMLEnumMacro(annotationSpace, AnnotationSpace);
  vtkDMMLReadXMLEnumMacro(annotationMode, AnnotationMode);
  vtkDMMLReadXMLIntMacro(doPropagateVolumeSelection, DoPropagateVolumeSelection);
  vtkDMMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLSliceCompositeNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkDMMLSliceCompositeNode *node = vtkDMMLSliceCompositeNode::SafeDownCast(anode);

  vtkDMMLCopyBeginMacro(node);
  vtkDMMLCopyIntMacro(Compositing);
  vtkDMMLCopyFloatMacro(ForegroundOpacity);
  vtkDMMLCopyFloatMacro(LabelOpacity);
  vtkDMMLCopyIntMacro(LinkedControl);
  vtkDMMLCopyIntMacro(HotLinkedControl);
  vtkDMMLCopyIntMacro(FiducialVisibility);
  vtkDMMLCopyIntMacro(FiducialLabelVisibility);
  // To avoid breaking current implementation, copy of the "LayoutName" attribute
  // will be enabled after revisiting the view initialization pipeline.
  //vtkDMMLCopyStringMacro(LayoutName);
  vtkDMMLCopyEnumMacro(AnnotationSpace);
  vtkDMMLCopyEnumMacro(AnnotationMode);
  vtkDMMLCopyIntMacro(DoPropagateVolumeSelection);
  vtkDMMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceCompositeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkDMMLPrintBeginMacro(os, indent);
  vtkDMMLPrintStringMacro(BackgroundVolumeID);
  vtkDMMLPrintStringMacro(ForegroundVolumeID);
  vtkDMMLPrintStringMacro(LabelVolumeID);
  vtkDMMLPrintIntMacro(Compositing);
  vtkDMMLPrintFloatMacro(ForegroundOpacity);
  vtkDMMLPrintFloatMacro(LabelOpacity);
  vtkDMMLPrintIntMacro(LinkedControl);
  vtkDMMLPrintIntMacro(HotLinkedControl);
  vtkDMMLPrintIntMacro(FiducialVisibility);
  vtkDMMLPrintIntMacro(FiducialLabelVisibility);
  vtkDMMLPrintStringMacro(LayoutName);
  vtkDMMLPrintEnumMacro(AnnotationSpace);
  vtkDMMLPrintEnumMacro(AnnotationMode);
  vtkDMMLPrintIntMacro(DoPropagateVolumeSelection);
  vtkDMMLPrintEndMacro();

  os << indent << "Interacting: " <<
    (this->Interacting ? "on" : "off") << "\n";
}

//-----------------------------------------------------------
void vtkDMMLSliceCompositeNode::SetBackgroundVolumeID(const char* id)
{
  this->SetNodeReferenceID(BackgroundVolumeNodeReferenceRole, id);
}

//-----------------------------------------------------------
const char* vtkDMMLSliceCompositeNode::GetBackgroundVolumeID()
{
  return this->GetNodeReferenceID(BackgroundVolumeNodeReferenceRole);
}

//-----------------------------------------------------------
void vtkDMMLSliceCompositeNode::SetForegroundVolumeID(const char* id)
{
  this->SetNodeReferenceID(ForegroundVolumeNodeReferenceRole, id);
}

//-----------------------------------------------------------
const char* vtkDMMLSliceCompositeNode::GetForegroundVolumeID()
{
  return this->GetNodeReferenceID(ForegroundVolumeNodeReferenceRole);
}

//-----------------------------------------------------------
void vtkDMMLSliceCompositeNode::SetLabelVolumeID(const char* id)
{
  this->SetNodeReferenceID(LabelVolumeNodeReferenceRole, id);
}

//-----------------------------------------------------------
const char* vtkDMMLSliceCompositeNode::GetLabelVolumeID()
{
  return this->GetNodeReferenceID(LabelVolumeNodeReferenceRole);
}

//-----------------------------------------------------------
int vtkDMMLSliceCompositeNode::GetSliceIntersectionVisibility()
{
  vtkWarningMacro("GetSliceIntersectionVisibility method is deprecated. Use GetIntersectingSlicesVisibility method"
    " of vtkDMMLSliceDisplayNode object instead.");
  vtkDMMLSliceDisplayNode* sliceDisplayNode = this->GetSliceDisplayNode();
  if (!sliceDisplayNode)
    {
    vtkWarningMacro("SetSliceIntersectionVisibility failed: no slice display node was found");
    return 0;
    }
  return sliceDisplayNode->GetIntersectingSlicesVisibility() ? 1 : 0;
}

//-----------------------------------------------------------
void vtkDMMLSliceCompositeNode::SetSliceIntersectionVisibility(int visibility)
{
  vtkWarningMacro("SetSliceIntersectionVisibility method is deprecated. Use SetIntersectingSlicesVisibility method"
    " of vtkDMMLSliceDisplayNode object instead.");
  vtkDMMLSliceDisplayNode* sliceDisplayNode = this->GetSliceDisplayNode();
  if (!sliceDisplayNode)
    {
    vtkWarningMacro("SetSliceIntersectionVisibility failed: no slice display node was found");
    return;
    }
  sliceDisplayNode->SetIntersectingSlicesVisibility(visibility != 0);
}

//----------------------------------------------------------------------------
std::string vtkDMMLSliceCompositeNode::GetCompositeNodeIDFromSliceModelNode(vtkDMMLModelNode* sliceModelNode)
{
  // Description of slice model nodes contain a string that specifies related slice node and composite node IDs:
  // "SliceID vtkDMMLSliceNodeRed CompositeID vtkDMMLSliceCompositeNodeRed".
  // This is an old mechanism that may go away in the future but it is likely to
  // stay long enough for the lifetime of the deprecated methods
  // GetSliceIntersectionVisibility/SetSliceIntersectionVisibility.

  if (!sliceModelNode || !sliceModelNode->GetDescription())
    {
    return "";
    }

  // Iterate through the description split by spaces.
  // If "CompositeID" component is found then the next component
  // is returned as composite node ID.
  std::stringstream description(sliceModelNode->GetDescription());
  std::string previous;
  std::string current;
  while (true)
    {
    current.clear();
    while (current.empty())
      {
      // Get the next string in a while loop to ignore multiple spaces
      if (!std::getline(description, current, ' '))
        {
        return "";
        }
      }
    if (previous == "CompositeID")
      {
      return current;
      }
    previous = current;
    }
}

//----------------------------------------------------------------------------
vtkDMMLSliceDisplayNode* vtkDMMLSliceCompositeNode::GetSliceDisplayNode()
{
  if (this->Scene == nullptr || this->GetLayoutName() == nullptr)
    {
    return nullptr;
    }

  // It is an expensive operation to determine the displayable node
  // (need to iterate through the scene), so the last found value
  // is cached. If it is still valid then we use it.
  if (this->LastFoundSliceDisplayNode != nullptr
    && this->LastFoundSliceDisplayNode->GetScene() == this->Scene)
    {
    vtkDMMLModelNode* sliceModelNode = vtkDMMLModelNode::SafeDownCast(this->LastFoundSliceDisplayNode->GetDisplayableNode());
    if (this->GetCompositeNodeIDFromSliceModelNode(sliceModelNode) == this->GetID())
      {
      return this->LastFoundSliceDisplayNode;
      }
    }

  vtkDMMLNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  vtkCollection* sceneNodes = this->Scene->GetNodes();
  for (sceneNodes->InitTraversal(it);
       (node = vtkDMMLNode::SafeDownCast(sceneNodes->GetNextItemAsObject(it))) ;)
    {
    vtkDMMLModelNode* sliceModelNode = vtkDMMLModelNode::SafeDownCast(node);
    if (this->GetCompositeNodeIDFromSliceModelNode(sliceModelNode) == this->GetID())
      {
      this->LastFoundSliceDisplayNode = vtkDMMLSliceDisplayNode::SafeDownCast(sliceModelNode->GetDisplayNode());
      return this->LastFoundSliceDisplayNode;
      }
    }
  this->LastFoundSliceDisplayNode = nullptr;
  return nullptr;
}
