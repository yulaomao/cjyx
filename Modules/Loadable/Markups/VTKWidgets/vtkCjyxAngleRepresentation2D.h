/*=========================================================================

 Copyright (c) ProxSim ltd., Kwun Tong, Hong Kong. All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 This file was originally developed by Davide Punzo, punzodavide@hotmail.it,
 and development was supported by ProxSim ltd.

=========================================================================*/

/**
 * @class   vtkCjyxAngleRepresentation2D
 * @brief   Default representation for the line widget
 *
 * This class provides the default concrete representation for the
 * vtkDMMLAbstractWidget. See vtkDMMLAbstractWidget
 * for details.
 * @sa
 * vtkCjyxMarkupsWidgetRepresentation2D vtkDMMLAbstractWidget
*/

#ifndef vtkCjyxAngleRepresentation2D_h
#define vtkCjyxAngleRepresentation2D_h

#include "vtkCjyxMarkupsModuleVTKWidgetsExport.h"
#include "vtkCjyxMarkupsWidgetRepresentation2D.h"

#include "vtkLookupTable.h"

class vtkArcSource;
class vtkDiscretizableColorTransferFunction;
class vtkSampleImplicitFunctionFilter;
class vtkTubeFilter;

class vtkDMMLInteractionEventData;

class VTK_CJYX_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkCjyxAngleRepresentation2D : public vtkCjyxMarkupsWidgetRepresentation2D
{
public:
  /// Instantiate this class.
  static vtkCjyxAngleRepresentation2D *New();

  /// Standard methods for instances of this class.
  vtkTypeMacro(vtkCjyxAngleRepresentation2D,vtkCjyxMarkupsWidgetRepresentation2D);
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

  bool GetTransformationReferencePoint(double referencePointWorld[3]) override;

protected:
  vtkCjyxAngleRepresentation2D();
  ~vtkCjyxAngleRepresentation2D() override;

  void SetMarkupsNode(vtkDMMLMarkupsNode *markupsNode) override;

  void BuildArc();

  // Update visibility of interaction handles for representation
  void UpdateInteractionPipeline() override;

  vtkSmartPointer<vtkPolyData> Line;
  vtkSmartPointer<vtkPolyDataMapper2D> LineMapper;
  vtkSmartPointer<vtkActor2D> LineActor;
  vtkSmartPointer<vtkArcSource> Arc;
  vtkSmartPointer<vtkPolyDataMapper2D> ArcMapper;
  vtkSmartPointer<vtkActor2D> ArcActor;
  vtkSmartPointer<vtkDiscretizableColorTransferFunction> ColorMap;

  vtkSmartPointer<vtkTubeFilter> TubeFilter;
  vtkSmartPointer<vtkTubeFilter> ArcTubeFilter;

  vtkSmartPointer<vtkTransformPolyDataFilter> LineWorldToSliceTransformer;
  vtkSmartPointer<vtkTransformPolyDataFilter> ArcWorldToSliceTransformer;

  vtkSmartPointer<vtkSampleImplicitFunctionFilter> LineSliceDistance;
  vtkSmartPointer<vtkSampleImplicitFunctionFilter> ArcSliceDistance;

private:
  vtkCjyxAngleRepresentation2D(const vtkCjyxAngleRepresentation2D&) = delete;
  void operator=(const vtkCjyxAngleRepresentation2D&) = delete;
};

#endif
