/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

==============================================================================*/


// qDMML includes
#include "qDMMLTransformInfoWidget.h"
#include "ui_qDMMLTransformInfoWidget.h"

// DMML includes
#include <vtkDMMLCrosshairNode.h>
#include <vtkDMMLTransformNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkMath.h>
#include <vtkTransform.h>
#include <vtkWeakPointer.h>

//------------------------------------------------------------------------------
class qDMMLTransformInfoWidgetPrivate: public Ui_qDMMLTransformInfoWidget
{
  Q_DECLARE_PUBLIC(qDMMLTransformInfoWidget);

protected:
  qDMMLTransformInfoWidget* const q_ptr;

public:
  qDMMLTransformInfoWidgetPrivate(qDMMLTransformInfoWidget& object);
  void init();

  void setAndObserveCrosshairNode();

  vtkWeakPointer<vtkDMMLScene> DMMLScene;
  vtkWeakPointer<vtkDMMLTransformNode> TransformNode;
  vtkWeakPointer<vtkDMMLCrosshairNode> CrosshairNode;
};

//------------------------------------------------------------------------------
qDMMLTransformInfoWidgetPrivate
::qDMMLTransformInfoWidgetPrivate(qDMMLTransformInfoWidget& object)
  : q_ptr(&object)
{
  this->TransformNode = nullptr;
  this->CrosshairNode = nullptr;
  this->DMMLScene = nullptr;
}

//------------------------------------------------------------------------------
void qDMMLTransformInfoWidgetPrivate::init()
{
  Q_Q(qDMMLTransformInfoWidget);
  this->setupUi(q);
  q->setEnabled(this->TransformNode.GetPointer() != nullptr);
  this->TransformToParentInfoTextBrowser->setTextInteractionFlags(Qt::TextSelectableByMouse);
  this->TransformFromParentInfoTextBrowser->setTextInteractionFlags(Qt::TextSelectableByMouse);
}

// --------------------------------------------------------------------------
void qDMMLTransformInfoWidgetPrivate::setAndObserveCrosshairNode()
{
  Q_Q(qDMMLTransformInfoWidget);

  vtkDMMLCrosshairNode* crosshairNode = nullptr;
  if (this->DMMLScene.GetPointer())
    {
    crosshairNode = vtkDMMLCrosshairNode::SafeDownCast(this->DMMLScene->GetFirstNodeByClass("vtkDMMLCrosshairNode"));
    }

  q->qvtkReconnect(this->CrosshairNode.GetPointer(), crosshairNode,
    vtkDMMLCrosshairNode::CursorPositionModifiedEvent,
    q, SLOT(updateTransformVectorDisplayFromDMML()));
  this->CrosshairNode = crosshairNode;
  q->updateTransformVectorDisplayFromDMML();
}

//------------------------------------------------------------------------------
qDMMLTransformInfoWidget::qDMMLTransformInfoWidget(QWidget *_parent)
  : Superclass(_parent)
  , d_ptr(new qDMMLTransformInfoWidgetPrivate(*this))
{
  Q_D(qDMMLTransformInfoWidget);
  d->init();
}

//------------------------------------------------------------------------------
qDMMLTransformInfoWidget::~qDMMLTransformInfoWidget() = default;

//------------------------------------------------------------------------------
void qDMMLTransformInfoWidget::processEvent(
  vtkObject* caller, void* callData, unsigned long eventId, void* clientData)
{
  Q_UNUSED(caller);
  Q_UNUSED(callData);
  Q_UNUSED(clientData);

  if (eventId == vtkDMMLCrosshairNode::CursorPositionModifiedEvent)
    {
    this->updateTransformVectorDisplayFromDMML();
    }
}

//------------------------------------------------------------------------------
vtkDMMLTransformNode* qDMMLTransformInfoWidget::dmmlTransformNode()const
{
  Q_D(const qDMMLTransformInfoWidget);
  return d->TransformNode.GetPointer();
}

//------------------------------------------------------------------------------
void qDMMLTransformInfoWidget::setDMMLTransformNode(vtkDMMLNode* node)
{
  this->setDMMLTransformNode(vtkDMMLTransformNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
vtkDMMLScene* qDMMLTransformInfoWidget::dmmlScene()const
{
  Q_D(const qDMMLTransformInfoWidget);
  return d->DMMLScene.GetPointer();
}

// --------------------------------------------------------------------------
void qDMMLTransformInfoWidget::setDMMLScene(vtkDMMLScene* scene)
{
  Q_D(qDMMLTransformInfoWidget);

  if (this->dmmlScene() == scene)
    {
    return;
    }

  d->DMMLScene = scene;
  d->setAndObserveCrosshairNode();
}

//------------------------------------------------------------------------------
void qDMMLTransformInfoWidget::setDMMLTransformNode(vtkDMMLTransformNode* transformNode)
{
  Q_D(qDMMLTransformInfoWidget);

  qvtkReconnect(d->TransformNode.GetPointer(), transformNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromDMML()));
  qvtkReconnect(d->TransformNode.GetPointer(), transformNode, vtkDMMLTransformableNode::TransformModifiedEvent,
                this, SLOT(updateWidgetFromDMML()));
  d->TransformNode = transformNode;

  this->updateWidgetFromDMML();
}

//------------------------------------------------------------------------------
void qDMMLTransformInfoWidget::showEvent(QShowEvent *)
{
  // Update the widget, now that it becomes becomes visible
  // (we might have missed some updates, because widget contents is not updated
  // if the widget is not visible).
  updateWidgetFromDMML();
}

//------------------------------------------------------------------------------
void qDMMLTransformInfoWidget::updateWidgetFromDMML()
{
  Q_D(qDMMLTransformInfoWidget);
  if (!this->isVisible())
    {
    // Getting the transform information is too expensive,
    // so if the widget is not visible then do not update
    return;
    }

  if (d->TransformNode.GetPointer())
    {
    d->TransformToParentInfoTextBrowser->setText(d->TransformNode->GetTransformToParentInfo());
    d->TransformFromParentInfoTextBrowser->setText(d->TransformNode->GetTransformFromParentInfo());
    }
  else
    {
    d->TransformToParentInfoTextBrowser->clear();
    d->TransformFromParentInfoTextBrowser->clear();
    }

  updateTransformVectorDisplayFromDMML();

  this->setEnabled(d->TransformNode.GetPointer() != nullptr);
}

//------------------------------------------------------------------------------
void qDMMLTransformInfoWidget::updateTransformVectorDisplayFromDMML()
{
  Q_D(qDMMLTransformInfoWidget);
  if (!this->isVisible())
    {
    // Getting the transform information is too expensive,
    // so if the widget is not visible then do not update
    return;
    }

  if (d->TransformNode.GetPointer() && d->CrosshairNode.GetPointer())
    {
    double ras[3]={0};
    bool validPosition = d->CrosshairNode->GetCursorPositionRAS(ras);
    if (validPosition)
      {
      // Get the displacement vector
      vtkAbstractTransform* transformToParent = d->TransformNode->GetTransformToParent();
      if (transformToParent)
        {
        double* rasDisplaced = transformToParent->TransformDoublePoint(ras[0], ras[1], ras[2]);

        // Verify if the transform is invertible at the current position
        vtkAbstractTransform* transformFromParent = d->TransformNode->GetTransformFromParent();
        if (transformFromParent)
          {
          double* rasDisplacedTransformedBack = transformFromParent->TransformDoublePoint(rasDisplaced[0], rasDisplaced[1], rasDisplaced[2]);
          static double INVERSE_COMPUTATION_ALLOWED_SQUARED_ERROR=0.1;
          bool inverseAccurate = vtkMath::Distance2BetweenPoints(ras,rasDisplacedTransformedBack)<INVERSE_COMPUTATION_ALLOWED_SQUARED_ERROR;

          d->ViewerDisplacementVectorRAS->setText(QString("Displacement vector  RAS: (%1, %2, %3)%4").
            arg(rasDisplaced[0] - ras[0], /* fieldWidth= */ 0, /* format = */ 'f', /* precision= */ 1).
            arg(rasDisplaced[1] - ras[1], /* fieldWidth= */ 0, /* format = */ 'f', /* precision= */ 1).
            arg(rasDisplaced[2] - ras[2], /* fieldWidth= */ 0, /* format = */ 'f', /* precision= */ 1).
            arg(inverseAccurate?"":"   Warning: inverse is inaccurate!") );
          return;
          }
        }
      }
    }
  // transform value is not available, so let's clear the display
  d->ViewerDisplacementVectorRAS->clear();
}
