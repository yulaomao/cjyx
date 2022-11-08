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

// qDMML includes
#include "qDMMLSceneDisplayableModel_p.h"

// DMML includes
#include <vtkDMMLScene.h>
#include <vtkDMMLDisplayableNode.h>
#include <vtkDMMLDisplayableHierarchyNode.h>
#include <vtkDMMLDisplayNode.h>
#include <vtkDMMLSelectionNode.h>

//------------------------------------------------------------------------------
qDMMLSceneDisplayableModelPrivate
::qDMMLSceneDisplayableModelPrivate(qDMMLSceneDisplayableModel& object)
  : qDMMLSceneHierarchyModelPrivate(object)
{
  this->OpacityColumn = -1;
  this->ColorColumn = -1;
}

//------------------------------------------------------------------------------
void qDMMLSceneDisplayableModelPrivate::init()
{
  Q_Q(qDMMLSceneDisplayableModel);
  q->setVisibilityColumn(q->nameColumn());
}

//------------------------------------------------------------------------------
vtkDMMLHierarchyNode* qDMMLSceneDisplayableModelPrivate::CreateHierarchyNode()const
{
  return vtkDMMLDisplayableHierarchyNode::New();
}

//------------------------------------------------------------------------------
vtkDMMLDisplayNode* qDMMLSceneDisplayableModelPrivate
::displayNode(vtkDMMLNode* node)const
{
  if (vtkDMMLDisplayNode::SafeDownCast(node))
    {
    return vtkDMMLDisplayNode::SafeDownCast(node);
    }

  vtkDMMLDisplayableNode* displayableNode = vtkDMMLDisplayableNode::SafeDownCast(node);
  if (displayableNode)
    {
    return displayableNode->GetDisplayNode();
    }

  vtkDMMLDisplayableHierarchyNode* displayableHierarchyNode
      = vtkDMMLDisplayableHierarchyNode::SafeDownCast(node);
  if (displayableHierarchyNode)
    {
    return displayableHierarchyNode->GetDisplayNode();
    }
  return nullptr;
}

//----------------------------------------------------------------------------

//------------------------------------------------------------------------------
qDMMLSceneDisplayableModel::qDMMLSceneDisplayableModel(QObject *vparent)
  :Superclass(new qDMMLSceneDisplayableModelPrivate(*this), vparent)
{
  Q_D(qDMMLSceneDisplayableModel);
  d->init();
}

//------------------------------------------------------------------------------
qDMMLSceneDisplayableModel::qDMMLSceneDisplayableModel(
  qDMMLSceneDisplayableModelPrivate* pimpl, QObject *vparent)
  :Superclass(pimpl, vparent)
{
}

//------------------------------------------------------------------------------
qDMMLSceneDisplayableModel::~qDMMLSceneDisplayableModel() = default;

//------------------------------------------------------------------------------
vtkDMMLNode* qDMMLSceneDisplayableModel::parentNode(vtkDMMLNode* node)const
{
  return vtkDMMLDisplayableHierarchyNode::SafeDownCast(
    this->Superclass::parentNode(node));
}

//------------------------------------------------------------------------------
bool qDMMLSceneDisplayableModel::canBeAChild(vtkDMMLNode* node)const
{
  if (!node)
    {
    return false;
    }
  return node->IsA("vtkDMMLDisplayableNode") ||
         node->IsA("vtkDMMLDisplayableHierarchyNode");
}

//------------------------------------------------------------------------------
bool qDMMLSceneDisplayableModel::canBeAParent(vtkDMMLNode* node)const
{
  return this->canBeAChild(node);
}

//------------------------------------------------------------------------------
void qDMMLSceneDisplayableModel::observeNode(vtkDMMLNode* node)
{
  this->Superclass::observeNode(node);
  if (node->IsA("vtkDMMLDisplayableNode"))
    {
    qvtkConnect(node, vtkDMMLDisplayableNode::DisplayModifiedEvent,
                this, SLOT(onDMMLNodeModified(vtkObject*)));
    }
}

//------------------------------------------------------------------------------
QFlags<Qt::ItemFlag> qDMMLSceneDisplayableModel::nodeFlags(vtkDMMLNode* node, int column)const
{
  Q_D(const qDMMLSceneDisplayableModel);
  QFlags<Qt::ItemFlag> flags = this->Superclass::nodeFlags(node, column);
  vtkDMMLNode *displayNode = d->displayNode(node);
  if (column == this->visibilityColumn() &&
      displayNode != nullptr)
    {
    flags |= Qt::ItemIsEditable;
    }
  if (column == this->colorColumn() &&
      displayNode != nullptr)
    {
    flags |= Qt::ItemIsEditable;
    }
  if (column == this->opacityColumn() &&
      displayNode != nullptr)
    {
    flags |= Qt::ItemIsEditable;
    }
  return flags;
}

//------------------------------------------------------------------------------
void qDMMLSceneDisplayableModel
::updateItemDataFromNode(QStandardItem* item, vtkDMMLNode* node, int column)
{
  Q_D(qDMMLSceneDisplayableModel);
  vtkDMMLDisplayNode* displayNode = d->displayNode(node);
  if (column == this->colorColumn())
    {
    if (displayNode)
      {
      double* rgbF = displayNode->GetColor();
      QColor color = QColor::fromRgbF(rgbF[0], rgbF[1], rgbF[2],
                                      displayNode->GetOpacity());
      item->setData(color, Qt::DecorationRole);
      item->setToolTip("Color");
      }
    }
  if (column == this->opacityColumn())
    {
    if (displayNode)
      {
      QString displayedOpacity
        = QString::number(displayNode->GetOpacity(), 'f', 2);
      item->setData(displayedOpacity, Qt::DisplayRole);
      item->setToolTip("Opacity");
      }
    }
  this->Superclass::updateItemDataFromNode(item, node, column);
}

//------------------------------------------------------------------------------
void qDMMLSceneDisplayableModel
::updateNodeFromItemData(vtkDMMLNode* node, QStandardItem* item)
{
  Q_D(qDMMLSceneDisplayableModel);
  if (item->column() == this->colorColumn())
    {
    QColor color = item->data(Qt::DecorationRole).value<QColor>();
    // Invalid color can happen when the item hasn't been initialized yet
    if (color.isValid())
      {
      vtkDMMLDisplayNode* displayNode = d->displayNode(node);
      if (displayNode)
        {
        int wasModifying = displayNode->StartModify();
        // QColor looses precision, don't change color/opacity if not "really"
        // changed.
        QColor oldColor = QColor::fromRgbF(displayNode->GetColor()[0],
                                           displayNode->GetColor()[1],
                                           displayNode->GetColor()[2],
                                           displayNode->GetOpacity());
        if (oldColor != color)
          {
          displayNode->SetColor(color.redF(), color.greenF(), color.blueF());
          displayNode->SetOpacity(color.alphaF());
          }
        displayNode->EndModify(wasModifying);
        }
      }
    }
  if (item->column() == this->opacityColumn())
    {
    QString displayedOpacity = item->data(Qt::EditRole).toString();
    if (!displayedOpacity.isEmpty())
      {
      vtkDMMLDisplayNode* displayNode = d->displayNode(node);
      // Invalid color can happen when the item hasn't been initialized yet
      if (displayNode)
        {
        QString currentOpacity = QString::number( displayNode->GetOpacity(), 'f', 2);
        if (displayedOpacity != currentOpacity)
          {
          displayNode->SetOpacity(displayedOpacity.toDouble());
          }
        }
      }
    }
  return this->Superclass::updateNodeFromItemData(node, item);
}

//------------------------------------------------------------------------------
int qDMMLSceneDisplayableModel::colorColumn()const
{
  Q_D(const qDMMLSceneDisplayableModel);
  return d->ColorColumn;
}

//------------------------------------------------------------------------------
void qDMMLSceneDisplayableModel::setColorColumn(int column)
{
  Q_D(qDMMLSceneDisplayableModel);
  d->ColorColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qDMMLSceneDisplayableModel::opacityColumn()const
{
  Q_D(const qDMMLSceneDisplayableModel);
  return d->OpacityColumn;
}

//------------------------------------------------------------------------------
void qDMMLSceneDisplayableModel::setOpacityColumn(int column)
{
  Q_D(qDMMLSceneDisplayableModel);
  d->OpacityColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qDMMLSceneDisplayableModel::maxColumnId()const
{
  Q_D(const qDMMLSceneDisplayableModel);
  int maxId = this->Superclass::maxColumnId();
  maxId = qMax(maxId, d->ColorColumn);
  maxId = qMax(maxId, d->OpacityColumn);
  return maxId;
}
