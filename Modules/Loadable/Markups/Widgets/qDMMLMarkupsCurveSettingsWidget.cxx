/*==============================================================================

  Copyright (c) The Intervention Centre
  Oslo University Hospital, Oslo, Norway. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Rafael Palomar (The Intervention Centre,
  Oslo University Hospital) and was supported by The Research Council of Norway
  through the ALive project (grant nr. 311393).

==============================================================================*/

#include "qDMMLMarkupsCurveSettingsWidget.h"
#include "ui_qDMMLMarkupsCurveSettingsWidget.h"

#include "vtkCjyxDijkstraGraphGeodesicPath.h"

// DMML Markups includes
#include <vtkDMMLMarkupsClosedCurveNode.h>
#include <vtkDMMLMarkupsCurveNode.h>
#include <vtkDMMLMarkupsNode.h>

// VTK includes
#include <vtkWeakPointer.h>
#include <vtkNew.h>

// Qt includes
#include <QTimer>

// --------------------------------------------------------------------------
class qDMMLMarkupsCurveSettingsWidget;

// --------------------------------------------------------------------------
class qDMMLMarkupsCurveSettingsWidgetPrivate
  : public Ui_qDMMLMarkupsCurveSettingsWidget
{
public:
  qDMMLMarkupsCurveSettingsWidgetPrivate(qDMMLMarkupsCurveSettingsWidget &widget);

  static const char* getCurveTypeAsHumanReadableString(int curveType);
  static const char* getCostFunctionAsHumanReadableString(int costFunction);

  void setupUi(QWidget* widget);

  virtual void setupUi(qDMMLMarkupsCurveSettingsWidget*);

  QTimer* editScalarFunctionDelay;

protected:
  qDMMLMarkupsCurveSettingsWidget* const q_ptr;

private:
  Q_DECLARE_PUBLIC(qDMMLMarkupsCurveSettingsWidget);
};

// --------------------------------------------------------------------------
qDMMLMarkupsCurveSettingsWidgetPrivate::qDMMLMarkupsCurveSettingsWidgetPrivate(qDMMLMarkupsCurveSettingsWidget& widget)
  : q_ptr(&widget)
{
  this->editScalarFunctionDelay = nullptr;
}

// --------------------------------------------------------------------------
void qDMMLMarkupsCurveSettingsWidgetPrivate::setupUi(qDMMLMarkupsCurveSettingsWidget* widget)
{
  Q_Q(qDMMLMarkupsCurveSettingsWidget);

  this->Ui_qDMMLMarkupsCurveSettingsWidget::setupUi(widget);

  this->curveTypeComboBox->clear();
  for (int curveType = 0; curveType < vtkCurveGenerator::CURVE_TYPE_LAST; ++curveType)
    {
    this->curveTypeComboBox->addItem(qDMMLMarkupsCurveSettingsWidgetPrivate::getCurveTypeAsHumanReadableString(curveType), curveType);
    }

  this->costFunctionComboBox->clear();
  for (int costFunction = 0; costFunction < vtkCjyxDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_LAST; ++costFunction)
    {
    this->costFunctionComboBox->addItem(qDMMLMarkupsCurveSettingsWidgetPrivate::getCostFunctionAsHumanReadableString(costFunction), costFunction);
    }

  this->editScalarFunctionDelay = new QTimer(q);
  this->editScalarFunctionDelay->setInterval(500);
  this->editScalarFunctionDelay->setSingleShot(true);



  QObject::connect(this->editScalarFunctionDelay, SIGNAL(timeout()),
                   q, SLOT(onCurveTypeParameterChanged()));
  QObject::connect(this->curveTypeComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onCurveTypeParameterChanged()));
  QObject::connect(this->modelNodeSelector, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
                   q, SLOT(onCurveTypeParameterChanged()));
  QObject::connect(this->costFunctionComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onCurveTypeParameterChanged()));
  QObject::connect(this->scalarFunctionLineEdit, SIGNAL(textChanged(QString)),
                   this->editScalarFunctionDelay, SLOT(start()));
  QObject::connect(this->projectCurveMaxSearchRadiusSliderWidget, SIGNAL(valueChanged(double)),
                   q, SLOT(onProjectCurveMaximumSearchRadiusChanged()));
  QObject::connect(this->resampleCurveButton, SIGNAL(clicked()),
                   q, SLOT(onApplyCurveResamplingPushButtonClicked()));

  q_ptr->setEnabled(q_ptr->MarkupsNode != nullptr);
}

//------------------------------------------------------------------------------
const char* qDMMLMarkupsCurveSettingsWidgetPrivate::getCurveTypeAsHumanReadableString(int curveType)
{
  switch (curveType)
    {
    case vtkCurveGenerator::CURVE_TYPE_LINEAR_SPLINE:
      {
      return "Linear";
      }
    case vtkCurveGenerator::CURVE_TYPE_CARDINAL_SPLINE:
      {
      return "Spline";
      }
    case vtkCurveGenerator::CURVE_TYPE_KOCHANEK_SPLINE:
      {
      return "Kochanek spline";
      }
    case vtkCurveGenerator::CURVE_TYPE_POLYNOMIAL:
      {
      return "Polynomial";
      }
    case vtkCurveGenerator::CURVE_TYPE_SHORTEST_DISTANCE_ON_SURFACE:
      {
      return "Shortest distance on surface";
      }
    default:
      {
      vtkGenericWarningMacro("Unknown curve type: " << curveType);
      return "Unknown";
      }
    }
}

//------------------------------------------------------------------------------
const char* qDMMLMarkupsCurveSettingsWidgetPrivate::getCostFunctionAsHumanReadableString(int costFunction)
{
  switch (costFunction)
    {
    case vtkCjyxDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_DISTANCE:
      {
      return "Distance";
      }
    case vtkCjyxDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_ADDITIVE:
      {
      return "Additive";
      }
    case vtkCjyxDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_MULTIPLICATIVE:
      {
      return "Multiplicative";
      }
    case vtkCjyxDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_INVERSE_SQUARED:
      {
      return "Inverse squared";
      }
    default:
      {
      return "";
      }
    }
}

// --------------------------------------------------------------------------
// qDMMLMarkupsCurveSettingsWidget methods

// --------------------------------------------------------------------------
qDMMLMarkupsCurveSettingsWidget::
qDMMLMarkupsCurveSettingsWidget(QWidget *parent)
  : Superclass(parent), d_ptr(new qDMMLMarkupsCurveSettingsWidgetPrivate(*this))
{
  this->setup();
}

// --------------------------------------------------------------------------
qDMMLMarkupsCurveSettingsWidget::~qDMMLMarkupsCurveSettingsWidget() = default;

// --------------------------------------------------------------------------
void qDMMLMarkupsCurveSettingsWidget::setup()
{
  Q_D(qDMMLMarkupsCurveSettingsWidget);
  d->setupUi(this);
}

// --------------------------------------------------------------------------
void qDMMLMarkupsCurveSettingsWidget::updateWidgetFromDMML()
{
  Q_D(qDMMLMarkupsCurveSettingsWidget);

  vtkDMMLMarkupsCurveNode *curveNode = vtkDMMLMarkupsCurveNode::SafeDownCast(this->MarkupsNode);
  if (!curveNode)
    {
    return;
    }

  // Update displayed node types.
  // Since updating this list resets the previous node selection,
  // we save and restore previous selection.
  vtkDMMLNode* previousOutputNode = d->resampleCurveOutputNodeSelector->currentNode();
  d->resampleCurveOutputNodeSelector->setNodeTypes(QStringList(QString(curveNode->GetClassName())));
  if (previousOutputNode && previousOutputNode->IsA(curveNode->GetClassName()))
    {
    d->resampleCurveOutputNodeSelector->setCurrentNode(previousOutputNode);
    }
  else
    {
    d->resampleCurveOutputNodeSelector->setCurrentNode(nullptr);
    }

  bool wasBlocked = d->curveTypeComboBox->blockSignals(true);
  d->curveTypeComboBox->setCurrentIndex(d->curveTypeComboBox->findData(curveNode->GetCurveType()));
  d->curveTypeComboBox->blockSignals(wasBlocked);

  vtkDMMLModelNode* modelNode = curveNode->GetSurfaceConstraintNode();
  wasBlocked = d->modelNodeSelector->blockSignals(true);
  d->modelNodeSelector->setCurrentNode(modelNode);
  d->modelNodeSelector->blockSignals(wasBlocked);

  wasBlocked = d->costFunctionComboBox->blockSignals(true);
  int costFunction = curveNode->GetSurfaceCostFunctionType();
  d->costFunctionComboBox->setCurrentIndex(d->costFunctionComboBox->findData(costFunction));
  d->costFunctionComboBox->blockSignals(wasBlocked);

  wasBlocked = d->scalarFunctionLineEdit->blockSignals(true);
  int currentCursorPosition = d->scalarFunctionLineEdit->cursorPosition();
  d->scalarFunctionLineEdit->setText(curveNode->GetSurfaceDistanceWeightingFunction());
  d->scalarFunctionLineEdit->setCursorPosition(currentCursorPosition);
  d->scalarFunctionLineEdit->blockSignals(wasBlocked);

  wasBlocked = d->projectCurveMaxSearchRadiusSliderWidget->blockSignals(true);
  d->projectCurveMaxSearchRadiusSliderWidget->setValue(curveNode->GetSurfaceConstraintMaximumSearchRadiusTolerance() * 100.);
  d->projectCurveMaxSearchRadiusSliderWidget->blockSignals(wasBlocked);

  if (costFunction == vtkCjyxDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_DISTANCE)
    {
    d->scalarFunctionLineEdit->setVisible(false);
    }
  else
    {
    d->scalarFunctionLineEdit->setVisible(true);
    }

  QString prefixString;
  QString suffixString;
  switch (costFunction)
    {
    case vtkCjyxDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_ADDITIVE:
      prefixString = "distance + ";
      break;
    case vtkCjyxDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_MULTIPLICATIVE:
      prefixString = "distance * ";
      break;
    case vtkCjyxDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_INVERSE_SQUARED:
      prefixString = "distance / (";
      suffixString = " ^ 2";
      break;
    default:
    case vtkCjyxDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_DISTANCE:
      prefixString = "distance";
      break;
    }
  d->scalarFunctionPrefixLabel->setText(prefixString);
  d->scalarFunctionSuffixLabel->setText(suffixString);

  if (curveNode->GetCurveType() == vtkCurveGenerator::CURVE_TYPE_SHORTEST_DISTANCE_ON_SURFACE)
    {
    d->surfaceCurveCollapsibleButton->setEnabled(true);
    }
  else
    {
    d->surfaceCurveCollapsibleButton->setEnabled(false);
    }
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsCurveSettingsWidget::onCurveTypeParameterChanged()
{
  Q_D(qDMMLMarkupsCurveSettingsWidget);

  vtkDMMLMarkupsCurveNode *curveNode = vtkDMMLMarkupsCurveNode::SafeDownCast(this->MarkupsNode);
  if (!curveNode)
    {
    return;
    }

  DMMLNodeModifyBlocker blocker(curveNode);
  curveNode->SetCurveType(d->curveTypeComboBox->currentData().toInt());
  curveNode->SetAndObserveSurfaceConstraintNode(vtkDMMLModelNode::SafeDownCast(d->modelNodeSelector->currentNode()));
  std::string functionString = d->scalarFunctionLineEdit->text().toStdString();
  curveNode->SetSurfaceCostFunctionType(d->costFunctionComboBox->currentData().toInt());
  curveNode->SetSurfaceDistanceWeightingFunction(functionString.c_str());
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsCurveSettingsWidget::onApplyCurveResamplingPushButtonClicked()
{
  Q_D(qDMMLMarkupsCurveSettingsWidget);

  double resampleNumberOfPoints = d->resampleCurveNumerOfOutputPointsSpinBox->value();
  if (resampleNumberOfPoints <= 1)
    {
    return;
    }

  vtkDMMLMarkupsCurveNode* inputNode = vtkDMMLMarkupsCurveNode::SafeDownCast(this->MarkupsNode);
  if (!inputNode)
    {
    return;
    }
  vtkDMMLMarkupsCurveNode* outputNode = vtkDMMLMarkupsCurveNode::SafeDownCast(d->resampleCurveOutputNodeSelector->currentNode());
  if (!outputNode)
    {
    outputNode = inputNode;
    }
  if(outputNode != inputNode)
    {
    DMMLNodeModifyBlocker blocker(outputNode);
    vtkNew<vtkPoints> originalControlPoints;
    inputNode->GetControlPointPositionsWorld(originalControlPoints);
    outputNode->SetControlPointPositionsWorld(originalControlPoints);
    vtkNew<vtkStringArray> originalLabels;
    inputNode->GetControlPointLabels(originalLabels);
    outputNode->SetControlPointLabels(originalLabels, originalControlPoints);
    outputNode->SetCurveType(inputNode->GetCurveType());
    outputNode->SetNumberOfPointsPerInterpolatingSegment(inputNode->GetNumberOfPointsPerInterpolatingSegment());
    outputNode->SetAndObserveSurfaceConstraintNode(inputNode->GetSurfaceConstraintNode());
    outputNode->SetSurfaceCostFunctionType(inputNode->GetSurfaceCostFunctionType());
    outputNode->SetSurfaceDistanceWeightingFunction(inputNode->GetSurfaceDistanceWeightingFunction());
    }
  double sampleDist = outputNode->GetCurveLengthWorld() / (resampleNumberOfPoints - 1);
  outputNode->ResampleCurveWorld(sampleDist);
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsCurveSettingsWidget::onProjectCurveMaximumSearchRadiusChanged()
{
  Q_D(qDMMLMarkupsCurveSettingsWidget);
  vtkDMMLMarkupsCurveNode* curveNode = vtkDMMLMarkupsCurveNode::SafeDownCast(this->MarkupsNode);
  if (!curveNode)
  {
    return;
  }
  const double maximumSearchRadius = 0.01 * d->projectCurveMaxSearchRadiusSliderWidget->value();
  curveNode->SetSurfaceConstraintMaximumSearchRadiusTolerance(maximumSearchRadius);
}

//-----------------------------------------------------------------------------
bool qDMMLMarkupsCurveSettingsWidget::canManageDMMLMarkupsNode(vtkDMMLMarkupsNode *markupsNode) const
{
  Q_D(const qDMMLMarkupsCurveSettingsWidget);

  vtkDMMLMarkupsCurveNode* curveNode = vtkDMMLMarkupsCurveNode::SafeDownCast(markupsNode);
  vtkDMMLMarkupsClosedCurveNode* closedCurveNode = vtkDMMLMarkupsClosedCurveNode::SafeDownCast(markupsNode);
  if (!curveNode && !closedCurveNode)
    {
    return false;
    }

  return true;
}

// --------------------------------------------------------------------------
void qDMMLMarkupsCurveSettingsWidget::setDMMLMarkupsNode(vtkDMMLMarkupsNode* markupsNode)
{
  Q_D(qDMMLMarkupsCurveSettingsWidget);

  this->MarkupsNode = vtkDMMLMarkupsCurveNode::SafeDownCast(markupsNode);
  this->setEnabled(this->MarkupsNode!= nullptr);
}

// --------------------------------------------------------------------------
void qDMMLMarkupsCurveSettingsWidget::setDMMLScene(vtkDMMLScene *dmmlScene)
{
  Q_D(qDMMLMarkupsCurveSettingsWidget);

  Superclass::setDMMLScene(dmmlScene);
  d->modelNodeSelector->setDMMLScene(dmmlScene);
  d->resampleCurveOutputNodeSelector->setDMMLScene(dmmlScene);
}
