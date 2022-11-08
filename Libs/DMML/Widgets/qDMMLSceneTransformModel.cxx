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
#include "qDMMLSceneTransformModel.h"
#include "qDMMLSceneModel_p.h"

// DMML includes
#include <vtkDMMLTransformNode.h>

//------------------------------------------------------------------------------
vtkDMMLNode* qDMMLSceneTransformModel::parentNode(vtkDMMLNode* node)const
{
  // DMML Transformable nodes
  vtkDMMLTransformableNode* transformableNode =
    vtkDMMLTransformableNode::SafeDownCast(node);
  if (transformableNode)
    {
    return transformableNode->GetParentTransformNode();
    }
  return nullptr;
}

/*
//------------------------------------------------------------------------------
int qDMMLSceneTransformModel::nodeIndex(vtkDMMLNode* node)const
{
  const char* nodeId = node ? node->GetID() : 0;
  if (nodeId == 0)
    {
    return -1;
    }
  const char* nId = 0;
  int index = -1;
  vtkDMMLNode* parent = qDMMLSceneTransformModel::parentNode(node);
  vtkCollection* nodes = node->GetScene()->GetNodes();
  vtkDMMLNode* n = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);
       (n = (vtkDMMLNode*)nodes->GetNextItemAsObject(it)) ;)
    {
    // note: parent can be nullptr, it means that the scene is the parent
    if (parent == qDMMLSceneTransformModel::parentNode(n))
      {
      ++index;
      nId = n->GetID();
      if (nId && !strcmp(nodeId, nId))
        {
        return index;
        }
      }
    }
  return -1;
}
*/

//------------------------------------------------------------------------------
bool qDMMLSceneTransformModel::canBeAChild(vtkDMMLNode* node)const
{
  return node ? node->IsA("vtkDMMLTransformableNode") : false;
}

//------------------------------------------------------------------------------
bool qDMMLSceneTransformModel::canBeAParent(vtkDMMLNode* node)const
{
  return node ? node->IsA("vtkDMMLTransformNode") : false;
}

//------------------------------------------------------------------------------
bool qDMMLSceneTransformModel::reparent(vtkDMMLNode* node, vtkDMMLNode* newParent)
{
  Q_ASSERT(node);
  if (!node || qDMMLSceneTransformModel::parentNode(node) == newParent)
    {
    return false;
    }
  Q_ASSERT(newParent != node);
  // DMML Transformable Nodes
  vtkDMMLTransformableNode* transformableNode =
    vtkDMMLTransformableNode::SafeDownCast(node);
  vtkDMMLTransformNode* transformNode =
    vtkDMMLTransformNode::SafeDownCast(newParent);
  if (transformableNode)
    {
    if (transformNode && !transformNode->IsTransformToWorldLinear() && !transformableNode->CanApplyNonLinearTransforms())
      {
      return false;
      }
    transformableNode->SetAndObserveTransformNodeID( newParent ? newParent->GetID() : nullptr );
    return true;
    }
  return false;
}

//------------------------------------------------------------------------------
class qDMMLSceneTransformModelPrivate: public qDMMLSceneModelPrivate
{
protected:
  Q_DECLARE_PUBLIC(qDMMLSceneTransformModel);
public:
  qDMMLSceneTransformModelPrivate(qDMMLSceneTransformModel& object);

};

//------------------------------------------------------------------------------
qDMMLSceneTransformModelPrivate
::qDMMLSceneTransformModelPrivate(qDMMLSceneTransformModel& object)
  : qDMMLSceneModelPrivate(object)
{

}

//----------------------------------------------------------------------------

//------------------------------------------------------------------------------
qDMMLSceneTransformModel::qDMMLSceneTransformModel(QObject *vparent)
  :qDMMLSceneModel(new qDMMLSceneTransformModelPrivate(*this), vparent)
{
}

//------------------------------------------------------------------------------
qDMMLSceneTransformModel::~qDMMLSceneTransformModel() = default;

//------------------------------------------------------------------------------
Qt::DropActions qDMMLSceneTransformModel::supportedDropActions()const
{
  return Qt::MoveAction;
}
