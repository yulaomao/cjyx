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
 * @class   vtkCjyxMarkupsWidgetRepresentation2D
 * @brief   Default representation for the cjyx markups widget
 *
 * This class provides the default concrete representation for the
 * vtkDMMLAbstractWidget. See vtkDMMLAbstractWidget
 * for details.
 * @sa
 * vtkCjyxMarkupsWidgetRepresentation2D vtkDMMLAbstractWidget
*/

#ifndef vtkCjyxMarkupsWidgetRepresentation2D_h
#define vtkCjyxMarkupsWidgetRepresentation2D_h

#include "vtkCjyxMarkupsModuleVTKWidgetsExport.h"
#include "vtkCjyxMarkupsWidgetRepresentation.h"

#include "vtkDMMLSliceNode.h"

class vtkActor2D;
class vtkDiscretizableColorTransferFunction;
class vtkGlyph2D;
class vtkLabelPlacementMapper;
class vtkMarkupsGlyphSource2D;
class vtkPlane;
class vtkPolyDataMapper2D;
class vtkProperty2D;

class vtkDMMLInteractionEventData;

class VTK_CJYX_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkCjyxMarkupsWidgetRepresentation2D : public vtkCjyxMarkupsWidgetRepresentation
{
public:
  /// Standard methods for instances of this class.
  vtkTypeMacro(vtkCjyxMarkupsWidgetRepresentation2D, vtkCjyxMarkupsWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Position is displayed (slice) position
  void CanInteract(vtkDMMLInteractionEventData* interactionEventData,
    int &foundComponentType, int &foundComponentIndex, double &closestDistance2) override;

  /// Check if interaction with the transformation handles is possible
  virtual void CanInteractWithHandles(vtkDMMLInteractionEventData* interactionEventData,
    int& foundComponentType, int& foundComponentIndex, double& closestDistance2);

  /// Checks if interaction with straight line between visible points is possible.
  /// Can be used on the output of CanInteract, as if no better component is found then the input is returned.
  void CanInteractWithLine(vtkDMMLInteractionEventData* interactionEventData,
    int &foundComponentType, int &foundComponentIndex, double &closestDistance2);

  /// Subclasses of vtkCjyxMarkupsWidgetRepresentation2D must implement these methods. These
  /// are the methods that the widget and its representation use to
  /// communicate with each other.
  void UpdateFromDMML(vtkDMMLNode* caller, unsigned long event, void *callData=nullptr) override;

  /// Methods to make this class behave as a vtkProp.
  void GetActors(vtkPropCollection *) override;
  void ReleaseGraphicsResources(vtkWindow *) override;
  int RenderOverlay(vtkViewport *viewport) override;
  int RenderOpaqueGeometry(vtkViewport *viewport) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport *viewport) override;
  vtkTypeBool HasTranslucentPolygonalGeometry() override;

  /// Get the nth node's position on the slice. Will return
  /// 1 on success, or 0 if there are not at least
  /// (n+1) nodes (0 based counting).
  int GetNthControlPointDisplayPosition(int n, double pos[2]) override;

  /// Set the Nth node visibility in this view (markup visibility is enabled
  /// and markup is on current slice).
  /// Useful for non-regression tests that need to inspect internal state of the widget.
  virtual bool GetNthControlPointViewVisibility(int n);

  /// Set the Nth node slice visibility (i.e. if it is on the slice).
  virtual void SetNthControlPointSliceVisibility(int n, bool visibility);

  /// Set the center slice visibility (i.e. if it is on the slice).
  virtual void SetCenterSliceVisibility(bool visibility);

  void GetSliceToWorldCoordinates(const double[2], double[3]);
  void GetWorldToSliceCoordinates(const double worldPos[3], double slicePos[2]);

  void UpdateInteractionPipeline() override;

protected:
  vtkCjyxMarkupsWidgetRepresentation2D();
  ~vtkCjyxMarkupsWidgetRepresentation2D() override;

  /// Reimplemented for 2D specific mapper/actor settings
  void SetupInteractionPipeline() override;

    /// Get DMML view node as slice view node
  vtkDMMLSliceNode *GetSliceNode();

  void UpdatePlaneFromSliceNode();

  void UpdateViewScaleFactor() override;
  void UpdateControlPointSize() override;

  // Return squared distance of maximum distance for picking a control point,
  // in pixels.
  double GetMaximumControlPointPickingDistance2();
  // Return squared distance of maximum distance for picking an interaction handle,
  // in pixels.
  double GetMaximumInteractionHandlePickingDistance2();

  bool GetAllControlPointsVisible() override;

  /// Check, if the point is displayable in the current slice geometry
  virtual bool IsControlPointDisplayableOnSlice(vtkDMMLMarkupsNode* node, int pointIndex = 0);

  // Update colormap based on provided base color (modulated with settings stored in the display node)
  virtual void UpdateDistanceColorMap(vtkDiscretizableColorTransferFunction* colormap, double color[3]);

  /// Check, if the point is behind in the current slice geometry
  virtual bool IsPointBehindSlice(vtkDMMLMarkupsNode* node, int pointIndex = 0);

  /// Check, if the point is in front in the current slice geometry
  virtual bool IsPointInFrontSlice(vtkDMMLMarkupsNode* node, int pointIndex = 0);

  /// Check, if the point is displayable in the current slice geometry
  virtual bool IsCenterDisplayableOnSlice(vtkDMMLMarkupsNode* node);

  /// Convert display to world coordinates
  void GetWorldToDisplayCoordinates(double r, double a, double s, double * displayCoordinates);
  void GetWorldToDisplayCoordinates(double * worldCoordinates, double * displayCoordinates);

  /// Check if the representation polydata intersects the slice
  bool IsRepresentationIntersectingSlice(vtkPolyData* representation, const char* arrayName);

  class ControlPointsPipeline2D : public ControlPointsPipeline
  {
  public:
    ControlPointsPipeline2D();
    ~ControlPointsPipeline2D() override;

    vtkSmartPointer<vtkActor2D> Actor;
    vtkSmartPointer<vtkPolyDataMapper2D> Mapper;
    vtkSmartPointer<vtkGlyph2D> Glypher;
    vtkSmartPointer<vtkActor2D> LabelsActor;
    vtkSmartPointer<vtkLabelPlacementMapper> LabelsMapper;
    // Properties used to control the appearance of selected objects and
    // the manipulator in general.
    vtkSmartPointer<vtkProperty2D> Property;
  };

  ControlPointsPipeline2D* GetControlPointsPipeline(int controlPointType);

  vtkSmartPointer<vtkIntArray> PointsVisibilityOnSlice;
  bool                         CenterVisibilityOnSlice = { false };
  bool                         AnyPointVisibilityOnSlice = { false };  // at least one point is visible

  vtkSmartPointer<vtkTransform> WorldToSliceTransform;
  vtkSmartPointer<vtkPlane> SlicePlane;

  virtual void UpdateAllPointsAndLabelsFromDMML(double labelsOffset);

  double GetWidgetOpacity(int controlPointType);

  class MarkupsInteractionPipeline2D : public MarkupsInteractionPipeline
  {
  public:
    MarkupsInteractionPipeline2D(vtkCjyxMarkupsWidgetRepresentation* representation);
    ~MarkupsInteractionPipeline2D() override = default;;

    void GetViewPlaneNormal(double viewPlaneNormal[3]) override;

    vtkSmartPointer<vtkTransformPolyDataFilter> WorldToSliceTransformFilter;
  };

private:
  vtkCjyxMarkupsWidgetRepresentation2D(const vtkCjyxMarkupsWidgetRepresentation2D&) = delete;
  void operator=(const vtkCjyxMarkupsWidgetRepresentation2D&) = delete;
};

#endif
