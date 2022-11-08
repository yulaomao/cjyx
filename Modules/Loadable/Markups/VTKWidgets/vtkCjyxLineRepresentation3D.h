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
 * @class   vtkCjyxLineRepresentation3D
 * @brief   Default representation for the line widget
 *
 * This class provides the default concrete representation for the
 * vtkDMMLAbstractWidget. See vtkDMMLAbstractWidget
 * for details.
 * @sa
 * vtkCjyxMarkupsWidgetRepresentation3D vtkDMMLAbstractWidget
*/

#ifndef vtkCjyxLineRepresentation3D_h
#define vtkCjyxLineRepresentation3D_h

#include "vtkCjyxMarkupsModuleVTKWidgetsExport.h"
#include "vtkCjyxMarkupsWidgetRepresentation3D.h"

class vtkActor;
class vtkPolyDataMapper;
class vtkPolyData;
class vtkTubeFilter;

class vtkDMMLInteractionEventData;

class VTK_CJYX_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkCjyxLineRepresentation3D : public vtkCjyxMarkupsWidgetRepresentation3D
{
public:
  /// Instantiate this class.
  static vtkCjyxLineRepresentation3D *New();

  /// Standard methods for instances of this class.
  vtkTypeMacro(vtkCjyxLineRepresentation3D,vtkCjyxMarkupsWidgetRepresentation3D);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Subclasses of vtkDMMLAbstractWidgetRepresentation must implement these methods. These
  /// are the methods that the widget and its representation use to
  /// communicate with each other.
  void UpdateFromDMML(vtkDMMLNode* caller, unsigned long event, void *callData=nullptr) override;

  void CanInteract(vtkDMMLInteractionEventData* interactionEventData,
    int &foundComponentType, int &foundComponentIndex, double &closestDistance2) override;

  /// Methods to make this class behave as a vtkProp.
  void GetActors(vtkPropCollection *) override;
  void ReleaseGraphicsResources(vtkWindow *) override;
  int RenderOverlay(vtkViewport *viewport) override;
  int RenderOpaqueGeometry(vtkViewport *viewport) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport *viewport) override;
  vtkTypeBool HasTranslucentPolygonalGeometry() override;

  /// Return the bounds of the representation
  double *GetBounds() override;

protected:
  vtkCjyxLineRepresentation3D();
  ~vtkCjyxLineRepresentation3D() override;

  /// Update interaction handle visibility for representation
  void UpdateInteractionPipeline() override;

  vtkSmartPointer<vtkPolyData> Line;
  vtkSmartPointer<vtkTubeFilter> TubeFilter;

  vtkSmartPointer<vtkPolyDataMapper> LineMapper;
  vtkSmartPointer<vtkPolyDataMapper> LineOccludedMapper;

  vtkSmartPointer<vtkActor> LineActor;
  vtkSmartPointer<vtkActor> LineOccludedActor;

private:
  vtkCjyxLineRepresentation3D(const vtkCjyxLineRepresentation3D&) = delete;
  void operator=(const vtkCjyxLineRepresentation3D&) = delete;
};

#endif
