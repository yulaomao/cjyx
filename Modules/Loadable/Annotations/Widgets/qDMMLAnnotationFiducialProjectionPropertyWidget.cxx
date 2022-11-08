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
#include "qDMMLAnnotationFiducialProjectionPropertyWidget.h"
#include "ui_qDMMLAnnotationFiducialProjectionPropertyWidget.h"

// DMML includes
#include <vtkDMMLAnnotationFiducialNode.h>
#include <vtkDMMLAnnotationPointDisplayNode.h>

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_Annotation
class qDMMLAnnotationFiducialProjectionPropertyWidgetPrivate
  : public Ui_qDMMLAnnotationFiducialProjectionPropertyWidget
{
  Q_DECLARE_PUBLIC(qDMMLAnnotationFiducialProjectionPropertyWidget);
protected:
  qDMMLAnnotationFiducialProjectionPropertyWidget* const q_ptr;
public:
  qDMMLAnnotationFiducialProjectionPropertyWidgetPrivate(qDMMLAnnotationFiducialProjectionPropertyWidget& object);
  void init();

  vtkDMMLAnnotationPointDisplayNode* FiducialDisplayNode;
};

//-----------------------------------------------------------------------------
// qDMMLAnnotationFiducialProjectionPropertyWidgetPrivate methods

//-----------------------------------------------------------------------------
qDMMLAnnotationFiducialProjectionPropertyWidgetPrivate
::qDMMLAnnotationFiducialProjectionPropertyWidgetPrivate(qDMMLAnnotationFiducialProjectionPropertyWidget& object)
  : q_ptr(&object)
{
  this->FiducialDisplayNode = nullptr;
}

//-----------------------------------------------------------------------------
void qDMMLAnnotationFiducialProjectionPropertyWidgetPrivate
::init()
{
  Q_Q(qDMMLAnnotationFiducialProjectionPropertyWidget);
  this->setupUi(q);
  QObject::connect(this->Point2DProjectionCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setProjectionVisibility(bool)));
  QObject::connect(this->PointProjectionColorPickerButton, SIGNAL(colorChanged(QColor)),
                   q, SLOT(setProjectionColor(QColor)));
  QObject::connect(this->PointUseFiducialColorCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setUseFiducialColor(bool)));
  QObject::connect(this->PointOutlinedBehindSlicePlaneCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setOutlinedBehindSlicePlane(bool)));
  q->updateWidgetFromDisplayNode();
}

//-----------------------------------------------------------------------------
// qDMMLAnnotationFiducialProjectionPropertyWidget methods

//-----------------------------------------------------------------------------
qDMMLAnnotationFiducialProjectionPropertyWidget
::qDMMLAnnotationFiducialProjectionPropertyWidget(QWidget *newParent) :
    Superclass(newParent)
  , d_ptr(new qDMMLAnnotationFiducialProjectionPropertyWidgetPrivate(*this))
{
  Q_D(qDMMLAnnotationFiducialProjectionPropertyWidget);
  d->init();
}

//-----------------------------------------------------------------------------
qDMMLAnnotationFiducialProjectionPropertyWidget
::~qDMMLAnnotationFiducialProjectionPropertyWidget() = default;

//-----------------------------------------------------------------------------
void qDMMLAnnotationFiducialProjectionPropertyWidget
::setDMMLFiducialNode(vtkDMMLAnnotationFiducialNode* fiducialNode)
{
  Q_D(qDMMLAnnotationFiducialProjectionPropertyWidget);
  vtkDMMLAnnotationPointDisplayNode* displayNode
    = fiducialNode->GetAnnotationPointDisplayNode();

  qvtkReconnect(d->FiducialDisplayNode, displayNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromDisplayNode()));

  d->FiducialDisplayNode = displayNode;
  this->updateWidgetFromDisplayNode();
}

//-----------------------------------------------------------------------------
void qDMMLAnnotationFiducialProjectionPropertyWidget
::setProjectionVisibility(bool showProjection)
{
  Q_D(qDMMLAnnotationFiducialProjectionPropertyWidget);
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
void qDMMLAnnotationFiducialProjectionPropertyWidget
::setProjectionColor(QColor newColor)
{
  Q_D(qDMMLAnnotationFiducialProjectionPropertyWidget);
  if (!d->FiducialDisplayNode)
    {
    return;
    }
  d->FiducialDisplayNode
    ->SetProjectedColor(newColor.redF(), newColor.greenF(), newColor.blueF());
}

//-----------------------------------------------------------------------------
void qDMMLAnnotationFiducialProjectionPropertyWidget
::setUseFiducialColor(bool useFiducialColor)
{
  Q_D(qDMMLAnnotationFiducialProjectionPropertyWidget);
  if (!d->FiducialDisplayNode)
    {
    return;
    }
  if (useFiducialColor)
    {
    d->FiducialDisplayNode->SliceProjectionUseFiducialColorOn();
    }
  else
    {
    d->FiducialDisplayNode->SliceProjectionUseFiducialColorOff();
    }
}

//-----------------------------------------------------------------------------
void qDMMLAnnotationFiducialProjectionPropertyWidget
::setOutlinedBehindSlicePlane(bool outlinedBehind)
{
  Q_D(qDMMLAnnotationFiducialProjectionPropertyWidget);
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
void qDMMLAnnotationFiducialProjectionPropertyWidget
::updateWidgetFromDisplayNode()
{
  Q_D(qDMMLAnnotationFiducialProjectionPropertyWidget);

  this->setEnabled(d->FiducialDisplayNode != nullptr);

  if (!d->FiducialDisplayNode)
    {
    return;
    }

  // Update widget if different from DMML node
  // -- 2D Projection Visibility
  d->Point2DProjectionCheckBox->setChecked(
    d->FiducialDisplayNode->GetSliceProjection() &
    vtkDMMLAnnotationDisplayNode::ProjectionOn);

  // -- Projection Color
  double pColor[3];
  d->FiducialDisplayNode->GetProjectedColor(pColor);
  QColor displayColor = QColor(pColor[0]*255, pColor[1]*255, pColor[2]*255);
  d->PointProjectionColorPickerButton->setColor(displayColor);

  // -- Use Fiducial Color
  d->PointUseFiducialColorCheckBox->setChecked(
    d->FiducialDisplayNode->GetSliceProjection() &
    vtkDMMLAnnotationPointDisplayNode::ProjectionUseFiducialColor);

  // -- Outlined Behind Slice Plane
  d->PointOutlinedBehindSlicePlaneCheckBox->setChecked(
    d->FiducialDisplayNode->GetSliceProjection() &
    vtkDMMLAnnotationPointDisplayNode::ProjectionOutlinedBehindSlicePlane);
}

