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

// Cjyx includes
#include "qCjyxCamerasModuleWidget.h"
#include "ui_qCjyxCamerasModuleWidget.h"
#include "vtkCjyxCamerasModuleLogic.h"

// DMML includes
#include "vtkDMMLViewNode.h"
#include "vtkDMMLCameraNode.h"
#include "vtkDMMLScene.h"

// STD includes

//-----------------------------------------------------------------------------
class qCjyxCamerasModuleWidgetPrivate: public Ui_qCjyxCamerasModuleWidget
{
public:
};

//-----------------------------------------------------------------------------
qCjyxCamerasModuleWidget::qCjyxCamerasModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qCjyxCamerasModuleWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qCjyxCamerasModuleWidget::~qCjyxCamerasModuleWidget() = default;

//-----------------------------------------------------------------------------
void qCjyxCamerasModuleWidget::setup()
{
  Q_D(qCjyxCamerasModuleWidget);
  d->setupUi(this);

  connect(d->ViewNodeSelector, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
          this, SLOT(onCurrentViewNodeChanged(vtkDMMLNode*)));
  connect(d->CameraNodeSelector, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
          this, SLOT(setCameraToCurrentView(vtkDMMLNode*)));
  connect(d->CameraNodeSelector, SIGNAL(nodeAdded(vtkDMMLNode*)),
          this, SLOT(onCameraNodeAdded(vtkDMMLNode*)));
  connect(d->CameraNodeSelector, SIGNAL(nodeAboutToBeRemoved(vtkDMMLNode*)),
          this, SLOT(onCameraNodeRemoved(vtkDMMLNode*)));
}

//-----------------------------------------------------------------------------
void qCjyxCamerasModuleWidget::onCurrentViewNodeChanged(vtkDMMLNode* dmmlNode)
{
  vtkDMMLViewNode* currentViewNode = vtkDMMLViewNode::SafeDownCast(dmmlNode);
  this->synchronizeCameraWithView(currentViewNode);
}

//-----------------------------------------------------------------------------
void qCjyxCamerasModuleWidget::synchronizeCameraWithView()
{
  Q_D(qCjyxCamerasModuleWidget);
  vtkDMMLViewNode* currentViewNode = vtkDMMLViewNode::SafeDownCast(
    d->ViewNodeSelector->currentNode());
  this->synchronizeCameraWithView(currentViewNode);
}

//-----------------------------------------------------------------------------
void qCjyxCamerasModuleWidget::synchronizeCameraWithView(vtkDMMLViewNode* currentViewNode)
{
  Q_D(qCjyxCamerasModuleWidget);
  if (!currentViewNode)
    {
    return;
    }
  vtkCjyxCamerasModuleLogic* camerasLogic =
    vtkCjyxCamerasModuleLogic::SafeDownCast(this->logic());
  vtkDMMLCameraNode *found_camera_node =
    camerasLogic->GetViewActiveCameraNode(currentViewNode);
  d->CameraNodeSelector->setCurrentNode(found_camera_node);
}


//-----------------------------------------------------------------------------
void qCjyxCamerasModuleWidget::setCameraToCurrentView(vtkDMMLNode* dmmlNode)
{
  Q_D(qCjyxCamerasModuleWidget);
  vtkDMMLCameraNode *currentCameraNode =
        vtkDMMLCameraNode::SafeDownCast(dmmlNode);
  if (!currentCameraNode)
    {// if the camera list is empty, there is no current camera
    return;
    }
  vtkDMMLViewNode *currentViewNode = vtkDMMLViewNode::SafeDownCast(
    d->ViewNodeSelector->currentNode());
  if (currentViewNode == nullptr)
    {
    return;
    }
  currentCameraNode->SetLayoutName(currentViewNode->GetLayoutName());
}

//-----------------------------------------------------------------------------
void qCjyxCamerasModuleWidget::onCameraNodeAdded(vtkDMMLNode* dmmlNode)
{
  vtkDMMLCameraNode *cameraNode = vtkDMMLCameraNode::SafeDownCast(dmmlNode);
  if (!cameraNode)
    {
    //Q_ASSERT(cameraNode);
    return;
    }
  this->qvtkConnect(cameraNode, vtkDMMLCameraNode::LayoutNameModifiedEvent,
                    this, SLOT(synchronizeCameraWithView()));
}

//-----------------------------------------------------------------------------
void qCjyxCamerasModuleWidget::onCameraNodeRemoved(vtkDMMLNode* dmmlNode)
{
  vtkDMMLCameraNode *cameraNode = vtkDMMLCameraNode::SafeDownCast(dmmlNode);
  if (!cameraNode)
    {
    //Q_ASSERT(cameraNode);
    return;
    }
  this->qvtkDisconnect(cameraNode, vtkDMMLCameraNode::LayoutNameModifiedEvent,
                       this, SLOT(synchronizeCameraWithView()));
}

//-----------------------------------------------------------------------------
void  qCjyxCamerasModuleWidget::setDMMLScene(vtkDMMLScene* scene)
{
  this->Superclass::setDMMLScene(scene);

  // When the view and camera selectors populate their items, the view might populate
  // its items before the camera, and the synchronizeCameraWithView() might do nothing.
  // Let's resync here.
  this->synchronizeCameraWithView();
}

//-----------------------------------------------------------
bool qCjyxCamerasModuleWidget::setEditedNode(vtkDMMLNode* node,
                                               QString role /* = QString()*/,
                                               QString context /* = QString()*/)
{
  Q_D(qCjyxCamerasModuleWidget);
  Q_UNUSED(role);
  Q_UNUSED(context);

  if (vtkDMMLViewNode::SafeDownCast(node))
    {
    d->ViewNodeSelector->setCurrentNode(node);
    return true;
    }

  if (vtkDMMLCameraNode::SafeDownCast(node))
    {
    vtkDMMLCameraNode* cameraNode = vtkDMMLCameraNode::SafeDownCast(node);
    vtkDMMLViewNode* viewNode = vtkDMMLViewNode::SafeDownCast(
      this->dmmlScene()->GetSingletonNode(cameraNode->GetLayoutName(), "vtkDMMLViewNode"));
    if (!viewNode)
      {
      return false;
      }
    d->ViewNodeSelector->setCurrentNode(viewNode);
    return true;
    }

  return false;
}
