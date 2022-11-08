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

#include "qDMMLCoordinatesWidget.h"

// CTK includes
#include <ctkLinearValueProxy.h>

// Qt includes
#include <QDebug>
#include <QHBoxLayout>

// qDMML includes
#include "qDMMLSpinBox.h"

// DMML nodes includes
#include <vtkDMMLScene.h>
#include <vtkDMMLSelectionNode.h>
#include <vtkDMMLUnitNode.h>

// STD
#include <cmath>

// --------------------------------------------------------------------------
class qDMMLCoordinatesWidgetPrivate
{
  Q_DECLARE_PUBLIC(qDMMLCoordinatesWidget);
protected:
  qDMMLCoordinatesWidget* const q_ptr;
public:
  qDMMLCoordinatesWidgetPrivate(qDMMLCoordinatesWidget& object);
  ~qDMMLCoordinatesWidgetPrivate();

  void setAndObserveSelectionNode();
  void updateValueProxy(vtkDMMLUnitNode* unitNode);

  QString Quantity;
  vtkDMMLScene* DMMLScene;
  vtkDMMLSelectionNode* SelectionNode;
  qDMMLCoordinatesWidget::UnitAwareProperties Flags;
  ctkLinearValueProxy* Proxy;
};

// --------------------------------------------------------------------------
qDMMLCoordinatesWidgetPrivate
::qDMMLCoordinatesWidgetPrivate(qDMMLCoordinatesWidget& object)
  : q_ptr(&object)
{
  this->DMMLScene = nullptr;
  this->SelectionNode = nullptr;
  this->Flags = qDMMLCoordinatesWidget::Prefix | qDMMLCoordinatesWidget::Suffix
    | qDMMLCoordinatesWidget::Precision | qDMMLCoordinatesWidget::MinimumValue
    | qDMMLCoordinatesWidget::MaximumValue;
  this->Proxy = new ctkLinearValueProxy;
}

// --------------------------------------------------------------------------
qDMMLCoordinatesWidgetPrivate::~qDMMLCoordinatesWidgetPrivate()
{
  delete this->Proxy;
}

// --------------------------------------------------------------------------
void qDMMLCoordinatesWidgetPrivate::setAndObserveSelectionNode()
{
  Q_Q(qDMMLCoordinatesWidget);

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
void qDMMLCoordinatesWidgetPrivate::updateValueProxy(vtkDMMLUnitNode* unitNode)
{
  Q_Q(qDMMLCoordinatesWidget);
  if (!unitNode)
    {
    q->setValueProxy(nullptr);
    this->Proxy->setCoefficient(1.0);
    this->Proxy->setOffset(0.0);
    return;
    }

  this->Proxy->setOffset(unitNode->GetDisplayOffset());
  this->Proxy->setCoefficient(unitNode->GetDisplayCoefficient());
  q->setValueProxy(this->Proxy);
}

// --------------------------------------------------------------------------
// qDMMLCoordinatesWidget

//------------------------------------------------------------------------------
qDMMLCoordinatesWidget::qDMMLCoordinatesWidget(QWidget* _parent)
: Superclass(_parent), d_ptr(new qDMMLCoordinatesWidgetPrivate(*this))
{
}

//------------------------------------------------------------------------------
qDMMLCoordinatesWidget::~qDMMLCoordinatesWidget() = default;

//-----------------------------------------------------------------------------
void qDMMLCoordinatesWidget::setQuantity(const QString& quantity)
{
  Q_D(qDMMLCoordinatesWidget);
  if (quantity == d->Quantity)
    {
    return;
    }

  d->Quantity = quantity;
  this->updateWidgetFromUnitNode();
}

//-----------------------------------------------------------------------------
QString qDMMLCoordinatesWidget::quantity()const
{
  Q_D(const qDMMLCoordinatesWidget);
  return d->Quantity;
}

// --------------------------------------------------------------------------
vtkDMMLScene* qDMMLCoordinatesWidget::dmmlScene()const
{
  Q_D(const qDMMLCoordinatesWidget);
  return d->DMMLScene;
}

// --------------------------------------------------------------------------
void qDMMLCoordinatesWidget::setDMMLScene(vtkDMMLScene* scene)
{
  Q_D(qDMMLCoordinatesWidget);

  if (this->dmmlScene() == scene)
    {
    return;
    }

  d->DMMLScene = scene;
  d->setAndObserveSelectionNode();
}

// --------------------------------------------------------------------------
qDMMLCoordinatesWidget::UnitAwareProperties
qDMMLCoordinatesWidget::unitAwareProperties()const
{
  Q_D(const qDMMLCoordinatesWidget);
  return d->Flags;
}

// --------------------------------------------------------------------------
void qDMMLCoordinatesWidget
::setUnitAwareProperties(UnitAwareProperties newFlags)
{
  Q_D(qDMMLCoordinatesWidget);
  if (newFlags == d->Flags)
    {
    return;
    }

  d->Flags = newFlags;
}

// --------------------------------------------------------------------------
void qDMMLCoordinatesWidget::updateWidgetFromUnitNode()
{
  Q_D(qDMMLCoordinatesWidget);

  if (d->SelectionNode)
    {
    vtkDMMLUnitNode* unitNode =
      vtkDMMLUnitNode::SafeDownCast(d->DMMLScene->GetNodeByID(
        d->SelectionNode->GetUnitNodeID(d->Quantity.toUtf8())));

    if (unitNode)
      {
      if (d->Flags.testFlag(qDMMLCoordinatesWidget::Scaling))
        {
        d->updateValueProxy(unitNode);
        }

      if (d->Flags.testFlag(qDMMLCoordinatesWidget::Precision))
        {
        this->setDecimals(unitNode->GetPrecision());
        this->setSingleStep(pow(10.0, -unitNode->GetPrecision()));
        }
      if (d->Flags.testFlag(qDMMLCoordinatesWidget::MinimumValue) &&
          d->Flags.testFlag(qDMMLCoordinatesWidget::MaximumValue))
        {
        this->setRange(unitNode->GetMinimumValue(), unitNode->GetMaximumValue());
        }
      else if (d->Flags.testFlag(qDMMLCoordinatesWidget::MinimumValue))
        {
        this->setMinimum(unitNode->GetMinimumValue());
        }
      else if (d->Flags.testFlag(qDMMLCoordinatesWidget::MaximumValue))
        {
        this->setMaximum(unitNode->GetMaximumValue());
        }
      for (int i = 0; i < this->dimension(); ++i)
        {
        if (d->Flags.testFlag(qDMMLCoordinatesWidget::Prefix))
          {
          this->spinBox(i)->setPrefix(unitNode->GetPrefix());
          }
        if (d->Flags.testFlag(qDMMLCoordinatesWidget::Suffix))
          {
          this->spinBox(i)->setSuffix(unitNode->GetSuffix());
          }
        }
      }
    }
}
