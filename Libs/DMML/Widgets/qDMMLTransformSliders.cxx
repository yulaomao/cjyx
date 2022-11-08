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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qDMMLTransformSliders.h"
#include "ui_qDMMLTransformSliders.h"

// Qt includes
#include <QStack>

// qDMML includes
#include <qDMMLUtils.h>

// DMML includes
#include "vtkDMMLTransformNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkTransform.h>


//-----------------------------------------------------------------------------
class qDMMLTransformSlidersPrivate: public Ui_qDMMLTransformSliders
{
public:
  qDMMLTransformSlidersPrivate()
    {
    this->TypeOfTransform = -1;
    this->DMMLTransformNode = nullptr;
    }

  int                                    TypeOfTransform;
  vtkDMMLTransformNode*                  DMMLTransformNode;
  QStack<qDMMLLinearTransformSlider*>    ActiveSliders;
};

// --------------------------------------------------------------------------
qDMMLTransformSliders::qDMMLTransformSliders(QWidget* slidersParent)
  : Superclass(slidersParent)
  , d_ptr(new qDMMLTransformSlidersPrivate)
{
  Q_D(qDMMLTransformSliders);

  d->setupUi(this);

  ctkDoubleSpinBox::DecimalsOptions decimalsOptions(ctkDoubleSpinBox::DecimalsByShortcuts | ctkDoubleSpinBox::DecimalsByKey |
    ctkDoubleSpinBox::InsertDecimals | ctkDoubleSpinBox::DecimalsAsMin);
  d->LRSlider->spinBox()->setDecimalsOption(decimalsOptions);
  d->PASlider->spinBox()->setDecimalsOption(decimalsOptions);
  d->ISSlider->spinBox()->setDecimalsOption(decimalsOptions);
  d->LRSlider->setSynchronizeSiblings(ctkSliderWidget::SynchronizeDecimals);
  d->PASlider->setSynchronizeSiblings(ctkSliderWidget::SynchronizeDecimals);
  d->ISSlider->setSynchronizeSiblings(ctkSliderWidget::SynchronizeDecimals);

  this->setCoordinateReference(qDMMLTransformSliders::GLOBAL);
  this->setTypeOfTransform(qDMMLTransformSliders::TRANSLATION);

  this->connect(d->LRSlider, SIGNAL(valueChanged(double)),
                SLOT(onSliderPositionChanged(double)));
  this->connect(d->PASlider, SIGNAL(valueChanged(double)),
                SLOT(onSliderPositionChanged(double)));
  this->connect(d->ISSlider, SIGNAL(valueChanged(double)),
                SLOT(onSliderPositionChanged(double)));

  this->connect(d->MinValueSpinBox, SIGNAL(valueChanged(double)),
                SLOT(onMinimumChanged(double)));
  this->connect(d->MaxValueSpinBox, SIGNAL(valueChanged(double)),
                SLOT(onMaximumChanged(double)));
  // the default values of min and max are set in the .ui file
  this->onMinimumChanged(d->MinValueSpinBox->value());
  this->onMaximumChanged(d->MaxValueSpinBox->value());

  this->connect(d->LRSlider, SIGNAL(decimalsChanged(int)),
                SIGNAL(decimalsChanged(int)));

  // disable as there is not DMML Node associated with the widget
  this->setEnabled(false);
}

// --------------------------------------------------------------------------
qDMMLTransformSliders::~qDMMLTransformSliders() = default;

// --------------------------------------------------------------------------
void qDMMLTransformSliders::setCoordinateReference(CoordinateReferenceType _coordinateReference)
{
  Q_D(qDMMLTransformSliders);

  qDMMLLinearTransformSlider::CoordinateReferenceType ref =
      static_cast<qDMMLLinearTransformSlider::CoordinateReferenceType>(
        _coordinateReference);

  if (this->coordinateReference() != _coordinateReference)
    {
    // reference changed
    if (this->typeOfTransform() == qDMMLTransformSliders::ROTATION
      || (this->typeOfTransform() == qDMMLTransformSliders::TRANSLATION
          && ref == qDMMLLinearTransformSlider::LOCAL) )
      {
      // No one-to-one correspondence between slider and transform matrix values
      bool blocked = false;
      blocked = d->LRSlider->blockSignals(true);
      d->LRSlider->reset();
      d->LRSlider->blockSignals(blocked);
      blocked = d->PASlider->blockSignals(true);
      d->PASlider->reset();
      d->PASlider->blockSignals(blocked);
      blocked = d->ISSlider->blockSignals(true);
      d->ISSlider->reset();
      d->ISSlider->blockSignals(blocked);
      }
    else
      {
      // make sure the current translation values can be set on the slider
      updateRangeFromTransform(d->DMMLTransformNode);
      }
    d->LRSlider->setCoordinateReference(ref);
    d->PASlider->setCoordinateReference(ref);
    d->ISSlider->setCoordinateReference(ref);
    }
}

// --------------------------------------------------------------------------
qDMMLTransformSliders::CoordinateReferenceType qDMMLTransformSliders::coordinateReference() const
{
  Q_D(const qDMMLTransformSliders);

  // Assumes settings of the sliders are all the same
  qDMMLLinearTransformSlider::CoordinateReferenceType ref =
    d->LRSlider->coordinateReference();
  return (ref == qDMMLLinearTransformSlider::GLOBAL) ? GLOBAL : LOCAL;
}

// --------------------------------------------------------------------------
void qDMMLTransformSliders::setTypeOfTransform(TransformType _typeOfTransform)
{
  Q_D(qDMMLTransformSliders);

  if (d->TypeOfTransform == _typeOfTransform) { return; }
  if (_typeOfTransform == qDMMLTransformSliders::TRANSLATION)
    {
    d->LRSlider->setTypeOfTransform(qDMMLLinearTransformSlider::TRANSLATION_LR);
    d->PASlider->setTypeOfTransform(qDMMLLinearTransformSlider::TRANSLATION_PA);
    d->ISSlider->setTypeOfTransform(qDMMLLinearTransformSlider::TRANSLATION_IS);
    }
  else if (_typeOfTransform == qDMMLTransformSliders::ROTATION)
    {
    d->LRSlider->setTypeOfTransform(qDMMLLinearTransformSlider::ROTATION_LR);
    d->PASlider->setTypeOfTransform(qDMMLLinearTransformSlider::ROTATION_PA);
    d->ISSlider->setTypeOfTransform(qDMMLLinearTransformSlider::ROTATION_IS);

    // Range of Rotation sliders should be fixed to (-180,180)
    this->setRange(-180.00, 180.00);
    }
  d->TypeOfTransform = _typeOfTransform;
}

// --------------------------------------------------------------------------
qDMMLTransformSliders::TransformType qDMMLTransformSliders::typeOfTransform() const
{
  Q_D(const qDMMLTransformSliders);
  return static_cast<qDMMLTransformSliders::TransformType>(d->TypeOfTransform);
}

// --------------------------------------------------------------------------
void qDMMLTransformSliders::setDMMLTransformNode(vtkDMMLNode* node)
{
  this->setDMMLTransformNode(vtkDMMLTransformNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qDMMLTransformSliders::setDMMLTransformNode(vtkDMMLTransformNode* transformNode)
{
  Q_D(qDMMLTransformSliders);

  if (d->DMMLTransformNode == transformNode)
    {
    // no change
    return;
    }

  this->qvtkReconnect(d->DMMLTransformNode, transformNode,
                      vtkDMMLTransformableNode::TransformModifiedEvent,
                      this, SLOT(onDMMLTransformNodeModified(vtkObject*)));

  bool blocked = d->LRSlider->blockSignals(true);
  d->LRSlider->reset();
  d->LRSlider->blockSignals(blocked);
  blocked = d->PASlider->blockSignals(true);
  d->PASlider->reset();
  d->PASlider->blockSignals(blocked);
  blocked = d->ISSlider->blockSignals(true);
  d->ISSlider->reset();
  d->ISSlider->blockSignals(blocked);

  this->onDMMLTransformNodeModified(transformNode);

  d->LRSlider->setDMMLTransformNode(transformNode);
  d->PASlider->setDMMLTransformNode(transformNode);
  d->ISSlider->setDMMLTransformNode(transformNode);

  // If the node is nullptr, any action on the widget is meaningless, this is why
  // the widget is disabled
  this->setEnabled(transformNode != nullptr && transformNode->IsLinear());
  d->DMMLTransformNode = transformNode;
}

// --------------------------------------------------------------------------
void qDMMLTransformSliders::onDMMLTransformNodeModified(vtkObject* caller)
{
  vtkDMMLTransformNode* transformNode = vtkDMMLTransformNode::SafeDownCast(caller);
  if (!transformNode)
    {
    return;
    }
  Q_ASSERT(transformNode);
  bool isLinear = transformNode->IsLinear();
  this->setEnabled(isLinear);
  if (!isLinear)
    {
    return;
    }

  // There is no one-to-one correspondence between matrix values and slider position if transform type is rotation;
  // or transform type is translation and coordinate reference is global. In these cases the slider range must not be updated:
  // it is not necessary (as the slider will be reset to 0 anyway when another slider is moved) and changing the slider range
  // can even cause instability (transform value increasing continuously) when the user drags the slider using the mouse.
  if (this->typeOfTransform() == qDMMLTransformSliders::ROTATION
    || (this->typeOfTransform() == qDMMLTransformSliders::TRANSLATION && coordinateReference() == LOCAL) )
    {
    return;
    }

  this->updateRangeFromTransform(transformNode);
}

// --------------------------------------------------------------------------
void qDMMLTransformSliders::updateRangeFromTransform(vtkDMMLTransformNode* transformNode)
{
  vtkNew<vtkTransform> transform;
  qDMMLUtils::getTransformInCoordinateSystem(transformNode,
      this->coordinateReference() == qDMMLTransformSliders::GLOBAL, transform.GetPointer());

  vtkMatrix4x4 * matrix = transform->GetMatrix();
  Q_ASSERT(matrix);
  if (!matrix) { return; }

  QPair<double, double> minmax = this->extractMinMaxTranslationValue(matrix, 0.0);
  if(minmax.first < this->minimum())
    {
    minmax.first = minmax.first - 0.3 * fabs(minmax.first);
    this->setMinimum(minmax.first);
    }
  if(minmax.second > this->maximum())
    {
    minmax.second = minmax.second + 0.3 * fabs(minmax.second);
    this->setMaximum(minmax.second);
    }
}

// --------------------------------------------------------------------------
CTK_GET_CPP(qDMMLTransformSliders, vtkDMMLTransformNode*, dmmlTransformNode, DMMLTransformNode);

// --------------------------------------------------------------------------
void qDMMLTransformSliders::setTitle(const QString& _title)
{
  Q_D(qDMMLTransformSliders);
  d->SlidersGroupBox->setTitle(_title);
}

// --------------------------------------------------------------------------
QString qDMMLTransformSliders::title()const
{
  Q_D(const qDMMLTransformSliders);
  return d->SlidersGroupBox->title();
}

// --------------------------------------------------------------------------
int qDMMLTransformSliders::decimals()const
{
  Q_D(const qDMMLTransformSliders);
  return d->LRSlider->decimals();
}

// --------------------------------------------------------------------------
void qDMMLTransformSliders::setDecimals(int newDecimals)
{
  Q_D(qDMMLTransformSliders);
  // setting the decimals to LRSlider will propagate to the other widgets.
  d->LRSlider->setDecimals(newDecimals);
}

// --------------------------------------------------------------------------
double qDMMLTransformSliders::minimum()const
{
  Q_D(const qDMMLTransformSliders);
  return d->MinValueSpinBox->value();
}

// --------------------------------------------------------------------------
double qDMMLTransformSliders::maximum()const
{
  Q_D(const qDMMLTransformSliders);
  return d->MaxValueSpinBox->value();
}

// --------------------------------------------------------------------------
void qDMMLTransformSliders::setMinimum(double min)
{
  Q_D(qDMMLTransformSliders);
  d->MinValueSpinBox->setValue(min);
}

// --------------------------------------------------------------------------
void qDMMLTransformSliders::setMaximum(double max)
{
  Q_D(qDMMLTransformSliders);
  d->MaxValueSpinBox->setValue(max);
}

// --------------------------------------------------------------------------
void qDMMLTransformSliders::setRange(double min, double max)
{
  Q_D(qDMMLTransformSliders);

  // Could be optimized here by blocking signals on spinboxes and manually
  // call the setRange method on the sliders. Does it really worth it ?
  d->MinValueSpinBox->setValue(min);
  d->MaxValueSpinBox->setValue(max);
}

// --------------------------------------------------------------------------
void qDMMLTransformSliders::onMinimumChanged(double min)
{
  Q_D(qDMMLTransformSliders);

  d->LRSlider->setMinimum(min);
  d->PASlider->setMinimum(min);
  d->ISSlider->setMinimum(min);

  emit this->rangeChanged(min, this->maximum());
}

// --------------------------------------------------------------------------
void qDMMLTransformSliders::onMaximumChanged(double max)
{
  Q_D(qDMMLTransformSliders);

  d->LRSlider->setMaximum(max);
  d->PASlider->setMaximum(max);
  d->ISSlider->setMaximum(max);

  emit this->rangeChanged(this->minimum(), max);
}

// --------------------------------------------------------------------------
void qDMMLTransformSliders::setMinMaxVisible(bool visible)
{
  Q_D(qDMMLTransformSliders);
  d->MinMaxWidget->setVisible(visible);
}

// --------------------------------------------------------------------------
bool qDMMLTransformSliders::isMinMaxVisible()const
{
  Q_D(const qDMMLTransformSliders);
  return d->MinMaxWidget->isVisibleTo(
    const_cast<qDMMLTransformSliders*>(this));
}

// --------------------------------------------------------------------------
double qDMMLTransformSliders::singleStep()const
{
  Q_D(const qDMMLTransformSliders);
  // Assumes settings of the sliders are all the same
  return d->PASlider->singleStep();
}

// --------------------------------------------------------------------------
void qDMMLTransformSliders::setSingleStep(double step)
{
  Q_D(qDMMLTransformSliders);

  d->LRSlider->setSingleStep(step);
  d->PASlider->setSingleStep(step);
  d->ISSlider->setSingleStep(step);
}

// --------------------------------------------------------------------------
QString qDMMLTransformSliders::lrLabel()const
{
  Q_D(const qDMMLTransformSliders);
  return d->LRLabel->text();
}

// --------------------------------------------------------------------------
QString qDMMLTransformSliders::paLabel()const
{
  Q_D(const qDMMLTransformSliders);
  return d->PALabel->text();
}

// --------------------------------------------------------------------------
QString qDMMLTransformSliders::isLabel()const
{
  Q_D(const qDMMLTransformSliders);
  return d->ISLabel->text();
}

// --------------------------------------------------------------------------
void qDMMLTransformSliders::setLRLabel(const QString& label)
{
  Q_D(qDMMLTransformSliders);
  d->LRLabel->setText(label);
}

// --------------------------------------------------------------------------
void qDMMLTransformSliders::setPALabel(const QString& label)
{
  Q_D(qDMMLTransformSliders);
  d->PALabel->setText(label);
}

// --------------------------------------------------------------------------
void qDMMLTransformSliders::setISLabel(const QString& label)
{
  Q_D(qDMMLTransformSliders);
  d->ISLabel->setText(label);
}

// --------------------------------------------------------------------------
void qDMMLTransformSliders::reset()
{
  Q_D(qDMMLTransformSliders);

  d->LRSlider->reset();
  d->PASlider->reset();
  d->ISSlider->reset();
}

// --------------------------------------------------------------------------
void qDMMLTransformSliders::resetUnactiveSliders()
{
  Q_D(qDMMLTransformSliders);

  if (!d->ActiveSliders.contains(d->LRSlider))
    {
    bool blocked = d->LRSlider->blockSignals(true);
    d->LRSlider->reset();
    d->LRSlider->blockSignals(blocked);
    }
  if (!d->ActiveSliders.contains(d->PASlider))
    {
    bool blocked = d->PASlider->blockSignals(true);
    d->PASlider->reset();
    d->PASlider->blockSignals(blocked);
    }
  if (!d->ActiveSliders.contains(d->ISSlider))
    {
    bool blocked = d->ISSlider->blockSignals(true);
    d->ISSlider->reset();
    d->ISSlider->blockSignals(blocked);
    }
}

// --------------------------------------------------------------------------
void qDMMLTransformSliders::onSliderPositionChanged(double position)
{
  Q_D(qDMMLTransformSliders);
  qDMMLLinearTransformSlider* slider =
    qobject_cast<qDMMLLinearTransformSlider*>(this->sender());
  Q_ASSERT(slider);
  d->ActiveSliders.push(slider);
  QWidget* focusWidget = this->focusWidget();

  // If update initiated from spinbox, consider it active, too
  // (when number of decimals are updated then it may change all the sliders
  // one by one, but that should not reset the axis that is currently being changed)
  if (focusWidget)
    {
    if (focusWidget->parent() == d->LRSlider->spinBox())
      {
      d->ActiveSliders.push(d->LRSlider);
      }
    if (focusWidget->parent() == d->PASlider->spinBox())
      {
      d->ActiveSliders.push(d->PASlider);
      }
    if (focusWidget->parent() == d->ISSlider->spinBox())
      {
      d->ActiveSliders.push(d->ISSlider);
      }
    }

  if (this->typeOfTransform() == qDMMLTransformSliders::ROTATION
    || (this->typeOfTransform() == qDMMLTransformSliders::TRANSLATION && coordinateReference() == LOCAL) )
    {
    // When a rotation slider is manipulated, the other rotation sliders are
    // reset to 0. Resetting the other sliders should no fire any event.
    this->resetUnactiveSliders();
    }
  slider->applyTransformation(position);
  emit this->valuesChanged();

  d->ActiveSliders.pop();
}

//-----------------------------------------------------------------------------
QPair<double, double> qDMMLTransformSliders::extractMinMaxTranslationValue(
                                             vtkMatrix4x4 * mat, double pad)
{
  QPair<double, double> minmax;
  if (!mat)
    {
    Q_ASSERT(mat);
    return minmax;
    }
  for (int i=0; i <3; i++)
    {
    minmax.first = qMin(minmax.first, mat->GetElement(i,3));
    minmax.second = qMax(minmax.second, mat->GetElement(i,3));
    }
  double range = minmax.second - minmax.first;
  minmax.first = minmax.first - pad * range;
  minmax.second = minmax.second + pad * range;
  return minmax;
}

