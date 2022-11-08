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
#include <QStringList>

// Annotations includes
#include "qDMMLSceneAnnotationModel_p.h"
#include "vtkDMMLAnnotationHierarchyNode.h"
#include "vtkDMMLAnnotationSnapshotNode.h"
#include "vtkCjyxAnnotationModuleLogic.h"

// DMML includes
#include <vtkDMMLScene.h>

//------------------------------------------------------------------------------
qDMMLSceneAnnotationModelPrivate
::qDMMLSceneAnnotationModelPrivate(qDMMLSceneAnnotationModel& object)
  : qDMMLSceneDisplayableModelPrivate(object)
{
  this->AnnotationsAreParent = false;
  this->LockColumn = -1;
  this->EditColumn = -1;
  this->ValueColumn = -1;
  this->TextColumn = -1;
}

//------------------------------------------------------------------------------
void qDMMLSceneAnnotationModelPrivate::init()
{
  Q_Q(qDMMLSceneAnnotationModel);
  this->Superclass::init();

  q->setCheckableColumn(0);
  q->setVisibilityColumn(1);
  q->setLockColumn(2);
  q->setEditColumn(3);
  q->setValueColumn(4);
  q->setNameColumn(5);
  q->setTextColumn(6);

  q->setHorizontalHeaderLabels(
    QStringList() << "" << "Vis" << "Lock" << "Edit" << "Value" << "Name" << "Description");
}

//------------------------------------------------------------------------------
qDMMLSceneAnnotationModel::qDMMLSceneAnnotationModel(QObject *vparent)
  : Superclass(new qDMMLSceneAnnotationModelPrivate(*this), vparent)
{
  Q_D(qDMMLSceneAnnotationModel);
  d->init();
}

//------------------------------------------------------------------------------
qDMMLSceneAnnotationModel::qDMMLSceneAnnotationModel(
  qDMMLSceneAnnotationModelPrivate* pimpl, QObject *vparent)
  : Superclass(pimpl, vparent)
{
  // init() is called by derived class.
}

//------------------------------------------------------------------------------
qDMMLSceneAnnotationModel::~qDMMLSceneAnnotationModel() = default;

//------------------------------------------------------------------------------
bool qDMMLSceneAnnotationModel::areAnnotationsParent()const
{
  Q_D(const qDMMLSceneAnnotationModel);
  return d->AnnotationsAreParent;
}

//------------------------------------------------------------------------------
void qDMMLSceneAnnotationModel::setAnnotationsAreParent(bool parentable)
{
  Q_D(qDMMLSceneAnnotationModel);
  d->AnnotationsAreParent = parentable;
}

//------------------------------------------------------------------------------
void qDMMLSceneAnnotationModel::updateNodeFromItemData(vtkDMMLNode* node, QStandardItem* item)
{
  vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(node);
  vtkDMMLAnnotationHierarchyNode* hierarchyNode = vtkDMMLAnnotationHierarchyNode::SafeDownCast(node);
  vtkDMMLAnnotationSnapshotNode* snapshotNode = vtkDMMLAnnotationSnapshotNode::SafeDownCast(node);

  int oldChecked = node->GetSelected();

  this->qDMMLSceneDisplayableModel::updateNodeFromItemData(node, item);

  // TODO move to logic ?
  if (hierarchyNode && oldChecked != hierarchyNode->GetSelected())
    {
    int newChecked = hierarchyNode->GetSelected();
    vtkCollection* children = vtkCollection::New();
    hierarchyNode->GetChildrenDisplayableNodes(children);

    children->InitTraversal();
    for (int i=0; i<children->GetNumberOfItems(); ++i)
      {
      vtkDMMLAnnotationNode* childNode = vtkDMMLAnnotationNode::SafeDownCast(children->GetItemAsObject(i));
      if (childNode)
        {
        // this is a valid annotation child node
        // set all children to have same selected as the hierarchy
        childNode->SetSelected(newChecked);
        }
      } // for loop
    }// if hierarchyNode

  if (item->column() == this->textColumn())
    {
    if (annotationNode)
      {
      // if we have an annotation node, the text can be changed by editing the textcolumn
      annotationNode->SetText(0,item->text().toUtf8(),0,1);
      if (annotationNode->IsA("vtkDMMLAnnotationFiducialNode"))
        {
        // also set the name
        //annotationNode->SetName(item->text().toUtf8());
        }
      }
    else if (hierarchyNode)
      {
      // if we have a hierarchy node, the description can be changed by editing the textcolumn
      hierarchyNode->SetDescription(item->text().toUtf8());
      }
    else if (snapshotNode)
      {
      // if we have a snapshot node, the name can be changed by editing the textcolumn
      snapshotNode->SetName(item->text().toUtf8());
      }
    }
}

//------------------------------------------------------------------------------
void qDMMLSceneAnnotationModel::updateItemDataFromNode(QStandardItem* item, vtkDMMLNode* node, int column)
{
  Q_D(qDMMLSceneAnnotationModel);
  if (!node)
    {
    return;
    }
  this->Superclass::updateItemDataFromNode(item, node, column);
  vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(node);
  vtkDMMLAnnotationHierarchyNode *hnode = vtkDMMLAnnotationHierarchyNode::SafeDownCast(node);
  if (column == this->visibilityColumn())
    {
    // the visibility icon
    if (annotationNode)
      {
      if (annotationNode->GetDisplayVisibility())
        {
        item->setData(QPixmap(":/Icons/Small/CjyxVisible.png"),Qt::DecorationRole);
        }
      else
        {
        item->setData(QPixmap(":/Icons/Small/CjyxInvisible.png"),Qt::DecorationRole);
        }
      }
    else if (hnode)
      {
      // don't show anything, handle it in property dialogue
      }
    }
  if (column == this->lockColumn())
    {
    // the lock/unlock icon
    if (annotationNode)
      {
      if (annotationNode->GetLocked())
        {
        item->setData(QPixmap(":/Icons/Small/CjyxLock.png"),Qt::DecorationRole);
        }
      else
        {
        item->setData(QPixmap(":/Icons/Small/CjyxUnlock.png"),Qt::DecorationRole);
        }
      }
    else if (hnode)
      {
      // don't show anything, handle it in property dialogue
      }
    }
  if (column == this->editColumn())
    {
    // the annotation type icon
    item->setData(
      QPixmap(d->AnnotationLogic->GetAnnotationIcon(node)), Qt::DecorationRole);
    }
  if (column == this->valueColumn())
    {
    if (annotationNode)
      {
      // the annotation measurement
      item->setText(
        QString(d->AnnotationLogic->GetAnnotationMeasurement(
                  annotationNode->GetID(),false)));
      }
    else if (hnode)
      {
      item->setText(QString(""));
      }
    }
  if (column == this->textColumn())
    {
    if (annotationNode)
      {
      // the annotation text
      item->setText(
        QString(d->AnnotationLogic->GetAnnotationText(
                  annotationNode->GetID())));
      }
    else if (hnode)
      {
      item->setText(QString(node->GetDescription()));
      }
    }
}

//------------------------------------------------------------------------------
QFlags<Qt::ItemFlag> qDMMLSceneAnnotationModel::nodeFlags(vtkDMMLNode* node, int column)const
{
  QFlags<Qt::ItemFlag> flags = this->qDMMLSceneDisplayableModel::nodeFlags(node, column);
  // remove the ItemIsEditable flag from any possible item (typically at column 0)
  flags = flags & ~Qt::ItemIsEditable;
  // and set it to the text and names columns
  if (column == this->nameColumn() ||
      column == this->textColumn())
    {
    flags = flags | Qt::ItemIsEditable;
    }
  // if this is an annotation with a hierarchy node that it's a 1:1 node, don't allow
  // dropping
  vtkDMMLDisplayableHierarchyNode *displayableHierarchyNode =
    vtkDMMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(
      node->GetScene(), node->GetID());
  if (displayableHierarchyNode  &&
      !displayableHierarchyNode->GetAllowMultipleChildren())
    {
    flags = flags & ~Qt::ItemIsDropEnabled;
    }
  return flags;
}

//------------------------------------------------------------------------------
bool qDMMLSceneAnnotationModel::canBeAParent(vtkDMMLNode* node)const
{
  bool res = this->Superclass::canBeAParent(node) ||
    (node && node->IsA("vtkDMMLAnnotationNode") && this->areAnnotationsParent());
  return res;
}

//------------------------------------------------------------------------------
vtkDMMLNode* qDMMLSceneAnnotationModel::parentNode(vtkDMMLNode* node)const
{
  if (node == nullptr)
    {
    return nullptr;
    }

  vtkDMMLDisplayableHierarchyNode* displayableHierarchyNode =
    vtkDMMLDisplayableHierarchyNode::SafeDownCast(node);
  if (displayableHierarchyNode == nullptr)
    {
    vtkDMMLDisplayableNode *displayableNode = vtkDMMLDisplayableNode::SafeDownCast(node);
    if (displayableNode != nullptr)
      {
      // get the displayable hierarchy node associated with this displayable node
      displayableHierarchyNode =
        vtkDMMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(
          displayableNode->GetScene(), displayableNode->GetID());
      if (displayableHierarchyNode &&
          !displayableHierarchyNode->GetHideFromEditors())
        {
        return displayableHierarchyNode;
        }
      }
    }
  if (displayableHierarchyNode != nullptr)
    {
    // this is a hidden hierarchy node, so we do not want to display it
    // instead, we will return the parent of the hidden hierarchy node
    // to be used as the parent for the displayableNode
    vtkDMMLDisplayableHierarchyNode* parent =
      vtkDMMLDisplayableHierarchyNode::SafeDownCast(
        displayableHierarchyNode->GetParentNode());
    // return it's parent
    if (this->areAnnotationsParent() &&
        parent && parent->GetHideFromEditors() &&
        parent->GetDisplayableNode())
      {
      return parent->GetDisplayableNode();
      }
    return parent;
    }
  return nullptr;
}

//------------------------------------------------------------------------------
vtkDMMLNode* qDMMLSceneAnnotationModel
::activeHierarchyNode(vtkDMMLNode* dmmlNode)const
{
  if (!dmmlNode)
    {
    return nullptr;
    }

  if(dmmlNode->IsA("vtkDMMLAnnotationHierarchyNode"))
    {
    return dmmlNode;
    }
  // If the node isn't a hierarchy node, reset the
  // active hierarchy to the parent hierarchy of this node (going via the
  // hierarchy node associated with this node)
  vtkDMMLHierarchyNode *hnode =
    vtkDMMLAnnotationHierarchyNode::GetAssociatedHierarchyNode(
      this->dmmlScene(), dmmlNode->GetID());
  if (hnode)
    {
    if (this->areAnnotationsParent())
      {
      return hnode;
      }
    else
      {
      return hnode->GetParentNode();
      }
    }
  return nullptr;
}

//------------------------------------------------------------------------------
int qDMMLSceneAnnotationModel::lockColumn()const
{
  Q_D(const qDMMLSceneAnnotationModel);
  return d->LockColumn;
}

//------------------------------------------------------------------------------
void qDMMLSceneAnnotationModel::setLockColumn(int column)
{
  Q_D(qDMMLSceneAnnotationModel);
  d->LockColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qDMMLSceneAnnotationModel::editColumn()const
{
  Q_D(const qDMMLSceneAnnotationModel);
  return d->EditColumn;
}

//------------------------------------------------------------------------------
void qDMMLSceneAnnotationModel::setEditColumn(int column)
{
  Q_D(qDMMLSceneAnnotationModel);
  d->EditColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qDMMLSceneAnnotationModel::valueColumn()const
{
  Q_D(const qDMMLSceneAnnotationModel);
  return d->ValueColumn;
}

//------------------------------------------------------------------------------
void qDMMLSceneAnnotationModel::setValueColumn(int column)
{
  Q_D(qDMMLSceneAnnotationModel);
  d->ValueColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qDMMLSceneAnnotationModel::textColumn()const
{
  Q_D(const qDMMLSceneAnnotationModel);
  return d->TextColumn;
}

//------------------------------------------------------------------------------
void qDMMLSceneAnnotationModel::setTextColumn(int column)
{
  Q_D(qDMMLSceneAnnotationModel);
  d->TextColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qDMMLSceneAnnotationModel::maxColumnId()const
{
  Q_D(const qDMMLSceneAnnotationModel);
  int maxId = this->Superclass::maxColumnId();
  maxId = qMax(maxId, d->LockColumn);
  maxId = qMax(maxId, d->EditColumn);
  maxId = qMax(maxId, d->ValueColumn);
  maxId = qMax(maxId, d->TextColumn);
  return maxId;
}

//-----------------------------------------------------------------------------
/// Set and observe the logic
//-----------------------------------------------------------------------------
void qDMMLSceneAnnotationModel::setLogic(vtkCjyxAnnotationModuleLogic* logic)
{
  Q_D(qDMMLSceneAnnotationModel);
  if (!logic)
    {
    return;
    }

  d->AnnotationLogic = logic;
}
