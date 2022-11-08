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
#include "qDMMLROIWidget.h"
#include "ui_qDMMLROIWidget.h"

// DMML includes
#include <vtkDMMLROINode.h>

// --------------------------------------------------------------------------
class qDMMLROIWidgetPrivate: public Ui_qDMMLROIWidget
{
  Q_DECLARE_PUBLIC(qDMMLROIWidget);
protected:
  qDMMLROIWidget* const q_ptr;
public:
  qDMMLROIWidgetPrivate(qDMMLROIWidget& object);
  void init();
  vtkDMMLROINode* ROINode;
};

// --------------------------------------------------------------------------
qDMMLROIWidgetPrivate::qDMMLROIWidgetPrivate(qDMMLROIWidget& object)
  : q_ptr(&object)
{
  this->ROINode = nullptr;
}

// --------------------------------------------------------------------------
void qDMMLROIWidgetPrivate::init()
{
  Q_Q(qDMMLROIWidget);
  this->setupUi(q);
  QObject::connect(this->DisplayClippingBoxButton, SIGNAL(toggled(bool)),
                   q, SLOT(setDisplayClippingBox(bool)));
  QObject::connect(this->InteractiveModeCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setInteractiveMode(bool)));
  QObject::connect(this->LRRangeWidget, SIGNAL(valuesChanged(double,double)),
                   q, SLOT(updateROI()));
  QObject::connect(this->PARangeWidget, SIGNAL(valuesChanged(double,double)),
                   q, SLOT(updateROI()));
  QObject::connect(this->ISRangeWidget, SIGNAL(valuesChanged(double,double)),
                   q, SLOT(updateROI()));
  q->setEnabled(this->ROINode != nullptr);
}

// --------------------------------------------------------------------------
// qDMMLROIWidget methods

// --------------------------------------------------------------------------
qDMMLROIWidget::qDMMLROIWidget(QWidget* _parent)
  : QWidget(_parent)
  , d_ptr(new qDMMLROIWidgetPrivate(*this))
{
  Q_D(qDMMLROIWidget);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLROIWidget::~qDMMLROIWidget() = default;

// --------------------------------------------------------------------------
vtkDMMLROINode* qDMMLROIWidget::dmmlROINode()const
{
  Q_D(const qDMMLROIWidget);
  return d->ROINode;
}

// --------------------------------------------------------------------------
void qDMMLROIWidget::setDMMLROINode(vtkDMMLROINode* roiNode)
{
  Q_D(qDMMLROIWidget);
  qvtkReconnect(d->ROINode, roiNode, vtkCommand::ModifiedEvent,
                this, SLOT(onDMMLNodeModified()));

  d->ROINode = roiNode;
  this->onDMMLNodeModified();
  this->setEnabled(roiNode != nullptr);
}

// --------------------------------------------------------------------------
void qDMMLROIWidget::setDMMLROINode(vtkDMMLNode* roiNode)
{
  this->setDMMLROINode(vtkDMMLROINode::SafeDownCast(roiNode));
}

// --------------------------------------------------------------------------
void qDMMLROIWidget::onDMMLNodeModified()
{
  Q_D(qDMMLROIWidget);
  if (!d->ROINode)
    {
    return;
    }
  // Visibility
  d->DisplayClippingBoxButton->setChecked(d->ROINode->GetVisibility());

  // Interactive Mode
  bool interactive = d->ROINode->GetInteractiveMode();
  d->LRRangeWidget->setTracking(interactive);
  d->PARangeWidget->setTracking(interactive);
  d->ISRangeWidget->setTracking(interactive);
  d->InteractiveModeCheckBox->setChecked(interactive);

  // ROI
  double *xyz = d->ROINode->GetXYZ();
  double *rxyz = d->ROINode->GetRadiusXYZ();
  double bounds[6];
  for (int i=0; i < 3; ++i)
    {
    bounds[i]   = xyz[i]-rxyz[i];
    bounds[3+i] = xyz[i]+rxyz[i];
    }
  d->LRRangeWidget->setValues(bounds[0], bounds[3]);
  d->PARangeWidget->setValues(bounds[1], bounds[4]);
  d->ISRangeWidget->setValues(bounds[2], bounds[5]);
}

// --------------------------------------------------------------------------
void qDMMLROIWidget::setExtent(double min, double max)
{
  Q_D(qDMMLROIWidget);
  d->LRRangeWidget->setRange(min, max);
  d->PARangeWidget->setRange(min, max);
  d->ISRangeWidget->setRange(min, max);
}

// --------------------------------------------------------------------------
void qDMMLROIWidget::setDisplayClippingBox(bool visible)
{
  Q_D(qDMMLROIWidget);
  d->ROINode->SetVisibility(visible);
}

// --------------------------------------------------------------------------
void qDMMLROIWidget::setInteractiveMode(bool interactive)
{
  Q_D(qDMMLROIWidget);
  d->ROINode->SetInteractiveMode(interactive);
}

// --------------------------------------------------------------------------
void qDMMLROIWidget::updateROI()
{
  Q_D(qDMMLROIWidget);
  double bounds[6];
  d->LRRangeWidget->values(bounds[0],bounds[1]);
  d->PARangeWidget->values(bounds[2],bounds[3]);
  d->ISRangeWidget->values(bounds[4],bounds[5]);

  int disabledModify = d->ROINode->StartModify();

  d->ROINode->SetXYZ(0.5*(bounds[1]+bounds[0]),
                     0.5*(bounds[3]+bounds[2]),
                     0.5*(bounds[5]+bounds[4]));
  d->ROINode->SetRadiusXYZ(0.5*(bounds[1]-bounds[0]),
                           0.5*(bounds[3]-bounds[2]),
                           0.5*(bounds[5]-bounds[4]));
  d->ROINode->EndModify(disabledModify);
}
