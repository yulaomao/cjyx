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

// qDMML includes
#include "qDMMLVolumePropertyNodeWidget.h"
#include "ui_qDMMLVolumePropertyNodeWidget.h"

// DMML includes
#include <vtkDMMLVolumePropertyNode.h>

// VTK includes
#include <vtkVolumeProperty.h>
#include <vtkWeakPointer.h>

//-----------------------------------------------------------------------------
class qDMMLVolumePropertyNodeWidgetPrivate
  : public Ui_qDMMLVolumePropertyNodeWidget
{
  Q_DECLARE_PUBLIC(qDMMLVolumePropertyNodeWidget);

protected:
  qDMMLVolumePropertyNodeWidget* const q_ptr;

public:
  qDMMLVolumePropertyNodeWidgetPrivate(qDMMLVolumePropertyNodeWidget& object);
  virtual ~qDMMLVolumePropertyNodeWidgetPrivate();

  virtual void setupUi();

  vtkWeakPointer<vtkDMMLVolumePropertyNode> VolumePropertyNode;
};

// --------------------------------------------------------------------------
qDMMLVolumePropertyNodeWidgetPrivate::qDMMLVolumePropertyNodeWidgetPrivate(
  qDMMLVolumePropertyNodeWidget& object)
  : q_ptr(&object)
{
  this->VolumePropertyNode = nullptr;
}

// --------------------------------------------------------------------------
qDMMLVolumePropertyNodeWidgetPrivate::~qDMMLVolumePropertyNodeWidgetPrivate() = default;

// --------------------------------------------------------------------------
void qDMMLVolumePropertyNodeWidgetPrivate::setupUi()
{
  Q_Q(qDMMLVolumePropertyNodeWidget);
  this->Ui_qDMMLVolumePropertyNodeWidget::setupUi(q);
  QObject::connect(this->VolumePropertyWidget, SIGNAL(chartsExtentChanged()),
                   q, SIGNAL(chartsExtentChanged()));
  QObject::connect(this->VolumePropertyWidget, SIGNAL(thresholdEnabledChanged(bool)),
                   q, SIGNAL(thresholdChanged(bool)));
}

// --------------------------------------------------------------------------
// qDMMLVolumePropertyNodeWidget
// --------------------------------------------------------------------------
qDMMLVolumePropertyNodeWidget::qDMMLVolumePropertyNodeWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qDMMLVolumePropertyNodeWidgetPrivate(*this))
{
  Q_D(qDMMLVolumePropertyNodeWidget);
  d->setupUi();
}

// --------------------------------------------------------------------------
qDMMLVolumePropertyNodeWidget::~qDMMLVolumePropertyNodeWidget() = default;

// --------------------------------------------------------------------------
vtkVolumeProperty* qDMMLVolumePropertyNodeWidget::volumeProperty()const
{
  Q_D(const qDMMLVolumePropertyNodeWidget);
  return d->VolumePropertyWidget->volumeProperty();
}

// --------------------------------------------------------------------------
void qDMMLVolumePropertyNodeWidget::setDMMLVolumePropertyNode(
  vtkDMMLNode* volumePropertyNode)
{
  this->setDMMLVolumePropertyNode(
    vtkDMMLVolumePropertyNode::SafeDownCast(volumePropertyNode));
}

// --------------------------------------------------------------------------
void qDMMLVolumePropertyNodeWidget::setDMMLVolumePropertyNode(
  vtkDMMLVolumePropertyNode* volumePropertyNode)
{
   Q_D(qDMMLVolumePropertyNodeWidget);
   this->qvtkReconnect(d->VolumePropertyNode, volumePropertyNode,
                        vtkCommand::ModifiedEvent,
                        this, SLOT(updateFromVolumePropertyNode()));
   d->VolumePropertyNode = volumePropertyNode;
   this->updateFromVolumePropertyNode();
}

// --------------------------------------------------------------------------
void qDMMLVolumePropertyNodeWidget::updateFromVolumePropertyNode()
{
  Q_D(qDMMLVolumePropertyNodeWidget);
  vtkVolumeProperty* newVolumeProperty =
    d->VolumePropertyNode ? d->VolumePropertyNode->GetVolumeProperty() : nullptr;
  qvtkReconnect(d->VolumePropertyWidget->volumeProperty(), newVolumeProperty,
                vtkCommand::ModifiedEvent, this, SIGNAL(volumePropertyChanged()));
  d->VolumePropertyWidget->setVolumeProperty(newVolumeProperty);
}

// --------------------------------------------------------------------------
void qDMMLVolumePropertyNodeWidget::chartsBounds(double bounds[4])const
{
  Q_D(const qDMMLVolumePropertyNodeWidget);
  d->VolumePropertyWidget->chartsBounds(bounds);
}

// ----------------------------------------------------------------------------
void qDMMLVolumePropertyNodeWidget::setChartsExtent(double extent[2])
{
  Q_D(qDMMLVolumePropertyNodeWidget);
  d->VolumePropertyWidget->chartsExtent(extent);
}

// ----------------------------------------------------------------------------
void qDMMLVolumePropertyNodeWidget::setChartsExtent(double min, double max)
{
  Q_D(qDMMLVolumePropertyNodeWidget);
  d->VolumePropertyWidget->setChartsExtent(min, max);
}

// --------------------------------------------------------------------------
void qDMMLVolumePropertyNodeWidget::chartsExtent(double extent[4])const
{
  Q_D(const qDMMLVolumePropertyNodeWidget);
  d->VolumePropertyWidget->chartsExtent(extent);
}

// --------------------------------------------------------------------------
void qDMMLVolumePropertyNodeWidget::setThreshold(bool enable)
{
  Q_D(qDMMLVolumePropertyNodeWidget);
  d->VolumePropertyWidget->setThresholdEnabled(enable);
}

// --------------------------------------------------------------------------
bool qDMMLVolumePropertyNodeWidget::hasThreshold()const
{
  Q_D(const qDMMLVolumePropertyNodeWidget);
  return d->VolumePropertyWidget->isThresholdEnabled();
}

// --------------------------------------------------------------------------
void qDMMLVolumePropertyNodeWidget::moveAllPoints(double x, double y, bool dontMoveFirstAndLast)
{
  Q_D(const qDMMLVolumePropertyNodeWidget);
  return d->VolumePropertyWidget->moveAllPoints(x, y, dontMoveFirstAndLast);
}

// --------------------------------------------------------------------------
void qDMMLVolumePropertyNodeWidget::spreadAllPoints(double factor, bool dontSpreadFirstAndLast)
{
  Q_D(const qDMMLVolumePropertyNodeWidget);
  return d->VolumePropertyWidget->spreadAllPoints(factor, dontSpreadFirstAndLast);
}
