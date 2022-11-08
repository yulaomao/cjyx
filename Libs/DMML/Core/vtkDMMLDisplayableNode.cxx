/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLDisplayableNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLDisplayableNode.h"
#include "vtkDMMLDisplayNode.h"
#include "vtkDMMLScene.h"

// when change the display node, update the scalars
//#include "vtkDMMLVolumeNode.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkMath.h>
#include <vtkNew.h>

// STD includes
#include <algorithm>
#include <cassert>
#include <sstream>

const char* vtkDMMLDisplayableNode::DisplayNodeReferenceRole = "display";
const char* vtkDMMLDisplayableNode::DisplayNodeReferenceDMMLAttributeName = "displayNodeRef";

//----------------------------------------------------------------------------
vtkDMMLDisplayableNode::vtkDMMLDisplayableNode()
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkDMMLDisplayableNode::DisplayModifiedEvent);

  this->AddNodeReferenceRole(this->GetDisplayNodeReferenceRole(),
                             this->GetDisplayNodeReferenceDMMLAttributeName(),
                             events.GetPointer());
}

//----------------------------------------------------------------------------
vtkDMMLDisplayableNode::~vtkDMMLDisplayableNode() = default;

//----------------------------------------------------------------------------
const char* vtkDMMLDisplayableNode::GetDisplayNodeReferenceRole()
{
  return vtkDMMLDisplayableNode::DisplayNodeReferenceRole;
}

//----------------------------------------------------------------------------
const char* vtkDMMLDisplayableNode::GetDisplayNodeReferenceDMMLAttributeName()
{
  return vtkDMMLDisplayableNode::DisplayNodeReferenceDMMLAttributeName;
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableNode::OnNodeReferenceAdded(vtkDMMLNodeReference *reference)
{
  this->Superclass::OnNodeReferenceAdded(reference);
  if (std::string(reference->GetReferenceRole()) == this->DisplayNodeReferenceRole)
    {
    this->InvokeEvent(vtkDMMLDisplayableNode::DisplayModifiedEvent, reference->GetReferencedNode());
    }
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableNode::OnNodeReferenceModified(vtkDMMLNodeReference *reference)
{
  this->Superclass::OnNodeReferenceModified(reference);
  if (std::string(reference->GetReferenceRole()) == this->DisplayNodeReferenceRole)
    {
    this->InvokeEvent(vtkDMMLDisplayableNode::DisplayModifiedEvent, reference->GetReferencedNode());
    }
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableNode::OnNodeReferenceRemoved(vtkDMMLNodeReference *reference)
{
  this->Superclass::OnNodeReferenceRemoved(reference);
  if (std::string(reference->GetReferenceRole()) == this->DisplayNodeReferenceRole)
    {
    this->InvokeEvent(vtkDMMLDisplayableNode::DisplayModifiedEvent, reference->GetReferencedNode());
    }
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkDMMLDisplayableNode::Copy(vtkDMMLNode *anode)
{
  int disabledModify = this->StartModify();

  this->Superclass::Copy(anode);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  int numDisplayNodes = this->GetNumberOfNodeReferences(
    this->GetDisplayNodeReferenceRole());

  for (int i=0; i<numDisplayNodes; i++)
    {
    const char * id = this->GetNthNodeReferenceID(
      this->GetDisplayNodeReferenceRole(), i);
    os << indent << "DisplayNodeIDs[" << i << "]: " << (id ? id : "(none)") << "\n";
    }
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableNode::SetAndObserveDisplayNodeID(const char *displayNodeID)
{
  this->SetAndObserveNodeReferenceID(this->GetDisplayNodeReferenceRole(), displayNodeID);
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableNode::AddAndObserveDisplayNodeID(const char *displayNodeID)
{
  this->AddAndObserveNodeReferenceID(this->GetDisplayNodeReferenceRole(), displayNodeID);
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableNode::RemoveNthDisplayNodeID(int n)
{
  this->RemoveNthNodeReferenceID(this->GetDisplayNodeReferenceRole(), n);
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableNode::RemoveAllDisplayNodeIDs()
{
  this->RemoveNodeReferenceIDs(this->GetDisplayNodeReferenceRole());
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableNode::SetAndObserveNthDisplayNodeID(int n, const char *displayNodeID)
{
  this->SetAndObserveNthNodeReferenceID(this->GetDisplayNodeReferenceRole(), n, displayNodeID);
}

//----------------------------------------------------------------------------
bool vtkDMMLDisplayableNode::HasDisplayNodeID(const char* displayNodeID)
{
  return this->HasNodeReferenceID(this->GetDisplayNodeReferenceRole(), displayNodeID);
}

//----------------------------------------------------------------------------
int vtkDMMLDisplayableNode::GetNumberOfDisplayNodes()
{
  return this->GetNumberOfNodeReferences(this->GetDisplayNodeReferenceRole());
}

//----------------------------------------------------------------------------
const char* vtkDMMLDisplayableNode::GetNthDisplayNodeID(int n)
{
  return this->GetNthNodeReferenceID(this->GetDisplayNodeReferenceRole(), n);
}

//----------------------------------------------------------------------------
const char* vtkDMMLDisplayableNode::GetDisplayNodeID()
{
  return this->GetNthDisplayNodeID(0);
}

//----------------------------------------------------------------------------
vtkDMMLDisplayNode* vtkDMMLDisplayableNode::GetNthDisplayNode(int n)
{
  return vtkDMMLDisplayNode::SafeDownCast(
    this->GetNthNodeReference(this->GetDisplayNodeReferenceRole(), n));
}

//----------------------------------------------------------------------------
vtkDMMLDisplayNode* vtkDMMLDisplayableNode::GetDisplayNode()
{
  return this->GetNthDisplayNode(0);
}

//---------------------------------------------------------------------------
void vtkDMMLDisplayableNode::ProcessDMMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessDMMLEvents(caller, event, callData);
  int numDisplayNodes = this->GetNumberOfNodeReferences(this->GetDisplayNodeReferenceRole());
  for (int i=0; i<numDisplayNodes; i++)
    {
    vtkDMMLDisplayNode *dnode = this->GetNthDisplayNode(i);
    if (dnode != nullptr && dnode == vtkDMMLDisplayNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
      {
      this->InvokeEvent(vtkDMMLDisplayableNode::DisplayModifiedEvent, dnode);
      }
    }
  return;
}

//---------------------------------------------------------------------------
void vtkDMMLDisplayableNode::CreateDefaultDisplayNodes()
{
  // does nothing by default
}

//---------------------------------------------------------------------------
void vtkDMMLDisplayableNode::CreateDefaultSequenceDisplayNodes()
{
  // By default, regular display nodes are created, but for some nodes
  // slightly modified nodes may be added.
  this->CreateDefaultDisplayNodes();
}

//---------------------------------------------------------------------------
int vtkDMMLDisplayableNode::GetDisplayVisibility()
{
  int ndnodes = this->GetNumberOfDisplayNodes();
  if (ndnodes == 0 || this->GetNthDisplayNode(0) == nullptr)
    {
    return 0;
    }
  int visible = this->GetNthDisplayNode(0)->GetVisibility();
  if (visible == 2)
    {
    return 2;
    }

  for (int i=1; i<ndnodes; i++)
    {
    vtkDMMLDisplayNode *displayNode = this->GetNthDisplayNode(i);
    if (displayNode && displayNode->IsShowModeDefault()
      && displayNode->GetVisibility() != visible)
      {
      return 2;
      }
    }
  return visible;
}

//---------------------------------------------------------------------------
int vtkDMMLDisplayableNode::GetDisplayClassVisibility(const char* nodeClass)
{
  if (nodeClass == nullptr || std::string(nodeClass).empty())
    {
    return this->GetDisplayVisibility();
    }
  int ndnodes = this->GetNumberOfDisplayNodes();
  int visible = 0;
  for (int i=0; i<ndnodes; i++)
    {
    vtkDMMLDisplayNode *displayNode = this->GetNthDisplayNode(i);
    if ( displayNode && displayNode->IsShowModeDefault()
      && displayNode->IsA(nodeClass) )
      {
      visible = displayNode->GetVisibility();
      }
    }
  return visible;
}

//---------------------------------------------------------------------------
void vtkDMMLDisplayableNode::SetDisplayVisibility(int visible)
{
  if (visible == 2)
    {
    return;
    }

  int ndnodes = this->GetNumberOfDisplayNodes();
  for (int i=0; i<ndnodes; i++)
    {
    vtkDMMLDisplayNode *displayNode = this->GetNthDisplayNode(i);
    if (displayNode && displayNode->IsShowModeDefault())
      {
      displayNode->SetVisibility(visible);
      }
    }
}

//---------------------------------------------------------------------------
void vtkDMMLDisplayableNode::SetDisplayClassVisibility(const char* nodeClass, int visible)
{
  if (nodeClass == nullptr || std::string(nodeClass).empty())
    {
    this->SetDisplayVisibility(visible);
    return;
    }
  if (visible == 2)
    {
    return;
    }

  int ndnodes = this->GetNumberOfDisplayNodes();
  for (int i=0; i<ndnodes; i++)
    {
    vtkDMMLDisplayNode *displayNode = this->GetNthDisplayNode(i);
    if (displayNode && displayNode->IsShowModeDefault()
      && displayNode->IsA(nodeClass))
      {
      displayNode->SetVisibility(visible);
      }
    }
}

//---------------------------------------------------------------------------
void vtkDMMLDisplayableNode::GetRASBounds(double bounds[6])
{
  vtkMath::UninitializeBounds(bounds);
}

//---------------------------------------------------------------------------
void vtkDMMLDisplayableNode::GetBounds(double bounds[6])
{
  vtkMath::UninitializeBounds(bounds);
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableNode::SetSelectable(int selectable)
{
  bool modified = (selectable != this->Selectable);
  Superclass::SetSelectable(selectable);
  if (modified)
    {
    this->InvokeEvent(vtkDMMLDisplayableNode::DisplayModifiedEvent, this);
    }
}
