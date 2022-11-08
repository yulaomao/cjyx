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

// .NAME vtkCjyxMarkupsLogic - cjyx logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkCjyxMarkupsLogic_h
#define __vtkCjyxMarkupsLogic_h

// Cjyx includes
#include "vtkCjyxModuleLogic.h"

// DMML includes
#include "vtkDMMLStorageNode.h"

// VTK includes
#include "vtkVector.h"

// STD includes
#include <cstdlib>
#include <string>

#include "vtkCjyxMarkupsModuleLogicExport.h"

class vtkIdList;
class vtkMatrix4x4;
class vtkDMMLMarkupsNode;
class vtkDMMLMarkupsClosedCurveNode;
class vtkDMMLMarkupsDisplayNode;
class vtkDMMLMarkupsJsonStorageNode;
class vtkDMMLMessageCollection;
class vtkDMMLSelectionNode;
class vtkDMMLTableNode;
class vtkPlane;
class vtkPoints;
class vtkPolyData;
class vtkCjyxMarkupsWidget;

/// \ingroup Cjyx_QtModules_Markups
class VTK_CJYX_MARKUPS_MODULE_LOGIC_EXPORT vtkCjyxMarkupsLogic :
  public vtkCjyxModuleLogic
{
public:

  enum Events{
    MarkupRegistered = vtkCommand::UserEvent + 1,
    MarkupUnregistered
  };

  static vtkCjyxMarkupsLogic *New();
  vtkTypeMacro(vtkCjyxMarkupsLogic,vtkCjyxModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void ProcessDMMLNodesEvents(vtkObject *caller,
                                      unsigned long event,
                                      void *callData ) override;

  /// Utility method to return the id of the selection node. Checks
  /// the dmml application logic if set, otherwise checks the scene
  /// for a singleton selection node.
  /// Returns an empty string on failure.
  std::string GetSelectionNodeID();

  /// Utility method to return the id of the active place node.
  /// Returns an empty string on failure.
  /// \sa SetActiveListID
  std::string GetActiveListID();

  /// This method is deprecated due to its confusing name. It is kept for backward compatibility only,
  /// SetActiveList method should be used instead.
  void SetActiveListID(vtkDMMLMarkupsNode *markupsNode);

  /// Utility method to set the active place node from the passed markups
  /// node. Does not set the interaction mode to place.
  /// \sa GetActiveListID, StartPlaceMode
  /// \sa vtkDMMLSelectionNode::SetReferenceActivePlaceNodeClassName
  /// \sa vtkDMMLSelectionNode::SetActivePlaceNodeID
  void SetActiveList(vtkDMMLMarkupsNode* markupsNode);

  /// Create a new display node and observe it on the markups node.
  /// On success, return the id, on failure return an empty string.
  /// If a display node already exists for this node, return the id of that
  /// node.
  std::string AddNewDisplayNodeForMarkupsNode(vtkDMMLNode *dmmlNode);

  /// Create a new markups fiducial node and associated display node, adding both to
  /// the scene. If the scene argument is null use the scene set on the logic
  /// class, and also make it the active on on the selection node, otherwise
  /// add to the passed scene.
  /// On success, return the id, on failure return an empty string.
  std::string AddNewFiducialNode(const char *name = "F", vtkDMMLScene *scene = nullptr);

  /// Create a new markups node and associated display node, adding both to the scene.
  /// For ROI nodes, each new node will have a unique color.
  vtkDMMLMarkupsNode* AddNewMarkupsNode(std::string className, std::string nodeName=std::string(), vtkDMMLScene* scene = nullptr);

  /// Add a new control point to the currently active markups fiducial node at the given RAS
  /// coordinates (default 0,0,0). Will create a markups fiducial node if one is not active.
  /// Returns -1 on failure, index of the added control point
  /// on success.
  int AddControlPoint(double r=0.0, double a=0.0, double s=0.0);

  /// jump the slice windows to the given coordinate
  /// If viewGroup is -1 then all all slice views are updated, otherwise only those views
  /// that are in the specified group.
  void JumpSlicesToLocation(double x, double y, double z, bool centered, int viewGroup = -1, vtkDMMLSliceNode* exclude = nullptr);
  /// jump the slice windows to the nth markup with the dmml id id
  /// \sa JumpSlicesToLocation
  void JumpSlicesToNthPointInMarkup(const char *id, int n, bool centered = false, int viewGroup = -1, vtkDMMLSliceNode* exclude = nullptr);
  /// refocus all of the 3D cameras to the nth markup with the dmml id id
  /// \sa FocusCameraOnNthPointInMarkup
  void FocusCamerasOnNthPointInMarkup(const char *id, int n);
  /// refocus the camera with the given cameraNodeID on the nth markup in
  /// the markups node with id markupNodeID
  /// \sa FocusCamerasOnNthPointInMarkup
  void FocusCameraOnNthPointInMarkup(const char *cameraNodeID, const char *markupNodeID, int n);

  /// Load a markups node from fileName, return nullptr on error, node ID string
  /// otherwise. Adds the appropriate storage and display nodes to the scene
  /// as well.
  char* LoadMarkups(const char* fileName, const char* fidsName=nullptr, vtkDMMLMessageCollection* userMessages=nullptr);

  /// This method is deprecated. It is kept for backward compatibility only, it does the same as LoadMarkups method.
  char* LoadMarkupsFiducials(const char* fileName, const char* fidsName=nullptr, vtkDMMLMessageCollection* userMessages=nullptr);

  char* LoadMarkupsFromFcsv(const char* fileName, const char* nodeName=nullptr, vtkDMMLMessageCollection* userMessages=nullptr);
  char* LoadMarkupsFromJson(const char* fileName, const char* nodeName=nullptr, vtkDMMLMessageCollection* userMessages=nullptr);

  /// Utility methods to operate on all control points in a markups node
  /// @{
  void SetAllControlPointsVisibility(vtkDMMLMarkupsNode *node, bool flag);
  void ToggleAllControlPointsVisibility(vtkDMMLMarkupsNode *node);
  void SetAllControlPointsLocked(vtkDMMLMarkupsNode *node, bool flag);
  void ToggleAllControlPointsLocked(vtkDMMLMarkupsNode *node);
  void SetAllControlPointsSelected(vtkDMMLMarkupsNode *node, bool flag);
  void ToggleAllControlPointsSelected(vtkDMMLMarkupsNode *node);
  /// @}

  /// Utility method to set up a display node from the defaults.
  /// Point labels visibility and properties label visibility setting is not saved to defaults,
  /// as typically it is better to initialize these based on the node type (point labels are more
  /// useful for fiducials, while properties label is more useful for other markups).
  void SetDisplayNodeToDefaults(vtkDMMLMarkupsDisplayNode *displayNode);

  /// utility method to set defaults from display node
  void SetDisplayDefaultsFromNode(vtkDMMLMarkupsDisplayNode *displayNode);

  /// utility method to copy a control point from one list to another, adding it
  /// to the end of the new list
  /// \sa vtkDMMLMarkupsNode::AddControlPoint
  /// Returns true on success, false on failure
  bool CopyNthControlPointToNewList(int n, vtkDMMLMarkupsNode *markupsNode,
                              vtkDMMLMarkupsNode *newMarkupsNode);

  /// utility method to move a control point from one list to another, trying to
  /// insert it at the given new index. If the new index is larger than the
  /// number of control points in the list, adds it to the end. If new index is
  /// smaller than 0, adds it at the beginning. Otherwise inserts at
  /// that index.
  /// \sa vtkDMMLMarkupsNode::InsertControlPoint
  /// Returns true on success, false on failure
  bool MoveNthControlPointToNewListAtIndex(int n, vtkDMMLMarkupsNode *markupsNode,
                                   vtkDMMLMarkupsNode *newMarkupsNode, int newIndex);

  /// Searches the scene for annotation fidicual nodes, collecting a list
  /// of annotation hierarchy nodes. Then iterates through those hierarchy nodes
  /// and moves the fiducials that are under them into new markups nodes. Leaves
  /// the top level hierarchy nodes intact as they may be parents to ruler or
  /// ROIs but deletes the 1:1 hierarchy nodes.
  void ConvertAnnotationFiducialsToMarkups();

  /// Iterate over the control points in the list and reset the control point labels using
  /// the current ControlPointLabelFormat setting. Try to keep current numbering.
  /// Will work if there's a %d, %g or %f in the format string, but precision
  /// is not handled.
  void RenameAllControlPointsFromCurrentFormat(vtkDMMLMarkupsNode *markupsNode);

  /// Put the interaction node into place mode, and set the persistence of
  /// place mode according to the persistent flag.
  /// Return true on successfully going into place mode, false otherwise.
  /// By default, the default interaction node is updated.
  /// \sa SetActiveIDList
  bool StartPlaceMode(bool persistent, vtkDMMLInteractionNode* interactionNode = nullptr);

  vtkSetMacro(AutoCreateDisplayNodes, bool);
  vtkGetMacro(AutoCreateDisplayNodes, bool);
  vtkBooleanMacro(AutoCreateDisplayNodes, bool);

  vtkDMMLMarkupsDisplayNode* GetDefaultMarkupsDisplayNode();

  /// Copies basic display properties between markups display nodes. This is used
  /// for updating a display node to defaults.
  void CopyBasicDisplayProperties(vtkDMMLMarkupsDisplayNode *sourceDisplayNode, vtkDMMLMarkupsDisplayNode *targetDisplayNode);

  /// Measure surface area of the smooth surface that fits on the closed curve in world coordinate system.
  /// If projectWarp option is enabled then FitSurfaceProjectWarp method is used
  /// otherwise FitSurfaceDiskWarp is used. FitSurfaceProjectWarp produces accurate results
  /// for all quasi-planar curves (while FitSurfaceDiskWarp may significantly overestimate surface area for
  /// planar convex curves). FitSurfaceDiskWarp is kept for compatison only and may be removed in the future.
  /// \param curveNode points to fit the surface to
  /// \param surface if not nullptr then the generated surface is saved into that
  static double GetClosedCurveSurfaceArea(vtkDMMLMarkupsClosedCurveNode* curveNode, vtkPolyData* surface = nullptr, bool projectWarp = true);

  /// Create a "soap bubble" surface that fits on the provided point list.
  /// It can fill arbitrarily complex (non-self-intersecting) polygons in a plane or in a slightly curved plane.
  /// First the contour points projected to best fit plane, triangulated in 2D, and warped to the non-planar shape.
  /// Convex surfaces are triangulated correctly. If the contour is self-intersecting after projected to best fit plane
  /// then the surface will be invalid.
  /// \param curvePoints: points to fit the surface to
  /// \param radiusScalingFactor size of the surface. Value of 1.0 (default) means the surface edge fits on the points.
  /// Larger values increase the generated soap bubble outer radius, which may be useful to avoid coincident points
  /// when using this surface for cutting another surface.
  /// \param numberOfInternalGridPoints specifies the number of additional grid points that are added to get a more evenly triangulated
  /// surface. Default is 225, which corresponds to 15x15 subdivisions for a square shaped region.
  /// \warning Specifying radiusScalingFactor has no effect. Associated feature is not yet implemented.
  static bool FitSurfaceProjectWarp(vtkPoints* curvePoints, vtkPolyData* surface, double radiusScalingFactor = 1.0, vtkIdType numberOfInternalGridPoints=225);

  /// Create a "soap bubble" surface that fits on the provided point list.
  /// Compared to FitSurfaceProjectWarp, this method can tolerate more if points are not on a plane but it may not be able to
  /// fill complicated shapes (with sharp edges or many indentations).
  /// A triangulated disk is warped so that its boundary matches the provided curve points using thin-plate spline transform.
  /// The generated surface may go beyond the boundary of the input points if the boundary is highly concave or curved.
  /// \param curvePoints: points to fit the surface to
  /// \param radiusScalingFactor size of the surface.Value of 1.0 (default) means the surface edge fits on the points.
  /// Larger values increase the generated soap bubble outer radius, which may be useful to avoid coincident points
  /// when using this surface for cutting another surface.
  static bool FitSurfaceDiskWarp(vtkPoints* curvePoints, vtkPolyData* surface, double radiusScalingFactor = 1.0);

  /// Return true if the polygon points are oriented clockwise.
  /// If pointIds is null then point IDs will be 0, 1, 2, ... n-1.
  static bool IsPolygonClockwise(vtkPoints* points, vtkIdList* pointIds=nullptr);

  /// Get best fit plane for a markup
  static bool GetBestFitPlane(vtkDMMLMarkupsNode* curveNode, vtkPlane* plane);

  std::string GetJsonStorageNodeClassNameForMarkupsType(std::string markupsType);
  void RegisterJsonStorageNodeForMarkupsType(std::string markupsType, std::string storageNodeClassName);
  vtkDMMLMarkupsJsonStorageNode* AddNewJsonStorageNodeForMarkupsType(std::string markupsType);

  /// Registers a markup and its corresponding widget to be handled by the Markups module.
  /// For a markup to be handled by this module (processed by the displayable
  /// manager, UI and subject hierarchy) it needs to be registered using this method.
  /// The method also registers the markupsNode class in the scene.
  /// \param markupsNode DMMLMarkups node to be registered.
  /// \param markupsWidget vtkCjyxWidget associated to the DMMLMarkups node registered.
  void RegisterMarkupsNode(vtkDMMLMarkupsNode* markupsNode,
                           vtkCjyxMarkupsWidget* markupsWidget,
                           bool createPushButton=true);

  /// Unregisters a markup and its corresponding widget. This will trigger the
  /// vtkCjyxMarkupsLogic::MarkupUnregistered event.
  /// \param markupsNode DMMLMarkups node to be unregistered.
  void UnregisterMarkupsNode(vtkDMMLMarkupsNode*  markupsNode);

  /// Returns true if the provided class name is a known markups class
  /// (it has ben registered in the logic using RegisterMarkupsNode).
  bool IsMarkupsNodeRegistered(const char* nodeType) const;

  /// This returns an instance to a corresponding vtkCjyxMarkupsWidget associated
  /// to the indicated markups name.
  /// \param markupsType registered class to retrieve the associated widget.
  /// \return pointer to associated vtkCJyxMarkupsWidget or nullptr if the DMML node
  /// class is not registered.
  vtkCjyxMarkupsWidget* GetWidgetByMarkupsType(const char* markupsType) const;

  /// This returns an instance to a corresponding vtkDMMLMarkupsNode associated
  /// to the indicated markups name.
  /// \param makrupsType registered class to retrieve the associated widget.
  /// \return pointer to associated vtkCJyxMarkupsWidget or nullptr if the DMML node
  /// class is not registered.
  vtkDMMLMarkupsNode* GetNodeByMarkupsType(const char* markupsType) const;

  /// This returns the list of the markups registered in the logic
  const std::list<std::string>& GetRegisteredMarkupsTypes() const;

  /// This returns the flags that indicates whether the GUI push button should be created.
  bool GetCreateMarkupsPushButton(const char* markupName) const;

  /// Import markups control points from table node
  /// Column names: label, r, a, s, (or l, p, s), defined, selected, visible, locked, description.
  static bool ImportControlPointsFromTable(vtkDMMLMarkupsNode* markupsNode, vtkDMMLTableNode* tableNode,
    int startRow = 0, int numberOfRows = -1);

  static bool ExportControlPointsToTable(vtkDMMLMarkupsNode* markupsNode, vtkDMMLTableNode* tableNode,
    int coordinateSystem = vtkDMMLStorageNode::CoordinateSystemRAS);

  /// Export markups node control points to CSV file.
  /// \param markupsNode Node that the control points are exported to.
  /// \param filename Output filename.
  /// \param lps Save files in LPS coordinate system. If set to false then RAS coordinate system is used.
  static bool ExportControlPointsToCSV(vtkDMMLMarkupsNode* markupsNode, const std::string filename, bool lps = true);

  /// Import markups node control points from CSV file.
  /// \param markupsNode Node that the control points are imported from.
  /// \param filename Input filename.
  static bool ImportControlPointsFromCSV(vtkDMMLMarkupsNode* markupsNode, const std::string filename);

  //-----------------------------------------------------------
  // All public methods below are deprecated
  //
  // These methods are deprecated because they use old terms (markup instead of control point),

  /// \deprecated Use CopyNthControlPointToNewList instead.
  bool CopyNthMarkupToNewList(int n, vtkDMMLMarkupsNode *markupsNode,
                              vtkDMMLMarkupsNode *newMarkupsNode)
    {
    vtkWarningMacro("vtkCjyxMarkupsLogic::CopyNthMarkupToNewList method is deprecated, please use CopyNthControlPointToNewList instead");
    return this->CopyNthControlPointToNewList(n, markupsNode, newMarkupsNode);
    }
  /// \deprecated Use MoveNthControlPointToNewList instead.
  bool MoveNthMarkupToNewList(int n, vtkDMMLMarkupsNode *markupsNode,
                              vtkDMMLMarkupsNode *newMarkupsNode, int newIndex)
    {
    vtkWarningMacro("vtkCjyxMarkupsLogic::MoveNthMarkupToNewList method is deprecated, please use MoveNthControlPointToNewListAtIndex instead");
    return this->MoveNthControlPointToNewListAtIndex(n, markupsNode, newMarkupsNode, newIndex);
    }
  /// \deprecated Use AddControlPoint instead.
  int AddFiducial(double r=0.0, double a=0.0, double s=0.0)
    {
    vtkWarningMacro("vtkCjyxMarkupsLogic::AddFiducial method is deprecated, please use AddControlPoint instead");
    return this->AddControlPoint(r, a, s);
    };
  /// \deprecated Use SetAllControlPointsVisibility instead.
  void SetAllMarkupsVisibility(vtkDMMLMarkupsNode *node, bool flag)
    {
    vtkWarningMacro("vtkCjyxMarkupsLogic::SetAllMarkupsVisibility method is deprecated, please use SetAllControlPointsVisibility instead");
    this->SetAllControlPointsVisibility(node, flag);
    };
  /// \deprecated Use ToggleAllControlPointsVisibility instead.
  void ToggleAllMarkupsVisibility(vtkDMMLMarkupsNode *node)
    {
    vtkWarningMacro("vtkCjyxMarkupsLogic::ToggleAllMarkupsVisibility method is deprecated, please use ToggleAllControlPointsVisibility instead");
    this->ToggleAllControlPointsVisibility(node);
    };
  /// \deprecated Use SetAllControlPointsLocked instead.
  void SetAllMarkupsLocked(vtkDMMLMarkupsNode *node, bool flag)
    {
    vtkWarningMacro("vtkCjyxMarkupsLogic::SetAllMarkupsLocked method is deprecated, please use SetAllControlPointsLocked instead");
    this->SetAllControlPointsLocked(node, flag);
    };
  /// \deprecated Use ToggleAllControlPointsLocked instead.
  void ToggleAllMarkupsLocked(vtkDMMLMarkupsNode *node)
    {
    vtkWarningMacro("vtkCjyxMarkupsLogic::ToggleAllMarkupsLocked method is deprecated, please use ToggleAllControlPointsLocked instead");
    this->ToggleAllControlPointsLocked(node);
    };
  /// \deprecated Use SetAllControlPointsSelected instead.
  void SetAllMarkupsSelected(vtkDMMLMarkupsNode *node, bool flag)
    {
    vtkWarningMacro("vtkCjyxMarkupsLogic::SetAllMarkupsSelected method is deprecated, please use SetAllControlPointsSelected instead");
    this->SetAllControlPointsSelected(node, flag);
    };
  /// \deprecated Use ToggleAllControlPointsSelected instead.
  void ToggleAllMarkupsSelected(vtkDMMLMarkupsNode *node)
    {
    vtkWarningMacro("vtkCjyxMarkupsLogic::ToggleAllMarkupsSelected method is deprecated, please use ToggleAllControlPointsSelected instead");
    this->ToggleAllControlPointsSelected(node);
    };
  /// \deprecated Use RenameAllControlPointsFromCurrentFormat instead.
  void RenameAllMarkupsFromCurrentFormat(vtkDMMLMarkupsNode *markupsNode)
    {
    vtkWarningMacro("vtkCjyxMarkupsLogic::RenameAllMarkupsFromCurrentFormat method is deprecated, please use RenameAllControlPointsFromCurrentFormat instead");
    this->RenameAllControlPointsFromCurrentFormat(markupsNode);
    };

  //@{
  /// Generate a unique color for a markup node.
  /// In the current implementation, the color is not globally unique, but colors are generated
  /// by iterating through the items in "MediumChart" color table.
  vtkVector3d GenerateUniqueColor();
  void GenerateUniqueColor(double color[3]);
  //@}

protected:

  vtkCjyxMarkupsLogic();
  ~vtkCjyxMarkupsLogic() override;

  /// Initialize listening to DMML events
  void SetDMMLSceneInternal(vtkDMMLScene * newScene) override;
  void ObserveDMMLScene() override;
  void SetAndObserveSelectionNode(vtkDMMLSelectionNode* selectionNode);

  /// Update list of place node class names in selection node based on currently registered markups.
  void UpdatePlaceNodeClassNamesInSelectionNode();

  /// Register DMML Node classes to Scene. Gets called automatically when the DMMLScene is attached to this logic class.
  void RegisterNodes() override;
  void UpdateFromDMMLScene() override;
  void OnDMMLSceneNodeAdded(vtkDMMLNode* node) override;
  void OnDMMLSceneNodeRemoved(vtkDMMLNode* node) override;

private:

  vtkCjyxMarkupsLogic(const vtkCjyxMarkupsLogic&) = delete;
  void operator=(const vtkCjyxMarkupsLogic&) = delete;

  bool AutoCreateDisplayNodes;

  class vtkInternal;
  vtkInternal* Internal;
};

#endif
