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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// qDMML includes
#include "qDMMLColors.h"
#include "qDMMLNavigationView.h"

// DMML includes
#include <vtkDMMLViewNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkWeakPointer.h>

//-----------------------------------------------------------------------------
class qDMMLNavigationViewPrivate
{
  Q_DECLARE_PUBLIC(qDMMLNavigationView);
protected:
  qDMMLNavigationView* const q_ptr;
public:
  qDMMLNavigationViewPrivate(qDMMLNavigationView& object);
  ~qDMMLNavigationViewPrivate();

  vtkDMMLScene*                      DMMLScene;
  vtkWeakPointer<vtkDMMLViewNode>    DMMLViewNode;
};

//--------------------------------------------------------------------------
// qDMMLNavigationViewPrivate methods

//---------------------------------------------------------------------------
qDMMLNavigationViewPrivate::qDMMLNavigationViewPrivate(qDMMLNavigationView& object)
  : q_ptr(&object)
{
  this->DMMLScene = nullptr;
}

//---------------------------------------------------------------------------
qDMMLNavigationViewPrivate::~qDMMLNavigationViewPrivate() = default;

// --------------------------------------------------------------------------
// qDMMLNavigationView methods

// --------------------------------------------------------------------------
qDMMLNavigationView::qDMMLNavigationView(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qDMMLNavigationViewPrivate(*this))
{
  // Set default background color
  this->setBackgroundColor(QColor::fromRgbF(
    vtkDMMLViewNode::defaultBackgroundColor()[0],
    vtkDMMLViewNode::defaultBackgroundColor()[1],
    vtkDMMLViewNode::defaultBackgroundColor()[2]));
}

// --------------------------------------------------------------------------
qDMMLNavigationView::~qDMMLNavigationView() = default;

//------------------------------------------------------------------------------
void qDMMLNavigationView::setDMMLScene(vtkDMMLScene* newScene)
{
  Q_D(qDMMLNavigationView);
  if (d->DMMLScene == newScene)
    {
    return;
    }
  this->qvtkReconnect(d->DMMLScene, newScene, vtkDMMLScene::NodeAddedEvent,
                      this, SLOT(updateFromDMMLScene()));
  this->qvtkReconnect(d->DMMLScene, newScene, vtkDMMLScene::NodeRemovedEvent,
                      this, SLOT(updateFromDMMLScene()));

  this->qvtkReconnect(d->DMMLScene, newScene, vtkDMMLScene::EndBatchProcessEvent,
                      this, SLOT(updateFromDMMLScene()));
  d->DMMLScene = newScene;
  if (!d->DMMLViewNode || newScene != d->DMMLViewNode->GetScene())
    {
    this->setDMMLViewNode(nullptr);
    }
  this->updateFromDMMLScene();
}

//---------------------------------------------------------------------------
void qDMMLNavigationView::setDMMLViewNode(vtkDMMLViewNode* newViewNode)
{
  Q_D(qDMMLNavigationView);
  if (d->DMMLViewNode == newViewNode)
    {
    return;
    }

  this->qvtkReconnect(d->DMMLViewNode, newViewNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateFromDMMLViewNode()));
  d->DMMLViewNode = newViewNode;
  this->updateFromDMMLViewNode();

  // Enable/disable widget
  this->setEnabled(d->DMMLViewNode != nullptr);
}

//---------------------------------------------------------------------------
vtkDMMLViewNode* qDMMLNavigationView::dmmlViewNode()const
{
  Q_D(const qDMMLNavigationView);
  return d->DMMLViewNode;
}

//---------------------------------------------------------------------------
void qDMMLNavigationView::updateFromDMMLScene()
{
  Q_D(qDMMLNavigationView);
  if (!d->DMMLScene || d->DMMLScene->IsBatchProcessing())
    {
    return;
    }
  this->updateBounds();
}

//---------------------------------------------------------------------------
void qDMMLNavigationView::updateFromDMMLViewNode()
{
  Q_D(qDMMLNavigationView);
  if (!d->DMMLViewNode)
    {
    return;
    }
  double backgroundColor[3];
  d->DMMLViewNode->GetBackgroundColor(backgroundColor);
  this->setBackgroundColor(
    QColor::fromRgbF(backgroundColor[0], backgroundColor[1], backgroundColor[2]));
}
