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

#ifndef __qCjyxTestLineWidget_h_
#define __qCjyxTestLineWidget_h_

// Markups widgets includes
#include "qDMMLMarkupsAbstractOptionsWidget.h"
#include "qCjyxTemplateKeyModuleWidgetsExport.h"

class qDMMLMarkupsTestLineWidgetPrivate;
class vtkDMMLMarkupsNode;

class Q_CJYX_MODULE_TEMPLATEKEY_WIDGETS_EXPORT
qDMMLMarkupsTestLineWidget : public qDMMLMarkupsAbstractOptionsWidget
{
  Q_OBJECT

  Q_PROPERTY(QString className READ className CONSTANT);

public:

  typedef qDMMLMarkupsAbstractOptionsWidget Superclass;
  qDMMLMarkupsTestLineWidget(QWidget* parent=nullptr);
  ~qDMMLMarkupsTestLineWidget() override;

  /// Gets the name of the additional options widget type
  const QString className() const override {return "qDMMLMarkupsTestLineWidget";}

  /// Updates the widget based on information from DMML.
  void updateWidgetFromDMML() override;

  /// Checks whether a given node can be handled by the widget
  bool canManageDMMLMarkupsNode(vtkDMMLMarkupsNode *markupsNode) const override;

  /// Returns an instance of the widget
  qDMMLMarkupsAbstractOptionsWidget* createInstance() const override
  { return new qDMMLMarkupsTestLineWidget(); }

public slots:
/// Set the DMML node of interest
  void setDMMLMarkupsNode(vtkDMMLMarkupsNode* node) override;

protected:
  void setup();

protected:
  QScopedPointer<qDMMLMarkupsTestLineWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLMarkupsTestLineWidget);
  Q_DISABLE_COPY(qDMMLMarkupsTestLineWidget);
};

#endif // __qCjyxTestLineWidget_h_
