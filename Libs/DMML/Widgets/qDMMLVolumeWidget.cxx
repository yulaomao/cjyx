/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QHBoxLayout>
#include <QMenu>
#include <QWidgetAction>

// CTK includes
#include <ctkUtils.h>

// qDMML includes
#include "qDMMLVolumeWidget.h"
#include "qDMMLVolumeWidget_p.h"
#include <qDMMLSpinBox.h>

// DMML includes
#include "vtkDMMLScalarVolumeNode.h"
#include "vtkDMMLScalarVolumeDisplayNode.h"

// VTK includes
#include <vtkImageData.h>

// --------------------------------------------------------------------------
qDMMLVolumeWidgetPrivate
::qDMMLVolumeWidgetPrivate(qDMMLVolumeWidget& object)
  : q_ptr(&object)
{
  this->VolumeNode = nullptr;
  this->VolumeDisplayNode = nullptr;
  this->OptionsMenu = nullptr;
  this->MinRangeSpinBox = nullptr;
  this->MaxRangeSpinBox = nullptr;
  this->DisplayScalarRange[0] = 0;
  this->DisplayScalarRange[1] = 0;
}

// --------------------------------------------------------------------------
qDMMLVolumeWidgetPrivate::~qDMMLVolumeWidgetPrivate()
{
  delete this->OptionsMenu;
  this->OptionsMenu = nullptr;
  this->MinRangeSpinBox = nullptr;
  this->MaxRangeSpinBox = nullptr;
}

// --------------------------------------------------------------------------
void qDMMLVolumeWidgetPrivate::init()
{
  Q_Q(qDMMLVolumeWidget);

  this->setParent(q);
  // disable as there is not DMML Node associated with the widget
  q->setEnabled(this->VolumeDisplayNode != nullptr);

  QWidget* rangeWidget = new QWidget(q);
  QHBoxLayout* rangeLayout = new QHBoxLayout;
  rangeWidget->setLayout(rangeLayout);
  rangeLayout->setContentsMargins(0,0,0,0);

  this->MinRangeSpinBox = new qDMMLSpinBox(rangeWidget);
  this->MinRangeSpinBox->setPrefix("Min: ");
  this->MinRangeSpinBox->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
  this->MinRangeSpinBox->setValue(this->MinRangeSpinBox->minimum());
  this->MinRangeSpinBox->setToolTip(
        qDMMLVolumeWidget::tr("Set the range boundaries to control large numbers or allow fine tuning"));
  connect(this->MinRangeSpinBox, SIGNAL(editingFinished()),
          this, SLOT(updateRangeFromSpinBox()));
  rangeLayout->addWidget(this->MinRangeSpinBox);

  this->MaxRangeSpinBox = new qDMMLSpinBox(rangeWidget);
  this->MaxRangeSpinBox->setPrefix("Max: ");
  this->MaxRangeSpinBox->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
  this->MaxRangeSpinBox->setValue(this->MaxRangeSpinBox->maximum());
  this->MaxRangeSpinBox->setToolTip(
        qDMMLVolumeWidget::tr("Set the range boundaries to control large numbers or allow fine tuning"));
  connect(this->MaxRangeSpinBox, SIGNAL(editingFinished()),
          this, SLOT(updateRangeFromSpinBox()));
  rangeLayout->addWidget(this->MaxRangeSpinBox);

  QWidgetAction* rangeAction = new QWidgetAction(this);
  rangeAction->setDefaultWidget(rangeWidget);

  this->OptionsMenu = new QMenu(q);
  this->OptionsMenu->addAction(rangeAction);

}

// --------------------------------------------------------------------------
bool qDMMLVolumeWidgetPrivate::blockSignals(bool block)
{
  return this->MinRangeSpinBox->blockSignals(block) && this->MaxRangeSpinBox->blockSignals(block);
}

// --------------------------------------------------------------------------
void qDMMLVolumeWidgetPrivate::updateSingleStep(double min, double max)
{
  double interval = max - min;
  int order = ctk::orderOfMagnitude(interval);
  if (order == std::numeric_limits<int>::min())
    {
    // the order of magnitude can't be computed (e.g. 0, inf, Nan, denorm)...
    order = -2;
    }

  int ratio = 2;
  double singleStep = pow(10., order - ratio);
  int decimals = qMax(0, -order + ratio);

  this->setDecimals(decimals);
  this->setSingleStep(singleStep);

  // The RangeWidget doesn't have to be as precise as the sliders/spinboxes.
  ratio = 1;
  singleStep = pow(10., order - ratio);
  decimals = qMax(0, -order + ratio);

  this->MinRangeSpinBox->setSingleStep(singleStep);
  this->MaxRangeSpinBox->setSingleStep(singleStep);
}

// --------------------------------------------------------------------------
void qDMMLVolumeWidgetPrivate::setDecimals(int decimals)
{
  Q_UNUSED(decimals);
}

// --------------------------------------------------------------------------
void qDMMLVolumeWidgetPrivate::setSingleStep(double singleStep)
{
  Q_UNUSED(singleStep);
}

// --------------------------------------------------------------------------
void qDMMLVolumeWidgetPrivate::setRange(double min, double max)
{
  this->updateSingleStep(min, max);
  this->MinRangeSpinBox->setValue(min);
  this->MaxRangeSpinBox->setValue(max);
}

// --------------------------------------------------------------------------
void qDMMLVolumeWidgetPrivate::updateRangeFromSpinBox()
{
  this->setRange(this->MinRangeSpinBox->value(),
                 this->MaxRangeSpinBox->value());
}

// --------------------------------------------------------------------------
qDMMLVolumeWidget::qDMMLVolumeWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qDMMLVolumeWidgetPrivate(*this))
{
  Q_D(qDMMLVolumeWidget);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLVolumeWidget
::qDMMLVolumeWidget(qDMMLVolumeWidgetPrivate* ptr, QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(ptr)
{
}

// --------------------------------------------------------------------------
qDMMLVolumeWidget::~qDMMLVolumeWidget() = default;

// --------------------------------------------------------------------------
void qDMMLVolumeWidget
::setDMMLVolumeDisplayNode(vtkDMMLScalarVolumeDisplayNode* node)
{
  Q_D(qDMMLVolumeWidget);
  if (d->VolumeDisplayNode == node)
    {
    return;
    }

  // each time the node is modified, the qt widgets are updated
  this->qvtkReconnect(d->VolumeDisplayNode, node, vtkCommand::ModifiedEvent,
                      this, SLOT(updateWidgetFromDMMLDisplayNode()));

  d->VolumeDisplayNode = node;

  this->updateWidgetFromDMMLDisplayNode();
}

// --------------------------------------------------------------------------
void qDMMLVolumeWidget::setDMMLVolumeNode(vtkDMMLNode* node)
{
  this->setDMMLVolumeNode(vtkDMMLScalarVolumeNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qDMMLVolumeWidget::setDMMLVolumeNode(vtkDMMLScalarVolumeNode* volumeNode)
{
  Q_D(qDMMLVolumeWidget);
  if (volumeNode == d->VolumeNode)
    {
    return;
    }

  this->qvtkReconnect(d->VolumeNode, volumeNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateWidgetFromDMMLVolumeNode()));

  d->VolumeNode = volumeNode;
  this->updateWidgetFromDMMLVolumeNode();
}

// --------------------------------------------------------------------------
vtkDMMLScalarVolumeNode* qDMMLVolumeWidget::dmmlVolumeNode()const
{
  Q_D(const qDMMLVolumeWidget);
  return d->VolumeNode;
}

// --------------------------------------------------------------------------
vtkDMMLScalarVolumeDisplayNode* qDMMLVolumeWidget::dmmlDisplayNode()const
{
  Q_D(const qDMMLVolumeWidget);
  return d->VolumeDisplayNode;
}

// --------------------------------------------------------------------------
void qDMMLVolumeWidget::updateWidgetFromDMMLVolumeNode()
{
  Q_D(qDMMLVolumeWidget);

  // Make sure the display node reference is up-to-date
  vtkDMMLScalarVolumeDisplayNode* newVolumeDisplayNode = d->VolumeNode ?
    vtkDMMLScalarVolumeDisplayNode::SafeDownCast(d->VolumeNode->GetVolumeDisplayNode()) : nullptr;
  this->setDMMLVolumeDisplayNode(newVolumeDisplayNode);

  // We always need to set the slider values and range at the same time
  // to make sure that they are consistent. This is implemented in one place,
  // in updateWidgetFromDMMLDisplayNode().
  this->updateWidgetFromDMMLDisplayNode();
}

// --------------------------------------------------------------------------
void qDMMLVolumeWidget::updateWidgetFromDMMLDisplayNode()
{
  Q_D(qDMMLVolumeWidget);
  this->setEnabled(d->VolumeDisplayNode != nullptr && d->VolumeNode != nullptr);
  if (d->VolumeDisplayNode && d->VolumeDisplayNode->GetInputImageData())
    {
    d->VolumeDisplayNode->GetDisplayScalarRange(d->DisplayScalarRange);
    }
  else
    {
    d->DisplayScalarRange[0] = 0.;
    d->DisplayScalarRange[1] = 0.;
    }
}
