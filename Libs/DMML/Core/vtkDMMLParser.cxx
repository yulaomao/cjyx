/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLParser.cxx,v $
Date:      $Date: 2006/03/11 19:51:14 $
Version:   $Revision: 1.8 $

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLParser.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLStorageNode.h"
#include "vtkDMMLNode.h"
#include "vtkDMMLSubjectHierarchyNode.h"
#include "vtkDMMLSubjectHierarchyLegacyNode.h"
#include "vtkDMMLSceneViewNode.h"
#include "vtkTagTable.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkObjectFactory.h>
#include <vtkStdString.h>

// STD includes
#include <sstream>

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLParser);

//------------------------------------------------------------------------------
void vtkDMMLParser::StartElement(const char* tagName, const char** atts)
{
  if (!strcmp(tagName, "DMML"))
    {
    //--- BEGIN test of user tags
    //--- pull out any tags describing the scene and fill the scene's tagtable.
    const char* attName;
    const char* attValue;
    while (*atts != nullptr)
      {
      attName = *(atts++);
      attValue = *(atts++);
      if (!strcmp(attName, "version"))
        {
        this->GetDMMLScene()->SetLastLoadedVersion(attValue);
        }
      if (!strcmp(attName, "extensions"))
        {
        this->GetDMMLScene()->SetLastLoadedExtensions(attValue);
        }
      else if (!strcmp(attName, "userTags"))
        {
        if ( this->DMMLScene->GetUserTagTable() == nullptr )
          {
          //--- null table, no tags are read.
          return;
          }
        std::stringstream ss(attValue);
        std::string kwd = "";
        std::string val = "";
        std::string::size_type i;
        while (!ss.eof())
          {
          std::string tags;
          ss >> tags;
          //--- now pull apart individual tags
          if ( tags.c_str() != nullptr )
            {
            i = tags.find("=");
            if ( i != std::string::npos)
              {
              kwd = tags.substr(0, i);
              val = tags.substr(i+1, std::string::npos );
              if ( kwd.c_str() != nullptr && val.c_str() != nullptr )
                {
                this->DMMLScene->GetUserTagTable()->AddOrUpdateTag ( kwd.c_str(), val.c_str(), 0 );
                }
              }
            }
          }
        } //--- END test of user tags.
      } // while
    return;
    } // DMML

  // SubjectHierarchyItem tag means the element belongs to a subject hierarchy item, not a DMML node
  //TODO: This special case can be resolved by a more generic mechanism that passes the non-node child
  //      elements to the containing node for parsing
  if (!strcmp(tagName, "SubjectHierarchyItem"))
    {
    // Have the recently loaded subject hierarchy node parse the item and add it as unresolved.
    // Getting the last loaded node safely returns the subject hierarchy nodes, as only it can have
    // children items named SubjectHierarchyItem.
    // Another possibility is that it's part of a scene view, in which case we need to access the
    // last node in the scene view's snapshot scene
    vtkDMMLSubjectHierarchyNode* subjectHierarchyNode = vtkDMMLSubjectHierarchyNode::SafeDownCast(
      this->NodeCollection->GetItemAsObject(this->NodeCollection->GetNumberOfItems()-1) );
    if (!subjectHierarchyNode)
      {
      vtkDMMLSceneViewNode* sceneViewNode = vtkDMMLSceneViewNode::SafeDownCast(
        this->NodeCollection->GetItemAsObject(this->NodeCollection->GetNumberOfItems()-1) );
      if (!sceneViewNode)
        {
        vtkWarningMacro("Invalid parent node element for SubjectHierarchyItem");
        return;
        }
      vtkCollection* shNodeCollection = sceneViewNode->GetStoredScene()->GetNodesByClass("vtkDMMLSubjectHierarchyNode");
      subjectHierarchyNode = vtkDMMLSubjectHierarchyNode::SafeDownCast(shNodeCollection->GetItemAsObject(0));
      shNodeCollection->Delete();
      }
    subjectHierarchyNode->ReadItemFromXML(atts);
    return;
    }

  const char* tmp = this->DMMLScene->GetClassNameByTag(tagName);
  std::string className = tmp ? tmp : "";

  // CreateNodeByClass should have a chance to instantiate non-registered node
  if (className.empty())
    {
    className = "vtkDMML";
    className += tagName;
    // Append 'Node' prefix only if required
    if (className.find("Node") != className.size() - 4)
      {
      className += "Node";
      }
    }

  vtkDMMLNode* node = this->DMMLScene->CreateNodeByClass( className.c_str() );
  if (!node)
    {
    vtkErrorMacro("Failed to CreateNodeByClass: " << className);
    return;
    }

  // It is needed to have the scene set before ReadXMLAttributes is
  // called on storage nodes.
  if (vtkDMMLStorageNode::SafeDownCast(node) != nullptr)
    {
    node->SetScene(this->GetDMMLScene());
    }

  node->ReadXMLAttributes(atts);

  // Cjyx3 snap shot nodes were hidden by default, show them so that
  // they show up in the tree views
#if DMML_APPLICATION_SUPPORT_VERSION < DMML_VERSION_CHECK(4, 0, 0)
  if (strcmp(tagName, "SceneSnapshot") == 0)
    {
    node->HideFromEditorsOff();
    }
#endif

  // Replace old-style label map nodes (vtkDMMLScalarVolumeNode with LabelMap custom attribute)
  // with new-style vtkDMMLLabelMapVolumeNode
  if (node->IsA("vtkDMMLScalarVolumeNode"))
    {
    const char* labelMapAttr = node->GetAttribute("LabelMap");
    bool isLabelMap = labelMapAttr ? (atoi(labelMapAttr)!=0) : false;
    if (isLabelMap)
      {
      // create a copy of the node of the correct class
      vtkDMMLNode* newTypeLabelMapNode = this->DMMLScene->CreateNodeByClass( "vtkDMMLLabelMapVolumeNode" );
      newTypeLabelMapNode->CopyWithScene(node); // copy all contents, including DMML node ID
      newTypeLabelMapNode->RemoveAttribute("LabelMap"); // this attribute is obsolete
      // replace the current node with the new one
      node->Delete();
      node=newTypeLabelMapNode;
      }
    }

  // Replace old-style subject hierarchy nodes (vtkDMMLSubjectHierarchy nodes without the version
  // attribute) with legacy node type that is handled by the hierarchy
  if (node->IsA("vtkDMMLSubjectHierarchyNode"))
    {
    const char* shVersionAttr = node->GetAttribute(
      vtkDMMLSubjectHierarchyNode::SUBJECTHIERARCHY_VERSION_ATTRIBUTE_NAME.c_str() );
    bool isOldShNode = shVersionAttr ? (atoi(shVersionAttr)<2) : true;
    if (isOldShNode)
      {
      // create a copy of the node of the correct class
      vtkDMMLSubjectHierarchyLegacyNode* legacyShNode = vtkDMMLSubjectHierarchyLegacyNode::New(); // Type is not registered
      // Set scene and read attributes manually, because CopyWithScene does not work due to vtkDMMLSubjectHierarchy node not
      // being child class of vtkDMMLHierarchyNode, and copying non-existent node references results in invalid memory access
      legacyShNode->SetScene(this->GetDMMLScene());
      legacyShNode->ReadXMLAttributes(atts);
      legacyShNode->HideFromEditorsOff(); // disable hide from editors so that the nodes can be added to subject hierarchy
      // replace the current node with the new one
      node->Delete();
      node=legacyShNode;
      }
    }

  if (!this->NodeStack.empty())
    {
    vtkDMMLNode* parentNode = this->NodeStack.top();
    parentNode->ProcessChildNode(node);
    }

  this->NodeStack.push(node);

  if (this->NodeCollection)
    {
    if (node->GetAddToScene())
      {
      this->NodeCollection->vtkCollection::AddItem((vtkObject *)node);
      }
    }
  else
    {
    this->DMMLScene->AddNode(node);
    }
  node->Delete();
}

//-----------------------------------------------------------------------------

void vtkDMMLParser::EndElement(const char *name)
{
  if ( !strcmp(name, "DMML") || this->NodeStack.empty() )
    {
    return;
    }

  const char* className = this->DMMLScene->GetClassNameByTag(name);
  if (className == nullptr)
    {
    // check for a renamed node
    if (strcmp(name, "SceneSnapshot") == 0)
      {
      className = this->DMMLScene->GetClassNameByTag("SceneView");
      if (className == nullptr)
        {
        return;
        }
      }
    else
      {
      return;
      }
    }

  this->NodeStack.pop();
}
