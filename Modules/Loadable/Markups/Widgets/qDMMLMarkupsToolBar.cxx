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
#include <QToolButton>
#include <QMenu>
#include <QCheckBox>
#include <QSignalMapper>
#include <QSplitter>
#include <QShortcut>
#include <QKeySequence>

// Cjyx includes
#include "qCjyxCoreApplication.h"
#include "qCjyxApplication.h"
#include "qCjyxLayoutManager.h"

// DMML includes
#include "qDMMLMarkupsToolBar_p.h"
#include "qDMMLNodeComboBox.h"
#include "qDMMLThreeDView.h"
#include "qDMMLThreeDWidget.h"
#include "qDMMLSliceView.h"
#include "qDMMLSliceWidget.h"
#include "qCjyxAbstractCoreModule.h"
#include "qCjyxModuleManager.h"

#include <vtkDMMLScene.h>
#include <vtkDMMLMarkupsNode.h>
#include <vtkDMMLInteractionNode.h>
#include <vtkDMMLSelectionNode.h>
#include <vtkDMMLLayoutLogic.h>
#include <vtkDMMLSelectionNode.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLViewNode.h>
#include <vtkDMMLWindowLevelWidget.h>
#include <qCjyxMarkupsPlaceWidget.h>
#include <vtkDMMLDisplayNode.h>
#include <vtkDMMLAnnotationNode.h>

// CjyxLogic includes
#include <vtkCjyxApplicationLogic.h>
// DMMLLogic includes
#include <vtkDMMLApplicationLogic.h>
#include <vtkCjyxMarkupsLogic.h>

// VTK includes
#include <vtkWeakPointer.h>
#include <vtkSmartPointer.h>

//---------------------------------------------------------------------------
qDMMLMarkupsToolBarPrivate::qDMMLMarkupsToolBarPrivate(qDMMLMarkupsToolBar& object)
  : q_ptr(&object)
{
  this->DefaultPlaceClassName = "vtkDMMLMarkupsFiducialNode";
}

//---------------------------------------------------------------------------
void qDMMLMarkupsToolBarPrivate::init()
{
  Q_Q(qDMMLMarkupsToolBar);

  // set up keyboard shortcuts
  q->addCreateNodeShortcut("Ctrl+Shift+A");
  q->addTogglePersistenceShortcut("Ctrl+Shift+T");
  q->addPlacePointShortcut("Ctrl+Shift+Space");

  // Markups node selector
  this->MarkupsNodeSelector = new qDMMLNodeComboBox(q);
  this->MarkupsNodeSelector->setObjectName(QString("MarkupsNodeSelector"));
  this->MarkupsNodeSelector->setNodeTypes(QStringList(QString("vtkDMMLMarkupsNode")));
  this->MarkupsNodeSelector->setNoneEnabled(false);
  this->MarkupsNodeSelector->setAddEnabled(false);
  this->MarkupsNodeSelector->setRenameEnabled(true);
  this->MarkupsNodeSelector->setEditEnabled(true);
  this->MarkupsNodeSelector->setMaximumWidth(165);
  this->MarkupsNodeSelector->setEnabled(true);
  this->MarkupsNodeSelector->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  this->MarkupsNodeSelector->setToolTip("Select active markup");
  this->MarkupsNodeSelector->setDMMLScene(qCjyxApplication::application()->dmmlScene());
  this->NodeSelectorAction = q->addWidget(this->MarkupsNodeSelector);

  connect(this->MarkupsNodeSelector, SIGNAL(currentNodeChanged(vtkDMMLNode*)), q, SIGNAL(activeMarkupsNodeChanged(vtkDMMLNode*)));
  connect(this->MarkupsNodeSelector, SIGNAL(nodeActivated(vtkDMMLNode*)), q, SLOT(onMarkupsNodeChanged(vtkDMMLNode*)));

  // Get scene and application logic
  q->setApplicationLogic(qCjyxApplication::application()->applicationLogic());
  q->setDMMLScene(qCjyxApplication::application()->dmmlScene());
}

// --------------------------------------------------------------------------
void qDMMLMarkupsToolBarPrivate::addSetModuleButton(vtkCjyxMarkupsLogic* markupsLogic, const QString& moduleName)
{
  Q_UNUSED(markupsLogic);
  Q_Q(qDMMLMarkupsToolBar);

  QPushButton* moduleButton = new QPushButton();
  moduleButton->setObjectName(QString(moduleName + " module shortcut"));
  moduleButton->setToolTip("Open the " + moduleName + " module");
  QString iconName = ":/Icons/" + moduleName + ".png";
  moduleButton->setIcon(QIcon(iconName));
  QSignalMapper* mapper = new QSignalMapper(moduleButton);
  QObject::connect(moduleButton, SIGNAL(clicked()), mapper, SLOT(map()));
  mapper->setMapping(moduleButton, moduleName);
  QObject::connect(mapper, SIGNAL(mapped(const QString&)),
    this, SLOT(onSetModule(const QString&)));
  q->addWidget(moduleButton);
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsToolBarPrivate::onSetModule(const QString& moduleName)
{
  qCjyxModuleManager* moduleManager = qCjyxCoreApplication::application()->moduleManager();
  if (!moduleManager)
    {
    return;
    }
  qCjyxAbstractCoreModule* module = moduleManager->module(moduleName);
  if (!module)
    {
    return;
    }
  qCjyxLayoutManager* layoutManager = qCjyxApplication::application()->layoutManager();
  if (!layoutManager)
    {
    return;
    }
  layoutManager->setCurrentModule(moduleName);
}

// --------------------------------------------------------------------------
void qDMMLMarkupsToolBarPrivate::setDMMLScene(vtkDMMLScene* newScene)
{
  Q_Q(qDMMLMarkupsToolBar);

  if (newScene == this->DMMLScene)
    {
    return;
    }

  this->qvtkReconnect(this->DMMLScene, newScene, vtkDMMLScene::StartBatchProcessEvent,
    this, SLOT(onDMMLSceneStartBatchProcess()));

  this->qvtkReconnect(this->DMMLScene, newScene, vtkDMMLScene::EndBatchProcessEvent,
    this, SLOT(onDMMLSceneEndBatchProcess()));

  this->DMMLScene = newScene;
  this->MarkupsNodeSelector->setDMMLScene(newScene);
  if (this->MarkupsPlaceWidget)
    {
    this->MarkupsPlaceWidget->setDMMLScene(newScene);
    }

  // watch for changes to the interaction, selection nodes so can update the widget
  q->setInteractionNode((this->DMMLAppLogic && this->DMMLScene) ? this->DMMLAppLogic->GetInteractionNode() : nullptr);
  q->setSelectionNode((this->DMMLAppLogic && this->DMMLScene) ? this->DMMLAppLogic->GetSelectionNode() : nullptr);

  vtkDMMLSelectionNode* selectionNode =
    (this->DMMLAppLogic && this->DMMLScene) ?
    this->DMMLAppLogic->GetSelectionNode() : nullptr;
  this->qvtkReconnect(selectionNode, vtkDMMLSelectionNode::ActivePlaceNodeClassNameChangedEvent,
    this, SLOT(updateWidgetFromDMML()));
  this->qvtkReconnect(selectionNode, vtkDMMLSelectionNode::PlaceNodeClassNameListModifiedEvent,
    this, SLOT(updateWidgetFromDMML()));

  // Update UI
  q->setEnabled(this->DMMLScene != nullptr);
  this->updateWidgetFromDMML();

}

//---------------------------------------------------------------------------
void qDMMLMarkupsToolBarPrivate::onDMMLSceneStartBatchProcess()
{
  Q_Q(qDMMLMarkupsToolBar);
  q->setEnabled(false);
}

//---------------------------------------------------------------------------
void qDMMLMarkupsToolBarPrivate::onDMMLSceneEndBatchProcess()
{
  Q_Q(qDMMLMarkupsToolBar);

  // re-enable in case it didn't get re-enabled for scene load
  q->setEnabled(true);

  q->setInteractionNode((this->DMMLAppLogic && this->DMMLScene) ? this->DMMLAppLogic->GetInteractionNode() : nullptr);
  q->setSelectionNode((this->DMMLAppLogic && this->DMMLScene) ? this->DMMLAppLogic->GetSelectionNode() : nullptr);

  // update the state from dmml
  this->updateWidgetFromDMML();
}


//---------------------------------------------------------------------------
QCursor qDMMLMarkupsToolBarPrivate::cursorFromIcon(QIcon& icon)
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
void qDMMLMarkupsToolBarPrivate::updateWidgetFromDMML()
{
  Q_Q(qDMMLMarkupsToolBar);
  vtkDMMLInteractionNode* interactionNode = q->interactionNode();
  if (!interactionNode)
    {
    qDebug() << "Markups ToolBar: no interaction node";
    q->setEnabled(false);
    return;
    }
  vtkDMMLSelectionNode* selectionNode = q->selectionNode();
  if (!selectionNode)
    {
    q->setEnabled(false);
    return;
    }

  q->setEnabled(true);

  // Update active markups node
  vtkDMMLMarkupsNode* activeMarkupsNode = nullptr;
  if (selectionNode->GetScene())
    {
    activeMarkupsNode = vtkDMMLMarkupsNode::SafeDownCast(
      selectionNode->GetScene()->GetNodeByID(selectionNode->GetActivePlaceNodeID()));
    }
  // do not block signals so that signals are emitted
  this->MarkupsNodeSelector->setCurrentNode(activeMarkupsNode);

  if (this->MarkupsPlaceWidget)
    {
    // do not block signals so that activeMarkupsPlaceModeChanged signals are emitted
    this->MarkupsPlaceWidget->setEnabled(true);
    this->MarkupsPlaceWidget->setInteractionNode(interactionNode);
    this->MarkupsPlaceWidget->setSelectionNode(selectionNode);
    this->MarkupsPlaceWidget->setCurrentNode(activeMarkupsNode);
    }
}
//---------------------------------------------------------------------------
void qDMMLMarkupsToolBarPrivate::onActivePlaceNodeClassNameChangedEvent()
{
  this->updateWidgetFromDMML();
}

//---------------------------------------------------------------------------
void qDMMLMarkupsToolBarPrivate::onPlaceNodeClassNameListModifiedEvent()
{
  this->updateWidgetFromDMML();
}

// --------------------------------------------------------------------------
// qDMMLMarkupsToolBar methods

// --------------------------------------------------------------------------
qDMMLMarkupsToolBar::qDMMLMarkupsToolBar(const QString& title, QWidget* parentWidget)
  :Superclass(title, parentWidget)
   , d_ptr(new qDMMLMarkupsToolBarPrivate(*this))
{
  Q_D(qDMMLMarkupsToolBar);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLMarkupsToolBar::qDMMLMarkupsToolBar(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qDMMLMarkupsToolBarPrivate(*this))
{
  Q_D(qDMMLMarkupsToolBar);
  d->init();
}

//---------------------------------------------------------------------------
qDMMLMarkupsToolBar::~qDMMLMarkupsToolBar() = default;

// --------------------------------------------------------------------------
void qDMMLMarkupsToolBar::setDMMLScene(vtkDMMLScene* scene)
{
  Q_D(qDMMLMarkupsToolBar);
  d->setDMMLScene(scene);
}

// --------------------------------------------------------------------------
vtkDMMLMarkupsNode* qDMMLMarkupsToolBar::activeMarkupsNode()
{
  Q_D(qDMMLMarkupsToolBar);
  return vtkDMMLMarkupsNode::SafeDownCast(d->MarkupsNodeSelector->currentNode());
}

// --------------------------------------------------------------------------
void qDMMLMarkupsToolBar::setActiveMarkupsNode(vtkDMMLMarkupsNode* newActiveNode)
{
  Q_D(qDMMLMarkupsToolBar);
  vtkDMMLSelectionNode* selectionNode = (d->DMMLAppLogic && d->DMMLScene) ?
    d->DMMLAppLogic->GetSelectionNode() : nullptr;
  if (selectionNode == nullptr && newActiveNode != nullptr)
    {
    qWarning() << Q_FUNC_INFO << " failed: invalid selection node";
    return;
    }
  selectionNode->SetActivePlaceNodeID(newActiveNode ? newActiveNode->GetID() : nullptr);
  // the GUI will be updated via DMML node observations
}

//-----------------------------------------------------------------------------
vtkDMMLInteractionNode* qDMMLMarkupsToolBar::interactionNode()const
{
  Q_D(const qDMMLMarkupsToolBar);
  return d->InteractionNode;
}

//-----------------------------------------------------------------------------
vtkDMMLSelectionNode* qDMMLMarkupsToolBar::selectionNode()const
{
  Q_D(const qDMMLMarkupsToolBar);
  return d->SelectionNode;
}

//---------------------------------------------------------------------------
void qDMMLMarkupsToolBar::setPersistence(bool persistent)
{
  Q_D(qDMMLMarkupsToolBar);
  if (d->MarkupsPlaceWidget)
    {
    d->MarkupsPlaceWidget->setPlaceModePersistency(persistent ? true : false);
    }
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsToolBar::setInteractionNode(vtkDMMLInteractionNode* interactionNode)
{
  Q_D(qDMMLMarkupsToolBar);
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
void qDMMLMarkupsToolBar::setSelectionNode(vtkDMMLSelectionNode* selectionNode)
{
  Q_D(qDMMLMarkupsToolBar);

  if (d->SelectionNode == selectionNode)
    {
    return;
    }
  d->qvtkReconnect(d->SelectionNode, selectionNode, vtkCommand::ModifiedEvent,
    d, SLOT(updateWidgetFromDMML()));
  d->SelectionNode = selectionNode;
  d->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsToolBar::interactionModeActionTriggered(bool toggled)
{
  Q_D(qDMMLMarkupsToolBar);
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
}

//---------------------------------------------------------------------------
void qDMMLMarkupsToolBar::setApplicationLogic(vtkCjyxApplicationLogic* appLogic)
{
  Q_D(qDMMLMarkupsToolBar);
  d->DMMLAppLogic = appLogic;
}

//---------------------------------------------------------------------------
void qDMMLMarkupsToolBar::initializeToolBarLayout()
{
  Q_D(qDMMLMarkupsToolBar);

  vtkCjyxMarkupsLogic* markupsLogic =
    vtkCjyxMarkupsLogic::SafeDownCast(d->DMMLAppLogic->GetModuleLogic("Markups"));
  if (!markupsLogic)
    {
    qWarning() << Q_FUNC_INFO << " failed: invalid markups logic";
    return;
    }
  // Markups place widget
  d->MarkupsPlaceWidget = new qCjyxMarkupsPlaceWidget(this);
  d->MarkupsPlaceWidget->setObjectName(QString("MarkupsPlaceWidget"));
  d->MarkupsPlaceWidget->setDeleteAllControlPointsOptionVisible(true);
  d->MarkupsPlaceWidget->setPlaceMultipleMarkups(qCjyxMarkupsPlaceWidget::ShowPlaceMultipleMarkupsOption);
  d->MarkupsPlaceWidget->setDMMLScene(qCjyxApplication::application()->dmmlScene());
  this->addWidget(d->MarkupsPlaceWidget);
  connect(d->MarkupsPlaceWidget, SIGNAL(activeMarkupsPlaceModeChanged(bool)), this, SIGNAL(activeMarkupsPlaceModeChanged(bool)));

  // Module shortcuts
  this->addSeparator();
  d->addSetModuleButton(markupsLogic, "Markups");
  d->addSetModuleButton(markupsLogic, "Annotations");

  // Add event observers for registration/unregistration of markups
  this->qvtkConnect(markupsLogic, vtkCjyxMarkupsLogic::MarkupRegistered,
    this, SLOT(updateToolBarLayout()));
  this->qvtkConnect(markupsLogic, vtkCjyxMarkupsLogic::MarkupUnregistered,
    this, SLOT(updateToolBarLayout()));

  this->updateToolBarLayout();
}

//---------------------------------------------------------------------------
void qDMMLMarkupsToolBar::updateToolBarLayout()
{
  // Node creation buttons
  Q_D(qDMMLMarkupsToolBar);

  vtkCjyxMarkupsLogic* markupsLogic =
    vtkCjyxMarkupsLogic::SafeDownCast(d->DMMLAppLogic->GetModuleLogic("Markups"));
  if (!markupsLogic)
    {
    qWarning() << Q_FUNC_INFO << " failed: invalid markups logic";
    return;
    }

  for (const auto markupName : markupsLogic->GetRegisteredMarkupsTypes())
    {
    vtkDMMLMarkupsNode* markupsNode = markupsLogic->GetNodeByMarkupsType(markupName.c_str());
    if (markupsNode && markupsLogic->GetCreateMarkupsPushButton(markupName.c_str()))
      {
      bool buttonExists = false;
      for (int index=0; index< this->layout()->count(); index++)
        {
        std::string buttonName = this->layout()->itemAt(index)->widget()->objectName().toStdString();
        if (buttonName == "Create" + markupName + "PushButton")
          {
          buttonExists = true;
          break;
          }
        }
      if (!buttonExists)
        {
        QPushButton* markupCreateButton = new QPushButton();
        QSignalMapper* mapper = new QSignalMapper(markupCreateButton);
        std::string markupType = markupsNode->GetMarkupType() ? markupsNode->GetMarkupType() : "";
        std::string markupDisplayName = markupsNode->GetTypeDisplayName() ? markupsNode->GetTypeDisplayName() : "";
        markupCreateButton->setObjectName(QString::fromStdString("Create"+markupType+"PushButton"));
        markupCreateButton->setToolTip("Create new " + QString::fromStdString(markupDisplayName));
        markupCreateButton->setIcon(QIcon(markupsNode->GetPlaceAddIcon()));
        markupCreateButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        this->insertWidget(d->NodeSelectorAction, markupCreateButton);
        QObject::connect(markupCreateButton, SIGNAL(clicked()), mapper, SLOT(map()));
        mapper->setMapping(markupCreateButton, markupsNode->GetClassName());
        QObject::connect(mapper, SIGNAL(mapped(const QString&)), this, SLOT(onAddNewMarkupsNodeByClass(const QString&)));
        }
      }
    }

  for (int index = this->layout()->count()-1; index >=0 ; index--)
    {
    QString buttonName = this->layout()->itemAt(index)->widget()->objectName();
    if (!buttonName.startsWith("Create") || !buttonName.endsWith("PushButton"))
      {
      // Not a markup create button, leave it as is
      continue;
      }
    bool markupExists = false;
    for (const auto markupName : markupsLogic->GetRegisteredMarkupsTypes())
      {
      //QString markupButtonName = QString("Create%1PushButton").arg(QString::fromStdString(markupName));
      if (QString::fromStdString("Create"+markupName+"PushButton") == buttonName)
        {
        markupExists = true;
        break;
        }
      }
    if (markupExists)
      {
      // This button is still needed
      continue;
      }
    // Corresponding markup type is no longer available, delete this button
    QLayoutItem* item = this->layout()->takeAt(index);
    delete item->widget();
    delete item;
    }
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsToolBar::onAddNewMarkupsNodeByClass(const QString& className)
{
  Q_D(qDMMLMarkupsToolBar);
  // Add new markups node to the scene
  vtkDMMLMarkupsNode* markupsNode = nullptr;
  vtkCjyxMarkupsLogic* markupsLogic =
    vtkCjyxMarkupsLogic::SafeDownCast(d->DMMLAppLogic->GetModuleLogic("Markups"));
  if (markupsLogic)
    {
    markupsNode = markupsLogic->AddNewMarkupsNode(className.toStdString());
    }
  if (!markupsNode)
    {
    qCritical() << Q_FUNC_INFO << ": failed to create new markups node by class " << className;
    return;
    }
  // Update GUI
  d->updateWidgetFromDMML();
  if (d->MarkupsPlaceWidget)
    {
    d->MarkupsPlaceWidget->setPlaceModeEnabled(true);
    }
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsToolBar::onAddNewAnnotationNodeByClass(const QString& className)
{
  Q_D(qDMMLMarkupsToolBar);
  if (!this->selectionNode() || !this->interactionNode())
    {
    qCritical() << Q_FUNC_INFO << " failed: invalid selection or interaction node";
    return;
    }
  d->updateWidgetFromDMML();
  this->selectionNode()->SetReferenceActivePlaceNodeClassName(className.toUtf8());
  this->interactionNode()->SetCurrentInteractionMode(vtkDMMLInteractionNode::Place);
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsToolBar::onMarkupsNodeChanged(vtkDMMLNode* markupsNode)
{
  // called when the user selects a node on the toolbar
  Q_D(qDMMLMarkupsToolBar);
  this->setActiveMarkupsNode(vtkDMMLMarkupsNode::SafeDownCast(markupsNode));
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsToolBar::addCreateNodeShortcut(QString keySequence)
{
  QObject::connect(new QShortcut(QKeySequence(keySequence), this), SIGNAL(activated()), SLOT(onCreateNodeShortcut()));
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsToolBar::addTogglePersistenceShortcut(QString keySequence)
{
  QObject::connect(new QShortcut(QKeySequence(keySequence), this), SIGNAL(activated()), SLOT(onTogglePersistenceShortcut()));
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsToolBar::addPlacePointShortcut(QString keySequence)
{
  QObject::connect(new QShortcut(QKeySequence(keySequence), this), SIGNAL(activated()), SLOT(onPlacePointShortcut()));
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsToolBar::onCreateNodeShortcut()
{
  Q_D(qDMMLMarkupsToolBar);

  vtkDMMLMarkupsNode* currentMarkupsNode = vtkDMMLMarkupsNode::SafeDownCast(d->MarkupsNodeSelector->currentNode());
  QString className = d->DefaultPlaceClassName;
  if (currentMarkupsNode != nullptr && d->MarkupsPlaceWidget)
    {
    className = d->MarkupsPlaceWidget->currentNode()->GetClassName();
    }
  this->onAddNewMarkupsNodeByClass(className);
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsToolBar::onTogglePersistenceShortcut()
{
  Q_D(qDMMLMarkupsToolBar);
  if (!d->MarkupsPlaceWidget)
    {
    return;
    }
  bool persistent = d->MarkupsPlaceWidget->placeModePersistency();
  this->setPersistence(persistent ? false : true);
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsToolBar::onPlacePointShortcut()
{
  Q_D(qDMMLMarkupsToolBar);
  if (!d->MarkupsPlaceWidget)
    {
    return;
    }
  bool placeModeActive = d->MarkupsPlaceWidget->placeModeEnabled();
  d->MarkupsPlaceWidget->setPlaceModeEnabled(placeModeActive ? false : true);
}
