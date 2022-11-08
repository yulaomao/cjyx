/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLTransformNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

#include "vtkDMMLTransformableNode.h"

// DMML includes
#include "vtkEventBroker.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLTransformNode.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkGeneralTransform.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>

const char* vtkDMMLTransformableNode::TransformNodeReferenceRole = "transform";
const char* vtkDMMLTransformableNode::TransformNodeReferenceDMMLAttributeName = "transformNodeRef";

//----------------------------------------------------------------------------
vtkDMMLTransformableNode::vtkDMMLTransformableNode()
{
  this->TransformNodeIDInternal = nullptr;

  this->HideFromEditors = 0;

  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkDMMLTransformableNode::TransformModifiedEvent);
  this->AddNodeReferenceRole(this->GetTransformNodeReferenceRole(),
                             this->GetTransformNodeReferenceDMMLAttributeName(),
                             events.GetPointer());
}

//----------------------------------------------------------------------------
vtkDMMLTransformableNode::~vtkDMMLTransformableNode() = default;

//----------------------------------------------------------------------------
const char* vtkDMMLTransformableNode::GetTransformNodeReferenceRole()
{
  return vtkDMMLTransformableNode::TransformNodeReferenceRole;
}

//----------------------------------------------------------------------------
const char* vtkDMMLTransformableNode::GetTransformNodeReferenceDMMLAttributeName()
{
  return vtkDMMLTransformableNode::TransformNodeReferenceDMMLAttributeName;
}

//----------------------------------------------------------------------------
void vtkDMMLTransformableNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkDMMLTransformableNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  this->EndModify(disabledModify);
}


//----------------------------------------------------------------------------
void vtkDMMLTransformableNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  const char* transformNodeID = this->GetNodeReferenceID(this->GetTransformNodeReferenceRole());

  os << indent << "TransformNodeID: " <<
    (transformNodeID ? transformNodeID : "(none)") << "\n";
}

//----------------------------------------------------------------------------
const char* vtkDMMLTransformableNode::GetTransformNodeID()
{
  this->SetTransformNodeIDInternal(
    this->GetNodeReferenceID(this->GetTransformNodeReferenceRole()));

  return this->GetTransformNodeIDInternal();
}

//----------------------------------------------------------------------------
vtkDMMLTransformNode* vtkDMMLTransformableNode::GetParentTransformNode()
{
  return vtkDMMLTransformNode::SafeDownCast(
        this->GetNodeReference(this->GetTransformNodeReferenceRole()));
}

//----------------------------------------------------------------------------
bool vtkDMMLTransformableNode::SetAndObserveTransformNodeID(const char *transformNodeID)
{
  // Prevent circular reference in transform tree
  vtkDMMLTransformNode* newParentTransformNode = vtkDMMLTransformNode::SafeDownCast(
    this->GetScene() != nullptr ? this->GetScene()->GetNodeByID(transformNodeID) : nullptr);
  if (newParentTransformNode)
    {
    vtkDMMLTransformNode* thisTransform = vtkDMMLTransformNode::SafeDownCast(this);
    if (thisTransform)
      {
      if (newParentTransformNode == thisTransform || thisTransform->IsTransformNodeMyChild(newParentTransformNode))
        {
        vtkErrorMacro("vtkDMMLTransformableNode::SetAndObserveTransformNodeID failed: parent transform cannot be self or a child transform");
        return false;
        }
      }
    }

  this->SetAndObserveNodeReferenceID(this->GetTransformNodeReferenceRole(), transformNodeID);
  return true;
}


//---------------------------------------------------------------------------
void vtkDMMLTransformableNode::ProcessDMMLEvents ( vtkObject *caller,
                                                  unsigned long event,
                                                  void *vtkNotUsed(callData) )
{
  // as retrieving the parent transform node can be costly (browse the scene)
  // do some checks here to prevent retrieving the node for nothing.
  if (caller == nullptr ||
      (event != vtkCommand::ModifiedEvent &&
      event != vtkDMMLTransformableNode::TransformModifiedEvent))
    {
    return;
    }
  vtkDMMLTransformNode *tnode = this->GetParentTransformNode();
  if (tnode == caller)
    {
    this->InvokeCustomModifiedEvent(vtkDMMLTransformableNode::TransformModifiedEvent, nullptr);
    }
}


//-----------------------------------------------------------
bool vtkDMMLTransformableNode::CanApplyNonLinearTransforms()const
{
  return false;
}

//-----------------------------------------------------------
void vtkDMMLTransformableNode::ApplyTransformMatrix(vtkMatrix4x4* transformMatrix)
{
  vtkNew<vtkTransform> transform;
  transform->SetMatrix(transformMatrix);
  this->ApplyTransform(transform.GetPointer());
}

//-----------------------------------------------------------
void vtkDMMLTransformableNode::ApplyTransform(vtkAbstractTransform* vtkNotUsed(transform))
{
  vtkErrorMacro("ApplyTransform is not implemented for node type "<<this->GetClassName());
}

//-----------------------------------------------------------
bool vtkDMMLTransformableNode::HardenTransform()
{
  vtkDMMLTransformNode* transformNode = this->GetParentTransformNode();
  if (!transformNode)
    {
    // already in the world coordinate system
    return true;
    }
  if (transformNode->IsTransformToWorldLinear())
    {
    vtkNew<vtkMatrix4x4> hardeningMatrix;
    transformNode->GetMatrixTransformToWorld(hardeningMatrix.GetPointer());
    this->ApplyTransformMatrix(hardeningMatrix.GetPointer());
    }
  else
    {
    vtkNew<vtkGeneralTransform> hardeningTransform;
    transformNode->GetTransformToWorld(hardeningTransform.GetPointer());
    this->ApplyTransform(hardeningTransform.GetPointer());
    }

  this->SetAndObserveTransformNodeID(nullptr);
  return true;
}

//-----------------------------------------------------------
void vtkDMMLTransformableNode::TransformPointToWorld(const double inLocal[3], double outWorld[3])
{
  vtkDMMLTransformNode* tnode = this->GetParentTransformNode();
  if (tnode == nullptr)
    {
    // not transformed
    outWorld[0] = inLocal[0];
    outWorld[1] = inLocal[1];
    outWorld[2] = inLocal[2];
    return;
    }

  // Get transform
  vtkNew<vtkGeneralTransform> transformToWorld;
  tnode->GetTransformToWorld(transformToWorld.GetPointer());

  // Convert coordinates
  transformToWorld->TransformPoint(inLocal, outWorld);
}

//-----------------------------------------------------------
void vtkDMMLTransformableNode::TransformPointFromWorld(const double inWorld[3], double outLocal[3])
{
  vtkDMMLTransformNode* tnode = this->GetParentTransformNode();
  if (tnode == nullptr)
    {
    // not transformed
    outLocal[0] = inWorld[0];
    outLocal[1] = inWorld[1];
    outLocal[2] = inWorld[2];
    return;
    }

  // Get transform
  vtkNew<vtkGeneralTransform> transformFromWorld;
  tnode->GetTransformFromWorld(transformFromWorld.GetPointer());

  // Convert coordinates
  transformFromWorld->TransformPoint(inWorld, outLocal);
}

//---------------------------------------------------------------------------
void vtkDMMLTransformableNode::TransformPointToWorld(const vtkVector3d &inLocal, vtkVector3d &outWorld)
{
  this->TransformPointToWorld(inLocal.GetData(),outWorld.GetData());
}

//---------------------------------------------------------------------------
void vtkDMMLTransformableNode::TransformPointFromWorld(const vtkVector3d &inWorld, vtkVector3d &outLocal)
{
  this->TransformPointFromWorld(inWorld.GetData(),outLocal.GetData());
}

//---------------------------------------------------------------------------
void vtkDMMLTransformableNode::OnNodeReferenceAdded(vtkDMMLNodeReference *reference)
{
  Superclass::OnNodeReferenceAdded(reference);
  if (std::string(reference->GetReferenceRole()) == this->TransformNodeReferenceRole)
    {
    this->OnTransformNodeReferenceChanged(vtkDMMLTransformNode::SafeDownCast(reference->GetReferencedNode()));
    }
}

//---------------------------------------------------------------------------
void vtkDMMLTransformableNode::OnNodeReferenceModified(vtkDMMLNodeReference *reference)
{
  Superclass::OnNodeReferenceModified(reference);
  if (std::string(reference->GetReferenceRole()) == this->TransformNodeReferenceRole)
    {
    this->OnTransformNodeReferenceChanged(vtkDMMLTransformNode::SafeDownCast(reference->GetReferencedNode()));
    }
}

//---------------------------------------------------------------------------
void vtkDMMLTransformableNode::OnNodeReferenceRemoved(vtkDMMLNodeReference *reference)
{
  Superclass::OnNodeReferenceRemoved(reference);
  if (std::string(reference->GetReferenceRole()) == this->TransformNodeReferenceRole)
    {
    this->OnTransformNodeReferenceChanged(vtkDMMLTransformNode::SafeDownCast(reference->GetReferencedNode()));
    }
}

//---------------------------------------------------------------------------
void vtkDMMLTransformableNode::OnTransformNodeReferenceChanged(vtkDMMLTransformNode* transformNode)
{
  this->InvokeCustomModifiedEvent(vtkDMMLTransformableNode::TransformModifiedEvent, transformNode);
}
