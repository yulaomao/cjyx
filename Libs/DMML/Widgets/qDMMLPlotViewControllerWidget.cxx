/*==============================================================================

  Copyright (c) Kapteyn Astronomical Institute
  University of Groningen, Groningen, Netherlands. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Davide Punzo, Kapteyn Astronomical Institute,
  and was supported through the European Research Council grant nr. 291531.

==============================================================================*/

// Qt includes
#include <QActionGroup>
#include <QDebug>
#include <QFileDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QToolButton>

// VTK includes
#include <vtkCollection.h>
#include <vtkFloatArray.h>
#include <vtkPlot.h>
#include <vtkPlotLine.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTable.h>

// CTK includes
#include <ctkPopupWidget.h>

// qDMML includes
#include "qDMMLColors.h"
#include "qDMMLNodeFactory.h"
#include "qDMMLSceneViewMenu.h"
#include "qDMMLPlotView.h"
#include "qDMMLPlotViewControllerWidget_p.h"

// DMML includes
#include <vtkDMMLScene.h>
#include <vtkDMMLPlotSeriesNode.h>
#include <vtkDMMLPlotChartNode.h>
#include <vtkDMMLPlotViewNode.h>
#include <vtkDMMLSelectionNode.h>
#include <vtkDMMLTableNode.h>

// STD include
#include <string>

//--------------------------------------------------------------------------
// qDMMLPlotViewControllerWidgetPrivate methods

//---------------------------------------------------------------------------
qDMMLPlotViewControllerWidgetPrivate::qDMMLPlotViewControllerWidgetPrivate(
  qDMMLPlotViewControllerWidget& object)
  : Superclass(object)
{
  this->FitToWindowToolButton = nullptr;

  this->PlotChartNode = nullptr;
  this->PlotView = nullptr;
}

//---------------------------------------------------------------------------
qDMMLPlotViewControllerWidgetPrivate::~qDMMLPlotViewControllerWidgetPrivate() = default;

//---------------------------------------------------------------------------
void qDMMLPlotViewControllerWidgetPrivate::setupPopupUi()
{
  Q_Q(qDMMLPlotViewControllerWidget);

  this->Superclass::setupPopupUi();
  this->PopupWidget->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
  this->Ui_qDMMLPlotViewControllerWidget::setupUi(this->PopupWidget);

  this->connect(this->plotChartComboBox, SIGNAL(currentNodeChanged(vtkDMMLNode*)), SLOT(onPlotChartNodeSelected(vtkDMMLNode*)));

  this->connect(this->plotSeriesComboBox, SIGNAL(checkedNodesChanged()), SLOT(onPlotSeriesNodesSelected()));
  this->connect(this->plotSeriesComboBox, SIGNAL(nodeAddedByUser(vtkDMMLNode*)), SLOT(onPlotSeriesNodeAdded(vtkDMMLNode*)));

  this->connect(this->plotTypeComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onPlotTypeChanged(int)));
  this->connect(this->interactionModeComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onInteractionModeChanged(int)));
  QObject::connect(this->actionFit_to_window, SIGNAL(triggered()), q, SLOT(fitPlotToAxes()));

  QObject::connect(this->exportPushButton, SIGNAL(clicked()), q, SLOT(onExportButton()));

  // Connect the scene
  QObject::connect(q, SIGNAL(dmmlSceneChanged(vtkDMMLScene*)), this->plotChartComboBox, SLOT(setDMMLScene(vtkDMMLScene*)));
  QObject::connect(q, SIGNAL(dmmlSceneChanged(vtkDMMLScene*)), this->plotSeriesComboBox, SLOT(setDMMLScene(vtkDMMLScene*)));
}

//---------------------------------------------------------------------------
void qDMMLPlotViewControllerWidgetPrivate::init()
{
  Q_Q(qDMMLPlotViewControllerWidget);

  this->Superclass::init();

  this->ViewLabel->setText(qDMMLPlotViewControllerWidget::tr("P"));
  this->BarLayout->addStretch(1);
  this->setColor(QColor(27, 198, 207));

  this->FitToWindowToolButton = new QToolButton(q);
  this->FitToWindowToolButton->setAutoRaise(true);
  this->FitToWindowToolButton->setDefaultAction(this->actionFit_to_window);
  this->FitToWindowToolButton->setFixedSize(15, 15);
  this->BarLayout->insertWidget(2, this->FitToWindowToolButton);
}


// --------------------------------------------------------------------------
vtkDMMLPlotChartNode* qDMMLPlotViewControllerWidgetPrivate::GetPlotChartNodeFromView()
{
  Q_Q(qDMMLPlotViewControllerWidget);

  if (!q->dmmlPlotViewNode() || !q->dmmlScene())
    {
    // qDebug() << "No PlotViewNode or no Scene";
    return nullptr;
    }

  // Get the current PlotChart node
  vtkDMMLPlotChartNode *PlotChartNodeFromViewNode
    = vtkDMMLPlotChartNode::SafeDownCast(q->dmmlScene()->GetNodeByID(q->dmmlPlotViewNode()->GetPlotChartNodeID()));

  return PlotChartNodeFromViewNode;
}

// --------------------------------------------------------------------------
void qDMMLPlotViewControllerWidgetPrivate::onPlotChartNodeSelected(vtkDMMLNode *node)
{
  Q_Q(qDMMLPlotViewControllerWidget);

  vtkDMMLPlotChartNode *dmmlPlotChartNode = vtkDMMLPlotChartNode::SafeDownCast(node);

  if (!q->dmmlPlotViewNode() || this->PlotChartNode == dmmlPlotChartNode)
    {
    return;
    }

  q->dmmlPlotViewNode()->SetPlotChartNodeID(dmmlPlotChartNode ? dmmlPlotChartNode->GetID() : nullptr);

  vtkDMMLSelectionNode* selectionNode = vtkDMMLSelectionNode::SafeDownCast(
    q->dmmlScene() ? q->dmmlScene()->GetNodeByID("vtkDMMLSelectionNodeSingleton") : nullptr);
  if (selectionNode)
    {
    selectionNode->SetActivePlotChartID(dmmlPlotChartNode ? dmmlPlotChartNode->GetID() : "");
    }

  q->updateWidgetFromDMML();
}


// --------------------------------------------------------------------------
void qDMMLPlotViewControllerWidgetPrivate::onPlotSeriesNodesSelected()
{
  Q_Q(qDMMLPlotViewControllerWidget);
  if (!q->dmmlPlotViewNode() || !this->PlotChartNode)
    {
    return;
    }

  std::vector<std::string> plotSeriesNodesIDs;
  this->PlotChartNode->GetPlotSeriesNodeIDs(plotSeriesNodesIDs);

  // loop over arrays in the widget
  for (int idx = 0; idx < this->plotSeriesComboBox->nodeCount(); idx++)
    {
    vtkDMMLPlotSeriesNode *dn = vtkDMMLPlotSeriesNode::SafeDownCast(this->plotSeriesComboBox->nodeFromIndex(idx));

    bool checked = (this->plotSeriesComboBox->checkState(dn) == Qt::Checked);

    // is the node in the Plot?
    bool found = false;
    std::vector<std::string>::iterator it = plotSeriesNodesIDs.begin();
    for (; it != plotSeriesNodesIDs.end(); ++it)
      {
      if (!strcmp(dn->GetID(), (*it).c_str()))
        {
        if (!checked)
          {
          // plot is not checked but currently in the LayoutPlot, remove it
          // (might want to cache the old name in case user adds it back)
          this->PlotChartNode->RemovePlotSeriesNodeID((*it).c_str());
          }
        found = true;
        break;
        }
      }
    if (!found)
      {
      if (checked)
        {
        // plot is checked but not currently in the LayoutPlot, add it
        this->PlotChartNode->AddAndObservePlotSeriesNodeID(dn->GetID());
        }
      }
  }
}

// --------------------------------------------------------------------------
void qDMMLPlotViewControllerWidgetPrivate::onPlotSeriesNodeAdded(vtkDMMLNode *node)
{
  Q_Q(qDMMLPlotViewControllerWidget);
  if (!this->PlotChartNode || !q->dmmlScene())
    {
    return;
    }
  vtkDMMLPlotSeriesNode *plotSeriesNode = vtkDMMLPlotSeriesNode::SafeDownCast(node);
  if (!plotSeriesNode)
    {
    return;
    }
  // Add the reference of the PlotSeriesNode in the active PlotChartNode
  this->PlotChartNode->AddAndObservePlotSeriesNodeID(plotSeriesNode->GetID());
}

// --------------------------------------------------------------------------
void qDMMLPlotViewControllerWidgetPrivate::onPlotTypeChanged(int plotType)
{
  if (!this->PlotChartNode)
    {
    return;
    }
  this->PlotChartNode->SetPropertyToAllPlotSeriesNodes(vtkDMMLPlotChartNode::PlotType,
    vtkDMMLPlotSeriesNode::GetPlotTypeAsString(plotType));
}

// --------------------------------------------------------------------------
void qDMMLPlotViewControllerWidgetPrivate::onInteractionModeChanged(int interactionMode)
{
  Q_Q(qDMMLPlotViewControllerWidget);
  if (!q->dmmlPlotViewNode())
    {
    return;
    }
  q->dmmlPlotViewNode()->SetInteractionMode(interactionMode);
}

// --------------------------------------------------------------------------
// qDMMLPlotViewControllerWidget methods

// --------------------------------------------------------------------------
qDMMLPlotViewControllerWidget::qDMMLPlotViewControllerWidget(QWidget* parentWidget)
  : Superclass(new qDMMLPlotViewControllerWidgetPrivate(*this), parentWidget)
{
  Q_D(qDMMLPlotViewControllerWidget);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLPlotViewControllerWidget::~qDMMLPlotViewControllerWidget()
{
  this->setDMMLScene(nullptr);
}

// --------------------------------------------------------------------------
void qDMMLPlotViewControllerWidget::setPlotView(qDMMLPlotView* view)
{
  Q_D(qDMMLPlotViewControllerWidget);
  d->PlotView = view;
}

//---------------------------------------------------------------------------
void qDMMLPlotViewControllerWidget::setViewLabel(const QString& newViewLabel)
{
  if (!this->dmmlPlotViewNode())
    {
    qCritical() << Q_FUNC_INFO << " failed: must set view node first";
    return;
    }
  this->dmmlPlotViewNode()->SetLayoutLabel(newViewLabel.toUtf8());
}

//---------------------------------------------------------------------------
QString qDMMLPlotViewControllerWidget::viewLabel()const
{
  if (!this->dmmlPlotViewNode())
    {
    qCritical() << Q_FUNC_INFO << " failed: must set view node first";
    return QString();
    }
  return this->dmmlPlotViewNode()->GetLayoutLabel();
}

//---------------------------------------------------------------------------
void qDMMLPlotViewControllerWidget::setDMMLPlotViewNode(
    vtkDMMLPlotViewNode * viewNode)
{
  Q_D(qDMMLPlotViewControllerWidget);
  this->setDMMLViewNode(viewNode);
}

//---------------------------------------------------------------------------
 vtkDMMLPlotViewNode* qDMMLPlotViewControllerWidget::dmmlPlotViewNode()const
{
  Q_D(const qDMMLPlotViewControllerWidget);
  return vtkDMMLPlotViewNode::SafeDownCast(this->dmmlViewNode());
}

//---------------------------------------------------------------------------
void qDMMLPlotViewControllerWidget::fitPlotToAxes()
{
  Q_D(qDMMLPlotViewControllerWidget);
  if(!d->PlotView)
    {
    return;
    }
  d->PlotView->fitToContent();
}

//---------------------------------------------------------------------------
void qDMMLPlotViewControllerWidget::onExportButton()
{
  Q_D(qDMMLPlotViewControllerWidget);
  if(!d->PlotView || !this->dmmlPlotViewNode())
    {
    return;
    }

  vtkDMMLPlotChartNode* dmmlPlotChartNode = d->GetPlotChartNodeFromView();

  QString name;
  if (dmmlPlotChartNode)
    {
    name = dmmlPlotChartNode->GetName();
    }

  QString fileName = QFileDialog::getSaveFileName(this, tr("Save as SVG"),
    name, tr("Scalable Vector Graphics (*.svg)"));
  if (!fileName.isEmpty())
    {
    d->PlotView->saveAsSVG(fileName);
    }
}

//---------------------------------------------------------------------------
void qDMMLPlotViewControllerWidget::updateWidgetFromDMML()
{
  Q_D(qDMMLPlotViewControllerWidget);

  if (!this->dmmlPlotViewNode() || !this->dmmlScene())
    {
    return;
    }

  d->ViewLabel->setText(this->dmmlPlotViewNode()->GetLayoutLabel());

  // PlotChartNode selector
  vtkDMMLPlotChartNode* dmmlPlotChartNode = d->GetPlotChartNodeFromView();

  if (dmmlPlotChartNode != d->PlotChartNode)
    {
    this->qvtkReconnect(d->PlotChartNode, dmmlPlotChartNode, vtkCommand::ModifiedEvent,
      this, SLOT(updateWidgetFromDMML()));
    d->PlotChartNode = dmmlPlotChartNode;
    }

  bool wasBlocked = d->plotChartComboBox->blockSignals(true);
  d->plotChartComboBox->setCurrentNode(dmmlPlotChartNode);
  d->plotChartComboBox->blockSignals(wasBlocked);

  d->plotTypeComboBox->setEnabled(dmmlPlotChartNode != nullptr);
  d->plotSeriesComboBox->setEnabled(dmmlPlotChartNode != nullptr);

  if (!dmmlPlotChartNode)
    {
    // Set the widgets to default states
    bool wasBlocked = d->plotTypeComboBox->blockSignals(true);
    d->plotTypeComboBox->setCurrentIndex(-1);
    d->plotTypeComboBox->blockSignals(wasBlocked);

    bool plotBlockSignals = d->plotSeriesComboBox->blockSignals(true);
    for (int idx = 0; idx < d->plotSeriesComboBox->nodeCount(); idx++)
      {
      d->plotSeriesComboBox->setCheckState(d->plotSeriesComboBox->nodeFromIndex(idx), Qt::Unchecked);
      }
    d->plotSeriesComboBox->blockSignals(plotBlockSignals);
    return;
    }

  // Plot series selector
  bool plotBlockSignals = d->plotSeriesComboBox->blockSignals(true);
  for (int idx = 0; idx < d->plotSeriesComboBox->nodeCount(); idx++)
    {
    vtkDMMLNode* plotSeriesNode = d->plotSeriesComboBox->nodeFromIndex(idx);
    Qt::CheckState checkState = Qt::Unchecked;
    if (plotSeriesNode && dmmlPlotChartNode->HasPlotSeriesNodeID(plotSeriesNode->GetID()))
      {
      checkState = Qt::Checked;
      }
    d->plotSeriesComboBox->setCheckState(plotSeriesNode, checkState);
    }
  d->plotSeriesComboBox->blockSignals(plotBlockSignals);

  d->actionShow_Grid->setChecked(dmmlPlotChartNode->GetGridVisibility());
  d->actionShow_Legend->setChecked(dmmlPlotChartNode->GetLegendVisibility());

  wasBlocked = d->interactionModeComboBox->blockSignals(true);
  d->interactionModeComboBox->setCurrentIndex(this->dmmlPlotViewNode()->GetInteractionMode());
  d->interactionModeComboBox->blockSignals(wasBlocked);

  // Show plot type if they are the same in all selected plot nodes.
  wasBlocked = d->plotTypeComboBox->blockSignals(true);
  std::string plotType;
  if (dmmlPlotChartNode->GetPropertyFromAllPlotSeriesNodes(vtkDMMLPlotChartNode::PlotType, plotType))
    {
    d->plotTypeComboBox->setCurrentIndex(vtkDMMLPlotSeriesNode::GetPlotTypeFromString(plotType.c_str()));
    }
  else
    {
    d->plotTypeComboBox->setCurrentIndex(-1);
    }
  d->plotTypeComboBox->blockSignals(wasBlocked);
}

//---------------------------------------------------------------------------
void qDMMLPlotViewControllerWidget::setDMMLScene(vtkDMMLScene* newScene)
{
  Q_D(qDMMLPlotViewControllerWidget);

  if (this->dmmlScene() == newScene)
    {
    return;
    }

   d->qvtkReconnect(this->dmmlScene(), newScene, vtkDMMLScene::EndBatchProcessEvent,
                    this, SLOT(updateWidgetFromDMML()));

  // Disable the node selectors as they would fire signal currentIndexChanged(0)
  // meaning that there is no current node anymore. It's not true, it just means
  // that the current node was not in the combo box list menu before.
  bool plotChartBlockSignals = d->plotChartComboBox->blockSignals(true);
  bool plotBlockSignals = d->plotSeriesComboBox->blockSignals(true);

  this->Superclass::setDMMLScene(newScene);

  d->plotChartComboBox->blockSignals(plotChartBlockSignals);
  d->plotSeriesComboBox->blockSignals(plotBlockSignals);

  if (this->dmmlScene())
    {
    this->updateWidgetFromDMML();
    }
}

// --------------------------------------------------------------------------
void qDMMLPlotViewControllerWidget::updateWidgetFromDMMLView()
{
  Superclass::updateWidgetFromDMMLView();
  this->updateWidgetFromDMML();
}
