/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

/**
 * @class   vtkCjyxROIRepresentation2D
 * @brief   Default representation for the plane widget
 *
 * This class provides the default concrete representation for the
 * vtkDMMLAbstractWidget. See vtkDMMLAbstractWidget
 * for details.
 * @sa
 * vtkCjyxMarkupsWidgetRepresentation2D vtkDMMLAbstractWidget
*/

#ifndef vtkCjyxROIRepresentation2D_h
#define vtkCjyxROIRepresentation2D_h

#include "vtkCjyxMarkupsModuleVTKWidgetsExport.h"
#include "vtkCjyxMarkupsWidgetRepresentation2D.h"
#include "vtkCjyxROIRepresentation3D.h"

class vtkAppendPolyData;
class vtkClipPolyData;
class vtkContourTriangulator;
class vtkCutter;
class vtkDiscretizableColorTransferFunction;
class vtkDMMLMarkupsROINode;
class vtkOutlineFilter;
class vtkSampleImplicitFunctionFilter;

class VTK_CJYX_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkCjyxROIRepresentation2D : public vtkCjyxMarkupsWidgetRepresentation2D
{
public:
  /// Instantiate this class.
  static vtkCjyxROIRepresentation2D *New();

  /// Standard methods for instances of this class.
  vtkTypeMacro(vtkCjyxROIRepresentation2D,vtkCjyxMarkupsWidgetRepresentation2D);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Subclasses of vtkContourCurveRepresentation must implement these methods. These
  /// are the methods that the widget and its representation use to
  /// communicate with each other.
  void UpdateFromDMML(vtkDMMLNode* caller, unsigned long event, void *callData = nullptr) override;

  /// Methods to make this class behave as a vtkProp.
  void GetActors(vtkPropCollection *) override;
  void ReleaseGraphicsResources(vtkWindow *) override;
  int RenderOverlay(vtkViewport *viewport) override;
  int RenderOpaqueGeometry(vtkViewport *viewport) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport *viewport) override;
  vtkTypeBool HasTranslucentPolygonalGeometry() override;

  /// Return the bounds of the representation
  double *GetBounds() override;

  void CanInteract(vtkDMMLInteractionEventData* interactionEventData,
    int& foundComponentType, int& foundComponentIndex, double& closestDistance2) override;

  void CanInteractWithROI(vtkDMMLInteractionEventData* interactionEventData,
    int& foundComponentType, int& foundComponentIndex, double& closestDistance2);

  // Update visibility of interaction handles for representation
  void UpdateInteractionPipeline() override;

protected:

  /// Update the cube source filter from the ROI node.
  virtual void UpdateCubeSourceFromDMML(vtkDMMLMarkupsROINode* roiNode);

  // Initialize interaction handle pipeline
  void SetupInteractionPipeline() override;

protected:
  vtkCjyxROIRepresentation2D();
  ~vtkCjyxROIRepresentation2D() override;

  void SetROISource(vtkPolyDataAlgorithm* roiSource);

  vtkSmartPointer<vtkPolyDataAlgorithm>       ROISource;

  vtkSmartPointer<vtkPassThroughFilter>       ROIPipelineInputFilter;
  vtkSmartPointer<vtkTransform>               ROIToWorldTransform;
  vtkSmartPointer<vtkTransformPolyDataFilter> ROIToWorldTransformFilter;
  vtkSmartPointer<vtkCutter>                  ROIOutlineCutter;
  vtkSmartPointer<vtkTransformPolyDataFilter> ROIOutlineWorldToSliceTransformFilter;
  vtkSmartPointer<vtkContourTriangulator>     ROIIntersectionTriangulator;

  vtkSmartPointer<vtkPolyDataMapper2D> ROIMapper;
  vtkSmartPointer<vtkProperty2D>       ROIProperty;
  vtkSmartPointer<vtkActor2D>          ROIActor;

  vtkSmartPointer<vtkPolyDataMapper2D> ROIOutlineMapper;
  vtkSmartPointer<vtkProperty2D>       ROIOutlineProperty;
  vtkSmartPointer<vtkActor2D>          ROIOutlineActor;

  class MarkupsInteractionPipelineROI2D : public vtkCjyxROIRepresentation3D::MarkupsInteractionPipelineROI
  {
  public:
    MarkupsInteractionPipelineROI2D(vtkCjyxMarkupsWidgetRepresentation* representation);
    ~MarkupsInteractionPipelineROI2D() override = default;
    void GetViewPlaneNormal(double viewPlaneNormal[3]) override;
    void UpdateScaleHandles() override;
    void AddScaleEdgeIntersection(int pointIndex, vtkIdTypeArray* visibilityArray, vtkPoints* scaleHandleArray,
      double sliceNormal[3], double sliceOrigin[3], double edgePoint[3], double edgeVector[3]);
    vtkSmartPointer<vtkTransformPolyDataFilter> WorldToSliceTransformFilter;
  };

private:
  vtkCjyxROIRepresentation2D(const vtkCjyxROIRepresentation2D&) = delete;
  void operator=(const vtkCjyxROIRepresentation2D&) = delete;
};

#endif
