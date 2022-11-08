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

  This file was originally developed by Matthew Holden, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __qCjyxTableColumnPropertiesWidget_h
#define __qCjyxTableColumnPropertiesWidget_h

// Qt includes
#include "qCjyxWidget.h"

#include "qDMMLUtils.h"

// Tables Widgets includes
#include "qCjyxTablesModuleWidgetsExport.h"
#include "ui_qCjyxTableColumnPropertiesWidget.h"


class qCjyxTableColumnPropertiesWidgetPrivate;
class vtkDMMLTableNode;
class qDMMLTableView;

/// \ingroup Cjyx_QtModules_CreateModels
class Q_CJYX_MODULE_TABLES_WIDGETS_EXPORT
qCjyxTableColumnPropertiesWidget : public qCjyxWidget
{
  Q_OBJECT
  Q_PROPERTY(bool columnNameVisible READ columnNameVisible WRITE setColumnNameVisible)
  Q_PROPERTY(bool confirmTypeChange READ confirmTypeChange WRITE setConfirmTypeChange)

public:
  typedef qCjyxWidget Superclass;
  qCjyxTableColumnPropertiesWidget(QWidget *parent=nullptr);
  ~qCjyxTableColumnPropertiesWidget() override;

  /// Get the table node the columns are edited of.
  Q_INVOKABLE vtkDMMLTableNode* dmmlTableNode()const;

  Q_INVOKABLE QStringList dmmlTableColumnNames();

  /// Get the selected table node and column from the specified tableView widget.
  Q_INVOKABLE void setSelectionFromDMMLTableView(qDMMLTableView* tableView);

  Q_INVOKABLE bool columnNameVisible() const;
  Q_INVOKABLE bool confirmTypeChange() const;

  Q_INVOKABLE void setColumnProperty(QString propertyName, QString propertyValue);
  Q_INVOKABLE QString columnProperty(QString propertyName) const;

public slots:

  void setDMMLScene(vtkDMMLScene* scene) override;

  void setDMMLTableNode(vtkDMMLTableNode* tableNode);
  /// Utility function to simply connect signals/slots with Qt Designer
  void setDMMLTableNode(vtkDMMLNode* tableNode);

  void setDMMLTableColumnName(const QString& selectedColumn);
  void setDMMLTableColumnNames(const QStringList& selectedColumns);

  /// Show table name row
  void setColumnNameVisible(bool);

  /// If enabled then column type change is not performed immediately but user
  /// must to confirmit  by pressing "Convert" button.
  void setConfirmTypeChange(bool);

protected slots:

  /// Called when selection is changed in the associated table view
  void tableViewSelectionChanged();

  void onDataTypeChanged(const QString&);

  void onPropertyChanged(const QString&);

  void onApplyTypeChange();
  void onCancelTypeChange();

  /// Update the GUI to reflect the currently selected table node.
  void updateWidget();

signals:

  /// This signal is emitted if updates to the widget have finished.
  /// It is called after fiducials are changed (added, position modified, etc).
  void updateFinished();

protected:
  QScopedPointer<qCjyxTableColumnPropertiesWidgetPrivate> d_ptr;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qCjyxTableColumnPropertiesWidget);
  Q_DISABLE_COPY(qCjyxTableColumnPropertiesWidget);

};

#endif
