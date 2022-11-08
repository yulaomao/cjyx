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

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

  ==============================================================================*/

// Markups widgets includes
// qDMML includes
#include "qDMMLMarkupsROIWidget.h"
#include "ui_qDMMLMarkupsROIWidget.h"

// DMML includes
#include <vtkDMMLMarkupsROINode.h>
#include <vtkDMMLMarkupsROIDisplayNode.h>

// VTK includes
#include <vtkWeakPointer.h>

// STD includes
#include <vector>

// 0.001 because the sliders only handle 2 decimals
#define SLIDERS_EPSILON 0.001

// --------------------------------------------------------------------------
class qDMMLMarkupsROIWidgetPrivate:
  public Ui_qDMMLMarkupsROIWidget
{

public:
  qDMMLMarkupsROIWidgetPrivate(qDMMLMarkupsROIWidget &widget);
  void setupUi(qDMMLMarkupsROIWidget* widget);

  bool IsProcessingOnDMMLNodeModified;
  bool AutoRange;

protected:
  qDMMLMarkupsROIWidget* const q_ptr;

private:
  Q_DECLARE_PUBLIC(qDMMLMarkupsROIWidget);

};

// --------------------------------------------------------------------------
qDMMLMarkupsROIWidgetPrivate::qDMMLMarkupsROIWidgetPrivate(qDMMLMarkupsROIWidget& widget)
  : q_ptr(&widget)
{
  this->IsProcessingOnDMMLNodeModified = false;
  this->AutoRange = true;
}

// --------------------------------------------------------------------------
void qDMMLMarkupsROIWidgetPrivate::setupUi(qDMMLMarkupsROIWidget* widget)
{
  Q_Q(qDMMLMarkupsROIWidget);

  this->Ui_qDMMLMarkupsROIWidget::setupUi(widget);

  this->roiTypeComboBox->clear();
  for (int roiType = 0; roiType < vtkDMMLMarkupsROINode::ROIType_Last; ++roiType)
    {
    this->roiTypeComboBox->addItem(vtkDMMLMarkupsROINode::GetROITypeAsString(roiType), roiType);
    }

  QObject::connect(this->roiTypeComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onROITypeParameterChanged()));
  QObject::connect(this->insideOutCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setInsideOut(bool)));
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
  q->setEnabled(q->MarkupsNode != nullptr);
}

// --------------------------------------------------------------------------
// qDMMLMarkupsROIWidget methods

// --------------------------------------------------------------------------
qDMMLMarkupsROIWidget::qDMMLMarkupsROIWidget(QWidget* parent)
  : Superclass(parent), d_ptr(new qDMMLMarkupsROIWidgetPrivate(*this))
{
  this->setup();
}

// --------------------------------------------------------------------------
qDMMLMarkupsROIWidget::~qDMMLMarkupsROIWidget() = default;

// --------------------------------------------------------------------------
void qDMMLMarkupsROIWidget::setup()
{
  Q_D(qDMMLMarkupsROIWidget);
  d->setupUi(this);
}

// --------------------------------------------------------------------------
vtkDMMLMarkupsROINode* qDMMLMarkupsROIWidget::dmmlROINode() const
{
  return vtkDMMLMarkupsROINode::SafeDownCast(this->MarkupsNode);
}


// --------------------------------------------------------------------------
void qDMMLMarkupsROIWidget::setExtent(double min, double max)
{
  this->setExtent(min, max, min, max, min, max);
}

// --------------------------------------------------------------------------
void qDMMLMarkupsROIWidget::setExtent(double minLR, double maxLR,
                                        double minPA, double maxPA,
                                        double minIS, double maxIS)
{
  Q_D(qDMMLMarkupsROIWidget);

  d->LRRangeWidget->setRange(minLR, maxLR);
  d->PARangeWidget->setRange(minPA, maxPA);
  d->ISRangeWidget->setRange(minIS, maxIS);
}


// --------------------------------------------------------------------------
void qDMMLMarkupsROIWidget::setDisplayClippingBox(bool visible)
{
  auto roiNode = vtkDMMLMarkupsROINode::SafeDownCast(this->MarkupsNode);
  if (!roiNode)
    {
    return;
    }

  int numberOfDisplayNodes = roiNode->GetNumberOfDisplayNodes();

  std::vector<int> wasModifying(numberOfDisplayNodes);
  for(int index = 0; index < numberOfDisplayNodes; index++)
    {
    vtkDMMLDisplayNode* displayNode = roiNode->GetNthDisplayNode(index);
    if (!displayNode)
      {
      continue;
      }
    wasModifying[index] = displayNode->StartModify();
    }

  this->MarkupsNode->SetDisplayVisibility(visible);

  for(int index = 0; index < numberOfDisplayNodes; index++)
    {
    vtkDMMLDisplayNode* displayNode = roiNode->GetNthDisplayNode(index);
    if (!displayNode)
      {
      continue;
      }
    displayNode->EndModify(wasModifying[index]);
    }
}

// --------------------------------------------------------------------------
void qDMMLMarkupsROIWidget::setInteractiveMode(bool interactive)
{
  auto roiNode = vtkDMMLMarkupsROINode::SafeDownCast(this->MarkupsNode);
  if (!roiNode)
    {
    return;
    }

  auto roiDisplayNode = vtkDMMLMarkupsROIDisplayNode::SafeDownCast(roiNode->GetDisplayNode());
  if (!roiDisplayNode)
    {
    roiNode->CreateDefaultDisplayNodes();
    }

  roiDisplayNode = vtkDMMLMarkupsROIDisplayNode::SafeDownCast(roiNode->GetDisplayNode());
  roiDisplayNode->SetHandlesInteractive(interactive);
}

// --------------------------------------------------------------------------
void qDMMLMarkupsROIWidget::updateROI()
{
  Q_D(qDMMLMarkupsROIWidget);

  // Ignore the calls from onDMMLNodeModified() as it
  // could set the node in an inconsistent state (except for
  // ISRangeWidget->setValues()).
  if (d->IsProcessingOnDMMLNodeModified)
    {
    return;
    }

  auto roiNode = vtkDMMLMarkupsROINode::SafeDownCast(this->MarkupsNode);
  if (!roiNode)
    {
    return;
    }

  double bounds[6];
  d->LRRangeWidget->values(bounds[0],bounds[1]);
  d->PARangeWidget->values(bounds[2],bounds[3]);
  d->ISRangeWidget->values(bounds[4],bounds[5]);

  DMMLNodeModifyBlocker blocker(roiNode);
  roiNode->SetXYZ(0.5*(bounds[1]+bounds[0]),
                  0.5*(bounds[3]+bounds[2]),
                  0.5*(bounds[5]+bounds[4]));
  roiNode->SetRadiusXYZ(0.5*(bounds[1]-bounds[0]),
                        0.5*(bounds[3]-bounds[2]),
                        0.5*(bounds[5]-bounds[4]));
}


// --------------------------------------------------------------------------
void qDMMLMarkupsROIWidget::setDMMLMarkupsNode(vtkDMMLMarkupsNode* markupsNode)
{
  this->qvtkReconnect(this->MarkupsNode, markupsNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateWidgetFromDMML()));

  this->qvtkReconnect(this->MarkupsNode, markupsNode, vtkDMMLDisplayableNode::DisplayModifiedEvent,
                      this, SLOT(onDMMLDisplayNodeModified()));

  this->MarkupsNode = markupsNode;

  this->updateWidgetFromDMML();

  this->onDMMLDisplayNodeModified();
  this->setEnabled(markupsNode != nullptr);
}

// --------------------------------------------------------------------------
void qDMMLMarkupsROIWidget::onDMMLDisplayNodeModified()
{
  Q_D(qDMMLMarkupsROIWidget);

  auto roiNode = vtkDMMLMarkupsROINode::SafeDownCast(this->MarkupsNode);
  if (!roiNode)
    {
    return;
    }

  // Visibility
  d->DisplayClippingBoxButton->setChecked(roiNode->GetDisplayVisibility());
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsROIWidget::onROITypeParameterChanged()
{
  Q_D(qDMMLMarkupsROIWidget);

  auto roiNode = vtkDMMLMarkupsROINode::SafeDownCast(this->MarkupsNode);
  if (!roiNode)
    {
    return;
    }

  DMMLNodeModifyBlocker blocker(roiNode);
  roiNode->SetROIType(d->roiTypeComboBox->currentData().toInt());
}

//-----------------------------------------------------------------------------
bool qDMMLMarkupsROIWidget::insideOut()
{
  Q_D(qDMMLMarkupsROIWidget);

  auto roiNode = vtkDMMLMarkupsROINode::SafeDownCast(this->MarkupsNode);
  if (!roiNode)
    {
    return false;
    }
  return roiNode->GetInsideOut();
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsROIWidget::setInsideOut(bool insideOut)
{
  Q_D(qDMMLMarkupsROIWidget);

  auto roiNode = vtkDMMLMarkupsROINode::SafeDownCast(this->MarkupsNode);
  if (!roiNode)
    {
    return;
    }

  DMMLNodeModifyBlocker blocker(roiNode);
  roiNode->SetInsideOut(insideOut);
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsROIWidget::updateWidgetFromDMML()
{
  Q_D(qDMMLMarkupsROIWidget);

  auto roiNode = vtkDMMLMarkupsROINode::SafeDownCast(this->MarkupsNode);
  if (!roiNode)
    {
    return;
    }

  d->IsProcessingOnDMMLNodeModified = true;

  // Interactive Mode
  bool interactive = false;
  if (roiNode->GetDisplayNode())
    {
    interactive = vtkDMMLMarkupsDisplayNode::SafeDownCast(roiNode->GetDisplayNode())->GetHandlesInteractive();
    }

  d->LRRangeWidget->setTracking(interactive);
  d->PARangeWidget->setTracking(interactive);
  d->ISRangeWidget->setTracking(interactive);
  d->InteractiveModeCheckBox->setChecked(interactive);

  // ROI
  double xyz[3];
  double rxyz[3];

  roiNode->GetXYZ(xyz);
  roiNode->GetRadiusXYZ(rxyz);

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

  bool wasBlocked = d->roiTypeComboBox->blockSignals(true);
  d->roiTypeComboBox->setCurrentIndex(d->roiTypeComboBox->findData(roiNode->GetROIType()));
  d->roiTypeComboBox->blockSignals(wasBlocked);

  wasBlocked = d->insideOutCheckBox->blockSignals(true);
  d->insideOutCheckBox->setChecked(roiNode->GetInsideOut());
  d->insideOutCheckBox->blockSignals(wasBlocked);
}

//-----------------------------------------------------------------------------
bool qDMMLMarkupsROIWidget::canManageDMMLMarkupsNode(vtkDMMLMarkupsNode *markupsNode) const
{
  auto roiNode = vtkDMMLMarkupsROINode::SafeDownCast(markupsNode);
  if (!roiNode)
    {
    return false;
    }

  return true;
}
