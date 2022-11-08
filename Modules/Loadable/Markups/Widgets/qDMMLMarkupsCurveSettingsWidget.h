/*==============================================================================

  Copyright (c) The Intervention Centre
  Oslo University Hospital, Oslo, Norway. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Rafael Palomar (The Intervention Centre,
  Oslo University Hospital) and was supported by The Research Council of Norway
  through the ALive project (grant nr. 311393).

==============================================================================*/

#ifndef __qCjyxCurveSettingsWidget_h_
#define __qCjyxCurveSettingsWidget_h_

// Markups widgets includes
#include "qDMMLMarkupsAbstractOptionsWidget.h"
#include "qCjyxMarkupsModuleWidgetsExport.h"

// ------------------------------------------------------------------------------
class qDMMLMarkupsCurveSettingsWidgetPrivate;
class vtkDMMLMarkupsNode;

// ------------------------------------------------------------------------------
class Q_CJYX_MODULE_MARKUPS_WIDGETS_EXPORT qDMMLMarkupsCurveSettingsWidget
  : public qDMMLMarkupsAbstractOptionsWidget
{
  Q_OBJECT

public:
  typedef qDMMLMarkupsAbstractOptionsWidget Superclass;
  qDMMLMarkupsCurveSettingsWidget(QWidget* parent=nullptr);
  ~qDMMLMarkupsCurveSettingsWidget() override;

  /// Gets the name of the additional options widget type
  const QString className() const override {return "qDMMLMarkupsCurveSettingsWidget";}

  /// Checks whether a given node can be handled by the widget
  bool canManageDMMLMarkupsNode(vtkDMMLMarkupsNode *markupsNode) const override;

  /// Updates the widget on DMML changes
  void updateWidgetFromDMML() override;

  /// Set the DMML node of interest
  void setDMMLMarkupsNode(vtkDMMLMarkupsNode* node) override;

  /// Returns an instance of the widget
  qDMMLMarkupsAbstractOptionsWidget* createInstance() const override
  { return new qDMMLMarkupsCurveSettingsWidget(); }

public slots:
  void onCurveTypeParameterChanged();
  void onProjectCurveMaximumSearchRadiusChanged();
  void onApplyCurveResamplingPushButtonClicked();
  void setDMMLScene(vtkDMMLScene* scene) override;

protected:
  qDMMLMarkupsCurveSettingsWidget(QWidget* parent, qDMMLMarkupsCurveSettingsWidgetPrivate &d);

protected:
  void setup();

protected:
  QScopedPointer<qDMMLMarkupsCurveSettingsWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLMarkupsCurveSettingsWidget);
  Q_DISABLE_COPY(qDMMLMarkupsCurveSettingsWidget);
};

#endif // __qCjyxCurveSettingsWidget_h_
