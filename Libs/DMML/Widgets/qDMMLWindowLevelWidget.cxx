/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// qDMML includes
#include "qDMMLSpinBox.h"
#include "qDMMLVolumeWidget_p.h"
#include "qDMMLWindowLevelWidget.h"
#include "ui_qDMMLWindowLevelWidget.h"

// DMML includes
#include "vtkDMMLScalarVolumeDisplayNode.h"

//-----------------------------------------------------------------------------
class qDMMLWindowLevelWidgetPrivate
  : public qDMMLVolumeWidgetPrivate
  , public Ui_qDMMLWindowLevelWidget
{
  Q_DECLARE_PUBLIC(qDMMLWindowLevelWidget);
protected:
  typedef qDMMLVolumeWidgetPrivate Superclass;

public:
  qDMMLWindowLevelWidgetPrivate(qDMMLWindowLevelWidget& object);
  ~qDMMLWindowLevelWidgetPrivate() override;
  void init() override;

  bool blockSignals(bool block) override;
  void setRange(double min, double max) override;
  void setDecimals(int decimals) override;
  void setSingleStep(double singleStep) override;
};

// --------------------------------------------------------------------------
qDMMLWindowLevelWidgetPrivate
::qDMMLWindowLevelWidgetPrivate(qDMMLWindowLevelWidget& object)
  : Superclass(object)
{
}

// --------------------------------------------------------------------------
qDMMLWindowLevelWidgetPrivate::~qDMMLWindowLevelWidgetPrivate() = default;

// --------------------------------------------------------------------------
void qDMMLWindowLevelWidgetPrivate::init()
{
  Q_Q(qDMMLWindowLevelWidget);

  this->Superclass::init();
  this->setupUi(q);

  q->setAutoWindowLevel(qDMMLWindowLevelWidget::Auto);

  QObject::connect(this->WindowLevelRangeSlider, SIGNAL(valuesChanged(double,double)),
                   q, SLOT(setMinMaxRangeValue(double,double)));

  QObject::connect(this->WindowSpinBox, SIGNAL(valueChanged(double)),
                   q, SLOT(setWindow(double)));
  QObject::connect(this->LevelSpinBox, SIGNAL(valueChanged(double)),
                   q, SLOT(setLevel(double)));

  QObject::connect(this->MinSpinBox, SIGNAL(valueChanged(double)),
                   q, SLOT(setMinimumValue(double)));
  QObject::connect(this->MaxSpinBox, SIGNAL(valueChanged(double)),
                   q, SLOT(setMaximumValue(double)));
  this->MinSpinBox->setVisible(false);
  this->MaxSpinBox->setVisible(false);

  QObject::connect(this->AutoManualComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(setAutoWindowLevel(int)));

  this->RangeButton->setMenu(this->OptionsMenu);
  this->RangeButton->setPopupMode(QToolButton::InstantPopup);

}

// --------------------------------------------------------------------------
bool qDMMLWindowLevelWidgetPrivate::blockSignals(bool block)
{
  bool res = this->Superclass::blockSignals(block);
  this->WindowLevelRangeSlider->blockSignals(block);
  this->WindowSpinBox->blockSignals(block);
  this->LevelSpinBox->blockSignals(block);
  this->MinSpinBox->blockSignals(block);
  this->MaxSpinBox->blockSignals(block);
  return res;
}

// --------------------------------------------------------------------------
void qDMMLWindowLevelWidgetPrivate::setRange(double min, double max)
{
  this->Superclass::setRange(min, max);
  this->WindowLevelRangeSlider->setRange(min, max);
  this->WindowSpinBox->setRange(0, max - min);
  this->LevelSpinBox->setRange(min, max);
  this->MinSpinBox->setRange(min, max);
  this->MaxSpinBox->setRange(min, max);
  //this->RangeWidget->setValues(min, max);
}

// --------------------------------------------------------------------------
void qDMMLWindowLevelWidgetPrivate::setDecimals(int decimals)
{
  this->Superclass::setDecimals(decimals);
  this->WindowSpinBox->setDecimals(decimals);
  this->LevelSpinBox->setDecimals(decimals);
  this->MinSpinBox->setDecimals(decimals);
  this->MaxSpinBox->setDecimals(decimals);
}

// --------------------------------------------------------------------------
void qDMMLWindowLevelWidgetPrivate::setSingleStep(double singleStep)
{
  this->Superclass::setSingleStep(singleStep);
  this->WindowLevelRangeSlider->setSingleStep(singleStep);
  this->WindowSpinBox->setSingleStep(singleStep);
  this->LevelSpinBox->setSingleStep(singleStep);
  this->MinSpinBox->setSingleStep(singleStep);
  this->MaxSpinBox->setSingleStep(singleStep);
}

// --------------------------------------------------------------------------
qDMMLWindowLevelWidget::qDMMLWindowLevelWidget(QWidget* parentWidget)
  : Superclass(new qDMMLWindowLevelWidgetPrivate(*this), parentWidget)
{
  Q_D(qDMMLWindowLevelWidget);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLWindowLevelWidget::~qDMMLWindowLevelWidget() = default;

// --------------------------------------------------------------------------
void qDMMLWindowLevelWidget::setAutoWindowLevel(ControlMode autoWindowLevel)
{
  Q_D(qDMMLWindowLevelWidget);

  if (!d->VolumeDisplayNode)
    {
    return;
    }

  bool blocked = d->AutoManualComboBox->blockSignals(true);
  if (d->AutoManualComboBox->currentIndex() != autoWindowLevel)
  {
    d->AutoManualComboBox->setCurrentIndex(autoWindowLevel);
  }
  d->AutoManualComboBox->blockSignals(blocked);

  int oldAuto = d->VolumeDisplayNode->GetAutoWindowLevel();

  //int disabledModify = this->VolumeDisplayNode->StartModify();
  d->VolumeDisplayNode->SetAutoWindowLevel(
    autoWindowLevel == qDMMLWindowLevelWidget::Auto ? 1 : 0);
  //this->VolumeDisplayNode->EndModify(disabledModify);

  switch (autoWindowLevel)
    {
    case qDMMLWindowLevelWidget::ManualMinMax:
      d->WindowLevelRangeSlider->setSymmetricMoves(false);
      d->WindowSpinBox->setVisible(false);
      d->LevelSpinBox->setVisible(false);
      d->MinSpinBox->setVisible(true);
      d->MaxSpinBox->setVisible(true);
      break;
    default:
    case qDMMLWindowLevelWidget::Auto:
    case qDMMLWindowLevelWidget::Manual:
      d->WindowLevelRangeSlider->setSymmetricMoves(true);
      d->MinSpinBox->setVisible(false);
      d->MaxSpinBox->setVisible(false);
      d->WindowSpinBox->setVisible(true);
      d->LevelSpinBox->setVisible(true);
      break;
    }

  if (autoWindowLevel != oldAuto)
    {
    emit this->autoWindowLevelValueChanged(
      autoWindowLevel == qDMMLWindowLevelWidget::Auto ?
        qDMMLWindowLevelWidget::Auto : qDMMLWindowLevelWidget::Manual);
    }
}

// --------------------------------------------------------------------------
void qDMMLWindowLevelWidget::setAutoWindowLevel(int autoWindowLevel)
{
  switch(autoWindowLevel)
    {
    case qDMMLWindowLevelWidget::Auto:
      this->setAutoWindowLevel(qDMMLWindowLevelWidget::Auto);
      break;
    case qDMMLWindowLevelWidget::Manual:
      this->setAutoWindowLevel(qDMMLWindowLevelWidget::Manual);
      break;
    case qDMMLWindowLevelWidget::ManualMinMax:
      this->setAutoWindowLevel(qDMMLWindowLevelWidget::ManualMinMax);
      break;
    default:
      break;
    }
}

// --------------------------------------------------------------------------
qDMMLWindowLevelWidget::ControlMode qDMMLWindowLevelWidget::autoWindowLevel() const
{
  Q_D(const qDMMLWindowLevelWidget);
  switch (d->AutoManualComboBox->currentIndex())
    {
    case qDMMLWindowLevelWidget::Auto:
      return qDMMLWindowLevelWidget::Auto;
      break;
    case qDMMLWindowLevelWidget::Manual:
      return qDMMLWindowLevelWidget::Manual;
      break;
    case qDMMLWindowLevelWidget::ManualMinMax:
      return qDMMLWindowLevelWidget::ManualMinMax;
      break;
    }
  return qDMMLWindowLevelWidget::Manual;
}

// --------------------------------------------------------------------------
void qDMMLWindowLevelWidget::setWindowLevel(double window, double level)
{
  Q_D(const qDMMLWindowLevelWidget);
  if (!d->VolumeDisplayNode)
    {
    return;
    }
  double oldWindow = d->VolumeDisplayNode->GetWindow();
  double oldLevel  = d->VolumeDisplayNode->GetLevel();

  int disabledModify = d->VolumeDisplayNode->StartModify();
  d->VolumeDisplayNode->SetWindowLevel(window, level);
  if (!qFuzzyCompare(oldWindow, d->VolumeDisplayNode->GetWindow()) ||
      !qFuzzyCompare(oldLevel, d->VolumeDisplayNode->GetLevel()))
    {
    if (this->autoWindowLevel() == qDMMLWindowLevelWidget::Auto)
      {
      this->setAutoWindowLevel(qDMMLWindowLevelWidget::Manual);
      }
    emit this->windowLevelValuesChanged(window, level);
    }
  d->VolumeDisplayNode->EndModify(disabledModify);
}

// --------------------------------------------------------------------------
void qDMMLWindowLevelWidget::setMinMaxRangeValue(double min, double max)
{
  double window = max - min;
  double level = 0.5*(min+max);

  this->setWindowLevel(window, level);
}

// --------------------------------------------------------------------------
void qDMMLWindowLevelWidget::setWindow(double window)
{
  Q_D(const qDMMLWindowLevelWidget);
  if (d->VolumeDisplayNode)
    {
    double level  = d->VolumeDisplayNode->GetLevel();
    this->setWindowLevel(window, level);
    }
}

// --------------------------------------------------------------------------
void qDMMLWindowLevelWidget::setLevel(double level)
{
  Q_D(qDMMLWindowLevelWidget);
  if (d->VolumeDisplayNode)
    {
    double window = d->VolumeDisplayNode->GetWindow();
    this->setWindowLevel(window, level);
    }
}

// --------------------------------------------------------------------------
double qDMMLWindowLevelWidget::window() const
{
  Q_D(const qDMMLWindowLevelWidget);

  double min = d->WindowLevelRangeSlider->minimumValue();
  double max = d->WindowLevelRangeSlider->maximumValue();

  return max - min;
}

// --------------------------------------------------------------------------
double qDMMLWindowLevelWidget::minimumValue() const
{
  Q_D(const qDMMLWindowLevelWidget);

  double min = d->WindowLevelRangeSlider->minimumValue();
  return min;
}

// --------------------------------------------------------------------------
double qDMMLWindowLevelWidget::maximumValue() const
{
  Q_D(const qDMMLWindowLevelWidget);

  double max = d->WindowLevelRangeSlider->maximumValue();
  return max;
}

// --------------------------------------------------------------------------
double qDMMLWindowLevelWidget::minimumBound() const
{
  Q_D(const qDMMLWindowLevelWidget);

  double min = d->WindowLevelRangeSlider->minimum();
  return min;
}

// --------------------------------------------------------------------------
double qDMMLWindowLevelWidget::maximumBound() const
{
  Q_D(const qDMMLWindowLevelWidget);

  double max = d->WindowLevelRangeSlider->maximum();
  return max;
}

// --------------------------------------------------------------------------
double qDMMLWindowLevelWidget::level() const
{
  Q_D(const qDMMLWindowLevelWidget);

  double min = d->WindowLevelRangeSlider->minimumValue();
  double max = d->WindowLevelRangeSlider->maximumValue();

  return 0.5*(max + min);
}

// --------------------------------------------------------------------------
void qDMMLWindowLevelWidget::setMinimumValue(double min)
{
  this->setMinMaxRangeValue(min, this->maximumValue());
}

// --------------------------------------------------------------------------
void qDMMLWindowLevelWidget::setMaximumValue(double max)
{
  this->setMinMaxRangeValue(this->minimumValue(), max);
}

// --------------------------------------------------------------------------
void qDMMLWindowLevelWidget::setMinimumBound(double min)
{
  this->setMinMaxBounds(min, this->maximumBound());
}

// --------------------------------------------------------------------------
void qDMMLWindowLevelWidget::setMaximumBound(double max)
{
  this->setMinMaxBounds(this->minimumBound(), max);
}

// --------------------------------------------------------------------------
void qDMMLWindowLevelWidget::setMinMaxBounds(double min, double max)
{
  Q_D(qDMMLWindowLevelWidget);
  d->setRange(min, max);
}

// --------------------------------------------------------------------------
void qDMMLWindowLevelWidget::updateWidgetFromDMMLVolumeNode()
{
  Q_D(qDMMLVolumeWidget);
  this->Superclass::updateWidgetFromDMMLVolumeNode();

  vtkDMMLScalarVolumeDisplayNode* displayNode = this->dmmlDisplayNode();
  if (!displayNode)
    {
    return;
    }

  double window = displayNode->GetWindow();
  double windowLevelMin = displayNode->GetWindowLevelMin();
  double windowLevelMax = displayNode->GetWindowLevelMax();

  // We block here to prevent the widgets to call setWindowLevel which could
  // change the AutoLevel from Auto into Manual.
  bool blocked = d->blockSignals(true);

  double sliderRange[2] = { windowLevelMin, windowLevelMax };
  if (window < 10.)
    {
    // unusually small range, make the slider range a bit larger than the current values
    sliderRange[0] = sliderRange[0] - window * 0.1;
    sliderRange[1] = sliderRange[1] + window * 0.1;
    }
  else
    {
    // usual range for CT, MRI, etc. make the slider range minimum +/- 600
    sliderRange[0] = qMin(-600., sliderRange[0] - window * 0.1);
    sliderRange[1] = qMax(600., sliderRange[1] + window * 0.1);
    }
  d->setRange(sliderRange[0], sliderRange[1]);

  d->blockSignals(blocked);
}

// --------------------------------------------------------------------------
void qDMMLWindowLevelWidget::updateWidgetFromDMMLDisplayNode()
{
  Q_D(qDMMLWindowLevelWidget);
  this->Superclass::updateWidgetFromDMMLDisplayNode();
  if (!d->VolumeDisplayNode)
    {
    return;
    }

  double window = d->VolumeDisplayNode->GetWindow();
  double level = d->VolumeDisplayNode->GetLevel();
  double windowLevelMin = d->VolumeDisplayNode->GetWindowLevelMin();
  double windowLevelMax = d->VolumeDisplayNode->GetWindowLevelMax();

  // We block here to prevent the widgets to call setWindowLevel which could
  // change the AutoLevel from Auto into Manual.
  bool blocked = d->blockSignals(true);

  // WindowLevelMinMax might have been set to values outside the current range
  const double minRangeValue = std::min(windowLevelMin, d->MinRangeSpinBox->value());
  const double maxRangeValue = std::max(windowLevelMax, d->MaxRangeSpinBox->value());
  d->setRange(minRangeValue, maxRangeValue);

  d->WindowSpinBox->setValue(window);
  d->LevelSpinBox->setValue(level);
  d->WindowLevelRangeSlider->setValues(windowLevelMin, windowLevelMax);
  d->MinSpinBox->setValue(windowLevelMin);
  d->MaxSpinBox->setValue(windowLevelMax);

  d->blockSignals(blocked);

  switch (d->VolumeDisplayNode->GetAutoWindowLevel())
    {
    case 1:
      d->AutoManualComboBox->setCurrentIndex(qDMMLWindowLevelWidget::Auto);
      break;
    case 0:
      if (d->AutoManualComboBox->currentIndex() == qDMMLWindowLevelWidget::Auto)
        {
        d->AutoManualComboBox->setCurrentIndex(qDMMLWindowLevelWidget::Manual);
        }
      break;
    }
}
