/*==============================================================================

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
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care
  and CANARIE.

==============================================================================*/

// Volume Rendering includes
#include "vtkDMMLVolumeRenderingDisplayableManager.h"

#include "vtkCjyxConfigure.h" // For Cjyx_VTK_RENDERING_USE_OpenGL2_BACKEND
#include "vtkCjyxVolumeRenderingLogic.h"
#include "vtkDMMLCPURayCastVolumeRenderingDisplayNode.h"
#include "vtkDMMLGPURayCastVolumeRenderingDisplayNode.h"
#include "vtkDMMLMultiVolumeRenderingDisplayNode.h"

// DMML includes
#include "vtkDMMLAnnotationROINode.h"
#include "vtkDMMLMarkupsROINode.h"
#include "vtkDMMLFolderDisplayNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLScalarVolumeNode.h"
#include "vtkDMMLTransformNode.h"
#include "vtkDMMLViewNode.h"
#include "vtkDMMLVolumePropertyNode.h"
#include "vtkDMMLShaderPropertyNode.h"
#include "vtkEventBroker.h"

// VTK includes
#include <vtkVersion.h> // must precede reference to VTK_MAJOR_VERSION
#include "vtkAddonMathUtilities.h"
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkImageAppendComponents.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageLuminance.h>
#include <vtkInteractorStyle.h>
#include <vtkMatrix4x4.h>
#include <vtkPlane.h>
#include <vtkPlanes.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkMultiVolume.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkDoubleArray.h>
#include <vtkVolumePicker.h>

#include <vtkImageData.h> //TODO: Used for workaround. Remove when fixed
#include <vtkTrivialProducer.h> //TODO: Used for workaround. Remove when fixed
#include <vtkPiecewiseFunction.h> //TODO: Used for workaround. Remove when fixed

// Register VTK object factory overrides
#include <vtkAutoInit.h>
#if defined(Cjyx_VTK_RENDERING_USE_OpenGL2_BACKEND)
VTK_MODULE_INIT(vtkRenderingContextOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
#else
VTK_MODULE_INIT(vtkRenderingContextOpenGL);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL);
#endif

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLVolumeRenderingDisplayableManager);

//---------------------------------------------------------------------------
int vtkDMMLVolumeRenderingDisplayableManager::DefaultGPUMemorySize = 256;

//---------------------------------------------------------------------------
class vtkDMMLVolumeRenderingDisplayableManager::vtkInternal
{
public:
  vtkInternal(vtkDMMLVolumeRenderingDisplayableManager* external);
  ~vtkInternal();

  //-------------------------------------------------------------------------
  class Pipeline
  {
  public:
    Pipeline()
    {
      this->VolumeActor = vtkSmartPointer<vtkVolume>::New();
      this->IJKToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();

      // Only RGBA volumes can be rendered using direct color mapping.
      // To render RGB volumes, the alpha channel is generated from luminance
      // of the colors and appended to the volume before it is passed to the mapper.
      this->ComputeAlphaChannel = vtkSmartPointer<vtkImageLuminance>::New();
      this->MergeAlphaChannelToRGB = vtkSmartPointer<vtkImageAppendComponents>::New();
    }
    virtual ~Pipeline()  = default;

    vtkWeakPointer<vtkDMMLVolumeRenderingDisplayNode> DisplayNode;
    vtkSmartPointer<vtkVolume> VolumeActor;
    vtkSmartPointer<vtkMatrix4x4> IJKToWorldMatrix;
    vtkSmartPointer<vtkImageLuminance> ComputeAlphaChannel;
    vtkSmartPointer<vtkImageAppendComponents> MergeAlphaChannelToRGB;
  };

  //-------------------------------------------------------------------------
  class PipelineCPU : public Pipeline
  {
  public:
    PipelineCPU() : Pipeline()
    {
      this->RayCastMapperCPU = vtkSmartPointer<vtkFixedPointVolumeRayCastMapper>::New();
      this->VolumeScaling = vtkSmartPointer<vtkImageChangeInformation>::New();
      this->RayCastMapperCPU->SetInputConnection(0, this->VolumeScaling->GetOutputPort());
    }
    vtkSmartPointer<vtkFixedPointVolumeRayCastMapper> RayCastMapperCPU;
    vtkSmartPointer<vtkImageChangeInformation> VolumeScaling;
  };
  //-------------------------------------------------------------------------
  class PipelineGPU : public Pipeline
  {
  public:
    PipelineGPU() : Pipeline()
    {
      this->RayCastMapperGPU = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
    }
    vtkSmartPointer<vtkGPUVolumeRayCastMapper> RayCastMapperGPU;
  };
  //-------------------------------------------------------------------------
  class PipelineMultiVolume : public Pipeline
  {
  public:
    PipelineMultiVolume(int actorPortIndex) : Pipeline()
    {
      this->ActorPortIndex = actorPortIndex;
    }

    unsigned int ActorPortIndex;
  };

  //-------------------------------------------------------------------------
  typedef std::vector< Pipeline* > PipelineListType;
  PipelineListType DisplayPipelines;

  std::vector< vtkWeakPointer<vtkDMMLVolumeNode> > ObservedVolumeNodes;

  vtkVolumeMapper* GetVolumeMapper(vtkDMMLVolumeRenderingDisplayNode* displayNode);

  // Volumes
  void AddVolumeNode(vtkDMMLVolumeNode* displayableNode);
  void RemoveVolumeNode(vtkDMMLVolumeNode* displayableNode);

  // Transforms
  // Return with true if pipelines may have changed.
  // If node==nullptr then all pipelines are updated.
  bool UpdatePipelineTransforms(vtkDMMLVolumeNode *node);
  bool GetVolumeTransformToWorld(vtkDMMLVolumeNode* node, vtkMatrix4x4* ijkToWorldMatrix);

  // ROIs
  void UpdatePipelineROIs(vtkDMMLVolumeRenderingDisplayNode* displayNode, const Pipeline* pipeline);

  // Display Nodes
  void AddDisplayNode(vtkDMMLVolumeRenderingDisplayNode* displayNode);
  void RemoveDisplayNode(vtkDMMLVolumeRenderingDisplayNode* displayNode);
  PipelineListType::iterator RemovePipelineIt(PipelineListType::iterator pipelineIt);
  void UpdateDisplayNode(vtkDMMLVolumeRenderingDisplayNode* displayNode);
  void UpdateDisplayNodePipeline(vtkDMMLVolumeRenderingDisplayNode* displayNode, const Pipeline* pipeline);

  double GetFramerate();
  vtkIdType GetMaxMemoryInBytes(vtkDMMLVolumeRenderingDisplayNode* displayNode);
  void UpdateDesiredUpdateRate(vtkDMMLVolumeRenderingDisplayNode* displayNode);

  // Observations
  void AddObservations(vtkDMMLVolumeNode* node);
  void RemoveObservations(vtkDMMLVolumeNode* node);
  bool IsNodeObserved(vtkDMMLVolumeNode* node);

  // Helper functions
  void RemoveOrphanPipelines();
  PipelineListType::iterator GetPipelineIt(vtkDMMLVolumeRenderingDisplayNode* displayNode);
  Pipeline* GetPipeline(vtkDMMLVolumeRenderingDisplayNode* displayNode);
  bool IsVisible(vtkDMMLVolumeRenderingDisplayNode* displayNode);
  bool UseDisplayNode(vtkDMMLVolumeRenderingDisplayNode* displayNode);
  bool UseDisplayableNode(vtkDMMLVolumeNode* node);
  void ClearDisplayableNodes();
  /// Calculate minimum sample distance as minimum of that for shown volumes, and set it to multi-volume mapper
  void UpdateMultiVolumeMapperSampleDistance();
  int GetNextAvailableMultiVolumeActorPortIndex();

  void FindPickedDisplayNodeFromVolumeActor(vtkVolume* volume);

public:
  vtkDMMLVolumeRenderingDisplayableManager* External;

  /// Flag indicating whether adding volume node is in progress
  bool AddingVolumeNode;

  /// Original desired update rate of renderer. Restored when volume hidden
  double OriginalDesiredUpdateRate;

  /// Observed events for the display nodes
  vtkIntArray* DisplayObservedEvents;

  /// When interaction is >0, we are in interactive mode (low level of detail)
  int Interaction;

  /// Picker of volume in renderer
  vtkSmartPointer<vtkVolumePicker> VolumePicker;

  /// Last picked volume rendering display node ID
  std::string PickedNodeID;

private:
  /// Multi-volume actor using a common mapper for rendering the multiple volumes
  vtkSmartPointer<vtkMultiVolume> MultiVolumeActor;
  /// Common GPU mapper for the multi-volume actor.
  /// Note: vtkMultiVolume only supports the GPU raycast mapper
  vtkSmartPointer<vtkGPUVolumeRayCastMapper> MultiVolumeMapper;

  friend class vtkDMMLVolumeRenderingDisplayableManager;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::vtkInternal(vtkDMMLVolumeRenderingDisplayableManager* external)
: External(external)
, AddingVolumeNode(false)
, OriginalDesiredUpdateRate(0.0) // 0 fps is a special value that means it hasn't been set
, Interaction(0)
, PickedNodeID("")
{
  this->MultiVolumeActor = vtkSmartPointer<vtkMultiVolume>::New();
  this->MultiVolumeMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();

  // Set GPU mapper to the multi-volume actor. Both objects are only used for multi-volume GPU ray casting.
  // vtkMultiVolume works differently than the other two rendering modes, in the sense that those use a
  // separate mapper instance for each volume. Instead, vtkMultiVolume operates like this:
  //
  //                             <-- vtkVolume #1 (transfer functions, transform, etc.)
  // Renderer <-- vtkMultiVolume <-- vtkVolume #2
  //                    ^        <-- vtkVolume #3
  //                    |
  //         vtkGPUVolumeRayCastMapper
  //         ^          ^            ^
  //         |          |            |
  //     InputCon1  InputCon2   InputCon3
  //  (vtkImageData)
  //
  this->MultiVolumeActor->SetMapper(this->MultiVolumeMapper);

  this->DisplayObservedEvents = vtkIntArray::New();
  this->DisplayObservedEvents->InsertNextValue(vtkCommand::StartEvent);
  this->DisplayObservedEvents->InsertNextValue(vtkCommand::EndEvent);
  this->DisplayObservedEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  this->DisplayObservedEvents->InsertNextValue(vtkCommand::StartInteractionEvent);
  this->DisplayObservedEvents->InsertNextValue(vtkCommand::InteractionEvent);
  this->DisplayObservedEvents->InsertNextValue(vtkCommand::EndInteractionEvent);

  this->VolumePicker = vtkSmartPointer<vtkVolumePicker>::New();
  this->VolumePicker->SetTolerance(0.005);
}

//---------------------------------------------------------------------------
vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::~vtkInternal()
{
  this->ClearDisplayableNodes();

  if (this->DisplayObservedEvents)
    {
    this->DisplayObservedEvents->Delete();
    this->DisplayObservedEvents = nullptr;
    }
}

//---------------------------------------------------------------------------
vtkVolumeMapper* vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::GetVolumeMapper(vtkDMMLVolumeRenderingDisplayNode* displayNode)
{
  if (!displayNode)
    {
    return nullptr;
    }
  if ( displayNode->IsA("vtkDMMLCPURayCastVolumeRenderingDisplayNode")
    || displayNode->IsA("vtkDMMLGPURayCastVolumeRenderingDisplayNode") )
    {
    vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::Pipeline* pipeline = this->GetPipeline(displayNode);
    if (!pipeline)
      {
      vtkErrorWithObjectMacro(this->External, "GetVolumeMapper: Failed to find pipeline for display node with ID " << displayNode->GetID());
      return nullptr;
      }
    if (displayNode->IsA("vtkDMMLCPURayCastVolumeRenderingDisplayNode"))
      {
      const PipelineCPU* pipelineCpu = dynamic_cast<const PipelineCPU*>(pipeline);
      if (pipelineCpu)
        {
        return pipelineCpu->RayCastMapperCPU;
        }
      }
    else if (displayNode->IsA("vtkDMMLGPURayCastVolumeRenderingDisplayNode"))
      {
      const PipelineGPU* pipelineGpu = dynamic_cast<const PipelineGPU*>(pipeline);
      if (pipelineGpu)
        {
        return pipelineGpu->RayCastMapperGPU;
        }
      }
    }
  else if (displayNode->IsA("vtkDMMLMultiVolumeRenderingDisplayNode"))
    {
    return this->MultiVolumeMapper;
    }
  vtkErrorWithObjectMacro(this->External, "GetVolumeMapper: Unsupported display class " << displayNode->GetClassName());
  return nullptr;
};

//---------------------------------------------------------------------------
bool vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::UseDisplayNode(vtkDMMLVolumeRenderingDisplayNode* displayNode)
{
  // Allow volumes to appear only in designated viewers
  if (displayNode && !displayNode->IsDisplayableInView(this->External->GetDMMLViewNode()->GetID()))
    {
    return false;
    }

  // Check whether display node can be shown in this view
  vtkDMMLVolumeRenderingDisplayNode* volRenDispNode = vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(displayNode);
  if ( !volRenDispNode
    || !volRenDispNode->GetVolumeNodeID()
    || !volRenDispNode->GetVolumePropertyNodeID() )
    {
    return false;
    }

  return true;
}

//---------------------------------------------------------------------------
bool vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::IsVisible(vtkDMMLVolumeRenderingDisplayNode* displayNode)
{
  if (displayNode && displayNode->GetFolderDisplayOverrideAllowed())
    {
    vtkDMMLDisplayableNode* displayableNode = displayNode->GetDisplayableNode();
    if (!vtkDMMLFolderDisplayNode::GetHierarchyVisibility(displayableNode))
      {
      return false;
      }
    }

  return displayNode && displayNode->GetVisibility() && displayNode->GetVisibility3D()
    && displayNode->GetOpacity() > 0
    && displayNode->IsDisplayableInView(this->External->GetDMMLViewNode()->GetID());
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::RemoveOrphanPipelines()
{
  vtkDMMLViewNode* viewNode = this->External->GetDMMLViewNode();
  bool autoRelease = viewNode && viewNode->GetAutoReleaseGraphicsResources();
  PipelineListType::iterator it;
  for (it = this->DisplayPipelines.begin(); it != this->DisplayPipelines.end();)
    {
    Pipeline* pipeline = *it;
    if (!pipeline->DisplayNode.GetPointer() || !pipeline->DisplayNode->GetDisplayableNode()
      || (autoRelease && pipeline->DisplayNode &&
        (!pipeline->DisplayNode->IsDisplayableInView(viewNode->GetID()) || !this->IsVisible(pipeline->DisplayNode))))
      {
      it = this->RemovePipelineIt(it);
      }
    else
      {
      ++it;
      }
    }
}

//---------------------------------------------------------------------------
vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::PipelineListType::iterator
vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::GetPipelineIt(vtkDMMLVolumeRenderingDisplayNode* displayNode)
{
  PipelineListType::iterator it;
  for (it = this->DisplayPipelines.begin(); it != this->DisplayPipelines.end(); ++it)
    {
    if ((*it)->DisplayNode == displayNode)
      {
      break;
      }
    }
  return it;
}

//---------------------------------------------------------------------------
vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::Pipeline*
vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::GetPipeline(vtkDMMLVolumeRenderingDisplayNode* displayNode)
{
  for (Pipeline* pipeline : this->DisplayPipelines)
  {
    if (pipeline->DisplayNode == displayNode)
    {
      return pipeline;
    }
  }
  return nullptr;
}

//---------------------------------------------------------------------------
int vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::GetNextAvailableMultiVolumeActorPortIndex()
{
  //TODO: Change back "port = 1" to to "port = 0" once the VTK issue https://gitlab.kitware.com/vtk/vtk/issues/17325 is fixed
  const int MAXIMUM_NUMBER_OF_MULTIVOLUME_ACTORS = 10;
  for (int port = 1; port < MAXIMUM_NUMBER_OF_MULTIVOLUME_ACTORS; port++)
    {
    // Find out if port is used
    bool portIsUsed = false;
    for (Pipeline* pipeline : this->DisplayPipelines)
      {
      PipelineMultiVolume* pipelineMulti = dynamic_cast<PipelineMultiVolume*>(pipeline);
      if (!pipelineMulti)
        {
        continue;
        }
      if (pipelineMulti->ActorPortIndex == static_cast<unsigned int>(port))
        {
        portIsUsed = true;
        break;
        }
      }
    // If not used then it is good, this is the next available port
    if (!portIsUsed)
      {
      return port;
      }
    }
  // No available port is found
  vtkErrorWithObjectMacro(this->External, "Maximum number of multivolumes (" << MAXIMUM_NUMBER_OF_MULTIVOLUME_ACTORS << ") reached.");
  return -1;
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::AddVolumeNode(vtkDMMLVolumeNode* node)
{
  if (this->AddingVolumeNode)
    {
    return;
    }
  // Check if node should be used
  if (!this->UseDisplayableNode(node))
    {
    return;
    }

  this->AddingVolumeNode = true;

  // Add Display Nodes
  int numDisplayNodes = node->GetNumberOfDisplayNodes();

  this->AddObservations(node);

  for (int i=0; i<numDisplayNodes; i++)
    {
    vtkDMMLVolumeRenderingDisplayNode *displayNode = vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(node->GetNthDisplayNode(i));
    if (this->UseDisplayNode(displayNode))
      {
      this->AddDisplayNode(displayNode);
      }
    }
  this->AddingVolumeNode = false;
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::RemoveVolumeNode(vtkDMMLVolumeNode* node)
{
  if (!node)
    {
    return;
    }
  for (int displayNodeIndex = 0; displayNodeIndex < node->GetNumberOfDisplayNodes(); ++displayNodeIndex)
    {
    this->RemoveDisplayNode(vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(node->GetNthDisplayNode(displayNodeIndex)));
    }
  this->RemoveObservations(node);
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::AddDisplayNode(vtkDMMLVolumeRenderingDisplayNode* displayNode)
{
  if (!displayNode)
    {
    return;
    }
  vtkDMMLVolumeNode* volumeNode = vtkDMMLVolumeNode::SafeDownCast(displayNode->GetDisplayableNode());
  if (!volumeNode)
    {
    return;
    }

  // Do not add the display node if it is already associated with a pipeline object.
  // This happens when a node already associated with a display node is copied into another
  // (using vtkDMMLNode::Copy()) and is added to the scene afterward.
  // Related issue are #3428 and #2608
  if (this->GetPipeline(displayNode))
    {
    return;
    }

  if (displayNode->IsA("vtkDMMLCPURayCastVolumeRenderingDisplayNode"))
    {
    PipelineCPU* pipelineCpu = new PipelineCPU();
    pipelineCpu->DisplayNode = displayNode;
    // Set volume to the mapper
    // Reconnection is expensive operation, therefore only do it if needed
    if (pipelineCpu->VolumeScaling->GetInputConnection(0, 0) != volumeNode->GetImageDataConnection())
      {
      pipelineCpu->VolumeScaling->SetInputConnection(0, volumeNode->GetImageDataConnection());
      }
    // Add volume actor to renderer and local cache
    this->External->GetRenderer()->AddVolume(pipelineCpu->VolumeActor);
    // Add pipeline
    this->DisplayPipelines.push_back(pipelineCpu);
    }
  else if (displayNode->IsA("vtkDMMLGPURayCastVolumeRenderingDisplayNode"))
    {
    PipelineGPU* pipelineGpu = new PipelineGPU();
    pipelineGpu->DisplayNode = displayNode;
    // Set volume to the mapper
    // Reconnection is expensive operation, therefore only do it if needed
    if (pipelineGpu->RayCastMapperGPU->GetInputConnection(0, 0) != volumeNode->GetImageDataConnection())
      {
      pipelineGpu->RayCastMapperGPU->SetInputConnection(0, volumeNode->GetImageDataConnection());
      }
    // Add volume actor to renderer and local cache
    this->External->GetRenderer()->AddVolume(pipelineGpu->VolumeActor);
    // Add pipeline
    this->DisplayPipelines.push_back(pipelineGpu);
    }
  else if (displayNode->IsA("vtkDMMLMultiVolumeRenderingDisplayNode"))
    {
    int actorPortIndex = this->GetNextAvailableMultiVolumeActorPortIndex();
    if (actorPortIndex < 0)
      {
      vtkErrorWithObjectMacro(this->External, "AddDisplayNode: Cannot add Cannot add volume " << volumeNode->GetName() << "to multi-volume renderer");
      return;
      }
    PipelineMultiVolume* pipelineMulti = new PipelineMultiVolume(actorPortIndex);
    pipelineMulti->DisplayNode = displayNode;
    // Create a dummy volume for port zero if this is the first volume. Necessary because the first transform is ignored,
    // see https://gitlab.kitware.com/vtk/vtk/issues/17325
    //TODO: Remove this workaround when the issue is fixed in VTK
    double* multiVolumeBounds = this->MultiVolumeActor->GetBounds();
    if (multiVolumeBounds[0] > multiVolumeBounds[1]) // Prevent error that GetVolume throws if volume is null (TODO: need GetNumberOfVolumes)
      {
      vtkNew<vtkImageData> dummyImage;
      dummyImage->SetExtent(0,1,0,1,0,1);
      dummyImage->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
      dummyImage->SetScalarComponentFromDouble(0,0,0,0, 0.0);
      vtkNew<vtkTrivialProducer> dummyTrivialProducer;
      dummyTrivialProducer->SetOutput(dummyImage);
      this->MultiVolumeMapper->SetInputConnection(0, dummyTrivialProducer->GetOutputPort());

      vtkNew<vtkPiecewiseFunction> dummyOpacity;
      dummyOpacity->AddPoint(0.0, 0.0);
      dummyOpacity->AddPoint(1.0, 0.0);
      vtkNew<vtkVolumeProperty> dummyVolumeProperty;
      dummyVolumeProperty->SetScalarOpacity(dummyOpacity);
      vtkNew<vtkVolume> dummyVolumeActor;
      dummyVolumeActor->SetProperty(dummyVolumeProperty);
      this->MultiVolumeActor->SetVolume(dummyVolumeActor, 0);
      }
    //TODO: Uncomment when https://gitlab.kitware.com/vtk/vtk/issues/17302 is fixed in VTK
    // Set image data to mapper
    //this->MultiVolumeMapper->SetInputConnection(pipelineMulti->ActorPortIndex, volumeNode->GetImageDataConnection());
    // Add volume to multi-volume actor
    //this->MultiVolumeActor->SetVolume(pipelineMulti->VolumeActor, pipelineMulti->ActorPortIndex);
    // Make sure common actor is added to renderer and local cache
    this->External->GetRenderer()->AddVolume(this->MultiVolumeActor);
    // Add pipeline
    this->DisplayPipelines.push_back(pipelineMulti);
    // Update sample distance considering the new volume
    this->UpdateMultiVolumeMapperSampleDistance();
    }

  this->External->GetDMMLNodesObserverManager()->AddObjectEvents(displayNode, this->DisplayObservedEvents);

  // Update cached matrix. Calls UpdateDisplayNodePipeline
  this->UpdatePipelineTransforms(volumeNode);
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::RemoveDisplayNode(vtkDMMLVolumeRenderingDisplayNode* displayNode)
{
  if (!displayNode)
    {
    return;
    }

  PipelineListType::iterator pipelineIt = this->GetPipelineIt(displayNode);
  if (pipelineIt == this->DisplayPipelines.end())
    {
    return;
    }
  this->RemovePipelineIt(pipelineIt);
}

//---------------------------------------------------------------------------
vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::PipelineListType::iterator
vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::RemovePipelineIt(PipelineListType::iterator pipelineIt)
{
  if (pipelineIt == this->DisplayPipelines.end())
    {
    // already removed
    return this->DisplayPipelines.end();
    }

  Pipeline* pipeline = *pipelineIt;

  if (pipeline)
  {
    if (pipeline->VolumeActor)
      {
      this->External->GetRenderer()->RemoveVolume(pipeline->VolumeActor);
      }

    PipelineMultiVolume* pipelineMulti = dynamic_cast<PipelineMultiVolume*>(pipeline);
    if (pipelineMulti)
      {
      // Remove volume actor from multi-volume actor collection
      this->MultiVolumeMapper->RemoveInputConnection(pipelineMulti->ActorPortIndex, 0);
      this->MultiVolumeActor->RemoveVolume(pipelineMulti->ActorPortIndex);

      // Remove common actor from renderer and local cache if the last volume have been removed
      bool foundMultiVolumeActor = false;
      for (Pipeline* pipeline : this->DisplayPipelines)
        {
        PipelineMultiVolume* otherPipelineMulti = dynamic_cast<PipelineMultiVolume*>(pipeline);
        if (otherPipelineMulti == pipelineMulti)
          {
          // this is the current pipeline, which will be deleted in a moment
          // so it does not count as a pipeline that still uses multivolume actor
          continue;
          }
        if (otherPipelineMulti)
          {
          foundMultiVolumeActor = true;
          break;
          }
        }
      if (!foundMultiVolumeActor)
        {
        this->External->GetRenderer()->RemoveVolume(this->MultiVolumeActor);
        }
      }

    delete pipeline;
    }

  return this->DisplayPipelines.erase(pipelineIt);
}

//---------------------------------------------------------------------------
bool vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::UpdatePipelineTransforms(vtkDMMLVolumeNode* volumeNode)
{
  // Update the pipeline for all tracked DisplayableNode
  bool pipelineModified = false;
  for (auto pipeline : this->DisplayPipelines)
    {
    if (!pipeline->DisplayNode)
      {
      continue;
      }
    vtkDMMLVolumeNode* currentVolumeNode = vtkDMMLVolumeNode::SafeDownCast(pipeline->DisplayNode->GetDisplayableNode());
    if (currentVolumeNode == nullptr
      || (volumeNode != nullptr && currentVolumeNode != volumeNode))
      {
      continue;
      }
    this->UpdateDisplayNodePipeline(pipeline->DisplayNode, pipeline);

    // Calculate and apply transform matrix
    this->GetVolumeTransformToWorld(currentVolumeNode, pipeline->IJKToWorldMatrix);
    if (pipeline->DisplayNode->IsA("vtkDMMLCPURayCastVolumeRenderingDisplayNode"))
      {
      const PipelineCPU* pipelineCpu = dynamic_cast<const PipelineCPU*>(pipeline);
      if (pipelineCpu)
        {
        vtkNew<vtkMatrix4x4> unscaledIJKToWorldMatrix;
        unscaledIJKToWorldMatrix->DeepCopy(pipeline->IJKToWorldMatrix);
        double scale[3] = { 1.0 };
        vtkAddonMathUtilities::NormalizeOrientationMatrixColumns(unscaledIJKToWorldMatrix, scale);
        pipelineCpu->VolumeScaling->SetSpacingScale(scale);
        pipeline->VolumeActor->SetUserMatrix(unscaledIJKToWorldMatrix);
        }
      }
    else
      {
      pipeline->VolumeActor->SetUserMatrix(pipeline->IJKToWorldMatrix.GetPointer());
      }
    pipelineModified = true;
    }
  return pipelineModified;
}

//---------------------------------------------------------------------------
bool vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::GetVolumeTransformToWorld(
  vtkDMMLVolumeNode* volumeNode, vtkMatrix4x4* outputIjkToWorldMatrix)
{
  if (volumeNode == nullptr)
    {
    vtkErrorWithObjectMacro(this->External, "GetVolumeTransformToWorld: Invalid volume node");
    return false;
    }

  // Check if we have a transform node
  vtkDMMLTransformNode* transformNode = volumeNode->GetParentTransformNode();
  if (transformNode == nullptr)
    {
    volumeNode->GetIJKToRASMatrix(outputIjkToWorldMatrix);
    return true;
    }

  // IJK to RAS
  vtkMatrix4x4* ijkToRasMatrix = vtkMatrix4x4::New();
  volumeNode->GetIJKToRASMatrix(ijkToRasMatrix);

  // Parent transforms
  vtkMatrix4x4* nodeToWorldMatrix = vtkMatrix4x4::New();
  int success = transformNode->GetMatrixTransformToWorld(nodeToWorldMatrix);
  if (!success)
    {
    vtkWarningWithObjectMacro(this->External, "GetVolumeTransformToWorld: Non-linear parent transform found for volume node " << volumeNode->GetName());
    outputIjkToWorldMatrix->Identity();
    return false;
    }

  // Transform world to RAS
  vtkMatrix4x4::Multiply4x4(nodeToWorldMatrix, ijkToRasMatrix, outputIjkToWorldMatrix);
  outputIjkToWorldMatrix->Modified(); // Needed because Multiply4x4 does not invoke Modified

  ijkToRasMatrix->Delete();
  nodeToWorldMatrix->Delete();
  return true;
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::UpdateDisplayNode(vtkDMMLVolumeRenderingDisplayNode* displayNode)
{
  // If the display node already exists, just update. Otherwise, add as new node
  if (!displayNode)
    {
    return;
    }
  Pipeline* pipeline = this->GetPipeline(displayNode);
  if (pipeline)
    {
    this->UpdateDisplayNodePipeline(displayNode, pipeline);
    }
  else
    {
    this->AddVolumeNode( vtkDMMLVolumeNode::SafeDownCast(displayNode->GetDisplayableNode()) );
    }
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::UpdateDisplayNodePipeline(
  vtkDMMLVolumeRenderingDisplayNode* displayNode, const Pipeline* pipeline)
{
  if (!displayNode || !pipeline)
    {
    vtkErrorWithObjectMacro(this->External, "UpdateDisplayNodePipeline: Display node or pipeline is invalid");
    return;
    }
  vtkDMMLViewNode* viewNode = this->External->GetDMMLViewNode();
  if (!viewNode)
    {
    vtkErrorWithObjectMacro(this->External, "UpdateDisplayNodePipeline: Failed to access view node");
    return;
    }

  // Get volume node
  vtkDMMLVolumeNode* volumeNode = displayNode ? displayNode->GetVolumeNode() : nullptr;
  if (!volumeNode)
    {
    return;
    }

  bool displayNodeVisible = this->IsVisible(displayNode);

  vtkAlgorithmOutput* imageConnection = volumeNode->GetImageDataConnection();
  vtkImageData* imageData = volumeNode->GetImageData();
  int numberOfChannels = (imageData == nullptr ? 1 : imageData->GetNumberOfScalarComponents());
  if (numberOfChannels == 3)
    {
    // RGB volume, generate alpha channel
    pipeline->ComputeAlphaChannel->SetInputConnection(volumeNode->GetImageDataConnection());
    pipeline->MergeAlphaChannelToRGB->RemoveAllInputs();
    pipeline->MergeAlphaChannelToRGB->AddInputConnection(volumeNode->GetImageDataConnection());
    pipeline->MergeAlphaChannelToRGB->AddInputConnection(pipeline->ComputeAlphaChannel->GetOutputPort());
    imageConnection = pipeline->MergeAlphaChannelToRGB->GetOutputPort();
    }
  else
    {
    // Scalar or RGBA volume, no need for generating alpha channel
    pipeline->ComputeAlphaChannel->RemoveAllInputConnections(0);
    pipeline->MergeAlphaChannelToRGB->RemoveAllInputConnections(0);
    }
  // Independent component means that the scalar components of the volume
  // are rendered independently (not as a colored voxel).
  bool independentComponents = (numberOfChannels != 3 && numberOfChannels != 4);

  // Set volume visibility, return if hidden
  pipeline->VolumeActor->SetVisibility(displayNodeVisible);
  // Workaround for lack of support for Visibility flag in vtkMultiVolume's vtkVolume members
  //TODO: Remove when https://gitlab.kitware.com/vtk/vtk/issues/17302 is fixed in VTK
  if (displayNode->IsA("vtkDMMLMultiVolumeRenderingDisplayNode"))
    {
    PipelineMultiVolume* pipelineMulti = dynamic_cast<PipelineMultiVolume*>(this->GetPipeline(displayNode));
    if (pipelineMulti)
      {
      if (displayNodeVisible)
        {
        this->MultiVolumeMapper->SetInputConnection(pipelineMulti->ActorPortIndex, imageConnection);
        this->MultiVolumeActor->SetVolume(pipelineMulti->VolumeActor, pipelineMulti->ActorPortIndex);
        }
      else
        {
        this->MultiVolumeMapper->RemoveInputConnection(pipelineMulti->ActorPortIndex, 0);
        this->MultiVolumeActor->RemoveVolume(pipelineMulti->ActorPortIndex);
        }

      // Workaround: if none of the volumes are visible then VTK renders a gray box,
      // so we need to hide the actor to prevent this.
      bool foundVisibleMultiVolumeActor = false;
      for (Pipeline* pipeline : this->DisplayPipelines)
        {
        PipelineMultiVolume* pipelineMulti = dynamic_cast<PipelineMultiVolume*>(pipeline);
        if (pipelineMulti && pipelineMulti->VolumeActor && pipelineMulti->VolumeActor->GetVisibility())
          {
          foundVisibleMultiVolumeActor = true;
          break;
          }
        }
      this->MultiVolumeActor->SetVisibility(foundVisibleMultiVolumeActor);
      }
    }
  if (!displayNodeVisible)
    {
    return;
    }

  // Get generic volume mapper
  vtkVolumeMapper* mapper = this->GetVolumeMapper(displayNode);
  if (!mapper)
    {
    vtkErrorWithObjectMacro(this->External, "UpdateDisplayNodePipeline: Unable to get volume mapper");
    return;
    }

  // Update specific volume mapper
  if (displayNode->IsA("vtkDMMLCPURayCastVolumeRenderingDisplayNode"))
    {
    vtkFixedPointVolumeRayCastMapper* cpuMapper = vtkFixedPointVolumeRayCastMapper::SafeDownCast(mapper);

    switch (viewNode->GetRaycastTechnique())
      {
      case vtkDMMLViewNode::Adaptive:
        cpuMapper->SetAutoAdjustSampleDistances(true);
        cpuMapper->SetLockSampleDistanceToInputSpacing(false);
        cpuMapper->SetImageSampleDistance(1.0);
        break;
      case vtkDMMLViewNode::Normal:
        cpuMapper->SetAutoAdjustSampleDistances(false);
        cpuMapper->SetLockSampleDistanceToInputSpacing(true);
        cpuMapper->SetImageSampleDistance(1.0);
        break;
      case vtkDMMLViewNode::Maximum:
        cpuMapper->SetAutoAdjustSampleDistances(false);
        cpuMapper->SetLockSampleDistanceToInputSpacing(false);
        cpuMapper->SetImageSampleDistance(0.5);
        break;
      }

    cpuMapper->SetSampleDistance(displayNode->GetSampleDistance());
    cpuMapper->SetInteractiveSampleDistance(displayNode->GetSampleDistance());

    // Make sure the correct mapper is set to the volume
    pipeline->VolumeActor->SetMapper(mapper);
    // Make sure the correct volume is set to the mapper
    const PipelineCPU* pipelineCpu = dynamic_cast<const PipelineCPU*>(pipeline);
    if (pipelineCpu)
      {
      // Reconnection is expensive operation, therefore only do it if needed
      if (pipelineCpu->VolumeScaling->GetInputConnection(0, 0) != imageConnection)
        {
        pipelineCpu->VolumeScaling->SetInputConnection(0, imageConnection);
        }
      }
    }
  else if (displayNode->IsA("vtkDMMLGPURayCastVolumeRenderingDisplayNode"))
    {
    vtkDMMLGPURayCastVolumeRenderingDisplayNode* gpuDisplayNode =
      vtkDMMLGPURayCastVolumeRenderingDisplayNode::SafeDownCast(displayNode);
    vtkGPUVolumeRayCastMapper* gpuMapper = vtkGPUVolumeRayCastMapper::SafeDownCast(mapper);

    switch (viewNode->GetVolumeRenderingQuality())
      {
      case vtkDMMLViewNode::Adaptive:
        gpuMapper->SetAutoAdjustSampleDistances(true);
        gpuMapper->SetLockSampleDistanceToInputSpacing(false);
        gpuMapper->SetUseJittering(viewNode->GetVolumeRenderingSurfaceSmoothing());
        break;
      case vtkDMMLViewNode::Normal:
        gpuMapper->SetAutoAdjustSampleDistances(false);
        gpuMapper->SetLockSampleDistanceToInputSpacing(true);
        gpuMapper->SetUseJittering(viewNode->GetVolumeRenderingSurfaceSmoothing());
        break;
      case vtkDMMLViewNode::Maximum:
        gpuMapper->SetAutoAdjustSampleDistances(false);
        gpuMapper->SetLockSampleDistanceToInputSpacing(false);
        gpuMapper->SetUseJittering(viewNode->GetVolumeRenderingSurfaceSmoothing());
        break;
      }

    gpuMapper->SetSampleDistance(gpuDisplayNode->GetSampleDistance());
    gpuMapper->SetMaxMemoryInBytes(this->GetMaxMemoryInBytes(gpuDisplayNode));

    // Make sure the correct mapper is set to the volume
    pipeline->VolumeActor->SetMapper(mapper);
    // Make sure the correct volume is set to the mapper
    // Reconnection is expensive operation, therefore only do it if needed
    if (mapper->GetInputConnection(0, 0) != imageConnection)
      {
      mapper->SetInputConnection(0, imageConnection);
      }
    }
  else if (displayNode->IsA("vtkDMMLMultiVolumeRenderingDisplayNode"))
    {
    vtkDMMLMultiVolumeRenderingDisplayNode* multiDisplayNode =
      vtkDMMLMultiVolumeRenderingDisplayNode::SafeDownCast(displayNode);
    vtkGPUVolumeRayCastMapper* gpuMultiMapper = vtkGPUVolumeRayCastMapper::SafeDownCast(mapper);

    switch (viewNode->GetRaycastTechnique())
      {
      case vtkDMMLViewNode::Adaptive:
        gpuMultiMapper->SetAutoAdjustSampleDistances(true);
        gpuMultiMapper->SetLockSampleDistanceToInputSpacing(false);
        gpuMultiMapper->SetUseJittering(viewNode->GetVolumeRenderingSurfaceSmoothing());
        break;
      case vtkDMMLViewNode::Normal:
        gpuMultiMapper->SetAutoAdjustSampleDistances(false);
        gpuMultiMapper->SetLockSampleDistanceToInputSpacing(true);
        gpuMultiMapper->SetUseJittering(viewNode->GetVolumeRenderingSurfaceSmoothing());
        break;
      case vtkDMMLViewNode::Maximum:
        gpuMultiMapper->SetAutoAdjustSampleDistances(false);
        gpuMultiMapper->SetLockSampleDistanceToInputSpacing(false);
        gpuMultiMapper->SetUseJittering(viewNode->GetVolumeRenderingSurfaceSmoothing());
        break;
      }

    gpuMultiMapper->SetMaxMemoryInBytes(this->GetMaxMemoryInBytes(multiDisplayNode));
    }
  else
    {
    vtkErrorWithObjectMacro(this->External, "UpdateDisplayNodePipeline: Display node type " << displayNode->GetNodeTagName() << " is not supported");
    return;
    }

  // Set ray casting technique
  switch (viewNode->GetRaycastTechnique())
    {
    case vtkDMMLViewNode::MaximumIntensityProjection:
      mapper->SetBlendMode(vtkVolumeMapper::MAXIMUM_INTENSITY_BLEND);
      break;
    case vtkDMMLViewNode::MinimumIntensityProjection:
      mapper->SetBlendMode(vtkVolumeMapper::MINIMUM_INTENSITY_BLEND);
      break;
    case vtkDMMLViewNode::Composite:
    default:
      mapper->SetBlendMode(vtkVolumeMapper::COMPOSITE_BLEND);
      break;
    }

  // Update ROI clipping planes
  this->UpdatePipelineROIs(displayNode, pipeline);

  // Set volume property
  vtkVolumeProperty* volumeProperty = displayNode->GetVolumePropertyNode() ? displayNode->GetVolumePropertyNode()->GetVolumeProperty() : nullptr;
  if (volumeProperty)
    {
    volumeProperty->SetIndependentComponents(independentComponents);
    }
  pipeline->VolumeActor->SetProperty(volumeProperty);
  // vtkMultiVolume's GetProperty returns the volume property from the first volume actor, and that is used when assembling the
  // shader, so need to set the volume property to the the first volume actor (in this case dummy actor, see above TODO)
  if (this->MultiVolumeActor)
    {
    double* multiVolumeBounds = this->MultiVolumeActor->GetBounds();
    if (multiVolumeBounds[0] < multiVolumeBounds[1]) // Prevent error that GetVolume throws if volume is null (TODO: need GetNumberOfVolumes)
      {
      this->MultiVolumeActor->GetVolume(0)->SetProperty(volumeProperty);
      }
    }

  // Set shader property
  vtkShaderProperty* shaderProperty = displayNode->GetShaderPropertyNode() ? displayNode->GetShaderPropertyNode()->GetShaderProperty() : nullptr;
  pipeline->VolumeActor->SetShaderProperty(shaderProperty);

  pipeline->VolumeActor->SetPickable(volumeNode->GetSelectable());

  this->UpdateDesiredUpdateRate(displayNode);
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::UpdatePipelineROIs(
  vtkDMMLVolumeRenderingDisplayNode* displayNode, const Pipeline* pipeline)
{
  if (!pipeline)
    {
    return;
    }
  vtkVolumeMapper* volumeMapper = this->GetVolumeMapper(displayNode);
  if (!volumeMapper)
    {
    vtkErrorWithObjectMacro(this->External, "UpdatePipelineROIs: Unable to get volume mapper");
    return;
    }
  if (!displayNode || displayNode->GetROINode() == nullptr || !displayNode->GetCroppingEnabled())
    {
    volumeMapper->RemoveAllClippingPlanes();
    return;
    }

  vtkDMMLMarkupsROINode* markupsROINode = displayNode->GetMarkupsROINode();
  vtkDMMLAnnotationROINode* annotationRoiNode = displayNode->GetAnnotationROINode();
  vtkNew<vtkPlanes> planes;
  if (markupsROINode)
    {
    // Calculate and set clipping planes
    markupsROINode->GetTransformedPlanes(planes.GetPointer(), true);
    }
  else if (annotationRoiNode)
    {
    // Make sure the ROI node's inside out flag is on
    annotationRoiNode->InsideOutOn();

    // Calculate and set clipping planes
    annotationRoiNode->GetTransformedPlanes(planes.GetPointer());
    }
  volumeMapper->SetClippingPlanes(planes.GetPointer());
}

//---------------------------------------------------------------------------
double vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::GetFramerate()
{
  vtkDMMLViewNode* viewNode = this->External->GetDMMLViewNode();
  if (!viewNode)
    {
    vtkErrorWithObjectMacro(this->External, "GetFramerate: Failed to access view node");
    return 15.;
    }

  return ( viewNode->GetVolumeRenderingQuality() == vtkDMMLViewNode::Maximum ?
           0.0 : // special value meaning full quality
           std::max(viewNode->GetExpectedFPS(), 0.0001) );
}

//---------------------------------------------------------------------------
vtkIdType vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::GetMaxMemoryInBytes(
  vtkDMMLVolumeRenderingDisplayNode* displayNode)
{
  vtkDMMLViewNode* viewNode = this->External->GetDMMLViewNode();
  if (!viewNode)
    {
    vtkErrorWithObjectMacro(this->External, "GetFramerate: Failed to access view node");
    }

  int gpuMemorySizeMB = vtkDMMLVolumeRenderingDisplayableManager::DefaultGPUMemorySize;
  if (viewNode && viewNode->GetGPUMemorySize() > 0)
    {
    gpuMemorySizeMB = viewNode->GetGPUMemorySize();
    }

  // Special case: for GPU volume raycast mapper, round up to nearest 128MB
  if ( displayNode->IsA("vtkDMMLGPURayCastVolumeRenderingDisplayNode")
    || displayNode->IsA("vtkDMMLMultiVolumeRenderingDisplayNode") )
    {
    if (gpuMemorySizeMB < 128)
      {
      gpuMemorySizeMB = 128;
      }
    else
      {
      gpuMemorySizeMB = ((gpuMemorySizeMB - 1) / 128 + 1) * 128;
      }
    }

  vtkIdType gpuMemorySizeB = vtkIdType(gpuMemorySizeMB) * vtkIdType(1024 * 1024);
  return gpuMemorySizeB;
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::UpdateDesiredUpdateRate(vtkDMMLVolumeRenderingDisplayNode* displayNode)
{
  vtkRenderWindow* renderWindow = this->External->GetRenderer()->GetRenderWindow();
  vtkRenderWindowInteractor* renderWindowInteractor = renderWindow ? renderWindow->GetInteractor() : nullptr;
  if (!renderWindowInteractor)
    {
    return;
    }
  double fps = this->GetFramerate();
  if (displayNode->GetVisibility())
    {
    if (this->OriginalDesiredUpdateRate == 0.0)
      {
      // Save the DesiredUpdateRate before it is changed.
      // It will then be restored when the volume rendering is hidden
      this->OriginalDesiredUpdateRate = renderWindowInteractor->GetDesiredUpdateRate();
      }

    // VTK is overly cautious when estimates rendering speed.
    // This usually results in lower quality and higher frame rates than requested.
    // We update the the desired update rate of the renderer
    // to make the actual update more closely match the desired.
    // desired fps -> correctedFps
    //           1 -> 0.1
    //          10 -> 3.1
    //          50 -> 35
    //         100 -> 100
    double correctedFps = pow(fps, 1.5) / 10.0;
    renderWindowInteractor->SetDesiredUpdateRate(correctedFps);
    }
  else if (this->OriginalDesiredUpdateRate != 0.0)
    {
    // Restore the DesiredUpdateRate to its original value.
    renderWindowInteractor->SetDesiredUpdateRate(this->OriginalDesiredUpdateRate);
    this->OriginalDesiredUpdateRate = 0.0;
    }
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::AddObservations(vtkDMMLVolumeNode* node)
{
  for (auto observedVolumeNode : this->ObservedVolumeNodes)
    {
    if (observedVolumeNode == node)
      {
      // already observed
      return;
      }
    }
  this->ObservedVolumeNodes.push_back(node);
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  broker->AddObservation(node, vtkCommand::ModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand());
  broker->AddObservation(node, vtkDMMLDisplayableNode::TransformModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand() );
  broker->AddObservation(node, vtkDMMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand() );
  broker->AddObservation(node, vtkDMMLVolumeNode::ImageDataModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand() );
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::RemoveObservations(vtkDMMLVolumeNode* node)
{
  for (auto volumeIt = this->ObservedVolumeNodes.begin(); volumeIt != this->ObservedVolumeNodes.end(); ++volumeIt)
    {
    if (volumeIt->GetPointer() == node)
      {
      this->ObservedVolumeNodes.erase(volumeIt);
      break;
      }
    }

  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkEventBroker::ObservationVector observations;
  observations = broker->GetObservations(node, vtkCommand::ModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand());
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(node, vtkDMMLTransformableNode::TransformModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand());
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(node, vtkDMMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(node, vtkDMMLVolumeNode::ImageDataModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
}

//---------------------------------------------------------------------------
bool vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::IsNodeObserved(vtkDMMLVolumeNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkCollection* observations = broker->GetObservationsForSubject(node);
  if (observations->GetNumberOfItems() > 0)
    {
    return true;
    }
  else
    {
    return false;
    }
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::ClearDisplayableNodes()
{
  auto observedVolumeNodesCopy = this->ObservedVolumeNodes;
  for (auto observedVolumeNode : observedVolumeNodesCopy)
    {
    this->RemoveVolumeNode(observedVolumeNode);
    }
  this->RemoveOrphanPipelines();
}

//---------------------------------------------------------------------------
bool vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::UseDisplayableNode(vtkDMMLVolumeNode* node)
{
  bool use = node && node->IsA("vtkDMMLVolumeNode");
  return use;
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::UpdateMultiVolumeMapperSampleDistance()
{
  if (this->AddingVolumeNode)
    {
    return;
    }

  double minimumSampleDistance = VTK_DOUBLE_MAX;
  for (Pipeline* pipeline : this->DisplayPipelines)
    {
    vtkDMMLMultiVolumeRenderingDisplayNode* multiDisplayNode =
      vtkDMMLMultiVolumeRenderingDisplayNode::SafeDownCast(pipeline->DisplayNode);
    if (!multiDisplayNode)
      {
      continue;
      }
    double currentSampleDistance = multiDisplayNode->GetSampleDistance();
    if (this->IsVisible(multiDisplayNode))
      {
      minimumSampleDistance = std::min(minimumSampleDistance, currentSampleDistance);
      }
    }

  vtkGPUVolumeRayCastMapper* gpuMultiMapper = vtkGPUVolumeRayCastMapper::SafeDownCast(this->MultiVolumeMapper);
  gpuMultiMapper->SetSampleDistance(minimumSampleDistance);
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::vtkInternal::FindPickedDisplayNodeFromVolumeActor(vtkVolume* volume)
{
  this->PickedNodeID = "";
  if (!volume)
    {
    return;
    }
  for (Pipeline* pipeline : this->DisplayPipelines)
    {
    vtkDMMLVolumeRenderingDisplayNode* currentDisplayNode = pipeline->DisplayNode;
    vtkVolume* currentVolumeActor = pipeline->VolumeActor.GetPointer();
    if (currentVolumeActor == volume && currentDisplayNode)
      {
      vtkDebugWithObjectMacro(currentDisplayNode, "FindPickedDisplayNodeFromVolumeActor: Found matching volume, pick was on volume "
        << (currentDisplayNode->GetDisplayableNode() ? currentDisplayNode->GetDisplayableNode()->GetName() : "NULL"));
      this->PickedNodeID = currentDisplayNode->GetID();
      }
    }
}

//---------------------------------------------------------------------------
// vtkDMMLVolumeRenderingDisplayableManager methods

//---------------------------------------------------------------------------
vtkDMMLVolumeRenderingDisplayableManager::vtkDMMLVolumeRenderingDisplayableManager()
{
  this->Internal = new vtkInternal(this);

  this->RemoveInteractorStyleObservableEvent(vtkCommand::LeftButtonPressEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::LeftButtonReleaseEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::RightButtonPressEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::RightButtonReleaseEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::MiddleButtonPressEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::MiddleButtonReleaseEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::MouseWheelBackwardEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::MouseWheelForwardEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::EnterEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::LeaveEvent);
  this->AddInteractorStyleObservableEvent(vtkCommand::StartInteractionEvent);
  this->AddInteractorStyleObservableEvent(vtkCommand::EndInteractionEvent);
}

//---------------------------------------------------------------------------
vtkDMMLVolumeRenderingDisplayableManager::~vtkDMMLVolumeRenderingDisplayableManager()
{
  delete this->Internal;
  this->Internal=nullptr;
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::PrintSelf( ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf ( os, indent );
  os << indent << "vtkDMMLVolumeRenderingDisplayableManager: " << this->GetClassName() << "\n";
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::Create()
{
  Superclass::Create();
  this->ObserveGraphicalResourcesCreatedEvent();
  this->SetUpdateFromDMMLRequested(true);
}

//----------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::ObserveGraphicalResourcesCreatedEvent()
{
  vtkDMMLViewNode* viewNode = this->GetDMMLViewNode();
  if (viewNode == nullptr)
    {
    vtkErrorMacro("OnCreate: Failed to access view node");
    return;
    }
  if (!vtkIsObservedDMMLNodeEventMacro(viewNode, vtkDMMLViewNode::GraphicalResourcesCreatedEvent))
    {
    vtkNew<vtkIntArray> events;
    events->InsertNextValue(vtkDMMLViewNode::GraphicalResourcesCreatedEvent);
    vtkObserveDMMLNodeEventsMacro(viewNode, events.GetPointer());
    }
}

//---------------------------------------------------------------------------
int vtkDMMLVolumeRenderingDisplayableManager::ActiveInteractionModes()
{
  // Observe all the modes
  return ~0;
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::UnobserveDMMLScene()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::OnDMMLSceneStartClose()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::OnDMMLSceneEndClose()
{
  this->SetUpdateFromDMMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::OnDMMLSceneEndBatchProcess()
{
  this->SetUpdateFromDMMLRequested(true);
  this->Internal->RemoveOrphanPipelines();
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::OnDMMLSceneEndImport()
{
  // UpdateFromDMML will be executed only if there has been some actions
  // during the import that requested it (don't call
  // SetUpdateFromDMMLRequested(1) here, it should be done somewhere else
  // maybe in OnDMMLSceneNodeAddedEvent, OnDMMLSceneNodeRemovedEvent or
  // OnDMMLDisplayableModelNodeModifiedEvent).
  this->ObserveGraphicalResourcesCreatedEvent();
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::OnDMMLSceneEndRestore()
{
  // UpdateFromDMML will be executed only if there has been some actions
  // during the restoration that requested it (don't call
  // SetUpdateFromDMMLRequested(1) here, it should be done somewhere else
  // maybe in OnDMMLSceneNodeAddedEvent, OnDMMLSceneNodeRemovedEvent or
  // OnDMMLDisplayableModelNodeModifiedEvent).
  this->ObserveGraphicalResourcesCreatedEvent();
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::OnDMMLSceneNodeAdded(vtkDMMLNode* node)
{
  if (node->IsA("vtkDMMLVolumeNode"))
    {
    // Escape if the scene is being closed, imported or connected
    if (this->GetDMMLScene()->IsBatchProcessing())
      {
      this->SetUpdateFromDMMLRequested(true);
      return;
      }

    this->Internal->AddVolumeNode(vtkDMMLVolumeNode::SafeDownCast(node));
    this->RequestRender();
    }
  else if (node->IsA("vtkDMMLViewNode"))
    {
    vtkEventBroker* broker = vtkEventBroker::GetInstance();
    if (!broker->GetObservationExist(node, vtkCommand::ModifiedEvent, this, this->GetDMMLNodesCallbackCommand()))
      {
      broker->AddObservation(node, vtkCommand::ModifiedEvent, this, this->GetDMMLNodesCallbackCommand());
      }
    }
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::OnDMMLSceneNodeRemoved(vtkDMMLNode* node)
{
  vtkDMMLVolumeNode* volumeNode = nullptr;
  vtkDMMLVolumeRenderingDisplayNode* displayNode = nullptr;

  if ( (volumeNode = vtkDMMLVolumeNode::SafeDownCast(node)) )
    {
    this->Internal->RemoveVolumeNode(volumeNode);
    this->RequestRender();
    }
  else if ( (displayNode = vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(node)) )
    {
    this->Internal->RemoveDisplayNode(displayNode);
    this->RequestRender();
    }
  else if (node->IsA("vtkDMMLViewNode"))
    {
    vtkEventBroker* broker = vtkEventBroker::GetInstance();
    vtkEventBroker::ObservationVector observations;
    observations = broker->GetObservations(node, vtkCommand::ModifiedEvent, this, this->GetDMMLNodesCallbackCommand());
    broker->RemoveObservations(observations);
    }

  this->Internal->RemoveOrphanPipelines();
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::ProcessDMMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  vtkDMMLScene* scene = this->GetDMMLScene();

  if (scene == nullptr || scene->IsBatchProcessing())
    {
    return;
    }

  //
  // Volume node events
  //
  vtkDMMLVolumeNode* volumeNode = vtkDMMLVolumeNode::SafeDownCast(caller);
  if (volumeNode)
    {
    if (event == vtkDMMLDisplayableNode::DisplayModifiedEvent)
      {
      vtkDMMLNode* callDataNode = reinterpret_cast<vtkDMMLDisplayNode *> (callData);
      vtkDMMLVolumeRenderingDisplayNode* displayNode = vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(callDataNode);
      if (displayNode)
        {
        // Don't update if we are in an interaction mode
        // (vtkCommand::InteractionEvent will be fired, so we can ignore Modified events)
        if (this->Internal->Interaction == 0)
          {
          this->Internal->UpdateDisplayNode(displayNode);
          this->Internal->UpdateMultiVolumeMapperSampleDistance();
          this->RequestRender();
          }
        }
      }
    else if ( (event == vtkDMMLDisplayableNode::TransformModifiedEvent)
           || (event == vtkDMMLTransformableNode::TransformModifiedEvent)
           || (event == vtkCommand::ModifiedEvent))
      {
      // Parent transforms, volume origin, etc. changed, so we need to recompute transforms
      if (this->Internal->UpdatePipelineTransforms(volumeNode))
        {
        // ROI must not be reset here, as it would make it impossible to replay clipped volume sequences
        this->RequestRender();
        }
      }
    else if (event == vtkDMMLScalarVolumeNode::ImageDataModifiedEvent)
      {
      int numDisplayNodes = volumeNode->GetNumberOfDisplayNodes();
      for (int i=0; i<numDisplayNodes; i++)
        {
        vtkDMMLVolumeRenderingDisplayNode* displayNode = vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(volumeNode->GetNthDisplayNode(i));
        if (this->Internal->UseDisplayNode(displayNode))
          {
          this->Internal->UpdateDisplayNode(displayNode);
          this->Internal->UpdateMultiVolumeMapperSampleDistance();
          this->RequestRender();
          }
        }
      }
    }
  //
  // View node events
  //
  else if (caller->IsA("vtkDMMLViewNode"))
    {
    this->Internal->UpdatePipelineTransforms(nullptr);
    }
  //
  // Other events
  //
  else if (event == vtkCommand::StartEvent ||
           event == vtkCommand::StartInteractionEvent)
    {
    ++this->Internal->Interaction;
    // We request the interactive mode, we might have nested interactions
    // so we just start the mode for the first time.
    if (this->Internal->Interaction == 1)
      {
      vtkInteractorStyle* interactorStyle = vtkInteractorStyle::SafeDownCast(this->GetInteractor()->GetInteractorStyle());
      if (interactorStyle->GetState() == VTKIS_NONE)
        {
        interactorStyle->StartState(VTKIS_VOLUME_PROPS);
        }
      }
    }
  else if (event == vtkCommand::EndEvent ||
           event == vtkCommand::EndInteractionEvent)
    {
    --this->Internal->Interaction;
    if (this->Internal->Interaction == 0)
      {
      vtkInteractorStyle* interactorStyle = vtkInteractorStyle::SafeDownCast(this->GetInteractor()->GetInteractorStyle());
      if (interactorStyle->GetState() == VTKIS_VOLUME_PROPS)
        {
        interactorStyle->StopState();
        }
      if (caller->IsA("vtkDMMLVolumeRenderingDisplayNode"))
        {
        this->Internal->UpdateDisplayNode(vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(caller));
        }
      }
    }
  else if (event == vtkCommand::InteractionEvent)
    {
    if (caller->IsA("vtkDMMLVolumeRenderingDisplayNode"))
      {
      this->Internal->UpdateDisplayNode(vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(caller));
      this->RequestRender();
      }
    }
  else if (event == vtkDMMLViewNode::GraphicalResourcesCreatedEvent)
    {
    this->UpdateFromDMML();
    }
  else
    {
    this->Superclass::ProcessDMMLNodesEvents(caller, event, callData);
    }

  this->Internal->RemoveOrphanPipelines();
}

//----------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::OnInteractorStyleEvent(int eventID)
{
  switch (eventID)
    {
    case vtkCommand::EndInteractionEvent:
    case vtkCommand::StartInteractionEvent:
      this->Internal->UpdatePipelineTransforms(nullptr);
      break;
    default:
      break;
    }
  this->Superclass::OnInteractorStyleEvent(eventID);
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeRenderingDisplayableManager::UpdateFromDMML()
{
  this->SetUpdateFromDMMLRequested(false);

  vtkDMMLScene* scene = this->GetDMMLScene();
  if (!scene)
    {
    vtkDebugMacro( "vtkDMMLVolumeRenderingDisplayableManager::UpdateFromDMML: Scene is not set");
    return;
    }
  this->Internal->ClearDisplayableNodes();

  vtkDMMLVolumeNode* volumeNode = nullptr;
  std::vector<vtkDMMLNode*> volumeNodes;
  int numOfVolumeNodes = scene ? scene->GetNodesByClass("vtkDMMLVolumeNode", volumeNodes) : 0;
  for (int i=0; i<numOfVolumeNodes; i++)
    {
    volumeNode = vtkDMMLVolumeNode::SafeDownCast(volumeNodes[i]);
    if (volumeNode && this->Internal->UseDisplayableNode(volumeNode))
      {
      this->Internal->AddVolumeNode(volumeNode);
      }
    }

  this->RequestRender();
}

//---------------------------------------------------------------------------
vtkVolumeMapper* vtkDMMLVolumeRenderingDisplayableManager::GetVolumeMapper(vtkDMMLVolumeNode* volumeNode)
{
  if (!volumeNode)
    {
    return nullptr;
    }
  vtkDMMLVolumeRenderingDisplayNode* displayNode = nullptr;
  for (auto pipeline : this->Internal->DisplayPipelines)
    {
    if (pipeline->DisplayNode && pipeline->DisplayNode->GetDisplayableNode() == volumeNode)
      {
      // found a match
      if (!displayNode)
        {
        // first match
        displayNode = pipeline->DisplayNode;
        }
      else
        {
        // second match
        vtkWarningMacro("GetVolumeMapper: More than one display node found, using the first one");
        break;
        }
      }
    }
  if (!displayNode)
    {
    vtkErrorMacro("GetVolumeMapper: No volume rendering display node found for volume " << volumeNode->GetName());
    return nullptr;
    }
  return this->Internal->GetVolumeMapper(displayNode);
}

//---------------------------------------------------------------------------
vtkVolume* vtkDMMLVolumeRenderingDisplayableManager::GetVolumeActor(vtkDMMLVolumeNode* volumeNode)
{
  if (!volumeNode)
    {
    return nullptr;
    }
  vtkVolume* volumeActor = nullptr;
  for (auto pipeline : this->Internal->DisplayPipelines)
    {
    if (pipeline->VolumeActor && pipeline->DisplayNode->GetDisplayableNode() == volumeNode)
      {
      // found a match
      if (!volumeActor)
        {
        // first match
        volumeActor = pipeline->VolumeActor;
        }
      else
        {
        // second match
        vtkWarningMacro("GetVolumeActor: More than one volume rendering actor found, using the first one");
        break;
        }
      }
    }
  if (!volumeActor)
    {
    vtkErrorMacro("GetVolumeActor: No volume rendering actor found for volume " << volumeNode->GetName());
    }
  return volumeActor;
}

//---------------------------------------------------------------------------
int vtkDMMLVolumeRenderingDisplayableManager::Pick3D(double ras[3])
{
  this->Internal->PickedNodeID = "";

  vtkRenderer* ren = this->GetRenderer();
  if (!ren)
    {
    vtkErrorMacro("Pick3D: Unable to get renderer");
    return 0;
    }

  if (this->Internal->VolumePicker->Pick3DPoint(ras, ren))
    {
    vtkVolume* volume = vtkVolume::SafeDownCast(this->Internal->VolumePicker->GetProp3D());
    // Find the volume this image data belongs to
    this->Internal->FindPickedDisplayNodeFromVolumeActor(volume);
    }

  return 1;
}

//---------------------------------------------------------------------------
const char* vtkDMMLVolumeRenderingDisplayableManager::GetPickedNodeID()
{
  return this->Internal->PickedNodeID.c_str();
}
