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

// Qt includes
#include <QDebug>
#include <QDropEvent>
#include <QEvent>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QMimeData>
#include <QToolButton>
#include <QUrl>

// CTK includes
#include <ctkAxesWidget.h>
#include <ctkPopupWidget.h>
#include <vtkLightBoxRendererManager.h>

// qDMML includes
#include "qDMMLColors.h"
#include "qDMMLSliceView_p.h"
#include "qDMMLUtils.h"

// DMMLDisplayableManager includes
#include <vtkDMMLAbstractDisplayableManager.h>
#include <vtkDMMLCrosshairDisplayableManager.h>
#include <vtkDMMLDisplayableManagerGroup.h>
#include <vtkDMMLLightBoxRendererManagerProxy.h>
#include <vtkDMMLSliceViewDisplayableManagerFactory.h>
#include <vtkDMMLScalarBarDisplayableManager.h>
#include <vtkDMMLSliceViewInteractorStyle.h>

// DMML includes
#include <vtkDMMLLabelMapVolumeNode.h>
#include <vtkDMMLSliceCompositeNode.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceLogic.h>
#include <vtkDMMLSubjectHierarchyNode.h>
#include <vtkDMMLVolumeNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>

//--------------------------------------------------------------------------
// qDMMLSliceViewPrivate::vtkInternalLightBoxRendererManagerProxy class

//--------------------------------------------------------------------------
// vtkInternalLightBoxRendereManagerProxy methods
//vtkStandardNewMacro(qDMMLSliceViewPrivate::vtkInternalLightBoxRendererManagerProxy );

//---------------------------------------------------------------------------
// Using the vtkStandardNewMacro results in a compiler error about
// vtkInstantiatorqDMMLSliceWidgetPrivate has not been declared. This
// seems to be due to how the macro uses the type passed into the
// vtkStandardNewMacro as both a type and a classname string. Below,
// we do the equivalent to the vtkStandardNewMacro but use the full
// path to the type where needed and the scoped name elsewhere.
qDMMLSliceViewPrivate::vtkInternalLightBoxRendererManagerProxy *
qDMMLSliceViewPrivate::vtkInternalLightBoxRendererManagerProxy::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkInternalLightBoxRendererManagerProxy");
  if (ret)
    {
    return static_cast<qDMMLSliceViewPrivate::vtkInternalLightBoxRendererManagerProxy*>(ret);
    }

  qDMMLSliceViewPrivate::vtkInternalLightBoxRendererManagerProxy* result =
    new qDMMLSliceViewPrivate::vtkInternalLightBoxRendererManagerProxy;
#ifdef VTK_HAS_INITIALIZE_OBJECT_BASE
  result->InitializeObjectBase();
#endif
  return result;
}

//---------------------------------------------------------------------------
qDMMLSliceViewPrivate::vtkInternalLightBoxRendererManagerProxy::vtkInternalLightBoxRendererManagerProxy()
{
  this->LightBoxRendererManager = nullptr;
}

//---------------------------------------------------------------------------
qDMMLSliceViewPrivate::vtkInternalLightBoxRendererManagerProxy::~vtkInternalLightBoxRendererManagerProxy()
{
  this->LightBoxRendererManager = nullptr;
}

//---------------------------------------------------------------------------
vtkRenderer* qDMMLSliceViewPrivate::vtkInternalLightBoxRendererManagerProxy::GetRenderer(int id)
{
  if (this->LightBoxRendererManager)
    {
    return this->LightBoxRendererManager->GetRenderer(id);
    }
  return nullptr;
}

//---------------------------------------------------------------------------
void qDMMLSliceViewPrivate::vtkInternalLightBoxRendererManagerProxy::SetLightBoxRendererManager(vtkLightBoxRendererManager *mgr)
{
  this->LightBoxRendererManager = mgr;
}

//--------------------------------------------------------------------------
// qDMMLSliceViewPrivate methods

//---------------------------------------------------------------------------
qDMMLSliceViewPrivate::qDMMLSliceViewPrivate(qDMMLSliceView& object)
  : q_ptr(&object)
{
  this->DisplayableManagerGroup = nullptr;
  this->DMMLScene = nullptr;
  this->DMMLSliceNode = nullptr;
  this->InactiveBoxColor = QColor(95, 95, 113);
  this->LightBoxRendererManagerProxy = vtkInternalLightBoxRendererManagerProxy::New();
}

//---------------------------------------------------------------------------
qDMMLSliceViewPrivate::~qDMMLSliceViewPrivate()
{
  if (this->DisplayableManagerGroup)
    {
    this->DisplayableManagerGroup->Delete();
    }
  if (this->LightBoxRendererManagerProxy)
    {
    this->LightBoxRendererManagerProxy->Delete();
    }
}

//---------------------------------------------------------------------------
void qDMMLSliceViewPrivate::init()
{
  Q_Q(qDMMLSliceView);

  // Highligh first RenderWindowItem
  q->setHighlightedBoxColor(this->InactiveBoxColor);

  q->setRenderEnabled(this->DMMLScene != nullptr);

  vtkNew<vtkDMMLSliceViewInteractorStyle> interactorStyle;
  q->interactor()->SetInteractorStyle(interactorStyle.GetPointer());

  this->LightBoxRendererManagerProxy->SetLightBoxRendererManager(
    q->lightBoxRendererManager());
  this->initDisplayableManagers();
  interactorStyle->SetDisplayableManagers(this->DisplayableManagerGroup);

  // Force an initial render to ensure that the render window creates an OpenGL
  // context. If operations that require a context--such as hardware
  // picking--are performed when no context exists, OpenGL errors will occur.
  // When using the VTK OpenGL2 backend the errors may be followed by a
  // segfault. Such a scenario can occur when the app is started using a layout
  // that hides one or more of the slice views.
  q->renderWindow()->Render();
}

//---------------------------------------------------------------------------
void qDMMLSliceViewPrivate::initDisplayableManagers()
{
  Q_Q(qDMMLSliceView);

  vtkDMMLSliceViewDisplayableManagerFactory* factory
    = vtkDMMLSliceViewDisplayableManagerFactory::GetInstance();

  QStringList displayableManagers;
  displayableManagers << "vtkDMMLVolumeGlyphSliceDisplayableManager";
  displayableManagers << "vtkDMMLModelSliceDisplayableManager";
  displayableManagers << "vtkDMMLCrosshairDisplayableManager";
  displayableManagers << "vtkDMMLOrientationMarkerDisplayableManager";
  displayableManagers << "vtkDMMLRulerDisplayableManager";
  displayableManagers << "vtkDMMLScalarBarDisplayableManager";
  foreach(const QString& displayableManager, displayableManagers)
    {
    if (!factory->IsDisplayableManagerRegistered(displayableManager.toUtf8()))
      {
      factory->RegisterDisplayableManager(displayableManager.toUtf8());
      }
    }

  this->DisplayableManagerGroup
    = factory->InstantiateDisplayableManagers(
      q->lightBoxRendererManager()->GetRenderer(0));
  // Observe displayable manager group to catch RequestRender events
  q->qvtkConnect(this->DisplayableManagerGroup, vtkCommand::UpdateEvent,
                 q, SLOT(scheduleRender()));

  // pass the lightbox manager proxy onto the display managers
  this->DisplayableManagerGroup->SetLightBoxRendererManagerProxy(this->LightBoxRendererManagerProxy);

}

//---------------------------------------------------------------------------
void qDMMLSliceViewPrivate::setDMMLScene(vtkDMMLScene* newScene)
{
  Q_Q(qDMMLSliceView);
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
void qDMMLSliceViewPrivate::onSceneStartProcessing()
{
  Q_Q(qDMMLSliceView);
  q->setRenderEnabled(false);
}

// --------------------------------------------------------------------------
void qDMMLSliceViewPrivate::onSceneEndProcessing()
{
  Q_Q(qDMMLSliceView);
  q->setRenderEnabled(true);
}

// --------------------------------------------------------------------------
void qDMMLSliceViewPrivate::updateWidgetFromDMML()
{
  Q_Q(qDMMLSliceView);
  if (!this->DMMLSliceNode)
    {
    return;
    }
  q->lightBoxRendererManager()->SetRenderWindowLayout(
    this->DMMLSliceNode->GetLayoutGridRows(),
    this->DMMLSliceNode->GetLayoutGridColumns());
  bool displayLightboxBorders =
    this->DMMLSliceNode->GetLayoutGridRows() != 1 ||
    this->DMMLSliceNode->GetLayoutGridColumns() != 1;
  q->lightBoxRendererManager()->SetHighlighted(0, 0, displayLightboxBorders);
}

// --------------------------------------------------------------------------
// qDMMLSliceView methods

// --------------------------------------------------------------------------
qDMMLSliceView::qDMMLSliceView(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qDMMLSliceViewPrivate(*this))
{
  Q_D(qDMMLSliceView);
  d->init();
  setAcceptDrops(true);
}

// --------------------------------------------------------------------------
qDMMLSliceView::~qDMMLSliceView() = default;

//------------------------------------------------------------------------------
void qDMMLSliceView::addDisplayableManager(const QString& displayableManagerName)
{
  Q_D(qDMMLSliceView);
  vtkSmartPointer<vtkDMMLAbstractDisplayableManager> displayableManager;
  displayableManager.TakeReference(
    vtkDMMLDisplayableManagerGroup::InstantiateDisplayableManager(
      displayableManagerName.toUtf8()));
  d->DisplayableManagerGroup->AddDisplayableManager(displayableManager);
}

//------------------------------------------------------------------------------
void qDMMLSliceView::getDisplayableManagers(vtkCollection *displayableManagers)
{
  Q_D(qDMMLSliceView);

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
vtkDMMLAbstractDisplayableManager* qDMMLSliceView::displayableManagerByClassName(const char* className)
{
  Q_D(qDMMLSliceView);
  return d->DisplayableManagerGroup->GetDisplayableManagerByClassName(className);
}


//------------------------------------------------------------------------------
void qDMMLSliceView::setDMMLScene(vtkDMMLScene* newScene)
{
  Q_D(qDMMLSliceView);
  d->setDMMLScene(newScene);

  if (d->DMMLSliceNode && newScene != d->DMMLSliceNode->GetScene())
    {
    this->setDMMLSliceNode(nullptr);
    }
}

//---------------------------------------------------------------------------
void qDMMLSliceView::setDMMLSliceNode(vtkDMMLSliceNode* newSliceNode)
{
  Q_D(qDMMLSliceView);
  if (d->DMMLSliceNode == newSliceNode)
    {
    return;
    }

  d->qvtkReconnect(
    d->DMMLSliceNode, newSliceNode,
    vtkCommand::ModifiedEvent, d, SLOT(updateWidgetFromDMML()));

  d->DMMLSliceNode = newSliceNode;
  d->updateWidgetFromDMML();

  d->DisplayableManagerGroup->SetDMMLDisplayableNode(newSliceNode);

  // Enable/disable widget
  this->setEnabled(newSliceNode != nullptr);
}

//---------------------------------------------------------------------------
vtkDMMLSliceNode* qDMMLSliceView::dmmlSliceNode()const
{
  Q_D(const qDMMLSliceView);
  return d->DMMLSliceNode;
}

//---------------------------------------------------------------------------
vtkDMMLSliceViewInteractorStyle* qDMMLSliceView::sliceViewInteractorStyle()const
{
  return vtkDMMLSliceViewInteractorStyle::SafeDownCast(this->interactorStyle());
}

// --------------------------------------------------------------------------
QList<double> qDMMLSliceView::convertDeviceToXYZ(const QList<int>& xy)const
{
  Q_D(const qDMMLSliceView);

  // Grab a displayable manager that is derived from
  // AbstractSliceViewDisplayableManager, like the CrosshairDisplayableManager
  vtkDMMLCrosshairDisplayableManager *cmgr =
    vtkDMMLCrosshairDisplayableManager::SafeDownCast(
      d->DisplayableManagerGroup->GetDisplayableManagerByClassName(
        "vtkDMMLCrosshairDisplayableManager"));
  if (cmgr)
    {
    double xyz[3];
    cmgr->ConvertDeviceToXYZ(xy[0], xy[1], xyz);
    QList<double> ret;
    ret << xyz[0] << xyz[1] << xyz[2];
    return ret;
    }

  QList<double> ret;
  ret << 0. << 0. << 0.;
  return ret;
}

// --------------------------------------------------------------------------
QList<double> qDMMLSliceView::convertRASToXYZ(const QList<double>& ras)const
{
  Q_D(const qDMMLSliceView);

  // Grab a displayable manager that is derived from
  // AbstractSliceViewDisplayableManager, like the CrosshairDisplayableManager
  vtkDMMLCrosshairDisplayableManager *cmgr =
    vtkDMMLCrosshairDisplayableManager::SafeDownCast(
      d->DisplayableManagerGroup->GetDisplayableManagerByClassName(
        "vtkDMMLCrosshairDisplayableManager"));
  if (cmgr)
    {
    double rasv[3], xyz[3];
    rasv[0] = ras[0]; rasv[1] = ras[1]; rasv[2] = ras[2];
    cmgr->ConvertRASToXYZ(rasv, xyz);
    QList<double> ret;
    ret << xyz[0] << xyz[1] << xyz[2];
    return ret;
    }

  QList<double> ret;
  ret << 0. << 0. << 0.;
  return ret;
}

// --------------------------------------------------------------------------
QList<double> qDMMLSliceView::convertXYZToRAS(const QList<double>& xyz)const
{
  Q_D(const qDMMLSliceView);

  // Grab a displayable manager that is derived from
  // AbstractSliceViewDisplayableManager, like the CrosshairDisplayableManager
  vtkDMMLCrosshairDisplayableManager *cmgr =
    vtkDMMLCrosshairDisplayableManager::SafeDownCast(
      d->DisplayableManagerGroup->GetDisplayableManagerByClassName(
        "vtkDMMLCrosshairDisplayableManager"));
  if (cmgr)
    {
    double xyzv[3], ras[3];
    xyzv[0] = xyz[0]; xyzv[1] = xyz[1]; xyzv[2] = xyz[2];
    cmgr->ConvertXYZToRAS(xyzv, ras);
    QList<double> ret;
    ret << ras[0] << ras[1] << ras[2];
    return ret;
    }

  QList<double> ret;
  ret << 0. << 0. << 0.;
  return ret;
}

// --------------------------------------------------------------------------
void qDMMLSliceView::setViewCursor(const QCursor &cursor)
{
  this->setCursor(cursor);
  if (this->VTKWidget() != nullptr)
    {
    this->VTKWidget()->setCursor(cursor);  // TODO: test if cursor settings works
    }
}

// --------------------------------------------------------------------------
void qDMMLSliceView::unsetViewCursor()
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
void qDMMLSliceView::setDefaultViewCursor(const QCursor &cursor)
{
  if (this->VTKWidget() != nullptr)
    {
    this->VTKWidget()->setDefaultCursor(cursor);  // TODO: test if cursor settings works
    }
}

//---------------------------------------------------------------------------
void qDMMLSliceView::dragEnterEvent(QDragEnterEvent* event)
{
  Q_D(qDMMLSliceView);
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
void qDMMLSliceView::dropEvent(QDropEvent* event)
{
  Q_D(qDMMLSliceView);
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
  shNode->ShowItemsInView(shItemIdList, this->dmmlSliceNode());
}
