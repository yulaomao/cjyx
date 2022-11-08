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
 * @class   vtkDMMLWindowLevelWidget
 * @brief   Show slice intersection lines
 *
 * The vtkDMMLWindowLevelWidget allows moving slices by interacting with
 * displayed slice intersection lines.
 *
 *
*/

#ifndef vtkDMMLWindowLevelWidget_h
#define vtkDMMLWindowLevelWidget_h

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


class VTK_DMML_DISPLAYABLEMANAGER_EXPORT vtkDMMLWindowLevelWidget : public vtkDMMLAbstractWidget
{
public:
  /**
   * Instantiate this class.
   */
  static vtkDMMLWindowLevelWidget *New();

  enum
  {
    ModeAdjust,
    ModeRectangle,
    ModeRectangleCentered,
    Mode_Last
  };

  //@{
  /**
   * Standard VTK class macros.
   */
  vtkTypeMacro(vtkDMMLWindowLevelWidget, vtkDMMLAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  //@}

  /**
   * Create the default widget representation if one is not set.
   */
  void CreateDefaultRepresentation();

  void SetSliceNode(vtkDMMLSliceNode* sliceNode);
  vtkDMMLSliceNode* GetSliceNode();

  vtkDMMLSliceLogic* GetSliceLogic();

  vtkGetMacro(BackgroundVolumeEditable, bool);
  vtkSetMacro(BackgroundVolumeEditable, bool);
  vtkBooleanMacro(BackgroundVolumeEditable, bool);

  vtkGetMacro(ForegroundVolumeEditable, bool);
  vtkSetMacro(ForegroundVolumeEditable, bool);
  vtkBooleanMacro(ForegroundVolumeEditable, bool);

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
    WidgetStateAdjustWindowLevel = WidgetStateUser,
    /// alternative state: if current mode is region-based then in alternative state
    /// the mode is adjustment; if current mode is adjustment then alternative state is region-based
    WidgetStateAdjustWindowLevelAlternative,
    };

  /// Widget events
  enum
    {
    WidgetEventResetWindowLevel = WidgetEventUser,
    WidgetEventAdjustWindowLevelStart,
    WidgetEventAdjustWindowLevelEnd,
    WidgetEventAdjustWindowLevelCancel,
    WidgetEventAdjustWindowLevelAlternativeStart,
    WidgetEventAdjustWindowLevelAlternativeEnd,
    WidgetEventAdjustWindowLevelAlternativeCancel,
    WidgetEventAlwaysOnResetWindowLevel,
    WidgetEventAlwaysOnAdjustWindowLevelStart,
    WidgetEventAlwaysOnAdjustWindowLevelEnd,
    WidgetEventAlwaysOnAdjustWindowLevelCancel,
    WidgetEventAlwaysOnAdjustWindowLevelAlternativeStart,
    WidgetEventAlwaysOnAdjustWindowLevelAlternativeEnd,
    WidgetEventAlwaysOnAdjustWindowLevelAlternativeCancel,
    };

  bool UpdateWindowLevelFromRectangle(int layer, int cornerPoint1[2], int cornerPoint2[2]);

  static const char* GetInteractionNodeAdjustWindowLevelModeAttributeName() { return "AdjustWindowLevelMode"; };

  static const char* GetAdjustWindowLevelModeAsString(int id);
  static int GetAdjustWindowLevelModeFromString(const char* name);

protected:
  vtkDMMLWindowLevelWidget();
  ~vtkDMMLWindowLevelWidget() override;

  bool ProcessStartMouseDrag(vtkDMMLInteractionEventData* eventData);
  bool ProcessMouseMove(vtkDMMLInteractionEventData* eventData);
  bool ProcessEndMouseDrag(vtkDMMLInteractionEventData* eventData);

  bool ProcessAdjustWindowLevelStart(vtkDMMLInteractionEventData* eventData);
  void ProcessAdjustWindowLevel(vtkDMMLInteractionEventData* eventData);

  bool ProcessSetWindowLevelFromRegionStart(vtkDMMLInteractionEventData* eventData);
  void ProcessSetWindowLevelFromRegion(vtkDMMLInteractionEventData* eventData);
  // If updateWindowLevel is set to false then the operation is cancelled without changing the window/level
  bool ProcessSetWindowLevelFromRegionEnd(vtkDMMLInteractionEventData* eventData, bool updateWindowLevel=true);

  bool ProcessResetWindowLevel(vtkDMMLInteractionEventData* eventData);

  int GetEditableLayerAtEventPosition(vtkDMMLInteractionEventData* eventData);

  vtkDMMLVolumeNode* GetVolumeNodeFromSliceLayer(int editedLayer);

  bool SetVolumeWindowLevel(double window, double level, bool isAutoWindowLevel);

  /// Rubberband is centered around the click position
  vtkGetMacro(CenteredRubberBand, bool);
  vtkSetMacro(CenteredRubberBand, bool);
  vtkBooleanMacro(CenteredRubberBand, bool);

  vtkWeakPointer<vtkDMMLSliceNode> SliceNode;
  vtkWeakPointer<vtkDMMLSliceLogic> SliceLogic;

  bool CenteredRubberBand;

  int StartEventPosition[2];
  int PreviousEventPosition[2];

  double VolumeScalarRange[2];

  // Auto window/level was active before starting to adjust it
  bool IsStartVolumeAutoWindowLevel;
  double StartVolumeWindowLevel[2];
  double LastVolumeWindowLevel[2];

  int WindowLevelAdjustedLayer;

  bool BackgroundVolumeEditable;
  bool ForegroundVolumeEditable;

  int AdjustMode;

private:
  vtkDMMLWindowLevelWidget(const vtkDMMLWindowLevelWidget&) = delete;
  void operator=(const vtkDMMLWindowLevelWidget&) = delete;
};

#endif
