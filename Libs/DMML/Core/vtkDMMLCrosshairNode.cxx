/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLCrosshairNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLCrosshairNode.h"

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//------------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLCrosshairNode);

//----------------------------------------------------------------------------
vtkDMMLCrosshairNode::vtkDMMLCrosshairNode()
{
  this->HideFromEditors = 1;
  this->SetSingletonTag("default");
}

//----------------------------------------------------------------------------
vtkDMMLCrosshairNode::~vtkDMMLCrosshairNode() = default;

//----------------------------------------------------------------------------
void vtkDMMLCrosshairNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLEnumMacro(crosshairMode, CrosshairMode);
  vtkDMMLWriteXMLEnumMacro(crosshairBehavior, CrosshairBehavior);
  vtkDMMLWriteXMLEnumMacro(crosshairThickness, CrosshairThickness);
  vtkDMMLWriteXMLVectorMacro(crosshairRAS, CrosshairRAS, double, 3);
  // This property is only for evaluation of this feature and the value is not stored persistently in the scene file.
  // vtkDMMLWriteXMLBooleanMacro(fastPick3D, FastPick3D);
  vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLCrosshairNode::ReadXMLAttributes(const char** atts)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::ReadXMLAttributes(atts);

  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLEnumMacro(crosshairMode, CrosshairMode);
  vtkDMMLReadXMLEnumMacro(crosshairBehavior, CrosshairBehavior);
  vtkDMMLReadXMLEnumMacro(crosshairThickness, CrosshairThickness);
  vtkDMMLReadXMLVectorMacro(crosshairRAS, CrosshairRAS, double, 3);
  // This property is only for evaluation of this feature and the value is not stored persistently in the scene file.
  // vtkDMMLReadXMLBooleanMacro(fastPick3D, FastPick3D);
  vtkDMMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLCrosshairNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkDMMLCrosshairNode* node = vtkDMMLCrosshairNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  vtkDMMLCopyBeginMacro(node);
  vtkDMMLCopyEnumMacro(CrosshairMode);
  vtkDMMLCopyEnumMacro(CrosshairBehavior);
  vtkDMMLCopyEnumMacro(CrosshairThickness);
  vtkDMMLCopyVectorMacro(CrosshairRAS, double, 3);
  vtkDMMLCopyBooleanMacro(FastPick3D);
  vtkDMMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLCrosshairNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkDMMLPrintBeginMacro(os, indent);
  vtkDMMLPrintEnumMacro(CrosshairMode);
  vtkDMMLPrintEnumMacro(CrosshairBehavior);
  vtkDMMLPrintEnumMacro(CrosshairThickness);
  vtkDMMLPrintVectorMacro(CrosshairRAS, double, 3);
  vtkDMMLPrintBooleanMacro(FastPick3D);
  vtkDMMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLCrosshairNode::SetCrosshairRAS(double ras[3], int id)
{
  bool modified = false;

  if (this->LightBoxPane != id)
    {
    modified = true;
    }

  if (this->CrosshairRAS[0] != ras[0]
      || this->CrosshairRAS[1] != ras[1]
      || this->CrosshairRAS[2] != ras[2])
    {
    modified = true;
    }

  this->CrosshairRAS[0] = ras[0];
  this->CrosshairRAS[1] = ras[1];
  this->CrosshairRAS[2] = ras[2];
  this->LightBoxPane = id;

  if (modified)
    {
    this->Modified();
    }
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairNode::SetCursorPositionRAS(double ras[3])
{
  this->CursorPositionRAS[0]=ras[0];
  this->CursorPositionRAS[1]=ras[1];
  this->CursorPositionRAS[2]=ras[2];
  this->CursorPositionRASValid=true;
  this->CursorSliceNode=nullptr; // slice position is not available
  this->InvokeEvent(vtkDMMLCrosshairNode::CursorPositionModifiedEvent, nullptr);
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairNode::SetCursorPositionXYZ(double xyz[3], vtkDMMLSliceNode *sliceNode)
{
  this->CursorPositionXYZ[0]=xyz[0];
  this->CursorPositionXYZ[1]=xyz[1];
  this->CursorPositionXYZ[2]=xyz[2];
  this->CursorSliceNode=sliceNode;

  // Cursor position in the slice viewer defines the RAS position, so update that as well
  if (this->CursorSliceNode)
    {
    double xyzw[4] = {xyz[0], xyz[1], xyz[2], 1.0 };
    double rasw[4] = {0.0, 0.0, 0.0, 1.0};
    sliceNode->GetXYToRAS()->MultiplyPoint(xyzw, rasw);
    this->CursorPositionRAS[0]=rasw[0]/rasw[3];
    this->CursorPositionRAS[1]=rasw[1]/rasw[3];
    this->CursorPositionRAS[2]=rasw[2]/rasw[3];
    this->CursorPositionRASValid=true;
    }

  this->InvokeEvent(vtkDMMLCrosshairNode::CursorPositionModifiedEvent, nullptr);
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairNode::SetCursorPositionInvalid()
{
  this->CursorPositionRASValid = false;
  this->CursorSliceNode = nullptr;
  this->InvokeEvent(vtkDMMLCrosshairNode::CursorPositionModifiedEvent, nullptr);
}

//---------------------------------------------------------------------------
bool vtkDMMLCrosshairNode::GetCursorPositionRAS(double ras[3])
{
  ras[0]=this->CursorPositionRAS[0];
  ras[1]=this->CursorPositionRAS[1];
  ras[2]=this->CursorPositionRAS[2];
  return this->CursorPositionRASValid;
}

//---------------------------------------------------------------------------
vtkDMMLSliceNode* vtkDMMLCrosshairNode::GetCursorPositionXYZ(double xyz[3])
{
  xyz[0]=this->CursorPositionXYZ[0];
  xyz[1]=this->CursorPositionXYZ[1];
  xyz[2]=this->CursorPositionXYZ[2];
  return this->CursorSliceNode;
}

//-----------------------------------------------------------
const char* vtkDMMLCrosshairNode::GetCrosshairModeAsString(int id)
{
  switch (id)
    {
    case vtkDMMLCrosshairNode::NoCrosshair: return "NoCrosshair";
    case vtkDMMLCrosshairNode::ShowBasic: return "ShowBasic";
    case vtkDMMLCrosshairNode::ShowIntersection: return "ShowIntersection";
    case vtkDMMLCrosshairNode::ShowHashmarks: return "ShowHashmarks";
    case vtkDMMLCrosshairNode::ShowAll: return "ShowAll";
    case vtkDMMLCrosshairNode::ShowSmallBasic: return "ShowSmallBasic";
    case vtkDMMLCrosshairNode::ShowSmallIntersection: return "ShowSmallIntersection";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkDMMLCrosshairNode::GetCrosshairModeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int i = 0; i < CrosshairMode_Last; i++)
    {
    if (strcmp(name, vtkDMMLCrosshairNode::GetCrosshairModeAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
const char* vtkDMMLCrosshairNode::GetCrosshairBehaviorAsString(int id)
{
  switch (id)
    {
    case vtkDMMLCrosshairNode::OffsetJumpSlice: return "OffsetJumpSlice";
    case vtkDMMLCrosshairNode::CenteredJumpSlice: return "CenteredJumpSlice";
    case vtkDMMLCrosshairNode::NoAction: return "NoAction";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkDMMLCrosshairNode::GetCrosshairBehaviorFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int i = 0; i < CrosshairBehavior_Last; i++)
    {
    if (strcmp(name, vtkDMMLCrosshairNode::GetCrosshairBehaviorAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // Alternative names for OffsetJumpSlice for legacy scenes
  if (!strcmp(name, "JumpSlice") || !strcmp(name, "Normal"))
    {
    return vtkDMMLCrosshairNode::OffsetJumpSlice;
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
const char* vtkDMMLCrosshairNode::GetCrosshairThicknessAsString(int id)
{
  switch (id)
    {
    case vtkDMMLCrosshairNode::Fine: return "Fine";
    case vtkDMMLCrosshairNode::Medium: return "Medium";
    case vtkDMMLCrosshairNode::Thick: return "Thick";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkDMMLCrosshairNode::GetCrosshairThicknessFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int i = 0; i < CrosshairThickness_Last; i++)
    {
    if (strcmp(name, vtkDMMLCrosshairNode::GetCrosshairThicknessAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // unknown name
  return -1;
}
