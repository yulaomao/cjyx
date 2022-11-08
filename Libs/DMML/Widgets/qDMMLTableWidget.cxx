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

// Qt includes
#include <QDebug>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QToolButton>

// CTK includes
#include <ctkPopupWidget.h>

// qDMML includes
#include "qDMMLTableViewControllerWidget.h"
#include "qDMMLTableView.h"
#include "qDMMLTableWidget.h"

//--------------------------------------------------------------------------
// qDMMLTableWidgetPrivate
class qDMMLTableWidgetPrivate
  : public QObject
{
  Q_DECLARE_PUBLIC(qDMMLTableWidget);
protected:
  qDMMLTableWidget* const q_ptr;
public:
  qDMMLTableWidgetPrivate(qDMMLTableWidget& object);
  ~qDMMLTableWidgetPrivate() override;

  void init();

  qDMMLTableView*       TableView;
  qDMMLTableViewControllerWidget* TableController;
};


//---------------------------------------------------------------------------
qDMMLTableWidgetPrivate::qDMMLTableWidgetPrivate(qDMMLTableWidget& object)
  : q_ptr(&object)
{
  this->TableView = nullptr;
  this->TableController = nullptr;
}

//---------------------------------------------------------------------------
qDMMLTableWidgetPrivate::~qDMMLTableWidgetPrivate() = default;

//---------------------------------------------------------------------------
void qDMMLTableWidgetPrivate::init()
{
  Q_Q(qDMMLTableWidget);

  QVBoxLayout* layout = new QVBoxLayout(q);
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);

  this->TableController = new qDMMLTableViewControllerWidget;
  layout->addWidget(this->TableController);

  this->TableView = new qDMMLTableView;
  layout->addWidget(this->TableView);

  this->TableController->setTableView(this->TableView);

  QObject::connect(q, SIGNAL(dmmlSceneChanged(vtkDMMLScene*)),
                   this->TableView, SLOT(setDMMLScene(vtkDMMLScene*)));
  QObject::connect(q, SIGNAL(dmmlSceneChanged(vtkDMMLScene*)),
                   this->TableController, SLOT(setDMMLScene(vtkDMMLScene*)));
}

// --------------------------------------------------------------------------
// qDMMLTableWidget methods

// --------------------------------------------------------------------------
qDMMLTableWidget::qDMMLTableWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qDMMLTableWidgetPrivate(*this))
{
  Q_D(qDMMLTableWidget);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLTableWidget::~qDMMLTableWidget()
{
  Q_D(qDMMLTableWidget);
  d->TableView->setDMMLScene(nullptr);
  d->TableController->setDMMLScene(nullptr);
}


// --------------------------------------------------------------------------
void qDMMLTableWidget::setDMMLTableViewNode(vtkDMMLTableViewNode* newTableViewNode)
{
  Q_D(qDMMLTableWidget);
  d->TableView->setDMMLTableViewNode(newTableViewNode);
  d->TableController->setDMMLTableViewNode(newTableViewNode);
}

// --------------------------------------------------------------------------
vtkDMMLTableViewNode* qDMMLTableWidget::dmmlTableViewNode()const
{
  Q_D(const qDMMLTableWidget);
  return d->TableView->dmmlTableViewNode();
}

// --------------------------------------------------------------------------
qDMMLTableView* qDMMLTableWidget::tableView()const
{
  Q_D(const qDMMLTableWidget);
  return d->TableView;
}

// --------------------------------------------------------------------------
qDMMLTableViewControllerWidget* qDMMLTableWidget::tableController()const
{
  Q_D(const qDMMLTableWidget);
  return d->TableController;
}

//---------------------------------------------------------------------------
void qDMMLTableWidget::setViewLabel(const QString& newTableViewLabel)
{
  Q_D(qDMMLTableWidget);
  d->TableController->setViewLabel(newTableViewLabel);
}

//---------------------------------------------------------------------------
QString qDMMLTableWidget::viewLabel()const
{
  Q_D(const qDMMLTableWidget);
  return d->TableController->viewLabel();
}
