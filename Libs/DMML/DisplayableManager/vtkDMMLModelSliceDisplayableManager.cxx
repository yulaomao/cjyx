/*==============================================================================

  Program: 3D Cjyx

  Copyright (c)

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// DMMLDisplayableManager includes
#include "vtkDMMLModelSliceDisplayableManager.h"
#include "vtkDMMLModelDisplayableManager.h"

// DMML includes
#include <vtkDMMLApplicationLogic.h>
#include <vtkDMMLColorNode.h>
#include <vtkDMMLDisplayNode.h>
#include <vtkDMMLDisplayableNode.h>
#include <vtkDMMLFolderDisplayNode.h>
#include <vtkDMMLModelDisplayNode.h>
#include <vtkDMMLModelNode.h>
#include <vtkDMMLProceduralColorNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceCompositeNode.h>
#include <vtkDMMLSliceLogic.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLTransformNode.h>

// VTK includes
#include <vtkActor2D.h>
#include <vtkAlgorithmOutput.h>
#include <vtkCallbackCommand.h>
#include <vtkColorTransferFunction.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkEventBroker.h>
#include <vtkGeneralTransform.h>
#include <vtkLookupTable.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkPointLocator.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkWeakPointer.h>

// VTK includes: customization
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkPlaneCutter.h>
#include <vtkSampleImplicitFunctionFilter.h>

// STD includes
#include <algorithm>
#include <cassert>
#include <set>
#include <map>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLModelSliceDisplayableManager );

//---------------------------------------------------------------------------
class vtkDMMLModelSliceDisplayableManager::vtkInternal
{
public:
  struct Pipeline
    {
    vtkSmartPointer<vtkGeneralTransform> NodeToWorld;
    vtkSmartPointer<vtkTransform> TransformToSlice;
    vtkSmartPointer<vtkTransformPolyDataFilter> Transformer;
    vtkSmartPointer<vtkDataSetSurfaceFilter> SurfaceExtractor;
    vtkSmartPointer<vtkTransformFilter> ModelWarper;
    vtkSmartPointer<vtkPlane> Plane;
    vtkSmartPointer<vtkPlaneCutter> Cutter;
    vtkSmartPointer<vtkCompositeDataGeometryFilter> GeometryFilter; // appends multiple cut pieces into a single polydata
    vtkSmartPointer<vtkSampleImplicitFunctionFilter> SliceDistance;
    vtkSmartPointer<vtkProp> Actor;
    };

  typedef std::map < vtkDMMLDisplayNode*, const Pipeline* > PipelinesCacheType;
  PipelinesCacheType DisplayPipelines;

  typedef std::map < vtkDMMLDisplayableNode*, std::set< vtkDMMLDisplayNode* > > ModelToDisplayCacheType;
  ModelToDisplayCacheType ModelToDisplayNodes;

  // Transforms
  void UpdateDisplayableTransforms(vtkDMMLDisplayableNode *node);
  void GetNodeTransformToWorld(vtkDMMLTransformableNode* node, vtkGeneralTransform* transformToWorld);
  // Slice Node
  void SetSliceNode(vtkDMMLSliceNode* sliceNode);
  void UpdateSliceNode();
  void SetSlicePlaneFromMatrix(vtkMatrix4x4* matrix, vtkPlane* plane);

  // Display Nodes
  void AddDisplayNode(vtkDMMLDisplayableNode*, vtkDMMLDisplayNode*);
  void UpdateDisplayNode(vtkDMMLDisplayNode* displayNode);
  void UpdateDisplayNodePipeline(vtkDMMLDisplayNode*, const Pipeline*);
  void RemoveDisplayNode(vtkDMMLDisplayNode* displayNode);

  // Observations
  void AddObservations(vtkDMMLDisplayableNode* node);
  void RemoveObservations(vtkDMMLDisplayableNode* node);
  bool IsNodeObserved(vtkDMMLDisplayableNode* node);

  // Helper functions
  bool IsVisible(vtkDMMLDisplayNode* displayNode);
  bool UseDisplayNode(vtkDMMLDisplayNode* displayNode);
  bool UseDisplayableNode(vtkDMMLDisplayableNode* displayNode);
  void ClearDisplayableNodes();

  vtkInternal( vtkDMMLModelSliceDisplayableManager* external );
  ~vtkInternal();

private:
  vtkSmartPointer<vtkMatrix4x4> SliceXYToRAS;
  vtkSmartPointer<vtkDMMLSliceNode> SliceNode;
  vtkDMMLModelSliceDisplayableManager* External;
};

//---------------------------------------------------------------------------
// vtkInternal methods
vtkDMMLModelSliceDisplayableManager::vtkInternal
::vtkInternal(vtkDMMLModelSliceDisplayableManager* external)
{
  this->External = external;
  this->SliceXYToRAS = vtkSmartPointer<vtkMatrix4x4>::New();
  this->SliceXYToRAS->Identity();
}

//---------------------------------------------------------------------------
vtkDMMLModelSliceDisplayableManager::vtkInternal
::~vtkInternal()
{
  this->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
bool vtkDMMLModelSliceDisplayableManager::vtkInternal::UseDisplayNode(vtkDMMLDisplayNode* displayNode)
{
  // Check whether DisplayNode should be shown in this view
  bool show = displayNode
              && displayNode->IsA("vtkDMMLModelDisplayNode")
              && ( !displayNode->IsA("vtkDMMLFiberBundleLineDisplayNode") )
              && ( !displayNode->IsA("vtkDMMLFiberBundleGlyphDisplayNode") ) ;
  return show;
}

//---------------------------------------------------------------------------
bool vtkDMMLModelSliceDisplayableManager::vtkInternal::IsVisible(vtkDMMLDisplayNode* displayNode)
{
  if (!displayNode)
    {
    return false;
    }
  if (vtkDMMLSliceLogic::IsSliceModelDisplayNode(displayNode))
    {
    // slice intersections are displayed by vtkDMMLCrosshairDisplayableManager
    return false;
    }
  bool visibleOnNode = true;
  if (this->SliceNode)
    {
    visibleOnNode = displayNode->GetVisibility() && displayNode->IsDisplayableInView(this->SliceNode->GetID());
    }
  else
    {
    visibleOnNode = (displayNode->GetVisibility() == 1);
    }
  return visibleOnNode && (displayNode->GetVisibility2D() != 0) ;
}

//---------------------------------------------------------------------------
void vtkDMMLModelSliceDisplayableManager::vtkInternal
::SetSliceNode(vtkDMMLSliceNode* sliceNode)
{
  if (!sliceNode || this->SliceNode == sliceNode)
    {
    return;
    }
  this->SliceNode = sliceNode;
  this->UpdateSliceNode();
}

//---------------------------------------------------------------------------
void vtkDMMLModelSliceDisplayableManager::vtkInternal
::UpdateSliceNode()
{
  // Update the Slice node transform
  //   then update the DisplayNode pipelines to account for plane location

  this->SliceXYToRAS->DeepCopy( this->SliceNode->GetXYToRAS() );
  PipelinesCacheType::iterator it;
  for (it = this->DisplayPipelines.begin(); it != this->DisplayPipelines.end(); ++it)
    {
    this->UpdateDisplayNodePipeline(it->first, it->second);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLModelSliceDisplayableManager::vtkInternal
::SetSlicePlaneFromMatrix(vtkMatrix4x4* sliceMatrix, vtkPlane* plane)
{
  double normal[3];
  double origin[3];

  // +/-1: orientation of the normal
  const int planeOrientation = 1;
  for (int i = 0; i < 3; i++)
    {
    normal[i] = planeOrientation * sliceMatrix->GetElement(i,2);
    origin[i] = sliceMatrix->GetElement(i,3);
    }

  vtkMath::Normalize(normal);
  plane->SetNormal(normal);
  plane->SetOrigin(origin);
}

//---------------------------------------------------------------------------
void vtkDMMLModelSliceDisplayableManager::vtkInternal
::GetNodeTransformToWorld(vtkDMMLTransformableNode* node, vtkGeneralTransform* transformToWorld)
{
  if (!node || !transformToWorld)
    {
    return;
    }

  vtkDMMLTransformNode* tnode =
    node->GetParentTransformNode();

  transformToWorld->Identity();
  if (tnode)
    {
    tnode->GetTransformToWorld(transformToWorld);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLModelSliceDisplayableManager::vtkInternal
::UpdateDisplayableTransforms(vtkDMMLDisplayableNode* mNode)
{
  // Update the NodeToWorld matrix for all tracked DisplayableNode

  PipelinesCacheType::iterator pipelinesIter;
  std::set<vtkDMMLDisplayNode *> displayNodes = this->ModelToDisplayNodes[mNode];
  std::set<vtkDMMLDisplayNode *>::iterator dnodesIter;
  for ( dnodesIter = displayNodes.begin(); dnodesIter != displayNodes.end(); dnodesIter++ )
    {
    if ( ((pipelinesIter = this->DisplayPipelines.find(*dnodesIter)) != this->DisplayPipelines.end()) )
      {
      this->GetNodeTransformToWorld( mNode, pipelinesIter->second->NodeToWorld);
      this->UpdateDisplayNodePipeline(pipelinesIter->first, pipelinesIter->second);
      }
    }
}

//---------------------------------------------------------------------------
void vtkDMMLModelSliceDisplayableManager::vtkInternal
::RemoveDisplayNode(vtkDMMLDisplayNode* displayNode)
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
void vtkDMMLModelSliceDisplayableManager::vtkInternal
::AddDisplayNode(vtkDMMLDisplayableNode* mNode, vtkDMMLDisplayNode* displayNode)
{
  if (!mNode || !displayNode)
    {
    return;
    }

  // Do not add the display node if it is already associated with a pipeline object.
  // This happens when a model node already associated with a display node
  // is copied into an other (using vtkDMMLNode::Copy()) and is added to the scene afterward.
  // Related issue are #3428 and #2608
  PipelinesCacheType::iterator it;
  it = this->DisplayPipelines.find(displayNode);
  if (it != this->DisplayPipelines.end())
    {
    return;
    }

  vtkNew<vtkActor2D> actor;
  if (displayNode->IsA("vtkDMMLModelDisplayNode"))
    {
    actor->SetMapper( vtkNew<vtkPolyDataMapper2D>().GetPointer() );
    }

  // Create pipeline
  Pipeline* pipeline = new Pipeline();
  pipeline->Actor = actor.GetPointer();
  pipeline->Cutter = vtkSmartPointer<vtkPlaneCutter>::New();
  pipeline->GeometryFilter = vtkSmartPointer<vtkCompositeDataGeometryFilter>::New();
  pipeline->SliceDistance = vtkSmartPointer<vtkSampleImplicitFunctionFilter>::New();
  pipeline->TransformToSlice = vtkSmartPointer<vtkTransform>::New();
  pipeline->NodeToWorld = vtkSmartPointer<vtkGeneralTransform>::New();
  pipeline->Transformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  pipeline->ModelWarper = vtkSmartPointer<vtkTransformFilter>::New();
  pipeline->SurfaceExtractor = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
  pipeline->Plane = vtkSmartPointer<vtkPlane>::New();

  // Set up pipeline
  pipeline->Transformer->SetTransform(pipeline->TransformToSlice);
  pipeline->Transformer->SetInputConnection(pipeline->GeometryFilter->GetOutputPort());
  pipeline->Cutter->SetPlane(pipeline->Plane);
  pipeline->Cutter->BuildTreeOff(); // the cutter crashes for complex geometries if build tree is enabled
  pipeline->Cutter->SetInputConnection(pipeline->ModelWarper->GetOutputPort());
  pipeline->GeometryFilter->SetInputConnection(pipeline->Cutter->GetOutputPort());
  // Projection is created from outer surface of volumetric meshes (for polydata surface
  // extraction is just shallow-copy)
  pipeline->SurfaceExtractor->SetInputConnection(pipeline->ModelWarper->GetOutputPort());
  pipeline->SliceDistance->SetImplicitFunction(pipeline->Plane);
  pipeline->SliceDistance->SetInputConnection(pipeline->SurfaceExtractor->GetOutputPort());
  pipeline->Actor->SetVisibility(0);

  // Add actor to Renderer and local cache
  this->External->GetRenderer()->AddActor( pipeline->Actor );
  this->DisplayPipelines.insert( std::make_pair(displayNode, pipeline) );

  // Update cached matrices. Calls UpdateDisplayNodePipeline
  this->UpdateDisplayableTransforms(mNode);
}

//---------------------------------------------------------------------------
void vtkDMMLModelSliceDisplayableManager::vtkInternal
::UpdateDisplayNode(vtkDMMLDisplayNode* displayNode)
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
    this->External->AddDisplayableNode( displayNode->GetDisplayableNode() );
    }
}

//---------------------------------------------------------------------------
void vtkDMMLModelSliceDisplayableManager::vtkInternal
::UpdateDisplayNodePipeline(vtkDMMLDisplayNode* displayNode, const Pipeline* pipeline)
{
  // Sets visibility, set pipeline mesh input, update color
  //   calculate and set pipeline transforms.

  if (!pipeline)
    {
    vtkErrorWithObjectMacro(this->External, "vtkDMMLModelSliceDisplayableManager::"
      "vtkInternal::UpdateDisplayNodePipeline failed: pipeline is invalid");
    return;
    }

  vtkDMMLModelDisplayNode* modelDisplayNode = vtkDMMLModelDisplayNode::SafeDownCast(displayNode);
  if (!modelDisplayNode)
    {
    vtkErrorWithObjectMacro(this->External, "vtkDMMLModelSliceDisplayableManager::"
      "vtkInternal::UpdateDisplayNodePipeline failed: vtkDMMLModelDisplayNode display node type is expected");
    pipeline->Actor->SetVisibility(false);
    return;
    }

  // Get display node from hierarchy that applies display properties on branch
  vtkDMMLDisplayableNode* displayableNode = displayNode->GetDisplayableNode();
  vtkDMMLDisplayNode* overrideHierarchyDisplayNode =
    vtkDMMLFolderDisplayNode::GetOverridingHierarchyDisplayNode(displayableNode);

  // Use hierarchy display node if any, and if overriding is allowed for the current display node.
  // If override is explicitly disabled, then do not apply hierarchy visibility or opacity either.
  bool hierarchyVisibility = true;
  double hierarchyOpacity = 1.0;
  if (displayNode->GetFolderDisplayOverrideAllowed())
    {
    if (overrideHierarchyDisplayNode)
      {
      displayNode = overrideHierarchyDisplayNode;
      }

    // Get visibility and opacity defined by the hierarchy.
    // These two properties are influenced by the hierarchy regardless the fact whether there is override
    // or not. Visibility of items defined by hierarchy is off if any of the ancestors is explicitly hidden,
    // and the opacity is the product of the ancestors' opacities.
    // However, this does not apply on display nodes that do not allow overrides (FolderDisplayOverrideAllowed)
    hierarchyVisibility = vtkDMMLFolderDisplayNode::GetHierarchyVisibility(displayableNode);
    hierarchyOpacity = vtkDMMLFolderDisplayNode::GetHierarchyOpacity(displayableNode);
    }

  if (!hierarchyVisibility || !this->IsVisible(displayNode))
    {
    pipeline->Actor->SetVisibility(false);
    return;
    }

  vtkPointSet* pointSet = modelDisplayNode->GetOutputMesh();
  if (!pointSet)
    {
    pipeline->Actor->SetVisibility(false);
    return;
    }

  // Need this to update bounds of the locator, to avoid crash in the cutter
  modelDisplayNode->GetOutputMeshConnection()->GetProducer()->Update();

  if (!pointSet->GetPoints() || pointSet->GetNumberOfPoints() == 0)
    {
    // there are no points, so there is nothing to cut
    pipeline->Actor->SetVisibility(false);
    return;
    }

  pipeline->ModelWarper->SetInputData(pointSet); //why here? +connection?
  pipeline->ModelWarper->SetTransform(pipeline->NodeToWorld);

  // Set Plane Transform
  this->SetSlicePlaneFromMatrix(this->SliceXYToRAS, pipeline->Plane);
  pipeline->Plane->Modified();

  if (modelDisplayNode->GetSliceDisplayMode() == vtkDMMLModelDisplayNode::SliceDisplayProjection
    || modelDisplayNode->GetSliceDisplayMode() == vtkDMMLModelDisplayNode::SliceDisplayDistanceEncodedProjection)
    {

    if (modelDisplayNode->GetSliceDisplayMode() == vtkDMMLModelDisplayNode::SliceDisplayProjection)
      {
      // remove cutter from the pipeline if projection mode is used, we just need to extract surface
      // and flatten the model
      pipeline->Transformer->SetInputConnection(pipeline->SurfaceExtractor->GetOutputPort());
      }
    else
      {
      // replace cutter in the pipeline by surface extraction, slice distance computation,
      // and flattening of the model
      pipeline->Transformer->SetInputConnection(pipeline->SliceDistance->GetOutputPort());
      }

    //  Set Poly Data Transform that projects model to the slice plane
    vtkNew<vtkMatrix4x4> rasToSliceXY;
    vtkMatrix4x4::Invert(this->SliceXYToRAS, rasToSliceXY.GetPointer());
    // Project all points to the slice plane (slice Z coordinate = 0)
    rasToSliceXY->SetElement(2, 0, 0);
    rasToSliceXY->SetElement(2, 1, 0);
    rasToSliceXY->SetElement(2, 2, 0);
    pipeline->TransformToSlice->SetMatrix(rasToSliceXY.GetPointer());
    }
  else
    {
    // show intersection in the slice view
    // include clipper in the pipeline
    pipeline->Transformer->SetInputConnection(pipeline->GeometryFilter->GetOutputPort());
    pipeline->Cutter->SetInputConnection(pipeline->ModelWarper->GetOutputPort());

    // If there is no input or if the input has no points, the vtkTransformPolyDataFilter will display an error message
    // on every update: "No input data".
    // To prevent the error, if the input is empty then the actor should not be visible since there is nothing to display.
    pipeline->GeometryFilter->Update();
    if (!pipeline->GeometryFilter->GetOutput() || pipeline->GeometryFilter->GetOutput()->GetNumberOfPoints() < 1)
      {
      pipeline->Actor->SetVisibility(false);
      return;
      }

    //  Set Poly Data Transform
    vtkNew<vtkMatrix4x4> rasToSliceXY;
    vtkMatrix4x4::Invert(this->SliceXYToRAS, rasToSliceXY.GetPointer());
    pipeline->TransformToSlice->SetMatrix(rasToSliceXY.GetPointer());
    }

  // Update pipeline actor
  vtkActor2D* actor = vtkActor2D::SafeDownCast(pipeline->Actor);
  vtkPolyDataMapper2D* mapper = vtkPolyDataMapper2D::SafeDownCast(actor->GetMapper());

  if (mapper)
    {
    mapper->SetInputConnection( pipeline->Transformer->GetOutputPort() );

    if (modelDisplayNode->GetSliceDisplayMode() == vtkDMMLModelDisplayNode::SliceDisplayDistanceEncodedProjection)
      {
      vtkDMMLColorNode* colorNode = modelDisplayNode->GetDistanceEncodedProjectionColorNode();
      vtkSmartPointer<vtkScalarsToColors> lut = nullptr;
      vtkSmartPointer<vtkDMMLProceduralColorNode> proceduralColor = vtkDMMLProceduralColorNode::SafeDownCast(colorNode);
      if (proceduralColor)
        {
        lut = vtkScalarsToColors::SafeDownCast(proceduralColor->GetColorTransferFunction());
        }
      else
        {
        vtkSmartPointer<vtkLookupTable> dNodeLUT = vtkSmartPointer<vtkLookupTable>::Take(colorNode ? colorNode->CreateLookupTableCopy() : nullptr);
        if (dNodeLUT)
          {
          lut = dNodeLUT;
          mapper->SetScalarRange(displayNode->GetScalarRange());
          lut->SetAlpha(hierarchyOpacity * displayNode->GetSliceIntersectionOpacity());
          }
        }

      if (lut != nullptr)
        {
        mapper->SetLookupTable(lut.GetPointer());
        mapper->SetScalarRange(lut->GetRange());
        mapper->SetScalarVisibility(true);
        }
      else
        {
        mapper->SetScalarVisibility(false);
        }
      }
    else if (displayNode->GetScalarVisibility())
      {
      // Check if using point data or cell data
      vtkDMMLModelNode* modelNode = vtkDMMLModelNode::SafeDownCast(displayableNode);
      if (vtkDMMLModelDisplayableManager::IsCellScalarsActive(displayNode, modelNode))
        {
        mapper->SetScalarModeToUseCellData();
        }
      else
        {
        mapper->SetScalarModeToUsePointData();
        }

      if (displayNode->GetScalarRangeFlag() == vtkDMMLDisplayNode::UseDirectMapping)
        {
        mapper->SetColorModeToDirectScalars();
        mapper->SetLookupTable(nullptr);
        }
      else
        {
        mapper->SetColorModeToMapScalars();

        // The renderer uses the lookup table scalar range to
        // render colors. By default, UseLookupTableScalarRange
        // is set to false and SetScalarRange can be used on the
        // mapper to map scalars into the lookup table. When set
        // to true, SetScalarRange has no effect and it is necessary
        // to force the scalarRange on the lookup table manually.
        // Whichever way is used, the look up table range needs
        // to be changed to render the correct scalar values, thus
        // one lookup table can not be shared by multiple mappers
        // if any of those mappers needs to map using its scalar
        // values range. It is therefore necessary to make a copy
        // of the colorNode vtkLookupTable in order not to impact
        // that lookup table original range.
        vtkSmartPointer<vtkLookupTable> dNodeLUT = vtkSmartPointer<vtkLookupTable>::Take(displayNode->GetColorNode() ?
          displayNode->GetColorNode()->CreateLookupTableCopy() : nullptr);
        if (dNodeLUT)
          {
          dNodeLUT->SetAlpha(hierarchyOpacity * displayNode->GetSliceIntersectionOpacity());
          }
        mapper->SetLookupTable(dNodeLUT);
        }

      // Set scalar range
      mapper->SetScalarRange(displayNode->GetScalarRange());

      mapper->SetScalarVisibility(true);
      }
    else
      {
      mapper->SetScalarVisibility(false);
      }
    }

  vtkProperty2D* actorProperties = actor->GetProperty();
  actorProperties->SetColor(displayNode->GetColor());
  actorProperties->SetPointSize(displayNode->GetSliceIntersectionThickness());
  actorProperties->SetLineWidth(displayNode->GetSliceIntersectionThickness());
  actorProperties->SetOpacity(hierarchyOpacity * displayNode->GetSliceIntersectionOpacity());

  // Opacity of the slice intersection is intentionally not set by
  // actorProperties->SetOpacity(displayNode->GetOpacity()),
  // because most often users only want to make 3D model transparent.
  // Visibility of slice intersections can be tuned by modifying
  // slice intersection thickness or a new SliceIntersectionOpacity attribute
  // could be introduced.

  actor->SetPosition(0,0);
  actor->SetVisibility(true);
}

//---------------------------------------------------------------------------
void vtkDMMLModelSliceDisplayableManager::vtkInternal
::AddObservations(vtkDMMLDisplayableNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkEventBroker::ObservationVector observations;

  if (!broker->GetObservationExist(node, vtkDMMLDisplayableNode::TransformModifiedEvent,
                                          this->External, this->External->GetDMMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkDMMLDisplayableNode::TransformModifiedEvent,
                            this->External, this->External->GetDMMLNodesCallbackCommand() );
    }

  if (!broker->GetObservationExist(node, vtkDMMLDisplayableNode::DisplayModifiedEvent,
                                          this->External, this->External->GetDMMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkDMMLDisplayableNode::DisplayModifiedEvent,
                            this->External, this->External->GetDMMLNodesCallbackCommand() );
    }

  if (!broker->GetObservationExist(node, vtkDMMLModelNode::PolyDataModifiedEvent,
                                          this->External, this->External->GetDMMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkDMMLModelNode::PolyDataModifiedEvent,
                            this->External, this->External->GetDMMLNodesCallbackCommand() );
    }
}

//---------------------------------------------------------------------------
void vtkDMMLModelSliceDisplayableManager::vtkInternal
::RemoveObservations(vtkDMMLDisplayableNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkEventBroker::ObservationVector observations;
  observations = broker->GetObservations(
    node, vtkDMMLModelNode::PolyDataModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(
    node, vtkDMMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(
    node, vtkDMMLDisplayableNode::TransformModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
}

//---------------------------------------------------------------------------
bool vtkDMMLModelSliceDisplayableManager::vtkInternal
::IsNodeObserved(vtkDMMLDisplayableNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkCollection* observations;
  observations = broker->GetObservationsForSubject(node);
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
void vtkDMMLModelSliceDisplayableManager::vtkInternal
::ClearDisplayableNodes()
{
  while(this->ModelToDisplayNodes.size() > 0)
    {
    this->External->RemoveDisplayableNode(this->ModelToDisplayNodes.begin()->first);
    }
}

//---------------------------------------------------------------------------
bool vtkDMMLModelSliceDisplayableManager::vtkInternal
::UseDisplayableNode(vtkDMMLDisplayableNode* node)
{
  bool show = node && node->IsA("vtkDMMLModelNode");

  if (!this->SliceNode->GetLayoutName())
    {
    vtkErrorWithObjectMacro(this->External, "No layout name to slice node " << this->SliceNode->GetID());
    return false;
    }
  // Ignore the slice model node for this slice DM.
  std::string cmpstr = std::string(this->SliceNode->GetLayoutName()) + " Volume Slice";
  show = show && ( cmpstr.compare(node->GetName()) );

  return show;
}

//---------------------------------------------------------------------------
// vtkDMMLModelSliceDisplayableManager methods

//---------------------------------------------------------------------------
vtkDMMLModelSliceDisplayableManager::vtkDMMLModelSliceDisplayableManager()
{
  this->Internal = new vtkInternal(this);
  this->AddingDisplayableNode = 0;
}

//---------------------------------------------------------------------------
vtkDMMLModelSliceDisplayableManager::~vtkDMMLModelSliceDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkDMMLModelSliceDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkDMMLModelSliceDisplayableManager::AddDisplayableNode(
  vtkDMMLDisplayableNode* node)
{
  if (this->AddingDisplayableNode)
    {
    return;
    }
  // Check if node should be used
  if (!this->Internal->UseDisplayableNode(node))
    {
    return;
    }

  this->AddingDisplayableNode = 1;
  // Add Display Nodes
  int nnodes = node->GetNumberOfDisplayNodes();

  this->Internal->AddObservations(node);

  for (int i=0; i<nnodes; i++)
  {
    vtkDMMLDisplayNode *dnode = node->GetNthDisplayNode(i);
    if ( this->Internal->UseDisplayNode(dnode) )
      {
      this->Internal->ModelToDisplayNodes[node].insert(dnode);
      this->Internal->AddDisplayNode( node, dnode );
      }
    }
  this->AddingDisplayableNode = 0;
}

//---------------------------------------------------------------------------
void vtkDMMLModelSliceDisplayableManager
::RemoveDisplayableNode(vtkDMMLDisplayableNode* node)
{
  if (!node)
    {
    return;
    }
  vtkInternal::ModelToDisplayCacheType::iterator displayableIt =
    this->Internal->ModelToDisplayNodes.find(node);
  if(displayableIt == this->Internal->ModelToDisplayNodes.end())
    {
    return;
    }

  std::set<vtkDMMLDisplayNode *> dnodes = displayableIt->second;
  std::set<vtkDMMLDisplayNode *>::iterator diter;
  for ( diter = dnodes.begin(); diter != dnodes.end(); ++diter)
    {
    this->Internal->RemoveDisplayNode(*diter);
    }
  this->Internal->RemoveObservations(node);
  this->Internal->ModelToDisplayNodes.erase(displayableIt);
}

//---------------------------------------------------------------------------
void vtkDMMLModelSliceDisplayableManager
::OnDMMLSceneNodeAdded(vtkDMMLNode* node)
{
  if (!node->IsA("vtkDMMLModelNode"))
    {
    return;
    }

  // Escape if the scene a scene is being closed, imported or connected
  if (this->GetDMMLScene()->IsBatchProcessing())
    {
    this->SetUpdateFromDMMLRequested(true);
    return;
    }

  this->AddDisplayableNode(vtkDMMLDisplayableNode::SafeDownCast(node));
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkDMMLModelSliceDisplayableManager
::OnDMMLSceneNodeRemoved(vtkDMMLNode* node)
{
  if ( node && !node->IsA("vtkDMMLModelNode")
       && !node->IsA("vtkDMMLModelDisplayNode") )
    {
    return;
    }

  vtkDMMLDisplayableNode* modelNode = nullptr;
  vtkDMMLDisplayNode* displayNode = nullptr;

  bool modified = false;
  if ( (modelNode = vtkDMMLDisplayableNode::SafeDownCast(node)) )
    {
    this->RemoveDisplayableNode(modelNode);
    modified = true;
    }
  else if ( (displayNode = vtkDMMLDisplayNode::SafeDownCast(node)) )
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
void vtkDMMLModelSliceDisplayableManager
::ProcessDMMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  vtkDMMLScene* scene = this->GetDMMLScene();

  if (scene == nullptr || scene->IsBatchProcessing())
    {
    return;
    }

  vtkDMMLDisplayableNode* displayableNode = vtkDMMLDisplayableNode::SafeDownCast(caller);

  if ( displayableNode )
    {
    vtkDMMLNode* callDataNode = reinterpret_cast<vtkDMMLDisplayNode *> (callData);
    vtkDMMLDisplayNode* displayNode = vtkDMMLDisplayNode::SafeDownCast(callDataNode);

    if ( displayNode && (event == vtkDMMLDisplayableNode::DisplayModifiedEvent) )
      {
      this->Internal->UpdateDisplayNode(displayNode);
      this->RequestRender();
      }
    else if ( (event == vtkDMMLDisplayableNode::TransformModifiedEvent)
             || (event == vtkDMMLModelNode::PolyDataModifiedEvent))
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
void vtkDMMLModelSliceDisplayableManager::UpdateFromDMML()
{
  this->SetUpdateFromDMMLRequested(false);

  vtkDMMLScene* scene = this->GetDMMLScene();
  if (!scene)
    {
    vtkDebugMacro( "vtkDMMLModelSliceDisplayableManager->UpdateFromDMML: Scene is not set.");
    return;
    }

  // Find the display nodes that need to be removed
  // (not deleted immediately because it would invalidate the pipeline iteration)
  std::set< vtkDMMLDisplayableNode* > displayableNodesToRemove;
  std::deque< vtkDMMLDisplayNode* > displayNodesToRemove;
  for (auto modelToDisplayNode : this->Internal->DisplayPipelines)
    {
    vtkDMMLDisplayNode* displayNode = modelToDisplayNode.first;
    if (!displayNode)
      {
      continue;
      }
    if (!scene->IsNodePresent(displayNode))
      {
      // the display node is deleted
      displayNodesToRemove.push_back(displayNode);
      continue;
      }
    vtkDMMLDisplayableNode* displayableNode = displayNode->GetDisplayableNode();
    if (displayableNodesToRemove.find(displayableNode) != displayableNodesToRemove.end())
      {
      // already marked for removal
      continue;
      }
    if (!this->Internal->UseDisplayableNode(displayableNode))
      {
      // the displayable node is deleted or not applicable anymore
      displayableNodesToRemove.insert(displayableNode);
      continue;
      }
    if (!this->Internal->UseDisplayNode(displayNode))
      {
      // the displayable node is deleted or not applicable anymore
      displayNodesToRemove.push_back(displayNode);
      continue;
      }
    }
  for (vtkDMMLDisplayableNode* displayableNodeToRemove : displayableNodesToRemove)
    {
    this->RemoveDisplayableNode(displayableNodeToRemove);
    }
  for (vtkDMMLDisplayNode* displayNodeToRemove : displayNodesToRemove)
    {
    this->Internal->RemoveDisplayNode(displayNodeToRemove);
    }

  vtkDMMLDisplayableNode* mNode = nullptr;
  std::vector<vtkDMMLNode *> mNodes;
  int nnodes = scene ? scene->GetNodesByClass("vtkDMMLDisplayableNode", mNodes) : 0;
  for (int i=0; i<nnodes; i++)
    {
    mNode  = vtkDMMLDisplayableNode::SafeDownCast(mNodes[i]);
    if (mNode && this->Internal->UseDisplayableNode(mNode))
      {
      this->AddDisplayableNode(mNode);
      }
    }
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkDMMLModelSliceDisplayableManager::UnobserveDMMLScene()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkDMMLModelSliceDisplayableManager::OnDMMLSceneStartClose()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkDMMLModelSliceDisplayableManager::OnDMMLSceneEndClose()
{
  this->SetUpdateFromDMMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkDMMLModelSliceDisplayableManager::OnDMMLSceneEndBatchProcess()
{
  this->SetUpdateFromDMMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkDMMLModelSliceDisplayableManager::Create()
{
  this->Internal->SetSliceNode(this->GetDMMLSliceNode());
  this->SetUpdateFromDMMLRequested(true);
}
