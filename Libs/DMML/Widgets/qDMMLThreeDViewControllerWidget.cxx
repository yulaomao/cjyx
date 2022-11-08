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
#include <QActionGroup>
#include <QDebug>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>

// CTK includes
#include <ctkButtonGroup.h>
#include <ctkPopupWidget.h>
#include <ctkSignalMapper.h>

// qDMML includes
#include "qDMMLColors.h"
#include "qDMMLNodeFactory.h"
#include "qDMMLSceneViewMenu.h"
#include "qDMMLThreeDView.h"
#include "qDMMLThreeDViewControllerWidget_p.h"

// DMML includes
#include <vtkDMMLCameraNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSceneViewNode.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkRenderWindow.h>

//--------------------------------------------------------------------------
// qDMMLThreeDViewControllerWidgetPrivate methods

//---------------------------------------------------------------------------
qDMMLThreeDViewControllerWidgetPrivate::qDMMLThreeDViewControllerWidgetPrivate(
  qDMMLThreeDViewControllerWidget& object)
  : Superclass(object)
  , CameraNode(nullptr)
  , ThreeDView(nullptr)
  , ViewLogic(nullptr)
  , StereoTypesMapper(nullptr)
  , AnimateViewButtonGroup(nullptr)
  , OrientationMarkerTypesMapper(nullptr)
  , OrientationMarkerSizesMapper(nullptr)
  , RulerTypesMapper(nullptr)
  , RulerColorMapper(nullptr)
  , CenterToolButton(nullptr)
{
}

//---------------------------------------------------------------------------
qDMMLThreeDViewControllerWidgetPrivate::~qDMMLThreeDViewControllerWidgetPrivate() = default;

//---------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidgetPrivate::setupPopupUi()
{
  Q_Q(qDMMLThreeDViewControllerWidget);

  this->Superclass::setupPopupUi();
  this->PopupWidget->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
  this->Ui_qDMMLThreeDViewControllerWidget::setupUi(this->PopupWidget);

  // Look from axes
  QObject::connect(this->AxesWidget,
                   SIGNAL(currentAxisChanged(ctkAxesWidget::Axis)),
                   q, SLOT(lookFromAxis(ctkAxesWidget::Axis)));

  // ViewLink button
  QObject::connect(this->ViewLinkButton, SIGNAL(toggled(bool)),
                   q, SLOT(setViewLink(bool)));

  // Orthographic/perspective button
  QObject::connect(this->OrthoButton, SIGNAL(toggled(bool)),
                   q, SLOT(setOrthographicModeEnabled(bool)));

  // ZoomIn, ZoomOut button
  QObject::connect(this->ZoomInButton, SIGNAL(clicked()),
                   q, SLOT(zoomIn()));
  QObject::connect(this->ZoomOutButton, SIGNAL(clicked()),
                   q, SLOT(zoomOut()));

  // ResetFocalPoint button
  this->CenterButton->setDefaultAction(this->actionCenter);
  QObject::connect(this->actionCenter, SIGNAL(triggered()),
                   q, SLOT(resetFocalPoint()));

  // StereoType actions
  this->StereoTypesMapper = new ctkSignalMapper(this->PopupWidget);
  this->StereoTypesMapper->setMapping(this->actionNoStereo,
                                      vtkDMMLViewNode::NoStereo);
  this->StereoTypesMapper->setMapping(this->actionSwitchToAnaglyphStereo,
                                      vtkDMMLViewNode::Anaglyph);
  this->StereoTypesMapper->setMapping(this->actionSwitchToQuadBufferStereo,
                                      vtkDMMLViewNode::QuadBuffer);
  this->StereoTypesMapper->setMapping(this->actionSwitchToInterlacedStereo,
                                      vtkDMMLViewNode::Interlaced);
  this->StereoTypesMapper->setMapping(this->actionSwitchToRedBlueStereo,
                                      vtkDMMLViewNode::RedBlue);
  this->StereoTypesMapper->setMapping(this->actionSwitchToUserDefinedStereo_1,
                                      vtkDMMLViewNode::UserDefined_1);
  this->StereoTypesMapper->setMapping(this->actionSwitchToUserDefinedStereo_2,
                                      vtkDMMLViewNode::UserDefined_2);
  this->StereoTypesMapper->setMapping(this->actionSwitchToUserDefinedStereo_3,
                                      vtkDMMLViewNode::UserDefined_3);
  QActionGroup* stereoTypesActions = new QActionGroup(this->PopupWidget);
  stereoTypesActions->setExclusive(true);
  stereoTypesActions->addAction(this->actionNoStereo);
  stereoTypesActions->addAction(this->actionSwitchToRedBlueStereo);
  stereoTypesActions->addAction(this->actionSwitchToAnaglyphStereo);
  stereoTypesActions->addAction(this->actionSwitchToInterlacedStereo);
  stereoTypesActions->addAction(this->actionSwitchToQuadBufferStereo);
  stereoTypesActions->addAction(this->actionSwitchToUserDefinedStereo_1);
  stereoTypesActions->addAction(this->actionSwitchToUserDefinedStereo_2);
  stereoTypesActions->addAction(this->actionSwitchToUserDefinedStereo_3);
  QMenu* stereoTypesMenu = new QMenu(qDMMLThreeDViewControllerWidget::tr("Stereo Modes"), this->PopupWidget);
  stereoTypesMenu->setObjectName("stereoTypesMenu");
  stereoTypesMenu->addActions(stereoTypesActions->actions());
  this->StereoButton->setMenu(stereoTypesMenu);
  QObject::connect(this->StereoTypesMapper, SIGNAL(mapped(int)),
                   q, SLOT(setStereoType(int)));
  QObject::connect(stereoTypesActions, SIGNAL(triggered(QAction*)),
                   this->StereoTypesMapper, SLOT(map(QAction*)));
  this->actionSwitchToQuadBufferStereo->setEnabled(false); // Disabled by default

  QMenu* visibilityMenu = new QMenu(qDMMLThreeDViewControllerWidget::tr("Visibility"), this->PopupWidget);
  visibilityMenu->setObjectName("visibilityMenu");
  this->VisibilityButton->setMenu(visibilityMenu);

  // Show 3D Axis, 3D Axis label
  visibilityMenu->addAction(this->actionSet3DAxisVisible);
  visibilityMenu->addAction(this->actionSet3DAxisLabelVisible);
  QObject::connect(this->actionSet3DAxisVisible, SIGNAL(triggered(bool)),
                   q, SLOT(set3DAxisVisible(bool)));
  QObject::connect(this->actionSet3DAxisLabelVisible, SIGNAL(triggered(bool)),
                   q, SLOT(set3DAxisLabelVisible(bool)));

  // OrientationMarker actions
  // Type
  this->OrientationMarkerTypesMapper = new ctkSignalMapper(this->PopupWidget);
  this->OrientationMarkerTypesMapper->setMapping(this->actionOrientationMarkerTypeNone, vtkDMMLAbstractViewNode::OrientationMarkerTypeNone);
  this->OrientationMarkerTypesMapper->setMapping(this->actionOrientationMarkerTypeCube, vtkDMMLAbstractViewNode::OrientationMarkerTypeCube);
  this->OrientationMarkerTypesMapper->setMapping(this->actionOrientationMarkerTypeHuman, vtkDMMLAbstractViewNode::OrientationMarkerTypeHuman);
  this->OrientationMarkerTypesMapper->setMapping(this->actionOrientationMarkerTypeAxes, vtkDMMLAbstractViewNode::OrientationMarkerTypeAxes);
  QActionGroup* orientationMarkerTypesActions = new QActionGroup(this->PopupWidget);
  orientationMarkerTypesActions->setExclusive(true);
  orientationMarkerTypesActions->addAction(this->actionOrientationMarkerTypeNone);
  orientationMarkerTypesActions->addAction(this->actionOrientationMarkerTypeCube);
  orientationMarkerTypesActions->addAction(this->actionOrientationMarkerTypeHuman);
  orientationMarkerTypesActions->addAction(this->actionOrientationMarkerTypeAxes);
  QObject::connect(this->OrientationMarkerTypesMapper, SIGNAL(mapped(int)),q, SLOT(setOrientationMarkerType(int)));
  QObject::connect(orientationMarkerTypesActions, SIGNAL(triggered(QAction*)),this->OrientationMarkerTypesMapper, SLOT(map(QAction*)));
  // Size
  this->OrientationMarkerSizesMapper = new ctkSignalMapper(this->PopupWidget);
  this->OrientationMarkerSizesMapper->setMapping(this->actionOrientationMarkerSizeSmall, vtkDMMLAbstractViewNode::OrientationMarkerSizeSmall);
  this->OrientationMarkerSizesMapper->setMapping(this->actionOrientationMarkerSizeMedium, vtkDMMLAbstractViewNode::OrientationMarkerSizeMedium);
  this->OrientationMarkerSizesMapper->setMapping(this->actionOrientationMarkerSizeLarge, vtkDMMLAbstractViewNode::OrientationMarkerSizeLarge);
  QActionGroup* orientationMarkerSizesActions = new QActionGroup(this->PopupWidget);
  orientationMarkerSizesActions->setExclusive(true);
  orientationMarkerSizesActions->addAction(this->actionOrientationMarkerSizeSmall);
  orientationMarkerSizesActions->addAction(this->actionOrientationMarkerSizeMedium);
  orientationMarkerSizesActions->addAction(this->actionOrientationMarkerSizeLarge);
  QObject::connect(this->OrientationMarkerSizesMapper, SIGNAL(mapped(int)),q, SLOT(setOrientationMarkerSize(int)));
  QObject::connect(orientationMarkerSizesActions, SIGNAL(triggered(QAction*)),this->OrientationMarkerSizesMapper, SLOT(map(QAction*)));
  // Menu
  QMenu* orientationMarkerMenu = new QMenu(qDMMLThreeDViewControllerWidget::tr("Orientation marker"), this->PopupWidget);
  orientationMarkerMenu->setObjectName("orientationMarkerMenu");
  this->OrientationMarkerButton->setMenu(orientationMarkerMenu);
  orientationMarkerMenu->addActions(orientationMarkerTypesActions->actions());
  orientationMarkerMenu->addSeparator();
  orientationMarkerMenu->addActions(orientationMarkerSizesActions->actions());

  // Ruler actions
  // Type
  this->RulerTypesMapper = new ctkSignalMapper(this->PopupWidget);
  this->RulerTypesMapper->setMapping(this->actionRulerTypeNone, vtkDMMLAbstractViewNode::RulerTypeNone);
  this->RulerTypesMapper->setMapping(this->actionRulerTypeThin, vtkDMMLAbstractViewNode::RulerTypeThin);
  this->RulerTypesMapper->setMapping(this->actionRulerTypeThick, vtkDMMLAbstractViewNode::RulerTypeThick);
  QActionGroup* rulerTypesActions = new QActionGroup(this->PopupWidget);
  rulerTypesActions->setExclusive(true);
  rulerTypesActions->addAction(this->actionRulerTypeNone);
  rulerTypesActions->addAction(this->actionRulerTypeThin);
  rulerTypesActions->addAction(this->actionRulerTypeThick);
  QObject::connect(this->RulerTypesMapper, SIGNAL(mapped(int)),q, SLOT(setRulerType(int)));
  QObject::connect(rulerTypesActions, SIGNAL(triggered(QAction*)),this->RulerTypesMapper, SLOT(map(QAction*)));
  // Color
  this->RulerColorMapper = new ctkSignalMapper(this->PopupWidget);
  this->RulerColorMapper->setMapping(this->actionRulerColorWhite, vtkDMMLAbstractViewNode::RulerColorWhite);
  this->RulerColorMapper->setMapping(this->actionRulerColorBlack, vtkDMMLAbstractViewNode::RulerColorBlack);
  this->RulerColorMapper->setMapping(this->actionRulerColorYellow, vtkDMMLAbstractViewNode::RulerColorYellow);
  QActionGroup* rulerColorActions = new QActionGroup(this->PopupWidget);
  rulerColorActions->setExclusive(true);
  rulerColorActions->addAction(this->actionRulerColorWhite);
  rulerColorActions->addAction(this->actionRulerColorBlack);
  rulerColorActions->addAction(this->actionRulerColorYellow);
  QObject::connect(this->RulerColorMapper, SIGNAL(mapped(int)),q, SLOT(setRulerColor(int)));
  QObject::connect(rulerColorActions, SIGNAL(triggered(QAction*)),this->RulerColorMapper, SLOT(map(QAction*)));

  // Menu
  QMenu* rulerMenu = new QMenu(qDMMLThreeDViewControllerWidget::tr("Ruler"), this->PopupWidget);
  rulerMenu->setObjectName("rulerMenu");
  this->RulerButton->setMenu(rulerMenu);
  rulerMenu->addActions(rulerTypesActions->actions());
  rulerMenu->addSeparator();
  rulerMenu->addActions(rulerColorActions->actions());

  // More controls
  QMenu* moreMenu = new QMenu(qDMMLThreeDViewControllerWidget::tr("More"), this->PopupWidget);
  moreMenu->addAction(this->actionUseDepthPeeling);
  moreMenu->addAction(this->actionSetFPSVisible);
  this->MoreToolButton->setMenu(moreMenu);

  // Depth peeling
  QObject::connect(this->actionUseDepthPeeling, SIGNAL(toggled(bool)),
                   q, SLOT(setUseDepthPeeling(bool)));

  // FPS
  QObject::connect(this->actionSetFPSVisible, SIGNAL(toggled(bool)),
                   q, SLOT(setFPSVisible(bool)));

  // Background color
  QActionGroup* backgroundColorActions = new QActionGroup(this->PopupWidget);
  backgroundColorActions->setExclusive(true);
  visibilityMenu->addSeparator();
  visibilityMenu->addAction(this->actionSetLightBlueBackground);
  visibilityMenu->addAction(this->actionSetBlackBackground);
  visibilityMenu->addAction(this->actionSetWhiteBackground);
  backgroundColorActions->addAction(this->actionSetLightBlueBackground);
  backgroundColorActions->addAction(this->actionSetBlackBackground);
  backgroundColorActions->addAction(this->actionSetWhiteBackground);
  QObject::connect(this->actionSetLightBlueBackground, SIGNAL(triggered()),
                   q, SLOT(setLightBlueBackground()));
  QObject::connect(this->actionSetWhiteBackground, SIGNAL(triggered()),
                   q, SLOT(setWhiteBackground()));
  QObject::connect(this->actionSetBlackBackground, SIGNAL(triggered()),
                   q, SLOT(setBlackBackground()));

  // SpinView, RockView buttons
  this->AnimateViewButtonGroup = new ctkButtonGroup(this->PopupWidget);
  this->AnimateViewButtonGroup->addButton(this->SpinButton, vtkDMMLViewNode::Spin);
  this->AnimateViewButtonGroup->addButton(this->RockButton, vtkDMMLViewNode::Rock);
  QObject::connect(this->SpinButton, SIGNAL(toggled(bool)),
                   q, SLOT(spinView(bool)));
  QObject::connect(this->RockButton, SIGNAL(toggled(bool)),
                   q, SLOT(rockView(bool)));
}

//---------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidgetPrivate::init()
{
  Q_Q(qDMMLThreeDViewControllerWidget);
  this->Superclass::init();

  this->CenterToolButton = new QToolButton(q);
  this->CenterToolButton->setAutoRaise(true);
  this->CenterToolButton->setDefaultAction(this->actionCenter);
  this->CenterToolButton->setFixedSize(15, 15);
  this->CenterToolButton->setObjectName("CenterButton_Header");
  this->BarLayout->insertWidget(2, this->CenterToolButton);

  this->ViewLabel->setText(qDMMLThreeDViewControllerWidget::tr("1"));
  this->BarLayout->addStretch(1);

  vtkNew<vtkDMMLViewLogic> defaultLogic;
  q->setViewLogic(defaultLogic.GetPointer());
}

// --------------------------------------------------------------------------
// qDMMLThreeDViewControllerWidget methods


// --------------------------------------------------------------------------
qDMMLThreeDViewControllerWidget::qDMMLThreeDViewControllerWidget(QWidget* parentWidget)
  : Superclass(new qDMMLThreeDViewControllerWidgetPrivate(*this), parentWidget)
{
  Q_D(qDMMLThreeDViewControllerWidget);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLThreeDViewControllerWidget::~qDMMLThreeDViewControllerWidget() = default;

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::setThreeDView(qDMMLThreeDView* view)
{
  Q_D(qDMMLThreeDViewControllerWidget);
  d->ThreeDView = view;
  if(d->ThreeDView != nullptr)
    {
    d->actionSwitchToQuadBufferStereo->setEnabled(
          d->ThreeDView->renderWindow()->GetStereoCapableWindow());
    // TODO: we could get layout name from the view node and keep it up-to-date using signal connection
    /*
    if (view->dmmlViewNode())
      {
      this->setDMMLViewNode(view->dmmlViewNode());
      }
    else if (this->dmml)
    */
    }
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::setDMMLViewNode(
    vtkDMMLAbstractViewNode * viewNode)
{
  Q_D(qDMMLThreeDViewControllerWidget);
  Superclass::setDMMLViewNode(viewNode);

  std::string layoutName;
  if (this->dmmlThreeDViewNode())
    {
    if (this->dmmlThreeDViewNode()->GetLayoutName())
      {
      layoutName = this->dmmlThreeDViewNode()->GetLayoutName();
      }
    else
      {
      qCritical() << "qDMMLThreeDViewControllerWidget::setDMMLViewNode failed: invalid layout name";
      }
    }
  d->CameraNode = d->ViewLogic->GetCameraNode(this->dmmlScene(), layoutName.c_str());
  this->qvtkReconnect(d->CameraNode, vtkDMMLCameraNode::CameraInteractionEvent,
                      this, SLOT(updateViewFromDMMLCamera()));

  this->updateViewFromDMMLCamera();
}

//---------------------------------------------------------------------------
vtkDMMLViewNode* qDMMLThreeDViewControllerWidget::dmmlThreeDViewNode()const
{
  Q_D(const qDMMLThreeDViewControllerWidget);
  return vtkDMMLViewNode::SafeDownCast(this->dmmlViewNode());
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::setViewLink(bool linked)
{
  if (!this->dmmlScene())
    {
    return;
    }

  vtkCollection* viewNodes = this->dmmlScene()->GetNodesByClass("vtkDMMLViewNode");
  if (!viewNodes)
    {
    return;
    }

  vtkDMMLViewNode* viewNode = nullptr;
  for(viewNodes->InitTraversal();
      (viewNode = vtkDMMLViewNode::SafeDownCast(
        viewNodes->GetNextItemAsObject()));)
    {
    viewNode->SetLinkedControl(linked);
    }
  viewNodes->Delete();
}

//---------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::setViewLabel(const QString& newViewLabel)
{
  Q_D(qDMMLThreeDViewControllerWidget);
  if (!this->dmmlThreeDViewNode())
    {
    qCritical() << Q_FUNC_INFO << " failed: must set view node first";
    return;
    }
  this->dmmlThreeDViewNode()->SetLayoutLabel(newViewLabel.toUtf8());
}

//---------------------------------------------------------------------------
QString qDMMLThreeDViewControllerWidget::viewLabel()const
{
  Q_D(const qDMMLThreeDViewControllerWidget);
  if (this->dmmlThreeDViewNode())
    {
    qCritical() << Q_FUNC_INFO << " failed: must set view node first";
    return QString();
    }
  return this->dmmlThreeDViewNode()->GetLayoutLabel();
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::updateWidgetFromDMMLViewLogic()
{
  Q_D(qDMMLThreeDViewControllerWidget);

  if (d->ViewLogic && d->ViewLogic->GetDMMLScene())
    {
    this->setDMMLScene(d->ViewLogic->GetDMMLScene());
    }

  // Update camera node connection
  vtkDMMLCameraNode* cameraNode = (d->ViewLogic ? d->ViewLogic->GetCameraNode() : nullptr);
  if (cameraNode != d->CameraNode)
    {
    this->qvtkReconnect(d->CameraNode, cameraNode, vtkDMMLCameraNode::CameraInteractionEvent,
      this, SLOT(updateViewFromDMMLCamera()));
    d->CameraNode = cameraNode;
    this->updateViewFromDMMLCamera();
    }

  // Update view node connection
  vtkDMMLViewNode* viewNode = (d->ViewLogic ? d->ViewLogic->GetViewNode() : nullptr);
  if (viewNode != this->dmmlThreeDViewNode())
    {
    this->setDMMLViewNode(viewNode);
    }
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::updateWidgetFromDMMLView()
{
  Q_D(qDMMLThreeDViewControllerWidget);
  Superclass::updateWidgetFromDMMLView();
  // Enable buttons
  vtkDMMLViewNode* viewNode = this->dmmlThreeDViewNode();
  QList<QWidget*> widgets;
  widgets << d->AxesWidget
    << d->CenterButton << d->OrthoButton << d->VisibilityButton
    << d->ZoomInButton << d->ZoomOutButton << d->StereoButton
    << d->RockButton << d->SpinButton << d->MoreToolButton
    << d->OrientationMarkerButton; // RulerButton enable state is not set here (it depends on render mode)
  foreach(QWidget* w, widgets)
    {
    w->setEnabled(viewNode != nullptr);
    }

  if (!viewNode)
    {
    return;
    }

  // In the axes widget the order of labels is: +X, -X, +Z, -Z, +Y, -Y
  // and in the view node axis labels order is: -X, +X, -Y, +Y, -Z, +Z.
  QStringList axesLabels;
  axesLabels << viewNode->GetAxisLabel(1); // +X
  axesLabels << viewNode->GetAxisLabel(0); // -X
  axesLabels << viewNode->GetAxisLabel(5); // +Z
  axesLabels << viewNode->GetAxisLabel(4); // -Z
  axesLabels << viewNode->GetAxisLabel(3); // +Y
  axesLabels << viewNode->GetAxisLabel(2); // -Y
  d->AxesWidget->setAxesLabels(axesLabels);

  // Update view link toggle. Must be done first as its state controls
  // different behaviors when properties are set.
  d->ViewLinkButton->setChecked(viewNode->GetLinkedControl());
  if (viewNode->GetLinkedControl())
    {
    d->ViewLinkButton->setIcon(QIcon(":Icons/LinkOn.png"));
    }
  else
    {
    d->ViewLinkButton->setIcon(QIcon(":Icons/LinkOff.png"));
    }

  d->actionSet3DAxisVisible->setChecked(viewNode->GetBoxVisible());
  d->actionSet3DAxisLabelVisible->setChecked(
    viewNode->GetAxisLabelsVisible());

  d->actionUseDepthPeeling->setChecked(viewNode->GetUseDepthPeeling());
  d->actionSetFPSVisible->setChecked(viewNode->GetFPSVisible());

  double* color = viewNode->GetBackgroundColor();
  QColor backgroundColor = QColor::fromRgbF(color[0], color[1], color[2]);
  d->actionSetBlackBackground->setChecked(backgroundColor == Qt::black);
  d->actionSetWhiteBackground->setChecked(backgroundColor == Qt::white);
  d->actionSetLightBlueBackground->setChecked(
    !d->actionSetBlackBackground->isChecked() &&
    !d->actionSetWhiteBackground->isChecked());

  d->OrthoButton->setChecked(viewNode->GetRenderMode() == vtkDMMLViewNode::Orthographic);

  QAction* action = qobject_cast<QAction*>(d->StereoTypesMapper->mapping(viewNode->GetStereoType()));
  if (action)
    {
    action->setChecked(true);
    }
  action = qobject_cast<QAction*>(d->OrientationMarkerTypesMapper->mapping(viewNode->GetOrientationMarkerType()));
  if (action)
    {
    action->setChecked(true);
    }
  action = qobject_cast<QAction*>(d->OrientationMarkerSizesMapper->mapping(viewNode->GetOrientationMarkerSize()));
  if (action)
    {
    action->setChecked(true);
    }
  action = qobject_cast<QAction*>(d->RulerTypesMapper->mapping(viewNode->GetRulerType()));
  if (action)
    {
    action->setChecked(true);
    }
  d->RulerButton->setEnabled(viewNode->GetRenderMode()==vtkDMMLViewNode::Orthographic);

  d->SpinButton->setChecked(viewNode->GetAnimationMode() == vtkDMMLViewNode::Spin);
  d->RockButton->setChecked(viewNode->GetAnimationMode() == vtkDMMLViewNode::Rock);

  d->ViewLabel->setText(viewNode->GetLayoutLabel());

  double* layoutColorVtk = viewNode->GetLayoutColor();
  QColor layoutColor = QColor::fromRgbF(layoutColorVtk[0], layoutColorVtk[1], layoutColorVtk[2]);
  d->setColor(layoutColor);
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::updateViewFromDMMLCamera()
{
  Q_D(qDMMLThreeDViewControllerWidget);
  if (d->CameraNode)
    {
    d->ViewLogic->StartCameraNodeInteraction(vtkDMMLCameraNode::CameraInteractionFlag);
    d->CameraNode->Modified();
    d->ViewLogic->EndCameraNodeInteraction();
    }
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::setQuadBufferStereoSupportEnabled(bool value)
{
  Q_D(qDMMLThreeDViewControllerWidget);
  d->actionSwitchToQuadBufferStereo->setEnabled(value);
}

// --------------------------------------------------------------------------
vtkDMMLViewLogic* qDMMLThreeDViewControllerWidget::viewLogic() const
{
  Q_D(const qDMMLThreeDViewControllerWidget);
  return d->ViewLogic;
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::setViewLogic(vtkDMMLViewLogic* newViewLogic)
{
  Q_D(qDMMLThreeDViewControllerWidget);
  if (d->ViewLogic == newViewLogic)
    {
    return;
    }

  this->qvtkReconnect(d->ViewLogic, newViewLogic, vtkCommand::ModifiedEvent,
    this, SLOT(updateWidgetFromDMMLViewLogic()));

  d->ViewLogic = newViewLogic;

  this->updateWidgetFromDMMLViewLogic();
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::setDMMLScene(vtkDMMLScene* newScene)
{
  Q_D(qDMMLThreeDViewControllerWidget);

  if (this->dmmlScene() == newScene)
    {
    return;
    }

  this->qvtkReconnect(this->dmmlScene(), newScene, vtkDMMLScene::EndBatchProcessEvent,
                      this, SLOT(updateWidgetFromDMMLView()));

   d->ViewLogic->SetDMMLScene(newScene);

  this->Superclass::setDMMLScene(newScene);

  if (this->dmmlScene())
   {
   this->updateWidgetFromDMMLView();
   }
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::setOrthographicModeEnabled(bool enabled)
{
  Q_D(qDMMLThreeDViewControllerWidget);

  if (!d->ViewLogic)
    {
    return;
    }

  d->ViewLogic->StartViewNodeInteraction(vtkDMMLViewNode::RenderModeFlag);
  this->dmmlThreeDViewNode()->SetRenderMode(
    enabled ? vtkDMMLViewNode::Orthographic : vtkDMMLViewNode::Perspective);
  d->ViewLogic->EndViewNodeInteraction();
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::lookFromAxis(const ctkAxesWidget::Axis& axis)
{
  Q_D(qDMMLThreeDViewControllerWidget);
  if (!d->ThreeDView)
    {
    return;
    }

  d->ViewLogic->StartCameraNodeInteraction(vtkDMMLCameraNode::LookFromAxis);
  d->ThreeDView->lookFromViewAxis(axis);
  d->ViewLogic->EndCameraNodeInteraction();
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::pitchView()
{
  Q_D(qDMMLThreeDViewControllerWidget);
  if (!d->ThreeDView)
    {
    return;
    }
  d->ThreeDView->pitch();
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::rollView()
{
  Q_D(qDMMLThreeDViewControllerWidget);
  if (!d->ThreeDView)
    {
    return;
    }
  d->ThreeDView->roll();
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::yawView()
{
  Q_D(qDMMLThreeDViewControllerWidget);
  if (!d->ThreeDView)
    {
    return;
    }
  d->ThreeDView->yaw();
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::zoomIn()
{
  Q_D(qDMMLThreeDViewControllerWidget);
  if (!this->dmmlThreeDViewNode())
    {
    return;
    }

  d->ViewLogic->StartCameraNodeInteraction(vtkDMMLCameraNode::ZoomInFlag);
  d->ThreeDView->zoomIn();
  d->ViewLogic->EndCameraNodeInteraction();
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::zoomOut()
{
  Q_D(qDMMLThreeDViewControllerWidget);
  if (!this->dmmlThreeDViewNode())
    {
    return;
    }

  d->ViewLogic->StartCameraNodeInteraction(vtkDMMLCameraNode::ZoomOutFlag);
  d->ThreeDView->zoomOut();
  d->ViewLogic->EndCameraNodeInteraction();
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::spinView(bool enabled)
{
  this->setAnimationMode(enabled ? vtkDMMLViewNode::Spin : vtkDMMLViewNode::Off);
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::rockView(bool enabled)
{
  this->setAnimationMode(enabled ? vtkDMMLViewNode::Rock : vtkDMMLViewNode::Off);
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::setAnimationMode(int newAnimationMode)
{
  Q_D(qDMMLThreeDViewControllerWidget);
  if (!this->dmmlThreeDViewNode())
    {
    return;
    }

  d->ViewLogic->StartViewNodeInteraction(vtkDMMLViewNode::AnimationModeFlag);
  this->dmmlThreeDViewNode()->SetAnimationMode(newAnimationMode);
  d->ViewLogic->EndViewNodeInteraction();
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::resetFocalPoint()
{
  Q_D(qDMMLThreeDViewControllerWidget);
  if (!d->ThreeDView)
    {
    return;
    }

  d->ViewLogic->StartCameraNodeInteraction(vtkDMMLCameraNode::CenterFlag);
  d->ThreeDView->resetFocalPoint();
  d->ViewLogic->EndCameraNodeInteraction();
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::set3DAxisVisible(bool visible)
{
  Q_D(qDMMLThreeDViewControllerWidget);
  if (!this->dmmlThreeDViewNode())
    {
    return;
    }

  d->ViewLogic->StartViewNodeInteraction(vtkDMMLViewNode::BoxVisibleFlag);
  this->dmmlThreeDViewNode()->SetBoxVisible(visible);
  d->ViewLogic->EndViewNodeInteraction();
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::set3DAxisLabelVisible(bool visible)
{
  Q_D(qDMMLThreeDViewControllerWidget);
  if (!this->dmmlThreeDViewNode())
    {
    return;
    }

  d->ViewLogic->StartViewNodeInteraction(vtkDMMLViewNode::BoxLabelVisibileFlag);
  this->dmmlThreeDViewNode()->SetAxisLabelsVisible(visible);
  d->ViewLogic->EndViewNodeInteraction();
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::setUseDepthPeeling(bool use)
{
  Q_D(qDMMLThreeDViewControllerWidget);
  if (!this->dmmlThreeDViewNode())
    {
    return;
    }

  d->ViewLogic->StartViewNodeInteraction(vtkDMMLViewNode::UseDepthPeelingFlag);
  this->dmmlThreeDViewNode()->SetUseDepthPeeling(use ? 1 : 0);
  d->ViewLogic->EndViewNodeInteraction();
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::setFPSVisible(bool visible)
{
  Q_D(qDMMLThreeDViewControllerWidget);
  if (!this->dmmlThreeDViewNode())
    {
    return;
    }

  d->ViewLogic->StartViewNodeInteraction(vtkDMMLViewNode::FPSVisibleFlag);
  this->dmmlThreeDViewNode()->SetFPSVisible(visible ? 1 : 0);
  d->ViewLogic->EndViewNodeInteraction();
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::setLightBlueBackground()
{
  this->setBackgroundColor(QColor::fromRgbF(
    vtkDMMLViewNode::defaultBackgroundColor()[0],
    vtkDMMLViewNode::defaultBackgroundColor()[1],
    vtkDMMLViewNode::defaultBackgroundColor()[2]),
    QColor::fromRgbF(
    vtkDMMLViewNode::defaultBackgroundColor2()[0],
    vtkDMMLViewNode::defaultBackgroundColor2()[1],
    vtkDMMLViewNode::defaultBackgroundColor2()[2]));
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::setBlackBackground()
{
  this->setBackgroundColor(Qt::black);
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::setWhiteBackground()
{
  this->setBackgroundColor(Qt::white);
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::setBackgroundColor(
  const QColor& newColor, QColor newColor2)
{
  Q_D(qDMMLThreeDViewControllerWidget);
  if (!this->dmmlThreeDViewNode())
    {
    return;
    }

  d->ViewLogic->StartViewNodeInteraction(vtkDMMLViewNode::BackgroundColorFlag);

  int wasModifying = this->dmmlThreeDViewNode()->StartModify();
  // The ThreeDView displayable manager will change the background color of
  // the renderer.
  this->dmmlThreeDViewNode()->SetBackgroundColor(newColor.redF(), newColor.greenF(), newColor.blueF());
  if (!newColor2.isValid())
    {
    newColor2 = newColor;
    }
  this->dmmlThreeDViewNode()->SetBackgroundColor2(newColor2.redF(), newColor2.greenF(), newColor2.blueF());
  this->dmmlThreeDViewNode()->EndModify(wasModifying);

  d->ViewLogic->EndViewNodeInteraction();
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::setStereoType(int newStereoType)
{
  Q_D(qDMMLThreeDViewControllerWidget);
  if (!this->dmmlThreeDViewNode())
    {
    return;
    }

  d->ViewLogic->StartViewNodeInteraction(vtkDMMLViewNode::StereoTypeFlag);
  this->dmmlThreeDViewNode()->SetStereoType(newStereoType);
  d->ViewLogic->EndViewNodeInteraction();
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::setOrientationMarkerType(int newOrientationMarkerType)
{
  Q_D(qDMMLThreeDViewControllerWidget);
  if (!this->dmmlThreeDViewNode())
    {
    return;
    }

  d->ViewLogic->StartViewNodeInteraction(vtkDMMLViewNode::OrientationMarkerTypeFlag);
  this->dmmlThreeDViewNode()->SetOrientationMarkerType(newOrientationMarkerType);
  d->ViewLogic->EndViewNodeInteraction();
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::setOrientationMarkerSize(int newOrientationMarkerSize)
{
  Q_D(qDMMLThreeDViewControllerWidget);
  if (!this->dmmlThreeDViewNode())
    {
    return;
    }

  d->ViewLogic->StartViewNodeInteraction(vtkDMMLViewNode::OrientationMarkerSizeFlag);
  this->dmmlThreeDViewNode()->SetOrientationMarkerSize(newOrientationMarkerSize);
  d->ViewLogic->EndViewNodeInteraction();
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::setRulerType(int newRulerType)
{
  Q_D(qDMMLThreeDViewControllerWidget);
  if (!this->dmmlThreeDViewNode())
    {
    return;
    }

  d->ViewLogic->StartViewNodeInteraction(vtkDMMLViewNode::RulerTypeFlag);
  this->dmmlThreeDViewNode()->SetRulerType(newRulerType);
  d->ViewLogic->EndViewNodeInteraction();

  // Switch to orthographic render mode automatically if ruler is enabled
  if (newRulerType!=vtkDMMLViewNode::RulerTypeNone &&
    this->dmmlThreeDViewNode()->GetRenderMode()!=vtkDMMLViewNode::Orthographic)
    {
    d->ViewLogic->StartViewNodeInteraction(vtkDMMLViewNode::RenderModeFlag);
    this->dmmlThreeDViewNode()->SetRenderMode(vtkDMMLViewNode::Orthographic);
    d->ViewLogic->EndViewNodeInteraction();
    }
}

// --------------------------------------------------------------------------
void qDMMLThreeDViewControllerWidget::setRulerColor(int newRulerColor)
{
  Q_D(qDMMLThreeDViewControllerWidget);
  if (!this->dmmlThreeDViewNode())
    {
    return;
    }

  d->ViewLogic->StartViewNodeInteraction(vtkDMMLViewNode::RulerColorFlag);
  this->dmmlThreeDViewNode()->SetRulerColor(newRulerColor);
  d->ViewLogic->EndViewNodeInteraction();
}
