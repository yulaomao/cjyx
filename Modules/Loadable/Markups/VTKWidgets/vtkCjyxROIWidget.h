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
 * @class   vtkCjyxROIWidget
 * @brief   Create an ROI representation
 *
 * The vtkCjyxROIWidget is used to create an ROI widget.
 *
*/

#ifndef vtkCjyxROIWidget_h
#define vtkCjyxROIWidget_h

#include "vtkCjyxMarkupsModuleVTKWidgetsExport.h"
#include "vtkCjyxMarkupsWidget.h"

class VTK_CJYX_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkCjyxROIWidget : public vtkCjyxMarkupsWidget
{
public:
  /// Instantiate this class.
  static vtkCjyxROIWidget *New();

  /// Standard methods for a VTK class.
  vtkTypeMacro(vtkCjyxROIWidget,vtkCjyxMarkupsWidget);

  /// Widget states
  enum
  {
    WidgetStateSymmetricScale = WidgetStateMarkups_Last,
    WidgetStateMarkupsROI_Last
  };

  // Widget events
  enum
  {
    WidgetEventSymmetricScaleStart = WidgetEventMarkups_Last,
    WidgetEventSymmetricScaleEnd,
    WidgetEventMarkupsROI_Last
  };

  /// Create the default widget representation and initializes the widget and representation.
  void CreateDefaultRepresentation(vtkDMMLMarkupsDisplayNode* markupsDisplayNode, vtkDMMLAbstractViewNode* viewNode, vtkRenderer* renderer) override;

  /// Flip the selected index across the specified axis.
  /// Ex. Switch between L--R face.
  /// Used when the user drags an ROI handle across the ROI origin.
  void FlipROIHandles(bool flipLRHandle, bool flipAPHandle, bool flipISHandle);

  /// Create instance of the markups widget
  vtkCjyxMarkupsWidgetCreateInstanceMacro(vtkCjyxROIWidget);

protected:
  vtkCjyxROIWidget();
  ~vtkCjyxROIWidget() override;

  bool CanProcessInteractionEvent(vtkDMMLInteractionEventData* eventData, double& distance2) override;
  bool ProcessInteractionEvent(vtkDMMLInteractionEventData* eventData) override;
  bool ProcessWidgetSymmetricScaleStart(vtkDMMLInteractionEventData* eventData);
  bool ProcessMouseMove(vtkDMMLInteractionEventData* eventData) override;
  bool ProcessEndMouseDrag(vtkDMMLInteractionEventData* eventData) override;

  void ScaleWidget(double eventPos[2]) override;
  void ScaleWidget(double eventPos[2], bool symmetricScale);

private:
  vtkCjyxROIWidget(const vtkCjyxROIWidget&) = delete;
  void operator=(const vtkCjyxROIWidget&) = delete;
};

#endif
