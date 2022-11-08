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

#include "qDMMLMarkupsAngleMeasurementsWidget.h"
#include "ui_qDMMLMarkupsAngleMeasurementsWidget.h"

// DMML Markups includes
#include <vtkDMMLMarkupsNode.h>
#include <vtkDMMLMarkupsAngleNode.h>

// VTK includes
#include <vtkWeakPointer.h>

// --------------------------------------------------------------------------
class qDMMLMarkupsAngleMeasurementsWidget;

// --------------------------------------------------------------------------
class qDMMLMarkupsAngleMeasurementsWidgetPrivate:
  public Ui_qDMMLMarkupsAngleMeasurementsWidget
{

public:
  qDMMLMarkupsAngleMeasurementsWidgetPrivate(qDMMLMarkupsAngleMeasurementsWidget &widget);
  void setupUi(qDMMLMarkupsAngleMeasurementsWidget* widget);

protected:
  qDMMLMarkupsAngleMeasurementsWidget* const q_ptr;

private:
  Q_DECLARE_PUBLIC(qDMMLMarkupsAngleMeasurementsWidget);

};

// --------------------------------------------------------------------------
qDMMLMarkupsAngleMeasurementsWidgetPrivate::qDMMLMarkupsAngleMeasurementsWidgetPrivate(qDMMLMarkupsAngleMeasurementsWidget& widget)
  : q_ptr(&widget)
{
}

// --------------------------------------------------------------------------
void qDMMLMarkupsAngleMeasurementsWidgetPrivate::setupUi(qDMMLMarkupsAngleMeasurementsWidget* widget)
{
  Q_Q(qDMMLMarkupsAngleMeasurementsWidget);

  this->Ui_qDMMLMarkupsAngleMeasurementsWidget::setupUi(widget);

  QObject::connect(this->angleMeasurementModeComboBox, SIGNAL(currentIndexChanged(int)),
                   q_ptr, SLOT(onAngleMeasurementModeChanged()));
  QObject::connect(this->rotationAxisCoordinatesWidget, SIGNAL(coordinatesChanged(double*)),
                   q_ptr, SLOT(onRotationAxisChanged()));

  q_ptr->setEnabled(q_ptr->MarkupsNode != nullptr);
}

// --------------------------------------------------------------------------
// qDMMLMarkupsAngleMeasurementsWidget methods

// --------------------------------------------------------------------------
qDMMLMarkupsAngleMeasurementsWidget::
qDMMLMarkupsAngleMeasurementsWidget(QWidget *parent)
  : Superclass(parent), d_ptr(new qDMMLMarkupsAngleMeasurementsWidgetPrivate(*this))
{
  this->setup();
}

// --------------------------------------------------------------------------
qDMMLMarkupsAngleMeasurementsWidget::~qDMMLMarkupsAngleMeasurementsWidget()
{
}

// --------------------------------------------------------------------------
void qDMMLMarkupsAngleMeasurementsWidget::setup()
{
  d_ptr->setupUi(this);
}

// --------------------------------------------------------------------------
void qDMMLMarkupsAngleMeasurementsWidget::updateWidgetFromDMML()
{
  vtkDMMLMarkupsAngleNode* angleNode = vtkDMMLMarkupsAngleNode::SafeDownCast(this->MarkupsNode);
  if (!angleNode)
    {
    return;
    }

    double axisVector[3] = {0.0, 0.0, 0.0};
    angleNode->GetOrientationRotationAxis(axisVector);
    bool wasBlocked = d_ptr->rotationAxisCoordinatesWidget->blockSignals(true);
    d_ptr->rotationAxisCoordinatesWidget->setCoordinates(axisVector);
    d_ptr->rotationAxisCoordinatesWidget->setEnabled(angleNode->GetAngleMeasurementMode() != vtkDMMLMarkupsAngleNode::Minimal);
    d_ptr->rotationAxisCoordinatesWidget->blockSignals(wasBlocked);
    d_ptr->angleMeasurementModeComboBox->setCurrentIndex(angleNode->GetAngleMeasurementMode());
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsAngleMeasurementsWidget::onAngleMeasurementModeChanged()
{
  Q_D(qDMMLMarkupsAngleMeasurementsWidget);
  vtkDMMLMarkupsAngleNode* markupsAngleNode = vtkDMMLMarkupsAngleNode::SafeDownCast(this->MarkupsNode);
  if (!markupsAngleNode)
    {
    return;
    }

  markupsAngleNode->SetAngleMeasurementMode(d_ptr->angleMeasurementModeComboBox->currentIndex());
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsAngleMeasurementsWidget::onRotationAxisChanged()
{
  vtkDMMLMarkupsAngleNode* markupsAngleNode = vtkDMMLMarkupsAngleNode::SafeDownCast(this->MarkupsNode);
  if (!markupsAngleNode)
    {
    return;
    }
  markupsAngleNode->SetOrientationRotationAxis(const_cast<double*>(d_ptr->rotationAxisCoordinatesWidget->coordinates()));
}

//-----------------------------------------------------------------------------
bool qDMMLMarkupsAngleMeasurementsWidget::canManageDMMLMarkupsNode(vtkDMMLMarkupsNode *markupsNode) const
{
  vtkDMMLMarkupsAngleNode* angleNode = vtkDMMLMarkupsAngleNode::SafeDownCast(markupsNode);
  if (!angleNode)
    {
    return false;
    }

  return true;
}

// --------------------------------------------------------------------------
void qDMMLMarkupsAngleMeasurementsWidget::setDMMLMarkupsNode(vtkDMMLMarkupsNode* markupsNode)
{
  this->MarkupsNode = vtkDMMLMarkupsAngleNode::SafeDownCast(markupsNode);

  if(!this->MarkupsNode)
    {
    return;
    }

  this->qvtkReconnect(this->MarkupsNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateWidgetFromDMML()));

  this->updateWidgetFromDMML();
  this->setEnabled(markupsNode != nullptr);
}
