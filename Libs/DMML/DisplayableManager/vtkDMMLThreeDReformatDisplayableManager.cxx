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

  This file was originally developed by Michael Jeulin-L, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// DMMLDisplayableManager includes
#include "vtkDMMLThreeDReformatDisplayableManager.h"

// DMML includes
#include "vtkDMMLApplicationLogic.h"
#include <vtkDMMLColors.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceCompositeNode.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLSliceLogic.h>
#include <vtkDMMLVolumeNode.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkColor.h>
#include <vtkImageData.h>
#include <vtkImplicitPlaneWidget2.h>
#include <vtkImplicitPlaneRepresentation.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTransform.h>

// STD includes
#include <algorithm>
#include <cmath>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLThreeDReformatDisplayableManager);

//---------------------------------------------------------------------------
class vtkDMMLThreeDReformatDisplayableManager::vtkInternal
{
public:
  typedef std::map<vtkDMMLSliceNode*, vtkImplicitPlaneWidget2*> SliceNodesLink;

  vtkInternal(vtkDMMLThreeDReformatDisplayableManager* external);
  ~vtkInternal();

  // View
  vtkDMMLViewNode* GetViewNode();

  // SliceNodes
  void AddSliceNode(vtkDMMLSliceNode*);
  void RemoveSliceNode(vtkDMMLSliceNode*);
  void RemoveSliceNode(SliceNodesLink::iterator);
  void RemoveAllSliceNodes();
  void UpdateSliceNodes();
  vtkDMMLSliceNode* GetSliceNode(vtkImplicitPlaneWidget2*);

  // Widget
  vtkImplicitPlaneWidget2* NewImplicitPlaneWidget();
  vtkImplicitPlaneWidget2* GetWidget(vtkDMMLSliceNode*);
  // return with true if rendering is required
  bool UpdateWidget(vtkDMMLSliceNode*, vtkImplicitPlaneWidget2*);

  SliceNodesLink                                SliceNodes;
  vtkDMMLThreeDReformatDisplayableManager*      External;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkDMMLThreeDReformatDisplayableManager::vtkInternal::vtkInternal(
    vtkDMMLThreeDReformatDisplayableManager* _external)
{
  this->External = _external;
}

//---------------------------------------------------------------------------
vtkDMMLThreeDReformatDisplayableManager::vtkInternal::~vtkInternal()
{
  this->RemoveAllSliceNodes();
}

//---------------------------------------------------------------------------
vtkDMMLViewNode* vtkDMMLThreeDReformatDisplayableManager::vtkInternal
::GetViewNode()
{
  return this->External->GetDMMLViewNode();
}

//---------------------------------------------------------------------------
void vtkDMMLThreeDReformatDisplayableManager::vtkInternal
::AddSliceNode(vtkDMMLSliceNode* sliceNode)
{
  if (!sliceNode ||
     this->SliceNodes.find(vtkDMMLSliceNode::SafeDownCast(sliceNode)) !=
     this->SliceNodes.end())
    {
    return;
    }

  // We associate the node with the widget if an instantiation is called.
  // We add the sliceNode without instantiating the widget first.
  sliceNode->AddObserver(vtkCommand::ModifiedEvent,
                           this->External->GetDMMLNodesCallbackCommand());
  this->SliceNodes.insert(
    std::pair<vtkDMMLSliceNode*, vtkImplicitPlaneWidget2*>(sliceNode, static_cast<vtkImplicitPlaneWidget2*>(nullptr)));
  this->UpdateWidget(sliceNode, nullptr);
}

//---------------------------------------------------------------------------
void vtkDMMLThreeDReformatDisplayableManager::vtkInternal
::RemoveSliceNode(vtkDMMLSliceNode* sliceNode)
{
  if (!sliceNode)
    {
    return;
    }

   this->RemoveSliceNode(
     this->SliceNodes.find(vtkDMMLSliceNode::SafeDownCast(sliceNode)));
}

//---------------------------------------------------------------------------
void vtkDMMLThreeDReformatDisplayableManager::vtkInternal
::RemoveSliceNode(SliceNodesLink::iterator it)
{
  if (it == this->SliceNodes.end())
    {
    return;
    }

  // The manager has the responsabilty to delete the widget.
  if (it->second)
    {
    it->second->Delete();
    }

  // TODO: it->first might have already been deleted
  it->first->RemoveObserver(this->External->GetDMMLNodesCallbackCommand());
  this->SliceNodes.erase(it);
}

//---------------------------------------------------------------------------
void vtkDMMLThreeDReformatDisplayableManager::vtkInternal
::RemoveAllSliceNodes()
{
  // The manager has the responsabilty to delete the widgets.
  while (this->SliceNodes.size() > 0)
    {
    this->RemoveSliceNode(this->SliceNodes.begin());
    }
  this->SliceNodes.clear();
}

//---------------------------------------------------------------------------
void vtkDMMLThreeDReformatDisplayableManager::vtkInternal::UpdateSliceNodes()
{
  if (this->External->GetDMMLScene() == nullptr)
    {
    this->RemoveAllSliceNodes();
    return;
    }

  vtkDMMLNode* node;
  vtkCollectionSimpleIterator it;
  vtkCollection* scene = this->External->GetDMMLScene()->GetNodes();
  for (scene->InitTraversal(it);
       (node = vtkDMMLNode::SafeDownCast(scene->GetNextItemAsObject(it))) ;)
    {
    vtkDMMLSliceNode* sliceNode = vtkDMMLSliceNode::SafeDownCast(node);
    if (sliceNode)
      {
      this->AddSliceNode(sliceNode);
      }
    }
}

//---------------------------------------------------------------------------
vtkDMMLSliceNode* vtkDMMLThreeDReformatDisplayableManager::vtkInternal::
GetSliceNode(vtkImplicitPlaneWidget2* planeWidget)
{
  if (!planeWidget)
    {
    return nullptr;
    }

  // Get the slice node
  vtkDMMLSliceNode* sliceNode = nullptr;
  for (SliceNodesLink::iterator it=this->SliceNodes.begin();
       it!=this->SliceNodes.end(); ++it)
    {
    if (it->second == planeWidget)
      {
      sliceNode = it->first;
      break;
      }
    }

  return sliceNode;
}

//---------------------------------------------------------------------------
vtkImplicitPlaneWidget2* vtkDMMLThreeDReformatDisplayableManager::vtkInternal::
NewImplicitPlaneWidget()
{
  // Instantiate implcite plane widget and his representation
  vtkNew<vtkImplicitPlaneRepresentation> rep;
  double defaultBounds[6] = {-100, 100, -100, 100, -100, 100};
  rep->PlaceWidget(defaultBounds);
  rep->SetOutlineTranslation(0);
  rep->SetScaleEnabled(0);
  rep->SetDrawPlane(0);

  // The Manager has to manage the destruction of the widgets
  vtkImplicitPlaneWidget2* planeWidget = vtkImplicitPlaneWidget2::New();
  planeWidget->SetInteractor(this->External->GetInteractor());
  planeWidget->SetRepresentation(rep.GetPointer());
  // TODO: enabling picking might make behavior more predicatable
  // when  multiple plane widgets are enabled
  // planeWidget->SetPickingManaged(true);
  planeWidget->SetEnabled(0);

  // Link widget evenement to the WidgetsCallbackCommand
  planeWidget->AddObserver(vtkCommand::StartInteractionEvent,
                           this->External->GetWidgetsCallbackCommand());
  planeWidget->AddObserver(vtkCommand::InteractionEvent,
                           this->External->GetWidgetsCallbackCommand());
  planeWidget->AddObserver(vtkCommand::EndInteractionEvent,
                           this->External->GetWidgetsCallbackCommand());
  planeWidget->AddObserver(vtkCommand::UpdateEvent,
                           this->External->GetWidgetsCallbackCommand());

  return planeWidget;
}

//---------------------------------------------------------------------------
vtkImplicitPlaneWidget2* vtkDMMLThreeDReformatDisplayableManager::vtkInternal
::GetWidget(vtkDMMLSliceNode* sliceNode)
{
  if (!sliceNode)
    {
    return nullptr;
    }

  SliceNodesLink::iterator it = this->SliceNodes.find(sliceNode);
  return (it != this->SliceNodes.end()) ? it->second : 0;
}

//---------------------------------------------------------------------------
bool vtkDMMLThreeDReformatDisplayableManager::vtkInternal
::UpdateWidget(vtkDMMLSliceNode* sliceNode,
               vtkImplicitPlaneWidget2* planeWidget)
{
  if (!sliceNode || (!planeWidget && !sliceNode->GetWidgetVisible()))
    {
    return false;
    }

  if (!planeWidget)
    {
    // Instantiate widget and link it if
    // there is no one associated to the sliceNode yet
    planeWidget = this->NewImplicitPlaneWidget();
    this->SliceNodes.find(sliceNode)->second  = planeWidget;
    }

  // Update the representation
  vtkImplicitPlaneRepresentation* rep =
    planeWidget->GetImplicitPlaneRepresentation();
  vtkMatrix4x4* sliceToRAS = sliceNode->GetSliceToRAS();

  // Color the Edge of the plane representation depending on the Slice
  rep->SetEdgeColor(sliceNode->GetLayoutColor());

  // Update Bound size
  vtkDMMLSliceCompositeNode* sliceCompositeNode =
    vtkDMMLSliceLogic::GetSliceCompositeNode(sliceNode);
  const char* volumeNodeID = nullptr;
  if (!volumeNodeID)
    {
    volumeNodeID = sliceCompositeNode ? sliceCompositeNode->GetBackgroundVolumeID() : nullptr;
    }
  if (!volumeNodeID)
    {
    volumeNodeID = sliceCompositeNode ? sliceCompositeNode->GetForegroundVolumeID() : nullptr;
    }
  if (!volumeNodeID)
    {
    volumeNodeID = sliceCompositeNode ? sliceCompositeNode->GetLabelVolumeID() : nullptr;
    }
  vtkDMMLVolumeNode* volumeNode = vtkDMMLVolumeNode::SafeDownCast(
    this->External->GetDMMLScene()->GetNodeByID(volumeNodeID));
  if (volumeNode)
    {
    double dimensions[3], center[3];
    vtkDMMLSliceLogic::GetVolumeRASBox(volumeNode, dimensions, center);
    double bounds[6] = {bounds[0] = center[0] - dimensions[0] / 2,
                        bounds[1] = center[0] + dimensions[0] / 2,
                        bounds[2] = center[1] - dimensions[1] / 2,
                        bounds[3] = center[1] + dimensions[1] / 2,
                        bounds[4] = center[2] - dimensions[2] / 2,
                        bounds[5] = center[2] + dimensions[2] / 2};
    rep->SetPlaceFactor(1.);
    rep->PlaceWidget(bounds);
    }

  // Update normal
  rep->SetNormal(sliceToRAS->GetElement(0,2),
                 sliceToRAS->GetElement(1,2),
                 sliceToRAS->GetElement(2,2));
  // Update origin position
  rep->SetOrigin(sliceToRAS->GetElement(0,3),
                 sliceToRAS->GetElement(1,3),
                 sliceToRAS->GetElement(2,3));

  rep->SetDrawOutline(sliceNode->GetWidgetOutlineVisible());

  // Update the widget itself if necessary
  bool visible =
    sliceNode->IsDisplayableInThreeDView(this->External->GetDMMLViewNode()->GetID())
    && sliceNode->GetWidgetVisible();

  // re-render if it was visible or now becomes visible
  bool renderingRequired = planeWidget->GetEnabled() || visible;

  if ((!planeWidget->GetEnabled() && visible) ||
     (planeWidget->GetEnabled() && !visible) ||
     (!rep->GetLockNormalToCamera() && sliceNode->GetWidgetNormalLockedToCamera()) ||
     (rep->GetLockNormalToCamera() && !sliceNode->GetWidgetNormalLockedToCamera()))
    {
    planeWidget->SetEnabled(sliceNode->GetWidgetVisible());
    planeWidget->SetLockNormalToCamera(sliceNode->GetWidgetNormalLockedToCamera());
    }

  return renderingRequired;
}

//---------------------------------------------------------------------------
// vtkDMMLSliceModelDisplayableManager methods

//---------------------------------------------------------------------------
vtkDMMLThreeDReformatDisplayableManager::vtkDMMLThreeDReformatDisplayableManager()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkDMMLThreeDReformatDisplayableManager::~vtkDMMLThreeDReformatDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkDMMLThreeDReformatDisplayableManager::UnobserveDMMLScene()
{
  this->Internal->RemoveAllSliceNodes();
}

//---------------------------------------------------------------------------
void vtkDMMLThreeDReformatDisplayableManager::UpdateFromDMMLScene()
{
  this->Internal->UpdateSliceNodes();
}

//---------------------------------------------------------------------------
void vtkDMMLThreeDReformatDisplayableManager
::OnDMMLSceneNodeAdded(vtkDMMLNode* nodeAdded)
{
  if (this->GetDMMLScene()->IsBatchProcessing() ||
      !nodeAdded->IsA("vtkDMMLSliceNode"))
    {
    return;
    }

  this->Internal->AddSliceNode(vtkDMMLSliceNode::SafeDownCast(nodeAdded));
}

//---------------------------------------------------------------------------
void vtkDMMLThreeDReformatDisplayableManager
::OnDMMLSceneNodeRemoved(vtkDMMLNode* nodeRemoved)
{
  if (!nodeRemoved->IsA("vtkDMMLSliceNode"))
    {
    return;
    }

  this->Internal->RemoveSliceNode(vtkDMMLSliceNode::SafeDownCast(nodeRemoved));
}

//---------------------------------------------------------------------------
void vtkDMMLThreeDReformatDisplayableManager::
OnDMMLNodeModified(vtkDMMLNode* node)
{
  vtkDMMLSliceNode* sliceNode = vtkDMMLSliceNode::SafeDownCast(node);
  assert(sliceNode);
  vtkImplicitPlaneWidget2* planeWidget = this->Internal->GetWidget(sliceNode);
  if (this->Internal->UpdateWidget(sliceNode, planeWidget))
    {
    this->RequestRender();
    }
}

//----------------------------------------------------------------------------
void vtkDMMLThreeDReformatDisplayableManager::
ProcessWidgetsEvents(vtkObject *caller,
                    unsigned long event,
                    void *vtkNotUsed(callData))
{
  vtkImplicitPlaneWidget2* planeWidget =
    vtkImplicitPlaneWidget2::SafeDownCast(caller);
  vtkDMMLSliceNode* sliceNode = Internal->GetSliceNode(planeWidget);
  vtkImplicitPlaneRepresentation* rep = (planeWidget) ?
    planeWidget->GetImplicitPlaneRepresentation() : nullptr;

  if (!planeWidget || !sliceNode || !rep)
    {
    return;
    }

  // Broadcast widget transformation
  vtkDMMLSliceLogic* sliceLogic = this->GetDMMLApplicationLogic()->GetSliceLogic(sliceNode);
  if (event == vtkCommand::StartInteractionEvent && sliceLogic )
    {
    sliceLogic->StartSliceNodeInteraction(vtkDMMLSliceNode::MultiplanarReformatFlag);
    return;
    }
  else if (event == vtkCommand::EndInteractionEvent && sliceLogic)
    {
    sliceLogic->EndSliceNodeInteraction();
    return;
    }
  // We should listen to the interactorStyle instead when LockNormalToCamera on.
  else if (planeWidget->GetImplicitPlaneRepresentation()->GetLockNormalToCamera() && sliceLogic)
    {
    sliceLogic->StartSliceNodeInteraction(vtkDMMLSliceNode::MultiplanarReformatFlag);
    }

  double cross[3], dot, rotation;
  vtkNew<vtkTransform> transform;
  vtkMatrix4x4* sliceToRAS = sliceNode->GetSliceToRAS();
  double sliceNormal[3] = {sliceToRAS->GetElement(0,2),
                           sliceToRAS->GetElement(1,2),
                           sliceToRAS->GetElement(2,2)};

  // Reset current translation
  sliceToRAS->SetElement(0,3,0);
  sliceToRAS->SetElement(1,3,0);
  sliceToRAS->SetElement(2,3,0);

  // Rotate the sliceNode to match the planeWidget normal
  vtkMath::Cross(sliceNormal, rep->GetNormal(), cross);
  dot = vtkMath::Dot(sliceNormal, rep->GetNormal());
  // Clamp the dot product
  dot = (dot < -1.0) ? -1.0 : (dot > 1.0 ? 1.0 : dot);
  rotation = vtkMath::DegreesFromRadians(acos(dot));

  // Apply the rotation
  transform->PostMultiply();
  transform->SetMatrix(sliceToRAS);
  transform->RotateWXYZ(rotation,cross);
  transform->GetMatrix(sliceToRAS); // Update the changes within sliceToRAS

  // Insert the widget translation
  double* planeWidgetOrigin = rep->GetOrigin();
  sliceToRAS->SetElement(0, 3, planeWidgetOrigin[0]);
  sliceToRAS->SetElement(1, 3, planeWidgetOrigin[1]);
  sliceToRAS->SetElement(2, 3, planeWidgetOrigin[2]);
  sliceNode->UpdateMatrices();

  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkDMMLThreeDReformatDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkDMMLThreeDReformatDisplayableManager::Create()
{
  this->Internal->UpdateSliceNodes();
}
