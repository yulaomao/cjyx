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
#include <QHBoxLayout>
#include <QMenu>
#include <QStyleOptionSlider>
#include <QToolButton>
#include <QWidgetAction>

// qDMML includes
#include <qDMMLSpinBox.h>

#include "qDMMLRangeWidget.h"

// --------------------------------------------------------------------------
// qDMMLRangeWidget
//

// --------------------------------------------------------------------------
qDMMLRangeWidget::qDMMLRangeWidget(QWidget* parentWidget)
  : ctkRangeWidget(parentWidget)
{
  this->setSlider(new qDMMLDoubleRangeSlider(nullptr));

  QWidget* rangeWidget = new QWidget(this);
  QHBoxLayout* rangeLayout = new QHBoxLayout;
  rangeWidget->setLayout(rangeLayout);
  rangeLayout->setContentsMargins(0,0,0,0);

  this->MinSpinBox = new qDMMLSpinBox(rangeWidget);
  this->MinSpinBox->setPrefix("Min: ");
  this->MinSpinBox->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
  this->MinSpinBox->setValue(this->minimum());
  connect(this->MinSpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateRange()));
  rangeLayout->addWidget(this->MinSpinBox);

  this->MaxSpinBox = new qDMMLSpinBox(rangeWidget);
  this->MaxSpinBox->setPrefix("Max: ");
  this->MaxSpinBox->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
  this->MaxSpinBox->setValue(this->maximum());
  connect(this->MaxSpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateRange()));
  rangeLayout->addWidget(this->MaxSpinBox);

  connect(this->slider(), SIGNAL(rangeChanged(double,double)),
          this, SLOT(updateSpinBoxRange(double,double)));

  QWidgetAction* rangeAction = new QWidgetAction(this);
  rangeAction->setDefaultWidget(rangeWidget);

  this->SymmetricAction = new QAction(tr("Symmetric handles"),this);
  this->SymmetricAction->setCheckable(true);
  connect(this->SymmetricAction, SIGNAL(toggled(bool)),
          this, SLOT(updateSymmetricMoves(bool)));
  this->SymmetricAction->setChecked(this->symmetricMoves());

  QMenu* optionsMenu = new QMenu(this);
  optionsMenu->addAction(rangeAction);
  optionsMenu->addAction(this->SymmetricAction);

  QToolButton* optionsButton = new QToolButton(this);
  optionsButton->setIcon(QIcon(":Icons/SliceMoreOptions.png"));
  optionsButton->setMenu(optionsMenu);
  optionsButton->setPopupMode(QToolButton::InstantPopup);
  QGridLayout* gridLayout = qobject_cast<QGridLayout*>(this->layout());
  gridLayout->addWidget(optionsButton,0,3);
}

// --------------------------------------------------------------------------
QPalette qDMMLRangeWidget::minimumHandlePalette()const
{
  return qobject_cast<qDMMLDoubleRangeSlider*>(this->slider())
    ->minimumHandlePalette();
}

// --------------------------------------------------------------------------
QPalette qDMMLRangeWidget::maximumHandlePalette()const
{
  return qobject_cast<qDMMLDoubleRangeSlider*>(this->slider())
    ->maximumHandlePalette();
}

// --------------------------------------------------------------------------
void qDMMLRangeWidget::setMinimumHandlePalette(const QPalette& palette)
{
  qobject_cast<qDMMLDoubleRangeSlider*>(this->slider())
    ->setMinimumHandlePalette(palette);
}

// --------------------------------------------------------------------------
void qDMMLRangeWidget::setMaximumHandlePalette(const QPalette& palette)
{
  qobject_cast<qDMMLDoubleRangeSlider*>(this->slider())
    ->setMaximumHandlePalette(palette);
}

// --------------------------------------------------------------------------
void qDMMLRangeWidget::updateSpinBoxRange(double min, double max)
{
  // We must set the values at the same time and update the pipeline
  // when the MinSpinBox is set but not the MaxSpinBox. This could generate
  // infinite loop
  bool minSpinBoxBlocked = this->MinSpinBox->blockSignals(true);
  bool maxSpinBoxBlocked = this->MaxSpinBox->blockSignals(true);
  this->MinSpinBox->setValue(min);
  this->MaxSpinBox->setValue(max);
  this->MinSpinBox->blockSignals(minSpinBoxBlocked);
  this->MaxSpinBox->blockSignals(maxSpinBoxBlocked);
  this->updateRange();
}

// --------------------------------------------------------------------------
void qDMMLRangeWidget::updateRange()
{
  this->setRange(this->MinSpinBox->value(),
                 this->MaxSpinBox->value());
}

// --------------------------------------------------------------------------
void qDMMLRangeWidget::updateSymmetricMoves(bool symmetric)
{
  this->setSymmetricMoves(symmetric);
}

// --------------------------------------------------------------------------
void qDMMLRangeWidget::setSymmetricMoves(bool symmetry)
{
  if (symmetry==this->symmetricMoves())
    {
    return;
    }
  ctkRangeWidget::setSymmetricMoves(symmetry);
  const QSignalBlocker blocker(this->SymmetricAction);
  this->SymmetricAction->setChecked(symmetry);
}

//-----------------------------------------------------------------------------
void qDMMLRangeWidget::setQuantity(const QString& quantity)
{
  if (quantity == this->quantity())
    {
    return;
    }

  this->MinSpinBox->setQuantity(quantity);
  this->MaxSpinBox->setQuantity(quantity);
}

//-----------------------------------------------------------------------------
QString qDMMLRangeWidget::quantity()const
{
  Q_ASSERT(this->MinSpinBox->quantity() == this->MaxSpinBox->quantity());
  return this->MinSpinBox->quantity();
}

// --------------------------------------------------------------------------
vtkDMMLScene* qDMMLRangeWidget::dmmlScene()const
{
  Q_ASSERT(this->MinSpinBox->dmmlScene() == this->MaxSpinBox->dmmlScene());
  return this->MinSpinBox->dmmlScene();
}

// --------------------------------------------------------------------------
void qDMMLRangeWidget::setDMMLScene(vtkDMMLScene* scene)
{
  if (this->dmmlScene() == scene)
    {
    return;
    }

  this->MinSpinBox->setDMMLScene(scene);
  this->MaxSpinBox->setDMMLScene(scene);
  this->setEnabled(this->isEnabled() && scene != nullptr);
}

// --------------------------------------------------------------------------
// qDMMLDoubleRangeSlider
//

// --------------------------------------------------------------------------
qDMMLDoubleRangeSlider::qDMMLDoubleRangeSlider(QWidget* parentWidget)
  :ctkDoubleRangeSlider(parentWidget)
{
  this->setSlider(new qDMMLRangeSlider(nullptr));
}

// --------------------------------------------------------------------------
QPalette qDMMLDoubleRangeSlider::minimumHandlePalette()const
{
  return qobject_cast<qDMMLRangeSlider*>(this->slider())
    ->minimumHandlePalette();
}

// --------------------------------------------------------------------------
QPalette qDMMLDoubleRangeSlider::maximumHandlePalette()const
{
  return qobject_cast<qDMMLRangeSlider*>(this->slider())
    ->maximumHandlePalette();
}

// --------------------------------------------------------------------------
void qDMMLDoubleRangeSlider::setMinimumHandlePalette(const QPalette& palette)
{
  qobject_cast<qDMMLRangeSlider*>(this->slider())
    ->setMinimumHandlePalette(palette);
}

// --------------------------------------------------------------------------
void qDMMLDoubleRangeSlider::setMaximumHandlePalette(const QPalette& palette)
{
  qobject_cast<qDMMLRangeSlider*>(this->slider())
    ->setMaximumHandlePalette(palette);
}

// --------------------------------------------------------------------------
// qDMMLRangeSlider
//

// --------------------------------------------------------------------------
class qDMMLRangeSliderPrivate
{
  Q_DECLARE_PUBLIC(qDMMLRangeSlider);
protected:
  qDMMLRangeSlider* const q_ptr;

public:
  qDMMLRangeSliderPrivate(qDMMLRangeSlider* widget);

  QPalette MinimumPalette;
  QPalette MaximumPalette;
};

// --------------------------------------------------------------------------
qDMMLRangeSliderPrivate::qDMMLRangeSliderPrivate(qDMMLRangeSlider* pub)
  : q_ptr(pub)
{
}

// --------------------------------------------------------------------------
qDMMLRangeSlider::qDMMLRangeSlider(QWidget* parentWidget)
  :ctkRangeSlider(parentWidget)
   ,d_ptr(new qDMMLRangeSliderPrivate(this))
{
}

// --------------------------------------------------------------------------
qDMMLRangeSlider::~qDMMLRangeSlider() = default;

// --------------------------------------------------------------------------
QPalette qDMMLRangeSlider::minimumHandlePalette()const
{
  Q_D(const qDMMLRangeSlider);
  return d->MinimumPalette;
}

// --------------------------------------------------------------------------
QPalette qDMMLRangeSlider::maximumHandlePalette()const
{
  Q_D(const qDMMLRangeSlider);
  return d->MaximumPalette;
}

// --------------------------------------------------------------------------
void qDMMLRangeSlider::setMinimumHandlePalette(const QPalette& palette)
{
  Q_D(qDMMLRangeSlider);
  d->MinimumPalette = palette;
  this->update();
}

// --------------------------------------------------------------------------
void qDMMLRangeSlider::setMaximumHandlePalette(const QPalette& palette)
{
  Q_D(qDMMLRangeSlider);
  d->MaximumPalette = palette;
  this->update();
}

//---------------------------------------------------------------------------
void qDMMLRangeSlider::initMinimumSliderStyleOption(QStyleOptionSlider* option) const
{
  Q_D(const qDMMLRangeSlider);
  this->ctkRangeSlider::initStyleOption(option);
  option->palette = d->MinimumPalette;
}

//---------------------------------------------------------------------------
void qDMMLRangeSlider::initMaximumSliderStyleOption(QStyleOptionSlider* option) const
{
  Q_D(const qDMMLRangeSlider);
  this->ctkRangeSlider::initStyleOption(option);
  option->palette = d->MaximumPalette;
}
