/*=========================================================================

  Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/

// DMMLDisplayableManager includes
#include "vtkDMMLModelDisplayableManager.h"
#include "vtkDMMLThreeDViewInteractorStyle.h"
#include "vtkDMMLApplicationLogic.h"

// DMML/Cjyx includes
#include <vtkEventBroker.h>
#include <vtkDMMLClipModelsNode.h>
#include <vtkDMMLColorNode.h>
#include <vtkDMMLDisplayNode.h>
#include <vtkDMMLDisplayableNode.h>
#include <vtkDMMLFolderDisplayNode.h>
#include <vtkDMMLInteractionNode.h>
#include <vtkDMMLModelDisplayNode.h>
#include <vtkDMMLModelNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSelectionNode.h>
#include <vtkDMMLSliceLogic.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLSubjectHierarchyConstants.h>
#include <vtkDMMLSubjectHierarchyNode.h>
#include <vtkDMMLTransformNode.h>
#include <vtkDMMLViewNode.h>
#include <vtkDMMLVolumeNode.h>

// VTK includes
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkAssignAttribute.h>
#include <vtkCallbackCommand.h>
#include <vtkCellArray.h>
#include <vtkClipDataSet.h>
#include <vtkClipPolyData.h>
#include <vtkColorTransferFunction.h>
#include <vtkDataSetAttributes.h>
#include <vtkDataSetMapper.h>
#include <vtkExtractGeometry.h>
#include <vtkExtractPolyDataGeometry.h>
#include <vtkGeneralTransform.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkImageMapper3D.h>
#include <vtkImplicitBoolean.h>
#include <vtkLookupTable.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp3DCollection.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkTexture.h>
#include <vtkTransformFilter.h>
#include <vtkVersion.h>
#include <vtkWeakPointer.h>
// For picking
#include <vtkCellPicker.h>
#include <vtkPointPicker.h>
#include <vtkPropPicker.h>
#include <vtkRendererCollection.h>
#include <vtkWorldPointPicker.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkDMMLModelDisplayableManager );

//---------------------------------------------------------------------------
class vtkDMMLModelDisplayableManager::vtkInternal
{
public:
  vtkInternal(vtkDMMLModelDisplayableManager* external);
  ~vtkInternal();

  void CreateClipSlices();

  /// Reset all the pick vars
  void ResetPick();
  /// Find picked node from mesh and set PickedNodeID in Internal
  void FindPickedDisplayNodeFromMesh(vtkPointSet* mesh, double pickedPoint[3]);
  /// Find node in scene from imageData and set PickedNodeID in Internal
  void FindDisplayNodeFromImageData(vtkDMMLScene* scene, vtkImageData* imageData);
  /// Find picked point index in mesh and picked cell (PickedCellID) and set PickedPointID in Internal
  void FindPickedPointOnMeshAndCell(vtkPointSet* mesh, double pickedPoint[3]);
  /// Find first picked node from prop3Ds in cell picker and set PickedNodeID in Internal
  void FindFirstPickedDisplayNodeFromPickerProp3Ds();

public:
  vtkDMMLModelDisplayableManager* External;

  std::map<std::string, vtkProp3D*>                DisplayedActors;
  std::map<std::string, vtkDMMLDisplayNode*>       DisplayedNodes;
  std::map<std::string, int>                       DisplayedClipState;
  std::map<std::string, vtkDMMLDisplayableNode*>   DisplayableNodes;
  std::map<std::string, int>                       RegisteredModelHierarchies;
  std::map<std::string, vtkTransformFilter*>       DisplayNodeTransformFilters;

  vtkDMMLSliceNode* RedSliceNode;
  vtkDMMLSliceNode* GreenSliceNode;
  vtkDMMLSliceNode* YellowSliceNode;

  vtkSmartPointer<vtkImplicitBoolean> SlicePlanes;
  vtkSmartPointer<vtkPlane>           RedSlicePlane;
  vtkSmartPointer<vtkPlane>           GreenSlicePlane;
  vtkSmartPointer<vtkPlane>           YellowSlicePlane;

  vtkDMMLClipModelsNode*  ClipModelsNode;
  int                     ClipType;
  int                     RedSliceClipState;
  int                     YellowSliceClipState;
  int                     GreenSliceClipState;
  int                     ClippingMethod;
  bool                    ClippingOn;

  bool IsUpdatingModelsFromDMML;

  vtkSmartPointer<vtkWorldPointPicker> WorldPointPicker;
  vtkSmartPointer<vtkPropPicker>       PropPicker;
  vtkSmartPointer<vtkCellPicker>       CellPicker;
  vtkSmartPointer<vtkPointPicker>      PointPicker;

  // Information about a pick event
  std::string  PickedDisplayNodeID;
  double       PickedRAS[3];
  vtkIdType    PickedCellID;
  vtkIdType    PickedPointID;

  // Used for caching the node pointer so that we do not have to search in the scene each time.
  // We do not add an observer therefore we can let the selection node deleted without our knowledge.
  vtkWeakPointer<vtkDMMLSelectionNode> SelectionNode;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkDMMLModelDisplayableManager::vtkInternal::vtkInternal(vtkDMMLModelDisplayableManager* external)
: External(external)
{
  this->ClipModelsNode = nullptr;
  this->RedSliceNode = nullptr;
  this->GreenSliceNode = nullptr;
  this->YellowSliceNode = nullptr;

  // Instantiate and initialize Pickers
  this->WorldPointPicker = vtkSmartPointer<vtkWorldPointPicker>::New();
  this->PropPicker = vtkSmartPointer<vtkPropPicker>::New();
  this->CellPicker = vtkSmartPointer<vtkCellPicker>::New();
  this->CellPicker->SetTolerance(0.00001);
  this->PointPicker = vtkSmartPointer<vtkPointPicker>::New();
  this->ResetPick();

  this->IsUpdatingModelsFromDMML = false;
}

//---------------------------------------------------------------------------
vtkDMMLModelDisplayableManager::vtkInternal::~vtkInternal() = default;

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::vtkInternal::CreateClipSlices()
{
  this->SlicePlanes = vtkSmartPointer<vtkImplicitBoolean>::New();
  this->SlicePlanes->SetOperationTypeToIntersection();

  this->RedSlicePlane = vtkSmartPointer<vtkPlane>::New();
  this->GreenSlicePlane = vtkSmartPointer<vtkPlane>::New();
  this->YellowSlicePlane = vtkSmartPointer<vtkPlane>::New();

  this->ClipType = vtkDMMLClipModelsNode::ClipIntersection;

  this->RedSliceClipState = vtkDMMLClipModelsNode::ClipOff;
  this->YellowSliceClipState = vtkDMMLClipModelsNode::ClipOff;
  this->GreenSliceClipState = vtkDMMLClipModelsNode::ClipOff;
  this->ClippingMethod = vtkDMMLClipModelsNode::Straight;

  this->ClippingOn = false;
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::vtkInternal::ResetPick()
{
  this->PickedDisplayNodeID.clear();
  for (int i=0; i < 3; i++)
    {
    this->PickedRAS[i] = 0.0;
    }
  this->PickedCellID = -1;
  this->PickedPointID = -1;
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::vtkInternal::FindPickedDisplayNodeFromMesh(vtkPointSet* mesh, double vtkNotUsed(pickedPoint)[3])
{
  if (!mesh)
    {
    return;
    }

  std::map<std::string, vtkDMMLDisplayNode *>::iterator modelIt;
  for (modelIt = this->DisplayedNodes.begin(); modelIt != this->DisplayedNodes.end(); modelIt++)
    {
    if (modelIt->second != 0)
      {
      if (vtkDMMLModelDisplayNode::SafeDownCast(modelIt->second) &&
          vtkDMMLModelDisplayNode::SafeDownCast(modelIt->second)->GetOutputMesh() == mesh)
        {
        this->PickedDisplayNodeID = modelIt->first;
        return; // Display node found
        }
      }
    }
}
//
//---------------------------------------------------------------------------
// for consistency with other vtkInternal classes this does not have access
// to the dmmlScene, so it is passed as a parameter
void vtkDMMLModelDisplayableManager::vtkInternal::FindDisplayNodeFromImageData(vtkDMMLScene *scene, vtkImageData* imageData)
{
  if (!scene || !imageData)
    {
    return;
    }
  // note that this library doesn't link to the VolumeRendering code because it is
  // a loadable module.  However we can still iterate over volume rendering nodes
  // and use the superclass abstract methods to confirm that the passed in imageData
  // corresponds to the display node.
  std::vector<vtkDMMLNode *> displayNodes;
  int nodeCount = scene->GetNodesByClass("vtkDMMLVolumeRenderingDisplayNode", displayNodes);
  for (int nodeIndex=0; nodeIndex < nodeCount; nodeIndex++)
    {
    vtkDMMLDisplayNode *displayNode = vtkDMMLDisplayNode::SafeDownCast(displayNodes[nodeIndex]);
    if (displayNode)
      {
      vtkDMMLVolumeNode *volumeNode = vtkDMMLVolumeNode::SafeDownCast(displayNode->GetDisplayableNode());
      vtkImageData *volumeImageData = nullptr;
      if (volumeNode)
        {
        volumeImageData = vtkImageData::SafeDownCast(volumeNode->GetImageData());
        }
      if (volumeImageData && volumeImageData == imageData)
        {
        this->PickedDisplayNodeID = displayNode->GetID();
        return; // Display node found
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::vtkInternal::FindPickedPointOnMeshAndCell(vtkPointSet* mesh, double pickedPoint[3])
{
  if (!mesh || this->PickedCellID < 0)
    {
    return;
    }

  // Figure out the closest vertex in the picked cell to the picked RAS
  // point. Only doing this on model nodes for now.
  vtkCell *cell = mesh->GetCell(this->PickedCellID);
  if (!cell)
    {
    return;
    }

  int numPoints = cell->GetNumberOfPoints();
  int closestPointId = -1;
  double closestDistance = 0.0l;
  for (int p = 0; p < numPoints; p++)
    {
    int pointId = cell->GetPointId(p);
    double *pointCoords = mesh->GetPoint(pointId);
    if (pointCoords != nullptr)
      {
      double distance = sqrt( pow(pointCoords[0]-pickedPoint[0], 2) +
                              pow(pointCoords[1]-pickedPoint[1], 2) +
                              pow(pointCoords[2]-pickedPoint[2], 2) );
      if (p == 0 || distance < closestDistance)
        {
        closestDistance = distance;
        closestPointId = pointId;
        }
      }
    }
  this->PickedPointID = closestPointId;
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::vtkInternal::FindFirstPickedDisplayNodeFromPickerProp3Ds()
{
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
    std::map<std::string, vtkProp3D*>::iterator propIt;
    for (propIt = this->DisplayedActors.begin(); propIt != this->DisplayedActors.end(); propIt++)
      {
      if (pickedProp == propIt->second)
        {
        this->PickedDisplayNodeID = propIt->first;
        return; // Display node found
        }
      }
    }
}


//---------------------------------------------------------------------------
// vtkDMMLModelDisplayableManager methods

//---------------------------------------------------------------------------
vtkDMMLModelDisplayableManager::vtkDMMLModelDisplayableManager()
{
  this->Internal = new vtkInternal(this);

  this->Internal->CreateClipSlices();
}

//---------------------------------------------------------------------------
vtkDMMLModelDisplayableManager::~vtkDMMLModelDisplayableManager()
{
  vtkSetDMMLNodeMacro(this->Internal->ClipModelsNode, 0);
  vtkSetDMMLNodeMacro(this->Internal->RedSliceNode, 0);
  vtkSetDMMLNodeMacro(this->Internal->GreenSliceNode, 0);
  vtkSetDMMLNodeMacro(this->Internal->YellowSliceNode, 0);
  this->Internal->SelectionNode = nullptr; // WeakPointer, therefore must not use vtkSetDMMLNodeMacro
  // release the DisplayedModelActors
  this->Internal->DisplayedActors.clear();

  // release transforms
  std::map<std::string, vtkTransformFilter *>::iterator tit;
  for (tit = this->Internal->DisplayNodeTransformFilters.begin();
       tit != this->Internal->DisplayNodeTransformFilters.end(); tit++ )
    {
    vtkTransformFilter  *transformFilter = (*tit).second;
    transformFilter->SetInputConnection(nullptr);
    transformFilter->SetTransform(nullptr);
    transformFilter->Delete();
    }

  this->Internal->DisplayNodeTransformFilters.clear();

  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->vtkObject::PrintSelf ( os, indent );

  os << indent << "vtkDMMLModelDisplayableManager: " << this->GetClassName() << "\n";

  os << indent << "ClipType = " << this->Internal->ClipType << "\n";
  os << indent << "RedSliceClipState = " << this->Internal->RedSliceClipState << "\n";
  os << indent << "YellowSliceClipState = " << this->Internal->YellowSliceClipState << "\n";
  os << indent << "GreenSliceClipState = " << this->Internal->GreenSliceClipState << "\n";
  os << indent << "ClippingMethod = " << this->Internal->ClippingMethod << "\n";
  os << indent << "ClippingOn = " << (this->Internal->ClippingOn ? "true" : "false") << "\n";

  os << indent << "PickedDisplayNodeID = " << this->Internal->PickedDisplayNodeID.c_str() << "\n";
  os << indent << "PickedRAS = (" << this->Internal->PickedRAS[0] << ", "
      << this->Internal->PickedRAS[1] << ", "<< this->Internal->PickedRAS[2] << ")\n";
  os << indent << "PickedCellID = " << this->Internal->PickedCellID << "\n";
  os << indent << "PickedPointID = " << this->Internal->PickedPointID << "\n";
}

//---------------------------------------------------------------------------
int vtkDMMLModelDisplayableManager::ActiveInteractionModes()
{
  //return vtkDMMLInteractionNode::ViewTransform;
  return 0;
}

//---------------------------------------------------------------------------
vtkDMMLClipModelsNode* vtkDMMLModelDisplayableManager::GetClipModelsNode()
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "returning Internal->ClipModelsNode of "
                << this->Internal->ClipModelsNode);
  return this->Internal->ClipModelsNode;
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::SetClipModelsNode(vtkDMMLClipModelsNode *snode)
{
  vtkSetAndObserveDMMLNodeMacro(this->Internal->ClipModelsNode, snode);
}

//---------------------------------------------------------------------------
int vtkDMMLModelDisplayableManager::UpdateClipSlicesFromDMML()
{
  if (this->GetDMMLScene() == nullptr)
    {
    return 0;
    }

  // update ClipModels node
  vtkDMMLClipModelsNode *clipNode = vtkDMMLClipModelsNode::SafeDownCast(this->GetDMMLScene()->GetFirstNodeByClass("vtkDMMLClipModelsNode"));
  if (clipNode != this->Internal->ClipModelsNode)
    {
    vtkSetAndObserveDMMLNodeMacro(this->Internal->ClipModelsNode, clipNode);
    }

  if (this->Internal->ClipModelsNode == nullptr)
    {
    return 0;
    }

  // update Slice nodes
  vtkDMMLSliceNode *nodeRed= nullptr;
  vtkDMMLSliceNode *nodeGreen= nullptr;
  vtkDMMLSliceNode *nodeYellow= nullptr;

  std::vector<vtkDMMLNode *> snodes;
  int nnodes = this->GetDMMLScene()->GetNodesByClass("vtkDMMLSliceNode", snodes);
  for (int n=0; n<nnodes; n++)
    {
    vtkDMMLSliceNode *node = vtkDMMLSliceNode::SafeDownCast (snodes[n]);
    //TODO: use perhaps SliceLogic to get the name instead of "Red" etc.
    if (!strcmp(node->GetLayoutName(), "Red"))
      {
      nodeRed = node;
      }
    else if (!strcmp(node->GetLayoutName(), "Green"))
      {
      nodeGreen = node;
      }
    else if (!strcmp(node->GetLayoutName(), "Yellow"))
      {
      nodeYellow = node;
      }
    }

  if (nodeRed != this->Internal->RedSliceNode)
    {
    vtkSetAndObserveDMMLNodeMacro(this->Internal->RedSliceNode, nodeRed);
    }
  if (nodeGreen != this->Internal->GreenSliceNode)
    {
    vtkSetAndObserveDMMLNodeMacro(this->Internal->GreenSliceNode, nodeGreen);
    }
  if (nodeYellow != this->Internal->YellowSliceNode)
    {
    vtkSetAndObserveDMMLNodeMacro(this->Internal->YellowSliceNode, nodeYellow);
    }

  if (this->Internal->RedSliceNode == nullptr ||
      this->Internal->GreenSliceNode == nullptr ||
      this->Internal->YellowSliceNode == nullptr)
    {
    return 0;
    }

  int modifiedState = 0;

  if ( this->Internal->ClipModelsNode->GetClipType() != this->Internal->ClipType)
    {
    modifiedState = 1;
    this->Internal->ClipType = this->Internal->ClipModelsNode->GetClipType();
    if (this->Internal->ClipType == vtkDMMLClipModelsNode::ClipIntersection)
      {
      this->Internal->SlicePlanes->SetOperationTypeToIntersection();
      }
    else if (this->Internal->ClipType == vtkDMMLClipModelsNode::ClipUnion)
      {
      this->Internal->SlicePlanes->SetOperationTypeToUnion();
      }
    else
      {
      vtkErrorMacro("vtkDMMLClipModelsNode:: Invalid Clip Type");
      }
    }

  if (this->Internal->ClipModelsNode->GetRedSliceClipState() != this->Internal->RedSliceClipState)
    {
    if (this->Internal->RedSliceClipState == vtkDMMLClipModelsNode::ClipOff)
      {
      this->Internal->SlicePlanes->AddFunction(this->Internal->RedSlicePlane);
      }
    else if (this->Internal->ClipModelsNode->GetRedSliceClipState() == vtkDMMLClipModelsNode::ClipOff)
      {
      this->Internal->SlicePlanes->RemoveFunction(this->Internal->RedSlicePlane);
      }
    modifiedState = 1;
    this->Internal->RedSliceClipState = this->Internal->ClipModelsNode->GetRedSliceClipState();
    }

  if (this->Internal->ClipModelsNode->GetGreenSliceClipState() != this->Internal->GreenSliceClipState)
    {
    if (this->Internal->GreenSliceClipState == vtkDMMLClipModelsNode::ClipOff)
      {
      this->Internal->SlicePlanes->AddFunction(this->Internal->GreenSlicePlane);
      }
    else if (this->Internal->ClipModelsNode->GetGreenSliceClipState() == vtkDMMLClipModelsNode::ClipOff)
      {
      this->Internal->SlicePlanes->RemoveFunction(this->Internal->GreenSlicePlane);
      }
    modifiedState = 1;
    this->Internal->GreenSliceClipState = this->Internal->ClipModelsNode->GetGreenSliceClipState();
    }

  if (this->Internal->ClipModelsNode->GetYellowSliceClipState() != this->Internal->YellowSliceClipState)
    {
    if (this->Internal->YellowSliceClipState == vtkDMMLClipModelsNode::ClipOff)
      {
      this->Internal->SlicePlanes->AddFunction(this->Internal->YellowSlicePlane);
      }
    else if (this->Internal->ClipModelsNode->GetYellowSliceClipState() == vtkDMMLClipModelsNode::ClipOff)
      {
      this->Internal->SlicePlanes->RemoveFunction(this->Internal->YellowSlicePlane);
      }
    modifiedState = 1;
    this->Internal->YellowSliceClipState = this->Internal->ClipModelsNode->GetYellowSliceClipState();
    }

  if (this->Internal->ClipModelsNode->GetClippingMethod() != this->Internal->ClippingMethod)
    {
    modifiedState = 1;
    this->Internal->ClippingMethod = this->Internal->ClipModelsNode->GetClippingMethod();
    }

  // compute clipping on/off
  if (this->Internal->ClipModelsNode->GetRedSliceClipState() == vtkDMMLClipModelsNode::ClipOff &&
      this->Internal->ClipModelsNode->GetGreenSliceClipState() == vtkDMMLClipModelsNode::ClipOff &&
      this->Internal->ClipModelsNode->GetYellowSliceClipState() == vtkDMMLClipModelsNode::ClipOff )
    {
    this->Internal->ClippingOn = false;
    }
  else
    {
    this->Internal->ClippingOn = true;
    }

  // set slice plane normals and origins
  vtkMatrix4x4 *sliceMatrix = nullptr;
  int planeDirection = 1;

  sliceMatrix = this->Internal->RedSliceNode->GetSliceToRAS();
  planeDirection = (this->Internal->RedSliceClipState == vtkDMMLClipModelsNode::ClipNegativeSpace) ? -1 : 1;
  this->SetClipPlaneFromMatrix(sliceMatrix, planeDirection, this->Internal->RedSlicePlane);

  sliceMatrix = this->Internal->GreenSliceNode->GetSliceToRAS();
  planeDirection = (this->Internal->GreenSliceClipState == vtkDMMLClipModelsNode::ClipNegativeSpace) ? -1 : 1;
  this->SetClipPlaneFromMatrix(sliceMatrix, planeDirection, this->Internal->GreenSlicePlane);

  sliceMatrix = this->Internal->YellowSliceNode->GetSliceToRAS();
  planeDirection = (this->Internal->YellowSliceClipState == vtkDMMLClipModelsNode::ClipNegativeSpace) ? -1 : 1;
  this->SetClipPlaneFromMatrix(sliceMatrix, planeDirection, this->Internal->YellowSlicePlane);

  return modifiedState;
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::SetClipPlaneFromMatrix(vtkMatrix4x4 *sliceMatrix,
                                                            int planeDirection,
                                                            vtkPlane *plane)
{
  double normal[3];
  double origin[3];

  for (int i = 0; i < 3; i++)
    {
    normal[i] = planeDirection * sliceMatrix->GetElement(i,2);
    origin[i] = sliceMatrix->GetElement(i,3);
    }
  plane->SetNormal(normal);
  plane->SetOrigin(origin);
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::ProcessDMMLNodesEvents(vtkObject *caller,
                                                           unsigned long event,
                                                           void *callData)
{
  if (this->GetDMMLScene() == nullptr)
    {
    return;
    }
  if ( this->GetInteractor() &&
     this->GetInteractor()->GetRenderWindow() &&
     this->GetInteractor()->GetRenderWindow()->CheckInRenderStatus())
    {
    vtkDebugMacro("skipping ProcessDMMLNodesEvents during render");
    return;
    }

  bool isUpdating = this->GetDMMLScene()->IsBatchProcessing();
  if (vtkDMMLDisplayableNode::SafeDownCast(caller))
    {
    // There is no need to request a render (which can be expensive if the
    // volume rendering is on) if nothing visible has changed.
    bool requestRender = true;
    vtkDMMLDisplayableNode* displayableNode = vtkDMMLDisplayableNode::SafeDownCast(caller);
    switch (event)
      {
      case vtkDMMLDisplayableNode::DisplayModifiedEvent:
         // don't go any further if the modified display node is not a model
        if (!this->IsModelDisplayable(displayableNode) &&
            !this->IsModelDisplayable(
              reinterpret_cast<vtkDMMLDisplayNode*>(callData)))
          {
          requestRender = false;
          break;
          } // else fall through
      case vtkCommand::ModifiedEvent:
      case vtkDMMLModelNode::MeshModifiedEvent:
      case vtkDMMLTransformableNode::TransformModifiedEvent:
        requestRender = this->OnDMMLDisplayableModelNodeModifiedEvent(displayableNode);
        break;
      default:
        // We don't expect any other types of events.
        break;
      }
    if (!isUpdating && requestRender)
      {
      this->RequestRender();
      }
    }
  else if (vtkDMMLClipModelsNode::SafeDownCast(caller))
    {
    if (event == vtkCommand::ModifiedEvent)
      {
      this->SetUpdateFromDMMLRequested(true);
      }
    if (!isUpdating)
      {
      this->RequestRender();
      }
    }
  else if (vtkDMMLSliceNode::SafeDownCast(caller))
    {
    bool requestRender = true;
    if (event == vtkCommand::ModifiedEvent)
      {
      if (this->UpdateClipSlicesFromDMML() || this->Internal->ClippingOn)
        {
        this->SetUpdateFromDMMLRequested(true);
        }
      else
        {
        requestRender = vtkDMMLSliceNode::SafeDownCast(caller)->GetSliceVisible() == 1;
        }
      }
    if (!isUpdating && requestRender)
      {
      this->RequestRender();
      }
    }
  else
    {
    this->Superclass::ProcessDMMLNodesEvents(caller, event, callData);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::UnobserveDMMLScene()
{
  this->RemoveModelProps();
  this->RemoveModelObservers(1);
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::OnDMMLSceneStartClose()
{
  this->RemoveModelObservers(0);
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::OnDMMLSceneEndClose()
{
  // Clean
  this->RemoveModelProps();
  this->RemoveModelObservers(1);

  this->SetUpdateFromDMMLRequested(true);
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::UpdateFromDMMLScene()
{
  // UpdateFromDMML will be executed only if there has been some actions
  // during the import that requested it (don't call
  // SetUpdateFromDMMLRequested(1) here, it should be done somewhere else
  // maybe in OnDMMLSceneNodeAddedEvent, OnDMMLSceneNodeRemovedEvent or
  // OnDMMLDisplayableModelNodeModifiedEvent).
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::OnDMMLSceneNodeAdded(vtkDMMLNode* node)
{
  if ( !node->IsA("vtkDMMLDisplayableNode") &&
       !node->IsA("vtkDMMLDisplayNode") &&
       !node->IsA("vtkDMMLClipModelsNode") )
    {
    return;
    }

  this->SetUpdateFromDMMLRequested(true);

  // Escape if the scene a scene is being closed, imported or connected
  if (this->GetDMMLScene()->IsBatchProcessing())
    {
    return;
    }

  // Node specific processing
  if (node->IsA("vtkDMMLClipModelsNode"))
    {
    vtkSetAndObserveDMMLNodeMacro(this->Internal->ClipModelsNode, node);
    }

  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::OnDMMLSceneNodeRemoved(vtkDMMLNode* node)
{
  if (!node->IsA("vtkDMMLDisplayableNode") &&
      !node->IsA("vtkDMMLDisplayNode") &&
      !node->IsA("vtkDMMLClipModelsNode"))
    {
    return;
    }

  this->SetUpdateFromDMMLRequested(true);

  // Escape if the scene a scene is being closed, imported or connected
  if (this->GetDMMLScene()->IsBatchProcessing())
    {
    return;
    }

  // Node specific processing
  if (node->IsA("vtkDMMLDisplayableNode"))
    {
    this->RemoveDisplayable(vtkDMMLDisplayableNode::SafeDownCast(node));
    }
  else if (node->IsA("vtkDMMLClipModelsNode"))
    {
    vtkSetDMMLNodeMacro(this->Internal->ClipModelsNode, 0);
    }

  this->RequestRender();
}

//---------------------------------------------------------------------------
bool vtkDMMLModelDisplayableManager::IsModelDisplayable(vtkDMMLDisplayableNode* node)const
{
  vtkDMMLModelNode* modelNode = vtkDMMLModelNode::SafeDownCast(node);
  if (!node ||
      (modelNode && modelNode->IsA("vtkDMMLAnnotationNode")))
    {
    /// issue 2666: don't manage annotation nodes - don't show lines between the control points
    return false;
    }
  if (modelNode && modelNode->GetMesh())
    {
    return true;
    }
  // Maybe a model node has no mesh but its display nodes have output
  //  (e.g. vtkDMMLGlyphableVolumeSliceDisplayNode).
  bool displayable = false;
  for (int i = 0; i < node->GetNumberOfDisplayNodes(); ++i)
    {
    displayable |= this->IsModelDisplayable(node->GetNthDisplayNode(i));
    if (displayable)
      {// Optimization: no need to search any further.
      continue;
      }
    }
  return displayable;
}

//---------------------------------------------------------------------------
bool vtkDMMLModelDisplayableManager::IsModelDisplayable(vtkDMMLDisplayNode* node)const
{
  vtkDMMLModelDisplayNode* modelDisplayNode = vtkDMMLModelDisplayNode::SafeDownCast(node);
  if (!modelDisplayNode)
    {
    return false;
    }
  if (modelDisplayNode->IsA("vtkDMMLAnnotationDisplayNode"))
    {
    /// issue 2666: don't manage annotation nodes - don't show lines between the control points
    return false;
    }
  return modelDisplayNode->GetOutputMesh() ? true : false;
}

//---------------------------------------------------------------------------
bool vtkDMMLModelDisplayableManager::OnDMMLDisplayableModelNodeModifiedEvent(
    vtkDMMLDisplayableNode* modelNode)
{
  if (!modelNode)
    {
    vtkErrorMacro("OnDMMLDisplayableModelNodeModifiedEvent: No model node given");
    return false;
    }

  if (!this->IsModelDisplayable(modelNode))
    {
    return false;
    }
  // If the node is already cached with an actor process only this one
  // If it was not visible and is still not visible do nothing
  int ndnodes = modelNode->GetNumberOfDisplayNodes();
  bool updateModel = false;
  bool updateDMML = false;
  for (int i=0; i<ndnodes; i++)
    {
    vtkDMMLDisplayNode *dnode = modelNode->GetNthDisplayNode(i);
    if (dnode == nullptr)
      {
      // display node has been removed
      updateDMML = true;
      break;
      }
    bool visible =
      (dnode->GetVisibility() == 1) && (dnode->GetVisibility3D() == 1) && this->IsModelDisplayable(dnode);
    bool hasActor =
      this->Internal->DisplayedActors.find(dnode->GetID()) != this->Internal->DisplayedActors.end();
    // If the displayNode is visible and doesn't have actors yet, then request
    // an updated
    if (visible && !hasActor)
      {
      updateDMML = true;
      break;
      }
    // If the displayNode visibility has changed or displayNode is visible, then
    // update the model.
    if (!(!visible && this->GetDisplayedModelsVisibility(dnode) == 0))
      {
      updateModel = true;
      break;
      }
    }
  if (updateModel)
    {
    this->UpdateModifiedModel(modelNode);
    }
  if (updateDMML)
    {
    this->SetUpdateFromDMMLRequested(true);
    }
  return updateModel || updateDMML;
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::UpdateFromDMML()
{
  if ( this->GetInteractor() &&
       this->GetInteractor()->GetRenderWindow() &&
       this->GetInteractor()->GetRenderWindow()->CheckInRenderStatus())
    {
    vtkDebugMacro("skipping update during render");
    return;
    }

  this->UpdateClipSlicesFromDMML();

  this->RemoveModelProps();

  this->UpdateModelsFromDMML();

  this->SetUpdateFromDMMLRequested(false);
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::UpdateModelsFromDMML()
{
  // UpdateModelsFromDMML may recursively trigger calling of UpdateModelsFromDMML
  // via node reference updates. IsUpdatingModelsFromDMML flag prevents restarting
  // UpdateModelsFromDMML if it is already in progress.
  if (this->Internal->IsUpdatingModelsFromDMML)
    {
    return;
    }
  this->Internal->IsUpdatingModelsFromDMML = true;
  vtkDMMLScene *scene = this->GetDMMLScene();
  vtkDMMLNode *node = nullptr;
  std::vector<vtkDMMLDisplayableNode *> slices;
  std::vector<vtkDMMLDisplayableNode *> nonSlices;

  // find volume slices
  bool clearDisplayedModels = scene ? false : true;

  std::vector<vtkDMMLNode *> dnodes;
  int nnodes = scene ? scene->GetNodesByClass("vtkDMMLDisplayableNode", dnodes) : 0;
  for (int n = 0; n<nnodes; n++)
    {
    node = dnodes[n];
    vtkDMMLDisplayableNode *model = vtkDMMLDisplayableNode::SafeDownCast(node);
    // render slices last so that transparent objects are rendered in front of them
    if (vtkDMMLSliceLogic::IsSliceModelNode(model))
      {
      slices.push_back(model);

      int ndnodes = model->GetNumberOfDisplayNodes();
      for (int i = 0; i<ndnodes && !clearDisplayedModels; i++)
        {
        vtkDMMLDisplayNode *dnode = model->GetNthDisplayNode(i);
        if (dnode && this->Internal->DisplayedActors.find(dnode->GetID()) == this->Internal->DisplayedActors.end())
          {
          // it is a new slice display node, therefore we need to remove all existing model node actors
          // and insert this slice actor before them
          clearDisplayedModels = true;
          break;
          }
        }
      }
    else
      {
      nonSlices.push_back(model);
      }
    }

  if (clearDisplayedModels)
    {
    for (std::pair< const std::string, vtkProp3D* > iter : this->Internal->DisplayedActors)
      {
      this->GetRenderer()->RemoveViewProp(iter.second);
      }
    this->RemoveModelObservers(1);
    this->Internal->DisplayedActors.clear();
    this->Internal->DisplayedNodes.clear();
    this->Internal->DisplayedClipState.clear();
    this->Internal->DisplayNodeTransformFilters.clear();
    }

  // render slices first
  for (vtkDMMLDisplayableNode * model : slices)
    {
    // add nodes that are not in the list yet
    int ndnodes = model->GetNumberOfDisplayNodes();
    for (int i = 0; i<ndnodes; i++)
      {
      vtkDMMLDisplayNode *dnode = model->GetNthDisplayNode(i);
      if (dnode && this->Internal->DisplayedActors.find(dnode->GetID()) == this->Internal->DisplayedActors.end())
        {
        this->UpdateModel(model);
        break;
        }
      }
    this->SetModelDisplayProperty(model);
    }

  // render the rest of the models
  for (vtkDMMLDisplayableNode* model : nonSlices)
    {
    this->UpdateModifiedModel(model);
    }
  this->Internal->IsUpdatingModelsFromDMML = false;
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::UpdateModifiedModel(vtkDMMLDisplayableNode *model)
{
  this->UpdateModel(model);
  this->SetModelDisplayProperty(model);
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::UpdateModelMesh(vtkDMMLDisplayableNode *displayableNode)
{
  int ndnodes = displayableNode->GetNumberOfDisplayNodes();
  int i;

  // if no model display nodes found, return
  int modelDisplayNodeCount = 0;
  for (i=0; i<ndnodes; i++)
    {
    vtkDMMLDisplayNode *dNode = displayableNode->GetNthDisplayNode(i);
    if (vtkDMMLModelDisplayNode::SafeDownCast(dNode) != nullptr)
      {
      modelDisplayNodeCount++;
      }
    }
  if (modelDisplayNodeCount == 0)
    {
    return;
    }

  vtkDMMLModelNode* modelNode = vtkDMMLModelNode::SafeDownCast(displayableNode);
  vtkDMMLDisplayNode *hdnode = vtkDMMLFolderDisplayNode::GetOverridingHierarchyDisplayNode(displayableNode);
  vtkDMMLModelNode *hierarchyModelDisplayNode = vtkDMMLModelNode::SafeDownCast(hdnode);

  bool hasNonLinearTransform = false;
  vtkDMMLTransformNode* tnode = displayableNode->GetParentTransformNode();
  vtkGeneralTransform *worldTransform = vtkGeneralTransform::New();
  worldTransform->Identity();
  if (tnode != nullptr && !tnode->IsTransformToWorldLinear())
    {
    hasNonLinearTransform = true;
    tnode->GetTransformToWorld(worldTransform);
    }

  for (i=0; i<ndnodes; i++)
    {
    vtkDMMLDisplayNode *displayNode = displayableNode->GetNthDisplayNode(i);
    vtkDMMLModelDisplayNode *modelDisplayNode = vtkDMMLModelDisplayNode::SafeDownCast(displayNode);

    // don't do anything if display node is invalid or it is a color legend
    if (!displayNode || (displayNode && displayNode->IsA("vtkDMMLColorLegendDisplayNode")))
      {
      continue;
      }

    vtkProp3D* prop = nullptr;

    int clipping = displayNode->GetClipping();
    vtkAlgorithmOutput *meshConnection = nullptr;
    if (this->IsModelDisplayable(modelDisplayNode))
      {
      meshConnection = modelDisplayNode->GetOutputMeshConnection();
      }
    if (hdnode)
      {
      clipping = hdnode->GetClipping();
      meshConnection = hierarchyModelDisplayNode ?
        hierarchyModelDisplayNode->GetMeshConnection() : meshConnection;
      }
    // hierarchy display nodes may not have mesh pointer
    if (meshConnection == nullptr && this->IsModelDisplayable(modelNode))
      {
      meshConnection = modelNode ? modelNode->GetMeshConnection() : nullptr;
      }
    bool hasMesh = (meshConnection != nullptr);

    if (!hasMesh)
      {
      continue;
      }

    // create TransformFilter for non-linear transform
    vtkTransformFilter* transformFilter = nullptr;
    if (hasNonLinearTransform)
      {
      std::map<std::string, vtkTransformFilter *>::iterator tit;
      tit = this->Internal->DisplayNodeTransformFilters.find(displayNode->GetID());
      if (tit == this->Internal->DisplayNodeTransformFilters.end() )
        {
        transformFilter = vtkTransformFilter::New();
        this->Internal->DisplayNodeTransformFilters[displayNode->GetID()] = transformFilter;
        }
      else
        {
        transformFilter = (*tit).second;
        }
      }

    if (transformFilter)
      {
      transformFilter->SetInputConnection(meshConnection);
      // It is important to only update the transform if the transform chain is actually changed,
      // because recomputing a non-linear transformation on a complex model may be very time-consuming.
      if (!vtkDMMLTransformNode::AreTransformsEqual(worldTransform, transformFilter->GetTransform()))
        {
        transformFilter->SetTransform(worldTransform);
        }
      }

    vtkDMMLModelNode::MeshTypeHint meshType = modelNode ? modelNode->GetMeshType() : vtkDMMLModelNode::PolyDataMeshType;

    std::map<std::string, vtkProp3D *>::iterator ait;
    ait = this->Internal->DisplayedActors.find(displayNode->GetID());
    if (ait == this->Internal->DisplayedActors.end() )
      {
      if (!prop)
        {
        prop = vtkActor::New();
        }
      }
    else
      {
      prop = (*ait).second;
      std::map<std::string, int>::iterator cit = this->Internal->DisplayedClipState.end();
      if (modelDisplayNode)
        {
        cit = this->Internal->DisplayedClipState.find(modelDisplayNode->GetID());
        }
      if (cit != this->Internal->DisplayedClipState.end() && cit->second == clipping )
        {
        // make sure that we are looking at the current mesh (most of the code in here
        // assumes a display node will never change what mesh it wants to view and hence
        // caches information to skip steps if the display node has already rendered. but we
        // can have rendered a display node but not rendered its current mesh.
        vtkActor *actor = vtkActor::SafeDownCast(prop);
        bool mapperUpdateNeeded = true; // mapper might not match the mesh type
        if (actor && actor->GetMapper())
          {
          vtkMapper *mapper = actor->GetMapper();
          if (transformFilter)
            {
            mapper->SetInputConnection(transformFilter->GetOutputPort());
            }
          else if (mapper && !(this->Internal->ClippingOn && clipping))
            {
            mapper->SetInputConnection(meshConnection);
            }
          if ((meshType == vtkDMMLModelNode::UnstructuredGridMeshType && mapper->IsA("vtkDataSetMapper"))
            || (meshType == vtkDMMLModelNode::PolyDataMeshType && mapper->IsA("vtkPolyDataMapper")))
            {
            mapperUpdateNeeded = false;
            }
          }
        vtkDMMLTransformNode* tnode = displayableNode->GetParentTransformNode();
        // clipped model could be transformed
        // TODO: handle non-linear transforms
        if ((clipping == 0 || tnode == nullptr || !tnode->IsTransformToWorldLinear()) && !mapperUpdateNeeded)
          {
          continue;
          }
        }
      }

    vtkActor *actor = vtkActor::SafeDownCast(prop);
    vtkAlgorithm *clipper = nullptr;
    if(actor)
      {
      if (this->Internal->ClippingOn && modelDisplayNode != nullptr && clipping)
        {
        clipper = this->CreateTransformedClipper(modelNode->GetParentTransformNode(), meshType);
        }

      vtkMapper *mapper = nullptr;
      if (meshType == vtkDMMLModelNode::UnstructuredGridMeshType)
        {
        mapper = vtkDataSetMapper::New();
        }
      else //if (meshType == vtkDMMLModelNode::PolyDataMeshType) // unknown when new. need to set type
        {
        mapper = vtkPolyDataMapper::New();
        }

      if (clipper)
        {
        if (transformFilter) clipper->SetInputConnection(transformFilter->GetOutputPort());
        else clipper->SetInputConnection(meshConnection);
        mapper->SetInputConnection(clipper->GetOutputPort());
        }
      else if (transformFilter)
        {
        mapper->SetInputConnection(transformFilter->GetOutputPort());
        }
      else
        {
        mapper->SetInputConnection(meshConnection);
        }

      actor->SetMapper(mapper);
      mapper->Delete();
      }

    if (hasMesh && ait == this->Internal->DisplayedActors.end())
      {
      this->GetRenderer()->AddViewProp(prop);
      this->Internal->DisplayedActors[modelDisplayNode->GetID()] = prop;
      this->Internal->DisplayedNodes[std::string(modelDisplayNode->GetID())] = modelDisplayNode;

      if (clipper)
        {
        this->Internal->DisplayedClipState[modelDisplayNode->GetID()] = 1;
        clipper->Delete();
        }
      else
        {
        this->Internal->DisplayedClipState[modelDisplayNode->GetID()] = 0;
        }
      prop->Delete();
      }
    else if (!hasMesh)
      {
      prop->Delete();
      }
    else
      {
      if (clipper)
        {
        this->Internal->DisplayedClipState[modelDisplayNode->GetID()] = 1;
        clipper->Delete();
        }
      else
        {
        this->Internal->DisplayedClipState[modelDisplayNode->GetID()] = 0;
        }
      }
    }
  worldTransform->Delete();
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::UpdateModel(vtkDMMLDisplayableNode *model)
{
  this->UpdateModelMesh(model);

  vtkEventBroker *broker = vtkEventBroker::GetInstance();
  vtkEventBroker::ObservationVector observations;
  // observe mesh;
  if (!broker->GetObservationExist(model, vtkDMMLModelNode::MeshModifiedEvent,
                                         this, this->GetDMMLNodesCallbackCommand()))
    {
    broker->AddObservation(model, vtkDMMLModelNode::MeshModifiedEvent,
                           this, this->GetDMMLNodesCallbackCommand());
    this->Internal->DisplayableNodes[model->GetID()] = model;
    }
  // observe display node
  if (!broker->GetObservationExist(model, vtkDMMLDisplayableNode::DisplayModifiedEvent,
                                         this, this->GetDMMLNodesCallbackCommand()))
    {
    broker->AddObservation(model, vtkDMMLDisplayableNode::DisplayModifiedEvent,
                           this, this->GetDMMLNodesCallbackCommand());
    }

  if (!broker->GetObservationExist(model, vtkDMMLTransformableNode::TransformModifiedEvent,
                                         this, this->GetDMMLNodesCallbackCommand()))
    {
    broker->AddObservation(model, vtkDMMLTransformableNode::TransformModifiedEvent,
                           this, this->GetDMMLNodesCallbackCommand());
    }
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::RemoveModelProps()
{
  std::map<std::string, vtkProp3D *>::iterator iter;
  std::map<std::string, int>::iterator clipIter;
  std::vector<std::string> removedIDs;
  for(iter=this->Internal->DisplayedActors.begin(); iter != this->Internal->DisplayedActors.end(); iter++)
    {
    vtkDMMLDisplayNode *modelDisplayNode = vtkDMMLDisplayNode::SafeDownCast(
      this->GetDMMLScene() ? this->GetDMMLScene()->GetNodeByID(iter->first) : nullptr);
    if (modelDisplayNode == nullptr)
      {
      this->GetRenderer()->RemoveViewProp(iter->second);
      removedIDs.push_back(iter->first);
      }
    else
      {
      int clipModel = 0;
      if (modelDisplayNode != nullptr)
        {
        clipModel = modelDisplayNode->GetClipping();
        }
      clipIter = this->Internal->DisplayedClipState.find(iter->first);
      if (clipIter == this->Internal->DisplayedClipState.end())
        {
        vtkErrorMacro ("vtkDMMLModelDisplayableManager::RemoveModelProps() Unknown clip state\n");
        }
      else
        {

        if (clipIter->second  || (this->Internal->ClippingOn && clipIter->second != clipModel))
          {
          this->GetRenderer()->RemoveViewProp(iter->second);
          removedIDs.push_back(iter->first);
          }
        }
      }
    }
  for (unsigned int i=0; i< removedIDs.size(); i++)
    {
    this->RemoveDisplayedID(removedIDs[i]);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::RemoveDisplayable(vtkDMMLDisplayableNode* model)
{
  if (!model)
    {
    return;
    }
  const int ndnodes = model->GetNumberOfDisplayNodes();
  std::vector<std::string> removedIDs;
  for (int i=0; i<ndnodes; i++)
    {
    const char* displayNodeIDToRemove = model->GetNthDisplayNodeID(i);
    if (!displayNodeIDToRemove)
      {
      continue;
      }
    std::map<std::string, vtkProp3D *>::iterator iter =
      this->Internal->DisplayedActors.find(displayNodeIDToRemove);
    if (iter != this->Internal->DisplayedActors.end())
      {
      this->GetRenderer()->RemoveViewProp(iter->second);
      removedIDs.push_back(iter->first);
      }
    }

  for (unsigned int i=0; i< removedIDs.size(); i++)
    {
    this->RemoveDisplayedID(removedIDs[i]);
    }
  this->RemoveDisplayableNodeObservers(model);
  this->Internal->DisplayableNodes.erase(model->GetID());
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::RemoveDisplayedID(std::string &id)
{
  std::map<std::string, vtkDMMLDisplayNode *>::iterator modelIter;
  this->Internal->DisplayedActors.erase(id);
  this->Internal->DisplayedClipState.erase(id);
  modelIter = this->Internal->DisplayedNodes.find(id);
  if (modelIter != this->Internal->DisplayedNodes.end())
    {
    this->Internal->DisplayedNodes.erase(modelIter->first);
    }
}

//---------------------------------------------------------------------------
int vtkDMMLModelDisplayableManager::GetDisplayedModelsVisibility(vtkDMMLDisplayNode* displayNode)
{
  if (!displayNode)
    {
    vtkErrorMacro("GetDisplayedModelsVisibility: No display node given");
    return 0;
    }

  std::map<std::string, vtkProp3D*>::iterator it = this->Internal->DisplayedActors.find(displayNode->GetID());
  if (it == this->Internal->DisplayedActors.end())
    {
    return 0;
    }

  vtkProp3D* actor = it->second;
  return actor->GetVisibility();
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::RemoveDMMLObservers()
{
  this->RemoveModelObservers(1);

  this->Superclass::RemoveDMMLObservers();
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::RemoveModelObservers(int clearCache)
{
  std::map<std::string, vtkDMMLDisplayableNode *>::iterator iter;

  for (iter=this->Internal->DisplayableNodes.begin();
       iter!=this->Internal->DisplayableNodes.end();
       iter++)
    {
    this->RemoveDisplayableNodeObservers(iter->second);
    }
  if (clearCache)
    {
    this->Internal->DisplayableNodes.clear();
    this->Internal->DisplayedActors.clear();
    this->Internal->DisplayedNodes.clear();
    this->Internal->DisplayedClipState.clear();
    }
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::RemoveDisplayableNodeObservers(vtkDMMLDisplayableNode* model)
{
  vtkEventBroker *broker = vtkEventBroker::GetInstance();
  vtkEventBroker::ObservationVector observations;
  if (model != nullptr)
    {
    observations = broker->GetObservations(
      model, vtkDMMLModelNode::MeshModifiedEvent, this, this->GetDMMLNodesCallbackCommand() );
    broker->RemoveObservations(observations);
    observations = broker->GetObservations(
      model, vtkDMMLDisplayableNode::DisplayModifiedEvent, this, this->GetDMMLNodesCallbackCommand() );
    broker->RemoveObservations(observations);
    observations = broker->GetObservations(
      model, vtkDMMLTransformableNode::TransformModifiedEvent, this, this->GetDMMLNodesCallbackCommand() );
    broker->RemoveObservations(observations);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::SetModelDisplayProperty(vtkDMMLDisplayableNode* model)
{
  // Get transformation applied on model
  vtkDMMLTransformNode* transformNode = model->GetParentTransformNode();
  vtkNew<vtkMatrix4x4> matrixTransformToWorld;
  if (transformNode != nullptr && transformNode->IsTransformToWorldLinear())
    {
    transformNode->GetMatrixTransformToWorld(matrixTransformToWorld.GetPointer());
    }

  // Get display node from hierarchy that applies display properties on branch
  vtkDMMLDisplayNode* overrideHierarchyDisplayNode =
    vtkDMMLFolderDisplayNode::GetOverridingHierarchyDisplayNode(model);

  // Set display properties to props for all display nodes
  int numberOfDisplayNodes = model->GetNumberOfDisplayNodes();
  for (int i=0; i<numberOfDisplayNodes; i++)
    {
    vtkDMMLDisplayNode* displayNode = model->GetNthDisplayNode(i);
    vtkDMMLModelDisplayNode* modelDisplayNode = vtkDMMLModelDisplayNode::SafeDownCast(displayNode);
    if (!modelDisplayNode)
      {
      continue;
      }
    vtkProp3D *prop = this->GetActorByID(modelDisplayNode->GetID());
    if (prop == nullptr)
      {
      continue;
      }

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
      hierarchyVisibility = vtkDMMLFolderDisplayNode::GetHierarchyVisibility(model);
      hierarchyOpacity = vtkDMMLFolderDisplayNode::GetHierarchyOpacity(model);
      }

    vtkActor *actor = vtkActor::SafeDownCast(prop);
    vtkImageActor *imageActor = vtkImageActor::SafeDownCast(prop);
    prop->SetUserMatrix(matrixTransformToWorld.GetPointer());

    // If there is an overriding hierarchy display node, then consider its visibility as well
    // as the model's. It is important to consider the model's visibility, because the user will
    // still want to show/hide children regardless of application of display properties from the
    // hierarchy.
    bool visible = hierarchyVisibility
      && modelDisplayNode->GetVisibility() && modelDisplayNode->GetVisibility3D()
      && modelDisplayNode->IsDisplayableInView(this->GetDMMLViewNode()->GetID());
    prop->SetVisibility(visible);

    vtkMapper* mapper = actor ? actor->GetMapper() : nullptr;
    if (mapper)
      {
      mapper->SetScalarVisibility(displayNode->GetScalarVisibility());
      // if the scalars are visible, set active scalars, the lookup table
      // and the scalar range
      if (visible && displayNode->GetScalarVisibility())
        {
        // Check if using point data or cell data
        vtkDMMLModelNode* modelNode = vtkDMMLModelNode::SafeDownCast(model);
        if (this->IsCellScalarsActive(displayNode, modelNode))
          {
          mapper->SetScalarModeToUseCellData();
          }
        else
          {
          mapper->SetScalarModeToUsePointData();
          }

        if (displayNode->GetScalarRangeFlag() == vtkDMMLDisplayNode::UseDirectMapping)
          {
          mapper->UseLookupTableScalarRangeOn(); // avoid warning about bad table range
          mapper->SetColorModeToDirectScalars();
          mapper->SetLookupTable(nullptr);
          }
        else
          {
          mapper->UseLookupTableScalarRangeOff();
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
          mapper->SetLookupTable(dNodeLUT);
          }

        // Set scalar range
        mapper->SetScalarRange(displayNode->GetScalarRange());
        }

      vtkProperty* actorProperties = actor->GetProperty();
      actorProperties->SetRepresentation(displayNode->GetRepresentation());
      actorProperties->SetPointSize(displayNode->GetPointSize());
      actorProperties->SetLineWidth(displayNode->GetLineWidth());
      actorProperties->SetLighting(displayNode->GetLighting());
      actorProperties->SetInterpolation(displayNode->GetInterpolation());
      actorProperties->SetShading(displayNode->GetShading());
      actorProperties->SetFrontfaceCulling(displayNode->GetFrontfaceCulling());
      actorProperties->SetBackfaceCulling(displayNode->GetBackfaceCulling());

      actor->SetPickable(model->GetSelectable());
      if (displayNode->GetSelected())
        {
        actorProperties->SetColor(displayNode->GetSelectedColor());
        actorProperties->SetAmbient(displayNode->GetSelectedAmbient());
        actorProperties->SetSpecular(displayNode->GetSelectedSpecular());
        }
      else
        {
        actorProperties->SetColor(displayNode->GetColor());
        actorProperties->SetAmbient(displayNode->GetAmbient());
        actorProperties->SetSpecular(displayNode->GetSpecular());
        }
      // Opacity will be the product of the opacities of the model and the overriding
      // hierarchy, in order to keep the relative opacities the same.
      actorProperties->SetOpacity(hierarchyOpacity * modelDisplayNode->GetOpacity());
      actorProperties->SetDiffuse(displayNode->GetDiffuse());
      actorProperties->SetSpecularPower(displayNode->GetPower());
      actorProperties->SetMetallic(displayNode->GetMetallic());
      actorProperties->SetRoughness(displayNode->GetRoughness());
      actorProperties->SetEdgeVisibility(displayNode->GetEdgeVisibility());
      actorProperties->SetEdgeColor(displayNode->GetEdgeColor());
      if (displayNode->GetTextureImageDataConnection() != nullptr)
        {
        if (actor->GetTexture() == nullptr)
          {
          vtkTexture *texture = vtkTexture::New();
          actor->SetTexture(texture);
          texture->Delete();
          }
        actor->GetTexture()->SetInputConnection(displayNode->GetTextureImageDataConnection());
        actor->GetTexture()->SetInterpolate(displayNode->GetInterpolateTexture());
        actorProperties->SetColor(1., 1., 1.);

        // Force actors to be treated as opaque. Otherwise, transparent
        // elements in the texture cause the actor to be treated as
        // translucent, i.e. rendered without writing to the depth buffer.
        // See https://github.com/Slicer/Slicer/issues/4253.
        actor->SetForceOpaque(actorProperties->GetOpacity() >= 1.0);
        }
      else
        {
        actor->SetTexture(nullptr);
        actor->ForceOpaqueOff();
        }

      // Set backface properties
      vtkProperty* actorBackfaceProperties = actor->GetBackfaceProperty();
      if (!actorBackfaceProperties)
        {
        vtkNew<vtkProperty> newActorBackfaceProperties;
        actor->SetBackfaceProperty(newActorBackfaceProperties);
        actorBackfaceProperties = newActorBackfaceProperties;
        }
      actorBackfaceProperties->DeepCopy(actorProperties);

      double offsetHsv[3];
      modelDisplayNode->GetBackfaceColorHSVOffset(offsetHsv);

      double colorHsv[3];
      vtkMath::RGBToHSV(actorProperties->GetColor(), colorHsv);
      double colorRgb[3];
      colorHsv[0] += offsetHsv[0];
      // wrap around hue value
      if (colorHsv[0] < 0.0)
        {
        colorHsv[0] += 1.0;
        }
      else if (colorHsv[0] > 1.0)
        {
        colorHsv[0] -= 1.0;
        }
      colorHsv[1] = vtkMath::ClampValue<double>(colorHsv[1] + offsetHsv[1], 0, 1);
      colorHsv[2] = vtkMath::ClampValue<double>(colorHsv[2] + offsetHsv[2], 0, 1);
      vtkMath::HSVToRGB(colorHsv, colorRgb);
      actorBackfaceProperties->SetColor(colorRgb);
      }
    else if (imageActor)
      {
      imageActor->GetMapper()->SetInputConnection(displayNode->GetTextureImageDataConnection());
      imageActor->SetDisplayExtent(-1, 0, 0, 0, 0, 0);
      }
    }
}

//---------------------------------------------------------------------------
const char* vtkDMMLModelDisplayableManager::GetActiveScalarName(
  vtkDMMLDisplayNode* displayNode, vtkDMMLModelNode* modelNode)
{
  const char* activeScalarName = nullptr;
  if (displayNode)
    {
    vtkDMMLModelDisplayNode *modelDisplayNode = vtkDMMLModelDisplayNode::SafeDownCast(displayNode);
    if (modelDisplayNode && modelDisplayNode->GetOutputMesh())
      {
      modelDisplayNode->GetOutputMeshConnection()->GetProducer()->Update();
      }
    activeScalarName = displayNode->GetActiveScalarName();
    }
  if (activeScalarName)
    {
    return activeScalarName;
    }
  if (modelNode)
    {
    if (modelNode->GetMesh())
      {
      vtkAlgorithmOutput *meshConnection = modelNode->GetMeshConnection();
      if (meshConnection != nullptr)
        {
        meshConnection->GetProducer()->Update();
        }
      }
    activeScalarName = modelNode->GetActiveCellScalarName(vtkDataSetAttributes::SCALARS);
    if (activeScalarName)
      {
      return activeScalarName;
      }
    activeScalarName = modelNode->GetActivePointScalarName(vtkDataSetAttributes::SCALARS);
    if (activeScalarName)
      {
      return activeScalarName;
      }
    }
  return nullptr;
}

//---------------------------------------------------------------------------
bool vtkDMMLModelDisplayableManager::IsCellScalarsActive(
  vtkDMMLDisplayNode* displayNode, vtkDMMLModelNode* modelNode)
{
  if (displayNode && displayNode->GetActiveScalarName())
    {
    return (displayNode->GetActiveAttributeLocation() == vtkAssignAttribute::CELL_DATA);
    }
  if (modelNode &&
      modelNode->GetActiveCellScalarName(vtkDataSetAttributes::SCALARS))
    {
    return true;
    }
  return false;
}

//---------------------------------------------------------------------------
// Description:
// return the current actor corresponding to a give DMML ID
vtkProp3D * vtkDMMLModelDisplayableManager::GetActorByID(const char *id)
{
  if ( !id )
    {
    return (nullptr);
    }

  std::map<std::string, vtkProp3D *>::iterator iter =
    this->Internal->DisplayedActors.find(std::string(id));
  if (iter != this->Internal->DisplayedActors.end())
    {
    return iter->second;
    }

  return (nullptr);
}

//---------------------------------------------------------------------------
// Description:
// return the ID for the given actor
const char * vtkDMMLModelDisplayableManager::GetIDByActor(vtkProp3D *actor)
{
  if ( !actor )
    {
    return (nullptr);
    }

  std::map<std::string, vtkProp3D *>::iterator iter;
  for(iter=this->Internal->DisplayedActors.begin();
      iter != this->Internal->DisplayedActors.end();
      iter++)
    {
    if ( iter->second && ( iter->second == actor ) )
      {
      return (iter->first.c_str());
      }
    }
  return (nullptr);
}

//---------------------------------------------------------------------------
vtkWorldPointPicker* vtkDMMLModelDisplayableManager::GetWorldPointPicker()
{
  vtkDebugMacro(<< "returning Internal->WorldPointPicker address "
                << this->Internal->WorldPointPicker.GetPointer());
  return this->Internal->WorldPointPicker;
}

//---------------------------------------------------------------------------
vtkPropPicker* vtkDMMLModelDisplayableManager::GetPropPicker()
{
  vtkDebugMacro(<< "returning Internal->PropPicker address "
                << this->Internal->PropPicker.GetPointer());
  return this->Internal->PropPicker;
}

//---------------------------------------------------------------------------
vtkCellPicker* vtkDMMLModelDisplayableManager::GetCellPicker()
{
  vtkDebugMacro(<< "returning Internal->CellPicker address "
                << this->Internal->CellPicker.GetPointer());
  return this->Internal->CellPicker;
}

//---------------------------------------------------------------------------
vtkPointPicker* vtkDMMLModelDisplayableManager::GetPointPicker()
{
  vtkDebugMacro(<< "returning Internal->PointPicker address "
                << this->Internal->PointPicker.GetPointer());
  return this->Internal->PointPicker;
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::SetPickTolerance(double tolerance)
{
  this->Internal->CellPicker->SetTolerance(tolerance);
}

//---------------------------------------------------------------------------
double vtkDMMLModelDisplayableManager::GetPickTolerance()
{
  return this->Internal->CellPicker->GetTolerance();
}

//---------------------------------------------------------------------------
int vtkDMMLModelDisplayableManager::Pick(int x, int y)
{
  double RASPoint[3] = {0.0, 0.0, 0.0};
  double pickPoint[3] = {0.0, 0.0, 0.0};

  // Reset the pick vars
  this->Internal->ResetPick();

  vtkRenderer* ren = this->GetRenderer();
  if (!ren)
    {
    vtkErrorMacro("Pick: unable to get renderer\n");
    return 0;
    }
  // get the current renderer's size
  int *renSize = ren->GetSize();
  // resize the interactor?

  // pass the event's display point to the world point picker
  double displayPoint[3];
  displayPoint[0] = x;
  displayPoint[1] = renSize[1] - y;
  displayPoint[2] = 0.0;

  if (this->Internal->CellPicker->Pick(displayPoint[0], displayPoint[1], displayPoint[2], ren))
    {
    this->Internal->CellPicker->GetPickPosition(pickPoint);
    this->SetPickedCellID(this->Internal->CellPicker->GetCellId());

    // look for either picked mesh or volume
    // and set picked display node accordingly
    vtkPointSet *mesh = vtkPointSet::SafeDownCast(this->Internal->CellPicker->GetDataSet());
    if (mesh)
      {
      // get the pointer to the mesh that the cell was in
      // and then find the model this mesh belongs to
      this->Internal->FindPickedDisplayNodeFromMesh(mesh, pickPoint);
      }
    vtkImageData *imageData = vtkImageData::SafeDownCast(this->Internal->CellPicker->GetDataSet());
    if (imageData)
      {
      // get the pointer to the picked imageData
      // and then find the volume this imageData belongs to
      this->Internal->FindDisplayNodeFromImageData(this->GetDMMLScene(), imageData);
      }
    }
  else
    {
    // there may not have been an actor at the picked point, but the Pick should be translated to a valid position
    // TBD: warn the user that they're picking in empty space?
    this->Internal->CellPicker->GetPickPosition(pickPoint);
    }

  // translate world to RAS
  for (int p = 0; p < 3; p++)
    {
    RASPoint[p] = pickPoint[p];
    }

  // now set up the class vars
  this->SetPickedRAS(RASPoint);

  return 1;
}

//---------------------------------------------------------------------------
int vtkDMMLModelDisplayableManager::Pick3D(double ras[3])
{
  // Reset the pick vars
  this->Internal->ResetPick();

  vtkRenderer* ren = this->GetRenderer();
  if (!ren)
    {
    vtkErrorMacro("Pick3D: Unable to get renderer");
    return 0;
    }

  if (this->Internal->CellPicker->Pick3DPoint(ras, ren))
    {
    this->SetPickedCellID(this->Internal->CellPicker->GetCellId());

    // Find first picked model from picker
    // Note: Getting the mesh using GetDataSet is not a good solution as the dataset is the first
    //   one that is picked and it may be of different type (volume, segmentation, etc.)
    this->Internal->FindFirstPickedDisplayNodeFromPickerProp3Ds();
    // Find picked point in mesh
    vtkDMMLModelDisplayNode* displayNode = vtkDMMLModelDisplayNode::SafeDownCast(
      this->GetDMMLScene()->GetNodeByID(this->Internal->PickedDisplayNodeID.c_str()) );
    if (displayNode)
      {
      this->Internal->FindPickedPointOnMeshAndCell(displayNode->GetOutputMesh(), ras);
      }

    this->SetPickedRAS(ras);
    }

  return 1;
}

//---------------------------------------------------------------------------
const char * vtkDMMLModelDisplayableManager::GetPickedNodeID()
{
  vtkDebugMacro(<< "returning this->Internal->PickedDisplayNodeID of "
                << (this->Internal->PickedDisplayNodeID.empty()?
                    "(empty)":this->Internal->PickedDisplayNodeID));
  return this->Internal->PickedDisplayNodeID.c_str();
}

//---------------------------------------------------------------------------
double* vtkDMMLModelDisplayableManager::GetPickedRAS()
{
  vtkDebugMacro(<< "returning Internal->PickedRAS pointer " << this->Internal->PickedRAS);
  return this->Internal->PickedRAS;
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::SetPickedRAS(double* newPickedRAS)
{
  int i;
  for (i=0; i<3; i++) { if ( newPickedRAS[i] != this->Internal->PickedRAS[i] ) { break; }}
  if (i < 3)
    {
    for (i=0; i<3; i++) { this->Internal->PickedRAS[i] = newPickedRAS[i]; }
    this->Modified();
    }
}

//---------------------------------------------------------------------------
vtkIdType vtkDMMLModelDisplayableManager::GetPickedCellID()
{
  vtkDebugMacro(<< "returning this->Internal->PickedCellID of " << this->Internal->PickedCellID);
  return this->Internal->PickedCellID;
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::SetPickedCellID(vtkIdType newCellID)
{
  vtkDebugMacro(<< "setting PickedCellID to " << newCellID);
  if (this->Internal->PickedCellID != newCellID)
    {
    this->Internal->PickedCellID = newCellID;
    this->Modified();
    }
}

//---------------------------------------------------------------------------
vtkIdType vtkDMMLModelDisplayableManager::GetPickedPointID()
{
  vtkDebugMacro(<< "returning this->Internal->PickedPointID of " << this->Internal->PickedPointID);
  return this->Internal->PickedPointID;
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::SetPickedPointID(vtkIdType newPointID)
{
  vtkDebugMacro(<< "setting PickedPointID to " << newPointID);
  if (this->Internal->PickedPointID != newPointID)
    {
    this->Internal->PickedPointID = newPointID;
    this->Modified();
    }
}

//---------------------------------------------------------------------------
vtkAlgorithm* vtkDMMLModelDisplayableManager
::CreateTransformedClipper(vtkDMMLTransformNode *tnode, vtkDMMLModelNode::MeshTypeHint type)
{
  vtkNew<vtkMatrix4x4> transformToWorld;
  transformToWorld->Identity();
  vtkSmartPointer<vtkImplicitBoolean> slicePlanes;
  if (tnode != nullptr && tnode->IsTransformToWorldLinear())
    {
    slicePlanes = vtkSmartPointer<vtkImplicitBoolean>::New();
    tnode->GetMatrixTransformToWorld(transformToWorld.GetPointer());


    if (this->Internal->ClipType == vtkDMMLClipModelsNode::ClipIntersection)
      {
      slicePlanes->SetOperationTypeToIntersection();
      }
    else if (this->Internal->ClipType == vtkDMMLClipModelsNode::ClipUnion)
      {
      slicePlanes->SetOperationTypeToUnion();
      }

    vtkNew<vtkPlane> redSlicePlane;
    vtkNew<vtkPlane> greenSlicePlane;
    vtkNew<vtkPlane> yellowSlicePlane;

    if (this->Internal->RedSliceClipState != vtkDMMLClipModelsNode::ClipOff)
      {
      slicePlanes->AddFunction(redSlicePlane.GetPointer());
      }

    if (this->Internal->GreenSliceClipState != vtkDMMLClipModelsNode::ClipOff)
      {
      slicePlanes->AddFunction(greenSlicePlane.GetPointer());
      }

    if (this->Internal->YellowSliceClipState != vtkDMMLClipModelsNode::ClipOff)
      {
      slicePlanes->AddFunction(yellowSlicePlane.GetPointer());
      }

    vtkMatrix4x4 *sliceMatrix = nullptr;
    vtkNew<vtkMatrix4x4> mat;
    int planeDirection = 1;
    transformToWorld->Invert();

    sliceMatrix = this->Internal->RedSliceNode->GetSliceToRAS();
    mat->Identity();
    vtkMatrix4x4::Multiply4x4(transformToWorld.GetPointer(), sliceMatrix, mat.GetPointer());
    planeDirection = (this->Internal->RedSliceClipState == vtkDMMLClipModelsNode::ClipNegativeSpace) ? -1 : 1;
    this->SetClipPlaneFromMatrix(mat.GetPointer(), planeDirection, redSlicePlane.GetPointer());

    sliceMatrix = this->Internal->GreenSliceNode->GetSliceToRAS();
    mat->Identity();
    vtkMatrix4x4::Multiply4x4(transformToWorld.GetPointer(), sliceMatrix, mat.GetPointer());
    planeDirection = (this->Internal->GreenSliceClipState == vtkDMMLClipModelsNode::ClipNegativeSpace) ? -1 : 1;
    this->SetClipPlaneFromMatrix(mat.GetPointer(), planeDirection, greenSlicePlane.GetPointer());

    sliceMatrix = this->Internal->YellowSliceNode->GetSliceToRAS();
    mat->Identity();
    vtkMatrix4x4::Multiply4x4(transformToWorld.GetPointer(), sliceMatrix, mat.GetPointer());
    planeDirection = (this->Internal->YellowSliceClipState == vtkDMMLClipModelsNode::ClipNegativeSpace) ? -1 : 1;
    this->SetClipPlaneFromMatrix(mat.GetPointer(), planeDirection, yellowSlicePlane.GetPointer());
    }
  else
    {
    slicePlanes = this->Internal->SlicePlanes;
    }
  if (type == vtkDMMLModelNode::UnstructuredGridMeshType)
    {
    if (this->Internal->ClippingMethod == vtkDMMLClipModelsNode::Straight)
      {
      vtkClipDataSet* clipper = vtkClipDataSet::New();
      clipper->SetClipFunction(slicePlanes);
      return clipper;
      }
    else
      {
      vtkExtractGeometry* clipper = vtkExtractGeometry::New();
      clipper->SetImplicitFunction(slicePlanes);
      clipper->ExtractInsideOff();
      if (this->Internal->ClippingMethod == vtkDMMLClipModelsNode::WholeCellsWithBoundary)
        {
        clipper->ExtractBoundaryCellsOn();
        }
      return clipper;
      }
    }
  else
    {
    if (this->Internal->ClippingMethod == vtkDMMLClipModelsNode::Straight)
      {
      vtkClipPolyData* clipper = vtkClipPolyData::New();
      clipper->SetValue(0.0);
      clipper->SetClipFunction(slicePlanes);
      return clipper;
      }
    else
      {
      vtkExtractPolyDataGeometry* clipper = vtkExtractPolyDataGeometry::New();
      clipper->SetImplicitFunction(slicePlanes);
      clipper->ExtractInsideOff();
      if (this->Internal->ClippingMethod == vtkDMMLClipModelsNode::WholeCellsWithBoundary)
        {
        clipper->ExtractBoundaryCellsOn();
        }
      return clipper;
      }
    }
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayableManager::OnInteractorStyleEvent(int eventid)
{
  bool keyPressed = false;
  char *keySym = this->GetInteractor()->GetKeySym();
  if (keySym && strcmp(keySym, "i") == 0)
    {
    keyPressed = true;
    }

  if (eventid == vtkCommand::LeftButtonPressEvent && keyPressed)
    {
    double x = this->GetInteractor()->GetEventPosition()[0];
    double y = this->GetInteractor()->GetEventPosition()[1];

    double windowWidth = this->GetInteractor()->GetRenderWindow()->GetSize()[0];
    double windowHeight = this->GetInteractor()->GetRenderWindow()->GetSize()[1];

    if (x < windowWidth && y < windowHeight)
      {
      // it's a 3D displayable manager and the click could have been on a node
      double yNew = windowHeight - y - 1;
      vtkDMMLDisplayNode *displayNode = nullptr;

      if (this->Pick(x,yNew) &&
          strcmp(this->GetPickedNodeID(),"") != 0)
        {
        // find the node id, the picked node name is probably the display node
        const char *pickedNodeID = this->GetPickedNodeID();

        vtkDMMLNode *dmmlNode = this->GetDMMLScene()->GetNodeByID(pickedNodeID);
        if (dmmlNode)
          {
          displayNode = vtkDMMLDisplayNode::SafeDownCast(dmmlNode);
          }
        else
          {
          vtkDebugMacro("couldn't find a dmml node with ID " << pickedNodeID);
          }
        }

      if (displayNode)
        {
        displayNode->SetColor(1.0, 0, 0);
        this->GetInteractionNode()->SetCurrentInteractionMode(vtkDMMLInteractionNode::ViewTransform);
        }
      }
    }
  if (keyPressed)
    {
    this->GetInteractor()->SetKeySym(nullptr);
    }

  this->PassThroughInteractorStyleEvent(eventid);

  return;
}
