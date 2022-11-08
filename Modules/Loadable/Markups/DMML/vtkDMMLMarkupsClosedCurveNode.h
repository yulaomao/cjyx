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

#ifndef __vtkDMMLMarkupsClosedCurveNode_h
#define __vtkDMMLMarkupsClosedCurveNode_h

// DMML includes
#include "vtkDMMLDisplayableNode.h"

// Markups includes
#include "vtkCjyxMarkupsModuleDMMLExport.h"
#include "vtkDMMLMarkupsDisplayNode.h"
#include "vtkDMMLMarkupsCurveNode.h"

/// \brief DMML node to represent a closed curve markup
/// Closed Curve Markups nodes contain N control points.
/// Visualization parameters are set in the vtkDMMLMarkupsDisplayNode class.
///
/// Markups is intended to be used for manual marking/editing of point positions.
///
/// \ingroup Cjyx_QtModules_Markups
class  VTK_CJYX_MARKUPS_MODULE_DMML_EXPORT vtkDMMLMarkupsClosedCurveNode : public vtkDMMLMarkupsCurveNode
{
public:
  static vtkDMMLMarkupsClosedCurveNode *New();
  vtkTypeMacro(vtkDMMLMarkupsClosedCurveNode, vtkDMMLMarkupsCurveNode);

  const char* GetIcon() override {return ":/Icons/MarkupsClosedCurve.png";}
  const char* GetAddIcon() override {return ":/Icons/MarkupsClosedCurveMouseModePlace.png";}
  const char* GetPlaceAddIcon() override {return ":/Icons/MarkupsClosedCurveMouseModePlaceAdd.png";}

  //--------------------------------------------------------------------------
  // DMMLNode methods
  //--------------------------------------------------------------------------

  vtkDMMLNode* CreateNodeInstance() override;
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "MarkupsClosedCurve";}

  /// Get markup type internal name
  const char* GetMarkupType() override {return "ClosedCurve";};

  // Get markup type GUI display name
  const char* GetTypeDisplayName() override {return "Closed Curve";};

  /// Get markup short name
  const char* GetDefaultNodeNamePrefix() override {return "CC";};

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkDMMLNode::CopyContent
  vtkDMMLCopyContentDefaultMacro(vtkDMMLMarkupsClosedCurveNode);


  //@{
  /// For internal use only.
  /// Moved here temporarily until pluggable markups will be implemented.
  /// Then the methods will be moved back to markups logic.
  static double GetClosedCurveSurfaceArea(vtkDMMLMarkupsClosedCurveNode* curveNode, vtkPolyData* surface = nullptr, bool projectWarp = true);
  static bool FitSurfaceProjectWarp(vtkPoints* curvePoints, vtkPolyData* surface, double radiusScalingFactor = 1.0, vtkIdType numberOfInternalGridPoints=225);
  static bool FitSurfaceDiskWarp(vtkPoints* curvePoints, vtkPolyData* surface, double radiusScalingFactor = 1.0);
  static bool IsPolygonClockwise(vtkPoints* points, vtkIdList* pointIds=nullptr);
  //@}

protected:
  vtkDMMLMarkupsClosedCurveNode();
  ~vtkDMMLMarkupsClosedCurveNode() override;
  vtkDMMLMarkupsClosedCurveNode(const vtkDMMLMarkupsClosedCurveNode&);
  void operator=(const vtkDMMLMarkupsClosedCurveNode&);

};

#endif
