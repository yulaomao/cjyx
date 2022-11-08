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

// qDMML includes
#include "qDMMLMarkupsPlaneWidget.h"
#include "ui_qDMMLMarkupsPlaneWidget.h"

// DMML includes
#include <vtkDMMLMarkupsPlaneDisplayNode.h>
#include <vtkDMMLMarkupsPlaneNode.h>

// STD includes
#include <vector>

// --------------------------------------------------------------------------
class qDMMLMarkupsPlaneWidgetPrivate:
  public Ui_qDMMLMarkupsPlaneWidget
{
public:
  qDMMLMarkupsPlaneWidgetPrivate(qDMMLMarkupsPlaneWidget& object);
  void setupUi(qDMMLMarkupsPlaneWidget* widget);

  const char* getPlaneTypeName(int planeType);

protected:
  qDMMLMarkupsPlaneWidget* const q_ptr;

private:
  Q_DECLARE_PUBLIC(qDMMLMarkupsPlaneWidget);
};

// --------------------------------------------------------------------------
qDMMLMarkupsPlaneWidgetPrivate::qDMMLMarkupsPlaneWidgetPrivate(qDMMLMarkupsPlaneWidget& widget)
  : q_ptr(&widget)
{
}

// --------------------------------------------------------------------------
void qDMMLMarkupsPlaneWidgetPrivate::setupUi(qDMMLMarkupsPlaneWidget* widget)
{
  Q_Q(qDMMLMarkupsPlaneWidget);

  this->Ui_qDMMLMarkupsPlaneWidget::setupUi(widget);

  this->planeTypeComboBox->clear();
  for (int planeType = 0; planeType < vtkDMMLMarkupsPlaneNode::PlaneType_Last; ++planeType)
    {
    this->planeTypeComboBox->addItem(this->getPlaneTypeName(planeType), planeType);
    }

  this->planeSizeModeComboBox->clear();
  for (int sizeMode = 0; sizeMode < vtkDMMLMarkupsPlaneNode::SizeMode_Last; ++sizeMode)
    {
    this->planeSizeModeComboBox->addItem(vtkDMMLMarkupsPlaneNode::GetSizeModeAsString(sizeMode), sizeMode);
    }

  QObject::connect(this->planeTypeComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onPlaneTypeIndexChanged()));
  QObject::connect(this->planeSizeModeComboBox, SIGNAL(currentIndexChanged(int)),
    q, SLOT(onPlaneSizeModeIndexChanged()));

  QObject::connect(this->sizeXSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(onPlaneSizeSpinBoxChanged()));
  QObject::connect(this->sizeYSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(onPlaneSizeSpinBoxChanged()));


  QObject::connect(this->boundsXMinSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(onPlaneBoundsSpinBoxChanged()));
  QObject::connect(this->boundsXMaxSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(onPlaneBoundsSpinBoxChanged()));
  QObject::connect(this->boundsYMinSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(onPlaneBoundsSpinBoxChanged()));
  QObject::connect(this->boundsYMaxSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(onPlaneBoundsSpinBoxChanged()));

  QObject::connect(this->normalVisibilityCheckBox, SIGNAL(stateChanged(int)), q, SLOT(onNormalVisibilityCheckBoxChanged()));
  QObject::connect(this->normalOpacitySlider, SIGNAL(valueChanged(double)), q, SLOT(onNormalOpacitySliderChanged()));

  q->setEnabled(vtkDMMLMarkupsPlaneNode::SafeDownCast(q->MarkupsNode) != nullptr);
  q->setVisible(vtkDMMLMarkupsPlaneNode::SafeDownCast(q->MarkupsNode) != nullptr);
}

// --------------------------------------------------------------------------
const char* qDMMLMarkupsPlaneWidgetPrivate::getPlaneTypeName(int planeType)
{
  switch (planeType)
    {
    case vtkDMMLMarkupsPlaneNode::PlaneType3Points:
      return "Three points";
    case vtkDMMLMarkupsPlaneNode::PlaneTypePointNormal:
      return "Point normal";
    case vtkDMMLMarkupsPlaneNode::PlaneTypePlaneFit:
      return "Plane fit";
    default:
      break;
    }
  return "";
}

// --------------------------------------------------------------------------
// qDMMLMarkupsPlaneWidget methods

// --------------------------------------------------------------------------
qDMMLMarkupsPlaneWidget::qDMMLMarkupsPlaneWidget(QWidget* parent)
  : Superclass(parent), d_ptr(new qDMMLMarkupsPlaneWidgetPrivate(*this))
{
  this->setup();
}

// --------------------------------------------------------------------------
qDMMLMarkupsPlaneWidget::~qDMMLMarkupsPlaneWidget() = default;

// --------------------------------------------------------------------------
void qDMMLMarkupsPlaneWidget::setup()
{
  Q_D(qDMMLMarkupsPlaneWidget);
  d->setupUi(this);
}

// --------------------------------------------------------------------------
vtkDMMLMarkupsPlaneNode* qDMMLMarkupsPlaneWidget::dmmlPlaneNode()const
{
  Q_D(const qDMMLMarkupsPlaneWidget);
  return vtkDMMLMarkupsPlaneNode::SafeDownCast(this->MarkupsNode);
}

// --------------------------------------------------------------------------
void qDMMLMarkupsPlaneWidget::setDMMLMarkupsNode(vtkDMMLMarkupsNode* markupsNode)
{
  this->qvtkReconnect(this->MarkupsNode, markupsNode, vtkCommand::ModifiedEvent,
    this, SLOT(updateWidgetFromDMML()));

  this->MarkupsNode = markupsNode;
  this->updateWidgetFromDMML();
}

// --------------------------------------------------------------------------
void qDMMLMarkupsPlaneWidget::updateWidgetFromDMML()
{
  Q_D(qDMMLMarkupsPlaneWidget);

  this->setEnabled(this->canManageDMMLMarkupsNode(this->MarkupsNode));
  this->setVisible(this->canManageDMMLMarkupsNode(this->MarkupsNode));

  vtkDMMLMarkupsPlaneNode* planeNode = vtkDMMLMarkupsPlaneNode::SafeDownCast(this->MarkupsNode);
  if (!planeNode)
    {
    return;
    }

  bool wasBlocked = d->planeTypeComboBox->blockSignals(true);
  d->planeTypeComboBox->setCurrentIndex(d->planeTypeComboBox->findData(planeNode->GetPlaneType()));
  d->planeTypeComboBox->blockSignals(wasBlocked);

  wasBlocked = d->planeSizeModeComboBox->blockSignals(true);
  d->planeSizeModeComboBox->setCurrentIndex(d->planeSizeModeComboBox->findData(planeNode->GetSizeMode()));
  d->planeSizeModeComboBox->blockSignals(wasBlocked);

  double* size = planeNode->GetSize();

  wasBlocked = d->sizeXSpinBox->blockSignals(true);
  d->sizeXSpinBox->setValue(size[0]);
  d->sizeXSpinBox->blockSignals(wasBlocked);
  d->sizeXSpinBox->setEnabled(planeNode->GetSizeMode() != vtkDMMLMarkupsPlaneNode::SizeModeAuto);

  wasBlocked = d->sizeYSpinBox->blockSignals(true);
  d->sizeYSpinBox->setValue(size[1]);
  d->sizeYSpinBox->blockSignals(wasBlocked);
  d->sizeYSpinBox->setEnabled(planeNode->GetSizeMode() != vtkDMMLMarkupsPlaneNode::SizeModeAuto);

  double* bounds = planeNode->GetPlaneBounds();

  wasBlocked = d->boundsXMinSpinBox->blockSignals(true);
  d->boundsXMinSpinBox->setValue(bounds[0]);
  d->boundsXMinSpinBox->blockSignals(wasBlocked);
  d->boundsXMinSpinBox->setEnabled(planeNode->GetSizeMode() != vtkDMMLMarkupsPlaneNode::SizeModeAuto);

  wasBlocked = d->boundsXMaxSpinBox->blockSignals(true);
  d->boundsXMaxSpinBox->setValue(bounds[1]);
  d->boundsXMaxSpinBox->blockSignals(wasBlocked);
  d->boundsXMaxSpinBox->setEnabled(planeNode->GetSizeMode() != vtkDMMLMarkupsPlaneNode::SizeModeAuto);

  wasBlocked = d->boundsYMinSpinBox->blockSignals(true);
  d->boundsYMinSpinBox->setValue(bounds[2]);
  d->boundsYMinSpinBox->blockSignals(wasBlocked);
  d->boundsYMinSpinBox->setEnabled(planeNode->GetSizeMode() != vtkDMMLMarkupsPlaneNode::SizeModeAuto);

  wasBlocked = d->boundsYMaxSpinBox->blockSignals(true);
  d->boundsYMaxSpinBox->setValue(bounds[3]);
  d->boundsYMaxSpinBox->blockSignals(wasBlocked);
  d->boundsYMaxSpinBox->setEnabled(planeNode->GetSizeMode() != vtkDMMLMarkupsPlaneNode::SizeModeAuto);

  vtkDMMLMarkupsPlaneDisplayNode* planeDisplayNode = vtkDMMLMarkupsPlaneDisplayNode::SafeDownCast(planeNode->GetDisplayNode());
  if (planeDisplayNode)
    {
    wasBlocked = d->normalVisibilityCheckBox->blockSignals(true);
    d->normalVisibilityCheckBox->setChecked(planeDisplayNode->GetNormalVisibility());
    d->normalVisibilityCheckBox->blockSignals(wasBlocked);

    wasBlocked = d->normalOpacitySlider->blockSignals(true);
    d->normalOpacitySlider->setValue(planeDisplayNode->GetNormalOpacity());
    d->normalOpacitySlider->blockSignals(wasBlocked);
    }
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsPlaneWidget::onPlaneTypeIndexChanged()
{
  Q_D(qDMMLMarkupsPlaneWidget);
  vtkDMMLMarkupsPlaneNode* planeNode = vtkDMMLMarkupsPlaneNode::SafeDownCast(this->MarkupsNode);
  if (!planeNode)
    {
    return;
    }
  planeNode->SetPlaneType(d->planeTypeComboBox->currentData().toInt());
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsPlaneWidget::onPlaneSizeModeIndexChanged()
{
  Q_D(qDMMLMarkupsPlaneWidget);
  vtkDMMLMarkupsPlaneNode* planeNode = vtkDMMLMarkupsPlaneNode::SafeDownCast(this->MarkupsNode);
  if (!planeNode)
    {
    return;
    }
  planeNode->SetSizeMode(d->planeSizeModeComboBox->currentData().toInt());
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsPlaneWidget::onPlaneSizeSpinBoxChanged()
{
  Q_D(qDMMLMarkupsPlaneWidget);
  vtkDMMLMarkupsPlaneNode* planeNode = vtkDMMLMarkupsPlaneNode::SafeDownCast(this->MarkupsNode);
  if (!planeNode)
    {
    return;
    }
  planeNode->SetSize(d->sizeXSpinBox->value(), d->sizeYSpinBox->value());
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsPlaneWidget::onPlaneBoundsSpinBoxChanged()
{
  Q_D(qDMMLMarkupsPlaneWidget);
  vtkDMMLMarkupsPlaneNode* planeNode = vtkDMMLMarkupsPlaneNode::SafeDownCast(this->MarkupsNode);
  if (!planeNode)
  {
    return;
  }
  double xMin = std::min(d->boundsXMinSpinBox->value(), d->boundsXMaxSpinBox->value());
  double xMax = std::max(d->boundsXMinSpinBox->value(), d->boundsXMaxSpinBox->value());
  double yMin = std::min(d->boundsYMinSpinBox->value(), d->boundsYMaxSpinBox->value());
  double yMax = std::max(d->boundsYMinSpinBox->value(), d->boundsYMaxSpinBox->value());
  planeNode->SetPlaneBounds(xMin, xMax, yMin, yMax);
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsPlaneWidget::onNormalVisibilityCheckBoxChanged()
{
  Q_D(qDMMLMarkupsPlaneWidget);
  vtkDMMLMarkupsPlaneNode* planeNode = vtkDMMLMarkupsPlaneNode::SafeDownCast(this->MarkupsNode);
  if (!planeNode)
    {
    return;
    }

  vtkDMMLMarkupsPlaneDisplayNode* displayNode = vtkDMMLMarkupsPlaneDisplayNode::SafeDownCast(planeNode->GetDisplayNode());
  if (!displayNode)
    {
    return;
    }

  displayNode->SetNormalVisibility(d->normalVisibilityCheckBox->checkState() == Qt::Checked);
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsPlaneWidget::onNormalOpacitySliderChanged()
{
  Q_D(qDMMLMarkupsPlaneWidget);
  vtkDMMLMarkupsPlaneNode* planeNode = vtkDMMLMarkupsPlaneNode::SafeDownCast(this->MarkupsNode);
  if (!planeNode)
    {
    return;
    }

  vtkDMMLMarkupsPlaneDisplayNode* displayNode = vtkDMMLMarkupsPlaneDisplayNode::SafeDownCast(planeNode->GetDisplayNode());
  if (!displayNode)
    {
    return;
    }

  displayNode->SetNormalOpacity(d->normalOpacitySlider->value());
}

//-----------------------------------------------------------------------------
bool qDMMLMarkupsPlaneWidget::canManageDMMLMarkupsNode(vtkDMMLMarkupsNode *markupsNode) const
{
  Q_D(const qDMMLMarkupsPlaneWidget);

  vtkDMMLMarkupsPlaneNode* planeNode = vtkDMMLMarkupsPlaneNode::SafeDownCast(markupsNode);
  if (!planeNode)
    {
    return false;
    }

  return true;
}
