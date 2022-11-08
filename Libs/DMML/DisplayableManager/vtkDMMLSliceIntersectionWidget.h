/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkDMMLSliceIntersectionWidget.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or https://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkDMMLSliceIntersectionWidget
 * @brief   Show slice intersection lines
 *
 * The vtkDMMLSliceIntersectionWidget allows moving slices by interacting with
 * displayed slice intersection lines.
 *
 *
*/

#ifndef vtkDMMLSliceIntersectionWidget_h
#define vtkDMMLSliceIntersectionWidget_h

#include "vtkDMMLDisplayableManagerExport.h" // For export macro
#include "vtkDMMLAbstractWidget.h"
#include "vtkDMMLSliceLogic.h"
#include "vtkDMMLSliceNode.h"

#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

class vtkSliceIntersectionRepresentation2D;
class vtkDMMLApplicationLogic;
class vtkDMMLSegmentationDisplayNode;
class vtkDMMLSliceDisplayNode;

class VTK_DMML_DISPLAYABLEMANAGER_EXPORT vtkDMMLSliceIntersectionWidget : public vtkDMMLAbstractWidget
{
public:
  /**
   * Instantiate this class.
   */
  static vtkDMMLSliceIntersectionWidget *New();

  //@{
  /**
   * Standard VTK class macros.
   */
  vtkTypeMacro(vtkDMMLSliceIntersectionWidget, vtkDMMLAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  //@}

  /**
   * Create the default widget representation if one is not set.
   */
  void CreateDefaultRepresentation();

  void SetSliceNode(vtkDMMLSliceNode* sliceNode);
  vtkDMMLSliceNode* GetSliceNode();

  vtkDMMLSliceDisplayNode* GetSliceDisplayNode();

  void SetDMMLApplicationLogic(vtkDMMLApplicationLogic* applicationLogic) override;

  /// Return true if the widget can process the event.
  bool CanProcessInteractionEvent(vtkDMMLInteractionEventData* eventData, double &distance2) override;

  /// Process interaction event.
  bool ProcessInteractionEvent(vtkDMMLInteractionEventData* eventData) override;

  /// Called when the the widget loses the focus.
  void Leave(vtkDMMLInteractionEventData* eventData) override;

  /// Widget states
  enum
    {
    // Interactions without handles
    WidgetStateMoveCrosshair = WidgetStateUser, ///< Move crosshair position, can be used for moving the crosshair with click-and-drag.
    WidgetStateBlend, ///< Fade between foreground/background volumes
    WidgetStateTranslateSlice, ///< Pan (translate in-plane) the current slice (using shift+left-click-and-drag or middle-click-and-drag)
    WidgetStateRotateIntersectingSlices, ///< Rotate all intersecting slices (ctrl+alt+left-click-and-drag)
    WidgetStateZoomSlice, ///< Zoom slice (using right-button or mouse wheel)
    WidgetStateTouchGesture, ///< Pinch/zoom/pan using touch gestures

    // Interactions with slice intersection handles
    WidgetStateOnTranslateIntersectingSlicesHandle, ///< hovering over a slice intersection point
    WidgetStateTranslateIntersectingSlicesHandle, ///< translating all intersecting slices by drag-and-dropping handle
    WidgetStateOnRotateIntersectingSlicesHandle, ///< hovering over a rotation interaction handle
    WidgetStateRotateIntersectingSlicesHandle, ///< rotating all intersecting slices by drag-and-dropping handle
    WidgetStateOnTranslateSingleIntersectingSliceHandle, ///< hovering over a single-slice translation interaction handle
    WidgetStateTranslateSingleIntersectingSliceHandle, ///< translating a single slice by drag-and-dropping handle

    WidgetState_Last
    };

  /// Widget events
  enum
    {
    // Interactions without handles
    WidgetEventTouchGestureStart = WidgetEventUser,
    WidgetEventTouchGestureEnd,
    WidgetEventTouchRotateSliceIntersection,
    WidgetEventTouchZoomSlice,
    WidgetEventTouchTranslateSlice,
    WidgetEventMoveCrosshairStart,
    WidgetEventMoveCrosshairEnd,
    WidgetEventBlendStart,
    WidgetEventBlendEnd,
    WidgetEventToggleLabelOpacity,
    WidgetEventToggleForegroundOpacity,
    WidgetEventIncrementSlice,
    WidgetEventDecrementSlice,
    WidgetEventZoomInSlice,
    WidgetEventZoomOutSlice,
    WidgetEventToggleSliceVisibility,
    WidgetEventToggleAllSlicesVisibility, // currently does not work, only toggles current slice
    WidgetEventResetFieldOfView,
    WidgetEventShowNextBackgroundVolume,
    WidgetEventShowPreviousBackgroundVolume,
    WidgetEventShowNextForegroundVolume,
    WidgetEventShowPreviousForegroundVolume,
    WidgetEventRotateIntersectingSlicesStart, // rotate all intersecting slices (ctrl-alt-left-click-and-drag)
    WidgetEventRotateIntersectingSlicesEnd,
    WidgetEventTranslateSliceStart,
    WidgetEventTranslateSliceEnd,
    WidgetEventZoomSliceStart,
    WidgetEventZoomSliceEnd,
    WidgetEventSetCrosshairPosition,
    WidgetEventSetCrosshairPositionBackground, //< set crosshair position without consuming the event (so that other widgets can process the event)
    WidgetEventMaximizeView,

    // Interactions with slice intersection handles
    // WidgetStateOnTranslateIntersectingSlicesHandle/WidgetStateTranslateIntersectingSlicesHandle
    WidgetEventTranslateIntersectingSlicesHandleStart,
    WidgetEventTranslateIntersectingSlicesHandleEnd,
    // WidgetStateOnRotateIntersectingSlicesHandle/WidgetStateRotateIntersectingSlicesHandle
    WidgetEventRotateIntersectingSlicesHandleStart,
    WidgetEventRotateIntersectingSlicesHandleEnd,
    // WidgetStateOnTranslateSingleIntersectingSliceHandle/WidgetStateOnTranslateSingleIntersectingSliceHandle
    WidgetEventTranslateSingleIntersectingSliceHandleStart,
    WidgetEventTranslateSingleIntersectingSliceHandleEnd,
    };

  /// Action State values and management
  enum
    {
    ActionNone = 0,
    ActionTranslate = 1,
    ActionZoom = 2,
    ActionRotate = 4, /* not used */
    ActionBlend = 8, /* fg to bg, labelmap to bg */
    ActionBrowseSlice = 64,
    ActionShowSlice = 128,
    ActionAdjustLightbox = 256,
    ActionSelectVolume = 512,
    ActionSetCursorPosition = 1024, /* adjust cursor position in crosshair node as mouse is moved */
    ActionSetCrosshairPosition = 2048, /* adjust cursor position in crosshair node as mouse is moved */
    ActionTranslateSliceIntersection = 4096,
    ActionRotateSliceIntersection = 8192,
    ActionAll = ActionTranslate | ActionZoom | ActionRotate | ActionBlend
    | ActionBrowseSlice | ActionShowSlice | ActionAdjustLightbox | ActionSelectVolume
    | ActionSetCursorPosition | ActionSetCrosshairPosition
    | ActionTranslateSliceIntersection | ActionRotateSliceIntersection
    };

  /// Set exact list of actions to enable.
  void SetActionsEnabled(int actions);

  /// Set full list of enabled actions.
  int GetActionsEnabled();

  /// Enable/disable the specified action (Translate, Zoom, Blend, etc.).
  /// Multiple actions can be specified by providing the sum of action ids.
  /// Set the value to AllActionsMask to enable/disable all actions.
  /// All actions are enabled by default.
  void SetActionEnabled(int actionsMask, bool enable = true);
  /// Returns true if the specified action is allowed.
  /// If multiple actions are specified, the return value is true if all actions are enabled.
  bool GetActionEnabled(int actionsMask);

  void UpdateInteractionEventMapping();

  // Allows the widget to request a cursor shape
  int GetMouseCursor() override;

protected:
  vtkDMMLSliceIntersectionWidget();
  ~vtkDMMLSliceIntersectionWidget() override;

  bool ProcessStartMouseDrag(vtkDMMLInteractionEventData* eventData);
  bool ProcessMouseMove(vtkDMMLInteractionEventData* eventData);
  bool ProcessEndMouseDrag(vtkDMMLInteractionEventData* eventData);
  bool ProcessBlend(vtkDMMLInteractionEventData* eventData);

  bool ProcessRotateIntersectingSlicesStart(vtkDMMLInteractionEventData* eventData);
  bool ProcessRotateIntersectingSlices(vtkDMMLInteractionEventData* eventData);
  bool ProcessSetCrosshair(vtkDMMLInteractionEventData* eventData);
  bool ProcessSetCrosshairBackground(vtkDMMLInteractionEventData* eventData);
  double GetSliceRotationAngleRad(double eventPos[2]);

  // Pan (move slice in-plane) by click-and-drag
  bool ProcessTranslateSlice(vtkDMMLInteractionEventData* eventData);

  bool ProcessZoomSlice(vtkDMMLInteractionEventData* eventData);

  bool ProcessTouchGestureStart(vtkDMMLInteractionEventData* eventData);
  bool ProcessTouchGestureEnd(vtkDMMLInteractionEventData* eventData);
  bool ProcessTouchRotate(vtkDMMLInteractionEventData* eventData);
  bool ProcessTouchZoom(vtkDMMLInteractionEventData* eventData);
  bool ProcessTouchTranslate(vtkDMMLInteractionEventData* eventData);

  bool ProcessTranslateIntersectingSlicesHandleStart(vtkDMMLInteractionEventData* eventData);
  bool ProcessTranslateIntersectingSlicesHandle(vtkDMMLInteractionEventData* eventData);
  bool ProcessTranslateSingleIntersectingSliceHandleStart(vtkDMMLInteractionEventData* eventData);
  bool ProcessTranslateSingleIntersectingSliceHandle(vtkDMMLInteractionEventData* eventData);
  bool ProcessRotateIntersectingSlicesHandleStart(vtkDMMLInteractionEventData* eventData);
  bool ProcessRotateIntersectingSlicesHandle(vtkDMMLInteractionEventData* eventData);

  bool ProcessWidgetMenu(vtkDMMLInteractionEventData* eventData);

  bool ProcessMaximizeView(vtkDMMLInteractionEventData* eventData);

  /// Rotate the message by the specified amount. Used for touchpad events.
  bool Rotate(double sliceRotationAngleRad);

  /// Adjust zoom factor. If zoomScaleFactor>1 then view is zoomed in,
  /// if 0<zoomScaleFactor<1 then view is zoomed out.
  /// Event position is used as center for zoom operation.
  void ScaleZoom(double zoomScaleFactor, vtkDMMLInteractionEventData* eventData);

  /// Get/Set labelmap or segmentation opacity
  void SetLabelOpacity(double opacity);
  double GetLabelOpacity();

  /// Returns true if mouse is inside the selected layer volume.
  /// Use background flag to choose between foreground/background layer.
  bool IsEventInsideVolume(bool background, vtkDMMLInteractionEventData* eventData);

  vtkDMMLSegmentationDisplayNode* GetVisibleSegmentationDisplayNode();

  static void SliceModifiedCallback(vtkObject* caller, unsigned long eid, void* clientData, void* callData);
  static void SliceLogicsModifiedCallback(vtkObject* caller, unsigned long eid, void* clientData, void* callData);

  vtkWeakPointer<vtkCollection> SliceLogics;
  vtkWeakPointer<vtkDMMLSliceNode> SliceNode;
  vtkWeakPointer<vtkDMMLSliceLogic> SliceLogic;
  vtkNew<vtkCallbackCommand> SliceLogicsModifiedCommand;
  vtkNew<vtkCallbackCommand> SliceModifiedCommand;

  double StartEventPosition[2];
  double PreviousRotationAngleRad;
  int PreviousEventPosition[2];
  double StartRotationCenter[2];
  double StartRotationCenter_RAS[4];

  double StartTranslationPoint[2];
  double StartTranslationPoint_RAS[3];
  double CurrentTranslationPoint_RAS[3];

  double StartActionFOV[3];
  double VolumeScalarRange[2];

  enum
    {
    LayerBackground,
    LayerForeground,
    LayerLabelmap
    };

  // Blend
  double LastForegroundOpacity;
  double LastLabelOpacity;
  vtkDMMLSegmentationDisplayNode* StartActionSegmentationDisplayNode;

  // Browse slice
  /// check for prescribed spacing, otherwise return best spacing amount
  /// for current layer setup (use logic to look for spacing of first non-null
  /// layer)
  double GetSliceSpacing();
  /// Adjust the slice position with respect to current slice node offset
  void IncrementSlice();
  void DecrementSlice();
  void MoveSlice(double delta);

  ///
  /// Change the displayed volume in the selected layer by moving
  /// in a loop through the volumes available in the scene.
  ///  - layer: are 0,1,2 for bg, fg, lb
  ///  - direction: positive or negative (wraps through volumes in scene)
  void CycleVolumeLayer(int layer, int direction);

  bool IsSliceIntersectionInteractive();

  /// Indicates whether the shift key was used during the previous action.
  /// This is used to require shift-up after a click-and-drag before accepting shift+mousemove.
  bool ModifierKeyPressedSinceLastClickAndDrag;

  int ActionsEnabled;

  double TouchRotationThreshold;
  double TouchTranslationThreshold;
  double TouchZoomThreshold;
  double TotalTouchRotation;
  bool TouchRotateEnabled;
  double TotalTouchTranslation;
  bool TouchTranslationEnabled;
  double TotalTouchZoom;
  bool TouchZoomEnabled;

  // Last intersecting slice node where interaction occurred
  std::string LastIntersectingSliceNodeID;

private:
  vtkDMMLSliceIntersectionWidget(const vtkDMMLSliceIntersectionWidget&) = delete;
  void operator=(const vtkDMMLSliceIntersectionWidget&) = delete;
};

#endif
