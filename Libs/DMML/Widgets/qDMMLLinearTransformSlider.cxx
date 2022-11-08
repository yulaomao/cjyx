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

// Qt includes

// qDMML includes
#include "qDMMLUtils.h"
#include "qDMMLLinearTransformSlider.h"

// DMML includes
#include "vtkDMMLTransformNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkTransform.h>
#include <vtkWeakPointer.h>

//-----------------------------------------------------------------------------
class qDMMLLinearTransformSliderPrivate
{
public:
  qDMMLLinearTransformSliderPrivate();
  qDMMLLinearTransformSlider::TransformType            TypeOfTransform;
  qDMMLLinearTransformSlider::CoordinateReferenceType  CoordinateReference;
  vtkWeakPointer<vtkDMMLTransformNode>                 DMMLTransformNode;
  double                                               OldPosition;
};

// --------------------------------------------------------------------------
qDMMLLinearTransformSliderPrivate::qDMMLLinearTransformSliderPrivate()
{
  this->TypeOfTransform = qDMMLLinearTransformSlider::TRANSLATION_LR;
  this->CoordinateReference = qDMMLLinearTransformSlider::GLOBAL;
  this->DMMLTransformNode = nullptr;
  this->OldPosition = 0;
}

// --------------------------------------------------------------------------
qDMMLLinearTransformSlider::qDMMLLinearTransformSlider(QWidget* sliderParent)
  : Superclass(sliderParent)
  , d_ptr(new qDMMLLinearTransformSliderPrivate)
{
  this->setQuantity("length");
  this->setUnitAwareProperties(
    qDMMLSliderWidget::Precision | qDMMLSliderWidget::Prefix |
    qDMMLSliderWidget::Scaling | qDMMLSliderWidget::Suffix);
}

// --------------------------------------------------------------------------
qDMMLLinearTransformSlider::~qDMMLLinearTransformSlider() = default;

// --------------------------------------------------------------------------
void qDMMLLinearTransformSlider::setTypeOfTransform(TransformType _typeOfTransform)
{
  Q_D(qDMMLLinearTransformSlider);
  d->TypeOfTransform = _typeOfTransform;
  if (this->isRotation())
    {
    this->setUnitAwareProperties(qDMMLLinearTransformSlider::None);
    this->setSuffix(QString::fromLatin1("\xb0")); // "degree" character
    }
  else
    {
    this->setUnitAwareProperties(
      qDMMLSliderWidget::Precision | qDMMLSliderWidget::Prefix |
      qDMMLSliderWidget::Scaling | qDMMLSliderWidget::Suffix);
    }
  this->onDMMLTransformNodeModified(d->DMMLTransformNode);
}

// --------------------------------------------------------------------------
qDMMLLinearTransformSlider::TransformType qDMMLLinearTransformSlider::typeOfTransform() const
{
  Q_D(const qDMMLLinearTransformSlider);
  return d->TypeOfTransform;
}

// --------------------------------------------------------------------------
bool qDMMLLinearTransformSlider::isRotation()const
{
  return (this->typeOfTransform() == ROTATION_LR ||
          this->typeOfTransform() == ROTATION_PA ||
          this->typeOfTransform() == ROTATION_IS);
}

// --------------------------------------------------------------------------
bool qDMMLLinearTransformSlider::isTranslation()const
{
  return (this->typeOfTransform() == TRANSLATION_LR ||
          this->typeOfTransform() == TRANSLATION_PA ||
          this->typeOfTransform() == TRANSLATION_IS);
}

// --------------------------------------------------------------------------
void qDMMLLinearTransformSlider::
setCoordinateReference(CoordinateReferenceType _coordinateReference)
{
  Q_D(qDMMLLinearTransformSlider);
  d->CoordinateReference = _coordinateReference;
  this->onDMMLTransformNodeModified(d->DMMLTransformNode);
}

// --------------------------------------------------------------------------
qDMMLLinearTransformSlider::CoordinateReferenceType qDMMLLinearTransformSlider::coordinateReference() const
{
  Q_D(const qDMMLLinearTransformSlider);
  return d->CoordinateReference;
}

// --------------------------------------------------------------------------
void qDMMLLinearTransformSlider::setDMMLTransformNode(vtkDMMLTransformNode* transformNode)
{
  Q_D(qDMMLLinearTransformSlider);

  if (d->DMMLTransformNode == transformNode) { return; }

  this->qvtkReconnect(d->DMMLTransformNode, transformNode,
    vtkDMMLTransformableNode::TransformModifiedEvent,
    this, SLOT(onDMMLTransformNodeModified(vtkObject*)));

  d->DMMLTransformNode = transformNode;
  this->onDMMLTransformNodeModified(transformNode);
  // If the node is nullptr, any action on the widget is meaningless, this is why
  // the widget is disabled
  this->setEnabled(transformNode != nullptr && transformNode->IsLinear());
}

// --------------------------------------------------------------------------
vtkDMMLTransformNode* qDMMLLinearTransformSlider::dmmlTransformNode()const
{
  Q_D(const qDMMLLinearTransformSlider);
  return d->DMMLTransformNode;
}

// --------------------------------------------------------------------------
void qDMMLLinearTransformSlider::onDMMLTransformNodeModified(vtkObject* caller)
{
  Q_D(qDMMLLinearTransformSlider);

  vtkDMMLTransformNode* transformNode = vtkDMMLTransformNode::SafeDownCast(caller);
  if (!transformNode)
    {
    return;
    }
  Q_ASSERT(d->DMMLTransformNode == transformNode);

  bool isLinear = transformNode->IsLinear();
  this->setEnabled(isLinear);
  if (!isLinear)
    {
    return;
    }

  vtkNew<vtkTransform> transform;
  if (d->DMMLTransformNode.GetPointer() != nullptr)
    {
    qDMMLUtils::getTransformInCoordinateSystem(d->DMMLTransformNode,
      d->CoordinateReference == qDMMLLinearTransformSlider::GLOBAL, transform.GetPointer());
    }

  vtkMatrix4x4 * matrix = transform->GetMatrix();
  Q_ASSERT(matrix);
  if (!matrix) { return; }

  double _value = 0.0;
  if (this->typeOfTransform() == TRANSLATION_LR)
    {
    _value = matrix->GetElement(0,3);
    }
  else if (this->typeOfTransform() == TRANSLATION_PA)
    {
    _value = matrix->GetElement(1,3);
    }
  else if (this->typeOfTransform() == TRANSLATION_IS)
    {
    _value = matrix->GetElement(2,3);
    }

  if (this->isTranslation())
    {
    // Slider values only match matrix translation values in case of GLOBAL reference
    if (d->CoordinateReference == qDMMLLinearTransformSlider::GLOBAL)
      {
      d->OldPosition = _value;
      // Only attempt to set the current slider value if the current value is reachable
      // (otherwise the value would be truncated to the current slider range).
      // If not reachable then choose closest (truncated) value and block signals to make sure we don't
      // emit signals with the truncated value.
      if (_value<this->minimum())
        {
        bool wasBlocked = this->blockSignals(true);
        this->setValue(this->minimum());
        this->blockSignals(wasBlocked);
        }
      else if (_value>this->maximum())
        {
        bool wasBlocked = this->blockSignals(true);
        this->setValue(this->maximum());
        this->blockSignals(wasBlocked);
        }
      else
        {
        this->setValue(_value);
        }
      }
    else
      {
      d->OldPosition = this->value();
      }
    }
  else if (this->isRotation())
    {
    d->OldPosition = this->value();
    }
}

// --------------------------------------------------------------------------
void qDMMLLinearTransformSlider::applyTransformation(double _sliderPosition)
{
  Q_D(qDMMLLinearTransformSlider);

  if (d->DMMLTransformNode.GetPointer() == nullptr || !d->DMMLTransformNode->IsLinear())
    {
    return;
    }

  vtkNew<vtkTransform> transform;
  qDMMLUtils::getTransformInCoordinateSystem(d->DMMLTransformNode,
    d->CoordinateReference == qDMMLLinearTransformSlider::GLOBAL, transform.GetPointer());

  vtkMatrix4x4 * matrix = transform->GetMatrix();
  Q_ASSERT(matrix);
  if (!matrix) { return; }

  bool transformChanged = false;
  const double rotationChangeTolerance = 0.00001;
  const double translationChangeTolerance = 0.00001;

  if (this->typeOfTransform() == ROTATION_LR)
    {
    double angle = _sliderPosition - d->OldPosition;
    transform->RotateX(angle);
    if (fabs(angle)>rotationChangeTolerance)
      {
      transformChanged = true;
      }
    }
  else if (this->typeOfTransform() == ROTATION_PA)
    {
    double angle = _sliderPosition - d->OldPosition;
    transform->RotateY(angle);
    if (fabs(angle)>rotationChangeTolerance)
      {
      transformChanged = true;
      }
    }
  else if (this->typeOfTransform() == ROTATION_IS)
    {
    double angle = _sliderPosition - d->OldPosition;
    transform->RotateZ(angle);
    if (fabs(angle)>rotationChangeTolerance)
      {
      transformChanged = true;
      }
    }
  else if (this->typeOfTransform() == TRANSLATION_LR)
    {
    double vector[] = {0., 0., 0.};
    // Slider values only match matrix translation values in case of GLOBAL reference
    if (d->CoordinateReference == qDMMLLinearTransformSlider::GLOBAL)
      {
      vector[0] = _sliderPosition - matrix->GetElement(0,3);
      }
    else
      {
      vector[0] = _sliderPosition - d->OldPosition;
      }
    transform->Translate(vector);
    if (fabs(vector[0])>translationChangeTolerance)
      {
      transformChanged = true;
      }
    }
  else if (this->typeOfTransform() == TRANSLATION_PA)
    {
    double vector[] = {0., 0., 0.};
    // Slider values only match matrix translation values in case of GLOBAL reference
    if (d->CoordinateReference == qDMMLLinearTransformSlider::GLOBAL)
      {
      vector[1] = _sliderPosition - matrix->GetElement(1,3);
      }
    else
      {
      vector[1] = _sliderPosition - d->OldPosition;
      }
    transform->Translate(vector);
    if (fabs(vector[1])>translationChangeTolerance)
      {
      transformChanged = true;
      }
    }
  else if (this->typeOfTransform() == TRANSLATION_IS)
    {
    double vector[] = {0., 0., 0.};
    // Slider values only match matrix translation values in case of GLOBAL reference
    if (d->CoordinateReference == qDMMLLinearTransformSlider::GLOBAL)
      {
      vector[2] = _sliderPosition - matrix->GetElement(2,3);
      }
    else
      {
      vector[2] = _sliderPosition - d->OldPosition;
      }
    transform->Translate(vector);
    if (fabs(vector[2])>translationChangeTolerance)
      {
      transformChanged = true;
      }
    }
  d->OldPosition = _sliderPosition;

  if (transformChanged)
    {
  d->DMMLTransformNode->SetMatrixTransformToParent(transform->GetMatrix());
    }
}
