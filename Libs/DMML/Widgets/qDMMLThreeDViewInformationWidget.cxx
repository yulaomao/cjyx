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
#include <QDebug>

// qDMML includes
#include "qDMMLThreeDViewInformationWidget_p.h"

// DMML includes
#include <vtkDMMLViewNode.h>

//--------------------------------------------------------------------------
// qDMMLThreeDViewViewPrivate methods

//---------------------------------------------------------------------------
qDMMLThreeDViewInformationWidgetPrivate::qDMMLThreeDViewInformationWidgetPrivate(qDMMLThreeDViewInformationWidget& object)
  : q_ptr(&object)
{
  this->DMMLViewNode = nullptr;
}

//---------------------------------------------------------------------------
qDMMLThreeDViewInformationWidgetPrivate::~qDMMLThreeDViewInformationWidgetPrivate() = default;

//---------------------------------------------------------------------------
void qDMMLThreeDViewInformationWidgetPrivate::setupUi(qDMMLWidget* widget)
{
  Q_Q(qDMMLThreeDViewInformationWidget);

  this->Ui_qDMMLThreeDViewInformationWidget::setupUi(widget);

  this->connect(this->ViewGroupSpinBox, SIGNAL(valueChanged(int)),
    q, SLOT(setViewGroup(int)));
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewInformationWidgetPrivate::updateWidgetFromDMMLViewNode()
{
  Q_Q(qDMMLThreeDViewInformationWidget);

  q->setEnabled(this->DMMLViewNode != nullptr);
  if (this->DMMLViewNode == nullptr)
    {
    return;
    }

  this->LayoutNameLineEdit->setText(this->DMMLViewNode->GetLayoutName());
  this->ViewGroupSpinBox->setValue(this->DMMLViewNode->GetViewGroup());
}

// --------------------------------------------------------------------------
// qDMMLThreeDViewView methods

// --------------------------------------------------------------------------
qDMMLThreeDViewInformationWidget::qDMMLThreeDViewInformationWidget(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qDMMLThreeDViewInformationWidgetPrivate(*this))
{
  Q_D(qDMMLThreeDViewInformationWidget);
  d->setupUi(this);
  this->setEnabled(false);
}

// --------------------------------------------------------------------------
qDMMLThreeDViewInformationWidget::~qDMMLThreeDViewInformationWidget() = default;

//---------------------------------------------------------------------------
vtkDMMLViewNode* qDMMLThreeDViewInformationWidget::dmmlViewNode()const
{
  Q_D(const qDMMLThreeDViewInformationWidget);
  return d->DMMLViewNode;
}

//---------------------------------------------------------------------------
void qDMMLThreeDViewInformationWidget::setDMMLViewNode(vtkDMMLNode* newNode)
{
  this->setDMMLViewNode(vtkDMMLViewNode::SafeDownCast(newNode));
}

//---------------------------------------------------------------------------
void qDMMLThreeDViewInformationWidget::setDMMLViewNode(vtkDMMLViewNode* newViewNode)
{
  Q_D(qDMMLThreeDViewInformationWidget);

  if (newViewNode == d->DMMLViewNode)
    {
    return;
    }

  d->qvtkReconnect(d->DMMLViewNode, newViewNode, vtkCommand::ModifiedEvent,
    d, SLOT(updateWidgetFromDMMLViewNode()));

  d->DMMLViewNode = newViewNode;

  // Update widget state given the new node
  d->updateWidgetFromDMMLViewNode();
}

//---------------------------------------------------------------------------
void qDMMLThreeDViewInformationWidget::setViewGroup(int viewGroup)
{
  Q_D(qDMMLThreeDViewInformationWidget);

  if (!d->DMMLViewNode)
    {
    return;
    }

  d->DMMLViewNode->SetViewGroup(viewGroup);
}
