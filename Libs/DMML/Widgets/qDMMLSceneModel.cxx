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
#include <QTimer>

// CTK includes
#include <ctkUtils.h>

// qDMML includes
#include "qDMMLSceneModel_p.h"

// DMML includes
#include <vtkDMMLDisplayableHierarchyNode.h>
#include <vtkDMMLDisplayableNode.h>
#include <vtkDMMLDisplayNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSelectionNode.h>

// VTK includes
#include <vtkCollection.h>

// STD includes

//------------------------------------------------------------------------------
qDMMLSceneModelPrivate::qDMMLSceneModelPrivate(qDMMLSceneModel& object)
  : q_ptr(&object)
{
  qRegisterMetaType<qDMMLSceneModel::NodeTypes>("qDMMLSceneModel::NodeTypes");

  this->CallBack = vtkSmartPointer<vtkCallbackCommand>::New();
  this->LazyUpdate = false;
  this->ListenNodeModifiedEvent = qDMMLSceneModel::NoNodes;
  this->PendingItemModified = -1; // -1 means not updating

  this->NameColumn = -1;
  this->IDColumn = -1;
  this->CheckableColumn = -1;
  this->VisibilityColumn = -1;
  this->ToolTipNameColumn = -1;
  this->ExtraItemColumn = 0;

  this->HiddenIcon = QIcon(":Icons/VisibleOff.png");
  this->VisibleIcon = QIcon(":Icons/VisibleOn.png");
  this->PartiallyVisibleIcon = QIcon(":Icons/VisiblePartially.png");

  this->DraggedItem = nullptr;

  qRegisterMetaType<QStandardItem* >("QStandardItem*");
}

//------------------------------------------------------------------------------
qDMMLSceneModelPrivate::~qDMMLSceneModelPrivate()
{
  if (this->DMMLScene)
    {
    this->DMMLScene->RemoveObserver(this->CallBack);
    }
}

//------------------------------------------------------------------------------
void qDMMLSceneModelPrivate::init()
{
  Q_Q(qDMMLSceneModel);
  this->CallBack->SetClientData(q);
  this->CallBack->SetCallback(qDMMLSceneModel::onDMMLSceneEvent);

  QObject::connect(q, SIGNAL(itemChanged(QStandardItem*)),
                   q, SLOT(onItemChanged(QStandardItem*)));

  q->setNameColumn(0);
  q->setListenNodeModifiedEvent(qDMMLSceneModel::OnlyVisibleNodes);
}

//------------------------------------------------------------------------------
QModelIndexList qDMMLSceneModelPrivate::indexes(const QString& nodeID)const
{
  Q_Q(const qDMMLSceneModel);
  QModelIndex scene = q->dmmlSceneIndex();
  if (scene == QModelIndex())
    {
    return QModelIndexList();
    }
  // QAbstractItemModel::match doesn't browse through columns
  // we need to do it manually
  QModelIndexList nodeIndexes = q->match(
    scene, qDMMLSceneModel::UIDRole, nodeID,
    1, Qt::MatchExactly | Qt::MatchRecursive);
  Q_ASSERT(nodeIndexes.size() <= 1); // we know for sure it won't be more than 1
  if (nodeIndexes.size() == 0)
    {
    return nodeIndexes;
    }
  // Add the QModelIndexes from the other columns
  const int row = nodeIndexes[0].row();
  QModelIndex nodeParentIndex = nodeIndexes[0].parent();
  const int sceneColumnCount = q->columnCount(nodeParentIndex);
  for (int j = 1; j < sceneColumnCount; ++j)
    {
    nodeIndexes << q->index(row, j, nodeParentIndex);
    }
  return nodeIndexes;
}

//------------------------------------------------------------------------------
void qDMMLSceneModelPrivate::listenNodeModifiedEvent()
{
  Q_Q(qDMMLSceneModel);
  QModelIndex sceneIndex = q->dmmlSceneIndex();
  const int count = q->rowCount(sceneIndex);
  for (int i = 0; i < count; ++i)
    {
    vtkDMMLNode* node = q->dmmlNodeFromIndex(ctk::modelChildIndex(q, sceneIndex, i, 0));
    q->qvtkDisconnect(node, vtkCommand::NoEvent, q, nullptr);
    if (this->ListenNodeModifiedEvent == qDMMLSceneModel::AllNodes)
      {
      q->observeNode(node);
      }
    }
}

//------------------------------------------------------------------------------
void qDMMLSceneModelPrivate::insertExtraItem(int row, QStandardItem* parent,
                                             const QString& text,
                                             const QString& extraType,
                                             const Qt::ItemFlags& flags)
{
  Q_ASSERT(parent);

  QList<QStandardItem*> items;
  // fill with empty column items
  for (int column = 0; column < parent->columnCount(); ++column)
    {
    QStandardItem* extraItem = new QStandardItem;
    if (column == this->ExtraItemColumn)
      {
      extraItem->setData(extraType, qDMMLSceneModel::UIDRole);
      if (text == "separator")
        {
        extraItem->setData("separator", Qt::AccessibleDescriptionRole);
        }
      else
        {
        extraItem->setText(text);
        }
      extraItem->setFlags(flags);
      }
    else
      {
      extraItem->setFlags(Qt::NoItemFlags);
      }
    items << extraItem;
    }
  parent->insertRow(row, items);

  // update extra item cache info (for faster retrieval)
  QMap<QString, QVariant> extraItems = parent->data(qDMMLSceneModel::ExtraItemsRole).toMap();
  extraItems[extraType] = extraItems[extraType].toStringList() << text;
  parent->setData(extraItems, qDMMLSceneModel::ExtraItemsRole );
}

//------------------------------------------------------------------------------
QStringList qDMMLSceneModelPrivate::extraItems(QStandardItem* parent, const QString& extraType)const
{
  QStringList res;
  if (parent == nullptr)
    {
    //parent = q->invisibleRootItem();
    return res;
    }
  // It is expensive to search, cache the extra items.
  res = parent->data(qDMMLSceneModel::ExtraItemsRole).toMap()[extraType].toStringList();
  /*
  const int rowCount = parent->rowCount();
  for (int i = 0; i < rowCount; ++i)
    {
    QStandardItem* child = parent->child(i);
    if (child && child->data(qDMMLSceneModel::UIDRole).toString() == extraType)
      {
      if (child->data(Qt::AccessibleDescriptionRole) == "separator")
        {
        res << "separator";
        }
      else
        {
        res << child->text();
        }
      }
    }
  */
  return res;
}

//------------------------------------------------------------------------------
void qDMMLSceneModelPrivate::removeAllExtraItems(QStandardItem* parent, const QString extraType)
{
  Q_Q(qDMMLSceneModel);
  Q_ASSERT(parent);
  QMap<QString, QVariant> extraItems =
    parent->data(qDMMLSceneModel::ExtraItemsRole).toMap();
  if (extraItems[extraType].toStringList().size() == 0)
    {
    return;
    }
  QModelIndex start = parent ? ctk::modelChildIndex(q, parent->index(), 0, 0) : ctk::modelChildIndex(q, QModelIndex(), 0, 0);
  QModelIndexList indexes =
    q->match(start, qDMMLSceneModel::UIDRole, extraType, 1, Qt::MatchExactly);
  while (start != QModelIndex() && indexes.size())
    {
    QModelIndex parentIndex = indexes[0].parent();
    int row = indexes[0].row();
    q->removeRow(row, parentIndex);
    // don't start the whole search from scratch, only from where we ended it
    start = ctk::modelChildIndex(q, parentIndex, row, 0);
    indexes = q->match(start, qDMMLSceneModel::UIDRole, extraType, 1, Qt::MatchExactly);
    }
  extraItems[extraType] = QStringList();
  parent->setData(extraItems, qDMMLSceneModel::ExtraItemsRole);
}

//------------------------------------------------------------------------------
bool qDMMLSceneModelPrivate::isExtraItem(const QStandardItem* item)const
{
  QString uid =
    item ? item->data(qDMMLSceneModel::UIDRole).toString() : QString();
  return uid == "preItem" || uid == "postItem";
}

//------------------------------------------------------------------------------
void qDMMLSceneModelPrivate::reparentItems(
  QList<QStandardItem*>& children, int newIndex, QStandardItem* newParentItem)
{
  Q_Q(qDMMLSceneModel);
  int min = q->preItems(newParentItem).count();
  int max = newParentItem->rowCount() - q->postItems(newParentItem).count();
  int pos = qMin(min + newIndex, max);
  newParentItem->insertRow(pos, children);
}

//------------------------------------------------------------------------------
// qDMMLSceneModel
//------------------------------------------------------------------------------
qDMMLSceneModel::qDMMLSceneModel(QObject *_parent)
  :QStandardItemModel(_parent)
  , d_ptr(new qDMMLSceneModelPrivate(*this))
{
  Q_D(qDMMLSceneModel);
  d->init(/*new qDMMLSceneModelItemHelperFactory*/);
}

//------------------------------------------------------------------------------
qDMMLSceneModel::qDMMLSceneModel(qDMMLSceneModelPrivate* pimpl, QObject *parentObject)
  :QStandardItemModel(parentObject)
  , d_ptr(pimpl)
{
  Q_D(qDMMLSceneModel);
  d->init(/*factory*/);
}

//------------------------------------------------------------------------------
qDMMLSceneModel::~qDMMLSceneModel() = default;

//------------------------------------------------------------------------------
void qDMMLSceneModel::setPreItems(const QStringList& extraItems, QStandardItem* parent)
{
  Q_D(qDMMLSceneModel);

  if (parent == nullptr)
    {
    return;
    }

  d->removeAllExtraItems(parent, "preItem");

  int row = 0;
  foreach(QString extraItem, extraItems)
    {
    d->insertExtraItem(row++, parent, extraItem, "preItem", Qt::ItemIsEnabled  | Qt::ItemIsSelectable);
    }
}

//------------------------------------------------------------------------------
QStringList qDMMLSceneModel::preItems(QStandardItem* parent)const
{
  Q_D(const qDMMLSceneModel);
  return d->extraItems(parent, "preItem");
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::setPostItems(const QStringList& extraItems, QStandardItem* parent)
{
  Q_D(qDMMLSceneModel);

  if (parent == nullptr)
    {
    return;
    }

  d->removeAllExtraItems(parent, "postItem");
  foreach(QString extraItem, extraItems)
    {
    d->insertExtraItem(parent->rowCount(), parent, extraItem, "postItem", Qt::ItemIsEnabled);
    }
}

//------------------------------------------------------------------------------
QStringList qDMMLSceneModel::postItems(QStandardItem* parent)const
{
  Q_D(const qDMMLSceneModel);
  return d->extraItems(parent, "postItem");
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::setDMMLScene(vtkDMMLScene* scene)
{
  Q_D(qDMMLSceneModel);
  /// it could go wrong if you try to set the same scene (specially because
  /// while updating the scene your signals/slots might call setDMMLScene again
  if (scene == d->DMMLScene)
    {
    return;
    }

  if (d->DMMLScene)
    {
    d->DMMLScene->RemoveObserver(d->CallBack);
    }
  d->DMMLScene = scene;
  this->updateScene();
  if (scene)
    {
    scene->AddObserver(vtkDMMLScene::NodeAboutToBeAddedEvent, d->CallBack, -10.);
    scene->AddObserver(vtkDMMLScene::NodeAddedEvent, d->CallBack, 10.);
    scene->AddObserver(vtkDMMLScene::NodeAboutToBeRemovedEvent, d->CallBack, -10.);
    scene->AddObserver(vtkDMMLScene::NodeRemovedEvent, d->CallBack, 10.);
    scene->AddObserver(vtkCommand::DeleteEvent, d->CallBack);
    scene->AddObserver(vtkDMMLScene::StartCloseEvent, d->CallBack);
    scene->AddObserver(vtkDMMLScene::EndCloseEvent, d->CallBack);
    scene->AddObserver(vtkDMMLScene::StartImportEvent, d->CallBack);
    scene->AddObserver(vtkDMMLScene::EndImportEvent, d->CallBack);
    scene->AddObserver(vtkDMMLScene::StartBatchProcessEvent, d->CallBack);
    scene->AddObserver(vtkDMMLScene::EndBatchProcessEvent, d->CallBack);
    }
}

//------------------------------------------------------------------------------
vtkDMMLScene* qDMMLSceneModel::dmmlScene()const
{
  Q_D(const qDMMLSceneModel);
  return d->DMMLScene;
}

//------------------------------------------------------------------------------
QStandardItem* qDMMLSceneModel::dmmlSceneItem()const
{
  Q_D(const qDMMLSceneModel);
  if (d->DMMLScene == nullptr || this->maxColumnId() == -1)
    {
    return nullptr;
    }
  int count = this->invisibleRootItem()->rowCount();
  for (int i = 0; i < count; ++i)
    {
    QStandardItem* child = this->invisibleRootItem()->child(i);
    if (!child)
      {
      continue;
      }
    QVariant uid = child->data(qDMMLSceneModel::UIDRole);
    if (uid.type() == QVariant::String &&
        uid.toString() == "scene")
      {
      return child;
      }
    }
  return nullptr;
}

//------------------------------------------------------------------------------
QModelIndex qDMMLSceneModel::dmmlSceneIndex()const
{
  QStandardItem* scene = this->dmmlSceneItem();
  if (scene == nullptr)
    {
    return QModelIndex();
    }
  return scene ? scene->index() : QModelIndex();
}

//------------------------------------------------------------------------------
vtkDMMLNode* qDMMLSceneModel::dmmlNodeFromItem(QStandardItem* nodeItem)const
{
  Q_D(const qDMMLSceneModel);
  // TODO: fasten by saving the pointer into the data
  if (d->DMMLScene == nullptr || nodeItem == nullptr)
    {
    return nullptr;
    }
  QVariant nodePointer = nodeItem->data(qDMMLSceneModel::PointerRole);
  if (!nodePointer.isValid() || nodeItem->data(qDMMLSceneModel::UIDRole).toString() == "scene")
    {
    return nullptr;
    }
  return nodeItem ? d->DMMLScene->GetNodeByID(
    nodeItem->data(qDMMLSceneModel::UIDRole).toString().toUtf8()) : nullptr;
}
//------------------------------------------------------------------------------
QStandardItem* qDMMLSceneModel::itemFromNode(vtkDMMLNode* node, int column)const
{
  QModelIndex nodeIndex = this->indexFromNode(node, column);
  QStandardItem* nodeItem = this->itemFromIndex(nodeIndex);
  return nodeItem;
}

//------------------------------------------------------------------------------
QModelIndex qDMMLSceneModel::indexFromNode(vtkDMMLNode* node, int column)const
{
  Q_D(const qDMMLSceneModel);

  if (node == nullptr || node->GetID() == nullptr )
    {
    return QModelIndex();
    }

  QModelIndex nodeIndex;

  // Try to find the nodeIndex in the cache first
  QMap<vtkDMMLNode*,QPersistentModelIndex>::iterator rowCacheIt=d->RowCache.find(node);
  if (rowCacheIt==d->RowCache.end())
    {
    // not found in cache, therefore it cannot be in the model
    return nodeIndex;
    }
  if (rowCacheIt.value().isValid())
    {
    // An entry found in the cache. If the item at the cached index matches the requested node ID
    // then we use it.
    QStandardItem* nodeItem = this->itemFromIndex(rowCacheIt.value());
    if (nodeItem!=nullptr)
      {
      if (nodeItem->data(qDMMLSceneModel::UIDRole).toString().compare(QString::fromUtf8(node->GetID()))==0)
        {
        // id matched
        nodeIndex=rowCacheIt.value();
        }
      }
    }

  // The cache was not up-to-date. Do a slow linear search.
  if (!nodeIndex.isValid())
    {
    // QAbstractItemModel::match doesn't browse through columns
    // we need to do it manually
    QModelIndexList nodeIndexes = this->match(
      this->dmmlSceneIndex(), qDMMLSceneModel::UIDRole, QString(node->GetID()),
      1, Qt::MatchExactly | Qt::MatchRecursive);
    Q_ASSERT(nodeIndexes.size() <= 1); // we know for sure it won't be more than 1
    if (nodeIndexes.size() == 0)
      {
      // maybe the node hasn't been added to the scene yet...
      // (if it's called from populateScene/inserteNode)
      d->RowCache.remove(node);
      return QModelIndex();
      }
    nodeIndex=nodeIndexes[0];
    d->RowCache[node]=nodeIndex;
    }
  if (column == 0)
    {
    // QAbstractItemModel::match only search through the first column
    // (because scene is in the first column)
    Q_ASSERT(nodeIndex.isValid());
    return nodeIndex;
    }
  // Add the QModelIndexes from the other columns
  const int row = nodeIndex.row();
  QModelIndex nodeParentIndex = nodeIndex.parent();
  Q_ASSERT( column < this->columnCount(nodeParentIndex) );
  return ctk::modelChildIndex(const_cast<qDMMLSceneModel*>(this), nodeParentIndex, row, column);
}

//------------------------------------------------------------------------------
QModelIndexList qDMMLSceneModel::indexes(vtkDMMLNode* node)const
{
  Q_D(const qDMMLSceneModel);
  return d->indexes(QString(node->GetID()));
}

//------------------------------------------------------------------------------
vtkDMMLNode* qDMMLSceneModel::parentNode(vtkDMMLNode* node)const
{
  Q_UNUSED(node);
  return nullptr;
}

//------------------------------------------------------------------------------
int qDMMLSceneModel::nodeIndex(vtkDMMLNode* node)const
{
  Q_D(const qDMMLSceneModel);
  if (!d->DMMLScene)
    {
    return -1;
    }
  const char* nodeId = node ? node->GetID() : nullptr;
  if (nodeId == nullptr)
    {
    return -1;
    }

  const char* nId = nullptr;
  int index = -1;
  vtkDMMLNode* parent = this->parentNode(node);

  // Iterate through the scene and see if there is any matching node.
  // First try to find based on ptr value, as it's much faster than comparing string IDs.
  vtkCollection* nodes = d->DMMLScene->GetNodes();
  vtkDMMLNode* n = nullptr;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);
       (n = (vtkDMMLNode*)(nodes->GetNextItemAsObject(it))) ;)
    {
    // note: parent can be nullptr, it means that the scene is the parent
    if (parent == this->parentNode(n))
      {
      ++index;
      if (node==n)
        {
        // found the node
        return index;
        }
      }
    }

  // Not found by node ptr, try to find it by ID (much slower)
  for (nodes->InitTraversal(it);
       (n = (vtkDMMLNode*)nodes->GetNextItemAsObject(it)) ;)
    {
    // note: parent can be nullptr, it means that the scene is the parent
    if (parent == this->parentNode(n))
      {
      ++index;
      nId = n->GetID();
      if (nId && !strcmp(nodeId, nId))
        {
        return index;
        }
      }
    }

  // Not found
  return -1;
}

//------------------------------------------------------------------------------
bool qDMMLSceneModel::canBeAChild(vtkDMMLNode* node)const
{
  Q_UNUSED(node);
  return false;
}

//------------------------------------------------------------------------------
bool qDMMLSceneModel::canBeAParent(vtkDMMLNode* node)const
{
  Q_UNUSED(node);
  return false;
}

//------------------------------------------------------------------------------
bool qDMMLSceneModel::reparent(vtkDMMLNode* node, vtkDMMLNode* newParent)
{
  Q_UNUSED(node);
  Q_UNUSED(newParent);
  return false;
}

//------------------------------------------------------------------------------
bool qDMMLSceneModel::isParentNode(vtkDMMLNode* child, vtkDMMLNode* parent)const
{
  for (; child; child = this->parentNode(child))
    {
    if (child == parent)
      {
      return true;
      }
    }
  return false;
}

//------------------------------------------------------------------------------
bool qDMMLSceneModel
::isAffiliatedNode(vtkDMMLNode* nodeA, vtkDMMLNode* nodeB)const
{
  return this->isParentNode(nodeA, nodeB) || this->isParentNode(nodeB, nodeA);
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::setListenNodeModifiedEvent(qDMMLSceneModel::NodeTypes listen)
{
  Q_D(qDMMLSceneModel);
  if (d->ListenNodeModifiedEvent == listen)
    {
    return;
    }
  d->ListenNodeModifiedEvent = listen;
  d->listenNodeModifiedEvent();
}

//------------------------------------------------------------------------------
qDMMLSceneModel::NodeTypes qDMMLSceneModel::listenNodeModifiedEvent()const
{
  Q_D(const qDMMLSceneModel);
  return d->ListenNodeModifiedEvent;
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::setLazyUpdate(bool lazy)
{
  Q_D(qDMMLSceneModel);
  if (d->LazyUpdate == lazy)
    {
    return;
    }
  d->LazyUpdate = lazy;
}

//------------------------------------------------------------------------------
bool qDMMLSceneModel::lazyUpdate()const
{
  Q_D(const qDMMLSceneModel);
  return d->LazyUpdate;
}

//------------------------------------------------------------------------------
QMimeData* qDMMLSceneModel::mimeData(const QModelIndexList& indexes)const
{
  Q_D(const qDMMLSceneModel);
  if (!indexes.size())
    {
    return nullptr;
    }
  QModelIndexList allColumnsIndexes;
  foreach(const QModelIndex& index, indexes)
    {
    QModelIndex parent = index.parent();
    for (int column = 0; column < this->columnCount(parent); ++column)
      {
      allColumnsIndexes << this->index(index.row(), column, parent);
      }
    d->DraggedNodes << this->dmmlNodeFromIndex(index);
    }
  // Remove duplicates
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
  allColumnsIndexes = QSet<QModelIndex>(allColumnsIndexes.begin(), allColumnsIndexes.end()).values();
#else
  allColumnsIndexes = allColumnsIndexes.toSet().values();
#endif
  return this->QStandardItemModel::mimeData(allColumnsIndexes);
}

//------------------------------------------------------------------------------
bool qDMMLSceneModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                   int row, int column, const QModelIndex &parent)
{
  Q_UNUSED(column);
  // We want to do drag&drop only into the first item of a line (and not on a
  // random column.
  bool res = this->Superclass::dropMimeData(
    data, action, row, 0, parent.sibling(parent.row(), 0));
  // Do not clear d->DraggedNodes yet, as node modification events may come
  // in before delayedItemChanged() is executed.
  return res;
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::updateScene()
{
  Q_D(qDMMLSceneModel);

  // Stop listening to all the nodes before we remove them (setRowCount) as some
  // weird behavior could arise when removing the nodes (e.g onDMMLNodeModified
  // could be called ...)
  qvtkDisconnect(nullptr, vtkCommand::ModifiedEvent,
                 this, SLOT(onDMMLNodeModified(vtkObject*)));
  qvtkDisconnect(nullptr, vtkDMMLNode::IDChangedEvent,
                 this, SLOT(onDMMLNodeIDChanged(vtkObject*,void*)));

  d->RowCache.clear();

  // Enabled so it can be interacted with
  this->invisibleRootItem()->setFlags(Qt::ItemIsEnabled);

  // Extra items before the scene item. Typically there is no top-level extra
  // items, only at the Scene level (before and after the nodes)
  const int preSceneItemCount = this->preItems(nullptr).count();
  const int postSceneItemCount = this->postItems(nullptr).count();

  if (!this->dmmlSceneItem() && d->DMMLScene)
    {
    // No scene item has been created yet, but the DMMLScene is valid so we
    // need to create one.
    QList<QStandardItem*> sceneItems;
    QStandardItem* sceneItem = new QStandardItem;
    sceneItem->setFlags(Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);
    sceneItem->setText("Scene");
    sceneItem->setData("scene", qDMMLSceneModel::UIDRole);
    sceneItems << sceneItem;
    for (int i = 1; i < this->columnCount(); ++i)
      {
      QStandardItem* sceneOtherColumn = new QStandardItem;
      sceneOtherColumn->setFlags(Qt::NoItemFlags);
      sceneItems << sceneOtherColumn;
      }
    // We need to set the column count in case there extra items,
    // they need to know how many columns the scene item has.
    sceneItem->setColumnCount(this->columnCount());
    this->insertRow(preSceneItemCount, sceneItems);
    }
  else if (!d->DMMLScene)
    {
    // TBD: Because we don't call clear, I don't think restoring the column
    // count is necessary because it shouldn't be changed.
    const int oldColumnCount = this->columnCount();
    this->removeRows(
      preSceneItemCount,
      this->rowCount() - preSceneItemCount - postSceneItemCount);
    this->setColumnCount(oldColumnCount);
    return;
    }

  // if there is no column, there is no scene item.
  if (!this->dmmlSceneItem())
    {
    return;
    }

  // Update the scene pointer in case d->DMMLScene has changed
  this->dmmlSceneItem()->setData(
    QVariant::fromValue(reinterpret_cast<long long>(d->DMMLScene.GetPointer())),
    qDMMLSceneModel::PointerRole);

  const int preNodesItemCount = this->preItems(this->dmmlSceneItem()).count();
  const int postNodesItemCount = this->postItems(this->dmmlSceneItem()).count();
  // Just remove the nodes, not the extra items like "None", "Create node" etc.
  this->dmmlSceneItem()->removeRows(
    preNodesItemCount,
    this->dmmlSceneItem()->rowCount() - preNodesItemCount - postNodesItemCount);

  // Populate scene with nodes
  this->populateScene();
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::populateScene()
{
  Q_D(qDMMLSceneModel);
  // Add nodes
  int index = -1;
  vtkDMMLNode *node = nullptr;
  vtkCollectionSimpleIterator it;
  d->MisplacedNodes.clear();
  if (!d->DMMLScene)
    {
    return;
    }
  for (d->DMMLScene->GetNodes()->InitTraversal(it);
       (node = (vtkDMMLNode*)d->DMMLScene->GetNodes()->GetNextItemAsObject(it)) ;)
    {
    index++;
    d->insertNode(node, index);
    }
  foreach(vtkDMMLNode* misplacedNode, d->MisplacedNodes)
    {
    this->onDMMLNodeModified(misplacedNode);
    }
}

//------------------------------------------------------------------------------
QStandardItem* qDMMLSceneModel::insertNode(vtkDMMLNode* node)
{
  Q_D(qDMMLSceneModel);
  return d->insertNode(node, this->nodeIndex(node));
}

//------------------------------------------------------------------------------
QStandardItem* qDMMLSceneModelPrivate::insertNode(vtkDMMLNode* node, int nodeIndex)
{
  Q_Q(qDMMLSceneModel);
  QStandardItem* nodeItem = q->itemFromNode(node);
  if (nodeItem != nullptr)
    {
    // It is possible that the node has been already added if it is the parent
    // of a child node already inserted.
    return nodeItem;
    }
  vtkDMMLNode* parentNode = q->parentNode(node);
  QStandardItem* parentItem =
    parentNode ? q->itemFromNode(parentNode) : q->dmmlSceneItem();
  if (!parentItem)
    {
    Q_ASSERT(parentNode);
    parentItem = q->insertNode(parentNode);
    Q_ASSERT(parentItem);
    }
  int min = q->preItems(parentItem).count();
  int max = parentItem->rowCount() - q->postItems(parentItem).count();
  int row = min + nodeIndex;
  if (row > max)
    {
    this->MisplacedNodes << node;
    row = max;
    }
  nodeItem = q->insertNode(node, parentItem, row);
  Q_ASSERT(q->itemFromNode(node) == nodeItem);
  return nodeItem;
}

//------------------------------------------------------------------------------
QStandardItem* qDMMLSceneModel::insertNode(vtkDMMLNode* node, QStandardItem* parent, int row)
{
  Q_D(qDMMLSceneModel);
  Q_ASSERT(vtkDMMLNode::SafeDownCast(node));

  QList<QStandardItem*> items;
  for (int i= 0; i < this->columnCount(); ++i)
    {
    QStandardItem* newNodeItem = new QStandardItem();
    this->updateItemFromNode(newNodeItem, node, i);
    items.append(newNodeItem);
    }

  // Insert an invalid item in the cache to indicate that the node is in the model
  // but we don't know its index yet. This is needed because a custom widget may be notified
  // abot row insertion before insertRow() returns (and the RowCache entry is added).
  // For example, qCjyxPresetComboBox::setIconToPreset() is called at the end of insertRow,
  // before the RowCache entry is added.
  d->RowCache[node]=QModelIndex();

  if (parent)
    {
    parent->insertRow(row, items);
    //Q_ASSERT(parent->columnCount() == 2);
    }
  else
    {
    this->insertRow(row,items);
    }
  d->RowCache[node]=items[0]->index();
  // TODO: don't listen to nodes that are hidden from editors ?
  if (d->ListenNodeModifiedEvent == AllNodes)
    {
    this->observeNode(node);
    }
  return items[0];
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::observeNode(vtkDMMLNode* node)
{
  qvtkConnect(node, vtkCommand::ModifiedEvent,
              this, SLOT(onDMMLNodeModified(vtkObject*)));
  qvtkConnect(node, vtkDMMLNode::IDChangedEvent,
              this, SLOT(onDMMLNodeIDChanged(vtkObject*,void*)));
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::updateItemFromNode(QStandardItem* item, vtkDMMLNode* node, int column)
{
  Q_D(qDMMLSceneModel);
  // We are going to make potentially multiple changes to the item. We want to
  // refresh the node only once, so we "block" the updates in onItemChanged().
  d->PendingItemModified = 0;
  item->setFlags(this->nodeFlags(node, column));
  // set UIDRole and set PointerRole need to be atomic
  bool blocked  = this->blockSignals(true);
  item->setData(QString(node->GetID()), qDMMLSceneModel::UIDRole);
  item->setData(QVariant::fromValue(reinterpret_cast<long long>(node)), qDMMLSceneModel::PointerRole);
  this->blockSignals(blocked);
  this->updateItemDataFromNode(item, node, column);

  bool itemChanged = (d->PendingItemModified > 0);
  d->PendingItemModified = -1;

  // Update parent, but only if the item is not being drag-and-dropped
  // (drag-and-drop is performed using delayed update, therefore
  // any node modifications, even those unrelated to changing the parent
  // would override drag-and-drop result).
  if (this->canBeAChild(node) && !d->DraggedNodes.contains(node))
    {
    QStandardItem* parentItem = item->parent();
    QStandardItem* newParentItem = this->itemFromNode(this->parentNode(node));
    if (newParentItem == nullptr)
      {
      newParentItem = this->dmmlSceneItem();
      }
    // If the item has no parent, then it means it hasn't been put into the scene yet.
    // and it will do it automatically.
    if (parentItem && parentItem != newParentItem)
      {
      int newIndex = this->nodeIndex(node);
      if (parentItem != newParentItem ||
          newIndex != item->row())
        {
        QList<QStandardItem*> children = parentItem->takeRow(item->row());
        d->reparentItems(children, newIndex, newParentItem);
        }
      }
    }
  if (itemChanged)
    {
    this->onItemChanged(item);
    }
}

//------------------------------------------------------------------------------
QFlags<Qt::ItemFlag> qDMMLSceneModel::nodeFlags(vtkDMMLNode* node, int column)const
{
  QFlags<Qt::ItemFlag> flags = Qt::ItemIsEnabled
                             | Qt::ItemIsSelectable;
  if (column == this->checkableColumn() && node->GetSelectable())
    {
    flags = flags | Qt::ItemIsUserCheckable;
    }
  if (column == this->nameColumn())
    {
    flags = flags | Qt::ItemIsEditable;
    }
  if (this->canBeAChild(node))
    {
    flags = flags | Qt::ItemIsDragEnabled;
    }
  if (this->canBeAParent(node))
    {
    flags = flags | Qt::ItemIsDropEnabled;
    }

  return flags;
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::updateItemDataFromNode(
  QStandardItem* item, vtkDMMLNode* node, int column)
{
  Q_D(qDMMLSceneModel);
  if (column == this->nameColumn())
    {
    item->setText(QString(node->GetName()));
    item->setToolTip(node->GetNodeTagName());
    }
  if (column == this->toolTipNameColumn())
    {
    item->setToolTip(QString(node->GetName()));
    }
  if (column == this->idColumn())
    {
    item->setText(QString(node->GetID()));
    }
  if (column == this->checkableColumn())
    {
    item->setCheckState(node->GetSelected() ? Qt::Checked : Qt::Unchecked);
    }
  if (column == this->visibilityColumn())
    {
    vtkDMMLDisplayNode* displayNode = vtkDMMLDisplayNode::SafeDownCast(node);
    vtkDMMLDisplayableNode* displayableNode =
      vtkDMMLDisplayableNode::SafeDownCast(node);
    vtkDMMLDisplayableHierarchyNode* displayableHierarchyNode =
      vtkDMMLDisplayableHierarchyNode::SafeDownCast(node);
    if (displayableHierarchyNode)
      {
      displayNode = displayableHierarchyNode->GetDisplayNode();
      }
    int visible = -1;
    if (displayNode)
      {
      visible = displayNode->GetVisibility();
      }
    else if (displayableNode)
      {
      visible = displayableNode->GetDisplayVisibility();
      }
    // It should be fine to set the icon even if it is the same, but due
    // to a bug in Qt (http://bugreports.qt.nokia.com/browse/QTBUG-20248),
    // it would fire a superfluous itemChanged() signal.
    if (item->data(VisibilityRole).isNull() ||
        item->data(VisibilityRole).toInt() != visible)
      {
      item->setData(visible, VisibilityRole);
      switch (visible)
        {
        case 0:
          item->setIcon(d->HiddenIcon);
          break;
        case 1:
          item->setIcon(d->VisibleIcon);
          break;
        case 2:
          item->setIcon(d->PartiallyVisibleIcon);
          break;
        default:
          // can get here if not a display or displayable node
          //qWarning() << "Unsupported visibility value: " << visible;
          break;
        }
      }
    }
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::updateNodeFromItem(vtkDMMLNode* node, QStandardItem* item)
{
  int wasModifying = node->StartModify();
  this->updateNodeFromItemData(node, item);
  node->EndModify(wasModifying);

  // the following only applies to tree hierarchies
  if (!this->canBeAChild(node))
    {
    return;
    }

 Q_ASSERT(node != this->dmmlNodeFromItem(item->parent()));

  QStandardItem* parentItem = item->parent();
  int columnCount = parentItem ? parentItem->columnCount() : 0;

  // Don't do the following if the row is not complete (reparenting an
  // incomplete row might lead to errors). (if there is no child yet for a given
  // column, it will get there next time updateNodeFromItem is called).
  // updateNodeFromItem() is called for every item drag&dropped (we ensure that
  // all the indexes of the row are reparented when entering the d&d function
  for (int i = 0; i < columnCount; ++i)
    {
    if (parentItem->child(item->row(), i) == nullptr)
      {
      return;
      }
    }

  vtkDMMLNode* parent = this->dmmlNodeFromItem(parentItem);
  int desiredNodeIndex = -1;
  if (this->parentNode(node) != parent)
    {
    emit aboutToReparentByDragAndDrop(node, parent);
    if (this->reparent(node, parent))
      {
      emit reparentedByDragAndDrop(node, parent);
      }
    else
      {
      this->updateItemFromNode(item, node, item->column());
      }
    }
  else if ((desiredNodeIndex = this->nodeIndex(node)) != item->row())
    {
    QStandardItem* parentItem = item->parent();
    if (parentItem && desiredNodeIndex <
          (parentItem->rowCount() - this->postItems(parentItem).count()))
      {
      this->updateItemFromNode(item, node, item->column());
      }
    }
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::updateNodeFromItemData(vtkDMMLNode* node, QStandardItem* item)
{
  if (item->column() == this->nameColumn())
    {
    node->SetName(item->text().toUtf8());
    }
  // ToolTip can't be edited, don't change the node
  // if (item->column() == this->toolTipNameColumn())
  // {
  // }
  if (item->column() == this->idColumn())
    {
    // Too dangerous
    //node->SetName(item->text().toUtf8());
    }
  if (item->column() == this->checkableColumn())
    {
    node->SetSelected(item->checkState() == Qt::Checked ? 1 : 0);
    }
  if (item->column() == this->visibilityColumn())
    {
    vtkDMMLDisplayNode* displayNode = vtkDMMLDisplayNode::SafeDownCast(node);
    vtkDMMLDisplayableNode* displayableNode =
      vtkDMMLDisplayableNode::SafeDownCast(node);
    vtkDMMLDisplayableHierarchyNode* displayableHierarchyNode =
      vtkDMMLDisplayableHierarchyNode::SafeDownCast(node);
    if (displayableHierarchyNode)
      {
      displayNode = displayableHierarchyNode->GetDisplayNode();
      }
    Q_ASSERT(!item->data(VisibilityRole).isNull());
    int visible = item->data(VisibilityRole).toInt();
    if (displayNode)
      {
      displayNode->SetVisibility(visible);
      }
    else if (displayableNode)
      {
      displayableNode->SetDisplayVisibility(visible);
      }
    }
}

//-----------------------------------------------------------------------------
void qDMMLSceneModel::onDMMLSceneEvent(vtkObject* vtk_obj, unsigned long event,
                                        void* client_data, void* call_data)
{
  vtkDMMLScene* scene = reinterpret_cast<vtkDMMLScene*>(vtk_obj);
  qDMMLSceneModel* sceneModel = reinterpret_cast<qDMMLSceneModel*>(client_data);
  vtkDMMLNode* node = reinterpret_cast<vtkDMMLNode*>(call_data);
  Q_ASSERT(scene);
  Q_ASSERT(sceneModel);
  switch(event)
    {
    case vtkDMMLScene::NodeAboutToBeAddedEvent:
      Q_ASSERT(node);
      sceneModel->onDMMLSceneNodeAboutToBeAdded(scene, node);
      break;
    case vtkDMMLScene::NodeAddedEvent:
      Q_ASSERT(node);
      sceneModel->onDMMLSceneNodeAdded(scene, node);
      break;
    case vtkDMMLScene::NodeAboutToBeRemovedEvent:
      Q_ASSERT(node);
      sceneModel->onDMMLSceneNodeAboutToBeRemoved(scene, node);
      break;
    case vtkDMMLScene::NodeRemovedEvent:
      Q_ASSERT(node);
      sceneModel->onDMMLSceneNodeRemoved(scene, node);
      break;
    case vtkCommand::DeleteEvent:
      sceneModel->onDMMLSceneDeleted(scene);
      break;
    case vtkDMMLScene::StartCloseEvent:
      sceneModel->onDMMLSceneAboutToBeClosed(scene);
      break;
    case vtkDMMLScene::EndCloseEvent:
      sceneModel->onDMMLSceneClosed(scene);
      break;
    case vtkDMMLScene::StartImportEvent:
      sceneModel->onDMMLSceneAboutToBeImported(scene);
      break;
    case vtkDMMLScene::EndImportEvent:
      sceneModel->onDMMLSceneImported(scene);
      break;
    case vtkDMMLScene::StartBatchProcessEvent:
      sceneModel->onDMMLSceneStartBatchProcess(scene);
      break;
    case vtkDMMLScene::EndBatchProcessEvent:
      sceneModel->onDMMLSceneEndBatchProcess(scene);
      break;
    }
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::onDMMLSceneNodeAboutToBeAdded(vtkDMMLScene* scene, vtkDMMLNode* node)
{
  Q_UNUSED(scene);
  Q_UNUSED(node);
#ifndef QT_NO_DEBUG
  Q_D(qDMMLSceneModel);
  Q_ASSERT(scene != nullptr);
  Q_ASSERT(scene == d->DMMLScene);
#endif
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::onDMMLSceneNodeAdded(vtkDMMLScene* scene, vtkDMMLNode* node)
{
  Q_D(qDMMLSceneModel);
  Q_UNUSED(d);
  Q_UNUSED(scene);
  Q_ASSERT(scene == d->DMMLScene);
  Q_ASSERT(vtkDMMLNode::SafeDownCast(node));

  if (d->DMMLScene->IsImporting() || (d->LazyUpdate && d->DMMLScene->IsBatchProcessing()))
    {
    // Node IDs and references are not valid until the import is completed, therefore do not attempt
    // to add a node during importing (see https://issues.slicer.org/view.php?id=4080).
    return;
    }
  this->insertNode(node);
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::onDMMLSceneNodeAboutToBeRemoved(vtkDMMLScene* scene, vtkDMMLNode* node)
{
  Q_D(qDMMLSceneModel);
  Q_UNUSED(d);
  Q_UNUSED(scene);
  Q_ASSERT(scene == d->DMMLScene);

  if (d->DMMLScene->IsClosing() || (d->LazyUpdate && d->DMMLScene->IsBatchProcessing()))
    {
    return;
    }

  int connectionsRemoved =
    qvtkDisconnect(node, vtkCommand::ModifiedEvent,
                   this, SLOT(onDMMLNodeModified(vtkObject*)));

  Q_ASSERT_X(((d->ListenNodeModifiedEvent == NoNodes) && connectionsRemoved == 0) ||
             (d->ListenNodeModifiedEvent != NoNodes && connectionsRemoved <= 1),
             "qDMMLSceneModel::onDMMLSceneNodeAboutToBeRemoved()",
             "A node has been removed from the scene but the scene model has "
             "never been notified it has been added in the first place. Maybe"
             " vtkDMMLScene::AddNodeNoNotify() has been used instead of "
             "vtkDMMLScene::AddNode");
  Q_UNUSED(connectionsRemoved);
  // Remove all the observations on the node
  qvtkDisconnect(node, vtkCommand::NoEvent, this, nullptr);

  // TODO: can be fasten by browsing the tree only once
  QModelIndexList indexes = this->match(this->dmmlSceneIndex(), qDMMLSceneModel::UIDRole,
                                        QString(node->GetID()), 1,
                                        Qt::MatchExactly | Qt::MatchRecursive);
  if (indexes.count())
    {
    QStandardItem* item = this->itemFromIndex(indexes[0].sibling(indexes[0].row(),0));
    // The children may be lost if not reparented, we ensure they got reparented.
    while (item->rowCount())
      {
      // we need to remove the children from the node to remove because they
      // would be automatically deleted in QStandardItemModel::removeRow()
      d->Orphans.push_back(item->takeRow(0));
      }
    // Remove the item from any orphan list if it exist as we don't want to
    // add it back later in onDMMLSceneNodeRemoved
    foreach(QList<QStandardItem*> orphans, d->Orphans)
      {
      if (orphans.contains(item))
        {
        d->Orphans.removeAll(orphans);
        }
      }
    this->removeRow(indexes[0].row(), indexes[0].parent());
    }
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::onDMMLSceneNodeRemoved(vtkDMMLScene* scene, vtkDMMLNode* node)
{
  Q_D(qDMMLSceneModel);
  Q_UNUSED(scene);
  Q_UNUSED(node);
  if (d->DMMLScene->IsClosing() || (d->LazyUpdate && d->DMMLScene->IsBatchProcessing()))
    {
    return;
    }
  // The removed node may had children, if they haven't been updated, they
  // are likely to be lost (not reachable when browsing the model), we need
  // to reparent them.
  foreach(QList<QStandardItem*> orphans, d->Orphans)
    {
    QStandardItem* orphan = orphans[0];
    // Make sure that the orphans have not already been reparented.
    if (orphan->parent())
      {
      // Not sure how it is possible, but if it is, then we might want to
      // review the logic behind.
      Q_ASSERT(orphan->parent() == nullptr);
      continue;
      }
    vtkDMMLNode* node = this->dmmlNodeFromItem(orphan);
    int newIndex = this->nodeIndex(node);
    QStandardItem* newParentItem = this->itemFromNode(this->parentNode(node));
    if (newParentItem == nullptr)
      {
      newParentItem = this->dmmlSceneItem();
      }
    Q_ASSERT(newParentItem);
    d->reparentItems(orphans, newIndex, newParentItem);
    }
  d->Orphans.clear();
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::onDMMLSceneDeleted(vtkDMMLScene* scene)
{
  Q_UNUSED(scene);
#ifndef QT_NO_DEBUG
  Q_D(qDMMLSceneModel);
  Q_ASSERT(scene == d->DMMLScene.GetPointer());  // GetPointer() is needed because on certain compilers (scene == d->DMMLScene) comparison is ambiguous
#endif
  this->setDMMLScene(nullptr);
}

//------------------------------------------------------------------------------
void printStandardItem(QStandardItem* item, const QString& offset)
{
  if (!item)
    {
    return;
    }
  qDebug() << offset << item << item->index() << item->text()
           << item->data(qDMMLSceneModel::UIDRole).toString() << item->row()
           << item->column() << item->rowCount() << item->columnCount();
  for(int i = 0; i < item->rowCount(); ++i )
    {
    for (int j = 0; j < item->columnCount(); ++j)
      {
      printStandardItem(item->child(i,j), offset + "   ");
      }
    }
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::updateNodeItems()
{
  QStandardItem* sceneItem = this->dmmlSceneItem();
  if (sceneItem == nullptr)
    {
    return;
    }
  sceneItem->setColumnCount(this->columnCount());
  vtkCollection* nodes = this->dmmlScene()->GetNodes();
  vtkDMMLNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);
       (node = (vtkDMMLNode*)nodes->GetNextItemAsObject(it)) ;)
    {
    this->updateNodeItems(node, QString(node->GetID()));
    }
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::onDMMLNodeModified(vtkObject* node)
{
  vtkDMMLNode* modifiedNode = vtkDMMLNode::SafeDownCast(node);
  this->updateNodeItems(modifiedNode, QString(modifiedNode->GetID()));
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::onDMMLNodeIDChanged(vtkObject* node, void* callData)
{
  char* oldID = reinterpret_cast<char *>(callData);
  this->updateNodeItems(vtkDMMLNode::SafeDownCast(node), QString(oldID));
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::updateNodeItems(vtkDMMLNode* node, const QString& nodeUID)
{
  Q_D(qDMMLSceneModel);

  if (d->DMMLScene->IsClosing() || d->DMMLScene->IsImporting() || (d->LazyUpdate && d->DMMLScene->IsBatchProcessing()))
    {
    return;
    }

  // If there is no node here or if the node has no scene. that means the node
  // has been removed from the scene but the scene model hasn't been notified
  // or that the scene model is still observing the node (in a subclass).
  // Another reason is that the scene has been closed, and when triggering the
  // SceneClosed event to all observers, one observer modifies the node and
  // the scene model has not yet been notified the scene was closed.
  Q_ASSERT(node);
  if (!node || !node->GetScene())
    {
    return;
    }
  //Q_ASSERT(node->GetScene()->IsNodePresent(node));
  QModelIndexList nodeIndexes = d->indexes(nodeUID);
  //qDebug() << "onDMMLNodeModified" << node->GetID() << nodeIndexes;
  Q_ASSERT(nodeIndexes.count());
  for (int i = 0; i < nodeIndexes.size(); ++i)
    {
    QModelIndex index = nodeIndexes[i];
    QStandardItem* item = this->itemFromIndex(index);
    int oldRow = item->row();
    QStandardItem* oldParent = item->parent();

    this->updateItemFromNode(item, node, item->column());
    // maybe the item has been reparented, then we need to rescan the
    // indexes again as they may be wrong.
    if (item->row() != oldRow || item->parent() != oldParent)
      {
      int oldSize = nodeIndexes.size();
      nodeIndexes = this->indexes(node);
      int newSize = nodeIndexes.size();
      // the number of columns shouldn't change
      Q_ASSERT(oldSize == newSize);
      Q_UNUSED(oldSize);
      Q_UNUSED(newSize);
      }
    }
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::onItemChanged(QStandardItem * item)
{
  Q_D(qDMMLSceneModel);

  if (d->PendingItemModified >= 0)
    {
    ++d->PendingItemModified;
    return;
    }
  // when a dnd occurs, the order of the items called with onItemChanged is
  // random, it could be the item in column 1 then the item in column 0
  //qDebug() << "onItemChanged: " << item << item->row() << item->column() << d->DraggedNodes.count();
  //printStandardItem(this->dmmlSceneItem(), "");
  //return;
  // check on the column is optional(no strong feeling), it is just there to be
  // faster though
  if (!this->isANode(item))
    {
    return;
    }

  if (d->DraggedNodes.count())
    {
    if (item->column() == 0)
      {
      //this->metaObject()->invokeMethod(
      //  this, "onItemChanged", Qt::QueuedConnection, Q_ARG(QStandardItem*, item));
      d->DraggedItem = item;
      QTimer::singleShot(200, this, SLOT(delayedItemChanged()));
      }
    return;
    }

  // Only nodes can be changed, scene and extra items should be not editable
  vtkDMMLNode* dmmlNode = this->dmmlNodeFromItem(item);
  Q_ASSERT(dmmlNode);
  if (dmmlNode==nullptr)
  {
    qCritical() << "qDMMLSceneModel::onItemChanged: Failed to get DMML node from scene model item";
    return;
  }
  this->updateNodeFromItem(dmmlNode, item);
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::delayedItemChanged()
{
  Q_D(qDMMLSceneModel);
  // Clear d->DraggedNodes before calling onItemChanged
  // to make process item changes immediately (instead of
  // triggering another delayed update)
  d->DraggedNodes.clear();

  this->onItemChanged(d->DraggedItem);
  d->DraggedItem = nullptr;
}

//------------------------------------------------------------------------------
bool qDMMLSceneModel::isANode(const QStandardItem * item)const
{
  Q_D(const qDMMLSceneModel);
  return item
    && item != this->dmmlSceneItem()
    && !d->isExtraItem(item);
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::onDMMLSceneAboutToBeImported(vtkDMMLScene* scene)
{
  Q_UNUSED(scene);
  //this->beginResetModel();
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::onDMMLSceneImported(vtkDMMLScene* scene)
{
  Q_D(qDMMLSceneModel);
  Q_UNUSED(scene);
  // Node IDs and references are not valid until the import is completed,
  // therefore we must update the model now (see https://issues.slicer.org/view.php?id=4080).
  this->updateScene();
  //this->endResetModel();
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::onDMMLSceneAboutToBeClosed(vtkDMMLScene* scene)
{
  Q_UNUSED(scene);
  //this->beginResetModel();
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::onDMMLSceneClosed(vtkDMMLScene* scene)
{
  Q_UNUSED(scene);
  this->updateScene();
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::onDMMLSceneStartBatchProcess(vtkDMMLScene* scene)
{
  Q_D(qDMMLSceneModel);
  Q_UNUSED(scene);
  if (d->LazyUpdate)
    {
    emit sceneAboutToBeUpdated();
    }
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::onDMMLSceneEndBatchProcess(vtkDMMLScene* scene)
{
  Q_D(qDMMLSceneModel);
  Q_UNUSED(scene);
  if (d->LazyUpdate)
    {
    this->updateScene();
    emit sceneUpdated();
    }
}

//------------------------------------------------------------------------------
Qt::DropActions qDMMLSceneModel::supportedDropActions()const
{
  return Qt::IgnoreAction;
}

//------------------------------------------------------------------------------
int qDMMLSceneModel::nameColumn()const
{
  Q_D(const qDMMLSceneModel);
  return d->NameColumn;
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::setNameColumn(int column)
{
  Q_D(qDMMLSceneModel);
  d->NameColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qDMMLSceneModel::idColumn()const
{
  Q_D(const qDMMLSceneModel);
  return d->IDColumn;
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::setIDColumn(int column)
{
  Q_D(qDMMLSceneModel);
  d->IDColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qDMMLSceneModel::checkableColumn()const
{
  Q_D(const qDMMLSceneModel);
  return d->CheckableColumn;
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::setCheckableColumn(int column)
{
  Q_D(qDMMLSceneModel);
  d->CheckableColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qDMMLSceneModel::visibilityColumn()const
{
  Q_D(const qDMMLSceneModel);
  return d->VisibilityColumn;
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::setVisibilityColumn(int column)
{
  Q_D(qDMMLSceneModel);
  d->VisibilityColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qDMMLSceneModel::toolTipNameColumn()const
{
  Q_D(const qDMMLSceneModel);
  return d->ToolTipNameColumn;
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::setToolTipNameColumn(int column)
{
  Q_D(qDMMLSceneModel);
  d->ToolTipNameColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qDMMLSceneModel::extraItemColumn()const
{
  Q_D(const qDMMLSceneModel);
  return d->ExtraItemColumn;
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::setExtraItemColumn(int column)
{
  Q_D(qDMMLSceneModel);
  d->ExtraItemColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
void qDMMLSceneModel::updateColumnCount()
{
  int max = this->maxColumnId();
  int oldColumnCount = this->columnCount();
  this->setColumnCount(max + 1);
  if (oldColumnCount == 0)
    {
    this->updateScene();
    }
  else
    {
    this->updateNodeItems();
    }
}

//------------------------------------------------------------------------------
int qDMMLSceneModel::maxColumnId()const
{
  Q_D(const qDMMLSceneModel);
  int maxId = 0; // information (scene, node uid... ) is stored in the 1st column
  maxId = qMax(maxId, d->NameColumn);
  maxId = qMax(maxId, d->IDColumn);
  maxId = qMax(maxId, d->CheckableColumn);
  maxId = qMax(maxId, d->VisibilityColumn);
  maxId = qMax(maxId, d->ToolTipNameColumn);
  maxId = qMax(maxId, d->ExtraItemColumn);
  return maxId;
}
