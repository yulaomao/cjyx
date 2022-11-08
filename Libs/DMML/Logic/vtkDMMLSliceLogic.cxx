/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLSliceLogic.cxx,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

// DMMLLogic includes
#include "vtkDMMLApplicationLogic.h"
#include "vtkDMMLSliceLogic.h"
#include "vtkDMMLSliceLayerLogic.h"

// DMML includes
#include <vtkEventBroker.h>
#include <vtkDMMLCrosshairNode.h>
#include <vtkDMMLDiffusionTensorVolumeSliceDisplayNode.h>
#include <vtkDMMLGlyphableVolumeDisplayNode.h>
#include <vtkDMMLLinearTransformNode.h>
#include <vtkDMMLModelNode.h>
#include <vtkDMMLProceduralColorNode.h>
#include <vtkDMMLScalarVolumeDisplayNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceCompositeNode.h>
#include <vtkDMMLSliceDisplayNode.h>

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkGeneralTransform.h>
#include <vtkImageAppendComponents.h>
#include <vtkImageBlend.h>
#include <vtkImageResample.h>
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkImageMathematics.h>
#include <vtkImageReslice.h>
#include <vtkImageThreshold.h>
#include <vtkInformation.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPlaneSource.h>
#include <vtkPolyDataCollection.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTransform.h>
#include <vtkVersion.h>

// VTKAddon includes
#include <vtkAddonMathUtilities.h>

// STD includes
#include <algorithm>

//----------------------------------------------------------------------------
const int vtkDMMLSliceLogic::SLICE_INDEX_ROTATED=-1;
const int vtkDMMLSliceLogic::SLICE_INDEX_OUT_OF_VOLUME=-2;
const int vtkDMMLSliceLogic::SLICE_INDEX_NO_VOLUME=-3;
const std::string vtkDMMLSliceLogic::SLICE_MODEL_NODE_NAME_SUFFIX = std::string("Volume Slice");

//----------------------------------------------------------------------------
struct SliceLayerInfo
  {
  SliceLayerInfo(vtkAlgorithmOutput* blendInput, double opacity)
    {
    this->BlendInput = blendInput;
    this->Opacity = opacity;
    }
  vtkSmartPointer<vtkAlgorithmOutput> BlendInput;
  double Opacity;
  };

//----------------------------------------------------------------------------
struct BlendPipeline
{
  BlendPipeline()
  {
    /*
    // AlphaBlending, ReverseAlphaBlending:
    //
    //   foreground \
    //               > Blend
    //   background /
    //
    // Add, Subtract:
    //
    //   Casting is needed to avoid overflow during adding (or subtracting).
    //
    //   AddSubMath adds/subtracts alpha channel, therefore we copy RGB and alpha
    //   components and copy of the background's alpha channel to the output.
    //   Splitting and appending channels is probably quite inefficient, but there does not
    //   seem to be simpler pipeline to do this in VTK.
    //
    //   foreground > AddSubForegroundCast \
    //                                      > AddSubMath > AddSubOutputCast ...
    //   background > AddSubBackroundCast  /
    //
    //
    //     ... AddSubOutputCast > AddSubExtractRGB \
    //                                              > AddSubAppendRGBA > Blend
    //             background > AddSubExtractAlpha /
    */

    this->AddSubForegroundCast->SetOutputScalarTypeToShort();
    this->AddSubBackgroundCast->SetOutputScalarTypeToShort();
    this->AddSubMath->SetOperationToAdd();
    this->AddSubMath->SetInputConnection(0, this->AddSubBackgroundCast->GetOutputPort());
    this->AddSubMath->SetInputConnection(1, this->AddSubForegroundCast->GetOutputPort());
    this->AddSubOutputCast->SetInputConnection(this->AddSubMath->GetOutputPort());

    this->AddSubExtractRGB->SetInputConnection(this->AddSubOutputCast->GetOutputPort());
    this->AddSubExtractRGB->SetComponents(0, 1, 2);
    this->AddSubExtractAlpha->SetComponents(3);
    this->AddSubAppendRGBA->AddInputConnection(this->AddSubExtractRGB->GetOutputPort());
    this->AddSubAppendRGBA->AddInputConnection(this->AddSubExtractAlpha->GetOutputPort());

    this->AddSubOutputCast->SetOutputScalarTypeToUnsignedChar();
    this->AddSubOutputCast->ClampOverflowOn();
  }

  void AddLayers(std::deque<SliceLayerInfo>& layers, int sliceCompositing,
    vtkAlgorithmOutput* backgroundImagePort,
    vtkAlgorithmOutput* foregroundImagePort, double foregroundOpacity,
    vtkAlgorithmOutput* labelImagePort, double labelOpacity)
  {
    if (sliceCompositing == vtkDMMLSliceCompositeNode::Add || sliceCompositing == vtkDMMLSliceCompositeNode::Subtract)
      {
      if (!backgroundImagePort || !foregroundImagePort)
        {
        // not enough inputs for add/subtract, so use alpha blending pipeline
        sliceCompositing = vtkDMMLSliceCompositeNode::Alpha;
        }
      }

    if (sliceCompositing == vtkDMMLSliceCompositeNode::Alpha)
      {
      if (backgroundImagePort)
        {
        layers.emplace_back(backgroundImagePort, 1.0);
        }
      if (foregroundImagePort)
        {
        layers.emplace_back(foregroundImagePort, foregroundOpacity);
        }
      }
    else if (sliceCompositing == vtkDMMLSliceCompositeNode::ReverseAlpha)
      {
      if (foregroundImagePort)
        {
        layers.emplace_back(foregroundImagePort, 1.0);
        }
      if (backgroundImagePort)
        {
        layers.emplace_back(backgroundImagePort, foregroundOpacity);
        }
      }
    else
      {
      this->AddSubForegroundCast->SetInputConnection(foregroundImagePort);
      this->AddSubBackgroundCast->SetInputConnection(backgroundImagePort);
      this->AddSubExtractAlpha->SetInputConnection(backgroundImagePort);
      if (sliceCompositing == vtkDMMLSliceCompositeNode::Add)
        {
        this->AddSubMath->SetOperationToAdd();
        }
      else
        {
        this->AddSubMath->SetOperationToSubtract();
        }
      layers.emplace_back(this->AddSubAppendRGBA->GetOutputPort(), 1.0);
      }

    // always blending the label layer
    if (labelImagePort)
      {
      layers.emplace_back(labelImagePort, labelOpacity);
      }
  }

  vtkNew<vtkImageCast> AddSubForegroundCast;
  vtkNew<vtkImageCast> AddSubBackgroundCast;
  vtkNew<vtkImageMathematics> AddSubMath;
  vtkNew<vtkImageExtractComponents> AddSubExtractRGB;
  vtkNew<vtkImageExtractComponents> AddSubExtractAlpha;
  vtkNew<vtkImageAppendComponents> AddSubAppendRGBA;
  vtkNew<vtkImageCast> AddSubOutputCast;
  vtkNew<vtkImageBlend> Blend;
};

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLSliceLogic);

//----------------------------------------------------------------------------
vtkDMMLSliceLogic::vtkDMMLSliceLogic()
{
  this->BackgroundLayer = nullptr;
  this->ForegroundLayer = nullptr;
  this->LabelLayer = nullptr;
  this->SliceNode = nullptr;
  this->SliceCompositeNode = nullptr;

  this->Pipeline = new BlendPipeline;
  this->PipelineUVW = new BlendPipeline;

  this->ExtractModelTexture = vtkImageReslice::New();
  this->ExtractModelTexture->SetOutputDimensionality (2);
  this->ExtractModelTexture->SetInputConnection(this->PipelineUVW->Blend->GetOutputPort());

  this->SliceModelNode = nullptr;
  this->SliceModelTransformNode = nullptr;
  this->SliceModelDisplayNode = nullptr;
  this->ImageDataConnection = nullptr;
  this->SliceSpacing[0] = this->SliceSpacing[1] = this->SliceSpacing[2] = 1;
  this->AddingSliceModelNodes = false;
}

//----------------------------------------------------------------------------
vtkDMMLSliceLogic::~vtkDMMLSliceLogic()
{
  this->SetSliceNode(nullptr);

  if (this->ImageDataConnection)
    {
    this->ImageDataConnection = nullptr;
    }

  delete this->Pipeline;
  delete this->PipelineUVW;

  if (this->ExtractModelTexture)
    {
    this->ExtractModelTexture->Delete();
    this->ExtractModelTexture = nullptr;
    }

  this->SetBackgroundLayer (nullptr);
  this->SetForegroundLayer (nullptr);
  this->SetLabelLayer (nullptr);

  if (this->SliceCompositeNode)
    {
    vtkSetAndObserveDMMLNodeMacro( this->SliceCompositeNode, 0);
    }

  this->DeleteSliceModel();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::SetDMMLSceneInternal(vtkDMMLScene * newScene)
{
  // List of events the slice logics should listen
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkDMMLScene::EndBatchProcessEvent);
  events->InsertNextValue(vtkDMMLScene::StartCloseEvent);
  events->InsertNextValue(vtkDMMLScene::EndImportEvent);
  events->InsertNextValue(vtkDMMLScene::EndRestoreEvent);
  events->InsertNextValue(vtkDMMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkDMMLScene::NodeRemovedEvent);

  this->SetAndObserveDMMLSceneEventsInternal(newScene, events.GetPointer());

  this->ProcessDMMLLogicsEvents();

  this->BackgroundLayer->SetDMMLScene(newScene);
  this->ForegroundLayer->SetDMMLScene(newScene);
  this->LabelLayer->SetDMMLScene(newScene);

  this->ProcessDMMLSceneEvents(newScene, vtkDMMLScene::EndBatchProcessEvent, nullptr);
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::UpdateSliceNode()
{
  if (!this->GetDMMLScene())
    {
    this->SetSliceNode(nullptr);
    }

}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::UpdateSliceNodeFromLayout()
{
  if (this->SliceNode == nullptr)
    {
    return;
    }
  this->SliceNode->SetOrientationToDefault();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::UpdateSliceCompositeNode()
{
  if (!this->GetDMMLScene() || !this->SliceNode)
    {
    this->SetSliceCompositeNode(nullptr);
    return;
    }

  // find SliceCompositeNode in the scene
  std::string layoutName = (this->SliceNode->GetLayoutName() ? this->SliceNode->GetLayoutName() : "");
  vtkDMMLSliceCompositeNode* updatedSliceCompositeNode = vtkDMMLSliceLogic::GetSliceCompositeNode(this->GetDMMLScene(), layoutName.c_str());

  if (this->SliceCompositeNode && updatedSliceCompositeNode &&
       (!this->SliceCompositeNode->GetID() || strcmp(this->SliceCompositeNode->GetID(), updatedSliceCompositeNode->GetID()) != 0) )
    {
    // local SliceCompositeNode is out of sync with the scene
    this->SetSliceCompositeNode(nullptr);
    }

  if (!this->SliceCompositeNode)
    {
    if (!updatedSliceCompositeNode && !layoutName.empty())
      {
      // Use CreateNodeByClass instead of New to use default node specified in the scene
      updatedSliceCompositeNode = vtkDMMLSliceCompositeNode::SafeDownCast(this->GetDMMLScene()->CreateNodeByClass("vtkDMMLSliceCompositeNode"));
      updatedSliceCompositeNode->SetLayoutName(layoutName.c_str());
      this->GetDMMLScene()->AddNode(updatedSliceCompositeNode);
      this->SetSliceCompositeNode(updatedSliceCompositeNode);
      updatedSliceCompositeNode->Delete();
      }
    else
      {
      this->SetSliceCompositeNode(updatedSliceCompositeNode);
      }
    }
}

//----------------------------------------------------------------------------
bool vtkDMMLSliceLogic::EnterDMMLCallback()const
{
  return this->AddingSliceModelNodes == false;
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::UpdateFromDMMLScene()
{
  this->UpdateSliceNodes();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::OnDMMLSceneNodeAdded(vtkDMMLNode* node)
{
  if (!(node->IsA("vtkDMMLSliceCompositeNode")
        || node->IsA("vtkDMMLSliceNode")
        || node->IsA("vtkDMMLVolumeNode")))
    {
    return;
    }
  this->UpdateSliceNodes();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::OnDMMLSceneNodeRemoved(vtkDMMLNode* node)
{
  if (!(node->IsA("vtkDMMLSliceCompositeNode")
        || node->IsA("vtkDMMLSliceNode")
        || node->IsA("vtkDMMLVolumeNode")))
    {
    return;
    }
  this->UpdateSliceNodes();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::OnDMMLSceneStartClose()
{
  this->UpdateSliceNodeFromLayout();
  this->DeleteSliceModel();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::OnDMMLSceneEndImport()
{
  this->SetupCrosshairNode();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::OnDMMLSceneEndRestore()
{
  this->SetupCrosshairNode();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::UpdateSliceNodes()
{
  if (this->GetDMMLScene()
      && this->GetDMMLScene()->IsBatchProcessing())
    {
    return;
    }
  // Set up the nodes
  this->UpdateSliceNode();
  this->UpdateSliceCompositeNode();

  // Set up the models
  this->CreateSliceModel();

  this->UpdatePipeline();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::SetupCrosshairNode()
{
  //
  // On a new scene or restore, create the singleton for the default crosshair
  // for navigation or cursor if it doesn't already exist in scene
  //
  bool foundDefault = false;
  vtkDMMLNode* node;
  vtkCollectionSimpleIterator it;
  vtkSmartPointer<vtkCollection> crosshairs = vtkSmartPointer<vtkCollection>::Take(this->GetDMMLScene()->GetNodesByClass("vtkDMMLCrosshairNode"));
  for (crosshairs->InitTraversal(it);
       (node = (vtkDMMLNode*)crosshairs->GetNextItemAsObject(it)) ;)
    {
    vtkDMMLCrosshairNode* crosshairNode =
      vtkDMMLCrosshairNode::SafeDownCast(node);
    if (crosshairNode
        && crosshairNode->GetCrosshairName() == std::string("default"))
      {
      foundDefault = true;
      break;
      }
    }

  if (!foundDefault)
    {
    vtkNew<vtkDMMLCrosshairNode> crosshair;
    this->GetDMMLScene()->AddNode(crosshair.GetPointer());
    }
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::OnDMMLNodeModified(vtkDMMLNode* node)
{
  assert(node);
  if (this->GetDMMLScene()->IsBatchProcessing())
    {
    return;
    }

  /// set slice extents in the layes
  this->SetSliceExtentsToSliceNode();

  // Update from SliceNode
  if (node == this->SliceNode)
    {
    // assert (sliceNode == this->SliceNode); not an assert because the node
    // might have change in CreateSliceModel() or UpdateSliceNode()
    vtkDMMLDisplayNode* sliceDisplayNode =
      this->SliceModelNode ? this->SliceModelNode->GetModelDisplayNode() : nullptr;
    if ( sliceDisplayNode)
      {
      sliceDisplayNode->SetVisibility( this->SliceNode->GetSliceVisible() );
      sliceDisplayNode->SetViewNodeIDs( this->SliceNode->GetThreeDViewIDs());
      }
    }
  else if (node == this->SliceCompositeNode)
    {
    this->UpdatePipeline();
    this->InvokeEvent(CompositeModifiedEvent);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic
::ProcessDMMLLogicsEvents(vtkObject* vtkNotUsed(caller),
                          unsigned long vtkNotUsed(event),
                          void* vtkNotUsed(callData))
{
  this->ProcessDMMLLogicsEvents();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::ProcessDMMLLogicsEvents()
{
  // Slice update may trigger redrawing many nodes, pause the render to
  // not spend time with intermediate renderings
  vtkDMMLApplicationLogic* appLogic = this->GetDMMLApplicationLogic();
  if (appLogic)
    {
    appLogic->PauseRender();
    }

  //
  // if we don't have layers yet, create them
  //
  if ( this->BackgroundLayer == nullptr )
    {
    vtkNew<vtkDMMLSliceLayerLogic> layer;
    this->SetBackgroundLayer(layer.GetPointer());
    }
  if ( this->ForegroundLayer == nullptr )
    {
    vtkNew<vtkDMMLSliceLayerLogic> layer;
    this->SetForegroundLayer(layer.GetPointer());
    }
  if ( this->LabelLayer == nullptr )
    {
    vtkNew<vtkDMMLSliceLayerLogic> layer;
    // turn on using the label outline only in this layer
    layer->IsLabelLayerOn();
    this->SetLabelLayer(layer.GetPointer());
    }
  // Update slice plane geometry
  if (this->SliceNode != nullptr
      && this->GetSliceModelNode() != nullptr
      && this->GetDMMLScene() != nullptr
      && this->GetDMMLScene()->GetNodeByID( this->SliceModelNode->GetID() ) != nullptr
      && this->SliceModelNode->GetPolyData() != nullptr )
    {
    int *dims1=nullptr;
    int dims[3];
    vtkMatrix4x4 *textureToRAS = nullptr;
    if (this->SliceNode->GetSliceResolutionMode() != vtkDMMLSliceNode::SliceResolutionMatch2DView)
      {
      textureToRAS = this->SliceNode->GetUVWToRAS();
      dims1 = this->SliceNode->GetUVWDimensions();
      dims[0] = dims1[0]-1;
      dims[1] = dims1[1]-1;
      }
    else
      {
      textureToRAS = this->SliceNode->GetXYToRAS();
      dims1 = this->SliceNode->GetDimensions();
      dims[0] = dims1[0];
      dims[1] = dims1[1];
      }

    // Force non-zero dimension to avoid "Bad plane coordinate system"
    // error from vtkPlaneSource when slice viewers have a height or width
    // of zero.
    dims[0] = std::max(1, dims[0]);
    dims[1] = std::max(1, dims[1]);

    // set the plane corner point for use in a model
    double inPt[4]={0,0,0,1};
    double outPt[4];
    double *outPt3 = outPt;

    // set the z position to be the active slice (from the lightbox)
    inPt[2] = this->SliceNode->GetActiveSlice();

    vtkPlaneSource* plane = vtkPlaneSource::SafeDownCast(
      this->SliceModelNode->GetPolyDataConnection()->GetProducer());

    int wasModified = this->SliceModelNode->StartModify();

    textureToRAS->MultiplyPoint(inPt, outPt);
    plane->SetOrigin(outPt3);

    inPt[0] = dims[0];
    textureToRAS->MultiplyPoint(inPt, outPt);
    plane->SetPoint1(outPt3);

    inPt[0] = 0;
    inPt[1] = dims[1];
    textureToRAS->MultiplyPoint(inPt, outPt);
    plane->SetPoint2(outPt3);

    this->SliceModelNode->EndModify(wasModified);

    this->UpdatePipeline();
    /// \tbd Ideally it should not be fired if the output polydata is not
    /// modified.
    plane->Modified();

    vtkDMMLModelDisplayNode *modelDisplayNode = this->SliceModelNode->GetModelDisplayNode();
    if ( modelDisplayNode )
      {
      if (this->LabelLayer && this->LabelLayer->GetImageDataConnectionUVW())
        {
        modelDisplayNode->SetInterpolateTexture(0);
        }
      else
        {
        modelDisplayNode->SetInterpolateTexture(1);
        }
      }
    }

  // This is called when a slice layer is modified, so pass it on
  // to anyone interested in changes to this sub-pipeline
  this->Modified();

  // All the updates are done, allow rendering again
  if (appLogic)
    {
    appLogic->ResumeRender();
    }
}

//----------------------------------------------------------------------------
vtkDMMLSliceNode* vtkDMMLSliceLogic::AddSliceNode(const char* layoutName)
{
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("vtkDMMLSliceLogic::AddSliceNode failed: scene is not set");
    return nullptr;
    }
  vtkSmartPointer<vtkDMMLSliceNode> node = vtkSmartPointer<vtkDMMLSliceNode>::Take(
    vtkDMMLSliceNode::SafeDownCast(this->GetDMMLScene()->CreateNodeByClass("vtkDMMLSliceNode")));
  node->SetName(layoutName);
  node->SetLayoutName(layoutName);
  this->GetDMMLScene()->AddNode(node);
  this->SetSliceNode(node);
  this->UpdateSliceNodeFromLayout();
  return node;
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::SetSliceNode(vtkDMMLSliceNode * newSliceNode)
{
  if (this->SliceNode == newSliceNode)
    {
    return;
    }

  // Observe the slice node for general properties like slice visibility.
  // But the slice layers will also notify us when things like transforms have
  // changed.
  // This class takes care of passing the one slice node to each of the layers
  // so that users of this class only need to set the node one place.
  vtkSetAndObserveDMMLNodeMacro( this->SliceNode, newSliceNode );

  this->UpdateSliceCompositeNode();

  if (this->BackgroundLayer)
    {
    this->BackgroundLayer->SetSliceNode(newSliceNode);
    }
  if (this->ForegroundLayer)
    {
    this->ForegroundLayer->SetSliceNode(newSliceNode);
    }
  if (this->LabelLayer)
    {
    this->LabelLayer->SetSliceNode(newSliceNode);
    }

  this->Modified();

}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::SetSliceCompositeNode(vtkDMMLSliceCompositeNode *sliceCompositeNode)
{
  if (this->SliceCompositeNode == sliceCompositeNode)
    {
    return;
    }

  // Observe the composite node, since this holds the parameters for this pipeline
  vtkSetAndObserveDMMLNodeMacro( this->SliceCompositeNode, sliceCompositeNode );
  this->UpdatePipeline();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::SetBackgroundLayer(vtkDMMLSliceLayerLogic *backgroundLayer)
{
  // TODO: Simplify the whole set using a macro similar to vtkDMMLSetAndObserve
  if (this->BackgroundLayer)
    {
    this->BackgroundLayer->SetDMMLScene( nullptr );
    this->BackgroundLayer->Delete();
    }
  this->BackgroundLayer = backgroundLayer;

  if (this->BackgroundLayer)
    {
    this->BackgroundLayer->Register(this);

    this->BackgroundLayer->SetDMMLScene(this->GetDMMLScene());

    this->BackgroundLayer->SetSliceNode(SliceNode);
    vtkEventBroker::GetInstance()->AddObservation(
      this->BackgroundLayer, vtkCommand::ModifiedEvent,
      this, this->GetDMMLLogicsCallbackCommand());
    }

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::SetForegroundLayer(vtkDMMLSliceLayerLogic *foregroundLayer)
{
  // TODO: Simplify the whole set using a macro similar to vtkDMMLSetAndObserve
  if (this->ForegroundLayer)
    {
    this->ForegroundLayer->SetDMMLScene( nullptr );
    this->ForegroundLayer->Delete();
    }
  this->ForegroundLayer = foregroundLayer;

  if (this->ForegroundLayer)
    {
    this->ForegroundLayer->Register(this);
    this->ForegroundLayer->SetDMMLScene( this->GetDMMLScene());

    this->ForegroundLayer->SetSliceNode(SliceNode);
    vtkEventBroker::GetInstance()->AddObservation(
      this->ForegroundLayer, vtkCommand::ModifiedEvent,
      this, this->GetDMMLLogicsCallbackCommand());
    }

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::SetLabelLayer(vtkDMMLSliceLayerLogic *labelLayer)
{
  // TODO: Simplify the whole set using a macro similar to vtkDMMLSetAndObserve
  if (this->LabelLayer)
    {
    this->LabelLayer->SetDMMLScene( nullptr );
    this->LabelLayer->Delete();
    }
  this->LabelLayer = labelLayer;

  if (this->LabelLayer)
    {
    this->LabelLayer->Register(this);

    this->LabelLayer->SetDMMLScene(this->GetDMMLScene());

    this->LabelLayer->SetSliceNode(SliceNode);
    vtkEventBroker::GetInstance()->AddObservation(
      this->LabelLayer, vtkCommand::ModifiedEvent,
      this, this->GetDMMLLogicsCallbackCommand());
    }

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic
::SetWindowLevel(double newWindow, double newLevel, int layer)
{
  vtkDMMLScalarVolumeNode* volumeNode =
    vtkDMMLScalarVolumeNode::SafeDownCast( this->GetLayerVolumeNode (layer) );
  vtkDMMLScalarVolumeDisplayNode* volumeDisplayNode =
    volumeNode ? volumeNode->GetScalarVolumeDisplayNode() : nullptr;
  if (!volumeDisplayNode)
    {
    return;
    }
  int disabledModify = volumeDisplayNode->StartModify();
  volumeDisplayNode->SetAutoWindowLevel(0);
  volumeDisplayNode->SetWindowLevel(newWindow, newLevel);
  volumeDisplayNode->EndModify(disabledModify);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic
::SetBackgroundWindowLevel(double newWindow, double newLevel)
{
  // 0 is background layer, defined in this::GetLayerVolumeNode
  SetWindowLevel(newWindow, newLevel, 0);
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic
::SetForegroundWindowLevel(double newWindow, double newLevel)
{
  // 1 is foreground layer, defined in this::GetLayerVolumeNode
  SetWindowLevel(newWindow, newLevel, 1);
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic
::GetBackgroundWindowLevelAndRange(double& window, double& level,
                                         double& rangeLow, double& rangeHigh)
{
  bool autoWindowLevel; // unused, just a placeholder to allow calling the method
  this->GetBackgroundWindowLevelAndRange(window, level, rangeLow, rangeHigh, autoWindowLevel);
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic
::GetBackgroundWindowLevelAndRange(double& window, double& level,
                                         double& rangeLow, double& rangeHigh, bool& autoWindowLevel)
{
  vtkDMMLScalarVolumeNode* volumeNode =
    vtkDMMLScalarVolumeNode::SafeDownCast( this->GetLayerVolumeNode (0) );
    // 0 is background layer, defined in this::GetLayerVolumeNode
  vtkDMMLScalarVolumeDisplayNode* volumeDisplayNode = nullptr;
  if (volumeNode)
    {
     volumeDisplayNode =
      vtkDMMLScalarVolumeDisplayNode::SafeDownCast( volumeNode->GetVolumeDisplayNode() );
    }
  vtkImageData* imageData;
  if (volumeDisplayNode && (imageData = volumeNode->GetImageData()) )
    {
    window = volumeDisplayNode->GetWindow();
    level = volumeDisplayNode->GetLevel();
    double range[2];
    imageData->GetScalarRange(range);
    rangeLow = range[0];
    rangeHigh = range[1];
    autoWindowLevel = (volumeDisplayNode->GetAutoWindowLevel() != 0);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic
::GetForegroundWindowLevelAndRange(double& window, double& level,
                                         double& rangeLow, double& rangeHigh)
{
  bool autoWindowLevel; // unused, just a placeholder to allow calling the method
  this->GetForegroundWindowLevelAndRange(window, level, rangeLow, rangeHigh, autoWindowLevel);
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic
::GetForegroundWindowLevelAndRange(double& window, double& level,
                                         double& rangeLow, double& rangeHigh, bool& autoWindowLevel)
{
  vtkDMMLScalarVolumeNode* volumeNode =
    vtkDMMLScalarVolumeNode::SafeDownCast( this->GetLayerVolumeNode (1) );
    // 0 is background layer, defined in this::GetLayerVolumeNode
  vtkDMMLScalarVolumeDisplayNode* volumeDisplayNode = nullptr;
  if (volumeNode)
    {
     volumeDisplayNode =
      vtkDMMLScalarVolumeDisplayNode::SafeDownCast( volumeNode->GetVolumeDisplayNode() );
    }
  vtkImageData* imageData;
  if (volumeDisplayNode && (imageData = volumeNode->GetImageData()) )
    {
    window = volumeDisplayNode->GetWindow();
    level = volumeDisplayNode->GetLevel();
    double range[2];
    imageData->GetScalarRange(range);
    rangeLow = range[0];
    rangeHigh = range[1];
    autoWindowLevel = (volumeDisplayNode->GetAutoWindowLevel() != 0);
    }
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput * vtkDMMLSliceLogic::GetImageDataConnection()
{
  return this->ImageDataConnection;
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::UpdateImageData ()
{
  if (this->SliceNode->GetSliceResolutionMode() == vtkDMMLSliceNode::SliceResolutionMatch2DView)
    {
    this->ExtractModelTexture->SetInputConnection( this->Pipeline->Blend->GetOutputPort() );
    this->ImageDataConnection = this->Pipeline->Blend->GetOutputPort();
    }
  else
    {
    this->ExtractModelTexture->SetInputConnection( this->PipelineUVW->Blend->GetOutputPort() );
    }
  // It seems very strange that the imagedata can be null.
  // It should probably be always a valid imagedata with invalid bounds if needed

  if ( (this->GetBackgroundLayer() != nullptr && this->GetBackgroundLayer()->GetImageDataConnection() != nullptr) ||
       (this->GetForegroundLayer() != nullptr && this->GetForegroundLayer()->GetImageDataConnection() != nullptr) ||
       (this->GetLabelLayer() != nullptr && this->GetLabelLayer()->GetImageDataConnection() != nullptr) )
    {
    if (this->ImageDataConnection == nullptr || this->Pipeline->Blend->GetOutputPort()->GetMTime() > this->ImageDataConnection->GetMTime())
      {
      this->ImageDataConnection = this->Pipeline->Blend->GetOutputPort();
      }
    }
  else
    {
    this->ImageDataConnection = nullptr;
    if (this->SliceNode->GetSliceResolutionMode() == vtkDMMLSliceNode::SliceResolutionMatch2DView)
      {
      this->ExtractModelTexture->SetInputConnection( this->ImageDataConnection );
      }
    else
      {
      this->ExtractModelTexture->SetInputConnection(this->PipelineUVW->Blend->GetOutputPort());
      }
    }
}

//----------------------------------------------------------------------------
bool vtkDMMLSliceLogic::UpdateBlendLayers(vtkImageBlend* blend, const std::deque<SliceLayerInfo> &layers)
{
  const int blendPort = 0;
  vtkMTimeType oldBlendMTime = blend->GetMTime();

  bool layersChanged = false;
  int numberOfLayers = layers.size();
  if (numberOfLayers == blend->GetNumberOfInputConnections(blendPort))
    {
    int layerIndex = 0;
    for (std::deque<SliceLayerInfo>::const_iterator layerIt = layers.begin(); layerIt != layers.end(); ++layerIt, ++layerIndex)
      {
      if (layerIt->BlendInput != blend->GetInputConnection(blendPort, layerIndex))
        {
        layersChanged = true;
        break;
        }
      }
    }
  else
    {
    layersChanged = true;
    }
  if (layersChanged)
    {
    blend->RemoveAllInputs();
    int layerIndex = 0;
    for (std::deque<SliceLayerInfo>::const_iterator layerIt = layers.begin(); layerIt != layers.end(); ++layerIt, ++layerIndex)
      {
      blend->AddInputConnection(layerIt->BlendInput);
      }
    }

  // Update opacities
    {
    int layerIndex = 0;
    for (std::deque<SliceLayerInfo>::const_iterator layerIt = layers.begin(); layerIt != layers.end(); ++layerIt, ++layerIndex)
      {
      blend->SetOpacity(layerIndex, layerIt->Opacity);
      }
    }

  bool modified = (blend->GetMTime() > oldBlendMTime);
  return modified;
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::UpdatePipeline()
{
  int modified = 0;
  if ( this->SliceCompositeNode )
    {
    // get the background and foreground image data from the layers
    // so we can use them as input to the image blend
    // TODO: change logic to use a volume node superclass rather than
    // a scalar volume node once the superclass is sorted out for vector/tensor Volumes

    const char *id;

    // Background
    id = this->SliceCompositeNode->GetBackgroundVolumeID();
    vtkDMMLVolumeNode *bgnode = nullptr;
    if (id)
      {
      bgnode = vtkDMMLVolumeNode::SafeDownCast (this->GetDMMLScene()->GetNodeByID(id));
      }

    if (this->BackgroundLayer)
      {
      if ( this->BackgroundLayer->GetVolumeNode() != bgnode )
        {
        this->BackgroundLayer->SetVolumeNode (bgnode);
        modified = 1;
        }
      }

    // Foreground
    id = this->SliceCompositeNode->GetForegroundVolumeID();
    vtkDMMLVolumeNode *fgnode = nullptr;
    if (id)
      {
      fgnode = vtkDMMLVolumeNode::SafeDownCast (this->GetDMMLScene()->GetNodeByID(id));
      }

    if (this->ForegroundLayer)
      {
      if ( this->ForegroundLayer->GetVolumeNode() != fgnode )
        {
        this->ForegroundLayer->SetVolumeNode (fgnode);
        modified = 1;
        }
      }

    // Label
    id = this->SliceCompositeNode->GetLabelVolumeID();
    vtkDMMLVolumeNode *lbnode = nullptr;
    if (id)
      {
      lbnode = vtkDMMLVolumeNode::SafeDownCast (this->GetDMMLScene()->GetNodeByID(id));
      }

    if (this->LabelLayer)
      {
      if ( this->LabelLayer->GetVolumeNode() != lbnode )
        {
        this->LabelLayer->SetVolumeNode (lbnode);
        modified = 1;
        }
      }

    /// set slice extents in the layers
    if (modified)
      {
      this->SetSliceExtentsToSliceNode();
      }

    // Now update the image blend with the background and foreground and label
    // -- layer 0 opacity is ignored, but since not all inputs may be non-0,
    //    we keep track so that someone could, for example, have a 0 background
    //    with a non-0 foreground and label and everything will work with the
    //    label opacity
    //

    vtkAlgorithmOutput* backgroundImagePort = this->BackgroundLayer ? this->BackgroundLayer->GetImageDataConnection() : nullptr;
    vtkAlgorithmOutput* foregroundImagePort = this->ForegroundLayer ? this->ForegroundLayer->GetImageDataConnection() : nullptr;

    vtkAlgorithmOutput* backgroundImagePortUVW = this->BackgroundLayer ? this->BackgroundLayer->GetImageDataConnectionUVW() : nullptr;
    vtkAlgorithmOutput* foregroundImagePortUVW = this->ForegroundLayer ? this->ForegroundLayer->GetImageDataConnectionUVW() : nullptr;

    vtkAlgorithmOutput* labelImagePort = this->LabelLayer ? this->LabelLayer->GetImageDataConnection() : nullptr;
    vtkAlgorithmOutput* labelImagePortUVW = this->LabelLayer ? this->LabelLayer->GetImageDataConnectionUVW() : nullptr;

    std::deque<SliceLayerInfo> layers;
    std::deque<SliceLayerInfo> layersUVW;

    this->Pipeline->AddLayers(layers, this->SliceCompositeNode->GetCompositing(),
      backgroundImagePort, foregroundImagePort, this->SliceCompositeNode->GetForegroundOpacity(),
      labelImagePort, this->SliceCompositeNode->GetLabelOpacity());
    this->PipelineUVW->AddLayers(layersUVW, this->SliceCompositeNode->GetCompositing(),
      backgroundImagePortUVW, foregroundImagePortUVW, this->SliceCompositeNode->GetForegroundOpacity(),
      labelImagePortUVW, this->SliceCompositeNode->GetLabelOpacity());

    if (this->UpdateBlendLayers(this->Pipeline->Blend.GetPointer(), layers))
      {
      modified = 1;
      }
    if (this->UpdateBlendLayers(this->PipelineUVW->Blend.GetPointer(), layersUVW))
      {
      modified = 1;
      }

    //Models
    this->UpdateImageData();
    vtkDMMLDisplayNode* displayNode = this->SliceModelNode ? this->SliceModelNode->GetModelDisplayNode() : nullptr;
    if ( displayNode && this->SliceNode )
      {
      displayNode->SetVisibility( this->SliceNode->GetSliceVisible() );
      displayNode->SetViewNodeIDs( this->SliceNode->GetThreeDViewIDs());
      if ( (this->SliceNode->GetSliceResolutionMode() != vtkDMMLSliceNode::SliceResolutionMatch2DView &&
          !((backgroundImagePortUVW != nullptr) || (foregroundImagePortUVW != nullptr) || (labelImagePortUVW != nullptr) ) ) ||
          (this->SliceNode->GetSliceResolutionMode() == vtkDMMLSliceNode::SliceResolutionMatch2DView &&
          !((backgroundImagePort != nullptr) || (foregroundImagePort != nullptr) || (labelImagePort != nullptr) ) ))
        {
        displayNode->SetTextureImageDataConnection(nullptr);
        }
      else if (displayNode->GetTextureImageDataConnection() != this->ExtractModelTexture->GetOutputPort())
        {
        displayNode->SetTextureImageDataConnection(this->ExtractModelTexture->GetOutputPort());
        }
        if ( this->LabelLayer && this->LabelLayer->GetImageDataConnection())
          {
          displayNode->SetInterpolateTexture(0);
          }
        else
          {
          displayNode->SetInterpolateTexture(1);
          }
       }
    if ( modified )
      {
      if (this->SliceModelNode && this->SliceModelNode->GetPolyData())
        {
        this->SliceModelNode->GetPolyData()->Modified();
        }
      this->Modified();
      }
    }
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  vtkIndent nextIndent;
  nextIndent = indent.GetNextIndent();

  os << indent << "CjyxSliceLogic:             " << this->GetClassName() << "\n";

  if (this->SliceNode)
    {
    os << indent << "SliceNode: ";
    os << (this->SliceNode->GetID() ? this->SliceNode->GetID() : "(0 ID)") << "\n";
    this->SliceNode->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << "SliceNode: (none)\n";
    }

  if (this->SliceCompositeNode)
    {
    os << indent << "SliceCompositeNode: ";
    os << (this->SliceCompositeNode->GetID() ? this->SliceCompositeNode->GetID() : "(0 ID)") << "\n";
    this->SliceCompositeNode->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << "SliceCompositeNode: (none)\n";
    }

  if (this->BackgroundLayer)
    {
    os << indent << "BackgroundLayer: ";
    this->BackgroundLayer->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << "BackgroundLayer: (none)\n";
    }

  if (this->ForegroundLayer)
    {
    os << indent << "ForegroundLayer: ";
    this->ForegroundLayer->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << "ForegroundLayer: (none)\n";
    }

  if (this->LabelLayer)
    {
    os << indent << "LabelLayer: ";
    this->LabelLayer->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << "LabelLayer: (none)\n";
    }

  if (this->Pipeline->Blend.GetPointer())
    {
    os << indent << "Blend: ";
    this->Pipeline->Blend->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << "Blend: (none)\n";
    }

  if (this->PipelineUVW->Blend.GetPointer())
    {
    os << indent << "BlendUVW: ";
    this->PipelineUVW->Blend->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << "BlendUVW: (none)\n";
    }

  os << indent << "SLICE_MODEL_NODE_NAME_SUFFIX: " << this->SLICE_MODEL_NODE_NAME_SUFFIX << "\n";

}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::DeleteSliceModel()
{
  // Remove References
  if (this->SliceModelNode != nullptr)
    {
    this->SliceModelNode->SetAndObserveDisplayNodeID(nullptr);
    this->SliceModelNode->SetAndObserveTransformNodeID(nullptr);
    this->SliceModelNode->SetPolyDataConnection(nullptr);
    }
  if (this->SliceModelDisplayNode != nullptr)
    {
    this->SliceModelDisplayNode->SetTextureImageDataConnection(nullptr);
    }

  // Remove Nodes
  if (this->SliceModelNode != nullptr)
    {
    if (this->GetDMMLScene() && this->GetDMMLScene()->IsNodePresent(this->SliceModelNode))
      {
      this->GetDMMLScene()->RemoveNode(this->SliceModelNode);
      }
    this->SliceModelNode->Delete();
    this->SliceModelNode = nullptr;
    }
  if (this->SliceModelDisplayNode != nullptr)
    {
    if (this->GetDMMLScene() && this->GetDMMLScene()->IsNodePresent(this->SliceModelDisplayNode))
      {
      this->GetDMMLScene()->RemoveNode(this->SliceModelDisplayNode);
      }
    this->SliceModelDisplayNode->Delete();
    this->SliceModelDisplayNode = nullptr;
    }
  if (this->SliceModelTransformNode != nullptr)
    {
    if (this->GetDMMLScene() && this->GetDMMLScene()->IsNodePresent(this->SliceModelTransformNode))
      {
      this->GetDMMLScene()->RemoveNode(this->SliceModelTransformNode);
      }
    this->SliceModelTransformNode->Delete();
    this->SliceModelTransformNode = nullptr;
    }
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::CreateSliceModel()
{
  if(!this->GetDMMLScene())
    {
    return;
    }

  if (this->SliceModelNode != nullptr &&
      this->GetDMMLScene()->GetNodeByID(this->GetSliceModelNode()->GetID()) == nullptr )
    {
    this->DeleteSliceModel();
    }

  if ( this->SliceModelNode == nullptr)
    {
    this->SliceModelNode = vtkDMMLModelNode::New();
    this->SliceModelNode->SetScene(this->GetDMMLScene());
    this->SliceModelNode->SetDisableModifiedEvent(1);

    this->SliceModelNode->SetHideFromEditors(1);
    // allow point picking (e.g., placing a markups point on the slice node)
    this->SliceModelNode->SetSelectable(1);
    this->SliceModelNode->SetSaveWithScene(0);

    // create plane slice
    vtkNew<vtkPlaneSource> planeSource;
    planeSource->Update();
    this->SliceModelNode->SetPolyDataConnection(planeSource->GetOutputPort());
    this->SliceModelNode->SetDisableModifiedEvent(0);

    // create display node and set texture
    vtkDMMLSliceDisplayNode* sliceDisplayNode = vtkDMMLSliceDisplayNode::SafeDownCast(this->GetDMMLScene()->CreateNodeByClass("vtkDMMLSliceDisplayNode"));
    this->SliceModelDisplayNode = sliceDisplayNode;
    this->SliceModelDisplayNode->SetScene(this->GetDMMLScene());
    this->SliceModelDisplayNode->SetDisableModifiedEvent(1);

    //this->SliceModelDisplayNode->SetInputPolyData(this->SliceModelNode->GetOutputPolyData());
    this->SliceModelDisplayNode->SetVisibility(0);
    this->SliceModelDisplayNode->SetOpacity(1);
    this->SliceModelDisplayNode->SetColor(1,1,1);

    // Show intersecting slices in new slice views if this is currently enabled in the application.
    vtkDMMLApplicationLogic* appLogic = this->GetDMMLApplicationLogic();
    if (appLogic)
      {
      sliceDisplayNode->SetIntersectingSlicesVisibility(appLogic->GetIntersectingSlicesEnabled(vtkDMMLApplicationLogic::IntersectingSlicesVisibility));
      sliceDisplayNode->SetIntersectingSlicesInteractive(appLogic->GetIntersectingSlicesEnabled(vtkDMMLApplicationLogic::IntersectingSlicesInteractive));
      sliceDisplayNode->SetIntersectingSlicesTranslationEnabled(appLogic->GetIntersectingSlicesEnabled(vtkDMMLApplicationLogic::IntersectingSlicesTranslation));
      sliceDisplayNode->SetIntersectingSlicesRotationEnabled(appLogic->GetIntersectingSlicesEnabled(vtkDMMLApplicationLogic::IntersectingSlicesRotation));
      }

    std::string displayName = "Slice Display";
    std::string modelNodeName = "Slice " + this->SLICE_MODEL_NODE_NAME_SUFFIX;
    std::string transformNodeName = "Slice Transform";
    if (this->SliceNode && this->SliceNode->GetLayoutName())
      {
      // Auto-set the colors based on the slice node
      this->SliceModelDisplayNode->SetColor(this->SliceNode->GetLayoutColor());
      displayName = this->SliceNode->GetLayoutName() + std::string(" Display");
      modelNodeName = this->SliceNode->GetLayoutName() + std::string(" ") + this->SLICE_MODEL_NODE_NAME_SUFFIX;
      transformNodeName = this->SliceNode->GetLayoutName() + std::string(" Transform");
      }
    this->SliceModelDisplayNode->SetAmbient(1);
    this->SliceModelDisplayNode->SetBackfaceCulling(0);
    this->SliceModelDisplayNode->SetDiffuse(0);
    this->SliceModelDisplayNode->SetTextureImageDataConnection(this->ExtractModelTexture->GetOutputPort());
    this->SliceModelDisplayNode->SetSaveWithScene(0);
    this->SliceModelDisplayNode->SetDisableModifiedEvent(0);
    // set an attribute to distinguish this from regular model display nodes
    this->SliceModelDisplayNode->SetAttribute("SliceLogic.IsSliceModelDisplayNode", "True");
    this->SliceModelDisplayNode->SetName(this->GetDMMLScene()->GenerateUniqueName(displayName).c_str());

    this->SliceModelNode->SetName(modelNodeName.c_str());

    // make the xy to RAS transform
    this->SliceModelTransformNode = vtkDMMLLinearTransformNode::New();
    this->SliceModelTransformNode->SetScene(this->GetDMMLScene());
    this->SliceModelTransformNode->SetDisableModifiedEvent(1);

    this->SliceModelTransformNode->SetHideFromEditors(1);
    this->SliceModelTransformNode->SetSelectable(0);
    this->SliceModelTransformNode->SetSaveWithScene(0);
    // set the transform for the slice model for use by an image actor in the viewer
    vtkNew<vtkMatrix4x4> identity;
    identity->Identity();
    this->SliceModelTransformNode->SetMatrixTransformToParent(identity.GetPointer());
    this->SliceModelTransformNode->SetName(this->GetDMMLScene()->GenerateUniqueName(transformNodeName).c_str());

    this->SliceModelTransformNode->SetDisableModifiedEvent(0);

    }

  if (this->SliceModelNode != nullptr && this->GetDMMLScene()->GetNodeByID( this->GetSliceModelNode()->GetID() ) == nullptr )
    {
    this->AddingSliceModelNodes = true;
    this->GetDMMLScene()->AddNode(this->SliceModelDisplayNode);
    this->GetDMMLScene()->AddNode(this->SliceModelTransformNode);
    this->SliceModelNode->SetAndObserveDisplayNodeID(this->SliceModelDisplayNode->GetID());
    this->GetDMMLScene()->AddNode(this->SliceModelNode);
    this->AddingSliceModelNodes = false;
    this->SliceModelDisplayNode->SetTextureImageDataConnection(this->ExtractModelTexture->GetOutputPort());
    this->SliceModelNode->SetAndObserveTransformNodeID(this->SliceModelTransformNode->GetID());
    }

  // update the description to refer back to the slice and composite nodes
  // TODO: this doesn't need to be done unless the ID change, but it needs
  // to happen after they have been set, so do it every event for now
  if ( this->SliceModelNode != nullptr )
    {
    char description[256];
    std::stringstream ssD;
    if (this->SliceNode && this->SliceNode->GetID() )
      {
      ssD << " SliceID " << this->SliceNode->GetID();
      }
    if (this->SliceCompositeNode && this->SliceCompositeNode->GetID() )
      {
      ssD << " CompositeID " << this->SliceCompositeNode->GetID();
      }

    ssD.getline(description,256);
    this->SliceModelNode->SetDescription(description);
    }
}

//----------------------------------------------------------------------------
vtkDMMLVolumeNode *vtkDMMLSliceLogic::GetLayerVolumeNode(int layer)
{
  if (!this->SliceNode || !this->SliceCompositeNode)
    {
    return (nullptr);
    }

  const char *id = nullptr;
  switch (layer)
    {
    case LayerBackground:
      {
      id = this->SliceCompositeNode->GetBackgroundVolumeID();
      break;
      }
    case LayerForeground:
      {
      id = this->SliceCompositeNode->GetForegroundVolumeID();
      break;
      }
    case LayerLabel:
      {
      id = this->SliceCompositeNode->GetLabelVolumeID();
      break;
      }
    }
  vtkDMMLScene* scene = this->GetDMMLScene();
  return scene ? vtkDMMLVolumeNode::SafeDownCast(
    scene->GetNodeByID( id )) : nullptr;
}

//----------------------------------------------------------------------------
// Get the size of the volume, transformed to RAS space
void vtkDMMLSliceLogic::GetVolumeRASBox(vtkDMMLVolumeNode *volumeNode, double rasDimensions[3], double rasCenter[3])
{
  rasCenter[0] = rasDimensions[0] = 0.0;
  rasCenter[1] = rasDimensions[1] = 0.0;
  rasCenter[2] = rasDimensions[2] = 0.0;


  vtkImageData *volumeImage;
  if ( !volumeNode || ! (volumeImage = volumeNode->GetImageData()) )
    {
    return;
    }

  double bounds[6];
  volumeNode->GetRASBounds(bounds);

  for (int i=0; i<3; i++)
    {
    rasDimensions[i] = bounds[2*i+1] - bounds[2*i];
    rasCenter[i] = 0.5*(bounds[2*i+1] + bounds[2*i]);
  }
}

//----------------------------------------------------------------------------
// Get the size of the volume, transformed to RAS space
void vtkDMMLSliceLogic::GetVolumeSliceDimensions(vtkDMMLVolumeNode *volumeNode, double sliceDimensions[3], double sliceCenter[3])
{
  sliceCenter[0] = sliceDimensions[0] = 0.0;
  sliceCenter[1] = sliceDimensions[1] = 0.0;
  sliceCenter[2] = sliceDimensions[2] = 0.0;

  double sliceBounds[6];

  this->GetVolumeSliceBounds(volumeNode, sliceBounds);

  for (int i=0; i<3; i++)
    {
    sliceDimensions[i] = sliceBounds[2*i+1] - sliceBounds[2*i];
    sliceCenter[i] = 0.5*(sliceBounds[2*i+1] + sliceBounds[2*i]);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::GetVolumeSliceBounds(vtkDMMLVolumeNode *volumeNode,
  double sliceBounds[6], bool useVoxelCenter/*=false*/)
{
  if (this->SliceNode == nullptr || volumeNode == nullptr)
    {
    sliceBounds[0] = sliceBounds[1] = 0.0;
    sliceBounds[2] = sliceBounds[3] = 0.0;
    sliceBounds[4] = sliceBounds[5] = 0.0;
    return;
    }
  //
  // figure out how big that volume is on this particular slice plane
  //
  vtkNew<vtkMatrix4x4> rasToSlice;
  rasToSlice->DeepCopy(this->SliceNode->GetSliceToRAS());
  rasToSlice->SetElement(0, 3, 0.0);
  rasToSlice->SetElement(1, 3, 0.0);
  rasToSlice->SetElement(2, 3, 0.0);
  rasToSlice->Invert();

  volumeNode->GetSliceBounds(sliceBounds, rasToSlice.GetPointer(), useVoxelCenter);
}

//----------------------------------------------------------------------------
// Get the spacing of the volume, transformed to slice space
double *vtkDMMLSliceLogic::GetVolumeSliceSpacing(vtkDMMLVolumeNode *volumeNode)
{
  if ( !volumeNode )
    {
    return (this->SliceSpacing);
    }

  if (!this->SliceNode)
    {
    return (this->SliceSpacing);
    }

  if (this->SliceNode->GetSliceSpacingMode() == vtkDMMLSliceNode::PrescribedSliceSpacingMode)
    {
    // jvm - should we cache the PrescribedSliceSpacing in SliceSpacing?
    double *pspacing = this->SliceNode->GetPrescribedSliceSpacing();
    this->SliceSpacing[0] = pspacing[0];
    this->SliceSpacing[1] = pspacing[1];
    this->SliceSpacing[2] = pspacing[2];
    return (pspacing);
    }

  // Compute slice spacing from the volume axis closest matching the slice axis, projected to the slice axis.

  vtkNew<vtkMatrix4x4> ijkToWorld;
  volumeNode->GetIJKToRASMatrix(ijkToWorld);

  // Apply transform to the volume axes, if the volume is transformed with a linear transform
  vtkDMMLTransformNode *transformNode = volumeNode->GetParentTransformNode();
  if ( transformNode != nullptr &&  transformNode->IsTransformToWorldLinear() )
    {
    vtkNew<vtkMatrix4x4> volumeRASToWorld;
    transformNode->GetMatrixTransformToWorld(volumeRASToWorld);
    //rasToRAS->Invert();
    vtkMatrix4x4::Multiply4x4(volumeRASToWorld, ijkToWorld, ijkToWorld);
    }

  vtkNew<vtkMatrix4x4> worldToIJK;
  vtkMatrix4x4::Invert(ijkToWorld, worldToIJK);
  vtkNew<vtkMatrix4x4> sliceToIJK;
  vtkMatrix4x4::Multiply4x4(worldToIJK, this->SliceNode->GetSliceToRAS(), sliceToIJK);
  vtkNew<vtkMatrix4x4> ijkToSlice;
  vtkMatrix4x4::Invert(sliceToIJK, ijkToSlice);

  // Find the volume IJK axis that has the most similar direction to the slice axis.
  // Use the spacing component of this volume IJK axis parallel to the slice axis.
  double scale[3]; // unused
  vtkAddonMathUtilities::NormalizeOrientationMatrixColumns(sliceToIJK, scale);
  // after normalization, sliceToIJK only contains slice axis directions
  for (int sliceAxisIndex = 0; sliceAxisIndex < 3; sliceAxisIndex++)
    {
    // Slice axis direction in IJK coordinate system
    double sliceAxisDirection_I = fabs(sliceToIJK->GetElement(0, sliceAxisIndex));
    double sliceAxisDirection_J = fabs(sliceToIJK->GetElement(1, sliceAxisIndex));
    double sliceAxisDirection_K = fabs(sliceToIJK->GetElement(2, sliceAxisIndex));
    if (sliceAxisDirection_I > sliceAxisDirection_J)
      {
      if (sliceAxisDirection_I > sliceAxisDirection_K)
        {
        // this sliceAxis direction is closest volume I axis direction
        this->SliceSpacing[sliceAxisIndex] = fabs(ijkToSlice->GetElement(sliceAxisIndex, 0 /*I*/));
        }
      else
        {
        // this sliceAxis direction is closest volume K axis direction
        this->SliceSpacing[sliceAxisIndex] = fabs(ijkToSlice->GetElement(sliceAxisIndex, 2 /*K*/));
        }
      }
    else
      {
      if (sliceAxisDirection_J > sliceAxisDirection_K)
        {
        // this sliceAxis direction is closest volume J axis direction
        this->SliceSpacing[sliceAxisIndex] = fabs(ijkToSlice->GetElement(sliceAxisIndex, 1 /*J*/));
        }
      else
        {
        // this sliceAxis direction is closest volume K axis direction
        this->SliceSpacing[sliceAxisIndex] = fabs(ijkToSlice->GetElement(sliceAxisIndex, 2 /*K*/));
        }
      }
    }

  return this->SliceSpacing;
}

//----------------------------------------------------------------------------
// adjust the node's field of view to match the extent of current volume
void vtkDMMLSliceLogic::FitSliceToVolume(vtkDMMLVolumeNode *volumeNode, int width, int height)
{
  vtkImageData *volumeImage;
  if ( !volumeNode || ! (volumeImage = volumeNode->GetImageData()) )
    {
    return;
    }

  if (!this->SliceNode)
    {
    return;
    }

  double rasDimensions[3], rasCenter[3];
  this->GetVolumeRASBox (volumeNode, rasDimensions, rasCenter);
  double sliceDimensions[3], sliceCenter[3];
  this->GetVolumeSliceDimensions (volumeNode, sliceDimensions, sliceCenter);

  double fitX, fitY, fitZ, displayX, displayY;
  displayX = fitX = fabs(sliceDimensions[0]);
  displayY = fitY = fabs(sliceDimensions[1]);
  fitZ = this->GetVolumeSliceSpacing(volumeNode)[2] * this->SliceNode->GetDimensions()[2];


  // fit fov to min dimension of window
  double pixelSize;
  if ( height > width )
    {
    pixelSize = fitX / (1.0 * width);
    fitY = pixelSize * height;
    }
  else
    {
    pixelSize = fitY / (1.0 * height);
    fitX = pixelSize * width;
    }

  // if volume is still too big, shrink some more
  if ( displayX > fitX )
    {
    fitY = fitY / ( fitX / (displayX * 1.0) );
    fitX = displayX;
    }
  if ( displayY > fitY )
    {
    fitX = fitX / ( fitY / (displayY * 1.0) );
    fitY = displayY;
    }

  this->SliceNode->SetFieldOfView(fitX, fitY, fitZ);

  //
  // set the origin to be the center of the volume in RAS
  //
  vtkNew<vtkMatrix4x4> sliceToRAS;
  sliceToRAS->DeepCopy(this->SliceNode->GetSliceToRAS());
  sliceToRAS->SetElement(0, 3, rasCenter[0]);
  sliceToRAS->SetElement(1, 3, rasCenter[1]);
  sliceToRAS->SetElement(2, 3, rasCenter[2]);
  this->SliceNode->GetSliceToRAS()->DeepCopy(sliceToRAS.GetPointer());
  this->SliceNode->SetSliceOrigin(0,0,0);
  //sliceNode->SetSliceOffset(offset);

  //TODO Fit UVW space
  this->SnapSliceOffsetToIJK();
  this->SliceNode->UpdateMatrices( );
}

//----------------------------------------------------------------------------
// Get the size of the volume, transformed to RAS space
void vtkDMMLSliceLogic::GetBackgroundRASBox(double rasDimensions[3], double rasCenter[3])
{
  vtkDMMLVolumeNode *backgroundNode = nullptr;
  backgroundNode = this->GetLayerVolumeNode (0);
  this->GetVolumeRASBox( backgroundNode, rasDimensions, rasCenter );
}

//----------------------------------------------------------------------------
// Get the size of the volume, transformed to RAS space
void vtkDMMLSliceLogic::GetBackgroundSliceDimensions(double sliceDimensions[3], double sliceCenter[3])
{
  vtkDMMLVolumeNode *backgroundNode = nullptr;
  backgroundNode = this->GetLayerVolumeNode (0);
  this->GetVolumeSliceDimensions( backgroundNode, sliceDimensions, sliceCenter );
}

//----------------------------------------------------------------------------
// Get the spacing of the volume, transformed to slice space
double *vtkDMMLSliceLogic::GetBackgroundSliceSpacing()
{
  vtkDMMLVolumeNode *backgroundNode = nullptr;
  backgroundNode = this->GetLayerVolumeNode (0);
  return (this->GetVolumeSliceSpacing( backgroundNode ));
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::GetBackgroundSliceBounds(double sliceBounds[6])
{
  vtkDMMLVolumeNode *backgroundNode = nullptr;
  backgroundNode = this->GetLayerVolumeNode (0);
  this->GetVolumeSliceBounds(backgroundNode, sliceBounds);
}

//----------------------------------------------------------------------------
// adjust the node's field of view to match the extent of current background volume
void vtkDMMLSliceLogic::FitSliceToBackground(int width, int height)
{
  vtkDMMLVolumeNode *backgroundNode = nullptr;
  backgroundNode = this->GetLayerVolumeNode (0);
  this->FitSliceToVolume( backgroundNode, width, height );
}

//----------------------------------------------------------------------------
// adjust the node's field of view to match the extent of all volume layers
void vtkDMMLSliceLogic::FitSliceToAll(int width, int height)
{
  // Use SliceNode dimensions if width and height parameters are omitted
  if (width < 0 || height < 0)
    {
    int* dimensions = this->SliceNode->GetDimensions();
    width = dimensions ? dimensions[0] : -1;
    height = dimensions ? dimensions[1] : -1;
    }

  if (width < 0 || height < 0)
    {
    vtkErrorMacro(<< __FUNCTION__ << "- Invalid size:" << width
                  << "x" << height);
    return;
    }

  vtkDMMLVolumeNode *volumeNode;
  for ( int layer=0; layer < 3; layer++ )
    {
    volumeNode = this->GetLayerVolumeNode (layer);
    if (volumeNode)
      {
      this->FitSliceToVolume( volumeNode, width, height );
      return;
      }
    }
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::FitFOVToBackground(double fov)
{
  // get backgroundNode  and imagedata
  vtkDMMLScalarVolumeNode* backgroundNode =
    vtkDMMLScalarVolumeNode::SafeDownCast(
      this->GetDMMLScene()->GetNodeByID(
        this->SliceCompositeNode->GetBackgroundVolumeID() ));
  vtkImageData *backgroundImage =
    backgroundNode ? backgroundNode->GetImageData() : nullptr;
  if (!backgroundImage)
    {
    return;
    }
  // get viewer's width and height. we may be using a LightBox
  // display, so base width and height on renderer0 in the SliceViewer.
  int width = this->SliceNode->GetDimensions()[0];
  int height = this->SliceNode->GetDimensions()[1];

  int dimensions[3];
  double rasDimensions[4];
  double doubleDimensions[4];
  vtkNew<vtkMatrix4x4> ijkToRAS;

  // what are the actual dimensions of the imagedata?
  backgroundImage->GetDimensions(dimensions);
  doubleDimensions[0] = static_cast<double>(dimensions[0]);
  doubleDimensions[1] = static_cast<double>(dimensions[1]);
  doubleDimensions[2] = static_cast<double>(dimensions[2]);
  doubleDimensions[3] = 0.0;
  backgroundNode->GetIJKToRASMatrix(ijkToRAS.GetPointer());
  ijkToRAS->MultiplyPoint(doubleDimensions, rasDimensions);

  // and what are their slice dimensions?
  vtkNew<vtkMatrix4x4> rasToSlice;
  double sliceDimensions[4];
  rasToSlice->DeepCopy(this->SliceNode->GetSliceToRAS());
  rasToSlice->SetElement(0, 3, 0.0);
  rasToSlice->SetElement(1, 3, 0.0);
  rasToSlice->SetElement(2, 3, 0.0);
  rasToSlice->Invert();
  rasToSlice->MultiplyPoint(rasDimensions, sliceDimensions);

  double fovh, fovv;
  // which is bigger, slice viewer width or height?
  // assign user-specified fov to smaller slice window
  // dimension
  if ( width < height )
    {
    fovh = fov;
    fovv = fov * height/width;
    }
  else
    {
    fovv = fov;
    fovh = fov * width/height;
    }
  // we want to compute the slice dimensions of the
  // user-specified fov (note that the slice node's z field of
  // view is NOT changed)
  this->SliceNode->SetFieldOfView(fovh, fovv, this->SliceNode->GetFieldOfView()[2]);

  vtkNew<vtkMatrix4x4> sliceToRAS;
  sliceToRAS->DeepCopy(this->SliceNode->GetSliceToRAS());
  this->SliceNode->GetSliceToRAS()->DeepCopy(sliceToRAS.GetPointer());
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::ResizeSliceNode(double newWidth, double newHeight)
{
  if (!this->SliceNode)
    {
    return;
    }

  // New size must be the active slice vtkRenderer size. It's the same than the window
  // if the layout is 1x1.
  newWidth /= this->SliceNode->GetLayoutGridColumns();
  newHeight /= this->SliceNode->GetLayoutGridRows();

  // The following was previously in SliceSWidget.tcl
  double sliceStep = this->SliceSpacing[2];
  int oldDimensions[3];
  this->SliceNode->GetDimensions(oldDimensions);
  double oldFOV[3];
  this->SliceNode->GetFieldOfView(oldFOV);

  double scalingX = (newWidth != 0 && oldDimensions[0] != 0 ? newWidth / oldDimensions[0] : 1.);
  double scalingY = (newHeight != 0 && oldDimensions[1] != 0 ? newHeight / oldDimensions[1] : 1.);

  double magnitudeX = (scalingX >= 1. ? scalingX : 1. / scalingX);
  double magnitudeY = (scalingY >= 1. ? scalingY : 1. / scalingY);

  double newFOV[3];
  if (magnitudeX < magnitudeY)
    {
    newFOV[0] = oldFOV[0];
    newFOV[1] = oldFOV[1] * scalingY / scalingX;
    }
  else
    {
    newFOV[0] = oldFOV[0] * scalingX / scalingY;
    newFOV[1] = oldFOV[1];
    }
  newFOV[2] = sliceStep * oldDimensions[2];
  double windowAspect = (newWidth != 0. ? newHeight / newWidth : 1.);
  double planeAspect = (newFOV[0] != 0. ? newFOV[1] / newFOV[0] : 1.);
  if (windowAspect != planeAspect)
    {
    newFOV[0] = (windowAspect != 0. ? newFOV[1] / windowAspect : newFOV[0]);
    }
  int disabled = this->SliceNode->StartModify();
  this->SliceNode->SetDimensions(newWidth, newHeight, oldDimensions[2]);
  this->SliceNode->SetFieldOfView(newFOV[0], newFOV[1], newFOV[2]);
  this->SliceNode->EndModify(disabled);
}

//----------------------------------------------------------------------------
double *vtkDMMLSliceLogic::GetLowestVolumeSliceSpacing()
{
  // TBD: Doesn't return the lowest slice spacing, just the first valid spacing
  vtkDMMLVolumeNode *volumeNode;
  for ( int layer=0; layer < 3; layer++ )
    {
    volumeNode = this->GetLayerVolumeNode (layer);
    if (volumeNode)
      {
      return this->GetVolumeSliceSpacing( volumeNode );
      }
    }
  return (this->SliceSpacing);
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::GetLowestVolumeSliceBounds(double sliceBounds[6], bool useVoxelCenter/*=false*/)
{
  vtkDMMLVolumeNode *volumeNode;
  for ( int layer=0; layer < 3; layer++ )
    {
    volumeNode = this->GetLayerVolumeNode (layer);
    if (volumeNode)
      {
      return this->GetVolumeSliceBounds(volumeNode, sliceBounds, useVoxelCenter);
      }
    }
  // return the default values
  return this->GetVolumeSliceBounds(nullptr, sliceBounds, useVoxelCenter);
}

#define LARGE_BOUNDS_NUM 1.0e10
#define SMALL_BOUNDS_NUM -1.0e10
//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::GetSliceBounds(double sliceBounds[6])
{
  int i;
  for (i=0; i<3; i++)
    {
    sliceBounds[2*i]   = LARGE_BOUNDS_NUM;
    sliceBounds[2*i+1] = SMALL_BOUNDS_NUM;
    }

  vtkDMMLVolumeNode *volumeNode;
  for ( int layer=0; layer < 3; layer++ )
    {
    volumeNode = this->GetLayerVolumeNode (layer);
    if (volumeNode)
      {
      double bounds[6];
      this->GetVolumeSliceBounds( volumeNode, bounds );
      for (i=0; i<3; i++)
        {
        if (bounds[2*i] < sliceBounds[2*i])
          {
          sliceBounds[2*i] = bounds[2*i];
          }
        if (bounds[2*i+1] > sliceBounds[2*i+1])
          {
          sliceBounds[2*i+1] = bounds[2*i+1];
          }
        }
      }
    }

  // default
  for (i=0; i<3; i++)
    {
    if (sliceBounds[2*i] == LARGE_BOUNDS_NUM)
      {
      sliceBounds[2*i] = -100;
      }
    if (sliceBounds[2*i+1] == SMALL_BOUNDS_NUM)
      {
      sliceBounds[2*i+1] = 100;
      }
    }

}

//----------------------------------------------------------------------------
// Get/Set the current distance from the origin to the slice plane
double vtkDMMLSliceLogic::GetSliceOffset()
{
  // this method has been moved to vtkDMMLSliceNode
  // the API stays for backwards compatibility

  if ( !this->SliceNode )
    {
    return 0.0;
    }

  return this->SliceNode->GetSliceOffset();

}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::SetSliceOffset(double offset)
{
  // this method has been moved to vtkDMMLSliceNode
  // the API stays for backwards compatibility
  if (!this->SliceNode)
    {
    return;
    }
  this->SliceNode->SetSliceOffset(offset);
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::StartSliceCompositeNodeInteraction(unsigned int parameters)
{
  if (!this->SliceCompositeNode)
    {
    return;
    }

  // Cache the flags on what parameters are going to be modified. Need
  // to this this outside the conditional on HotLinkedControl and LinkedControl
  this->SliceCompositeNode->SetInteractionFlags(parameters);

  // If we have hot linked controls, then we want to broadcast changes
  if (this->SliceCompositeNode->GetHotLinkedControl() && this->SliceCompositeNode->GetLinkedControl())
    {
    this->SliceCompositeNode->InteractingOn();
    }
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::EndSliceCompositeNodeInteraction()
{
  if (!this->SliceCompositeNode)
    {
    return;
    }
  // If we have linked controls, then we want to broadcast changes
  if (this->SliceCompositeNode->GetLinkedControl())
    {
    // Need to trigger a final message to broadcast to all the nodes
    // that are linked
    this->SliceCompositeNode->InteractingOn();
    this->SliceCompositeNode->Modified();
    this->SliceCompositeNode->InteractingOff();
    }

  this->SliceCompositeNode->SetInteractionFlags(0);
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::StartSliceNodeInteraction(unsigned int parameters)
{
  if (this->SliceNode == nullptr || this->SliceCompositeNode == nullptr)
    {
    return;
    }

  // Cache the flags on what parameters are going to be modified. Need
  // to this this outside the conditional on HotLinkedControl and LinkedControl
  this->SliceNode->SetInteractionFlags(parameters);

  // If we have hot linked controls, then we want to broadcast changes
  if ((this->SliceCompositeNode->GetHotLinkedControl() || parameters == vtkDMMLSliceNode::MultiplanarReformatFlag)
      && this->SliceCompositeNode->GetLinkedControl())
    {
    this->SliceNode->InteractingOn();
    }
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::SetSliceExtentsToSliceNode()
{
  if (this->SliceNode == nullptr)
    {
    return;
    }

  double sliceBounds[6];
  this->GetSliceBounds( sliceBounds );

  double extents[3];
  extents[0] = sliceBounds[1] - sliceBounds[0];
  extents[1] = sliceBounds[3] - sliceBounds[2];
  extents[2] = sliceBounds[5] - sliceBounds[4];

  if (this->SliceNode->GetSliceResolutionMode() == vtkDMMLSliceNode::SliceResolutionMatch2DView)
    {
    this->SliceNode->SetUVWExtentsAndDimensions(this->SliceNode->GetFieldOfView(),
                                                this->SliceNode->GetUVWDimensions());
    }
 else if (this->SliceNode->GetSliceResolutionMode() == vtkDMMLSliceNode::SliceResolutionMatchVolumes)
    {
    double *spacing = this->GetLowestVolumeSliceSpacing();
    double minSpacing = spacing[0];
    minSpacing = minSpacing < spacing[1] ? minSpacing:spacing[1];
    minSpacing = minSpacing < spacing[2] ? minSpacing:spacing[2];

    int sliceResolutionMax = 200;
    if (minSpacing > 0.0)
      {
      double maxExtent = extents[0];
      maxExtent = maxExtent > extents[1] ? maxExtent:extents[1];
      maxExtent = maxExtent > extents[2] ? maxExtent:extents[2];

      sliceResolutionMax = maxExtent/minSpacing;
      }
    int dimensions[]={sliceResolutionMax, sliceResolutionMax, 1};

    this->SliceNode->SetUVWExtentsAndDimensions(extents, dimensions);
    }
  else if (this->SliceNode->GetSliceResolutionMode() == vtkDMMLSliceNode::SliceFOVMatch2DViewSpacingMatchVolumes)
    {
    double *spacing = this->GetLowestVolumeSliceSpacing();
    double minSpacing = spacing[0];
    minSpacing = minSpacing < spacing[1] ? minSpacing:spacing[1];
    minSpacing = minSpacing < spacing[2] ? minSpacing:spacing[2];

    double fov[3];
    int dimensions[]={0,0,1};
    this->SliceNode->GetFieldOfView(fov);
    for (int i=0; i<2; i++)
      {
       dimensions[i] = ceil(fov[i]/minSpacing +0.5);
      }
    this->SliceNode->SetUVWExtentsAndDimensions(fov, dimensions);
    }
  else if (this->SliceNode->GetSliceResolutionMode() == vtkDMMLSliceNode::SliceFOVMatchVolumesSpacingMatch2DView)
    {
    // compute RAS spacing in 2D view
    vtkMatrix4x4 *xyToRAS = this->SliceNode->GetXYToRAS();
    int  dims[3];

    //
    double inPt[4]={0,0,0,1};
    double outPt0[4];
    double outPt1[4];
    double outPt2[4];

    // set the z position to be the active slice (from the lightbox)
    inPt[2] = this->SliceNode->GetActiveSlice();

    // transform XYZ = (0,0,0)
    xyToRAS->MultiplyPoint(inPt, outPt0);

    // transform XYZ = (1,0,0)
    inPt[0] = 1;
    xyToRAS->MultiplyPoint(inPt, outPt1);

    // transform XYZ = (0,1,0)
    inPt[0] = 0;
    inPt[1] = 1;
    xyToRAS->MultiplyPoint(inPt, outPt2);

    double xSpacing = sqrt(vtkMath::Distance2BetweenPoints(outPt0, outPt1));
    double ySpacing = sqrt(vtkMath::Distance2BetweenPoints(outPt0, outPt2));

    dims[0] = extents[0]/xSpacing+1;
    dims[1] = extents[2]/ySpacing+1;
    dims[2] = 1;

    this->SliceNode->SetUVWExtentsAndDimensions(extents, dims);
    }

}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::EndSliceNodeInteraction()
{
  if (this->SliceNode == nullptr || this->SliceCompositeNode == nullptr)
    {
    return;
    }

  // If we have linked controls, then we want to broadcast changes
  if (this->SliceCompositeNode->GetLinkedControl())
    {
    // Need to trigger a final message to broadcast to all the nodes
    // that are linked
    this->SliceNode->InteractingOn();
    this->SliceNode->Modified();
    this->SliceNode->InteractingOff();
    }

  this->SliceNode->SetInteractionFlags(0);
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::StartSliceOffsetInteraction()
{
  // This method is here in case we want to do something specific when
  // we start SliceOffset interactions

  this->StartSliceNodeInteraction(vtkDMMLSliceNode::SliceToRASFlag);
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::EndSliceOffsetInteraction()
{
  // This method is here in case we want to do something specific when
  // we complete SliceOffset interactions

  this->EndSliceNodeInteraction();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::SnapSliceOffsetToIJK()
{
  double offset, *spacing, bounds[6];
  double oldOffset = this->GetSliceOffset();
  spacing = this->GetLowestVolumeSliceSpacing();
  this->GetLowestVolumeSliceBounds( bounds );

  // number of slices along the offset dimension (depends on ijkToRAS and Transforms)
  // - find the slice index corresponding to the current slice offset
  // - move the offset to the middle of that slice
  // - note that bounds[4] 'furthest' edge of the volume from the point of view of this slice
  // - note also that spacing[2] may correspond to i, j, or k depending on ijkToRAS and sliceToRAS
  double slice = (oldOffset - bounds[4]) / spacing[2];
  int intSlice = static_cast<int> (slice);
  offset = (intSlice + 0.5) * spacing[2] + bounds[4];
  this->SetSliceOffset( offset );
}


//----------------------------------------------------------------------------
std::vector< vtkDMMLDisplayNode*> vtkDMMLSliceLogic::GetPolyDataDisplayNodes()
{
  std::vector< vtkDMMLDisplayNode*> nodes;
  std::vector<vtkDMMLSliceLayerLogic *> layerLogics;
  layerLogics.push_back(this->GetBackgroundLayer());
  layerLogics.push_back(this->GetForegroundLayer());
  for (unsigned int i=0; i<layerLogics.size(); i++)
    {
    vtkDMMLSliceLayerLogic *layerLogic = layerLogics[i];
    if (layerLogic && layerLogic->GetVolumeNode())
      {
      vtkDMMLVolumeNode *volumeNode = vtkDMMLVolumeNode::SafeDownCast (layerLogic->GetVolumeNode());
      vtkDMMLGlyphableVolumeDisplayNode *displayNode = vtkDMMLGlyphableVolumeDisplayNode::SafeDownCast( layerLogic->GetVolumeNode()->GetDisplayNode() );
      if (displayNode)
        {
        std::vector< vtkDMMLGlyphableVolumeSliceDisplayNode*> dnodes  = displayNode->GetSliceGlyphDisplayNodes(volumeNode);
        for (unsigned int n=0; n<dnodes.size(); n++)
          {
          vtkDMMLGlyphableVolumeSliceDisplayNode* dnode = dnodes[n];
          if (layerLogic->GetSliceNode()
            && layerLogic->GetSliceNode()->GetLayoutName()
            && !strcmp(layerLogic->GetSliceNode()->GetLayoutName(), dnode->GetName()) )
            {
            nodes.push_back(dnode);
            }
          }
        }//  if (volumeNode)
      }// if (layerLogic && layerLogic->GetVolumeNode())
    }
  return nodes;
}

//----------------------------------------------------------------------------
int vtkDMMLSliceLogic::GetSliceIndexFromOffset(double sliceOffset, vtkDMMLVolumeNode *volumeNode)
{
  if ( !volumeNode )
    {
    return SLICE_INDEX_NO_VOLUME;
    }
  vtkImageData *volumeImage=nullptr;
  if ( !(volumeImage = volumeNode->GetImageData()) )
    {
    return SLICE_INDEX_NO_VOLUME;
    }
  if (!this->SliceNode)
    {
    return SLICE_INDEX_NO_VOLUME;
    }

  vtkNew<vtkMatrix4x4> ijkToRAS;
  volumeNode->GetIJKToRASMatrix (ijkToRAS.GetPointer());
  vtkDMMLTransformNode *transformNode = volumeNode->GetParentTransformNode();
  if ( transformNode )
    {
    vtkNew<vtkMatrix4x4> rasToRAS;
    transformNode->GetMatrixTransformToWorld(rasToRAS.GetPointer());
    vtkMatrix4x4::Multiply4x4 (rasToRAS.GetPointer(), ijkToRAS.GetPointer(), ijkToRAS.GetPointer());
    }

  // Get the slice normal in RAS

  vtkNew<vtkMatrix4x4> rasToSlice;
  rasToSlice->DeepCopy(this->SliceNode->GetSliceToRAS());
  rasToSlice->Invert();

  double sliceNormal_IJK[4]={0,0,1,0};  // slice normal vector in IJK coordinate system
  double sliceNormal_RAS[4]={0,0,0,0};  // slice normal vector in RAS coordinate system
  this->SliceNode->GetSliceToRAS()->MultiplyPoint(sliceNormal_IJK, sliceNormal_RAS);

  // Find an axis normal that has the same orientation as the slice normal
  double axisDirection_RAS[3]={0,0,0};
  int axisIndex=0;
  double volumeSpacing=1.0; // spacing along axisIndex
  for (axisIndex=0; axisIndex<3; axisIndex++)
    {
    axisDirection_RAS[0]=ijkToRAS->GetElement(0,axisIndex);
    axisDirection_RAS[1]=ijkToRAS->GetElement(1,axisIndex);
    axisDirection_RAS[2]=ijkToRAS->GetElement(2,axisIndex);
    volumeSpacing=vtkMath::Norm(axisDirection_RAS); // spacing along axisIndex
    vtkMath::Normalize(sliceNormal_RAS);
    vtkMath::Normalize(axisDirection_RAS);
    double dotProd=vtkMath::Dot(sliceNormal_RAS, axisDirection_RAS);
    // Due to numerical inaccuracies the dot product of two normalized vectors
    // can be slightly bigger than 1 (and acos cannot be computed) - fix that.
    if (dotProd>1.0)
      {
      dotProd=1.0;
      }
    else if (dotProd<-1.0)
      {
      dotProd=-1.0;
      }
    double axisMisalignmentDegrees=acos(dotProd)*180.0/vtkMath::Pi();
    if (fabs(axisMisalignmentDegrees)<0.1)
      {
      // found an axis that is aligned to the slice normal
      break;
      }
    if (fabs(axisMisalignmentDegrees-180)<0.1 || fabs(axisMisalignmentDegrees+180)<0.1)
      {
      // found an axis that is aligned to the slice normal, just points to the opposite direction
      volumeSpacing*=-1.0;
      break;
      }
    }

  if (axisIndex>=3)
    {
    // no aligned axis is found
    return SLICE_INDEX_ROTATED;
    }

  // Determine slice index
  double originPos_RAS[4]={
    ijkToRAS->GetElement( 0, 3 ),
    ijkToRAS->GetElement( 1, 3 ),
    ijkToRAS->GetElement( 2, 3 ),
    0};
  double originPos_Slice[4]={0,0,0,0};
  rasToSlice->MultiplyPoint(originPos_RAS, originPos_Slice);
  double volumeOriginOffset=originPos_Slice[2];
  double sliceShift=sliceOffset-volumeOriginOffset;
  double normalizedSliceShift=sliceShift/volumeSpacing;
  int sliceIndex=vtkMath::Round(normalizedSliceShift)+1; // +0.5 because the slice plane is displayed in the center of the slice

  // Check if slice index is within the volume
  int sliceCount=volumeImage->GetDimensions()[axisIndex];
  if (sliceIndex<1 || sliceIndex>sliceCount)
    {
    sliceIndex=SLICE_INDEX_OUT_OF_VOLUME;
    }

  return sliceIndex;
}

//----------------------------------------------------------------------------
// sliceIndex: DICOM slice index, 1-based
int vtkDMMLSliceLogic::GetSliceIndexFromOffset(double sliceOffset)
{
  vtkDMMLVolumeNode *volumeNode;
  for (int layer=0; layer < 3; layer++ )
    {
    volumeNode = this->GetLayerVolumeNode (layer);
    if (volumeNode)
      {
      int sliceIndex=this->GetSliceIndexFromOffset( sliceOffset, volumeNode );
      // return the result for the first available layer
      return sliceIndex;
      }
    }
  // slice is not aligned to any of the layers or out of the volume
  return SLICE_INDEX_NO_VOLUME;
}

//----------------------------------------------------------------------------
vtkDMMLSliceCompositeNode* vtkDMMLSliceLogic
::GetSliceCompositeNode(vtkDMMLSliceNode* sliceNode)
{
  return sliceNode ? vtkDMMLSliceLogic::GetSliceCompositeNode(
    sliceNode->GetScene(), sliceNode->GetLayoutName()) : nullptr;
}

//----------------------------------------------------------------------------
vtkDMMLSliceCompositeNode* vtkDMMLSliceLogic
::GetSliceCompositeNode(vtkDMMLScene* scene, const char* layoutName)
{
  if (!scene || !layoutName)
    {
    return nullptr;
    }
  vtkDMMLNode* node;
  vtkCollectionSimpleIterator it;
  for (scene->GetNodes()->InitTraversal(it);
       (node = (vtkDMMLNode*)scene->GetNodes()->GetNextItemAsObject(it)) ;)
    {
    vtkDMMLSliceCompositeNode* sliceCompositeNode =
      vtkDMMLSliceCompositeNode::SafeDownCast(node);
    if (sliceCompositeNode &&
        sliceCompositeNode->GetLayoutName() &&
        !strcmp(sliceCompositeNode->GetLayoutName(), layoutName))
      {
      return sliceCompositeNode;
      }
    }
  return nullptr;
}

//----------------------------------------------------------------------------
vtkDMMLSliceNode* vtkDMMLSliceLogic
::GetSliceNode(vtkDMMLSliceCompositeNode* sliceCompositeNode)
{
  if (!sliceCompositeNode)
    {
    return nullptr;
    }
  return sliceCompositeNode ? vtkDMMLSliceLogic::GetSliceNode(
    sliceCompositeNode->GetScene(), sliceCompositeNode->GetLayoutName()) : nullptr;
}

//----------------------------------------------------------------------------
vtkDMMLSliceNode* vtkDMMLSliceLogic
::GetSliceNode(vtkDMMLScene* scene, const char* layoutName)
{
  if (!scene || !layoutName)
    {
    return nullptr;
    }
  vtkObject* itNode = nullptr;
  vtkCollectionSimpleIterator it;
  for (scene->GetNodes()->InitTraversal(it); (itNode = scene->GetNodes()->GetNextItemAsObject(it));)
    {
    vtkDMMLSliceNode* sliceNode = vtkDMMLSliceNode::SafeDownCast(itNode);
    if (!sliceNode)
      {
      continue;
      }
    if (sliceNode->GetLayoutName() &&
      !strcmp(sliceNode->GetLayoutName(), layoutName))
      {
      return sliceNode;
      }
    }
  return nullptr;
}

//----------------------------------------------------------------------------
bool vtkDMMLSliceLogic::IsSliceModelNode(vtkDMMLNode *dmmlNode)
{
  if (dmmlNode != nullptr &&
      dmmlNode->IsA("vtkDMMLModelNode") &&
      dmmlNode->GetName() != nullptr &&
      strstr(dmmlNode->GetName(), vtkDMMLSliceLogic::SLICE_MODEL_NODE_NAME_SUFFIX.c_str()) != nullptr)
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
bool vtkDMMLSliceLogic::IsSliceModelDisplayNode(vtkDMMLDisplayNode *dmmlDisplayNode)
{
  if (vtkDMMLSliceDisplayNode::SafeDownCast(dmmlDisplayNode))
    {
    return true;
    }
  if (dmmlDisplayNode != nullptr &&
      dmmlDisplayNode->IsA("vtkDMMLModelDisplayNode"))
    {
    const char *attrib = dmmlDisplayNode->GetAttribute("SliceLogic.IsSliceModelDisplayNode");
    // allow the attribute to be set to anything but 0
    if (attrib != nullptr &&
        strcmp(attrib, "0") != 0)
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
vtkImageBlend* vtkDMMLSliceLogic::GetBlend()
{
  return this->Pipeline->Blend.GetPointer();
}

//----------------------------------------------------------------------------
vtkImageBlend* vtkDMMLSliceLogic::GetBlendUVW()
{
  return this->PipelineUVW->Blend.GetPointer();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLogic::RotateSliceToLowestVolumeAxes(bool forceSlicePlaneToSingleSlice/*=true*/)
{
  vtkDMMLVolumeNode* volumeNode;
  for (int layer = 0; layer < 3; layer++)
    {
    volumeNode = this->GetLayerVolumeNode(layer);
    if (volumeNode)
      {
      break;
      }
    }
  if (!volumeNode)
    {
    return;
    }
  vtkDMMLSliceNode* sliceNode = this->GetSliceNode();
  if (!sliceNode)
    {
    return;
    }
  sliceNode->RotateToVolumePlane(volumeNode, forceSlicePlaneToSingleSlice);
  this->SnapSliceOffsetToIJK();
}

//----------------------------------------------------------------------------
int vtkDMMLSliceLogic::GetEditableLayerAtWorldPosition(double worldPos[3],
  bool backgroundVolumeEditable/*=true*/, bool foregroundVolumeEditable/*=true*/)
{
  vtkDMMLSliceNode *sliceNode = this->GetSliceNode();
  if (!sliceNode)
    {
    return vtkDMMLSliceLogic::LayerNone;
    }
  vtkDMMLSliceCompositeNode *sliceCompositeNode = this->GetSliceCompositeNode();
  if (!sliceCompositeNode)
    {
    return vtkDMMLSliceLogic::LayerNone;
    }

  bool foregroundEditable = this->VolumeWindowLevelEditable(sliceCompositeNode->GetForegroundVolumeID())
    && foregroundVolumeEditable;
  bool backgroundEditable = this->VolumeWindowLevelEditable(sliceCompositeNode->GetBackgroundVolumeID())
    && backgroundVolumeEditable;

  if (!foregroundEditable && !backgroundEditable)
    {
    // window/level editing is disabled on both volumes
    return vtkDMMLSliceLogic::LayerNone;
    }
  // By default adjust background volume, if available
  bool adjustForeground = !backgroundEditable;

  // If both foreground and background volumes are visible then choose adjustment of
  // foreground volume, if foreground volume is visible in current mouse position
  if (foregroundEditable && backgroundEditable)
    {
    adjustForeground = (sliceCompositeNode->GetForegroundOpacity() >= 0.01)
      && this->IsEventInsideVolume(true, worldPos)   // inside background (used as mask for displaying foreground)
      && this->vtkDMMLSliceLogic::IsEventInsideVolume(false, worldPos); // inside foreground
    }

  return (adjustForeground ? vtkDMMLSliceLogic::LayerForeground : vtkDMMLSliceLogic::LayerBackground);
}

//----------------------------------------------------------------------------
bool vtkDMMLSliceLogic::VolumeWindowLevelEditable(const char* volumeNodeID)
{
  if (!volumeNodeID)
    {
    return false;
    }
  vtkDMMLScene *scene = this->GetDMMLScene();
  if (!scene)
    {
    return false;
    }
  vtkDMMLVolumeNode* volumeNode =
    vtkDMMLVolumeNode::SafeDownCast(scene->GetNodeByID(volumeNodeID));
  if (volumeNode == nullptr)
    {
    return false;
    }
  vtkDMMLScalarVolumeDisplayNode* scalarVolumeDisplayNode =
    vtkDMMLScalarVolumeDisplayNode::SafeDownCast(volumeNode->GetVolumeDisplayNode());
  if (!scalarVolumeDisplayNode)
    {
    return false;
    }
  return !scalarVolumeDisplayNode->GetWindowLevelLocked();
}

//----------------------------------------------------------------------------
bool vtkDMMLSliceLogic::IsEventInsideVolume(bool background, double worldPos[3])
{
  vtkDMMLSliceNode *sliceNode = this->GetSliceNode();
  if (!sliceNode)
    {
    return false;
    }
  vtkDMMLSliceLayerLogic* layerLogic = background ?
    this->GetBackgroundLayer() : this->GetForegroundLayer();
  if (!layerLogic)
    {
    return false;
    }
  vtkDMMLVolumeNode* volumeNode = layerLogic->GetVolumeNode();
  if (!volumeNode || !volumeNode->GetImageData())
    {
    return false;
    }

  vtkNew<vtkGeneralTransform> inputVolumeIJKToWorldTransform;
  inputVolumeIJKToWorldTransform->PostMultiply();

  vtkNew<vtkMatrix4x4> inputVolumeIJK2RASMatrix;
  volumeNode->GetIJKToRASMatrix(inputVolumeIJK2RASMatrix);
  inputVolumeIJKToWorldTransform->Concatenate(inputVolumeIJK2RASMatrix);

  vtkNew<vtkGeneralTransform> inputVolumeRASToWorld;
  vtkDMMLTransformNode::GetTransformBetweenNodes(volumeNode->GetParentTransformNode(), nullptr, inputVolumeRASToWorld);
  inputVolumeIJKToWorldTransform->Concatenate(inputVolumeRASToWorld);

  double ijkPos[3] = { 0.0, 0.0, 0.0 };
  inputVolumeIJKToWorldTransform->GetInverse()->TransformPoint(worldPos, ijkPos);

  int volumeExtent[6] = { 0 };
  volumeNode->GetImageData()->GetExtent(volumeExtent);
  for (int i = 0; i < 3; i++)
    {
    if (ijkPos[i]<volumeExtent[i * 2] || ijkPos[i]>volumeExtent[i * 2 + 1])
      {
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
vtkDMMLSliceDisplayNode* vtkDMMLSliceLogic::GetSliceDisplayNode()
{
  return vtkDMMLSliceDisplayNode::SafeDownCast(this->GetSliceModelDisplayNode());
}
