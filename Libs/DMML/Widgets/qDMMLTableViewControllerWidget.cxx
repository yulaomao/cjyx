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
#include <QActionGroup>
#include <QDebug>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QHBoxLayout>

// VTK includes
#include <vtkStringArray.h>

// CTK includes
#include <ctkPopupWidget.h>

// qDMML includes
#include "qDMMLColors.h"
#include "qDMMLNodeFactory.h"
#include "qDMMLSceneViewMenu.h"
#include "qDMMLTableView.h"
#include "qDMMLTableViewControllerWidget_p.h"

// DMML includes
#include <vtkDMMLScene.h>
#include <vtkDMMLTableViewNode.h>
#include <vtkDMMLTableNode.h>

// STD include
#include <string>

//--------------------------------------------------------------------------
// qDMMLTableViewControllerWidgetPrivate methods

//---------------------------------------------------------------------------
qDMMLTableViewControllerWidgetPrivate::qDMMLTableViewControllerWidgetPrivate(
  qDMMLTableViewControllerWidget& object)
  : Superclass(object)
{
  this->TableNode = nullptr;
  this->TableView = nullptr;
  this->CopyAction = nullptr;
  this->PasteAction = nullptr;
  this->PlotAction = nullptr;
}

//---------------------------------------------------------------------------
qDMMLTableViewControllerWidgetPrivate::~qDMMLTableViewControllerWidgetPrivate() = default;

//---------------------------------------------------------------------------
void qDMMLTableViewControllerWidgetPrivate::setupPopupUi()
{
  Q_Q(qDMMLTableViewControllerWidget);

  this->Superclass::setupPopupUi();
  this->PopupWidget->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
  this->Ui_qDMMLTableViewControllerWidget::setupUi(this->PopupWidget);

  // Create shortcuts for copy/paste
  this->CopyAction = new QAction(this);
  this->CopyAction->setIcon(QIcon(":Icons/Medium/CjyxEditCopy.png"));
  this->CopyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  // set CTRL+C shortcut
  this->CopyAction->setShortcuts(QKeySequence::Copy);
  this->CopyAction->setToolTip(tr("Copy"));
  q->addAction(this->CopyAction);
  this->PasteAction = new QAction(this);
  this->PasteAction->setIcon(QIcon(":Icons/Medium/CjyxEditPaste.png"));
  this->PasteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  // set CTRL+V shortcut
  this->PasteAction->setShortcuts(QKeySequence::Paste);
  this->PasteAction->setToolTip(tr("Paste"));
  q->addAction(this->PasteAction);
  this->PlotAction = new QAction(this);
  this->PlotAction->setIcon(QIcon(":Icons/Medium/CjyxInteractivePlotting.png"));
  this->PlotAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  // set CTRL+P shortcut
  this->PlotAction->setShortcuts(QKeySequence::Print);
  this->PlotAction->setToolTip(tr("Generate an Interactive Plot based on user-selection"
                               " of the columns of the table."));
  q->addAction(this->PlotAction);

  // Connect Table selector
  this->connect(this->tableComboBox, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
                SLOT(onTableNodeSelected(vtkDMMLNode*)));

  this->connect(this->LockTableButton, SIGNAL(clicked()), SLOT(onLockTableButtonClicked()));

  this->connect(this->ColumnInsertButton, SIGNAL(clicked()), SLOT(insertColumn()));
  this->connect(this->ColumnDeleteButton, SIGNAL(clicked()), SLOT(deleteColumn()));
  this->connect(this->RowInsertButton, SIGNAL(clicked()), SLOT(insertRow()));
  this->connect(this->RowDeleteButton, SIGNAL(clicked()), SLOT(deleteRow()));
  this->connect(this->LockFirstRowButton, SIGNAL(toggled(bool)), SLOT(setFirstRowLocked(bool)));
  this->connect(this->LockFirstColumnButton, SIGNAL(toggled(bool)), SLOT(setFirstColumnLocked(bool)));

  // Connect copy and paste actions
  this->CopyButton->setDefaultAction(this->CopyAction);
  this->connect(this->CopyAction, SIGNAL(triggered()), SLOT(copySelection()));
  this->PasteButton->setDefaultAction(this->PasteAction);
  this->connect(this->PasteAction, SIGNAL(triggered()), SLOT(pasteSelection()));
  this->PlotButton->setDefaultAction(this->PlotAction);
  this->connect(this->PlotAction, SIGNAL(triggered()), SLOT(plotSelection()));

  // Connect the scene
  QObject::connect(q, SIGNAL(dmmlSceneChanged(vtkDMMLScene*)),
                   this->tableComboBox, SLOT(setDMMLScene(vtkDMMLScene*)));

  onTableNodeSelected(nullptr);
}

//---------------------------------------------------------------------------
void qDMMLTableViewControllerWidgetPrivate::init()
{
  this->Superclass::init();
  this->ViewLabel->setText(qDMMLTableViewControllerWidget::tr("1"));
  this->BarLayout->addStretch(1);
  this->setColor(QColor("#e1ba3c"));
}

// --------------------------------------------------------------------------
void qDMMLTableViewControllerWidgetPrivate::onTableNodeSelected(vtkDMMLNode * node)
{
  Q_Q(qDMMLTableViewControllerWidget);

  if (!q->dmmlTableViewNode())
    {
    return;
    }

  if (this->TableNode.GetPointer() == node)
    {
    return;
    }

  this->qvtkReconnect(this->TableNode, node, vtkCommand::ModifiedEvent,
                      q, SLOT(updateWidgetFromDMML()));
  this->TableNode = vtkDMMLTableNode::SafeDownCast(node);

  q->dmmlTableViewNode()->SetTableNodeID(this->TableNode ? this->TableNode->GetID() : nullptr);

  q->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qDMMLTableViewControllerWidgetPrivate::onLockTableButtonClicked()
{
  if (!this->TableNode)
    {
    qWarning("qDMMLTableViewControllerWidgetPrivate::onLockTableButtonClicked failed: tableNode is invalid");
    return;
    }

  // toggle the lock
  int locked = this->TableNode->GetLocked();
  this->TableNode->SetLocked(!locked);
}

//-----------------------------------------------------------------------------
void qDMMLTableViewControllerWidgetPrivate::insertColumn()
{
  if (!this->TableView)
    {
    qWarning("qDMMLTableViewControllerWidgetPrivate::insertColumn failed: TableView is invalid");
    return;
    }
  this->TableView->insertColumn();
}

//-----------------------------------------------------------------------------
void qDMMLTableViewControllerWidgetPrivate::deleteColumn()
{
  if (!this->TableView)
    {
    qWarning("qDMMLTableViewControllerWidgetPrivate::deleteColumn failed: TableView is invalid");
    return;
    }
  this->TableView->deleteColumn();
}

//-----------------------------------------------------------------------------
void qDMMLTableViewControllerWidgetPrivate::insertRow()
{
  if (!this->TableView)
    {
    qWarning("qDMMLTableViewControllerWidgetPrivate::insertRow failed: TableView is invalid");
    return;
    }
  this->TableView->insertRow();
}

//-----------------------------------------------------------------------------
void qDMMLTableViewControllerWidgetPrivate::deleteRow()
{
  if (!this->TableView)
    {
    qWarning("qDMMLTableViewControllerWidgetPrivate::deleteRow failed: TableView is invalid");
    return;
    }
  this->TableView->deleteRow();
}

//-----------------------------------------------------------------------------
void qDMMLTableViewControllerWidgetPrivate::setFirstRowLocked(bool locked)
{
  if (!this->TableView)
    {
    qWarning("qDMMLTableViewControllerWidgetPrivate::setFirstRowLocked failed: TableView is invalid");
    return;
    }
  this->TableView->setFirstRowLocked(locked);
}

//-----------------------------------------------------------------------------
void qDMMLTableViewControllerWidgetPrivate::setFirstColumnLocked(bool locked)
{
  if (!this->TableView)
    {
    qWarning("qDMMLTableViewControllerWidgetPrivate::setFirstColumnLocked failed: TableView is invalid");
    return;
    }
  this->TableView->setFirstColumnLocked(locked);
}

//-----------------------------------------------------------------------------
void qDMMLTableViewControllerWidgetPrivate::copySelection()
{
  if (!this->TableView)
    {
    qWarning("qDMMLTableViewControllerWidgetPrivate::copySelection failed: TableView is invalid");
    return;
    }
  this->TableView->copySelection();
}

//-----------------------------------------------------------------------------
void qDMMLTableViewControllerWidgetPrivate::pasteSelection()
{
  if (!this->TableView)
    {
    qWarning("qDMMLTableViewControllerWidgetPrivate::pasteSelection failed: TableView is invalid");
    return;
    }
  this->TableView->pasteSelection();
}

//-----------------------------------------------------------------------------
void qDMMLTableViewControllerWidgetPrivate::plotSelection()
{
  if (!this->TableView)
    {
    qWarning("qDMMLTableViewControllerWidgetPrivate::plotSelection failed: TableView is invalid");
    return;
    }
  this->TableView->plotSelection();
}

// --------------------------------------------------------------------------
// qDMMLTableViewControllerWidget methods

// --------------------------------------------------------------------------
qDMMLTableViewControllerWidget::qDMMLTableViewControllerWidget(QWidget* parentWidget)
  : Superclass(new qDMMLTableViewControllerWidgetPrivate(*this), parentWidget)
{
  Q_D(qDMMLTableViewControllerWidget);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLTableViewControllerWidget::~qDMMLTableViewControllerWidget()
{
  this->setDMMLScene(nullptr);
}

// --------------------------------------------------------------------------
void qDMMLTableViewControllerWidget::setTableView(qDMMLTableView* view)
{
  Q_D(qDMMLTableViewControllerWidget);

  d->TableView = view;
}

//---------------------------------------------------------------------------
void qDMMLTableViewControllerWidget::setViewLabel(const QString& newViewLabel)
{
  Q_D(qDMMLTableViewControllerWidget);
  if (!this->dmmlTableViewNode())
    {
    qCritical() << Q_FUNC_INFO << " failed: must set view node first";
    return;
    }
  this->dmmlTableViewNode()->SetLayoutLabel(newViewLabel.toUtf8());
}

//---------------------------------------------------------------------------
QString qDMMLTableViewControllerWidget::viewLabel()const
{
  Q_D(const qDMMLTableViewControllerWidget);
  if (this->dmmlTableViewNode())
    {
    qCritical() << Q_FUNC_INFO << " failed: must set view node first";
    return QString();
    }
  return this->dmmlTableViewNode()->GetLayoutLabel();
}

// --------------------------------------------------------------------------
void qDMMLTableViewControllerWidget::setDMMLTableViewNode(
    vtkDMMLTableViewNode * viewNode)
{
  Q_D(qDMMLTableViewControllerWidget);
  this->setDMMLViewNode(viewNode);
}

//---------------------------------------------------------------------------
vtkDMMLTableViewNode* qDMMLTableViewControllerWidget::dmmlTableViewNode()const
{
  Q_D(const qDMMLTableViewControllerWidget);
  return vtkDMMLTableViewNode::SafeDownCast(this->dmmlViewNode());
}

// --------------------------------------------------------------------------
void qDMMLTableViewControllerWidget::updateWidgetFromDMML()
{
  Q_D(qDMMLTableViewControllerWidget);

  //qDebug() << "qDMMLTableViewControllerWidget::updateWidgetFromDMML()";

  if (!this->dmmlTableViewNode() || !this->dmmlScene())
    {
    return;
    }

  d->ViewLabel->setText(this->dmmlTableViewNode()->GetLayoutLabel());

  vtkDMMLTableNode *tableNode
    = vtkDMMLTableNode::SafeDownCast(this->dmmlScene()->GetNodeByID(this->dmmlTableViewNode()->GetTableNodeID()));

  // TableNode selector
  d->tableComboBox->setCurrentNodeID(tableNode ? tableNode->GetID() : nullptr);

  bool validNode = tableNode != nullptr;
  bool editableNode = tableNode != nullptr && !tableNode->GetLocked();

  d->LockTableButton->setEnabled(validNode);
  d->CopyButton->setEnabled(validNode);
  d->PasteButton->setEnabled(editableNode);
  d->EditControlsFrame->setEnabled(editableNode);

  if (!tableNode)
    {
    return;
    }

  if (tableNode->GetLocked())
    {
    d->LockTableButton->setIcon(QIcon(":Icons/Medium/CjyxLock.png"));
    d->LockTableButton->setToolTip(QString("Click to unlock this table so that values can be modified"));
    }
  else
    {
    d->LockTableButton->setIcon(QIcon(":Icons/Medium/CjyxUnlock.png"));
    d->LockTableButton->setToolTip(QString("Click to lock this table to prevent modification of the values in the user interface"));
    }

  if (tableNode->GetUseColumnNameAsColumnHeader() != d->LockFirstRowButton->isChecked())
    {
    bool wasBlocked = d->LockFirstRowButton->blockSignals(true);
    d->LockFirstRowButton->setChecked(tableNode->GetUseColumnNameAsColumnHeader());
    d->LockFirstRowButton->blockSignals(wasBlocked);
    }

  if (tableNode->GetUseFirstColumnAsRowHeader() != d->LockFirstColumnButton->isChecked())
    {
    bool wasBlocked = d->LockFirstColumnButton->blockSignals(true);
    d->LockFirstColumnButton->setChecked(tableNode->GetUseFirstColumnAsRowHeader());
    d->LockFirstColumnButton->blockSignals(wasBlocked);
    }
}

// --------------------------------------------------------------------------
void qDMMLTableViewControllerWidget::setDMMLScene(vtkDMMLScene* newScene)
{
  Q_D(qDMMLTableViewControllerWidget);

  if (this->dmmlScene() == newScene)
    {
    return;
    }

   d->qvtkReconnect(this->dmmlScene(), newScene, vtkDMMLScene::EndBatchProcessEvent,
                    this, SLOT(updateWidgetFromDMML()));

  // Disable the node selectors as they would fire signal currentIndexChanged(0)
  // meaning that there is no current node anymore. It's not true, it just means
  // that the current node was not in the combo box list menu before
  bool tableBlockSignals = d->tableComboBox->blockSignals(true);
  //bool arrayBlockSignals = d->arrayComboBox->blockSignals(true);

  this->Superclass::setDMMLScene(newScene);

  d->tableComboBox->blockSignals(tableBlockSignals);
  //d->arrayComboBox->blockSignals(arrayBlockSignals);

  if (this->dmmlScene())
    {
    this->updateWidgetFromDMML();
    }
}

// --------------------------------------------------------------------------
void qDMMLTableViewControllerWidget::updateWidgetFromDMMLView()
{
  Superclass::updateWidgetFromDMMLView();
  this->updateWidgetFromDMML();
}
