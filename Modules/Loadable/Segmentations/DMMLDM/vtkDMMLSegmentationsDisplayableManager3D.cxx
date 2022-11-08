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

==============================================================================*/

// DMMLDisplayableManager includes
#include "vtkDMMLSegmentationsDisplayableManager3D.h"

// Segmentations includes
#include "vtkDMMLSegmentationNode.h"
#include "vtkDMMLSegmentationDisplayNode.h"

// DMML includes
#include <vtkEventBroker.h>
#include <vtkDMMLFolderDisplayNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLViewNode.h>
#include <vtkDMMLTransformNode.h>

// VTK includes
#include <vtkDataSetAttributes.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp3DCollection.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkCallbackCommand.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkGeneralTransform.h>
#include <vtkCellPicker.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkDMMLSegmentationsDisplayableManager3D );

//---------------------------------------------------------------------------
class vtkDMMLSegmentationsDisplayableManager3D::vtkInternal
{
public:
  vtkInternal(vtkDMMLSegmentationsDisplayableManager3D* external);
  ~vtkInternal();

  struct Pipeline
    {

    Pipeline()
      {
      this->Actor = vtkSmartPointer<vtkActor>::New();
      vtkNew<vtkPolyDataMapper> mapper;
      mapper->SetScalarVisibility(false); // ignore any scalars that an input mesh may contain
      this->Actor->SetMapper(mapper.GetPointer());
      this->Actor->SetVisibility(false);

      this->NodeToWorldTransform = vtkSmartPointer<vtkGeneralTransform>::New();
      this->ModelWarper = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      this->ModelWarper->SetTransform(this->NodeToWorldTransform);
      mapper->SetInputConnection(this->ModelWarper->GetOutputPort());
      }

    vtkSmartPointer<vtkActor> Actor;
    vtkSmartPointer<vtkGeneralTransform> NodeToWorldTransform;
    vtkSmartPointer<vtkTransformPolyDataFilter> ModelWarper;
    };

  typedef std::map<std::string, const Pipeline*> PipelineMapType; // first: segment ID; second: display pipeline
  typedef std::map<vtkDMMLSegmentationDisplayNode*, PipelineMapType> PipelinesCacheType;
  PipelinesCacheType DisplayPipelines;

  typedef std::map < vtkDMMLSegmentationNode*, std::set< vtkDMMLSegmentationDisplayNode* > > SegmentationToDisplayCacheType;
  SegmentationToDisplayCacheType SegmentationToDisplayNodes;

  // Segmentations
  void AddSegmentationNode(vtkDMMLSegmentationNode* displayableNode);
  void RemoveSegmentationNode(vtkDMMLSegmentationNode* displayableNode);

  // Transforms
  void UpdateDisplayableTransforms(vtkDMMLSegmentationNode *node);
  void GetNodeTransformToWorld(vtkDMMLTransformableNode* node, vtkGeneralTransform* transformToWorld);

  // Display Nodes
  void AddDisplayNode(vtkDMMLSegmentationNode*, vtkDMMLSegmentationDisplayNode*);
  Pipeline* CreateSegmentPipeline(std::string segmentID);
  void UpdateDisplayNode(vtkDMMLSegmentationDisplayNode* displayNode);
  void UpdateAllDisplayNodesForSegment(vtkDMMLSegmentationNode* segmentationNode);
  void UpdateSegmentPipelines(vtkDMMLSegmentationDisplayNode* displayNode, PipelineMapType &segmentPipelines);
  void UpdateDisplayNodePipeline(vtkDMMLSegmentationDisplayNode* displayNode, PipelineMapType &segmentPipelines);
  void RemoveDisplayNode(vtkDMMLSegmentationDisplayNode* displayNode);

  // Observations
  void AddObservations(vtkDMMLSegmentationNode* node);
  void RemoveObservations(vtkDMMLSegmentationNode* node);
  bool IsNodeObserved(vtkDMMLSegmentationNode* node);

  // Helper functions
  bool IsVisible(vtkDMMLSegmentationDisplayNode* displayNode);
  bool UseDisplayNode(vtkDMMLSegmentationDisplayNode* displayNode);
  bool UseDisplayableNode(vtkDMMLSegmentationNode* node);
  void ClearDisplayableNodes();

  /// Find picked node from mesh and set PickedNodeID in Internal
  void FindPickedDisplayNodeFromMesh(vtkPointSet* mesh);
  /// Find first picked node from prop3Ds in cell picker and set PickedNodeID in Internal
  void FindFirstPickedDisplayNodeFromPickerProp3Ds();

public:
  /// Picker of segment prop in renderer
  vtkSmartPointer<vtkCellPicker> CellPicker;

  /// Last picked segmentation display node ID
  std::string PickedDisplayNodeID;

  /// Last picked segment ID
  std::string PickedSegmentID;

private:
  vtkDMMLSegmentationsDisplayableManager3D* External;
  bool AddingSegmentationNode;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkDMMLSegmentationsDisplayableManager3D::vtkInternal::vtkInternal(vtkDMMLSegmentationsDisplayableManager3D * external)
: External(external)
, AddingSegmentationNode(false)
{
  this->CellPicker = vtkSmartPointer<vtkCellPicker>::New();
  this->CellPicker->SetTolerance(0.00001);
}

//---------------------------------------------------------------------------
vtkDMMLSegmentationsDisplayableManager3D::vtkInternal::~vtkInternal()
{
  this->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
bool vtkDMMLSegmentationsDisplayableManager3D::vtkInternal::UseDisplayNode(vtkDMMLSegmentationDisplayNode* displayNode)
{
  // Allow segmentations to appear only in designated viewers
  if (displayNode && !displayNode->IsDisplayableInView(this->External->GetDMMLViewNode()->GetID()))
    {
    return false;
    }

  // Check whether DisplayNode should be shown in this view
  bool use = displayNode && displayNode->IsA("vtkDMMLSegmentationDisplayNode");

  return use;
}

//---------------------------------------------------------------------------
bool vtkDMMLSegmentationsDisplayableManager3D::vtkInternal::IsVisible(vtkDMMLSegmentationDisplayNode* displayNode)
{
  return displayNode
    && displayNode->GetVisibility(this->External->GetDMMLViewNode()->GetID())
    && displayNode->GetOpacity3D() > 0;
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationsDisplayableManager3D::vtkInternal::AddSegmentationNode(vtkDMMLSegmentationNode* node)
{
  if (this->AddingSegmentationNode)
    {
    return;
    }
  // Check if node should be used
  if (!this->UseDisplayableNode(node))
    {
    return;
    }

  this->AddingSegmentationNode = true;

  // Add Display Nodes
  int nnodes = node->GetNumberOfDisplayNodes();

  this->AddObservations(node);

  for (int i=0; i<nnodes; i++)
    {
    vtkDMMLSegmentationDisplayNode *dnode = vtkDMMLSegmentationDisplayNode::SafeDownCast(node->GetNthDisplayNode(i));
    if ( this->UseDisplayNode(dnode) )
      {
      this->SegmentationToDisplayNodes[node].insert(dnode);
      this->AddDisplayNode( node, dnode );
      }
    }
  this->AddingSegmentationNode = false;
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationsDisplayableManager3D::vtkInternal::RemoveSegmentationNode(vtkDMMLSegmentationNode* node)
{
  if (!node)
    {
    return;
    }
  vtkInternal::SegmentationToDisplayCacheType::iterator displayableIt =
    this->SegmentationToDisplayNodes.find(node);
  if(displayableIt == this->SegmentationToDisplayNodes.end())
    {
    return;
    }

  std::set<vtkDMMLSegmentationDisplayNode *> dnodes = displayableIt->second;
  std::set<vtkDMMLSegmentationDisplayNode *>::iterator diter;
  for ( diter = dnodes.begin(); diter != dnodes.end(); ++diter)
    {
    this->RemoveDisplayNode(*diter);
    }
  this->RemoveObservations(node);
  this->SegmentationToDisplayNodes.erase(displayableIt);
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationsDisplayableManager3D::vtkInternal::GetNodeTransformToWorld(
  vtkDMMLTransformableNode* node, vtkGeneralTransform* transformToWorld)
{
  if (!node || !transformToWorld)
    {
    return;
    }

  vtkDMMLTransformNode* tnode = node->GetParentTransformNode();

  transformToWorld->Identity();
  if (tnode)
    {
    tnode->GetTransformToWorld(transformToWorld);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationsDisplayableManager3D::vtkInternal::UpdateDisplayableTransforms(vtkDMMLSegmentationNode* mNode)
{
  // Update the pipeline for all tracked DisplayableNode
  PipelinesCacheType::iterator pipelinesIter;
  std::set< vtkDMMLSegmentationDisplayNode* > displayNodes = this->SegmentationToDisplayNodes[mNode];
  std::set< vtkDMMLSegmentationDisplayNode* >::iterator dnodesIter;
  for ( dnodesIter = displayNodes.begin(); dnodesIter != displayNodes.end(); dnodesIter++ )
    {
    if ( ((pipelinesIter = this->DisplayPipelines.find(*dnodesIter)) != this->DisplayPipelines.end()) )
      {
      this->UpdateDisplayNodePipeline(pipelinesIter->first, pipelinesIter->second);
      for (PipelineMapType::iterator pipelineIt=pipelinesIter->second.begin(); pipelineIt!=pipelinesIter->second.end(); ++pipelineIt)
        {
        const Pipeline* currentPipeline = pipelineIt->second;
        vtkNew<vtkGeneralTransform> nodeToWorld;
        this->GetNodeTransformToWorld(mNode, nodeToWorld.GetPointer());
        // It is important to only update the transform if the transform chain is actually changed,
        // because recomputing a non-linear transformation on a complex model may be very time-consuming.
        if (!vtkDMMLTransformNode::AreTransformsEqual(nodeToWorld.GetPointer(), currentPipeline->NodeToWorldTransform))
          {
          currentPipeline->NodeToWorldTransform->DeepCopy(nodeToWorld);
          }
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationsDisplayableManager3D::vtkInternal::RemoveDisplayNode(vtkDMMLSegmentationDisplayNode* displayNode)
{
  PipelinesCacheType::iterator pipelinesIter = this->DisplayPipelines.find(displayNode);
  if (pipelinesIter == this->DisplayPipelines.end())
    {
    return;
    }
  PipelineMapType::iterator pipelineIt;
  for (pipelineIt = pipelinesIter->second.begin(); pipelineIt != pipelinesIter->second.end(); ++pipelineIt)
    {
    const Pipeline* pipeline = pipelineIt->second;
    this->External->GetRenderer()->RemoveActor(pipeline->Actor);
    delete pipeline;
    }
  this->DisplayPipelines.erase(pipelinesIter);
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationsDisplayableManager3D::vtkInternal::AddDisplayNode(vtkDMMLSegmentationNode* mNode, vtkDMMLSegmentationDisplayNode* displayNode)
{
  if (!mNode || !displayNode)
    {
    return;
    }

  // Do not add the display node if it is already associated with a pipeline object.
  // This happens when a segmentation node already associated with a display node
  // is copied into an other (using vtkDMMLNode::Copy()) and is added to the scene afterward.
  // Related issue are #3428 and #2608
  PipelinesCacheType::iterator it;
  it = this->DisplayPipelines.find(displayNode);
  if (it != this->DisplayPipelines.end())
    {
    return;
    }

  // Create pipelines for each segment
  vtkSegmentation* segmentation = mNode->GetSegmentation();
  if (!segmentation)
    {
    return;
    }
  PipelineMapType pipelineVector;
  std::vector< std::string > segmentIDs;
  segmentation->GetSegmentIDs(segmentIDs);
  for (std::vector< std::string >::const_iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt)
    {
    pipelineVector[*segmentIdIt] = this->CreateSegmentPipeline(*segmentIdIt);
    }

  this->DisplayPipelines.insert( std::make_pair(displayNode, pipelineVector) );

  // Update cached matrices. Calls UpdateDisplayNodePipeline
  this->UpdateDisplayableTransforms(mNode);
}

//---------------------------------------------------------------------------
vtkDMMLSegmentationsDisplayableManager3D::vtkInternal::Pipeline*
vtkDMMLSegmentationsDisplayableManager3D::vtkInternal::CreateSegmentPipeline(
    std::string vtkNotUsed(segmentID))
{
  Pipeline* pipeline = new Pipeline();

  // Add actor to Renderer and local cache
  this->External->GetRenderer()->AddActor( pipeline->Actor );

  return pipeline;
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationsDisplayableManager3D::vtkInternal::UpdateDisplayNode(vtkDMMLSegmentationDisplayNode* displayNode)
{
  // If the DisplayNode already exists, just update. Otherwise, add as new node
  if (!displayNode)
    {
    return;
    }
  PipelinesCacheType::iterator displayNodeIt;
  displayNodeIt = this->DisplayPipelines.find(displayNode);
  if (displayNodeIt != this->DisplayPipelines.end())
    {
    this->UpdateSegmentPipelines(displayNode, displayNodeIt->second);
    this->UpdateDisplayNodePipeline(displayNode, displayNodeIt->second);
    }
  else
    {
    this->AddSegmentationNode( vtkDMMLSegmentationNode::SafeDownCast(displayNode->GetDisplayableNode()) );
    }
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationsDisplayableManager3D::vtkInternal::UpdateAllDisplayNodesForSegment(vtkDMMLSegmentationNode* segmentationNode)
{
  std::set<vtkDMMLSegmentationDisplayNode *> displayNodes = this->SegmentationToDisplayNodes[segmentationNode];
  for (std::set<vtkDMMLSegmentationDisplayNode *>::iterator dnodesIter = displayNodes.begin(); dnodesIter != displayNodes.end(); dnodesIter++)
    {
    this->UpdateDisplayNode(*dnodesIter);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationsDisplayableManager3D::vtkInternal::UpdateSegmentPipelines(
  vtkDMMLSegmentationDisplayNode* displayNode, PipelineMapType &segmentPipelines)
{
  // Get segmentation
  vtkDMMLSegmentationNode* segmentationNode = vtkDMMLSegmentationNode::SafeDownCast(displayNode->GetDisplayableNode());
  if (!segmentationNode)
    {
    return;
    }
  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  if (!segmentation)
    {
    return;
    }

  // Make sure each segment has a pipeline
  std::vector< std::string > segmentIDs;
  segmentation->GetSegmentIDs(segmentIDs);
  for (std::vector< std::string >::const_iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt)
    {
    // If segment does not have a pipeline, create one
    PipelineMapType::iterator pipelineIt = segmentPipelines.find(*segmentIdIt);
    if (pipelineIt == segmentPipelines.end())
      {
      segmentPipelines[*segmentIdIt] = this->CreateSegmentPipeline(*segmentIdIt);
      vtkNew<vtkGeneralTransform> nodeToWorld;
      this->GetNodeTransformToWorld(segmentationNode, nodeToWorld.GetPointer());
      // It is important to only update the transform if the transform chain is actually changed,
      // because recomputing a non-linear transformation on a complex model may be very time-consuming.
      if (!vtkDMMLTransformNode::AreTransformsEqual(nodeToWorld.GetPointer(), segmentPipelines[*segmentIdIt]->NodeToWorldTransform))
        {
        segmentPipelines[*segmentIdIt]->NodeToWorldTransform->DeepCopy(nodeToWorld);
        }
      }
    }

  // Make sure each pipeline belongs to an existing segment
  PipelineMapType::iterator pipelineIt = segmentPipelines.begin();
  while (pipelineIt != segmentPipelines.end())
    {
    const Pipeline* pipeline = pipelineIt->second;
    vtkSegment* segment = segmentation->GetSegment(pipelineIt->first);
    if (segment == nullptr)
      {
      PipelineMapType::iterator erasedIt = pipelineIt;
      ++pipelineIt;
      segmentPipelines.erase(erasedIt);
      this->External->GetRenderer()->RemoveActor(pipeline->Actor);
      delete pipeline;
      }
    else
      {
      ++pipelineIt;
      }
    }
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationsDisplayableManager3D::vtkInternal::UpdateDisplayNodePipeline(
  vtkDMMLSegmentationDisplayNode* displayNode, PipelineMapType &segmentPipelines)
{
  // Sets visibility, set pipeline polydata input, update color
  if (!displayNode)
    {
    return;
    }

  // Get display node from hierarchy that applies display properties on branch
  vtkDMMLDisplayableNode* displayableNode = displayNode->GetDisplayableNode();
  vtkDMMLDisplayNode* overrideHierarchyDisplayNode =
    vtkDMMLFolderDisplayNode::GetOverridingHierarchyDisplayNode(displayableNode);
  vtkDMMLDisplayNode* genericDisplayNode = displayNode;

  // Use hierarchy display node if any, and if overriding is allowed for the current display node.
  // If override is explicitly disabled, then do not apply hierarchy visibility or opacity either.
  bool hierarchyVisibility = true;
  double hierarchyOpacity = 1.0;
  if (displayNode->GetFolderDisplayOverrideAllowed())
    {
    if (overrideHierarchyDisplayNode)
      {
      genericDisplayNode = overrideHierarchyDisplayNode;
      }

    // Get visibility and opacity defined by the hierarchy.
    // These two properties are influenced by the hierarchy regardless the fact whether there is override
    // or not. Visibility of items defined by hierarchy is off if any of the ancestors is explicitly hidden,
    // and the opacity is the product of the ancestors' opacities.
    // However, this does not apply on display nodes that do not allow overrides (FolderDisplayOverrideAllowed)
    hierarchyVisibility = vtkDMMLFolderDisplayNode::GetHierarchyVisibility(displayableNode);
    hierarchyOpacity = vtkDMMLFolderDisplayNode::GetHierarchyOpacity(displayableNode);
    }

  bool displayNodeVisible = this->IsVisible(displayNode);

  // Determine which representation to show
  std::string shownRepresentationName = displayNode->GetDisplayRepresentationName3D();
  if (shownRepresentationName.empty())
    {
    // Hide segmentation if there is no poly data representation to show
    for (PipelineMapType::iterator pipelineIt=segmentPipelines.begin(); pipelineIt!=segmentPipelines.end(); ++pipelineIt)
      {
      pipelineIt->second->Actor->SetVisibility(false);
      }
    return;
    }

  // Get segmentation
  vtkDMMLSegmentationNode* segmentationNode = vtkDMMLSegmentationNode::SafeDownCast(displayableNode);
  if (!segmentationNode)
    {
    return;
    }
  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  if (!segmentation)
    {
    return;
    }
  // Make sure the requested representation exists
  if (!segmentation->CreateRepresentation(shownRepresentationName))
    {
    return;
    }

  // For all pipelines (pipeline per segment)
  for (PipelineMapType::iterator pipelineIt=segmentPipelines.begin(); pipelineIt!=segmentPipelines.end(); ++pipelineIt)
    {
    const Pipeline* pipeline = pipelineIt->second;

    // Update visibility
    vtkDMMLSegmentationDisplayNode::SegmentDisplayProperties properties;
    displayNode->GetSegmentDisplayProperties(pipelineIt->first, properties);
    bool segmentVisible = hierarchyVisibility && displayNodeVisible && properties.Visible && properties.Visible3D;
    pipeline->Actor->SetVisibility(segmentVisible);
    if (!segmentVisible)
      {
      continue;
      }

    // Get poly data to display
    vtkPolyData* polyData = vtkPolyData::SafeDownCast(
      segmentation->GetSegmentRepresentation(pipelineIt->first, shownRepresentationName));
    if (!polyData || polyData->GetNumberOfPoints() == 0)
      {
      pipeline->Actor->SetVisibility(false);
      continue;
      }

    // Set node to world transform to identity by default
    // For all pipelines (pipeline per segment)
    vtkNew<vtkGeneralTransform> nodeToWorld;
    this->GetNodeTransformToWorld(segmentationNode, nodeToWorld.GetPointer());
    if (!vtkDMMLTransformNode::AreTransformsEqual(nodeToWorld.GetPointer(), pipeline->NodeToWorldTransform))
      {
      pipeline->NodeToWorldTransform->DeepCopy(nodeToWorld);
      }
    if (pipeline->ModelWarper->GetInputDataObject(0, 0) != polyData)
      {
      pipeline->ModelWarper->SetInputData(polyData);
      }

    // Get displayed color (if no override is defined then use the color from the segment)
    double color[3] = {vtkSegment::SEGMENT_COLOR_INVALID[0], vtkSegment::SEGMENT_COLOR_INVALID[1], vtkSegment::SEGMENT_COLOR_INVALID[2]};
    if (overrideHierarchyDisplayNode)
      {
      overrideHierarchyDisplayNode->GetColor(color);
      }
    else
      {
      displayNode->GetSegmentColor(pipelineIt->first, color);
      }

    // Update pipeline actor
    pipeline->Actor->GetProperty()->SetRepresentation(genericDisplayNode->GetRepresentation());
    pipeline->Actor->GetProperty()->SetPointSize(genericDisplayNode->GetPointSize());
    pipeline->Actor->GetProperty()->SetLineWidth(genericDisplayNode->GetLineWidth());
    pipeline->Actor->GetProperty()->SetLighting(genericDisplayNode->GetLighting());
    pipeline->Actor->GetProperty()->SetInterpolation(genericDisplayNode->GetInterpolation());
    pipeline->Actor->GetProperty()->SetShading(genericDisplayNode->GetShading());
    pipeline->Actor->GetProperty()->SetFrontfaceCulling(genericDisplayNode->GetFrontfaceCulling());
    pipeline->Actor->GetProperty()->SetBackfaceCulling(genericDisplayNode->GetBackfaceCulling());

    pipeline->Actor->GetProperty()->SetColor(color[0], color[1], color[2]);
    pipeline->Actor->GetProperty()->SetOpacity(
      hierarchyOpacity * properties.Opacity3D * displayNode->GetOpacity3D() * genericDisplayNode->GetOpacity());

    pipeline->Actor->SetPickable(segmentationNode->GetSelectable());
    if (genericDisplayNode->GetSelected())
      {
      pipeline->Actor->GetProperty()->SetAmbient(genericDisplayNode->GetSelectedAmbient());
      pipeline->Actor->GetProperty()->SetSpecular(genericDisplayNode->GetSelectedSpecular());
      }
    else
      {
      pipeline->Actor->GetProperty()->SetAmbient(genericDisplayNode->GetAmbient());
      pipeline->Actor->GetProperty()->SetSpecular(genericDisplayNode->GetSpecular());
      }
    pipeline->Actor->GetProperty()->SetDiffuse(genericDisplayNode->GetDiffuse());
    pipeline->Actor->GetProperty()->SetSpecularPower(genericDisplayNode->GetPower());
    pipeline->Actor->GetProperty()->SetMetallic(displayNode->GetMetallic());
    pipeline->Actor->GetProperty()->SetRoughness(displayNode->GetRoughness());
    pipeline->Actor->GetProperty()->SetEdgeVisibility(genericDisplayNode->GetEdgeVisibility());
    pipeline->Actor->GetProperty()->SetEdgeColor(genericDisplayNode->GetEdgeColor());

    pipeline->Actor->SetTexture(nullptr);
  }
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationsDisplayableManager3D::vtkInternal::AddObservations(vtkDMMLSegmentationNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  if (!broker->GetObservationExist(node, vtkCommand::ModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand()))
    {
    broker->AddObservation(node, vtkCommand::ModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand());
    }
  if (!broker->GetObservationExist(node, vtkDMMLDisplayableNode::TransformModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand()))
    {
    broker->AddObservation(node, vtkDMMLDisplayableNode::TransformModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand() );
    }
  if (!broker->GetObservationExist(node, vtkDMMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkDMMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand() );
    }
  if (!broker->GetObservationExist(node, vtkSegmentation::RepresentationModified, this->External, this->External->GetDMMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkSegmentation::RepresentationModified, this->External, this->External->GetDMMLNodesCallbackCommand() );
    }
  if (!broker->GetObservationExist(node, vtkSegmentation::SegmentAdded, this->External, this->External->GetDMMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkSegmentation::SegmentAdded, this->External, this->External->GetDMMLNodesCallbackCommand() );
    }
  if (!broker->GetObservationExist(node, vtkSegmentation::SegmentRemoved, this->External, this->External->GetDMMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkSegmentation::SegmentRemoved, this->External, this->External->GetDMMLNodesCallbackCommand() );
    }
  if (!broker->GetObservationExist(node, vtkSegmentation::SegmentModified, this->External, this->External->GetDMMLNodesCallbackCommand()))
    {
    broker->AddObservation(node, vtkSegmentation::SegmentModified, this->External, this->External->GetDMMLNodesCallbackCommand());
    }
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationsDisplayableManager3D::vtkInternal::RemoveObservations(vtkDMMLSegmentationNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkEventBroker::ObservationVector observations;
  observations = broker->GetObservations(
    node, vtkCommand::ModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand());
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(
    node, vtkDMMLTransformableNode::TransformModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand());
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(
    node, vtkDMMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(
    node, vtkSegmentation::RepresentationModified, this->External, this->External->GetDMMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(
    node, vtkSegmentation::SegmentAdded, this->External, this->External->GetDMMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(
    node, vtkSegmentation::SegmentRemoved, this->External, this->External->GetDMMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(
    node, vtkSegmentation::SegmentModified, this->External, this->External->GetDMMLNodesCallbackCommand());
  broker->RemoveObservations(observations);
}

//---------------------------------------------------------------------------
bool vtkDMMLSegmentationsDisplayableManager3D::vtkInternal::IsNodeObserved(vtkDMMLSegmentationNode* node)
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
void vtkDMMLSegmentationsDisplayableManager3D::vtkInternal::ClearDisplayableNodes()
{
  while(this->SegmentationToDisplayNodes.size() > 0)
    {
    this->RemoveSegmentationNode(this->SegmentationToDisplayNodes.begin()->first);
    }
}

//---------------------------------------------------------------------------
bool vtkDMMLSegmentationsDisplayableManager3D::vtkInternal::UseDisplayableNode(vtkDMMLSegmentationNode* node)
{
  bool use = node && node->IsA("vtkDMMLSegmentationNode");
  return use;
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationsDisplayableManager3D::vtkInternal::FindPickedDisplayNodeFromMesh(vtkPointSet* mesh)
{
  this->PickedDisplayNodeID = "";
  this->PickedSegmentID = "";
  if (!mesh)
    {
    return;
    }

  PipelinesCacheType::iterator pipelinesIt;
  for (pipelinesIt = this->DisplayPipelines.begin(); pipelinesIt!=this->DisplayPipelines.end(); ++pipelinesIt)
    {
    vtkDMMLSegmentationDisplayNode* currentDisplayNode = pipelinesIt->first;
    for (PipelineMapType::iterator pipelineIt=pipelinesIt->second.begin(); pipelineIt!=pipelinesIt->second.end(); ++pipelineIt)
      {
      if (pipelineIt->second->ModelWarper->GetOutput() == mesh)
        {
        this->PickedDisplayNodeID = currentDisplayNode->GetID();
        this->PickedSegmentID = pipelineIt->first;
        return; // Display node and segment found
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationsDisplayableManager3D::vtkInternal::FindFirstPickedDisplayNodeFromPickerProp3Ds()
{
  this->PickedDisplayNodeID = "";
  this->PickedSegmentID = "";
  if (!this->CellPicker)
    {
    return;
    }

  vtkProp3DCollection* props = this->CellPicker->GetProp3Ds();
  for (int propIndex=0; propIndex<props->GetNumberOfItems(); ++propIndex)
    {
    vtkProp3D* pickedProp = vtkProp3D::SafeDownCast(props->GetItemAsObject(propIndex));
    if (!pickedProp)
      {
      continue;
      }

    PipelinesCacheType::iterator pipelinesIt;
    for (pipelinesIt = this->DisplayPipelines.begin(); pipelinesIt!=this->DisplayPipelines.end(); ++pipelinesIt)
      {
      vtkDMMLSegmentationDisplayNode* currentDisplayNode = pipelinesIt->first;
      for (PipelineMapType::iterator pipelineIt=pipelinesIt->second.begin(); pipelineIt!=pipelinesIt->second.end(); ++pipelineIt)
        {
        if (pipelineIt->second->Actor.GetPointer() == pickedProp)
          {
          this->PickedDisplayNodeID = currentDisplayNode->GetID();
          this->PickedSegmentID = pipelineIt->first;
          return; // Display node and segment found
          }
        }
      }
    }
}


//---------------------------------------------------------------------------
// vtkDMMLSegmentationsDisplayableManager3D methods

//---------------------------------------------------------------------------
vtkDMMLSegmentationsDisplayableManager3D::vtkDMMLSegmentationsDisplayableManager3D()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkDMMLSegmentationsDisplayableManager3D::~vtkDMMLSegmentationsDisplayableManager3D()
{
  delete this->Internal;
  this->Internal=nullptr;
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationsDisplayableManager3D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf ( os, indent );
  os << indent << "vtkDMMLSegmentationsDisplayableManager3D: " << this->GetClassName() << "\n";
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationsDisplayableManager3D::OnDMMLSceneNodeAdded(vtkDMMLNode* node)
{
  if ( !node->IsA("vtkDMMLSegmentationNode") )
    {
    return;
    }

  // Escape if the scene is being closed, imported or connected
  if (this->GetDMMLScene()->IsBatchProcessing())
    {
    this->SetUpdateFromDMMLRequested(true);
    return;
    }

  this->Internal->AddSegmentationNode(vtkDMMLSegmentationNode::SafeDownCast(node));
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationsDisplayableManager3D::OnDMMLSceneNodeRemoved(vtkDMMLNode* node)
{
  if ( node
    && (!node->IsA("vtkDMMLSegmentationNode"))
    && (!node->IsA("vtkDMMLSegmentationDisplayNode")) )
    {
    return;
    }

  vtkDMMLSegmentationNode* segmentationNode = nullptr;
  vtkDMMLSegmentationDisplayNode* displayNode = nullptr;

  bool modified = false;
  if ( (segmentationNode = vtkDMMLSegmentationNode::SafeDownCast(node)) )
    {
    this->Internal->RemoveSegmentationNode(segmentationNode);
    modified = true;
    }
  else if ( (displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(node)) )
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
void vtkDMMLSegmentationsDisplayableManager3D::ProcessDMMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  vtkDMMLScene* scene = this->GetDMMLScene();

  if (scene == nullptr || scene->IsBatchProcessing())
    {
    return;
    }

  vtkDMMLSegmentationNode* displayableNode = vtkDMMLSegmentationNode::SafeDownCast(caller);
  if (displayableNode)
    {
    if (event == vtkDMMLDisplayableNode::DisplayModifiedEvent)
      {
      vtkDMMLNode* callDataNode = reinterpret_cast<vtkDMMLDisplayNode *> (callData);
      vtkDMMLSegmentationDisplayNode* displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(callDataNode);
      if (displayNode)
        {
        this->Internal->UpdateDisplayNode(displayNode);
        this->RequestRender();
        }
      }
    else if ( (event == vtkDMMLDisplayableNode::TransformModifiedEvent)
           || (event == vtkDMMLTransformableNode::TransformModifiedEvent)
           || (event == vtkSegmentation::RepresentationModified)
           || (event == vtkSegmentation::SegmentModified) )
      {
      this->Internal->UpdateDisplayableTransforms(displayableNode);
      this->RequestRender();
      }
    else if ( (event == vtkCommand::ModifiedEvent) // segmentation object may be replaced
           || (event == vtkSegmentation::SegmentAdded)
           || (event == vtkSegmentation::SegmentRemoved) )
      {
      this->Internal->UpdateAllDisplayNodesForSegment(displayableNode);
      this->RequestRender();
      }
    }
  else
    {
    this->Superclass::ProcessDMMLNodesEvents(caller, event, callData);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationsDisplayableManager3D::UpdateFromDMML()
{
  this->SetUpdateFromDMMLRequested(false);

  vtkDMMLScene* scene = this->GetDMMLScene();
  if (!scene)
    {
    vtkDebugMacro( "vtkDMMLSegmentationsDisplayableManager3D::UpdateFromDMML: Scene is not set");
    return;
    }
  this->Internal->ClearDisplayableNodes();

  vtkDMMLSegmentationNode* segmentationNode = nullptr;
  std::vector<vtkDMMLNode*> segmentationNodes;
  int numOfSegmentationNodes = scene ? scene->GetNodesByClass("vtkDMMLSegmentationNode", segmentationNodes) : 0;
  for (int i=0; i<numOfSegmentationNodes; i++)
    {
    segmentationNode = vtkDMMLSegmentationNode::SafeDownCast(segmentationNodes[i]);
    if (segmentationNode && this->Internal->UseDisplayableNode(segmentationNode))
      {
      this->Internal->AddSegmentationNode(segmentationNode);
      }
    }
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationsDisplayableManager3D::UnobserveDMMLScene()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationsDisplayableManager3D::OnDMMLSceneStartClose()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationsDisplayableManager3D::OnDMMLSceneEndClose()
{
  this->SetUpdateFromDMMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationsDisplayableManager3D::OnDMMLSceneEndBatchProcess()
{
  this->SetUpdateFromDMMLRequested(true);
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationsDisplayableManager3D::Create()
{
  Superclass::Create();
  this->SetUpdateFromDMMLRequested(true);
}

//---------------------------------------------------------------------------
int vtkDMMLSegmentationsDisplayableManager3D::Pick3D(double ras[3])
{
  this->Internal->PickedDisplayNodeID = "";
  this->Internal->PickedSegmentID = "";

  vtkRenderer* ren = this->GetRenderer();
  if (!ren)
    {
    vtkErrorMacro("Pick3D: Unable to get renderer");
    return 0;
    }

  if (this->Internal->CellPicker->Pick3DPoint(ras, ren))
    {
    // Find first picked segmentation and segment from picker
    // Note: Getting the mesh using GetDataSet is not a good solution as the dataset is the first
    //   one that is picked and it may be of different type (volume, model, etc.)
    this->Internal->FindFirstPickedDisplayNodeFromPickerProp3Ds();
    }

  return 1;
}

//---------------------------------------------------------------------------
const char* vtkDMMLSegmentationsDisplayableManager3D::GetPickedNodeID()
{
  return this->Internal->PickedDisplayNodeID.c_str();
}

//---------------------------------------------------------------------------
const char* vtkDMMLSegmentationsDisplayableManager3D::GetPickedSegmentID()
{
  return this->Internal->PickedSegmentID.c_str();
}
