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

#ifndef __vtkProjectMarkupsCurvePointsFilter_h
#define __vtkProjectMarkupsCurvePointsFilter_h

#include "vtkCjyxMarkupsModuleDMMLExport.h"

#include <vtkInformation.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkWeakPointer.h>

class vtkDoubleArray;
class vtkOBBTree;
class vtkPointLocator;
class vtkPoints;
class vtkPolyData;

class vtkDMMLMarkupsCurveNode;
class vtkDMMLModelNode;

/// \brief Projects curve points from a vtkDMMLMarkupsCurveNode to the surface of a model.
///
/// If the \a inputCurveNode set by SetInputCurveNode does not have a surface constraint node,
/// this filter will act as a pass through and leave the points unchanged.
///
/// The intended use is for vtkDMMLMarkupsCurveNode to project generated points along a curve
/// to a surface. It is expected that the points given to SetInputData/SetInputConnection are
/// actually along the curve defined by the curve node's control point positions world.
///
/// This class is not meant to be a general purpose point projection filter.
class VTK_CJYX_MARKUPS_MODULE_DMML_EXPORT vtkProjectMarkupsCurvePointsFilter : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkProjectMarkupsCurvePointsFilter, vtkPolyDataAlgorithm);
  static vtkProjectMarkupsCurvePointsFilter* New();

  /// Sets the input curve node.
  ///
  /// The surface model to project to is retrieved using vtkDMMLMarkupsCurveNode::GetSurfaceConstraintNode()
  /// and the input curve's control points are retrieved using vtkDMMLMarkupsCurveNode::GetControlPointPositionsWorld().
  ///
  /// If the \a inputCurveNode does not have a surface constraint node, this filter acts as a pass through
  /// and leave the points unchanged.
  ///
  /// It does not take ownership of the node and will not extend its lifetime.
  ///
  /// If the \a inputCurveNode is deleted during this object's lifetime it will be as if nullptr was passed
  /// into this function.
  void SetInputCurveNode(vtkDMMLMarkupsCurveNode* inputCurveNode);

  ///@{
  /// Sets the points to project. These points are expected to be world coordinate points along the
  /// curve that was specified using SetInputCurveNode.
  ///
  /// They are meant to be either the curve points generated by the CurveGenerator, or the generated resampled
  /// control points.
  ///
  /// \sa SetInputConnection, SetInputData
  using vtkPolyDataAlgorithm::SetInputData;
  using vtkPolyDataAlgorithm::SetInputConnection;
  ///@}

  /// Constrain points to a specified model surface.
  ///
  /// \param originalPoints The points to constrain.
  /// \param normalVectors The normals for the originalPoints.
  /// \param surfacePolydata The surface to constrain to.
  /// \param[out] surfacePoints The points data structure to put the constrained points in.
  /// \param maximumSearchRadiusTolerance Maximum distance a point can be constrained, specified as a percentage
  ///     of the model's bounding box diagonal in world coordinate system. Valid range for this is 0 to 1.
  /// \return true if successful, false otherwise.
  ///
  /// \sa vtkDMMLMarkupsCurveNode::ConstrainPointsToSurface
  static bool ConstrainPointsToSurface(vtkPoints* originalPoints, vtkDoubleArray* normalVectors, vtkPolyData* surfacePolydata,
    vtkPoints* surfacePoints, double maximumSearchRadiusTolerance);

  ///@{
  /// Set/Get maximumSearchRadiusTolerance defining the allowable projection distance.
  ///
  /// It is specified as a percentage of the model's bounding box diagonal in world coordinate system.
  ///
  /// \sa ConstrainPointsToSurface()
  /// \sa vtkBoundingBox::GetDiagonalLength()
  void SetMaximumSearchRadiusTolerance(double maximumSearchRadiusTolerance);
  double GetMaximumSearchRadiusTolerance() const;
  ///@}

protected:
  int FillInputPortInformation(int port, vtkInformation* info) override;
  int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector) override;

  vtkProjectMarkupsCurvePointsFilter();
private:
  vtkWeakPointer<vtkDMMLMarkupsCurveNode> InputCurveNode;
  double MaximumSearchRadiusTolerance;

  bool ProjectPointsToSurface(vtkDMMLModelNode* modelNode, double maximumSearchRadiusTolerance, vtkPoints* interpolatedPoints, vtkPoints* outputPoints);
  static bool ConstrainPointsToSurfaceImpl(vtkOBBTree* surfaceObbTree, vtkPointLocator* pointLocator,
      vtkPoints* originalPoints, vtkDoubleArray* normalVectors, vtkPolyData* surfacePolydata,
      vtkPoints* surfacePoints, double maximumSearchRadius=.25);

  class PointProjectionHelper
  {
  public:
    PointProjectionHelper();
    void SetModel(vtkDMMLModelNode* model);
    /// Gets the point normals on the model at the points with the given controlPoints.
    /// Both points and control points must have no outstanding transformations.
    vtkSmartPointer<vtkDoubleArray> GetPointNormals(vtkPoints* points, vtkPoints* controlPoints);
    vtkPointLocator* GetPointLocator();
    vtkOBBTree* GetObbTree();
    vtkPolyData* GetSurfacePolyData();

  private:
    vtkDMMLModelNode* Model;
    vtkMTimeType LastModelModifiedTime;
    vtkMTimeType LastTransformModifiedTime;
    vtkSmartPointer<vtkDataArray> ModelNormalVectorArray;
    vtkSmartPointer<vtkPointLocator> ModelPointLocator;
    vtkSmartPointer<vtkOBBTree> ModelObbTree;
    vtkSmartPointer<vtkPolyData> SurfacePolyData;

    bool UpdateAll();
    static vtkIdType GetClosestControlPointIndex(const double point[3], vtkPoints* controlPoints);
  };

  PointProjectionHelper PointProjection;
};

#endif
