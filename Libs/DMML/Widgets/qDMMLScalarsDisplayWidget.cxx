/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright 2020 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QDebug>

// CTK includes
#include "ctkUtils.h"

// qDMML includes
#include "qDMMLScalarsDisplayWidget.h"
#include "ui_qDMMLScalarsDisplayWidget.h"

// DMML include
#include <vtkDMMLScene.h>
#include <vtkDMMLColorTableNode.h>
#include <vtkDMMLDisplayableNode.h>
#include <vtkDMMLDisplayNode.h>
#include <vtkDMMLModelDisplayNode.h>

// VTK includes
#include <vtkPointSet.h>

//-----------------------------------------------------------------------------
class qDMMLScalarsDisplayWidgetPrivate : public Ui_qDMMLScalarsDisplayWidget
{
  Q_DECLARE_PUBLIC(qDMMLScalarsDisplayWidget);

protected:
  qDMMLScalarsDisplayWidget* const q_ptr;

public:
  qDMMLScalarsDisplayWidgetPrivate(qDMMLScalarsDisplayWidget& object);
  void init();

  QList<vtkDMMLModelDisplayNode*> currentModelDisplayNodes()const;

public:
  QList< vtkWeakPointer<vtkDMMLDisplayNode> > CurrentDisplayNodes;

  // Store what data range was used to automatically slider range of display range,
  // to prevent resetting slider range when user moves the slider.
  double DataRangeUsedForAutoDisplayRange[2];
};

//------------------------------------------------------------------------------
qDMMLScalarsDisplayWidgetPrivate::qDMMLScalarsDisplayWidgetPrivate(qDMMLScalarsDisplayWidget& object)
  : q_ptr(&object)
{
  this->DataRangeUsedForAutoDisplayRange[0] = 0.0;
  this->DataRangeUsedForAutoDisplayRange[1] = 0.0;
}

// --------------------------------------------------------------------------
void qDMMLScalarsDisplayWidgetPrivate::init()
{
  Q_Q(qDMMLScalarsDisplayWidget);

  this->setupUi(q);

  // Scalar
  QObject::connect(this->ScalarsVisibilityCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setScalarsVisibility(bool)));
  QObject::connect(this->ActiveScalarComboBox, SIGNAL(activated(int)),
    q, SLOT(onCurrentArrayActivated()));
  QObject::connect(this->ScalarsColorNodeComboBox,
    SIGNAL(currentNodeChanged(vtkDMMLNode*)), q, SLOT(setScalarsColorNode(vtkDMMLNode*)));
  // scalar range
  QObject::connect(this->DisplayedScalarRangeModeComboBox, SIGNAL(currentIndexChanged(int)),
    q, SLOT(setScalarRangeMode(int)));
  QObject::connect(this->DisplayedScalarRangeWidget, SIGNAL(valuesChanged(double,double)),
    q, SLOT(setScalarsDisplayRange(double,double)));

  // Thresholding
  this->ThresholdCheckBox->setChecked(false);
  this->ThresholdRangeWidget->setEnabled(false);
  QObject::connect(this->ThresholdCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setTresholdEnabled(bool)));
  QObject::connect(this->ThresholdRangeWidget, SIGNAL(valuesChanged(double,double)),
                   q, SLOT(setThresholdRange(double,double)));

  // Set default range mode
  q->setScalarRangeMode(vtkDMMLDisplayNode::UseDataScalarRange);
}

//------------------------------------------------------------------------------
QList<vtkDMMLModelDisplayNode*> qDMMLScalarsDisplayWidgetPrivate::currentModelDisplayNodes()const
{
  QList<vtkDMMLModelDisplayNode*> modelDisplayNodes;
  foreach (vtkDMMLDisplayNode* displayNode, this->CurrentDisplayNodes)
    {
    vtkDMMLModelDisplayNode* modelDisplayNode = vtkDMMLModelDisplayNode::SafeDownCast(displayNode);
    if (modelDisplayNode)
      {
      modelDisplayNodes << modelDisplayNode;
      }
    }
  return modelDisplayNodes;
}

// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
qDMMLScalarsDisplayWidget::qDMMLScalarsDisplayWidget(QWidget* parentWidget)
  : qDMMLWidget(parentWidget)
  , d_ptr(new qDMMLScalarsDisplayWidgetPrivate(*this))
{
  Q_D(qDMMLScalarsDisplayWidget);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLScalarsDisplayWidget::~qDMMLScalarsDisplayWidget() = default;

//------------------------------------------------------------------------------
vtkDMMLDisplayNode* qDMMLScalarsDisplayWidget::dmmlDisplayNode()const
{
  Q_D(const qDMMLScalarsDisplayWidget);
  if (d->CurrentDisplayNodes.size() > 0)
    {
    return d->CurrentDisplayNodes[0];
    }

  return nullptr;
}

//------------------------------------------------------------------------------
QList<vtkDMMLDisplayNode*> qDMMLScalarsDisplayWidget::dmmlDisplayNodes()const
{
  Q_D(const qDMMLScalarsDisplayWidget);
  QList<vtkDMMLDisplayNode*> displayNodes; // this list will only contain valid (non-null) display node pointers
  for (vtkDMMLDisplayNode* displayNode: d->CurrentDisplayNodes)
    {
    displayNodes << displayNode;
    }
  return displayNodes;
}

//------------------------------------------------------------------------------
void qDMMLScalarsDisplayWidget::setDMMLDisplayNodes(QList<vtkDMMLDisplayNode*> displayNodes)
{
  Q_D(qDMMLScalarsDisplayWidget);

  displayNodes.removeAll(nullptr);

  // Only the first display node is observed
  qvtkReconnect(
    (d->CurrentDisplayNodes.size() > 0 ? d->CurrentDisplayNodes[0] : nullptr),
    (displayNodes.size() > 0 ? displayNodes[0] : nullptr),
    vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromDMML()));

  d->CurrentDisplayNodes.clear();
  for (vtkDMMLDisplayNode* displayNode : displayNodes)
    {
    d->CurrentDisplayNodes << displayNode;
    }

  if (d->CurrentDisplayNodes.size() > 0)
    {
    d->CurrentDisplayNodes[0]->UpdateScalarRange();
    }

  this->updateWidgetFromDMML();
}

//------------------------------------------------------------------------------
void qDMMLScalarsDisplayWidget::setDMMLDisplayNode(vtkDMMLNode* node)
{
  this->setDMMLDisplayNode(vtkDMMLDisplayNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qDMMLScalarsDisplayWidget::setDMMLDisplayNode(vtkDMMLDisplayNode* displayNode)
{
  Q_D(qDMMLScalarsDisplayWidget);
  QList<vtkDMMLDisplayNode*> displayNodes;
  if (displayNode)
    {
    displayNodes << displayNode;
    }
  this->setDMMLDisplayNodes(displayNodes);
}

//------------------------------------------------------------------------------
void qDMMLScalarsDisplayWidget::setScalarsVisibility(bool visible)
{
  Q_D(qDMMLScalarsDisplayWidget);

  foreach (vtkDMMLDisplayNode* displayNode, d->CurrentDisplayNodes)
    {
    displayNode->SetScalarVisibility(visible);
    displayNode->UpdateAssignedAttribute();
    }
}

//------------------------------------------------------------------------------
bool qDMMLScalarsDisplayWidget::scalarsVisibility()const
{
  Q_D(const qDMMLScalarsDisplayWidget);
  return d->ScalarsVisibilityCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qDMMLScalarsDisplayWidget::onCurrentArrayActivated()
{
  Q_D(qDMMLScalarsDisplayWidget);
  this->setActiveScalarName(d->ActiveScalarComboBox->currentArrayName());
}

//------------------------------------------------------------------------------
void qDMMLScalarsDisplayWidget::setActiveScalarName(const QString& arrayName)
{
  Q_D(qDMMLScalarsDisplayWidget);

  foreach (vtkDMMLDisplayNode* displayNode, d->CurrentDisplayNodes)
    {
    int wasModified = displayNode->StartModify();

    displayNode->SetActiveScalar(arrayName.toUtf8(), d->ActiveScalarComboBox->currentArrayLocation());

    // if there's no color node set for a non empty array name, use a default
    if (!arrayName.isEmpty() && displayNode->GetColorNodeID() == nullptr)
      {
      const char* colorNodeID = "vtkDMMLColorTableNodeFileViridis.txt";
      displayNode->SetAndObserveColorNodeID(colorNodeID);
      }
    displayNode->EndModify(wasModified);
    }
}

//------------------------------------------------------------------------------
QString qDMMLScalarsDisplayWidget::activeScalarName()const
{
  Q_D(const qDMMLScalarsDisplayWidget);
  // TODO: use currentArrayName()
  vtkAbstractArray* array = d->ActiveScalarComboBox->currentArray();
  return array ? array->GetName() : "";
}

//------------------------------------------------------------------------------
void qDMMLScalarsDisplayWidget::setScalarsColorNode(vtkDMMLNode* colorNode)
{
  this->setScalarsColorNode(vtkDMMLColorNode::SafeDownCast(colorNode));
}

//------------------------------------------------------------------------------
void qDMMLScalarsDisplayWidget::setScalarsColorNode(vtkDMMLColorNode* colorNode)
{
  Q_D(qDMMLScalarsDisplayWidget);

  foreach (vtkDMMLDisplayNode* displayNode, d->CurrentDisplayNodes)
    {
    if (displayNode)
      {
      displayNode->SetAndObserveColorNodeID(colorNode ? colorNode->GetID() : nullptr);
      }
    }
}

//------------------------------------------------------------------------------
vtkDMMLColorNode* qDMMLScalarsDisplayWidget::scalarsColorNode()const
{
  Q_D(const qDMMLScalarsDisplayWidget);
  return vtkDMMLColorNode::SafeDownCast(d->ScalarsColorNodeComboBox->currentNode());
}

// --------------------------------------------------------------------------
void qDMMLScalarsDisplayWidget::setScalarRangeMode(vtkDMMLDisplayNode::ScalarRangeFlagType mode)
{
  Q_D(qDMMLScalarsDisplayWidget);

  bool modified = false;
  foreach (vtkDMMLDisplayNode* displayNode, d->CurrentDisplayNodes)
    {
    int currentScalarRangeMode = displayNode->GetScalarRangeFlag();
    if (currentScalarRangeMode != mode)
      {
      displayNode->SetScalarRangeFlag(mode);
      modified = true;
      }
    }
  if (modified)
    {
    emit this->scalarRangeModeValueChanged(mode);
    }
}

// --------------------------------------------------------------------------
void qDMMLScalarsDisplayWidget::setScalarRangeMode(int scalarRangeMode)
{
  this->setScalarRangeMode((vtkDMMLDisplayNode::ScalarRangeFlagType)scalarRangeMode);
}

// --------------------------------------------------------------------------
vtkDMMLDisplayNode::ScalarRangeFlagType qDMMLScalarsDisplayWidget::scalarRangeMode() const
{
  Q_D(const qDMMLScalarsDisplayWidget);
  return (vtkDMMLDisplayNode::ScalarRangeFlagType)d->DisplayedScalarRangeModeComboBox->currentIndex();
}

//------------------------------------------------------------------------------
void qDMMLScalarsDisplayWidget::setScalarsDisplayRange(double min, double max)
{
  Q_D(qDMMLScalarsDisplayWidget);

  foreach (vtkDMMLDisplayNode* displayNode, d->CurrentDisplayNodes)
    {
    double *range = displayNode->GetScalarRange();
    if (range[0] == min && range[1] == max)
      {
      return;
      }
    displayNode->SetScalarRange(min, max);
    }
}

//------------------------------------------------------------------------------
void qDMMLScalarsDisplayWidget::setTresholdEnabled(bool b)
{
  Q_D(qDMMLScalarsDisplayWidget);

  QList<vtkDMMLModelDisplayNode*> currentModelDisplayNodes = d->currentModelDisplayNodes();
  foreach (vtkDMMLModelDisplayNode* modelDisplayNode, currentModelDisplayNodes)
    {
    modelDisplayNode->SetThresholdEnabled(b);
    }
}

//------------------------------------------------------------------------------
void qDMMLScalarsDisplayWidget::setThresholdRange(double min, double max)
{
  Q_D(qDMMLScalarsDisplayWidget);

  QList<vtkDMMLModelDisplayNode*> currentModelDisplayNodes = d->currentModelDisplayNodes();
  foreach (vtkDMMLModelDisplayNode* modelDisplayNode, currentModelDisplayNodes)
    {
    double oldMin = modelDisplayNode->GetThresholdMin();
    double oldMax = modelDisplayNode->GetThresholdMax();
    if (oldMin == min && oldMax == max)
      {
      return;
      }
    modelDisplayNode->SetThresholdRange(min, max);
    }
}

// --------------------------------------------------------------------------
double qDMMLScalarsDisplayWidget::minimumValue() const
{
  Q_D(const qDMMLScalarsDisplayWidget);

  return d->DisplayedScalarRangeWidget->minimumValue();
}

// --------------------------------------------------------------------------
double qDMMLScalarsDisplayWidget::maximumValue() const
{
  Q_D(const qDMMLScalarsDisplayWidget);

  return d->DisplayedScalarRangeWidget->maximumValue();
}

// --------------------------------------------------------------------------
void qDMMLScalarsDisplayWidget::setMinimumValue(double min)
{
  Q_D(const qDMMLScalarsDisplayWidget);

  if (d->CurrentDisplayNodes.size() > 1)
    {
    qWarning() << Q_FUNC_INFO << ": Multi-selection not supported for this property";
    }

  d->DisplayedScalarRangeWidget->setMinimumValue(min);
}

// --------------------------------------------------------------------------
void qDMMLScalarsDisplayWidget::setMaximumValue(double max)
{
  Q_D(const qDMMLScalarsDisplayWidget);

  if (d->CurrentDisplayNodes.size() > 1)
    {
    qWarning() << Q_FUNC_INFO << ": Multi-selection not supported for this property";
    }

  d->DisplayedScalarRangeWidget->setMaximumValue(max);
}

// --------------------------------------------------------------------------
void qDMMLScalarsDisplayWidget::updateWidgetFromDMML()
{
  Q_D(qDMMLScalarsDisplayWidget);

  vtkDMMLDisplayNode* firstDisplayNode = (d->CurrentDisplayNodes.size() > 0 ? d->CurrentDisplayNodes[0] : nullptr);
  vtkDMMLModelDisplayNode* firstModelDisplayNode = vtkDMMLModelDisplayNode::SafeDownCast(firstDisplayNode);

  // The Threshold section is only available for models
  d->ThresholdLabel->setVisible(firstModelDisplayNode != nullptr);
  d->ThresholdCheckBox->setVisible(firstModelDisplayNode != nullptr);
  d->ThresholdRangeWidget->setVisible(firstModelDisplayNode != nullptr);

  this->setEnabled(firstDisplayNode != nullptr);
  if (!firstDisplayNode)
    {
    emit displayNodeChanged();
    return;
    }

  bool wasBlocking = false;

  if (d->ScalarsVisibilityCheckBox->isChecked() != (bool)firstDisplayNode->GetScalarVisibility())
    {
    wasBlocking = d->ScalarsVisibilityCheckBox->blockSignals(true);
    d->ScalarsVisibilityCheckBox->setChecked(firstDisplayNode->GetScalarVisibility());
    d->ScalarsVisibilityCheckBox->blockSignals(wasBlocking);
    }

  double dataRange[2] = { 0.0, 0.0 };
  vtkDataArray* dataArray = firstDisplayNode->GetActiveScalarArray();
  if (dataArray)
    {
    dataArray->GetRange(dataRange);
    }

  // Update scalar values, range, decimals and single step
  double precision = 1.0;
  double dataMin = 0.0;
  double dataMax = 0.0;
  int decimals = 0;
  bool resetSliderRange = false;
  if (dataRange[0] < dataRange[1])
    {
    if (d->DataRangeUsedForAutoDisplayRange[0] != dataRange[0]
      || d->DataRangeUsedForAutoDisplayRange[1] != dataRange[1])
      {
      d->DataRangeUsedForAutoDisplayRange[0] = dataRange[0];
      d->DataRangeUsedForAutoDisplayRange[1] = dataRange[1];
      resetSliderRange = true;
      }
    // Begin with a precision of 1% of the range
    precision = dataRange[1]/100.0 - dataRange[0]/100.0;
    // Extend min/max by 20% to give some room to work with
    dataMin = (floor(dataRange[0]/precision) - 20 ) * precision;
    dataMax = (ceil(dataRange[1]/precision) + 20 ) * precision;
    // Use closest power of ten value as a step value
    precision = ctk::closestPowerOfTen(precision);
    // Find significant decimals to show
    double stepDecimals = ctk::significantDecimals(precision);
    double minDecimals = ctk::significantDecimals(dataRange[0]);
    double maxDecimals = ctk::significantDecimals(dataRange[1]);
    decimals = std::max(stepDecimals, std::max(minDecimals, maxDecimals));
    }

  double* displayRange = firstDisplayNode->GetScalarRange();

  wasBlocking = d->DisplayedScalarRangeWidget->blockSignals(true);
  if (resetSliderRange)
    {
    d->DisplayedScalarRangeWidget->setRange(std::min(dataMin, displayRange[0]), std::max(dataMax, displayRange[1]));
    }
  else
    {
    double currentRange[2] = { 0.0 };
    d->DisplayedScalarRangeWidget->range(currentRange);
    d->DisplayedScalarRangeWidget->setRange(std::min(currentRange[0], displayRange[0]),
      std::max(currentRange[1], displayRange[1]));
    }
  d->DisplayedScalarRangeWidget->setValues(displayRange[0], displayRange[1]);
  d->DisplayedScalarRangeWidget->setDecimals(decimals);
  d->DisplayedScalarRangeWidget->setSingleStep(precision);
  d->DisplayedScalarRangeWidget->setEnabled(firstDisplayNode->GetScalarRangeFlag() == vtkDMMLDisplayNode::UseManualScalarRange);
  d->DisplayedScalarRangeWidget->blockSignals(wasBlocking);

  double thresholdRange[2] = { 0.0, 0.0 };
  if (firstModelDisplayNode)
    {
    firstModelDisplayNode->GetThresholdRange(thresholdRange);
    }

  wasBlocking = d->ThresholdRangeWidget->blockSignals(true);
  d->ThresholdRangeWidget->setEnabled(firstModelDisplayNode && firstModelDisplayNode->GetThresholdEnabled());
  d->ThresholdRangeWidget->setRange(dataRange[0] - precision, dataRange[1] + precision);
  if (thresholdRange[0] <= thresholdRange[1])
    {
    // there is a valid threshold range
    // If current threshold values do not fit in the current data range
    // then we move the slider handles to make them fit,
    // but values in the display node will not be changed until the user moves the handles.
    d->ThresholdRangeWidget->setValues(std::max(dataRange[0] - precision, thresholdRange[0]),
      std::min(dataRange[1] + precision, thresholdRange[1]));
    }
  else
    {
    // no valid threshold range is set move handles to the center
    d->ThresholdRangeWidget->setValues(dataRange[0], dataRange[1]);
    }
  d->ThresholdRangeWidget->setDecimals(decimals);
  d->ThresholdRangeWidget->setSingleStep(precision);
  d->ThresholdRangeWidget->blockSignals(wasBlocking);

  wasBlocking = d->ThresholdCheckBox->blockSignals(true);
  d->ThresholdCheckBox->setEnabled(firstModelDisplayNode);
  d->ThresholdCheckBox->setChecked(firstModelDisplayNode && firstModelDisplayNode->GetThresholdEnabled());
  d->ThresholdCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->DisplayedScalarRangeModeComboBox->blockSignals(true);
  d->DisplayedScalarRangeModeComboBox->setCurrentIndex(firstDisplayNode->GetScalarRangeFlag());
  d->DisplayedScalarRangeModeComboBox->blockSignals(wasBlocking);

  wasBlocking = d->ActiveScalarComboBox->blockSignals(true);
  d->ActiveScalarComboBox->setEnabled(firstDisplayNode);
  if (firstDisplayNode)
    {
    if (d->ActiveScalarComboBox->dataSet() != firstDisplayNode->GetScalarDataSet())
      {
      d->ActiveScalarComboBox->setDataSet(firstDisplayNode->GetScalarDataSet());
      }
    if (d->ActiveScalarComboBox->currentArrayName() != firstDisplayNode->GetActiveScalarName())
      {
      d->ActiveScalarComboBox->setCurrentArray(firstDisplayNode->GetActiveScalarName());
      // Array location would need to be set in d->ActiveScalarComboBox if
      // same scalar name is used in multiple locations.
      }
    }
  d->ActiveScalarComboBox->blockSignals(wasBlocking);

  wasBlocking = d->ScalarsColorNodeComboBox->blockSignals(true);
  if (d->ScalarsColorNodeComboBox->dmmlScene() != this->dmmlScene())
    {
    d->ScalarsColorNodeComboBox->setDMMLScene(this->dmmlScene());
    }
  if (d->ScalarsColorNodeComboBox->currentNodeID() != firstDisplayNode->GetColorNodeID() )
    {
    d->ScalarsColorNodeComboBox->setCurrentNodeID(firstDisplayNode->GetColorNodeID());
    }
  d->ScalarsColorNodeComboBox->setEnabled(firstDisplayNode->GetScalarRangeFlag() != vtkDMMLDisplayNode::UseDirectMapping );
  d->ScalarsColorNodeComboBox->blockSignals(wasBlocking);

  emit displayNodeChanged();
}
