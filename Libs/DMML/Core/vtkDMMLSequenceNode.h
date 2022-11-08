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

#ifndef __vtkDMMLSequenceNode_h
#define __vtkDMMLSequenceNode_h

// DMML includes
#include <vtkDMML.h>
#include <vtkDMMLStorableNode.h>

// std includes
#include <deque>
#include <set>


/// \brief DMML node for representing a sequence of DMML nodes
///
/// This node contains a sequence of nodes (data nodes).
///
/// The data nodes can be referred to by an index, for example:
/// IndexName: time
/// IndexUnit: s
/// IndexNumeric: YES
/// IndexValue: 3.4567
///
/// If an index is numeric then it is sorted differently and equality determined using
/// a numerical tolerance instead of exact string matching.

class VTK_DMML_EXPORT vtkDMMLSequenceNode : public vtkDMMLStorableNode
{
public:
  static vtkDMMLSequenceNode *New();
  vtkTypeMacro(vtkDMMLSequenceNode,vtkDMMLStorableNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Create instance of a sequence node
  vtkDMMLNode* CreateNodeInstance() override;

  /// Set node attributes from name/value pairs
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
/// Copy the node's attributes to this object
  void Copy(vtkDMMLNode* node) override;

  /// Copy sequence index information (index name, unit, type, values, etc)
  /// Does not copy data nodes.
  virtual void CopySequenceIndex(vtkDMMLNode *node);

  /// Update sequence index to point to nodes
  void UpdateSequenceIndex();

  /// Get unique node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "Sequence";};

  /// Set index name (example: time)
  void SetIndexName(const std::string& str);
  /// Get index name (example: time)
  vtkGetMacro(IndexName, std::string);

  /// Set unit for the index (example: s)
  void SetIndexUnit(const std::string& str);
  /// Get unit for the index (example: s)
  vtkGetMacro(IndexUnit, std::string);

  /// Set the type of the index (numeric, text, ...)
  void SetIndexType(int indexType);
  void SetIndexTypeFromString(const char *indexTypeString);
  /// Get the type of the index (numeric, text, ...)
  vtkGetMacro(IndexType, int);
  virtual std::string GetIndexTypeAsString();

  /// Get tolerance value for comparing numerix index values. If index values differ by less than the tolerance
  /// then the index values considered to be equal.
  vtkGetMacro(NumericIndexValueTolerance, double);
  /// Set tolerance value for comparing numerix index values.
  void SetNumericIndexValueTolerance(double tolerance);

  /// Helper functions for converting between string and code representation of the index type
  static std::string GetIndexTypeAsString(int indexType);
  static int GetIndexTypeFromString(const std::string &indexTypeString);

  /// Add a copy of the provided node to this sequence as a data node.
  /// If a sequence item is not found by that index, a new item is added.
  /// Always performs deep-copy.
  /// Returns the data node copy that has just been created.
  vtkDMMLNode* SetDataNodeAtValue(vtkDMMLNode* node, const std::string& indexValue);

  /// Update an existing data node.
  /// Return true if a data node was found by that index.
  bool UpdateDataNodeAtValue(vtkDMMLNode* node, const std::string& indexValue, bool shallowCopy = false);

  /// Remove data node corresponding to the specified index
  void RemoveDataNodeAtValue(const std::string& indexValue);

  /// Remove all data nodes from the sequence
  void RemoveAllDataNodes();

  /// Get the node corresponding to the specified index value
  /// If exact match is not required and index is numeric then the best matching data node is returned.
  vtkDMMLNode* GetDataNodeAtValue(const std::string& indexValue, bool exactMatchRequired = true);

  /// Get the data node corresponding to the n-th index value
  vtkDMMLNode* GetNthDataNode(int itemNumber);

  /// Index value of n-th data node.
  std::string GetNthIndexValue(int itemNumber);

  /// If exact match is not required and index is numeric then the best matching data node is returned.
  /// If the sequences has numeric index, uses data node just before the index value in the case of non-exact match
  int GetItemNumberFromIndexValue(const std::string& indexValue, bool exactMatchRequired = true);

  /// Change index value of an existing data node.
  bool UpdateIndexValue(const std::string& oldIndexValue, const std::string& newIndexValue);

  /// Return the number of nodes stored in this sequence.
  int GetNumberOfDataNodes();

  /// Return the class name of the data nodes (e.g., vtkDMMLTransformNode). If there are no data nodes yet then it returns empty string.
  std::string GetDataNodeClassName();

  /// Return the human-readable type name of the data nodes (e.g., TransformNode). If there are no data nodes yet then it returns the string "undefined".
  std::string GetDataNodeTagName();

  /// Return the internal scene that stores all the data nodes.
  /// If autoCreate is enabled then the sequence scene is created
  /// (if it has not been created already).
  vtkDMMLScene* GetSequenceScene(bool autoCreate=true);

  /// Create default storage node. Uses vtkDMMLSequenceStorageNode unless the data node
  /// requests a more specific storage node class.
  vtkDMMLStorageNode* CreateDefaultStorageNode() override;

  /// Returns the most specific storage node possible (such as stvtkDMMLVolumeSequenceStorageNode
  /// if sequence contains volumes with the same type and geometry, or vtkDMMLLinearTransformSequenceStorageNode
  /// if sequence contains a list of linear transforms) and generic vtkDMMLSequenceStorageNode otherwise.
  std::string GetDefaultStorageNodeClassName(const char* filename = nullptr) override;

  /// Update node IDs in case of node ID conflicts on scene import
  void UpdateScene(vtkDMMLScene *scene) override;

  /// Type of the index. Controls the behavior of sorting, finding, etc.
  /// Additional types may be added in the future, such as tag cloud, two-dimensional index, ...
  enum IndexTypes
    {
    NumericIndex = 0,
    TextIndex,
    NumberOfIndexTypes // this line must be the last one
    };

protected:
  vtkDMMLSequenceNode();
  ~vtkDMMLSequenceNode() override;
  vtkDMMLSequenceNode(const vtkDMMLSequenceNode&);
  void operator=(const vtkDMMLSequenceNode&);

  /// Get the index where an item would need to be inserted to.
  /// If numeric index then insert it by respecting sorting order, otherwise insert to the end.
  int GetInsertPosition(const std::string& indexValue);

  void ReadIndexValues(const std::string& indexText);

  vtkDMMLNode* DeepCopyNodeToScene(vtkDMMLNode* source, vtkDMMLScene* scene);

  struct IndexEntryType
    {
    std::string IndexValue;
    vtkDMMLNode* DataNode;
    std::string DataNodeID; // only used temporarily, during scene load
    };

protected:

  /// Describes index of the sequence node
  std::string IndexName;
  std::string IndexUnit;
  int IndexType{vtkDMMLSequenceNode::NumericIndex};
  double NumericIndexValueTolerance{0.001};

  /// Need to store the nodes in the scene, because for reading/writing nodes
  /// we need DMML storage nodes, which only work if they refer to a data node in the same scene
  vtkDMMLScene* SequenceScene{0};

  /// List of data items (the scene may contain some more nodes, such as storage nodes)
  std::deque< IndexEntryType > IndexEntries;
};

#endif
