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

#ifndef __qDMMLTableViewControllerWidget_h
#define __qDMMLTableViewControllerWidget_h

// CTK includes
#include <ctkVTKObject.h>

// qDMMLWidget includes
#include "qDMMLViewControllerBar.h"
class qDMMLTableViewControllerWidgetPrivate;
class qDMMLTableView;

// DMML includes
class vtkDMMLTableViewNode;

///
/// qDMMLTableViewControllerWidget offers controls to a table view
/// (vtkDMMLTableViewNode and vtkDMMLTableNode). This controller
/// allows for the content (data) and style (properties) of a table to
/// be defined.
class QDMML_WIDGETS_EXPORT qDMMLTableViewControllerWidget
  : public qDMMLViewControllerBar
{
  Q_OBJECT
  QVTK_OBJECT

public:
  /// Superclass typedef
  typedef qDMMLViewControllerBar Superclass;

  /// Constructors
  explicit qDMMLTableViewControllerWidget(QWidget* parent = nullptr);
  ~qDMMLTableViewControllerWidget() override;

  /// Set the label for the table view (abbreviation for the view name)
  void setViewLabel(const QString& newViewLabel);

  /// Get the label for the view (abbreviation for the view name)
  QString viewLabel()const;

  /// Get ChartViewNode associated with this ChartViewController.
  Q_INVOKABLE vtkDMMLTableViewNode* dmmlTableViewNode() const;

public slots:
  /// Set the scene
  void setDMMLScene(vtkDMMLScene* newScene) override;

  /// Set the TableView with which this controller interacts
  void setTableView(qDMMLTableView* TableView);

  /// Set the TableViewNode associated with this TableViewController.
  /// TableViewNodes are 1-to-1 with TableViews
  void setDMMLTableViewNode(vtkDMMLTableViewNode* tableViewNode);

protected slots:
  void updateWidgetFromDMMLView() override;
  void updateWidgetFromDMML();

private:
  Q_DECLARE_PRIVATE(qDMMLTableViewControllerWidget);
  Q_DISABLE_COPY(qDMMLTableViewControllerWidget);
};

#endif
