/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLGlyphableVolumeDisplayNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include "vtkDMMLColorNode.h"
#include "vtkDMMLGlyphableVolumeDisplayNode.h"
#include "vtkDMMLScene.h"

#include "vtkCallbackCommand.h"
#include "vtkObjectFactory.h"

#include <sstream>

// Initialize static member that controls resampling --
// old comment: "This offset will be changed to 0.5 from 0.0 per 2/8/2002 Cjyx
// development meeting, to move ijk coordinates to voxel centers."
vtkCxxSetReferenceStringMacro(vtkDMMLGlyphableVolumeDisplayNode, GlyphColorNodeID);

//------------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLGlyphableVolumeDisplayNode);

//----------------------------------------------------------------------------
vtkDMMLGlyphableVolumeDisplayNode::vtkDMMLGlyphableVolumeDisplayNode()
{
  // Strings

  this->GlyphColorNodeID = nullptr;
  this->GlyphColorNode = nullptr;
  this->VisualizationMode = vtkDMMLGlyphableVolumeDisplayNode::visModeScalar;
  // try setting a default greyscale color map
  //this->SetDefaultColorMap(0);
}

//----------------------------------------------------------------------------
void vtkDMMLGlyphableVolumeDisplayNode::SetDefaultColorMap(/*int isLabelMap*/)
{
  // set up a default color node
  // TODO: figure out if can use vtkCjyxColorLogic's helper methods
  /*if (isLabelMap)
    {
    this->SetAndObserveGlyphColorNodeID("vtkDMMLColorTableNodeLabels");
    }
  else
    {*/
  this->SetAndObserveGlyphColorNodeID("vtkDMMLColorTableNodeGrey");
  //  }
  if (this->GlyphColorNode == nullptr)
    {
    vtkDebugMacro("vtkDMMLGlyphableVolumeDisplayNode: FAILED setting default  color node, it's still null\n");
    }
  else
    {
    vtkDebugMacro("vtkDMMLGlyphableVolumeDisplayNode: set up the default color node\n");
    }
}

//----------------------------------------------------------------------------
vtkDMMLGlyphableVolumeDisplayNode::~vtkDMMLGlyphableVolumeDisplayNode()
{
  this->SetAndObserveGlyphColorNodeID( nullptr);
}

//----------------------------------------------------------------------------
void vtkDMMLGlyphableVolumeDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  if (this->GlyphColorNodeID != nullptr)
    {
    of << " glyphColorNodeRef=\"" << this->GlyphColorNodeID << "\"";
    }

  std::stringstream ss;
  ss << this->VisualizationMode;
  of << " visualizationMode=\"" << ss.str() << "\"";
}

//----------------------------------------------------------------------------
void vtkDMMLGlyphableVolumeDisplayNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID, newID);
  if (this->GlyphColorNodeID && !strcmp(oldID, this->GlyphColorNodeID))
    {
    this->SetGlyphColorNodeID(newID);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLGlyphableVolumeDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "glyphColorNodeRef"))
      {
      this->SetGlyphColorNodeID(attValue);
      }
    if (!strcmp(attName, "visualizationMode"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->VisualizationMode;
      }

    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkDMMLGlyphableVolumeDisplayNode::Copy(vtkDMMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkDMMLGlyphableVolumeDisplayNode *node = (vtkDMMLGlyphableVolumeDisplayNode *) anode;

  this->SetGlyphColorNodeID(node->GlyphColorNodeID);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLGlyphableVolumeDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{

  Superclass::PrintSelf(os,indent);

 os << indent << "GlyphColorNodeID: " <<
    (this->GlyphColorNodeID ? this->GlyphColorNodeID : "(none)") << "\n";
 os << indent << "Visualization Mode:   " << this->VisualizationMode << "\n";
}

//-----------------------------------------------------------
void vtkDMMLGlyphableVolumeDisplayNode::SetSceneReferences()
{
  this->Superclass::SetSceneReferences();
  this->Scene->AddReferencedNodeID(this->GlyphColorNodeID, this);
}

//-----------------------------------------------------------
void vtkDMMLGlyphableVolumeDisplayNode::UpdateScene(vtkDMMLScene *scene)
{
   Superclass::UpdateScene(scene);

   this->SetAndObserveGlyphColorNodeID(this->GetGlyphColorNodeID());
}

//-----------------------------------------------------------
void vtkDMMLGlyphableVolumeDisplayNode::UpdateReferences()
{
   Superclass::UpdateReferences();

  if (this->GlyphColorNodeID != nullptr && this->Scene->GetNodeByID(this->GlyphColorNodeID) == nullptr)
    {
    this->SetAndObserveGlyphColorNodeID(nullptr);
    }
}

//----------------------------------------------------------------------------
vtkDMMLColorNode* vtkDMMLGlyphableVolumeDisplayNode::GetGlyphColorNode()
{
  vtkDMMLColorNode* node = nullptr;
  if (this->GetScene() && this->GetGlyphColorNodeID() )
    {
    vtkDMMLNode* cnode = this->GetScene()->GetNodeByID(this->GlyphColorNodeID);
    node = vtkDMMLColorNode::SafeDownCast(cnode);
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkDMMLGlyphableVolumeDisplayNode::SetAndObserveGlyphColorNodeID(std::string glyphColorNodeID)
{
  this->SetAndObserveGlyphColorNodeID(glyphColorNodeID.c_str());
}

//----------------------------------------------------------------------------
void vtkDMMLGlyphableVolumeDisplayNode::SetAndObserveGlyphColorNodeID(const char *glyphColorNodeID)
{
  vtkSetAndObserveDMMLObjectMacro(this->GlyphColorNode, nullptr);

  this->SetGlyphColorNodeID(glyphColorNodeID);

  vtkDMMLColorNode *cnode = this->GetGlyphColorNode();
  if (cnode != nullptr)
    {
    vtkSetAndObserveDMMLObjectMacro(this->GlyphColorNode, cnode);
    }
  if (this->Scene)
    {
    this->Scene->AddReferencedNodeID(glyphColorNodeID, this);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLGlyphableVolumeDisplayNode::ProcessDMMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessDMMLEvents(caller, event, callData);

  vtkDMMLColorNode *cnode = this->GetGlyphColorNode();
  if (cnode != nullptr && cnode == vtkDMMLColorNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, nullptr);
    }
  return;
}


