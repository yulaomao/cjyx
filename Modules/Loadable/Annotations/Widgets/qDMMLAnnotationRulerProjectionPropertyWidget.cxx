/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Brigham and Women's Hospital

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Laurent Chauvin, Brigham and Women's
  Hospital. The project was supported by grants 5P01CA067165,
  5R01CA124377, 5R01CA138586, 2R44DE019322, 7R01CA124377,
  5R42CA137886, 5R42CA137886

==============================================================================*/

// qDMML includes
#include "qDMMLAnnotationRulerProjectionPropertyWidget.h"
#include "ui_qDMMLAnnotationRulerProjectionPropertyWidget.h"

// DMML includes
#include <vtkDMMLAnnotationRulerNode.h>
#include <vtkDMMLAnnotationLineDisplayNode.h>

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_Annotation
class qDMMLAnnotationRulerProjectionPropertyWidgetPrivate
  : public Ui_qDMMLAnnotationRulerProjectionPropertyWidget
{
  Q_DECLARE_PUBLIC(qDMMLAnnotationRulerProjectionPropertyWidget);
protected:
  qDMMLAnnotationRulerProjectionPropertyWidget* const q_ptr;
public:
  qDMMLAnnotationRulerProjectionPropertyWidgetPrivate(qDMMLAnnotationRulerProjectionPropertyWidget& object);
  void init();

  vtkDMMLAnnotationLineDisplayNode* RulerDisplayNode;
};

//-----------------------------------------------------------------------------
// qDMMLAnnotationRulerProjectionPropertyWidgetPrivate methods

//-----------------------------------------------------------------------------
qDMMLAnnotationRulerProjectionPropertyWidgetPrivate
::qDMMLAnnotationRulerProjectionPropertyWidgetPrivate(qDMMLAnnotationRulerProjectionPropertyWidget& object)
  : q_ptr(&object)
{
  this->RulerDisplayNode = nullptr;
}

//-----------------------------------------------------------------------------
void qDMMLAnnotationRulerProjectionPropertyWidgetPrivate
::init()
{
  Q_Q(qDMMLAnnotationRulerProjectionPropertyWidget);
  this->setupUi(q);
  QObject::connect(this->Line2DProjectionCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setProjectionVisibility(bool)));
  QObject::connect(this->LineUseRulerColorCheckBox, SIGNAL(toggled(bool)),
                  q, SLOT(setUseRulerColor(bool)));
  QObject::connect(this->LineProjectionColorPickerButton, SIGNAL(colorChanged(QColor)),
                   q, SLOT(setProjectionColor(QColor)));
  QObject::connect(this->LineOverlineThicknessSpinBox, SIGNAL(valueChanged(int)),
                   q, SLOT(setOverlineThickness(int)));
  QObject::connect(this->LineUnderlineThicknessSpinBox, SIGNAL(valueChanged(int)),
                   q, SLOT(setUnderlineThickness(int)));
  QObject::connect(this->LineColoredWhenParallelCheckBox, SIGNAL(toggled(bool)),
                  q, SLOT(setColoredWhenParallel(bool)));
  QObject::connect(this->LineThickerOnTopCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setThickerOnTop(bool)));
  QObject::connect(this->LineDashedCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setDashed(bool)));
  q->updateWidgetFromDisplayNode();
}

//-----------------------------------------------------------------------------
// qDMMLAnnotationRulerProjectionPropertyWidget methods

//-----------------------------------------------------------------------------
qDMMLAnnotationRulerProjectionPropertyWidget
::qDMMLAnnotationRulerProjectionPropertyWidget(QWidget *newParent) :
    Superclass(newParent)
  , d_ptr(new qDMMLAnnotationRulerProjectionPropertyWidgetPrivate(*this))
{
  Q_D(qDMMLAnnotationRulerProjectionPropertyWidget);
  d->init();
}

//-----------------------------------------------------------------------------
qDMMLAnnotationRulerProjectionPropertyWidget
::~qDMMLAnnotationRulerProjectionPropertyWidget() = default;

//-----------------------------------------------------------------------------
void qDMMLAnnotationRulerProjectionPropertyWidget
::setDMMLRulerNode(vtkDMMLAnnotationRulerNode* rulerNode)
{
  Q_D(qDMMLAnnotationRulerProjectionPropertyWidget);
  vtkDMMLAnnotationLineDisplayNode* displayNode
    = rulerNode->GetAnnotationLineDisplayNode();
  if (!displayNode)
    {
    return;
    }

  qvtkReconnect(d->RulerDisplayNode, displayNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromDisplayNode()));

  d->RulerDisplayNode = displayNode;
  this->updateWidgetFromDisplayNode();
}

//-----------------------------------------------------------------------------
void qDMMLAnnotationRulerProjectionPropertyWidget
::setProjectionVisibility(bool showProjection)
{
  Q_D(qDMMLAnnotationRulerProjectionPropertyWidget);
  if (!d->RulerDisplayNode)
    {
    return;
    }

  if (showProjection)
    {
    d->RulerDisplayNode->SliceProjectionOn();
    }
  else
    {
    d->RulerDisplayNode->SliceProjectionOff();
    }
}

//-----------------------------------------------------------------------------
void qDMMLAnnotationRulerProjectionPropertyWidget
::setUseRulerColor(bool useRulerColor)
{
  Q_D(qDMMLAnnotationRulerProjectionPropertyWidget);
  if (!d->RulerDisplayNode)
    {
    return;
    }

  if (useRulerColor)
    {
    d->RulerDisplayNode->SliceProjectionUseRulerColorOn();
    }
  else
    {
    d->RulerDisplayNode->SliceProjectionUseRulerColorOff();
    }
}

//-----------------------------------------------------------------------------
void qDMMLAnnotationRulerProjectionPropertyWidget
::setProjectionColor(QColor newColor)
{
  Q_D(qDMMLAnnotationRulerProjectionPropertyWidget);
  if (!d->RulerDisplayNode)
    {
    return;
    }
  d->RulerDisplayNode
    ->SetProjectedColor(newColor.redF(), newColor.greenF(), newColor.blueF());
}

//-----------------------------------------------------------------------------
void qDMMLAnnotationRulerProjectionPropertyWidget
::setOverlineThickness(int thickness)
{
  Q_D(qDMMLAnnotationRulerProjectionPropertyWidget);
  if (!d->RulerDisplayNode)
    {
    return;
    }

  if (d->RulerDisplayNode->GetOverLineThickness() != thickness)
    {
    d->RulerDisplayNode->SetOverLineThickness(thickness);
    }
}

//-----------------------------------------------------------------------------
void qDMMLAnnotationRulerProjectionPropertyWidget
::setUnderlineThickness(int thickness)
{
  Q_D(qDMMLAnnotationRulerProjectionPropertyWidget);
  if (!d->RulerDisplayNode)
    {
    return;
    }

  if (d->RulerDisplayNode->GetUnderLineThickness() != thickness)
    {
    d->RulerDisplayNode->SetUnderLineThickness(thickness);
    }
}

//-----------------------------------------------------------------------------
void qDMMLAnnotationRulerProjectionPropertyWidget
::setColoredWhenParallel(bool coloredWhenParallel)
{
  Q_D(qDMMLAnnotationRulerProjectionPropertyWidget);
  if (!d->RulerDisplayNode)
    {
    return;
    }

  if (coloredWhenParallel)
    {
    d->RulerDisplayNode->SliceProjectionColoredWhenParallelOn();
    }
  else
    {
    d->RulerDisplayNode->SliceProjectionColoredWhenParallelOff();
    }
}

//-----------------------------------------------------------------------------
void qDMMLAnnotationRulerProjectionPropertyWidget
::setThickerOnTop(bool thickerOnTop)
{
  Q_D(qDMMLAnnotationRulerProjectionPropertyWidget);
  if (!d->RulerDisplayNode)
    {
    return;
    }

  if (thickerOnTop)
    {
    d->RulerDisplayNode->SliceProjectionThickerOnTopOn();
    }
  else
    {
    d->RulerDisplayNode->SliceProjectionThickerOnTopOff();
    }
}

//-----------------------------------------------------------------------------
void qDMMLAnnotationRulerProjectionPropertyWidget
::setDashed(bool dashed)
{
  Q_D(qDMMLAnnotationRulerProjectionPropertyWidget);
  if (!d->RulerDisplayNode)
    {
    return;
    }

  if (dashed)
    {
    d->RulerDisplayNode->SliceProjectionDashedOn();
    }
  else
    {
    d->RulerDisplayNode->SliceProjectionDashedOff();
    }
}

//-----------------------------------------------------------------------------
void qDMMLAnnotationRulerProjectionPropertyWidget
::updateWidgetFromDisplayNode()
{
  Q_D(qDMMLAnnotationRulerProjectionPropertyWidget);

  this->setEnabled(d->RulerDisplayNode != nullptr);

  if (!d->RulerDisplayNode)
    {
    return;
    }

  // Update widget if different from DMML node
  // -- 2D Projection Visibility
  d->Line2DProjectionCheckBox->setChecked(
    d->RulerDisplayNode->GetSliceProjection() &
    vtkDMMLAnnotationDisplayNode::ProjectionOn);

  // -- Projection Color
  double pColor[3];
  d->RulerDisplayNode->GetProjectedColor(pColor);
  QColor displayColor = QColor(pColor[0]*255, pColor[1]*255, pColor[2]*255);
  d->LineProjectionColorPickerButton->setColor(displayColor);

  // -- Overline thickness
  double dmmlOverlineThickness = d->RulerDisplayNode->GetOverLineThickness();
  if (d->LineOverlineThicknessSpinBox->value() != dmmlOverlineThickness)
    {
    d->LineOverlineThicknessSpinBox->setValue(dmmlOverlineThickness);
    }

  // -- Underline thickness
  double dmmlUnderlineThickness = d->RulerDisplayNode->GetUnderLineThickness();
  if (d->LineUnderlineThicknessSpinBox->value() != dmmlUnderlineThickness)
    {
    d->LineUnderlineThicknessSpinBox->setValue(dmmlUnderlineThickness);
    }

  // -- Colored When Parallel
  d->LineColoredWhenParallelCheckBox->setChecked(
    d->RulerDisplayNode->GetSliceProjection() &
    vtkDMMLAnnotationLineDisplayNode::ProjectionColoredWhenParallel);

  // -- Thicker On Top
  d->LineThickerOnTopCheckBox->setChecked(
    d->RulerDisplayNode->GetSliceProjection() &
    vtkDMMLAnnotationLineDisplayNode::ProjectionThickerOnTop);

  // -- Dashed
  d->LineDashedCheckBox->setChecked(
    d->RulerDisplayNode->GetSliceProjection() &
    vtkDMMLAnnotationLineDisplayNode::ProjectionDashed);
}
