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

/**
 * @class   vtkDMMLCameraWidget
 * @brief   Process camera manipulation events.
 *
 * This widget does not have a visible representation, only translates
 * and processes camera manipulation (rotate, translate, etc.) events.
 * It is implemented as a widget instead of an interactor style so that
 * mouse and keyboard interaction events can be dynamically remapped
 * to camera manipulation actions and to process all kinds of interaction
 * events (camera manipulation, markups manipulation, ...) in a similar way.
*/

#ifndef vtkDMMLCameraWidget_h
#define vtkDMMLCameraWidget_h

#include "vtkDMMLDisplayableManagerExport.h" // For export macro
#include "vtkDMMLAbstractWidget.h"
#include "vtkDMMLCameraNode.h"

class vtkSliceIntersectionRepresentation2D;
class vtkDMMLSegmentationDisplayNode;


class VTK_DMML_DISPLAYABLEMANAGER_EXPORT vtkDMMLCameraWidget : public vtkDMMLAbstractWidget
{
public:
  /**
   * Instantiate this class.
   */
  static vtkDMMLCameraWidget *New();

  //@{
  /**
   * Standard VTK class macros.
   */
  vtkTypeMacro(vtkDMMLCameraWidget, vtkDMMLAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  //@}

  /**
   * Create the default widget representation if one is not set.
   */
  void CreateDefaultRepresentation();

  void SetCameraNode(vtkDMMLCameraNode* cameraNode);
  vtkDMMLCameraNode* GetCameraNode();

  /// Return true if the widget can process the event.
  bool CanProcessInteractionEvent(vtkDMMLInteractionEventData* eventData, double &distance2) override;

  /// Process interaction event.
  bool ProcessInteractionEvent(vtkDMMLInteractionEventData* eventData) override;

  //@{
  /// Get set tilt lock. It tilt is locked then the view cannot be rotated around the horizontal axis
  /// (can only be rotated along the vertical axis).
  bool GetTiltLocked();
  void SetTiltLocked(bool lockState);
  //@}

  /// Widget states
  enum
    {
    WidgetStateMoveCrosshair = WidgetStateUser, ///< Move crosshair position, can be used for moving the crosshair with click-and-drag.
    WidgetStateSpin,
    WidgetStateTouchGesture
    };

  /// Widget events
  enum
    {
    WidgetEventSpinStart = WidgetEventUser,
    WidgetEventSpinEnd,

    WidgetEventMoveCrosshairStart,
    WidgetEventMoveCrosshairEnd,

    WidgetEventCameraRotateToRight,
    WidgetEventCameraRotateToLeft,
    WidgetEventCameraRotateToAnterior,
    WidgetEventCameraRotateToPosterior,
    WidgetEventCameraRotateToSuperior,
    WidgetEventCameraRotateToInferior,

    WidgetEventCameraTranslateForwardX,
    WidgetEventCameraTranslateBackwardX,
    WidgetEventCameraTranslateForwardY,
    WidgetEventCameraTranslateBackwardY,
    WidgetEventCameraTranslateForwardZ,
    WidgetEventCameraTranslateBackwardZ,

    WidgetEventCameraRotateCcwX,
    WidgetEventCameraRotateCwX,
    WidgetEventCameraRotateCcwY,
    WidgetEventCameraRotateCwY,
    WidgetEventCameraRotateCcwZ,
    WidgetEventCameraRotateCwZ,

    WidgetEventCameraZoomIn,
    WidgetEventCameraZoomOut,
    WidgetEventCameraWheelZoomIn, // same as WidgetEventCameraZoomIn but with using wheel scaling factor
    WidgetEventCameraWheelZoomOut,

    WidgetEventToggleCameraTiltLock,

    WidgetEventCameraReset,
    WidgetEventCameraResetTranslation,
    WidgetEventCameraResetRotation,
    WidgetEventCameraResetFieldOfView, // VTK's standard camera reset (centers and resets field of view)

    WidgetEventCameraRotate,
    WidgetEventCameraPan,

    WidgetEventTouchGestureStart,
    WidgetEventTouchGestureEnd,
    WidgetEventTouchSpinCamera,
    WidgetEventTouchPinchZoom,
    WidgetEventTouchPanTranslate,

    WidgetEventSetCrosshairPosition,
    WidgetEventSetCrosshairPositionBackground, //< set crosshair position without consuming the event (so that other widgets can process the event)
    WidgetEventMaximizeView,
    };

  /// Defines speed of rotation actions by mouse click-and-drag.
  vtkGetMacro(MotionFactor, double);
  vtkSetMacro(MotionFactor, double);

  /// Defines step size for mouse wheel actions.
  vtkGetMacro(MouseWheelMotionFactor, double);
  vtkSetMacro(MouseWheelMotionFactor, double);

protected:
  vtkDMMLCameraWidget();
  ~vtkDMMLCameraWidget() override;

  bool ProcessStartMouseDrag(vtkDMMLInteractionEventData* eventData);
  bool ProcessMouseMove(vtkDMMLInteractionEventData* eventData);
  bool ProcessEndMouseDrag(vtkDMMLInteractionEventData* eventData);

  bool ProcessTranslate(vtkDMMLInteractionEventData* eventData);
  bool ProcessRotate(vtkDMMLInteractionEventData* eventData);
  bool ProcessScale(vtkDMMLInteractionEventData* eventData);
  bool ProcessSpin(vtkDMMLInteractionEventData* eventData);
  bool ProcessSetCrosshair(vtkDMMLInteractionEventData* eventData);
  bool ProcessSetCrosshairBackground(vtkDMMLInteractionEventData* eventData);

  bool ProcessTouchGestureStart(vtkDMMLInteractionEventData* eventData);
  bool ProcessTouchGestureEnd(vtkDMMLInteractionEventData* eventData);
  bool ProcessTouchCameraSpin(vtkDMMLInteractionEventData* eventData);
  bool ProcessTouchCameraZoom(vtkDMMLInteractionEventData* eventData);
  bool ProcessTouchCameraTranslate(vtkDMMLInteractionEventData* eventData);

  bool ProcessWidgetMenu(vtkDMMLInteractionEventData* eventData);

  bool ProcessMaximizeView(vtkDMMLInteractionEventData* eventData);

  bool Dolly(double factor);
  vtkCamera* GetCamera();

  bool CameraModifyStart();
  void CameraModifyEnd(bool wasModified, bool updateClippingRange, bool updateLights);

  void SaveStateForUndo();

  double MotionFactor;
  double MouseWheelMotionFactor;
  bool CameraTiltLocked;

  vtkWeakPointer<vtkDMMLCameraNode> CameraNode;

  double StartEventPosition[2];
  int PreviousEventPosition[2];

  /// Indicates whether the shift key was used during the previous action.
  /// This is used to require shift-up after a click-and-drag before accepting shift+mousemove.
  bool ModifierKeyPressedSinceLastClickAndDrag;


private:
  vtkDMMLCameraWidget(const vtkDMMLCameraWidget&) = delete;
  void operator=(const vtkDMMLCameraWidget&) = delete;
};

#endif
