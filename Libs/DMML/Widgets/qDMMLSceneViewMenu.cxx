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
#include <QPushButton>

// CTK includes
#include <ctkMessageBox.h>

// qDMML includes
#include "qDMMLSceneViewMenu_p.h"

// DMML includes
#include <vtkDMMLSceneViewNode.h>
#include <vtkDMMLMessageCollection.h>

//--------------------------------------------------------------------------
// qDMMLSceneViewMenuPrivate methods

//---------------------------------------------------------------------------
qDMMLSceneViewMenuPrivate::qDMMLSceneViewMenuPrivate(qDMMLSceneViewMenu& object)
  : Superclass(&object), q_ptr(&object)
{
  connect(&this->RestoreActionMapper, SIGNAL(mapped(QString)), SLOT(restoreSceneView(QString)));
  connect(&this->DeleteActionMapper, SIGNAL(mapped(QString)), SLOT(deleteSceneView(QString)));
}

// --------------------------------------------------------------------------
void qDMMLSceneViewMenuPrivate::resetMenu()
{
  Q_Q(qDMMLSceneViewMenu);
  Q_ASSERT(this->DMMLScene);

  // Clear menu
  q->clear();

  QAction * noSceneViewAction = q->addAction(this->NoSceneViewText);
  noSceneViewAction->setDisabled(true);

  // Loop over sceneView nodes and associated menu entry
  const char* className = "vtkDMMLSceneViewNode";
  int nnodes = this->DMMLScene->GetNumberOfNodesByClass(className);
  for (int n = 0; n < nnodes; n++)
    {
    this->addMenuItem(this->DMMLScene->GetNthNodeByClass(n, className));
    }
}

// --------------------------------------------------------------------------
void qDMMLSceneViewMenuPrivate::onDMMLNodeAdded(vtkObject* dmmlScene, vtkObject * dmmlNode)
{
  Q_UNUSED(dmmlScene);
  vtkDMMLSceneViewNode * sceneViewNode = vtkDMMLSceneViewNode::SafeDownCast(dmmlNode);
  if (!sceneViewNode)
    {
    return;
    }

  this->addMenuItem(sceneViewNode);
}

// --------------------------------------------------------------------------
void qDMMLSceneViewMenuPrivate::addMenuItem(vtkDMMLNode * sceneViewNode)
{
  Q_Q(qDMMLSceneViewMenu);
  vtkDMMLSceneViewNode * node = vtkDMMLSceneViewNode::SafeDownCast(sceneViewNode);
  if (!node)
    {
    return;
    }

  // Reload the menu each time a sceneView node is modified, if there
  // are performance issues, the relation between menu item and sceneView should
  // be tracked either using a QModel or a QHash
  this->qvtkReconnect(sceneViewNode, vtkCommand::ModifiedEvent,
                      this, SLOT(onDMMLSceneViewNodeModified(vtkObject*)));

  if (this->hasNoSceneViewItem())
    {
    q->clear();
    }

  QMenu* sceneViewMenu = q->addMenu(QString::fromUtf8(node->GetName()));
  sceneViewMenu->setObjectName("sceneViewMenu");

  QAction* restoreAction = sceneViewMenu->addAction(QIcon(":/Icons/SnapshotRestore.png"), "Restore",
                                                   &this->RestoreActionMapper, SLOT(map()));
  this->RestoreActionMapper.setMapping(restoreAction, QString::fromUtf8(node->GetID()));

  QAction* deleteAction = sceneViewMenu->addAction(QIcon(":/Icons/SnapshotDelete.png"), "Delete",
                                                  &this->DeleteActionMapper, SLOT(map()));
  this->DeleteActionMapper.setMapping(deleteAction, QString::fromUtf8(node->GetID()));
}

// --------------------------------------------------------------------------
void qDMMLSceneViewMenuPrivate::onDMMLNodeRemoved(vtkObject* dmmlScene, vtkObject * dmmlNode)
{
  Q_UNUSED(dmmlScene);
  vtkDMMLSceneViewNode * sceneViewNode = vtkDMMLSceneViewNode::SafeDownCast(dmmlNode);
  if (!sceneViewNode)
    {
    return;
    }

  this->removeMenuItem(sceneViewNode);
}

// --------------------------------------------------------------------------
void qDMMLSceneViewMenuPrivate::removeMenuItem(vtkDMMLNode * sceneViewNode)
{
  Q_Q(qDMMLSceneViewMenu);
  vtkDMMLSceneViewNode * node = vtkDMMLSceneViewNode::SafeDownCast(sceneViewNode);
  if (!node)
    {
    return;
    }

  // Do not listen for ModifiedEvent anymore
  this->qvtkDisconnect(sceneViewNode, vtkCommand::ModifiedEvent,
                       this, SLOT(onDMMLSceneViewNodeModified(vtkObject*)));

  QList<QAction*> actions = q->actions();
  foreach(QAction * action, actions)
    {
    if (action->text().compare(QString::fromUtf8(node->GetName())) == 0)
      {
      q->removeAction(action);
      break;
      }
    }

  if (q->actions().isEmpty())
    {
    QAction * noSceneViewAction = q->addAction(this->NoSceneViewText);
    noSceneViewAction->setDisabled(true);
    }
}

// --------------------------------------------------------------------------
void qDMMLSceneViewMenuPrivate::onDMMLSceneViewNodeModified(vtkObject * sceneViewNode)
{
  vtkDMMLSceneViewNode * node = vtkDMMLSceneViewNode::SafeDownCast(sceneViewNode);
  if (!node)
    {
    return;
    }

  this->resetMenu();
}

// --------------------------------------------------------------------------
bool qDMMLSceneViewMenuPrivate::hasNoSceneViewItem()const
{
  Q_Q(const qDMMLSceneViewMenu);
  QList<QAction*> actions = q->actions();
  Q_ASSERT(actions.count() > 0); // At least one item is expected
  return (actions.at(0)->text().compare(this->NoSceneViewText) == 0);
}

// --------------------------------------------------------------------------
void qDMMLSceneViewMenuPrivate::restoreSceneView(const QString& sceneViewNodeId)
{
  vtkDMMLSceneViewNode * sceneViewNode = vtkDMMLSceneViewNode::SafeDownCast(
      this->DMMLScene->GetNodeByID(sceneViewNodeId.toUtf8()));
  Q_ASSERT(sceneViewNode);
  this->DMMLScene->SaveStateForUndo();
  // pass false to not delete nodes from the scene
  vtkNew<vtkDMMLMessageCollection> userMessages;
  userMessages->SetObservedObject(sceneViewNode);
  sceneViewNode->RestoreScene(false);
  userMessages->SetObservedObject(nullptr);
  if (userMessages->GetNumberOfMessagesOfType(vtkCommand::ErrorEvent)>0)
    {
    QString errorMsg = QString::fromStdString(userMessages->GetAllMessagesAsString());

    // ask the user if they wish to continue removing the node(s) or
    // add the missing nodes to the scene view
    ctkMessageBox missingNodesMsgBox;
    missingNodesMsgBox.setWindowTitle("Data missing from Scene View");
    QString sceneViewName = QString(sceneViewNode->GetName());
    QString labelText = QString("Add data to scene view \"")
      + sceneViewName
      + QString("\" before restoring?\n"
                "\n");
    QString infoText = QString(
      "Data is present in the current scene but not in the scene view.\n"
      "\n"
      "If you don't add and restore, data not already saved to disk"
      ", or saved in another scene view,"
      " will be permanently lost!\n");
    missingNodesMsgBox.setText(labelText + infoText);
    // until CTK bug is fixed, informative text will overlap the don't show
    // again message so put it all in the label text
    // missingNodesMsgBox.setInformativeText(infoText);
    QPushButton *continueButton = missingNodesMsgBox.addButton(QMessageBox::Discard);
    continueButton->setText("Restore without saving");
    QPushButton *addButton = missingNodesMsgBox.addButton(QMessageBox::Save);
    addButton->setText("Add and Restore");
    missingNodesMsgBox.addButton(QMessageBox::Cancel);

    missingNodesMsgBox.setIcon(QMessageBox::Warning);
    missingNodesMsgBox.setDontShowAgainVisible(true);
    missingNodesMsgBox.setDontShowAgainSettingsKey("SceneViewMenu/AlwaysRemoveNodes");
    int ret = missingNodesMsgBox.exec();
    switch (ret)
      {
      case QMessageBox::Discard:
        sceneViewNode->RestoreScene(true);
        break;
      case QMessageBox::Save:
        sceneViewNode->AddMissingNodes();
        // try to restore again
        this->restoreSceneView(sceneViewNode->GetID());
        break;
      case QMessageBox::Cancel:
      default:
        break;
      }
    }
}

// --------------------------------------------------------------------------
void qDMMLSceneViewMenuPrivate::deleteSceneView(const QString& sceneViewNodeId)
{
  vtkDMMLSceneViewNode * sceneViewNode = vtkDMMLSceneViewNode::SafeDownCast(
      this->DMMLScene->GetNodeByID(sceneViewNodeId.toUtf8()));
  Q_ASSERT(sceneViewNode);
  this->DMMLScene->SaveStateForUndo();
  this->DMMLScene->RemoveNode(sceneViewNode);
}

// --------------------------------------------------------------------------
// qDMMLSceneViewMenu methods

// --------------------------------------------------------------------------
qDMMLSceneViewMenu::qDMMLSceneViewMenu(QWidget* newParent) : Superclass(newParent)
  , d_ptr(new qDMMLSceneViewMenuPrivate(*this))
{
  Q_D(qDMMLSceneViewMenu);
  d->NoSceneViewText = tr("No scene views");
}

// --------------------------------------------------------------------------
qDMMLSceneViewMenu::~qDMMLSceneViewMenu() = default;

//-----------------------------------------------------------------------------
void qDMMLSceneViewMenu::setDMMLScene(vtkDMMLScene* scene)
{
  Q_D(qDMMLSceneViewMenu);
  if (scene == d->DMMLScene)
    {
    return ;
    }

  qvtkReconnect(d->DMMLScene, scene,
                vtkDMMLScene::NodeAddedEvent, d, SLOT(onDMMLNodeAdded(vtkObject*,vtkObject*)));

  qvtkReconnect(d->DMMLScene, scene,
                vtkDMMLScene::NodeRemovedEvent, d, SLOT(onDMMLNodeRemoved(vtkObject*,vtkObject*)));

  d->DMMLScene = scene;
  emit dmmlSceneChanged(scene);

  if (d->DMMLScene)
    {
    d->resetMenu();
    }
}

//-----------------------------------------------------------------------------
vtkDMMLScene* qDMMLSceneViewMenu::dmmlScene() const
{
  Q_D(const qDMMLSceneViewMenu);
  return d->DMMLScene;
}

//-----------------------------------------------------------------------------
QString qDMMLSceneViewMenu::noSceneViewText()const
{
  Q_D(const qDMMLSceneViewMenu);
  return d->NoSceneViewText;
}

//-----------------------------------------------------------------------------
void qDMMLSceneViewMenu::setNoSceneViewText(const QString& newText)
{
  Q_D(qDMMLSceneViewMenu);
  d->NoSceneViewText = newText;
}
