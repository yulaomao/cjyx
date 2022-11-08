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

#ifndef __vtkDMMLMarkupsFiducialNode_h
#define __vtkDMMLMarkupsFiducialNode_h

// DMML includes
#include "vtkDMMLDisplayableNode.h"

// Markups includes
#include "vtkCjyxMarkupsModuleDMMLExport.h"
#include "vtkDMMLMarkupsDisplayNode.h"
#include "vtkDMMLMarkupsNode.h"

/// \brief DMML node to represent a fiducial markup
/// Fiducial Markups nodes contain a list of control points.
/// Visualization parameters are set in the vtkDMMLMarkupsDisplayNode class.
///
/// Markups is intended to be used for manual marking/editing of point positions.
/// There is no specific limit for number of points that can be added to a list,
/// but performance is optimal if there are less than 2000 points.
///
/// \ingroup Cjyx_QtModules_Markups
class  VTK_CJYX_MARKUPS_MODULE_DMML_EXPORT vtkDMMLMarkupsFiducialNode : public vtkDMMLMarkupsNode
{
public:
  static vtkDMMLMarkupsFiducialNode *New();
  vtkTypeMacro(vtkDMMLMarkupsFiducialNode,vtkDMMLMarkupsNode);
  /// Print out the node information to the output stream
  void PrintSelf(ostream& os, vtkIndent indent) override;

  const char* GetIcon() override {return ":/Icons/MarkupsFiducial.png";}
  const char* GetAddIcon() override {return ":/Icons/MarkupsFiducialMouseModePlace.png";}
  const char* GetPlaceAddIcon() override {return ":/Icons/MarkupsFiducialMouseModePlaceAdd.png";}

  //--------------------------------------------------------------------------
  // DMMLNode methods
  //--------------------------------------------------------------------------

  vtkDMMLNode* CreateNodeInstance() override;

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "MarkupsFiducial";}

  /// Get markup type internal name
  const char* GetMarkupType() override {return "Fiducial";};

  // Get markup type GUI display name
  const char* GetTypeDisplayName() override {return "Point List";};

  /// Get markup short name
  const char* GetDefaultNodeNamePrefix() override {return "F";};

  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkDMMLNode::CopyContent
  vtkDMMLCopyContentMacro(vtkDMMLMarkupsFiducialNode);

  /// Maximum number of control points limits the number of markups allowed in the node.
  /// If maximum number of control points is set to 0 then no it means there
  /// is no limit (this is the default value).
  /// The value is an indication to the user interface and does not affect
  /// prevent adding markups to a node programmatically.
  /// If value is set to lower value than the number of markups in the node, then
  /// existing markups are not deleted.
  /// 2 for line, and 3 for angle Markups
  vtkSetMacro(MaximumNumberOfControlPoints, int);

  /// Set the number of control points that are required for defining this widget.
  /// Interaction mode remains in "place" mode until this number is reached.
  /// If the number is set to 0 then no it means there is no preference (this is the default value).
  vtkSetMacro(RequiredNumberOfControlPoints, int);

  /// Create and observe default display node(s)
  void CreateDefaultDisplayNodes() override;

  //-----------------------------------------------------------
  // All public methods below are deprecated
  //
  // These methods are either deprecated because they use old terms (markup instead of control point),
  // or include "array", "vector", "pointer" in the name (it is redundant, as input arguments can be
  // deduced from the type; and return type for vectors is always vtkVectorNd).
  //

  /// \deprecated Use GetNumberOfControlPoints instead.
  int GetNumberOfFiducials()
    {
    vtkWarningMacro("vtkDMMLMarkupsFiducialNode::GetNumberOfFiducials method is deprecated, please use GetNumberOfControlPoints instead");
    return this->GetNumberOfControlPoints();
    };
  /// \deprecated Use AddControlPoint instead.
  int AddFiducial(double x, double y, double z, std::string label = std::string())
    {
    vtkWarningMacro("vtkDMMLMarkupsFiducialNode::AddFiducial method is deprecated, please use AddControlPoint instead");
    return this->AddControlPoint(vtkVector3d(x, y, z), label);
    }
  /// Add a new fiducial from an array and return the fiducial index
  /// \deprecated Use AddControlPoint instead.
  int AddFiducialFromArray(double pos[3], std::string label = std::string())
    {
    vtkWarningMacro("vtkDMMLMarkupsFiducialNode::AddFiducialFromArray method is deprecated, please use AddControlPoint instead");
    return this->AddControlPoint(pos, label);
    }
  /// \deprecated Use GetNthControlPointPosition instead.
  void GetNthFiducialPosition(int n, double pos[3])
    {
    vtkWarningMacro("vtkDMMLMarkupsFiducialNode::GetNthFiducialPosition method is deprecated, please use GetNthControlPointPosition instead");
    this->GetNthControlPointPosition(n, pos);
    }
  /// \deprecated Use SetNthControlPointPosition instead.
  void SetNthFiducialPosition(int n, double x, double y, double z)
    {
    vtkWarningMacro("vtkDMMLMarkupsFiducialNode::SetNthFiducialPosition method is deprecated, please use SetNthControlPointPosition instead");
    this->SetNthControlPointPosition(n, x, y, z);
    };
  /// \deprecated Use SetNthControlPointPositionFromArray instead.
  void SetNthFiducialPositionFromArray(int n, double pos[3])
    {
    vtkWarningMacro("vtkDMMLMarkupsFiducialNode::SetNthFiducialPositionFromArray method is deprecated, please use SetNthControlPointPositionFromArray instead");
    this->SetNthControlPointPositionFromArray(n, pos);
    };
  /// \deprecated Use GetNthControlPointSelected instead.
  bool GetNthFiducialSelected(int n = 0)
    {
    vtkWarningMacro("vtkDMMLMarkupsFiducialNode::GetNthFiducialSelected method is deprecated, please use GetNthControlPointSelected instead");
    return this->GetNthControlPointSelected(n);
    };
  /// \deprecated Use SetNthControlPointSelected instead.
  void SetNthFiducialSelected(int n, bool flag)
    {
    vtkWarningMacro("vtkDMMLMarkupsFiducialNode::SetNthFiducialSelected method is deprecated, please use SetNthControlPointSelected instead");
    this->SetNthControlPointSelected(n, flag);
    };
  /// \deprecated Use GetNthControlPointLocked instead.
  bool GetNthFiducialLocked(int n = 0)
    {
    vtkWarningMacro("vtkDMMLMarkupsFiducialNode::GetNthFiducialLocked method is deprecated, please use GetNthControlPointLocked instead");
    return this->GetNthControlPointLocked(n);
    };
  /// \deprecated Use SetNthControlPointLocked instead.
  void SetNthFiducialLocked(int n, bool flag)
    {
    vtkWarningMacro("vtkDMMLMarkupsFiducialNode::SetNthFiducialLocked method is deprecated, please use SetNthControlPointLocked instead");
    this->SetNthControlPointLocked(n, flag);
    };
  /// \deprecated Use GetNthControlPointVisibility instead.
  bool GetNthFiducialVisibility(int n = 0)
    {
    vtkWarningMacro("vtkDMMLMarkupsFiducialNode::GetNthFiducialVisibility method is deprecated, please use GetNthControlPointVisibility instead");
    return this->GetNthControlPointVisibility(n);
    };
  /// \deprecated Use SetNthControlPointVisibility instead.
  void SetNthFiducialVisibility(int n, bool flag)
    {
    vtkWarningMacro("vtkDMMLMarkupsFiducialNode::SetNthFiducialVisibility method is deprecated, please use SetNthControlPointVisibility instead");
    this->SetNthControlPointVisibility(n, flag);
    };
  /// \deprecated Use GetNthControlPointLabel instead.
  std::string GetNthFiducialLabel(int n = 0)
    {
    vtkWarningMacro("vtkDMMLMarkupsFiducialNode::GetNthFiducialLabel method is deprecated, please use GetNthControlPointLabel instead");
    return this->GetNthControlPointLabel(n);
    };
  /// \deprecated Use SetNthControlPointLabel instead.
  void SetNthFiducialLabel(int n, std::string label)
    {
    vtkWarningMacro("vtkDMMLMarkupsFiducialNode::SetNthFiducialLabel method is deprecated, please use SetNthControlPointLabel instead");
    this->SetNthControlPointLabel(n, label);
    };
  /// \deprecated Use GetNthControlPointAssociatedNodeID instead.
  std::string GetNthFiducialAssociatedNodeID(int n = 0)
    {
    vtkWarningMacro("vtkDMMLMarkupsFiducialNode::GetNthFiducialAssociatedNodeID method is deprecated, please use GetNthControlPointAssociatedNodeID instead");
    return this->GetNthControlPointAssociatedNodeID(n);
    };
  /// \deprecated Use SetNthControlPointAssociatedNodeID instead.
  void SetNthFiducialAssociatedNodeID(int n, const char* id)
    {
    vtkWarningMacro("vtkDMMLMarkupsFiducialNode::SetNthFiducialAssociatedNodeID method is deprecated, please use SetNthControlPointAssociatedNodeID instead");
    this->SetNthControlPointAssociatedNodeID(n, (id ? std::string(id) : ""));
    };
  /// \deprecated Use SetNthControlPointPositionWorld instead.
  void SetNthFiducialWorldCoordinates(int n, double coords[4])
    {
    vtkWarningMacro("vtkDMMLMarkupsFiducialNode::SetNthFiducialWorldCoordinates method is deprecated, please use SetNthControlPointPositionWorld instead");
    this->SetNthControlPointPositionWorld(n, coords[0], coords[1], coords[2]);
    };
  /// \deprecated Use GetNthControlPointPositionWorld instead.
  void GetNthFiducialWorldCoordinates(int n, double coords[4])
    {
    vtkWarningMacro("vtkDMMLMarkupsFiducialNode::GetNthFiducialWorldCoordinates method is deprecated, please use GetNthControlPointPositionWorld instead");
    this->GetNthControlPointPositionWorld(n, coords);
    };

protected:
  vtkDMMLMarkupsFiducialNode();
  ~vtkDMMLMarkupsFiducialNode() override;
  vtkDMMLMarkupsFiducialNode(const vtkDMMLMarkupsFiducialNode&);
  void operator=(const vtkDMMLMarkupsFiducialNode&);

};

#endif
