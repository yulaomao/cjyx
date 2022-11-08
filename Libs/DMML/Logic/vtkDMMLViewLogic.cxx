/*==============================================================================

  Copyright (c) Kapteyn Astronomical Institute
  University of Groningen, Groningen, Netherlands. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Davide Punzo, Kapteyn Astronomical Institute,
  and was supported through the European Research Council grant nr. 291531.

==============================================================================*/

// DMMLLogic includes
#include "vtkDMMLViewLogic.h"
#include "vtkDMMLSliceLayerLogic.h"

// DMML includes
#include <vtkEventBroker.h>
#include <vtkDMMLCameraNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkVersion.h>

// VTKAddon includes
#include <vtkAddonMathUtilities.h>

// STD includes
#include <algorithm>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLViewLogic);

//----------------------------------------------------------------------------
vtkDMMLViewLogic::vtkDMMLViewLogic()
{
  this->UpdatingDMMLNodes = false;
  this->ViewNode = nullptr;
  this->CameraNode = nullptr;
}

//----------------------------------------------------------------------------
vtkDMMLViewLogic::~vtkDMMLViewLogic()
{
  this->Name.clear();
  if (this->CameraNode)
    {
    vtkSetAndObserveDMMLNodeMacro(this->CameraNode, 0);
    this->CameraNode = nullptr;
    }

  if (this->ViewNode)
    {
    vtkSetAndObserveDMMLNodeMacro(this->ViewNode, 0);
    this->ViewNode = nullptr;
  }
}

//----------------------------------------------------------------------------
void vtkDMMLViewLogic::SetDMMLSceneInternal(vtkDMMLScene* newScene)
{
  // List of events the slice logics should listen
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkDMMLScene::EndBatchProcessEvent);
  events->InsertNextValue(vtkDMMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkDMMLScene::NodeRemovedEvent);

  this->SetAndObserveDMMLSceneEventsInternal(newScene, events.GetPointer());

  this->UpdateDMMLNodes();
}

//----------------------------------------------------------------------------
void vtkDMMLViewLogic::OnDMMLSceneNodeAdded(vtkDMMLNode* node)
{
  if (node->IsA("vtkDMMLViewNode") || node->IsA("vtkDMMLCameraNode"))
    {
    this->UpdateDMMLNodes();
    }
}

//----------------------------------------------------------------------------
void vtkDMMLViewLogic::OnDMMLSceneNodeRemoved(vtkDMMLNode* node)
{
  if (node->IsA("vtkDMMLViewNode") || node->IsA("vtkDMMLCameraNode"))
    {
    this->UpdateDMMLNodes();
    }
}

//----------------------------------------------------------------------------
void vtkDMMLViewLogic::UpdateFromDMMLScene()
{
  this->UpdateDMMLNodes();
}

//----------------------------------------------------------------------------
void vtkDMMLViewLogic::UpdateDMMLNodes()
{
  if (this->GetDMMLScene() && this->GetDMMLScene()->IsBatchProcessing())
    {
    return;
    }
  if (this->UpdatingDMMLNodes)
    {
    return;
    }
  this->UpdatingDMMLNodes = true;

  vtkDMMLViewNode* updatedViewNode = vtkDMMLViewLogic::GetViewNode(this->GetDMMLScene(), this->GetName());
  this->SetViewNode(updatedViewNode);

  vtkDMMLCameraNode* updatedCameraNode = vtkDMMLViewLogic::GetCameraNode(this->GetDMMLScene(), this->GetName());
  this->SetCameraNode(updatedCameraNode);

  this->UpdatingDMMLNodes = false;
}

//----------------------------------------------------------------------------
vtkDMMLViewNode* vtkDMMLViewLogic::GetViewNode(vtkDMMLScene* scene, const char* layoutName)
{
  if (!scene || !layoutName)
    {
    return nullptr;
    }

  vtkSmartPointer<vtkCollection> viewNodes = vtkSmartPointer<vtkCollection>::Take
      (scene->GetNodesByClass("vtkDMMLViewNode"));
  for(int viewNodeIndex = 0; viewNodeIndex < viewNodes->GetNumberOfItems(); ++viewNodeIndex)
    {
    vtkDMMLViewNode* viewNode =
        vtkDMMLViewNode::SafeDownCast(viewNodes->GetItemAsObject(viewNodeIndex));
    if (viewNode &&
        viewNode->GetLayoutName() &&
        !strcmp(viewNode->GetLayoutName(), layoutName))
      {
      return viewNode;
      }
    }
  return nullptr;
}

//----------------------------------------------------------------------------
vtkDMMLCameraNode* vtkDMMLViewLogic::GetCameraNode(vtkDMMLScene* scene, const char* layoutName)
{
  if (!scene || !layoutName || strlen(layoutName)==0)
    {
    return nullptr;
    }
  vtkDMMLCameraNode* cameraNode = vtkDMMLCameraNode::SafeDownCast(
    scene->GetSingletonNode(layoutName, "vtkDMMLCameraNode"));
  return cameraNode;
}

//----------------------------------------------------------------------------
void vtkDMMLViewLogic::SetCameraNode(vtkDMMLCameraNode* newCameraNode)
{
  if (this->CameraNode == newCameraNode)
    {
    return;
    }

  // Observe the camera node for general properties.
  vtkSetAndObserveDMMLNodeMacro(this->CameraNode, newCameraNode);

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkDMMLViewLogic::SetViewNode(vtkDMMLViewNode* newViewNode)
{
  if (this->ViewNode == newViewNode)
    {
    return;
    }

  // Observe the view node for general properties.
  vtkSetAndObserveDMMLNodeMacro(this->ViewNode, newViewNode);
}

//----------------------------------------------------------------------------
void vtkDMMLViewLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  vtkIndent nextIndent;
  nextIndent = indent.GetNextIndent();

  os << indent << "CjyxViewLogic:             " << this->GetClassName() << "\n";

  if (this->CameraNode)
    {
    os << indent << "CameraNode: ";
    os << (this->CameraNode->GetID() ? this->CameraNode->GetID() : "(0 ID)") << "\n";
    this->CameraNode->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << "CameraNode: (none)\n";
    }

  if (this->ViewNode)
    {
    os << indent << "ViewNode: ";
    os << (this->ViewNode->GetID() ? this->ViewNode->GetID() : "(0 ID)") << "\n";
    this->ViewNode->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << "ViewNode: (none)\n";
    }
}

//----------------------------------------------------------------------------
void vtkDMMLViewLogic::StartViewNodeInteraction(unsigned int parameters)
{
  if (!this->ViewNode)
    {
    return;
    }

  // Cache the flags on what parameters are going to be modified. Need
  // to this this outside the conditional on LinkedControl
  this->ViewNode->SetInteractionFlags(parameters);

  // If we have linked controls, then we want to broadcast changes
  if (this->ViewNode->GetLinkedControl())
    {
    // Activate interaction
    this->ViewNode->InteractingOn();
    }
}

//----------------------------------------------------------------------------
void vtkDMMLViewLogic::EndViewNodeInteraction()
{
  if (!this->ViewNode)
    {
    return;
    }

  if (this->ViewNode->GetLinkedControl())
    {
    this->ViewNode->InteractingOff();
    this->ViewNode->SetInteractionFlags(0);
  }
}

//----------------------------------------------------------------------------
vtkDMMLViewNode* vtkDMMLViewLogic::AddViewNode(const char* layoutName)
{
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("vtkDMMLViewLogic::AddViewNode failed: scene is not set");
    return nullptr;
    }
  vtkSmartPointer<vtkDMMLViewNode> node = vtkSmartPointer<vtkDMMLViewNode>::Take(
    vtkDMMLViewNode::SafeDownCast(this->GetDMMLScene()->CreateNodeByClass("vtkDMMLViewNode")));
  node->SetLayoutName(layoutName);
  this->GetDMMLScene()->AddNode(node);
  this->SetViewNode(node);
  return node;
}

//----------------------------------------------------------------------------
void vtkDMMLViewLogic::StartCameraNodeInteraction(unsigned int parameters)
{
  if (!this->ViewNode || !this->CameraNode)
    {
    return;
    }

  // Cache the flags on what parameters are going to be modified. Need
  // to this this outside the conditional on LinkedControl
  this->CameraNode->SetInteractionFlags(parameters);

  // If we have hot linked controls, then we want to broadcast changes
  if (this->ViewNode->GetLinkedControl())
    {
    // Activate interaction
    this->CameraNode->InteractingOn();
    }
}

//----------------------------------------------------------------------------
void vtkDMMLViewLogic::EndCameraNodeInteraction()
{
  if (!this->ViewNode || !this->CameraNode)
    {
    return;
    }

  if (this->ViewNode->GetLinkedControl())
    {
    this->CameraNode->InteractingOff();
    this->CameraNode->SetInteractionFlags(0);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLViewLogic::SetName(const char* name)
{
  std::string newName = (name ? name : "");
  if (newName == this->Name)
  {
    return;
  }
  this->Name = newName;
  this->UpdateDMMLNodes();
  this->Modified();
}

//----------------------------------------------------------------------------
const char* vtkDMMLViewLogic::GetName() const
{
  return this->Name.c_str();
}
