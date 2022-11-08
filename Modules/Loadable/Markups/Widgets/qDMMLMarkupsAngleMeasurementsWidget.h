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

#ifndef __qDMMLMarkupsAngleMeasurementsWidget_h_
#define __qDMMLMarkupsAngleMeasurementsWidget_h_

// Markups widgets includes
#include "qDMMLMarkupsAbstractOptionsWidget.h"
#include "qCjyxMarkupsModuleWidgetsExport.h"

#include <ctkVTKObject.h>

// ------------------------------------------------------------------------------
class qDMMLMarkupsAngleMeasurementsWidgetPrivate;
class vtkDMMLMarkupsNode;

// ------------------------------------------------------------------------------
class Q_CJYX_MODULE_MARKUPS_WIDGETS_EXPORT qDMMLMarkupsAngleMeasurementsWidget
  : public qDMMLMarkupsAbstractOptionsWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qDMMLMarkupsAbstractOptionsWidget Superclass;
  qDMMLMarkupsAngleMeasurementsWidget(QWidget* parent=nullptr);
  ~qDMMLMarkupsAngleMeasurementsWidget() override;

  /// Gets the name of the additional options widget type
  const QString className() const override { return "qDMMLMarkupsAngleMeasurementsWidget"; }

  /// Checks whether a given node can be handled by the widget
  bool canManageDMMLMarkupsNode(vtkDMMLMarkupsNode *markupsNode) const override;

  /// Set the DMML node of interest
  void setDMMLMarkupsNode(vtkDMMLMarkupsNode* node) override;

  /// Returns an instance of the widget
  qDMMLMarkupsAbstractOptionsWidget* createInstance() const override
  { return new qDMMLMarkupsAngleMeasurementsWidget(); }

public slots:
  /// Change angle mode of current angle markup if combobox selection is made.
  void onAngleMeasurementModeChanged();
  /// Update angle measurement rotation axis if the user edits the column vector
  void onRotationAxisChanged();
  /// Updates the widget on DMML changes
  void updateWidgetFromDMML() override;

protected:
  qDMMLMarkupsAngleMeasurementsWidget(QWidget* parent, qDMMLMarkupsAngleMeasurementsWidgetPrivate &d);

protected:
  void setup();

protected:
  QScopedPointer<qDMMLMarkupsAngleMeasurementsWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLMarkupsAngleMeasurementsWidget);
  Q_DISABLE_COPY(qDMMLMarkupsAngleMeasurementsWidget);
};

#endif // __qDMMLMarkupsAngleMeasurementsWidget_h_
