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
#include <QButtonGroup>

// qDMML includes
#include "qDMMLClipNodeWidget.h"
#include "ui_qDMMLClipNodeWidget.h"

// DMML includes
#include <vtkDMMLClipModelsNode.h>

// VTK includes
#include <vtkSmartPointer.h>

//------------------------------------------------------------------------------
class qDMMLClipNodeWidgetPrivate: public Ui_qDMMLClipNodeWidget
{
  Q_DECLARE_PUBLIC(qDMMLClipNodeWidget);

protected:
  qDMMLClipNodeWidget* const q_ptr;

public:
  qDMMLClipNodeWidgetPrivate(qDMMLClipNodeWidget& object);
  void init();

  vtkSmartPointer<vtkDMMLClipModelsNode> DMMLClipNode;
  bool                                   IsUpdatingWidgetFromDMML;
};

//------------------------------------------------------------------------------
qDMMLClipNodeWidgetPrivate::qDMMLClipNodeWidgetPrivate(qDMMLClipNodeWidget& object)
  : q_ptr(&object)
{
  this->IsUpdatingWidgetFromDMML = false;
}

//------------------------------------------------------------------------------
void qDMMLClipNodeWidgetPrivate::init()
{
  Q_Q(qDMMLClipNodeWidget);
  this->setupUi(q);

  QButtonGroup* clipTypeGroup = new QButtonGroup(q);
  clipTypeGroup->addButton(this->UnionRadioButton);
  clipTypeGroup->addButton(this->IntersectionRadioButton);

  QButtonGroup* redSliceClipStateGroup = new QButtonGroup(q);
  redSliceClipStateGroup->addButton(this->RedPositiveRadioButton);
  redSliceClipStateGroup->addButton(this->RedNegativeRadioButton);

  QButtonGroup* yellowSliceClipStateGroup = new QButtonGroup(q);
  yellowSliceClipStateGroup->addButton(this->YellowPositiveRadioButton);
  yellowSliceClipStateGroup->addButton(this->YellowNegativeRadioButton);

  QButtonGroup* greenSliceClipStateGroup = new QButtonGroup(q);
  greenSliceClipStateGroup->addButton(this->GreenPositiveRadioButton);
  greenSliceClipStateGroup->addButton(this->GreenNegativeRadioButton);

  QObject::connect(this->UnionRadioButton, SIGNAL(toggled(bool)),
                   q, SLOT(updateNodeClipType()));
  QObject::connect(this->IntersectionRadioButton, SIGNAL(toggled(bool)),
                   q, SLOT(updateNodeClipType()));

  QObject::connect(this->RedSliceClippingCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(updateNodeRedClipState()));
  QObject::connect(this->RedPositiveRadioButton, SIGNAL(toggled(bool)),
                   q, SLOT(updateNodeRedClipState()));
  QObject::connect(this->RedNegativeRadioButton, SIGNAL(toggled(bool)),
                   q, SLOT(updateNodeRedClipState()));

  QObject::connect(this->YellowSliceClippingCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(updateNodeYellowClipState()));
  QObject::connect(this->YellowPositiveRadioButton, SIGNAL(toggled(bool)),
                   q, SLOT(updateNodeYellowClipState()));
  QObject::connect(this->YellowNegativeRadioButton, SIGNAL(toggled(bool)),
                   q, SLOT(updateNodeYellowClipState()));

  QObject::connect(this->GreenSliceClippingCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(updateNodeGreenClipState()));
  QObject::connect(this->GreenPositiveRadioButton, SIGNAL(toggled(bool)),
                   q, SLOT(updateNodeGreenClipState()));
  QObject::connect(this->GreenNegativeRadioButton, SIGNAL(toggled(bool)),
                   q, SLOT(updateNodeGreenClipState()));

  QObject::connect(this->WholeCellClippingCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(updateNodeClippingMethod()));


  q->setEnabled(this->DMMLClipNode.GetPointer() != nullptr);
}

//------------------------------------------------------------------------------
qDMMLClipNodeWidget::qDMMLClipNodeWidget(QWidget *_parent)
  : QWidget(_parent)
  , d_ptr(new qDMMLClipNodeWidgetPrivate(*this))
{
  Q_D(qDMMLClipNodeWidget);
  d->init();
}

//------------------------------------------------------------------------------
qDMMLClipNodeWidget::~qDMMLClipNodeWidget() = default;

//------------------------------------------------------------------------------
vtkDMMLClipModelsNode* qDMMLClipNodeWidget::dmmlClipNode()const
{
  Q_D(const qDMMLClipNodeWidget);
  return d->DMMLClipNode;
}

//------------------------------------------------------------------------------
void qDMMLClipNodeWidget::setDMMLClipNode(vtkDMMLNode* node)
{
  this->setDMMLClipNode(vtkDMMLClipModelsNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qDMMLClipNodeWidget::setDMMLClipNode(vtkDMMLClipModelsNode* clipNode)
{
  Q_D(qDMMLClipNodeWidget);
  qvtkReconnect(d->DMMLClipNode, clipNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromDMML()));
  d->DMMLClipNode = clipNode;
  this->updateWidgetFromDMML();
}

//------------------------------------------------------------------------------
void qDMMLClipNodeWidget::setClipType(int type)
{
  Q_D(qDMMLClipNodeWidget);
  if (!d->DMMLClipNode.GetPointer())
    {
    return;
    }
  d->DMMLClipNode->SetClipType(type);
}

//------------------------------------------------------------------------------
int qDMMLClipNodeWidget::clipType()const
{
  Q_D(const qDMMLClipNodeWidget);
  return d->UnionRadioButton->isChecked() ?
    vtkDMMLClipModelsNode::ClipUnion :
    vtkDMMLClipModelsNode::ClipIntersection;
}

//------------------------------------------------------------------------------
void qDMMLClipNodeWidget::setRedSliceClipState(int state)
{
  Q_D(qDMMLClipNodeWidget);
  if (!d->DMMLClipNode.GetPointer())
    {
    return;
    }
  d->DMMLClipNode->SetRedSliceClipState(state);
}

//------------------------------------------------------------------------------
int qDMMLClipNodeWidget::redSliceClipState()const
{
  Q_D(const qDMMLClipNodeWidget);
  return d->RedSliceClippingCheckBox->isChecked() ?
    (d->RedPositiveRadioButton->isChecked() ?
      vtkDMMLClipModelsNode::ClipPositiveSpace :
      vtkDMMLClipModelsNode::ClipNegativeSpace) :
    vtkDMMLClipModelsNode::ClipOff;
}

//------------------------------------------------------------------------------
void qDMMLClipNodeWidget::setYellowSliceClipState(int state)
{
  Q_D(qDMMLClipNodeWidget);
  if (!d->DMMLClipNode.GetPointer())
    {
    return;
    }
  d->DMMLClipNode->SetYellowSliceClipState(state);
}

//------------------------------------------------------------------------------
int qDMMLClipNodeWidget::yellowSliceClipState()const
{
  Q_D(const qDMMLClipNodeWidget);
  return d->YellowSliceClippingCheckBox->isChecked() ?
    (d->YellowPositiveRadioButton->isChecked() ?
      vtkDMMLClipModelsNode::ClipPositiveSpace :
      vtkDMMLClipModelsNode::ClipNegativeSpace) :
    vtkDMMLClipModelsNode::ClipOff;
}

//------------------------------------------------------------------------------
void qDMMLClipNodeWidget::setGreenSliceClipState(int state)
{
  Q_D(qDMMLClipNodeWidget);
  if (!d->DMMLClipNode.GetPointer())
    {
    return;
    }
  d->DMMLClipNode->SetGreenSliceClipState(state);
}

//------------------------------------------------------------------------------
int qDMMLClipNodeWidget::greenSliceClipState()const
{
  Q_D(const qDMMLClipNodeWidget);
  return d->GreenSliceClippingCheckBox->isChecked() ?
    (d->GreenPositiveRadioButton->isChecked() ?
      vtkDMMLClipModelsNode::ClipPositiveSpace :
      vtkDMMLClipModelsNode::ClipNegativeSpace) :
    vtkDMMLClipModelsNode::ClipOff;
}

//------------------------------------------------------------------------------
void qDMMLClipNodeWidget::setClippingMethod(vtkDMMLClipModelsNode::ClippingMethodType state)
{
  Q_D(qDMMLClipNodeWidget);
  if (!d->DMMLClipNode.GetPointer())
    {
    return;
    }
  d->DMMLClipNode->SetClippingMethod(state);
}

//------------------------------------------------------------------------------
vtkDMMLClipModelsNode::ClippingMethodType qDMMLClipNodeWidget::clippingMethod()const
{
  Q_D(const qDMMLClipNodeWidget);
  return d->WholeCellClippingCheckBox->isChecked() ? vtkDMMLClipModelsNode::WholeCells : vtkDMMLClipModelsNode::Straight;
}

//------------------------------------------------------------------------------
void qDMMLClipNodeWidget::updateWidgetFromDMML()
{
  Q_D(qDMMLClipNodeWidget);
  this->setEnabled(d->DMMLClipNode.GetPointer() != nullptr);
  if (d->DMMLClipNode.GetPointer() == nullptr)
    {
    return;
    }
  bool oldUpdating = d->IsUpdatingWidgetFromDMML;
  d->IsUpdatingWidgetFromDMML = true;

  d->UnionRadioButton->setChecked(
    d->DMMLClipNode->GetClipType() == vtkDMMLClipModelsNode::ClipUnion);
  d->IntersectionRadioButton->setChecked(
    d->DMMLClipNode->GetClipType() == vtkDMMLClipModelsNode::ClipIntersection);

  // Setting one checkbox might trigger a signal which result to action in an unstable state
  // to be a valid state, all the checkboxes need to be set.
  d->RedSliceClippingCheckBox->setChecked(
    d->DMMLClipNode->GetRedSliceClipState() != vtkDMMLClipModelsNode::ClipOff);
  d->RedPositiveRadioButton->setChecked(
    d->DMMLClipNode->GetRedSliceClipState() == vtkDMMLClipModelsNode::ClipPositiveSpace);
  d->RedNegativeRadioButton->setChecked(
    d->DMMLClipNode->GetRedSliceClipState() == vtkDMMLClipModelsNode::ClipNegativeSpace);

  d->YellowSliceClippingCheckBox->setChecked(
    d->DMMLClipNode->GetYellowSliceClipState() != vtkDMMLClipModelsNode::ClipOff);
  d->YellowPositiveRadioButton->setChecked(
    d->DMMLClipNode->GetYellowSliceClipState() == vtkDMMLClipModelsNode::ClipPositiveSpace);
  d->YellowNegativeRadioButton->setChecked(
    d->DMMLClipNode->GetYellowSliceClipState() == vtkDMMLClipModelsNode::ClipNegativeSpace);

  d->GreenSliceClippingCheckBox->setChecked(
    d->DMMLClipNode->GetGreenSliceClipState() != vtkDMMLClipModelsNode::ClipOff);
  d->GreenPositiveRadioButton->setChecked(
    d->DMMLClipNode->GetGreenSliceClipState() == vtkDMMLClipModelsNode::ClipPositiveSpace);
  d->GreenNegativeRadioButton->setChecked(
    d->DMMLClipNode->GetGreenSliceClipState() == vtkDMMLClipModelsNode::ClipNegativeSpace);

  d->WholeCellClippingCheckBox->setChecked(
    d->DMMLClipNode->GetClippingMethod() != vtkDMMLClipModelsNode::Straight);

  d->IsUpdatingWidgetFromDMML = oldUpdating;
}

//------------------------------------------------------------------------------
void qDMMLClipNodeWidget::updateNodeClipType()
{
  this->setClipType(this->clipType());
}

//------------------------------------------------------------------------------
void qDMMLClipNodeWidget::updateNodeRedClipState()
{
  Q_D(const qDMMLClipNodeWidget);
  if (d->IsUpdatingWidgetFromDMML)
    {
    return;
    }
  this->setRedSliceClipState(this->redSliceClipState());
}

//------------------------------------------------------------------------------
void qDMMLClipNodeWidget::updateNodeYellowClipState()
{
  Q_D(const qDMMLClipNodeWidget);
  if (d->IsUpdatingWidgetFromDMML)
    {
    return;
    }
  this->setYellowSliceClipState(this->yellowSliceClipState());
}

//------------------------------------------------------------------------------
void qDMMLClipNodeWidget::updateNodeGreenClipState()
{
  Q_D(const qDMMLClipNodeWidget);
  if (d->IsUpdatingWidgetFromDMML)
    {
    return;
    }
  this->setGreenSliceClipState(this->greenSliceClipState());
}

void qDMMLClipNodeWidget::updateNodeClippingMethod()
{
  Q_D(const qDMMLClipNodeWidget);
  if (d->IsUpdatingWidgetFromDMML)
    {
    return;
    }
  this->setClippingMethod(this->clippingMethod());
}
