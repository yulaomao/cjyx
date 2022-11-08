/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLStorableNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLMessageCollection.h"
#include "vtkDMMLStorableNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSequenceStorageNode.h"
#include "vtkDMMLStorageNode.h"
#include "vtkTagTable.h"

// VTK includes
#include <vtkCallbackCommand.h>

// STD includes
#include <sstream>

const char* vtkDMMLStorableNode::StorageNodeReferenceRole = "storage";
const char* vtkDMMLStorableNode::StorageNodeReferenceDMMLAttributeName = "storageNodeRef";

//----------------------------------------------------------------------------
vtkDMMLStorableNode::vtkDMMLStorableNode()
{
  this->UserTagTable = vtkTagTable::New();
  this->CjyxDataType = "";
  this->DefaultSequenceStorageNodeClassName = "vtkDMMLSequenceStorageNode";
  this->AddNodeReferenceRole(this->GetStorageNodeReferenceRole(),
                             this->GetStorageNodeReferenceDMMLAttributeName());
}

//----------------------------------------------------------------------------
vtkDMMLStorableNode::~vtkDMMLStorableNode()
{
  if ( this->UserTagTable )
    {
    this->UserTagTable->Delete();
    this->UserTagTable = nullptr;
    }
  this->CjyxDataType.clear();
}

//----------------------------------------------------------------------------
const char* vtkDMMLStorableNode::GetStorageNodeReferenceRole()
{
  return vtkDMMLStorableNode::StorageNodeReferenceRole;
}

//----------------------------------------------------------------------------
const char* vtkDMMLStorableNode::GetStorageNodeReferenceDMMLAttributeName()
{
  return vtkDMMLStorableNode::StorageNodeReferenceDMMLAttributeName;
}

//----------------------------------------------------------------------------
void vtkDMMLStorableNode::SetAndObserveStorageNodeID(const char *storageNodeID)
{
  this->SetAndObserveNodeReferenceID(this->GetStorageNodeReferenceRole(), storageNodeID);
}

//----------------------------------------------------------------------------
void vtkDMMLStorableNode::AddAndObserveStorageNodeID(const char *storageNodeID)
{
  this->AddAndObserveNodeReferenceID(this->GetStorageNodeReferenceRole(), storageNodeID);
}

//----------------------------------------------------------------------------
void vtkDMMLStorableNode::SetAndObserveNthStorageNodeID(int n, const char *storageNodeID)
{
  this->SetAndObserveNthNodeReferenceID(this->GetStorageNodeReferenceRole(), n, storageNodeID);
}

//----------------------------------------------------------------------------
bool vtkDMMLStorableNode::HasStorageNodeID(const char* storageNodeID)
{
  return this->HasNodeReferenceID(this->GetStorageNodeReferenceRole(), storageNodeID);
}

//----------------------------------------------------------------------------
void vtkDMMLStorableNode::SetCjyxDataType ( const char *type )
{
  this->CjyxDataType.clear();
  this->CjyxDataType = type;
  if (this->Scene)
    {
    this->Scene->InvokeEvent ( vtkDMMLScene::MetadataAddedEvent );
    }
}


//----------------------------------------------------------------------------
const char* vtkDMMLStorableNode::GetCjyxDataType ()
{
  return ( this->CjyxDataType.c_str() );
}

//----------------------------------------------------------------------------
int vtkDMMLStorableNode::GetNumberOfStorageNodes()
{
  return this->GetNumberOfNodeReferences(this->GetStorageNodeReferenceRole());
}

//----------------------------------------------------------------------------
const char* vtkDMMLStorableNode::GetNthStorageNodeID(int n)
{
  return this->GetNthNodeReferenceID(this->GetStorageNodeReferenceRole(), n);
}

//----------------------------------------------------------------------------
const char* vtkDMMLStorableNode::GetStorageNodeID()
{
return this->GetNthStorageNodeID(0);
}

//----------------------------------------------------------------------------
void vtkDMMLStorableNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  std::stringstream ss;

  //---write any user tags.
  if ( this->GetUserTagTable() != nullptr )
    {
    ss.clear();
    ss.str ( "" );
    int numc = this->GetUserTagTable()->GetNumberOfTags();
    const char *kwd, *val;
    for (int i=0; i < numc; i++ )
      {
      kwd = this->GetUserTagTable()->GetTagAttribute(i);
      val = this->GetUserTagTable()->GetTagValue (i);
      if (kwd != nullptr && val != nullptr)
        {
        ss << kwd << "=" << val;
        if ( i < (numc-1) )
          {
          ss << " ";
          }
        }
      }
    if ( ss.str().c_str()!= nullptr )
      {
      of << " userTags=\"" << ss.str().c_str() << "\"";
      }
    }
}


//----------------------------------------------------------------------------
void vtkDMMLStorableNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    //---Read any user tags
    if (!strcmp (attName, "userTags"))
      {
      if ( this->GetUserTagTable() == nullptr )
        {
        this->UserTagTable = vtkTagTable::New();
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
              this->GetUserTagTable()->AddOrUpdateTag ( kwd.c_str(), val.c_str(), 0 );
              }
            }
          }
        }
      }
    }

  this->EndModify(disabledModify);

}

//----------------------------------------------------------------------------
void vtkDMMLStorableNode::Copy(vtkDMMLNode* anode)
{
  Superclass::Copy(anode);
  vtkDMMLStorableNode *node = (vtkDMMLStorableNode *) anode;
  if (!node)
    {
    return;
    }

  this->SetDefaultSequenceStorageNodeClassName(node->GetDefaultSequenceStorageNodeClassName());
}

//----------------------------------------------------------------------------
void vtkDMMLStorableNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkDMMLStorableNode *node = (vtkDMMLStorableNode *) anode;
  if (!node)
    {
    return;
    }

  //---
  //--- Copy any user tags
  //---
  if ( node->GetUserTagTable() != nullptr )
    {
    //--- make sure the destination node has a TagTable.
    if ( this->GetUserTagTable() == nullptr )
      {
      this->UserTagTable = vtkTagTable::New();
      }

    //--- copy.
    int numc = node->GetUserTagTable()->GetNumberOfTags();
    const char *kwd, *val;
    int sel;
    for ( int j=0; j < numc; j++ )
      {
      kwd = node->GetUserTagTable()->GetTagAttribute(j);
      val = node->GetUserTagTable()->GetTagValue (j);
      sel = node->GetUserTagTable()->IsTagSelected ( kwd );
      if (kwd != nullptr && val != nullptr && sel >= 0 )
        {
        this->UserTagTable->AddOrUpdateTag ( kwd, val, sel );
        }
      }
    }

  this->SetDefaultSequenceStorageNodeClassName(node->GetDefaultSequenceStorageNodeClassName());
}

//----------------------------------------------------------------------------
void vtkDMMLStorableNode::PrintSelf(ostream& os, vtkIndent indent)
{

  Superclass::PrintSelf(os,indent);
  this->UserTagTable->PrintSelf(os, indent);

  int numStorageNodes = this->GetNumberOfNodeReferences(this->GetStorageNodeReferenceRole());

  for (int i=0; i < numStorageNodes; i++)
    {
    const char * id = this->GetNthNodeReferenceID(this->GetStorageNodeReferenceRole(), i);
    os << indent << "StorageNodeIDs[" << i << "]: " <<
      (id ? id : "(none)") << "\n";
    }
}


//-----------------------------------------------------------
void vtkDMMLStorableNode::UpdateScene(vtkDMMLScene *scene)
{
  Superclass::UpdateScene(scene);

  if (!this->AddToScene)
    {
    return;
    }

  std::string errorMessages;

  int numStorageNodes = this->GetNumberOfNodeReferences(this->GetStorageNodeReferenceRole());
  vtkDebugMacro("UpdateScene: going through the storage node ids: " <<  numStorageNodes);
  for (int i=0; i < numStorageNodes; i++)
    {
    vtkDebugMacro("UpdateScene: getting storage node at i = " << i);
    vtkDMMLStorageNode *pnode = this->GetNthStorageNode(i);

    std::string fname = std::string("(null)");
    if (pnode)
      {
      if (pnode->GetFileName() != nullptr)
        {
        fname = std::string(pnode->GetFileName());
        }
      else if (pnode->GetURI() != nullptr)
        {
        fname = std::string(pnode->GetURI());
        }
      vtkDebugMacro("UpdateScene: calling ReadData, fname = " << fname.c_str());
      pnode->GetUserMessages()->ClearMessages();
      if (pnode->ReadData(this) == 0)
        {
        std::string msg = std::string("Failed to read node ") + (this->GetName() ? this->GetName() : "(null)")
          + " (" + (this->GetID() ? this->GetID() : "(null)") + ") using storage node "
          + (pnode->GetID() ? pnode->GetID() : "(null)") + ".";
        vtkErrorMacro("vtkDMMLStorableNode::UpdateScene failed: " << msg);
        errorMessages += msg;
        std::string details = pnode->GetUserMessages()->GetAllMessagesAsString();
        if (!details.empty())
          {
          errorMessages += " Details:\n" + details;
          }
        }
      else
        {
        vtkDebugMacro("UpdateScene: read data called and succeeded reading " << fname.c_str());
        }
      }
    else
      {
      vtkErrorMacro("UpdateScene: error getting " << i << "th storage node, id = " << (this->GetNthStorageNodeID(i) == nullptr ? "null" : this->GetNthStorageNodeID(i)));
      }
    }
}

vtkDMMLStorageNode* vtkDMMLStorableNode::GetNthStorageNode(int n)
{
  return vtkDMMLStorageNode::SafeDownCast(this->GetNthNodeReference(this->GetStorageNodeReferenceRole(), n));
}

vtkDMMLStorageNode* vtkDMMLStorableNode::GetStorageNode()
{
  return this->GetNthStorageNode(0);
}



/*
std::vector<vtkDMMLStorageNode*> vtkDMMLStorableNode::GetStorageNodes()const
{
}

//----------------------------------------------------------------------------
vtkDMMLStorageNode* vtkDMMLStorableNode::GetStorageNode()
{
  vtkDMMLStorageNode* node = nullptr;
  if (this->GetScene() && this->GetStorageNodeID() )
    {
    vtkDMMLNode* snode = this->GetScene()->GetNodeByID(this->StorageNodeID);
    node = vtkDMMLStorageNode::SafeDownCast(snode);
    }
  return node;
}
*/

//---------------------------------------------------------------------------
void vtkDMMLStorableNode::ProcessDMMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessDMMLEvents(caller, event, callData);

  int numStorageNodes = this->GetNumberOfNodeReferences(this->GetStorageNodeReferenceRole());

  for (int i=0; i<numStorageNodes; i++)
    {
    vtkDMMLStorageNode *dnode = this->GetNthStorageNode(i);
    if (dnode != nullptr && dnode == vtkDMMLStorageNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
      {
      vtkDebugMacro("Got a modified event on a storage node, id = " << dnode->GetID());
      // read?
      }
    }
  return;
}

//---------------------------------------------------------------------------
vtkDMMLStorageNode* vtkDMMLStorableNode::CreateDefaultStorageNode()
{
  return nullptr;
}

//---------------------------------------------------------------------------
bool vtkDMMLStorableNode::GetModifiedSinceRead()
{
  vtkTimeStamp storedTime = this->GetStoredTime();
  return storedTime < this->StorableModifiedTime;
}

//---------------------------------------------------------------------------
void vtkDMMLStorableNode::StorableModified()
{
  this->StorableModifiedTime.Modified();
}

//---------------------------------------------------------------------------
vtkTimeStamp vtkDMMLStorableNode::GetStoredTime()
{
  vtkTimeStamp storedTime;

  int numStorageNodes = this->GetNumberOfNodeReferences(this->GetStorageNodeReferenceRole());

  for (int i = 0; i < numStorageNodes; ++i)
    {
    vtkDMMLStorageNode *dnode = this->GetNthStorageNode(i);
    if (dnode != nullptr && storedTime < dnode->GetStoredTime())
      {
      storedTime = dnode->GetStoredTime();
      }
    }
  return storedTime;
}

//---------------------------------------------------------------------------
std::string vtkDMMLStorableNode::GetDefaultStorageNodeClassName(const char* vtkNotUsed(filename) /* =nullptr */)
{
  std::string defaultStorageNodeClassName;
  vtkSmartPointer<vtkDMMLStorageNode> defaultStorageNode = vtkSmartPointer<vtkDMMLStorageNode>::Take(this->CreateDefaultStorageNode());
  if (defaultStorageNode && defaultStorageNode->GetClassName())
    {
    defaultStorageNodeClassName = defaultStorageNode->GetClassName();
    }
  return defaultStorageNodeClassName;
}

//---------------------------------------------------------------------------
bool vtkDMMLStorableNode::AddDefaultStorageNode(const char* filename /* =nullptr */)
{
  vtkDMMLStorageNode* storageNode = this->GetStorageNode();
  if (storageNode)
    {
    // storage node exists already, no need to add a new one
    return true;
    }
  std::string defaultStorageNodeClassName = this->GetDefaultStorageNodeClassName(filename);
  if (defaultStorageNodeClassName.empty())
    {
    // node can be stored in the scene
    return true;
    }
  if (!this->GetScene())
  {
    vtkErrorMacro("vtkDMMLStorableNode::AddDefaultStorageNode failed: node is not in a scene " << (this->GetID() ? this->GetID() : "(unknown)"));
    return false;
  }
  vtkSmartPointer<vtkDMMLNode> newStorageNode = vtkSmartPointer<vtkDMMLNode>::Take(this->GetScene()->CreateNodeByClass(defaultStorageNodeClassName.c_str()));
  storageNode = vtkDMMLStorageNode::SafeDownCast(newStorageNode);
  if (!storageNode)
    {
    vtkErrorMacro("vtkDMMLStorableNode::AddDefaultStorageNode failed: failed to create storage node for node "
      << (this->GetID() ? this->GetID() : "(unknown)"));
    return false;
    }
  storageNode->SetFileName(filename);
  this->GetScene()->AddNode(storageNode);
  this->SetAndObserveStorageNodeID(storageNode->GetID());
  return storageNode;
}

//---------------------------------------------------------------------------
vtkDMMLStorageNode* vtkDMMLStorableNode:: CreateDefaultSequenceStorageNode()
{
  vtkObject* ret = nullptr;
  if (this->GetScene())
    {
    ret = this->GetScene()->CreateNodeByClass(this->DefaultSequenceStorageNodeClassName.c_str());
    }

  vtkDMMLStorageNode* storageNode = vtkDMMLStorageNode::SafeDownCast(ret);
  if (storageNode)
    {
    return storageNode;
    }

  if (ret)
    {
    // Specified class is not a valid sequence storage node
    ret->Delete();
    }

  // No valid sequence storage node
  return nullptr;
}
