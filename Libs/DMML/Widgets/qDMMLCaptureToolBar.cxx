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
#include <QMenu>
#include <QInputDialog>
#include <QTimer>
#include <QToolButton>

// CTK includes
#include <ctkMessageBox.h>

// qDMML includes
#include "qDMMLCaptureToolBar.h"
#include "qDMMLSceneViewMenu.h"
#include "qDMMLNodeFactory.h"

// DMML includes
#include <vtkDMMLViewNode.h>
#include <vtkDMMLSceneViewNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

//-----------------------------------------------------------------------------
class qDMMLCaptureToolBarPrivate
{
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qDMMLCaptureToolBar);
protected:
  qDMMLCaptureToolBar* const q_ptr;
  bool timeOutFlag;
public:
  qDMMLCaptureToolBarPrivate(qDMMLCaptureToolBar& object);
  void init();
  void setDMMLScene(vtkDMMLScene* newScene);
  QAction*                         ScreenshotAction;
  QAction*                         SceneViewAction;
  qDMMLSceneViewMenu*              SceneViewMenu;

  // TODO In LayoutManager, use GetActive/IsActive flag ...
  vtkWeakPointer<vtkDMMLViewNode>  ActiveDMMLThreeDViewNode;
  vtkSmartPointer<vtkDMMLScene>    DMMLScene;

public slots:
  void OnDMMLSceneStartBatchProcessing();
  void OnDMMLSceneEndBatchProcessing();
  void updateWidgetFromDMML();
  void createSceneView();
};

//--------------------------------------------------------------------------
// qDMMLCaptureToolBarPrivate methods

//---------------------------------------------------------------------------
qDMMLCaptureToolBarPrivate::qDMMLCaptureToolBarPrivate(qDMMLCaptureToolBar& object)
  : q_ptr(&object)
{
  this->ScreenshotAction = nullptr;
  this->SceneViewAction = nullptr;
  this->SceneViewMenu = nullptr;
  this->timeOutFlag = false;
}

// --------------------------------------------------------------------------
void qDMMLCaptureToolBarPrivate::updateWidgetFromDMML()
{
  Q_Q(qDMMLCaptureToolBar);
  // Enable buttons
  q->setEnabled(this->DMMLScene != nullptr);
  this->ScreenshotAction->setEnabled(this->ActiveDMMLThreeDViewNode != nullptr);
}

//---------------------------------------------------------------------------
void qDMMLCaptureToolBarPrivate::init()
{
  Q_Q(qDMMLCaptureToolBar);

  // Screenshot button
  this->ScreenshotAction = new QAction(q);
  this->ScreenshotAction->setIcon(QIcon(":/Icons/ViewCapture.png"));
  this->ScreenshotAction->setText(qDMMLCaptureToolBar::tr("Screenshot"));
  this->ScreenshotAction->setToolTip(qDMMLCaptureToolBar::tr(
    "Capture a screenshot of the full layout, 3D view or slice views. Use File, Save to save the image. Edit in the Annotations module."));
  QObject::connect(this->ScreenshotAction, SIGNAL(triggered()),
                   q, SIGNAL(screenshotButtonClicked()));
  q->addAction(this->ScreenshotAction);

  // Scene View buttons
  this->SceneViewAction = new QAction(q);
  this->SceneViewAction->setIcon(QIcon(":/Icons/ViewCamera.png"));
  this->SceneViewAction->setText(qDMMLCaptureToolBar::tr("Scene view"));
  this->SceneViewAction->setToolTip(qDMMLCaptureToolBar::tr("Capture and name a scene view."));
  QObject::connect(this->SceneViewAction, SIGNAL(triggered()),
                   q, SIGNAL(sceneViewButtonClicked()));
  q->addAction(this->SceneViewAction);

  // Scene view menu
  QToolButton* sceneViewMenuButton = new QToolButton(q);
  sceneViewMenuButton->setText(qDMMLCaptureToolBar::tr("Restore view"));
  sceneViewMenuButton->setIcon(QIcon(":/Icons/ViewCameraSelect.png"));
  sceneViewMenuButton->setToolTip(qDMMLCaptureToolBar::tr("Restore or delete saved scene views."));
  this->SceneViewMenu = new qDMMLSceneViewMenu(sceneViewMenuButton);
  sceneViewMenuButton->setMenu(this->SceneViewMenu);
  sceneViewMenuButton->setPopupMode(QToolButton::InstantPopup);
  //QObject::connect(q, SIGNAL(dmmlSceneChanged(vtkDMMLScene*)),
  //                 this->SceneViewMenu, SLOT(setDMMLScene(vtkDMMLScene*)));
  q->addWidget(sceneViewMenuButton);
  QObject::connect(q, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
                  sceneViewMenuButton,
                  SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));
}
// --------------------------------------------------------------------------
void qDMMLCaptureToolBarPrivate::setDMMLScene(vtkDMMLScene* newScene)
{
  if (newScene == this->DMMLScene)
    {
    return;
    }
/*
  this->qvtkReconnect(this->DMMLScene, newScene, vtkDMMLScene::StartBatchProcessEvent,
                      this, SLOT(OnDMMLSceneStartBatchProcessing()));

  this->qvtkReconnect(this->DMMLScene, newScene, vtkDMMLScene::EndBatchProcessEvent,
                      this, SLOT(OnDMMLSceneEndBatchProcessing()));

*/

  this->DMMLScene = newScene;

  this->SceneViewMenu->setDMMLScene(newScene);

  // Update UI
  this->updateWidgetFromDMML();
}

// --------------------------------------------------------------------------
void qDMMLCaptureToolBarPrivate::OnDMMLSceneStartBatchProcessing()
{
  Q_Q(qDMMLCaptureToolBar);
  q->setEnabled(false);
}

// --------------------------------------------------------------------------
void qDMMLCaptureToolBarPrivate::OnDMMLSceneEndBatchProcessing()
{
  this->updateWidgetFromDMML();
}

// --------------------------------------------------------------------------
void qDMMLCaptureToolBarPrivate::createSceneView()
{
  Q_Q(qDMMLCaptureToolBar);

  // Ask user for a name
  bool ok = false;
  QString sceneViewName = QInputDialog::getText(q, qDMMLCaptureToolBar::tr("SceneView Name"),
                                                qDMMLCaptureToolBar::tr("SceneView Name:"), QLineEdit::Normal,
                                                "View", &ok);
  if (!ok || sceneViewName.isEmpty())
    {
    return;
    }

  // Create scene view
  qDMMLNodeFactory nodeFactory;
  nodeFactory.setDMMLScene(this->DMMLScene);
  nodeFactory.setBaseName("vtkDMMLSceneViewNode", sceneViewName);
  vtkDMMLNode * newNode = nodeFactory.createNode("vtkDMMLSceneViewNode");
  vtkDMMLSceneViewNode * newSceneViewNode = vtkDMMLSceneViewNode::SafeDownCast(newNode);
  newSceneViewNode->StoreScene();
}

// --------------------------------------------------------------------------
// qDMMLCaptureToolBar methods

// --------------------------------------------------------------------------
qDMMLCaptureToolBar::qDMMLCaptureToolBar(const QString& title, QWidget* parentWidget)
  :Superclass(title, parentWidget)
   , d_ptr(new qDMMLCaptureToolBarPrivate(*this))
{
  Q_D(qDMMLCaptureToolBar);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLCaptureToolBar::qDMMLCaptureToolBar(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qDMMLCaptureToolBarPrivate(*this))
{
  Q_D(qDMMLCaptureToolBar);
  d->init();
}

//---------------------------------------------------------------------------
qDMMLCaptureToolBar::~qDMMLCaptureToolBar() = default;

// --------------------------------------------------------------------------
void qDMMLCaptureToolBar::setDMMLScene(vtkDMMLScene* scene)
{
  Q_D(qDMMLCaptureToolBar);
  d->setDMMLScene(scene);
}

// --------------------------------------------------------------------------
void qDMMLCaptureToolBar::setActiveDMMLThreeDViewNode(
  vtkDMMLViewNode * newActiveDMMLThreeDViewNode)
{
  Q_D(qDMMLCaptureToolBar);
  d->ActiveDMMLThreeDViewNode = newActiveDMMLThreeDViewNode;
  d->updateWidgetFromDMML();
}

// --------------------------------------------------------------------------
bool qDMMLCaptureToolBar::popupsTimeOut() const
{
  Q_D(const qDMMLCaptureToolBar);

  return d->timeOutFlag;
}

// --------------------------------------------------------------------------
void qDMMLCaptureToolBar::setPopupsTimeOut(bool flag)
{
  Q_D(qDMMLCaptureToolBar);

  d->timeOutFlag = flag;
}
