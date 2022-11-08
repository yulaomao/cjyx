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

// qDMML includes
#include "qDMMLSceneModel.h"
#include "qDMMLSortFilterProxyModel.h"

// VTK includes
#include <vtkDMMLNode.h>
#include <vtkDMMLScene.h>

// -----------------------------------------------------------------------------
// qDMMLSortFilterProxyModelPrivate

// -----------------------------------------------------------------------------
class qDMMLSortFilterProxyModelPrivate
{
public:
  qDMMLSortFilterProxyModelPrivate();

  QStringList                      NodeTypes;
  bool                             ShowHidden;
  QStringList                      ShowHiddenForTypes;
  bool                             ShowChildNodeTypes;
  QStringList                      HideChildNodeTypes;
  QStringList                      HiddenNodeIDs;
  QStringList                      VisibleNodeIDs;
  QString                          HideNodesUnaffiliatedWithNodeID;
  typedef QPair<QString, QVariant> AttributeType;
  QHash<QString, AttributeType>    Attributes;
  qDMMLSortFilterProxyModel::FilterType Filter;
};

// -----------------------------------------------------------------------------
qDMMLSortFilterProxyModelPrivate::qDMMLSortFilterProxyModelPrivate()
{
  this->ShowHidden = false;
  this->ShowChildNodeTypes = true;
  this->Filter = qDMMLSortFilterProxyModel::UseFilters;
}

// -----------------------------------------------------------------------------
// qDMMLSortFilterProxyModel

//------------------------------------------------------------------------------
qDMMLSortFilterProxyModel::qDMMLSortFilterProxyModel(QObject *vparent)
  :QSortFilterProxyModel(vparent)
  , d_ptr(new qDMMLSortFilterProxyModelPrivate)
{
  // For speed issue, we might want to disable the dynamic sorting however
  // when having source models using QStandardItemModel, drag&drop is handled
  // in 2 steps, first a new row is created (which automatically calls
  // filterAcceptsRow() that returns false) and then set the row with the
  // correct values (which doesn't call filterAcceptsRow() on the up to date
  // value unless DynamicSortFilter is true).
  this->setDynamicSortFilter(true);

  this->setFilterCaseSensitivity(Qt::CaseInsensitive);
}

//------------------------------------------------------------------------------
qDMMLSortFilterProxyModel::~qDMMLSortFilterProxyModel() = default;

// -----------------------------------------------------------------------------
QStandardItem* qDMMLSortFilterProxyModel::sourceItem(const QModelIndex& sourceIndex)const
{
  qDMMLSceneModel* sceneModel = qobject_cast<qDMMLSceneModel*>(this->sourceModel());
  if (sceneModel == nullptr)
    {
    //Q_ASSERT(sceneModel);
    return nullptr;
    }
  return sourceIndex.isValid() ? sceneModel->itemFromIndex(sourceIndex) : sceneModel->invisibleRootItem();
}

//-----------------------------------------------------------------------------
vtkDMMLScene* qDMMLSortFilterProxyModel::dmmlScene()const
{
  qDMMLSceneModel* sceneModel = qobject_cast<qDMMLSceneModel*>(this->sourceModel());
  return sceneModel->dmmlScene();
}

//-----------------------------------------------------------------------------
QModelIndex qDMMLSortFilterProxyModel::dmmlSceneIndex()const
{
  qDMMLSceneModel* sceneModel = qobject_cast<qDMMLSceneModel*>(this->sourceModel());
  return this->mapFromSource(sceneModel->dmmlSceneIndex());
}

//-----------------------------------------------------------------------------
vtkDMMLNode* qDMMLSortFilterProxyModel::dmmlNodeFromIndex(const QModelIndex& proxyIndex)const
{
  qDMMLSceneModel* sceneModel = qobject_cast<qDMMLSceneModel*>(this->sourceModel());
  return sceneModel->dmmlNodeFromIndex(this->mapToSource(proxyIndex));
}

//-----------------------------------------------------------------------------
QModelIndex qDMMLSortFilterProxyModel::indexFromDMMLNode(vtkDMMLNode* node, int column)const
{
  qDMMLSceneModel* sceneModel = qobject_cast<qDMMLSceneModel*>(this->sourceModel());
  return this->mapFromSource(sceneModel->indexFromNode(node, column));
}

//-----------------------------------------------------------------------------
void qDMMLSortFilterProxyModel::addAttribute(const QString& nodeType,
                                              const QString& attributeName,
                                              const QVariant& attributeValue)
{
  Q_D(qDMMLSortFilterProxyModel);
  if (!d->NodeTypes.contains(nodeType) ||
      (d->Attributes.value(nodeType).first == attributeName &&
       d->Attributes.value(nodeType).second == attributeValue))
    {
    return;
    }
  d->Attributes[nodeType] =
    qDMMLSortFilterProxyModelPrivate::AttributeType(attributeName, attributeValue);
  this->invalidateFilter();
}

//------------------------------------------------------------------------------
void qDMMLSortFilterProxyModel::removeAttribute(const QString& nodeType,
                                              const QString& attributeName)
{
  Q_D(qDMMLSortFilterProxyModel);
  if (!d->NodeTypes.contains(nodeType) ||
      d->Attributes.value(nodeType).first != attributeName)
    {
    return;
    }
  d->Attributes.remove(nodeType);
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
QVariant qDMMLSortFilterProxyModel::attributeFilter(const QString& nodeType,
                                                    const QString& attributeName) const
{
  Q_UNUSED(attributeName);
  Q_D(const qDMMLSortFilterProxyModel);
  return d->Attributes.value(nodeType).second;
}

//------------------------------------------------------------------------------
//bool qDMMLSortFilterProxyModel::filterAcceptsColumn(int source_column, const QModelIndex & source_parent)const;

//------------------------------------------------------------------------------
bool qDMMLSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent)const
{
  QStandardItem* parentItem = this->sourceItem(source_parent);
  if (parentItem == nullptr)
    {
    //Q_ASSERT(parentItem);
    return false;
    }
  QStandardItem* item = nullptr;
  // Sometimes the row is not complete (DnD), search for a non null item
  for (int childIndex = 0; childIndex < parentItem->columnCount(); ++childIndex)
    {
    item = parentItem->child(source_row, childIndex);
    if (item)
      {
      break;
      }
    }
  if (item == nullptr)
    {
    //Q_ASSERT(item);
    return false;
    }
  qDMMLSceneModel* sceneModel = qobject_cast<qDMMLSceneModel*>(this->sourceModel());
  vtkDMMLNode* node = sceneModel->dmmlNodeFromItem(item);
  AcceptType accept = this->filterAcceptsNode(node);
  bool acceptRow = (accept == Accept);
  if (accept == AcceptButPotentiallyRejectable)
    {
    acceptRow = this->QSortFilterProxyModel::filterAcceptsRow(source_row,
                                                              source_parent);
    }
  if (node &&
      sceneModel->listenNodeModifiedEvent() == qDMMLSceneModel::OnlyVisibleNodes &&
      accept != Reject)
    {
    sceneModel->observeNode(node);
    }
  return acceptRow;
}

//------------------------------------------------------------------------------
qDMMLSortFilterProxyModel::AcceptType qDMMLSortFilterProxyModel
::filterAcceptsNode(vtkDMMLNode* node)const
{
  Q_D(const qDMMLSortFilterProxyModel);
  qDMMLSceneModel* sceneModel = qobject_cast<qDMMLSceneModel*>(this->sourceModel());
  if (!node || !node->GetID())
    {
    return Accept;
    }
  if (this->showAll())
    {
    return Accept;
    }
  if (this->hideAll())
    {
    return Reject;
    }
  if (d->HiddenNodeIDs.contains(node->GetID()))
    {
    return Reject;
    }
  if (d->VisibleNodeIDs.contains(node->GetID()))
    {
    return Accept;
    }
  // HideFromEditors property
  if (!d->ShowHidden && node->GetHideFromEditors())
    {
    bool hide = true;
    foreach(const QString& nodeType, d->ShowHiddenForTypes)
      {
      if (node->IsA(nodeType.toUtf8()))
        {
        hide = false;
        break;
        }
      }
    if (hide)
      {
      return Reject;
      }
    }

  if (!d->HideNodesUnaffiliatedWithNodeID.isEmpty())
    {
    vtkDMMLNode* theNode = sceneModel->dmmlScene()->GetNodeByID(
      d->HideNodesUnaffiliatedWithNodeID.toUtf8());
    bool affiliated = sceneModel->isAffiliatedNode(node, theNode);
    if (!affiliated)
      {
      return Reject;
      }
    }

  // Accept all the nodes if no type has been set
  if (d->NodeTypes.isEmpty())
    {
    // Apply filter if any
    return AcceptButPotentiallyRejectable;
    }
  foreach(const QString& nodeType, d->NodeTypes)
    {
    // filter by node type
    if (!node->IsA(nodeType.toUtf8().data()))
      {
      //std::cout << "Reject node: " << node->GetName() << "(" << node->GetID()
      //          << ") type: " << typeid(*node).name() <<std::endl;
      continue;
      }
    // filter by excluded child node types
    if (!d->ShowChildNodeTypes && nodeType != node->GetClassName())
      {
      continue;
      }
    // filter by HideChildNodeType
    if (d->ShowChildNodeTypes)
      {
      foreach(const QString& hideChildNodeType, d->HideChildNodeTypes)
        {
        if (node->IsA(hideChildNodeType.toUtf8().data()))
          {
          return Reject;
          }
        }
      }

    // filter by attributes
    if (d->Attributes.contains(nodeType))
      {
      // can be optimized if the event is AttributeModifiedEvent instead of modifiedevent
      const_cast<qDMMLSortFilterProxyModel*>(this)->qvtkConnect(
        node, vtkCommand::ModifiedEvent,
        const_cast<qDMMLSortFilterProxyModel*>(this),
        SLOT(invalidate()),0., Qt::UniqueConnection);

      QString attributeName = d->Attributes[nodeType].first;
      const char *nodeAttribute = node->GetAttribute(attributeName.toUtf8());
      QString nodeAttributeQString = node->GetAttribute(attributeName.toUtf8());
      QString testAttribute = d->Attributes[nodeType].second.toString();

      //std::cout << "attribute name = " << qPrintable(attributeName) << "\n\ttestAttribute = " << qPrintable(testAttribute) << "\n\t" << node->GetID() << " nodeAttributeQString = " << qPrintable(nodeAttributeQString) << "\n\t\tas char str = " << (nodeAttribute ? nodeAttribute : "null") << "." << std::endl;
      // fail if the attribute isn't defined on the node at all
      if (nodeAttribute == nullptr)
        {
        return RejectButPotentiallyAcceptable;
        }
      // if the filter value is null, any node attribute value will match
      if (!d->Attributes[nodeType].second.isNull())
        {
        // otherwise, the node and filter attributes have to match
        if (testAttribute != nodeAttribute)
          {
          return RejectButPotentiallyAcceptable;
          }
        }
      }
    // Apply filter if any
    return AcceptButPotentiallyRejectable;
    }
  return Reject;
}

//-----------------------------------------------------------------------------
QStringList qDMMLSortFilterProxyModel::hideChildNodeTypes()const
{
  Q_D(const qDMMLSortFilterProxyModel);
  return d->HideChildNodeTypes;
}

// --------------------------------------------------------------------------
QStringList qDMMLSortFilterProxyModel::nodeTypes()const
{
  Q_D(const qDMMLSortFilterProxyModel);
  return d->NodeTypes;
}

//-----------------------------------------------------------------------------
void qDMMLSortFilterProxyModel::setHideChildNodeTypes(const QStringList& _nodeTypes)
{
  Q_D(qDMMLSortFilterProxyModel);
  if (_nodeTypes == d->HideChildNodeTypes)
    {
    return;
    }
  d->HideChildNodeTypes = _nodeTypes;
  this->invalidateFilter();
}

// --------------------------------------------------------------------------
void qDMMLSortFilterProxyModel::setNodeTypes(const QStringList& _nodeTypes)
{
  Q_D(qDMMLSortFilterProxyModel);
  if (d->NodeTypes == _nodeTypes)
    {
    return;
    }
  d->NodeTypes = _nodeTypes;
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
void qDMMLSortFilterProxyModel::setShowChildNodeTypes(bool _show)
{
  Q_D(qDMMLSortFilterProxyModel);
  if (_show == d->ShowChildNodeTypes)
    {
    return;
    }
  d->ShowChildNodeTypes = _show;
  invalidateFilter();
}

//-----------------------------------------------------------------------------
bool qDMMLSortFilterProxyModel::showChildNodeTypes()const
{
  Q_D(const qDMMLSortFilterProxyModel);
  return d->ShowChildNodeTypes;
}

// --------------------------------------------------------------------------
void qDMMLSortFilterProxyModel::setShowHidden(bool enable)
{
  Q_D(qDMMLSortFilterProxyModel);
  if (enable == d->ShowHidden)
    {
    return;
    }
  d->ShowHidden = enable;
  this->invalidateFilter();
}

// --------------------------------------------------------------------------
bool qDMMLSortFilterProxyModel::showHidden()const
{
  Q_D(const qDMMLSortFilterProxyModel);
  return d->ShowHidden;
}

// --------------------------------------------------------------------------
void qDMMLSortFilterProxyModel::setShowHiddenForTypes(const QStringList& types)
{
  Q_D(qDMMLSortFilterProxyModel);
  if (types == d->ShowHiddenForTypes)
    {
    return;
    }
  d->ShowHiddenForTypes = types;
  this->invalidateFilter();
}

// --------------------------------------------------------------------------
QStringList qDMMLSortFilterProxyModel::showHiddenForTypes()const
{
  Q_D(const qDMMLSortFilterProxyModel);
  return d->ShowHiddenForTypes;
}

// --------------------------------------------------------------------------
void qDMMLSortFilterProxyModel::setHiddenNodeIDs(const QStringList& nodeIDs)
{
  Q_D(qDMMLSortFilterProxyModel);
  if (nodeIDs == d->HiddenNodeIDs)
    {
    return;
    }
  d->HiddenNodeIDs = nodeIDs;
  this->invalidateFilter();
}

// --------------------------------------------------------------------------
QStringList qDMMLSortFilterProxyModel::hiddenNodeIDs()const
{
  Q_D(const qDMMLSortFilterProxyModel);
  return d->HiddenNodeIDs;
}

// --------------------------------------------------------------------------
void qDMMLSortFilterProxyModel::setVisibleNodeIDs(const QStringList& nodeIDs)
{
  Q_D(qDMMLSortFilterProxyModel);
  if (nodeIDs == d->VisibleNodeIDs)
    {
    return;
    }
  d->VisibleNodeIDs = nodeIDs;
  this->invalidateFilter();
}

// --------------------------------------------------------------------------
QStringList qDMMLSortFilterProxyModel::visibleNodeIDs()const
{
  Q_D(const qDMMLSortFilterProxyModel);
  return d->VisibleNodeIDs;
}

// --------------------------------------------------------------------------
void qDMMLSortFilterProxyModel
::setHideNodesUnaffiliatedWithNodeID(const QString& nodeID)
{
  Q_D(qDMMLSortFilterProxyModel);
  if (nodeID == d->HideNodesUnaffiliatedWithNodeID)
    {
    return;
    }
  d->HideNodesUnaffiliatedWithNodeID = nodeID;
  this->invalidateFilter();
}

// --------------------------------------------------------------------------
QString qDMMLSortFilterProxyModel::hideNodesUnaffiliatedWithNodeID()const
{
  Q_D(const qDMMLSortFilterProxyModel);
  return d->HideNodesUnaffiliatedWithNodeID;
}

// --------------------------------------------------------------------------
void qDMMLSortFilterProxyModel
::setFilterType(FilterType filterType)
{
  Q_D(qDMMLSortFilterProxyModel);
  if (filterType == d->Filter)
    {
    return;
    }
  d->Filter = filterType;
  this->invalidateFilter();
}

// --------------------------------------------------------------------------
qDMMLSortFilterProxyModel::FilterType qDMMLSortFilterProxyModel
::filterType()const
{
  Q_D(const qDMMLSortFilterProxyModel);
  return d->Filter;
}

// --------------------------------------------------------------------------
void qDMMLSortFilterProxyModel
::setShowAll(bool show)
{
  if (show == this->showAll())
    {
    return;
    }
  this->setFilterType((show ? qDMMLSortFilterProxyModel::ShowAll :
                       (this->hideAll() ? qDMMLSortFilterProxyModel::HideAll :
                        qDMMLSortFilterProxyModel::UseFilters)));
}

// --------------------------------------------------------------------------
bool qDMMLSortFilterProxyModel::showAll()const
{
  Q_D(const qDMMLSortFilterProxyModel);
  return d->Filter == qDMMLSortFilterProxyModel::ShowAll;
}

// --------------------------------------------------------------------------
void qDMMLSortFilterProxyModel
::setHideAll(bool hide)
{
  if (hide == this->hideAll())
    {
    return;
    }
  this->setFilterType((hide ? qDMMLSortFilterProxyModel::HideAll :
                       (this->showAll() ? qDMMLSortFilterProxyModel:: ShowAll :
                        qDMMLSortFilterProxyModel::UseFilters)));
}

// --------------------------------------------------------------------------
bool qDMMLSortFilterProxyModel::hideAll()const
{
  Q_D(const qDMMLSortFilterProxyModel);
  return d->Filter == qDMMLSortFilterProxyModel::HideAll;
}

// --------------------------------------------------------------------------
qDMMLSceneModel* qDMMLSortFilterProxyModel::sceneModel()const
{
  return qobject_cast<qDMMLSceneModel*>(this->sourceModel());
}
