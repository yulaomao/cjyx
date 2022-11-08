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
#include "qDMMLSortFilterHierarchyProxyModel.h"

// VTK includes
#include <vtkDMMLHierarchyNode.h>

// -----------------------------------------------------------------------------
// qDMMLSortFilterHierarchyProxyModelPrivate

// -----------------------------------------------------------------------------
class qDMMLSortFilterHierarchyProxyModelPrivate
{
public:
  qDMMLSortFilterHierarchyProxyModelPrivate();
};

// -----------------------------------------------------------------------------
qDMMLSortFilterHierarchyProxyModelPrivate::qDMMLSortFilterHierarchyProxyModelPrivate() = default;

// -----------------------------------------------------------------------------
// qDMMLSortFilterHierarchyProxyModel

//------------------------------------------------------------------------------
qDMMLSortFilterHierarchyProxyModel::qDMMLSortFilterHierarchyProxyModel(QObject *vparent)
  : qDMMLSortFilterProxyModel(vparent)
  , d_ptr(new qDMMLSortFilterHierarchyProxyModelPrivate)
{
}

//------------------------------------------------------------------------------
qDMMLSortFilterHierarchyProxyModel::~qDMMLSortFilterHierarchyProxyModel() = default;

//------------------------------------------------------------------------------
qDMMLSortFilterProxyModel::AcceptType qDMMLSortFilterHierarchyProxyModel
::filterAcceptsNode(vtkDMMLNode* node)const
{
  //Q_D(const qDMMLSortFilterHierarchyProxyModel);
  AcceptType res = this->Superclass::filterAcceptsNode(node);
  if (res == Accept || res == AcceptButPotentiallyRejectable)
    {
    return res;
    }
  vtkDMMLHierarchyNode* hNode = vtkDMMLHierarchyNode::SafeDownCast(node);
  if (!hNode)
    {
    return res;
    }
  // Don't show vtkDMMLHierarchyNode if they are tied to a vtkDMMLModelNode
  // The only vtkDMMLHierarchyNode to display are the ones who reference other
  // vtkDMMLHierarchyNode (tree parent) or empty (tree parent to be)
  if (hNode->GetAssociatedNode())
    {
    return RejectButPotentiallyAcceptable;
    }
  return res;
}
