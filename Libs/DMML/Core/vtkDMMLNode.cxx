/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.11 $

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLNode.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// STD includes
#include <iostream>
#include <sstream>
#include <algorithm> // for std::sort

//------------------------------------------------------------------------------
vtkDMMLNode::vtkDMMLNode()
{
  this->ContentModifiedEvents = vtkIntArray::New();
  this->ContentModifiedEvents->InsertNextValue(vtkCommand::ModifiedEvent);

  // Set up callbacks
  this->DMMLCallbackCommand = vtkCallbackCommand::New();
  this->DMMLCallbackCommand->SetClientData( reinterpret_cast<void *>(this) );
  this->DMMLCallbackCommand->SetCallback( vtkDMMLNode::DMMLCallback );

  this->DMMLObserverManager = vtkObserverManager::New();
  this->DMMLObserverManager->AssignOwner( this );
  this->DMMLObserverManager->GetCallbackCommand()->SetClientData( reinterpret_cast<void *> (this) );
  this->DMMLObserverManager->GetCallbackCommand()->SetCallback(vtkDMMLNode::DMMLCallback);
}

//----------------------------------------------------------------------------
vtkDMMLNode::~vtkDMMLNode()
{
  // Delete all references in bulk, which is faster than RemoveNodeReferenceIDs
  // but it does not send individual NodeReferencesRemoved events (the whole class
  // is deleted, so references cannot be meaningful anymore) or updates node
  // references in the scene (the scene removes all node references anyway when
  // a node is deleted).
  // Need to remove all observers by calling InvalidateNodeReferences
  // before clearing this->NodeReferences to avoid memory leaks.
  this->InvalidateNodeReferences();
  this->NodeReferences.clear();
  this->NodeReferenceEvents.clear();

  this->SetID(nullptr);
  this->SetName(nullptr);
  this->SetDescription(nullptr);

  if (this->DMMLObserverManager)
    {
    this->DMMLObserverManager->Delete();
    this->DMMLObserverManager = nullptr;
    }

  // unregister and set null pointers.
  if ( this->DMMLCallbackCommand )
    {
    this->DMMLCallbackCommand->SetClientData( nullptr );
    this->DMMLCallbackCommand->Delete ( );
    this->DMMLCallbackCommand = nullptr;
    }

  if (this->ContentModifiedEvents)
    {
    this->ContentModifiedEvents->Delete();
    this->ContentModifiedEvents = nullptr;
    }

  this->SetTempURLString(nullptr);
  this->SetSingletonTag(nullptr);
}

//----------------------------------------------------------------------------
void vtkDMMLNode::CopyWithScene(vtkDMMLNode *node)
{
  DMMLNodeModifyBlocker blocker(this);
  if (node->GetScene())
    {
    this->SetScene(node->GetScene());
    }
  if (node->GetID())
    {
    this->SetID( node->GetID() );
    }
  this->Copy(node);
}

//----------------------------------------------------------------------------
void vtkDMMLNode::Copy(vtkDMMLNode *node)
{
  DMMLNodeModifyBlocker blocker(this);
  vtkDMMLCopyBeginMacro(node);

  // Only copy the node name when in the source node it is not empty
  // this is used for singleton node updates and for resetting nodes.
  if (node->GetName() && strcmp(node->GetName(),""))
    {
    vtkDMMLCopyStringMacro(Name);
    }
  vtkDMMLCopyBooleanMacro(HideFromEditors);
  vtkDMMLCopyBooleanMacro(AddToScene);
  if (node->GetSingletonTag())
    {
    vtkDMMLCopyStringMacro(SingletonTag);
    }
  vtkDMMLCopyBooleanMacro(UndoEnabled);
  vtkDMMLCopyEndMacro();
  this->CopyContent(node);
  this->CopyReferences(node);
}

//----------------------------------------------------------------------------
void vtkDMMLNode::CopyContent(vtkDMMLNode* node, bool vtkNotUsed(deepCopy)/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  vtkDMMLCopyBeginMacro(node);
  vtkDMMLCopyStringMacro(Description);
  vtkDMMLCopyBooleanMacro(Selectable);
  vtkDMMLCopyEndMacro();
  this->Attributes = node->Attributes;
}

//----------------------------------------------------------------------------
bool ArraysEqual(vtkIntArray* array1, vtkIntArray* array2)
{
  if (array1 == nullptr && array2 == nullptr)
    {
    return true;
    }
  if (array1 == nullptr || array2 == nullptr)
    {
    return false;
    }
  if (array1->GetNumberOfTuples() != array2->GetNumberOfTuples())
    {
    return false;
    }
  int arraySize = array1->GetNumberOfTuples();
  for (int i = 0; i<arraySize; i++)
    {
    if (array1->GetValue(i) != array2->GetValue(i))
      {
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
void vtkDMMLNode::CopyReferences(vtkDMMLNode* node)
{
  this->NodeReferenceDMMLAttributeNames = node->NodeReferenceDMMLAttributeNames;
  this->NodeReferenceEvents = node->NodeReferenceEvents;

  // Only update references is they are different (to avoid unnecessary event invocations)
  std::vector<std::string> referenceRoles;
  this->GetNodeReferenceRoles(referenceRoles);
  std::vector<std::string> referenceRolesInSource;
  node->GetNodeReferenceRoles(referenceRolesInSource);
  bool referencesAreEqual = false;
  if (referenceRoles == referenceRolesInSource)
    {
    referencesAreEqual = true;
    for (NodeReferencesType::iterator sourceNodeReferencesIt = node->NodeReferences.begin();
      sourceNodeReferencesIt != node->NodeReferences.end() && referencesAreEqual; sourceNodeReferencesIt++)
      {
      std::string referenceRole = sourceNodeReferencesIt->first;
      NodeReferencesType::iterator targetNodeReferencesIt = this->NodeReferences.find(referenceRole);
      if (sourceNodeReferencesIt->second.size() != targetNodeReferencesIt->second.size())
        {
        referencesAreEqual = false;
        break;
        }
      int numberOfNodeReferences = sourceNodeReferencesIt->second.size();
      for (int i = 0; i < numberOfNodeReferences; i++)
        {
        vtkDMMLNodeReference* sourceReference = sourceNodeReferencesIt->second[i];
        vtkDMMLNodeReference* targetReference = targetNodeReferencesIt->second[i];
        if (!sourceReference || !targetReference)
          {
          vtkErrorMacro(<< "CopyReferences: invalid reference found.");
          referencesAreEqual = false;
          break;
          }
        if (strcmp(sourceReference->GetReferencedNodeID(), targetReference->GetReferencedNodeID()) != 0
          || !ArraysEqual(sourceReference->GetEvents(), targetReference->GetEvents()))
          {
          referencesAreEqual = false;
          break;
          }
        }
      }
    }

  if (referencesAreEqual)
    {
    // no need to copy, they are already the same
    return;
    }

  // Remove all existing references
  this->RemoveNodeReferenceIDs(nullptr);

  // Add node references
  for (NodeReferencesType::iterator it = node->NodeReferences.begin(); it != node->NodeReferences.end(); it++)
    {
    std::string referenceRole = it->first;
    NodeReferenceListType::iterator it1;
    for (it1 = it->second.begin(); it1 != it->second.end(); it1++)
      {
      vtkDMMLNodeReference* reference = *it1;
      if (!reference)
        {
        vtkErrorMacro(<< "CopyReferences: Reference is expected to be non nullptr.");
        return;
        }
      // We must not use SetAndObserveNthNodeReferenceID here, because referenced node IDs may change
      // due to conflict with node IDs existing in the scene.
      vtkNew<vtkDMMLNodeReference> copiedReference;
      copiedReference->SetReferenceRole(referenceRole.c_str());
      copiedReference->SetReferencedNodeID(reference->GetReferencedNodeID());
      copiedReference->SetReferencingNode(this);
      copiedReference->SetEvents(reference->GetEvents());
      this->NodeReferences[std::string(referenceRole)].push_back(copiedReference.GetPointer());
      }
    }
}

//----------------------------------------------------------------------------
void vtkDMMLNode::Reset(vtkDMMLNode* defaultNode)
{
  vtkSmartPointer<vtkDMMLNode> newNode;
  if (defaultNode)
    {
    newNode = defaultNode;
    }
  else
    {
    newNode = vtkSmartPointer<vtkDMMLNode>::Take(this->CreateNodeInstance());
    }

  // Preserve SaveWithScene, HideFromEditors, Selectable, and SingletonTag
  // properties during reset by saving the original values and then restoring them.

  // Save
  int save = this->GetSaveWithScene();
  int hide = this->GetHideFromEditors();
  int select = this->GetSelectable();
  char *tag = this->GetSingletonTag();

  int wasModifying = this->StartModify();

  // Copy
  this->CopyWithScene(newNode);

  // Restore
  this->SetSaveWithScene(save);
  this->SetHideFromEditors(hide);
  this->SetSelectable(select);
  this->SetSingletonTag(tag);

  this->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
void vtkDMMLNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDMMLPrintBeginMacro(os, indent);
  vtkDMMLPrintStringMacro(ID);
  vtkDMMLPrintStringMacro(ClassName);
  vtkDMMLPrintStringMacro(Name);

  // vtkObject's PrintSelf prints a long list of registered events, which
  // is too long and not useful, therefore we don't call vtkObject::PrintSelf
  // but print essential information on the vtkObject base.
  vtkDMMLPrintBooleanMacro(Debug);
  vtkDMMLPrintIntMacro(MTime);

  vtkDMMLPrintStringMacro(Description);
  vtkDMMLPrintStringMacro(SingletonTag);
  vtkDMMLPrintBooleanMacro(HideFromEditors);
  vtkDMMLPrintBooleanMacro(Selectable);
  vtkDMMLPrintBooleanMacro(Selected);
  vtkDMMLPrintBooleanMacro(UndoEnabled)
  vtkDMMLPrintEndMacro();

  if (!this->Attributes.empty())
    {
    os << indent << "Attributes:\n";
    AttributesType::const_iterator it;
    AttributesType::const_iterator begin = this->Attributes.begin();
    AttributesType::const_iterator end = this->Attributes.end();
    for (it = begin; it != end; ++it)
      {
      os << indent.GetNextIndent() << it->first << ':' << it->second << "\n";
      }
    }

  if (!this->NodeReferences.empty())
    {
    os << indent << "Node references:\n";
    NodeReferencesType::iterator it;
    for (it = this->NodeReferences.begin(); it != this->NodeReferences.end(); it++)
      {
      const std::string& referenceRole = it->first;
      os << indent.GetNextIndent() << referenceRole;
      const char* refAttribute = this->GetDMMLAttributeNameFromReferenceRole(referenceRole.c_str());
      if (refAttribute != nullptr)
        {
        os << " [" << refAttribute << "]";
        }
      os << ":";
      std::vector< const char* > referencedNodeIds;
      GetNodeReferenceIDs(referenceRole.c_str(), referencedNodeIds);
      if (referencedNodeIds.empty())
        {
        os << " (none)\n";
        }
      else
        {
        for (std::vector< const char* >::iterator referencedNodeIdsIt=referencedNodeIds.begin(); referencedNodeIdsIt!=referencedNodeIds.end(); ++referencedNodeIdsIt)
          {
          const char * id = *referencedNodeIdsIt;
          os << " " << (id ? id : "(nullptr)");
          }
        os << "\n";
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkDMMLNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLStringMacro(id, ID);
  vtkDMMLReadXMLStringMacro(name, Name);
  vtkDMMLReadXMLStringMacro(description, Description);
  vtkDMMLReadXMLBooleanMacro(hideFromEditors, HideFromEditors);
  vtkDMMLReadXMLBooleanMacro(selectable, Selectable);
  vtkDMMLReadXMLBooleanMacro(selected, Selected);
  vtkDMMLReadXMLStringMacro(singletonTag, SingletonTag);
  vtkDMMLReadXMLBooleanMacro(undoEnabled, UndoEnabled)
  vtkDMMLReadXMLEndMacro();

  std::set<std::string> references;
  const char** xmlReadAtts = atts;
  const char* attName;
  const char* attValue;
  while (*xmlReadAtts != nullptr)
    {
    attName = *(xmlReadAtts++);
    attValue = *(xmlReadAtts++);
    if (!strcmp(attName, "attributes"))
      {
      std::stringstream attributes(attValue);
      std::string attribute;
      while (std::getline(attributes, attribute, ';'))
        {
        int colonIndex = attribute.find(':');
        std::string name = attribute.substr(0, colonIndex);
        std::string value = attribute.substr(colonIndex + 1);
        // decode percent sign and semicolon (semicolon is a special character because it separates attributes)
        vtksys::SystemTools::ReplaceString(value, "%3B", ";");
        vtksys::SystemTools::ReplaceString(value, "%25", "%");
        this->SetAttribute(name.c_str(), value.c_str());
        }
      }
    else if (!strcmp(attName, "references"))
      {
      this->ParseReferencesAttribute(attValue, references);
      }
    }

  // It is important that we look for legacy node references after the "reference" attribute.
  // If the reference has already been read from the "references" attribute, then we skip it here.
  xmlReadAtts = atts;
  while (*xmlReadAtts != nullptr)
    {
    attName = *(xmlReadAtts++);
    attValue = *(xmlReadAtts++);
    if ( const char* referenceRole =
                this->GetReferenceRoleFromDMMLAttributeName(attName) )
      {
      // Reference role has already been read
      if (references.find(referenceRole) != references.end())
        {
        continue;
        }

      std::stringstream ss(attValue);
      while (!ss.eof())
        {
        std::string id;
        ss >> id;
        if (!id.empty())
          {
          this->AddNodeReferenceID(referenceRole, id.c_str());
          references.insert(std::string(referenceRole));
          }
        }
      }
    }
  this->EndModify(disabledModify);

  return;
}


//----------------------------------------------------------------------------
void vtkDMMLNode::ParseReferencesAttribute(const char *attValue, std::set<std::string> &references)
{
  /// parse references in the form "role1:id1 id2;role2:id3;"
  std::string attribute(attValue);

  std::size_t start = 0;
  std::size_t end = attribute.find_first_of(';', start);
  std::size_t sep = attribute.find_first_of(':', start);
  while (start != std::string::npos && sep != std::string::npos && start != end && start != sep)
    {
    std::string role = attribute.substr(start, sep-start);
    // Only process this role if it has not been encountered already
    if (references.find(role) == references.end())
      {
      std::string ids = attribute.substr(sep + 1, end - sep - 1);
      std::stringstream ss(ids);
      while (!ss.eof())
        {
        std::string id;
        ss >> id;
        if (!id.empty())
          {
          this->AddNodeReferenceID(role.c_str(), id.c_str());
          references.insert(role);
          }
        }
      }
    start = (end == std::string::npos) ? std::string::npos : end+1;
    end = attribute.find_first_of(';', start);
    sep = attribute.find_first_of(':', start);
    }
}


//----------------------------------------------------------------------------
void vtkDMMLNode::WriteXML(ostream& of, int nIndent)
{
  vtkIndent indent(nIndent);

  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLStringMacro(id, ID);
  vtkDMMLWriteXMLStringMacro(name, Name);
  vtkDMMLWriteXMLStringMacro(description, Description);
  vtkDMMLWriteXMLBooleanMacro(hideFromEditors, HideFromEditors);
  vtkDMMLWriteXMLBooleanMacro(selectable, Selectable);
  vtkDMMLWriteXMLBooleanMacro(selected, Selected);
  vtkDMMLWriteXMLStringMacro(singletonTag, SingletonTag);
  if (this->UndoEnabled)
    {
    // Only write out UndoEnabled flag in case of non-default value is used,
    // to keep the written XML file cleaner.
    vtkDMMLWriteXMLBooleanMacro(undoEnabled, UndoEnabled);
    }
  vtkDMMLWriteXMLEndMacro();

  if (this->Attributes.size())
    {
    of << " attributes=\"";
    AttributesType::const_iterator it;
    AttributesType::const_iterator begin = this->Attributes.begin();
    AttributesType::const_iterator end = this->Attributes.end();
    for (it = begin; it != end; ++it)
      {
      if (it != begin)
        {
        of << ';';
        }
      std::string attributeValue = it->second;
      // encode percent sign and semicolon (semicolon is a special character because it separates attributes)
      vtksys::SystemTools::ReplaceString(attributeValue, "%", "%25");
      vtksys::SystemTools::ReplaceString(attributeValue, ";", "%3B");
      of << this->XMLAttributeEncodeString(it->first) << ':' << this->XMLAttributeEncodeString(attributeValue);
      }
    of << "\"";
    }

  //write node references
  std::stringstream ssRef;
  NodeReferencesType::iterator it;
  std::map< std::string, std::string>::iterator itName;
  for (it = this->NodeReferences.begin(); it != this->NodeReferences.end(); it++)
    {
    const std::string& referenceRole = it->first;
    int numReferencedNodes = this->GetNumberOfNodeReferences(referenceRole.c_str());

    bool referenceFound = false;
    for (int n=0; n < numReferencedNodes; n++)
      {
      const char * id = this->GetNthNodeReferenceID(referenceRole.c_str(), n);
      if (!id)
        {
        continue;
        }
      if (referenceFound)
        {
        // additional ID
        ssRef << " ";
        }
      else
        {
        // first ID
        ssRef << referenceRole << ":";
        referenceFound = true;
        }
      ssRef << id;
      }
    if (referenceFound)
      {
      ssRef << ";";
      }
    }

    if (!(ssRef.str().empty()))
      {
      of << " " << "references=\"" << ssRef.str().c_str() << "\"";
      }
}

//----------------------------------------------------------------------------
void vtkDMMLNode::WriteNodeBodyXML(ostream &, int )
{
}

//----------------------------------------------------------------------------
void vtkDMMLNode::ProcessDMMLEvents (vtkObject *caller,
                                     unsigned long event,
                                     void *vtkNotUsed(callData) )
{

  // Refreshes the reference (by calling GetNthNodeReference).
  // Also invokes vtkDMMLNode::ReferencedNodeModifiedEvent if any of the observed
  // referenced nodes are changed. Used by extensions.
  NodeReferencesType::iterator it;
  for (it=this->NodeReferences.begin(); it!=this->NodeReferences.end(); it++)
    {
    if (it->first.c_str())
      {
      NodeReferenceListType references = this->NodeReferences[it->first];
      for (unsigned int i=0; i<references.size(); i++)
        {
        vtkDMMLNode *node = this->GetNthNodeReference(it->first.c_str(), i);
        if (node != nullptr && node == vtkDMMLNode::SafeDownCast(caller) &&
          event ==  vtkCommand::ModifiedEvent)
          {
          this->InvokeEvent(vtkDMMLNode::ReferencedNodeModifiedEvent, node);
          }
        }
      }
    }
  return;
}

//----------------------------------------------------------------------------
vtkDMMLScene* vtkDMMLNode::GetScene()
{
  return this->Scene.GetPointer();
}

//----------------------------------------------------------------------------
void vtkDMMLNode::SetScene(vtkDMMLScene* scene)
{
  if (this->Scene == scene)
    {
    return;
    }
  if (this->Scene)
    {
    this->InvalidateNodeReferences();
    }

  this->Scene = scene;
  if (this->Scene)
    {
    this->SetSceneReferences();
    // We must not call UpdateNodeReferences() here yet, because referenced node IDs may change
    // due to conflict with node IDs existing in the scene.
    }
}

//----------------------------------------------------------------------------
void vtkDMMLNode::AddNodeReferenceRole(const char *refRole, const char *dmmlAttributeName, vtkIntArray *events)
{
  if (!refRole)
    {
    return;
    }
  const std::string referenceRole(refRole);
  this->NodeReferenceDMMLAttributeNames[referenceRole] =
    dmmlAttributeName ? std::string(dmmlAttributeName) : referenceRole;
  if (!this->IsReferenceRoleGeneric(refRole))
    {
    this->NodeReferences[referenceRole] = NodeReferenceListType();
    }

  this->NodeReferenceEvents[referenceRole] = vtkSmartPointer<vtkIntArray>::New();
  if (events)
    {
    for (int i=0; i<events->GetNumberOfTuples(); i++)
      {
      this->NodeReferenceEvents[referenceRole]->InsertNextValue(events->GetValue(i));
      }
    }
}

//----------------------------------------------------------------------------
const char* vtkDMMLNode::GetReferenceRoleFromDMMLAttributeName(const char* attName)
{
  if (attName == nullptr)
    {
    return nullptr;
    }
  std::string attributeName(attName);
  // Search if the attribute name has been registered using AddNodeReferenceRole.
  std::map< std::string, std::string>::iterator it;
  for (it = this->NodeReferenceDMMLAttributeNames.begin();
       it != this->NodeReferenceDMMLAttributeNames.end();
       ++it)
    {
    const std::string& nodeReferenceRole = it->first;
    const std::string& nodeDMMLAttributeName = it->second;
    if (nodeDMMLAttributeName == attributeName)
      {
      return nodeReferenceRole.c_str();
      }
    else if (this->IsReferenceRoleGeneric(nodeReferenceRole.c_str()) &&
             (attributeName.length() >= nodeDMMLAttributeName.length()) &&
             attributeName.compare(
              attributeName.length() - nodeDMMLAttributeName.length(),
              nodeDMMLAttributeName.length(), nodeDMMLAttributeName) == 0)
      {
      // if attName = "lengthUnitRef" and  [refRole,attName] = ["unit/","UnitRef"]
      // then return "unit/length"
      static std::string referenceRole;
      referenceRole = nodeReferenceRole;
      referenceRole += attributeName.substr(0, attributeName.length() -
                                               nodeDMMLAttributeName.length());
      return referenceRole.c_str();
      }
    }
  return nullptr;
}

//----------------------------------------------------------------------------
const char* vtkDMMLNode
::GetDMMLAttributeNameFromReferenceRole(const char* refRole)
{
  if (refRole == nullptr)
    {
    return nullptr;
    }
  std::string referenceRole(refRole);
  // Try first to see if the reference role is registered as is.
  std::map< std::string, std::string>::const_iterator it =
    this->NodeReferenceDMMLAttributeNames.find(referenceRole);
  if (it != this->NodeReferenceDMMLAttributeNames.end())
    {
    return it->second.c_str();
    }
  // Otherwise, it might be a generic reference role.
  for (it = this->NodeReferenceDMMLAttributeNames.begin();
       it != this->NodeReferenceDMMLAttributeNames.end();
       ++it)
    {
    const std::string& nodeReferenceRole = it->first;
    const std::string& nodeDMMLAttributeName = it->second;
    if (this->IsReferenceRoleGeneric(nodeReferenceRole.c_str()) &&
        referenceRole.compare(0, nodeReferenceRole.length(),
                              nodeReferenceRole) == 0)
      {
      // if refRole = "unit/length" and  [refRole,attName] = ["unit/","UnitRef"]
      // then return "lengthUnitRef"
      static std::string dmmlAttributeName;
      dmmlAttributeName = referenceRole.substr(nodeReferenceRole.length());
      dmmlAttributeName += nodeDMMLAttributeName;
      return dmmlAttributeName.c_str();
      }
    }
  return nullptr;
}

//----------------------------------------------------------------------------
bool vtkDMMLNode::IsReferenceRoleGeneric(const char* refRole)
{
  if (refRole == nullptr)
    {
    return false;
    }
  std::string nodeReferenceRole(refRole);
  return nodeReferenceRole.at(nodeReferenceRole.length()-1) == '/';
}

//----------------------------------------------------------------------------
void vtkDMMLNode::SetSceneReferences()
{
  if (!this->Scene)
    {
    vtkErrorMacro(<< "SetSceneReferences: Scene is expected to be non nullptr.");
    return;
    }

  NodeReferencesType::iterator it;
  for (it = this->NodeReferences.begin(); it != this->NodeReferences.end(); it++)
    {
    for (unsigned int i=0; i<it->second.size(); i++)
      {
      vtkDMMLNodeReference* reference = it->second[i];
      if (!reference)
        {
        vtkErrorMacro(<< "SetSceneReferences: Reference " << i << " is expected to be non nullptr.");
        return;
        }
      this->Scene->AddReferencedNodeID(reference->GetReferencedNodeID(), this);
      }
    }
}


//----------------------------------------------------------------------------
void vtkDMMLNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  int wasModifying = this->StartModify();

  NodeReferencesType::iterator it;
  for (it = this->NodeReferences.begin(); it != this->NodeReferences.end(); it++)
    {
    for (unsigned int i=0; i<it->second.size(); i++)
      {
      vtkDMMLNodeReference* reference = it->second[i];
      if (!reference)
        {
        vtkErrorMacro(<< "UpdateReferenceID: Reference " << i << " is expected to be non nullptr.");
        continue;
        }
      if (std::string(oldID) == std::string(reference->GetReferencedNodeID()))
        {
        this->SetAndObserveNthNodeReferenceID(reference->GetReferenceRole(), i, newID, reference->GetEvents());
        }
      }
    }
  this->EndModify(wasModifying);
}


//----------------------------------------------------------------------------
void vtkDMMLNode::SetAttribute(const char* name, const char* value)
{
  if (!name)
    {
    vtkErrorMacro(<< "SetAttribute: Name parameter is expected to be non nullptr.");
    return;
    }
  if (strlen(name) == 0)
    {
    vtkErrorMacro(<< "SetAttribute: Name parameter is expected to have at least one character.");
    return;
    }
  const char* oldValue = this->GetAttribute(name);
  if ((!oldValue && !value) ||
      (oldValue && value && !strcmp(oldValue, value)))
    {
    return;
    }
  if (value != nullptr)
    {
    this->Attributes[std::string(name)] = std::string(value);
    }
  else
    {
    this->Attributes.erase(std::string(name));
    }
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkDMMLNode::RemoveAttribute(const char* name)
{
  if (!name)
    {
    vtkErrorMacro(<< "RemoveAttribute: Name parameter is expected to be non nullptr.");
    return;
    }
  if (strlen(name) == 0)
    {
    vtkErrorMacro(<< "RemoveAttribute: Name parameter is expected to have at least one character.");
    return;
    }
  this->SetAttribute(name, nullptr);
}

//----------------------------------------------------------------------------
const char* vtkDMMLNode::GetAttribute(const char* name)
{
  if (!name)
    {
    vtkErrorMacro(<< "GetAttribute: Name parameter is expected to be non nullptr.");
    return nullptr;
    }
  if (strlen(name) == 0)
    {
    vtkErrorMacro(<< "GetAttribute: Name parameter is expected to have at least one character.");
    return nullptr;
    }
  AttributesType::const_iterator iter =
    this->Attributes.find(std::string(name));
  if (iter == Attributes.end())
    {
    return nullptr;
    }
  else
    {
    return iter->second.c_str();
    }
}

//----------------------------------------------------------------------------
std::vector< std::string > vtkDMMLNode::GetAttributeNames()
{
  std::vector< std::string > attributeNamesVector;
  for ( AttributesType::iterator iter = this->Attributes.begin(); iter != this->Attributes.end(); ++iter )
    {
    attributeNamesVector.push_back(iter->first);
    }
  return attributeNamesVector;
}

//----------------------------------------------------------------------------
void vtkDMMLNode::GetAttributeNames(vtkStringArray* attributeNames)
{
  if (attributeNames == nullptr)
    {
    vtkErrorMacro("vtkDMMLNode::GetAttributeNames: attributeNames is invalid");
    return;
    }
  attributeNames->Reset();
  for (AttributesType::iterator iter = this->Attributes.begin(); iter != this->Attributes.end(); ++iter)
    {
    attributeNames->InsertNextValue(iter->first);
    }
}

//----------------------------------------------------------------------------
// Description:
// the DMMLCallback is a static function to relay modified events from the
// observed dmml node back into the gui layer for further processing
//
void vtkDMMLNode::DMMLCallback(vtkObject *caller,
                               unsigned long eid,
                               void *clientData,
                               void *callData)
{
  vtkDMMLNode *self = reinterpret_cast<vtkDMMLNode *>(clientData);

  if ( self == nullptr )
    {
    //vtkDebugMacro(self, "In vtkDMMLNode *********DMMLCallback called after delete!");
    return;
    }


  if (self->GetInDMMLCallbackFlag())
    {
    vtkDebugWithObjectMacro(self, "In vtkDMMLNode *********DMMLCallback called recursively?");
    return;
    }

  vtkDebugWithObjectMacro(self, "In vtkDMMLNode DMMLCallback");

  self->SetInDMMLCallbackFlag(1);
  self->ProcessDMMLEvents(caller, eid, callData);
  self->SetInDMMLCallbackFlag(0);
}

//----------------------------------------------------------------------------
void vtkDMMLNode::SetAddToSceneNoModify(int value)
{
   this->AddToScene = value;
}

//----------------------------------------------------------------------------
void vtkDMMLNode::SetID (const char* _arg)
{
  // Mostly copied from vtkSetStringMacro() in vtkSetGet.cxx
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting ID to " << (_arg?_arg:"(null)") );
  if ( this->ID == nullptr && _arg == nullptr) { return;}
  if ( this->ID && _arg && (!strcmp(this->ID,_arg))) { return;}
  char* oldID = this->ID;
  if (_arg)
    {
    size_t n = strlen(_arg) + 1;
    char *cp1 =  new char[n];
    const char *cp2 = (_arg);
    this->ID = cp1;
    do { *cp1++ = *cp2++; } while ( --n );
    }
   else
    {
    this->ID = nullptr;
    }
  this->InvokeEvent(vtkDMMLNode::IDChangedEvent, oldID);
  if (oldID) { delete [] oldID; }
  this->Modified();
}

//----------------------------------------------------------------------------
const char * vtkDMMLNode::URLEncodeString(const char *inString)
{
  if (inString == nullptr)
    {
    return "(null)";
    }
  if (strcmp(inString, "") == 0)
    {
    return "";
    }

  std::string kwInString = std::string(inString);
  // encode %
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "%", "%25");
  // encode space
  vtksys::SystemTools::ReplaceString(kwInString,
                                     " ", "%20");
  // encode single quote
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "'", "%27");
  // encode greater than
  vtksys::SystemTools::ReplaceString(kwInString,
                                     ">", "%3E");
  // encode less than
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "<", "%3C");
  // encode double quote
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "\"", "%22");

  this->DisableModifiedEventOn();
  this->SetTempURLString(kwInString.c_str());
  this->DisableModifiedEventOff();
  return (this->GetTempURLString());
}

//----------------------------------------------------------------------------
const char * vtkDMMLNode::URLDecodeString(const char *inString)
{
  if (inString == nullptr)
    {
    return "(null)";
    }
  if (strcmp(inString, "") == 0)
    {
    return "";
    }
  std::string kwInString = std::string(inString);

  // decode in the opposite order they were encoded in

  // decode double quote
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "%22", "\"");
  // decode less than
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "%3C", "<");
  // decode greater than
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "%3E", ">");
  // decode single quote
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "%27", "'");
  // decode space
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "%20", " ");
  // decode %
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "%25", "%");

  this->DisableModifiedEventOn();
  this->SetTempURLString(kwInString.c_str());
  this->DisableModifiedEventOff();
  return (this->GetTempURLString());
}

//----------------------------------------------------------------------------
std::string vtkDMMLNode::XMLAttributeEncodeString(const std::string& inString)
{
  std::string outString = inString;

  vtksys::SystemTools::ReplaceString(outString, "&", "&amp;");
  vtksys::SystemTools::ReplaceString(outString, "\"", "&quot;");
  vtksys::SystemTools::ReplaceString(outString, "'", "&apos;");
  vtksys::SystemTools::ReplaceString(outString, "<", "&lt;");
  vtksys::SystemTools::ReplaceString(outString, ">", "&gt;");

  // It is valid to have newline character in attribute string, but
  // the XML parser is allowed to replace them by newline characters
  // therefore we encode CR and LF to ensure they are preserved verbatim.
  vtksys::SystemTools::ReplaceString(outString, "\n", "&#10;");
  vtksys::SystemTools::ReplaceString(outString, "\r", "&#13;");

  return outString;
}

//----------------------------------------------------------------------------
std::string vtkDMMLNode::XMLAttributeDecodeString(const std::string& inString)
{
  std::string outString = inString;
  vtksys::SystemTools::ReplaceString(outString, "&quot;", "\"");
  vtksys::SystemTools::ReplaceString(outString, "&apos;", "'");
  vtksys::SystemTools::ReplaceString(outString, "&lt;", "<");
  vtksys::SystemTools::ReplaceString(outString, "&gt;", ">");
  vtksys::SystemTools::ReplaceString(outString, "&amp;", "&");
  return outString;
}

//// Reference API

//-----------------------------------------------------------
void vtkDMMLNode::UpdateReferences()
{
  if (!this->Scene)
    {
    return;
    }
  NodeReferencesType::iterator it;
  for (it = this->NodeReferences.begin(); it != this->NodeReferences.end(); it++)
    {
    for (unsigned int i=0; i<it->second.size();)
      {
      vtkDMMLNodeReference* reference = it->second[i];
      if (reference->GetReferencedNodeID() &&
          std::string(reference->GetReferencedNodeID()) != "" &&
          this->Scene->GetNodeByID(reference->GetReferencedNodeID()) == nullptr)
        {
        this->RemoveNthNodeReferenceID(reference->GetReferenceRole(), i);
        }
      else
        {
        ++i;
        }
      }
    }
}

//-----------------------------------------------------------
void vtkDMMLNode::RemoveInvalidReferences(const std::set<std::string>& validNodeIDs)
{
  // If the referenced node is not in the imported scene then the reference must be
  // removed to make sure it will not be linked to a node in the main scene now
  // or later (as new nodes are added to the main scene).
  // 2. If the reference node is among the existing
  if (!this->Scene)
    {
    return;
    }
  NodeReferencesType::iterator it;
  for (it = this->NodeReferences.begin(); it != this->NodeReferences.end(); it++)
    {
    for (unsigned int i=0; i<it->second.size();)
      {
      vtkDMMLNodeReference* reference = it->second[i];
      char* referencedNodeID = reference->GetReferencedNodeID();
      if (!referencedNodeID || strlen(referencedNodeID) == 0)
        {
        // Reference is already removed, no need to remove it
        ++i;
        continue;
        }
      if (validNodeIDs.find(referencedNodeID) != validNodeIDs.end())
        {
        // Reference to a valid node, keep it
        ++i;
        continue;
        }
      vtkDMMLNode* referencedNode = this->Scene->GetNodeByID(referencedNodeID);
      if (referencedNode && referencedNode->IsSingleton())
        {
        // Reference to a singleton node, keep it
        // (for example, unit nodes are not saved with the scene
        // but expected to reference to whatever unit is in the current scene)
        ++i;
        continue;
        }
      // This reference is not to a valid node, remove it
      this->RemoveNthNodeReferenceID(reference->GetReferenceRole(), i);
      }
    }
}

//----------------------------------------------------------------------------
void vtkDMMLNode::RemoveNodeReferenceIDs(const char* referenceRole)
{
  if (!referenceRole)
    {
    int wasModifying = this->StartModify();
    NodeReferencesType::iterator it;
    for (it=this->NodeReferences.begin(); it!=this->NodeReferences.end(); it++)
      {
      if (it->first.c_str())
        {
        this->RemoveNodeReferenceIDs(it->first.c_str());
        }
      }
    this->EndModify(wasModifying);
    return;
    }

  int wasModifying = this->StartModify();
  while(this->GetNumberOfNodeReferences(referenceRole) > 0)
    {
    this->RemoveNthNodeReferenceID(referenceRole, 0);
    }
  this->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
void vtkDMMLNode::GetNodeReferenceRoles(std::vector<std::string> &roles)
{
  roles.clear();
  for (NodeReferencesType::iterator roleIt = this->NodeReferences.begin(); roleIt != this->NodeReferences.end(); roleIt++)
    {
    roles.push_back(roleIt->first);
    }
}

//----------------------------------------------------------------------------
int vtkDMMLNode::GetNumberOfNodeReferenceRoles()
{
  return this->NodeReferences.size();
}

//----------------------------------------------------------------------------
const char* vtkDMMLNode::GetNthNodeReferenceRole(int n)
{
  if (n < 0 || n >= static_cast<int>(this->NodeReferences.size()))
    {
    vtkErrorMacro("vtkDMMLNode::GetNthNodeReferenceRole failed: n=" << n << " is out of range");
    return nullptr;
    }
  NodeReferencesType::iterator roleIt( this->NodeReferences.begin() );
  std::advance( roleIt, n );
  return roleIt->first.c_str();
}

//----------------------------------------------------------------------------
void vtkDMMLNode::GetNodeReferences(const char* referenceRole, std::vector<vtkDMMLNode*> &nodes)
{
  if (referenceRole)
    {
    this->UpdateNodeReferences(referenceRole);
    NodeReferenceListType &references = this->NodeReferences[std::string(referenceRole)];
    for (unsigned int i=0; i<references.size(); i++)
      {
      nodes.push_back(references[i]->GetReferencedNode());
      }
    }
}

//----------------------------------------------------------------------------
void vtkDMMLNode::GetNodeReferenceIDs(const char* referenceRole,
                                      std::vector<const char*> &referencedNodeIDs)
{
  if (!referenceRole)
    {
    return;
    }

  NodeReferenceListType &references =
    this->NodeReferences[std::string(referenceRole)];
  for (unsigned int i=0; i<references.size(); ++i)
    {
    referencedNodeIDs.push_back(references[i] ? references[i]->GetReferencedNodeID() : nullptr);
    }
}


//----------------------------------------------------------------------------
const char * vtkDMMLNode::GetNthNodeReferenceID(const char* referenceRole, int n)
{
  if (!referenceRole || n < 0)
    {
    return nullptr;
    }

  NodeReferenceListType &references = this->NodeReferences[std::string(referenceRole)];
  if (n >= static_cast<int>(references.size()))
    {
    return nullptr;
    }
  if (!references[n])
    {
    vtkErrorMacro(<< "GetNthNodeReferenceID: Reference " << n << "should NOT be nullptr.");
    return nullptr;
    }
  return references[n]->GetReferencedNodeID();
}

//----------------------------------------------------------------------------
vtkDMMLNode* vtkDMMLNode::GetNthNodeReference(const char* referenceRole, int n)
{

  if (!referenceRole || n < 0 )
    {
    return nullptr;
    }

  NodeReferenceListType &references = this->NodeReferences[std::string(referenceRole)];
  if (n >= static_cast<int>(references.size()))
    {
    return nullptr;
    }

  vtkDMMLNode* node = references[n]->GetReferencedNode();
  // Maybe the node was not yet in the scene when the node ID was set.
  // Check to see if it's now there.
  // Similarly, if the scene is 0, clear the node if not already null.
  if ((!node || node->GetScene() != this->GetScene()) ||
      (node && this->GetScene() == nullptr))
    {
    this->UpdateNthNodeReference(referenceRole, n);
    NodeReferenceListType &references = this->NodeReferences[std::string(referenceRole)];
    node = references[n]->GetReferencedNode();
    }
  return node;
}

//-----------------------------------------------------------
void vtkDMMLNode::UpdateNodeReferences(const char* referenceRole)
{
  if (referenceRole==nullptr)
    {
    // update all roles
    int wasModifying = this->StartModify();
    NodeReferencesType::iterator it;
    for (it=this->NodeReferences.begin(); it!=this->NodeReferences.end(); it++)
      {
      if (it->first.c_str())
        {
        this->UpdateNodeReferences(it->first.c_str());
        }
      }
    this->EndModify(wasModifying);
    return;
    }

  int wasModifying = this->StartModify();
  NodeReferenceListType &references = this->NodeReferences[std::string(referenceRole)];
  for (unsigned int i=0; i<references.size(); i++)
    {
    this->UpdateNthNodeReference(referenceRole, i);
    }
  this->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
void vtkDMMLNode::UpdateNthNodeReference(const char* referenceRole, int n)
{
  if (!referenceRole)
    {
    vtkErrorMacro(<< "UpdateNthNodeReference: Non-null role is expected.");
    return;
    }

  NodeReferenceListType &references = this->NodeReferences[std::string(referenceRole)];

  if (n >= static_cast<int>(references.size()))
    {
    vtkErrorMacro(<< "UpdateNthNodeReference: n is " << n << "."
        << " Value is expected to be smaller than " << references.size());
    return;
    }

  this->SetAndObserveNthNodeReferenceID(references[n]->GetReferenceRole(), n, references[n]->GetReferencedNodeID(), references[n]->GetEvents());
}

//----------------------------------------------------------------------------
vtkDMMLNode* vtkDMMLNode::SetNthNodeReferenceID(const char* referenceRole,
                                                 int n,
                                                 const char* referencedNodeID)
{
  return SetAndObserveNthNodeReferenceID(referenceRole, n, referencedNodeID, nullptr);
}

//----------------------------------------------------------------------------
vtkDMMLNode* vtkDMMLNode::SetAndObserveNthNodeReferenceID(const char* referenceRole,
                                                           int n,
                                                           const char* referencedNodeID,
                                                           vtkIntArray *events)
{
  if (!referenceRole)
    {
    vtkErrorMacro(<< "SetAndObserveNthNodeReferenceID: Non-null role is expected.");
    return nullptr;
    }
  if (n < 0)
    {
    vtkErrorMacro(<< "SetAndObserveNthNodeReferenceID: Non-negative reference index is expected.");
    return nullptr;
    }

  NodeReferenceListType &references = this->NodeReferences[std::string(referenceRole)];

  vtkDMMLNodeReference* oldReference = nullptr;
  vtkDMMLNode* oldReferencedNode = nullptr;
  std::string oldReferencedNodeID;
  NodeReferenceListType::iterator referenceIt=references.end();
  if (n < static_cast<int>(references.size()))
    {
    referenceIt = references.begin()+n;
    oldReference = referenceIt->GetPointer();
    oldReferencedNode = oldReference->GetReferencedNode();
    oldReferencedNodeID = oldReference->GetReferencedNodeID();
    }

  vtkDMMLNode* referencedNode = nullptr;
  if (referencedNodeID && strlen(referencedNodeID)>0)
    {
    // Add/update reference
    if (this->Scene)
      {
      referencedNode = this->Scene->GetNodeByID(referencedNodeID);
      }

    if (referenceIt==references.end())
      {
      vtkNew<vtkDMMLNodeReference> reference;
      reference->SetReferencingNode(this);
      reference->SetReferenceRole(referenceRole);
      references.push_back(reference.GetPointer());
      referenceIt = references.begin() + (references.size()-1);
      }

    // Update node observations
    if (events == nullptr)
      {
      // If no events are specified then use the default events specified for the role.
      if (this->NodeReferenceEvents[referenceRole] && this->NodeReferenceEvents[referenceRole]->GetNumberOfTuples() > 0)
        {
        events = this->NodeReferenceEvents[referenceRole];
        }
      else
        {
        // Node reference events can be specified for a group of reference role, specified as group/item,
        // for example units/length, units/area.
        std::string referenceRoleStr(referenceRole);
        std::size_t groupSeparatorPos = referenceRoleStr.find('/');
        if (groupSeparatorPos != std::string::npos)
          {
          std::string referenceRoleGroup = referenceRoleStr.substr(0, groupSeparatorPos + 1);
          if (this->NodeReferenceEvents[referenceRoleGroup] && this->NodeReferenceEvents[referenceRoleGroup]->GetNumberOfTuples() > 0)
            {
            events = this->NodeReferenceEvents[referenceRoleGroup];
            }
          }
        }
      }
    this->UpdateNodeReferenceEventObserver(oldReferencedNode, referencedNode, events, referenceIt->GetPointer());

    (*referenceIt)->SetReferencedNodeID(referencedNodeID);
    (*referenceIt)->SetReferencedNode(referencedNode);
    (*referenceIt)->SetEvents(events);

    if (oldReferencedNode==nullptr && referencedNode != nullptr)
      {
      this->OnNodeReferenceAdded(referenceIt->GetPointer());
      }
    else if (oldReferencedNode!=nullptr && referencedNode == nullptr)
      {
      this->OnNodeReferenceRemoved(referenceIt->GetPointer());
      }
    else if (oldReferencedNode!=referencedNode)
      {
      this->OnNodeReferenceModified(referenceIt->GetPointer());
      }

    }
  else
    {
    // Delete reference
    if (referenceIt!=references.end())
      {
      this->UpdateNodeReferenceEventObserver(oldReferencedNode, nullptr, nullptr, oldReference);
      vtkSmartPointer<vtkDMMLNodeReference> nodeRefToDelete = referenceIt->GetPointer();
      references.erase(referenceIt);

      if (oldReferencedNode != nullptr)
        {
        this->OnNodeReferenceRemoved(nodeRefToDelete);
        }

      // Already removed the ReferencedNode reference by calling UpdateNodeReferenceEventObserver,
      // so we have to set ReferencedNode to nullptr to avoid removing the reference again in the
      // reference's destructor.
      nodeRefToDelete->SetReferencedNode(nullptr);
      }
    }

  // Update scene node references
  std::string newReferencedNodeID = (referencedNodeID ? referencedNodeID : "");
  if (oldReferencedNodeID!=newReferencedNodeID)
    {
    this->Modified();
    }
  if (this->Scene && oldReferencedNodeID!=newReferencedNodeID)
    {
    if (!oldReferencedNodeID.empty() && !this->HasNodeReferenceID(nullptr, oldReferencedNodeID.c_str()))
      {
      // the old referenced node ID is not used anymore by any node references
      this->Scene->RemoveReferencedNodeID(oldReferencedNodeID.c_str(), this);
      }
    if (!newReferencedNodeID.empty())
      {
      // The new node may or may not be referenced by the scene.
      // Let the scene know that it is not referenced.
      // It is not a problem if the referenced node ID is already added (the scene then ignores the call).
      this->Scene->AddReferencedNodeID(newReferencedNodeID.c_str(), this);
      }
    }

  return referencedNode;
}

//----------------------------------------------------------------------------
void vtkDMMLNode::GetUpdatedReferencedNodeEventList(int& oldReferencedNodeUseCount, int& newReferencedNodeUseCount,
  vtkIntArray* oldConsolidatedEventList, vtkIntArray* newConsolidatedEventList,
  vtkDMMLNode* oldReferencedNode, vtkDMMLNode* newReferencedNode, vtkDMMLNodeReference* referenceToIgnore, vtkIntArray* newEvents)
{
  oldReferencedNodeUseCount = 0; // only computed if referencedNode is not the same as the old one
  newReferencedNodeUseCount = 0;
  oldConsolidatedEventList->SetNumberOfTuples(0);
  newConsolidatedEventList->SetNumberOfTuples(0);
  if (oldReferencedNode == nullptr && newReferencedNode == nullptr)
    {
    return;
    }

  std::vector<int> oldReferencedNodeEvents; // only computed if referencedNode is not the same as the old one
  std::vector<int> newReferencedNodeEvents;
  for (NodeReferencesType::iterator roleIt = this->NodeReferences.begin(); roleIt != this->NodeReferences.end(); roleIt++)
    {
    for (NodeReferenceListType::iterator it = roleIt->second.begin(); it != roleIt->second.end(); it++)
      {
      vtkDMMLNodeReference* reference = it->GetPointer();
      if (!reference->GetReferencedNodeID() || strlen(reference->GetReferencedNodeID()) == 0)
        {
        // deleted reference
        continue;
        }
      if (reference == referenceToIgnore)
        {
        // ignore the reference that we are processing right now
        continue;
        }
      if (newReferencedNode && reference->GetReferencedNode() == newReferencedNode)
        {
        newReferencedNodeUseCount++;
        vtkIntArray* events = reference->GetEvents();
        if (events)
          {
          int eventCount = events->GetNumberOfTuples();
          for (int i=0; i<eventCount; i++)
            {
            newReferencedNodeEvents.push_back(events->GetValue(i));
            }
          }
        }
      if (oldReferencedNode == newReferencedNode)
        {
        // don't compute events and uses of old and new referenced nodes separately
        // if they are the same
        continue;
        }
      if (oldReferencedNode && reference->GetReferencedNode() == oldReferencedNode)
        {
        oldReferencedNodeUseCount++;
        vtkIntArray* events = reference->GetEvents();
        if (events)
          {
          int eventCount = events->GetNumberOfTuples();
          for (int i=0; i<eventCount; i++)
            {
            oldReferencedNodeEvents.push_back(events->GetValue(i));
            }
          }
        }
      } // references
    } // roles

  // Determine the consolidated new event list
  if (newReferencedNode != nullptr)
    {
    // Add new events
    if (newEvents)
      {
      int eventCount = newEvents->GetNumberOfTuples();
      for (int i=0; i<eventCount; i++)
        {
        newReferencedNodeEvents.push_back(newEvents->GetValue(i));
        }
      }
    // Standardize the event list (sort it and remove duplicates)
    std::sort(newReferencedNodeEvents.begin(),newReferencedNodeEvents.end());
    std::vector<int>::iterator lastUnique = std::unique(newReferencedNodeEvents.begin(), newReferencedNodeEvents.end());
    newReferencedNodeEvents.erase(lastUnique, newReferencedNodeEvents.end());
    // Convert consolidated event list to vtkIntArray
    for (std::vector<int>::iterator it=newReferencedNodeEvents.begin(); it!=newReferencedNodeEvents.end(); ++it)
      {
      newConsolidatedEventList->InsertNextValue(*it);
      }
    }

  // Determine the consolidated old event list
  if (oldReferencedNode != nullptr && oldReferencedNode != newReferencedNode)
    {
    // Standardize the event list (sort it and remove duplicates)
    std::sort(oldReferencedNodeEvents.begin(),oldReferencedNodeEvents.end());
    std::vector<int>::iterator lastUnique = std::unique(oldReferencedNodeEvents.begin(), oldReferencedNodeEvents.end());
    oldReferencedNodeEvents.erase(lastUnique, oldReferencedNodeEvents.end());
    for (std::vector<int>::iterator it=oldReferencedNodeEvents.begin(); it!=oldReferencedNodeEvents.end(); ++it)
      {
      oldConsolidatedEventList->InsertNextValue(*it);
      }
    }
}

//----------------------------------------------------------------------------
vtkDMMLNode* vtkDMMLNode::UpdateNodeReferenceEventObserver(vtkDMMLNode *oldReferencedNode, vtkDMMLNode *newReferencedNode, vtkIntArray *newEvents, vtkDMMLNodeReference* referenceToIgnore)
{
  if (oldReferencedNode == nullptr && newReferencedNode == nullptr)
    {
    // both old and new references are nullptr, there is nothing to do
    return nullptr;
    }

  std::string oldReferencedNodeID;
  if (oldReferencedNode != nullptr && oldReferencedNode->GetID() != nullptr)
    {
    oldReferencedNodeID = oldReferencedNode->GetID();
    }

  std::string newReferencedNodeID;
  if (newReferencedNode != nullptr && newReferencedNode->GetID() != nullptr)
    {
    newReferencedNodeID = newReferencedNode->GetID();
    }

  // Get consolidated event counts and references to the old and new referenced node
  int oldReferencedNodeUseCount = 0; // only computed if referencedNode is not the same as the old one
  int newReferencedNodeUseCount = 0;
  vtkNew<vtkIntArray> oldConsolidatedEventList; // only computed if referencedNode is not the same as the old one
  vtkNew<vtkIntArray> newConsolidatedEventList;
  this->GetUpdatedReferencedNodeEventList(oldReferencedNodeUseCount, newReferencedNodeUseCount,
    oldConsolidatedEventList.GetPointer(), newConsolidatedEventList.GetPointer(),
    oldReferencedNode, newReferencedNode, referenceToIgnore, newEvents);

  // Update events
  if (oldReferencedNode==newReferencedNode)
    {
    // Referenced node not changed: only update the events
    // The same events may be already observed by other nodes, so suppress the warning if there are no changes in the event list.
    vtkSetAndObserveDMMLObjectEventsMacroNoWarning(newReferencedNode, newReferencedNode, newConsolidatedEventList.GetPointer()); // update the event list
    }
  else
    {
    // Update events of the old node
    if (oldReferencedNode != nullptr)
      {
      if (oldReferencedNodeUseCount==0)
        {
        // This was the last reference that used the oldReferencedNode node: remove all event observers and unregister
        vtkDMMLNode* oldReferencedNodeCopy = oldReferencedNode; // make a copy of oldReferencedNode (vtkSetAndObserveDMMLObjectMacro would overwrite the value with nullptr)
        vtkSetAndObserveDMMLObjectMacro(oldReferencedNodeCopy, nullptr); // unregister & remove events
        }
      else
        {
        // This reference does not use oldReferencedNode anymore but other references still use it:
        // update the event list and unregister
        vtkSetAndObserveDMMLObjectEventsMacroNoWarning(oldReferencedNode, oldReferencedNode, oldConsolidatedEventList.GetPointer()); // update the event list
        oldReferencedNode->UnRegister(this->DMMLObserverManager); // unregister
        }
      }
    // Update events of the new node
    if (newReferencedNode != nullptr)
      {
      // This reference is now using a new node: register with an updated event list
      vtkDMMLNode* dummyNullReferencedNode = nullptr; // forces registration
      // the same events may be already observed by other nodes, so suppress the warning if there are no changes in the event list
      vtkSetAndObserveDMMLObjectEventsMacroNoWarning(dummyNullReferencedNode, newReferencedNode, newConsolidatedEventList.GetPointer()); // update the event list & register
      }
    }

  return newReferencedNode;
}

//----------------------------------------------------------------------------
bool vtkDMMLNode::HasNodeReferenceID(const char* referenceRole, const char* referencedNodeID)
{
  if (!referencedNodeID)
    {
    return false;
    }

  if (!referenceRole)
    {
    NodeReferencesType::iterator it;
    for (it=this->NodeReferences.begin(); it!=this->NodeReferences.end(); it++)
      {
      if (it->first.c_str())
        {
        if (this->HasNodeReferenceID(it->first.c_str(), referencedNodeID))
          {
          return true;
          }
        }
      }
    return false;
    }

  NodeReferenceListType &references = this->NodeReferences[std::string(referenceRole)];
  NodeReferenceListType::iterator it;
  std::string sID(referencedNodeID);
  for (it=references.begin(); it!=references.end(); it++)
    {
    vtkDMMLNodeReference* reference = *it;
    if (sID == std::string(reference->GetReferencedNodeID()))
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
void vtkDMMLNode::InvalidateNodeReferences()
{
  // Remove the referenced node pointers, but keep the IDs and events
  NodeReferencesType::iterator it;
  for (it = this->NodeReferences.begin(); it != this->NodeReferences.end(); it++)
    {
    NodeReferenceListType::iterator it1;
    for (it1 = it->second.begin(); it1 != it->second.end(); it1++)
      {
      vtkDMMLNodeReference* reference = *it1;
      if (reference->GetReferencedNode())
        {
        vtkDMMLNode *nodePtr = reference->GetReferencedNode(); // vtkSetAndObserveDMMLObjectMacro overwrites the argument, so we need to make a copy
        vtkSetAndObserveDMMLObjectMacro(nodePtr, nullptr);
        reference->SetReferencedNode(nullptr);
        }
      }
    }
}

//----------------------------------------------------------------------------
vtkDMMLNode* vtkDMMLNode::SetNodeReferenceID(const char* referenceRole, const char *referencedNodeID)
{
  return this->SetNthNodeReferenceID(referenceRole, 0, referencedNodeID);
}

//----------------------------------------------------------------------------
vtkDMMLNode* vtkDMMLNode::AddNodeReferenceID(const char* referenceRole, const char *referencedNodeID)
{
  return this->SetNthNodeReferenceID(referenceRole, this->GetNumberOfNodeReferences(referenceRole), referencedNodeID);
}

//----------------------------------------------------------------------------
const char * vtkDMMLNode::GetNodeReferenceID(const char* referenceRole)
{
  return this->GetNthNodeReferenceID(referenceRole, 0);
}

//----------------------------------------------------------------------------
vtkDMMLNode* vtkDMMLNode::GetNodeReference(const char* referenceRole)
{
  return this->GetNthNodeReference(referenceRole, 0);
}

//----------------------------------------------------------------------------
vtkDMMLNode* vtkDMMLNode::SetAndObserveNodeReferenceID(const char* referenceRole, const char *referencedNodeID, vtkIntArray *events)
{
  return this->SetAndObserveNthNodeReferenceID(referenceRole, 0, referencedNodeID, events);
}

//----------------------------------------------------------------------------
vtkDMMLNode* vtkDMMLNode::AddAndObserveNodeReferenceID(const char* referenceRole, const char *referencedNodeID, vtkIntArray *events)
{
  return this->SetAndObserveNthNodeReferenceID(referenceRole, this->GetNumberOfNodeReferences(referenceRole), referencedNodeID, events);
}

//----------------------------------------------------------------------------
void vtkDMMLNode::RemoveNthNodeReferenceID(const char* referenceRole, int n)
{
  this->SetAndObserveNthNodeReferenceID(referenceRole, n, nullptr);
}

//----------------------------------------------------------------------------
int vtkDMMLNode::GetNumberOfNodeReferences(const char* referenceRole)
{
  int n=0;
  if (referenceRole)
    {
    NodeReferenceListType &references = this->NodeReferences[std::string(referenceRole)];
    NodeReferenceListType::iterator it;
    for (it = references.begin(); it != references.end(); it++)
      {
      vtkDMMLNodeReference* reference = *it;
      if (reference->GetReferencedNodeID() && strlen(reference->GetReferencedNodeID()) > 0)
        {
        n++;
        }
      }
    }
  return n;
}


//----------------------------------------------------------------------------
vtkDMMLNode::vtkDMMLNodeReference* vtkDMMLNode::vtkDMMLNodeReference::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkDMMLNodeReference");
  if(ret)
    {
    return static_cast<vtkDMMLNode::vtkDMMLNodeReference*>(ret);
    }

  vtkDMMLNode::vtkDMMLNodeReference* result = new vtkDMMLNode::vtkDMMLNodeReference;
#ifdef VTK_HAS_INITIALIZE_OBJECT_BASE
  result->InitializeObjectBase();
#endif
  return result;
}

//----------------------------------------------------------------------------
vtkDMMLNode::vtkDMMLNodeReference::vtkDMMLNodeReference()
{
  this->ReferencedNodeID = nullptr;
  this->ReferenceRole = nullptr;
}

//----------------------------------------------------------------------------
vtkDMMLNode::vtkDMMLNodeReference::~vtkDMMLNodeReference()
{
  if (this->GetReferencedNode() != nullptr)
    {
    // The referenced node has to be nullptr before getting to this destructor.
    // We cannot properly clean up the referenced node in the destructor, because the referencing node may be already nullptr and
    // we don't know which event observers we would need to remove. Therefore, we just report the error to allow easier debugging.
    const char* referencedNodeId = this->ReferencedNode->GetID() ? this->ReferencedNode->GetID() : "(unknown)";
    const char* referencingNodeId = (this->ReferencingNode.GetPointer() && this->ReferencingNode->GetID()) ? this->ReferencingNode->GetID() : "(unknown)";
    vtkWarningMacro("While deleting a reference object an active node reference is found to node "<<referencedNodeId<<" from node "<<referencingNodeId<<". Remaining references and observations may cause memory leaks.");
    }
  SetReferencedNodeID(nullptr);
  SetReferenceRole(nullptr);
}

//----------------------------------------------------------------------------
vtkIntArray* vtkDMMLNode::vtkDMMLNodeReference::GetEvents() const
{
  return this->Events.GetPointer();
}

//----------------------------------------------------------------------------
void vtkDMMLNode::vtkDMMLNodeReference::SetEvents(vtkIntArray* events)
{
  // The events are stored and used sometime later (when the references are updated).
  // Make a copy of the events to make sure the current values are used (and not the values that are current at the time of node reference update).
  if (events==this->Events)
    {
    // no change
    return;
    }
  if (events)
    {
    if (!this->Events.GetPointer())
      {
      this->Events = vtkSmartPointer<vtkIntArray>::New();
      }
    this->Events->DeepCopy(events);
    }
  else
    {
    this->Events=nullptr;
    }
  Modified();
}

//----------------------------------------------------------------------------
vtkDMMLNode* vtkDMMLNode::vtkDMMLNodeReference::GetReferencingNode() const
{
  return this->ReferencingNode.GetPointer();
}

//----------------------------------------------------------------------------
void vtkDMMLNode::vtkDMMLNodeReference::SetReferencingNode(vtkDMMLNode* node)
{
  if (this->ReferencingNode.GetPointer() == node)
  {
    return;
  }
  this->ReferencingNode = node;
  this->Modified();
}

//----------------------------------------------------------------------------
vtkDMMLNode* vtkDMMLNode::vtkDMMLNodeReference::GetReferencedNode() const
{
  return this->ReferencedNode.GetPointer();
}

//----------------------------------------------------------------------------
void vtkDMMLNode::vtkDMMLNodeReference::SetReferencedNode(vtkDMMLNode* node)
{
  if (this->ReferencedNode.GetPointer() == node)
  {
    return;
  }
  this->ReferencedNode = node;
  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkDMMLNode::HasCopyContent() const
{
  // This may be overridden in derived classes by placing vtkDMMLNodeHasCopyContentMacro
  // in the header.
  return strcmp("vtkDMMLNode", this->GetClassNameInternal()) == 0;
}
