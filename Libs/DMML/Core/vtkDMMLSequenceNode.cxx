/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// DMMLSequence includes
#include "vtkDMMLLinearTransformSequenceStorageNode.h"
#include "vtkDMMLSequenceNode.h"
#include "vtkDMMLSequenceStorageNode.h"
#include "vtkDMMLStorableNode.h"

// DMML includes
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkNew.h>
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkTimerLog.h>

// STD includes
#include <sstream>

#define SAFE_CHAR_POINTER(unsafeString) ( unsafeString==nullptr?"":unsafeString )

// This macro sets a member variable and sets both this node and the storage node as modified.
// This macro can be used for properties that are stored in both the scene and in the stored file.
#define vtkCxxSetVariableInDataAndStorageNodeMacro(name, type) \
  void vtkDMMLSequenceNode::Set##name(type arg) \
  { \
    if (arg == this->name) { return; } \
    this->name = arg; \
    this->StorableModifiedTime.Modified(); \
    this->Modified(); \
  }

//------------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLSequenceNode);
vtkCxxSetVariableInDataAndStorageNodeMacro(IndexName, const std::string&);
vtkCxxSetVariableInDataAndStorageNodeMacro(IndexUnit, const std::string&);
vtkCxxSetVariableInDataAndStorageNodeMacro(IndexType, int);
vtkCxxSetVariableInDataAndStorageNodeMacro(NumericIndexValueTolerance, double);

//----------------------------------------------------------------------------
vtkDMMLSequenceNode::vtkDMMLSequenceNode()
{
  this->SetIndexName("time");
  this->SetIndexUnit("s");
  this->HideFromEditorsOff();
  // sequence scene cannot be created here because vtkDMMLScene instantiates this node
  // in its constructor, which would lead to infinite loop
  this->SequenceScene = nullptr;
}

//----------------------------------------------------------------------------
vtkDMMLSequenceNode::~vtkDMMLSequenceNode()
{
  if (this->SequenceScene)
    {
    this->SequenceScene->Delete();
    this->SequenceScene = nullptr;
    }
}

//----------------------------------------------------------------------------
void vtkDMMLSequenceNode::RemoveAllDataNodes()
{
  this->IndexEntries.clear();
  if (!this->SequenceScene)
    {
    return;
    }
  this->SequenceScene->Delete();
  this->SequenceScene = nullptr;
  this->Modified();
  this->StorableModifiedTime.Modified();
}

//----------------------------------------------------------------------------
void vtkDMMLSequenceNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all DMML node attributes into output stream
  vtkIndent indent(nIndent);

  of << indent << " indexName=\"" << this->IndexName << "\"";
  of << indent << " indexUnit=\"" << this->IndexUnit << "\"";

  std::string indexTypeString=GetIndexTypeAsString();
  of << indent << " indexType=\"" << indexTypeString << "\"";

  of << indent << " numericIndexValueTolerance=\"" << this->NumericIndexValueTolerance << "\"";

  of << indent << " indexValues=\"";
  for(std::deque< IndexEntryType >::iterator indexIt=this->IndexEntries.begin(); indexIt!=this->IndexEntries.end(); ++indexIt)
    {
    if (indexIt!=this->IndexEntries.begin())
      {
      // not the first index, add a separator before adding values
      of << ";";
      }
    if (indexIt->DataNode==nullptr)
      {
      // If we have a data node ID then store that, it is the most we know about the node that should be there
      if (!indexIt->DataNodeID.empty())
        {
        // this is normal when sequence node is in scene view
        of << indexIt->DataNodeID << ":" << indexIt->IndexValue;
        }
      else
        {
        vtkErrorMacro("Error while writing node "<<(this->GetID()?this->GetID():"(unknown)")
          << " to XML: data node is invalid at index value "<<indexIt->IndexValue);
        }
      }
    else
      {
      of << indexIt->DataNode->GetID() << ":" << indexIt->IndexValue;
      }
    }
  of << "\"";
}

//----------------------------------------------------------------------------
void vtkDMMLSequenceNode::ReadXMLAttributes(const char** atts)
{
  vtkDMMLNode::ReadXMLAttributes(atts);

  // Read all DMML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "indexName"))
      {
      this->SetIndexName(attValue);
      }
    else if (!strcmp(attName, "indexUnit"))
      {
      this->SetIndexUnit(attValue);
      }
    else if (!strcmp(attName, "indexType"))
      {
      int indexType=GetIndexTypeFromString(attValue);
      if (indexType<0 || indexType>=vtkDMMLSequenceNode::NumberOfIndexTypes)
        {
        vtkErrorMacro("Invalid index type: "<<(attValue?attValue:"(empty). Assuming TextIndex."));
        indexType=vtkDMMLSequenceNode::TextIndex;
        }
      SetIndexType(indexType);
      }
    else if (!strcmp(attName, "numericIndexValueTolerance"))
      {
      std::stringstream ss;
      ss << attValue;
      double numericIndexValueTolerance = 0.001;
      ss >> numericIndexValueTolerance;
      this->SetNumericIndexValueTolerance(numericIndexValueTolerance);
      }
    else if (!strcmp(attName, "indexValues"))
      {
      ReadIndexValues(attValue);
      }
    }
}

//----------------------------------------------------------------------------
void vtkDMMLSequenceNode::ReadIndexValues(const std::string& indexText)
{
  bool modified = false;

  if (!this->IndexEntries.empty())
    {
    this->IndexEntries.clear();
    modified = true;
    }

  std::stringstream ss(indexText);
  std::string nodeId_indexValue;
  while (std::getline(ss, nodeId_indexValue, ';'))
    {
    std::size_t indexValueSeparatorPos = nodeId_indexValue.find_first_of(':');
    if (indexValueSeparatorPos>0 && indexValueSeparatorPos != std::string::npos)
      {
      std::string nodeId = nodeId_indexValue.substr(0, indexValueSeparatorPos);
      std::string indexValue = nodeId_indexValue.substr(indexValueSeparatorPos+1, nodeId_indexValue.size()-indexValueSeparatorPos-1);

      IndexEntryType indexEntry;
      indexEntry.IndexValue=indexValue;
      // The nodes are not read yet, so we can only store the node ID and get the pointer to the node later (in UpdateScene())
      indexEntry.DataNodeID=nodeId;
      indexEntry.DataNode=nullptr;
      this->IndexEntries.push_back(indexEntry);
      modified = true;
      }
    }

  if (modified)
    {
    this->Modified();
    }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkDMMLSequenceNode::Copy(vtkDMMLNode *anode)
{
  int wasModified = this->StartModify();
  Superclass::Copy(anode);

  vtkDMMLSequenceNode *snode = (vtkDMMLSequenceNode *) anode;
  if (!snode)
    {
    vtkErrorMacro("vtkDMMLSequenceNode::Copy failed: invalid input node");
    return;
    }

  this->SetIndexName(snode->GetIndexName());
  this->SetIndexUnit(snode->GetIndexUnit());
  this->SetIndexType(snode->GetIndexType());
  this->SetNumericIndexValueTolerance(snode->GetNumericIndexValueTolerance());

  // Clear nodes: RemoveAllNodes is not a public method, so it's simpler to just delete and recreate the scene
  if (this->SequenceScene)
    {
    this->SequenceScene->Delete();
    }
  this->SequenceScene=vtkDMMLScene::New();

  // Get data node ID in the target scene from the data node ID in the source scene
  std::map< std::string, std::string > sourceToTargetDataNodeID;

  if (snode->SequenceScene)
    {
    for (int n = 0; n < snode->SequenceScene->GetNodes()->GetNumberOfItems(); n++)
      {
      vtkDMMLNode* node = (vtkDMMLNode*)snode->SequenceScene->GetNodes()->GetItemAsObject(n);
      if (node == nullptr)
        {
        vtkErrorMacro("Invalid node in vtkDMMLSequenceNode");
        continue;
        }
      vtkDMMLNode* targetDataNode = this->DeepCopyNodeToScene(node, this->SequenceScene);
      sourceToTargetDataNodeID[node->GetID()] = targetDataNode->GetID();
      }
    }

  this->IndexEntries.clear();
  for(std::deque< IndexEntryType >::iterator sourceIndexIt=snode->IndexEntries.begin(); sourceIndexIt!=snode->IndexEntries.end(); ++sourceIndexIt)
    {
    IndexEntryType seqItem;
    seqItem.IndexValue=sourceIndexIt->IndexValue;
    seqItem.DataNode = nullptr;
    if (sourceIndexIt->DataNode!=nullptr)
      {
      std::string targetDataNodeID = sourceToTargetDataNodeID[sourceIndexIt->DataNode->GetID()];
      seqItem.DataNode = this->SequenceScene->GetNodeByID(targetDataNodeID);
      seqItem.DataNodeID.clear();
      }
    if (seqItem.DataNode==nullptr)
      {
      // data node was not found, at least copy its ID
      std::string targetDataNodeID = sourceToTargetDataNodeID[sourceIndexIt->DataNodeID];
      seqItem.DataNodeID = targetDataNodeID;
      if (seqItem.DataNodeID.empty())
        {
        vtkWarningMacro("vtkDMMLSequenceNode::Copy: node was not found at index value "<<seqItem.IndexValue);
        }
      }
    this->IndexEntries.push_back(seqItem);
    }
  this->Modified();
  this->StorableModifiedTime.Modified();

  this->EndModify(wasModified);
}

//----------------------------------------------------------------------------
void vtkDMMLSequenceNode::CopySequenceIndex(vtkDMMLNode *anode)
{
  int wasModified = this->StartModify();
  vtkDMMLSequenceNode *snode = (vtkDMMLSequenceNode *)anode;
  this->SetIndexName(snode->GetIndexName());
  this->SetIndexUnit(snode->GetIndexUnit());
  this->SetIndexType(snode->GetIndexType());
  this->SetNumericIndexValueTolerance(snode->GetNumericIndexValueTolerance());
  if (this->IndexEntries.size() > 0 || snode->IndexEntries.size() > 0)
    {
    this->IndexEntries.clear();
    for (std::deque< IndexEntryType >::iterator sourceIndexIt = snode->IndexEntries.begin(); sourceIndexIt != snode->IndexEntries.end(); ++sourceIndexIt)
      {
      IndexEntryType seqItem;
      seqItem.IndexValue = sourceIndexIt->IndexValue;
      if (sourceIndexIt->DataNode != nullptr)
        {
        seqItem.DataNodeID = sourceIndexIt->DataNode->GetID();
        }
      else
        {
        seqItem.DataNodeID = sourceIndexIt->DataNodeID;
        }
      seqItem.DataNode = nullptr;
      this->IndexEntries.push_back(seqItem);
      }
    this->Modified();
    }
  this->EndModify(wasModified);
}

//----------------------------------------------------------------------------
void vtkDMMLSequenceNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDMMLNode::PrintSelf(os,indent);
  os << indent << "indexName: " << this->IndexName + "\n";
  os << indent << "indexUnit: " << this->IndexUnit + "\n";

  std::string indexTypeString = GetIndexTypeAsString();
  os << indent << "indexType: " << indexTypeString << "\n";

  os << indent << "numericIndexValueTolerance: " << this->NumericIndexValueTolerance << "\n";

  os << indent << "indexValues: ";
  if (this->IndexEntries.empty())
    {
    os << "(none)";
    }
  else
    {
    os << this->IndexEntries[0].IndexValue;
    if (this->IndexEntries.size() > 1)
      {
      os << " ... " << this->IndexEntries[this->IndexEntries.size()-1].IndexValue;
      os << " (" << this->IndexEntries.size() << " items)";
      }
    }
  os << "\n";
}

//----------------------------------------------------------------------------
bool vtkDMMLSequenceNode::UpdateDataNodeAtValue(vtkDMMLNode* node, const std::string& indexValue, bool shallowCopy /* = false */)
{
  if (node==nullptr)
    {
    vtkErrorMacro("vtkDMMLSequenceNode::UpdateDataNodeAtValue failed, invalid node");
    return false;
    }
  vtkDMMLNode* nodeToBeUpdated = this->GetDataNodeAtValue(indexValue);
  if (!nodeToBeUpdated)
    {
    vtkDebugMacro("vtkDMMLSequenceNode::UpdateDataNodeAtValue failed, indexValue not found");
    return false;
    }
  nodeToBeUpdated->CopyContent(node, !shallowCopy);
  this->Modified();
  this->StorableModifiedTime.Modified();
  return true;
}

//----------------------------------------------------------------------------
int vtkDMMLSequenceNode::GetInsertPosition(const std::string& indexValue)
{
  int insertPosition = this->IndexEntries.size();
  if (this->IndexType == vtkDMMLSequenceNode::NumericIndex && !this->IndexEntries.empty())
    {
    int itemNumber = this->GetItemNumberFromIndexValue(indexValue, false);
    double numericIndexValue = atof(indexValue.c_str());
    double foundNumericIndexValue = atof(this->IndexEntries[itemNumber].IndexValue.c_str());
    if (numericIndexValue < foundNumericIndexValue) // Deals with case of index value being smaller than any in the sequence and numeric tolerances
      {
      insertPosition = itemNumber;
      }
    else
      {
      insertPosition = itemNumber + 1;
      }
    }
  return insertPosition;
}

//----------------------------------------------------------------------------
vtkDMMLNode* vtkDMMLSequenceNode::SetDataNodeAtValue(vtkDMMLNode* node, const std::string& indexValue)
{
  if (node == nullptr)
    {
    vtkErrorMacro("vtkDMMLSequenceNode::SetDataNodeAtValue failed, invalid node");
    return nullptr;
    }
  // Make sure the sequence scene is created
  this->GetSequenceScene();
  // Add a copy of the node to the sequence's scene
  vtkDMMLNode* newNode = this->DeepCopyNodeToScene(node, this->SequenceScene);
  int seqItemIndex = this->GetItemNumberFromIndexValue(indexValue);
  if (seqItemIndex<0)
    {
    // The sequence item doesn't exist yet
    seqItemIndex = GetInsertPosition(indexValue);
    // Create new item
    IndexEntryType seqItem;
    seqItem.IndexValue = indexValue;
    this->IndexEntries.insert(this->IndexEntries.begin() + seqItemIndex, seqItem);
    }
  this->IndexEntries[seqItemIndex].DataNode = newNode;
  this->IndexEntries[seqItemIndex].DataNodeID.clear();
  this->Modified();
  this->StorableModifiedTime.Modified();
  return newNode;
}

//----------------------------------------------------------------------------
void vtkDMMLSequenceNode::RemoveDataNodeAtValue(const std::string& indexValue)
{
  int seqItemIndex = GetItemNumberFromIndexValue(indexValue);
  if (seqItemIndex<0)
    {
    vtkWarningMacro("vtkDMMLSequenceNode::RemoveDataNodeAtValue: node was not found at index value "<<indexValue);
    return;
    }
  if (!this->SequenceScene)
    {
    vtkWarningMacro("vtkDMMLSequenceNode::RemoveDataNodeAtValue: internal scene is already empty");
    return;
    }
  // TODO: remove associated nodes as well (such as storage node)?
  this->SequenceScene->RemoveNode(this->IndexEntries[seqItemIndex].DataNode);
  this->IndexEntries.erase(this->IndexEntries.begin()+seqItemIndex);
  this->Modified();
  this->StorableModifiedTime.Modified();
}

//---------------------------------------------------------------------------
int vtkDMMLSequenceNode::GetItemNumberFromIndexValue(const std::string& indexValue, bool exactMatchRequired /* =true */)
{
  int numberOfSeqItems=this->IndexEntries.size();
  if (numberOfSeqItems == 0)
    {
    return -1;
    }

  // Binary search will be faster for numeric index
  if (this->IndexType == NumericIndex)
    {
    int lowerBound = 0;
    int upperBound = numberOfSeqItems-1;

    // Deal with index values not within the range of index values in the Sequence
    double numericIndexValue = atof(indexValue.c_str());
    double lowerNumericIndexValue = atof(this->IndexEntries[lowerBound].IndexValue.c_str());
    double upperNumericIndexValue = atof(this->IndexEntries[upperBound].IndexValue.c_str());
    if (numericIndexValue <= lowerNumericIndexValue + this->NumericIndexValueTolerance)
      {
      if (numericIndexValue < lowerNumericIndexValue - this->NumericIndexValueTolerance && exactMatchRequired)
        {
        return -1;
        }
      else
        {
        return lowerBound;
        }
      }
    if (numericIndexValue >= upperNumericIndexValue - this->NumericIndexValueTolerance)
      {
      if (numericIndexValue > upperNumericIndexValue + this->NumericIndexValueTolerance && exactMatchRequired)
        {
        return -1;
        }
      else
        {
        return upperBound;
        }
      }

    while (upperBound - lowerBound > 1)
      {
      // Note that if middle is equal to either lowerBound or upperBound then upperBound - lowerBound <= 1
      int middle = int((lowerBound + upperBound)/2);
      double middleNumericIndexValue = atof(this->IndexEntries[middle].IndexValue.c_str());
      if (fabs(numericIndexValue - middleNumericIndexValue) <= this->NumericIndexValueTolerance)
        {
        return middle;
        }
      if (numericIndexValue > middleNumericIndexValue)
        {
        lowerBound = middle;
        }
      if (numericIndexValue < middleNumericIndexValue)
        {
        upperBound = middle;
        }
      }
    if (!exactMatchRequired)
      {
      return lowerBound;
      }
    }

  // Need linear search for non-numeric index
  for (int i=0; i<numberOfSeqItems; i++)
    {
    if (this->IndexEntries[i].IndexValue.compare(indexValue)==0)
      {
      return i;
      }
    }

  return -1;
}

//---------------------------------------------------------------------------
vtkDMMLNode* vtkDMMLSequenceNode::GetDataNodeAtValue(const std::string& indexValue, bool exactMatchRequired /* =true */)
{
  if (!this->SequenceScene)
    {
    // no data nodes are stored
    return nullptr;
    }
  int seqItemIndex = this->GetItemNumberFromIndexValue(indexValue, exactMatchRequired);
  if (seqItemIndex < 0)
    {
    // not found
    return nullptr;
    }
  return this->IndexEntries[seqItemIndex].DataNode;
}

//---------------------------------------------------------------------------
std::string vtkDMMLSequenceNode::GetNthIndexValue(int seqItemIndex)
{
  if (seqItemIndex<0 || seqItemIndex>=static_cast<int>(this->IndexEntries.size()))
    {
    vtkErrorMacro("vtkDMMLSequenceNode::GetNthIndexValue failed, invalid seqItemIndex value: "<<seqItemIndex);
    return "";
    }
  return this->IndexEntries[seqItemIndex].IndexValue;
}

//-----------------------------------------------------------------------------
int vtkDMMLSequenceNode::GetNumberOfDataNodes()
{
  return this->IndexEntries.size();
}

//-----------------------------------------------------------------------------
bool vtkDMMLSequenceNode::UpdateIndexValue(const std::string& oldIndexValue, const std::string& newIndexValue)
{
  if (oldIndexValue == newIndexValue)
    {
    // no change
    return true;
    }
  int oldSeqItemIndex = GetItemNumberFromIndexValue(oldIndexValue);
  if (oldSeqItemIndex<0)
    {
    vtkErrorMacro("vtkDMMLSequenceNode::UpdateIndexValue failed, no data node found with index value "<<oldIndexValue);
    return false;
    }
  if (this->GetItemNumberFromIndexValue(newIndexValue) >= 0)
    {
    vtkErrorMacro("vtkDMMLSequenceNode::UpdateIndexValue failed, data node is already defined at index value " << newIndexValue);
    return false;
    }
  // Update the index value
  this->IndexEntries[oldSeqItemIndex].IndexValue = newIndexValue;
  if (this->IndexType == vtkDMMLSequenceNode::NumericIndex)
    {
    IndexEntryType movingEntry = this->IndexEntries[oldSeqItemIndex];
    // Remove from current position
    this->IndexEntries.erase(this->IndexEntries.begin() + oldSeqItemIndex);
    // Insert into new position
    int insertPosition = this->GetInsertPosition(newIndexValue);
    this->IndexEntries.insert(this->IndexEntries.begin() + insertPosition, movingEntry);
    }
  this->Modified();
  this->StorableModifiedTime.Modified();
  return true;
}

//-----------------------------------------------------------------------------
std::string vtkDMMLSequenceNode::GetDataNodeClassName()
{
  if (this->IndexEntries.empty())
    {
    return "";
    }
  // All the nodes should be of the same class, so just get the class from the first one
  vtkDMMLNode* node=this->IndexEntries[0].DataNode;
  if (node==nullptr)
    {
    vtkErrorMacro("vtkDMMLSequenceNode::GetDataNodeClassName node is invalid");
    return "";
    }
  const char* className=node->GetClassName();
  return SAFE_CHAR_POINTER(className);
}

//-----------------------------------------------------------------------------
std::string vtkDMMLSequenceNode::GetDataNodeTagName()
{
  std::string undefinedReturn="undefined";
  if (this->IndexEntries.empty())
    {
    return undefinedReturn;
    }
  // All the nodes should be of the same class, so just get the class from the first one
  vtkDMMLNode* node=this->IndexEntries[0].DataNode;
  if (node==nullptr)
    {
    vtkErrorMacro("vtkDMMLSequenceNode::GetDataNodeClassName node is invalid");
    return undefinedReturn;
    }
  const char* tagName=node->GetNodeTagName();
  if (tagName==nullptr)
    {
    return undefinedReturn;
    }
  return tagName;
}

//-----------------------------------------------------------------------------
vtkDMMLNode* vtkDMMLSequenceNode::GetNthDataNode(int itemNumber)
{
  if (static_cast<int>(this->IndexEntries.size())<=itemNumber)
    {
    vtkErrorMacro("vtkDMMLSequenceNode::GetNthDataNode failed: itemNumber "<<itemNumber<<" is out of range");
    return nullptr;
    }
  return this->IndexEntries[itemNumber].DataNode;
}

//-----------------------------------------------------------------------------
vtkDMMLScene* vtkDMMLSequenceNode::GetSequenceScene(bool autoCreate/*=true*/)
{
  if (!this->SequenceScene && autoCreate)
    {
    this->SequenceScene = vtkDMMLScene::New();
    }
  return this->SequenceScene;
}

//-----------------------------------------------------------------------------
vtkDMMLStorageNode* vtkDMMLSequenceNode::CreateDefaultStorageNode()
{
  vtkDMMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkDebugMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }

  return vtkDMMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass(this->GetDefaultStorageNodeClassName().c_str()));
}

//-----------------------------------------------------------
std::string vtkDMMLSequenceNode::GetDefaultStorageNodeClassName(const char* filename /* =nullptr */)
{
  // No need to create storage node if there are no nodes to store
  if (this->GetSequenceScene() == nullptr || this->GetSequenceScene()->GetNumberOfNodes() == 0)
    {
    return "";
    }

  // Use specific sequence storage node, if possible
  vtkDMMLStorableNode* storableNode = vtkDMMLStorableNode::SafeDownCast(this->GetNthDataNode(0));
  if (storableNode && this->GetScene())
    {
    std::string sequenceStorageNodeClassName = storableNode->GetDefaultSequenceStorageNodeClassName();
    vtkSmartPointer<vtkDMMLStorageNode> storageNode = vtkSmartPointer<vtkDMMLStorageNode>::Take(
      vtkDMMLStorageNode::SafeDownCast(this->GetScene()->CreateNodeByClass(sequenceStorageNodeClassName.c_str())));
    if (storageNode)
      {
      // Filename is not specified or it is specified and there is a supported file extension
      if (!filename || (filename && !storageNode->GetSupportedFileExtension(filename, false, true).empty()))
        {
        if (storageNode->CanWriteFromReferenceNode(this))
          {
          return storageNode->GetClassName();
          }
        }
      }
    }

  // Use generic storage node
  return "vtkDMMLSequenceStorageNode";
}

//-----------------------------------------------------------
void vtkDMMLSequenceNode::UpdateScene(vtkDMMLScene *scene)
{
  Superclass::UpdateScene(scene);

  // By now the storage node imported the sequence scene, so we can get the pointers to the data nodes
  this->UpdateSequenceIndex();
}

//-----------------------------------------------------------
void vtkDMMLSequenceNode::UpdateSequenceIndex()
{
  if (!this->SequenceScene)
    {
    // nothing to update
    return;
    }
  for (std::deque< IndexEntryType >::iterator indexIt = this->IndexEntries.begin(); indexIt != this->IndexEntries.end(); ++indexIt)
    {
    if (indexIt->DataNode == nullptr)
      {
      indexIt->DataNode = this->SequenceScene->GetNodeByID(indexIt->DataNodeID);
      if (indexIt->DataNode != nullptr)
        {
        // clear the ID to remove redundancy in the data
        indexIt->DataNodeID.clear();
        }
      }
    }
}

//-----------------------------------------------------------
void vtkDMMLSequenceNode::SetIndexTypeFromString(const char *indexTypeString)
{
  int indexType=GetIndexTypeFromString(indexTypeString);
  this->SetIndexType(indexType);
}

//-----------------------------------------------------------
std::string vtkDMMLSequenceNode::GetIndexTypeAsString()
{
  return vtkDMMLSequenceNode::GetIndexTypeAsString(this->IndexType);
}

//-----------------------------------------------------------
std::string vtkDMMLSequenceNode::GetIndexTypeAsString(int indexType)
{
  switch (indexType)
    {
    case vtkDMMLSequenceNode::NumericIndex: return "numeric";
    case vtkDMMLSequenceNode::TextIndex: return "text";
    default:
      return "";
    }
}

//-----------------------------------------------------------
int vtkDMMLSequenceNode::GetIndexTypeFromString(const std::string& indexTypeString)
{
  for (int i=0; i<vtkDMMLSequenceNode::NumberOfIndexTypes; i++)
    {
    if (indexTypeString == GetIndexTypeAsString(i))
      {
      // found it
      return i;
      }
    }
  return -1;
}

vtkDMMLNode* vtkDMMLSequenceNode::DeepCopyNodeToScene(vtkDMMLNode* source, vtkDMMLScene* scene)
{
  if (source == nullptr)
    {
    vtkGenericWarningMacro("vtkDMMLSequenceNode::DeepCopyNodeToScene failed, invalid node");
    return nullptr;
    }
  std::string baseName = "Data";
  if (source->GetAttribute("Sequences.BaseName") != 0)
    {
    baseName = source->GetAttribute("Sequences.BaseName");
    }
  else if (source->GetName() != 0)
    {
    baseName = source->GetName();
    }
  std::string newNodeName = baseName;

  vtkSmartPointer<vtkDMMLNode> target = vtkSmartPointer<vtkDMMLNode>::Take(source->CreateNodeInstance());
  target->CopyContent(source); // deep-copy

  // Generating unique node names is slow, and makes adding many nodes to a sequence too slow
  // We will instead ensure that all file names for storable nodes are unique when saving
  target->SetName(newNodeName.c_str());
  target->SetAttribute("Sequences.BaseName", baseName.c_str());

  vtkDMMLNode* addedTargetNode = scene->AddNode(target);
  return addedTargetNode;
}
