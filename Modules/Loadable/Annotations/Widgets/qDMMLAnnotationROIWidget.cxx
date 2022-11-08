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
#include "qDMMLAnnotationROIWidget.h"
#include "ui_qDMMLAnnotationROIWidget.h"

// DMML includes
#include <vtkDMMLAnnotationROINode.h>
#include <vtkDMMLDisplayNode.h>

// STD includes
#include <vector>

// 0.001 because the sliders only handle 2 decimals
#define SLIDERS_EPSILON 0.001

// --------------------------------------------------------------------------
class qDMMLAnnotationROIWidgetPrivate: public Ui_qDMMLAnnotationROIWidget
{
  Q_DECLARE_PUBLIC(qDMMLAnnotationROIWidget);
protected:
  qDMMLAnnotationROIWidget* const q_ptr;
public:
  qDMMLAnnotationROIWidgetPrivate(qDMMLAnnotationROIWidget& object);
  void init();

  vtkDMMLAnnotationROINode* ROINode;
  bool IsProcessingOnDMMLNodeModified;
  bool AutoRange;
};

// --------------------------------------------------------------------------
qDMMLAnnotationROIWidgetPrivate::qDMMLAnnotationROIWidgetPrivate(qDMMLAnnotationROIWidget& object)
  : q_ptr(&object)
{
  this->ROINode = nullptr;
  this->IsProcessingOnDMMLNodeModified = false;
  this->AutoRange = true;
}

// --------------------------------------------------------------------------
void qDMMLAnnotationROIWidgetPrivate::init()
{
  Q_Q(qDMMLAnnotationROIWidget);
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
// qDMMLAnnotationROIWidget methods

// --------------------------------------------------------------------------
qDMMLAnnotationROIWidget::qDMMLAnnotationROIWidget(QWidget* _parent)
  : QWidget(_parent)
  , d_ptr(new qDMMLAnnotationROIWidgetPrivate(*this))
{
  Q_D(qDMMLAnnotationROIWidget);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLAnnotationROIWidget::~qDMMLAnnotationROIWidget() = default;

// --------------------------------------------------------------------------
vtkDMMLAnnotationROINode* qDMMLAnnotationROIWidget::dmmlROINode()const
{
  Q_D(const qDMMLAnnotationROIWidget);
  return d->ROINode;
}

// --------------------------------------------------------------------------
void qDMMLAnnotationROIWidget::setDMMLAnnotationROINode(vtkDMMLAnnotationROINode* roiNode)
{
  Q_D(qDMMLAnnotationROIWidget);

  this->qvtkReconnect(d->ROINode, roiNode, vtkCommand::ModifiedEvent,
                this, SLOT(onDMMLNodeModified()));

  this->qvtkReconnect(d->ROINode, roiNode, vtkDMMLDisplayableNode::DisplayModifiedEvent,
                      this, SLOT(onDMMLDisplayNodeModified()));

  d->ROINode = roiNode;

  this->onDMMLNodeModified();
  this->onDMMLDisplayNodeModified();
  this->setEnabled(roiNode != nullptr);
}

// --------------------------------------------------------------------------
void qDMMLAnnotationROIWidget::setDMMLAnnotationROINode(vtkDMMLNode* roiNode)
{
  this->setDMMLAnnotationROINode(vtkDMMLAnnotationROINode::SafeDownCast(roiNode));
}

// --------------------------------------------------------------------------
void qDMMLAnnotationROIWidget::onDMMLNodeModified()
{
  Q_D(qDMMLAnnotationROIWidget);

  if (!d->ROINode)
    {
    return;
    }

  d->IsProcessingOnDMMLNodeModified = true;

  // Interactive Mode
  bool interactive = d->ROINode->GetInteractiveMode();
  d->LRRangeWidget->setTracking(interactive);
  d->PARangeWidget->setTracking(interactive);
  d->ISRangeWidget->setTracking(interactive);
  d->InteractiveModeCheckBox->setChecked(interactive);

  // ROI
  double xyz[3];
  double rxyz[3];

  d->ROINode->GetXYZ(xyz);
  d->ROINode->GetRadiusXYZ(rxyz);

  double bounds[6];
  for (int i=0; i < 3; ++i)
    {
    bounds[i]   = xyz[i]-rxyz[i];
    bounds[3+i] = xyz[i]+rxyz[i];
    }

  if (d->AutoRange)
    {
    d->LRRangeWidget->setRange(
      qMin(bounds[0], d->LRRangeWidget->minimum()),
      qMax(bounds[3], d->LRRangeWidget->maximum()));
    d->PARangeWidget->setRange(
      qMin(bounds[1], d->PARangeWidget->minimum()),
      qMax(bounds[4], d->PARangeWidget->maximum()));
    d->ISRangeWidget->setRange(
      qMin(bounds[2], d->ISRangeWidget->minimum()),
      qMax(bounds[5], d->ISRangeWidget->maximum()));
    }

  d->LRRangeWidget->setValues(bounds[0], bounds[3]);
  d->PARangeWidget->setValues(bounds[1], bounds[4]);
  d->ISRangeWidget->setValues(bounds[2], bounds[5]);

  d->IsProcessingOnDMMLNodeModified = false;
}

// --------------------------------------------------------------------------
void qDMMLAnnotationROIWidget::setExtent(double min, double max)
{
  this->setExtent(min, max, min, max, min, max);
}

// --------------------------------------------------------------------------
void qDMMLAnnotationROIWidget::setExtent(double minLR, double maxLR,
                                         double minPA, double maxPA,
                                         double minIS, double maxIS)
{
  Q_D(qDMMLAnnotationROIWidget);
  d->LRRangeWidget->setRange(minLR, maxLR);
  d->PARangeWidget->setRange(minPA, maxPA);
  d->ISRangeWidget->setRange(minIS, maxIS);
}

// --------------------------------------------------------------------------
void qDMMLAnnotationROIWidget::setDisplayClippingBox(bool visible)
{
  Q_D(qDMMLAnnotationROIWidget);

  int numberOfDisplayNodes = d->ROINode->GetNumberOfDisplayNodes();

  std::vector<int> wasModifying(numberOfDisplayNodes);
  for(int index = 0; index < numberOfDisplayNodes; index++)
    {
    wasModifying[index] = d->ROINode->GetNthDisplayNode(index)->StartModify();
    }

  d->ROINode->SetDisplayVisibility(visible);

  for(int index = 0; index < numberOfDisplayNodes; index++)
    {
    d->ROINode->GetNthDisplayNode(index)->EndModify(wasModifying[index]);
    }
}

// --------------------------------------------------------------------------
void qDMMLAnnotationROIWidget::setInteractiveMode(bool interactive)
{
  Q_D(qDMMLAnnotationROIWidget);
  d->ROINode->SetInteractiveMode(interactive);
}

// --------------------------------------------------------------------------
void qDMMLAnnotationROIWidget::updateROI()
{
  Q_D(qDMMLAnnotationROIWidget);

  // Ignore the calls from onDMMLNodeModified() as it
  // could set the node in an inconsistent state (except for
  // ISRangeWidget->setValues()).
  if (d->IsProcessingOnDMMLNodeModified)
    {
    return;
    }

  double bounds[6];
  d->LRRangeWidget->values(bounds[0],bounds[1]);
  d->PARangeWidget->values(bounds[2],bounds[3]);
  d->ISRangeWidget->values(bounds[4],bounds[5]);

  int wasModifying = d->ROINode->StartModify();
  d->ROINode->SetXYZ(0.5*(bounds[1]+bounds[0]),
                     0.5*(bounds[3]+bounds[2]),
                     0.5*(bounds[5]+bounds[4]));
  d->ROINode->SetRadiusXYZ(0.5*(bounds[1]-bounds[0]),
                           0.5*(bounds[3]-bounds[2]),
                           0.5*(bounds[5]-bounds[4]));
  d->ROINode->EndModify(wasModifying);
}

// --------------------------------------------------------------------------
void qDMMLAnnotationROIWidget::onDMMLDisplayNodeModified()
{
  Q_D(qDMMLAnnotationROIWidget);

  if (!d->ROINode)
    {
    return;
    }

  // Visibility
  d->DisplayClippingBoxButton->setChecked(d->ROINode->GetDisplayVisibility());
}
