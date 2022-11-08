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
#include "qDMMLDisplayNodeWidget.h"
#include "ui_qDMMLDisplayNodeWidget.h"

// DMML includes
#include <vtkDMMLDisplayableNode.h>
#include <vtkDMMLDisplayNode.h>

// VTK includes
#include <vtkProperty.h>
#include <vtkSmartPointer.h>

//------------------------------------------------------------------------------
class qDMMLDisplayNodeWidgetPrivate: public Ui_qDMMLDisplayNodeWidget
{
  Q_DECLARE_PUBLIC(qDMMLDisplayNodeWidget);

protected:
  qDMMLDisplayNodeWidget* const q_ptr;

public:
  qDMMLDisplayNodeWidgetPrivate(qDMMLDisplayNodeWidget& object);
  void init();

  vtkSmartPointer<vtkDMMLDisplayNode> DMMLDisplayNode;
  vtkSmartPointer<vtkProperty> Property;
};

//------------------------------------------------------------------------------
qDMMLDisplayNodeWidgetPrivate::qDMMLDisplayNodeWidgetPrivate(
  qDMMLDisplayNodeWidget& object)
  : q_ptr(&object)
{
  this->Property = vtkSmartPointer<vtkProperty>::New();
}

//------------------------------------------------------------------------------
void qDMMLDisplayNodeWidgetPrivate::init()
{
  Q_Q(qDMMLDisplayNodeWidget);
  this->setupUi(q);

  QObject::connect(this->VisibilityCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setVisibility(bool)));
  QObject::connect(this->SelectedCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setSelected(bool)));
  QObject::connect(this->ClippingCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setClipping(bool)));
  QObject::connect(this->ThreeDVisibilityCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(set3DVisible(bool)));
  QObject::connect(this->SliceIntersectionVisibilityCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setSliceIntersectionVisible(bool)));
  QObject::connect(this->SliceIntersectionThicknessSpinBox, SIGNAL(valueChanged(int)),
                   q, SLOT(setSliceIntersectionThickness(int)));
  QObject::connect(this->SliceIntersectionOpacitySlider, SIGNAL(valueChanged(double)),
                   q, SLOT(setSliceIntersectionOpacity(double)));

  this->PropertyWidget->setProperty(this->Property);
  q->qvtkConnect(this->Property, vtkCommand::ModifiedEvent,
                 q, SLOT(updateNodeFromProperty()));
  q->setEnabled(this->DMMLDisplayNode.GetPointer() != nullptr);
}

//------------------------------------------------------------------------------
qDMMLDisplayNodeWidget::qDMMLDisplayNodeWidget(QWidget *_parent)
  : QWidget(_parent)
  , d_ptr(new qDMMLDisplayNodeWidgetPrivate(*this))
{
  Q_D(qDMMLDisplayNodeWidget);
  d->init();
}

//------------------------------------------------------------------------------
qDMMLDisplayNodeWidget::~qDMMLDisplayNodeWidget() = default;


//------------------------------------------------------------------------------
vtkDMMLDisplayNode* qDMMLDisplayNodeWidget::dmmlDisplayNode()const
{
  Q_D(const qDMMLDisplayNodeWidget);
  return d->DMMLDisplayNode;
}

//------------------------------------------------------------------------------
void qDMMLDisplayNodeWidget::setDMMLDisplayableNode(vtkDMMLNode* node)
{
  vtkDMMLDisplayableNode* displayableNode =
    vtkDMMLDisplayableNode::SafeDownCast(node);
  this->setDMMLDisplayNode(displayableNode ? displayableNode->GetDisplayNode() : nullptr);
}

//------------------------------------------------------------------------------
void qDMMLDisplayNodeWidget::setDMMLDisplayNode(vtkDMMLNode* node)
{
  this->setDMMLDisplayNode(vtkDMMLDisplayNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qDMMLDisplayNodeWidget::setDMMLDisplayNode(vtkDMMLDisplayNode* displayNode)
{
  Q_D(qDMMLDisplayNodeWidget);
  qvtkReconnect(d->DMMLDisplayNode, displayNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromDMML()));
  d->DMMLDisplayNode = displayNode;
  this->updateWidgetFromDMML();
}

//------------------------------------------------------------------------------
void qDMMLDisplayNodeWidget::setVisibility(bool visible)
{
  Q_D(qDMMLDisplayNodeWidget);
  if (!d->DMMLDisplayNode.GetPointer())
    {
    return;
    }
  d->DMMLDisplayNode->SetVisibility(visible);
}

//------------------------------------------------------------------------------
bool qDMMLDisplayNodeWidget::visibility()const
{
  Q_D(const qDMMLDisplayNodeWidget);
  return d->VisibilityCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qDMMLDisplayNodeWidget::setVisibilityVisible(bool visible)
{
  Q_D(const qDMMLDisplayNodeWidget);
  d->VisibilityCheckBox->setVisible(visible);
}

//------------------------------------------------------------------------------
void qDMMLDisplayNodeWidget::setSelected(bool selected)
{
  Q_D(qDMMLDisplayNodeWidget);
  if (!d->DMMLDisplayNode.GetPointer())
    {
    return;
    }
  d->DMMLDisplayNode->SetSelected(selected);
}

//------------------------------------------------------------------------------
bool qDMMLDisplayNodeWidget::selected()const
{
  Q_D(const qDMMLDisplayNodeWidget);
  return d->SelectedCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qDMMLDisplayNodeWidget::setSelectedVisible(bool visible)
{
  Q_D(const qDMMLDisplayNodeWidget);
  d->SelectedLabel->setVisible(visible);
  d->SelectedCheckBox->setVisible(visible);
}

//------------------------------------------------------------------------------
void qDMMLDisplayNodeWidget::setClipping(bool clip)
{
  Q_D(qDMMLDisplayNodeWidget);
  if (!d->DMMLDisplayNode.GetPointer())
    {
    return;
    }
  d->DMMLDisplayNode->SetClipping(clip);
}

//------------------------------------------------------------------------------
bool qDMMLDisplayNodeWidget::clipping()const
{
  Q_D(const qDMMLDisplayNodeWidget);
  return d->ClippingCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qDMMLDisplayNodeWidget::setClippingVisible(bool visible)
{
  Q_D(qDMMLDisplayNodeWidget);
  d->ClippingCheckBox->setVisible(visible);
}

//------------------------------------------------------------------------------
void qDMMLDisplayNodeWidget::setThreeDVisible(bool visible)
{
  Q_D(qDMMLDisplayNodeWidget);
  if (!d->DMMLDisplayNode.GetPointer())
    {
    return;
    }
  d->DMMLDisplayNode->SetVisibility3D(visible);
}

//------------------------------------------------------------------------------
void qDMMLDisplayNodeWidget::setSliceIntersectionVisible(bool visible)
{
  Q_D(qDMMLDisplayNodeWidget);
  if (!d->DMMLDisplayNode.GetPointer())
    {
    return;
    }
  d->DMMLDisplayNode->SetVisibility2D(visible);
}

//------------------------------------------------------------------------------
bool qDMMLDisplayNodeWidget::threeDVisible()const
{
  Q_D(const qDMMLDisplayNodeWidget);
  return d->ThreeDVisibilityCheckBox->isChecked();
}

//------------------------------------------------------------------------------
bool qDMMLDisplayNodeWidget::sliceIntersectionVisible()const
{
  Q_D(const qDMMLDisplayNodeWidget);
  return d->SliceIntersectionVisibilityCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qDMMLDisplayNodeWidget::setThreeDVisibleVisible(bool visible)
{
  Q_D(qDMMLDisplayNodeWidget);
  d->ThreeDVisibilityCheckBox->setVisible(visible);
}

//------------------------------------------------------------------------------
void qDMMLDisplayNodeWidget::setSliceIntersectionVisibleVisible(bool visible)
{
  Q_D(qDMMLDisplayNodeWidget);
  d->SliceIntersectionVisibilityCheckBox->setVisible(visible);
}

//------------------------------------------------------------------------------
void qDMMLDisplayNodeWidget::setSliceIntersectionThickness(int thickness)
{
  Q_D(qDMMLDisplayNodeWidget);
  if (!d->DMMLDisplayNode.GetPointer())
    {
    return;
    }
  d->DMMLDisplayNode->SetSliceIntersectionThickness(thickness);
}

//------------------------------------------------------------------------------
int qDMMLDisplayNodeWidget::sliceIntersectionThickness()const
{
  Q_D(const qDMMLDisplayNodeWidget);
  return d->SliceIntersectionThicknessSpinBox->value();
}

//------------------------------------------------------------------------------
void qDMMLDisplayNodeWidget::setSliceIntersectionThicknessVisible(bool visible)
{
  Q_D(qDMMLDisplayNodeWidget);
  d->SliceIntersectionThicknessSpinBox->setVisible(visible);
}

//------------------------------------------------------------------------------
void qDMMLDisplayNodeWidget::setSliceIntersectionOpacity(double opacity)
{
  Q_D(qDMMLDisplayNodeWidget);
  if (!d->DMMLDisplayNode.GetPointer())
    {
    return;
    }
  d->DMMLDisplayNode->SetSliceIntersectionOpacity(opacity);
}

//------------------------------------------------------------------------------
double qDMMLDisplayNodeWidget::sliceIntersectionOpacity()const
{
  Q_D(const qDMMLDisplayNodeWidget);
  return d->SliceIntersectionOpacitySlider->value();
}

//------------------------------------------------------------------------------
void qDMMLDisplayNodeWidget::setSliceIntersectionOpacityVisible(bool visible)
{
  Q_D(qDMMLDisplayNodeWidget);
  d->SliceIntersectionOpacitySlider->setVisible(visible);
}

//------------------------------------------------------------------------------
void qDMMLDisplayNodeWidget::updateWidgetFromDMML()
{
  Q_D(qDMMLDisplayNodeWidget);
  this->setEnabled(d->DMMLDisplayNode.GetPointer() != nullptr);
  if (!d->DMMLDisplayNode.GetPointer())
    {
    return;
    }
  d->VisibilityCheckBox->setChecked(d->DMMLDisplayNode->GetVisibility());
  d->DisplayNodeViewComboBox->setDMMLDisplayNode(d->DMMLDisplayNode);
  d->SelectedCheckBox->setEnabled(d->DMMLDisplayNode->GetSelectable());
  d->SelectedCheckBox->setChecked(d->DMMLDisplayNode->GetSelected());
  d->ClippingCheckBox->setChecked(d->DMMLDisplayNode->GetClipping());
  d->ThreeDVisibilityCheckBox->setChecked(
    d->DMMLDisplayNode->GetVisibility3D());
  d->SliceIntersectionVisibilityCheckBox->setChecked(
    d->DMMLDisplayNode->GetVisibility2D());
  d->SliceIntersectionThicknessSpinBox->setValue(
    d->DMMLDisplayNode->GetSliceIntersectionThickness());
  d->SliceIntersectionOpacitySlider->setValue(
    d->DMMLDisplayNode->GetSliceIntersectionOpacity());

  // While updating Property, its state is unstable.
  qvtkBlock(d->Property, vtkCommand::ModifiedEvent, this);

  // Representation
  d->Property->SetRepresentation(d->DMMLDisplayNode->GetRepresentation());
  d->Property->SetPointSize(d->DMMLDisplayNode->GetPointSize());
  d->Property->SetLineWidth(d->DMMLDisplayNode->GetLineWidth());
  d->Property->SetFrontfaceCulling(d->DMMLDisplayNode->GetFrontfaceCulling());
  d->Property->SetBackfaceCulling(d->DMMLDisplayNode->GetBackfaceCulling());
  // Color
  d->Property->SetColor(d->DMMLDisplayNode->GetColor()[0],
                        d->DMMLDisplayNode->GetColor()[1],
                        d->DMMLDisplayNode->GetColor()[2]);
  d->Property->SetOpacity(d->DMMLDisplayNode->GetOpacity());
  d->Property->SetEdgeVisibility(d->DMMLDisplayNode->GetEdgeVisibility());
  d->Property->SetEdgeColor(d->DMMLDisplayNode->GetEdgeColor()[0],
                            d->DMMLDisplayNode->GetEdgeColor()[1],
                            d->DMMLDisplayNode->GetEdgeColor()[2]);
  // Lighting
  d->Property->SetLighting(d->DMMLDisplayNode->GetLighting());
  d->Property->SetInterpolation(d->DMMLDisplayNode->GetInterpolation());
  d->Property->SetShading(d->DMMLDisplayNode->GetShading());
  // Material
  d->Property->SetAmbient(d->DMMLDisplayNode->GetAmbient());
  d->Property->SetDiffuse(d->DMMLDisplayNode->GetDiffuse());
  d->Property->SetSpecular(d->DMMLDisplayNode->GetSpecular());
  d->Property->SetSpecularPower(d->DMMLDisplayNode->GetPower());
  d->Property->SetMetallic(d->DMMLDisplayNode->GetMetallic());
  d->Property->SetRoughness(d->DMMLDisplayNode->GetRoughness());
  qvtkUnblock(d->Property, vtkCommand::ModifiedEvent, this);
}

//------------------------------------------------------------------------------
void qDMMLDisplayNodeWidget::updateNodeFromProperty()
{
  Q_D(qDMMLDisplayNodeWidget);
  if (!d->DMMLDisplayNode.GetPointer())
    {
    return;
    }
  int wasModifying = d->DMMLDisplayNode->StartModify();
  // Representation
  d->DMMLDisplayNode->SetRepresentation(d->Property->GetRepresentation());
  d->DMMLDisplayNode->SetPointSize(d->Property->GetPointSize());
  d->DMMLDisplayNode->SetLineWidth(d->Property->GetLineWidth());
  d->DMMLDisplayNode->SetFrontfaceCulling(d->Property->GetFrontfaceCulling());
  d->DMMLDisplayNode->SetBackfaceCulling(d->Property->GetBackfaceCulling());
  // Color
  d->DMMLDisplayNode->SetColor(d->Property->GetColor()[0],
                               d->Property->GetColor()[1],
                               d->Property->GetColor()[2]);
  d->DMMLDisplayNode->SetOpacity(d->Property->GetOpacity());
  d->DMMLDisplayNode->SetEdgeVisibility(d->Property->GetEdgeVisibility());
  d->DMMLDisplayNode->SetEdgeColor(d->Property->GetEdgeColor()[0],
                                   d->Property->GetEdgeColor()[1],
                                   d->Property->GetEdgeColor()[2]);
  // Lighting
  d->DMMLDisplayNode->SetLighting(d->Property->GetLighting());
  d->DMMLDisplayNode->SetInterpolation(d->Property->GetInterpolation());
  d->DMMLDisplayNode->SetShading(d->Property->GetShading());
  // Material
  d->DMMLDisplayNode->SetAmbient(d->Property->GetAmbient());
  d->DMMLDisplayNode->SetDiffuse(d->Property->GetDiffuse());
  d->DMMLDisplayNode->SetSpecular(d->Property->GetSpecular());
  d->DMMLDisplayNode->SetPower(d->Property->GetSpecularPower());

  d->DMMLDisplayNode->EndModify(wasModifying);
}
