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
  and was supported through CANARIE.

==============================================================================*/

#include "vtkDMMLFolderDisplayNode.h"

// DMML includes
#include "vtkDMMLDisplayableNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSubjectHierarchyNode.h"

// VTK includes
#include <vtkCallbackCommand.h>

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLFolderDisplayNode);

//-----------------------------------------------------------------------------
vtkDMMLFolderDisplayNode::vtkDMMLFolderDisplayNode() = default;

//-----------------------------------------------------------------------------
vtkDMMLFolderDisplayNode::~vtkDMMLFolderDisplayNode() = default;

//----------------------------------------------------------------------------
void vtkDMMLFolderDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  vtkDMMLPrintBeginMacro(os, indent);
  vtkDMMLPrintBooleanMacro(ApplyDisplayPropertiesOnBranch);
  vtkDMMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLFolderDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  this->Superclass::WriteXML(of, nIndent);

  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLBooleanMacro(applyDisplayPropertiesOnBranch, ApplyDisplayPropertiesOnBranch);
  vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLFolderDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  this->Superclass::ReadXMLAttributes(atts);

  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLBooleanMacro(applyDisplayPropertiesOnBranch, ApplyDisplayPropertiesOnBranch);
  vtkDMMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLFolderDisplayNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkDMMLFolderDisplayNode* node = vtkDMMLFolderDisplayNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  vtkDMMLCopyBeginMacro(anode);
  vtkDMMLCopyBooleanMacro(ApplyDisplayPropertiesOnBranch);
  vtkDMMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLFolderDisplayNode::SetScene(vtkDMMLScene* scene)
{
  Superclass::SetScene(scene);

  if (scene)
    {
    // Observe subject hierarchy item reparented event
    vtkDMMLSubjectHierarchyNode* shNode = vtkDMMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene);
    if (!shNode)
      {
      vtkErrorMacro("SetScene: Failed to get subject hierarchy node from current scene");
      return;
      }
    if (!shNode->HasObserver(vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemReparentedEvent, this->DMMLCallbackCommand))
      {
      shNode->AddObserver(vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemReparentedEvent, this->DMMLCallbackCommand);
      }
    }
}

//---------------------------------------------------------------------------
void vtkDMMLFolderDisplayNode::ProcessDMMLEvents(vtkObject *caller, unsigned long event, void *callData)
{
  Superclass::ProcessDMMLEvents(caller, event, callData);

  if ( event == vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemReparentedEvent
    && vtkDMMLSubjectHierarchyNode::SafeDownCast(caller) )
    {
    // No-op if this folder node does not apply display properties on its branch
    if (!this->ApplyDisplayPropertiesOnBranch)
      {
      return;
      }
    // Get item ID for subject hierarchy node events
    vtkIdType reparentedItemID = vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID;
    if (callData)
      {
      vtkIdType* itemIdPtr = reinterpret_cast<vtkIdType*>(callData);
      if (itemIdPtr)
        {
        reparentedItemID = *itemIdPtr;
        }
      }
    vtkDMMLSubjectHierarchyNode* shNode = vtkDMMLSubjectHierarchyNode::SafeDownCast(caller);
    vtkDMMLDisplayableNode* displayableReparentedNode = vtkDMMLDisplayableNode::SafeDownCast(
      shNode->GetItemDataNode(reparentedItemID) );
    // Trigger display update for reparented displayable node if it is in a folder that applies
    // display properties on its branch (only display nodes that allow overriding)
    for (int i=0; i<displayableReparentedNode->GetNumberOfDisplayNodes(); ++i)
      {
      vtkDMMLDisplayNode* currentDisplayNode = displayableReparentedNode->GetNthDisplayNode(i);
      if (currentDisplayNode && currentDisplayNode->GetFolderDisplayOverrideAllowed())
        {
        currentDisplayNode->Modified();
        }
      } // For all display nodes
    } // SubjectHierarchyItemReparentedEvent
}

//----------------------------------------------------------------------------
void vtkDMMLFolderDisplayNode::SetApplyDisplayPropertiesOnBranch(bool on)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting ApplyDisplayPropertiesOnBranch to " << on);
  if (this->ApplyDisplayPropertiesOnBranch == on)
  {
    return;
  }

  this->ApplyDisplayPropertiesOnBranch = on;
  this->Superclass::Modified();

  // Trigger display update of branch both when turned on and off
  this->ChildDisplayNodesModified();
}

//----------------------------------------------------------------------------
void vtkDMMLFolderDisplayNode::Modified()
{
  this->Superclass::Modified();

  // Always invoke modified on display nodes in branch (that allow overriding), because
  // visibility and opacity are applied even if ApplyDisplayPropertiesOnBranch is off
  this->ChildDisplayNodesModified();
}

//---------------------------------------------------------------------------
void vtkDMMLFolderDisplayNode::ChildDisplayNodesModified()
{
  if (!this->GetScene())
    {
    return;
    }
  vtkDMMLSubjectHierarchyNode* shNode = vtkDMMLSubjectHierarchyNode::GetSubjectHierarchyNode(this->GetScene());
  if (!shNode)
    {
    vtkErrorMacro("ChildDisplayNodesModified: Failed to get subject hierarchy node from scene");
    return;
    }
  vtkIdType folderItemId = shNode->GetItemByDataNode(this);
  if (!folderItemId)
    {
    return;
    }

  // Get items in branch
  std::vector<vtkIdType> childItemIDs;
  shNode->GetItemChildren(folderItemId, childItemIDs, true);

  bool batchProcessing = (childItemIDs.size() > 10);
  if (batchProcessing)
    {
    this->GetScene()->StartState(vtkDMMLScene::BatchProcessState);
    }

  std::vector<vtkIdType>::iterator childIt;
  for (childIt=childItemIDs.begin(); childIt!=childItemIDs.end(); ++childIt)
    {
    vtkDMMLDisplayableNode* childDisplayableNode = vtkDMMLDisplayableNode::SafeDownCast(
      shNode->GetItemDataNode(*childIt) );
    if (!childDisplayableNode)
      {
      continue;
      }
    // Trigger display update for display node of child nodes that allow overriding
    for (int i=0; i<childDisplayableNode->GetNumberOfDisplayNodes(); ++i)
      {
      vtkDMMLDisplayNode* currentDisplayNode = childDisplayableNode->GetNthDisplayNode(i);
      if (currentDisplayNode && currentDisplayNode->GetFolderDisplayOverrideAllowed())
        {
        currentDisplayNode->Modified();
        }
      } // For all display nodes
    }

  if (batchProcessing)
    {
    this->GetScene()->EndState(vtkDMMLScene::BatchProcessState);
    }
}

//---------------------------------------------------------------------------
vtkDMMLDisplayNode* vtkDMMLFolderDisplayNode::GetOverridingHierarchyDisplayNode(vtkDMMLDisplayableNode* node)
{
  if (!node || !node->GetScene() || node->GetScene()->IsImporting())
    {
    return nullptr;
    }
  vtkDMMLSubjectHierarchyNode* shNode = vtkDMMLSubjectHierarchyNode::GetSubjectHierarchyNode(node->GetScene());
  if (!shNode)
    {
    vtkErrorWithObjectMacro(node, "GetOverridingHierarchyDisplayNode: Failed to get subject hierarchy node from current scene");
    return nullptr;
    }
  vtkIdType nodeShId = shNode->GetItemByDataNode(node);
  if (!nodeShId)
    {
    // May happen if an AddNode event is caught before SH had the chance to add the item
    return nullptr;
    }

  // Get effective display node from hierarchy:
  // go through parents and return display node of first that applies color

  vtkIdType overridingFolderId = vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID;
  for (vtkIdType currentParentId = shNode->GetItemParent(nodeShId); currentParentId != vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID;
    currentParentId = shNode->GetItemParent(currentParentId))
    {
    vtkDMMLFolderDisplayNode* folderDisplayNode = vtkDMMLFolderDisplayNode::SafeDownCast(shNode->GetItemDataNode(currentParentId) );
    if (folderDisplayNode)
      {
      if (folderDisplayNode->GetApplyDisplayPropertiesOnBranch())
        {
        overridingFolderId = currentParentId;
        }
      }
    }
  if (overridingFolderId == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    return nullptr;
    }
  return vtkDMMLDisplayNode::SafeDownCast(shNode->GetItemDataNode(overridingFolderId));
}

//---------------------------------------------------------------------------
bool vtkDMMLFolderDisplayNode::GetHierarchyVisibility(vtkDMMLDisplayableNode* node)
{
  if (!node || node->GetHideFromEditors())
    {
    // Nodes that have HideFromEditors on do not appear in the hierarchy
    return true;
    }
  vtkDMMLSubjectHierarchyNode* shNode = vtkDMMLSubjectHierarchyNode::GetSubjectHierarchyNode(node->GetScene());
  if (!shNode)
    {
    vtkErrorWithObjectMacro(node, "GetHierarchyVisibility: Failed to get subject hierarchy node from current scene");
    return true;
    }
  vtkIdType sceneItemID = shNode->GetSceneItemID();
  vtkIdType nodeShId = shNode->GetItemByDataNode(node);
  if (!nodeShId)
    {
    return true;
    }

  // Traverse all parents
  vtkIdType parentItemID = nodeShId;
  while ( (parentItemID = shNode->GetItemParent(parentItemID)) != sceneItemID ) // The double parentheses avoids a Linux build warning
    {
    if (!parentItemID)
      {
      vtkErrorWithObjectMacro(node, "GetHierarchyVisibility: Invalid parent of subject hierarchy item");
      return false;
      }
    vtkDMMLDisplayNode* displayNode = vtkDMMLDisplayNode::SafeDownCast(shNode->GetItemDataNode(parentItemID));
    if (displayNode && displayNode->GetVisibility() == 0)
      {
      // If any of the ancestors are hidden, then the visibility of the node defined by the hierarchy is off
      return false;
      }
    }

  return true;
}

//---------------------------------------------------------------------------
double vtkDMMLFolderDisplayNode::GetHierarchyOpacity(vtkDMMLDisplayableNode* node)
{
  if (!node || node->GetHideFromEditors())
    {
    // Nodes that have HideFromEditors on do not appear in the hierarchy
    return 1.0;
    }
  vtkDMMLSubjectHierarchyNode* shNode = vtkDMMLSubjectHierarchyNode::GetSubjectHierarchyNode(node->GetScene());
  if (!shNode)
    {
    vtkErrorWithObjectMacro(node, "GetHierarchyOpacity: Failed to get subject hierarchy node from current scene");
    return 1.0;
    }
  vtkIdType sceneItemID = shNode->GetSceneItemID();
  vtkIdType nodeShId = shNode->GetItemByDataNode(node);
  if (!nodeShId)
    {
    return 1.0;
    }

  // Traverse all parents
  double opacityProduct = 1.0;
  vtkIdType parentItemID = nodeShId;
  while ( (parentItemID = shNode->GetItemParent(parentItemID)) != sceneItemID ) // The double parentheses avoids a Linux build warning
    {
    vtkDMMLDisplayNode* displayNode = vtkDMMLDisplayNode::SafeDownCast(shNode->GetItemDataNode(parentItemID));
    if (displayNode)
      {
      opacityProduct *= displayNode->GetOpacity();
      }
    }

  return opacityProduct;
}
