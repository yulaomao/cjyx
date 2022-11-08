/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QMimeData>
#include <QApplication>
#include <QMessageBox>
#include <QTimer>
#include <QUrl>

// CTK includes
#include <ctkUtils.h>

// qDMML includes
#include "qDMMLSubjectHierarchyModel_p.h"

// Cjyx includes
#include <qCjyxApplication.h>
#include <qCjyxCoreApplication.h>
#include <qCjyxModuleManager.h>
#include <qCjyxAbstractCoreModule.h>

// DMML includes
#include <vtkDMMLSubjectHierarchyNode.h>
#include <vtkDMMLDisplayableNode.h>
#include <vtkDMMLDisplayNode.h>
#include <vtkDMMLTransformNode.h>
#include <vtkDMMLScene.h>

// Terminologies includes
#include "qCjyxTerminologyItemDelegate.h"
#include "vtkCjyxTerminologyEntry.h"
#include "vtkCjyxTerminologiesModuleLogic.h"

// Subject Hierarchy includes
#include "vtkCjyxSubjectHierarchyModuleLogic.h"
#include "qCjyxSubjectHierarchyPluginHandler.h"
#include "qCjyxSubjectHierarchyAbstractPlugin.h"
#include "qCjyxSubjectHierarchyDefaultPlugin.h"


//------------------------------------------------------------------------------
qDMMLSubjectHierarchyModelPrivate::qDMMLSubjectHierarchyModelPrivate(qDMMLSubjectHierarchyModel& object)
  : q_ptr(&object)
  , NameColumn(-1)
  , IDColumn(-1)
  , VisibilityColumn(-1)
  , ColorColumn(-1)
  , TransformColumn(-1)
  , DescriptionColumn(-1)
  , NoneEnabled(false)
  , NoneDisplay(qDMMLSubjectHierarchyModel::tr("None"))
  , SubjectHierarchyNode(nullptr)
  , DMMLScene(nullptr)
  , TerminologiesModuleLogic(nullptr)
  , IsDroppedInside(false)
{
  this->CallBack = vtkSmartPointer<vtkCallbackCommand>::New();
  this->PendingItemModified = -1; // -1 means not updating

  this->HiddenIcon = QIcon(":Icons/VisibleOff.png");
  this->VisibleIcon = QIcon(":Icons/VisibleOn.png");
  this->PartiallyVisibleIcon = QIcon(":Icons/VisiblePartially.png");

  this->UnknownIcon = QIcon(":Icons/Unknown.png");
  this->WarningIcon = QIcon(":Icons/Warning.png");

  this->NoTransformIcon = QIcon(":/Icons/NoTransform.png");
  this->FolderTransformIcon = QIcon(":/Icons/FolderTransform.png");
  this->LinearTransformIcon = QIcon(":/Icons/LinearTransform.png");
  this->DeformableTransformIcon = QIcon(":Icons/DeformableTransform.png");

  this->DelayedItemChangedInvoked = false;

  qRegisterMetaType<QStandardItem*>("QStandardItem*");
}

//------------------------------------------------------------------------------
qDMMLSubjectHierarchyModelPrivate::~qDMMLSubjectHierarchyModelPrivate()
{
  if (this->SubjectHierarchyNode)
    {
    this->SubjectHierarchyNode->RemoveObserver(this->CallBack);
    }
  if (this->DMMLScene)
    {
    this->DMMLScene->RemoveObserver(this->CallBack);
    }
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModelPrivate::init()
{
  Q_Q(qDMMLSubjectHierarchyModel);
  this->CallBack->SetClientData(q);
  this->CallBack->SetCallback(qDMMLSubjectHierarchyModel::onEvent);

  QObject::connect(q, SIGNAL(itemChanged(QStandardItem*)), q, SLOT(onItemChanged(QStandardItem*)));

  q->setNameColumn(0);
  q->setDescriptionColumn(1);
  q->setVisibilityColumn(2);
  q->setColorColumn(3);
  q->setTransformColumn(4);
  q->setIDColumn(5);

  q->setHorizontalHeaderLabels(
    QStringList() << "Node" << "Description" << "" /*visibility*/ << "" /*color*/ << "" /*transform*/ << "IDs" );

  q->horizontalHeaderItem(q->nameColumn())->setToolTip(qDMMLSubjectHierarchyModel::tr("Node name and type"));
  q->horizontalHeaderItem(q->descriptionColumn())->setToolTip(qDMMLSubjectHierarchyModel::tr("Node description"));
  q->horizontalHeaderItem(q->visibilityColumn())->setToolTip(qDMMLSubjectHierarchyModel::tr("Show/hide branch or node"));
  q->horizontalHeaderItem(q->colorColumn())->setToolTip(qDMMLSubjectHierarchyModel::tr("Node color"));
  q->horizontalHeaderItem(q->transformColumn())->setToolTip(qDMMLSubjectHierarchyModel::tr("Applied transform"));
  q->horizontalHeaderItem(q->idColumn())->setToolTip(qDMMLSubjectHierarchyModel::tr("Node ID"));

  q->horizontalHeaderItem(q->visibilityColumn())->setIcon(QIcon(":/Icons/Small/CjyxVisibleInvisible.png"));
  q->horizontalHeaderItem(q->colorColumn())->setIcon(QIcon(":/Icons/Colors.png"));
  q->horizontalHeaderItem(q->transformColumn())->setIcon(QIcon(":/Icons/Transform.png"));

  // Set visibility icons from model to the default plugin
  qCjyxSubjectHierarchyPluginHandler::instance()->defaultPlugin()->setDefaultVisibilityIcons(
    this->VisibleIcon, this->HiddenIcon, this->PartiallyVisibleIcon );
}

//------------------------------------------------------------------------------
QString qDMMLSubjectHierarchyModelPrivate::subjectHierarchyItemName(vtkIdType itemID)
{
  if (!this->SubjectHierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return "Error";
    }
  return QString(this->SubjectHierarchyNode->GetItemName(itemID).c_str());
}

//------------------------------------------------------------------------------
QStandardItem* qDMMLSubjectHierarchyModelPrivate::insertSubjectHierarchyItem(vtkIdType itemID, int index)
{
  Q_Q(qDMMLSubjectHierarchyModel);
  QStandardItem* item = q->itemFromSubjectHierarchyItem(itemID);
  if (item)
    {
    // It is possible that the item has been already added if it is the parent of a child item already inserted
    return item;
    }
  vtkIdType parentItemID = q->parentSubjectHierarchyItem(itemID);
  QStandardItem* parentItem = q->itemFromSubjectHierarchyItem(parentItemID);
  if (!parentItem)
    {
    if (!parentItemID)
      {
      qCritical() << Q_FUNC_INFO << ": Unable to get parent for subject hierarchy item with ID " << itemID;
      return nullptr;
      }
    parentItem = q->insertSubjectHierarchyItem(parentItemID);
    if (!parentItem)
      {
      qCritical() << Q_FUNC_INFO << ": Failed to insert parent subject hierarchy item with ID " << parentItemID;
      return nullptr;
      }
    }
  item = q->insertSubjectHierarchyItem(itemID, parentItem, index);
  if (q->itemFromSubjectHierarchyItem(itemID) != item)
    {
    qCritical() << Q_FUNC_INFO << ": Item mismatch when inserting subject hierarchy item with ID " << itemID;
    return nullptr;
    }
  return item;
}

//------------------------------------------------------------------------------
vtkCjyxTerminologiesModuleLogic* qDMMLSubjectHierarchyModelPrivate::terminologiesModuleLogic()
{
  if (this->TerminologiesModuleLogic)
    {
    return this->TerminologiesModuleLogic;
    }
  vtkCjyxTerminologiesModuleLogic* terminologiesLogic = vtkCjyxTerminologiesModuleLogic::SafeDownCast(
    qCjyxCoreApplication::application()->moduleLogic("Terminologies"));
  return terminologiesLogic;
}


//------------------------------------------------------------------------------
// qDMMLSubjectHierarchyModel
//------------------------------------------------------------------------------
qDMMLSubjectHierarchyModel::qDMMLSubjectHierarchyModel(QObject *_parent)
  :QStandardItemModel(_parent)
  , d_ptr(new qDMMLSubjectHierarchyModelPrivate(*this))
{
  Q_D(qDMMLSubjectHierarchyModel);
  d->init();
}

//------------------------------------------------------------------------------
qDMMLSubjectHierarchyModel::qDMMLSubjectHierarchyModel(qDMMLSubjectHierarchyModelPrivate* pimpl, QObject* parent)
  : QStandardItemModel(parent)
  , d_ptr(pimpl)
{
  Q_D(qDMMLSubjectHierarchyModel);
  d->init();
}

//------------------------------------------------------------------------------
qDMMLSubjectHierarchyModel::~qDMMLSubjectHierarchyModel() = default;

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::setDMMLScene(vtkDMMLScene* scene)
{
  Q_D(qDMMLSubjectHierarchyModel);
  if (scene == d->DMMLScene)
    {
    return;
    }

  if (d->DMMLScene)
    {
    d->DMMLScene->RemoveObserver(d->CallBack);
    }

  d->DMMLScene = scene;
  this->setSubjectHierarchyNode(scene ? vtkDMMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene) : nullptr);

  if (scene)
    {
    scene->AddObserver(vtkDMMLScene::EndCloseEvent, d->CallBack);
    scene->AddObserver(vtkDMMLScene::EndImportEvent, d->CallBack);
    scene->AddObserver(vtkDMMLScene::StartBatchProcessEvent, d->CallBack);
    scene->AddObserver(vtkDMMLScene::EndBatchProcessEvent, d->CallBack);
    scene->AddObserver(vtkDMMLScene::NodeRemovedEvent, d->CallBack);
    }
}

//------------------------------------------------------------------------------
vtkDMMLScene* qDMMLSubjectHierarchyModel::dmmlScene()const
{
  Q_D(const qDMMLSubjectHierarchyModel);
  return d->DMMLScene;
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::setSubjectHierarchyNode(vtkDMMLSubjectHierarchyNode* shNode)
{
  Q_D(qDMMLSubjectHierarchyModel);
  if (shNode == d->SubjectHierarchyNode)
    {
    return;
    }

  if (d->SubjectHierarchyNode)
    {
    d->SubjectHierarchyNode->RemoveObserver(d->CallBack);
    }

  d->SubjectHierarchyNode = shNode;

  // Remove all items
  const int oldColumnCount = this->columnCount();
  this->removeRows(0, this->rowCount());
  this->setColumnCount(oldColumnCount);

  // Update whole subject hierarchy
  this->rebuildFromSubjectHierarchy();

  if (shNode)
    {
    // Using priority value of -10 in certain observations results in those callbacks being called after
    // those with neutral priorities. Useful to have the plugin handler deal with new items before allowing
    // them to be handled by the model.
    // Same idea for +10, in which case the callback is called first.
    shNode->AddObserver(vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemAddedEvent, d->CallBack, -10.0);
    shNode->AddObserver(vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemAboutToBeRemovedEvent, d->CallBack, +10.0);
    shNode->AddObserver(vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemRemovedEvent, d->CallBack, -10.0);
    shNode->AddObserver(vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemModifiedEvent, d->CallBack, -10.0);
    shNode->AddObserver(vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemTransformModifiedEvent, d->CallBack, -10.0);
    shNode->AddObserver(vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemDisplayModifiedEvent, d->CallBack, -10.0);
    shNode->AddObserver(vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemReparentedEvent, d->CallBack, -10.0);
    }
}

//------------------------------------------------------------------------------
vtkDMMLSubjectHierarchyNode* qDMMLSubjectHierarchyModel::subjectHierarchyNode()const
{
  Q_D(const qDMMLSubjectHierarchyModel);
  return d->SubjectHierarchyNode;
}

//------------------------------------------------------------------------------
QStandardItem* qDMMLSubjectHierarchyModel::subjectHierarchySceneItem()const
{
  Q_D(const qDMMLSubjectHierarchyModel);
  if (!d->SubjectHierarchyNode || this->maxColumnId() == -1)
    {
    return nullptr;
    }
  int count = this->invisibleRootItem()->rowCount();
  for (int row=0; row<count; ++row)
    {
    QStandardItem* child = this->invisibleRootItem()->child(row);
    if (!child)
      {
      continue;
      }
    QVariant uid = child->data(qDMMLSubjectHierarchyModel::SubjectHierarchyItemIDRole);
    if (uid.type() == QVariant::LongLong && uid == d->SubjectHierarchyNode->GetSceneItemID())
      {
      return child;
      }
    }
  return nullptr;
}

//------------------------------------------------------------------------------
QModelIndex qDMMLSubjectHierarchyModel::subjectHierarchySceneIndex()const
{
  QStandardItem* shSceneItem = this->subjectHierarchySceneItem();
  if (shSceneItem == nullptr)
    {
    return QModelIndex();
    }
  return shSceneItem ? shSceneItem->index() : QModelIndex();
}

// -----------------------------------------------------------------------------
vtkIdType qDMMLSubjectHierarchyModel::subjectHierarchyItemFromIndex(const QModelIndex &index)const
{
  return this->subjectHierarchyItemFromItem(this->itemFromIndex(index));
}

//------------------------------------------------------------------------------
vtkIdType qDMMLSubjectHierarchyModel::subjectHierarchyItemFromItem(QStandardItem* item)const
{
  Q_D(const qDMMLSubjectHierarchyModel);
  if (!d->SubjectHierarchyNode || !item)
    {
    return vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }
  QVariant shItemID = item->data(qDMMLSubjectHierarchyModel::SubjectHierarchyItemIDRole);
  if (!shItemID.isValid())
    {
    return vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }
  return item->data(qDMMLSubjectHierarchyModel::SubjectHierarchyItemIDRole).toLongLong();
}
//------------------------------------------------------------------------------
QStandardItem* qDMMLSubjectHierarchyModel::itemFromSubjectHierarchyItem(vtkIdType itemID, int column/*=0*/)const
{
  QModelIndex index = this->indexFromSubjectHierarchyItem(itemID, column);
  QStandardItem* item = this->itemFromIndex(index);
  return item;
}

//------------------------------------------------------------------------------
QModelIndex qDMMLSubjectHierarchyModel::indexFromSubjectHierarchyItem(vtkIdType itemID, int column/*=0*/)const
{
  Q_D(const qDMMLSubjectHierarchyModel);

  QModelIndex itemIndex;
  if (!itemID)
    {
    return itemIndex;
    }

  // Try to find the nodeIndex in the cache first
  QMap<vtkIdType,QPersistentModelIndex>::iterator rowCacheIt = d->RowCache.find(itemID);
  if (rowCacheIt==d->RowCache.end())
    {
    // Not found in cache, therefore it cannot be in the model
    return itemIndex;
    }
  if (rowCacheIt.value().isValid())
    {
    // An entry found in the cache. If the item at the cached index matches the requested item ID then we use it.
    QStandardItem* item = this->itemFromIndex(rowCacheIt.value());
    if (item && item->data(qDMMLSubjectHierarchyModel::SubjectHierarchyItemIDRole).toLongLong() == itemID)
      {
      // ID matched
      itemIndex = rowCacheIt.value();
      }
    }

  // The cache was not up-to-date. Do a slow linear search.
  if (!itemIndex.isValid())
    {
    // QAbstractItemModel::match doesn't browse through columns, we need to do it manually
    QModelIndexList itemIndexes = this->match(
      this->subjectHierarchySceneIndex(), SubjectHierarchyItemIDRole, itemID, 1, Qt::MatchExactly | Qt::MatchRecursive);
    if (itemIndexes.size() == 0)
      {
      d->RowCache.remove(itemID);
      return QModelIndex();
      }
    itemIndex = itemIndexes[0];
    d->RowCache[itemID] = itemIndex;
    }
  if (column == 0)
    {
    // QAbstractItemModel::match only search through the first column
    return itemIndex;
    }
  // Add the QModelIndexes from the other columns
  const int row = itemIndex.row();
  QModelIndex nodeParentIndex = itemIndex.parent();
  if (column >= this->columnCount(nodeParentIndex))
    {
    qCritical() << Q_FUNC_INFO << ": Invalid column " << column;
    return QModelIndex();
    }
  return ctk::modelChildIndex(const_cast<qDMMLSubjectHierarchyModel*>(this), nodeParentIndex, row, column);
}

//------------------------------------------------------------------------------
QModelIndexList qDMMLSubjectHierarchyModel::indexes(vtkIdType itemID)const
{
  QModelIndex scene = this->subjectHierarchySceneIndex();
  if (scene == QModelIndex())
    {
    return QModelIndexList();
    }
  // QAbstractItemModel::match doesn't browse through columns, we need to do it manually
  QModelIndexList shItemIndexes = this->match(
    scene, qDMMLSubjectHierarchyModel::SubjectHierarchyItemIDRole, QVariant(qlonglong(itemID)), 1, Qt::MatchExactly | Qt::MatchRecursive);
  if (shItemIndexes.size() != 1)
    {
    return QModelIndexList(); // If 0 it's empty, if >1 it's invalid (one item for each UID)
    }
  // Add the QModelIndexes from the other columns
  const int row = shItemIndexes[0].row();
  QModelIndex shItemParentIndex = shItemIndexes[0].parent();
  const int sceneColumnCount = this->columnCount(shItemParentIndex);
  for (int col=1; col<sceneColumnCount; ++col)
    {
    shItemIndexes << this->index(row, col, shItemParentIndex);
    }
  return shItemIndexes;
}

//------------------------------------------------------------------------------
vtkIdType qDMMLSubjectHierarchyModel::parentSubjectHierarchyItem(vtkIdType itemID)const
{
  Q_D(const qDMMLSubjectHierarchyModel);
  if (!d->SubjectHierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }
  return d->SubjectHierarchyNode->GetItemParent(itemID);
}

//------------------------------------------------------------------------------
int qDMMLSubjectHierarchyModel::subjectHierarchyItemIndex(vtkIdType itemID)const
{
  Q_D(const qDMMLSubjectHierarchyModel);
  if (!d->SubjectHierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return -1;
    }
  int extraItemCount = 0;
  if (d->NoneEnabled && d->SubjectHierarchyNode->GetItemParent(itemID) == d->SubjectHierarchyNode->GetSceneItemID())
    {
    ++extraItemCount;
    }
  return d->SubjectHierarchyNode->GetItemPositionUnderParent(itemID) + extraItemCount;
}

//------------------------------------------------------------------------------
bool qDMMLSubjectHierarchyModel::canBeAChild(vtkIdType itemID)const
{
  Q_D(const qDMMLSubjectHierarchyModel);
  if (!d->SubjectHierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return false;
    }
  // Only the root and invalid item cannot be child
  return (itemID && itemID != d->SubjectHierarchyNode->GetSceneItemID());
}

//------------------------------------------------------------------------------
bool qDMMLSubjectHierarchyModel::canBeAParent(vtkIdType itemID)const
{
  Q_D(const qDMMLSubjectHierarchyModel);
  if (!d->SubjectHierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return false;
    }
  // Only invalid item cannot be parent
  return (bool)itemID;
}

//------------------------------------------------------------------------------
bool qDMMLSubjectHierarchyModel::isAncestorItem(vtkIdType child, vtkIdType ancestor)const
{
  Q_D(const qDMMLSubjectHierarchyModel);
  if (!d->SubjectHierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return false;
    }

  for (; child != vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID; child = d->SubjectHierarchyNode->GetItemParent(child))
    {
    if (child == ancestor)
      {
      return true;
      }
    }
  return false;
}

//------------------------------------------------------------------------------
bool qDMMLSubjectHierarchyModel::isAffiliatedItem(vtkIdType itemA, vtkIdType itemB)const
  {
  return this->isAncestorItem(itemA, itemB) || this->isAncestorItem(itemB, itemA);
  }

//------------------------------------------------------------------------------
bool qDMMLSubjectHierarchyModel::reparent(vtkIdType itemID, vtkIdType newParentID)
{
  if (!itemID || !newParentID || newParentID == itemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input";
    return false;
    }

  vtkIdType oldParentID = this->parentSubjectHierarchyItem(itemID);
  if (oldParentID == newParentID)
    {
    return false;
    }

  Q_D(const qDMMLSubjectHierarchyModel);
  if (!d->SubjectHierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return false;
    }

  if (!this->canBeAParent(newParentID))
    {
    qCritical() << Q_FUNC_INFO << ": Target parent (" << d->SubjectHierarchyNode->GetItemName(newParentID).c_str() << ") is not a valid parent!";
    return false;
    }

  // If dropped from within the subject hierarchy tree
  QList<qCjyxSubjectHierarchyAbstractPlugin*> foundPlugins =
    qCjyxSubjectHierarchyPluginHandler::instance()->pluginsForReparentingItemInSubjectHierarchy(itemID, newParentID);
  qCjyxSubjectHierarchyAbstractPlugin* selectedPlugin = nullptr;
  if (foundPlugins.size() > 1)
    {
    // Let the user choose a plugin if more than one returned the same non-zero confidence value
    vtkDMMLNode* dataNode = d->SubjectHierarchyNode->GetItemDataNode(itemID);
    QString textToDisplay = QString(
      "Equal confidence number found for more than one subject hierarchy plugin for reparenting.\n\n"
      "Select plugin to reparent item\n'%1'\n(type %2)\nParent item: %3").arg(
      d->SubjectHierarchyNode->GetItemName(itemID).c_str()).arg(
      dataNode?dataNode->GetNodeTagName():d->SubjectHierarchyNode->GetItemLevel(itemID).c_str()).arg(
      d->SubjectHierarchyNode->GetItemName(newParentID).c_str() );
    selectedPlugin = qCjyxSubjectHierarchyPluginHandler::instance()->selectPluginFromDialog(textToDisplay, foundPlugins);
    }
  else if (foundPlugins.size() == 1)
    {
    selectedPlugin = foundPlugins[0];
    }
  else
    {
    // Choose default plugin if all registered plugins returned confidence value 0
    selectedPlugin = qCjyxSubjectHierarchyPluginHandler::instance()->defaultPlugin();
    }

  // If default plugin was chosen to reparent virtual item (an item in a virtual branch), or into a virtual branch,
  // then abort reparenting (it means that the actual owner plugin cannot reparent its own virtual item, so it then
  // cannot be reparented).
  if ( ( ( !d->SubjectHierarchyNode->GetItemAttribute(newParentID,
             vtkDMMLSubjectHierarchyConstants::GetSubjectHierarchyVirtualBranchAttributeName().c_str()).empty() )
      || ( !d->SubjectHierarchyNode->GetItemAttribute(oldParentID,
             vtkDMMLSubjectHierarchyConstants::GetSubjectHierarchyVirtualBranchAttributeName().c_str()).empty() ) )
    && selectedPlugin == qCjyxSubjectHierarchyPluginHandler::instance()->defaultPlugin() )
  {
    qCritical() << Q_FUNC_INFO << ": Failed to reparent virtual item "
      << d->SubjectHierarchyNode->GetItemName(itemID).c_str() << " under parent " << d->SubjectHierarchyNode->GetItemName(newParentID).c_str();
    return false;
  }

  // Have the selected plugin reparent the node
  bool successfullyReparentedByPlugin = selectedPlugin->reparentItemInsideSubjectHierarchy(itemID, newParentID);
  if (!successfullyReparentedByPlugin)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to reparent item "
      << d->SubjectHierarchyNode->GetItemName(itemID).c_str() << " through plugin '"
      << selectedPlugin->name().toUtf8().constData() << "'";
    return false;
    }

  return true;
}

//------------------------------------------------------------------------------
bool qDMMLSubjectHierarchyModel::moveToRow(vtkIdType itemID, int newRow)
{
  Q_D(const qDMMLSubjectHierarchyModel);
  if (!d->SubjectHierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return false;
    }

  if (!itemID || itemID == d->SubjectHierarchyNode->GetSceneItemID())
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item ID";
    return false;
    }

  vtkIdType parentItemID = this->parentSubjectHierarchyItem(itemID);
  if (!parentItemID)
    {
    qCritical() << Q_FUNC_INFO << ": No parent found for item " << itemID;
    return false;
    }

  // Get item currently next to desired position
  vtkIdType beforeItemID = d->SubjectHierarchyNode->GetItemByPositionUnderParent(parentItemID, newRow);

  // Move item to position
  return d->SubjectHierarchyNode->MoveItem(itemID, beforeItemID);
}

//------------------------------------------------------------------------------
QMimeData* qDMMLSubjectHierarchyModel::mimeData(const QModelIndexList& indexes)const
{
  Q_D(const qDMMLSubjectHierarchyModel);
  d->DraggedSubjectHierarchyItems.clear();
  const_cast<qDMMLSubjectHierarchyModelPrivate*>(d)->IsDroppedInside = false;
  if (!indexes.size())
    {
    return nullptr;
    }
  QList<QUrl> selectedShItemUrls;
  QModelIndexList allColumnsIndexes;
  foreach(const QModelIndex& index, indexes)
    {
    QModelIndex parent = index.parent();
    for (int column = 0; column < this->columnCount(parent); ++column)
      {
      allColumnsIndexes << this->index(index.row(), column, parent);
      }
    if (index.column() == 0) // Prevent duplicate IDs
      {
      vtkIdType itemId = this->subjectHierarchyItemFromIndex(index);
      d->DraggedSubjectHierarchyItems << itemId;
      QString urlString = QString("dmml://scene/subjecthierarchy/item?id=%1").arg(itemId);
      selectedShItemUrls.push_back(QUrl(urlString));
      }
    }
  // Remove duplicates (mixes up order of items)
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
  allColumnsIndexes = QSet<QModelIndex>(allColumnsIndexes.begin(), allColumnsIndexes.end()).values();
#else
  allColumnsIndexes = allColumnsIndexes.toSet().toList();
#endif

  QMimeData* mimeData = this->QStandardItemModel::mimeData(allColumnsIndexes);
  mimeData->setUrls(selectedShItemUrls);

  return mimeData;
}

//------------------------------------------------------------------------------
bool qDMMLSubjectHierarchyModel::dropMimeData( const QMimeData *data, Qt::DropAction action,
                                            int row, int column, const QModelIndex &parent )
{
  Q_D(qDMMLSubjectHierarchyModel);
  Q_UNUSED(column);
  // Prevent dropping above the None item
  if (d->NoneEnabled && row == 0 && parent == this->subjectHierarchySceneIndex())
    {
    return false;
    }
  // We want to do drag&drop only into the first item of a line (and not on a random column)
  d->IsDroppedInside = true;
  bool res = this->Superclass::dropMimeData(
    data, action, row, 0, parent.sibling(parent.row(), 0));
  return res;
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::rebuildFromSubjectHierarchy()
{
  Q_D(qDMMLSubjectHierarchyModel);

  d->RowCache.clear();

  // Enabled so it can be interacted with
  this->invisibleRootItem()->setFlags(Qt::ItemIsEnabled);

  if (!d->SubjectHierarchyNode)
    {
    // Remove all items
    const int oldColumnCount = this->columnCount();
    this->removeRows(0, this->rowCount());
    this->setColumnCount(oldColumnCount);
    return;
    }
  else if (!this->subjectHierarchySceneItem())
    {
    // No subject hierarchy root item has been created yet, but the subject hierarchy
    // node is valid, so we need to create a scene item
    vtkIdType sceneItemID = d->SubjectHierarchyNode->GetSceneItemID();
    QList<QStandardItem*> sceneItems;
    QStandardItem* sceneItem = new QStandardItem();
    sceneItem->setFlags(Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);
    sceneItem->setText("Scene");
    sceneItem->setData(sceneItemID, qDMMLSubjectHierarchyModel::SubjectHierarchyItemIDRole);
    sceneItems << sceneItem;
    for (int i = 1; i < this->columnCount(); ++i)
      {
      QStandardItem* sceneOtherColumn = new QStandardItem();
      sceneOtherColumn->setFlags(Qt::NoItemFlags);
      sceneItems << sceneOtherColumn;
      }
    sceneItem->setColumnCount(this->columnCount());

    d->RowCache[sceneItemID] = QModelIndex(); // Insert invalid item in cache to indicate that item is in the model but its index is unknown
    this->insertRow(0, sceneItems);
    d->RowCache[sceneItemID] = sceneItem->index();
    }
  else
    {
    // Update the scene item index in case subject hierarchy node has changed
    this->subjectHierarchySceneItem()->setData(
      QVariant::fromValue(d->SubjectHierarchyNode->GetSceneItemID()), qDMMLSubjectHierarchyModel::SubjectHierarchyItemIDRole );
    d->RowCache[d->SubjectHierarchyNode->GetSceneItemID()] = this->subjectHierarchySceneItem()->index();
    }

  if (!this->subjectHierarchySceneItem())
    {
    qCritical() << Q_FUNC_INFO << ": Failed to create subject hierarchy scene item";
    return;
    }

  // Remove rows before populating
  this->subjectHierarchySceneItem()->removeRows(0, this->subjectHierarchySceneItem()->rowCount());

  // Insert None item on top if enabled
  if (d->NoneEnabled)
    {
    QList<QStandardItem*> items;
    for (int col=0; col<this->columnCount(); ++col)
      {
      QStandardItem* newItem = new QStandardItem();
      newItem->setData(d->extraItemIdentifier(), Qt::WhatsThisRole);
      if (col == this->nameColumn())
        {
        newItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        newItem->setText(d->NoneDisplay);
        newItem->setToolTip("Indicate empty selection");
        }
      else
        {
        newItem->setFlags(Qt::NoItemFlags);
        }
      items.append(newItem);
      }
    this->subjectHierarchySceneItem()->insertRow(0, items);
    }

  // Populate subject hierarchy with the items
  std::vector<vtkIdType> allItemIDs;
  d->SubjectHierarchyNode->GetItemChildren(d->SubjectHierarchyNode->GetSceneItemID(), allItemIDs, true);
  for (std::vector<vtkIdType>::iterator itemIt=allItemIDs.begin(); itemIt!=allItemIDs.end(); ++itemIt)
    {
    vtkIdType itemID = (*itemIt);
    int index = this->subjectHierarchyItemIndex(itemID);
    d->insertSubjectHierarchyItem(itemID, index);
    }

  // Update expanded states (during inserting the update calls did not find valid indices, so
  // expand and collapse statuses were not set in the tree view)
  for (std::vector<vtkIdType>::iterator itemIt=allItemIDs.begin(); itemIt!=allItemIDs.end(); ++itemIt)
    {
    vtkIdType itemID = (*itemIt);
    // Expanded states are handled with the name column
    QStandardItem* item = this->itemFromSubjectHierarchyItem(itemID, this->nameColumn());
    this->updateItemDataFromSubjectHierarchyItem(item, itemID, this->nameColumn());
    }

  emit subjectHierarchyUpdated();
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::updateFromSubjectHierarchy()
{
  Q_D(qDMMLSubjectHierarchyModel);

  if (!d->SubjectHierarchyNode)
    {
    // Remove all items
    const int oldColumnCount = this->columnCount();
    this->removeRows(0, this->rowCount());
    this->setColumnCount(oldColumnCount);
    return;
    }
  else if (!this->subjectHierarchySceneItem())
    {
    this->rebuildFromSubjectHierarchy();
    return;
    }
  else
    {
    // Update the scene item index in case subject hierarchy node has changed
    this->subjectHierarchySceneItem()->setData(
      QVariant::fromValue(d->SubjectHierarchyNode->GetSceneItemID()), qDMMLSubjectHierarchyModel::SubjectHierarchyItemIDRole );
    d->RowCache[d->SubjectHierarchyNode->GetSceneItemID()] = this->subjectHierarchySceneItem()->index();
    }

  // Get all subject hierarchy items
  std::vector<vtkIdType> allItemIDs;
  d->SubjectHierarchyNode->GetItemChildren(d->SubjectHierarchyNode->GetSceneItemID(), allItemIDs, true);

  // Update all items
  for (std::vector<vtkIdType>::iterator itemIt=allItemIDs.begin(); itemIt!=allItemIDs.end(); ++itemIt)
    {
    vtkIdType itemID = (*itemIt);
    for (int col=0; col<this->columnCount(); ++col)
      {
      QStandardItem* item = this->itemFromSubjectHierarchyItem(itemID, col);
      this->updateItemFromSubjectHierarchyItem(item, itemID, col);
      }
    }

  // Update expanded states (during inserting the update calls did not find valid indices, so
  // expand and collapse statuses were not set in the tree view)
  for (std::vector<vtkIdType>::iterator itemIt=allItemIDs.begin(); itemIt!=allItemIDs.end(); ++itemIt)
    {
    vtkIdType itemID = (*itemIt);
    // Expanded states are handled with the name column
    QStandardItem* item = this->itemFromSubjectHierarchyItem(itemID, this->nameColumn());
    this->updateItemDataFromSubjectHierarchyItem(item, itemID, this->nameColumn());
    }

  emit subjectHierarchyUpdated();
}

//------------------------------------------------------------------------------
QStandardItem* qDMMLSubjectHierarchyModel::insertSubjectHierarchyItem(vtkIdType itemID)
{
  Q_D(qDMMLSubjectHierarchyModel);
  return d->insertSubjectHierarchyItem(itemID, this->subjectHierarchyItemIndex(itemID));
}

//------------------------------------------------------------------------------
QStandardItem* qDMMLSubjectHierarchyModel::insertSubjectHierarchyItem(vtkIdType itemID, QStandardItem* parent, int row/*=-1*/ )
{
  Q_D(qDMMLSubjectHierarchyModel);

  if (!parent)
    {
    // The scene is inserted individually, and the other items must always have a valid parent (if not other then the scene)
    qCritical() << Q_FUNC_INFO << ": Invalid parent to inserted subject hierarchy item with ID " << itemID;
    return nullptr;
    }

  QList<QStandardItem*> items;
  for (int col=0; col<this->columnCount(); ++col)
    {
    QStandardItem* newItem = new QStandardItem();
    this->updateItemFromSubjectHierarchyItem(newItem, itemID, col);
    items.append(newItem);
    }

  // Insert an invalid item in the cache to indicate that the subject hierarchy item is in the
  // model but we don't know its index yet. This is needed because a custom widget may be notified
  // about row insertion before insertRow() returns (and the RowCache entry is added).
  d->RowCache[itemID] = QModelIndex();
  parent->insertRow(row, items);
  d->RowCache[itemID] = items[0]->index();

  return items[0];
}

//------------------------------------------------------------------------------
QFlags<Qt::ItemFlag> qDMMLSubjectHierarchyModel::subjectHierarchyItemFlags(vtkIdType itemID, int column)const
{
  Q_D(const qDMMLSubjectHierarchyModel);

  QFlags<Qt::ItemFlag> flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  if (!d->SubjectHierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return flags;
    }

  if (column == this->nameColumn() || column == this->colorColumn()
    || column == this->descriptionColumn())
    {
    flags |= Qt::ItemIsEditable;
    }

  if (this->canBeAChild(itemID))
    {
    flags |= Qt::ItemIsDragEnabled;
    }
  if (this->canBeAParent(itemID))
    {
    flags |= Qt::ItemIsDropEnabled;
    }

  // Drop is also enabled for virtual branches.
  // (a virtual branch is a branch where the children items do not correspond to actual DMML data nodes,
  // but to implicit items contained by the parent DMML node, e.g. in case of Markups or Segmentations)
  if ( d->SubjectHierarchyNode->HasItemAttribute( itemID,
    vtkDMMLSubjectHierarchyConstants::GetSubjectHierarchyVirtualBranchAttributeName()) )
    {
    flags |= Qt::ItemIsDropEnabled;
    }
  // Along the same logic, drop is not enabled to children nodes in virtual branches
  vtkIdType parentItemID = d->SubjectHierarchyNode->GetItemParent(itemID);
  if (parentItemID
    && d->SubjectHierarchyNode->HasItemAttribute(
         parentItemID, vtkDMMLSubjectHierarchyConstants::GetSubjectHierarchyVirtualBranchAttributeName()) )
    {
    flags &= ~Qt::ItemIsDropEnabled;
    }

  return flags;
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::updateItemFromSubjectHierarchyItem(QStandardItem* item, vtkIdType shItemID, int column)
{
  Q_D(qDMMLSubjectHierarchyModel);

  if (item->data(Qt::WhatsThisRole).toString() == d->extraItemIdentifier())
    {
    return;
    }

  // We are going to make potentially multiple changes to the item. We want to refresh
  // the subject hierarchy item only once, so we "block" the updates in onItemChanged().
  d->PendingItemModified = 0;
  item->setFlags(this->subjectHierarchyItemFlags(shItemID, column));

  // Set ID
  bool blocked = this->blockSignals(true);
  item->setData(shItemID, qDMMLSubjectHierarchyModel::SubjectHierarchyItemIDRole);
  this->blockSignals(blocked);

  // Update item data for the current column
  this->updateItemDataFromSubjectHierarchyItem(item, shItemID, column);

  bool itemChanged = (d->PendingItemModified > 0);
  d->PendingItemModified = -1;

  if (this->canBeAChild(shItemID))
    {
    QStandardItem* parentItem = item->parent();
    QStandardItem* newParentItem = this->itemFromSubjectHierarchyItem(this->parentSubjectHierarchyItem(shItemID));
    if (!newParentItem)
      {
      newParentItem = this->subjectHierarchySceneItem();
      }
    // If the item has no parent, then it means it hasn't been put into the hierarchy yet and it will do it automatically
    if (parentItem && parentItem != newParentItem)
      {
      int newIndex = this->subjectHierarchyItemIndex(shItemID);
      if (parentItem != newParentItem || newIndex != item->row())
        {
        // Reparent items
        QList<QStandardItem*> children = parentItem->takeRow(item->row());
        newParentItem->insertRow(newIndex, children);
        }
      }
    }
  if (itemChanged)
    {
    this->onItemChanged(item);
    }
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::updateItemDataFromSubjectHierarchyItem(QStandardItem* item, vtkIdType shItemID, int column)
{
  Q_D(qDMMLSubjectHierarchyModel);
  if (!d->SubjectHierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
    }
  if (!item)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid item";
    return;
    }
  if (shItemID == d->SubjectHierarchyNode->GetSceneItemID())
    {
    return;
    }

  qCjyxSubjectHierarchyAbstractPlugin* ownerPlugin = nullptr;
  if (!d->SubjectHierarchyNode->GetItemOwnerPluginName(shItemID).empty())
    {
    ownerPlugin = qCjyxSubjectHierarchyPluginHandler::instance()->getOwnerPluginForSubjectHierarchyItem(shItemID);
    if (!ownerPlugin)
      {
      if (column == this->nameColumn())
        {
        item->setText(d->subjectHierarchyItemName(shItemID));
        item->setToolTip(tr("No subject hierarchy role assigned! Please report error"));
        if (item->icon().cacheKey() != d->WarningIcon.cacheKey()) // Only set if it changed (https://bugreports.qt-project.org/browse/QTBUG-20248)
          {
          item->setIcon(d->WarningIcon);
          }
        }
        return;
      }
    }
  else
    {
    qDebug() << Q_FUNC_INFO << ": No owner plugin for subject hierarchy item '" << d->subjectHierarchyItemName(shItemID);

    // Owner plugin name is not set for subject hierarchy item. Show it as a regular node
    if (column == this->nameColumn())
      {
      item->setText(QString(d->SubjectHierarchyNode->GetItemName(shItemID).c_str()));
      if (item->icon().cacheKey() != d->UnknownIcon.cacheKey()) // Only set if it changed (https://bugreports.qt-project.org/browse/QTBUG-20248)
        {
        item->setIcon(d->UnknownIcon);
        }
      }
    if (column == this->idColumn())
      {
      vtkDMMLNode* dataNode = d->SubjectHierarchyNode->GetItemDataNode(shItemID);
      if (dataNode)
        {
        item->setText(QString(dataNode->GetID()));
        }
      }
    return;
    }

  // Owner plugin exists, show information normally

  // Name column
  if (column == this->nameColumn())
    {
    item->setText(ownerPlugin->displayedItemName(shItemID));
    item->setToolTip(ownerPlugin->tooltip(shItemID));

    // Have owner plugin set the icon
    QIcon icon = ownerPlugin->icon(shItemID);
    if (!icon.isNull())
      {
      if (item->icon().cacheKey() != icon.cacheKey()) // Only set if it changed (https://bugreports.qt-project.org/browse/QTBUG-20248)
        {
        item->setIcon(icon);
        }
      }
    else if (item->icon().cacheKey() != d->UnknownIcon.cacheKey()) // Only set if it changed (https://bugreports.qt-project.org/browse/QTBUG-20248)
      {
      item->setIcon(d->UnknownIcon);
      }

    // Set expanded state (in the name column so that it is only processed once for each item)
    if (d->SubjectHierarchyNode->GetItemExpanded(shItemID))
      {
      emit requestExpandItem(shItemID);
      }
    else
      {
      emit requestCollapseItem(shItemID);
      }
    }
  // Description column
  if (column == this->descriptionColumn())
    {
    vtkDMMLNode* dataNode = d->SubjectHierarchyNode->GetItemDataNode(shItemID);
    if (dataNode)
      {
      item->setText(QString(dataNode->GetDescription()));
      }
    }
  // ID column
  if (column == this->idColumn())
    {
    vtkDMMLNode* dataNode = d->SubjectHierarchyNode->GetItemDataNode(shItemID);
    if (dataNode)
      {
      item->setText(QString(dataNode->GetID()));
      }
    }
  // Visibility column
  if (column == this->visibilityColumn())
    {
    // Have owner plugin give the visibility state and icon
    int visible = ownerPlugin->getDisplayVisibility(shItemID);
    QIcon visibilityIcon = ownerPlugin->visibilityIcon(visible);

    // It should be fine to set the icon even if it is the same, but due
    // to a bug in Qt (http://bugreports.qt.nokia.com/browse/QTBUG-20248),
    // it would fire a superfluous itemChanged() signal.
    if ( item->data(VisibilityRole).isNull()
      || item->data(VisibilityRole).toInt() != visible )
      {
      item->setData(visible, VisibilityRole);
      if (!visibilityIcon.isNull())
        {
        item->setIcon(visibilityIcon);
        }
      }
    }
  // Color column
  if (column == this->colorColumn())
    {
    // Get color and terminology metadata from owner plugin
    QMap<int, QVariant> terminologyMetaData;
    QColor color = ownerPlugin->getDisplayColor(shItemID, terminologyMetaData);

    if (terminologyMetaData.contains(qCjyxTerminologyItemDelegate::TerminologyRole))
      {
      QString terminologyString = terminologyMetaData[qCjyxTerminologyItemDelegate::TerminologyRole].toString();
      item->setData(terminologyString, qCjyxTerminologyItemDelegate::TerminologyRole);
      }
    if (terminologyMetaData.contains(qCjyxTerminologyItemDelegate::NameRole))
      {
      QString nameFromColorItem = terminologyMetaData[qCjyxTerminologyItemDelegate::NameRole].toString();
      item->setData(nameFromColorItem, qCjyxTerminologyItemDelegate::NameRole);
      }
    if (terminologyMetaData.contains(qCjyxTerminologyItemDelegate::NameAutoGeneratedRole))
      {
      bool nameAutoGenerated = terminologyMetaData[qCjyxTerminologyItemDelegate::NameAutoGeneratedRole].toBool();
      item->setData(nameAutoGenerated, qCjyxTerminologyItemDelegate::NameAutoGeneratedRole);
      }
    if (terminologyMetaData.contains(qCjyxTerminologyItemDelegate::ColorAutoGeneratedRole))
      {
      bool colorAutoGenerated = terminologyMetaData[qCjyxTerminologyItemDelegate::ColorAutoGeneratedRole].toBool();
      item->setData(colorAutoGenerated, qCjyxTerminologyItemDelegate::ColorAutoGeneratedRole);
      }

    // Set item color
    item->setData(color, Qt::DecorationRole);

    // Assemble and set tooltip
    vtkCjyxTerminologiesModuleLogic* terminologiesLogic = d->terminologiesModuleLogic();
    if (!terminologiesLogic)
      {
      qCritical() << Q_FUNC_INFO << ": Terminologies module is not found";
      }
    vtkSmartPointer<vtkCjyxTerminologyEntry> terminologyEntry = vtkSmartPointer<vtkCjyxTerminologyEntry>::New();
    terminologiesLogic->DeserializeTerminologyEntry(
      item->data(qCjyxTerminologyItemDelegate::TerminologyRole).toString().toUtf8().constData(), terminologyEntry);
    item->setToolTip(terminologiesLogic->GetInfoStringFromTerminologyEntry(terminologyEntry).c_str());
    }
  // Transform column
  if (column == this->transformColumn())
    {
    if (item->data(Qt::WhatsThisRole).toString().isEmpty())
      {
      item->setData("Transform", Qt::WhatsThisRole);
      }

    QIcon icon;
    vtkDMMLNode* dataNode = d->SubjectHierarchyNode->GetItemDataNode(shItemID);
    vtkDMMLTransformableNode* transformableNode = vtkDMMLTransformableNode::SafeDownCast(dataNode);
    if (transformableNode)
      {
      icon = d->NoTransformIcon;
      vtkDMMLTransformNode* parentTransformNode = ( transformableNode->GetParentTransformNode() ? transformableNode->GetParentTransformNode() : nullptr );
      QString transformNodeId( parentTransformNode ? parentTransformNode->GetID() : "" );
      QString transformNodeName( parentTransformNode ? parentTransformNode->GetName() : "" );
      // Only change item if the transform itself changed
      if (item->data().toString().compare(transformNodeId))
        {
        item->setData(transformNodeId, TransformIDRole);
        item->setToolTip( parentTransformNode ? tr("%1 (%2)").arg(parentTransformNode->GetName()).arg(parentTransformNode->GetID()) : "" );
        if (parentTransformNode)
          {
          icon = (parentTransformNode->IsLinear() ? d->LinearTransformIcon : d->DeformableTransformIcon);
          }
        }
      }
    else if (d->SubjectHierarchyNode->GetNumberOfItemChildren(shItemID, true))
      {
      icon = d->FolderTransformIcon;
      item->setToolTip(tr("Apply transform to children"));
      }
    else
      {
      item->setToolTip(tr("This node is not transformable"));
      }
    if (item->icon().cacheKey() != icon.cacheKey()) // Only set if it changed (https://bugreports.qt-project.org/browse/QTBUG-20248)
      {
      item->setIcon(icon);
      }
    }
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::updateSubjectHierarchyItemFromItem(vtkIdType shItemID, QStandardItem* item)
{
  if (!item)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid item";
    return;
    }

  //int wasModifying = node->StartModify(); //TODO: Add feature to item if there are performance issues
  this->updateSubjectHierarchyItemFromItemData(shItemID, item);
  //node->EndModify(wasModifying);

  // the following only applies to tree hierarchies
  if (!this->canBeAChild(shItemID))
    {
    return;
    }

 Q_ASSERT(shItemID != this->subjectHierarchyItemFromItem(item->parent()));

  QStandardItem* parentItem = item->parent();
  int columnCount = parentItem ? parentItem->columnCount() : 0;
  // Don't do the following if the row is not complete (reparenting an incomplete row might lead to errors;
  // if there is no child yet for a given column, it will get there next time updateNodeFromItem is called).
  // updateNodeFromItem is called for every item drag&dropped (we ensure that all the indexes of the row are
  // reparented when entering the d&d function)
  for (int col=0; col<columnCount; ++col)
    {
    if (parentItem->child(item->row(), col) == nullptr)
      {
      return;
      }
    }

  vtkIdType parentItemID = this->subjectHierarchyItemFromItem(parentItem);
  if (this->parentSubjectHierarchyItem(shItemID) != parentItemID)
    {
    // Parent changed, need to reparent the subject hierarchy item in the node
    emit aboutToReparentByDragAndDrop(shItemID, parentItemID);
    if (this->reparent(shItemID, parentItemID))
      {
      emit reparentedByDragAndDrop(shItemID, parentItemID);
      emit requestExpandItem(parentItemID);
      }
    else
      {
      this->updateItemFromSubjectHierarchyItem(item, shItemID, item->column());
      }
    }
  else if (this->subjectHierarchyItemIndex(shItemID) != item->row())
    {
    // Moved within parent, need to re-order subject hierarchy item in the node
    int oldRow = this->subjectHierarchyItemIndex(shItemID);
    int newRow = item->row();
    // When moving down, the item before which this item needs to be inserted was one row down
    if (!this->moveToRow(shItemID, (newRow>oldRow ? newRow+1 : newRow) ))
      {
      this->updateItemFromSubjectHierarchyItem(item, shItemID, item->column());
      }
    }
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::updateSubjectHierarchyItemFromItemData(vtkIdType shItemID, QStandardItem* item)
{
  Q_D(qDMMLSubjectHierarchyModel);
  if (!d->SubjectHierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
    }
  if (!item)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid item";
    return;
    }

  qCjyxSubjectHierarchyAbstractPlugin* ownerPlugin =
    qCjyxSubjectHierarchyPluginHandler::instance()->getOwnerPluginForSubjectHierarchyItem(shItemID);

  // Name column
  if (item->column() == this->nameColumn())
    {
    // This call renames associated data node if any
    d->SubjectHierarchyNode->SetItemName(shItemID, item->text().toUtf8().constData());
    }
  // Description column
  if (item->column() == this->descriptionColumn())
    {
    std::string newDescriptionStr = item->text().toUtf8().constData();
    vtkDMMLNode* dataNode = vtkDMMLNode::SafeDownCast(d->SubjectHierarchyNode->GetItemDataNode(shItemID));
    if (!dataNode)
      {
      return;
      }
    if (dataNode->GetDescription() && newDescriptionStr.compare(dataNode->GetDescription()))
      {
      dataNode->SetDescription(newDescriptionStr.c_str());
      }
    }
  // Visibility column
  if (item->column() == this->visibilityColumn() && !item->data(VisibilityRole).isNull())
    {
    int visible = item->data(VisibilityRole).toInt();
    if (visible > -1 && visible != ownerPlugin->getDisplayVisibility(shItemID))
      {
      // Have owner plugin set the display visibility
      ownerPlugin->setDisplayVisibility(shItemID, visible);
      }
    }
  // Color column
  if (item->column() == this->colorColumn())
    {
    QColor color = item->data(Qt::DecorationRole).value<QColor>();
    QString terminologyString = item->data(qCjyxTerminologyItemDelegate::TerminologyRole).toString();
    // Invalid color can happen when the item hasn't been initialized yet
    if (color.isValid())
      {
      // Get color and terminology metadata from owner plugin
      QMap<int, QVariant> terminologyMetaData;
      QColor oldColor = ownerPlugin->getDisplayColor(shItemID, terminologyMetaData);
      QString oldTerminologyString = terminologyMetaData[qCjyxTerminologyItemDelegate::TerminologyRole].toString();
      if (oldColor != color || oldTerminologyString != terminologyString)
        {
        // Get terminology metadata
        QString nameFromColorItem = item->data(qCjyxTerminologyItemDelegate::NameRole).toString();
        bool nameAutoGenerated = item->data(qCjyxTerminologyItemDelegate::NameAutoGeneratedRole).toBool();
        bool colorAutoGenerated = item->data(qCjyxTerminologyItemDelegate::ColorAutoGeneratedRole).toBool();
        terminologyMetaData[qCjyxTerminologyItemDelegate::TerminologyRole] = terminologyString;
        terminologyMetaData[qCjyxTerminologyItemDelegate::NameRole] = nameFromColorItem;
        terminologyMetaData[qCjyxTerminologyItemDelegate::NameAutoGeneratedRole] = nameAutoGenerated;
        terminologyMetaData[qCjyxTerminologyItemDelegate::ColorAutoGeneratedRole] = colorAutoGenerated;

        // Have owner plugin set the color
        ownerPlugin->setDisplayColor(shItemID, color, terminologyMetaData);
        }
      }
    }
  // Transform column
  if (item->column() == this->transformColumn())
    {
    QVariant transformIdData = item->data(TransformIDRole);
    std::string newParentTransformNodeIdStr = transformIdData.toString().toUtf8().constData();
    vtkDMMLTransformNode* newParentTransformNode =
      vtkDMMLTransformNode::SafeDownCast( d->DMMLScene->GetNodeByID(newParentTransformNodeIdStr) );

    // No action if the chosen transform is the same as the applied one
    vtkDMMLTransformableNode* dataNode = vtkDMMLTransformableNode::SafeDownCast(
      d->SubjectHierarchyNode->GetItemDataNode(shItemID) );
    vtkDMMLTransformNode* currentTransformNode = (dataNode ? dataNode->GetParentTransformNode() : nullptr);
    if (currentTransformNode == newParentTransformNode)
      {
      return;
      }

    // No checks and questions when the transform is being removed
    if (!newParentTransformNode)
      {
      vtkCjyxSubjectHierarchyModuleLogic::TransformBranch(d->SubjectHierarchyNode, shItemID, nullptr, false);
      return;
      }

    // Ask the user if any child node in the branch is transformed with a transform different from the chosen one
    bool hardenExistingTransforms = false;
    if (d->SubjectHierarchyNode->IsAnyNodeInBranchTransformed(shItemID, false))
      {
      QMessageBox::StandardButton answer =
        QMessageBox::question(nullptr, tr("Some nodes in the branch are already transformed"),
        tr("Do you want to harden all already applied transforms before setting the new one?\n\n"
        "  Note: If you choose no, then the applied transform will simply be replaced."),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
        QMessageBox::Yes);
      if (answer == QMessageBox::Yes)
        {
        hardenExistingTransforms = true;
        }
      else if (answer == QMessageBox::Cancel)
        {
        return;
        }
      }

    vtkCjyxSubjectHierarchyModuleLogic::TransformBranch(
      d->SubjectHierarchyNode, shItemID, newParentTransformNode, hardenExistingTransforms );
    }
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::updateModelItems(vtkIdType itemID)
{
  Q_D(qDMMLSubjectHierarchyModel);
  if (d->DMMLScene->IsClosing() || d->DMMLScene->IsBatchProcessing())
    {
    return;
    }

  QModelIndexList itemIndexes = this->indexes(itemID);
  if (!itemIndexes.count())
    {
    // Can happen while the item is added, the plugin handler sets the owner plugin, which triggers
    // item modified before it can be inserted to the model
    return;
    }

  for (int currentIndex=0; currentIndex<itemIndexes.size(); ++currentIndex)
    {
    // Note: If this loop is changed to foreach update after reparenting stops working.
    //   Apparently foreach makes a deep copy of itemIndexes, and as the indices change after the
    //   first reparenting in the updateItemFromSubjectHierarchyItem for column 0, the old indices
    //   are being used for the subsequent columns, which yield new items, and reparenting is
    //   performed on those too. With regular for and [] operator, the updated indices are got,
    //   so reparenting is only performed once, which is the desired behavior.
    QModelIndex index = itemIndexes[currentIndex];
    QStandardItem* item = this->itemFromIndex(index);
    int oldRow = item->row();
    QStandardItem* oldParent = item->parent();

    this->updateItemFromSubjectHierarchyItem(item, itemID, item->column());

    // If the item was reparented, then we need to rescan the indexes again as they may be wrong
    if (item->row() != oldRow || item->parent() != oldParent)
      {
      int oldSize = itemIndexes.size();
      itemIndexes = this->indexes(itemID);
      int newSize = itemIndexes.size();
      if (oldSize != newSize)
        {
        qCritical() << Q_FUNC_INFO << ": Index mismatch";
        return;
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::onEvent(
  vtkObject* caller, unsigned long event, void* clientData, void* callData )
{
  vtkDMMLSubjectHierarchyNode* shNode = reinterpret_cast<vtkDMMLSubjectHierarchyNode*>(caller);
  vtkDMMLScene* scene = reinterpret_cast<vtkDMMLScene*>(caller);
  qDMMLSubjectHierarchyModel* sceneModel = reinterpret_cast<qDMMLSubjectHierarchyModel*>(clientData);
  if (!sceneModel || (!shNode && !scene))
    {
    qCritical() << Q_FUNC_INFO << ": Invalid event parameters";
    return;
    }

  // Get item ID for subject hierarchy node events
  vtkIdType itemID = vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID;
  if (callData)
    {
    vtkIdType* itemIdPtr = reinterpret_cast<vtkIdType*>(callData);
    if (itemIdPtr)
      {
      itemID = *itemIdPtr;
      }
    }

  // Get node for scene events
  vtkDMMLNode* node = reinterpret_cast<vtkDMMLNode*>(callData);

  switch (event)
    {
    case vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemAddedEvent:
      sceneModel->onSubjectHierarchyItemAdded(itemID);
      break;
    case vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemAboutToBeRemovedEvent:
      sceneModel->onSubjectHierarchyItemAboutToBeRemoved(itemID);
      break;
    case vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemRemovedEvent:
      sceneModel->onSubjectHierarchyItemRemoved(itemID);
      break;
    case vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemModifiedEvent:
    case vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemTransformModifiedEvent:
    case vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemDisplayModifiedEvent:
    case vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemReparentedEvent:
      sceneModel->onSubjectHierarchyItemModified(itemID);
      break;
    case vtkDMMLScene::EndImportEvent:
      sceneModel->onDMMLSceneImported(scene);
      break;
    case vtkDMMLScene::EndCloseEvent:
      sceneModel->onDMMLSceneClosed(scene);
      break;
    case vtkDMMLScene::StartBatchProcessEvent:
      sceneModel->onDMMLSceneStartBatchProcess(scene);
      break;
    case vtkDMMLScene::EndBatchProcessEvent:
      sceneModel->onDMMLSceneEndBatchProcess(scene);
      break;
    case vtkDMMLScene::NodeRemovedEvent:
      sceneModel->onDMMLNodeRemoved(node);
      break;
    }
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::onSubjectHierarchyItemAdded(vtkIdType itemID)
{
  this->insertSubjectHierarchyItem(itemID);
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::onSubjectHierarchyItemAboutToBeRemoved(vtkIdType itemID)
{
  Q_D(qDMMLSubjectHierarchyModel);
  if (d->DMMLScene->IsClosing() || d->DMMLScene->IsBatchProcessing())
    {
    return;
    }

  QModelIndexList itemIndexes = this->match(
    this->subjectHierarchySceneIndex(), SubjectHierarchyItemIDRole, itemID, 1, Qt::MatchExactly | Qt::MatchRecursive );
  if (itemIndexes.count() > 0)
    {
    QStandardItem* item = this->itemFromIndex(itemIndexes[0].sibling(itemIndexes[0].row(),0));
    // The children may be lost if not reparented, we ensure they got reparented.
    while (item->rowCount())
      {
      // Need to remove the children from the removed item because they would be automatically deleted in QStandardItemModel::removeRow()
      d->Orphans.push_back(item->takeRow(0));
      }
    // Remove the item from any orphan list if it exist as we don't want to add it back later in onSubjectHierarchyItemRemoved
    foreach (QList<QStandardItem*> orphans, d->Orphans)
      {
      if (orphans.contains(item))
        {
        d->Orphans.removeAll(orphans);
        }
      }
    this->removeRow(itemIndexes[0].row(), itemIndexes[0].parent());
    }
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::onSubjectHierarchyItemRemoved(vtkIdType removedItemID)
{
  Q_D(qDMMLSubjectHierarchyModel);
  Q_UNUSED(removedItemID);
  if (d->DMMLScene->IsClosing() || d->DMMLScene->IsBatchProcessing())
    {
    return;
    }
  // The removed item may have had children, if they haven't been updated, they are likely to be lost
  // (not reachable when browsing the model), we need to reparent them.
  foreach(QList<QStandardItem*> orphans, d->Orphans)
    {
    QStandardItem* orphan = orphans[0];
    // Make sure that the orphans have not already been reparented.
    if (orphan->parent())
      {
      continue;
      }
    vtkIdType itemID = this->subjectHierarchyItemFromItem(orphan);
    int newIndex = this->subjectHierarchyItemIndex(itemID);
    QStandardItem* newParentItem = this->itemFromSubjectHierarchyItem(
      this->parentSubjectHierarchyItem(itemID) );
    if (!newParentItem)
      {
      newParentItem = this->subjectHierarchySceneItem();
      }
    // Reparent orphans
    newParentItem->insertRow(newIndex, orphans);
    }
  d->Orphans.clear();
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::onSubjectHierarchyItemModified(vtkIdType itemID)
{
  this->updateModelItems(itemID);
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::onDMMLSceneImported(vtkDMMLScene* scene)
{
  Q_UNUSED(scene);
  this->rebuildFromSubjectHierarchy();
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::onDMMLSceneClosed(vtkDMMLScene* scene)
{
  // Make sure there is one subject hierarchy node in the scene, and it is used by the model
  vtkDMMLSubjectHierarchyNode* newSubjectHierarchyNode = vtkDMMLSubjectHierarchyNode::ResolveSubjectHierarchy(scene);
  if (!newSubjectHierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": No subject hierarchy node could be retrieved from the scene";
    }

  this->setSubjectHierarchyNode(newSubjectHierarchyNode);
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::onDMMLSceneStartBatchProcess(vtkDMMLScene* scene)
{
  Q_UNUSED(scene);
  emit subjectHierarchyAboutToBeUpdated();
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::onDMMLSceneEndBatchProcess(vtkDMMLScene* scene)
{
  Q_UNUSED(scene);
  this->updateFromSubjectHierarchy();
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::onDMMLNodeRemoved(vtkDMMLNode* node)
{
  Q_D(qDMMLSubjectHierarchyModel);
  if (d->DMMLScene->IsClosing())
    {
    return;
    }

  if (node->IsA("vtkDMMLSubjectHierarchyNode"))
    {
    // Make sure there is one subject hierarchy node in the scene, and it is used by the model
    vtkDMMLSubjectHierarchyNode* newSubjectHierarchyNode = vtkDMMLSubjectHierarchyNode::ResolveSubjectHierarchy(d->DMMLScene);
    if (!newSubjectHierarchyNode)
      {
      qCritical() << Q_FUNC_INFO << ": No subject hierarchy node could be retrieved from the scene";
      }

    this->setSubjectHierarchyNode(newSubjectHierarchyNode);
    }
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::onItemChanged(QStandardItem* item)
{
  Q_D(qDMMLSubjectHierarchyModel);
  if (d->PendingItemModified >= 0)
    {
    ++d->PendingItemModified;
    return;
    }
  // When a drag&drop occurs, the order of the items called with onItemChanged is
  // random, it could be the item in column 1 then the item in column 0
  if (!d->DraggedSubjectHierarchyItems.empty())
    {
    // Item changed will be triggered multiple times in course of the drag&drop event. Setting this flag
    // makes sure the final onItemChanged with the collected DraggedSubjectHierarchyItems is called only once.
    if (!d->DelayedItemChangedInvoked)
      {
      d->DelayedItemChangedInvoked = true;
      QTimer::singleShot(50, this, SLOT(delayedItemChanged()));
      }
    return;
    }

  this->updateSubjectHierarchyItemFromItem(this->subjectHierarchyItemFromItem(item), item);
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::delayedItemChanged()
{
  Q_D(qDMMLSubjectHierarchyModel);

  // Update each dropped item
  foreach(vtkIdType draggedShItemID, d->DraggedSubjectHierarchyItems)
    {
    this->updateSubjectHierarchyItemFromItem(
      draggedShItemID, this->itemFromSubjectHierarchyItem(draggedShItemID) );
    }

  // Re-select dropped items.
  // Only needed if drag-and-dropping inside the widget (reparenting removes selection).
  if (d->IsDroppedInside)
    {
    emit requestSelectItems(d->DraggedSubjectHierarchyItems);
    }
  // Reset state
  d->DraggedSubjectHierarchyItems.clear();
  d->DelayedItemChangedInvoked = false;
  d->IsDroppedInside = false;
}

//------------------------------------------------------------------------------
Qt::DropActions qDMMLSubjectHierarchyModel::supportedDropActions()const
{
  return Qt::MoveAction;
}

//------------------------------------------------------------------------------
int qDMMLSubjectHierarchyModel::nameColumn()const
{
  Q_D(const qDMMLSubjectHierarchyModel);
  return d->NameColumn;
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::setNameColumn(int column)
{
  Q_D(qDMMLSubjectHierarchyModel);
  d->NameColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qDMMLSubjectHierarchyModel::idColumn()const
{
  Q_D(const qDMMLSubjectHierarchyModel);
  return d->IDColumn;
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::setIDColumn(int column)
{
  Q_D(qDMMLSubjectHierarchyModel);
  d->IDColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qDMMLSubjectHierarchyModel::visibilityColumn()const
{
  Q_D(const qDMMLSubjectHierarchyModel);
  return d->VisibilityColumn;
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::setVisibilityColumn(int column)
{
  Q_D(qDMMLSubjectHierarchyModel);
  d->VisibilityColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qDMMLSubjectHierarchyModel::colorColumn()const
{
  Q_D(const qDMMLSubjectHierarchyModel);
  return d->ColorColumn;
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::setColorColumn(int column)
{
  Q_D(qDMMLSubjectHierarchyModel);
  d->ColorColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qDMMLSubjectHierarchyModel::transformColumn()const
{
  Q_D(const qDMMLSubjectHierarchyModel);
  return d->TransformColumn;
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::setTransformColumn(int column)
{
  Q_D(qDMMLSubjectHierarchyModel);
  d->TransformColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qDMMLSubjectHierarchyModel::descriptionColumn()const
{
  Q_D(const qDMMLSubjectHierarchyModel);
  return d->DescriptionColumn;
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::setDescriptionColumn(int column)
{
  Q_D(qDMMLSubjectHierarchyModel);
  d->DescriptionColumn = column;
  this->updateColumnCount();
}

//--------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::setNoneEnabled(bool enable)
{
  Q_D(qDMMLSubjectHierarchyModel);
  if (d->NoneEnabled == enable)
    {
    return;
    }
  d->NoneEnabled = enable;
  this->rebuildFromSubjectHierarchy();
}

//--------------------------------------------------------------------------
bool qDMMLSubjectHierarchyModel::noneEnabled()const
{
  Q_D(const qDMMLSubjectHierarchyModel);
  return d->NoneEnabled;
}

//--------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::setNoneDisplay(const QString& displayName)
{
  Q_D(qDMMLSubjectHierarchyModel);
  if (d->NoneDisplay == displayName)
    {
    return;
    }
  d->NoneDisplay = displayName;

  if (d->NoneEnabled)
    {
    this->subjectHierarchySceneItem()->child(0, this->nameColumn())->setText(d->NoneDisplay);
    }
}

//--------------------------------------------------------------------------
QString qDMMLSubjectHierarchyModel::noneDisplay()const
{
  Q_D(const qDMMLSubjectHierarchyModel);
  return d->NoneDisplay;
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::updateColumnCount()
{
  Q_D(const qDMMLSubjectHierarchyModel);

  int max = this->maxColumnId();
  int oldColumnCount = this->columnCount();
  this->setColumnCount(max + 1);
  if (oldColumnCount == 0)
    {
    this->rebuildFromSubjectHierarchy();
    }
  else
    {
    // Update all items
    if (!d->SubjectHierarchyNode)
      {
      return;
      }
    std::vector<vtkIdType> allItemIDs;
    d->SubjectHierarchyNode->GetItemChildren(d->SubjectHierarchyNode->GetSceneItemID(), allItemIDs, true);
    for (std::vector<vtkIdType>::iterator itemIt=allItemIDs.begin(); itemIt!=allItemIDs.end(); ++itemIt)
      {
      this->updateModelItems(*itemIt);
      }
    }
}

//------------------------------------------------------------------------------
int qDMMLSubjectHierarchyModel::maxColumnId()const
{
  Q_D(const qDMMLSubjectHierarchyModel);
  int maxId = -1;
  maxId = qMax(maxId, d->NameColumn);
  maxId = qMax(maxId, d->DescriptionColumn);
  maxId = qMax(maxId, d->IDColumn);
  maxId = qMax(maxId, d->VisibilityColumn);
  maxId = qMax(maxId, d->ColorColumn);
  maxId = qMax(maxId, d->TransformColumn);
  return maxId;
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::onHardenTransformOnBranchOfCurrentItem()
{
  Q_D(const qDMMLSubjectHierarchyModel);
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

  vtkIdType currentItemID = qCjyxSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID)
    {
    vtkCjyxSubjectHierarchyModuleLogic::HardenTransformOnBranch(d->SubjectHierarchyNode, currentItemID);
    }

  QApplication::restoreOverrideCursor();
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyModel::onRemoveTransformsFromBranchOfCurrentItem()
{
  Q_D(const qDMMLSubjectHierarchyModel);
  vtkIdType currentItemID = qCjyxSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID)
    {
    vtkCjyxSubjectHierarchyModuleLogic::TransformBranch(d->SubjectHierarchyNode, currentItemID, nullptr, false);
    }
}

//------------------------------------------------------------------------------
void printStandardItem(QStandardItem* item, const QString& offset)
{
  if (!item)
    {
    return;
    }
  qDebug() << offset << item << item->index() << item->text()
           << item->data(qDMMLSubjectHierarchyModel::SubjectHierarchyItemIDRole).toString() << item->row()
           << item->column() << item->rowCount() << item->columnCount();
  for(int i = 0; i < item->rowCount(); ++i )
    {
    for (int j = 0; j < item->columnCount(); ++j)
      {
      printStandardItem(item->child(i,j), offset + "   ");
      }
    }
}
