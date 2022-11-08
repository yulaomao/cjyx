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

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

==============================================================================*/


// DMMLDisplayableManager includes
#include "vtkDMMLTransformsDisplayableManager3D.h"

#include "vtkCjyxTransformLogic.h"

// DMML includes
#include <vtkEventBroker.h>
#include <vtkDMMLProceduralColorNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLTransformDisplayNode.h>
#include <vtkDMMLTransformNode.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkColorTransferFunction.h>
#include <vtkDataSetAttributes.h>
#include <vtkLookupTable.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkDMMLTransformsDisplayableManager3D );

//---------------------------------------------------------------------------
class vtkDMMLTransformsDisplayableManager3D::vtkInternal
{
public:

  vtkInternal(vtkDMMLTransformsDisplayableManager3D* external);
  ~vtkInternal();

  struct Pipeline
    {
    vtkSmartPointer<vtkActor> Actor;
    vtkSmartPointer<vtkPolyData> InputPolyData;
    };

  typedef std::map < vtkDMMLTransformDisplayNode*, const Pipeline* > PipelinesCacheType;
  PipelinesCacheType DisplayPipelines;

  typedef std::map < vtkDMMLTransformNode*, std::set< vtkDMMLTransformDisplayNode* > > TransformToDisplayCacheType;
  TransformToDisplayCacheType TransformToDisplayNodes;

  // Transforms
  void AddTransformNode(vtkDMMLTransformNode* displayableNode);
  void RemoveTransformNode(vtkDMMLTransformNode* displayableNode);
  void UpdateDisplayableTransforms(vtkDMMLTransformNode *node);

  // Display Nodes
  void AddDisplayNode(vtkDMMLTransformNode*, vtkDMMLTransformDisplayNode*);
  void UpdateDisplayNode(vtkDMMLTransformDisplayNode* displayNode);
  void UpdateDisplayNodePipeline(vtkDMMLTransformDisplayNode*, const Pipeline*);
  void RemoveDisplayNode(vtkDMMLTransformDisplayNode* displayNode);
  void SetTransformDisplayProperty(vtkDMMLTransformDisplayNode *displayNode, vtkActor* actor);

  // Observations
  void AddObservations(vtkDMMLTransformNode* node);
  void RemoveObservations(vtkDMMLTransformNode* node);
  bool IsNodeObserved(vtkDMMLTransformNode* node);

  // Helper functions
  bool IsVisible(vtkDMMLTransformDisplayNode* displayNode);
  bool UseDisplayNode(vtkDMMLTransformDisplayNode* displayNode);
  bool UseDisplayableNode(vtkDMMLTransformNode* node);
  void ClearDisplayableNodes();

private:
  vtkDMMLTransformsDisplayableManager3D* External;
  bool AddingTransformNode;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkDMMLTransformsDisplayableManager3D::vtkInternal::vtkInternal(vtkDMMLTransformsDisplayableManager3D * external)
: External(external)
, AddingTransformNode(false)
{
}

//---------------------------------------------------------------------------
vtkDMMLTransformsDisplayableManager3D::vtkInternal::~vtkInternal()
{
  this->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
bool vtkDMMLTransformsDisplayableManager3D::vtkInternal::UseDisplayNode(vtkDMMLTransformDisplayNode* displayNode)
{
   // allow annotations to appear only in designated viewers
  if (displayNode && !displayNode->IsDisplayableInView(this->External->GetDMMLViewNode()->GetID()))
    {
    return false;
    }

  // Check whether DisplayNode should be shown in this view
  bool use = displayNode && displayNode->IsA("vtkDMMLTransformDisplayNode");

  return use;
}

//---------------------------------------------------------------------------
bool vtkDMMLTransformsDisplayableManager3D::vtkInternal::IsVisible(vtkDMMLTransformDisplayNode* displayNode)
{
  return displayNode && (displayNode->GetVisibility() != 0) && (displayNode->GetVisibility3D() != 0);
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager3D::vtkInternal::AddTransformNode(vtkDMMLTransformNode* node)
{
  if (this->AddingTransformNode)
    {
    return;
    }
  // Check if node should be used
  if (!this->UseDisplayableNode(node))
    {
    return;
    }

  this->AddingTransformNode = true;
  // Add Display Nodes
  int nnodes = node->GetNumberOfDisplayNodes();

  this->AddObservations(node);

  for (int i=0; i<nnodes; i++)
    {
    vtkDMMLTransformDisplayNode *dnode = vtkDMMLTransformDisplayNode::SafeDownCast(node->GetNthDisplayNode(i));
    if ( this->UseDisplayNode(dnode) )
      {
      this->TransformToDisplayNodes[node].insert(dnode);
      this->AddDisplayNode( node, dnode );
      }
    }
  this->AddingTransformNode = false;
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager3D::vtkInternal::RemoveTransformNode(vtkDMMLTransformNode* node)
{
  if (!node)
    {
    return;
    }
  vtkInternal::TransformToDisplayCacheType::iterator displayableIt =
    this->TransformToDisplayNodes.find(node);
  if(displayableIt == this->TransformToDisplayNodes.end())
    {
    return;
    }

  std::set<vtkDMMLTransformDisplayNode *> dnodes = displayableIt->second;
  std::set<vtkDMMLTransformDisplayNode *>::iterator diter;
  for ( diter = dnodes.begin(); diter != dnodes.end(); ++diter)
    {
    this->RemoveDisplayNode(*diter);
    }
  this->RemoveObservations(node);
  this->TransformToDisplayNodes.erase(displayableIt);
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager3D::vtkInternal::UpdateDisplayableTransforms(vtkDMMLTransformNode* mNode)
{
  // Update the pipeline for all tracked DisplayableNode

  PipelinesCacheType::iterator pipelinesIter;
  std::set< vtkDMMLTransformDisplayNode* > displayNodes = this->TransformToDisplayNodes[mNode];
  std::set< vtkDMMLTransformDisplayNode* >::iterator dnodesIter;
  for ( dnodesIter = displayNodes.begin(); dnodesIter != displayNodes.end(); dnodesIter++ )
    {
    if ( ((pipelinesIter = this->DisplayPipelines.find(*dnodesIter)) != this->DisplayPipelines.end()) )
      {
      this->UpdateDisplayNodePipeline(pipelinesIter->first, pipelinesIter->second);
      }
    }
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager3D::vtkInternal::RemoveDisplayNode(vtkDMMLTransformDisplayNode* displayNode)
{
  PipelinesCacheType::iterator actorsIt = this->DisplayPipelines.find(displayNode);
  if(actorsIt == this->DisplayPipelines.end())
    {
    return;
    }
  const Pipeline* pipeline = actorsIt->second;
  this->External->GetRenderer()->RemoveActor(pipeline->Actor);
  delete pipeline;
  this->DisplayPipelines.erase(actorsIt);
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager3D::vtkInternal::AddDisplayNode(vtkDMMLTransformNode* mNode, vtkDMMLTransformDisplayNode* displayNode)
{
  if (!mNode || !displayNode)
    {
    return;
    }

  // Do not add the display node if it is already associated with a pipeline object.
  // This happens when a transform node already associated with a display node
  // is copied into an other (using vtkDMMLNode::Copy()) and is added to the scene afterward.
  // Related issue are #3428 and #2608
  PipelinesCacheType::iterator it;
  it = this->DisplayPipelines.find(displayNode);
  if (it != this->DisplayPipelines.end())
    {
    return;
    }

  // Create pipeline
  Pipeline* pipeline = new Pipeline();

  pipeline->Actor = vtkSmartPointer<vtkActor>::New();
  vtkNew<vtkPolyDataMapper> mapper;
  pipeline->Actor->SetMapper(mapper.GetPointer());
  pipeline->Actor->SetVisibility(false);
  pipeline->InputPolyData = vtkSmartPointer<vtkPolyData>::New();
  mapper->SetInputData(pipeline->InputPolyData);

  // Add actor to Renderer and local cache
  this->External->GetRenderer()->AddActor( pipeline->Actor );
  this->DisplayPipelines.insert( std::make_pair(displayNode, pipeline) );

  // Update cached matrices. Calls UpdateDisplayNodePipeline
  this->UpdateDisplayableTransforms(mNode);
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager3D::vtkInternal::UpdateDisplayNode(vtkDMMLTransformDisplayNode* displayNode)
{
  // If the DisplayNode already exists, just update.
  //   otherwise, add as new node

  if (!displayNode)
    {
    return;
    }
  PipelinesCacheType::iterator it;
  it = this->DisplayPipelines.find(displayNode);
  if (it != this->DisplayPipelines.end())
    {
    this->UpdateDisplayNodePipeline(displayNode, it->second);
    }
  else
    {
    this->AddTransformNode( vtkDMMLTransformNode::SafeDownCast(displayNode->GetDisplayableNode()) );
    }
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager3D::vtkInternal::UpdateDisplayNodePipeline(vtkDMMLTransformDisplayNode* displayNode, const Pipeline* pipeline)
{
  // Sets visibility, set pipeline polydata input, update color
  //   calculate and set pipeline transforms.

  if (!displayNode || !pipeline)
    {
    return;
    }

  vtkDMMLTransformNode* transformNode=vtkDMMLTransformNode::SafeDownCast(displayNode->GetDisplayableNode());
  if (transformNode==nullptr)
    {
    pipeline->Actor->SetVisibility(false);
    return;
    }

  vtkDMMLNode* regionNode=displayNode->GetRegionNode();
  if (displayNode->GetVisualizationMode() == vtkDMMLTransformDisplayNode::VIS_MODE_GLYPH && displayNode->GetGlyphPointsNode())
    {
    // If a node is specified for glyph visualization then region is ignored.
    regionNode = displayNode->GetGlyphPointsNode();
    }
  if (regionNode==nullptr)
    {
    pipeline->Actor->SetVisibility(false);
    return;
    }

  // Update visibility
  bool visible = this->IsVisible(displayNode);
  pipeline->Actor->SetVisibility(visible);
  if (!visible)
    {
    return;
    }

  if (!vtkCjyxTransformLogic::GetVisualization3d(pipeline->InputPolyData, displayNode, regionNode))
  {
    vtkWarningWithObjectMacro(displayNode, "Failed to show transform in 3D: unsupported ROI type");
    pipeline->Actor->SetVisibility(false);
    return;
  }

  if (pipeline->InputPolyData->GetNumberOfPoints()==0)
    {
    pipeline->Actor->SetVisibility(false);
    return;
    }

  // Update pipeline actor
  this->SetTransformDisplayProperty(displayNode, pipeline->Actor);
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager3D::vtkInternal::AddObservations(vtkDMMLTransformNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  if (!broker->GetObservationExist(node, vtkDMMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkDMMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand() );
    }
  if (!broker->GetObservationExist(node, vtkDMMLTransformableNode::TransformModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkDMMLTransformableNode::TransformModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand() );
    }
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager3D::vtkInternal::RemoveObservations(vtkDMMLTransformNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkEventBroker::ObservationVector observations;
  observations = broker->GetObservations(node, vtkDMMLTransformableNode::TransformModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(node, vtkDMMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
}

//---------------------------------------------------------------------------
bool vtkDMMLTransformsDisplayableManager3D::vtkInternal::IsNodeObserved(vtkDMMLTransformNode* node)
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
void vtkDMMLTransformsDisplayableManager3D::vtkInternal::ClearDisplayableNodes()
{
  while(this->TransformToDisplayNodes.size() > 0)
    {
    this->RemoveTransformNode(this->TransformToDisplayNodes.begin()->first);
    }
}

//---------------------------------------------------------------------------
bool vtkDMMLTransformsDisplayableManager3D::vtkInternal::UseDisplayableNode(vtkDMMLTransformNode* node)
{
  bool use = node && node->IsA("vtkDMMLTransformNode");
  return use;
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager3D::vtkInternal::SetTransformDisplayProperty(vtkDMMLTransformDisplayNode *displayNode, vtkActor* actor)
{
  bool visible = this->IsVisible(displayNode);
  actor->SetVisibility(visible);

  vtkMapper* mapper=actor->GetMapper();

  // if the scalars are visible, set active scalars
  bool scalarVisibility = false;
  if (displayNode->GetScalarVisibility())
    {
    vtkColorTransferFunction* colorTransferFunction=displayNode->GetColorMap();
    if (colorTransferFunction != nullptr && colorTransferFunction->GetSize()>0)
      {
      // Copy the transfer function to not share them between multiple mappers
      vtkNew<vtkColorTransferFunction> colorTransferFunctionCopy;
      colorTransferFunctionCopy->DeepCopy(colorTransferFunction);
      mapper->SetLookupTable(colorTransferFunctionCopy.GetPointer());
      mapper->SetScalarModeToUsePointData();
      mapper->SetColorModeToMapScalars();
      mapper->SelectColorArray(vtkCjyxTransformLogic::GetVisualizationDisplacementMagnitudeScalarName());
      mapper->UseLookupTableScalarRangeOff();
      mapper->SetScalarRange(displayNode->GetScalarRange());
      scalarVisibility = true;
      }
    }
  mapper->SetScalarVisibility(scalarVisibility);

  actor->GetProperty()->SetRepresentation(displayNode->GetRepresentation());
  actor->GetProperty()->SetPointSize(displayNode->GetPointSize());
  actor->GetProperty()->SetLineWidth(displayNode->GetLineWidth());
  actor->GetProperty()->SetLighting(displayNode->GetLighting());
  actor->GetProperty()->SetInterpolation(displayNode->GetInterpolation());
  actor->GetProperty()->SetShading(displayNode->GetShading());
  actor->GetProperty()->SetFrontfaceCulling(displayNode->GetFrontfaceCulling());
  actor->GetProperty()->SetBackfaceCulling(displayNode->GetBackfaceCulling());

  if (displayNode->GetSelected())
    {
    actor->GetProperty()->SetColor(displayNode->GetSelectedColor());
    actor->GetProperty()->SetAmbient(displayNode->GetSelectedAmbient());
    actor->GetProperty()->SetSpecular(displayNode->GetSelectedSpecular());
    }
  else
    {
    actor->GetProperty()->SetColor(displayNode->GetColor());
    actor->GetProperty()->SetAmbient(displayNode->GetAmbient());
    actor->GetProperty()->SetSpecular(displayNode->GetSpecular());
    }
  actor->GetProperty()->SetOpacity(displayNode->GetOpacity());
  actor->GetProperty()->SetDiffuse(displayNode->GetDiffuse());
  actor->GetProperty()->SetSpecularPower(displayNode->GetPower());
  actor->GetProperty()->SetMetallic(displayNode->GetMetallic());
  actor->GetProperty()->SetRoughness(displayNode->GetRoughness());
  actor->GetProperty()->SetEdgeVisibility(displayNode->GetEdgeVisibility());
  actor->GetProperty()->SetEdgeColor(displayNode->GetEdgeColor());

  actor->SetTexture(nullptr);
}

//---------------------------------------------------------------------------
// vtkDMMLTransformsDisplayableManager3D methods

//---------------------------------------------------------------------------
vtkDMMLTransformsDisplayableManager3D::vtkDMMLTransformsDisplayableManager3D()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkDMMLTransformsDisplayableManager3D::~vtkDMMLTransformsDisplayableManager3D()
{
  delete this->Internal;
  this->Internal=nullptr;
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager3D::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf ( os, indent );
  os << indent << "vtkDMMLTransformsDisplayableManager3D: " << this->GetClassName() << "\n";
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager3D::OnDMMLSceneNodeAdded(vtkDMMLNode* node)
{
  if ( !node->IsA("vtkDMMLTransformNode") )
    {
    return;
    }

  // Escape if the scene a scene is being closed, imported or connected
  if (this->GetDMMLScene()->IsBatchProcessing())
    {
    this->SetUpdateFromDMMLRequested(true);
    return;
    }

  this->Internal->AddTransformNode(vtkDMMLTransformNode::SafeDownCast(node));
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager3D::OnDMMLSceneNodeRemoved(vtkDMMLNode* node)
{
  if ( node
    && (!node->IsA("vtkDMMLTransformNode"))
    && (!node->IsA("vtkDMMLTransformDisplayNode")) )
    {
    return;
    }

  vtkDMMLTransformNode* transformNode = nullptr;
  vtkDMMLTransformDisplayNode* displayNode = nullptr;

  bool modified = false;
  if ( (transformNode = vtkDMMLTransformNode::SafeDownCast(node)) )
    {
    this->Internal->RemoveTransformNode(transformNode);
    modified = true;
    }
  else if ( (displayNode = vtkDMMLTransformDisplayNode::SafeDownCast(node)) )
    {
    this->Internal->RemoveDisplayNode(displayNode);
    modified = true;
    }
  if (modified)
    {
    this->RequestRender();
    }
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager3D::ProcessDMMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  vtkDMMLScene* scene = this->GetDMMLScene();

  if (scene == nullptr || scene->IsBatchProcessing())
    {
    return;
    }

  vtkDMMLTransformNode* displayableNode = vtkDMMLTransformNode::SafeDownCast(caller);

  if ( displayableNode )
    {
    vtkDMMLNode* callDataNode = reinterpret_cast<vtkDMMLDisplayNode *> (callData);
    vtkDMMLTransformDisplayNode* displayNode = vtkDMMLTransformDisplayNode::SafeDownCast(callDataNode);

    if ( displayNode && (event == vtkDMMLDisplayableNode::DisplayModifiedEvent) )
      {
      this->Internal->UpdateDisplayNode(displayNode);
      this->RequestRender();
      }
    else if (event == vtkDMMLTransformableNode::TransformModifiedEvent)
      {
      this->Internal->UpdateDisplayableTransforms(displayableNode);
      this->RequestRender();
      }
    }
  else
    {
    this->Superclass::ProcessDMMLNodesEvents(caller, event, callData);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager3D::UpdateFromDMML()
{
  this->SetUpdateFromDMMLRequested(false);

  vtkDMMLScene* scene = this->GetDMMLScene();
  if (!scene)
    {
    vtkDebugMacro( "vtkDMMLTransformsDisplayableManager3D->UpdateFromDMML: Scene is not set.");
    return;
    }
  this->Internal->ClearDisplayableNodes();

  vtkDMMLTransformNode* mNode = nullptr;
  std::vector<vtkDMMLNode *> mNodes;
  int nnodes = scene ? scene->GetNodesByClass("vtkDMMLTransformNode", mNodes) : 0;
  for (int i=0; i<nnodes; i++)
    {
    mNode  = vtkDMMLTransformNode::SafeDownCast(mNodes[i]);
    if (mNode && this->Internal->UseDisplayableNode(mNode))
      {
      this->Internal->AddTransformNode(mNode);
      }
    }
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager3D::UnobserveDMMLScene()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager3D::OnDMMLSceneStartClose()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager3D::OnDMMLSceneEndClose()
{
  this->SetUpdateFromDMMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager3D::OnDMMLSceneEndBatchProcess()
{
  this->SetUpdateFromDMMLRequested(true);
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager3D::Create()
{
  Superclass::Create();
  this->SetUpdateFromDMMLRequested(true);
}
