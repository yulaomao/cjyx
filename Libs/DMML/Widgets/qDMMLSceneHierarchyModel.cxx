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

// qDMML includes
#include "qDMMLSceneHierarchyModel_p.h"

// DMML includes
#include <vtkDMMLDisplayableHierarchyNode.h>
#include <vtkDMMLHierarchyNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>

//------------------------------------------------------------------------------
qDMMLSceneHierarchyModelPrivate
::qDMMLSceneHierarchyModelPrivate(qDMMLSceneHierarchyModel& object)
  : Superclass(object)
  , ExpandColumn(-1)
{
}

//------------------------------------------------------------------------------
void qDMMLSceneHierarchyModelPrivate::init()
{
}

//------------------------------------------------------------------------------
vtkDMMLHierarchyNode* qDMMLSceneHierarchyModelPrivate::CreateHierarchyNode()const
{
  return vtkDMMLHierarchyNode::New();
}

//----------------------------------------------------------------------------

//------------------------------------------------------------------------------
qDMMLSceneHierarchyModel::qDMMLSceneHierarchyModel(QObject *vparent)
  :Superclass(new qDMMLSceneHierarchyModelPrivate(*this), vparent)
{
}

//------------------------------------------------------------------------------
qDMMLSceneHierarchyModel::qDMMLSceneHierarchyModel(
  qDMMLSceneHierarchyModelPrivate* pimpl, QObject *parent)
  :Superclass(pimpl, parent)
{
}

//------------------------------------------------------------------------------
qDMMLSceneHierarchyModel::~qDMMLSceneHierarchyModel() = default;

//------------------------------------------------------------------------------
int qDMMLSceneHierarchyModel::expandColumn()const
{
  Q_D(const qDMMLSceneHierarchyModel);
  return d->ExpandColumn;
}

//------------------------------------------------------------------------------
void qDMMLSceneHierarchyModel::setExpandColumn(int column)
{
  Q_D(qDMMLSceneHierarchyModel);
  d->ExpandColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qDMMLSceneHierarchyModel::maxColumnId()const
{
  Q_D(const qDMMLSceneHierarchyModel);
  int maxId = this->Superclass::maxColumnId();
  maxId = qMax(maxId, d->ExpandColumn);
  return maxId;
}

/*

//------------------------------------------------------------------------------
vtkDMMLNode* qDMMLSceneHierarchyModel::childNode(vtkDMMLNode* node, int childIndex)
{
  // shortcut the following search if we are sure that the node has no children
  if (childIndex < 0 || node == 0 || !qDMMLSceneHierarchyModel::canBeAParent(node))
    {
    return 0;
    }
  // DMML Transformable nodes
  QString nodeId = QString(node->GetID());
  vtkCollection* nodes = node->GetScene()->GetNodes();
  vtkDMMLNode* n = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);
       (n = (vtkDMMLNode*)nodes->GetNextItemAsObject(it)) ;)
    {
    vtkDMMLNode* parent = qDMMLSceneHierarchyModel::parentNode(n);
    if (parent == 0)
      {
      continue;
      }
    // compare IDs not pointers
    if (nodeId == parent->GetID())
      {
      if (childIndex-- == 0)
        {
        return n;
        }
      }
    }
  return 0;
}
*/
//------------------------------------------------------------------------------
vtkDMMLNode* qDMMLSceneHierarchyModel::parentNode(vtkDMMLNode* node)const
{
  vtkDMMLHierarchyNode* hierarchyNode = vtkDMMLHierarchyNode::SafeDownCast(node);
  if (!hierarchyNode)
    {
    hierarchyNode = vtkDMMLHierarchyNode::GetAssociatedHierarchyNode(node->GetScene(), node->GetID());
    }
  return hierarchyNode ? hierarchyNode->GetParentNode() : nullptr;
}

//------------------------------------------------------------------------------
int qDMMLSceneHierarchyModel::nodeIndex(vtkDMMLNode* node)const
{
  Q_D(const qDMMLSceneHierarchyModel);
  if (!d->DMMLScene)
    {
    return -1;
    }

  const char* nodeId = node ? node->GetID() : nullptr;
  if (nodeId == nullptr)
    {
    return -1;
    }

  // is there a hierarchy node associated with this node?
  vtkDMMLHierarchyNode *assocHierarchyNode = vtkDMMLHierarchyNode::GetAssociatedHierarchyNode(d->DMMLScene, node->GetID());
  if (assocHierarchyNode)
    {
    int assocHierarchyNodeIndex = this->nodeIndex(assocHierarchyNode);
    return assocHierarchyNodeIndex + 1;
    }

  const char* nId = nullptr;
  vtkDMMLNode* parent = this->parentNode(node);
  int index = 0;
  // if it's part of a hierarchy, use the GetIndexInParent call
  if (parent)
    {
    vtkDMMLHierarchyNode *hnode = vtkDMMLHierarchyNode::SafeDownCast(node);
    if (hnode)
      {
      vtkDMMLHierarchyNode* parentHierarchy = vtkDMMLHierarchyNode::SafeDownCast(parent);
      if (parentHierarchy == nullptr)
        {
        // sometimes the parent is not a hierarchy node but the associated node
        // of a hierarchy node (if the hierarchy node is filtered out from the view).
        // We need to find that hierarchy node.
        parentHierarchy = vtkDMMLHierarchyNode::GetAssociatedHierarchyNode(
          d->DMMLScene, parent->GetID());
        }
      const int childrenCount = parentHierarchy->GetNumberOfChildrenNodes();
      for ( int i = 0; i < childrenCount ; ++i)
        {
        vtkDMMLHierarchyNode* child = parentHierarchy->GetNthChildNode(i);
        if (child == hnode)
          {
          return index;
          }
        ++index;
        // the associated node of a hierarchynode is displayed after the hierarchynode
        if (child->GetAssociatedNode())
          {
          ++index;
          }
        }
      }
    }

  // otherwise, iterate through the scene
  vtkCollection* nodes = d->DMMLScene->GetNodes();
  vtkDMMLNode* n = nullptr;
  vtkCollectionSimpleIterator it;

  for (nodes->InitTraversal(it);
       (n = (vtkDMMLNode*)nodes->GetNextItemAsObject(it)) ;)
    {
    // note: parent can be nullptr, it means that the scene is the parent
    vtkDMMLHierarchyNode *currentHierarchyNode =
      vtkDMMLHierarchyNode::GetAssociatedHierarchyNode(d->DMMLScene, n->GetID());
    vtkDMMLNode *currentParentNode =
        (currentHierarchyNode ? currentHierarchyNode->GetParentNode() : nullptr);
    if (parent == currentParentNode)
      {
      nId = n->GetID();
      if (nId && !strcmp(nodeId, nId))
        {
//      std::cout << "nodeIndex:  no parent for node " << node->GetID() << " index = " << index << std::endl;
        return index;
        }
      if (!currentHierarchyNode)
        {
        ++index;
        }
      vtkDMMLHierarchyNode* hierarchy = vtkDMMLHierarchyNode::SafeDownCast(n);
      if (hierarchy && hierarchy->GetAssociatedNode())
        {
        // if the current node is a hierarchynode associated with the node,
        // then it should have been caught at the beginning of the function
        Q_ASSERT(strcmp(nodeId, hierarchy->GetAssociatedNodeID()));
        ++index;
        }
      }
    }
  return -1;
}

//------------------------------------------------------------------------------
bool qDMMLSceneHierarchyModel::canBeAChild(vtkDMMLNode* node)const
{
  return node != nullptr;
}

//------------------------------------------------------------------------------
bool qDMMLSceneHierarchyModel::canBeAParent(vtkDMMLNode* node)const
{
  return node && node->IsA("vtkDMMLHierarchyNode");
}

//------------------------------------------------------------------------------
bool qDMMLSceneHierarchyModel::reparent(vtkDMMLNode* node, vtkDMMLNode* newParent)
{
  Q_D(qDMMLSceneHierarchyModel);
  if (!node)
    {
    return false;
    }

  vtkDMMLNode *dmmlNode = vtkDMMLNode::SafeDownCast(node);
  vtkDMMLHierarchyNode *hierarchyNode = vtkDMMLHierarchyNode::SafeDownCast(node);
  vtkDMMLNode *dmmlParentNode = nullptr;
  vtkDMMLHierarchyNode *hierarchyParentNode = nullptr;
  if (newParent)
    {
    dmmlParentNode = vtkDMMLNode::SafeDownCast(newParent);
    hierarchyParentNode = vtkDMMLHierarchyNode::SafeDownCast(newParent);
    }

  // we can be reparenting a hierarchy node to another hierarchy node, or a
  // dmml node (under it's hierarchy)
  if (hierarchyNode)
    {
    if (!hierarchyParentNode &&
        dmmlParentNode &&
        dmmlParentNode->GetScene() &&
        dmmlParentNode->GetID())
      {
      // get it's hierarchy node
      hierarchyParentNode = vtkDMMLHierarchyNode::GetAssociatedHierarchyNode(dmmlParentNode->GetScene(), dmmlParentNode->GetID());
      }
    // else use the safe down cast of the parent node
    if (hierarchyParentNode &&
        hierarchyParentNode->GetID())
      {
      hierarchyNode->SetParentNodeID(hierarchyParentNode->GetID());
      }
    else
      {
      // reparenting to top with null parent id
      hierarchyNode->SetParentNodeID(nullptr);
      }
    return true;
    }
  // we can be reparenting a dmml node to another dmml node or a
  // hierarchy node.
  else if (dmmlNode)
    {
    if (dmmlNode->GetScene() &&
        dmmlNode->GetID())
      {
      hierarchyNode = vtkDMMLHierarchyNode::GetAssociatedHierarchyNode(dmmlNode->GetScene(), dmmlNode->GetID());
      // Create hierarchy node if does not exist or different type than the new parent
      // (hierarchy node types need to match within a hierarchy to avoid "mixing" of hierarchies of different type)
      // Note: This may need to be revised if mixing across classes is to be allowed (e.g. displayable and model,
      //   in which case base classes might be allowed as well)
      if (!hierarchyNode ||
          (newParent && strcmp(newParent->GetClassName(), hierarchyNode->GetClassName())))
        {
        vtkDMMLHierarchyNode* newHierarchyNode = d->CreateHierarchyNode();
        newHierarchyNode->SetName(this->dmmlScene()->GetUniqueNameByString(
          newHierarchyNode->GetNodeTagName()));
        newHierarchyNode->SetHideFromEditors(1);
        //newHierarchyNode->AllowMultipleChildrenOff();
        newHierarchyNode->SetAssociatedNodeID(dmmlNode->GetID());
        dmmlNode->GetScene()->AddNode(newHierarchyNode);
        qWarning() << "qDMMLSceneHierarchyModel::reparent: Added a new hierarchy node " << newHierarchyNode->GetID();
        newHierarchyNode->Delete();
        // try again
        hierarchyNode = vtkDMMLHierarchyNode::GetAssociatedHierarchyNode(dmmlNode->GetScene(), dmmlNode->GetID());
        }
      }
    Q_ASSERT_X(hierarchyNode != dmmlParentNode, "qDMMLSceneHierarchyNode::reparent", "Shouldn't be possible, maybe the droppable flag wasn't set");
    if (!hierarchyParentNode && dmmlParentNode && dmmlParentNode->GetScene() &&  dmmlParentNode->GetID())
      {
      hierarchyParentNode = vtkDMMLHierarchyNode::GetAssociatedHierarchyNode(dmmlParentNode->GetScene(), dmmlParentNode->GetID());
      }
    // else it uses the safe down cast to a hierarchy node of the newParent
    if (hierarchyNode)
      {
      if (hierarchyParentNode && hierarchyParentNode->GetID())
        {
        hierarchyNode->SetParentNodeID(hierarchyParentNode->GetID());
        }
      else
        {
        // reparenting to top with null parent id
        hierarchyNode->SetParentNodeID(nullptr);
        }
      return true;
      }
    }
  return false;
}

//------------------------------------------------------------------------------
Qt::DropActions qDMMLSceneHierarchyModel::supportedDropActions()const
{
  return Qt::MoveAction;
}

//------------------------------------------------------------------------------
QFlags<Qt::ItemFlag> qDMMLSceneHierarchyModel::nodeFlags(vtkDMMLNode* node, int column)const
{
  QFlags<Qt::ItemFlag> flags = this->Superclass::nodeFlags(node, column);
  if (!this->canBeAParent(node))
    {
    return flags;
    }
  vtkDMMLHierarchyNode* hierarchyNode = vtkDMMLHierarchyNode::SafeDownCast(node);
  if (!hierarchyNode)
    {
    return flags;
    }
  if ((hierarchyNode->GetAssociatedNode() ||
       (!hierarchyNode->GetAllowMultipleChildren() &&
        hierarchyNode->GetNumberOfChildrenNodes() > 0)))
    {
    flags &= ~Qt::ItemIsDropEnabled;
    }
  if (column == this->expandColumn())
    {
    flags = flags | Qt::ItemIsUserCheckable;
    }
  return flags;
}

//------------------------------------------------------------------------------
void qDMMLSceneHierarchyModel::updateItemDataFromNode(
  QStandardItem* item, vtkDMMLNode* node, int column)
{
  this->Superclass::updateItemDataFromNode(item, node, column);
  vtkDMMLHierarchyNode* hierarchyNode = vtkDMMLHierarchyNode::SafeDownCast(node);
  if (hierarchyNode)
    {
    if (column == this->expandColumn())
      {
      vtkDMMLDisplayableHierarchyNode *hnode =
        vtkDMMLDisplayableHierarchyNode::SafeDownCast(node);
      if (hnode)
        {
        item->setCheckState(hnode->GetExpanded() ? Qt::Unchecked : Qt::Checked);
        }
      item->setToolTip(tr("Checked: Force color to children"));
      }
    }
}

//------------------------------------------------------------------------------
void qDMMLSceneHierarchyModel::updateNodeFromItemData(vtkDMMLNode* node, QStandardItem* item)
{
  this->Superclass::updateNodeFromItemData(node, item);
  if (item->column() == this->expandColumn())
    {
    vtkDMMLDisplayableHierarchyNode *hnode =
      vtkDMMLDisplayableHierarchyNode::SafeDownCast(node);
    if (hnode)
      {
      hnode->SetExpanded(item->checkState() == Qt::Unchecked ? 1 : 0);
      }
    }
}

//------------------------------------------------------------------------------
void qDMMLSceneHierarchyModel::observeNode(vtkDMMLNode* node)
{
  this->Superclass::observeNode(node);
  qvtkConnect(node, vtkDMMLNode::HierarchyModifiedEvent,
              this, SLOT(onDMMLNodeModified(vtkObject*)));
}

