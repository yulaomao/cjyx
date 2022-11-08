/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

/// Markups Module DMML storage nodes
///
/// vtkDMMLMarkupsJsonStorageNode - DMML node for storing markups in JSON file
///

#ifndef __vtkDMMLMarkupsJsonStorageNode_h
#define __vtkDMMLMarkupsJsonStorageNode_h

// Markups includes
#include "vtkCjyxMarkupsModuleDMMLExport.h"
#include "vtkDMMLMarkupsStorageNode.h"

class vtkDMMLMarkupsNode;
class vtkDMMLMarkupsDisplayNode;

/// \ingroup Cjyx_QtModules_Markups
class VTK_CJYX_MARKUPS_MODULE_DMML_EXPORT vtkDMMLMarkupsJsonStorageNode : public vtkDMMLMarkupsStorageNode
{
public:
  static vtkDMMLMarkupsJsonStorageNode *New();
  vtkTypeMacro(vtkDMMLMarkupsJsonStorageNode,vtkDMMLMarkupsStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  /// Read a markups node from a file.
  vtkDMMLMarkupsNode* AddNewMarkupsNodeFromFile(const char* filePath, const char* nodeName=nullptr, int markupIndex=0);

  ///
  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "MarkupsJsonStorage";};

  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object
  void Copy(vtkDMMLNode *node) override;

  bool CanReadInReferenceNode(vtkDMMLNode *refNode) override;

  /// Returns a list of all markup types ("Curve", "ClosedCurve", "Angle", "Plane", etc.) in the json file.
  /// The types are ordered by the index in which they appear in the Json file.
  void GetMarkupsTypesInFile(const char* filePath, std::vector<std::string>& outputMarkupsTypes);

protected:
  vtkDMMLMarkupsJsonStorageNode();
  ~vtkDMMLMarkupsJsonStorageNode() override;
  vtkDMMLMarkupsJsonStorageNode(const vtkDMMLMarkupsJsonStorageNode&);
  void operator=(const vtkDMMLMarkupsJsonStorageNode&);

  /// Initialize all the supported write file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  /// Read data and set it in the referenced node
  int ReadDataInternal(vtkDMMLNode *refNode) override;

  /// Write data from a  referenced node.
  int WriteDataInternal(vtkDMMLNode *refNode) override;

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal;
};

#endif
