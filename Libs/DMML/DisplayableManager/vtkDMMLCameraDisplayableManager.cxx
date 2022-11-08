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

  Based on Cjyx/Base/GUI/vtkCjyxViewerWidget.cxx,
  this file was developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// DMMLDisplayableManager includes
#include "vtkDMMLCameraDisplayableManager.h"
#include "vtkDMMLThreeDViewInteractorStyle.h"

// DMML includes
#include <vtkEventBroker.h>
#include <vtkDMMLInteractionEventData.h>
#include <vtkDMMLCameraWidget.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLViewNode.h>
#include <vtkDMMLViewLogic.h>

// VTK includes
#include <vtkCamera.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>

// STD includes
#include <cassert>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLCameraDisplayableManager );

//---------------------------------------------------------------------------
class vtkDMMLCameraDisplayableManager::vtkInternal
{
public:
  vtkInternal();
  ~vtkInternal();

  vtkDMMLCameraNode* CameraNode;
  int UpdatingCameraNode;
  vtkSmartPointer<vtkDMMLCameraWidget> CameraWidget;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkDMMLCameraDisplayableManager::vtkInternal::vtkInternal()
{
  this->CameraNode = nullptr;
  this->UpdatingCameraNode = 0;
  this->CameraWidget = vtkSmartPointer<vtkDMMLCameraWidget>::New();
}

//---------------------------------------------------------------------------
vtkDMMLCameraDisplayableManager::vtkInternal::~vtkInternal()
{
  this->CameraWidget->SetRenderer(nullptr);
  this->CameraWidget->SetCameraNode(nullptr);
}

//---------------------------------------------------------------------------
// vtkDMMLCameraDisplayableManager methods

//---------------------------------------------------------------------------
vtkDMMLCameraDisplayableManager::vtkDMMLCameraDisplayableManager()
{
  this->Internal = new vtkInternal;
}

//---------------------------------------------------------------------------
vtkDMMLCameraDisplayableManager::~vtkDMMLCameraDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkDMMLCameraDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "CameraNode: " << this->Internal->CameraNode << "\n";
}

//---------------------------------------------------------------------------
void vtkDMMLCameraDisplayableManager::Create()
{
  this->UpdateCameraNode();
}

//---------------------------------------------------------------------------
void vtkDMMLCameraDisplayableManager::OnDMMLSceneEndClose()
{
  this->UpdateCameraNode();
}

//---------------------------------------------------------------------------
void vtkDMMLCameraDisplayableManager::OnDMMLSceneStartImport()
{

}

//---------------------------------------------------------------------------
void vtkDMMLCameraDisplayableManager::OnDMMLSceneEndImport()
{
  if (!this->GetDMMLScene())
    {
    this->SetAndObserveCameraNode(nullptr);
    return;
    }
  if (this->GetDMMLViewNode())
    {
    this->UpdateCameraNode();
    }
}

//---------------------------------------------------------------------------
void vtkDMMLCameraDisplayableManager::OnDMMLSceneEndRestore()
{
  if (this->GetDMMLViewNode())
    {
    this->UpdateCameraNode();
    }

}

//---------------------------------------------------------------------------
void vtkDMMLCameraDisplayableManager::OnDMMLSceneNodeAdded(vtkDMMLNode* node)
{
  if (this->GetDMMLScene()->IsBatchProcessing() ||
      !node->IsA("vtkDMMLCameraNode"))
    {
    return;
    }
  // maybe the camera node is a good match for the observed view node?
  this->UpdateCameraNode();
}

//---------------------------------------------------------------------------
void vtkDMMLCameraDisplayableManager::OnDMMLSceneNodeRemoved(vtkDMMLNode* node)
{
  if (this->GetDMMLScene()->IsBatchProcessing() ||
      !node->IsA("vtkDMMLCameraNode"))
    {
    return;
    }
  if (node == this->Internal->CameraNode)
    {
    // we need to find another camera node for the observed view node
    this->UpdateCameraNode();
    }
}

//---------------------------------------------------------------------------
void vtkDMMLCameraDisplayableManager::ProcessDMMLNodesEvents(vtkObject *caller,
                                                        unsigned long event,
                                                        void *callData)
{
  switch(event)
    {
    // Maybe something external removed the view/camera link, make sure the
    // view observes a camera node, create a new camera node if needed
    case vtkDMMLCameraNode::LayoutNameModifiedEvent:
      assert(vtkDMMLCameraNode::SafeDownCast(caller));
      this->UpdateCameraNode();
      vtkDebugMacro("ProcessingDMML: got a camera node modified event");
      break;
    case vtkDMMLCameraNode::ResetCameraClippingEvent:
      assert(vtkDMMLCameraNode::SafeDownCast(caller));
      vtkDebugMacro("ProcessingDMML: got a camera node modified event");
      if (this->GetRenderer())
        {
        this->GetRenderer()->ResetCameraClippingRange();
        this->GetRenderer()->UpdateLightsGeometryToFollowCamera();
        }
      else if (this->GetCameraNode() && this->GetCameraNode()->GetCamera())
        {
        vtkCamera* camera = this->GetCameraNode()->GetCamera();
        camera->SetClippingRange(0.1, camera->GetDistance()*2);
        }
      break;
    default:
      this->Superclass::ProcessDMMLNodesEvents(caller, event, callData);
      break;
    }
}

//---------------------------------------------------------------------------
void vtkDMMLCameraDisplayableManager::OnDMMLNodeModified(
#ifndef NDEBUG
 vtkDMMLNode* node)
#else
  vtkDMMLNode* vtkNotUsed(node))
#endif
{
  this->RequestRender();
}

//---------------------------------------------------------------------------
vtkDMMLCameraNode* vtkDMMLCameraDisplayableManager::GetCameraNode()
{
  return this->Internal->CameraNode;
}

//---------------------------------------------------------------------------
void vtkDMMLCameraDisplayableManager::SetAndObserveCameraNode(vtkDMMLCameraNode * newCameraNode)
{
  // If a camera node already points to me. Do I already have it?
  if (newCameraNode == this->Internal->CameraNode)
    {
    if (newCameraNode)
      {
      // I'm already pointing to it
      vtkDebugMacro("UpdateCamera: CameraNode [" << newCameraNode->GetID()
                    << "] is already pointing to my ViewNode and I'm observing it - "
                    << "Internal->CameraNode [" << this->Internal->CameraNode->GetID() << "]");
      }
    return;
    }
  // Do not associate the old camera node anymore with this view
  if (this->Internal->CameraNode && this->Internal->CameraNode->GetLayoutName()
    && this->GetDMMLViewNode() && this->GetDMMLViewNode()->GetLayoutName()
    && strcmp(this->Internal->CameraNode->GetLayoutName(), this->GetDMMLViewNode()->GetLayoutName())==0)
    {
    this->Internal->CameraNode->SetLayoutName(nullptr);
    }
  // Associate the new camera node
  if (newCameraNode)
    {
    newCameraNode->SetLayoutName(this->GetDMMLViewNode()->GetLayoutName());
    }
  vtkNew<vtkIntArray> cameraNodeEvents;
  cameraNodeEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  cameraNodeEvents->InsertNextValue(vtkDMMLCameraNode::LayoutNameModifiedEvent);
  cameraNodeEvents->InsertNextValue(vtkDMMLCameraNode::ResetCameraClippingEvent);

  vtkSetAndObserveDMMLNodeEventsMacro(
    this->Internal->CameraNode, newCameraNode, cameraNodeEvents.GetPointer());

  this->SetCameraToRenderer();
  this->SetCameraToInteractor();
  this->InvokeEvent(vtkDMMLCameraDisplayableManager::ActiveCameraChangedEvent, newCameraNode);
  vtkDMMLViewNode *viewNode = this->GetDMMLViewNode();
  if (viewNode)
    {
    viewNode->Modified(); // update vtkCamera from view node (perspective/parallel, etc)
    }

  this->Internal->CameraWidget->SetRenderer(this->GetRenderer());
  this->Internal->CameraWidget->SetCameraNode(this->Internal->CameraNode);
};

//---------------------------------------------------------------------------
void vtkDMMLCameraDisplayableManager::RemoveDMMLObservers()
{
//  this->RemoveCameraObservers();
  this->SetAndObserveCameraNode(nullptr);

  this->Superclass::RemoveDMMLObservers();
}

//---------------------------------------------------------------------------
void vtkDMMLCameraDisplayableManager::UpdateCameraNode()
{
  if (this->Internal->UpdatingCameraNode)
    {
    return;
    }
  if (this->GetDMMLScene() == nullptr)
    {
    vtkErrorMacro("UpdateCameraNode: DisplayableManager does NOT have a scene set, "
                  "can't find CameraNodes");
    return;
    }

  vtkDMMLViewNode* viewNode = this->GetDMMLViewNode();
  if (!viewNode)
    {
    vtkErrorMacro("UpdateCameraNode: DisplayableManager does NOT have a valid view node");
    return;
    }

  // find ViewNode in the scene
  vtkSmartPointer<vtkDMMLCameraNode> updatedCameraNode = vtkDMMLViewLogic::GetCameraNode(this->GetDMMLScene(), viewNode->GetLayoutName());
  if (updatedCameraNode != nullptr && this->Internal->CameraNode == updatedCameraNode)
    {
    // no change
    return;
    }

  this->Internal->UpdatingCameraNode = 1;

  if (!updatedCameraNode && viewNode->GetLayoutName() && strlen(viewNode->GetLayoutName()) > 0)
    {
    // Use CreateNodeByClass instead of New to make the new node based on default node stored in the scene
    updatedCameraNode = vtkSmartPointer<vtkDMMLCameraNode>::Take(
      vtkDMMLCameraNode::SafeDownCast(this->GetDMMLScene()->CreateNodeByClass("vtkDMMLCameraNode")));
    updatedCameraNode->SetName(this->GetDMMLScene()->GetUniqueNameByString(updatedCameraNode->GetNodeTagName()));
    updatedCameraNode->SetDescription("Default Scene Camera"); // indicates that this is an automatically created default camera
    updatedCameraNode->SetLayoutName(viewNode->GetLayoutName());
    this->GetDMMLScene()->AddNode(updatedCameraNode);
    }
  this->SetAndObserveCameraNode(updatedCameraNode);

  this->Internal->UpdatingCameraNode = 0;
}

//---------------------------------------------------------------------------
void vtkDMMLCameraDisplayableManager::AdditionalInitializeStep()
{
  assert(this->GetRenderer());
  this->SetCameraToRenderer();
}

//---------------------------------------------------------------------------
void vtkDMMLCameraDisplayableManager::SetCameraToRenderer()
{
  if (!this->GetRenderer())
    {
    return;
    }
  vtkCamera *camera = this->Internal->CameraNode ? this->Internal->CameraNode->GetCamera() : nullptr;
  this->GetRenderer()->SetActiveCamera(camera);
  if (camera)
    {
    // Do not call if there is no camera otherwise it will create a new one without a CameraNode
    this->GetRenderer()->ResetCameraClippingRange();
    // Default to ParallelProjection Off
    //camera->ParallelProjectionOff();
    }
}

//---------------------------------------------------------------------------
void vtkDMMLCameraDisplayableManager::SetCameraToInteractor()
{
  if (!this->GetInteractor())
    {
    return;
    }
  vtkInteractorObserver *iobs = this->GetInteractor()->GetInteractorStyle();
  vtkDMMLThreeDViewInteractorStyle *istyle =
    vtkDMMLThreeDViewInteractorStyle::SafeDownCast(iobs);
  if (istyle)
    {
    istyle->SetCameraNode(this->Internal->CameraNode);
    }
}

//---------------------------------------------------------------------------
bool vtkDMMLCameraDisplayableManager::CanProcessInteractionEvent(vtkDMMLInteractionEventData* eventData, double &closestDistance2)
{
  // The CameraWidget does not have representation, so it cannot use the usual representation->GetInteractionNode()
  // method to get the interactor, therefore we make sure here that the view node is passed to it.
  if (!eventData->GetViewNode())
    {
    eventData->SetViewNode(this->GetDMMLViewNode());
    }
  return this->Internal->CameraWidget->CanProcessInteractionEvent(eventData, closestDistance2);
}

//---------------------------------------------------------------------------
bool vtkDMMLCameraDisplayableManager::ProcessInteractionEvent(vtkDMMLInteractionEventData* eventData)
{
  // The CameraWidget does not have representation, so it cannot use the usual representation->GetInteractionNode()
  // method to get the interactor, therefore we make sure here that the view node is passed to it.
  if (!eventData->GetViewNode())
    {
    eventData->SetViewNode(this->GetDMMLViewNode());
    }
  return this->Internal->CameraWidget->ProcessInteractionEvent(eventData);
}

//---------------------------------------------------------------------------
vtkDMMLCameraWidget* vtkDMMLCameraDisplayableManager::GetCameraWidget()
{
  return this->Internal->CameraWidget;
}
