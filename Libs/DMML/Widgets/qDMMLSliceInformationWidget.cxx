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
#include <QButtonGroup>
#include <QDebug>

// qDMML includes
#include "qDMMLSliceControllerWidget_p.h" // For updateSliceOrientationSelector
#include "qDMMLSliceInformationWidget_p.h"

// DMML includes
#include <vtkDMMLSliceNode.h>

//--------------------------------------------------------------------------
// qDMMLSliceViewPrivate methods

//---------------------------------------------------------------------------
qDMMLSliceInformationWidgetPrivate::qDMMLSliceInformationWidgetPrivate(qDMMLSliceInformationWidget& object)
  : q_ptr(&object)
  , SliceSpacingModeGroup(nullptr)
{
}

//---------------------------------------------------------------------------
qDMMLSliceInformationWidgetPrivate::~qDMMLSliceInformationWidgetPrivate() = default;

//---------------------------------------------------------------------------
void qDMMLSliceInformationWidgetPrivate::setupUi(qDMMLWidget* widget)
{
  Q_Q(qDMMLSliceInformationWidget);

  this->Ui_qDMMLSliceInformationWidget::setupUi(widget);

  // LayoutName is readonly

  // Connect Orientation selector
  this->connect(this->SliceOrientationSelector, SIGNAL(currentIndexChanged(QString)),
                q, SLOT(setSliceOrientation(QString)));

  // Connect Slice visibility toggle
  this->connect(this->SliceVisibilityToggle, SIGNAL(clicked(bool)),
                q, SLOT(setSliceVisible(bool)));

  // Connect Widget visibility toggle
  this->connect(this->WidgetVisibilityToggle, SIGNAL(clicked(bool)),
                q, SLOT(setWidgetVisible(bool)));

  // Dimension and Field of View are readonly

  this->connect(this->ViewGroupSpinBox, SIGNAL(valueChanged(int)),
    q, SLOT(setViewGroup(int)));

  // Connect LightBox layout
  this->connect(this->LightboxLayoutRowsSpinBox, SIGNAL(valueChanged(int)),
                q, SLOT(setLightboxLayoutRows(int)));
  this->connect(this->LightboxLayoutColumnsSpinBox, SIGNAL(valueChanged(int)),
                q, SLOT(setLightboxLayoutColumns(int)));

  // Connect SliceSpacingMode
  this->SliceSpacingModeGroup = new QButtonGroup(widget);
  this->SliceSpacingModeGroup->addButton(this->AutomaticSliceSpacingRadioButton,
                                         vtkDMMLSliceNode::AutomaticSliceSpacingMode);
  this->SliceSpacingModeGroup->addButton(this->PrescribedSliceSpacingRadioButton,
                                         vtkDMMLSliceNode::PrescribedSliceSpacingMode);
  this->connect(this->SliceSpacingModeGroup, SIGNAL(buttonReleased(int)),
                q, SLOT(setSliceSpacingMode(int)));

  // Connect Prescribed spacing
  this->connect(this->PrescribedSpacingSpinBox, SIGNAL(valueChanged(double)),
                q, SLOT(setPrescribedSliceSpacing(double)));
}

// --------------------------------------------------------------------------
void qDMMLSliceInformationWidgetPrivate::updateWidgetFromDMMLSliceNode()
{
  Q_Q(qDMMLSliceInformationWidget);

  q->setEnabled(this->DMMLSliceNode != nullptr);
  if (this->DMMLSliceNode == nullptr)
    {
    return;
    }

  //qDebug() << "qDMMLSliceInformationWidgetPrivate::updateWidgetFromDMMLSliceNode";

  // Update layout name
  this->LayoutNameLineEdit->setText(QString::fromUtf8(this->DMMLSliceNode->GetLayoutName()));

  qDMMLSliceControllerWidgetPrivate::updateSliceOrientationSelector(
        this->DMMLSliceNode, this->SliceOrientationSelector);

  // Update slice visibility toggle
  this->SliceVisibilityToggle->setChecked(this->DMMLSliceNode->GetSliceVisible());

  // Update widget visibility toggle
  this->WidgetVisibilityToggle->setChecked(this->DMMLSliceNode->GetWidgetVisible());

  // Update dimension
  int dimensions[3] = {0, 0, 0};
  this->DMMLSliceNode->GetDimensions(dimensions);
  double coordinatesInDouble[3];
  coordinatesInDouble[0] = dimensions[0];
  coordinatesInDouble[1] = dimensions[1];
  coordinatesInDouble[2] = dimensions[2];
  this->DimensionWidget->setCoordinates(coordinatesInDouble);

  // Update field of view
  double fieldOfView[3] = {0.0, 0.0, 0.0};
  this->DMMLSliceNode->GetFieldOfView(fieldOfView);
  coordinatesInDouble[0] = fieldOfView[0];
  coordinatesInDouble[1] = fieldOfView[1];
  coordinatesInDouble[2] = fieldOfView[2];
  this->FieldOfViewWidget->setCoordinates(coordinatesInDouble);

  this->ViewGroupSpinBox->setValue(this->DMMLSliceNode->GetViewGroup());

  // Update lightbox rows/columns entries
  this->LightboxLayoutRowsSpinBox->setValue(this->DMMLSliceNode->GetLayoutGridRows());
  this->LightboxLayoutColumnsSpinBox->setValue(this->DMMLSliceNode->GetLayoutGridColumns());

  // Update spacing mode
  if (this->DMMLSliceNode->GetSliceSpacingMode() == vtkDMMLSliceNode::AutomaticSliceSpacingMode)
    {
    this->AutomaticSliceSpacingRadioButton->setChecked(true);
    }
  else if (this->DMMLSliceNode->GetSliceSpacingMode() ==
           vtkDMMLSliceNode::PrescribedSliceSpacingMode)
    {
    this->PrescribedSliceSpacingRadioButton->setChecked(true);
    double prescribedSpacing[3] = {0.0, 0.0, 0.0};
    this->DMMLSliceNode->GetPrescribedSliceSpacing(prescribedSpacing);
    this->PrescribedSpacingSpinBox->setValue(prescribedSpacing[2]);
    }
}

// --------------------------------------------------------------------------
// qDMMLSliceView methods

// --------------------------------------------------------------------------
qDMMLSliceInformationWidget::qDMMLSliceInformationWidget(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qDMMLSliceInformationWidgetPrivate(*this))
{
  Q_D(qDMMLSliceInformationWidget);
  d->setupUi(this);
  this->setEnabled(false);
}

// --------------------------------------------------------------------------
qDMMLSliceInformationWidget::~qDMMLSliceInformationWidget() = default;

//---------------------------------------------------------------------------
vtkDMMLSliceNode* qDMMLSliceInformationWidget::dmmlSliceNode()const
{
  Q_D(const qDMMLSliceInformationWidget);
  return d->DMMLSliceNode;
}

//---------------------------------------------------------------------------
void qDMMLSliceInformationWidget::setDMMLSliceNode(vtkDMMLNode* newNode)
{
  vtkDMMLSliceNode * newSliceNode = vtkDMMLSliceNode::SafeDownCast(newNode);
  if (!newSliceNode)
    {
    return;
    }
  this->setDMMLSliceNode(newSliceNode);
}

//---------------------------------------------------------------------------
void qDMMLSliceInformationWidget::setDMMLSliceNode(vtkDMMLSliceNode* newSliceNode)
{
  Q_D(qDMMLSliceInformationWidget);

  if (newSliceNode == d->DMMLSliceNode)
    {
    return;
    }

  d->qvtkReconnect(d->DMMLSliceNode, newSliceNode, vtkCommand::ModifiedEvent,
                   d, SLOT(updateWidgetFromDMMLSliceNode()));

  d->DMMLSliceNode = newSliceNode;

  // Update widget state given the new node
  d->updateWidgetFromDMMLSliceNode();
}

//---------------------------------------------------------------------------
void qDMMLSliceInformationWidget::setSliceOrientation(const QString& orientation)
{
  Q_D(qDMMLSliceInformationWidget);

  if (!d->DMMLSliceNode)
    {
    return;
    }

  d->DMMLSliceNode->SetOrientation(orientation.toUtf8());
}

//---------------------------------------------------------------------------
void qDMMLSliceInformationWidget::setSliceVisible(bool visible)
{
  Q_D(qDMMLSliceInformationWidget);

  if (!d->DMMLSliceNode)
    {
    return;
    }

  d->DMMLSliceNode->SetSliceVisible(visible);
}

//---------------------------------------------------------------------------
void qDMMLSliceInformationWidget::setViewGroup(int viewGroup)
{
  Q_D(qDMMLSliceInformationWidget);

  if (!d->DMMLSliceNode)
    {
    return;
    }

  d->DMMLSliceNode->SetViewGroup(viewGroup);
}

//---------------------------------------------------------------------------
void qDMMLSliceInformationWidget::setWidgetVisible(bool visible)
{
  Q_D(qDMMLSliceInformationWidget);

  if (!d->DMMLSliceNode)
    {
    return;
    }

  d->DMMLSliceNode->SetWidgetVisible(visible);
}

//---------------------------------------------------------------------------
void qDMMLSliceInformationWidget::setLightboxLayoutRows(int rowCount)
{
  Q_D(qDMMLSliceInformationWidget);

  if (!d->DMMLSliceNode)
    {
    return;
    }

  d->DMMLSliceNode->SetLayoutGridRows(rowCount);
}

//---------------------------------------------------------------------------
void qDMMLSliceInformationWidget::setLightboxLayoutColumns(int columnCount)
{
  Q_D(qDMMLSliceInformationWidget);

  if (!d->DMMLSliceNode)
    {
    return;
    }

  d->DMMLSliceNode->SetLayoutGridColumns(columnCount);
}

//---------------------------------------------------------------------------
void qDMMLSliceInformationWidget::setSliceSpacingMode(int spacingMode)
{
  Q_D(qDMMLSliceInformationWidget);

  if (spacingMode != vtkDMMLSliceNode::AutomaticSliceSpacingMode &&
      spacingMode != vtkDMMLSliceNode::PrescribedSliceSpacingMode)
    {
    qWarning() << "setSliceSpacingMode - Invalid spacingMode:" << spacingMode;
    return;
    }

  if (!d->DMMLSliceNode)
    {
    return;
    }

  d->DMMLSliceNode->SetSliceSpacingMode(spacingMode);
}

//---------------------------------------------------------------------------
void qDMMLSliceInformationWidget::setPrescribedSliceSpacing(double spacing)
{
  Q_D(qDMMLSliceInformationWidget);

  if (!d->DMMLSliceNode)
    {
    return;
    }

  double spacingArray[3] = {0.0, 0.0, 0.0};
  d->DMMLSliceNode->GetPrescribedSliceSpacing(spacingArray);
  spacingArray[2] = spacing;
  d->DMMLSliceNode->SetPrescribedSliceSpacing(spacingArray);
}

