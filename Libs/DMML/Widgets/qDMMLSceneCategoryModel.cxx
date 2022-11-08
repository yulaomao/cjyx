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

// CTK includes
#include <ctkUtils.h>

// qDMML includes
#include "qDMMLSceneCategoryModel.h"
#include "qDMMLSceneModel_p.h"

// DMML includes
#include <vtkDMMLNode.h>

// VTK includes

//------------------------------------------------------------------------------
class qDMMLSceneCategoryModelPrivate: public qDMMLSceneModelPrivate
{
protected:
  Q_DECLARE_PUBLIC(qDMMLSceneCategoryModel);
public:
  qDMMLSceneCategoryModelPrivate(qDMMLSceneCategoryModel& object);

};

//------------------------------------------------------------------------------
qDMMLSceneCategoryModelPrivate
::qDMMLSceneCategoryModelPrivate(qDMMLSceneCategoryModel& object)
  : qDMMLSceneModelPrivate(object)
{

}

//----------------------------------------------------------------------------

//------------------------------------------------------------------------------
qDMMLSceneCategoryModel::qDMMLSceneCategoryModel(QObject *vparent)
  :qDMMLSceneModel(new qDMMLSceneCategoryModelPrivate(*this), vparent)
{
}

//------------------------------------------------------------------------------
qDMMLSceneCategoryModel::~qDMMLSceneCategoryModel() = default;

//------------------------------------------------------------------------------
QStandardItem* qDMMLSceneCategoryModel::itemFromCategory(const QString& category)const
{
  if (category.isEmpty())
    {
    return this->dmmlSceneItem();
    }
  // doesn't search category items recursively.
  // options to optimize, categories are continuous and are the first children
  // of the dmmlSceneItem
  int rowCount = this->dmmlSceneItem()->rowCount();
  for (int i = 0; i < rowCount; ++i)
    {
    QStandardItem* child = this->dmmlSceneItem()->child(i,0);
    if (child &&
        child->data(qDMMLSceneModel::UIDRole).toString() == "category" &&
        child->text() == category)
      {
      return child;
      }
    }
  return this->dmmlSceneItem();
}

//------------------------------------------------------------------------------
int qDMMLSceneCategoryModel::categoryCount()const
{
  return this->match(ctk::modelChildIndex(const_cast<qDMMLSceneCategoryModel*>(this), this->dmmlSceneIndex(), 0, 0),
                     qDMMLSceneModel::UIDRole,
                     QString("category"),
                     -1,
                     Qt::MatchExactly)
    .size();
}

//------------------------------------------------------------------------------
QStandardItem* qDMMLSceneCategoryModel::insertCategory(const QString& category, int row)
{
  Q_ASSERT(!category.isEmpty());

  QList<QStandardItem*> categoryItems;
  categoryItems << new QStandardItem;
  this->updateItemFromCategory(categoryItems[0], category);
  categoryItems << new QStandardItem;
  categoryItems[1]->setFlags(Qt::NoItemFlags);

  this->dmmlSceneItem()->insertRow(row, categoryItems);
  Q_ASSERT(this->dmmlSceneItem()->columnCount() == 2);
  return categoryItems[0];
}

//------------------------------------------------------------------------------
QStandardItem* qDMMLSceneCategoryModel::insertNode(vtkDMMLNode* node)
{
  QStandardItem* nodeItem = this->itemFromNode(node);
  if (nodeItem)
    {
    return nodeItem;
    }
  // WARNING: works only if the nodes are in the scene in the correct order:
  // parents are before children
  QString category = QString(node->GetAttribute("Category"));
  QStandardItem* parentItem = this->itemFromCategory(category);
  Q_ASSERT(parentItem);
  if (!category.isEmpty() && parentItem == this->dmmlSceneItem())
    {
    parentItem = this->insertCategory(category,
                                      this->preItems(parentItem).count()
                                      + this->categoryCount());
    }
  //int min = this->preItems(parentItem).count();
  int max = parentItem->rowCount() - this->postItems(parentItem).count();
  nodeItem = this->insertNode(node, parentItem, max);
  return nodeItem;
}

//------------------------------------------------------------------------------
bool qDMMLSceneCategoryModel::isANode(const QStandardItem * item)const
{
  return this->qDMMLSceneModel::isANode(item)
    && item->data(qDMMLSceneModel::UIDRole).toString() != "category";
}

//------------------------------------------------------------------------------
void qDMMLSceneCategoryModel::updateItemFromCategory(QStandardItem* item, const QString& category)
{
  item->setData(QString("category"), qDMMLSceneModel::UIDRole);
  item->setFlags(Qt::ItemIsEnabled);
  item->setText(category);
}

//------------------------------------------------------------------------------
void qDMMLSceneCategoryModel::updateItemFromNode(QStandardItem* item, vtkDMMLNode* node, int column)
{
  this->qDMMLSceneModel::updateItemFromNode(item, node, column);
  QStandardItem* parentItem = item->parent();
  QString category = QString(node->GetAttribute("Category"));
  QStandardItem* newParentItem = this->itemFromCategory(category);
  // if the item has no parent, then it means it hasn't been put into the scene yet.
  // and it will do it automatically.
  if (parentItem != nullptr && (parentItem != newParentItem))
    {
    QList<QStandardItem*> children = parentItem->takeRow(item->row());
    //int min = this->preItems(newParentItem).count();
    int max = newParentItem->rowCount() - this->postItems(newParentItem).count();
    int pos = max;
    newParentItem->insertRow(pos, children);
    }
}

//------------------------------------------------------------------------------
void qDMMLSceneCategoryModel::updateNodeFromItem(vtkDMMLNode* node, QStandardItem* item)
{
  this->qDMMLSceneModel::updateNodeFromItem(node, item);
  Q_ASSERT(node != this->dmmlNodeFromItem(item->parent()));

  // Don't do the following if the row is not complete (reparenting an
  // incomplete row might lead to errors). updateNodeFromItem is typically
  // called for every item changed, so it should be
  QStandardItem* parentItem = item->parent();
  for (int i = 0; i < parentItem->columnCount(); ++i)
    {
    if (parentItem->child(item->row(), i) == nullptr)
      {
      return;
      }
    }
  QString category =
    (parentItem != this->dmmlSceneItem()) ? parentItem->text() : QString();
  // If the attribute has never been set, don't set it with an empty string.
  if (!(node->GetAttribute("Category") == nullptr &&
        category.isEmpty()))
    {
    node->SetAttribute("Category", category.toUtf8());
    }
}
