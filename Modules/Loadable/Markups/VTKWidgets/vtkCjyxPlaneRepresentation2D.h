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
 * @class   vtkCjyxPlaneRepresentation2D
 * @brief   Default representation for the plane widget
 *
 * This class provides the default concrete representation for the
 * vtkDMMLAbstractWidget. See vtkDMMLAbstractWidget
 * for details.
 * @sa
 * vtkCjyxMarkupsWidgetRepresentation2D vtkDMMLAbstractWidget
*/

#ifndef vtkCjyxPlaneRepresentation2D_h
#define vtkCjyxPlaneRepresentation2D_h

#include "vtkCjyxMarkupsModuleVTKWidgetsExport.h"
#include "vtkCjyxMarkupsWidgetRepresentation2D.h"
#include "vtkCjyxPlaneRepresentation3D.h"
#include "vtkGlyphSource2D.h"

class vtkAppendPolyData;
class vtkClipPolyData;
class vtkCompositeDataGeometryFilter;
class vtkDiscretizableColorTransferFunction;
class vtkFeatureEdges;
class vtkDMMLInteractionEventData;
class vtkPlaneCutter;
class vtkPlaneSource;
class vtkSampleImplicitFunctionFilter;

class VTK_CJYX_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkCjyxPlaneRepresentation2D : public vtkCjyxMarkupsWidgetRepresentation2D
{
public:
  /// Instantiate this class.
  static vtkCjyxPlaneRepresentation2D *New();

  /// Standard methods for instances of this class.
  vtkTypeMacro(vtkCjyxPlaneRepresentation2D,vtkCjyxMarkupsWidgetRepresentation2D);
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
    int &foundComponentType, int &foundComponentIndex, double &closestDistance2) override;

  void CanInteractWithPlane(vtkDMMLInteractionEventData* interactionEventData,
    int& foundComponentType, int& foundComponentIndex, double& closestDistance2);

  bool GetTransformationReferencePoint(double referencePointWorld[3]) override;

  void BuildPlane();

  // Initialize interaction handle pipeline
  void SetupInteractionPipeline() override;

  // Update visibility of interaction handles for representation
  void UpdateInteractionPipeline() override;

protected:
  vtkCjyxPlaneRepresentation2D();
  ~vtkCjyxPlaneRepresentation2D() override;

  virtual void UpdatePlaneFillColorMap(vtkDiscretizableColorTransferFunction* colormap, double color[3]);
  virtual void UpdatePlaneOutlineColorMap(vtkDiscretizableColorTransferFunction* colormap, double color[3]);

  vtkNew<vtkPlaneSource> PlaneFilter;
  vtkNew<vtkPlaneCutter> PlaneCutter;

  vtkNew<vtkClipPolyData> PlaneClipperSlicePlane;
  vtkNew<vtkClipPolyData> PlaneClipperStartFadeNear;
  vtkNew<vtkClipPolyData> PlaneClipperEndFadeNear;
  vtkNew<vtkClipPolyData> PlaneClipperStartFadeFar;
  vtkNew<vtkClipPolyData> PlaneClipperEndFadeFar;

  vtkNew<vtkCompositeDataGeometryFilter> PlaneCompositeFilter;
  vtkNew<vtkAppendPolyData> PlaneAppend;
  vtkNew<vtkTransformPolyDataFilter> PlaneWorldToSliceTransformer;
  vtkNew<vtkPolyDataMapper2D> PlaneFillMapper;
  vtkNew<vtkActor2D> PlaneFillActor;

  vtkNew<vtkFeatureEdges> PlaneOutlineFilter;
  vtkNew<vtkDiscretizableColorTransferFunction> PlaneOutlineColorMap;
  vtkNew<vtkTransformPolyDataFilter> PlaneOutlineWorldToSliceTransformer;
  vtkNew<vtkPolyDataMapper2D> PlaneOutlineMapper;
  vtkNew<vtkActor2D> PlaneOutlineActor;

  vtkNew<vtkAppendPolyData> PlanePickingAppend;

  vtkNew<vtkGlyphSource2D> ArrowFilter;
  vtkNew<vtkGlyph2D> ArrowGlypher;
  vtkNew<vtkPolyDataMapper2D> ArrowMapper;
  vtkNew<vtkActor2D> ArrowActor;

  vtkNew<vtkDiscretizableColorTransferFunction> PlaneFillColorMap;
  vtkNew<vtkSampleImplicitFunctionFilter> PlaneSliceDistance;
  std::string LabelFormat;

  class MarkupsInteractionPipelinePlane2D : public vtkCjyxPlaneRepresentation3D::MarkupsInteractionPipelinePlane
  {
  public:
    MarkupsInteractionPipelinePlane2D(vtkCjyxMarkupsWidgetRepresentation* representation);
    ~MarkupsInteractionPipelinePlane2D() override = default;;

    void GetViewPlaneNormal(double viewPlaneNormal[3]) override;

    vtkSmartPointer<vtkTransformPolyDataFilter> WorldToSliceTransformFilter;
  };


private:
  vtkCjyxPlaneRepresentation2D(const vtkCjyxPlaneRepresentation2D&) = delete;
  void operator=(const vtkCjyxPlaneRepresentation2D&) = delete;
};

#endif
