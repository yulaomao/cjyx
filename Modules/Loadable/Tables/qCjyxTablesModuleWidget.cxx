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
#include <QAction>
#include <QFileDialog>
#include <QStringBuilder>

// C++ includes
#include <cmath>

// Cjyx includes
#include "qCjyxTablesModuleWidget.h"
#include "ui_qCjyxTablesModuleWidget.h"

// vtkCjyxLogic includes
#include "vtkCjyxTablesLogic.h"

// DMMLWidgets includes
#include <qDMMLUtils.h>
#include <qDMMLTableModel.h>

// DMML includes
#include "vtkDMMLTableNode.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTable.h>

//-----------------------------------------------------------------------------
class qCjyxTablesModuleWidgetPrivate: public Ui_qCjyxTablesModuleWidget
{
  Q_DECLARE_PUBLIC(qCjyxTablesModuleWidget);
protected:
  qCjyxTablesModuleWidget* const q_ptr;
public:
  qCjyxTablesModuleWidgetPrivate(qCjyxTablesModuleWidget& object);
//  static QList<vtkSmartPointer<vtkDMMLTransformableNode> > getSelectedNodes(qDMMLTreeView* tree);

  vtkCjyxTablesLogic*      logic()const;
  vtkTable* table()const;

  vtkWeakPointer<vtkDMMLTableNode> DMMLTableNode;
  QAction*                      CopyAction;
  QAction*                      PasteAction;
  QAction*                      PlotAction;
};

//-----------------------------------------------------------------------------
qCjyxTablesModuleWidgetPrivate::qCjyxTablesModuleWidgetPrivate(qCjyxTablesModuleWidget& object)
  : q_ptr(&object)
{
  this->DMMLTableNode = nullptr;
  this->CopyAction = nullptr;
  this->PasteAction = nullptr;
  this->PlotAction = nullptr;
}
//-----------------------------------------------------------------------------
vtkCjyxTablesLogic* qCjyxTablesModuleWidgetPrivate::logic()const
{
  Q_Q(const qCjyxTablesModuleWidget);
  return vtkCjyxTablesLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
vtkTable* qCjyxTablesModuleWidgetPrivate::table()const
{
  if (this->DMMLTableNode.GetPointer()==nullptr)
    {
    return nullptr;
    }
  return this->DMMLTableNode->GetTable();
}

//-----------------------------------------------------------------------------
qCjyxTablesModuleWidget::qCjyxTablesModuleWidget(QWidget* _parentWidget)
  : Superclass(_parentWidget)
  , d_ptr(new qCjyxTablesModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qCjyxTablesModuleWidget::~qCjyxTablesModuleWidget() = default;

//-----------------------------------------------------------------------------
void qCjyxTablesModuleWidget::setup()
{
  Q_D(qCjyxTablesModuleWidget);
  d->setupUi(this);

  // Create shortcuts for copy/paste
  d->CopyAction = new QAction(this);
  d->CopyAction->setIcon(QIcon(":Icons/Medium/CjyxEditCopy.png"));
  d->CopyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  // set CTRL+C shortcut
  d->CopyAction->setShortcuts(QKeySequence::Copy);
  d->CopyAction->setToolTip(tr("Copy"));
  this->addAction(d->CopyAction);
  d->PasteAction = new QAction(this);
  d->PasteAction->setIcon(QIcon(":Icons/Medium/CjyxEditPaste.png"));
  d->PasteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  // set CTRL+V shortcut
  d->PasteAction->setShortcuts(QKeySequence::Paste);
  d->PasteAction->setToolTip(tr("Paste"));
  this->addAction(d->PasteAction);
  d->PlotAction = new QAction(this);
  d->PlotAction->setIcon(QIcon(":Icons/Medium/CjyxInteractivePlotting.png"));
  d->PlotAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  // set CTRL+P shortcut
  d->PlotAction->setShortcuts(QKeySequence::Print);
  d->PlotAction->setToolTip(tr("Generate an Interactive Plot based on user-selection"
                               " of the columns of the table."));
  this->addAction(d->PlotAction);

  // Connect node selector with module itself
  this->connect(d->TableNodeSelector, SIGNAL(currentNodeChanged(vtkDMMLNode*)), SLOT(onNodeSelected(vtkDMMLNode*)));

  this->connect(d->LockTableButton, SIGNAL(clicked()), SLOT(onLockTableButtonClicked()));

  this->connect(d->ColumnInsertButton, SIGNAL(clicked()), d->TableView, SLOT(insertColumn()));
  this->connect(d->ColumnDeleteButton, SIGNAL(clicked()), d->TableView, SLOT(deleteColumn()));
  this->connect(d->RowInsertButton, SIGNAL(clicked()), d->TableView, SLOT(insertRow()));
  this->connect(d->RowDeleteButton, SIGNAL(clicked()), d->TableView, SLOT(deleteRow()));
  this->connect(d->LockFirstRowButton, SIGNAL(toggled(bool)), d->TableView, SLOT(setFirstRowLocked(bool)));
  this->connect(d->LockFirstColumnButton, SIGNAL(toggled(bool)), d->TableView, SLOT(setFirstColumnLocked(bool)));

  // Connect copy, paste and plot actions
  d->CopyButton->setDefaultAction(d->CopyAction);
  this->connect(d->CopyAction, SIGNAL(triggered()), d->TableView, SLOT(copySelection()));
  d->PasteButton->setDefaultAction(d->PasteAction);
  this->connect(d->PasteAction, SIGNAL(triggered()), d->TableView, SLOT(pasteSelection()));
  d->PlotButton->setDefaultAction(d->PlotAction);
  this->connect(d->PlotAction, SIGNAL(triggered()), d->TableView, SLOT(plotSelection()));

  d->SelectedColumnPropertiesWidget->setSelectionFromDMMLTableView(d->TableView);

  d->NewColumnPropertiesWidget->setDMMLTableColumnName(vtkDMMLTableNode::GetDefaultColumnName());
  this->connect(d->TableNodeSelector, SIGNAL(currentNodeChanged(vtkDMMLNode*)), d->NewColumnPropertiesWidget, SLOT(setDMMLTableNode(vtkDMMLNode*)));

  this->onNodeSelected(nullptr);
}

//-----------------------------------------------------------------------------
void qCjyxTablesModuleWidget::onNodeSelected(vtkDMMLNode* node)
{
  Q_D(qCjyxTablesModuleWidget);
  vtkDMMLTableNode* tableNode = vtkDMMLTableNode::SafeDownCast(node);

  this->qvtkReconnect(d->DMMLTableNode, tableNode, vtkCommand::ModifiedEvent, this, SLOT(onDMMLTableNodeModified(vtkObject*)));
  d->DMMLTableNode = tableNode;

  // Update GUI from the newly selected node
  this->onDMMLTableNodeModified(d->DMMLTableNode);
}

//-----------------------------------------------------------------------------
void qCjyxTablesModuleWidget::onDMMLTableNodeModified(vtkObject* caller)
{
  Q_D(qCjyxTablesModuleWidget);

#ifndef QT_NO_DEBUG
  vtkDMMLTableNode* tableNode = vtkDMMLTableNode::SafeDownCast(caller);
  Q_ASSERT(d->DMMLTableNode == tableNode);
#else
  Q_UNUSED(caller);
#endif

  bool validNode = d->DMMLTableNode != nullptr;
  bool editableNode = d->DMMLTableNode != nullptr && !d->DMMLTableNode->GetLocked();

  d->DisplayEditCollapsibleWidget->setEnabled(validNode);
  d->LockTableButton->setEnabled(validNode);
  d->CopyButton->setEnabled(validNode);
  d->PasteButton->setEnabled(editableNode);
  d->EditControlsFrame->setEnabled(editableNode);

  if (!d->DMMLTableNode)
    {
    return;
    }

  if (d->DMMLTableNode->GetLocked())
    {
    d->LockTableButton->setIcon(QIcon(":Icons/Medium/CjyxLock.png"));
    d->LockTableButton->setToolTip(QString("Click to unlock this table so that values can be modified"));
    }
  else
    {
    d->LockTableButton->setIcon(QIcon(":Icons/Medium/CjyxUnlock.png"));
    d->LockTableButton->setToolTip(QString("Click to lock this table to prevent modification of the values in the user interface"));
    }

  if (d->DMMLTableNode->GetUseColumnNameAsColumnHeader() != d->LockFirstRowButton->isChecked())
    {
    bool wasBlocked = d->LockFirstRowButton->blockSignals(true);
    d->LockFirstRowButton->setChecked(d->DMMLTableNode->GetUseColumnNameAsColumnHeader());
    d->LockFirstRowButton->blockSignals(wasBlocked);
    }

  if (d->DMMLTableNode->GetUseFirstColumnAsRowHeader() != d->LockFirstColumnButton->isChecked())
    {
    bool wasBlocked = d->LockFirstColumnButton->blockSignals(true);
    d->LockFirstColumnButton->setChecked(d->DMMLTableNode->GetUseFirstColumnAsRowHeader());
    d->LockFirstColumnButton->blockSignals(wasBlocked);
    }
}

//-----------------------------------------------------------------------------
void qCjyxTablesModuleWidget::onLockTableButtonClicked()
{
  Q_D(qCjyxTablesModuleWidget);

  if (!d->DMMLTableNode)
    {
    return;
    }

  // toggle the lock
  int locked = d->DMMLTableNode->GetLocked();
  d->DMMLTableNode->SetLocked(!locked);
}

//-----------------------------------------------------------------------------
void qCjyxTablesModuleWidget::setCurrentTableNode(vtkDMMLNode* tableNode)
{
  Q_D(qCjyxTablesModuleWidget);
  d->TableNodeSelector->setCurrentNode(tableNode);
}

//-----------------------------------------------------------
bool qCjyxTablesModuleWidget::setEditedNode(vtkDMMLNode* node,
                                              QString role /* = QString()*/,
                                              QString context /* = QString()*/)
{
  Q_D(qCjyxTablesModuleWidget);
  Q_UNUSED(role);
  Q_UNUSED(context);

  if (vtkDMMLTableNode::SafeDownCast(node))
    {
    d->TableNodeSelector->setCurrentNode(node);
    return true;
    }
  return false;
}
