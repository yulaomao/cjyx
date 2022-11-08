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
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QLineEdit>

// qDMML includes
#include "qDMMLPlotSeriesPropertiesWidget_p.h"

// DMML includes
#include <vtkDMMLColorNode.h>
#include <vtkDMMLPlotChartNode.h>
#include <vtkDMMLPlotSeriesNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLTableNode.h>

// VTK includes
#include <vtkPen.h>
#include <vtkPlot.h>
#include <vtkPlotLine.h>

// stream includes
#include <sstream>

//--------------------------------------------------------------------------
// qDMMLPlotSeriesPropertiesWidgetPrivate methods

//---------------------------------------------------------------------------
qDMMLPlotSeriesPropertiesWidgetPrivate::qDMMLPlotSeriesPropertiesWidgetPrivate(qDMMLPlotSeriesPropertiesWidget& object)
  : q_ptr(&object)
{
  this->PlotSeriesNode = nullptr;
}

//---------------------------------------------------------------------------
qDMMLPlotSeriesPropertiesWidgetPrivate::~qDMMLPlotSeriesPropertiesWidgetPrivate() = default;

//---------------------------------------------------------------------------
void qDMMLPlotSeriesPropertiesWidgetPrivate::setupUi(qDMMLWidget* widget)
{
  Q_Q(qDMMLPlotSeriesPropertiesWidget);

  this->Ui_qDMMLPlotSeriesPropertiesWidget::setupUi(widget);

  this->connect(this->inputTableComboBox, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
                this, SLOT(onInputTableNodeChanged(vtkDMMLNode*)));
  this->connect(this->xAxisComboBox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(onXAxisChanged(int)));
  this->connect(this->labelsComboBox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(onLabelsChanged(int)));
  this->connect(this->yAxisComboBox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(onYAxisChanged(int)));
  this->connect(this->plotTypeComboBox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(onPlotTypeChanged(int)));
  this->connect(this->markersStyleComboBox, SIGNAL(currentIndexChanged(const QString&)),
                this, SLOT(onMarkersStyleChanged(const QString&)));
  this->connect(this->markersSizeDoubleSpinBox, SIGNAL(valueChanged(double)),
                this, SLOT(onMarkersSizeChanged(double)));
  this->connect(this->lineStyleComboBox, SIGNAL(currentIndexChanged(const QString&)),
                this, SLOT(onLineStyleChanged(const QString&)));
  this->connect(this->lineWidthDoubleSpinBox, SIGNAL(valueChanged(double)),
                this, SLOT(onLineWidthChanged(double)));
  this->connect(this->plotSeriesColorPickerButton, SIGNAL(colorChanged(const QColor&)),
                this, SLOT(onPlotSeriesColorChanged(const QColor&)));
}

// --------------------------------------------------------------------------
void qDMMLPlotSeriesPropertiesWidgetPrivate::updateWidgetFromDMML()
{
  Q_Q(qDMMLPlotSeriesPropertiesWidget);

  q->setEnabled(this->PlotSeriesNode != nullptr);

  if (!this->PlotSeriesNode)
    {
    this->inputTableComboBox->setCurrentNode(nullptr);
    this->xAxisComboBox->clear();
    this->labelsComboBox->clear();
    this->yAxisComboBox->clear();
    this->plotTypeComboBox->setCurrentIndex(0);
    this->markersStyleComboBox->setCurrentIndex(0);
    this->plotSeriesColorPickerButton->setColor(QColor(127,127,127));
    return;
    }

  // Update the TableNode ComboBox
  vtkDMMLTableNode* dmmlTableNode = this->PlotSeriesNode->GetTableNode();
  this->inputTableComboBox->setCurrentNode(dmmlTableNode);

  // Update the xAxis and yAxis ComboBoxes
  bool xAxisBlockSignals = this->xAxisComboBox->blockSignals(true);
  bool labelsBlockSignals = this->labelsComboBox->blockSignals(true);
  bool yAxisBlockSignals = this->yAxisComboBox->blockSignals(true);

  bool xColumnRequired = this->PlotSeriesNode->IsXColumnRequired();

  this->xAxisComboBox->clear();
  this->labelsComboBox->clear();
  this->yAxisComboBox->clear();
  if (dmmlTableNode)
    {
    if (this->labelsComboBox->findData(QString()) == -1)
      {
      this->labelsComboBox->addItem("(none)", QString());
      }
    for (int columnIndex = 0; columnIndex < dmmlTableNode->GetNumberOfColumns(); columnIndex++)
      {
      std::string columnName = dmmlTableNode->GetColumnName(columnIndex);
      int columnType = dmmlTableNode->GetColumnType(columnName);
      if (columnType == VTK_STRING)
        {
        if (this->labelsComboBox->findData(QString(columnName.c_str())) == -1)
          {
          this->labelsComboBox->addItem(columnName.c_str(), QString(columnName.c_str()));
          }
        }
      else if (columnType != VTK_BIT)
        {
        if (xColumnRequired && this->xAxisComboBox->findData(QString(columnName.c_str())) == -1)
          {
          this->xAxisComboBox->addItem(columnName.c_str(), QString(columnName.c_str()));
          }
        if (this->yAxisComboBox->findData(QString(columnName.c_str())) == -1)
          {
          this->yAxisComboBox->addItem(columnName.c_str(), QString(columnName.c_str()));
          }
        }
      }
    }

  if (xColumnRequired)
    {
    std::string xAxisName = this->PlotSeriesNode->GetXColumnName();
    int xAxisIndex = this->xAxisComboBox->findData(QString(xAxisName.c_str()));
    if (xAxisIndex < 0)
      {
      this->xAxisComboBox->addItem(xAxisName.c_str(), QString(xAxisName.c_str()));
      xAxisIndex = this->xAxisComboBox->findData(QString(xAxisName.c_str()));
      }
    this->xAxisComboBox->setCurrentIndex(xAxisIndex);
    this->xAxisComboBox->setToolTip("");
    }
  else
    {
    this->xAxisComboBox->addItem("(Indexes)", QString());
    this->xAxisComboBox->setCurrentIndex(0);
    this->xAxisComboBox->setToolTip(tr("This plot type uses indexes as X axis values. Switch to scatter plot type to allow column selection."));
    }

  std::string labelsName = this->PlotSeriesNode->GetLabelColumnName();
  int labelsIndex = this->labelsComboBox->findData(QString(labelsName.c_str()));
  if (labelsIndex < 0)
    {
    this->labelsComboBox->addItem(labelsName.c_str(), QString(labelsName.c_str()));
    labelsIndex = this->labelsComboBox->findData(QString(labelsName.c_str()));
    }
  this->labelsComboBox->setCurrentIndex(labelsIndex);
  this->labelsComboBox->setEnabled(dmmlTableNode != nullptr);

  std::string yAxisName = this->PlotSeriesNode->GetYColumnName();
  int yAxisIndex = this->yAxisComboBox->findData(QString(yAxisName.c_str()));
  if (yAxisIndex < 0)
    {
    this->yAxisComboBox->addItem(yAxisName.c_str(), QString(yAxisName.c_str()));
    yAxisIndex = this->yAxisComboBox->findData(QString(yAxisName.c_str()));
    }
  this->yAxisComboBox->setCurrentIndex(yAxisIndex);

  this->xAxisComboBox->blockSignals(xAxisBlockSignals);
  this->labelsComboBox->blockSignals(labelsBlockSignals);
  this->yAxisComboBox->blockSignals(yAxisBlockSignals);
  this->xAxisComboBox->setEnabled(dmmlTableNode != nullptr && this->PlotSeriesNode->IsXColumnRequired());

  this->yAxisComboBox->setEnabled(dmmlTableNode != nullptr);

  // Update the PlotType ComboBox
  bool wasBlocked = this->plotTypeComboBox->blockSignals(true);
  this->plotTypeComboBox->setCurrentIndex(this->PlotSeriesNode->GetPlotType());
  this->plotTypeComboBox->blockSignals(wasBlocked);

  // Update Markers Style
  wasBlocked = this->markersStyleComboBox->blockSignals(true);
  const char* plotMarkersStyle = this->PlotSeriesNode->GetMarkerStyleAsString(this->PlotSeriesNode->GetMarkerStyle());
  // After Qt5 migration, the next line can be replaced by this call:
  // this->markersStyleComboBox->setCurrentText(plotMarkersStyle);
  this->markersStyleComboBox->setCurrentIndex(this->markersStyleComboBox->findText(plotMarkersStyle));
  this->markersStyleComboBox->setEnabled(
    this->PlotSeriesNode->GetPlotType() == vtkDMMLPlotSeriesNode::PlotTypeScatter
    || this->PlotSeriesNode->GetPlotType() == vtkDMMLPlotSeriesNode::PlotTypeLine);
  this->markersStyleComboBox->blockSignals(wasBlocked);

  // Update Markers Size
  wasBlocked = this->markersSizeDoubleSpinBox->blockSignals(true);
  this->markersSizeDoubleSpinBox->setValue(this->PlotSeriesNode->GetMarkerSize());
  this->markersSizeDoubleSpinBox->setEnabled(
    this->PlotSeriesNode->GetPlotType() == vtkDMMLPlotSeriesNode::PlotTypeScatter
    || this->PlotSeriesNode->GetPlotType() == vtkDMMLPlotSeriesNode::PlotTypeLine);
  this->markersSizeDoubleSpinBox->blockSignals(wasBlocked);

  // Update Line Style
  wasBlocked = this->lineStyleComboBox->blockSignals(true);
  const char* plotLineStyle = this->PlotSeriesNode->GetLineStyleAsString(this->PlotSeriesNode->GetLineStyle());
  // After Qt5 migration, the next line can be replaced by this call:
  // this->markersStyleComboBox->setCurrentText(plotMarkersStyle);
  this->lineStyleComboBox->setCurrentIndex(this->lineStyleComboBox->findText(plotLineStyle));
  this->lineStyleComboBox->setEnabled(
    this->PlotSeriesNode->GetPlotType() == vtkDMMLPlotSeriesNode::PlotTypeScatter
    || this->PlotSeriesNode->GetPlotType() == vtkDMMLPlotSeriesNode::PlotTypeLine);
  this->lineStyleComboBox->blockSignals(wasBlocked);

  // Update Line Width
  wasBlocked = this->lineWidthDoubleSpinBox->blockSignals(true);
  this->lineWidthDoubleSpinBox->setValue(this->PlotSeriesNode->GetLineWidth());
  this->lineWidthDoubleSpinBox->setEnabled(
    this->PlotSeriesNode->GetPlotType() == vtkDMMLPlotSeriesNode::PlotTypeScatter
    || this->PlotSeriesNode->GetPlotType() == vtkDMMLPlotSeriesNode::PlotTypeLine);
  this->lineWidthDoubleSpinBox->blockSignals(wasBlocked);

  // Update PlotSeriesColorPickerButton
  wasBlocked = this->plotSeriesColorPickerButton->blockSignals(true);
  double* rgb = this->PlotSeriesNode->GetColor();
  QColor color;
  color.setRedF(rgb[0]);
  color.setGreenF(rgb[1]);
  color.setBlueF(rgb[2]);
  color.setAlphaF(this->PlotSeriesNode->GetOpacity());
  this->plotSeriesColorPickerButton->setColor(color);
  this->plotSeriesColorPickerButton->blockSignals(wasBlocked);
}

// --------------------------------------------------------------------------
void qDMMLPlotSeriesPropertiesWidgetPrivate::onPlotSeriesNodeChanged(vtkDMMLNode *node)
{
  vtkDMMLPlotSeriesNode *dmmlPlotSeriesNode = vtkDMMLPlotSeriesNode::SafeDownCast(node);

  if (this->PlotSeriesNode == dmmlPlotSeriesNode)
    {
    return;
    }

  this->qvtkReconnect(this->PlotSeriesNode, dmmlPlotSeriesNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateWidgetFromDMMLPlotSeriesNode()));

  this->PlotSeriesNode = dmmlPlotSeriesNode;

  this->updateWidgetFromDMML();
}

// --------------------------------------------------------------------------
void qDMMLPlotSeriesPropertiesWidgetPrivate::onInputTableNodeChanged(vtkDMMLNode *node)
{
  vtkDMMLTableNode *dmmlTableNode = vtkDMMLTableNode::SafeDownCast(node);

  if (!this->PlotSeriesNode || this->PlotSeriesNode->GetTableNode() == dmmlTableNode)
    {
    return;
    }

  if (dmmlTableNode)
    {
    this->PlotSeriesNode->SetAndObserveTableNodeID(dmmlTableNode->GetID());
    }
  else
    {
    this->PlotSeriesNode->SetAndObserveTableNodeID(nullptr);
    }
}

// --------------------------------------------------------------------------
void qDMMLPlotSeriesPropertiesWidgetPrivate::onXAxisChanged(int index)
{
  if (!this->PlotSeriesNode)
    {
    return;
    }

  this->PlotSeriesNode->SetXColumnName(this->xAxisComboBox->itemData(index).toString().toUtf8().constData());
}

// --------------------------------------------------------------------------
void qDMMLPlotSeriesPropertiesWidgetPrivate::onLabelsChanged(int index)
{
  if (!this->PlotSeriesNode)
  {
    return;
  }

  this->PlotSeriesNode->SetLabelColumnName(this->labelsComboBox->itemData(index).toString().toUtf8().constData());
}

// --------------------------------------------------------------------------
void qDMMLPlotSeriesPropertiesWidgetPrivate::onYAxisChanged(int index)
{
  if (!this->PlotSeriesNode)
    {
    return;
    }

  this->PlotSeriesNode->SetYColumnName(this->yAxisComboBox->itemData(index).toString().toUtf8().constData());
}

// --------------------------------------------------------------------------
void qDMMLPlotSeriesPropertiesWidgetPrivate::onPlotTypeChanged(int index)
{
  if (!this->PlotSeriesNode)
    {
    return;
    }

  this->PlotSeriesNode->SetPlotType(index);
}

// --------------------------------------------------------------------------
void qDMMLPlotSeriesPropertiesWidgetPrivate::onMarkersStyleChanged(const QString &style)
{
  if (!this->PlotSeriesNode)
    {
    return;
    }

  this->PlotSeriesNode->SetMarkerStyle(this->PlotSeriesNode->
    GetMarkerStyleFromString(style.toStdString().c_str()));
}

// --------------------------------------------------------------------------
void qDMMLPlotSeriesPropertiesWidgetPrivate::onMarkersSizeChanged(double size)
{
  if (!this->PlotSeriesNode)
    {
    return;
    }

  this->PlotSeriesNode->SetMarkerSize(size);
}

// --------------------------------------------------------------------------
void qDMMLPlotSeriesPropertiesWidgetPrivate::onLineStyleChanged(const QString &style)
{
  if (!this->PlotSeriesNode)
    {
    return;
    }

  this->PlotSeriesNode->SetLineStyle(this->PlotSeriesNode->
    GetLineStyleFromString(style.toStdString().c_str()));
}

// --------------------------------------------------------------------------
void qDMMLPlotSeriesPropertiesWidgetPrivate::onLineWidthChanged(double width)
{
  if (!this->PlotSeriesNode)
    {
    return;
    }

  this->PlotSeriesNode->SetLineWidth(width);
}

// --------------------------------------------------------------------------
void qDMMLPlotSeriesPropertiesWidgetPrivate::onPlotSeriesColorChanged(const QColor & color)
{
  if (!this->PlotSeriesNode)
    {
    return;
    }
  double rgb[3] = { color.redF() , color.greenF() , color.blueF() };
  this->PlotSeriesNode->SetColor(rgb);
  this->PlotSeriesNode->SetOpacity(color.alphaF());
}

// --------------------------------------------------------------------------
// qDMMLPlotViewView methods

// --------------------------------------------------------------------------
qDMMLPlotSeriesPropertiesWidget::qDMMLPlotSeriesPropertiesWidget(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qDMMLPlotSeriesPropertiesWidgetPrivate(*this))
{
  Q_D(qDMMLPlotSeriesPropertiesWidget);
  d->setupUi(this);
  this->setEnabled(false);
}

// --------------------------------------------------------------------------
qDMMLPlotSeriesPropertiesWidget::~qDMMLPlotSeriesPropertiesWidget() = default;


//---------------------------------------------------------------------------
 vtkDMMLPlotSeriesNode* qDMMLPlotSeriesPropertiesWidget::dmmlPlotSeriesNode()const
{
  Q_D(const qDMMLPlotSeriesPropertiesWidget);
  return d->PlotSeriesNode;
}

//---------------------------------------------------------------------------
void qDMMLPlotSeriesPropertiesWidget::setDMMLPlotSeriesNode(vtkDMMLNode* node)
{
  Q_D(qDMMLPlotSeriesPropertiesWidget);
  vtkDMMLPlotSeriesNode* plotSeriesNode = vtkDMMLPlotSeriesNode::SafeDownCast(node);
  this->setDMMLPlotSeriesNode(plotSeriesNode);
}

//---------------------------------------------------------------------------
void qDMMLPlotSeriesPropertiesWidget::setDMMLPlotSeriesNode(vtkDMMLPlotSeriesNode* plotSeriesNode)
{
  Q_D(qDMMLPlotSeriesPropertiesWidget);
  if (plotSeriesNode == d->PlotSeriesNode)
    {
    return;
    }

  d->qvtkReconnect(d->PlotSeriesNode, plotSeriesNode, vtkCommand::ModifiedEvent, d, SLOT(updateWidgetFromDMML()));
  d->PlotSeriesNode = plotSeriesNode;

  d->updateWidgetFromDMML();
}
