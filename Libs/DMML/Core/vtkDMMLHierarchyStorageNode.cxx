/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLFiducialListStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.6 $

=========================================================================auto=*/

#include "vtkDMMLHierarchyStorageNode.h"
#include "vtkDMMLHierarchyNode.h"
#include "vtkDMMLScene.h"

#include <vtkObjectFactory.h>
#include <vtkStringArray.h>

//------------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLHierarchyStorageNode);

//----------------------------------------------------------------------------
vtkDMMLHierarchyStorageNode::vtkDMMLHierarchyStorageNode()
{
  this->DefaultWriteFileExtension = "txt";
}

//----------------------------------------------------------------------------
vtkDMMLHierarchyStorageNode::~vtkDMMLHierarchyStorageNode() = default;

//----------------------------------------------------------------------------
void vtkDMMLHierarchyStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDMMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
bool vtkDMMLHierarchyStorageNode::CanReadInReferenceNode(vtkDMMLNode *refNode)
{
  return refNode->IsA("vtkDMMLHierarchyNode");
}

//----------------------------------------------------------------------------
int vtkDMMLHierarchyStorageNode::ReadDataInternal(vtkDMMLNode *refNode)
{
  std::string fullName = this->GetFullNameFromFileName();

  if (fullName.empty())
    {
    vtkErrorMacro("vtkDMMLHierarchyStorageNode: File name not specified");
    return 0;
    }

  // cast the input node
  vtkDMMLHierarchyNode *hierarchyNode = nullptr;
  if ( refNode->IsA("vtkDMMLHierarchyNode") )
    {
    hierarchyNode = dynamic_cast <vtkDMMLHierarchyNode *> (refNode);
    }

  if (hierarchyNode == nullptr)
    {
    vtkErrorMacro("ReadData: unable to cast input node " << refNode->GetID() << " to a hierarchy node");
    return 0;
    }

  // open the file for reading input
  fstream fstr;

  fstr.open(fullName.c_str(), fstream::in);

  if (fstr.is_open())
    {
    //turn off modified events
    int modFlag = hierarchyNode->GetDisableModifiedEvent();
    hierarchyNode->DisableModifiedEventOn();

    // do the reading here if necessary, but it's not right now

    hierarchyNode->SetDisableModifiedEvent(modFlag);
    hierarchyNode->InvokeEvent(vtkDMMLScene::NodeAddedEvent, hierarchyNode);
    fstr.close();
    }
  else
    {
    vtkErrorMacro("ERROR opening file " << this->FileName << endl);
    return 0;
    }

  // make sure that the list node points to this storage node
  //-------------------------> hierarchyNode->SetAndObserveStorageNodeID(this->GetID());

  return 1;
}

//----------------------------------------------------------------------------
int vtkDMMLHierarchyStorageNode::WriteDataInternal(vtkDMMLNode *refNode)
{
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("vtkDMMLHierarchyStorageNode: File name not specified");
    return 0;
    }

  // cast the input node
  vtkDMMLHierarchyNode *hierarchyNode = nullptr;
  if ( refNode->IsA("vtkDMMLHierarchyNode") )
    {
    hierarchyNode = dynamic_cast <vtkDMMLHierarchyNode *> (refNode);
    }

  if (hierarchyNode == nullptr)
    {
    vtkErrorMacro("WriteData: unable to cast input node " << refNode->GetID() << " to a known hierarchy node");
    return 0;
    }

  // open the file for writing
  fstream of;

  of.open(fullName.c_str(), fstream::out);

  if (!of.is_open())
  {
  vtkErrorMacro("WriteData: unable to open file " << fullName.c_str() << " for writing");
  return 0;
  }

  // put down a header
  of << "# hierarchy file " << (this->GetFileName() != nullptr ? this->GetFileName() : "null") << endl;

  of.close();

  this->StageWriteData(refNode);

  return 1;

}

//----------------------------------------------------------------------------
void vtkDMMLHierarchyStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("Text (.txt)");
}

//----------------------------------------------------------------------------
void vtkDMMLHierarchyStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Text (.txt)");
}
