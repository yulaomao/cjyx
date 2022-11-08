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

// DMMLDisplayableManager includes
#include "vtkDMMLVolumeGlyphSliceDisplayableManager.h"

// DMML includes
#include <vtkDMMLColorNode.h>
#include <vtkDMMLDiffusionTensorVolumeNode.h>
#include <vtkDMMLDiffusionTensorVolumeSliceDisplayNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceCompositeNode.h>
#include <vtkDMMLSliceNode.h>

// VTK includes
#include <vtkActor2D.h>
#include <vtkCallbackCommand.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkRenderer.h>
#include <vtkWeakPointer.h>
#include <vtkVersion.h>

// STD includes
#include <algorithm>
#include <cassert>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLVolumeGlyphSliceDisplayableManager );

//---------------------------------------------------------------------------
class vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal
{
public:
  typedef std::map<vtkDMMLDisplayableNode*, std::vector<vtkDMMLDisplayNode*> > DisplayNodesType;
  typedef std::map<vtkDMMLDisplayNode*, vtkProp*> DisplayActorsType;

  vtkInternal(vtkDMMLVolumeGlyphSliceDisplayableManager * external);
  ~vtkInternal();

  vtkObserverManager* GetDMMLNodesObserverManager()const;
  void Modified();

  bool IsDisplayable(vtkDMMLDisplayNode*);
  bool IsVisible(vtkDMMLDisplayNode*);

  // Slice
  vtkDMMLSliceNode* GetSliceNode();
  void UpdateSliceNode();
  // Slice Composite
  vtkDMMLSliceCompositeNode* FindSliceCompositeNode();
  void SetSliceCompositeNode(vtkDMMLSliceCompositeNode* compositeNode);
  void UpdateSliceCompositeNode(vtkDMMLSliceCompositeNode* compositeNode);
  std::vector<vtkDMMLDisplayableNode*>  GetVolumeNodes(
    vtkDMMLSliceCompositeNode*);

  // Volume
  void SetVolumeNodes(const std::vector<vtkDMMLDisplayableNode*>& volumes);
  // Add a volume to the list if it is not yet added. Observe its events
  // Return true on success, false if the volume already exists.
  bool AddVolume(vtkDMMLDisplayableNode* volume);
  // Remove the volume pointed by the iterator, return an iterator pointing to
  // the following
  void RemoveVolume(vtkDMMLDisplayableNode* volume);
  std::vector<vtkDMMLDisplayableNode*>::iterator RemoveVolume(
    std::vector<vtkDMMLDisplayableNode*>::iterator volumeIt);
  void UpdateVolume(vtkDMMLDisplayableNode* volume);

  // VolumeDisplayNode
  void SetVolumeDisplayNodes(vtkDMMLDisplayableNode* volume,
                             std::vector<vtkDMMLDisplayNode*> displayNodes);
  void AddVolumeDisplayNode(DisplayNodesType::iterator displayNodesIt,
                             vtkDMMLDisplayNode* displayNode);
  void RemoveVolumeDisplayNodes(DisplayNodesType::iterator displayNodesIt);
  std::vector<vtkDMMLDisplayNode*>::iterator RemoveVolumeDisplayNode(
    std::vector<vtkDMMLDisplayNode*>& displayNodes,
    std::vector<vtkDMMLDisplayNode*>::iterator displayNodeIt);
  void UpdateVolumeDisplayNode(vtkDMMLDisplayNode* displayNode);

  // Actors
  void AddActor(vtkDMMLDisplayNode* displayNode);
  void RemoveActor(DisplayActorsType::iterator actorIt);
  void UpdateActor(vtkDMMLDisplayNode* displayNode, vtkProp* actor);

  vtkWeakPointer<vtkDMMLSliceCompositeNode> SliceCompositeNode;
  std::vector<vtkDMMLDisplayableNode*>      VolumeNodes;
  DisplayNodesType                          DisplayNodes;
  DisplayActorsType                         Actors;
  vtkDMMLVolumeGlyphSliceDisplayableManager*      External;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal
::vtkInternal(vtkDMMLVolumeGlyphSliceDisplayableManager * external)
{
  this->External = external;
  this->SliceCompositeNode = nullptr;
}

//---------------------------------------------------------------------------
vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal::~vtkInternal()
{
  this->SetSliceCompositeNode(nullptr);
  // everything should be empty
  assert(this->SliceCompositeNode == nullptr);
  assert(this->VolumeNodes.size() == 0);
  assert(this->DisplayNodes.size() == 0);
  assert(this->Actors.size() == 0);
}

//---------------------------------------------------------------------------
vtkObserverManager* vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal::GetDMMLNodesObserverManager()const
{
  return this->External->GetDMMLNodesObserverManager();
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal::Modified()
{
  return this->External->Modified();
}

//---------------------------------------------------------------------------
vtkDMMLSliceNode* vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal
::GetSliceNode()
{
  return this->External->GetDMMLSliceNode();
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal::UpdateSliceNode()
{
  assert(!this->GetSliceNode() || this->GetSliceNode()->GetLayoutName());
  // search the scene for a matching slice composite node
  if (!this->SliceCompositeNode.GetPointer() || // the slice composite has been deleted
      !this->SliceCompositeNode->GetLayoutName() || // the slice composite points to a diff slice node
      strcmp(this->SliceCompositeNode->GetLayoutName(),
             this->GetSliceNode()->GetLayoutName()))
    {
    vtkDMMLSliceCompositeNode* sliceCompositeNode =
      this->FindSliceCompositeNode();
    this->SetSliceCompositeNode(sliceCompositeNode);
    }
}

//---------------------------------------------------------------------------
vtkDMMLSliceCompositeNode* vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal
::FindSliceCompositeNode()
{
  if (this->External->GetDMMLScene() == nullptr ||
      !this->GetSliceNode())
    {
    return nullptr;
    }

  vtkDMMLNode* node;
  vtkCollectionSimpleIterator it;
  vtkCollection* scene = this->External->GetDMMLScene()->GetNodes();
  for (scene->InitTraversal(it);
       (node = (vtkDMMLNode*)scene->GetNextItemAsObject(it)) ;)
    {
    vtkDMMLSliceCompositeNode* sliceCompositeNode =
      vtkDMMLSliceCompositeNode::SafeDownCast(node);
    if (sliceCompositeNode)
      {
      const char* compositeLayoutName = sliceCompositeNode->GetLayoutName();
      const char* sliceLayoutName = this->GetSliceNode()->GetLayoutName();
      if (compositeLayoutName && !strcmp(compositeLayoutName, sliceLayoutName))
        {
        return sliceCompositeNode;
        }
      }
    }
  // no matching slice composite node is found
  return nullptr;
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal
::SetSliceCompositeNode(vtkDMMLSliceCompositeNode* compositeNode)
{
  if (this->SliceCompositeNode == compositeNode)
    {
    return;
    }

  vtkSetAndObserveDMMLNodeMacro(this->SliceCompositeNode, compositeNode);
  this->UpdateSliceCompositeNode(this->SliceCompositeNode);
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal
::UpdateSliceCompositeNode(vtkDMMLSliceCompositeNode* compositeNode)
{
  assert(compositeNode == this->SliceCompositeNode);
  this->SetVolumeNodes(this->GetVolumeNodes(compositeNode));
}

//---------------------------------------------------------------------------
std::vector<vtkDMMLDisplayableNode*>  vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal
::GetVolumeNodes(vtkDMMLSliceCompositeNode* compositeNode)
{
  std::vector<vtkDMMLDisplayableNode*> res;
  if (!compositeNode)
    {
    return res;
    }
  std::vector<vtkDMMLNode*> allVolumes;
  allVolumes.push_back(
    this->External->GetDMMLScene()->GetNodeByID(
      compositeNode->GetBackgroundVolumeID()));
  allVolumes.push_back(
    this->External->GetDMMLScene()->GetNodeByID(
      compositeNode->GetForegroundVolumeID()));
  // As we only support diffusiontensorvolumes for now, we can't find them in
  // the label layer. Feel free to uncomment if needed
  //allVolumes.push_back(
  //  this->External->GetDMMLScene()->GetNodeByID(compositeNode->GetLabelVolumeID()));
  for (unsigned int i = 0; i < allVolumes.size(); ++i)
    {
    // currently only support diffusiontensorvolumes but can easily be extended to other volumes
    vtkDMMLDiffusionTensorVolumeNode* dtiNode =
      vtkDMMLDiffusionTensorVolumeNode::SafeDownCast(allVolumes[i]);
    if (dtiNode)
      {
      res.push_back(dtiNode);
      }
    // Add other volumes supported here
    }
  return res;
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal::SetVolumeNodes(
  const std::vector<vtkDMMLDisplayableNode*>& volumes)
{
  if (volumes == this->VolumeNodes)
    {
    return;
    }
  // Removes the volumes that are not into the new volume list
  for (std::vector<vtkDMMLDisplayableNode*>::iterator it = this->VolumeNodes.begin();
       it != this->VolumeNodes.end(); )
    {
    vtkDMMLDisplayableNode* volume = *it;
    // Don't remove the volume if it belongs to the new list
    if (std::find(volumes.begin(), volumes.end(), volume) == volumes.end())
      {
      // RemoveVolume() returns a pointer to the next volume
      it = this->RemoveVolume(it);
      }
    else
      {
      ++it;
      }
    }
  // Add the new volumes if they are not present yet.
  for (unsigned int i = 0; i < volumes.size(); ++i)
    {
    this->AddVolume(volumes[i]);
    }
}

//---------------------------------------------------------------------------
bool vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal
::AddVolume(vtkDMMLDisplayableNode* volume)
{
  if (std::find(this->VolumeNodes.begin(), this->VolumeNodes.end(), volume) != this->VolumeNodes.end())
    {
    // volume already exists in the list, don't need to add it
    return false;
    }
  // Only observe when a display node is added/removed from the displayable
  // node.
  volume->AddObserver(vtkDMMLDisplayableNode::DisplayModifiedEvent,
                      this->External->GetDMMLNodesCallbackCommand());
  this->VolumeNodes.push_back(volume);
  this->UpdateVolume(volume);
  return true;
}

//---------------------------------------------------------------------------
std::vector<vtkDMMLDisplayableNode*>::iterator vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal
::RemoveVolume(std::vector<vtkDMMLDisplayableNode*>::iterator volumeIt)
{
  vtkDMMLDisplayableNode* volume = *volumeIt;
  // Stop listening to events
  volume->RemoveObserver(this->External->GetDMMLNodesCallbackCommand());
  // Remove displaynodes and actors
  DisplayNodesType::iterator it = this->DisplayNodes.find(volume);
  if (it != this->DisplayNodes.end())
    {
    this->RemoveVolumeDisplayNodes(it);
    }
  // vector::erase() returns a random access iterator pointing to the new location
  // of the element that followed the last element erased by the function call.
  return this->VolumeNodes.erase(volumeIt);
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal
::RemoveVolume(vtkDMMLDisplayableNode* volume)
{
  // Sanity check: make sure the volume exists in the list
  std::vector<vtkDMMLDisplayableNode*>::iterator volumeIt = std::find(this->VolumeNodes.begin(), this->VolumeNodes.end(), volume);
  if (volumeIt != this->VolumeNodes.end())
    {
    this->RemoveVolume(volumeIt);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal
::UpdateVolume(vtkDMMLDisplayableNode* volume)
{
  // Sanity check: make sure the volume exists in the list
  assert(std::find(this->VolumeNodes.begin(), this->VolumeNodes.end(), volume) != this->VolumeNodes.end());
  // The volume has just been added to the volume list or has been modified,
  // update its display node list.
  int nnodes = volume->GetNumberOfDisplayNodes();
  std::vector<vtkDMMLDisplayNode*> displayNodes;
  for (int i=0; i<nnodes; i++)
    {
    if (!volume->GetNthDisplayNode(i))
      {
      continue;
      }
    displayNodes.push_back(volume->GetNthDisplayNode(i));
    }
  this->SetVolumeDisplayNodes(volume, displayNodes);
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal
::SetVolumeDisplayNodes(vtkDMMLDisplayableNode* volume,
                        std::vector<vtkDMMLDisplayNode*> newDisplayNodes)
{
  // Make sure it doesn't exist yet in the display node list
  DisplayNodesType::iterator displayNodesIt = this->DisplayNodes.find(volume);
  if (displayNodesIt != this->DisplayNodes.end())
    {
    if (displayNodesIt->second == newDisplayNodes)
      {
      // Nothing to do here, the list has already been added and is the same.
      // All the display nodes are already observed.
      return;
      }
    // Remove the display nodes that are not in the new display node list
    std::vector<vtkDMMLDisplayNode*>::iterator it;
    for (it = displayNodesIt->second.begin();
         it != displayNodesIt->second.end(); )
      {
      if (std::find(newDisplayNodes.begin(), newDisplayNodes.end(), *it) ==
          newDisplayNodes.end())
        {
        it = this->RemoveVolumeDisplayNode(displayNodesIt->second, it);
        }
      else
        {
        ++it;
        }
      }
    }
  else // volume is not yet in the DisplayNodes list, add it.
    {
    this->DisplayNodes[volume] = std::vector<vtkDMMLDisplayNode*>();
    displayNodesIt = this->DisplayNodes.find(volume);
    assert(displayNodesIt != this->DisplayNodes.end());
    }
  // Add the display nodes that are not added yet
  for (unsigned int i = 0; i < newDisplayNodes.size(); ++i)
    {
    this->AddVolumeDisplayNode(displayNodesIt, newDisplayNodes[i]);
    }
  // Make sure there is no more display nodes than the volume has
  assert( displayNodesIt->second.size() <=
          (unsigned int) displayNodesIt->first->GetNumberOfDisplayNodes());
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal::AddVolumeDisplayNode(
  DisplayNodesType::iterator displayNodesIt, vtkDMMLDisplayNode* displayNode)
{
  std::vector<vtkDMMLDisplayNode*>& displayNodes = displayNodesIt->second;
  if (std::find(displayNodes.begin(), displayNodes.end(), displayNode) !=
      displayNodes.end())
    {
    // displayNode is already in the list and already taken care of
    return;
    }
  if (!this->IsDisplayable(displayNode))
    {
    return;
    }
  displayNode->AddObserver(vtkCommand::ModifiedEvent,
                           this->External->GetDMMLNodesCallbackCommand());
  displayNodesIt->second.push_back(displayNode);
  this->UpdateVolumeDisplayNode(displayNode);
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal::RemoveVolumeDisplayNodes(
  DisplayNodesType::iterator displayNodesIt)
{
  if (displayNodesIt == this->DisplayNodes.end())
    {
    return;
    }
  // Stop listening to all the displayNodes associtaed with the volume
  for (std::vector<vtkDMMLDisplayNode*>::iterator it = displayNodesIt->second.begin();
       it != displayNodesIt->second.end(); )
    {
    it = this->RemoveVolumeDisplayNode(displayNodesIt->second, it);
    }
  assert(displayNodesIt->second.size() == 0);
  this->DisplayNodes.erase(displayNodesIt);
  return;
}

//---------------------------------------------------------------------------
std::vector<vtkDMMLDisplayNode*>::iterator vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal
::RemoveVolumeDisplayNode(std::vector<vtkDMMLDisplayNode*>& displayNodes,
                          std::vector<vtkDMMLDisplayNode*>::iterator displayNodeIt)
{
  // Stop listening to events
  vtkDMMLDisplayNode* displayNode = *displayNodeIt;
  displayNode->RemoveObserver(this->External->GetDMMLNodesCallbackCommand());
  // Remove actors associated with the displayNode
  this->RemoveActor(this->Actors.find(displayNode));

  return displayNodes.erase(displayNodeIt);
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal::UpdateVolumeDisplayNode(
  vtkDMMLDisplayNode* displayNode)
{
  if (!this->IsDisplayable(displayNode))
    {
    return;
    }
  DisplayActorsType::const_iterator it = this->Actors.find(displayNode);
  // If the actors haven't been created yet and the actors should not be visible
  // anyway, then don't even create them. However if the actors were created
  // and the visibility has later been turned off, then update the actor by
  // setting its visibility to off.
  if (!this->IsVisible(displayNode) && it == this->Actors.end())
    {
    return;
    }
  std::cout << "vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal::UpdateVolumeDisplayNode()" << std::endl;
  if (it == this->Actors.end())
    {
    this->AddActor(displayNode);
    }
  else
    {
    // there is already an actor, just update
    this->UpdateActor(it->first, it->second);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal::AddActor(
  vtkDMMLDisplayNode* displayNode)
{
  vtkActor2D* actor = vtkActor2D::New();
  if (displayNode->IsA("vtkDMMLDiffusionTensorVolumeSliceDisplayNode"))
    {
    vtkNew<vtkPolyDataMapper2D> mapper;
    actor->SetMapper(mapper.GetPointer());
    }
  this->External->GetRenderer()->AddActor( actor );
  this->Actors[displayNode] = actor;
  this->UpdateActor(displayNode, actor);
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal::UpdateActor(
  vtkDMMLDisplayNode* displayNode, vtkProp* actor)
{
  if (displayNode->IsA("vtkDMMLDiffusionTensorVolumeSliceDisplayNode"))
    {
    vtkDMMLDiffusionTensorVolumeSliceDisplayNode* dtiDisplayNode =
      vtkDMMLDiffusionTensorVolumeSliceDisplayNode::SafeDownCast(displayNode);

    vtkActor2D* actor2D = vtkActor2D::SafeDownCast(actor);
    vtkPolyDataMapper2D* mapper = vtkPolyDataMapper2D::SafeDownCast(
      actor2D->GetMapper());
    mapper->SetInputConnection( dtiDisplayNode->GetSliceOutputPort() );
    mapper->SetLookupTable( dtiDisplayNode->GetColorNode() ?
                            dtiDisplayNode->GetColorNode()->GetScalarsToColors() : nullptr);
    mapper->SetScalarRange(dtiDisplayNode->GetScalarRange());
    }
  actor->SetVisibility(this->IsVisible(displayNode));
  // TBD: Not sure a render request has to systematically be called
  this->External->RequestRender();
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal::RemoveActor(
  DisplayActorsType::iterator actorIt)
{
  if (actorIt == this->Actors.end())
    {
    return;
    }
  this->External->GetRenderer()->RemoveActor( actorIt->second );
  actorIt->second->Delete();
  this->Actors.erase(actorIt);
}

//---------------------------------------------------------------------------
bool vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal::IsDisplayable(
  vtkDMMLDisplayNode* displayNode)
{
  // Currently only support DTI Slice display nodes, add here more type if
  // needed
  return displayNode
    && displayNode->IsA("vtkDMMLDiffusionTensorVolumeSliceDisplayNode")
    && (std::string(displayNode->GetName()) == this->GetSliceNode()->GetLayoutName());
}

//---------------------------------------------------------------------------
bool vtkDMMLVolumeGlyphSliceDisplayableManager::vtkInternal::IsVisible(
  vtkDMMLDisplayNode* displayNode)
{
  return displayNode->GetVisibility() && displayNode->GetVisibility2D() &&
         displayNode->GetScalarVisibility();
}

//---------------------------------------------------------------------------
// vtkDMMLVolumeGlyphSliceDisplayableManager methods

//---------------------------------------------------------------------------
vtkDMMLVolumeGlyphSliceDisplayableManager::vtkDMMLVolumeGlyphSliceDisplayableManager()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkDMMLVolumeGlyphSliceDisplayableManager::~vtkDMMLVolumeGlyphSliceDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeGlyphSliceDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeGlyphSliceDisplayableManager
::UnobserveDMMLScene()
{
  this->Internal->SetSliceCompositeNode(nullptr);
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeGlyphSliceDisplayableManager
::UpdateFromDMMLScene()
{
  this->Internal->UpdateSliceNode();
}
//---------------------------------------------------------------------------
void vtkDMMLVolumeGlyphSliceDisplayableManager
::OnDMMLSceneStartClose()
{
  std::vector<vtkDMMLDisplayableNode*> volumes = this->Internal->VolumeNodes;
  for (std::vector<vtkDMMLDisplayableNode*>::iterator it = volumes.begin();
       it != volumes.end(); it++)
    {
    this->Internal->RemoveVolume(*it);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeGlyphSliceDisplayableManager
::ProcessDMMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  if (event == vtkDMMLDisplayableNode::DisplayModifiedEvent)
    {
    if (vtkDMMLDisplayableNode::SafeDownCast(caller))
      {
        if (callData == nullptr || vtkDMMLDiffusionTensorVolumeSliceDisplayNode::SafeDownCast((vtkObject *)callData) !=nullptr ) // a display node is added/removed/modified
        {
        this->Internal->UpdateVolume(vtkDMMLDisplayableNode::SafeDownCast(caller));
        }
      }
    }
  else if (event == vtkCommand::ModifiedEvent)
    {
    if (vtkDMMLSliceCompositeNode::SafeDownCast(caller))
      {
      this->Internal->UpdateSliceCompositeNode(vtkDMMLSliceCompositeNode::SafeDownCast(caller));
      }
    else if (vtkDMMLDisplayNode::SafeDownCast(caller))
      {
      this->Internal->UpdateVolumeDisplayNode(vtkDMMLDisplayNode::SafeDownCast(caller));
      }
    }
  else
    {
    this->Superclass::ProcessDMMLNodesEvents(caller, event, callData);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeGlyphSliceDisplayableManager::Create()
{
  this->Internal->UpdateSliceNode();
}
