/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLTransformNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLSnapshotClipNode.h"
#include "vtkDMMLSceneViewNode.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLSnapshotClipNode);

//----------------------------------------------------------------------------
vtkDMMLSnapshotClipNode::vtkDMMLSnapshotClipNode()
{
  this->HideFromEditors = 1;

  this->SceneSnapshotNodes = vtkCollection::New();

}

//----------------------------------------------------------------------------
vtkDMMLSnapshotClipNode::~vtkDMMLSnapshotClipNode()
{
  if (this->SceneSnapshotNodes)
    {
    this->SceneSnapshotNodes->RemoveAllItems();
    this->SceneSnapshotNodes->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkDMMLSnapshotClipNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkDMMLSceneViewNode * node = nullptr;
  std::stringstream ss;
  int n;
  for (n=0; n < this->SceneSnapshotNodes->GetNumberOfItems(); n++)
    {
    node = vtkDMMLSceneViewNode::SafeDownCast(this->SceneSnapshotNodes->GetItemAsObject(n));
    ss << node->GetID();
    if (n < this->SceneSnapshotNodes->GetNumberOfItems()-1)
      {
      ss << " ";
      }
    }
    of << " sceneSnapshotIDs=\"" << ss.str().c_str() << "\"";
}

//----------------------------------------------------------------------------
void vtkDMMLSnapshotClipNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  this->SceneSnapshotNodeIDs.clear();

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "sceneSnapshotIDs"))
      {
      std::stringstream ss(attValue);
      while (!ss.eof())
        {
        std::string id;
        ss >> id;
        this->SceneSnapshotNodeIDs.push_back(id);
        }
      }
    }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkDMMLSnapshotClipNode::Copy(vtkDMMLNode *anode)
{
  Superclass::Copy(anode);
  //vtkDMMLSnapshotClipNode *snode = (vtkDMMLSnapshotClipNode *) anode;

  if (this->SceneSnapshotNodes == nullptr)
    {
    this->SceneSnapshotNodes = vtkCollection::New();
    }
  else
    {
    this->SceneSnapshotNodes->RemoveAllItems();
    }
  vtkDMMLNode *node = nullptr;
  int n;
  for (n=0; n < this->SceneSnapshotNodes->GetNumberOfItems(); n++)
    {
    node = (vtkDMMLNode*)this->SceneSnapshotNodes->GetItemAsObject(n);
    if (node)
      {
      this->SceneSnapshotNodes->vtkCollection::AddItem((vtkObject *)node);
      }
    }
}

//----------------------------------------------------------------------------
void vtkDMMLSnapshotClipNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//-----------------------------------------------------------
void vtkDMMLSnapshotClipNode::UpdateScene(vtkDMMLScene *scene)
{
  Superclass::UpdateReferences();
  this->SceneSnapshotNodes->RemoveAllItems();

  for (unsigned int n=0; n<this->SceneSnapshotNodeIDs.size(); n++)
    {
    vtkDMMLSceneViewNode *node = vtkDMMLSceneViewNode::SafeDownCast(scene->GetNodeByID(this->SceneSnapshotNodeIDs[n]));
    this->SceneSnapshotNodes->AddItem(node);
    }
}

void vtkDMMLSnapshotClipNode::AddSceneSnapshotNode(vtkDMMLSceneViewNode * node)
{
  this->SceneSnapshotNodes->AddItem(node);
}

///
/// Get Numbre of SceneSnapshot nodes
int vtkDMMLSnapshotClipNode::GetNumberOfSceneSnapshotNodes()
{
  return this->SceneSnapshotNodes->GetNumberOfItems();
}

///
/// Get SceneSnapshot node
vtkDMMLSceneViewNode* vtkDMMLSnapshotClipNode::GetSceneSnapshotNode(int index)
{
  return vtkDMMLSceneViewNode::SafeDownCast(this->SceneSnapshotNodes->GetItemAsObject(index));
}
