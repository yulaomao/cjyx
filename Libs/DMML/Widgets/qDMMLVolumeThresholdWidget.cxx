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
#include "qDMMLVolumeThresholdWidget.h"
#include "qDMMLVolumeWidget_p.h"
#include "ui_qDMMLVolumeThresholdWidget.h"

// DMML includes
#include "vtkDMMLScalarVolumeDisplayNode.h"

//-----------------------------------------------------------------------------
class qDMMLVolumeThresholdWidgetPrivate
  : public qDMMLVolumeWidgetPrivate
  , public Ui_qDMMLVolumeThresholdWidget
{
  Q_DECLARE_PUBLIC(qDMMLVolumeThresholdWidget);
protected:
  typedef qDMMLVolumeWidgetPrivate Superclass;

public:
  qDMMLVolumeThresholdWidgetPrivate(qDMMLVolumeThresholdWidget& object);
  void init() override;

  bool blockSignals(bool block) override;
  void setRange(double min, double max) override;
  void setDecimals(int decimals) override;
  void setSingleStep(double singleStep) override;
};

// --------------------------------------------------------------------------
qDMMLVolumeThresholdWidgetPrivate
::qDMMLVolumeThresholdWidgetPrivate(qDMMLVolumeThresholdWidget& object)
  : Superclass(object)
{
}

// --------------------------------------------------------------------------
void qDMMLVolumeThresholdWidgetPrivate::init()
{
  Q_Q(qDMMLVolumeThresholdWidget);

  this->Superclass::init();
  this->setupUi(q);
  this->VolumeThresholdRangeWidget->minimumSpinBox()->setDecimalsOption(
    ctkDoubleSpinBox::DecimalsByKey|ctkDoubleSpinBox::DecimalsByShortcuts);
  this->VolumeThresholdRangeWidget->maximumSpinBox()->setDecimalsOption(
    ctkDoubleSpinBox::DecimalsByKey|ctkDoubleSpinBox::DecimalsByShortcuts);

  q->setAutoThreshold(qDMMLVolumeThresholdWidget::Off);

  this->connect(this->VolumeThresholdRangeWidget, SIGNAL(valuesChanged(double,double)),
                q, SLOT(setThreshold(double,double)));

  this->connect(this->AutoManualComboBox, SIGNAL(currentIndexChanged(int)),
                q, SLOT(setAutoThreshold(int)));

  this->RangeButton->setMenu(this->OptionsMenu);
  this->RangeButton->setPopupMode(QToolButton::InstantPopup);

}


// --------------------------------------------------------------------------
bool qDMMLVolumeThresholdWidgetPrivate::blockSignals(bool block)
{
  bool res = this->Superclass::blockSignals(block);
  this->VolumeThresholdRangeWidget->blockSignals(block);
  return res;
}

// --------------------------------------------------------------------------
void qDMMLVolumeThresholdWidgetPrivate::setRange(double min, double max)
{
  this->Superclass::setRange(min, max);
  this->VolumeThresholdRangeWidget->setRange(min, max);
}

// --------------------------------------------------------------------------
void qDMMLVolumeThresholdWidgetPrivate::setDecimals(int decimals)
{
  this->Superclass::setDecimals(decimals);
  this->VolumeThresholdRangeWidget->setDecimals(decimals);
}

// --------------------------------------------------------------------------
void qDMMLVolumeThresholdWidgetPrivate::setSingleStep(double singleStep)
{
  this->Superclass::setSingleStep(singleStep);
  this->VolumeThresholdRangeWidget->setSingleStep(singleStep);
}

// --------------------------------------------------------------------------
qDMMLVolumeThresholdWidget::qDMMLVolumeThresholdWidget(QWidget* parentWidget)
  : Superclass(new qDMMLVolumeThresholdWidgetPrivate(*this), parentWidget)
{
  Q_D(qDMMLVolumeThresholdWidget);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLVolumeThresholdWidget::~qDMMLVolumeThresholdWidget() = default;

// --------------------------------------------------------------------------
void qDMMLVolumeThresholdWidget::setThresholdBounds(double min, double max)
{
  Q_D(qDMMLVolumeThresholdWidget);
  d->setRange(min, max);
}

// --------------------------------------------------------------------------
void qDMMLVolumeThresholdWidget::setAutoThreshold(int autoThreshold)
{
  this->setAutoThreshold(static_cast<ControlMode>(autoThreshold));
}

// --------------------------------------------------------------------------
void qDMMLVolumeThresholdWidget::setAutoThreshold(ControlMode autoThreshold)
{
  Q_D(qDMMLVolumeThresholdWidget);

  if (!d->VolumeDisplayNode)
    {
    return;
    }
  int oldAuto = d->VolumeDisplayNode->GetAutoThreshold();
  int oldApply = d->VolumeDisplayNode->GetApplyThreshold();

  int disabledModify = d->VolumeDisplayNode->StartModify();
  if (autoThreshold == qDMMLVolumeThresholdWidget::Off)
    {
    d->VolumeDisplayNode->SetApplyThreshold(0);
    }
  else
    {
    d->VolumeDisplayNode->SetApplyThreshold(1);
    d->VolumeDisplayNode->SetAutoThreshold(
      autoThreshold == qDMMLVolumeThresholdWidget::Auto ? 1 : 0);
    }

  if (!oldApply && autoThreshold == qDMMLVolumeThresholdWidget::Manual)
    {
    // Previously the threshold was turned off and now it is set to manual.
    // Since the default threshold range is VTK_SHORT_MIN to VTK_SHORT_MAX,
    // we don't want these values to appear on the GUI but instead set
    // the threshold range to the full scalar range of the volume (because
    // this corresponds to the previous state of the thresholding: having
    // the full scalar range of the volume in the threshold range).
    d->VolumeDisplayNode->SetThreshold(d->DisplayScalarRange[0], d->DisplayScalarRange[1]);
    }

  d->VolumeDisplayNode->EndModify(disabledModify);

  if (oldAuto != d->VolumeDisplayNode->GetAutoThreshold() ||
      oldApply != d->VolumeDisplayNode->GetApplyThreshold())
    {
    emit this->autoThresholdValueChanged(autoThreshold);
    }
}

// --------------------------------------------------------------------------
qDMMLVolumeThresholdWidget::ControlMode qDMMLVolumeThresholdWidget
::autoThreshold() const
{
  Q_D(const qDMMLVolumeThresholdWidget);
  return static_cast<ControlMode>(d->AutoManualComboBox->currentIndex());
}

// --------------------------------------------------------------------------
bool qDMMLVolumeThresholdWidget::isOff() const
{
  Q_D(const qDMMLVolumeThresholdWidget);
  return d->AutoManualComboBox->currentIndex() == qDMMLVolumeThresholdWidget::Off;
}

// --------------------------------------------------------------------------
void qDMMLVolumeThresholdWidget::setThreshold(double lowerThreshold, double upperThreshold)
{
  Q_D(qDMMLVolumeThresholdWidget);
  if (d->VolumeDisplayNode)
    {
    double oldLowerThreshold = d->VolumeDisplayNode->GetLowerThreshold();
    double oldUpperThreshold  = d->VolumeDisplayNode->GetUpperThreshold();

    int wasModify = d->VolumeDisplayNode->StartModify();
    d->VolumeDisplayNode->SetLowerThreshold(lowerThreshold);
    d->VolumeDisplayNode->SetUpperThreshold(upperThreshold);
    bool changed =
      (oldLowerThreshold != d->VolumeDisplayNode->GetLowerThreshold() ||
       oldUpperThreshold != d->VolumeDisplayNode->GetUpperThreshold());
    if (changed)
      {
      this->setAutoThreshold(qDMMLVolumeThresholdWidget::Manual);
      emit this->thresholdValuesChanged(lowerThreshold, upperThreshold);
      }
    d->VolumeDisplayNode->EndModify(wasModify);
    }
}

// --------------------------------------------------------------------------
void qDMMLVolumeThresholdWidget::setLowerThreshold(double lowerThreshold)
{
  Q_D(qDMMLVolumeThresholdWidget);
  if (d->VolumeDisplayNode)
    {
    double upperThreshold  = d->VolumeDisplayNode->GetUpperThreshold();
    this->setThreshold(lowerThreshold, upperThreshold);
    }
}

// --------------------------------------------------------------------------
void qDMMLVolumeThresholdWidget::setUpperThreshold(double upperThreshold)
{
  Q_D(qDMMLVolumeThresholdWidget);
  if (d->VolumeDisplayNode)
    {
    double lowerThreshold = d->VolumeDisplayNode->GetLowerThreshold();
    this->setThreshold(lowerThreshold, upperThreshold);
    }
}

// --------------------------------------------------------------------------
double qDMMLVolumeThresholdWidget::lowerThreshold() const
{
  Q_D(const qDMMLVolumeThresholdWidget);
  return d->VolumeThresholdRangeWidget->minimumValue();
}

// --------------------------------------------------------------------------
double qDMMLVolumeThresholdWidget::upperThreshold() const
{
  Q_D(const qDMMLVolumeThresholdWidget);
  return d->VolumeThresholdRangeWidget->maximumValue();
}

// --------------------------------------------------------------------------
double qDMMLVolumeThresholdWidget::lowerThresholdBound() const
{
  Q_D(const qDMMLVolumeThresholdWidget);
  return d->VolumeThresholdRangeWidget->minimum();
}

// --------------------------------------------------------------------------
double qDMMLVolumeThresholdWidget::upperThresholdBound() const
{
  Q_D(const qDMMLVolumeThresholdWidget);
  return d->VolumeThresholdRangeWidget->maximum();
}

// --------------------------------------------------------------------------
void qDMMLVolumeThresholdWidget::setLowerThresholdBound(double lowerThresholdBound)
{
  double upperThresholdBound = this->upperThresholdBound();
  this->setThresholdBounds(lowerThresholdBound, upperThresholdBound);
}

// --------------------------------------------------------------------------
void qDMMLVolumeThresholdWidget::setUpperThresholdBound(double upperThresholdBound)
{
  double lowerThresholdBound = this->lowerThresholdBound();
  this->setThresholdBounds(lowerThresholdBound, upperThresholdBound);
}

// --------------------------------------------------------------------------
void qDMMLVolumeThresholdWidget::setMinimum(double min)
{
  Q_D(qDMMLVolumeThresholdWidget);
  d->VolumeThresholdRangeWidget->setMinimum(min);
}

// --------------------------------------------------------------------------
void qDMMLVolumeThresholdWidget::setMaximum(double max)
{
  Q_D(qDMMLVolumeThresholdWidget);
  d->VolumeThresholdRangeWidget->setMaximum(max);
}

// --------------------------------------------------------------------------
void qDMMLVolumeThresholdWidget::updateWidgetFromDMMLDisplayNode()
{
  Q_D(qDMMLVolumeThresholdWidget);
  Superclass::updateWidgetFromDMMLDisplayNode();

  if (!d->VolumeDisplayNode)
    {
    return;
    }

  // We don't want the slider to fire signals saying that the threshold values
  // have changed, it would set the AutoThrehold mode to Manual automatically
  // even if the values have been just set programmatically/automatically.
  bool wasBlocking = d->VolumeThresholdRangeWidget->blockSignals(true);

  const int autoThresh = d->VolumeDisplayNode->GetAutoThreshold();
  const int applyThresh = d->VolumeDisplayNode->GetApplyThreshold();
  // 0 = auto, 1 = manual, 2 = off
  ControlMode index = (applyThresh == 0) ? qDMMLVolumeThresholdWidget::Off :
                      (autoThresh == 1) ? qDMMLVolumeThresholdWidget::Auto :
                      qDMMLVolumeThresholdWidget::Manual;
  d->AutoManualComboBox->setCurrentIndex(index);

  if (applyThresh)
    {
    // Thresholding is on.
    // Show the threshold values (and widen the slider's range if needed).
    double range[2] = { 0.0, 0.0 };
    d->VolumeThresholdRangeWidget->range(range);
    double lower = d->VolumeDisplayNode->GetLowerThreshold();
    double upper = d->VolumeDisplayNode->GetUpperThreshold();
    double minRangeValue = std::min(range[0], lower);
    double maxRangeValue = std::max(range[1], upper);
    d->setRange(minRangeValue, maxRangeValue);
    d->VolumeThresholdRangeWidget->setValues(lower, upper);
    }
  else
    {
    // Thresholding is off.
    // Instead of showing the threshold values (which may be very large),
    // show the volume's entire scalar range.
    double minRangeValue = d->DisplayScalarRange[0];
    double maxRangeValue = d->DisplayScalarRange[1];
    d->setRange(minRangeValue, maxRangeValue);
    d->VolumeThresholdRangeWidget->setValues(minRangeValue, maxRangeValue);
    }
  d->VolumeThresholdRangeWidget->blockSignals(wasBlocking);
}
