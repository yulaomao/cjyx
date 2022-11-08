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
#include "vtkDMMLTransformsDisplayableManager2D.h"

#include "vtkCjyxTransformLogic.h"

// DMML includes
#include <vtkDMMLMarkupsNode.h>
#include <vtkDMMLProceduralColorNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceCompositeNode.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLTransformDisplayNode.h>
#include <vtkDMMLTransformNode.h>

// VTK includes
#include <vtkActor2D.h>
#include <vtkCallbackCommand.h>
#include <vtkColorTransferFunction.h>
#include <vtkEventBroker.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkWeakPointer.h>
#include <vtkPointLocator.h>

// STD includes
#include <algorithm>
#include <cassert>
#include <set>
#include <map>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLTransformsDisplayableManager2D );

//---------------------------------------------------------------------------
class vtkDMMLTransformsDisplayableManager2D::vtkInternal
{
public:

  vtkInternal( vtkDMMLTransformsDisplayableManager2D* external );
  ~vtkInternal();
  struct Pipeline
    {
    vtkSmartPointer<vtkProp> Actor;
    vtkSmartPointer<vtkTransform> TransformToSlice;
    vtkSmartPointer<vtkTransformPolyDataFilter> Transformer;
    };

  typedef std::map < vtkDMMLTransformDisplayNode*, const Pipeline* > PipelinesCacheType;
  PipelinesCacheType DisplayPipelines;

  typedef std::map < vtkDMMLTransformNode*, std::set< vtkDMMLTransformDisplayNode* > > TransformToDisplayCacheType;
  TransformToDisplayCacheType TransformToDisplayNodes;

  // Transforms
  void AddTransformNode(vtkDMMLTransformNode* displayableNode);
  void RemoveTransformNode(vtkDMMLTransformNode* displayableNode);
  void UpdateDisplayableTransforms(vtkDMMLTransformNode *node);

  // Slice Node
  void SetSliceNode(vtkDMMLSliceNode* sliceNode);
  void UpdateSliceNode();

  // Display Nodes
  void AddDisplayNode(vtkDMMLTransformNode*, vtkDMMLTransformDisplayNode*);
  void UpdateDisplayNode(vtkDMMLTransformDisplayNode* displayNode);
  void UpdateDisplayNodePipeline(vtkDMMLTransformDisplayNode*, const Pipeline*);
  void RemoveDisplayNode(vtkDMMLTransformDisplayNode* displayNode);

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
  vtkDMMLTransformsDisplayableManager2D* External;
  bool AddingTransformNode;
  vtkSmartPointer<vtkDMMLSliceNode> SliceNode;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkDMMLTransformsDisplayableManager2D::vtkInternal::vtkInternal(vtkDMMLTransformsDisplayableManager2D* external)
: External(external)
, AddingTransformNode(false)
{
}

//---------------------------------------------------------------------------
vtkDMMLTransformsDisplayableManager2D::vtkInternal::~vtkInternal()
{
  this->ClearDisplayableNodes();
  this->SliceNode = nullptr;
}

//---------------------------------------------------------------------------
bool vtkDMMLTransformsDisplayableManager2D::vtkInternal::UseDisplayNode(vtkDMMLTransformDisplayNode* displayNode)
{
   // allow annotations to appear only in designated viewers
  if (displayNode && !displayNode->IsDisplayableInView(this->SliceNode->GetID()))
    {
    return false;
    }

  // Check whether DisplayNode should be shown in this view
  bool use = displayNode && displayNode->IsA("vtkDMMLTransformDisplayNode");

  return use;
}

//---------------------------------------------------------------------------
bool vtkDMMLTransformsDisplayableManager2D::vtkInternal::IsVisible(vtkDMMLTransformDisplayNode* displayNode)
{
  return displayNode && (displayNode->GetVisibility() != 0) && (displayNode->GetVisibility2D() != 0);
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager2D::vtkInternal::SetSliceNode(vtkDMMLSliceNode* sliceNode)
{
  if (!sliceNode || this->SliceNode == sliceNode)
    {
    return;
    }
  this->SliceNode=sliceNode;
  this->UpdateSliceNode();
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager2D::vtkInternal::UpdateSliceNode()
{
  // Update the Slice node transform

  PipelinesCacheType::iterator it;
  for (it = this->DisplayPipelines.begin(); it != this->DisplayPipelines.end(); ++it)
    {
    this->UpdateDisplayNodePipeline(it->first, it->second);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager2D::vtkInternal::AddTransformNode(vtkDMMLTransformNode* node)
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
void vtkDMMLTransformsDisplayableManager2D::vtkInternal::RemoveTransformNode(vtkDMMLTransformNode* node)
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

  std::set< vtkDMMLTransformDisplayNode* > dnodes = displayableIt->second;
  std::set< vtkDMMLTransformDisplayNode* >::iterator diter;
  for ( diter = dnodes.begin(); diter != dnodes.end(); ++diter)
    {
    this->RemoveDisplayNode(*diter);
    }
  this->RemoveObservations(node);
  this->TransformToDisplayNodes.erase(displayableIt);
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager2D::vtkInternal::UpdateDisplayableTransforms(vtkDMMLTransformNode* mNode)
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
void vtkDMMLTransformsDisplayableManager2D::vtkInternal::RemoveDisplayNode(vtkDMMLTransformDisplayNode* displayNode)
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
void vtkDMMLTransformsDisplayableManager2D::vtkInternal::AddDisplayNode(vtkDMMLTransformNode* mNode, vtkDMMLTransformDisplayNode* displayNode)
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

  vtkNew<vtkActor2D> actor;
  if (displayNode->IsA("vtkDMMLTransformDisplayNode"))
    {
    actor->SetMapper( vtkNew<vtkPolyDataMapper2D>().GetPointer() );
    }

  // Create pipeline
  Pipeline* pipeline = new Pipeline();
  pipeline->Actor = actor.GetPointer();
  pipeline->TransformToSlice = vtkSmartPointer<vtkTransform>::New();
  pipeline->Transformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();

  // Set up pipeline
  pipeline->Transformer->SetTransform(pipeline->TransformToSlice);
  pipeline->Actor->SetVisibility(0);

  // Add actor to Renderer and local cache
  this->External->GetRenderer()->AddActor( pipeline->Actor );
  this->DisplayPipelines.insert( std::make_pair(displayNode, pipeline) );

  // Update cached matrices. Calls UpdateDisplayNodePipeline
  this->UpdateDisplayableTransforms(mNode);
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager2D::vtkInternal::UpdateDisplayNode(vtkDMMLTransformDisplayNode* displayNode)
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
void vtkDMMLTransformsDisplayableManager2D::vtkInternal::UpdateDisplayNodePipeline(vtkDMMLTransformDisplayNode* displayNode, const Pipeline* pipeline)
{
  // Sets visibility, set pipeline polydata input, update color
  //   calculate and set pipeline transforms.

  if (!displayNode || !pipeline)
    {
    return;
    }

  // Update visibility
  bool visible = this->IsVisible(displayNode);
  pipeline->Actor->SetVisibility(visible);
  if (!visible)
    {
    return;
    }

  vtkDMMLTransformDisplayNode* transformDisplayNode = vtkDMMLTransformDisplayNode::SafeDownCast(displayNode);

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  vtkDMMLMarkupsNode* glyphPointsNode = vtkDMMLMarkupsNode::SafeDownCast(displayNode->GetGlyphPointsNode());
  vtkCjyxTransformLogic::GetVisualization2d(polyData, transformDisplayNode, this->SliceNode, glyphPointsNode);

  pipeline->Transformer->SetInputData(polyData);

  if (polyData->GetNumberOfPoints()==0)
    {
    return;
    }

  // Set PolyData Transform
  vtkNew<vtkMatrix4x4> rasToXY;
  vtkMatrix4x4::Invert(this->SliceNode->GetXYToRAS(), rasToXY.GetPointer());
  pipeline->TransformToSlice->SetMatrix(rasToXY.GetPointer());

  // Update pipeline actor
  vtkActor2D* actor = vtkActor2D::SafeDownCast(pipeline->Actor);
  vtkPolyDataMapper2D* mapper = vtkPolyDataMapper2D::SafeDownCast(actor->GetMapper());
  mapper->SetInputConnection( pipeline->Transformer->GetOutputPort() );

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
      mapper->ColorByArrayComponent(const_cast<char*>(vtkCjyxTransformLogic::GetVisualizationDisplacementMagnitudeScalarName()),0);
      mapper->UseLookupTableScalarRangeOff();
      mapper->SetScalarRange(displayNode->GetScalarRange());
      scalarVisibility = true;
      }
    }
  mapper->SetScalarVisibility(scalarVisibility);

  actor->SetPosition(0,0);
  vtkProperty2D* actorProperties = actor->GetProperty();
  actorProperties->SetColor(displayNode->GetColor() );
  actorProperties->SetLineWidth(displayNode->GetSliceIntersectionThickness() );
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager2D::vtkInternal::AddObservations(vtkDMMLTransformNode* node)
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
void vtkDMMLTransformsDisplayableManager2D::vtkInternal::RemoveObservations(vtkDMMLTransformNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkEventBroker::ObservationVector observations;
  observations = broker->GetObservations(node, vtkDMMLTransformableNode::TransformModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(node, vtkDMMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
}

//---------------------------------------------------------------------------
bool vtkDMMLTransformsDisplayableManager2D::vtkInternal::IsNodeObserved(vtkDMMLTransformNode* node)
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
void vtkDMMLTransformsDisplayableManager2D::vtkInternal::ClearDisplayableNodes()
{
  while(this->TransformToDisplayNodes.size() > 0)
    {
    this->RemoveTransformNode(this->TransformToDisplayNodes.begin()->first);
    }
}

//---------------------------------------------------------------------------
bool vtkDMMLTransformsDisplayableManager2D::vtkInternal::UseDisplayableNode(vtkDMMLTransformNode* node)
{
  bool use = node && node->IsA("vtkDMMLTransformNode");
  return use;
}

//---------------------------------------------------------------------------
// vtkDMMLTransformsDisplayableManager2D methods

//---------------------------------------------------------------------------
vtkDMMLTransformsDisplayableManager2D::vtkDMMLTransformsDisplayableManager2D()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkDMMLTransformsDisplayableManager2D::~vtkDMMLTransformsDisplayableManager2D()
{
  delete this->Internal;
  this->Internal=nullptr;
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager2D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "vtkDMMLTransformsDisplayableManager2D: " << this->GetClassName() << "\n";
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager2D::OnDMMLSceneNodeAdded(vtkDMMLNode* node)
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
void vtkDMMLTransformsDisplayableManager2D::OnDMMLSceneNodeRemoved(vtkDMMLNode* node)
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
void vtkDMMLTransformsDisplayableManager2D::ProcessDMMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
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
  else if ( vtkDMMLSliceNode::SafeDownCast(caller) )
      {
      this->Internal->UpdateSliceNode();
      this->RequestRender();
      }
  else
    {
    this->Superclass::ProcessDMMLNodesEvents(caller, event, callData);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager2D::UpdateFromDMML()
{
  this->SetUpdateFromDMMLRequested(false);

  vtkDMMLScene* scene = this->GetDMMLScene();
  if (!scene)
    {
    vtkDebugMacro( "vtkDMMLTransformsDisplayableManager2D->UpdateFromDMML: Scene is not set.");
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
void vtkDMMLTransformsDisplayableManager2D::UnobserveDMMLScene()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager2D::OnDMMLSceneStartClose()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager2D::OnDMMLSceneEndClose()
{
  this->SetUpdateFromDMMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager2D::OnDMMLSceneEndBatchProcess()
{
  this->SetUpdateFromDMMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkDMMLTransformsDisplayableManager2D::Create()
{
  this->Internal->SetSliceNode(this->GetDMMLSliceNode());
  this->SetUpdateFromDMMLRequested(true);
}
