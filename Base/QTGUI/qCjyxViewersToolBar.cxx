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
#include <QToolButton>

// Cjyx includes
#include "qCjyxApplication.h"
#include "qCjyxLayoutManager.h"
#include "qDMMLSliceWidget.h"
#include "qCjyxViewersToolBar_p.h"

// CjyxLogic includes
#include <vtkCjyxApplicationLogic.h>

// DMML includes
#include <vtkDMMLCrosshairNode.h>
#include "vtkDMMLSliceLogic.h"
#include <vtkDMMLModelNode.h>
#include <vtkDMMLSliceDisplayNode.h>

//---------------------------------------------------------------------------
// qCjyxViewersToolBarPrivate methods

//---------------------------------------------------------------------------
qCjyxViewersToolBarPrivate::qCjyxViewersToolBarPrivate(qCjyxViewersToolBar& object)
  : q_ptr(&object)
{
}

//---------------------------------------------------------------------------
void qCjyxViewersToolBarPrivate::init()
{
  Q_Q(qCjyxViewersToolBar);

  /// Crosshair
  ///

  // Style
  QActionGroup* crosshairJumpSlicesActions = new QActionGroup(q);
  crosshairJumpSlicesActions->setExclusive(true);

  this->CrosshairJumpSlicesDisabledAction = new QAction(q);
  this->CrosshairJumpSlicesDisabledAction->setText(tr("No jump slices"));
  this->CrosshairJumpSlicesDisabledAction->setToolTip(tr("Slice views are not repositioned when crosshair is moved."));
  this->CrosshairJumpSlicesDisabledAction->setCheckable(true);

  this->CrosshairJumpSlicesOffsetAction = new QAction(q);
  this->CrosshairJumpSlicesOffsetAction->setText(tr("Jump slices - offset"));
  this->CrosshairJumpSlicesOffsetAction->setToolTip(tr("Slice view planes are shifted to match crosshair position (even if crosshair is not displayed)."));
  this->CrosshairJumpSlicesOffsetAction->setCheckable(true);

  this->CrosshairJumpSlicesCenteredAction = new QAction(q);
  this->CrosshairJumpSlicesCenteredAction->setText(tr("Jump slices - centered"));
  this->CrosshairJumpSlicesCenteredAction->setToolTip(tr("Slice views are centered on crosshair position (even if crosshair is not displayed)."));
  this->CrosshairJumpSlicesCenteredAction->setCheckable(true);

  crosshairJumpSlicesActions->addAction(this->CrosshairJumpSlicesDisabledAction);
  crosshairJumpSlicesActions->addAction(this->CrosshairJumpSlicesOffsetAction);
  crosshairJumpSlicesActions->addAction(this->CrosshairJumpSlicesCenteredAction);

  this->CrosshairJumpSlicesMapper = new ctkSignalMapper(q);
  this->CrosshairJumpSlicesMapper->setMapping(this->CrosshairJumpSlicesDisabledAction, vtkDMMLCrosshairNode::NoAction);
  this->CrosshairJumpSlicesMapper->setMapping(this->CrosshairJumpSlicesOffsetAction, vtkDMMLCrosshairNode::OffsetJumpSlice);
  this->CrosshairJumpSlicesMapper->setMapping(this->CrosshairJumpSlicesCenteredAction, vtkDMMLCrosshairNode::CenteredJumpSlice);
  QObject::connect(crosshairJumpSlicesActions, SIGNAL(triggered(QAction*)), this->CrosshairJumpSlicesMapper, SLOT(map(QAction*)));
  QObject::connect(this->CrosshairJumpSlicesMapper, SIGNAL(mapped(int)), this, SLOT(setCrosshairJumpSlicesMode(int)));

  // Crosshair Style
  QActionGroup* crosshairActions = new QActionGroup(q);
  crosshairActions->setExclusive(true);

  this->CrosshairNoAction = new QAction(q);
  this->CrosshairNoAction->setText(tr("No crosshair"));
  this->CrosshairNoAction->setToolTip(tr("No crosshair displayed."));
  this->CrosshairNoAction->setCheckable(true);

  this->CrosshairBasicAction = new QAction(q);
  this->CrosshairBasicAction->setText(tr("Basic crosshair"));
  this->CrosshairBasicAction->setToolTip(tr("Basic crosshair extending across the field of view with a small gap at the crosshair position."));
  this->CrosshairBasicAction->setCheckable(true);

  this->CrosshairBasicIntersectionAction = new QAction(q);
  this->CrosshairBasicIntersectionAction->setText(tr("Basic + intersection"));
  this->CrosshairBasicIntersectionAction->setToolTip(tr("Basic crosshair extending across the field of view."));
  this->CrosshairBasicIntersectionAction->setCheckable(true);

  this->CrosshairSmallBasicAction = new QAction(q);
  this->CrosshairSmallBasicAction->setText(tr("Small basic crosshair"));
  this->CrosshairSmallBasicAction->setToolTip(tr("Small crosshair with a small gap at the crosshair position."));
  this->CrosshairSmallBasicAction->setCheckable(true);

  this->CrosshairSmallBasicIntersectionAction = new QAction(q);
  this->CrosshairSmallBasicIntersectionAction->setText(tr("Small basic + intersection"));
  this->CrosshairSmallBasicIntersectionAction->setToolTip(tr("Small crosshair."));
  this->CrosshairSmallBasicIntersectionAction->setCheckable(true);

  crosshairActions->addAction(this->CrosshairNoAction);
  crosshairActions->addAction(this->CrosshairBasicAction);
  crosshairActions->addAction(this->CrosshairBasicIntersectionAction);
  crosshairActions->addAction(this->CrosshairSmallBasicAction);
  crosshairActions->addAction(this->CrosshairSmallBasicIntersectionAction);

  this->CrosshairMapper = new ctkSignalMapper(q);
  this->CrosshairMapper->setMapping(this->CrosshairNoAction,
                                    vtkDMMLCrosshairNode::NoCrosshair);
  this->CrosshairMapper->setMapping(this->CrosshairBasicAction,
                                    vtkDMMLCrosshairNode::ShowBasic);
  this->CrosshairMapper->setMapping(this->CrosshairBasicIntersectionAction,
                                    vtkDMMLCrosshairNode::ShowIntersection);
  this->CrosshairMapper->setMapping(this->CrosshairSmallBasicAction,
                                    vtkDMMLCrosshairNode::ShowSmallBasic);
  this->CrosshairMapper->setMapping(this->CrosshairSmallBasicIntersectionAction,
      vtkDMMLCrosshairNode::ShowSmallIntersection);
  QObject::connect(crosshairActions, SIGNAL(triggered(QAction*)),
                   this->CrosshairMapper, SLOT(map(QAction*)));
  QObject::connect(this->CrosshairMapper, SIGNAL(mapped(int)),
                   this, SLOT(setCrosshairMode(int)));

  // Crosshair Thickness
  QActionGroup* crosshairThicknessActions = new QActionGroup(q);
  crosshairThicknessActions->setExclusive(true);

  this->CrosshairFineAction = new QAction(q);
  this->CrosshairFineAction->setText(tr("Fine crosshair"));
  this->CrosshairFineAction->setToolTip(tr("Fine crosshair."));
  this->CrosshairFineAction->setCheckable(true);

  this->CrosshairMediumAction = new QAction(q);
  this->CrosshairMediumAction->setText(tr("Medium crosshair"));
  this->CrosshairMediumAction->setToolTip(tr("Medium crosshair."));
  this->CrosshairMediumAction->setCheckable(true);

  this->CrosshairThickAction = new QAction(q);
  this->CrosshairThickAction->setText(tr("Thick crosshair"));
  this->CrosshairThickAction->setToolTip(tr("Thick crosshair."));
  this->CrosshairThickAction->setCheckable(true);

  crosshairThicknessActions->addAction(this->CrosshairFineAction);
  crosshairThicknessActions->addAction(this->CrosshairMediumAction);
  crosshairThicknessActions->addAction(this->CrosshairThickAction);
  this->CrosshairThicknessMapper = new ctkSignalMapper(q);
  this->CrosshairThicknessMapper->setMapping(this->CrosshairFineAction,
                                             vtkDMMLCrosshairNode::Fine);
  this->CrosshairThicknessMapper->setMapping(this->CrosshairMediumAction,
                                             vtkDMMLCrosshairNode::Medium);
  this->CrosshairThicknessMapper->setMapping(this->CrosshairThickAction,
                                             vtkDMMLCrosshairNode::Thick);
  QObject::connect(crosshairThicknessActions, SIGNAL(triggered(QAction*)),
                   this->CrosshairThicknessMapper, SLOT(map(QAction*)));
  QObject::connect(this->CrosshairThicknessMapper, SIGNAL(mapped(int)),
                   this, SLOT(setCrosshairThickness(int)));

  // Crosshair Menu
  this->CrosshairMenu = new QMenu(tr("Crosshair"), q);
  this->CrosshairMenu->addActions(crosshairJumpSlicesActions->actions());
  this->CrosshairMenu->addSeparator();
  this->CrosshairMenu->addActions(crosshairActions->actions());
  this->CrosshairMenu->addSeparator();
  this->CrosshairMenu->addActions(crosshairThicknessActions->actions());

  // Crosshair ToolButton
  this->CrosshairToolButton = new QToolButton();
  this->CrosshairToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  this->CrosshairToolButton->setToolTip(tr("Crosshair"));
  this->CrosshairToolButton->setText(tr("Crosshair"));
  this->CrosshairToolButton->setMenu(this->CrosshairMenu);
  this->CrosshairToolButton->setPopupMode(QToolButton::MenuButtonPopup);

  // Default Crosshair action
  this->CrosshairToggleAction = new QAction(q);
  this->CrosshairToggleAction->setIcon(QIcon(":/Icons/SlicesCrosshair.png"));
  this->CrosshairToggleAction->setCheckable(true);
  this->CrosshairToggleAction->setToolTip(tr(
    "Toggle crosshair visibility. Hold Shift key and move mouse in a view to set crosshair position."));
  this->CrosshairToggleAction->setText(tr("Crosshair"));
  this->CrosshairToolButton->setDefaultAction(this->CrosshairToggleAction);
  QObject::connect(this->CrosshairToggleAction, SIGNAL(toggled(bool)),
                   this, SLOT(setCrosshairEnabled(bool)));

  q->addWidget(this->CrosshairToolButton);
  QObject::connect(q, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
                   this->CrosshairToolButton,
                   SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));


  // Slice Intersections Style
  QActionGroup* sliceIntersectionsActions = new QActionGroup(q);
  sliceIntersectionsActions->setExclusive(true);

  this->SliceIntersectionsFullIntersectionAction = new QAction(q);
  this->SliceIntersectionsFullIntersectionAction->setText(tr("Full lines"));
  this->SliceIntersectionsFullIntersectionAction->setToolTip(tr("Full slice intersection lines extending across the field of view."));
  this->SliceIntersectionsFullIntersectionAction->setCheckable(true);
  this->SliceIntersectionsFullIntersectionAction->setChecked(true);

  this->SliceIntersectionsSkipIntersectionAction = new QAction(q);
  this->SliceIntersectionsSkipIntersectionAction->setText(tr("Skip line crossings"));
  this->SliceIntersectionsSkipIntersectionAction->setToolTip(tr("Slice intersection lines extending across the field of view with a gap at the intersection."));
  this->SliceIntersectionsSkipIntersectionAction->setCheckable(true);

  sliceIntersectionsActions->addAction(this->SliceIntersectionsFullIntersectionAction);
  sliceIntersectionsActions->addAction(this->SliceIntersectionsSkipIntersectionAction);

  this->SliceIntersectionsMapper = new ctkSignalMapper(q);
  this->SliceIntersectionsMapper->setMapping(this->SliceIntersectionsSkipIntersectionAction,
                                    vtkDMMLSliceDisplayNode::SkipLineCrossings);
  this->SliceIntersectionsMapper->setMapping(this->SliceIntersectionsFullIntersectionAction,
                                    vtkDMMLSliceDisplayNode::FullLines);
  QObject::connect(sliceIntersectionsActions, SIGNAL(triggered(QAction*)),
                   this->SliceIntersectionsMapper, SLOT(map(QAction*)));
  QObject::connect(this->SliceIntersectionsMapper, SIGNAL(mapped(int)),
                   this, SLOT(setIntersectingSlicesIntersectionMode(int)));

  // Slice Intersections Thickness
  QActionGroup* sliceIntersectionsThicknessActions = new QActionGroup(q);
  sliceIntersectionsThicknessActions->setExclusive(true);

  this->SliceIntersectionsFineAction = new QAction(q);
  this->SliceIntersectionsFineAction->setText(tr("Fine lines"));
  this->SliceIntersectionsFineAction->setToolTip(tr("Fine lines."));
  this->SliceIntersectionsFineAction->setCheckable(true);
  this->SliceIntersectionsFineAction->setChecked(true);

  this->SliceIntersectionsMediumAction = new QAction(q);
  this->SliceIntersectionsMediumAction->setText(tr("Medium lines"));
  this->SliceIntersectionsMediumAction->setToolTip(tr("Medium lines."));
  this->SliceIntersectionsMediumAction->setCheckable(true);

  this->SliceIntersectionsThickAction = new QAction(q);
  this->SliceIntersectionsThickAction->setText(tr("Thick lines"));
  this->SliceIntersectionsThickAction->setToolTip(tr("Thick lines."));
  this->SliceIntersectionsThickAction->setCheckable(true);

  sliceIntersectionsThicknessActions->addAction(this->SliceIntersectionsFineAction);
  sliceIntersectionsThicknessActions->addAction(this->SliceIntersectionsMediumAction);
  sliceIntersectionsThicknessActions->addAction(this->SliceIntersectionsThickAction);
  this->SliceIntersectionsThicknessMapper = new ctkSignalMapper(q);
  this->SliceIntersectionsThicknessMapper->setMapping(this->SliceIntersectionsFineAction,
                                             vtkDMMLSliceDisplayNode::FineLines);
  this->SliceIntersectionsThicknessMapper->setMapping(this->SliceIntersectionsMediumAction,
                                             vtkDMMLSliceDisplayNode::MediumLines);
  this->SliceIntersectionsThicknessMapper->setMapping(this->SliceIntersectionsThickAction,
                                             vtkDMMLSliceDisplayNode::ThickLines);
  QObject::connect(sliceIntersectionsThicknessActions, SIGNAL(triggered(QAction*)),
                   this->SliceIntersectionsThicknessMapper, SLOT(map(QAction*)));
  QObject::connect(this->SliceIntersectionsThicknessMapper, SIGNAL(mapped(int)),
                   this, SLOT(setIntersectingSlicesLineThicknessMode(int)));

  // Interactive slice intersections
  this->IntersectingSlicesInteractiveAction = new QAction(q);
  this->IntersectingSlicesInteractiveAction->setText(tr("Interaction"));
  this->IntersectingSlicesInteractiveAction->setToolTip(tr("Show handles for slice interaction."));
  this->IntersectingSlicesInteractiveAction->setCheckable(true);
  QObject::connect(this->IntersectingSlicesInteractiveAction, SIGNAL(triggered(bool)),
    this, SLOT(setIntersectingSlicesInteractive(bool)));

  // Interaction options
  this->IntersectingSlicesTranslationEnabledAction = new QAction(q);
  this->IntersectingSlicesTranslationEnabledAction->setText(tr("Translate"));
  this->IntersectingSlicesTranslationEnabledAction->setToolTip(tr("Control visibility of translation handles for slice intersection."));
  this->IntersectingSlicesTranslationEnabledAction->setCheckable(true);
  QObject::connect(this->IntersectingSlicesTranslationEnabledAction, SIGNAL(triggered(bool)),
    this, SLOT(setIntersectingSlicesTranslationEnabled(bool)));

  this->IntersectingSlicesRotationEnabledAction = new QAction(q);
  this->IntersectingSlicesRotationEnabledAction->setText(tr("Rotate"));
  this->IntersectingSlicesRotationEnabledAction->setToolTip(tr("Control visibility of rotation handles for slice intersection."));
  this->IntersectingSlicesRotationEnabledAction->setCheckable(true);
  QObject::connect(this->IntersectingSlicesRotationEnabledAction, SIGNAL(triggered(bool)),
    this, SLOT(setIntersectingSlicesRotationEnabled(bool)));

  this->IntersectingSlicesInteractionModesMenu = new QMenu();
  this->IntersectingSlicesInteractionModesMenu->setTitle(tr("Interaction options"));
  this->IntersectingSlicesInteractionModesMenu->addAction(this->IntersectingSlicesTranslationEnabledAction);
  this->IntersectingSlicesInteractionModesMenu->addAction(this->IntersectingSlicesRotationEnabledAction);

  // Slice Intersections Menu
  this->SliceIntersectionsMenu = new QMenu(tr("Slice intersections"), q);
  this->SliceIntersectionsMenu->addAction(this->IntersectingSlicesInteractiveAction);
  this->SliceIntersectionsMenu->addMenu(this->IntersectingSlicesInteractionModesMenu);
  this->SliceIntersectionsMenu->addSeparator();
  this->SliceIntersectionsMenu->addActions(sliceIntersectionsActions->actions());
  this->SliceIntersectionsMenu->addSeparator();
  this->SliceIntersectionsMenu->addActions(sliceIntersectionsThicknessActions->actions());

  // Add connection to update slice intersection checkboxes before showing the dropdown menu
  QObject::connect(this->SliceIntersectionsMenu, SIGNAL(aboutToShow()),
    this, SLOT(updateWidgetFromDMML()));

  // Slice Intersections ToolButton
  this->SliceIntersectionsToolButton = new QToolButton();
  this->SliceIntersectionsToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  this->SliceIntersectionsToolButton->setToolTip(tr("Slice intersections"));
  this->SliceIntersectionsToolButton->setText(tr("Slice intersections"));
  this->SliceIntersectionsToolButton->setMenu(this->SliceIntersectionsMenu);
  this->SliceIntersectionsToolButton->setPopupMode(QToolButton::MenuButtonPopup);

  // Default Slice intersections action
  this->IntersectingSlicesVisibleAction = new QAction(q);
  this->IntersectingSlicesVisibleAction->setIcon(QIcon(":/Icons/SliceIntersections.png"));
  this->IntersectingSlicesVisibleAction->setText(tr("Slice intersections"));
  this->IntersectingSlicesVisibleAction->setToolTip(tr("Show how the other slice planes intersect each slice plane."));
  this->IntersectingSlicesVisibleAction->setCheckable(true);
  this->SliceIntersectionsToolButton->setDefaultAction(this->IntersectingSlicesVisibleAction);
  QObject::connect(this->IntersectingSlicesVisibleAction, SIGNAL(triggered(bool)),
                   this, SLOT(setIntersectingSlicesVisibility(bool)));

  q->addWidget(this->SliceIntersectionsToolButton);
  QObject::connect(q, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
                   this->SliceIntersectionsToolButton,
                   SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));

  /// Other controls
  ///
}

//---------------------------------------------------------------------------
void qCjyxViewersToolBarPrivate::setCrosshairJumpSlicesMode(int jumpSlicesMode)
{
//  Q_Q(qCjyxViewersToolBar);

  vtkSmartPointer<vtkCollection> nodes;
  nodes.TakeReference(this->DMMLScene->GetNodesByClass("vtkDMMLCrosshairNode"));
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkDMMLCrosshairNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkDMMLCrosshairNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetCrosshairBehavior(jumpSlicesMode);
    }
}

//---------------------------------------------------------------------------
void qCjyxViewersToolBarPrivate::setCrosshairEnabled(bool enabled)
{
//  Q_Q(qCjyxViewersToolBar);

  vtkSmartPointer<vtkCollection> nodes;
  nodes.TakeReference(this->DMMLScene->GetNodesByClass("vtkDMMLCrosshairNode"));
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkDMMLCrosshairNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkDMMLCrosshairNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    if (enabled)
      {
      node->SetCrosshairMode(this->CrosshairLastMode);
      }
    else
      {
      node->SetCrosshairMode(vtkDMMLCrosshairNode::NoCrosshair);
      }
    }
}

//---------------------------------------------------------------------------
void qCjyxViewersToolBarPrivate::setCrosshairMode(int mode)
{
//  Q_Q(qCjyxViewersToolBar);

  vtkSmartPointer<vtkCollection> nodes;
  nodes.TakeReference(this->DMMLScene->GetNodesByClass("vtkDMMLCrosshairNode"));
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkDMMLCrosshairNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkDMMLCrosshairNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    // cache before carry
    if (mode != vtkDMMLCrosshairNode::NoCrosshair)
      {
      this->CrosshairLastMode = mode;
      }

      node->SetCrosshairMode(mode);
    }
}

//---------------------------------------------------------------------------
void qCjyxViewersToolBarPrivate::setCrosshairThickness(int thickness)
{
  vtkSmartPointer<vtkCollection> nodes;
  nodes.TakeReference(this->DMMLScene->GetNodesByClass("vtkDMMLCrosshairNode"));
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkDMMLCrosshairNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkDMMLCrosshairNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetCrosshairThickness(thickness);
    }
}

//---------------------------------------------------------------------------
void qCjyxViewersToolBarPrivate::setIntersectingSlicesLineThicknessMode(int mode)
{
  if (!this->DMMLAppLogic)
    {
    qWarning() << Q_FUNC_INFO << " failed: application logic is invalid";
    return;
    }
  this->DMMLAppLogic->SetIntersectingSlicesLineThicknessMode(mode);
}

//---------------------------------------------------------------------------
void qCjyxViewersToolBarPrivate::setIntersectingSlicesIntersectionMode(int mode)
{
  if (!this->DMMLAppLogic)
    {
    qWarning() << Q_FUNC_INFO << " failed: application logic is invalid";
    return;
    }
  this->DMMLAppLogic->SetIntersectingSlicesIntersectionMode(mode);
}

// --------------------------------------------------------------------------
void qCjyxViewersToolBarPrivate::setIntersectingSlicesVisibility(bool visible)
{
  if (!this->DMMLAppLogic)
    {
    qWarning() << Q_FUNC_INFO << " failed: application logic is invalid";
    return;
    }
  this->DMMLAppLogic->SetIntersectingSlicesEnabled(vtkDMMLApplicationLogic::IntersectingSlicesVisibility, visible);
}

// --------------------------------------------------------------------------
void qCjyxViewersToolBarPrivate::setIntersectingSlicesInteractive(bool visible)
{
  if (!this->DMMLAppLogic)
    {
    qWarning() << Q_FUNC_INFO << " failed: application logic is invalid";
    return;
    }
  this->DMMLAppLogic->SetIntersectingSlicesEnabled(vtkDMMLApplicationLogic::IntersectingSlicesInteractive, visible);
}

// --------------------------------------------------------------------------
void qCjyxViewersToolBarPrivate::setIntersectingSlicesRotationEnabled(bool visible)
{
  if (!this->DMMLAppLogic)
    {
    qWarning() << Q_FUNC_INFO << " failed: application logic is invalid";
    return;
    }
  this->DMMLAppLogic->SetIntersectingSlicesEnabled(vtkDMMLApplicationLogic::IntersectingSlicesRotation, visible);
}

// --------------------------------------------------------------------------
void qCjyxViewersToolBarPrivate::setIntersectingSlicesTranslationEnabled(bool visible)
{
  if (!this->DMMLAppLogic)
    {
    qWarning() << Q_FUNC_INFO << " failed: application logic is invalid";
    return;
    }
  this->DMMLAppLogic->SetIntersectingSlicesEnabled(vtkDMMLApplicationLogic::IntersectingSlicesTranslation, visible);
}

//---------------------------------------------------------------------------
void qCjyxViewersToolBarPrivate::setDMMLScene(vtkDMMLScene* newScene)
{
  Q_Q(qCjyxViewersToolBar);

  if (newScene == this->DMMLScene)
    {
    return;
    }

  this->qvtkReconnect(this->DMMLScene, newScene, vtkDMMLScene::StartCloseEvent,
                      this, SLOT(OnDMMLSceneStartClose()));

  this->qvtkReconnect(this->DMMLScene, newScene, vtkDMMLScene::EndImportEvent,
                      this, SLOT(OnDMMLSceneEndImport()));

  this->qvtkReconnect(this->DMMLScene, newScene, vtkDMMLScene::EndCloseEvent,
                      this, SLOT(OnDMMLSceneEndClose()));

  this->DMMLScene = newScene;

  if (this->DMMLScene)
    {
    // Watch the crosshairs
    vtkDMMLNode *node;
    vtkCollectionSimpleIterator it;
    vtkSmartPointer<vtkCollection> crosshairs;
    crosshairs.TakeReference(this->DMMLScene->GetNodesByClass("vtkDMMLCrosshairNode"));
    for (crosshairs->InitTraversal(it);
         (node = (vtkDMMLNode*)crosshairs->GetNextItemAsObject(it));)
      {
      vtkDMMLCrosshairNode* crosshairNode = vtkDMMLCrosshairNode::SafeDownCast(node);
      if (crosshairNode)
        {
        this->qvtkReconnect(crosshairNode, vtkCommand::ModifiedEvent,
                          this, SLOT(onCrosshairNodeModeChangedEvent()));
        }
      }
    }

  // Update UI
  q->setEnabled(this->DMMLScene != nullptr);
  if (this->DMMLScene)
    {
    this->updateWidgetFromDMML();
    }
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void qCjyxViewersToolBarPrivate::updateWidgetFromDMML()
{
  Q_ASSERT(this->DMMLScene);

  vtkDMMLNode *node;
  vtkCollectionSimpleIterator it;
  vtkDMMLCrosshairNode* crosshairNode = nullptr;
  vtkSmartPointer<vtkCollection> crosshairs;
  crosshairs.TakeReference(this->DMMLScene->GetNodesByClass("vtkDMMLCrosshairNode"));
  for (crosshairs->InitTraversal(it);
       (node = (vtkDMMLNode*)crosshairs->GetNextItemAsObject(it));)
    {
    crosshairNode = vtkDMMLCrosshairNode::SafeDownCast(node);
    if (crosshairNode  && crosshairNode->GetCrosshairName() == std::string("default"))
      {
      break;
      }
    }
  if (crosshairNode)
    {
    // toggle on/off, jump slices, style of crosshair
    //

    // jump slices
    if (this->CrosshairJumpSlicesMapper->mapping(crosshairNode->GetCrosshairBehavior()) != nullptr)
      {
      QAction* action = (QAction *)(this->CrosshairJumpSlicesMapper->mapping(crosshairNode->GetCrosshairBehavior()));
      if (action)
        {
        action->setChecked(true);
        }
      }

    // style of crosshair
    if (this->CrosshairMapper->mapping(crosshairNode->GetCrosshairMode()) != nullptr)
      {
      QAction* action = (QAction *)(this->CrosshairMapper->mapping(crosshairNode->GetCrosshairMode()));
      if (action)
        {
        action->setChecked(true);
        }
      }

    // thickness
    if (this->CrosshairThicknessMapper->mapping(crosshairNode->GetCrosshairThickness()) != nullptr)
      {
      QAction* action = (QAction *)(this->CrosshairThicknessMapper->mapping(crosshairNode->GetCrosshairThickness()));
      if (action)
        {
        action->setChecked(true);
        }
      }

    // cache the mode
    if (crosshairNode->GetCrosshairMode() != vtkDMMLCrosshairNode::NoCrosshair)
      {
      this->CrosshairLastMode = crosshairNode->GetCrosshairMode();
      }

    // on/off
    // Checking the crosshair button may trigger a crosshair enable/disable action
    // therefore this toggle action should be the last.
    if (this->CrosshairToolButton)
      {
      this->CrosshairToggleAction->setChecked( crosshairNode->GetCrosshairMode() != vtkDMMLCrosshairNode::NoCrosshair );
      }
    }

  if (this->DMMLAppLogic)
    {
    // Cjyx intersection visibility
    this->IntersectingSlicesVisibleAction->setChecked(
      this->DMMLAppLogic->GetIntersectingSlicesEnabled(vtkDMMLApplicationLogic::IntersectingSlicesVisibility));
    // Cjyx intersection interactive
    this->IntersectingSlicesInteractiveAction->setChecked(
      this->DMMLAppLogic->GetIntersectingSlicesEnabled(vtkDMMLApplicationLogic::IntersectingSlicesInteractive));
    this->SliceIntersectionsFullIntersectionAction->setEnabled(this->IntersectingSlicesInteractiveAction->isChecked());
    this->SliceIntersectionsSkipIntersectionAction->setEnabled(this->IntersectingSlicesInteractiveAction->isChecked());
    this->SliceIntersectionsFineAction->setEnabled(this->IntersectingSlicesInteractiveAction->isChecked());
    this->SliceIntersectionsMediumAction->setEnabled(this->IntersectingSlicesInteractiveAction->isChecked());
    this->SliceIntersectionsThickAction->setEnabled(this->IntersectingSlicesInteractiveAction->isChecked());
    // Interaction options
    this->IntersectingSlicesInteractionModesMenu->setEnabled(this->IntersectingSlicesVisibleAction->isChecked());
    this->IntersectingSlicesTranslationEnabledAction->setChecked(
      this->DMMLAppLogic->GetIntersectingSlicesEnabled(vtkDMMLApplicationLogic::IntersectingSlicesTranslation));
    this->IntersectingSlicesRotationEnabledAction->setChecked(
      this->DMMLAppLogic->GetIntersectingSlicesEnabled(vtkDMMLApplicationLogic::IntersectingSlicesRotation));
    }
}

//---------------------------------------------------------------------------
void qCjyxViewersToolBarPrivate::OnDMMLSceneStartClose()
{
  Q_Q(qCjyxViewersToolBar);
  q->setEnabled(false);
}

//---------------------------------------------------------------------------
void qCjyxViewersToolBarPrivate::OnDMMLSceneEndImport()
{
  Q_Q(qCjyxViewersToolBar);

  // re-enable in case it didn't get re-enabled for scene load
  q->setEnabled(true);

  // update the state from dmml
  this->updateWidgetFromDMML();
}

//---------------------------------------------------------------------------
void qCjyxViewersToolBarPrivate::OnDMMLSceneEndClose()
{
  Q_Q(qCjyxViewersToolBar);
  Q_ASSERT(this->DMMLScene);
  if (!this->DMMLScene || this->DMMLScene->IsBatchProcessing())
    {
    return;
    }
  // re-enable it and update
  q->setEnabled(true);
  this->updateWidgetFromDMML();
}

//---------------------------------------------------------------------------
void qCjyxViewersToolBarPrivate::onCrosshairNodeModeChangedEvent()
{
  this->updateWidgetFromDMML();
}

//---------------------------------------------------------------------------
void qCjyxViewersToolBarPrivate::onSliceDisplayNodeChangedEvent()
{
  this->updateWidgetFromDMML();
}

//---------------------------------------------------------------------------
// qCjyxModuleSelectorToolBar methods

//---------------------------------------------------------------------------
qCjyxViewersToolBar::qCjyxViewersToolBar(const QString& title, QWidget* parentWidget)
  :Superclass(title, parentWidget)
  , d_ptr(new qCjyxViewersToolBarPrivate(*this))
{
  Q_D(qCjyxViewersToolBar);
  d->init();
}

//---------------------------------------------------------------------------
qCjyxViewersToolBar::qCjyxViewersToolBar(QWidget* parentWidget):Superclass(parentWidget)
  , d_ptr(new qCjyxViewersToolBarPrivate(*this))
{
  Q_D(qCjyxViewersToolBar);
  d->init();
}

//---------------------------------------------------------------------------
qCjyxViewersToolBar::~qCjyxViewersToolBar() = default;

//---------------------------------------------------------------------------
void qCjyxViewersToolBar::setApplicationLogic(vtkCjyxApplicationLogic* appLogic)
{
  Q_D(qCjyxViewersToolBar);
  d->DMMLAppLogic = appLogic;
}

//---------------------------------------------------------------------------
void qCjyxViewersToolBar::setDMMLScene(vtkDMMLScene* newScene)
{
  Q_D(qCjyxViewersToolBar);
  d->setDMMLScene(newScene);
}