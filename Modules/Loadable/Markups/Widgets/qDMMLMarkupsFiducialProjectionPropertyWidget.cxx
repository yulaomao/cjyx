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
  It was then updated for the Markups module by Nicole Aucoin, BWH.

==============================================================================*/

// qDMML includes
#include "qDMMLMarkupsFiducialProjectionPropertyWidget.h"
#include "ui_qDMMLMarkupsFiducialProjectionPropertyWidget.h"

// DMML includes
#include <vtkDMMLMarkupsFiducialNode.h>
#include <vtkDMMLMarkupsDisplayNode.h>

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_Markups
class qDMMLMarkupsFiducialProjectionPropertyWidgetPrivate
  : public Ui_qDMMLMarkupsFiducialProjectionPropertyWidget
{
  Q_DECLARE_PUBLIC(qDMMLMarkupsFiducialProjectionPropertyWidget);
protected:
  qDMMLMarkupsFiducialProjectionPropertyWidget* const q_ptr;
public:
  qDMMLMarkupsFiducialProjectionPropertyWidgetPrivate(qDMMLMarkupsFiducialProjectionPropertyWidget& object);
  void init();

  vtkDMMLMarkupsDisplayNode* FiducialDisplayNode;
};

//-----------------------------------------------------------------------------
// qDMMLMarkupsFiducialProjectionPropertyWidgetPrivate methods

//-----------------------------------------------------------------------------
qDMMLMarkupsFiducialProjectionPropertyWidgetPrivate
::qDMMLMarkupsFiducialProjectionPropertyWidgetPrivate(qDMMLMarkupsFiducialProjectionPropertyWidget& object)
  : q_ptr(&object)
{
  this->FiducialDisplayNode = nullptr;
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsFiducialProjectionPropertyWidgetPrivate
::init()
{
  Q_Q(qDMMLMarkupsFiducialProjectionPropertyWidget);
  this->setupUi(q);
  QObject::connect(this->point2DProjectionCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setProjectionVisibility(bool)));
  QObject::connect(this->pointProjectionColorPickerButton, SIGNAL(colorChanged(QColor)),
                   q, SLOT(setProjectionColor(QColor)));
  QObject::connect(this->pointUseFiducialColorCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setUseFiducialColor(bool)));
  QObject::connect(this->pointOutlinedBehindSlicePlaneCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setOutlinedBehindSlicePlane(bool)));
  QObject::connect(this->projectionOpacitySliderWidget, SIGNAL(valueChanged(double)),
                   q, SLOT(setProjectionOpacity(double)));
  q->updateWidgetFromDisplayNode();
}

//-----------------------------------------------------------------------------
// qDMMLMarkupsFiducialProjectionPropertyWidget methods

//-----------------------------------------------------------------------------
qDMMLMarkupsFiducialProjectionPropertyWidget
::qDMMLMarkupsFiducialProjectionPropertyWidget(QWidget *newParent) :
    Superclass(newParent)
  , d_ptr(new qDMMLMarkupsFiducialProjectionPropertyWidgetPrivate(*this))
{
  Q_D(qDMMLMarkupsFiducialProjectionPropertyWidget);
  d->init();
}

//-----------------------------------------------------------------------------
qDMMLMarkupsFiducialProjectionPropertyWidget
::~qDMMLMarkupsFiducialProjectionPropertyWidget() = default;

//-----------------------------------------------------------------------------
void qDMMLMarkupsFiducialProjectionPropertyWidget
::setDMMLMarkupsNode(vtkDMMLMarkupsNode* markupsNode)
{
  Q_D(qDMMLMarkupsFiducialProjectionPropertyWidget);
  vtkDMMLMarkupsDisplayNode* displayNode = (markupsNode ? markupsNode->GetMarkupsDisplayNode() : nullptr);
  this->setDMMLMarkupsDisplayNode(displayNode);
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsFiducialProjectionPropertyWidget
::setDMMLMarkupsDisplayNode(vtkDMMLMarkupsDisplayNode* markupsDisplayNode)
{
  Q_D(qDMMLMarkupsFiducialProjectionPropertyWidget);
  if (markupsDisplayNode == d->FiducialDisplayNode)
    {
    // no change
    return;
    }

  qvtkReconnect(d->FiducialDisplayNode, markupsDisplayNode, vtkCommand::ModifiedEvent,
    this, SLOT(updateWidgetFromDisplayNode()));

  d->FiducialDisplayNode = markupsDisplayNode;
  this->updateWidgetFromDisplayNode();
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsFiducialProjectionPropertyWidget
::setProjectionVisibility(bool showProjection)
{
  Q_D(qDMMLMarkupsFiducialProjectionPropertyWidget);
  if (!d->FiducialDisplayNode)
    {
    return;
    }
  if (showProjection)
    {
    d->FiducialDisplayNode->SliceProjectionOn();
    }
  else
    {
    d->FiducialDisplayNode->SliceProjectionOff();
    }
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsFiducialProjectionPropertyWidget
::setProjectionColor(QColor newColor)
{
  Q_D(qDMMLMarkupsFiducialProjectionPropertyWidget);
  if (!d->FiducialDisplayNode)
    {
    return;
    }
  d->FiducialDisplayNode
    ->SetSliceProjectionColor(newColor.redF(), newColor.greenF(), newColor.blueF());
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsFiducialProjectionPropertyWidget
::setUseFiducialColor(bool useFiducialColor)
{
  Q_D(qDMMLMarkupsFiducialProjectionPropertyWidget);
  if (!d->FiducialDisplayNode)
    {
    return;
    }
  if (useFiducialColor)
    {
    d->FiducialDisplayNode->SliceProjectionUseFiducialColorOn();
    d->pointProjectionColorLabel->setEnabled(false);
    d->pointProjectionColorPickerButton->setEnabled(false);
    }
  else
    {
    d->FiducialDisplayNode->SliceProjectionUseFiducialColorOff();
    d->pointProjectionColorLabel->setEnabled(true);
    d->pointProjectionColorPickerButton->setEnabled(true);
    }
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsFiducialProjectionPropertyWidget
::setOutlinedBehindSlicePlane(bool outlinedBehind)
{
  Q_D(qDMMLMarkupsFiducialProjectionPropertyWidget);
  if (!d->FiducialDisplayNode)
    {
    return;
    }
  if (outlinedBehind)
    {
    d->FiducialDisplayNode->SliceProjectionOutlinedBehindSlicePlaneOn();
    }
  else
    {
    d->FiducialDisplayNode->SliceProjectionOutlinedBehindSlicePlaneOff();
    }
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsFiducialProjectionPropertyWidget
::setProjectionOpacity(double opacity)
{
  Q_D(qDMMLMarkupsFiducialProjectionPropertyWidget);
  if (!d->FiducialDisplayNode)
    {
    return;
    }
  d->FiducialDisplayNode->SetSliceProjectionOpacity(opacity);
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsFiducialProjectionPropertyWidget
::updateWidgetFromDisplayNode()
{
  Q_D(qDMMLMarkupsFiducialProjectionPropertyWidget);

  this->setEnabled(d->FiducialDisplayNode != nullptr);

  if (!d->FiducialDisplayNode)
    {
    return;
    }

  // Update widget if different from DMML node
  // -- 2D Projection Visibility
  d->point2DProjectionCheckBox->setChecked(
    d->FiducialDisplayNode->GetSliceProjection());

  // -- Projection Color
  double pColor[3];
  d->FiducialDisplayNode->GetSliceProjectionColor(pColor);
  QColor displayColor = QColor(pColor[0]*255, pColor[1]*255, pColor[2]*255);
  d->pointProjectionColorPickerButton->setColor(displayColor);

  // -- Use Fiducial Color
  bool useFiducialColor = d->FiducialDisplayNode->GetSliceProjectionUseFiducialColor();
  d->pointUseFiducialColorCheckBox->setChecked(useFiducialColor);
  d->pointProjectionColorLabel->setEnabled(!useFiducialColor);
  d->pointProjectionColorPickerButton->setEnabled(!useFiducialColor);

  // -- Outlined Behind Slice Plane
  d->pointOutlinedBehindSlicePlaneCheckBox->setChecked(
     d->FiducialDisplayNode->GetSliceProjectionOutlinedBehindSlicePlane());

  // -- Opacity
  d->projectionOpacitySliderWidget->setValue(
     d->FiducialDisplayNode->GetSliceProjectionOpacity());
}
