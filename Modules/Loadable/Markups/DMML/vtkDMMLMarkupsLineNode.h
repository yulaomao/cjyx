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

#ifndef __vtkDMMLMarkupsLineNode_h
#define __vtkDMMLMarkupsLineNode_h

// DMML includes
#include "vtkDMMLDisplayableNode.h"

// Markups includes
#include "vtkCjyxMarkupsModuleDMMLExport.h"
#include "vtkDMMLMarkupsDisplayNode.h"
#include "vtkDMMLMarkupsNode.h"

/// \brief DMML node to represent a line markup
/// Line Markups nodes contain two control points.
/// Visualization parameters are set in the vtkDMMLMarkupsDisplayNode class.
///
/// Markups is intended to be used for manual marking/editing of point positions.
///
/// \ingroup Cjyx_QtModules_Markups
class  VTK_CJYX_MARKUPS_MODULE_DMML_EXPORT vtkDMMLMarkupsLineNode : public vtkDMMLMarkupsNode
{
public:
  static vtkDMMLMarkupsLineNode *New();
  vtkTypeMacro(vtkDMMLMarkupsLineNode,vtkDMMLMarkupsNode);
  /// Print out the node information to the output stream
  void PrintSelf(ostream& os, vtkIndent indent) override;

  const char* GetIcon() override {return ":/Icons/MarkupsLine.png";}
  const char* GetAddIcon() override {return ":/Icons/MarkupsLineMouseModePlace.png";}
  const char* GetPlaceAddIcon() override {return ":/Icons/MarkupsLineMouseModePlaceAdd.png";}

  //--------------------------------------------------------------------------
  // DMMLNode methods
  //--------------------------------------------------------------------------

  vtkDMMLNode* CreateNodeInstance() override;
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "MarkupsLine";}

  /// Get markup type internal name
  const char* GetMarkupType() override {return "Line";};

  // Get markup type GUI display name
  const char* GetTypeDisplayName() override {return "Line";};

  /// Get markup short name
  const char* GetDefaultNodeNamePrefix() override {return "L";};

  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkDMMLNode::CopyContent
  vtkDMMLCopyContentDefaultMacro(vtkDMMLMarkupsLineNode);

  /// Return line length (distance between the two line endpoints) in world coordinate system.
  virtual double GetLineLengthWorld();

protected:
  vtkDMMLMarkupsLineNode();
  ~vtkDMMLMarkupsLineNode() override;
  vtkDMMLMarkupsLineNode(const vtkDMMLMarkupsLineNode&);
  void operator=(const vtkDMMLMarkupsLineNode&);

  /// Calculates the handle to world matrix based on the current control points
  void UpdateInteractionHandleToWorldMatrix() override;
};

#endif
