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

// STL includes
#include <deque>

// Qt includes
#include <QDebug>
#include <QHeaderView>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QScrollBar>

// qDMML includes
#include "qDMMLItemDelegate.h"
#include "qDMMLSceneDisplayableModel.h"
#include "qDMMLSceneTransformModel.h"
#include "qDMMLSortFilterHierarchyProxyModel.h"
#include "qDMMLTreeView_p.h"

// DMML includes
#include <vtkDMMLDisplayableHierarchyNode.h>
#include <vtkDMMLDisplayableNode.h>
#include <vtkDMMLDisplayNode.h>
#include <vtkDMMLModelNode.h>
#include <vtkDMMLSelectionNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkCollectionIterator.h>
#include <vtkWeakPointer.h>

//------------------------------------------------------------------------------
qDMMLTreeViewPrivate::qDMMLTreeViewPrivate(qDMMLTreeView& object)
  : q_ptr(&object)
{
  this->SceneModel = nullptr;
  this->SortFilterModel = nullptr;
  this->FitSizeToVisibleIndexes = true;
  this->TreeViewSizeHint = QSize();
  this->TreeViewMinSizeHint = QSize(120, 120);
  this->ShowScene = true;
  this->ShowRootNode = false;
  this->NodeMenu = nullptr;
  this->RenameAction = nullptr;
  this->DeleteAction = nullptr;
  this->EditAction = nullptr;
  this->SceneMenu = nullptr;
  this->ExpandedNodes = vtkCollection::New();
}
//------------------------------------------------------------------------------
qDMMLTreeViewPrivate::~qDMMLTreeViewPrivate()
{
  this->ExpandedNodes->Delete();
}

//------------------------------------------------------------------------------
void qDMMLTreeViewPrivate::init()
{
  Q_Q(qDMMLTreeView);

  q->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding));
  q->setItemDelegate(new qDMMLItemDelegate(q));
  q->setAutoScrollMargin(32); // scroll hot area sensitivity
  this->setSortFilterProxyModel(new qDMMLSortFilterProxyModel(q));
  q->setSceneModelType("Transform");

  //ctkModelTester * tester = new ctkModelTester(p);
  //tester->setModel(this->SortFilterModel);
  //QObject::connect(q, SIGNAL(activated(QModelIndex)),
  //                 q, SLOT(onActivated(QModelIndeK)));
  //QObject::connect(q, SIGNAL(clicked(QModelIndex)),
  //                 q, SLOT(onActivated(QModelIndex)));

  q->setUniformRowHeights(true);

  QObject::connect(q, SIGNAL(collapsed(QModelIndex)),
                   q, SLOT(onNumberOfVisibleIndexChanged()));
  QObject::connect(q, SIGNAL(expanded(QModelIndex)),
                   q, SLOT(onNumberOfVisibleIndexChanged()));
//QObject::connect(q->header(), SIGNAL(sectionResized(int,int,int)),
  //                  q, SLOT(onSectionResized()));
  q->horizontalScrollBar()->installEventFilter(q);

  this->NodeMenu = new QMenu(q);
  this->NodeMenu->setObjectName("nodeMenuTreeView");

  // rename node
  this->RenameAction =
    new QAction(qDMMLTreeView::tr("Rename"),this->NodeMenu);
  this->NodeMenu->addAction(this->RenameAction);
  QObject::connect(this->RenameAction, SIGNAL(triggered()),
                   q, SLOT(renameCurrentNode()));

  // delete node
  this->DeleteAction =
    new QAction(qDMMLTreeView::tr("Delete"),this->NodeMenu);
  this->NodeMenu->addAction(this->DeleteAction);
  QObject::connect(this->DeleteAction, SIGNAL(triggered()),
                   q, SLOT(deleteCurrentNode()));
  // EditAction is hidden by default
  this->EditAction =
    new QAction(qDMMLTreeView::tr("Edit properties..."), this->NodeMenu);
  QObject::connect(this->EditAction, SIGNAL(triggered()),
                   q, SLOT(editCurrentNode()));
  this->SceneMenu = new QMenu(q);
  this->SceneMenu->setObjectName("sceneMenuTreeView");
  this->ExpandedNodes->RemoveAllItems();

  q->setContextMenuPolicy(Qt::CustomContextMenu);
  QObject::connect(q, SIGNAL(customContextMenuRequested(const QPoint&)), q, SLOT(onCustomContextMenu(const QPoint&)));
}

//------------------------------------------------------------------------------
void qDMMLTreeViewPrivate::setSceneModel(qDMMLSceneModel* newModel)
{
  Q_Q(qDMMLTreeView);
  if (!newModel)
    {
    return;
    }

  newModel->setDMMLScene(q->dmmlScene());

  this->SceneModel = newModel;
  this->SortFilterModel->setSourceModel(this->SceneModel);
  QObject::connect(this->SceneModel, SIGNAL(sceneAboutToBeUpdated()),
                   q, SLOT(saveTreeExpandState()));
  QObject::connect(this->SceneModel, SIGNAL(sceneUpdated()),
                   q, SLOT(loadTreeExpandState()));
  q->expandToDepth(2);
}

//------------------------------------------------------------------------------
void qDMMLTreeViewPrivate::setSortFilterProxyModel(qDMMLSortFilterProxyModel* newSortModel)
{
  Q_Q(qDMMLTreeView);
  if (newSortModel == this->SortFilterModel)
    {
    return;
    }

  // delete the previous filter
  delete this->SortFilterModel;
  this->SortFilterModel = newSortModel;
  // Set the input of the view
  // if no filter is given then let's show the scene model directly
  q->QTreeView::setModel(this->SortFilterModel
    ? static_cast<QAbstractItemModel*>(this->SortFilterModel)
    : static_cast<QAbstractItemModel*>(this->SceneModel));
  // Setting a new model to the view resets the selection model. Reobserve
  // the selectionChanged signal. Observing currentRowChanged() is discouraged.
  QObject::connect(q->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                   q, SLOT(onSelectionChanged(QItemSelection,QItemSelection)));
  if (!this->SortFilterModel)
    {
    return;
    }
  this->SortFilterModel->setParent(q);
  // Set the input of the filter
  this->SortFilterModel->setSourceModel(this->SceneModel);

  // resize the view if new rows are added/removed
  QObject::connect(this->SortFilterModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                   q, SLOT(onNumberOfVisibleIndexChanged()));
  QObject::connect(this->SortFilterModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                   q, SLOT(onNumberOfVisibleIndexChanged()));

  q->expandToDepth(2);
  q->onNumberOfVisibleIndexChanged();
}

//------------------------------------------------------------------------------
void qDMMLTreeViewPrivate::recomputeSizeHint(bool force)
{
  Q_Q(qDMMLTreeView);
  this->TreeViewSizeHint = QSize();
  if ((this->FitSizeToVisibleIndexes || force) && q->isVisible())
    {
    // TODO: if the number of items changes often, don't update geometry,
    // it might be too expensive, maybe use a timer
    q->updateGeometry();
    }
}

//------------------------------------------------------------------------------
QSize qDMMLTreeViewPrivate::sizeHint()const
{
  Q_Q(const qDMMLTreeView);
  if (!this->FitSizeToVisibleIndexes)
    {
    return q->QTreeView::sizeHint();
    }
  if (this->TreeViewSizeHint.isValid())
    {
    return this->TreeViewSizeHint;
    }
  int visibleIndexCount = 0;
  for(QModelIndex index = this->SortFilterModel->dmmlSceneIndex();
      index.isValid();
      index = q->indexBelow(index))
    {
    ++visibleIndexCount;
    }
  this->TreeViewSizeHint = q->QTreeView::sizeHint();
  this->TreeViewSizeHint.setHeight(
    q->frameWidth()
    + (q->isHeaderHidden() ? 0 : q->header()->sizeHint().height())
    + visibleIndexCount * q->sizeHintForRow(0)
    + (q->horizontalScrollBar()->isVisibleTo(const_cast<qDMMLTreeView*>(q)) ? q->horizontalScrollBar()->height() : 0)
    + q->frameWidth());
  // Add half a line to give some space under the tree
  this->TreeViewSizeHint.rheight() += q->sizeHintForRow(0) / 2;
  this->TreeViewSizeHint =
    this->TreeViewSizeHint.expandedTo(this->TreeViewMinSizeHint);
  return this->TreeViewSizeHint;
}

//------------------------------------------------------------------------------
void qDMMLTreeViewPrivate::saveChildrenExpandState(QModelIndex &parentIndex)
{
  Q_Q(qDMMLTreeView);
  vtkDMMLNode* parentNode = q->sortFilterProxyModel()->dmmlNodeFromIndex(parentIndex);

  // Check if the node is currently present in the scene.
  // When a node/hierarchy is being deleted from the vtkDMMLScene, there is
  // some reference of the deleted node left dangling in the qDMMLSceneModel.
  // As a result, dmmlNodeFromIndex returns a reference to a non-existent node.
  // We do not need to save the tree hierarchy in such cases.
  if (!parentNode ||
      !q->sortFilterProxyModel()->dmmlScene()->IsNodePresent(parentNode))
    {
    return;
    }

    if (q->isExpanded(parentIndex))
      {
      this->ExpandedNodes->AddItem(parentNode);
      }
    // Iterate over children nodes recursively to save their expansion state
    unsigned int numChildrenRows = q->sortFilterProxyModel()->rowCount(parentIndex);
    for(unsigned int row = 0; row < numChildrenRows; ++row)
      {
      QModelIndex childIndex = q->sortFilterProxyModel()->index(row, 0, parentIndex);
      this->saveChildrenExpandState(childIndex);
      }
}

//-----------------------------------------------------------------------------
void qDMMLTreeViewPrivate::scrollTo(const QString& name, bool next)
{
  Q_Q(qDMMLTreeView);
  this->LastScrollToName = name;
  QModelIndex startIndex = q->model()->index(0,0);
  QModelIndexList matchingModels = q->model()->match(
    startIndex,
    Qt::DisplayRole, name, -1,
    Qt::MatchContains|Qt::MatchWrap|Qt::MatchRecursive);
  if (!matchingModels.size())
    {
    return;
    }
  int lastSearch = matchingModels.indexOf(
    q->selectionModel()->currentIndex());
  if (lastSearch == -1 || next)
    {
    ++lastSearch;
    }
  int newSearch = lastSearch % matchingModels.size();
  QModelIndex modelIndex = matchingModels[newSearch];
  q->scrollTo(
    modelIndex, QAbstractItemView::PositionAtTop);
  q->selectionModel()->setCurrentIndex(
    modelIndex, QItemSelectionModel::Current);
}

//------------------------------------------------------------------------------
// qDMMLTreeView
//------------------------------------------------------------------------------
qDMMLTreeView::qDMMLTreeView(QWidget *_parent)
  :QTreeView(_parent)
  , d_ptr(new qDMMLTreeViewPrivate(*this))
{
  Q_D(qDMMLTreeView);
  d->init();
}

//------------------------------------------------------------------------------
qDMMLTreeView::qDMMLTreeView(qDMMLTreeViewPrivate* pimpl, QWidget *parentObject)
  :Superclass(parentObject)
  , d_ptr(pimpl)
{
  Q_D(qDMMLTreeView);
  d->init(/*factory*/);
}

//------------------------------------------------------------------------------
qDMMLTreeView::~qDMMLTreeView() = default;

//------------------------------------------------------------------------------
void qDMMLTreeView::setDMMLScene(vtkDMMLScene* scene)
{
  Q_D(qDMMLTreeView);
  Q_ASSERT(d->SortFilterModel);
  vtkDMMLNode* rootNode = this->rootNode();
  // only qDMMLSceneModel needs the scene, the other proxies don't care.
  d->SceneModel->setDMMLScene(scene);
  this->setRootNode(rootNode);
  this->expandToDepth(2);
}

//------------------------------------------------------------------------------
QString qDMMLTreeView::sceneModelType()const
{
  Q_D(const qDMMLTreeView);
  return d->SceneModelType;
}

//------------------------------------------------------------------------------
void qDMMLTreeView::setSceneModelType(const QString& modelName)
{
  Q_D(qDMMLTreeView);

  qDMMLSceneModel* newModel = nullptr;
  qDMMLSortFilterProxyModel* newFilterModel = d->SortFilterModel;
  // switch on the incoming model name
  if (modelName == QString("Transform"))
    {
    newModel = new qDMMLSceneTransformModel(this);
    }
  else if (modelName == QString("Displayable"))
    {
    newModel = new qDMMLSceneDisplayableModel(this);
    }
  else if (modelName == QString(""))
    {
    newModel = new qDMMLSceneModel(this);
    }
  if (newModel)
    {
    d->SceneModelType = modelName;
    newModel->setListenNodeModifiedEvent(this->listenNodeModifiedEvent());
    }
  if (newFilterModel)
    {
    newFilterModel->setNodeTypes(this->nodeTypes());
    newFilterModel->setShowHidden(this->showHidden());
    }
  d->setSceneModel(newModel);
  d->setSortFilterProxyModel(newFilterModel);
}

//------------------------------------------------------------------------------
void qDMMLTreeView::setSceneModel(qDMMLSceneModel* newSceneModel, const QString& modelType)
{
  Q_D(qDMMLTreeView);

  if (!newSceneModel)
    {
    return;
    }
  d->SceneModelType = modelType;
  d->setSceneModel(newSceneModel);
}

//------------------------------------------------------------------------------
void qDMMLTreeView::setSortFilterProxyModel(qDMMLSortFilterProxyModel* newFilterModel)
{
  Q_D(qDMMLTreeView);

  if (!newFilterModel)
    {
    return;
    }
  d->setSortFilterProxyModel(newFilterModel);
}

//------------------------------------------------------------------------------
vtkDMMLScene* qDMMLTreeView::dmmlScene()const
{
  Q_D(const qDMMLTreeView);
  return d->SceneModel ? d->SceneModel->dmmlScene() : nullptr;
}

//------------------------------------------------------------------------------
vtkDMMLNode* qDMMLTreeView::currentNode()const
{
  Q_D(const qDMMLTreeView);
  return d->SortFilterModel->dmmlNodeFromIndex(this->selectionModel()->currentIndex());
}

//------------------------------------------------------------------------------
void qDMMLTreeView::setCurrentNode(vtkDMMLNode* node)
{
  Q_D(const qDMMLTreeView);
  QModelIndex nodeIndex = d->SortFilterModel->indexFromDMMLNode(node);
  this->setCurrentIndex(nodeIndex);
}

//------------------------------------------------------------------------------
void qDMMLTreeView::onSelectionChanged(const QItemSelection & selected,
                                       const QItemSelection & deselected)
{
  Q_UNUSED(deselected);
  Q_D(qDMMLTreeView);
  vtkDMMLNode* newCurrentNode = nullptr;
  if (selected.indexes().count() > 0)
    {
    newCurrentNode = d->SortFilterModel->dmmlNodeFromIndex(selected.indexes()[0]);
    }
  emit currentNodeChanged(newCurrentNode);
}

//------------------------------------------------------------------------------
void qDMMLTreeView::setListenNodeModifiedEvent(qDMMLSceneModel::NodeTypes listen)
{
  Q_D(qDMMLTreeView);
  Q_ASSERT(d->SceneModel);
  d->SceneModel->setListenNodeModifiedEvent(listen);
}

//------------------------------------------------------------------------------
qDMMLSceneModel::NodeTypes qDMMLTreeView::listenNodeModifiedEvent()const
{
  Q_D(const qDMMLTreeView);
  return d->SceneModel ? d->SceneModel->listenNodeModifiedEvent() : qDMMLSceneModel::OnlyVisibleNodes;
}

// --------------------------------------------------------------------------
QStringList qDMMLTreeView::nodeTypes()const
{
  return this->sortFilterProxyModel()->nodeTypes();
}

// --------------------------------------------------------------------------
void qDMMLTreeView::setNodeTypes(const QStringList& _nodeTypes)
{
  this->sortFilterProxyModel()->setNodeTypes(_nodeTypes);
}

//--------------------------------------------------------------------------
bool qDMMLTreeView::isRenameMenuActionVisible()const
{
  Q_D(const qDMMLTreeView);
  return d->NodeMenu->actions().contains(d->RenameAction);
}

//--------------------------------------------------------------------------
void qDMMLTreeView::setRenameMenuActionVisible(bool show)
{
  Q_D(qDMMLTreeView);
  if (show)
    {
    // Prepend the action in the menu
    this->prependNodeMenuAction(d->RenameAction);
    }
  else
    {
    d->NodeMenu->removeAction(d->RenameAction);
    }
}

//--------------------------------------------------------------------------
bool qDMMLTreeView::isDeleteMenuActionVisible()const
{
  Q_D(const qDMMLTreeView);
  return d->NodeMenu->actions().contains(d->DeleteAction);
}

//--------------------------------------------------------------------------
void qDMMLTreeView::setDeleteMenuActionVisible(bool show)
{
  Q_D(qDMMLTreeView);
  if (show)
    {
    // Prepend the action in the menu
    this->prependNodeMenuAction(d->DeleteAction);
    }
  else
    {
    d->NodeMenu->removeAction(d->DeleteAction);
    }
}

//--------------------------------------------------------------------------
bool qDMMLTreeView::isEditMenuActionVisible()const
{
  Q_D(const qDMMLTreeView);
  return d->NodeMenu->actions().contains(d->EditAction);
}

//--------------------------------------------------------------------------
void qDMMLTreeView::setEditMenuActionVisible(bool show)
{
  Q_D(qDMMLTreeView);
  if (show)
    {
    // Prepend the action in the menu
    this->prependNodeMenuAction(d->EditAction);
    }
  else
    {
    d->NodeMenu->removeAction(d->EditAction);
    }
}

//--------------------------------------------------------------------------
void qDMMLTreeView::prependNodeMenuAction(QAction* action)
{
  Q_D(qDMMLTreeView);
  // Prepend the action in the menu
  d->NodeMenu->insertAction(d->NodeMenu->actions()[0], action);
}

//--------------------------------------------------------------------------
void qDMMLTreeView::appendNodeMenuAction(QAction* action)
{
  Q_D(qDMMLTreeView);
  // Append the new action to the menu
  d->NodeMenu->addAction(action);
}

//--------------------------------------------------------------------------
void qDMMLTreeView::appendSceneMenuAction(QAction* action)
{
  Q_D(qDMMLTreeView);
  // Appends the new action to the menu
  d->SceneMenu->addAction(action);
}

//--------------------------------------------------------------------------
void qDMMLTreeView::prependSceneMenuAction(QAction* action)
{
  Q_D(qDMMLTreeView);
  // Prepend the action in the menu
  QAction* beforeAction =
    d->SceneMenu->actions().size() ? d->SceneMenu->actions()[0] : 0;
  d->SceneMenu->insertAction(beforeAction, action);
}

//--------------------------------------------------------------------------
void qDMMLTreeView::removeNodeMenuAction(QAction* action)
{
  Q_D(qDMMLTreeView);
  d->NodeMenu->removeAction(action);
}

//--------------------------------------------------------------------------
void qDMMLTreeView::editCurrentNode()
{
  if (!this->currentNode())
    {
    // not sure if it's a request to have a valid node.
    Q_ASSERT(this->currentNode());
    return;
    }
  emit editNodeRequested(this->currentNode());
}

//--------------------------------------------------------------------------
void qDMMLTreeView::setShowScene(bool show)
{
  Q_D(qDMMLTreeView);
  if (d->ShowScene == show)
    {
    return;
    }
  vtkDMMLNode* oldRootNode = this->rootNode();
  d->ShowScene = show;
  this->setRootNode(oldRootNode);
}

//--------------------------------------------------------------------------
bool qDMMLTreeView::showScene()const
{
  Q_D(const qDMMLTreeView);
  return d->ShowScene;
}

//--------------------------------------------------------------------------
void qDMMLTreeView::setShowRootNode(bool show)
{
  Q_D(qDMMLTreeView);
  if (d->ShowRootNode == show)
    {
    return;
    }
  vtkDMMLNode* oldRootNode = this->rootNode();
  d->ShowRootNode = show;
  this->setRootNode(oldRootNode);
}

//--------------------------------------------------------------------------
bool qDMMLTreeView::showRootNode()const
{
  Q_D(const qDMMLTreeView);
  return d->ShowRootNode;
}

//--------------------------------------------------------------------------
void qDMMLTreeView::setRootNode(vtkDMMLNode* rootNode)
{
  Q_D(qDMMLTreeView);
  // Need to reset the filter to be able to find indexes from nodes that
  // could potentially be filtered out.
  this->sortFilterProxyModel()->setHideNodesUnaffiliatedWithNodeID(QString());
  QModelIndex treeRootIndex;
  if (rootNode == nullptr)
    {
    if (!d->ShowScene)
      {
      treeRootIndex = this->sortFilterProxyModel()->dmmlSceneIndex();
      }
    }
  else
    {
    treeRootIndex = this->sortFilterProxyModel()->indexFromDMMLNode(rootNode);
    if (d->ShowRootNode)
      {
      // Hide the siblings of the root node.
      this->sortFilterProxyModel()->setHideNodesUnaffiliatedWithNodeID(
        rootNode->GetID());
      // The parent of the root node becomes the root for QTreeView.
      treeRootIndex = treeRootIndex.parent();
      rootNode = this->sortFilterProxyModel()->dmmlNodeFromIndex(treeRootIndex);
      }
    }
  qvtkReconnect(this->rootNode(), rootNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateRootNode(vtkObject*)));
  this->setRootIndex(treeRootIndex);
}

//--------------------------------------------------------------------------
vtkDMMLNode* qDMMLTreeView::rootNode()const
{
  Q_D(const qDMMLTreeView);
  vtkDMMLNode* treeRootNode =
    this->sortFilterProxyModel()->dmmlNodeFromIndex(this->rootIndex());
  if (d->ShowRootNode &&
      this->dmmlScene() &&
      this->sortFilterProxyModel()->hideNodesUnaffiliatedWithNodeID()
        .isEmpty())
    {
    return this->dmmlScene()->GetNodeByID(
      this->sortFilterProxyModel()->hideNodesUnaffiliatedWithNodeID().toUtf8());
    }
  return treeRootNode;
}

//--------------------------------------------------------------------------
void qDMMLTreeView::updateRootNode(vtkObject* node)
{
  // Maybe the node has changed of QModelIndex, need to resync
  this->setRootNode(vtkDMMLNode::SafeDownCast(node));
}

//--------------------------------------------------------------------------
qDMMLSortFilterProxyModel* qDMMLTreeView::sortFilterProxyModel()const
{
  Q_D(const qDMMLTreeView);
  Q_ASSERT(d->SortFilterModel);
  return d->SortFilterModel;
}

//--------------------------------------------------------------------------
qDMMLSceneModel* qDMMLTreeView::sceneModel()const
{
  Q_D(const qDMMLTreeView);
  Q_ASSERT(d->SceneModel);
  return d->SceneModel;
}

//--------------------------------------------------------------------------
QSize qDMMLTreeView::minimumSizeHint()const
{
  Q_D(const qDMMLTreeView);
  return d->sizeHint();
}

//--------------------------------------------------------------------------
QSize qDMMLTreeView::sizeHint()const
{
  Q_D(const qDMMLTreeView);
  return d->sizeHint();
}

//--------------------------------------------------------------------------
void qDMMLTreeView::updateGeometries()
{
  // don't update the geometries if it's not visible on screen
  // UpdateGeometries is for tree child widgets geometry
  if (!this->isVisible())
    {
    return;
    }
  this->QTreeView::updateGeometries();
}

//--------------------------------------------------------------------------
void qDMMLTreeView::onNumberOfVisibleIndexChanged()
{
  Q_D(qDMMLTreeView);
  d->recomputeSizeHint();
}

//--------------------------------------------------------------------------
void qDMMLTreeView::setFitSizeToVisibleIndexes(bool enable)
{
  Q_D(qDMMLTreeView);
  d->FitSizeToVisibleIndexes = enable;
  d->recomputeSizeHint(true);
}

//--------------------------------------------------------------------------
bool qDMMLTreeView::fitSizeToVisibleIndexes()const
{
  Q_D(const qDMMLTreeView);
  return d->FitSizeToVisibleIndexes;
}

//--------------------------------------------------------------------------
void qDMMLTreeView::setMinSizeHint(QSize min)
{
  Q_D(qDMMLTreeView);
  d->TreeViewMinSizeHint = min;
  d->recomputeSizeHint();
}

//--------------------------------------------------------------------------
QSize qDMMLTreeView::minSizeHint()const
{
  Q_D(const qDMMLTreeView);
  return d->TreeViewMinSizeHint;
}

//------------------------------------------------------------------------------
void qDMMLTreeView::mousePressEvent(QMouseEvent* e)
{
  Q_D(qDMMLTreeView);
  this->QTreeView::mousePressEvent(e);
}

//------------------------------------------------------------------------------
void qDMMLTreeView::mouseReleaseEvent(QMouseEvent* e)
{
  if (e->button() == Qt::LeftButton)
    {
    // get the index of the current column
    QModelIndex index = this->indexAt(e->pos());
    QStyleOptionViewItem opt = this->viewOptions();
    opt.rect = this->visualRect(index);
    qobject_cast<qDMMLItemDelegate*>(this->itemDelegate())->initStyleOption(&opt,index);
    QRect decorationElement =
      this->style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, this);
    //decorationElement.translate(this->visualRect(index).topLeft());
    if (decorationElement.contains(e->pos()))
      {
      if (this->clickDecoration(index))
        {
        return;
        }
      }
    }

  this->QTreeView::mouseReleaseEvent(e);
}

//------------------------------------------------------------------------------
void qDMMLTreeView::keyPressEvent(QKeyEvent* e)
{
#ifndef _NDEBUG
  if (e->key() == Qt::Key_Exclam)
    {
    qDMMLSortFilterProxyModel::FilterType filter =
      static_cast<qDMMLSortFilterProxyModel::FilterType>(
        (this->sortFilterProxyModel()->filterType() + 1) % 3);
    qDebug() << "Filter type: " << filter;
    this->sortFilterProxyModel()->setFilterType(filter);
    }
#endif
  this->Superclass::keyPressEvent(e);
}

//------------------------------------------------------------------------------
bool qDMMLTreeView::clickDecoration(const QModelIndex& index)
{
  bool res = false;
  QModelIndex sourceIndex = this->sortFilterProxyModel()->mapToSource(index);
  if (!(sourceIndex.flags() & Qt::ItemIsEnabled))
    {
    res = false;
    }
  else if (sourceIndex.column() == this->sceneModel()->visibilityColumn())
    {
    this->toggleVisibility(index);
    res = true;
    }

  if (res)
    {
    emit decorationClicked(index);
    }
  return res;
}

//------------------------------------------------------------------------------
void qDMMLTreeView::toggleVisibility(const QModelIndex& index)
{
  vtkDMMLNode* node = this->sortFilterProxyModel()->dmmlNodeFromIndex(index);
  vtkDMMLDisplayNode* displayNode = vtkDMMLDisplayNode::SafeDownCast(node);
  vtkDMMLDisplayableNode* displayableNode = vtkDMMLDisplayableNode::SafeDownCast(node);

  if (displayableNode && displayableNode->GetDisplayNode())
    {
    displayNode = displayableNode->GetDisplayNode();
    }

  vtkDMMLSelectionNode* selectionNode = vtkDMMLSelectionNode::SafeDownCast(
    this->dmmlScene()->GetNodeByID("vtkDMMLSelectionNodeSingleton"));

  if (selectionNode && displayNode)
    {
    displayNode->SetVisibility(displayNode->GetVisibility() ? 0 : 1);
    }
}

//------------------------------------------------------------------------------
void qDMMLTreeView::saveTreeExpandState()
{
  Q_D(qDMMLTreeView);
  // Check if there is a scene loaded
  QStandardItem* sceneItem = this->sceneModel()->dmmlSceneItem();
  if (!sceneItem)
    {
    return;
    }
  // Erase previous tree expand state
  d->ExpandedNodes->RemoveAllItems();
  QModelIndex sceneIndex = this->sortFilterProxyModel()->dmmlSceneIndex();

  // First pass for the scene node
  vtkDMMLNode* sceneNode = this->sortFilterProxyModel()->dmmlNodeFromIndex(sceneIndex);
  if (this->isExpanded(sceneIndex))
    {
    if (sceneNode && this->sortFilterProxyModel()->dmmlScene()->IsNodePresent(sceneNode))
      d->ExpandedNodes->AddItem(sceneNode);
    }
  unsigned int numChildrenRows = this->sortFilterProxyModel()->rowCount(sceneIndex);
  for(unsigned int row = 0; row < numChildrenRows; ++row)
    {
    QModelIndex childIndex = this->sortFilterProxyModel()->index(row, 0, sceneIndex);
    d->saveChildrenExpandState(childIndex);
    }
}

//------------------------------------------------------------------------------
void qDMMLTreeView::loadTreeExpandState()
{
  Q_D(qDMMLTreeView);
  // Check if there is a scene loaded
  QStandardItem* sceneItem = this->sceneModel()->dmmlSceneItem();
  if (!sceneItem)
    {
    return;
    }
  // Iterate over the vtkCollection of expanded nodes
  vtkCollectionIterator* iter = d->ExpandedNodes->NewIterator();
  for(iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
    {
    vtkDMMLNode* node = vtkDMMLNode::SafeDownCast(iter->GetCurrentObject());
    // Check if the node is currently present in the scene.
    if (node && this->sortFilterProxyModel()->dmmlScene()->IsNodePresent(node))
      {
      // Expand the node
      QModelIndex nodeIndex = this->sortFilterProxyModel()->indexFromDMMLNode(node);
      this->expand(nodeIndex);
      }
    }
  // Clear the vtkCollection now
  d->ExpandedNodes->RemoveAllItems();
  iter->Delete();
}

//------------------------------------------------------------------------------
void qDMMLTreeView::renameCurrentNode()
{
  if (!this->currentNode())
    {
    Q_ASSERT(this->currentNode());
    return;
    }
  // pop up an entry box for the new name, with the old name as default
  QString oldName = this->currentNode()->GetName();

  bool ok = false;
  QString newName = QInputDialog::getText(
    this, "Rename " + oldName, "New name:",
    QLineEdit::Normal, oldName, &ok);
  if (!ok)
    {
    return;
    }
  this->currentNode()->SetName(newName.toUtf8());
  emit currentNodeRenamed(newName);
}

//------------------------------------------------------------------------------
void qDMMLTreeView::deleteCurrentNode()
{
//  Q_D(qDMMLTreeView);

  if (!this->currentNode())
    {
    Q_ASSERT(this->currentNode());
    return;
    }
  this->dmmlScene()->RemoveNode(this->currentNode());
  emit currentNodeDeleted(this->currentIndex());
}

//------------------------------------------------------------------------------
bool qDMMLTreeView::isAncestor(const QModelIndex& index, const QModelIndex& potentialAncestor)
{
  QModelIndex ancestor = index.parent();
  while(ancestor.isValid())
    {
    if (ancestor == potentialAncestor)
      {
      return true;
      }
    ancestor = ancestor.parent();
    }
  return false;
}

//------------------------------------------------------------------------------
QModelIndex qDMMLTreeView::findAncestor(const QModelIndex& index, const QModelIndexList& potentialAncestors)
{
  foreach(const QModelIndex& potentialAncestor, potentialAncestors)
    {
    if (qDMMLTreeView::isAncestor(index, potentialAncestor))
      {
      return potentialAncestor;
      }
    }
  return QModelIndex();
}

//------------------------------------------------------------------------------
QModelIndexList qDMMLTreeView::removeChildren(const QModelIndexList& indexes)
{
  QModelIndexList noAncestorIndexList;
  foreach(QModelIndex index, indexes)
    {
    if (!qDMMLTreeView::findAncestor(index, indexes).isValid())
      {
      noAncestorIndexList << index;
      }
    }
  return noAncestorIndexList;
}

//-----------------------------------------------------------------------------
void qDMMLTreeView::scrollTo(const QString& name)
{
  Q_D(qDMMLTreeView);
  d->scrollTo(name, false);
}

//-----------------------------------------------------------------------------
void qDMMLTreeView::scrollToNext()
{
  Q_D(qDMMLTreeView);
  d->scrollTo(d->LastScrollToName, true);
}

//------------------------------------------------------------------------------
void qDMMLTreeView::showEvent(QShowEvent* event)
{
  Q_D(qDMMLTreeView);
  this->Superclass::showEvent(event);
  if (d->FitSizeToVisibleIndexes &&
      !d->TreeViewSizeHint.isValid())
    {
    this->updateGeometry();
    }
}

//------------------------------------------------------------------------------
bool qDMMLTreeView::eventFilter(QObject* object, QEvent* e)
{
  Q_D(qDMMLTreeView);
  bool res = this->QTreeView::eventFilter(object, e);
  // When the horizontal scroll bar is shown/hidden, the sizehint should be
  // updated ?
  if (d->FitSizeToVisibleIndexes &&
      object == this->horizontalScrollBar() &&
      (e->type() == QEvent::Show ||
       e->type() == QEvent::Hide))
    {
    d->recomputeSizeHint();
    }
  return res;
}

//------------------------------------------------------------------------------
void qDMMLTreeView::onCustomContextMenu(const QPoint& point)
{
  Q_D(qDMMLTreeView);

  // get the index of the current column
  QModelIndex index = this->indexAt(point);

  QPoint globalPoint = this->viewport()->mapToGlobal(point);
  vtkDMMLNode* node = this->sortFilterProxyModel()->dmmlNodeFromIndex(index);
  if (node)
    {
    d->NodeMenu->exec(globalPoint);
    }
  else if (index == this->sortFilterProxyModel()->dmmlSceneIndex())
    {
    d->SceneMenu->exec(globalPoint);
    }
}
