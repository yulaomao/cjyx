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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qDMMLSliderWidget.h"

// CTK includes
#include <ctkLinearValueProxy.h>
#include <ctkUtils.h>

// DMML includes
#include <vtkDMMLNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSelectionNode.h>
#include <vtkDMMLUnitNode.h>

// STD includes
#include <cmath>

// VTK includes
#include <vtkCommand.h>

// --------------------------------------------------------------------------
class qDMMLSliderWidgetPrivate
{
  Q_DECLARE_PUBLIC(qDMMLSliderWidget);
protected:
  qDMMLSliderWidget* const q_ptr;
public:
  qDMMLSliderWidgetPrivate(qDMMLSliderWidget& object);
  ~qDMMLSliderWidgetPrivate();

  void setAndObserveSelectionNode();
  void updateValueProxy(vtkDMMLUnitNode* unitNode);

  QString Quantity;
  vtkDMMLScene* DMMLScene;
  vtkDMMLSelectionNode* SelectionNode;
  qDMMLSliderWidget::UnitAwareProperties Flags;
  ctkLinearValueProxy* Proxy;
};

// --------------------------------------------------------------------------
qDMMLSliderWidgetPrivate::qDMMLSliderWidgetPrivate(qDMMLSliderWidget& object)
  : q_ptr(&object)
{
  this->Quantity = "";
  this->DMMLScene = nullptr;
  this->SelectionNode = nullptr;
  this->Flags = qDMMLSliderWidget::Prefix | qDMMLSliderWidget::Suffix
    | qDMMLSliderWidget::Precision | qDMMLSliderWidget::Scaling;
  this->Proxy = new ctkLinearValueProxy;
}

// --------------------------------------------------------------------------
qDMMLSliderWidgetPrivate::~qDMMLSliderWidgetPrivate()
{
  delete this->Proxy;
}

// --------------------------------------------------------------------------
void qDMMLSliderWidgetPrivate::setAndObserveSelectionNode()
{
  Q_Q(qDMMLSliderWidget);

  vtkDMMLSelectionNode* selectionNode = nullptr;
  if (this->DMMLScene)
    {
    selectionNode = vtkDMMLSelectionNode::SafeDownCast(
      this->DMMLScene->GetNodeByID("vtkDMMLSelectionNodeSingleton"));
    }

  q->qvtkReconnect(this->SelectionNode, selectionNode,
    vtkDMMLSelectionNode::UnitModifiedEvent,
    q, SLOT(updateWidgetFromUnitNode()));
  this->SelectionNode = selectionNode;
  q->updateWidgetFromUnitNode();
}

// --------------------------------------------------------------------------
void qDMMLSliderWidgetPrivate::updateValueProxy(vtkDMMLUnitNode* unitNode)
{
  Q_Q(qDMMLSliderWidget);
  if (!unitNode)
    {
    q->setValueProxy(nullptr);
    this->Proxy->setCoefficient(1.0);
    this->Proxy->setOffset(0.0);
    return;
    }

  q->setValueProxy(this->Proxy);
  this->Proxy->setOffset(unitNode->GetDisplayOffset());
  this->Proxy->setCoefficient(unitNode->GetDisplayCoefficient());
}

// --------------------------------------------------------------------------
// qDMMLSliderWidget

// --------------------------------------------------------------------------
qDMMLSliderWidget::qDMMLSliderWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qDMMLSliderWidgetPrivate(*this))
{
}

// --------------------------------------------------------------------------
qDMMLSliderWidget::~qDMMLSliderWidget() = default;

//-----------------------------------------------------------------------------
void qDMMLSliderWidget::setQuantity(const QString& quantity)
{
  Q_D(qDMMLSliderWidget);
  if (quantity == d->Quantity)
    {
    return;
    }

  d->Quantity = quantity;
  this->updateWidgetFromUnitNode();
}

//-----------------------------------------------------------------------------
QString qDMMLSliderWidget::quantity()const
{
  Q_D(const qDMMLSliderWidget);
  return d->Quantity;
}

// --------------------------------------------------------------------------
vtkDMMLScene* qDMMLSliderWidget::dmmlScene()const
{
  Q_D(const qDMMLSliderWidget);
  return d->DMMLScene;
}

// --------------------------------------------------------------------------
void qDMMLSliderWidget::setDMMLScene(vtkDMMLScene* scene)
{
  Q_D(qDMMLSliderWidget);

  if (this->dmmlScene() == scene)
    {
    return;
    }

  d->DMMLScene = scene;
  d->setAndObserveSelectionNode();

  this->setEnabled(scene != nullptr);
}

// --------------------------------------------------------------------------
qDMMLSliderWidget::UnitAwareProperties
qDMMLSliderWidget::unitAwareProperties()const
{
  Q_D(const qDMMLSliderWidget);
  return d->Flags;
}

// --------------------------------------------------------------------------
void qDMMLSliderWidget::setUnitAwareProperties(UnitAwareProperties newFlags)
{
  Q_D(qDMMLSliderWidget);
  if (newFlags == d->Flags)
    {
    return;
    }

  d->Flags = newFlags;
}

// --------------------------------------------------------------------------
void qDMMLSliderWidget::updateWidgetFromUnitNode()
{
  Q_D(qDMMLSliderWidget);

  if (d->SelectionNode)
    {
    vtkDMMLUnitNode* unitNode =
      vtkDMMLUnitNode::SafeDownCast(d->DMMLScene->GetNodeByID(
        d->SelectionNode->GetUnitNodeID(d->Quantity.toUtf8())));

    if (unitNode)
      {
      if (d->Flags.testFlag(qDMMLSliderWidget::Precision))
        {
        // setDecimals overwrites values therefore it is important
        // to call it only when it is necessary (without this check,
        // for example a setValue call may be ineffective if the min/max
        // value is changing at the same time)
        if (this->decimals()!=unitNode->GetPrecision())
          {
          this->setDecimals(unitNode->GetPrecision());
          }
        }
      if (d->Flags.testFlag(qDMMLSliderWidget::Prefix))
        {
        this->setPrefix(unitNode->GetPrefix());
        }
      if (d->Flags.testFlag(qDMMLSliderWidget::Suffix))
        {
        this->setSuffix(unitNode->GetSuffix());
        }
      if (d->Flags.testFlag(qDMMLSliderWidget::MinimumValue))
        {
        this->setMinimum(unitNode->GetMinimumValue());
        }
      if (d->Flags.testFlag(qDMMLSliderWidget::MaximumValue))
        {
        this->setMaximum(unitNode->GetMaximumValue());
        }
      if (d->Flags.testFlag(qDMMLSliderWidget::Scaling))
        {
        d->updateValueProxy(unitNode);
        }
      if (d->Flags.testFlag(qDMMLSliderWidget::Precision))
        {
        double range = this->maximum() - this->minimum();
        if (d->Flags.testFlag(qDMMLSliderWidget::Scaling))
          {
          range = unitNode->GetDisplayValueFromValue(this->maximum()) -
                  unitNode->GetDisplayValueFromValue(this->minimum());
          }
        double powerOfTen = ctk::closestPowerOfTen(range);
        if (powerOfTen != 0.)
          {
          this->setSingleStep(powerOfTen / 100);
          }
        }
      }
    }
}

// --------------------------------------------------------------------------
void qDMMLSliderWidget::setMinimum(double newMinimumValue)
{
  this->Superclass::setMinimum(newMinimumValue);
  if (this->unitAwareProperties().testFlag(qDMMLSliderWidget::Precision))
    {
    this->updateWidgetFromUnitNode();
    }
}

// --------------------------------------------------------------------------
void qDMMLSliderWidget::setMaximum(double newMaximumValue)
{
  this->Superclass::setMaximum(newMaximumValue);
  if (this->unitAwareProperties().testFlag(qDMMLSliderWidget::Precision))
    {
    this->updateWidgetFromUnitNode();
    }
}

// --------------------------------------------------------------------------
void qDMMLSliderWidget::setRange(double newMinimumValue, double newMaximumValue)
{
  this->Superclass::setRange(newMinimumValue, newMaximumValue);
  if (this->unitAwareProperties().testFlag(qDMMLSliderWidget::Precision))
    {
    this->updateWidgetFromUnitNode();
    }
}
