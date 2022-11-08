/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under this License.

==============================================================================*/

#ifndef __qDMMLMarkupsDisplayNodeWidget_h
#define __qDMMLMarkupsDisplayNodeWidget_h

// DMMLWidgets includes
#include "qDMMLWidget.h"
#include "qDMMLScalarsDisplayWidget.h"

// CTK includes
#include <ctkVTKObject.h>

// qDMML includes
#include "qCjyxMarkupsModuleWidgetsExport.h"

class qDMMLMarkupsDisplayNodeWidgetPrivate;
class vtkDMMLScene;
class vtkDMMLNode;
class vtkDMMLMarkupsDisplayNode;
class vtkDMMLMarkupsNode;
class vtkDMMLSelectionNode;

class Q_CJYX_MODULE_MARKUPS_WIDGETS_EXPORT qDMMLMarkupsDisplayNodeWidget : public qDMMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qDMMLWidget Superclass;
  qDMMLMarkupsDisplayNodeWidget(QWidget *parent=nullptr);
  ~qDMMLMarkupsDisplayNodeWidget() override;

  vtkDMMLMarkupsDisplayNode* dmmlMarkupsDisplayNode()const;

  bool visibility()const;

  bool glyphSizeIsAbsolute()const;
  bool curveLineSizeIsAbsolute()const;

  bool propertiesLabelVisibility()const;
  bool pointLabelsVisibility()const;

signals:
  /// Signal sent if the any property in the display node is changed
  void displayNodeChanged();
  /// Signal sent if the auto/manual value is updated
  void scalarRangeModeValueChanged(vtkDMMLDisplayNode::ScalarRangeFlagType value);

public slots:
  /// Set the markups display node to show edit properties of
  void setDMMLMarkupsDisplayNode(vtkDMMLMarkupsDisplayNode *node);
  /// Utility function to be connected with generic signals
  void setDMMLMarkupsDisplayNode(vtkDMMLNode *node);

  /// Set the markups display node to show edit properties of,
  /// by specifying markups node.
  void setDMMLMarkupsNode(vtkDMMLMarkupsNode* node);
  /// Utility function to be connected with generic signals
  void setDMMLMarkupsNode(vtkDMMLNode* node);

  void setVisibility(bool);

  void setGlyphSizeIsAbsolute(bool absolute);
  void setCurveLineSizeIsAbsolute(bool absolute);

  void setPropertiesLabelVisibility(bool visible);
  void setPointLabelsVisibility(bool visible);

  void setMaximumMarkupsScale(double maxScale);
  void setMaximumMarkupsSize(double maxScale);

  void setFillVisibility(bool visibility);
  void setOutlineVisibility(bool visibility);
  void onFillOpacitySliderWidgetChanged(double opacity);
  void onOutlineOpacitySliderWidgetChanged(double opacity);
  void setOccludedVisibility(bool visibility);
  void setOccludedOpacity(double OccludedOpacity);

protected slots:
  void updateWidgetFromDMML();
  vtkDMMLSelectionNode* getSelectionNode(vtkDMMLScene *dmmlScene);

  void onSelectedColorPickerButtonChanged(QColor qcolor);
  void onUnselectedColorPickerButtonChanged(QColor qcolor);
  void onActiveColorPickerButtonChanged(QColor qcolor);
  void onGlyphTypeComboBoxChanged(QString value);
  void onGlyphScaleSliderWidgetChanged(double value);
  void onGlyphSizeSliderWidgetChanged(double value);
  void onCurveLineThicknessSliderWidgetChanged(double percentValue);
  void onCurveLineDiameterSliderWidgetChanged(double value);
  void onTextScaleSliderWidgetChanged(double value);
  void onOpacitySliderWidgetChanged(double value);
  void onSnapModeWidgetChanged();
  void onTextPropertyWidgetsChanged();

  void onInteractionCheckBoxChanged(int state);

protected:
  QScopedPointer<qDMMLMarkupsDisplayNodeWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLMarkupsDisplayNodeWidget);
  Q_DISABLE_COPY(qDMMLMarkupsDisplayNodeWidget);
};

#endif
