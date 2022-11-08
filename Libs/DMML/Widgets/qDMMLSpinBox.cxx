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

#include "qDMMLSpinBox.h"

// CTK includes
#include <ctkLinearValueProxy.h>

// Qt includes
#include <QDebug>

// DMML includes
#include <vtkDMMLNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSelectionNode.h>
#include <vtkDMMLUnitNode.h>

#include <cmath>

// VTK includes
#include <vtkCommand.h>

// --------------------------------------------------------------------------
class qDMMLSpinBoxPrivate
{
  Q_DECLARE_PUBLIC(qDMMLSpinBox);
protected:
  qDMMLSpinBox* const q_ptr;
public:
  qDMMLSpinBoxPrivate(qDMMLSpinBox& object);
  ~qDMMLSpinBoxPrivate();

  void setAndObserveSelectionNode();
  void updateValueProxy(vtkDMMLUnitNode* unitNode);

  QString Quantity;
  vtkDMMLScene* DMMLScene;
  vtkDMMLSelectionNode* SelectionNode;
  qDMMLSpinBox::UnitAwareProperties Flags;
  ctkLinearValueProxy* Proxy;
};

// --------------------------------------------------------------------------
qDMMLSpinBoxPrivate::qDMMLSpinBoxPrivate(qDMMLSpinBox& object)
  :q_ptr(&object)
{
  this->Quantity = "";
  this->DMMLScene = nullptr;
  this->SelectionNode = nullptr;
  this->Flags = qDMMLSpinBox::Prefix | qDMMLSpinBox::Suffix
    | qDMMLSpinBox::Precision
    | qDMMLSpinBox::MinimumValue | qDMMLSpinBox::MaximumValue;
  this->Proxy = new ctkLinearValueProxy();
}

// --------------------------------------------------------------------------
qDMMLSpinBoxPrivate::~qDMMLSpinBoxPrivate()
{
  delete this->Proxy;
}

// --------------------------------------------------------------------------
void qDMMLSpinBoxPrivate::setAndObserveSelectionNode()
{
  Q_Q(qDMMLSpinBox);

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
void qDMMLSpinBoxPrivate::updateValueProxy(vtkDMMLUnitNode* unitNode)
{
  Q_Q(qDMMLSpinBox);
  if (!unitNode)
    {
    q->setValueProxy(nullptr);
    this->Proxy->setCoefficient(1.0);
    this->Proxy->setOffset(0.0);
    return;
    }

  this->Proxy->setOffset(unitNode->GetDisplayOffset());
  q->setValueProxy(this->Proxy);
  this->Proxy->setCoefficient(unitNode->GetDisplayCoefficient());
}

// --------------------------------------------------------------------------
// qDMMLSpinBox

// --------------------------------------------------------------------------
qDMMLSpinBox::qDMMLSpinBox(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qDMMLSpinBoxPrivate(*this))
{
}

// --------------------------------------------------------------------------
qDMMLSpinBox::~qDMMLSpinBox() = default;

//-----------------------------------------------------------------------------
void qDMMLSpinBox::setQuantity(const QString& quantity)
{
  Q_D(qDMMLSpinBox);
  if (quantity == d->Quantity)
    {
    return;
    }

  d->Quantity = quantity;
  this->updateWidgetFromUnitNode();
}

//-----------------------------------------------------------------------------
QString qDMMLSpinBox::quantity()const
{
  Q_D(const qDMMLSpinBox);
  return d->Quantity;
}

// --------------------------------------------------------------------------
vtkDMMLScene* qDMMLSpinBox::dmmlScene()const
{
  Q_D(const qDMMLSpinBox);
  return d->DMMLScene;
}

// --------------------------------------------------------------------------
void qDMMLSpinBox::setDMMLScene(vtkDMMLScene* scene)
{
  Q_D(qDMMLSpinBox);

  if (this->dmmlScene() == scene)
    {
    return;
    }

  d->DMMLScene = scene;
  d->setAndObserveSelectionNode();

  this->setEnabled(scene != nullptr);
}

// --------------------------------------------------------------------------
qDMMLSpinBox::UnitAwareProperties qDMMLSpinBox::unitAwareProperties()const
{
  Q_D(const qDMMLSpinBox);
  return d->Flags;
}

// --------------------------------------------------------------------------
void qDMMLSpinBox::setUnitAwareProperties(UnitAwareProperties newFlags)
{
  Q_D(qDMMLSpinBox);
  if (newFlags == d->Flags)
    {
    return;
    }

  d->Flags = newFlags;
}

// --------------------------------------------------------------------------
void qDMMLSpinBox::updateWidgetFromUnitNode()
{
  Q_D(qDMMLSpinBox);

  if (d->SelectionNode)
    {
    vtkDMMLUnitNode* unitNode =
      vtkDMMLUnitNode::SafeDownCast(d->DMMLScene->GetNodeByID(
        d->SelectionNode->GetUnitNodeID(d->Quantity.toUtf8())));

    if (unitNode)
      {
      if (d->Flags.testFlag(qDMMLSpinBox::Precision))
        {
        this->setDecimals(unitNode->GetPrecision());
        this->setSingleStep(pow(10.0, -unitNode->GetPrecision()));
        }
      if (d->Flags.testFlag(qDMMLSpinBox::Prefix))
        {
        this->setPrefix(unitNode->GetPrefix());
        }
      if (d->Flags.testFlag(qDMMLSpinBox::Suffix))
        {
        this->setSuffix(unitNode->GetSuffix());
        }
      if (d->Flags.testFlag(qDMMLSpinBox::MinimumValue))
        {
        this->setMinimum(unitNode->GetMinimumValue());
        }
      if (d->Flags.testFlag(qDMMLSpinBox::MaximumValue))
        {
        this->setMaximum(unitNode->GetMaximumValue());
        }
      if (d->Flags.testFlag(qDMMLSpinBox::Scaling))
        {
        d->updateValueProxy(unitNode);
        }
      }
    }
}
