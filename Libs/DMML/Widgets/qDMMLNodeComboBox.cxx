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
#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QKeyEvent>
#include <QListView>

// CTK includes
#include <ctkComboBox.h>
#include <ctkUtils.h>

// DMMLWidgets includes
#include "qDMMLNodeComboBoxDelegate.h"
#include "qDMMLNodeComboBoxMenuDelegate.h"
#include "qDMMLNodeComboBox_p.h"
#include "qDMMLNodeFactory.h"
#include "qDMMLSceneModel.h"

// DMML includes
#include <vtkDMMLInteractionNode.h>
#include <vtkDMMLNode.h>
#include <vtkDMMLScene.h>

// --------------------------------------------------------------------------
qDMMLNodeComboBoxPrivate::qDMMLNodeComboBoxPrivate(qDMMLNodeComboBox& object)
  : q_ptr(&object)
{
  this->ComboBox = nullptr;
  this->DMMLNodeFactory = nullptr;
  this->DMMLSceneModel = nullptr;
  this->NoneEnabled = false;
  this->AddEnabled = true;
  this->RemoveEnabled = true;
  this->EditEnabled = false;
  // "Singleton" is the default tag for the interaction singleton node
  this->InteractionNodeSingletonTag = "Singleton";
  this->RenameEnabled = false;

  this->SelectNodeUponCreation = true;
  this->NoneDisplay = qDMMLNodeComboBox::tr("None");
  this->AutoDefaultText = true;

  this->CallBack = vtkSmartPointer<vtkCallbackCommand>::New();
}

// --------------------------------------------------------------------------
qDMMLNodeComboBoxPrivate::~qDMMLNodeComboBoxPrivate()
{
  if (this->DMMLScene)
    {
    this->DMMLScene->RemoveObserver(this->CallBack);
    }
}

// --------------------------------------------------------------------------
void qDMMLNodeComboBoxPrivate::init(QAbstractItemModel* model)
{
  Q_Q(qDMMLNodeComboBox);
  Q_ASSERT(this->DMMLNodeFactory == nullptr);

  q->setLayout(new QHBoxLayout);
  q->layout()->setContentsMargins(0,0,0,0);
  q->setSizePolicy(QSizePolicy(QSizePolicy::Preferred,
                               QSizePolicy::Fixed,
                               QSizePolicy::ComboBox));

  if (this->ComboBox == nullptr)
    {
    ctkComboBox* comboBox = new ctkComboBox(q);
    comboBox->setElideMode(Qt::ElideMiddle);
    q->setComboBox(comboBox);
    }
  else
    {
    QComboBox* comboBox = this->ComboBox;
    this->ComboBox = nullptr;
    q->setComboBox(comboBox);
    }

  this->DMMLNodeFactory = new qDMMLNodeFactory(q);

  QAbstractItemModel* rootModel = model;
  while (qobject_cast<QAbstractProxyModel*>(rootModel) &&
         qobject_cast<QAbstractProxyModel*>(rootModel)->sourceModel())
    {
    rootModel = qobject_cast<QAbstractProxyModel*>(rootModel)->sourceModel();
    }
  this->DMMLSceneModel = qobject_cast<qDMMLSceneModel*>(rootModel);
  Q_ASSERT(this->DMMLSceneModel);
  // no need to reset the root model index here as the model is not yet set
  this->updateNoneItem(false);
  this->updateActionItems(false);

  qDMMLSortFilterProxyModel* sortFilterModel = new qDMMLSortFilterProxyModel(q);
  sortFilterModel->setSourceModel(model);
  this->setModel(sortFilterModel);

  // nodeTypeLabel() works only when the model is set.
  this->updateDefaultText();

  this->CallBack->SetClientData(this);
  this->CallBack->SetCallback(qDMMLNodeComboBoxPrivate::onDMMLSceneEvent);

  q->setEnabled(q->dmmlScene() != nullptr);
}

//-----------------------------------------------------------------------------
void qDMMLNodeComboBoxPrivate::onDMMLSceneEvent(vtkObject* vtk_obj, unsigned long event,
  void* client_data, void* call_data)
{
  Q_UNUSED(vtk_obj);
  Q_UNUSED(call_data);
  qDMMLNodeComboBoxPrivate* self = reinterpret_cast<qDMMLNodeComboBoxPrivate*>(client_data);
  if (!self)
    {
    return;
    }
  if (event == vtkDMMLScene::NodeClassRegisteredEvent)
    {
    self->updateDefaultText();
    self->updateNoneItem(false);
    self->updateActionItems(false);
    }
}

// --------------------------------------------------------------------------
void qDMMLNodeComboBoxPrivate::setModel(QAbstractItemModel* model)
{
  Q_Q(qDMMLNodeComboBox);
  if (model == nullptr)
    {// it's invalid to set a null model to a combobox
    return;
    }
  if (this->ComboBox->model() != model)
    {
    this->ComboBox->setModel(model);
    }
  q->connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
             q, SLOT(emitNodesAdded(QModelIndex,int,int)));
  q->connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
             q, SLOT(emitNodesAboutToBeRemoved(QModelIndex,int,int)));
  q->connect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
             q, SLOT(refreshIfCurrentNodeHidden()));
  q->connect(model, SIGNAL(modelReset()), q, SLOT(refreshIfCurrentNodeHidden()));
  q->connect(model, SIGNAL(layoutChanged()), q, SLOT(refreshIfCurrentNodeHidden()));
}

// --------------------------------------------------------------------------
vtkDMMLNode* qDMMLNodeComboBoxPrivate::dmmlNode(int row)const
{
  QModelIndex modelIndex;
  if (qobject_cast<QListView*>(this->ComboBox->view()))
    {
    modelIndex  = this->ComboBox->model()->index(
      row, this->ComboBox->modelColumn(), this->ComboBox->rootModelIndex());
    }
  else
    {// special case where the view can handle a tree... currentIndex could be
    // from any parent, not only a top level..
    modelIndex = this->ComboBox->view()->currentIndex();
    modelIndex = this->ComboBox->model()->index(
      row, this->ComboBox->modelColumn(), modelIndex.parent());
    }
  /*
  Q_Q(const qDMMLNodeComboBox);
  QString nodeId =
    this->ComboBox->itemData(index, qDMMLSceneModel::UIDRole).toString();
  if (nodeId.isEmpty())
    {
    return 0;
    }
  vtkDMMLScene* scene = q->dmmlScene();
  return scene ? scene->GetNodeByID(nodeId.toUtf8()) : 0;
  */
  return this->dmmlNodeFromIndex(modelIndex);
}

// --------------------------------------------------------------------------
vtkDMMLNode* qDMMLNodeComboBoxPrivate::dmmlNodeFromIndex(const QModelIndex& index)const
{
  Q_Q(const qDMMLNodeComboBox);
  Q_ASSERT(q->model());
  QString nodeId =
    this->ComboBox->model()->data(index, qDMMLSceneModel::UIDRole).toString();
  if (nodeId.isEmpty())
    {
    return nullptr;
    }
  vtkDMMLScene* scene = q->dmmlScene();
  return scene ? scene->GetNodeByID(nodeId.toUtf8()) : nullptr;
}

// --------------------------------------------------------------------------
QModelIndexList qDMMLNodeComboBoxPrivate::indexesFromDMMLNodeID(const QString& nodeID)const
{
  return this->ComboBox->model()->match(
    this->ComboBox->model()->index(0, 0), qDMMLSceneModel::UIDRole, nodeID, 1,
    Qt::MatchRecursive | Qt::MatchExactly | Qt::MatchWrap);
}

// --------------------------------------------------------------------------
void qDMMLNodeComboBoxPrivate::updateDefaultText()
{
  Q_Q(const qDMMLNodeComboBox);
  if (!this->AutoDefaultText)
    {
    return;
    }
  ctkComboBox* cb = qobject_cast<ctkComboBox*>(this->ComboBox);
  if (cb)
    {
    // Use the first node type label to give a hint to the user
    // what kind of node is expected
    QString nodeType;
    QStringList nodeTypes = q->nodeTypes();
    if (!nodeTypes.empty())
      {
      nodeType = nodeTypes[0];
      }
    cb->setDefaultText(qDMMLNodeComboBox::tr("Select a ") + q->nodeTypeLabel(nodeType));
    }
}

// --------------------------------------------------------------------------
void qDMMLNodeComboBoxPrivate::updateNoneItem(bool resetRootIndex)
{
  Q_UNUSED(resetRootIndex);
  //Q_Q(qDMMLNodeComboBox);
  QStringList noneItem;
  if (this->NoneEnabled)
    {
    noneItem.append(this->NoneDisplay);
    }
  //QVariant currentNode =
  //  this->ComboBox->itemData(this->ComboBox->currentIndex(), qDMMLSceneModel::UIDRole);
  //qDebug() << "updateNoneItem: " << this->DMMLSceneModel->dmmlSceneItem();
  if (this->DMMLSceneModel->dmmlSceneItem())
    {
    this->DMMLSceneModel->setPreItems(noneItem, this->DMMLSceneModel->dmmlSceneItem());
    }
/*  if (resetRootIndex)
    {
    this->ComboBox->setRootModelIndex(q->model()->index(0, 0));
    // setting the rootmodel index looses the current item
    // try to set the current item back
    q->setCurrentNode(currentNode.toString());
    }
*/
}

// --------------------------------------------------------------------------
void qDMMLNodeComboBoxPrivate::updateActionItems(bool resetRootIndex)
{
  Q_Q(qDMMLNodeComboBox);
  Q_UNUSED(resetRootIndex);


  QStringList extraItems;
  if (q->dmmlScene())
    {
    // Action items are not updated when selection is changed, therefore use the actual
    // node type label if there is only one type and use a generic name if there are multiple node types (or none)
    QString nodeType;
    QStringList nodeTypes = q->nodeTypes();
    if (nodeTypes.size()==1)
      {
      nodeType = nodeTypes[0];
      }
    QString label = q->nodeTypeLabel(nodeType);

    if (this->AddEnabled || this->RemoveEnabled || this->EditEnabled
        || this->RenameEnabled || !this->UserMenuActions.empty())
      {
      extraItems.append("separator");
      }
    if (this->RenameEnabled)
      {
      extraItems.append(qDMMLNodeComboBox::tr("Rename current ")  + label);
      }
    if (this->EditEnabled)
      {
      extraItems.append(qDMMLNodeComboBox::tr("Edit current ")  + label);
      }
    if (this->AddEnabled)
      {
      foreach (QString nodeType, q->nodeTypes())
        {
        QString label = q->nodeTypeLabel(nodeType);
        extraItems.append(qDMMLNodeComboBox::tr("Create new ") + label);
        if (this->RenameEnabled)
          {
          extraItems.append(qDMMLNodeComboBox::tr("Create new ") + label + qDMMLNodeComboBox::tr(" as..."));
          }
        }
      }
    if (this->RemoveEnabled)
      {
      extraItems.append(qDMMLNodeComboBox::tr("Delete current ")  + label);
      }
    foreach (QAction *action, this->UserMenuActions)
      {
      extraItems.append(action->text());
      }
    }

  // setPostItems inserts rows, which changes selection if selection was previously invalid (-1).
  // Since NoneDisplay is only shown if selection is -1, we save and restore the current index.
  int currentIndex = this->ComboBox->currentIndex();
  this->DMMLSceneModel->setPostItems(extraItems, this->DMMLSceneModel->dmmlSceneItem());
  this->ComboBox->setCurrentIndex(currentIndex);

  QObject::connect(this->ComboBox->view(), SIGNAL(clicked(QModelIndex)),
                   q, SLOT(activateExtraItem(QModelIndex)),
                   Qt::UniqueConnection);
  /*
  if (resetRootIndex)
    {
    this->ComboBox->setRootModelIndex(q->model()->index(0, 0));
    // setting the rootmodel index looses the current item
    // try to set the current item back
    q->setCurrentNode(currentNode.toString());
    }
  */
}

//--------------------------------------------------------------------------
void qDMMLNodeComboBoxPrivate::updateDelegate(bool force)
{
  Q_Q(qDMMLNodeComboBox);
  QStyleOptionComboBox opt;
  opt.editable = this->ComboBox->isEditable();

  if (this->ComboBox->style()->styleHint(
      QStyle::SH_ComboBox_Popup, &opt, this->ComboBox))
    {
      if (force ||
          qobject_cast<qDMMLNodeComboBoxDelegate *>(this->ComboBox->itemDelegate()))
        {
        this->ComboBox->setItemDelegate(
            new qDMMLNodeComboBoxMenuDelegate(q->parent(), q->comboBox()));
        }
    }
  else
    {
      if (force ||
          qobject_cast<qDMMLNodeComboBoxMenuDelegate *>(this->ComboBox->itemDelegate()))
        {
        this->ComboBox->setItemDelegate(
            new qDMMLNodeComboBoxDelegate(q->parent(), q->comboBox()));
        }
    }
}

// --------------------------------------------------------------------------
bool qDMMLNodeComboBoxPrivate::hasPostItem(const QString& name)const
{
  foreach(const QString& item,
          this->DMMLSceneModel->postItems(this->DMMLSceneModel->dmmlSceneItem()))
    {
    if (item.startsWith(name))
      {
      return true;
      }
    }
  return false;
}

// --------------------------------------------------------------------------
// qDMMLNodeComboBox

// --------------------------------------------------------------------------
qDMMLNodeComboBox::qDMMLNodeComboBox(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qDMMLNodeComboBoxPrivate(*this))
{
  Q_D(qDMMLNodeComboBox);
  d->init(new qDMMLSceneModel(this));
}

// --------------------------------------------------------------------------
qDMMLNodeComboBox::qDMMLNodeComboBox(QAbstractItemModel* sceneModel, QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qDMMLNodeComboBoxPrivate(*this))
{
  Q_D(qDMMLNodeComboBox);
  d->init(sceneModel);
}

// --------------------------------------------------------------------------
qDMMLNodeComboBox::qDMMLNodeComboBox(qDMMLNodeComboBoxPrivate* pimpl, QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(pimpl)
{
  Q_D(qDMMLNodeComboBox);
  d->init(new qDMMLSceneModel(this));
}

// --------------------------------------------------------------------------
qDMMLNodeComboBox::~qDMMLNodeComboBox() = default;

// --------------------------------------------------------------------------
void qDMMLNodeComboBox::activateExtraItem(const QModelIndex& index)
{
  Q_D(qDMMLNodeComboBox);
  // FIXME: check the type of the item on a different role instead of the display role
  QString data = this->model()->data(index, Qt::DisplayRole).toString();
  if (d->AddEnabled && data.startsWith(tr("Create new ")) && !data.endsWith(tr(" as...")))
    {
    QString label = data.right(data.length()-tr("Create new ").length());
    QString nodeTypeName;
    foreach (QString nodeType, this->nodeTypes())
      {
      QString foundLabel = this->nodeTypeLabel(nodeType);
      if (foundLabel==label)
        {
        nodeTypeName = nodeType;
        }
      }
    d->ComboBox->hidePopup();
    this->addNode(nodeTypeName);
    }
  else if (d->RemoveEnabled && data.startsWith(tr("Delete current ")))
    {
    d->ComboBox->hidePopup();
    this->removeCurrentNode();
    }
  else if (d->EditEnabled && data.startsWith(tr("Edit current ")))
    {
    d->ComboBox->hidePopup();
    this->editCurrentNode();
    }
  else if (d->RenameEnabled && data.startsWith(tr("Rename current ")))
    {
    d->ComboBox->hidePopup();
    this->renameCurrentNode();
    }
  else if (d->RenameEnabled && d->AddEnabled
           && data.startsWith(tr("Create new ")) && data.endsWith(tr(" as...")))
    {
    // Get the node type label by stripping "Create new" and "as..." from left and right
    QString label = data.mid(tr("Create new ").length(), data.length()-tr("Create new ").length()-tr(" as...").length());
    QString nodeTypeName;
    foreach (QString nodeType, this->nodeTypes())
      {
      QString foundLabel = this->nodeTypeLabel(nodeType);
      if (foundLabel==label)
        {
        nodeTypeName = nodeType;
        }
      }
    d->ComboBox->hidePopup();
    this->addNode(nodeTypeName);
    this->renameCurrentNode();
    }
  else
    {
    // check for user added items
    foreach (QAction *action, d->UserMenuActions)
      {
      if (data.startsWith(action->text()))
        {
        d->ComboBox->hidePopup();
        action->trigger();
        break;
        }
      }
    }

}

//-----------------------------------------------------------------------------
void qDMMLNodeComboBox::addAttribute(const QString& nodeType,
                                     const QString& attributeName,
                                     const QVariant& attributeValue)
{
  Q_D(qDMMLNodeComboBox);

  // Add a warning to make it easier to detect issue in obsolete modules that have not been updated
  // since the "LabelMap" attribute was replaced by vtkDMMLLabelMapVolumeNode.
  // The "LabelMap" attribute filter is not applied to make obsolete modules still usable (if we
  // applied the filter then no volume would show up in the selector; as we ignore it both scalar
  // and labelmap volumes show up, which is just a slight inconvenience).
  // Probably this check can be removed by Summer 2016 (one year after the vtkDMMLLabelMapVolumeNode
  // was introduced).
  if (nodeType=="vtkDMMLScalarVolumeNode" && attributeName=="LabelMap")
  {
    qWarning("vtkDMMLScalarVolumeNode does not have a LabelMap attribute anymore. Update your code according to "
      "https://www.slicer.org/w/index.php/Documentation/Labs/Segmentations#Module_update_instructions");
    return;
  }

  d->DMMLNodeFactory->addAttribute(attributeName, attributeValue.toString());
  this->sortFilterProxyModel()->addAttribute(nodeType, attributeName, attributeValue);
}

//-----------------------------------------------------------------------------
void qDMMLNodeComboBox::removeAttribute(const QString& nodeType,
                                     const QString& attributeName)
{
  Q_D(qDMMLNodeComboBox);

  d->DMMLNodeFactory->removeAttribute(attributeName);
  this->sortFilterProxyModel()->removeAttribute(nodeType, attributeName);
}

//-----------------------------------------------------------------------------
void qDMMLNodeComboBox::setBaseName(const QString& baseName, const QString& nodeType /* ="" */ )
{
  Q_D(qDMMLNodeComboBox);
  if (!nodeType.isEmpty())
    {
    d->DMMLNodeFactory->setBaseName(nodeType, baseName);
    return;
    }
  // If no node type is defined then we set the base name for all already specified node types
  QStringList nodeTypes = this->nodeTypes();
  if (nodeTypes.isEmpty())
    {
    qWarning("qDMMLNodeComboBox::setBaseName failed: no node types have been set yet");
    return;
    }
  foreach (QString aNodeType, nodeTypes)
    {
    d->DMMLNodeFactory->setBaseName(aNodeType, baseName);
    }
}

//-----------------------------------------------------------------------------
QString qDMMLNodeComboBox::baseName(const QString& nodeType /* ="" */ )const
{
  Q_D(const qDMMLNodeComboBox);
  if (!nodeType.isEmpty())
    {
    return d->DMMLNodeFactory->baseName(nodeType);
    }
  // If nodeType is not specified then base name of the first node type is returned.
  QStringList nodeClasses = this->nodeTypes();
  if (nodeClasses.isEmpty())
    {
    qWarning("qDMMLNodeComboBox::baseName failed: no node types have been set yet");
    return QString();
    }
  return d->DMMLNodeFactory->baseName(nodeClasses[0]);
}

//-----------------------------------------------------------------------------
void qDMMLNodeComboBox::setNodeTypeLabel(const QString& label, const QString& nodeType)
{
  Q_D(qDMMLNodeComboBox);
  if (nodeType.isEmpty())
    {
    qWarning() << Q_FUNC_INFO << " failed: nodeType is invalid";
    return;
    }
  if (label.isEmpty())
    {
    d->NodeTypeLabels.remove(nodeType);
    }
  else
    {
    d->NodeTypeLabels[nodeType] = label;
    }
  d->updateDefaultText();
  d->updateActionItems();
}

//-----------------------------------------------------------------------------
QString qDMMLNodeComboBox::nodeTypeLabel(const QString& nodeType)const
{
  Q_D(const qDMMLNodeComboBox);
  // If a label was explicitly specified then use that
  if (d->NodeTypeLabels.contains(nodeType))
    {
    return d->NodeTypeLabels[nodeType];
    }
  // Otherwise use the node tag
  if (this->dmmlScene())
    {
    QString label = QString::fromStdString(this->dmmlScene()->GetTypeDisplayNameByClassName(nodeType.toStdString()));
    if (!label.isEmpty())
      {
      return label;
      }
    }
  // Otherwise just label the node as "node"
  return tr("node");
}

// --------------------------------------------------------------------------
vtkDMMLNode* qDMMLNodeComboBox::addNode(QString nodeType)
{
  Q_D(qDMMLNodeComboBox);
  if (!this->nodeTypes().contains(nodeType))
    {
    qWarning("qDMMLNodeComboBox::addNode() attempted with node type %s, which is not among the allowed node types", qPrintable(nodeType));
    return nullptr;
    }
  // Create the DMML node via the DMML Scene
  vtkDMMLNode * newNode = d->DMMLNodeFactory->createNode(nodeType);
  // The created node is appended at the bottom of the current list
  if (newNode==nullptr)
    {
    qWarning("qDMMLNodeComboBox::addNode() failed with node type %s", qPrintable(nodeType));
    return nullptr;
    }
  if (this->selectNodeUponCreation())
    {// select the created node.
    this->setCurrentNode(newNode);
    }
  emit this->nodeAddedByUser(newNode);
  return newNode;
}

// --------------------------------------------------------------------------
vtkDMMLNode* qDMMLNodeComboBox::addNode()
{
  if (this->nodeTypes().isEmpty())
    {
    return nullptr;
    }
  return this->addNode(this->nodeTypes()[0]);
}

// --------------------------------------------------------------------------
vtkDMMLNode* qDMMLNodeComboBox::currentNode()const
{
  Q_D(const qDMMLNodeComboBox);
  return d->dmmlNode(d->ComboBox->currentIndex());
}

// --------------------------------------------------------------------------
QString qDMMLNodeComboBox::currentNodeID()const
{
  vtkDMMLNode* node = this->currentNode();
  return node ? node->GetID() : "";
}

// --------------------------------------------------------------------------
QString qDMMLNodeComboBox::currentNodeId()const
{
  qWarning() << "This function is deprecated. Use currentNodeID() instead";
  return this->currentNodeID();
}

// --------------------------------------------------------------------------
void qDMMLNodeComboBox::editCurrentNode()
{
  Q_D(const qDMMLNodeComboBox);
  vtkDMMLNode* node = this->currentNode();
  emit this->nodeAboutToBeEdited(node);

  if (!d->InteractionNodeSingletonTag.isEmpty())
    {
    vtkDMMLInteractionNode* interactionNode = vtkDMMLInteractionNode::SafeDownCast(
      this->dmmlScene()->GetSingletonNode(d->InteractionNodeSingletonTag.toUtf8(), "vtkDMMLInteractionNode"));
    if (interactionNode)
      {
      interactionNode->EditNode(node);
      }
    else
      {
      qWarning() << Q_FUNC_INFO << " failed: interaction node not found with singleton tag " << d->InteractionNodeSingletonTag;
      }
    }
}

// --------------------------------------------------------------------------
void qDMMLNodeComboBox::renameCurrentNode()
{
  vtkDMMLNode* node = this->currentNode();
  if (!node)
    {
    return;
    }

  bool ok = false;
  QString newName = QInputDialog::getText(
    this, "Rename " + this->nodeTypeLabel(node->GetClassName()), "New name:",
    QLineEdit::Normal, node->GetName(), &ok);
  if (!ok)
    {
    return;
    }
  node->SetName(newName.toUtf8());
  emit currentNodeRenamed(newName);
}

// --------------------------------------------------------------------------
void qDMMLNodeComboBox::emitCurrentNodeChanged()
{
  Q_D(qDMMLNodeComboBox);
  int currentIndex = d->ComboBox->currentIndex();
  vtkDMMLNode*  node = d->dmmlNode(currentIndex);
  if (!node && ((!d->NoneEnabled &&currentIndex != -1) || (d->NoneEnabled && currentIndex != 0)) )
    {
    // we only set the current node if the new selected is different
    // (not nullptr) to avoid warning in QAccessibleTable::child
    vtkDMMLNode* newSelectedNode = this->nodeFromIndex(this->nodeCount() - 1);
    if (newSelectedNode)
      {
      this->setCurrentNode(newSelectedNode);
      }
    }
  else
    {
    emit currentNodeChanged(node);
    emit currentNodeChanged(node != nullptr);
    emit currentNodeIDChanged(node ? node->GetID() : "");
    }
}

// --------------------------------------------------------------------------
void qDMMLNodeComboBox::emitNodeActivated(int currentIndex)
{
  Q_D(qDMMLNodeComboBox);
  vtkDMMLNode*  node = d->dmmlNode(currentIndex);
  // Fire only if the user clicked on a node or "None", don't fire the signal
  // if the user clicked on an "action" (post item) like "Add Node".
  if (node || (d->NoneEnabled && currentIndex == 0))
    {
    emit nodeActivated(node);
    }
}
// --------------------------------------------------------------------------
vtkDMMLScene* qDMMLNodeComboBox::dmmlScene()const
{
  Q_D(const qDMMLNodeComboBox);
  return d->DMMLSceneModel->dmmlScene();
}

// --------------------------------------------------------------------------
int qDMMLNodeComboBox::nodeCount()const
{
  Q_D(const qDMMLNodeComboBox);
  int extraItemsCount =
    d->DMMLSceneModel->preItems(d->DMMLSceneModel->dmmlSceneItem()).count()
    + d->DMMLSceneModel->postItems(d->DMMLSceneModel->dmmlSceneItem()).count();
  //qDebug() << d->DMMLSceneModel->invisibleRootItem() << d->DMMLSceneModel->dmmlSceneItem() << d->ComboBox->count() <<extraItemsCount;
  //printStandardItem(d->DMMLSceneModel->invisibleRootItem(), "  ");
  //qDebug() << d->ComboBox->rootModelIndex();
  return this->dmmlScene() ? d->ComboBox->count() - extraItemsCount : 0;
}

// --------------------------------------------------------------------------
vtkDMMLNode* qDMMLNodeComboBox::nodeFromIndex(int index)const
{
  Q_D(const qDMMLNodeComboBox);
  return d->dmmlNode(d->NoneEnabled ? index + 1 : index);
}

// --------------------------------------------------------------------------
void qDMMLNodeComboBox::removeCurrentNode()
{
  this->dmmlScene()->RemoveNode(this->currentNode());
}

// --------------------------------------------------------------------------
void qDMMLNodeComboBox::setDMMLScene(vtkDMMLScene* scene)
{
  Q_D(qDMMLNodeComboBox);

  // Be careful when commenting that out. you really need a good reason for
  // forcing a new set. You should probably expose
  // qDMMLSceneModel::UpdateScene() and make sure there is no nested calls
  if (d->DMMLSceneModel->dmmlScene() == scene)
    {
    return ;
    }

  // The Add button is valid only if the scene is non-empty
  //this->setAddEnabled(scene != 0);
  QString oldCurrentNode = d->ComboBox->itemData(d->ComboBox->currentIndex(), qDMMLSceneModel::UIDRole).toString();
  bool previousSceneWasValid = (this->nodeCount() > 0);

  // Update factory
  d->DMMLNodeFactory->setDMMLScene(scene);
  d->DMMLSceneModel->setDMMLScene(scene);

  if (d->DMMLScene)
    {
    d->DMMLScene->RemoveObserver(d->CallBack);
    }
  d->DMMLScene = scene;
  if (scene)
    {
    scene->AddObserver(vtkDMMLScene::NodeClassRegisteredEvent, d->CallBack);
    }
  d->updateNoneItem(false);
  d->updateActionItems(false);

  //qDebug()<< "setDMMLScene:" << this->model()->index(0, 0);
  // updating the action items reset the root model index. Set it back
  // setting the rootmodel index looses the current item
  d->ComboBox->setRootModelIndex(this->model()->index(0, 0));

  // try to set the current item back
  // if there was no node in the scene (or scene not set), then the
  // oldCurrentNode was not meaningful and we probably don't want to
  // set it back. Please consider make it a behavior property if it doesn't fit
  // your need, as this behavior is currently wanted for some cases (
  // vtkDMMLClipModels selector in the Models module)
  if (previousSceneWasValid)
    {
    this->setCurrentNodeID(oldCurrentNode);
    }
  // if the new nodeCount is 0, then let's make sure to select 'invalid' node
  // (None(0) or -1). we can't do nothing otherwise the Scene index (rootmodelIndex)
  // would be selected and "Scene" would be displayed (see vtkDMMLNodeComboboxTest5)
  else
    {
    QString newNodeID = this->currentNodeID();
    if (!d->RequestedNodeID.isEmpty())
      {
      newNodeID = d->RequestedNodeID;
      }
    else if (d->RequestedNode != nullptr && d->RequestedNode->GetID() != nullptr)
      {
      newNodeID = d->RequestedNode->GetID();
      }
    this->setCurrentNodeID(newNodeID);
    }
  d->RequestedNodeID.clear();
  d->RequestedNode = nullptr;

  // Need to update the default text after currentIndex is restored
  // (the text is only displayed if current index is set to -1).
  d->updateDefaultText();

  this->setEnabled(scene != nullptr);
}

// --------------------------------------------------------------------------
void qDMMLNodeComboBox::setCurrentNode(vtkDMMLNode* newCurrentNode)
{
  Q_D(qDMMLNodeComboBox);
  if (!this->dmmlScene())
    {
    d->RequestedNodeID.clear();
    d->RequestedNode = newCurrentNode;
    }
  this->setCurrentNodeID(newCurrentNode ? newCurrentNode->GetID() : "");
}

// --------------------------------------------------------------------------
void qDMMLNodeComboBox::setCurrentNode(const QString& nodeID)
{
  qWarning() << "This function is deprecated. Use setCurrentNodeID() instead";
  this->setCurrentNodeID(nodeID);
}

// --------------------------------------------------------------------------
void qDMMLNodeComboBox::setCurrentNodeID(const QString& nodeID)
{
  Q_D(qDMMLNodeComboBox);
  if (!this->dmmlScene())
    {
    d->RequestedNodeID = nodeID;
    d->RequestedNode = nullptr;
    }
  // A straight forward implementation of setCurrentNode would be:
  //    int index = !nodeID.isEmpty() ? d->ComboBox->findData(nodeID, qDMMLSceneModel::UIDRole) : -1;
  //    if (index == -1 && d->NoneEnabled)
  //      {
  //      index = 0;
  //      }
  //    d->ComboBox->setCurrentIndex(index);
  // However it doesn't work for custom comboxboxes that display non-flat lists
  // (typically if it is a tree model/view)
  // let's use a more generic one
  QModelIndexList indexes = d->indexesFromDMMLNodeID(nodeID);
  if (indexes.size() == 0)
    {
    QModelIndex sceneIndex = d->ComboBox->model()->index(0, 0);
    d->ComboBox->setRootModelIndex(sceneIndex);
    // The combobox updates the current index of the view only when he needs
    // it (in popup()), however we want the view to be always synchronized
    // with the currentIndex as we use it to know if it has changed. This is
    // why we set it here.
    QModelIndex noneIndex = ctk::modelChildIndex(d->ComboBox->model(), sceneIndex, 0, d->ComboBox->modelColumn());
    d->ComboBox->view()->setCurrentIndex(
      d->NoneEnabled ? noneIndex : sceneIndex);
    d->ComboBox->setCurrentIndex(d->NoneEnabled ? 0 : -1);
    return;
    }
  //d->ComboBox->setRootModelIndex(indexes[0].parent());
  //d->ComboBox->setCurrentIndex(indexes[0].row());
  QModelIndex oldIndex = d->ComboBox->view()->currentIndex();
  if (oldIndex != indexes[0])
    {
    d->ComboBox->view()->setCurrentIndex(indexes[0]);
    QKeyEvent event(QEvent::ShortcutOverride, Qt::Key_Enter, Qt::NoModifier);
    // here we conditionally send the event, otherwise, nodeActivated would be
    // fired even if the user didn't manually select the node.
    // Warning: please note that sending a KeyEvent will close the popup menu
    // of the combobox if it is open.
    QApplication::sendEvent(d->ComboBox->view(), &event);
    }
}

// --------------------------------------------------------------------------
void qDMMLNodeComboBox::setCurrentNodeIndex(int index)
{
  Q_D(qDMMLNodeComboBox);
  if (index >= this->nodeCount())
    {
    index = -1;
    }
  if (d->NoneEnabled)
    {
    // If the "None" extra item is present, shift all the indexes
    ++index;
    }
  d->ComboBox->setCurrentIndex(index);
}

//--------------------------------------------------------------------------
CTK_SET_CPP(qDMMLNodeComboBox, bool, setSelectNodeUponCreation, SelectNodeUponCreation);
CTK_GET_CPP(qDMMLNodeComboBox, bool, selectNodeUponCreation, SelectNodeUponCreation);

// --------------------------------------------------------------------------
QStringList qDMMLNodeComboBox::nodeTypes()const
{
  qDMMLSortFilterProxyModel* m = this->sortFilterProxyModel();
  return m ? m->nodeTypes() : QStringList();
}

// --------------------------------------------------------------------------
void qDMMLNodeComboBox::setNodeTypes(const QStringList& _nodeTypes)
{
  Q_D(qDMMLNodeComboBox);

  // Remove empty elements (empty elements may be created accidentally when
  // string lists are constructed in Python)
  QStringList nodeTypesFiltered = _nodeTypes;
  nodeTypesFiltered.removeAll("");

  this->sortFilterProxyModel()->setNodeTypes(nodeTypesFiltered);
  d->updateDefaultText();
  d->updateActionItems();
}

//--------------------------------------------------------------------------
void qDMMLNodeComboBox::setNoneEnabled(bool enable)
{
  Q_D(qDMMLNodeComboBox);
  if (d->NoneEnabled == enable)
    {
    return;
    }
  d->NoneEnabled = enable;
  d->updateNoneItem();
}

//--------------------------------------------------------------------------
bool qDMMLNodeComboBox::noneEnabled()const
{
  Q_D(const qDMMLNodeComboBox);
  return d->NoneEnabled;
}

//--------------------------------------------------------------------------
void qDMMLNodeComboBox::setAddEnabled(bool enable)
{
  Q_D(qDMMLNodeComboBox);
  if (d->AddEnabled == enable)
    {
    return;
    }
  if (enable && d->hasPostItem(tr("Create new ")))
    {
    qDebug() << "setAddEnabled: An action starting with name "
             << tr("Create new ") << " already exists. "
                "Not enabling this property.";
    return;
    }
  d->AddEnabled = enable;
  d->updateActionItems();
}

//--------------------------------------------------------------------------
bool qDMMLNodeComboBox::addEnabled()const
{
  Q_D(const qDMMLNodeComboBox);
  return d->AddEnabled;
}

//--------------------------------------------------------------------------
void qDMMLNodeComboBox::setRemoveEnabled(bool enable)
{
  Q_D(qDMMLNodeComboBox);
  if (d->RemoveEnabled == enable)
    {
    return;
    }
  if (enable && d->hasPostItem(tr("Delete current ")))
    {
    qDebug() << "setRemoveEnabled: An action starting with name "
             << tr("Delete current ") << " already exists. "
                "Not enabling this property.";
    return;
    }
  d->RemoveEnabled = enable;
  d->updateActionItems();
}

//--------------------------------------------------------------------------
bool qDMMLNodeComboBox::removeEnabled()const
{
  Q_D(const qDMMLNodeComboBox);
  return d->RemoveEnabled;
}

//--------------------------------------------------------------------------
void qDMMLNodeComboBox::setEditEnabled(bool enable)
{
  Q_D(qDMMLNodeComboBox);
  if (d->EditEnabled == enable)
    {
    return;
    }
  if (enable && d->hasPostItem(tr("Edit current ")))
    {
    qDebug() << "setEditEnabled: An action starting with name "
             << tr("Edit current ") << " already exists. "
                "Not enabling this property.";
    return;
    }
  d->EditEnabled = enable;
  d->updateActionItems();
}

//--------------------------------------------------------------------------
bool qDMMLNodeComboBox::editEnabled()const
{
  Q_D(const qDMMLNodeComboBox);
  return d->EditEnabled;
}

//--------------------------------------------------------------------------
void qDMMLNodeComboBox::setRenameEnabled(bool enable)
{
  Q_D(qDMMLNodeComboBox);
  if (d->RenameEnabled == enable)
    {
    return;
    }
  if (enable && d->hasPostItem(tr("Rename current ")))
    {
    qDebug() << "setRenameEnabled: An action starting with name "
             << tr("Rename current ") << " already exists. "
                "Not enabling this property.";
    return;
    }
  d->RenameEnabled = enable;
  d->updateActionItems();
}

//--------------------------------------------------------------------------
bool qDMMLNodeComboBox::renameEnabled()const
{
  Q_D(const qDMMLNodeComboBox);
  return d->RenameEnabled;
}

//--------------------------------------------------------------------------
void qDMMLNodeComboBox::setNoneDisplay(const QString& displayName)
{
  Q_D(qDMMLNodeComboBox);
  if (d->NoneDisplay == displayName)
    {
    return;
    }
  d->NoneDisplay = displayName;
  d->updateNoneItem(false);
}

//--------------------------------------------------------------------------
QString qDMMLNodeComboBox::noneDisplay()const
{
  Q_D(const qDMMLNodeComboBox);
  return d->NoneDisplay;
}

//--------------------------------------------------------------------------
QList<vtkDMMLNode*> qDMMLNodeComboBox::nodes()const
{
  QList<vtkDMMLNode*> nodeList;
  for (int i = 0; i < this->nodeCount(); ++i)
    {
    vtkDMMLNode* node = this->nodeFromIndex(i);
    Q_ASSERT(node);
    if (node)
      {
      nodeList << node;
      }
    }
  return nodeList;
}

//--------------------------------------------------------------------------
qDMMLSortFilterProxyModel* qDMMLNodeComboBox::sortFilterProxyModel()const
{
  Q_ASSERT(qobject_cast<qDMMLSortFilterProxyModel*>(this->model()));
  return qobject_cast<qDMMLSortFilterProxyModel*>(this->model());
}

//--------------------------------------------------------------------------
QAbstractItemModel* qDMMLNodeComboBox::model()const
{
  Q_D(const qDMMLNodeComboBox);
  return d->ComboBox ? d->ComboBox->model() : nullptr;
}

//--------------------------------------------------------------------------
qDMMLSceneModel* qDMMLNodeComboBox::sceneModel()const
{
  Q_ASSERT(this->sortFilterProxyModel());
  return this->sortFilterProxyModel()->sceneModel();
}

//--------------------------------------------------------------------------
QAbstractItemModel* qDMMLNodeComboBox::rootModel()const
{
  Q_D(const qDMMLNodeComboBox);
  return d->DMMLSceneModel;
}

//--------------------------------------------------------------------------
qDMMLNodeFactory* qDMMLNodeComboBox::nodeFactory()const
{
  Q_D(const qDMMLNodeComboBox);
  return d->DMMLNodeFactory;
}

//--------------------------------------------------------------------------
void qDMMLNodeComboBox::setComboBox(QComboBox* comboBox)
{
  Q_D(qDMMLNodeComboBox);
  if (comboBox == d->ComboBox)
    {
    return;
    }

  QAbstractItemModel* oldModel = this->model();
  QComboBox* oldComboBox = d->ComboBox;

  this->layout()->addWidget(comboBox);
  d->ComboBox = comboBox;
  d->ComboBox->setFocusProxy(this);
  d->setModel(oldModel);

  connect(d->ComboBox, SIGNAL(currentIndexChanged(QString)),
          this, SLOT(emitCurrentNodeChanged()));
  connect(d->ComboBox, SIGNAL(activated(int)),
          this, SLOT(emitNodeActivated(int)));
  d->ComboBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                         QSizePolicy::Expanding,
                                         QSizePolicy::DefaultType));
  delete oldComboBox;

  /// Set the new item delegate to force the highlight in case the item is not
  /// selectable but current.
  if (d->ComboBox)
    {
    d->updateDelegate(
      d->ComboBox->view()->metaObject()->className() == QString("QComboBoxListView"));
    }
}

//--------------------------------------------------------------------------
QComboBox* qDMMLNodeComboBox::comboBox()const
{
  Q_D(const qDMMLNodeComboBox);
  return d->ComboBox;
}

//--------------------------------------------------------------------------
void qDMMLNodeComboBox::emitNodesAdded(const QModelIndex & parent, int start, int end)
{
  Q_D(qDMMLNodeComboBox);
  Q_ASSERT(this->model());
  for(int i = start; i <= end; ++i)
    {
    vtkDMMLNode* node = d->dmmlNodeFromIndex(this->model()->index(start, 0, parent));
    if (node)
      {
      emit nodeAdded(node);
      }
    }
}

//--------------------------------------------------------------------------
void qDMMLNodeComboBox::emitNodesAboutToBeRemoved(const QModelIndex & parent, int start, int end)
{
  Q_D(qDMMLNodeComboBox);
  Q_ASSERT(this->model());
  for(int i = start; i <= end; ++i)
    {
    vtkDMMLNode* node = d->dmmlNodeFromIndex(this->model()->index(start, 0, parent));
    if (node)
      {
      emit nodeAboutToBeRemoved(node);
      }
    }
}

//--------------------------------------------------------------------------
void qDMMLNodeComboBox::refreshIfCurrentNodeHidden()
{
  /// Sometimes, a node can disappear/hide from the combobox
  /// (qDMMLSortFilterProxyModel) because of a changed property.
  /// If the node is the current node, we need to unselect it because it is
  /// not a valid current node anymore.
  vtkDMMLNode* node = this->currentNode();
  if (!node)
    {
    this->setCurrentNode(nullptr);
    }
}

//--------------------------------------------------------------------------
QComboBox::SizeAdjustPolicy qDMMLNodeComboBox::sizeAdjustPolicy()const
{
  Q_D(const qDMMLNodeComboBox);
  return d->ComboBox->sizeAdjustPolicy();
}

//--------------------------------------------------------------------------
void qDMMLNodeComboBox::setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy policy)
{
  Q_D(qDMMLNodeComboBox);
  d->ComboBox->setSizeAdjustPolicy(policy);
}

//--------------------------------------------------------------------------
void qDMMLNodeComboBox::changeEvent(QEvent *event)
{
  Q_D(qDMMLNodeComboBox);
  if(event->type() == QEvent::StyleChange)
    {
    d->updateDelegate();
    }
  this->Superclass::changeEvent(event);
}

// --------------------------------------------------------------------------
void qDMMLNodeComboBox::addMenuAction(QAction *newAction)
{
  Q_D(qDMMLNodeComboBox);

  // is an action with the same text already in the user list?
  foreach (QAction *action, d->UserMenuActions)
    {
    if (action->text() == newAction->text())
      {
      qDebug() << "addMenuAction: duplicate action text of "
               << newAction->text()
               << ", not adding this action";
      return;
      }
    }
  if ((d->AddEnabled
       && newAction->text().startsWith(tr("Create new "))) ||
      (d->RemoveEnabled
       && newAction->text().startsWith(tr("Delete current "))) ||
      (d->EditEnabled
       && newAction->text().startsWith(tr("Edit current "))) ||
      (d->RenameEnabled
       && newAction->text().startsWith(tr("Rename current "))))
    {
    qDebug() << "addMenuAction: warning: the text on this action, "
             << newAction->text()
             << ", matches the start of an enabled default action text and "
                "will not get triggered, not adding it.";
    return;
    }

  d->UserMenuActions.append(newAction);

  // update with the new action
  d->updateActionItems(false);
}

//--------------------------------------------------------------------------
QString qDMMLNodeComboBox::interactionNodeSingletonTag()const
{
  Q_D(const qDMMLNodeComboBox);
  return d->InteractionNodeSingletonTag;
}

//--------------------------------------------------------------------------
void qDMMLNodeComboBox::setInteractionNodeSingletonTag(const QString& tag)
{
  Q_D(qDMMLNodeComboBox);
  d->InteractionNodeSingletonTag = tag;
}
