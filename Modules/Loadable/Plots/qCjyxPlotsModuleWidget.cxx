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

// C++ includes
#include <cmath>

// Cjyx includes
#include "qCjyxApplication.h"
#include "qCjyxAbstractModuleWidget.h"
#include "qCjyxLayoutManager.h"
#include "qCjyxPlotsModuleWidget.h"
#include "ui_qCjyxPlotsModuleWidget.h"

// vtkCjyxLogic includes
#include "vtkCjyxPlotsLogic.h"

// DMMLWidgets includes
#include <qDMMLUtils.h>
//#include <qDMMLPlotModel.h>

// DMML includes
#include <vtkDMMLLayoutNode.h>
#include "vtkDMMLPlotChartNode.h"
#include "vtkDMMLPlotSeriesNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSelectionNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkPlot.h>

//-----------------------------------------------------------------------------
class qCjyxPlotsModuleWidgetPrivate: public Ui_qCjyxPlotsModuleWidget
{
  Q_DECLARE_PUBLIC(qCjyxPlotsModuleWidget);
protected:
  qCjyxPlotsModuleWidget* const q_ptr;
public:
  qCjyxPlotsModuleWidgetPrivate(qCjyxPlotsModuleWidget& object);

  vtkCjyxPlotsLogic*      logic()const;

  vtkWeakPointer<vtkDMMLPlotChartNode> DMMLPlotChartNode;
  vtkWeakPointer<vtkDMMLPlotSeriesNode> DMMLPlotSeriesNode;
};

//-----------------------------------------------------------------------------
qCjyxPlotsModuleWidgetPrivate::qCjyxPlotsModuleWidgetPrivate(qCjyxPlotsModuleWidget& object)
  : q_ptr(&object)
{
  this->DMMLPlotChartNode = nullptr;
  this->DMMLPlotSeriesNode = nullptr;
}
//-----------------------------------------------------------------------------
vtkCjyxPlotsLogic* qCjyxPlotsModuleWidgetPrivate::logic()const
{
  Q_Q(const qCjyxPlotsModuleWidget);
  return vtkCjyxPlotsLogic::SafeDownCast(q->logic());
}

/*
//-----------------------------------------------------------------------------
vtkPlot* qCjyxPlotsModuleWidgetPrivate::table()const
{
  if (this->DMMLPlotChartNode.GetPointer()==nullptr)
    {
    return nullptr;
    }
  return this->DMMLPlotChartNode->GetPlot();
}
*/

//-----------------------------------------------------------------------------
qCjyxPlotsModuleWidget::qCjyxPlotsModuleWidget(QWidget* _parentWidget)
  : Superclass(_parentWidget)
  , d_ptr(new qCjyxPlotsModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qCjyxPlotsModuleWidget::~qCjyxPlotsModuleWidget() = default;

//-----------------------------------------------------------------------------
void qCjyxPlotsModuleWidget::setup()
{
  Q_D(qCjyxPlotsModuleWidget);
  d->setupUi(this);

  this->connect(d->PlotChartNodeSelector, SIGNAL(currentNodeChanged(vtkDMMLNode*)), SLOT(onNodeSelected(vtkDMMLNode*)));

  this->connect(d->PlotSeriesNodeSelector, SIGNAL(nodeAddedByUser(vtkDMMLNode*)), SLOT(onSeriesNodeAddedByUser(vtkDMMLNode*)));
  this->connect(d->PlotChartPropertiesWidget, SIGNAL(seriesNodeAddedByUser(vtkDMMLNode*)), SLOT(onSeriesNodeAddedByUser(vtkDMMLNode*)));

  this->connect(d->ShowChartButton, SIGNAL(clicked()), SLOT(onShowChartButtonClicked()));
}

//-----------------------------------------------------------------------------
void qCjyxPlotsModuleWidget::onNodeSelected(vtkDMMLNode* node)
{
  Q_D(qCjyxPlotsModuleWidget);
  vtkDMMLPlotChartNode* chartNode = vtkDMMLPlotChartNode::SafeDownCast(node);

  this->qvtkReconnect(d->DMMLPlotChartNode, chartNode, vtkCommand::ModifiedEvent, this, SLOT(onDMMLPlotChartNodeModified(vtkObject*)));
  d->DMMLPlotChartNode = chartNode;

  // Update GUI from the newly selected node
  //this->onDMMLPlotChartNodeModified(d->DMMLPlotChartNode);
}

//-----------------------------------------------------------------------------
void qCjyxPlotsModuleWidget::onDMMLPlotChartNodeModified(vtkObject* caller)
{
  Q_UNUSED(caller);
  /*
  Q_D(qCjyxPlotsModuleWidget);
#ifndef QT_NO_DEBUG
  vtkDMMLPlotChartNode* tableNode = vtkDMMLPlotChartNode::SafeDownCast(caller);
  Q_ASSERT(d->DMMLPlotChartNode == tableNode);
#else
  Q_UNUSED(caller);
#endif

  bool validNode = d->DMMLPlotChartNode != 0;
  bool editableNode = d->DMMLPlotChartNode != 0 && !d->DMMLPlotChartNode->GetLocked();

  d->DisplayEditCollapsibleWidget->setEnabled(validNode);
  d->LockPlotButton->setEnabled(validNode);
  d->CopyButton->setEnabled(validNode);
  d->PasteButton->setEnabled(editableNode);
  d->EditControlsFrame->setEnabled(editableNode);

  if (!d->DMMLPlotChartNode)
    {
    return;
    }

  if (d->DMMLPlotChartNode->GetLocked())
    {
    d->LockPlotButton->setIcon(QIcon(":Icons/Medium/CjyxLock.png"));
    d->LockPlotButton->setToolTip(QString("Click to unlock this table so that values can be modified"));
    }
  else
    {
    d->LockPlotButton->setIcon(QIcon(":Icons/Medium/CjyxUnlock.png"));
    d->LockPlotButton->setToolTip(QString("Click to lock this table to prevent modification of the values in the user interface"));
    }

  if (d->DMMLPlotChartNode->GetUseColumnNameAsColumnHeader() != d->LockFirstRowButton->isChecked())
    {
    bool wasBlocked = d->LockFirstRowButton->blockSignals(true);
    d->LockFirstRowButton->setChecked(d->DMMLPlotChartNode->GetUseColumnNameAsColumnHeader());
    d->LockFirstRowButton->blockSignals(wasBlocked);
    }

  if (d->DMMLPlotChartNode->GetUseFirstColumnAsRowHeader() != d->LockFirstColumnButton->isChecked())
    {
    bool wasBlocked = d->LockFirstColumnButton->blockSignals(true);
    d->LockFirstColumnButton->setChecked(d->DMMLPlotChartNode->GetUseFirstColumnAsRowHeader());
    d->LockFirstColumnButton->blockSignals(wasBlocked);
    }
    */
}

//-----------------------------------------------------------------------------
void qCjyxPlotsModuleWidget::onLockPlotButtonClicked()
{
  Q_D(qCjyxPlotsModuleWidget);

  if (!d->DMMLPlotChartNode)
    {
    return;
    }

  // toggle the lock
  //int locked = d->DMMLPlotChartNode->GetLocked();
  //d->DMMLPlotChartNode->SetLocked(!locked);
}

//-----------------------------------------------------------------------------
void qCjyxPlotsModuleWidget::setCurrentPlotNode(vtkDMMLNode* tableNode)
{
  Q_D(qCjyxPlotsModuleWidget);
  d->PlotChartNodeSelector->setCurrentNode(tableNode);
  d->PlotsTabWidget->setCurrentIndex(0);
}

//-----------------------------------------------------------
bool qCjyxPlotsModuleWidget::setEditedNode(vtkDMMLNode* node,
                                              QString role /* = QString()*/,
                                              QString context /* = QString()*/)
{
  Q_D(qCjyxPlotsModuleWidget);
  Q_UNUSED(role);
  Q_UNUSED(context);

  if (vtkDMMLPlotChartNode::SafeDownCast(node))
    {
    d->PlotChartNodeSelector->setCurrentNode(node);
    d->PlotsTabWidget->setCurrentIndex(0);
    return true;
    }
  else if (vtkDMMLPlotSeriesNode::SafeDownCast(node))
    {
    d->PlotSeriesNodeSelector->setCurrentNode(node);
    d->PlotsTabWidget->setCurrentIndex(1);
    return true;
    }

  return false;
}

// --------------------------------------------------------------------------
void qCjyxPlotsModuleWidget::onCopyPlotSeriesNodeClicked()
{
  Q_D(const qCjyxPlotsModuleWidget);

  if (!d->DMMLPlotSeriesNode)
    {
    return;
    }

  vtkCjyxPlotsLogic* logic = d->logic();
  if (!logic)
    {
    qCritical() << Q_FUNC_INFO << "failed: plot logic is not set";
    return;
    }


  vtkDMMLPlotSeriesNode *clonedSeriesNode = logic->CloneSeries(d->DMMLPlotSeriesNode, nullptr);

  // Add the cloned node to the selected chart node
  if (d->DMMLPlotChartNode!= nullptr)
    {
    d->DMMLPlotChartNode->AddAndObservePlotSeriesNodeID(clonedSeriesNode->GetID());
    }
}

//-----------------------------------------------------------------------------
void qCjyxPlotsModuleWidget::onShowChartButtonClicked()
{
  Q_D(qCjyxPlotsModuleWidget);

  if (!d->DMMLPlotChartNode)
    {
    return;
    }

  vtkCjyxPlotsLogic* logic = d->logic();
  if (!logic)
    {
    qCritical() << Q_FUNC_INFO << "failed: plot logic is not set";
    return;
    }

  logic->ShowChartInLayout(d->DMMLPlotChartNode);
}

//-----------------------------------------------------------------------------
void qCjyxPlotsModuleWidget::onSeriesNodeAddedByUser(vtkDMMLNode* addedNode)
{
  Q_D(qCjyxPlotsModuleWidget);

  if (!d->DMMLPlotChartNode)
    {
    return;
    }
  vtkDMMLPlotSeriesNode* addedSeriesNode = vtkDMMLPlotSeriesNode::SafeDownCast(addedNode);
  if (!addedSeriesNode)
    {
    return;
    }
  addedSeriesNode->SetUniqueColor();

  d->PlotSeriesNodeSelector->setCurrentNode(addedNode);
}
