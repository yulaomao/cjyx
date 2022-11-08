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
#include <QDropEvent>
#include <QEvent>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QMimeData>
#include <QToolButton>

// CTK includes
#include <ctkAxesWidget.h>

// qDMML includes
#include "qDMMLColors.h"
#include "qDMMLThreeDView_p.h"
#include "qDMMLUtils.h"

// DMMLDisplayableManager includes
#include <vtkDMMLAbstractDisplayableManager.h>
#include <vtkDMMLCrosshairDisplayableManager.h>
#include <vtkDMMLDisplayableManagerGroup.h>
#include <vtkDMMLThreeDViewDisplayableManagerFactory.h>
#include <vtkDMMLThreeDViewInteractorStyle.h>

// DMML includes
#include <vtkDMMLViewNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLCameraNode.h>
#include <vtkDMMLCrosshairNode.h>
#include <vtkDMMLSubjectHierarchyNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>

//--------------------------------------------------------------------------
// qDMMLThreeDViewPrivate methods

//---------------------------------------------------------------------------
qDMMLThreeDViewPrivate::qDMMLThreeDViewPrivate(qDMMLThreeDView& object)
  : q_ptr(&object)
{
  this->DisplayableManagerGroup = nullptr;
  this->DMMLScene = nullptr;
  this->DMMLViewNode = nullptr;
}

//---------------------------------------------------------------------------
qDMMLThreeDViewPrivate::~qDMMLThreeDViewPrivate()
{
  if (this->DisplayableManagerGroup)
    {
    this->DisplayableManagerGroup->Delete();
    }
}

//---------------------------------------------------------------------------
void qDMMLThreeDViewPrivate::init()
{
  Q_Q(qDMMLThreeDView);
  q->setRenderEnabled(this->DMMLScene != nullptr);

  vtkNew<vtkDMMLThreeDViewInteractorStyle> interactorStyle;
  q->interactor()->SetInteractorStyle(interactorStyle.GetPointer());

  // Set default background color
  q->setBackgroundColor(QColor::fromRgbF(
    vtkDMMLViewNode::defaultBackgroundColor()[0],
    vtkDMMLViewNode::defaultBackgroundColor()[1],
    vtkDMMLViewNode::defaultBackgroundColor()[2]));

  q->setGradientBackground(true);

  // Hide orientation widget
  q->setOrientationWidgetVisible(false);

  q->setZoomFactor(0.05);

  q->setPitchDirection(ctkVTKRenderView::PitchUp);
  q->setRollDirection(ctkVTKRenderView::RollRight);
  q->setYawDirection(ctkVTKRenderView::YawLeft);

  this->initDisplayableManagers();
  interactorStyle->SetDisplayableManagers(this->DisplayableManagerGroup);
}

//---------------------------------------------------------------------------
void qDMMLThreeDViewPrivate::initDisplayableManagers()
{
  Q_Q(qDMMLThreeDView);
  vtkDMMLThreeDViewDisplayableManagerFactory* factory
    = vtkDMMLThreeDViewDisplayableManagerFactory::GetInstance();

  QStringList displayableManagers;
  displayableManagers << "vtkDMMLCameraDisplayableManager"
                      << "vtkDMMLViewDisplayableManager"
                      << "vtkDMMLModelDisplayableManager"
                      << "vtkDMMLThreeDReformatDisplayableManager"
                      << "vtkDMMLCrosshairDisplayableManager3D"
                      << "vtkDMMLOrientationMarkerDisplayableManager"
                      << "vtkDMMLRulerDisplayableManager";
  foreach(const QString& displayableManager, displayableManagers)
    {
    if(!factory->IsDisplayableManagerRegistered(displayableManager.toUtf8()))
      {
      factory->RegisterDisplayableManager(displayableManager.toUtf8());
      }
    }

  this->DisplayableManagerGroup
    = factory->InstantiateDisplayableManagers(q->renderer());
  // Observe displayable manager group to catch RequestRender events
  this->qvtkConnect(this->DisplayableManagerGroup, vtkCommand::UpdateEvent,
                    q, SLOT(scheduleRender()));
}

//---------------------------------------------------------------------------
void qDMMLThreeDViewPrivate::setDMMLScene(vtkDMMLScene* newScene)
{
  Q_Q(qDMMLThreeDView);
  if (newScene == this->DMMLScene)
    {
    return;
    }

  this->qvtkReconnect(
    this->DMMLScene, newScene,
    vtkDMMLScene::StartBatchProcessEvent, this, SLOT(onSceneStartProcessing()));

  this->qvtkReconnect(
    this->DMMLScene, newScene,
    vtkDMMLScene::EndBatchProcessEvent, this, SLOT(onSceneEndProcessing()));

  this->DMMLScene = newScene;
  q->setRenderEnabled(
    this->DMMLScene != nullptr && !this->DMMLScene->IsBatchProcessing());
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewPrivate::onSceneStartProcessing()
{
  //qDebug() << "qDMMLThreeDViewPrivate::onSceneStartProcessing";
  Q_Q(qDMMLThreeDView);
  q->setRenderEnabled(false);
}

//
// --------------------------------------------------------------------------
void qDMMLThreeDViewPrivate::onSceneEndProcessing()
{
  //qDebug() << "qDMMLThreeDViewPrivate::onSceneImportedEvent";
  Q_Q(qDMMLThreeDView);
  q->setRenderEnabled(true);
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewPrivate::updateWidgetFromDMML()
{
  Q_Q(qDMMLThreeDView);
  if (!this->DMMLViewNode)
    {
    return;
    }
  q->setAnimationIntervalMs(this->DMMLViewNode->GetAnimationMs());
  q->setPitchRollYawIncrement(this->DMMLViewNode->GetRotateDegrees());
  q->setSpinIncrement(this->DMMLViewNode->GetSpinDegrees());
  q->setRockIncrement(this->DMMLViewNode->GetRockCount());
  q->setRockLength(this->DMMLViewNode->GetRockLength());

  q->setSpinEnabled(this->DMMLViewNode->GetAnimationMode() == vtkDMMLViewNode::Spin);
  q->setRockEnabled(this->DMMLViewNode->GetAnimationMode() == vtkDMMLViewNode::Rock);

  q->setUseDepthPeeling(this->DMMLViewNode->GetUseDepthPeeling() != 0);
  q->setFPSVisible(this->DMMLViewNode->GetFPSVisible() != 0);
}

// --------------------------------------------------------------------------
// qDMMLThreeDView methods

// --------------------------------------------------------------------------
namespace
{
void ClickCallbackFunction (
  vtkObject* caller,
  long unsigned int eventId,
  void* vtkNotUsed(clientData),
  void* vtkNotUsed(callData) )
{
  vtkRenderWindowInteractor *iren =
     static_cast<vtkRenderWindowInteractor*>(caller);

  vtkDMMLThreeDViewInteractorStyle* style = vtkDMMLThreeDViewInteractorStyle::SafeDownCast
    (iren ? iren->GetInteractorStyle() : nullptr);
  if (!style)
    {
    qCritical() << "qDMMLThreeDView::mouseMoveEvent: no valid interactor style.";
    return;
    }

  vtkDMMLCameraNode* cam = style->GetCameraNode();
  if (!cam)
    {
    qCritical() << "qDMMLThreeDView::mouseMoveEvent: can not retrieve camera node.";
    return;
    }

  switch(eventId)
    {
    case vtkCommand::MouseWheelForwardEvent:
      {
      cam->InvokeCustomModifiedEvent(vtkDMMLCameraNode::CameraInteractionEvent);
      }
    break;
    case vtkCommand::MouseWheelBackwardEvent:
      {
      cam->InvokeCustomModifiedEvent(vtkDMMLCameraNode::CameraInteractionEvent);
      }
    break;
    case vtkCommand::InteractionEvent:
      {
      cam->InvokeCustomModifiedEvent(vtkDMMLCameraNode::CameraInteractionEvent);
      }
    break;
    case vtkCommand::KeyPressEvent:
      {
      cam->InvokeCustomModifiedEvent(vtkDMMLCameraNode::CameraInteractionEvent);
      }
    break;
    }
}
}

// --------------------------------------------------------------------------
qDMMLThreeDView::qDMMLThreeDView(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qDMMLThreeDViewPrivate(*this))
{
  Q_D(qDMMLThreeDView);
  d->init();
  setAcceptDrops(true);

  vtkRenderWindowInteractor* renderWindowInteractor = this->interactor();

  vtkSmartPointer<vtkCallbackCommand> clickCallback =
      vtkSmartPointer<vtkCallbackCommand>::New();
  clickCallback->SetCallback(ClickCallbackFunction);

  renderWindowInteractor->AddObserver(vtkCommand::MouseWheelForwardEvent, clickCallback);
  renderWindowInteractor->AddObserver(vtkCommand::MouseWheelBackwardEvent, clickCallback);
  renderWindowInteractor->AddObserver(vtkCommand::InteractionEvent, clickCallback);
  renderWindowInteractor->AddObserver(vtkCommand::KeyPressEvent, clickCallback);
}

// --------------------------------------------------------------------------
qDMMLThreeDView::~qDMMLThreeDView() = default;

//------------------------------------------------------------------------------
void qDMMLThreeDView::addDisplayableManager(const QString& displayableManagerName)
{
  Q_D(qDMMLThreeDView);
  vtkSmartPointer<vtkDMMLAbstractDisplayableManager> displayableManager;
  displayableManager.TakeReference(
    vtkDMMLDisplayableManagerGroup::InstantiateDisplayableManager(
      displayableManagerName.toUtf8()));
  d->DisplayableManagerGroup->AddDisplayableManager(displayableManager);
}

//------------------------------------------------------------------------------
vtkDMMLCameraNode* qDMMLThreeDView::cameraNode()
{
  vtkDMMLThreeDViewInteractorStyle* style = vtkDMMLThreeDViewInteractorStyle::SafeDownCast(this->interactorStyle());
  if (!style)
    {
    return nullptr;
    }
  vtkDMMLCameraNode* cam = style->GetCameraNode();
  return cam;
}

//------------------------------------------------------------------------------
void qDMMLThreeDView::rotateToViewAxis(unsigned int axisId)
{
  vtkDMMLThreeDViewInteractorStyle* style =
    vtkDMMLThreeDViewInteractorStyle::SafeDownCast(this->interactorStyle());
  if (!style)
    {
    qCritical() << "qDMMLThreeDView::rotateToViewAxis: no valid interactor style.";
    return;
    }

  vtkDMMLCameraNode* cam = style->GetCameraNode();
  if (!cam)
    {
    qCritical() << "qDMMLThreeDView::rotateToViewAxis: can not retrieve camera node.";
    return;
    }

  switch (axisId)
    {
  case 0:
    cam->RotateTo(vtkDMMLCameraNode::Left);
    break;
  case 1:
    cam->RotateTo(vtkDMMLCameraNode::Right);
    break;
  case 2:
    cam->RotateTo(vtkDMMLCameraNode::Posterior);
    break;
  case 3:
    cam->RotateTo(vtkDMMLCameraNode::Anterior);
    break;
  case 4:
    cam->RotateTo(vtkDMMLCameraNode::Inferior);
    break;
  case 5:
    cam->RotateTo(vtkDMMLCameraNode::Superior);
    break;
  default:
    qWarning() << "qDMMLThreeDView::rotateToViewAxis: " << axisId
               << " is not a valid axis id (0 to 5 : "
               << "-X, +X, -Y, +Y, -Z, +Z).";
    break;
    }
}

//------------------------------------------------------------------------------
void qDMMLThreeDView::rotateToViewAxis(const std::string& axisLabel)
{
  Q_D(qDMMLThreeDView);
  if (!d->DMMLViewNode)
    {
    qCritical() << "qDMMLThreeDView::rotateToViewAxis: no valid view node.";
    return;
    }

  for (int i = 0; i < vtkDMMLAbstractViewNode::AxisLabelsCount; ++i)
    {
    if (axisLabel == std::string(d->DMMLViewNode->GetAxisLabel(i)))
      {
      this->rotateToViewAxis(i);
      return;
      }
    }
  qWarning() << "qDMMLThreeDView::rotateToViewAxis: " << QString(axisLabel.c_str())
              << "is not a valid axis label.";
}

//------------------------------------------------------------------------------
void qDMMLThreeDView
::resetCamera(bool resetRotation, bool resetTranslation, bool resetDistance)
{
  vtkDMMLThreeDViewInteractorStyle* style =
    vtkDMMLThreeDViewInteractorStyle::SafeDownCast(this->interactorStyle());
  if (!style)
    {
    qCritical() << "qDMMLThreeDView::resetCamera: no valid interactor style.";
    return;
    }

  vtkDMMLCameraNode* cam = style->GetCameraNode();
  if (!cam)
    {
    qCritical() << "qDMMLThreeDView::resetCamera: can not retrieve camera node.";
    return;
    }

  cam->Reset(resetRotation, resetTranslation, resetDistance, this->renderer());
}

//------------------------------------------------------------------------------
void qDMMLThreeDView::setDMMLScene(vtkDMMLScene* newScene)
{
  Q_D(qDMMLThreeDView);
  d->setDMMLScene(newScene);

  if (d->DMMLViewNode && newScene != d->DMMLViewNode->GetScene())
    {
    this->setDMMLViewNode(nullptr);
    }
}

//---------------------------------------------------------------------------
void qDMMLThreeDView::setDMMLViewNode(vtkDMMLViewNode* newViewNode)
{
  Q_D(qDMMLThreeDView);
  if (d->DMMLViewNode == newViewNode)
    {
    return;
    }

  d->qvtkReconnect(
    d->DMMLViewNode, newViewNode,
    vtkCommand::ModifiedEvent, d, SLOT(updateWidgetFromDMML()));

  d->DMMLViewNode = newViewNode;
  d->DisplayableManagerGroup->SetDMMLDisplayableNode(newViewNode);

  d->updateWidgetFromDMML();
  // Enable/disable widget
  this->setEnabled(newViewNode != nullptr);
}

//---------------------------------------------------------------------------
vtkDMMLViewNode* qDMMLThreeDView::dmmlViewNode()const
{
  Q_D(const qDMMLThreeDView);
  return d->DMMLViewNode;
}

// --------------------------------------------------------------------------
void qDMMLThreeDView::lookFromViewAxis(const ctkAxesWidget::Axis& axis)
{
  Q_D(qDMMLThreeDView);
  if (!d->DMMLViewNode)
    {
    qCritical() << "qDMMLThreeDView::lookFromViewAxis: no valid view node.";
    return;
    }
  double fov = d->DMMLViewNode->GetFieldOfView();
  Q_ASSERT(fov >= 0.0);
  this->lookFromAxis(axis, fov);
}

// --------------------------------------------------------------------------
void qDMMLThreeDView::resetFocalPoint()
{
  Q_D(qDMMLThreeDView);

  bool savedBoxVisibile = true;
  bool savedAxisLabelVisible = true;
  if (d->DMMLViewNode)
    {
    // Save current visibility state of Box and AxisLabel
    savedBoxVisibile = d->DMMLViewNode->GetBoxVisible();
    savedAxisLabelVisible = d->DMMLViewNode->GetAxisLabelsVisible();

    int wasModifying = d->DMMLViewNode->StartModify();
    // Hide Box and AxisLabel so they don't get taken into account when computing
    // the view boundaries
    d->DMMLViewNode->SetBoxVisible(0);
    d->DMMLViewNode->SetAxisLabelsVisible(0);
    d->DMMLViewNode->EndModify(wasModifying);
    }

  // Exclude crosshair from focal point computation
  vtkDMMLCrosshairNode* crosshairNode = vtkDMMLCrosshairDisplayableManager::FindCrosshairNode(d->DMMLScene);
  int crosshairMode = 0;
  if (crosshairNode)
    {
    crosshairMode = crosshairNode->GetCrosshairMode();
    crosshairNode->SetCrosshairMode(vtkDMMLCrosshairNode::NoCrosshair);
    }

  // Superclass resets the camera.
  this->Superclass::resetFocalPoint();

  if (d->DMMLViewNode)
    {
    // Restore visibility state
    int wasModifying = d->DMMLViewNode->StartModify();
    d->DMMLViewNode->SetBoxVisible(savedBoxVisibile);
    d->DMMLViewNode->SetAxisLabelsVisible(savedAxisLabelVisible);
    d->DMMLViewNode->EndModify(wasModifying);
    // Inform the displayable manager that the view is reset, so it can
    // update the box/labels bounds.
    d->DMMLViewNode->InvokeEvent(vtkDMMLViewNode::ResetFocalPointRequestedEvent);
    }

  if (crosshairNode)
    {
    crosshairNode->SetCrosshairMode(crosshairMode);
    }

  if (this->renderer())
    {
    this->renderer()->ResetCameraClippingRange();
    }
}

//------------------------------------------------------------------------------
void qDMMLThreeDView::getDisplayableManagers(vtkCollection *displayableManagers)
{
  Q_D(qDMMLThreeDView);

  if (!displayableManagers)
    {
    return;
    }
  int num = d->DisplayableManagerGroup->GetDisplayableManagerCount();
  for (int n = 0; n < num; n++)
    {
    displayableManagers->AddItem(d->DisplayableManagerGroup->GetNthDisplayableManager(n));
    }
}

//------------------------------------------------------------------------------
vtkDMMLAbstractDisplayableManager* qDMMLThreeDView::displayableManagerByClassName(const char* className)
{
  Q_D(qDMMLThreeDView);
  return d->DisplayableManagerGroup->GetDisplayableManagerByClassName(className);
}

// --------------------------------------------------------------------------
void qDMMLThreeDView::setViewCursor(const QCursor &cursor)
{
  this->setCursor(cursor);
  if (this->VTKWidget() != nullptr)
    {
    this->VTKWidget()->setCursor(cursor);  // TODO: test if cursor settings works
    }
}

// --------------------------------------------------------------------------
void qDMMLThreeDView::unsetViewCursor()
{
  this->unsetCursor();
  if (this->VTKWidget() != nullptr)
    {
    // TODO: it would be better to restore default cursor, but QVTKOpenGLNativeWidget
    // API does not have an accessor method to the default cursor.
    this->VTKWidget()->setCursor(QCursor(Qt::ArrowCursor));  // TODO: test if cursor settings works
    }
}

// --------------------------------------------------------------------------
void qDMMLThreeDView::setDefaultViewCursor(const QCursor &cursor)
{
  if (this->VTKWidget() != nullptr)
    {
    this->VTKWidget()->setDefaultCursor(cursor);  // TODO: test if cursor settings works
    }
}

//---------------------------------------------------------------------------
void qDMMLThreeDView::dragEnterEvent(QDragEnterEvent* event)
{
  Q_D(qDMMLThreeDView);
  vtkNew<vtkIdList> shItemIdList;
  qDMMLUtils::mimeDataToSubjectHierarchyItemIDs(event->mimeData(), shItemIdList);
  if (shItemIdList->GetNumberOfIds() > 0)
    {
    event->accept();
    return;
    }
  Superclass::dragEnterEvent(event);
}

//-----------------------------------------------------------------------------
void qDMMLThreeDView::dropEvent(QDropEvent* event)
{
  Q_D(qDMMLThreeDView);
  vtkNew<vtkIdList> shItemIdList;
  qDMMLUtils::mimeDataToSubjectHierarchyItemIDs(event->mimeData(), shItemIdList);
  if (!shItemIdList->GetNumberOfIds())
    {
    return;
    }
  vtkDMMLSubjectHierarchyNode* shNode = vtkDMMLSubjectHierarchyNode::GetSubjectHierarchyNode(d->DMMLScene);
  if (!shNode)
    {
    qWarning() << Q_FUNC_INFO << " failed: invalid subject hierarchy node";
    return;
    }
  shNode->ShowItemsInView(shItemIdList, this->dmmlViewNode());
}
