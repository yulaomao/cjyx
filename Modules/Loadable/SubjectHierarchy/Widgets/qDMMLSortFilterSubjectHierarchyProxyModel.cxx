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

#include "qDMMLSortFilterSubjectHierarchyProxyModel.h"

// DMML include
#include "vtkDMMLSubjectHierarchyNode.h"

// Subject Hierarchy includes
#include "qCjyxSubjectHierarchyPluginHandler.h"
#include "qCjyxSubjectHierarchyAbstractPlugin.h"
#include "qDMMLSubjectHierarchyModel.h"

// Qt includes
#include <QDebug>
#include <QStandardItem>

// STD includes
#include <algorithm>

// -----------------------------------------------------------------------------
// qDMMLSortFilterSubjectHierarchyProxyModelPrivate

// -----------------------------------------------------------------------------
/// \ingroup Cjyx_DMMLWidgets
class qDMMLSortFilterSubjectHierarchyProxyModelPrivate
{
public:
  qDMMLSortFilterSubjectHierarchyProxyModelPrivate();

  class AttributeFilter
    {
    public:
      AttributeFilter(QString attributeName)
        : AttributeName(attributeName) { };

      AttributeFilter(QString attributeName, QVariant attributeValue, bool include, QString className=QString())
        : AttributeName(attributeName), AttributeValue(attributeValue), Include(include), ClassName(className) { };

      /// Name of the attribute to filter
      QString AttributeName;
      /// Value of the attribute to filter. Empty by default (i.e. allow any value)
      QVariant AttributeValue{QString()};
      /// Flag indicating whether this is an include filter or exclude filter.
      /// - Include filter means that only the items are shown that match the filter.
      /// - Exclude filter hides items that match the filter. Overrides include filters.
      /// True by default (i.e. include filter).
      bool Include{true};
      /// Only filter attributes on a certain type. Empty by default (i.e. allow all classes).
      /// Not used for item filters, only node filters.
      QString ClassName{QString()};
      };

    /// Find item attribute filter
    /// \return List index if found, -1 otherwise.
    int findItemAttributeFilter(QString attributeName, QVariant attributeValue, bool include);
    /// Find item attribute filters for an attribute name and include flag
    /// \return List of list indices for the found filters. Empty list if not found
    QList<int> findItemAttributeFilters(QString attributeName, bool include);
    /// Find node attribute filter
    /// \return List index if found, -1 otherwise.
    int findNodeAttributeFilter(QString attributeName, QVariant attributeValue, bool include, QString className);
    /// Find node attribute filters for an attribute name and include flag
    /// \return List of list indices for the found filters. Empty list if not found
    QList<int> findNodeAttributeFilters(QString attributeName, bool include);
    /// Remove include or exclude filters from a given filter list
    void removeFiltersByIncludeFlag(QList<AttributeFilter>& filterList, bool include);
  private:
    /// Find attribute filter in given filter list
    /// \return List index if found, -1 otherwise.
    int findAttributeFilter(QList<AttributeFilter> filterList, QString attributeName, QVariant attributeValue, bool include, QString className);
    /// Find node attribute filters for an attribute name and include flag in given filter list
    /// \param attributeName Attribute name that the found filters contain. If empty then all filters are considered
    /// \param include Include/exclude flag for the found filters.
    /// \return List of list indices for the found filters. Empty list if not found
    QList<int> findAttributeFilters(QList<AttributeFilter> filterList, QString attributeName, bool include);

  public:
    QString NameFilter;
    QStringList LevelFilter;
    QStringList NodeTypes;
    QStringList HideChildNodeTypes;
    vtkIdType HideItemsUnaffiliatedWithItemID;
    bool ShowEmptyHierarchyItems;
    QList<AttributeFilter> ItemAttributeFilters;
    QList<AttributeFilter> NodeAttributeFilters;
};

// -----------------------------------------------------------------------------
qDMMLSortFilterSubjectHierarchyProxyModelPrivate::qDMMLSortFilterSubjectHierarchyProxyModelPrivate()
  : NameFilter(QString())
  , LevelFilter(QStringList())
  , NodeTypes(QStringList())
  , HideChildNodeTypes(QStringList())
  , HideItemsUnaffiliatedWithItemID(vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
  , ShowEmptyHierarchyItems(true)
  , ItemAttributeFilters(QList<AttributeFilter>())
  , NodeAttributeFilters(QList<AttributeFilter>())
{
}

// -----------------------------------------------------------------------------
int qDMMLSortFilterSubjectHierarchyProxyModelPrivate::findItemAttributeFilter(
  QString attributeName, QVariant attributeValue, bool include)
{
  return this->findAttributeFilter(this->ItemAttributeFilters, attributeName, attributeValue, include, QString());
}

// -----------------------------------------------------------------------------
QList<int> qDMMLSortFilterSubjectHierarchyProxyModelPrivate::findItemAttributeFilters(QString attributeName, bool include)
{
  return this->findAttributeFilters(this->ItemAttributeFilters, attributeName, include);
}

// -----------------------------------------------------------------------------
int qDMMLSortFilterSubjectHierarchyProxyModelPrivate::findNodeAttributeFilter(
  QString attributeName, QVariant attributeValue, bool include, QString className)
{
  return this->findAttributeFilter(this->NodeAttributeFilters, attributeName, attributeValue, include, className);
}

// -----------------------------------------------------------------------------
QList<int> qDMMLSortFilterSubjectHierarchyProxyModelPrivate::findNodeAttributeFilters(QString attributeName, bool include)
{
  return this->findAttributeFilters(this->NodeAttributeFilters, attributeName, include);
}

// -----------------------------------------------------------------------------
int qDMMLSortFilterSubjectHierarchyProxyModelPrivate::findAttributeFilter(
  QList<AttributeFilter> filterList, QString attributeName, QVariant attributeValue, bool include, QString className)
{
  int index = 0;
  foreach (AttributeFilter filter, filterList)
    {
    if (filter.AttributeName == attributeName && filter.AttributeValue == attributeValue
      && filter.Include == include && filter.ClassName == className)
      {
      return index;
      }
    index++;
    }
  return -1;
}

// -----------------------------------------------------------------------------
QList<int> qDMMLSortFilterSubjectHierarchyProxyModelPrivate::findAttributeFilters(
  QList<AttributeFilter> filterList, QString attributeName, bool include)
{
  QList<int> foundIndices;
  int index = 0;
  foreach (AttributeFilter filter, filterList)
    {
    if ((attributeName.isEmpty() || filter.AttributeName == attributeName) && filter.Include == include)
      {
      foundIndices << index;
      }
    index++;
    }
  return foundIndices;
}

// -----------------------------------------------------------------------------
void qDMMLSortFilterSubjectHierarchyProxyModelPrivate::removeFiltersByIncludeFlag(QList<AttributeFilter>& filterList, bool include)
{
  QList<int> foundIndices = this->findAttributeFilters(filterList, QString(), include);
  if (foundIndices.size() == 0)
    {
    return;
    }

  while (foundIndices.size() > 0)
    {
    // Remove the largest index first
    int lastIndex = foundIndices.takeLast();
    filterList.removeAt(lastIndex);
    }
}

// -----------------------------------------------------------------------------
// qDMMLSortFilterSubjectHierarchyProxyModel

// -----------------------------------------------------------------------------
CTK_GET_CPP(qDMMLSortFilterSubjectHierarchyProxyModel, QString, nameFilter, NameFilter);
CTK_GET_CPP(qDMMLSortFilterSubjectHierarchyProxyModel, QStringList, levelFilter, LevelFilter);
CTK_GET_CPP(qDMMLSortFilterSubjectHierarchyProxyModel, QStringList, nodeTypes, NodeTypes);
CTK_GET_CPP(qDMMLSortFilterSubjectHierarchyProxyModel, QStringList, hideChildNodeTypes, HideChildNodeTypes);

//------------------------------------------------------------------------------
qDMMLSortFilterSubjectHierarchyProxyModel::qDMMLSortFilterSubjectHierarchyProxyModel(QObject *vparent)
 : QSortFilterProxyModel(vparent)
 , d_ptr(new qDMMLSortFilterSubjectHierarchyProxyModelPrivate)
{
  // For speed issue, we might want to disable the dynamic sorting however
  // when having source models using QStandardItemModel, drag&drop is handled
  // in 2 steps, first a new row is created (which automatically calls
  // filterAcceptsRow() that returns false) and then set the row with the
  // correct values (which doesn't call filterAcceptsRow() on the up to date
  // value unless DynamicSortFilter is true).
  this->setDynamicSortFilter(true);
}

//------------------------------------------------------------------------------
qDMMLSortFilterSubjectHierarchyProxyModel::~qDMMLSortFilterSubjectHierarchyProxyModel() = default;

//-----------------------------------------------------------------------------
vtkDMMLScene* qDMMLSortFilterSubjectHierarchyProxyModel::dmmlScene()const
{
  qDMMLSubjectHierarchyModel* model = qobject_cast<qDMMLSubjectHierarchyModel*>(this->sourceModel());
  if (!model)
    {
    return nullptr;
    }
  return model->dmmlScene();
}

//-----------------------------------------------------------------------------
vtkDMMLSubjectHierarchyNode* qDMMLSortFilterSubjectHierarchyProxyModel::subjectHierarchyNode()const
{
  qDMMLSubjectHierarchyModel* model = qobject_cast<qDMMLSubjectHierarchyModel*>(this->sourceModel());
  if (!model)
    {
    return nullptr;
    }
  return model->subjectHierarchyNode();
}

//-----------------------------------------------------------------------------
void qDMMLSortFilterSubjectHierarchyProxyModel::setNameFilter(QString filter)
{
  Q_D(qDMMLSortFilterSubjectHierarchyProxyModel);
  if (d->NameFilter == filter)
    {
    return;
    }
  d->NameFilter = filter;
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
void qDMMLSortFilterSubjectHierarchyProxyModel::addItemAttributeFilter(
  QString attributeName, QVariant attributeValue/*=QString()*/, bool include/*=true*/)
{
  Q_D(qDMMLSortFilterSubjectHierarchyProxyModel);
  if (d->findItemAttributeFilter(attributeName, attributeValue, include) >= 0)
    {
    return;
    }

  qDMMLSortFilterSubjectHierarchyProxyModelPrivate::AttributeFilter newFilter(attributeName, attributeValue, include);
  d->ItemAttributeFilters << newFilter;
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
void qDMMLSortFilterSubjectHierarchyProxyModel::removeItemAttributeFilter(QString attributeName, QVariant attributeValue, bool include)
{
  Q_D(qDMMLSortFilterSubjectHierarchyProxyModel);
  int foundIndex = d->findItemAttributeFilter(attributeName, attributeValue, include);
  if (foundIndex < 0)
    {
    qWarning() << Q_FUNC_INFO << ": Failed to remove item attribute filter by exact match";
    return;
    }

  d->ItemAttributeFilters.removeAt(foundIndex);
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
void qDMMLSortFilterSubjectHierarchyProxyModel::removeItemAttributeFilter(QString attributeName, bool include)
{
  Q_D(qDMMLSortFilterSubjectHierarchyProxyModel);
  QList<int> foundIndices = d->findItemAttributeFilters(attributeName, include);
  if (foundIndices.size() == 0)
    {
    qWarning() << Q_FUNC_INFO << ": Failed to remove item attribute filter by name and include flag";
    return;
    }

  while (foundIndices.size() > 0)
    {
    // Remove the largest index first
    int lastIndex = foundIndices.takeLast();
    d->ItemAttributeFilters.removeAt(lastIndex);
    }
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
void qDMMLSortFilterSubjectHierarchyProxyModel::addNodeAttributeFilter(
  QString attributeName, QVariant attributeValue/*=QString()*/, bool include/*=true*/, QString className/*=QString()*/)
{
  Q_D(qDMMLSortFilterSubjectHierarchyProxyModel);
  if (d->findNodeAttributeFilter(attributeName, attributeValue, include, className) >= 0)
    {
    return;
    }

  qDMMLSortFilterSubjectHierarchyProxyModelPrivate::AttributeFilter newFilter(attributeName, attributeValue, include, className);
  d->NodeAttributeFilters << newFilter;
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
void qDMMLSortFilterSubjectHierarchyProxyModel::removeNodeAttributeFilter(QString attributeName, QVariant attributeValue, bool include, QString className)
{
  Q_D(qDMMLSortFilterSubjectHierarchyProxyModel);
  int foundIndex = d->findNodeAttributeFilter(attributeName, attributeValue, include, className);
  if (foundIndex < 0)
    {
    qWarning() << Q_FUNC_INFO << ": Failed to remove node attribute filter by exact match";
    return;
    }

  d->NodeAttributeFilters.removeAt(foundIndex);
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
void qDMMLSortFilterSubjectHierarchyProxyModel::removeNodeAttributeFilter(QString attributeName, bool include)
{
  Q_D(qDMMLSortFilterSubjectHierarchyProxyModel);
  QList<int> foundIndices = d->findNodeAttributeFilters(attributeName, include);
  if (foundIndices.size() == 0)
    {
    qWarning() << Q_FUNC_INFO << ": Failed to remove node attribute filter by name and include flag";
    return;
    }

  while (foundIndices.size() > 0)
    {
    // Remove the largest index first
    int lastIndex = foundIndices.takeLast();
    d->NodeAttributeFilters.removeAt(lastIndex);
    }
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
void qDMMLSortFilterSubjectHierarchyProxyModel::setIncludeItemAttributeNamesFilter(QStringList filterList)
{
  Q_D(qDMMLSortFilterSubjectHierarchyProxyModel);

  d->removeFiltersByIncludeFlag(d->ItemAttributeFilters, true);
  foreach (QString filter, filterList)
    {
    this->addItemAttributeFilter(filter);
    }
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
QStringList qDMMLSortFilterSubjectHierarchyProxyModel::includeItemAttributeNamesFilter()const
{
  Q_D(const qDMMLSortFilterSubjectHierarchyProxyModel);

  QStringList filteredAttributeNameList;
  QList<qDMMLSortFilterSubjectHierarchyProxyModelPrivate::AttributeFilter>::const_iterator it;
  for (it = d->ItemAttributeFilters.constBegin(); it != d->ItemAttributeFilters.constEnd(); ++it)
    {
    if (it->Include == true)
      {
      filteredAttributeNameList << it->AttributeName;
      }
    }
  return filteredAttributeNameList;
}

//-----------------------------------------------------------------------------
void qDMMLSortFilterSubjectHierarchyProxyModel::setIncludeNodeAttributeNamesFilter(QStringList filterList)
{
  Q_D(qDMMLSortFilterSubjectHierarchyProxyModel);

  d->removeFiltersByIncludeFlag(d->NodeAttributeFilters, true);
  foreach (QString filter, filterList)
    {
    this->addNodeAttributeFilter(filter);
    }
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
QStringList qDMMLSortFilterSubjectHierarchyProxyModel::includeNodeAttributeNamesFilter()const
{
  Q_D(const qDMMLSortFilterSubjectHierarchyProxyModel);

  QStringList filteredAttributeNameList;
  QList<qDMMLSortFilterSubjectHierarchyProxyModelPrivate::AttributeFilter>::const_iterator it;
  for (it = d->NodeAttributeFilters.constBegin(); it != d->NodeAttributeFilters.constEnd(); ++it)
    {
    if (it->Include == true)
      {
      filteredAttributeNameList << it->AttributeName;
      }
    }
  return filteredAttributeNameList;
}

//-----------------------------------------------------------------------------
void qDMMLSortFilterSubjectHierarchyProxyModel::setExcludeItemAttributeNamesFilter(QStringList filterList)
{
  Q_D(qDMMLSortFilterSubjectHierarchyProxyModel);

  d->removeFiltersByIncludeFlag(d->ItemAttributeFilters, false);
  foreach (QString filter, filterList)
    {
    this->addItemAttributeFilter(filter, QString(), false);
    }
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
QStringList qDMMLSortFilterSubjectHierarchyProxyModel::excludeItemAttributeNamesFilter()const
{
  Q_D(const qDMMLSortFilterSubjectHierarchyProxyModel);

  QStringList filteredAttributeNameList;
  QList<qDMMLSortFilterSubjectHierarchyProxyModelPrivate::AttributeFilter>::const_iterator it;
  for (it = d->ItemAttributeFilters.constBegin(); it != d->ItemAttributeFilters.constEnd(); ++it)
    {
    if (it->Include == false)
      {
      filteredAttributeNameList << it->AttributeName;
      }
    }
  return filteredAttributeNameList;
}

//-----------------------------------------------------------------------------
void qDMMLSortFilterSubjectHierarchyProxyModel::setExcludeNodeAttributeNamesFilter(QStringList filterList)
{
  Q_D(qDMMLSortFilterSubjectHierarchyProxyModel);

  d->removeFiltersByIncludeFlag(d->NodeAttributeFilters, false);
  foreach (QString filter, filterList)
    {
    this->addNodeAttributeFilter(filter, QString(), false);
    }
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
QStringList qDMMLSortFilterSubjectHierarchyProxyModel::excludeNodeAttributeNamesFilter()const
{
  Q_D(const qDMMLSortFilterSubjectHierarchyProxyModel);

  QStringList filteredAttributeNameList;
  QList<qDMMLSortFilterSubjectHierarchyProxyModelPrivate::AttributeFilter>::const_iterator it;
  for (it = d->NodeAttributeFilters.constBegin(); it != d->NodeAttributeFilters.constEnd(); ++it)
    {
    if (it->Include == false)
      {
      filteredAttributeNameList << it->AttributeName;
      }
    }
  return filteredAttributeNameList;
}

//-----------------------------------------------------------------------------
void qDMMLSortFilterSubjectHierarchyProxyModel::setAttributeNameFilter(QString filter)
{
  Q_D(qDMMLSortFilterSubjectHierarchyProxyModel);
  d->ItemAttributeFilters.clear();
  if (!filter.isEmpty())
    {
    this->addItemAttributeFilter(filter);
    }
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
QString qDMMLSortFilterSubjectHierarchyProxyModel::attributeNameFilter()const
{
  Q_D(const qDMMLSortFilterSubjectHierarchyProxyModel);
  if (d->ItemAttributeFilters.size() > 0)
    {
    return d->ItemAttributeFilters[0].AttributeName;
    }
  return QString();
}

//-----------------------------------------------------------------------------
void qDMMLSortFilterSubjectHierarchyProxyModel::setAttributeValueFilter(QString filter)
{
  Q_D(qDMMLSortFilterSubjectHierarchyProxyModel);
  if (d->ItemAttributeFilters.size() != 1)
    {
    qCritical() << Q_FUNC_INFO << ": Attribute value filter must be set after setting name filter using attributeNameFilter";
    return;
    }

  d->ItemAttributeFilters[0].AttributeValue = filter;
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
QString qDMMLSortFilterSubjectHierarchyProxyModel::attributeValueFilter() const
{
  Q_D(const qDMMLSortFilterSubjectHierarchyProxyModel);
  if (d->ItemAttributeFilters.size() > 0)
    {
    return d->ItemAttributeFilters[0].AttributeValue.toString();
    }
  return QString();
}

//-----------------------------------------------------------------------------
void qDMMLSortFilterSubjectHierarchyProxyModel::setLevelFilter(QStringList filter)
{
  Q_D(qDMMLSortFilterSubjectHierarchyProxyModel);
  if (d->LevelFilter == filter)
    {
    return;
    }
  d->LevelFilter = filter;
  this->invalidateFilter();
}

// --------------------------------------------------------------------------
void qDMMLSortFilterSubjectHierarchyProxyModel::setNodeTypes(const QStringList& types)
{
  Q_D(qDMMLSortFilterSubjectHierarchyProxyModel);
  if (d->NodeTypes == types)
    {
    return;
    }
  d->NodeTypes = types;
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
void qDMMLSortFilterSubjectHierarchyProxyModel::setHideChildNodeTypes(const QStringList& types)
{
  Q_D(qDMMLSortFilterSubjectHierarchyProxyModel);
  if (d->HideChildNodeTypes == types)
    {
    return;
    }
  d->HideChildNodeTypes = types;
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
vtkIdType qDMMLSortFilterSubjectHierarchyProxyModel::hideItemsUnaffiliatedWithItemID()const
{
  Q_D(const qDMMLSortFilterSubjectHierarchyProxyModel);
  return d->HideItemsUnaffiliatedWithItemID;
}

//-----------------------------------------------------------------------------
void qDMMLSortFilterSubjectHierarchyProxyModel::setHideItemsUnaffiliatedWithItemID(vtkIdType itemID)
{
  Q_D(qDMMLSortFilterSubjectHierarchyProxyModel);
  if (d->HideItemsUnaffiliatedWithItemID == itemID)
    {
    return;
    }
  d->HideItemsUnaffiliatedWithItemID = itemID;
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
bool qDMMLSortFilterSubjectHierarchyProxyModel::showEmptyHierarchyItems()const
{
  Q_D(const qDMMLSortFilterSubjectHierarchyProxyModel);
  return d->ShowEmptyHierarchyItems;
}

//-----------------------------------------------------------------------------
void qDMMLSortFilterSubjectHierarchyProxyModel::setShowEmptyHierarchyItems(bool show)
{
  Q_D(qDMMLSortFilterSubjectHierarchyProxyModel);
  if (d->ShowEmptyHierarchyItems == show)
    {
    return;
    }
  d->ShowEmptyHierarchyItems = show;
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
QModelIndex qDMMLSortFilterSubjectHierarchyProxyModel::subjectHierarchySceneIndex()const
{
  qDMMLSubjectHierarchyModel* sceneModel = qobject_cast<qDMMLSubjectHierarchyModel*>(this->sourceModel());
  return this->mapFromSource(sceneModel->subjectHierarchySceneIndex());
}

//-----------------------------------------------------------------------------
vtkIdType qDMMLSortFilterSubjectHierarchyProxyModel::subjectHierarchyItemFromIndex(const QModelIndex& index)const
{
  qDMMLSubjectHierarchyModel* sceneModel = qobject_cast<qDMMLSubjectHierarchyModel*>(this->sourceModel());
  return sceneModel->subjectHierarchyItemFromIndex( this->mapToSource(index) );
}

//-----------------------------------------------------------------------------
QModelIndex qDMMLSortFilterSubjectHierarchyProxyModel::indexFromSubjectHierarchyItem(vtkIdType itemID, int column)const
{
  qDMMLSubjectHierarchyModel* sceneModel = qobject_cast<qDMMLSubjectHierarchyModel*>(this->sourceModel());
  return this->mapFromSource(sceneModel->indexFromSubjectHierarchyItem(itemID, column));
}

//------------------------------------------------------------------------------
int qDMMLSortFilterSubjectHierarchyProxyModel::acceptedItemCount(vtkIdType rootItemID/*=0*/)const
{
  vtkDMMLSubjectHierarchyNode* shNode = this->subjectHierarchyNode();
  if (!shNode)
    {
    return 0;
    }
  if (!rootItemID)
    {
    rootItemID = shNode->GetSceneItemID();
    }

  // Count the accepted items under the root item
  int itemCount = 0;
  std::vector<vtkIdType> childItemIDs;
  shNode->GetItemChildren(rootItemID, childItemIDs, true);
  for (std::vector<vtkIdType>::iterator childIt=childItemIDs.begin(); childIt!=childItemIDs.end(); ++childIt)
    {
    if (this->filterAcceptsItem(*childIt) != Reject)
      {
      itemCount++;
      }
    }
  return itemCount;
}

//-----------------------------------------------------------------------------
QStandardItem* qDMMLSortFilterSubjectHierarchyProxyModel::sourceItem(const QModelIndex& sourceIndex)const
{
  qDMMLSubjectHierarchyModel* model = qobject_cast<qDMMLSubjectHierarchyModel*>(this->sourceModel());
  if (!model)
    {
    return nullptr;
    }
  return sourceIndex.isValid() ? model->itemFromIndex(sourceIndex) : model->invisibleRootItem();
}

//------------------------------------------------------------------------------
bool qDMMLSortFilterSubjectHierarchyProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent)const
{
  QStandardItem* parentItem = this->sourceItem(sourceParent);
  if (!parentItem)
    {
    return false;
    }
  QStandardItem* item = nullptr;

  // Sometimes the row is not complete (DnD), search for a non null item
  for (int childIndex=0; childIndex < parentItem->columnCount(); ++childIndex)
    {
    item = parentItem->child(sourceRow, childIndex);
    if (item)
      {
      break;
      }
    }
  if (item == nullptr)
    {
    return false;
    }
  qDMMLSubjectHierarchyModel* model = qobject_cast<qDMMLSubjectHierarchyModel*>(this->sourceModel());
  vtkIdType itemID = model->subjectHierarchyItemFromItem(item);
  return (this->filterAcceptsItem(itemID) != Reject);
}

//------------------------------------------------------------------------------
qDMMLSortFilterSubjectHierarchyProxyModel::AcceptType qDMMLSortFilterSubjectHierarchyProxyModel::filterAcceptsItem(
  vtkIdType itemID, bool canAcceptIfAnyChildIsAccepted/*=true*/)const
{
  Q_D(const qDMMLSortFilterSubjectHierarchyProxyModel);

  if (!itemID)
    {
    return Accept;
    }
  vtkDMMLSubjectHierarchyNode* shNode = this->subjectHierarchyNode();
  if (!shNode)
    {
    return Accept;
    }
  if (itemID == shNode->GetSceneItemID())
    {
    // Always accept scene
    return Accept;
    }
  qDMMLSubjectHierarchyModel* model = qobject_cast<qDMMLSubjectHierarchyModel*>(this->sourceModel());

  // Declare condition flag that is set to true if an item check fails.
  // Needed because if an item would be filtered out based on the criteria but any of its children are shown,
  // the item needs to be shown (so that there are no orphan items in the filtered model)
  bool onlyAcceptIfAnyChildIsAccepted = false;

  // Handle hiding unaffiliated item
  // Used when the root item needs to be shown in the tree but not its siblings or other branches in the tree
  if (d->HideItemsUnaffiliatedWithItemID)
    {
    if (!model->isAffiliatedItem(itemID, d->HideItemsUnaffiliatedWithItemID))
      {
      return Reject;
      }
    }

  // Filter by data node properties
  vtkDMMLNode* dataNode = shNode->GetItemDataNode(itemID);
  if (dataNode)
    {
    // Filter by hide from editor property
    if (dataNode->GetHideFromEditors())
      {
      return Reject;
      }

    // Filter by exclude attribute
    if (dataNode->GetAttribute(vtkDMMLSubjectHierarchyConstants::GetSubjectHierarchyExcludeFromTreeAttributeName().c_str()))
      {
      return Reject;
      }

    // Filter by node type
    bool nodeTypeAccepted = false;
    if (!d->NodeTypes.isEmpty())
      {
      foreach (const QString& nodeType, d->NodeTypes)
        {
        if (dataNode->IsA(nodeType.toUtf8().data()))
          {
          nodeTypeAccepted = true;
          break;
          }
        }
      }
    else
      {
      nodeTypeAccepted = true;
      }
    if (nodeTypeAccepted)
      {
      foreach (const QString& hideChildNodeType, d->HideChildNodeTypes)
        {
        if (dataNode->IsA(hideChildNodeType.toUtf8().data()))
          {
          nodeTypeAccepted = false;
          }
        }
      }
    if (!nodeTypeAccepted)
      {
      if (canAcceptIfAnyChildIsAccepted)
        {
        // If node type was requested but is different, then only show if any of its children are shown
        onlyAcceptIfAnyChildIsAccepted = true;
        }
      else
        {
        return Reject;
        }
      }

    // Filter by node attribute
    std::vector<std::string> nodeAttributeNames = dataNode->GetAttributeNames();
    std::vector<std::string>::iterator nodeAttrIt;
    QList<qDMMLSortFilterSubjectHierarchyProxyModelPrivate::AttributeFilter>::const_iterator filterIt;
    // Handle exclude filters first
    for (filterIt = d->NodeAttributeFilters.constBegin(); filterIt != d->NodeAttributeFilters.constEnd(); ++filterIt)
      {
      if (filterIt->Include)
        {
        continue; // Skip include filters
        }
      if (!filterIt->ClassName.isEmpty() && !dataNode->IsA(filterIt->ClassName.toUtf8().constData()))
        {
        // If class name is specified and the data node is not of the class then filter does not apply
        continue;
        }
      nodeAttrIt = std::find(nodeAttributeNames.begin(), nodeAttributeNames.end(), filterIt->AttributeName.toUtf8().constData());
      if (nodeAttrIt == nodeAttributeNames.end())
        {
        // If filtered attribute name is not present in the data node then filter does not apply
        continue;
        }
      if (filterIt->AttributeValue.toString().isEmpty())
        {
        // If attribute value is not specified in the filter then reject node
        return Reject;
        }
      const char* attributeValue = dataNode->GetAttribute(nodeAttrIt->c_str());
      if (!filterIt->AttributeValue.toString().compare(attributeValue))
        {
        // If attribute value is specified and matches the attribute value in the data node then reject node
        return Reject;
        }
      }
    // Handle include filters second
    bool hasIncludeFilter = false;
    bool anyIncludedAttributeFound = false;
    for (filterIt = d->NodeAttributeFilters.constBegin(); filterIt != d->NodeAttributeFilters.constEnd(); ++filterIt)
      {
      if (!filterIt->Include)
        {
        continue; // Skip exclude filters
        }
      hasIncludeFilter = true;
      if (!filterIt->ClassName.isEmpty() && !dataNode->IsA(filterIt->ClassName.toUtf8().constData()))
        {
        // If class name is specified the data node is not of the class then filter does not apply
        continue;
        }
      nodeAttrIt = std::find(nodeAttributeNames.begin(), nodeAttributeNames.end(), filterIt->AttributeName.toUtf8().constData());
      if (nodeAttrIt == nodeAttributeNames.end())
        {
        // If filtered attribute name is not present in the data node then filter does not apply
        continue;
        }
      if (filterIt->AttributeValue.toString().isEmpty())
        {
        // If attribute value is not specified in the filter then accept
        anyIncludedAttributeFound = true;
        break;
        }
      const char* attributeValue = dataNode->GetAttribute(nodeAttrIt->c_str());
      if (!filterIt->AttributeValue.toString().compare(attributeValue))
        {
        // If attribute value is specified and matches the attribute value in the data node then accept
        anyIncludedAttributeFound = true;
        break;
        }
      }
    if (hasIncludeFilter && !anyIncludedAttributeFound)
      {
      if (canAcceptIfAnyChildIsAccepted)
        {
        // If included node attribute is missing, then only show if any of its children are shown
        onlyAcceptIfAnyChildIsAccepted = true;
        }
      else
        {
        return Reject;
        }
      }

    } // If data node
  else if (this->includeNodeAttributeNamesFilter().size() > 0)
    {
    // If there is no data node but there is an active node attribute include filter, then do not show node-less item unless any child is shown
    if (canAcceptIfAnyChildIsAccepted)
      {
      // If level was requested but is different, then only show if any of its children are shown
      onlyAcceptIfAnyChildIsAccepted = true;
      }
    else
      {
      return Reject;
      }
    }

  // Filter by level
  bool itemLevelAccepted = false;
  if (!d->LevelFilter.isEmpty())
    {
    QString itemLevel(shNode->GetItemLevel(itemID).c_str());
    foreach (const QString& levelFilter, d->LevelFilter)
      {
      if (itemLevel == levelFilter)
        {
        itemLevelAccepted = true;
        break;
        }
      }
    }
  else
    {
    itemLevelAccepted = true;
    }
  if (!itemLevelAccepted)
    {
    if (canAcceptIfAnyChildIsAccepted)
      {
      // If level was requested but is different, then only show if any of its children are shown
      onlyAcceptIfAnyChildIsAccepted = true;
      }
    else
      {
      return Reject;
      }
    }

  // Do not show items in virtual branches if their parent is not accepted for any reason
  vtkIdType parentItemID = shNode->GetItemParent(itemID);
  if (parentItemID && shNode->IsItemVirtualBranchParent(parentItemID))
    {
    if (this->filterAcceptsItem(parentItemID, false) == Reject)
      {
      return Reject;
      }
    }

  // Filter by item attribute
  QList<qDMMLSortFilterSubjectHierarchyProxyModelPrivate::AttributeFilter>::const_iterator filterIt;
  // Handle exclude filters first
  for (filterIt = d->ItemAttributeFilters.constBegin(); filterIt != d->ItemAttributeFilters.constEnd(); ++filterIt)
    {
    if (filterIt->Include)
      {
      continue; // Skip include filters
      }
    if (!shNode->HasItemAttribute(itemID, filterIt->AttributeName.toUtf8().constData()))
      {
      // If filtered attribute name is not present in the item then filter does not apply
      continue;
      }
    if (filterIt->AttributeValue.toString().isEmpty())
      {
      // If attribute value is not specified in the filter then reject item
      return Reject;
      }
    std::string attributeValue = shNode->GetItemAttribute(itemID, filterIt->AttributeName.toUtf8().constData());
    if (!filterIt->AttributeValue.toString().compare(attributeValue.c_str()))
      {
      // If attribute value is specified and matches the attribute value in the item then reject item
      return Reject;
      }
    }
  // Handle include filters second
  bool hasIncludeFilter = false;
  bool anyIncludedAttributeFound = false;
  for (filterIt = d->ItemAttributeFilters.constBegin(); filterIt != d->ItemAttributeFilters.constEnd(); ++filterIt)
    {
    if (!filterIt->Include)
      {
      continue; // Skip exclude filters
      }
    hasIncludeFilter = true;
    if (!shNode->HasItemAttribute(itemID, filterIt->AttributeName.toUtf8().constData()))
      {
      // If filtered attribute name is not present in the item then filter does not apply
      continue;
      }
    if (filterIt->AttributeValue.toString().isEmpty())
      {
      // If attribute value is not specified in the filter then accept
      anyIncludedAttributeFound = true;
      break;
      }
    std::string attributeValue = shNode->GetItemAttribute(itemID, filterIt->AttributeName.toUtf8().constData());
    if (!filterIt->AttributeValue.toString().compare(attributeValue.c_str()))
      {
      // If attribute value is specified and matches the attribute value in the item then accept
      anyIncludedAttributeFound = true;
      break;
      }
    }
  if (hasIncludeFilter && !anyIncludedAttributeFound)
    {
    if (canAcceptIfAnyChildIsAccepted)
      {
      // If included item attribute is missing, then only show if any of its children are shown
      onlyAcceptIfAnyChildIsAccepted = true;
      }
    else
      {
      return Reject;
      }
    }

  // Filter by item name
  if (!d->NameFilter.isEmpty())
    {
    QString itemName(shNode->GetItemName(itemID).c_str());
    if (!itemName.contains(d->NameFilter, Qt::CaseInsensitive))
      {
      if (canAcceptIfAnyChildIsAccepted)
        {
        // If item name was requested but different, then only show if any of its children are shown
        onlyAcceptIfAnyChildIsAccepted = true;
        }
      else
        {
        return Reject;
        }
      }
    }

  // Hide hierarchy item if none of its children are accepted and the corresponding filter is turned on
  if (!d->ShowEmptyHierarchyItems && (!dataNode || dataNode->IsA("vtkDMMLFolderDisplayNode")))
    {
    onlyAcceptIfAnyChildIsAccepted = true;
    }

  // If the visibility of an item depends on whether any of its children are shown, then evaluate that condition
  if (onlyAcceptIfAnyChildIsAccepted)
    {
    bool isChildShown = false;
    std::vector<vtkIdType> childItemIDs;
    shNode->GetItemChildren(itemID, childItemIDs, true);
    for (std::vector<vtkIdType>::iterator childIt=childItemIDs.begin(); childIt!=childItemIDs.end(); ++childIt)
      {
      if (this->filterAcceptsItem(*childIt) != Reject)
        {
        isChildShown = true;
        break;
        }
      }
    if (isChildShown)
      {
      return AcceptDueToBeingParentOfAccepted;
      }
    else
      {
      return Reject;
      }
    }

  // All criteria were met
  return Accept;
}

//------------------------------------------------------------------------------
Qt::ItemFlags qDMMLSortFilterSubjectHierarchyProxyModel::flags(const QModelIndex & index)const
{
  vtkIdType itemID = this->subjectHierarchyItemFromIndex(index);
  if (!itemID)
    {
    // Extra item (e.g. None)
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

  bool isSelectable = (this->filterAcceptsItem(itemID) == Accept);
  qDMMLSubjectHierarchyModel* sceneModel = qobject_cast<qDMMLSubjectHierarchyModel*>(this->sourceModel());
  QStandardItem* item = sceneModel->itemFromSubjectHierarchyItem(itemID, index.column());
  if (!item)
    {
    return Qt::ItemFlags();
    }

  if (isSelectable)
    {
    return item->flags() | Qt::ItemIsSelectable;
    }
  else
    {
    return item->flags() & ~Qt::ItemIsSelectable;
    }
}
