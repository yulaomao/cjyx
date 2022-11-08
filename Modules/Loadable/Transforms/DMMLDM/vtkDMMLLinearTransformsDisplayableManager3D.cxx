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
#include "vtkDMMLLinearTransformsDisplayableManager3D.h"

#include "vtkCjyxTransformLogic.h"

// DMML includes
#include <vtkEventBroker.h>
#include <vtkDMMLProceduralColorNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLTransformDisplayNode.h>
#include <vtkDMMLTransformNode.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkBoxRepresentation.h>
#include <vtkBoxWidget2.h>
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkGeneralTransform.h>
#include <vtkLineSource.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPickingManager.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>

const double DEFAULT_SCALE[3] = {100.0, 100.0, 100.0};

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkDMMLLinearTransformsDisplayableManager3D );

//---------------------------------------------------------------------------
// vtkDMMLLinearTransformsDisplayableManager3D Callback
class vtkLinearTransformWidgetCallback : public vtkCommand
{
public:
  static vtkLinearTransformWidgetCallback *New()
  { return new vtkLinearTransformWidgetCallback; }

  vtkLinearTransformWidgetCallback() = default;

  void Execute (vtkObject *vtkNotUsed(caller), unsigned long event, void *vtkNotUsed(callData)) override
  {
    if ((event == vtkCommand::StartInteractionEvent) || (event == vtkCommand::EndInteractionEvent) || (event == vtkCommand::InteractionEvent))
      {
      // sanity checks
      if (!this->DisplayableManager)
        {
        return;
        }
      if (!this->Node)
        {
        return;
        }
      if (!this->Widget)
        {
        return;
        }
      // sanity checks end
      }

    if (event == vtkCommand::StartInteractionEvent)
      {
      // save the state of the node when starting interaction
      if (this->Node->GetScene())
        {
        this->Node->GetScene()->SaveStateForUndo();
        }
      }
  }

  void SetWidget(vtkAbstractWidget *w)
    {
    this->Widget = w;
    }
  void SetNode(vtkDMMLTransformNode *n)
    {
    this->Node = n;
    }
  void SetDisplayableManager(vtkDMMLLinearTransformsDisplayableManager3D* dm)
    {
    this->DisplayableManager = dm;
    }

  vtkAbstractWidget * Widget{nullptr};
  vtkDMMLTransformNode* Node{nullptr};
  vtkDMMLLinearTransformsDisplayableManager3D* DisplayableManager{nullptr};
};

//---------------------------------------------------------------------------
class vtkDMMLLinearTransformsDisplayableManager3D::vtkInternal
{
public:

  vtkInternal(vtkDMMLLinearTransformsDisplayableManager3D* external);
  ~vtkInternal();

  struct Pipeline
    {
    vtkSmartPointer<vtkBoxWidget2> Widget;
    vtkSmartPointer<vtkTransform> WidgetDisplayTransform;
    bool UpdateWidgetBounds;
    };

  typedef std::map < vtkDMMLTransformDisplayNode*, Pipeline* > PipelinesCacheType;
  PipelinesCacheType DisplayPipelines;

  typedef std::map < vtkDMMLTransformNode*, std::set< vtkDMMLTransformDisplayNode* > > TransformToDisplayCacheType;
  TransformToDisplayCacheType TransformToDisplayNodes;

  typedef std::map < vtkBoxWidget2*, vtkDMMLTransformDisplayNode* > WidgetToNodeMapType;
  WidgetToNodeMapType WidgetMap;

  // Transforms
  void AddTransformNode(vtkDMMLTransformNode* displayableNode);
  void RemoveTransformNode(vtkDMMLTransformNode* displayableNode);
  void UpdateDisplayableTransforms(vtkDMMLTransformNode *node, bool);

  // Display Nodes
  void AddDisplayNode(vtkDMMLTransformNode*, vtkDMMLTransformDisplayNode*);
  void UpdateDisplayNode(vtkDMMLTransformDisplayNode* displayNode);
  void UpdateDisplayNodePipeline(vtkDMMLTransformDisplayNode*, Pipeline*);
  void RemoveDisplayNode(vtkDMMLTransformDisplayNode* displayNode);
  void SetTransformDisplayProperty(vtkDMMLTransformDisplayNode *displayNode, vtkActor* actor);

  // Widget
  void UpdateWidgetDisplayTransform(Pipeline*, vtkDMMLTransformNode*);
  void UpdateWidgetFromNode(vtkDMMLTransformDisplayNode*, vtkDMMLTransformNode*, Pipeline*);
  void UpdateNodeFromWidget(vtkBoxWidget2*);

  // Observations
  void AddObservations(vtkDMMLTransformNode* node);
  void RemoveObservations(vtkDMMLTransformNode* node);
  void AddDisplayObservations(vtkDMMLTransformDisplayNode* node);
  void RemoveDisplayObservations(vtkDMMLTransformDisplayNode* node);

  // Helper functions
  bool UseDisplayNode(vtkDMMLTransformDisplayNode* displayNode);
  bool UseDisplayableNode(vtkDMMLTransformNode* node);
  void ClearDisplayableNodes();

private:
  vtkDMMLLinearTransformsDisplayableManager3D* External;
  bool AddingTransformNode;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkDMMLLinearTransformsDisplayableManager3D::vtkInternal::vtkInternal(vtkDMMLLinearTransformsDisplayableManager3D * external)
: External(external)
, AddingTransformNode(false)
{
}

//---------------------------------------------------------------------------
vtkDMMLLinearTransformsDisplayableManager3D::vtkInternal::~vtkInternal()
{
  this->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
bool vtkDMMLLinearTransformsDisplayableManager3D::vtkInternal::UseDisplayNode(vtkDMMLTransformDisplayNode* displayNode)
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
void vtkDMMLLinearTransformsDisplayableManager3D::vtkInternal::AddTransformNode(vtkDMMLTransformNode* node)
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
void vtkDMMLLinearTransformsDisplayableManager3D::vtkInternal::RemoveTransformNode(vtkDMMLTransformNode* node)
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
void vtkDMMLLinearTransformsDisplayableManager3D::vtkInternal::UpdateDisplayableTransforms(vtkDMMLTransformNode* mNode, bool updateBounds)
{
  // Update the pipeline for all tracked DisplayableNode
  PipelinesCacheType::iterator pipelinesIter;
  std::set< vtkDMMLTransformDisplayNode* > displayNodes = this->TransformToDisplayNodes[mNode];
  std::set< vtkDMMLTransformDisplayNode* >::iterator dnodesIter;
  for ( dnodesIter = displayNodes.begin(); dnodesIter != displayNodes.end(); dnodesIter++ )
    {
    if ( ((pipelinesIter = this->DisplayPipelines.find(*dnodesIter)) != this->DisplayPipelines.end()) )
      {
      Pipeline* pipeline = pipelinesIter->second;
      pipeline->UpdateWidgetBounds |= updateBounds;
      this->UpdateDisplayNodePipeline(pipelinesIter->first, pipeline);
      }
    }
}

//---------------------------------------------------------------------------
void vtkDMMLLinearTransformsDisplayableManager3D::vtkInternal::RemoveDisplayNode(vtkDMMLTransformDisplayNode* displayNode)
{
  PipelinesCacheType::iterator actorsIt = this->DisplayPipelines.find(displayNode);
  if(actorsIt == this->DisplayPipelines.end())
    {
    return;
    }
  this->RemoveDisplayObservations(displayNode);
  Pipeline* pipeline = actorsIt->second;
  if (pipeline->Widget)
    {
    // The widget must be disabled because that removes its internal KeyEventCallbackCommand observer.
    // Without this, a keypress event in the 3D view would call vtkBoxWidget2::ProcessKeyEvents method
    // of the deleted widget, making the application crash.
    pipeline->Widget->SetEnabled(false);
    // Remove the widget observers that this class has added.
    pipeline->Widget->RemoveAllObservers();
    }
  this->WidgetMap.erase(pipeline->Widget);
  delete pipeline;
  this->DisplayPipelines.erase(actorsIt);
}

//---------------------------------------------------------------------------
void vtkDMMLLinearTransformsDisplayableManager3D::vtkInternal::AddDisplayNode(vtkDMMLTransformNode* mNode, vtkDMMLTransformDisplayNode* displayNode)
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

  this->AddDisplayObservations(displayNode);

  // Create pipeline
  Pipeline* pipeline = new Pipeline();
  pipeline->UpdateWidgetBounds = true;
  this->DisplayPipelines.insert( std::make_pair(displayNode, pipeline) );

  // Interaction VTK
  //  - Widget
  pipeline->Widget = vtkSmartPointer<vtkBoxWidget2>::New();
  vtkNew<vtkBoxRepresentation> widgetRep;
  pipeline->Widget->SetRepresentation(widgetRep);
  //  - Transform
  pipeline->WidgetDisplayTransform = vtkSmartPointer<vtkTransform>::New();
  // - Widget events
  pipeline->Widget->AddObserver(
    vtkCommand::InteractionEvent, this->External->GetWidgetsCallbackCommand());

  if (this->External->GetInteractor()->GetPickingManager())
    {
    if (!(this->External->GetInteractor()->GetPickingManager()->GetEnabled()))
      {
      // if the picking manager is not already turned on for this
      // interactor, enable it
      this->External->GetInteractor()->GetPickingManager()->EnabledOn();
      }
    }

  // Add actor / set renderers and cache
  pipeline->Widget->SetInteractor(this->External->GetInteractor());
  pipeline->Widget->SetCurrentRenderer(this->External->GetRenderer());
  this->WidgetMap.insert( std::make_pair(pipeline->Widget, displayNode) );

  // add the callback
  vtkNew<vtkLinearTransformWidgetCallback> widgetCallback;
  widgetCallback->SetNode(mNode);
  widgetCallback->SetWidget(pipeline->Widget);
  widgetCallback->SetDisplayableManager(this->External);
  pipeline->Widget->AddObserver(vtkCommand::StartInteractionEvent, widgetCallback);
  pipeline->Widget->AddObserver(vtkCommand::EndInteractionEvent, widgetCallback);
  pipeline->Widget->AddObserver(vtkCommand::InteractionEvent, widgetCallback);

  // Update cached matrices. Calls UpdateDisplayNodePipeline
  this->UpdateDisplayableTransforms(mNode, true);
}

//---------------------------------------------------------------------------
void vtkDMMLLinearTransformsDisplayableManager3D::vtkInternal::UpdateDisplayNode(vtkDMMLTransformDisplayNode* displayNode)
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
void vtkDMMLLinearTransformsDisplayableManager3D::vtkInternal
::UpdateWidgetDisplayTransform(
  Pipeline* pipeline, vtkDMMLTransformNode* transformNode)
{
  // WidgetDisplayTransform is in charge of moving the widget around
  // the bounding box of the objects under the transform, wherever they
  // are.
  assert(pipeline && transformNode);

  std::vector<vtkDMMLDisplayableNode*> transformedNodes;
  vtkCjyxTransformLogic::GetTransformedNodes(
    this->External->GetDMMLScene(), transformNode, transformedNodes, true);

  bool validBounds = false;
  double bounds[6];
  if (transformedNodes.size() > 0)
    {
    vtkCjyxTransformLogic::GetNodesBounds(transformedNodes, bounds);
    validBounds =
      (bounds[0] <= bounds[1] || bounds[2] <= bounds[3] || bounds[4] <= bounds[5]);
    }

  if (validBounds)
    {
    // Get the bounding box around the UNTRANSFORMED objects so we have
    // the actual box around the object.
    double center[3], scales[3];
    for (int i = 0; i < 3; ++i)
      {
      double scale = 0.5*(bounds[2*i + 1] - bounds[2*i]);
      center[i] = bounds[2*i] + scale;
      scales[i] = 4*scale;
      }

    pipeline->WidgetDisplayTransform->Identity();
    pipeline->WidgetDisplayTransform->Translate(center);
    pipeline->WidgetDisplayTransform->Scale(scales);
    }
  else
    {
    // No objects, just add a default scaling so the widget can be interacted
    // with more easily.
    pipeline->WidgetDisplayTransform->Identity();
    pipeline->WidgetDisplayTransform->Scale(DEFAULT_SCALE);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLLinearTransformsDisplayableManager3D::vtkInternal
::UpdateWidgetFromNode(vtkDMMLTransformDisplayNode* displayNode,
                       vtkDMMLTransformNode* transformNode,
                       Pipeline* pipeline)
{
  assert(displayNode && transformNode && pipeline);

  if (pipeline->UpdateWidgetBounds)
    {
    this->UpdateWidgetDisplayTransform(pipeline, transformNode);
    pipeline->UpdateWidgetBounds = false;
    }

  vtkNew<vtkMatrix4x4> toWorldMatrix;
  transformNode->GetMatrixTransformToWorld(toWorldMatrix.GetPointer());
  vtkNew<vtkTransform> widgetTransform;
  widgetTransform->Concatenate(toWorldMatrix.GetPointer());
  widgetTransform->Concatenate(pipeline->WidgetDisplayTransform);

  vtkBoxRepresentation* representation =
    vtkBoxRepresentation::SafeDownCast(pipeline->Widget->GetRepresentation());

  representation->SetTransform(widgetTransform.GetPointer());

  pipeline->Widget->SetTranslationEnabled(
    displayNode->GetEditorTranslationEnabled());
  pipeline->Widget->SetRotationEnabled(
    displayNode->GetEditorRotationEnabled());
  pipeline->Widget->SetScalingEnabled(
    displayNode->GetEditorScalingEnabled());
  pipeline->Widget->SetMoveFacesEnabled(
    displayNode->GetEditorScalingEnabled());
}

//---------------------------------------------------------------------------
void vtkDMMLLinearTransformsDisplayableManager3D::vtkInternal
::UpdateNodeFromWidget(vtkBoxWidget2* widget)
{
  assert(widget);
  vtkDMMLTransformDisplayNode* displayNode = this->WidgetMap[widget];
  assert(displayNode);
  Pipeline* pipeline = this->DisplayPipelines[displayNode];
  vtkDMMLTransformNode* node =
    vtkDMMLTransformNode::SafeDownCast(displayNode->GetDisplayableNode());

  vtkBoxRepresentation* representation =
    vtkBoxRepresentation::SafeDownCast(widget->GetRepresentation());

  vtkNew<vtkTransform> widgetTransform;
  representation->GetTransform(widgetTransform.GetPointer());

  vtkNew<vtkTransform> toParent;
  vtkDMMLTransformNode* parentNode = node->GetParentTransformNode();
  if (parentNode)
    {
    vtkNew<vtkMatrix4x4> worldFromParentMatrix;
    parentNode->GetMatrixTransformFromWorld(worldFromParentMatrix.GetPointer());
    toParent->Concatenate(worldFromParentMatrix.GetPointer());
    }
  toParent->Concatenate(widgetTransform.GetPointer());
  toParent->Concatenate(pipeline->WidgetDisplayTransform->GetLinearInverse());

  node->SetMatrixTransformToParent(toParent->GetMatrix());
}

//---------------------------------------------------------------------------
void vtkDMMLLinearTransformsDisplayableManager3D::vtkInternal
::UpdateDisplayNodePipeline(
  vtkDMMLTransformDisplayNode* displayNode,
  Pipeline* pipeline)
{
  if (!displayNode || !pipeline)
    {
    return;
    }

  vtkDMMLTransformNode* transformNode =
    vtkDMMLTransformNode::SafeDownCast(displayNode->GetDisplayableNode());
  if (transformNode==nullptr)
    {
    pipeline->Widget->SetEnabled(false);
    return;
    }

  if (!transformNode->IsLinear())
    {
    vtkDebugWithObjectMacro(transformNode, "Cannot show interactive widget: Transform is not linear");
    pipeline->Widget->SetEnabled(false);
    return;
    }

  bool visible = displayNode->GetEditorVisibility();
  pipeline->Widget->SetEnabled(visible);
  if (visible)
    {
    this->UpdateWidgetFromNode(displayNode, transformNode, pipeline);
    return;
    }
}

//---------------------------------------------------------------------------
void vtkDMMLLinearTransformsDisplayableManager3D::vtkInternal::AddObservations(vtkDMMLTransformNode* node)
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
void vtkDMMLLinearTransformsDisplayableManager3D::vtkInternal
::AddDisplayObservations(vtkDMMLTransformDisplayNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  if (!broker->GetObservationExist(node, vtkDMMLTransformDisplayNode::TransformUpdateEditorBoundsEvent, this->External, this->External->GetDMMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkDMMLTransformDisplayNode::TransformUpdateEditorBoundsEvent, this->External, this->External->GetDMMLNodesCallbackCommand() );
    }
}

//---------------------------------------------------------------------------
void vtkDMMLLinearTransformsDisplayableManager3D::vtkInternal::RemoveObservations(vtkDMMLTransformNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkEventBroker::ObservationVector observations;
  observations = broker->GetObservations(node, vtkDMMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand() );
  observations = broker->GetObservations(node, vtkDMMLTransformableNode::TransformModifiedEvent, this->External, this->External->GetDMMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
}

//---------------------------------------------------------------------------
void vtkDMMLLinearTransformsDisplayableManager3D::vtkInternal
::RemoveDisplayObservations(vtkDMMLTransformDisplayNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkEventBroker::ObservationVector observations;
  observations = broker->GetObservations(node, vtkDMMLTransformDisplayNode::TransformUpdateEditorBoundsEvent, this->External, this->External->GetDMMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
}

//---------------------------------------------------------------------------
void vtkDMMLLinearTransformsDisplayableManager3D::vtkInternal::ClearDisplayableNodes()
{
  while(this->TransformToDisplayNodes.size() > 0)
    {
    this->RemoveTransformNode(this->TransformToDisplayNodes.begin()->first);
    }
}

//---------------------------------------------------------------------------
bool vtkDMMLLinearTransformsDisplayableManager3D::vtkInternal::UseDisplayableNode(vtkDMMLTransformNode* node)
{
  bool use = node && node->IsA("vtkDMMLTransformNode");
  return use;
}

//---------------------------------------------------------------------------
// vtkDMMLLinearTransformsDisplayableManager3D methods

//---------------------------------------------------------------------------
vtkDMMLLinearTransformsDisplayableManager3D::vtkDMMLLinearTransformsDisplayableManager3D()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkDMMLLinearTransformsDisplayableManager3D::~vtkDMMLLinearTransformsDisplayableManager3D()
{
  delete this->Internal;
  this->Internal=nullptr;
}

//---------------------------------------------------------------------------
void vtkDMMLLinearTransformsDisplayableManager3D::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf ( os, indent );
  os << indent << "vtkDMMLLinearTransformsDisplayableManager3D: "
     << this->GetClassName() << "\n";
}

//---------------------------------------------------------------------------
void vtkDMMLLinearTransformsDisplayableManager3D::OnDMMLSceneNodeAdded(vtkDMMLNode* node)
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
void vtkDMMLLinearTransformsDisplayableManager3D::OnDMMLSceneNodeRemoved(vtkDMMLNode* node)
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
void vtkDMMLLinearTransformsDisplayableManager3D::ProcessDMMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  vtkDMMLScene* scene = this->GetDMMLScene();

  if (scene == nullptr || scene->IsBatchProcessing())
    {
    return;
    }

  vtkDMMLTransformNode* displayableNode = vtkDMMLTransformNode::SafeDownCast(caller);
  vtkDMMLTransformDisplayNode* displayNode = vtkDMMLTransformDisplayNode::SafeDownCast(caller);

  if ( displayableNode )
    {
    vtkDMMLNode* callDataNode = reinterpret_cast<vtkDMMLDisplayNode *> (callData);
    displayNode = vtkDMMLTransformDisplayNode::SafeDownCast(callDataNode);

    if ( displayNode && (event == vtkDMMLDisplayableNode::DisplayModifiedEvent) )
      {
      this->Internal->UpdateDisplayNode(displayNode);
      this->RequestRender();
      }
    else if (event == vtkDMMLTransformableNode::TransformModifiedEvent)
      {
      this->Internal->UpdateDisplayableTransforms(displayableNode, false);
      this->RequestRender();
      }
    }
  else if ( displayNode )
    {
    displayableNode = vtkDMMLTransformNode::SafeDownCast(displayNode->GetDisplayableNode());
    if ( displayNode && event == vtkDMMLTransformDisplayNode::TransformUpdateEditorBoundsEvent)
      {
      this->Internal->UpdateDisplayableTransforms(displayableNode, true);
      this->RequestRender();
      }
    }
  else
    {
    this->Superclass::ProcessDMMLNodesEvents(caller, event, callData);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLLinearTransformsDisplayableManager3D::UpdateFromDMML()
{
  this->SetUpdateFromDMMLRequested(false);

  vtkDMMLScene* scene = this->GetDMMLScene();
  if (!scene)
    {
    vtkDebugMacro( "vtkDMMLLinearTransformsDisplayableManager3D->UpdateFromDMML: Scene is not set.");
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
void vtkDMMLLinearTransformsDisplayableManager3D::UnobserveDMMLScene()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkDMMLLinearTransformsDisplayableManager3D::OnDMMLSceneStartClose()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkDMMLLinearTransformsDisplayableManager3D::OnDMMLSceneEndClose()
{
  this->SetUpdateFromDMMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkDMMLLinearTransformsDisplayableManager3D::OnDMMLSceneEndBatchProcess()
{
  this->SetUpdateFromDMMLRequested(true);
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkDMMLLinearTransformsDisplayableManager3D::Create()
{
  Superclass::Create();
  this->SetUpdateFromDMMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkDMMLLinearTransformsDisplayableManager3D
::ProcessWidgetsEvents(vtkObject* caller, unsigned long event, void* callData)
{
  Superclass::ProcessWidgetsEvents(caller, event, callData);

  vtkBoxWidget2* boxWidget = vtkBoxWidget2::SafeDownCast(caller);
  if (boxWidget)
    {
    this->Internal->UpdateNodeFromWidget(boxWidget);
    this->RequestRender();
    }
}

//---------------------------------------------------------------------------
vtkAbstractWidget* vtkDMMLLinearTransformsDisplayableManager3D
::GetWidget(vtkDMMLTransformDisplayNode* displayNode)
{
  vtkDMMLLinearTransformsDisplayableManager3D::vtkInternal
    ::PipelinesCacheType::iterator pipelineIter =
      this->Internal->DisplayPipelines.find(displayNode);
  if (pipelineIter != this->Internal->DisplayPipelines.end())
    {
    return pipelineIter->second->Widget;
    }
  return nullptr;
}
