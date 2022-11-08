/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Subject Hierarchy includes
#include "vtkDMMLSubjectHierarchyLegacyNode.h"
#include "vtkDMMLSubjectHierarchyConstants.h"

// DMML includes
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
const std::string vtkDMMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_ITEM_SEPARATOR = std::string(":");
const std::string vtkDMMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_NAME_VALUE_SEPARATOR = std::string("; ");

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLSubjectHierarchyLegacyNode);

//----------------------------------------------------------------------------
vtkDMMLSubjectHierarchyLegacyNode::vtkDMMLSubjectHierarchyLegacyNode()
{
  this->SetLevel("Other");

  this->UIDs.clear();
}

//----------------------------------------------------------------------------
vtkDMMLSubjectHierarchyLegacyNode::~vtkDMMLSubjectHierarchyLegacyNode()
{
  this->UIDs.clear();

  this->SetLevel(nullptr);
  this->SetOwnerPluginName(nullptr);
}

//----------------------------------------------------------------------------
void vtkDMMLSubjectHierarchyLegacyNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << " Level=\""
    << (this->Level ? this->Level : "NULL" ) << "\n";

  os << indent << " OwnerPluginName=\""
    << (this->OwnerPluginName ? this->OwnerPluginName : "NULL" ) << "\n";

  os << indent << " UIDs=\"";
  for (std::map<std::string, std::string>::iterator uidsIt = this->UIDs.begin(); uidsIt != this->UIDs.end(); ++uidsIt)
    {
    os << uidsIt->first << vtkDMMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_NAME_VALUE_SEPARATOR.c_str()
      << uidsIt->second << vtkDMMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_ITEM_SEPARATOR.c_str();
    }
  os << "\"";
}

//----------------------------------------------------------------------------
const char* vtkDMMLSubjectHierarchyLegacyNode::GetNodeTagName()
{
  return "SubjectHierarchyLegacy";
}

//----------------------------------------------------------------------------
void vtkDMMLSubjectHierarchyLegacyNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "Level"))
      {
      this->SetLevel(attValue);
      }
    else if (!strcmp(attName, "OwnerPluginName"))
      {
      this->SetOwnerPluginName(attValue);
      }
    else if (!strcmp(attName, "UIDs"))
      {
      std::stringstream ss;
      ss << attValue;
      std::string valueStr = ss.str();

      this->UIDs.clear();
      size_t itemSeparatorPosition = valueStr.find(vtkDMMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_ITEM_SEPARATOR);
      while (itemSeparatorPosition != std::string::npos)
        {
        std::string itemStr = valueStr.substr(0, itemSeparatorPosition);
        size_t nameValueSeparatorPosition = itemStr.find(vtkDMMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_NAME_VALUE_SEPARATOR);

        std::string name = itemStr.substr(0, nameValueSeparatorPosition);
        std::string value = itemStr.substr(nameValueSeparatorPosition + vtkDMMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_NAME_VALUE_SEPARATOR.size());
        this->UIDs[name] = value;

        valueStr = valueStr.substr(itemSeparatorPosition + vtkDMMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_ITEM_SEPARATOR.size());
        itemSeparatorPosition = valueStr.find(vtkDMMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_ITEM_SEPARATOR);
        }
      if (! valueStr.empty() )
        {
        std::string itemStr = valueStr.substr(0, itemSeparatorPosition);
        size_t tagLevelSeparatorPosition = itemStr.find(vtkDMMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_NAME_VALUE_SEPARATOR);

        std::string name = itemStr.substr(0, tagLevelSeparatorPosition);
        std::string value = itemStr.substr(tagLevelSeparatorPosition + vtkDMMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_NAME_VALUE_SEPARATOR.size());
        this->UIDs[name] = value;
        }
      }
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLSubjectHierarchyLegacyNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);

  of << " Level=\"" << (this->Level ? this->Level : "" ) << "\"";

  of << " OwnerPluginName=\""
    << (this->OwnerPluginName ? this->OwnerPluginName : "" ) << "\"";

  of << " UIDs=\"";
  for (std::map<std::string, std::string>::iterator uidsIt = this->UIDs.begin(); uidsIt != this->UIDs.end(); ++uidsIt)
    {
    of << vtkDMMLNode::XMLAttributeEncodeString(uidsIt->first)
       << vtkDMMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_NAME_VALUE_SEPARATOR
       << vtkDMMLNode::XMLAttributeEncodeString(uidsIt->second)
       << vtkDMMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_ITEM_SEPARATOR;
    }
  of << "\"";
}

//----------------------------------------------------------------------------
void vtkDMMLSubjectHierarchyLegacyNode::Copy(vtkDMMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  vtkDMMLSubjectHierarchyLegacyNode* otherNode = vtkDMMLSubjectHierarchyLegacyNode::SafeDownCast(anode);
  if (otherNode)
    {
    this->SetLevel(otherNode->Level);
    this->SetOwnerPluginName(otherNode->OwnerPluginName);

    this->UIDs = otherNode->UIDs;
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
std::string vtkDMMLSubjectHierarchyLegacyNode::GetUID(std::string uidName)
{
  // Use the find function to prevent adding an empty UID to the list
  if (this->UIDs.find(uidName) == this->UIDs.end())
    {
    return std::string();
    }
  return this->UIDs[uidName];
}

//----------------------------------------------------------------------------
std::map<std::string, std::string> vtkDMMLSubjectHierarchyLegacyNode::GetUIDs()const
{
  return this->UIDs;
}

//---------------------------------------------------------------------------
vtkDMMLSubjectHierarchyLegacyNode* vtkDMMLSubjectHierarchyLegacyNode::GetSubjectHierarchyLegacyNodeByUID(
  vtkDMMLScene* scene, const char* uidName, const char* uidValue)
{
  if (!scene || !uidName || !uidValue)
    {
    std::cerr << "vtkDMMLSubjectHierarchyLegacyNode::GetSubjectHierarchyNodeByUID: Invalid scene or searched UID!" << std::endl;
    return nullptr;
    }

  std::vector<vtkDMMLNode*> subjectHierarchyNodes;
  unsigned int numberOfNodes = scene->GetNodesByClass("vtkDMMLSubjectHierarchyLegacyNode", subjectHierarchyNodes);
  for (unsigned int shNodeIndex=0; shNodeIndex<numberOfNodes; shNodeIndex++)
    {
    vtkDMMLSubjectHierarchyLegacyNode* node = vtkDMMLSubjectHierarchyLegacyNode::SafeDownCast(subjectHierarchyNodes[shNodeIndex]);
    if (node)
      {
      std::string nodeUidValueStr = node->GetUID(uidName);
      const char* nodeUidValue = nodeUidValueStr.c_str();
      if (nodeUidValue && !strcmp(uidValue, nodeUidValue))
        {
        return node;
        }
      }
    }

  return nullptr;
}

//---------------------------------------------------------------------------
vtkDMMLSubjectHierarchyLegacyNode* vtkDMMLSubjectHierarchyLegacyNode::GetSubjectHierarchyLegacyNodeByUIDList(
  vtkDMMLScene* scene, const char* uidName, const char* uidValue)
{
  if (!scene || !uidName || !uidValue)
    {
    std::cerr << "vtkDMMLSubjectHierarchyLegacyNode::GetSubjectHierarchyNodeByUID: Invalid scene or searched UID!" << std::endl;
    return nullptr;
    }

  std::vector<vtkDMMLNode*> subjectHierarchyNodes;
  unsigned int numberOfNodes = scene->GetNodesByClass("vtkDMMLSubjectHierarchyLegacyNode", subjectHierarchyNodes);
  for (unsigned int shNodeIndex=0; shNodeIndex<numberOfNodes; shNodeIndex++)
    {
    vtkDMMLSubjectHierarchyLegacyNode* node = vtkDMMLSubjectHierarchyLegacyNode::SafeDownCast(subjectHierarchyNodes[shNodeIndex]);
    if (node)
      {
      std::string nodeUidValueStr = node->GetUID(uidName);
      if (nodeUidValueStr.find(uidValue) != std::string::npos)
        {
        return node;
        }
      }
    }

  return nullptr;
}

//---------------------------------------------------------------------------
void vtkDMMLSubjectHierarchyLegacyNode::DeserializeUIDList(std::string uidListString, std::vector<std::string>& deserializedUIDList)
{
  deserializedUIDList.clear();
  char separatorCharacter = ' ';
  size_t separatorPosition = uidListString.find( separatorCharacter );
  while (separatorPosition != std::string::npos)
    {
    std::string uid = uidListString.substr(0, separatorPosition);
    deserializedUIDList.push_back(uid);
    uidListString = uidListString.substr( separatorPosition+1 );
    separatorPosition = uidListString.find( separatorCharacter );
    }
  // Add last UID in case there was no space at the end (which is default behavior)
  if (!uidListString.empty() && uidListString.find(separatorCharacter) == std::string::npos)
    {
    deserializedUIDList.push_back(uidListString);
    }
}

//---------------------------------------------------------------------------
std::vector<vtkDMMLSubjectHierarchyLegacyNode*> vtkDMMLSubjectHierarchyLegacyNode::GetSubjectHierarchyNodesReferencedByDICOM()
{
  std::vector<vtkDMMLSubjectHierarchyLegacyNode*> referencedNodes;

  vtkDMMLScene* scene = this->GetScene();
  if (!scene)
    {
    vtkErrorMacro("GetSubjectHierarchyNodesReferencedByDICOM: Invalid DMML scene!");
    return referencedNodes;
    }

  // Get referenced SOP instance UIDs
  const char* referencedInstanceUIDsAttribute = this->GetAttribute(
    vtkDMMLSubjectHierarchyConstants::GetDICOMReferencedInstanceUIDsAttributeName().c_str() );
  if (!referencedInstanceUIDsAttribute)
    {
    return referencedNodes;
    }

  // De-serialize SOP instance UID list
  std::vector<std::string> referencedSopInstanceUids;
  this->DeserializeUIDList(referencedInstanceUIDsAttribute, referencedSopInstanceUids);

  // Find subject hierarchy nodes by SOP instance UIDs
  std::vector<std::string>::iterator uidIt;
  for (uidIt = referencedSopInstanceUids.begin(); uidIt != referencedSopInstanceUids.end(); ++uidIt)
    {
    // Find first referenced node in whole scene
    if (referencedNodes.empty())
      {
      vtkDMMLSubjectHierarchyLegacyNode* node = vtkDMMLSubjectHierarchyLegacyNode::GetSubjectHierarchyLegacyNodeByUIDList(
        scene, vtkDMMLSubjectHierarchyConstants::GetDICOMInstanceUIDName(), (*uidIt).c_str() );
      if (node)
        {
        referencedNodes.push_back(node);
        }
      }
    else
      {
      // If we found a referenced node, check the other instances in those nodes first to save time
      bool foundUidInFoundReferencedNodes = false;
      std::vector<vtkDMMLSubjectHierarchyLegacyNode*>::iterator nodeIt;
      for (nodeIt = referencedNodes.begin(); nodeIt != referencedNodes.end(); ++nodeIt)
        {
        // Get instance UIDs of the referenced node
        std::string uids = (*nodeIt)->GetUID( vtkDMMLSubjectHierarchyConstants::GetDICOMInstanceUIDName() );
        if (uids.find(*uidIt) != std::string::npos)
          {
          // If we found the UID in the already found referenced nodes, then we don't need to do anything
          foundUidInFoundReferencedNodes = true;
          break;
          }
        }
      // If the referenced SOP instance UID is not contained in the already found referenced nodes, then we look in the scene
      if (!foundUidInFoundReferencedNodes)
        {
        vtkDMMLSubjectHierarchyLegacyNode* node = vtkDMMLSubjectHierarchyLegacyNode::GetSubjectHierarchyLegacyNodeByUIDList(
          scene, vtkDMMLSubjectHierarchyConstants::GetDICOMInstanceUIDName(), (*uidIt).c_str() );
        if (node)
          {
          referencedNodes.push_back(node);
          }
        }
      }
    }

  return referencedNodes;
}
