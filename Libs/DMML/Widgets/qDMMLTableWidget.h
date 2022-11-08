/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

#ifndef __qDMMLTableWidget_h
#define __qDMMLTableWidget_h

// Qt includes
#include <QWidget>
class QResizeEvent;

// qDMMLWidget includes
#include "qDMMLWidget.h"
class qDMMLTableViewControllerWidget;
class qDMMLTableView;
class qDMMLTableWidgetPrivate;

// DMML includes
class vtkDMMLTableViewNode;
class vtkDMMLScene;

/// \brief qDMMLTableWidget is the toplevel table widget that can be
/// packed in a layout.
///
/// qDMMLTableWidget provides tabling capabilities with a display
/// canvas for the table and a controller widget to control the
/// content and properties of the table.
class QDMML_WIDGETS_EXPORT qDMMLTableWidget : public qDMMLWidget
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef qDMMLWidget Superclass;

  /// Constructors
  explicit qDMMLTableWidget(QWidget* parent = nullptr);
  ~qDMMLTableWidget() override;

  /// Get the tabl node observed by view.
  vtkDMMLTableViewNode* dmmlTableViewNode()const;

  /// Get a reference to the underlying Table View
  /// Becareful if you change the TableView, you might
  /// unsynchronize the view from the nodes/logics.
  Q_INVOKABLE qDMMLTableView* tableView()const;

  /// Get table view controller widget
  Q_INVOKABLE qDMMLTableViewControllerWidget* tableController()const;

  /// Get the view label for the table.
  /// \sa qDMMLTableControllerWidget::tableViewLabel()
  /// \sa setTableViewLabel()
  QString viewLabel()const;

  /// Set the view label for the table.
  /// \sa qDMMLTableControllerWidget::tableViewLabel()
  /// \sa tableViewLabel()
  void setViewLabel(const QString& newTableViewLabel);

public slots:
  /// Set the current \a viewNode to observe
  void setDMMLTableViewNode(vtkDMMLTableViewNode* newTableViewNode);

protected:
  QScopedPointer<qDMMLTableWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLTableWidget);
  Q_DISABLE_COPY(qDMMLTableWidget);
};

#endif
