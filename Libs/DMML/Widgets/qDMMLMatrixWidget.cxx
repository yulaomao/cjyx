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
#include <QDebug>

// qDMML includes
#include "qDMMLUtils.h"
#include "qDMMLMatrixWidget.h"

// DMML includes
#include <vtkDMMLTransformNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkWeakPointer.h>

//-----------------------------------------------------------------------------
class qDMMLMatrixWidgetPrivate
{
public:
  qDMMLMatrixWidgetPrivate()
    {
    this->CoordinateReference = qDMMLMatrixWidget::GLOBAL;
    this->DMMLTransformNode = nullptr;
    this->UserUpdates = true;
    }

  qDMMLMatrixWidget::CoordinateReferenceType   CoordinateReference;
  vtkWeakPointer<vtkDMMLTransformNode>         DMMLTransformNode;
  // Warning, this is not the real "transform, the real can be retrieved
  // by qVTKAbstractMatrixWidget->transform();
  vtkSmartPointer<vtkTransform>                Transform;
  // Indicates whether the changes come from the user or are programmatic
  bool                                         UserUpdates;
};

// --------------------------------------------------------------------------
qDMMLMatrixWidget::qDMMLMatrixWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qDMMLMatrixWidgetPrivate)
{
  connect(this, SIGNAL(matrixChanged()),
          this, SLOT(updateTransformNode()));
}

// --------------------------------------------------------------------------
qDMMLMatrixWidget::~qDMMLMatrixWidget() = default;

// --------------------------------------------------------------------------
void qDMMLMatrixWidget::setCoordinateReference(CoordinateReferenceType _coordinateReference)
{
  Q_D(qDMMLMatrixWidget);
  if (d->CoordinateReference == _coordinateReference)
    {
    return;
    }

  d->CoordinateReference = _coordinateReference;

  this->updateMatrix();
}

// --------------------------------------------------------------------------
qDMMLMatrixWidget::CoordinateReferenceType qDMMLMatrixWidget::coordinateReference() const
{
  Q_D(const qDMMLMatrixWidget);
  return d->CoordinateReference;
}

// --------------------------------------------------------------------------
void qDMMLMatrixWidget::setDMMLTransformNode(vtkDMMLNode* node)
{
  this->setDMMLTransformNode(vtkDMMLTransformNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qDMMLMatrixWidget::setDMMLTransformNode(vtkDMMLTransformNode* transformNode)
{
  Q_D(qDMMLMatrixWidget);

  if (d->DMMLTransformNode == transformNode)
    {
    return;
    }

  this->qvtkReconnect(d->DMMLTransformNode, transformNode,
                      vtkDMMLTransformableNode::TransformModifiedEvent,
                      this, SLOT(updateMatrix()));

  d->DMMLTransformNode = transformNode;

  this->setEnabled(transformNode ? transformNode->IsLinear() : false);

  this->updateMatrix();
}

// --------------------------------------------------------------------------
vtkDMMLTransformNode* qDMMLMatrixWidget::dmmlTransformNode()const
{
  Q_D(const qDMMLMatrixWidget);
  return d->DMMLTransformNode;
}

// --------------------------------------------------------------------------
void qDMMLMatrixWidget::updateMatrix()
{
  Q_D(qDMMLMatrixWidget);

  if (d->DMMLTransformNode == nullptr)
    {
    this->setMatrixInternal(nullptr);
    d->Transform = nullptr;
    return;
    }

  bool isLinear = d->DMMLTransformNode->IsLinear();
  this->setEnabled(isLinear);
  if (!isLinear)
    {
    return;
    }

  vtkNew<vtkTransform> transform;
  qDMMLUtils::getTransformInCoordinateSystem(
    d->DMMLTransformNode,
    d->CoordinateReference == qDMMLMatrixWidget::GLOBAL,
    transform.GetPointer());
  int oldUserUpdates = d->UserUpdates;
  d->UserUpdates = false;

  // update the matrix with the new values.
  this->setMatrixInternal( transform->GetMatrix() );
  d->UserUpdates = oldUserUpdates;
  // keep a ref on the transform otherwise, the matrix will be reset when transform
  // goes out of scope (because ctkVTKAbstractMatrixWidget has a weak ref on the matrix).
  d->Transform = transform.GetPointer();
}

// --------------------------------------------------------------------------
void qDMMLMatrixWidget::updateTransformNode()
{
  Q_D(qDMMLMatrixWidget);
  if (d->DMMLTransformNode == nullptr ||
      !d->UserUpdates)
    {
    return;
    }
  d->DMMLTransformNode->SetMatrixTransformToParent(this->matrix());
}
