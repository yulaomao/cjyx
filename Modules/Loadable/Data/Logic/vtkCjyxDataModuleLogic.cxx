/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

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

// Data Logic includes
#include "vtkCjyxDataModuleLogic.h"

// DMML includes
#include <vtkDMMLNode.h>
#include <vtkDMMLDisplayableNode.h>
#include <vtkDMMLDisplayNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLStorageNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkCjyxDataModuleLogic);

//----------------------------------------------------------------------------
vtkCjyxDataModuleLogic::vtkCjyxDataModuleLogic()
{
  this->SceneChanged = false;
  this->AutoRemoveDisplayAndStorageNodes = true;
}

//----------------------------------------------------------------------------
vtkCjyxDataModuleLogic::~vtkCjyxDataModuleLogic() = default;

//----------------------------------------------------------------------------
void vtkCjyxDataModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "AutoRemoveDisplayAndStorageNode: " <<
    (AutoRemoveDisplayAndStorageNodes ? "On" : "Off") << "\n";

 }

//---------------------------------------------------------------------------
void vtkCjyxDataModuleLogic::SetDMMLSceneInternal(vtkDMMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkDMMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkDMMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkDMMLScene::EndBatchProcessEvent);
  events->InsertNextValue(vtkDMMLScene::EndCloseEvent);
  this->SetAndObserveDMMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkCjyxDataModuleLogic::RegisterNodes()
{
  assert(this->GetDMMLScene() != nullptr);
}

//---------------------------------------------------------------------------
void vtkCjyxDataModuleLogic::UpdateFromDMMLScene()
{
  this->SceneChangedOn();
}


//----------------------------------------------------------------------------
void vtkCjyxDataModuleLogic::OnDMMLSceneNodeRemoved(vtkDMMLNode* node)
{
  vtkDMMLDisplayableNode* displayableNode = vtkDMMLDisplayableNode::SafeDownCast(node);
  if (!displayableNode || this->GetDMMLScene()->IsClosing() || !this->AutoRemoveDisplayAndStorageNodes)
    {
    return;
    }

  // Collect a list of storage and display nodes that are only
  // referenced by the node to be removed.
  std::vector< vtkWeakPointer<vtkDMMLNode> > nodesToRemove;

  /// we can't get the display node directly as it might be 0 because the
  /// displayable node has no longer access to the scene
  std::vector<vtkDMMLNode *> referencingNodes;
  for (int i = 0; i < displayableNode->GetNumberOfDisplayNodes(); ++i)
    {
    vtkDMMLNode *dnode = this->GetDMMLScene()->GetNodeByID(
      displayableNode->GetNthDisplayNodeID(i));

    // make sure no other nodes reference this display node
    this->GetDMMLScene()->GetReferencingNodes(dnode, referencingNodes);

    if (referencingNodes.size() == 0 ||
        (referencingNodes.size() == 1 && referencingNodes[0] == node) )
      {
      nodesToRemove.emplace_back(dnode);
      }
    }
  for (int i = 0; i < displayableNode->GetNumberOfStorageNodes(); ++i)
    {
    vtkDMMLNode *snode = this->GetDMMLScene()->GetNodeByID(
      displayableNode->GetNthStorageNodeID(i));

    // make sure no other nodes reference this storage node
    this->GetDMMLScene()->GetReferencingNodes(snode, referencingNodes);

    if (referencingNodes.size() == 0 ||
        (referencingNodes.size() == 1 && referencingNodes[0] == node) )
      {
      nodesToRemove.emplace_back(snode);
      }
    }

  // Now remove the collected nodes. Batch process is only used if many nodes will be removed
  // because entering/exiting batch processing is a very expensive operation (the display flickers,
  // lots of things are recomputed), so it should be only done if we save time by skipping many small updates.
  int toRemove = nodesToRemove.size();
  bool useBatchMode = toRemove > 10; // Switch to batch mode if more than 10 nodes to remove
  int progress = 0;
  if (useBatchMode)
    {
    this->GetDMMLScene()->StartState(vtkDMMLScene::BatchProcessState, toRemove);
    }
  std::vector< vtkWeakPointer<vtkDMMLNode> >::const_iterator nodeIterator;
  nodeIterator = nodesToRemove.begin();
  while (nodeIterator != nodesToRemove.end())
    {
    if (nodeIterator->GetPointer())
       {
      this->GetDMMLScene()->RemoveNode(*nodeIterator);
      if (useBatchMode)
        {
        this->GetDMMLScene()->ProgressState(vtkDMMLScene::BatchProcessState, ++progress);
        }
      }
    ++nodeIterator;
    }
  if (useBatchMode)
    {
    this->GetDMMLScene()->EndState(vtkDMMLScene::BatchProcessState);
    }
}
