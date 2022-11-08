/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLClipModelsNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLClipModelsNode.h"

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//------------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLClipModelsNode);

//----------------------------------------------------------------------------
vtkDMMLClipModelsNode::vtkDMMLClipModelsNode()
{
  this->SetSingletonTag("vtkDMMLClipModelsNode");
  this->HideFromEditors = true;
  this->ClipType = 0;
  this->RedSliceClipState = 0;
  this->YellowSliceClipState = 0;
  this->GreenSliceClipState = 0;
  this->ClippingMethod = vtkDMMLClipModelsNode::Straight;
}

//----------------------------------------------------------------------------
vtkDMMLClipModelsNode::~vtkDMMLClipModelsNode() = default;

//----------------------------------------------------------------------------
void vtkDMMLClipModelsNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  of << " clipType=\"" << this->ClipType << "\"";

  of << " redSliceClipState=\"" << this->RedSliceClipState << "\"";
  of << " yellowSliceClipState=\"" << this->YellowSliceClipState << "\"";
  of << " greenSliceClipState=\"" << this->GreenSliceClipState << "\"";
  if (this->ClippingMethod != vtkDMMLClipModelsNode::Straight)
    {
    of << " clippingMethod=\"" << (this->GetClippingMethodAsString(this->ClippingMethod)) << "\"";
    }
}

//----------------------------------------------------------------------------
void vtkDMMLClipModelsNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "yellowSliceClipState"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> YellowSliceClipState;
      }
    else if (!strcmp(attName, "redSliceClipState"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> RedSliceClipState;
      }
    else if (!strcmp(attName, "greenSliceClipState"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> GreenSliceClipState;
      }
    else if (!strcmp(attName, "clipType"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> ClipType;
      }
    else if (!strcmp(attName, "clippingMethod"))
      {
      std::stringstream ss;
      ss << attValue;
      int id = this->GetClippingMethodFromString(attValue);
      if (id < 0)
        {
        vtkWarningMacro("Invalid Clipping Methods: "<<(attValue?attValue:"(none)"));
        }
      else
        {
        this->ClippingMethod = static_cast<ClippingMethodType>(id);
        }
      }
    }
    this->EndModify(disabledModify);

}

//----------------------------------------------------------------------------
void vtkDMMLClipModelsNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkDMMLClipModelsNode* node = vtkDMMLClipModelsNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  this->SetClipType(node->ClipType);
  this->SetYellowSliceClipState(node->YellowSliceClipState);
  this->SetGreenSliceClipState(node->GreenSliceClipState);
  this->SetRedSliceClipState(node->RedSliceClipState);
  this->SetClippingMethod(node->ClippingMethod);
}

//----------------------------------------------------------------------------
void vtkDMMLClipModelsNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "ClipType:        " << this->ClipType << "\n";
  os << indent << "YellowSliceClipState: " << this->YellowSliceClipState << "\n";
  os << indent << "GreenSliceClipState:  " << this->GreenSliceClipState << "\n";
  os << indent << "RedSliceClipState:    " << this->RedSliceClipState << "\n";
  os << indent << " clippingMethod=\"" << (this->GetClippingMethodAsString(this->ClippingMethod)) << "\n";
}

//-----------------------------------------------------------------------------
int vtkDMMLClipModelsNode::GetClippingMethodFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  if (!strcmp(name, "Straight"))
    {
    return (int)Straight;
    }
  else if (!strcmp(name, "WholeCells")
    || !strcmp(name, "Whole Cells"))  // for backward compatibility
    {
    return (int)WholeCells;
    }
  else if (!strcmp(name, "WholeCellsWithBoundary")
    || !strcmp(name, "Whole Cells With Boundary"))  // for backward compatibility
    {
    return (int)WholeCellsWithBoundary;
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------------------------
const char* vtkDMMLClipModelsNode::GetClippingMethodAsString(ClippingMethodType id)
{
 switch (id)
    {
    case Straight: return "Straight";
    case WholeCells: return "WholeCells";
    case WholeCellsWithBoundary: return "WholeCellsWithBoundary";
    default:
      // invalid id
      return "";
    }
}

