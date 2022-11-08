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
#include <QMainWindow>

// DMML includes
#include <vtkDMMLInteractionNode.h>
#include <vtkDMMLLayoutLogic.h>
#include <vtkDMMLSelectionNode.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLViewNode.h>
#include <vtkDMMLWindowLevelWidget.h>

// Cjyx includes
#include "qCjyxApplication.h"
#include "qCjyxLayoutManager.h"
#include "qDMMLThreeDView.h"
#include "qDMMLThreeDWidget.h"
#include "qDMMLSliceView.h"
#include "qDMMLSliceWidget.h"
#include "qCjyxMouseModeToolBar_p.h"

// CjyxLogic includes
#include <vtkCjyxApplicationLogic.h>


//---------------------------------------------------------------------------
// qCjyxMouseModeToolBarPrivate methods

//---------------------------------------------------------------------------
qCjyxMouseModeToolBarPrivate::qCjyxMouseModeToolBarPrivate(qCjyxMouseModeToolBar& object)
  : q_ptr(&object)
{
  this->AdjustViewAction = nullptr;
  this->AdjustWindowLevelAction = nullptr;
  this->AdjustWindowLevelAdjustModeAction = nullptr;
  this->AdjustWindowLevelRegionModeAction = nullptr;
  this->AdjustWindowLevelCenteredRegionModeAction = nullptr;
  this->AdjustWindowLevelModeMapper = nullptr;
  this->AdjustWindowLevelMenu = nullptr;

  this->PlaceWidgetAction = nullptr;

  this->InteractionModesActionGroup = nullptr;
  this->DefaultPlaceClassName = "vtkDMMLMarkupsFiducialNode";
}

//---------------------------------------------------------------------------
void qCjyxMouseModeToolBarPrivate::init()
{
  Q_Q(qCjyxMouseModeToolBar);

  this->InteractionModesActionGroup = new QActionGroup(q);
  this->InteractionModesActionGroup->setExclusive(true);

  // Adjust view mode

  this->AdjustViewAction = new QAction(this);
  this->AdjustViewAction->setObjectName("AdjustViewAction");
  this->AdjustViewAction->setData(vtkDMMLInteractionNode::ViewTransform);
  this->AdjustViewAction->setToolTip(qCjyxMouseModeToolBar::tr("Translate/rotate view, adjust displayed objects"));
  this->AdjustViewAction->setIcon(QIcon(":/Icons/MouseViewTransformMode.png"));
  this->AdjustViewAction->setText(qCjyxMouseModeToolBar::tr("View"));
  this->AdjustViewAction->setCheckable(true);

  QObject::connect(this->AdjustViewAction, SIGNAL(toggled(bool)),
    q, SLOT(interactionModeActionTriggered(bool)));
  q->addAction(this->AdjustViewAction);
  this->InteractionModesActionGroup->addAction(this->AdjustViewAction);

  // Window/level mode

  QActionGroup* windowLevelModeActions = new QActionGroup(q);
  windowLevelModeActions->setExclusive(true);

  this->AdjustWindowLevelAdjustModeAction = new QAction(q);
  this->AdjustWindowLevelAdjustModeAction->setText(tr("Adjust"));
  this->AdjustWindowLevelAdjustModeAction->setToolTip(tr("Adjust window/level by click-and-drag in a slice viewer."));
  this->AdjustWindowLevelAdjustModeAction->setCheckable(true);

  this->AdjustWindowLevelRegionModeAction = new QAction(q);
  this->AdjustWindowLevelRegionModeAction->setText(tr("Select region"));
  this->AdjustWindowLevelRegionModeAction->setToolTip(
    tr("Set window level based on a rectangular region, specified by click-and-drag in a slice viewer. Click position is used as region corner."));
  this->AdjustWindowLevelRegionModeAction->setCheckable(true);

  this->AdjustWindowLevelCenteredRegionModeAction = new QAction(q);
  this->AdjustWindowLevelCenteredRegionModeAction->setText(tr("Select region - centered"));
  this->AdjustWindowLevelCenteredRegionModeAction->setToolTip(
    tr("Set window level based on a rectangular region, specified by click-and-drag in a slice viewer. Click position is used as region center."));
  this->AdjustWindowLevelCenteredRegionModeAction->setCheckable(true);

  windowLevelModeActions->addAction(this->AdjustWindowLevelAdjustModeAction);
  windowLevelModeActions->addAction(this->AdjustWindowLevelRegionModeAction);
  windowLevelModeActions->addAction(this->AdjustWindowLevelCenteredRegionModeAction);

  this->AdjustWindowLevelModeMapper = new ctkSignalMapper(q);
  this->AdjustWindowLevelModeMapper->setMapping(this->AdjustWindowLevelAdjustModeAction, vtkDMMLWindowLevelWidget::ModeAdjust);
  this->AdjustWindowLevelModeMapper->setMapping(this->AdjustWindowLevelRegionModeAction, vtkDMMLWindowLevelWidget::ModeRectangle);
  this->AdjustWindowLevelModeMapper->setMapping(this->AdjustWindowLevelCenteredRegionModeAction, vtkDMMLWindowLevelWidget::ModeRectangleCentered);
  QObject::connect(windowLevelModeActions, SIGNAL(triggered(QAction*)), this->AdjustWindowLevelModeMapper, SLOT(map(QAction*)));
  QObject::connect(this->AdjustWindowLevelModeMapper, SIGNAL(mapped(int)), q, SLOT(setAdjustWindowLevelMode(int)));

  // Menu
  this->AdjustWindowLevelMenu = new QMenu(tr("Adjust window/level"), q);
  this->AdjustWindowLevelMenu->addActions(windowLevelModeActions->actions());

  this->AdjustWindowLevelAction = new QAction(this);
  this->AdjustWindowLevelAction->setObjectName("AdjustWindowLevelAction");
  this->AdjustWindowLevelAction->setData(vtkDMMLInteractionNode::AdjustWindowLevel);
  this->AdjustWindowLevelAction->setToolTip(qCjyxMouseModeToolBar::tr(
    "Adjust window/level of volume by left-click-and-drag in slice views."
    " Hold down Ctrl/Cmd key for temporarily switch between adjustment and region-based setting."));
  this->AdjustWindowLevelAction->setIcon(QIcon(":/Icons/MouseWindowLevelMode.png"));
  this->AdjustWindowLevelAction->setText(qCjyxMouseModeToolBar::tr("Window/level"));
  this->AdjustWindowLevelAction->setCheckable(true);
  this->AdjustWindowLevelAction->setMenu(this->AdjustWindowLevelMenu);
  //this->AdjustWindowLevelAction->setPopupMode(QToolButton::MenuButtonPopup);

  QObject::connect(this->AdjustWindowLevelAction, SIGNAL(toggled(bool)),
    q, SLOT(interactionModeActionTriggered(bool)));
  q->addAction(this->AdjustWindowLevelAction);
  this->InteractionModesActionGroup->addAction(this->AdjustWindowLevelAction);

  // Place mode
  this->ToolBarAction = new QAction(this);
  this->ToolBarAction->setObjectName("ToolBarAction");
  this->ToolBarAction->setToolTip(qCjyxMouseModeToolBar::tr("Toggle Markups Toolbar"));
  this->ToolBarAction->setText(qCjyxMouseModeToolBar::tr("Toggle Markups Toolbar"));
  this->ToolBarAction->setEnabled(true);
  this->ToolBarAction->setIcon(QIcon(":/Icons/MarkupsDisplayToolBar.png"));

  QObject::connect(this->ToolBarAction, SIGNAL(triggered()),
    q, SLOT(toggleMarkupsToolBar()));
  q->addAction(this->ToolBarAction);

  this->PlaceWidgetMenu = new QMenu(qCjyxMouseModeToolBar::tr("Place Menu"), q);
  this->PlaceWidgetMenu->setObjectName("PlaceWidgetMenu");
  this->PlaceWidgetMenu->addAction(this->ToolBarAction);

  this->PlaceWidgetAction = new QAction(this);
  this->PlaceWidgetAction->setObjectName("PlaceWidgetAction");
  this->PlaceWidgetAction->setData(vtkDMMLInteractionNode::Place);
  this->PlaceWidgetAction->setToolTip(qCjyxMouseModeToolBar::tr("Create and Place"));
  this->PlaceWidgetAction->setText(qCjyxMouseModeToolBar::tr("Place"));
  this->PlaceWidgetAction->setCheckable(true);
  this->PlaceWidgetAction->setEnabled(true);
  this->PlaceWidgetAction->setMenu(this->PlaceWidgetMenu);

  connect(this->PlaceWidgetAction, SIGNAL(triggered()), q, SLOT(switchPlaceMode()));
  this->InteractionModesActionGroup->addAction(this->PlaceWidgetAction);

}

//---------------------------------------------------------------------------
void qCjyxMouseModeToolBarPrivate::setDMMLScene(vtkDMMLScene* newScene)
{
  Q_Q(qCjyxMouseModeToolBar);

  if (newScene == this->DMMLScene)
    {
    return;
    }

  this->qvtkReconnect(this->DMMLScene, newScene, vtkDMMLScene::StartBatchProcessEvent,
                      this, SLOT(onDMMLSceneStartBatchProcess()));

  this->qvtkReconnect(this->DMMLScene, newScene, vtkDMMLScene::EndBatchProcessEvent,
                      this, SLOT(onDMMLSceneEndBatchProcess()));

  this->DMMLScene = newScene;

  // watch for changes to the interaction, selection nodes so can update the widget
  q->setInteractionNode((this->DMMLAppLogic && this->DMMLScene) ? this->DMMLAppLogic->GetInteractionNode() : nullptr);

  vtkDMMLSelectionNode* selectionNode =
    (this->DMMLAppLogic && this->DMMLScene) ?
    this->DMMLAppLogic->GetSelectionNode() : nullptr;
  this->qvtkReconnect(selectionNode, vtkDMMLSelectionNode::ActivePlaceNodeClassNameChangedEvent,
                      this, SLOT(updateWidgetFromDMML()));
  this->qvtkReconnect(selectionNode, vtkDMMLSelectionNode::PlaceNodeClassNameListModifiedEvent,
                      this, SLOT(updateWidgetFromDMML()));
  this->qvtkReconnect(selectionNode, vtkDMMLSelectionNode::ActivePlaceNodeIDChangedEvent,
    this, SLOT(updateWidgetFromDMML()));
  this->qvtkReconnect(selectionNode, vtkDMMLSelectionNode::ActivePlaceNodePlacementValidEvent,
    this, SLOT(updateWidgetFromDMML()));

  // Update UI
  q->setEnabled(this->DMMLScene != nullptr);
  this->updateWidgetFromDMML();
}

//---------------------------------------------------------------------------
QCursor qCjyxMouseModeToolBarPrivate::cursorFromIcon(QIcon& icon)
{
  QList<QSize> availableSizes = icon.availableSizes();
  if (availableSizes.size() > 0)
    {
    return QCursor(icon.pixmap(availableSizes[0]));
    }
  else
    {
    // use a default
    return QCursor(icon.pixmap(20));
    }
}

//---------------------------------------------------------------------------
void qCjyxMouseModeToolBarPrivate::updateWidgetFromDMML()
{
  Q_Q(qCjyxMouseModeToolBar);
  vtkDMMLInteractionNode* interactionNode = q->interactionNode();
  if (!interactionNode)
    {
    qDebug() << "Mouse Mode ToolBar: no interaction node";
    q->setEnabled(false);
    return;
    }
  q->setEnabled(true);

  // Find action corresponding to current interaction mode
  int currentInteractionMode = interactionNode->GetCurrentInteractionMode();
  QAction* currentAction = nullptr;
  foreach(QAction* action, this->InteractionModesActionGroup->actions())
    {
    if (action->data().toInt() == currentInteractionMode)
      {
      currentAction = action;
      break;
      }
    }

  // Set action for current interaction mode checked
  if (currentAction)
    {
    currentAction->setChecked(true);
    }
  else
    {
    // uncheck all actions
    QAction* checkedAction = this->InteractionModesActionGroup->checkedAction();
    if (checkedAction)
      {
      checkedAction->setChecked(false);
      }
    }

  // Update place widget action
  this->updatePlaceWidget();

  // Update persistence checkbox
  int persistence = interactionNode->GetPlaceModePersistence();

  // find the active place node class name and set it's corresponding action to be checked
  QString activePlaceNodeClassName;
  vtkDMMLSelectionNode *selectionNode = (this->DMMLAppLogic ? this->DMMLAppLogic->GetSelectionNode() : nullptr);
  if (selectionNode && selectionNode->GetActivePlaceNodeClassName())
    {
    activePlaceNodeClassName = selectionNode->GetActivePlaceNodeClassName();
    }
  // Update checked state of place actions
  if (activePlaceNodeClassName.isEmpty())
    {
    activePlaceNodeClassName = this->DefaultPlaceClassName;
    }

  int adjustWindowLevelMode = vtkDMMLWindowLevelWidget::GetAdjustWindowLevelModeFromString(
    interactionNode->GetAttribute(vtkDMMLWindowLevelWidget::GetInteractionNodeAdjustWindowLevelModeAttributeName()));
    switch (adjustWindowLevelMode)
    {
    case vtkDMMLWindowLevelWidget::ModeRectangle:
      {
      this->AdjustWindowLevelRegionModeAction->setChecked(true);
      }
      break;
    case vtkDMMLWindowLevelWidget::ModeRectangleCentered:
      {
      this->AdjustWindowLevelCenteredRegionModeAction->setChecked(true);
      }
      break;
    case vtkDMMLWindowLevelWidget::ModeAdjust:
    default:
      {
      this->AdjustWindowLevelAdjustModeAction->setChecked(true);
      }
      break;
    }
  this->updateCursor();
}

//---------------------------------------------------------------------------
void qCjyxMouseModeToolBarPrivate::updatePlaceWidget()
{
  Q_Q(qCjyxMouseModeToolBar);
  vtkDMMLInteractionNode* interactionNode = q->interactionNode();
  if (!interactionNode)
    {
    return;
    }
  vtkDMMLSelectionNode *selectionNode =
    this->DMMLAppLogic ? this->DMMLAppLogic->GetSelectionNode() : nullptr;
  if (!selectionNode)
    {
    return;
    }
  QString activePlaceNodeClassName = selectionNode->GetActivePlaceNodeClassName();
  bool validNodeForPlacement = selectionNode->GetActivePlaceNodePlacementValid();
  if (!validNodeForPlacement || activePlaceNodeClassName.isEmpty())
    {
    q->removeAction(this->PlaceWidgetAction);
    q->addAction(this->ToolBarAction);
    return;
    }

  QString activePlaceNodeID = selectionNode->GetActivePlaceNodeID();
  if (activePlaceNodeID.isEmpty())
    {
    q->removeAction(this->PlaceWidgetAction);
    q->addAction(this->ToolBarAction);
    return;
    }

  const int numClassNames = selectionNode->GetNumberOfPlaceNodeClassNamesInList();
  QString placeNodeResource;
  QString placeNodeIconName;
  for (int i = 0; i < numClassNames; ++i)
    {
    if (activePlaceNodeClassName == QString(selectionNode->GetPlaceNodeClassNameByIndex(i).c_str()))
      {
      placeNodeResource = QString(selectionNode->GetPlaceNodeResourceByIndex(i).c_str());
      placeNodeIconName = QString(selectionNode->GetPlaceNodeIconNameByIndex(i).c_str());
      break;
      }
    }
  q->removeAction(this->ToolBarAction);

  QIcon icon(placeNodeResource);
  if (icon.availableSizes().empty())
    {
    qWarning() << "Failed to load icon from resource " << placeNodeResource;
    }
  this->PlaceWidgetAction->setIcon(icon);
  this->PlaceWidgetAction->setText(placeNodeIconName);
  this->PlaceWidgetAction->setData(vtkDMMLInteractionNode::Place);
  QString tooltip = QString("Place a control point");
  this->PlaceWidgetAction->setToolTip(tooltip);
  this->PlaceWidgetAction->setCheckable(true);

  connect(this->PlaceWidgetAction, SIGNAL(triggered()), q, SLOT(switchPlaceMode()));
  q->addAction(this->PlaceWidgetAction);
}

//---------------------------------------------------------------------------
void qCjyxMouseModeToolBarPrivate::updateCursor()
{
  Q_Q(qCjyxMouseModeToolBar);

  if (!this->DMMLAppLogic)
    {
    qWarning() << "Mouse Mode Tool Bar not set up with application logic";
    return;
    }
  vtkDMMLInteractionNode* interactionNode = q->interactionNode();
  if (!interactionNode)
    {
    return;
    }

  // Use the action's icon as cursor
  // Except when in view mode (then use default cursor) or when in place mode
  // (then place node class is used).
  int currentInteractionMode = interactionNode->GetCurrentInteractionMode();
  if (currentInteractionMode != vtkDMMLInteractionNode::Place)
    {
    if (interactionNode->GetCurrentInteractionMode() == vtkDMMLInteractionNode::ViewTransform)
      {
      q->changeCursorTo(QCursor());
      }
    else
      {
      // Find action corresponding to current interaction mode
      foreach(QAction * action, this->InteractionModesActionGroup->actions())
        {
        if (action->data().toInt() == currentInteractionMode)
          {
          QIcon icon = action->icon();
          q->changeCursorTo(this->cursorFromIcon(icon));
          break;
          }
        }
      }
    return;
    }

  const char* placeNodeClassName = nullptr;
  vtkDMMLSelectionNode* selectionNode =
    this->DMMLAppLogic ? this->DMMLAppLogic->GetSelectionNode() : nullptr;
  if (selectionNode)
    {
    placeNodeClassName = selectionNode->GetActivePlaceNodeClassName();
    }
  if (!placeNodeClassName)
    {
    q->changeCursorTo(QCursor());
    return;
    }

  std::string resource = selectionNode->GetPlaceNodeResourceByClassName(std::string(placeNodeClassName));
  if (!resource.empty())
    {
    q->changeCursorTo(QCursor(QPixmap(resource.c_str()), -1, 0));
    }
}

//---------------------------------------------------------------------------
void qCjyxMouseModeToolBarPrivate::onDMMLSceneStartBatchProcess()
{
  Q_Q(qCjyxMouseModeToolBar);
  q->setEnabled(false);
}

//---------------------------------------------------------------------------
void qCjyxMouseModeToolBarPrivate::onDMMLSceneEndBatchProcess()
{
  Q_Q(qCjyxMouseModeToolBar);

  // re-enable in case it didn't get re-enabled for scene load
  q->setEnabled(true);

  q->setInteractionNode((this->DMMLAppLogic && this->DMMLScene) ? this->DMMLAppLogic->GetInteractionNode() : nullptr);

  // update the state from dmml
  this->updateWidgetFromDMML();
}

//---------------------------------------------------------------------------
void qCjyxMouseModeToolBarPrivate::onActivePlaceNodeClassNameChangedEvent()
{
  this->updateWidgetFromDMML();
}

//---------------------------------------------------------------------------
void qCjyxMouseModeToolBarPrivate::onPlaceNodeClassNameListModifiedEvent()
{
  this->updateWidgetFromDMML();
}

//---------------------------------------------------------------------------
// qCjyxModuleSelectorToolBar methods

//---------------------------------------------------------------------------
qCjyxMouseModeToolBar::qCjyxMouseModeToolBar(const QString& title, QWidget* parentWidget)
  :Superclass(title, parentWidget)
  , d_ptr(new qCjyxMouseModeToolBarPrivate(*this))
{
  Q_D(qCjyxMouseModeToolBar);
  d->init();
}

//---------------------------------------------------------------------------
qCjyxMouseModeToolBar::qCjyxMouseModeToolBar(QWidget* parentWidget):Superclass(parentWidget)
  , d_ptr(new qCjyxMouseModeToolBarPrivate(*this))
{
  Q_D(qCjyxMouseModeToolBar);
  d->init();
}

//---------------------------------------------------------------------------
qCjyxMouseModeToolBar::~qCjyxMouseModeToolBar() = default;

//---------------------------------------------------------------------------
void qCjyxMouseModeToolBar::setApplicationLogic(vtkCjyxApplicationLogic* appLogic)
{
  Q_D(qCjyxMouseModeToolBar);
  d->DMMLAppLogic = appLogic;
}

//---------------------------------------------------------------------------
void qCjyxMouseModeToolBar::setDMMLScene(vtkDMMLScene* newScene)
{
  Q_D(qCjyxMouseModeToolBar);
  d->setDMMLScene(newScene);
}

//---------------------------------------------------------------------------
void qCjyxMouseModeToolBar::switchToViewTransformMode()
{
  Q_D(qCjyxMouseModeToolBar);

  if (!d->DMMLAppLogic)
    {
    qWarning() << "Mouse Mode Tool Bar not set up with application logic";
    return;
    }
  vtkDMMLInteractionNode * intNode = this->interactionNode();
  if (!intNode)
    {
    qWarning() << "Mouse Mode Tool Bar not set up with application logic";
    return;
    }

  // update the interaction node, should trigger a cursor update
  intNode->SwitchToViewTransformMode();
}

//---------------------------------------------------------------------------
void qCjyxMouseModeToolBar::changeCursorTo(QCursor cursor)
{
  if (!qCjyxApplication::application())
    {
    qWarning() << "changeCursorTo: can't get a qCjyxApplication";
    return;
    }
  qDMMLLayoutManager *layoutManager = qCjyxApplication::application()->layoutManager();

  if (!layoutManager)
    {
    return;
    }

  // Updated all mapped 3D viewers
  for (int i=0; i < layoutManager->threeDViewCount(); ++i)
    {
    qDMMLThreeDView* threeDView = layoutManager->threeDWidget(i)->threeDView();
    if (!threeDView->dmmlViewNode()->IsMappedInLayout())
      {
      continue;
      }
    // Update cursor only if view interaction node corresponds to the one associated with the mouse toolbar
    if (threeDView->dmmlViewNode()->GetInteractionNode() != this->interactionNode())
      {
      continue;
      }
    threeDView->setViewCursor(cursor);
    threeDView->setDefaultViewCursor(cursor);
    }

  // Updated all mapped cjyx viewers
  foreach(const QString& viewerName, layoutManager->sliceViewNames())
    {
    qDMMLSliceView* sliceView = layoutManager->sliceWidget(viewerName)->sliceView();
    if (!sliceView->dmmlSliceNode()->IsMappedInLayout())
      {
      continue;
      }
    // Update cursor only if view interaction node corresponds to the one associated with the mouse toolbar
    if (sliceView->dmmlSliceNode()->GetInteractionNode() != this->interactionNode())
      {
      continue;
      }
    sliceView->setViewCursor(cursor);
    sliceView->setDefaultViewCursor(cursor);
    }
}

//---------------------------------------------------------------------------
void qCjyxMouseModeToolBar::switchPlaceMode()
{
  Q_D(qCjyxMouseModeToolBar);

  if (!d->DMMLAppLogic)
    {
    qWarning() << "Mouse Mode Tool Bar not set up with application logic";
    return;
    }
  vtkDMMLInteractionNode* interactionNode = this->interactionNode();
  interactionNode->SetCurrentInteractionMode(vtkDMMLInteractionNode::Place);
}

//---------------------------------------------------------------------------
QAction* qCjyxMouseModeToolBar::actionFromPlaceNodeClassName(QString placeNodeClassName, QMenu *menu)
{
  foreach(QAction* action, menu->actions())
    {
    if (action->objectName() == placeNodeClassName)
      {
      return action;
      }
    }
  return nullptr;
}

//---------------------------------------------------------------------------
void qCjyxMouseModeToolBar::setPersistence(bool persistent)
{
  vtkDMMLInteractionNode *interactionNode = this->interactionNode();
  if (interactionNode)
    {
    interactionNode->SetPlaceModePersistence(persistent ? 1 : 0);
    }
  else
    {
    qWarning() << __FUNCTION__ << ": no interaction node found to toggle.";
    }
}

//---------------------------------------------------------------------------
QString qCjyxMouseModeToolBar::defaultPlaceClassName()const
{
  Q_D(const qCjyxMouseModeToolBar);
  return d->DefaultPlaceClassName;
}

//---------------------------------------------------------------------------
void qCjyxMouseModeToolBar::setDefaultPlaceClassName(const QString& className)
{
  Q_D(qCjyxMouseModeToolBar);
  d->DefaultPlaceClassName = className;
}

//-----------------------------------------------------------------------------
vtkDMMLInteractionNode* qCjyxMouseModeToolBar::interactionNode()const
{
  Q_D(const qCjyxMouseModeToolBar);
  return d->InteractionNode;
}

//-----------------------------------------------------------------------------
void qCjyxMouseModeToolBar::setInteractionNode(vtkDMMLInteractionNode* interactionNode)
{
  Q_D(qCjyxMouseModeToolBar);
  if (d->InteractionNode == interactionNode)
    {
    return;
    }
  d->qvtkReconnect(d->InteractionNode, interactionNode, vtkCommand::ModifiedEvent,
                   d, SLOT(updateWidgetFromDMML()));
  d->InteractionNode = interactionNode;
  d->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qCjyxMouseModeToolBar::interactionModeActionTriggered(bool toggled)
{
  Q_D(qCjyxMouseModeToolBar);
  if (!toggled)
    {
    return;
    }
  QAction* sourceAction = qobject_cast<QAction*>(sender());
  if (!sourceAction)
    {
    return;
    }
  int selectedInteractionMode = sourceAction->data().toInt();
  if (!d->InteractionNode)
    {
    return;
    }
  d->InteractionNode->SetCurrentInteractionMode(selectedInteractionMode);

  // If no active place node class name is selected then use the default class
  if (d->InteractionNode->GetCurrentInteractionMode() == vtkDMMLInteractionNode::Place)
    {
    vtkDMMLSelectionNode* selectionNode = (d->DMMLAppLogic && d->DMMLScene) ?
      d->DMMLAppLogic->GetSelectionNode() : nullptr;
    if (selectionNode)
      {
      const char* currentPlaceNodeClassName = selectionNode->GetActivePlaceNodeClassName();
      if (!currentPlaceNodeClassName || strlen(currentPlaceNodeClassName) == 0)
        {
        selectionNode->SetReferenceActivePlaceNodeClassName(d->DefaultPlaceClassName.toUtf8());
        }
      }
    }
  d->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qCjyxMouseModeToolBar::setAdjustWindowLevelMode(int adjustWindowLevelMode)
{
  Q_D(qCjyxMouseModeToolBar);
  vtkDMMLInteractionNode* interactionNode = this->interactionNode();
  if (!interactionNode)
    {
    qDebug() << "setAdjustWindowLevelMode: no interaction node";
    return;
    }
  interactionNode->SetAttribute(vtkDMMLWindowLevelWidget::GetInteractionNodeAdjustWindowLevelModeAttributeName(),
    vtkDMMLWindowLevelWidget::GetAdjustWindowLevelModeAsString(adjustWindowLevelMode));
}

//-----------------------------------------------------------------------------
void qCjyxMouseModeToolBar::toggleMarkupsToolBar()
{
  QMainWindow* mainWindow = qCjyxApplication::application()->mainWindow();
  if (mainWindow == nullptr)
    {
    qDebug("qCjyxMouseModeToolBar::toggleMarkupsToolBar: no main window is available, toolbar is not added");
    return;
    }
  foreach(QToolBar * toolBar, mainWindow->findChildren<QToolBar*>())
    {
    if (toolBar->objectName() == QString("MarkupsToolBar"))
      {
      bool visibility = toolBar->isVisible();
      toolBar->setVisible(!visibility);
      }
    }
}
