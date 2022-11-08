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

// Segmentations includes
#include "vtkCjyxSegmentationsModuleLogic.h"

// SegmentationCore includes
#include "vtkBinaryLabelmapToClosedSurfaceConversionRule.h"
#include "vtkClosedSurfaceToBinaryLabelmapConversionRule.h"
#include "vtkClosedSurfaceToFractionalLabelmapConversionRule.h"
#include "vtkFractionalLabelmapToClosedSurfaceConversionRule.h"
#include "vtkOrientedImageData.h"
#include "vtkOrientedImageDataResample.h"
#include "vtkSegmentationConverterFactory.h"
#include <vtkSegmentationModifier.h>

// Terminologies includes
#include "vtkCjyxTerminologiesModuleLogic.h"
#include "vtkCjyxTerminologyEntry.h"

// VTK includes
#include <vtkActor.h>
#include <vtkAppendPolyData.h>
#include <vtkCallbackCommand.h>
#include <vtkDataObject.h>
#include <vtkGeneralTransform.h>
#include <vtkGeometryFilter.h>
#include <vtkImageAccumulate.h>
#include <vtkImageConstantPad.h>
#include <vtkImageMathematics.h>
#include <vtkImageThreshold.h>
#include <vtkLookupTable.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkOBJExporter.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSTLWriter.h>
#include <vtkStringArray.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTriangleFilter.h>
#include <vtkTrivialProducer.h>
#include <vtkUnstructuredGrid.h>
#include <vtksys/SystemTools.hxx>
#include <vtksys/RegularExpression.hxx>

// VTKITK includes
#include <vtkITKImageWriter.h>

// DMML includes
#include <vtkDMMLScene.h>
#include "vtkDMMLSegmentationNode.h"
#include "vtkDMMLSegmentationDisplayNode.h"
#include "vtkDMMLSegmentationStorageNode.h"
#include "vtkDMMLSegmentEditorNode.h"
#include <vtkDMMLSubjectHierarchyNode.h>
#include <vtkDMMLTransformNode.h>
#include <vtkDMMLColorTableNode.h>
#include <vtkDMMLLabelMapVolumeNode.h>
#include <vtkDMMLLabelMapVolumeDisplayNode.h>
#include <vtkDMMLModelDisplayNode.h>
#include <vtkDMMLModelNode.h>
#include <vtkEventBroker.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkCjyxSegmentationsModuleLogic);

//----------------------------------------------------------------------------
vtkCjyxSegmentationsModuleLogic::vtkCjyxSegmentationsModuleLogic()
{
  this->SubjectHierarchyUIDCallbackCommand = vtkCallbackCommand::New();
  this->SubjectHierarchyUIDCallbackCommand->SetClientData( reinterpret_cast<void *>(this) );
  this->SubjectHierarchyUIDCallbackCommand->SetCallback( vtkCjyxSegmentationsModuleLogic::OnSubjectHierarchyUIDAdded );
}

//----------------------------------------------------------------------------
vtkCjyxSegmentationsModuleLogic::~vtkCjyxSegmentationsModuleLogic()
{
  if (this->SubjectHierarchyUIDCallbackCommand)
    {
    this->SubjectHierarchyUIDCallbackCommand->SetClientData(nullptr);
    this->SubjectHierarchyUIDCallbackCommand->Delete();
    this->SubjectHierarchyUIDCallbackCommand = nullptr;
    }
}

//----------------------------------------------------------------------------
void vtkCjyxSegmentationsModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkCjyxSegmentationsModuleLogic::SetDMMLSceneInternal(vtkDMMLScene* newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkDMMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkDMMLScene::NodeRemovedEvent);
  this->SetAndObserveDMMLSceneEvents(newScene, events.GetPointer());

  // Observe subject hierarchy UID events
  vtkDMMLSubjectHierarchyNode* shNode = vtkDMMLSubjectHierarchyNode::GetSubjectHierarchyNode(newScene);
  if (shNode)
    {
    vtkEventBroker::GetInstance()->AddObservation(
      shNode, vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemUIDAddedEvent, this, this->SubjectHierarchyUIDCallbackCommand );
    }
}

//-----------------------------------------------------------------------------
void vtkCjyxSegmentationsModuleLogic::RegisterNodes()
{
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("RegisterNodes: Invalid DMML scene");
    return;
    }

  // vtkDMMLSegmentationNode, vtkDMMLSegmentationDisplayNode, and
  // vtkDMMLSegmentationStorageNode nodes are registered in vtkDMMLScene.
  this->GetDMMLScene()->RegisterNodeClass(vtkSmartPointer<vtkDMMLSegmentEditorNode>::New());

  // Register converter rules
  vtkSegmentationConverterFactory::GetInstance()->RegisterConverterRule(
    vtkSmartPointer<vtkBinaryLabelmapToClosedSurfaceConversionRule>::New() );
  vtkSegmentationConverterFactory::GetInstance()->RegisterConverterRule(
    vtkSmartPointer<vtkClosedSurfaceToBinaryLabelmapConversionRule>::New() );
  vtkSegmentationConverterFactory::GetInstance()->RegisterConverterRule(
    vtkSmartPointer<vtkClosedSurfaceToFractionalLabelmapConversionRule>::New() );
  vtkSegmentationConverterFactory::GetInstance()->RegisterConverterRule(
    vtkSmartPointer<vtkFractionalLabelmapToClosedSurfaceConversionRule>::New() );
}

//---------------------------------------------------------------------------
void vtkCjyxSegmentationsModuleLogic::OnDMMLSceneNodeAdded(vtkDMMLNode* node)
{
  if (!node || !this->GetDMMLScene())
    {
    vtkErrorMacro("OnDMMLSceneNodeAdded: Invalid DMML scene or input node");
    return;
    }

  if (node->IsA("vtkDMMLSubjectHierarchyNode"))
    {
    vtkEventBroker::GetInstance()->AddObservation(
      node, vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemUIDAddedEvent, this, this->SubjectHierarchyUIDCallbackCommand );
    }
}

//---------------------------------------------------------------------------
void vtkCjyxSegmentationsModuleLogic::OnSubjectHierarchyUIDAdded(vtkObject* caller,
                                                                   unsigned long vtkNotUsed(eid),
                                                                   void* clientData,
                                                                   void* callData)
{
  vtkCjyxSegmentationsModuleLogic* self = reinterpret_cast<vtkCjyxSegmentationsModuleLogic*>(clientData);
  vtkDMMLSubjectHierarchyNode* shNode = reinterpret_cast<vtkDMMLSubjectHierarchyNode*>(caller);
  vtkIdType* itemWithNewUID = reinterpret_cast<vtkIdType*>(callData);
  if (!self || !shNode || !itemWithNewUID)
    {
    return;
    }

  // Call callback function in all segmentation nodes. The callback function establishes the right
  // connection between loaded DICOM volumes and segmentations (related to reference image geometry)
  std::vector<vtkDMMLNode*> segmentationNodes;
  unsigned int numberOfNodes = self->GetDMMLScene()->GetNodesByClass("vtkDMMLSegmentationNode", segmentationNodes);
  for (unsigned int nodeIndex=0; nodeIndex<numberOfNodes; nodeIndex++)
    {
    vtkDMMLSegmentationNode* node = vtkDMMLSegmentationNode::SafeDownCast(segmentationNodes[nodeIndex]);
    if (node)
      {
      node->OnSubjectHierarchyUIDAdded(shNode, *itemWithNewUID);
      }
    }
}

//-----------------------------------------------------------------------------
vtkDMMLSegmentationNode* vtkCjyxSegmentationsModuleLogic::GetSegmentationNodeForSegmentation(vtkDMMLScene* scene, vtkSegmentation* segmentation)
{
  if (!scene || !segmentation)
    {
    return nullptr;
    }

  std::vector<vtkDMMLNode*> segmentationNodes;
  unsigned int numberOfNodes = scene->GetNodesByClass("vtkDMMLSegmentationNode", segmentationNodes);
  for (unsigned int nodeIndex=0; nodeIndex<numberOfNodes; nodeIndex++)
    {
    vtkDMMLSegmentationNode* node = vtkDMMLSegmentationNode::SafeDownCast(segmentationNodes[nodeIndex]);
    if (node && node->GetSegmentation() == segmentation)
      {
      return node;
      }
    }

  return nullptr;
}

//-----------------------------------------------------------------------------
vtkDMMLSegmentationNode* vtkCjyxSegmentationsModuleLogic::GetSegmentationNodeForSegment(vtkDMMLScene* scene, vtkSegment* segment, std::string& segmentId)
{
  segmentId = "";
  if (!scene || !segment)
    {
    return nullptr;
    }

  std::vector<vtkDMMLNode*> segmentationNodes;
  unsigned int numberOfNodes = scene->GetNodesByClass("vtkDMMLSegmentationNode", segmentationNodes);
  for (unsigned int nodeIndex=0; nodeIndex<numberOfNodes; nodeIndex++)
    {
    vtkDMMLSegmentationNode* node = vtkDMMLSegmentationNode::SafeDownCast(segmentationNodes[nodeIndex]);
    segmentId = node->GetSegmentation()->GetSegmentIdBySegment(segment);
    if (!segmentId.empty())
      {
      return node;
      }
    }
  return nullptr;
}

//-----------------------------------------------------------------------------
vtkDMMLSegmentationNode* vtkCjyxSegmentationsModuleLogic::LoadSegmentationFromFile(const char* fileName,
  bool autoOpacities/*=true*/, const char* nodeName/*=nullptr*/, vtkDMMLColorTableNode* colorNode/*=nullptr*/)
{
  if (this->GetDMMLScene() == nullptr || fileName == nullptr)
    {
    return nullptr;
    }
  vtkSmartPointer<vtkDMMLSegmentationNode> segmentationNode = vtkSmartPointer<vtkDMMLSegmentationNode>::New();
  vtkSmartPointer<vtkDMMLSegmentationStorageNode> storageNode = vtkSmartPointer<vtkDMMLSegmentationStorageNode>::New();
  storageNode->SetFileName(fileName);

  // Check to see which node can read this type of file
  if (!storageNode->SupportedFileType(fileName))
    {
    vtkErrorMacro("LoadSegmentationFromFile: Segmentation storage node unable to load segmentation file.");
    return nullptr;
    }

  std::string uname;
  if (nodeName && strlen(nodeName)>0)
    {
    uname = nodeName;
    }
  else
    {
    uname = this->GetDMMLScene()->GetUniqueNameByString(storageNode->GetFileNameWithoutExtension(fileName).c_str());
    }

  segmentationNode->SetName(uname.c_str());
  std::string storageUName = uname + "_Storage";
  storageNode->SetName(storageUName.c_str());
  this->GetDMMLScene()->AddNode(storageNode.GetPointer());

  segmentationNode->SetScene(this->GetDMMLScene());
  segmentationNode->SetAndObserveStorageNodeID(storageNode->GetID());
  if (colorNode)
    {
    segmentationNode->SetLabelmapConversionColorTableNodeID(colorNode->GetID());
    }

  this->GetDMMLScene()->AddNode(segmentationNode);

  // Read file
  vtkDebugMacro("LoadSegmentationFromFile: calling read on the storage node");
  int success = storageNode->ReadData(segmentationNode);
  if (success != 1)
    {
    vtkErrorMacro("LoadSegmentationFromFile: Error reading " << fileName);
    this->GetDMMLScene()->RemoveNode(segmentationNode);
    return nullptr;
    }

  // Show closed surface poly data if it exist. By default the preferred representation is shown,
  // but we do not have a display node for the segmentation here. In its absence the master representation
  // is shown if it's poly data, but closed surface model is specifically for 3D visualization)
  if (segmentationNode->GetSegmentation()->ContainsRepresentation(
    vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName()) )
    {
    if (!segmentationNode->GetDisplayNode())
      {
      segmentationNode->CreateDefaultDisplayNodes();
      }
    vtkDMMLSegmentationDisplayNode* displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());
    if (displayNode)
      {
      // If not loading segmentation from a scene (where display information is available),
      // then calculate and set auto-opacity for the displayed poly data for better visualization
      if (autoOpacities)
        {
        displayNode->CalculateAutoOpacitiesForSegments();
        }
      }
    }

  return segmentationNode.GetPointer();
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::CreateLabelmapVolumeFromOrientedImageData(
  vtkOrientedImageData* orientedImageData, vtkDMMLLabelMapVolumeNode* labelmapVolumeNode)
{
  if (!vtkCjyxSegmentationsModuleLogic::CopyOrientedImageDataToVolumeNode(orientedImageData, labelmapVolumeNode, true, true))
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::CreateLabelmapVolumeFromOrientedImageData: failed to copy into volume node");
    return false;
    }

  // Create default display node if it does not have one
  if (labelmapVolumeNode->GetScene())
    {
    vtkSmartPointer<vtkDMMLLabelMapVolumeDisplayNode> labelmapVolumeDisplayNode = vtkDMMLLabelMapVolumeDisplayNode::SafeDownCast(
      labelmapVolumeNode->GetDisplayNode() );
    if (!labelmapVolumeDisplayNode.GetPointer())
      {
      labelmapVolumeDisplayNode = vtkSmartPointer<vtkDMMLLabelMapVolumeDisplayNode>::New();
      labelmapVolumeNode->GetScene()->AddNode(labelmapVolumeDisplayNode);
      labelmapVolumeNode->SetAndObserveDisplayNodeID(labelmapVolumeDisplayNode->GetID());
      labelmapVolumeDisplayNode->SetDefaultColorMap();
      }
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::CopyOrientedImageDataToVolumeNode(
  vtkOrientedImageData* orientedImageData, vtkDMMLVolumeNode* volumeNode, bool shallowCopy /*=true*/, bool shiftImageDataExtentToZeroStart /*=true*/)
{
  if (!orientedImageData)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::CopyOrientedImageDataToVolumeNode: Invalid input image data");
    return false;
    }
  if (!volumeNode)
    {
    vtkErrorWithObjectMacro(orientedImageData, "CopyOrientedImageDataToVolumeNode: Invalid input volume node");
    return false;
    }

  // Create an identity (zero origin, unit spacing, identity orientation) vtkImageData that can be stored in vtkDMMLVolumeNode
  vtkSmartPointer<vtkImageData> identityImageData = vtkSmartPointer<vtkImageData>::New();
  if (shallowCopy)
    {
    identityImageData->ShallowCopy(orientedImageData);
    }
  else
    {
    identityImageData->DeepCopy(orientedImageData);
    }
  identityImageData->SetOrigin(0,0,0);
  identityImageData->SetSpacing(1,1,1);
  volumeNode->SetAndObserveImageData(identityImageData);

  vtkSmartPointer<vtkMatrix4x4> labelmapImageToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  orientedImageData->GetImageToWorldMatrix(labelmapImageToWorldMatrix);
  volumeNode->SetIJKToRASMatrix(labelmapImageToWorldMatrix);

  if (shiftImageDataExtentToZeroStart)
    {
    volumeNode->ShiftImageDataExtentToZeroStart();
    }

  return true;
}

//-----------------------------------------------------------------------------
vtkOrientedImageData* vtkCjyxSegmentationsModuleLogic::CreateOrientedImageDataFromVolumeNode(vtkDMMLScalarVolumeNode* volumeNode, vtkDMMLTransformNode* outputParentTransformNode /* = nullptr */)
{
  if (!volumeNode || !volumeNode->GetImageData())
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::CreateOrientedImageDataFromVolumeNode: Invalid volume node");
    return nullptr;
    }

  vtkOrientedImageData* orientedImageData = vtkOrientedImageData::New();
  orientedImageData->vtkImageData::DeepCopy(volumeNode->GetImageData());

  vtkSmartPointer<vtkMatrix4x4> ijkToRasMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  volumeNode->GetIJKToRASMatrix(ijkToRasMatrix);
  orientedImageData->SetGeometryFromImageToWorldMatrix(ijkToRasMatrix);

  // Apply parent transform of the volume node if any
  if (volumeNode->GetParentTransformNode() != outputParentTransformNode)
    {
    vtkSmartPointer<vtkGeneralTransform> nodeToOutputTransform = vtkSmartPointer<vtkGeneralTransform>::New();
    vtkDMMLTransformNode::GetTransformBetweenNodes(volumeNode->GetParentTransformNode(), outputParentTransformNode, nodeToOutputTransform);
    vtkOrientedImageDataResample::TransformOrientedImage(orientedImageData, nodeToOutputTransform);
    }

  return orientedImageData;
}

//-----------------------------------------------------------------------------
int vtkCjyxSegmentationsModuleLogic::DoesLabelmapContainSingleLabel(vtkDMMLLabelMapVolumeNode* labelmapVolumeNode)
{
  if (!labelmapVolumeNode || !labelmapVolumeNode->GetImageData())
  {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::DoesLabelmapContainSingleLabel: Invalid labelmap volume DMML node");
    return 0;
  }
  int highLabel = (int)(ceil(labelmapVolumeNode->GetImageData()->GetScalarRange()[1]));
  if (highLabel == 0)
  {
    return 0;
  }
  vtkSmartPointer<vtkImageAccumulate> imageAccumulate = vtkSmartPointer<vtkImageAccumulate>::New();
  imageAccumulate->SetInputConnection(labelmapVolumeNode->GetImageDataConnection());
  imageAccumulate->IgnoreZeroOn();
  imageAccumulate->Update();
  int lowLabel = (int)imageAccumulate->GetMin()[0];
  highLabel = (int)imageAccumulate->GetMax()[0];
  if (lowLabel != highLabel)
    {
    return 0;
    }

  return lowLabel;
}

//-----------------------------------------------------------------------------
void vtkCjyxSegmentationsModuleLogic::GetAllLabelValues(vtkIntArray* labels, vtkImageData* labelmap)
{
  if (!labels)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::GetAllLabelValues: Invalid labels");
    return;
    }
  labels->Reset();
  if (!labelmap)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::GetAllLabelValues: Invalid labelmap");
    return;
   }

  int dimensions[3] = { 0 };
  labelmap->GetDimensions(dimensions);
  if (dimensions[0] <= 0 || dimensions[1] <= 0 || dimensions[2] <= 0)
    {
    // Labelmap is empty, there are no label values.
    // Running vtkImageAccumulate would cause a crash.
    return;
    }

  double* scalarRange = labelmap->GetScalarRange();
  int lowLabel = (int)(floor(scalarRange[0]));
  int highLabel = (int)(ceil(scalarRange[1]));
  vtkSmartPointer<vtkImageAccumulate> imageAccumulate = vtkSmartPointer<vtkImageAccumulate>::New();
  imageAccumulate->SetInputData(labelmap);
  imageAccumulate->IgnoreZeroOn(); // Do not create segment from background
  imageAccumulate->SetComponentExtent(lowLabel, highLabel, 0, 0, 0, 0);
  imageAccumulate->SetComponentOrigin(0, 0, 0);
  imageAccumulate->SetComponentSpacing(1, 1, 1);
  imageAccumulate->Update();

  for (int label = lowLabel; label <= highLabel; ++label)
    {
    if (label == 0)
      {
      continue;
      }
    double frequency = imageAccumulate->GetOutput()->GetPointData()->GetScalars()->GetTuple1(label - lowLabel);
    if (frequency == 0.0)
      {
      continue;
      }
    labels->InsertNextValue(label);
    }
}

//-----------------------------------------------------------------------------
vtkSegment* vtkCjyxSegmentationsModuleLogic::CreateSegmentFromLabelmapVolumeNode(vtkDMMLLabelMapVolumeNode* labelmapVolumeNode, vtkDMMLSegmentationNode* segmentationNode/*=nullptr*/)
{
  if (!labelmapVolumeNode)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::CreateSegmentFromLabelmapVolumeNode: Invalid labelmap volume DMML node");
    return nullptr;
    }

  // Cannot create single segment from labelmap node if it contains more than one segment
  int label = vtkCjyxSegmentationsModuleLogic::DoesLabelmapContainSingleLabel(labelmapVolumeNode);
  if (!label)
    {
    vtkErrorWithObjectMacro(labelmapVolumeNode, "CreateSegmentFromLabelmapVolumeNode: Unable to create single segment from labelmap volume node, as labelmap contains more than one label");
    return nullptr;
    }

  // Create segment
  vtkSegment* segment = vtkSegment::New();
  segment->SetName(labelmapVolumeNode->GetName());

  // Set segment color
  double color[4] = { vtkSegment::SEGMENT_COLOR_INVALID[0],
                      vtkSegment::SEGMENT_COLOR_INVALID[1],
                      vtkSegment::SEGMENT_COLOR_INVALID[2], 1.0 };
  vtkDMMLColorTableNode* colorNode = nullptr;
  if (labelmapVolumeNode->GetDisplayNode())
    {
    colorNode = vtkDMMLColorTableNode::SafeDownCast(labelmapVolumeNode->GetDisplayNode()->GetColorNode());
    if (colorNode)
      {
      colorNode->GetColor(label, color);
      }
    }
  segment->SetColor(color[0], color[1], color[2]);

  // Create oriented image data from labelmap
  vtkSmartPointer<vtkOrientedImageData> orientedImageData = vtkSmartPointer<vtkOrientedImageData>::Take(
    vtkCjyxSegmentationsModuleLogic::CreateOrientedImageDataFromVolumeNode(labelmapVolumeNode) );

  // Apply parent transforms if any
  if (labelmapVolumeNode->GetParentTransformNode() || (segmentationNode && segmentationNode->GetParentTransformNode()))
    {
    vtkSmartPointer<vtkGeneralTransform> labelmapToSegmentationTransform = vtkSmartPointer<vtkGeneralTransform>::New();
    if (segmentationNode)
      {
      vtkCjyxSegmentationsModuleLogic::GetTransformBetweenRepresentationAndSegmentation(labelmapVolumeNode, segmentationNode, labelmapToSegmentationTransform);
      }
    else
      {
      // Get parent transform which is the representation to world transform in absence of a segmentation node
      vtkDMMLTransformNode* representationParentTransformNode = labelmapVolumeNode->GetParentTransformNode();
      if (representationParentTransformNode)
        {
        representationParentTransformNode->GetTransformToWorld(labelmapToSegmentationTransform);
        }
      }
    vtkOrientedImageDataResample::TransformOrientedImage(orientedImageData, labelmapToSegmentationTransform);
    }

  // Add oriented image data as binary labelmap representation
  segment->AddRepresentation(
    vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName(),
    orientedImageData );

  return segment;
}

//-----------------------------------------------------------------------------
vtkSegment* vtkCjyxSegmentationsModuleLogic::CreateSegmentFromModelNode(vtkDMMLModelNode* modelNode, vtkDMMLSegmentationNode* segmentationNode/*=nullptr*/)
{
  if (!modelNode)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::CreateSegmentFromModelNode: Invalid model DMML node");
    return nullptr;
    }
  vtkSmartPointer<vtkPolyData> inputPolyData;
  if (modelNode->GetPolyData())
    {
    inputPolyData = modelNode->GetPolyData();
    }
  else if (modelNode->GetUnstructuredGrid())
    {
    vtkNew<vtkGeometryFilter> extractSurface;
    extractSurface->SetInputData(modelNode->GetUnstructuredGrid());
    extractSurface->Update();
    inputPolyData = extractSurface->GetOutput();
    }
  else
    {
    vtkErrorWithObjectMacro(modelNode, "CreateSegmentFromModelNode: Model node does not contain poly or unstructured grid");
    return nullptr;
    }

  double color[3] = { vtkSegment::SEGMENT_COLOR_INVALID[0],
                      vtkSegment::SEGMENT_COLOR_INVALID[1],
                      vtkSegment::SEGMENT_COLOR_INVALID[2] };

  // Create oriented image data from labelmap volume node
  vtkSegment* segment = vtkSegment::New();
    segment->SetName(modelNode->GetName());

  // Color from display node
  vtkDMMLDisplayNode* modelDisplayNode = modelNode->GetDisplayNode();
  if (modelDisplayNode)
    {
    modelDisplayNode->GetColor(color);
    segment->SetColor(color);
    }

  // Make a copy of the model's poly data to set it in the segment
  vtkSmartPointer<vtkPolyData> polyDataCopy = vtkSmartPointer<vtkPolyData>::New();

  // Apply parent transforms if any
  if (modelNode->GetParentTransformNode() || (segmentationNode && segmentationNode->GetParentTransformNode()))
    {
    vtkSmartPointer<vtkGeneralTransform> modelToSegmentationTransform = vtkSmartPointer<vtkGeneralTransform>::New();
    if (segmentationNode)
      {
      vtkCjyxSegmentationsModuleLogic::GetTransformBetweenRepresentationAndSegmentation(modelNode, segmentationNode, modelToSegmentationTransform);
      }
    else
      {
      // Get parent transform which is the representation to world transform in absence of a segmentation node
      vtkDMMLTransformNode* representationParentTransformNode = modelNode->GetParentTransformNode();
      if (representationParentTransformNode)
        {
        representationParentTransformNode->GetTransformToWorld(modelToSegmentationTransform);
        }
      }

    vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformFilter->SetInputData(inputPolyData);
    transformFilter->SetTransform(modelToSegmentationTransform);
    transformFilter->Update();
    polyDataCopy->DeepCopy(transformFilter->GetOutput());
    }
  else
    {
    polyDataCopy->DeepCopy(inputPolyData);
    }

  // Add model poly data as closed surface representation
  segment->AddRepresentation(
    vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName(),
    polyDataCopy);

  return segment;
}

//-----------------------------------------------------------------------------
vtkDMMLSegmentationNode* vtkCjyxSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem(
  vtkIdType segmentShItemID, vtkDMMLScene* scene )
{
  if (!scene)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem: Invalid DMML scene");
    return nullptr;
    }
  if (segmentShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    vtkErrorWithObjectMacro(scene, "vtkCjyxSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem: Invalid subject hierarchy item");
    return nullptr;
    }

  vtkDMMLSubjectHierarchyNode* shNode = vtkDMMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene);
  if (!shNode)
    {
    vtkErrorWithObjectMacro(scene, "vtkCjyxSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem: Failed to access subject hierarchy");
    return nullptr;
    }

  vtkDMMLSegmentationNode* segmentationNode = vtkDMMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(segmentShItemID));
  if (segmentationNode)
    {
    // a segmentation is selected
    return segmentationNode;
    }

  // If a segment is selected then the parent is the segmentation node.
  vtkIdType parentShItem = shNode->GetItemParent(segmentShItemID);
  if (parentShItem == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    vtkErrorWithObjectMacro(scene, "vtkCjyxSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem:"
      << " Segment subject hierarchy item has no segmentation parent");
    return nullptr;
    }
  segmentationNode = vtkDMMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(parentShItem) );
  if (!segmentationNode)
    {
    vtkErrorWithObjectMacro(scene, "vtkCjyxSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem:"
      << " Segment subject hierarchy item's parent has no associated segmentation node");
    return nullptr;
    }

  return segmentationNode;
}

//-----------------------------------------------------------------------------
vtkSegment* vtkCjyxSegmentationsModuleLogic::GetSegmentForSegmentSubjectHierarchyItem(vtkIdType segmentShItemID, vtkDMMLScene* scene)
{
  if (!scene)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::GetSegmentForSegmentSubjectHierarchyItem: Invalid DMML scene");
    return nullptr;
    }
  vtkDMMLSubjectHierarchyNode* shNode = vtkDMMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene);
  if (!shNode)
    {
    vtkErrorWithObjectMacro(scene, "vtkCjyxSegmentationsModuleLogic::GetSegmentForSegmentSubjectHierarchyItem: Failed to access subject hierarchy");
    return nullptr;
    }

  vtkDMMLSegmentationNode* segmentationNode =
    vtkCjyxSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem(segmentShItemID, scene);
  if (!segmentationNode)
    {
    return nullptr;
    }

  std::string segmentId = shNode->GetItemAttribute(segmentShItemID, vtkDMMLSegmentationNode::GetSegmentIDAttributeName());
  if (segmentId.empty())
    {
    vtkErrorWithObjectMacro(scene, "vtkCjyxSegmentationsModuleLogic::GetSegmentForSegmentSubjectHierarchyItem:"
      << " Segment subject hierarchy item " << shNode->GetItemName(segmentShItemID) << " does not contain segment ID");
    return nullptr;
    }

  vtkSegment* segment = segmentationNode->GetSegmentation()->GetSegment(segmentId);
  if (!segment)
    {
    vtkErrorWithObjectMacro(scene, "vtkCjyxSegmentationsModuleLogic::GetSegmentForSegmentSubjectHierarchyItem: "
      "Segmentation does not contain segment with given ID: " << (segmentId.empty() ? "(empty)" : segmentId.c_str()));
    }

  return segment;
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::ExportSegmentToRepresentationNode(vtkSegment* segment, vtkDMMLNode* representationNode)
{
  if (!segment)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::ExportSegmentToRepresentationNode: Invalid segment");
    return false;
    }
  if (!representationNode)
    {
    vtkErrorWithObjectMacro(segment, "ExportSegmentToRepresentationNode: Invalid representation DMML node");
    return false;
    }
  vtkDMMLLabelMapVolumeNode* labelmapNode = vtkDMMLLabelMapVolumeNode::SafeDownCast(representationNode);
  vtkDMMLModelNode* modelNode = vtkDMMLModelNode::SafeDownCast(representationNode);
  if (!labelmapNode && !modelNode)
    {
    vtkErrorWithObjectMacro(representationNode, "ExportSegmentToRepresentationNode: Representation DMML node should be either labelmap volume node or model node");
    return false;
    }

  // Determine segment ID and set it as name of the representation node if found
  std::string segmentId("");
  vtkDMMLSegmentationNode* segmentationNode = vtkCjyxSegmentationsModuleLogic::GetSegmentationNodeForSegment(
    representationNode->GetScene(), segment, segmentId);
  vtkDMMLTransformNode* parentTransformNode = nullptr;
  if (segmentationNode)
    {
    representationNode->SetName(segment->GetName());
    parentTransformNode = segmentationNode->GetParentTransformNode();
    }

  if (labelmapNode)
    {
    // Make sure binary labelmap representation exists in segment
    bool binaryLabelmapPresent = segment->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
    if (!binaryLabelmapPresent && !segmentationNode)
      {
      vtkErrorWithObjectMacro(representationNode, "ExportSegmentToRepresentationNode: Segment does not contain binary labelmap representation and cannot convert, because it is not in a segmentation");
      return false;
      }
    binaryLabelmapPresent = segmentationNode->GetSegmentation()->CreateRepresentation(
      vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
    if (!binaryLabelmapPresent)
      {
      vtkErrorWithObjectMacro(representationNode, "ExportSegmentToRepresentationNode: Unable to convert segment to binary labelmap representation");
      return false;
      }

    // Export binary labelmap representation into labelmap volume node
    vtkNew<vtkOrientedImageData> orientedImageData;
    segmentationNode->GetBinaryLabelmapRepresentation(segmentId, orientedImageData);
    bool success = vtkCjyxSegmentationsModuleLogic::CreateLabelmapVolumeFromOrientedImageData(orientedImageData, labelmapNode);
    if (!success)
      {
      return false;
      }

    // Set segmentation's parent transform to exported node
    if (parentTransformNode)
      {
      labelmapNode->SetAndObserveTransformNodeID(parentTransformNode->GetID());
      }

    return true;
    }
  else if (modelNode)
    {
    // Make sure closed surface representation exists in segment
    bool closedSurfacePresent = segmentationNode->GetSegmentation()->CreateRepresentation(
      vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
    if (!closedSurfacePresent)
      {
      vtkErrorWithObjectMacro(representationNode, "ExportSegmentToRepresentationNode: Unable to convert segment to closed surface representation");
      return false;
      }

    // Export closed surface representation into model node
    vtkPolyData* polyData = vtkPolyData::SafeDownCast(
      segment->GetRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName()) );
    vtkSmartPointer<vtkPolyData> polyDataCopy = vtkSmartPointer<vtkPolyData>::New();
    polyDataCopy->DeepCopy(polyData); // Make copy of poly data so that the model node does not change if segment changes
    modelNode->SetAndObservePolyData(polyDataCopy);

    // Set color of the exported model
    vtkDMMLSegmentationDisplayNode* segmentationDisplayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());
    vtkDMMLDisplayNode* modelDisplayNode = modelNode->GetDisplayNode();
    if (!modelDisplayNode)
      {
      // Create display node
      vtkSmartPointer<vtkDMMLModelDisplayNode> displayNode = vtkSmartPointer<vtkDMMLModelDisplayNode>::New();
      displayNode = vtkDMMLModelDisplayNode::SafeDownCast(modelNode->GetScene()->AddNode(displayNode));
      displayNode->VisibilityOn();
      modelNode->SetAndObserveDisplayNodeID(displayNode->GetID());
      modelDisplayNode = displayNode.GetPointer();
      }
    if (segmentationDisplayNode && modelDisplayNode)
      {
      modelDisplayNode->SetColor(segment->GetColor());
      modelDisplayNode->SetOpacity(segmentationDisplayNode->GetSegmentOpacity3D(segmentId));
      }

    // Set segmentation's parent transform to exported node
    if (parentTransformNode)
      {
      modelNode->SetAndObserveTransformNodeID(parentTransformNode->GetID());
      }
      else
      {
      modelNode->SetAndObserveTransformNodeID(nullptr);
      }

    return true;
    }

  // Representation node is neither labelmap, nor model
  return false;
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::ExportSegmentsToModels(vtkDMMLSegmentationNode* segmentationNode,
  const std::vector<std::string>& segmentIDs, vtkIdType folderItemId)
{
  if (!segmentationNode || !segmentationNode->GetScene())
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::ExportSegmentsToModels: Invalid segmentation node");
    return false;
    }
  vtkDMMLScene* scene = segmentationNode->GetScene();
  vtkDMMLSubjectHierarchyNode* shNode = vtkDMMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene);
  if (!shNode)
    {
    vtkErrorWithObjectMacro(segmentationNode, "vtkCjyxSegmentationsModuleLogic::ExportSegmentsToModels: Failed to access subject hierarchy");
    return false;
    }

  // Make sure closed surface representation exists in segment
  bool closedSurfacePresent = segmentationNode->GetSegmentation()->CreateRepresentation(
    vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName() );
  if (!closedSurfacePresent)
    {
    vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsToModels: Unable to convert segment to closed surface representation");
    return false;
    }

  // Create a map that can be used for quickly looking up existing models in the hierarchy
  std::map< std::string, vtkDMMLModelNode* > existingModelNamesToModels;
  std::vector<vtkIdType> childItemIDs;
  shNode->GetItemChildren(folderItemId, childItemIDs);
  for (std::vector<vtkIdType>::iterator itemIt=childItemIDs.begin(); itemIt!=childItemIDs.end(); ++itemIt)
    {
    vtkDMMLModelNode* modelNode = vtkDMMLModelNode::SafeDownCast(
      shNode->GetItemDataNode(*itemIt) );
    if (!modelNode)
      {
      continue;
      }
    existingModelNamesToModels[modelNode->GetName()] = modelNode;
    }

  std::vector<std::string> exportedSegmentIDs;
  if (segmentIDs.empty())
    {
    segmentationNode->GetSegmentation()->GetSegmentIDs(exportedSegmentIDs);
    }
  else
    {
    exportedSegmentIDs = segmentIDs;
    }

  // Export each segment into a model
  for (std::vector<std::string>::iterator segmentIdIt = exportedSegmentIDs.begin(); segmentIdIt != exportedSegmentIDs.end(); ++segmentIdIt)
    {
    // Export segment into model node
    vtkSegment* segment = segmentationNode->GetSegmentation()->GetSegment(*segmentIdIt);
    vtkDMMLModelNode* modelNode = nullptr;
    if (existingModelNamesToModels.find(segment->GetName()) != existingModelNamesToModels.end())
      {
      // Model by the same name exists in the selected hierarchy, overwrite that model
      modelNode = existingModelNamesToModels[segment->GetName()];
      }
    else
      {
      // Create new model node
      vtkNew<vtkDMMLModelNode> newModelNode;
      scene->AddNode(newModelNode.GetPointer());
      newModelNode->CreateDefaultDisplayNodes();
      modelNode = newModelNode.GetPointer();
      // Add to folder
      shNode->SetItemParent( shNode->GetItemByDataNode(newModelNode), folderItemId );
      }

    // Export segment into model node
    if (!vtkCjyxSegmentationsModuleLogic::ExportSegmentToRepresentationNode(segment, modelNode))
      {
      vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsToModels: Failed to export segmentation into model hierarchy");
      return false;
      }
    }

  // Move exported representation under same parent as segmentation
  shNode->SetItemParent( folderItemId,
    shNode->GetItemParent(shNode->GetItemByDataNode(segmentationNode)) );

  return true;
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::ExportSegmentsToModels(
  vtkDMMLSegmentationNode* segmentationNode, vtkStringArray* segmentIds, vtkIdType folderItemId)
{
  std::vector<std::string> segmentIdsVector;
  if (segmentIds == nullptr)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::ExportSegmentsToModels failed: invalid segmentIDs");
    return false;
    }
  for (int segmentIndex = 0; segmentIndex < segmentIds->GetNumberOfValues(); ++segmentIndex)
    {
    segmentIdsVector.push_back(segmentIds->GetValue(segmentIndex));
    }
  return vtkCjyxSegmentationsModuleLogic::ExportSegmentsToModels(segmentationNode, segmentIdsVector, folderItemId);
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::ExportVisibleSegmentsToModels(vtkDMMLSegmentationNode* segmentationNode, vtkIdType folderItemId)
{
  if (!segmentationNode)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::ExportVisibleSegmentsToModels: Invalid segmentation node");
    return false;
    }

  std::vector<std::string> visibleSegmentIDs;
  vtkDMMLSegmentationDisplayNode* displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());
  displayNode->GetVisibleSegmentIDs(visibleSegmentIDs);

  return vtkCjyxSegmentationsModuleLogic::ExportSegmentsToModels(segmentationNode, visibleSegmentIDs, folderItemId);
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::ExportAllSegmentsToModels(vtkDMMLSegmentationNode* segmentationNode, vtkIdType folderItemId)
{
  std::vector<std::string> segmentIDs;
  return vtkCjyxSegmentationsModuleLogic::ExportSegmentsToModels(segmentationNode, segmentIDs, folderItemId);
}

//-----------------------------------------------------------------------------
void vtkCjyxSegmentationsModuleLogic::GenerateMergedLabelmapInReferenceGeometry(vtkDMMLSegmentationNode* segmentationNode,
  vtkDMMLVolumeNode* referenceVolumeNode, vtkStringArray* segmentIDs, int extentComputationMode, vtkOrientedImageData* mergedLabelmap_Reference,
  vtkIntArray* labelValues/*=nullptr*/)
{
  // Get reference geometry in the segmentation node's coordinate system
  vtkSmartPointer<vtkOrientedImageData> referenceGeometry_Reference; // reference geometry in reference node coordinate system
  vtkSmartPointer<vtkOrientedImageData> referenceGeometry_Segmentation;
  vtkSmartPointer<vtkGeneralTransform> referenceGeometryToSegmentationTransform;
  if (referenceVolumeNode && referenceVolumeNode->GetImageData())
    {
    // Create (non-allocated) image data that matches reference geometry
    referenceGeometry_Reference = vtkSmartPointer<vtkOrientedImageData>::New();
    referenceGeometry_Reference->SetExtent(referenceVolumeNode->GetImageData()->GetExtent());
    vtkSmartPointer<vtkMatrix4x4> ijkToRasMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    referenceVolumeNode->GetIJKToRASMatrix(ijkToRasMatrix);
    referenceGeometry_Reference->SetGeometryFromImageToWorldMatrix(ijkToRasMatrix);

    // Transform it to the segmentation node coordinate system
    referenceGeometry_Segmentation = vtkSmartPointer<vtkOrientedImageData>::New();
    referenceGeometry_Segmentation->DeepCopy(referenceGeometry_Reference);

    // Get transform between reference volume and segmentation node
    if (referenceVolumeNode->GetParentTransformNode() != segmentationNode->GetParentTransformNode())
      {
      referenceGeometryToSegmentationTransform = vtkSmartPointer<vtkGeneralTransform>::New();
      vtkDMMLTransformNode::GetTransformBetweenNodes(referenceVolumeNode->GetParentTransformNode(),
        segmentationNode->GetParentTransformNode(), referenceGeometryToSegmentationTransform);
      vtkOrientedImageDataResample::TransformOrientedImage(referenceGeometry_Segmentation, referenceGeometryToSegmentationTransform, true /* geometry only */);
      }
    }

  // Generate shared labelmap for the exported segments in segmentation coordinates
  vtkSmartPointer<vtkOrientedImageData> sharedImage_Segmentation = vtkSmartPointer<vtkOrientedImageData>::New();
  if (!segmentationNode->GenerateMergedLabelmapForAllSegments(sharedImage_Segmentation, extentComputationMode,
    referenceGeometry_Segmentation, segmentIDs, labelValues))
    {
    vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsToLabelmapNode: Failed to generate shared labelmap");
    return;
    }

  // Transform shared labelmap to reference geometry coordinate system
  if (referenceGeometryToSegmentationTransform)
    {
    vtkAbstractTransform* segmentationToReferenceGeometryTransform = referenceGeometryToSegmentationTransform->GetInverse();
    segmentationToReferenceGeometryTransform->Update();
    vtkOrientedImageDataResample::ResampleOrientedImageToReferenceOrientedImage(sharedImage_Segmentation, referenceGeometry_Reference, mergedLabelmap_Reference,
      false /* nearest neighbor interpolation*/, false /* no padding */, segmentationToReferenceGeometryTransform);
    }
  else
    {
    mergedLabelmap_Reference->DeepCopy(sharedImage_Segmentation);
    }
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::ExportSegmentsToLabelmapNode(vtkDMMLSegmentationNode* segmentationNode,
  const std::vector<std::string> &segmentIDs, vtkDMMLLabelMapVolumeNode* labelmapNode, vtkDMMLVolumeNode* referenceVolumeNode /*=nullptr*/,
  int extentComputationMode /*=vtkSegmentation::EXTENT_UNION_OF_EFFECTIVE_SEGMENTS*/, vtkDMMLColorTableNode* exportColorTable/*=nullptr*/)
{
  if (!segmentationNode)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::ExportSegmentsToLabelmapNode: Invalid segmentation node");
    return false;
    }
  if (!labelmapNode)
    {
    vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsToLabelmapNode: Invalid labelmap volume node");
    return false;
    }

  // Make sure binary labelmap representation exists in segment
  bool binaryLabelmapPresent = segmentationNode->GetSegmentation()->CreateRepresentation(
    vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
  if (!binaryLabelmapPresent)
    {
    vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsToLabelmapNode: Unable to convert segment to binary labelmap representation");
    return false;
    }

  // Use reference volume's parent transform if available, otherwise put under the same transform as segmentation node
  vtkDMMLTransformNode* parentTransformNode = nullptr;
  if (referenceVolumeNode)
    {
    parentTransformNode = referenceVolumeNode->GetParentTransformNode();
    }
  else
    {
    parentTransformNode = segmentationNode->GetParentTransformNode();
    }
  labelmapNode->SetAndObserveTransformNodeID(parentTransformNode ? parentTransformNode->GetID() : "");

  std::vector<std::string> exportedSegmentIDs;
  if (segmentIDs.empty())
    {
    segmentationNode->GetSegmentation()->GetSegmentIDs(exportedSegmentIDs);
    }
  else
    {
    exportedSegmentIDs = segmentIDs;
    }

  vtkNew<vtkOrientedImageData> mergedLabelmap_Reference;
  vtkNew<vtkStringArray> segmentIDsArray;
  for (std::string segmentID : exportedSegmentIDs)
    {
    segmentIDsArray->InsertNextValue(segmentID);
    }
  vtkSmartPointer<vtkIntArray> labelValues = nullptr;
  if (exportColorTable)
    {
    labelValues = vtkSmartPointer<vtkIntArray>::New();
    vtkCjyxSegmentationsModuleLogic::GetLabelValuesFromColorNode(segmentationNode, exportColorTable, segmentIDsArray, labelValues);
    }
  vtkCjyxSegmentationsModuleLogic::GenerateMergedLabelmapInReferenceGeometry(segmentationNode, referenceVolumeNode,
    segmentIDsArray, extentComputationMode, mergedLabelmap_Reference, labelValues);

  // Export shared labelmap to the output node
  if (!vtkCjyxSegmentationsModuleLogic::CreateLabelmapVolumeFromOrientedImageData(mergedLabelmap_Reference, labelmapNode))
    {
    vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsToLabelmapNode: Failed to create labelmap from shared segments image");
    return false;
    }

  // Create/update color table to labelmap so that the labels appear in the same color
  if (!labelmapNode->GetDisplayNode())
    {
    labelmapNode->CreateDefaultDisplayNodes();
    }
  if (!segmentationNode->GetDisplayNode())
    {
    segmentationNode->CreateDefaultDisplayNodes();
    }

  if (exportColorTable)
    {
    labelmapNode->GetDisplayNode()->SetAndObserveColorNodeID(exportColorTable->GetID());
    }
  else if (!labelmapNode->GetDisplayNode()->GetColorNode() || labelmapNode->GetDisplayNode()->GetColorNode()->GetType() != vtkDMMLColorNode::User)
    {
    // Create new color table node if labelmap node doesn't have a color node or if the existing one is not user type
    vtkSmartPointer<vtkDMMLColorTableNode> newColorTable = vtkSmartPointer<vtkDMMLColorTableNode>::New();
    // Need to make the color table node visible because only non-hidden storable nodes are offered to be saved
    newColorTable->SetHideFromEditors(false);
    std::string colorTableNodeName(labelmapNode->GetName());
    colorTableNodeName.append("_ColorTable");
    newColorTable->SetName(colorTableNodeName.c_str());
    newColorTable->SetTypeToUser();
    newColorTable->NamesInitialisedOn();
    newColorTable->SetAttribute("Category", "Segmentations");
    // Add an item to the color table, otherwise we get a warning
    // when we set it in the display node.
    newColorTable->SetNumberOfColors(1);
    newColorTable->GetLookupTable()->SetRange(0, 0);
    newColorTable->GetLookupTable()->SetNumberOfTableValues(1);
    // Use NoName as color name to not list the "background" color in the color legend.
    newColorTable->SetColor(0, newColorTable->GetNoName(), 0.0, 0.0, 0.0, 0.0);
    labelmapNode->GetScene()->AddNode(newColorTable);
    labelmapNode->GetDisplayNode()->SetAndObserveColorNodeID(newColorTable->GetID());
    }

  // Copy segment colors to color table node
  vtkDMMLColorTableNode* colorTableNode = vtkDMMLColorTableNode::SafeDownCast(
    labelmapNode->GetDisplayNode()->GetColorNode() ); // Always valid, as it was created just above if was missing
  vtkDMMLSegmentationDisplayNode* displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(
    segmentationNode->GetDisplayNode() );

  int numberOfColors = exportedSegmentIDs.size() + 1;
  if (labelValues)
    {
    for (int i = 0; i < static_cast<int>(exportedSegmentIDs.size()); ++i)
      {
      numberOfColors = std::max(numberOfColors, labelValues->GetValue(i) + 1);
      }
    }

  int colorFillStartIndex = 1;
  if (exportColorTable)
    {
    // If we are using an export color table, we don't want to overwrite the existing values in the table,
    // even if they are not used in the segmentation currently.
    colorFillStartIndex = exportColorTable->GetNumberOfColors();
    }
  colorTableNode->SetNumberOfColors(numberOfColors);
  colorTableNode->GetLookupTable()->SetRange(0, numberOfColors - 1);
  colorTableNode->GetLookupTable()->SetNumberOfTableValues(numberOfColors);
  // Use NoName as color name to not list the "background" color in the color legend.
  colorTableNode->SetColor(0, colorTableNode->GetNoName(), 0.0, 0.0, 0.0, 0.0);

  for (int i = colorFillStartIndex; i < colorTableNode->GetNumberOfColors(); ++i)
    {
    // Fill color table with none
    colorTableNode->SetColor(i, "(none)", 0.0, 0.0, 0.0);
    }

  short colorIndex = 0;
  for (std::vector<std::string>::iterator segmentIt = exportedSegmentIDs.begin(); segmentIt != exportedSegmentIDs.end(); ++segmentIt, ++colorIndex)
    {
    int labelValue = colorIndex + 1;
    if (labelValues)
      {
      labelValue = labelValues->GetValue(colorIndex);
      }
    vtkSegment* segment = segmentationNode->GetSegmentation()->GetSegment(*segmentIt);
    if (!segment)
      {
      vtkWarningWithObjectMacro(segmentationNode, "ExportSegmentsToLabelmapNode: failed to set color table entry, could not find segment by ID " << *segmentIt);
      colorTableNode->SetColor(labelValue, "(none)", 0.0, 0.0, 0.0);
      continue;
      }
    const char* segmentName = segment->GetName();
    vtkVector3d color = displayNode->GetSegmentColor(*segmentIt);
    colorTableNode->SetColor(labelValue, segmentName, color.GetX(), color.GetY(), color.GetZ());
    }

  // Move exported labelmap node under same parent as segmentation
  vtkDMMLSubjectHierarchyNode* shNode = vtkDMMLSubjectHierarchyNode::GetSubjectHierarchyNode(segmentationNode->GetScene());
  if (shNode)
    {
    shNode->SetItemParent(shNode->GetItemByDataNode(labelmapNode),
      shNode->GetItemParent(shNode->GetItemByDataNode(segmentationNode)) );
    }
  else
    {
    vtkWarningWithObjectMacro(segmentationNode, "ExportSegmentsToLabelmapNode: Failed to access subject hierarchy node");
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::ExportSegmentsToLabelmapNode(vtkDMMLSegmentationNode* segmentationNode,
  vtkStringArray* segmentIds, vtkDMMLLabelMapVolumeNode* labelmapNode, vtkDMMLVolumeNode* referenceVolumeNode /*=nullptr*/,
  int extentComputationMode /*=vtkSegmentation::EXTENT_UNION_OF_EFFECTIVE_SEGMENTS*/, vtkDMMLColorTableNode* exportColorTable/*=nullptr*/)
{
  std::vector<std::string> segmentIdsVector;
  if (segmentIds == nullptr)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::ExportSegmentsToLabelmapNode failed: invalid segmentIDs");
    return false;
    }
  for (int segmentIndex = 0; segmentIndex < segmentIds->GetNumberOfValues(); ++segmentIndex)
    {
    segmentIdsVector.push_back(segmentIds->GetValue(segmentIndex));
    }
  return vtkCjyxSegmentationsModuleLogic::ExportSegmentsToLabelmapNode(segmentationNode, segmentIdsVector, labelmapNode,
    referenceVolumeNode, extentComputationMode, exportColorTable);
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::ExportVisibleSegmentsToLabelmapNode(vtkDMMLSegmentationNode* segmentationNode,
  vtkDMMLLabelMapVolumeNode* labelmapNode, vtkDMMLVolumeNode* referenceVolumeNode /*=nullptr*/,
  int extentComputationMode /*=vtkSegmentation::EXTENT_UNION_OF_EFFECTIVE_SEGMENTS*/)
{
  if (!segmentationNode)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::ExportVisibleSegmentsToLabelmapNode failed: Invalid segmentation node");
    return false;
    }

  std::vector<std::string> visibleSegmentIDs;
  vtkDMMLSegmentationDisplayNode* displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());
  if (displayNode)
    {
    displayNode->GetVisibleSegmentIDs(visibleSegmentIDs);
    }
  else
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::ExportVisibleSegmentsToLabelmapNode: display node not found, exporting all segments.");
    }

  return vtkCjyxSegmentationsModuleLogic::ExportSegmentsToLabelmapNode(segmentationNode, visibleSegmentIDs, labelmapNode,
    referenceVolumeNode, extentComputationMode);
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::ExportAllSegmentsToLabelmapNode(vtkDMMLSegmentationNode* segmentationNode, vtkDMMLLabelMapVolumeNode* labelmapNode,
  int extentComputationMode /*=vtkSegmentation::EXTENT_UNION_OF_EFFECTIVE_SEGMENTS*/)
{
  std::vector<std::string> segmentIDs;
  return vtkCjyxSegmentationsModuleLogic::ExportSegmentsToLabelmapNode(segmentationNode, segmentIDs, labelmapNode, nullptr, extentComputationMode);
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::ImportModelToSegmentationNode(vtkDMMLModelNode* modelNode,
  vtkDMMLSegmentationNode* segmentationNode, std::string insertBeforeSegmentId/*=""*/)
{
  if (!segmentationNode)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::ImportModelToSegmentationNode: Invalid segmentation node");
    return false;
    }
  if (!modelNode || !modelNode->GetMesh())
    {
    vtkErrorWithObjectMacro(segmentationNode, "ImportModelToSegmentationNode: Invalid model node");
    return false;
    }
  vtkSmartPointer<vtkSegment> segment = vtkSmartPointer<vtkSegment>::Take(
    vtkCjyxSegmentationsModuleLogic::CreateSegmentFromModelNode(modelNode, segmentationNode));
  if (!segment.GetPointer())
    {
    return false;
    }

  if (!segmentationNode->GetDisplayNode())
    {
    segmentationNode->CreateDefaultDisplayNodes();
    }

  // Add segment to current segmentation
  if (!segmentationNode->GetSegmentation()->AddSegment(segment, "", insertBeforeSegmentId))
    {
    vtkErrorWithObjectMacro(segmentationNode, "vtkCjyxSegmentationsModuleLogic: Failed to add segment to segmentation");
    return false;
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::ImportModelsToSegmentationNode(vtkIdType folderItemId,
  vtkDMMLSegmentationNode* segmentationNode, std::string vtkNotUsed(insertBeforeSegmentId)/*=""*/)
{
  if (!segmentationNode || !segmentationNode->GetScene())
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::ImportModelsToSegmentationNode: Invalid segmentation node");
    return false;
    }
  vtkDMMLSubjectHierarchyNode* shNode = vtkDMMLSubjectHierarchyNode::GetSubjectHierarchyNode(segmentationNode->GetScene());
  if (!shNode)
    {
      vtkErrorWithObjectMacro(segmentationNode, "vtkCjyxSegmentationsModuleLogic::ImportModelsToSegmentationNode: Failed to access subject hierarchy");
      return false;
      }

  // Get model nodes
  bool returnValue = true;
  std::vector<vtkIdType> childItemIDs;
  shNode->GetItemChildren(folderItemId, childItemIDs);
  for (std::vector<vtkIdType>::iterator itemIt=childItemIDs.begin(); itemIt!=childItemIDs.end(); ++itemIt)
    {
    vtkDMMLModelNode* modelNode = vtkDMMLModelNode::SafeDownCast(
      shNode->GetItemDataNode(*itemIt) );
    if (!modelNode)
      {
      continue;
      }
    // TODO: look up segment with matching name and overwrite that
    if (!vtkCjyxSegmentationsModuleLogic::ImportModelToSegmentationNode(modelNode, segmentationNode))
      {
      vtkErrorWithObjectMacro(segmentationNode, "ImportModelsToSegmentationNode: Failed to import model node "
        << modelNode->GetName() << " to segmentation " << segmentationNode->GetName());
      returnValue = false;
      }
    }

  return returnValue;
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::ImportLabelmapToSegmentationNode(vtkDMMLLabelMapVolumeNode* labelmapNode,
  vtkDMMLSegmentationNode* segmentationNode, std::string insertBeforeSegmentId/*=""*/)
{
  if (!segmentationNode)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::ImportLabelmapToSegmentationNode: Invalid segmentation node");
    return false;
    }
  if (!labelmapNode || !labelmapNode->GetImageData())
    {
    vtkErrorWithObjectMacro(segmentationNode, "ImportLabelmapToSegmentationNode: Invalid labelmap volume node");
    return false;
    }

  // Get labelmap geometry
  vtkSmartPointer<vtkMatrix4x4> labelmapIjkToRasMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  labelmapNode->GetIJKToRASMatrix(labelmapIjkToRasMatrix);

  // Note: Splitting code ported from EditorLib/HelperBox.py:split

  // Get color node
  vtkDMMLColorTableNode* colorTableNode = nullptr;
  if (labelmapNode->GetDisplayNode())
    {
    colorTableNode = vtkDMMLColorTableNode::SafeDownCast(labelmapNode->GetDisplayNode()->GetColorNode());
    }
  if (colorTableNode)
    {
    segmentationNode->SetLabelmapConversionColorTableNodeID(colorTableNode->GetID());
    }

  if (!segmentationNode->GetDisplayNode())
    {
    segmentationNode->CreateDefaultDisplayNodes();
    }

  // Split labelmap node into per-label image data

  vtkNew<vtkIntArray> labelValues;
  vtkCjyxSegmentationsModuleLogic::GetAllLabelValues(labelValues.GetPointer(), labelmapNode->GetImageData());

  vtkSmartPointer<vtkOrientedImageData> labelOrientedImageData = vtkSmartPointer<vtkOrientedImageData>::New();
  labelOrientedImageData->vtkImageData::DeepCopy(labelmapNode->GetImageData());
  labelOrientedImageData->SetGeometryFromImageToWorldMatrix(labelmapIjkToRasMatrix);

  // Apply parent transforms if any
  if (labelmapNode->GetParentTransformNode() || segmentationNode->GetParentTransformNode())
    {
    vtkSmartPointer<vtkGeneralTransform> labelmapToSegmentationTransform = vtkSmartPointer<vtkGeneralTransform>::New();
    vtkCjyxSegmentationsModuleLogic::GetTransformBetweenRepresentationAndSegmentation(labelmapNode, segmentationNode, labelmapToSegmentationTransform);
    vtkOrientedImageDataResample::TransformOrientedImage(labelOrientedImageData, labelmapToSegmentationTransform);
    }

  DMMLNodeModifyBlocker blocker(segmentationNode);
  for (int labelIndex = 0; labelIndex < labelValues->GetNumberOfValues(); ++labelIndex)
    {
    int label = labelValues->GetValue(labelIndex);
    vtkSmartPointer<vtkSegment> segment = vtkSmartPointer<vtkSegment>::New();
    segment->SetLabelValue(label);

    // Set segment color
    double color[4] = { vtkSegment::SEGMENT_COLOR_INVALID[0],
                        vtkSegment::SEGMENT_COLOR_INVALID[1],
                        vtkSegment::SEGMENT_COLOR_INVALID[2], 1.0 };
    const char* labelName = nullptr;
    if (colorTableNode)
      {
      labelName = colorTableNode->GetColorName(label);
      colorTableNode->GetColor(label, color);
      }
    segment->SetColor(color[0], color[1], color[2]);

    // If the labelname could not be found in the color node, and if there is only one label,
    // then the (only) segment name will be the labelmap name
    if (!labelName && labelValues->GetNumberOfValues() == 1)
      {
      labelName = labelmapNode->GetName();
      }

    // Set segment name
    if (!labelName)
      {
      std::stringstream ss;
      ss << "Label_" << label;
      labelName = ss.str().c_str();
      }
    segment->SetName(labelName);

    // Clip to effective extent
    int labelOrientedImageDataEffectiveExtent[6] = { 0, -1, 0, -1, 0, -1 };
    vtkOrientedImageDataResample::CalculateEffectiveExtent(labelOrientedImageData, labelOrientedImageDataEffectiveExtent);
    vtkSmartPointer<vtkImageConstantPad> padder = vtkSmartPointer<vtkImageConstantPad>::New();
    padder->SetInputData(labelOrientedImageData);
    padder->SetOutputWholeExtent(labelOrientedImageDataEffectiveExtent);
    padder->Update();
    labelOrientedImageData->ShallowCopy(padder->GetOutput());

    // Add oriented image data as binary labelmap representation
    segment->AddRepresentation(
      vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName(),
      labelOrientedImageData );

    if (!segmentationNode->GetSegmentation()->AddSegment(segment, "", insertBeforeSegmentId))
      {
      vtkErrorWithObjectMacro(segmentationNode, "ImportLabelmapToSegmentationNode: Failed to add segment to segmentation");
      return false;
      }
    } // for each label

  return true;
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::ImportLabelmapToSegmentationNode(vtkOrientedImageData* labelmapImage,
  vtkDMMLSegmentationNode* segmentationNode, std::string baseSegmentName/*=""*/, std::string insertBeforeSegmentId/*=""*/)
{
  if (!segmentationNode)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::ImportLabelmapToSegmentationNode: Invalid segmentation node");
    return false;
    }
  if (!labelmapImage)
    {
    vtkErrorWithObjectMacro(labelmapImage, "ImportLabelmapToSegmentationNode: Invalid labelmap image");
    return false;
    }

  if (!segmentationNode->GetDisplayNode())
    {
    segmentationNode->CreateDefaultDisplayNodes();
    }

  // Note: Splitting code ported from EditorLib/HelperBox.py:split

  // Split labelmap node into per-label image data

  vtkNew<vtkIntArray> labelValues;
  vtkCjyxSegmentationsModuleLogic::GetAllLabelValues(labelValues.GetPointer(), labelmapImage);

  DMMLNodeModifyBlocker blocker(segmentationNode);

  // Clip to effective extent
  int labelOrientedImageDataEffectiveExtent[6] = { 0, -1, 0, -1, 0, -1 };
  vtkOrientedImageDataResample::CalculateEffectiveExtent(labelmapImage, labelOrientedImageDataEffectiveExtent);

  vtkSmartPointer<vtkImageConstantPad> padder = vtkSmartPointer<vtkImageConstantPad>::New();
  padder->SetInputData(labelmapImage);
  padder->SetOutputWholeExtent(labelOrientedImageDataEffectiveExtent);
  padder->Update();

  vtkNew<vtkOrientedImageData> labelOrientedImageData;
  labelOrientedImageData->ShallowCopy(padder->GetOutput());

  vtkSmartPointer<vtkMatrix4x4> labelmapImageToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  labelmapImage->GetImageToWorldMatrix(labelmapImageToWorldMatrix);
  labelOrientedImageData->SetGeometryFromImageToWorldMatrix(labelmapImageToWorldMatrix);

  for (int labelIndex = 0; labelIndex < labelValues->GetNumberOfValues(); ++labelIndex)
    {
    int label = labelValues->GetValue(labelIndex);

    vtkSmartPointer<vtkSegment> segment = vtkSmartPointer<vtkSegment>::New();

    // Set segment name
    std::stringstream ss;
    ss << (baseSegmentName.empty() ? "Label" : baseSegmentName) << "_" << labelIndex+1;
    segment->SetName(ss.str().c_str());

    // Set segment label value
    segment->SetLabelValue(label);

    // Add oriented image data as binary labelmap representation
    segment->AddRepresentation(
      vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName(),
      labelOrientedImageData );

    if (!segmentationNode->GetSegmentation()->AddSegment(segment, "", insertBeforeSegmentId))
      {
      vtkErrorWithObjectMacro(segmentationNode, "ImportLabelmapToSegmentationNode: Failed to add segment to segmentation");
      return false;
      }
    } // for each label

  return true;
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::ImportLabelmapToSegmentationNode(
  vtkDMMLLabelMapVolumeNode* labelmapNode, vtkDMMLSegmentationNode* segmentationNode, vtkStringArray* updatedSegmentIDs)
{
  if (!segmentationNode || !segmentationNode->GetSegmentation())
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::ImportLabelmapToSegmentationNode: Invalid segmentation node");
    return false;
    }
  if (!labelmapNode || !labelmapNode->GetImageData())
    {
    vtkErrorWithObjectMacro(segmentationNode, "vtkCjyxSegmentationsModuleLogic::ImportLabelmapToSegmentationNode: Invalid labelmap volume node");
    return false;
    }

  // Get oriented image data from labelmap volume node
  vtkNew<vtkOrientedImageData> labelOrientedImageData;
  labelOrientedImageData->vtkImageData::ShallowCopy(labelmapNode->GetImageData());
  vtkNew<vtkMatrix4x4> ijkToRasMatrix;
  labelmapNode->GetIJKToRASMatrix(ijkToRasMatrix.GetPointer());
  labelOrientedImageData->SetGeometryFromImageToWorldMatrix(ijkToRasMatrix.GetPointer());

  // Apply transforms if segmentation and labelmap are not in the same coordinate system
  vtkSmartPointer<vtkGeneralTransform> labelmapToSegmentationTransform;
  if (labelmapNode->GetParentTransformNode() != segmentationNode->GetParentTransformNode())
    {
    labelmapToSegmentationTransform = vtkSmartPointer<vtkGeneralTransform>::New();
    vtkCjyxSegmentationsModuleLogic::GetTransformBetweenRepresentationAndSegmentation(labelmapNode, segmentationNode, labelmapToSegmentationTransform);
    vtkOrientedImageDataResample::TransformOrientedImage(labelOrientedImageData, labelmapToSegmentationTransform);
    }

  return vtkCjyxSegmentationsModuleLogic::ImportLabelmapToSegmentationNode(
    labelOrientedImageData, segmentationNode, updatedSegmentIDs,
    labelmapToSegmentationTransform);
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::ImportLabelmapToSegmentationNode(
  vtkOrientedImageData* labelmapImage, vtkDMMLSegmentationNode* segmentationNode, vtkStringArray* updatedSegmentIDs,
  vtkGeneralTransform* labelmapToSegmentationTransform /*=nullptr*/)
{
  if (!segmentationNode || !segmentationNode->GetSegmentation())
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::ImportLabelmapToSegmentationNode: Invalid segmentation node");
    return false;
    }
  if (!labelmapImage)
    {
    vtkErrorWithObjectMacro(segmentationNode, "vtkCjyxSegmentationsModuleLogic::ImportLabelmapToSegmentationNode: Invalid labelmap volume");
    return false;
    }
  if (!updatedSegmentIDs)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::ImportLabelmapToSegmentationNode: Invalid updatedSegmentIDs");
    return false;
    }

  // If master representation is not binary labelmap, then cannot add
  // (this should have been done by the UI classes, notifying the users about hazards of changing the master representation)
  if (segmentationNode->GetSegmentation()->GetMasterRepresentationName() != vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName())
    {
    vtkErrorWithObjectMacro(segmentationNode, "vtkCjyxSegmentationsModuleLogic::ImportLabelmapToSegmentationNode:"
      "Master representation of the target segmentation node "
      << (segmentationNode->GetName() ? segmentationNode->GetName() : "NULL") << " is not binary labelmap");
    return false;
    }

  if (!segmentationNode->GetDisplayNode())
    {
    segmentationNode->CreateDefaultDisplayNodes();
    }

  // Get labelmap geometry
  vtkSmartPointer<vtkMatrix4x4> labelmapIjkToRasMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  labelmapImage->GetImageToWorldMatrix(labelmapIjkToRasMatrix);

  // Create oriented image data for label
  vtkSmartPointer<vtkOrientedImageData> labelOrientedImageData = vtkSmartPointer<vtkOrientedImageData>::New();
  labelOrientedImageData->ShallowCopy(labelmapImage);
  labelOrientedImageData->SetGeometryFromImageToWorldMatrix(labelmapIjkToRasMatrix);

  // Apply parent transforms if any
  if (labelmapToSegmentationTransform)
    {
    vtkOrientedImageDataResample::TransformOrientedImage(labelOrientedImageData, labelmapToSegmentationTransform);
    }

  DMMLNodeModifyBlocker blocker(segmentationNode);
  for (int segmentIndex = 0; segmentIndex < updatedSegmentIDs->GetNumberOfValues(); ++segmentIndex)
    {
    std::string segmentId = updatedSegmentIDs->GetValue(segmentIndex);
    if (segmentId.empty())
      {
      continue;
      }
    vtkSegment* segment = segmentationNode->GetSegmentation()->GetSegment(segmentId);
    if (!segment)
      {
      continue;
      }

    // Clear current content of the segment (before setting new label)
    segmentationNode->GetSegmentation()->ClearSegment(segmentId);

    int label = segmentIndex + 1;
    segment->SetLabelValue(label);
    segment->AddRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName(), labelOrientedImageData);

    } // for each label

  return true;
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::ImportLabelmapToSegmentationNodeWithTerminology(vtkDMMLLabelMapVolumeNode* labelmapNode,
  vtkDMMLSegmentationNode* segmentationNode, std::string terminologyContextName, std::string insertBeforeSegmentId/*=""*/)
{
  DMMLNodeModifyBlocker blocker(segmentationNode);

  // Import labelmap to segmentation
  if (! vtkCjyxSegmentationsModuleLogic::ImportLabelmapToSegmentationNode(
        labelmapNode, segmentationNode, insertBeforeSegmentId ) )
    {
    vtkErrorMacro("ImportLabelmapToSegmentationNodeWithTerminology: Invalid labelmap volume");
    return false;
    }

  // Assign terminology to segments in the populated segmentation based on the labels of the imported labelmap
  return this->SetTerminologyToSegmentationFromLabelmapNode(segmentationNode, labelmapNode, terminologyContextName);
}

//-----------------------------------------------------------------------------
vtkDataObject* vtkCjyxSegmentationsModuleLogic::CreateRepresentationForOneSegment(
  vtkSegmentation* segmentation, std::string segmentID, std::string representationName )
{
  if (!segmentation)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::CreateRepresentationForOneSegment: Invalid segmentation");
    return nullptr;
    }

  // Temporarily duplicate selected segment to only convert them, not the whole segmentation (to save time)
  vtkSmartPointer<vtkSegmentation> segmentationCopy = vtkSmartPointer<vtkSegmentation>::New();
  segmentationCopy->SetMasterRepresentationName(segmentation->GetMasterRepresentationName());
  segmentationCopy->CopyConversionParameters(segmentation);
  segmentationCopy->CopySegmentFromSegmentation(segmentation, segmentID);
  if (!segmentationCopy->CreateRepresentation(representationName, true))
    {
    vtkErrorWithObjectMacro(segmentation, "CreateRepresentationForOneSegment: Failed to convert segment " << segmentID << " to " << representationName);
    return nullptr;
    }

  // If conversion succeeded,
  vtkDataObject* segmentTempRepresentation = vtkDataObject::SafeDownCast(
    segmentationCopy->GetSegment(segmentID)->GetRepresentation(representationName) );
  if (!segmentTempRepresentation)
    {
    vtkErrorWithObjectMacro(segmentation, "CreateRepresentationForOneSegment: Failed to get representation "
      << representationName << " from segment " << segmentID);
    return nullptr;
    }

  // Copy representation into new data object (the representation will be deleted when segmentation copy gets out of scope)
  vtkDataObject* representationCopy =
    vtkSegmentationConverterFactory::GetInstance()->ConstructRepresentationObjectByClass(segmentTempRepresentation->GetClassName());
  representationCopy->ShallowCopy(segmentTempRepresentation);
  return representationCopy;
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::ApplyParentTransformToOrientedImageData(
  vtkDMMLTransformableNode* transformableNode, vtkOrientedImageData* orientedImageData, bool linearInterpolation/*=false*/, double backgroundColor[4]/*=nullptr*/ )
{
  if (!transformableNode || !orientedImageData)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::ApplyParentTransformToOrientedImageData: Invalid inputs");
    return false;
    }

  // Get world to reference RAS transform
  vtkSmartPointer<vtkGeneralTransform> nodeToWorldTransform = vtkSmartPointer<vtkGeneralTransform>::New();
  vtkDMMLTransformNode* parentTransformNode = transformableNode->GetParentTransformNode();
  if (!parentTransformNode)
    {
    // There is no parent transform for segmentation, nothing to apply
    return true;
    }

  // Transform oriented image data
  parentTransformNode->GetTransformToWorld(nodeToWorldTransform);
  vtkOrientedImageDataResample::TransformOrientedImage(orientedImageData, nodeToWorldTransform, false, false, linearInterpolation, backgroundColor);

  return true;
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::ApplyParentTransformToPolyData(vtkDMMLTransformableNode* transformableNode, vtkPolyData* polyData)
{
  if (!transformableNode || !polyData)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::ApplyParentTransformToPolyData: Invalid inputs");
    return false;
    }

  // Get world to reference RAS transform
  vtkSmartPointer<vtkGeneralTransform> nodeToWorldTransform = vtkSmartPointer<vtkGeneralTransform>::New();
  vtkDMMLTransformNode* parentTransformNode = transformableNode->GetParentTransformNode();
  if (!parentTransformNode)
    {
    // There is no parent transform for segmentation, nothing to apply
    return true;
    }

  // Transform oriented image data
  parentTransformNode->GetTransformToWorld(nodeToWorldTransform);

  vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  transformFilter->SetInputData(polyData);
  transformFilter->SetTransform(nodeToWorldTransform);
  transformFilter->Update();
  polyData->DeepCopy(transformFilter->GetOutput());

  return true;
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::GetTransformBetweenRepresentationAndSegmentation(
  vtkDMMLTransformableNode* representationNode, vtkDMMLSegmentationNode* segmentationNode, vtkGeneralTransform* representationToSegmentationTransform )
{
  if (!representationNode || !segmentationNode || !representationToSegmentationTransform)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::GetTransformBetweenRepresentationAndSegmentation: Invalid inputs");
    return false;
    }
  vtkDMMLTransformNode::GetTransformBetweenNodes(
    representationNode->GetParentTransformNode(), segmentationNode->GetParentTransformNode(), representationToSegmentationTransform);

  return true;
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::GetSegmentRepresentation(
  vtkDMMLSegmentationNode* segmentationNode, std::string segmentID, std::string representationName,
  vtkDataObject* segmentRepresentation, bool applyParentTransform/*=true*/ )
{
  if (!segmentationNode || segmentID.empty() || representationName.empty() || !segmentRepresentation)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::GetSegmentRepresentation: Invalid inputs");
    return false;
    }

  // Get requested segment
  vtkSegment* segment = segmentationNode->GetSegmentation()->GetSegment(segmentID);
  if (!segment)
    {
    vtkErrorWithObjectMacro(segmentationNode, "vtkCjyxSegmentationsModuleLogic::GetSegmentRepresentation: Unable to find segment with ID "
      << segmentID << " in segmentation " << segmentationNode->GetName());
    return false;
    }

  if (segmentationNode->GetSegmentation()->ContainsRepresentation(representationName))
    {
    // Get and copy representation into output data object
    vtkDataObject* representationObject = segment->GetRepresentation(representationName);
    if (!representationObject)
      {
      vtkErrorWithObjectMacro(segmentationNode, "vtkCjyxSegmentationsModuleLogic::GetSegmentRepresentation: Unable to get '" << representationName
        << "' representation from segment with ID " << segmentID << " in segmentation " << segmentationNode->GetName());
      return false;
      }
    segmentRepresentation->DeepCopy(representationObject);
    }
  else // Need to convert
    {
    // Temporarily duplicate selected segment to only convert them, not the whole segmentation (to save time)
    vtkSmartPointer<vtkDataObject> representationObject = vtkSmartPointer<vtkDataObject>::Take(
      vtkCjyxSegmentationsModuleLogic::CreateRepresentationForOneSegment(segmentationNode->GetSegmentation(), segmentID, representationName) );
    if (!representationObject.GetPointer())
      {
      vtkErrorWithObjectMacro(segmentationNode, "vtkCjyxSegmentationsModuleLogic::GetSegmentRepresentation: Unable to convert segment with ID "
        << segmentID << " to '" << representationName << "' representation in segmentation " << segmentationNode->GetName());
      return false;
      }
    segmentRepresentation->DeepCopy(representationObject);
    }

  // Apply parent transformation nodes if necessary
  if (applyParentTransform && segmentationNode->GetParentTransformNode())
    {
    vtkOrientedImageData* segmentRepresentationOrientedImageData = vtkOrientedImageData::SafeDownCast(segmentRepresentation);
    vtkPolyData* segmentRepresentationPolyData = vtkPolyData::SafeDownCast(segmentRepresentation);
    if (segmentRepresentationOrientedImageData)
      {
      if (!vtkCjyxSegmentationsModuleLogic::ApplyParentTransformToOrientedImageData(segmentationNode, segmentRepresentationOrientedImageData))
        {
        vtkErrorWithObjectMacro(segmentationNode, "vtkCjyxSegmentationsModuleLogic::GetSegmentRepresentation: Failed to apply parent transform of "
          << "segmentation " << segmentationNode->GetName() << " on representation oriented image data");
        return false;
        }
      }
    else if (segmentRepresentationPolyData)
      {
        if (!vtkCjyxSegmentationsModuleLogic::ApplyParentTransformToPolyData(segmentationNode, segmentRepresentationPolyData))
        {
        vtkErrorWithObjectMacro(segmentationNode, "vtkCjyxSegmentationsModuleLogic::GetSegmentRepresentation: Failed to apply parent transform of "
          << "segmentation " << segmentationNode->GetName() << " on representation poly data");
        return false;
        }
      }
    else
      {
      vtkErrorWithObjectMacro(segmentationNode, "vtkCjyxSegmentationsModuleLogic::GetSegmentRepresentation: Failed to apply parent transform of "
        << "segmentation " << segmentationNode->GetName() << " due to unsupported representation with class name " << segmentRepresentation->GetClassName());
      return false;
      }
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::GetSegmentBinaryLabelmapRepresentation(
  vtkDMMLSegmentationNode* segmentationNode, std::string segmentID, vtkOrientedImageData* imageData, bool applyParentTransform/*=true*/ )
{
  if (!segmentationNode || segmentID.empty() || !imageData)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::GetSegmentBinaryLabelmapRepresentation: Invalid inputs");
    return false;
    }

  return vtkCjyxSegmentationsModuleLogic::GetSegmentRepresentation(segmentationNode, segmentID,
    vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName(), imageData, applyParentTransform );
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::GetSegmentClosedSurfaceRepresentation(vtkDMMLSegmentationNode* segmentationNode,
  std::string segmentID, vtkPolyData* polyData, bool applyParentTransform/*=true*/)
{
  if (!segmentationNode || segmentID.empty() || !polyData)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::GetSegmentBinaryLabelmapRepresentation: Invalid inputs");
    return false;
    }

  return vtkCjyxSegmentationsModuleLogic::GetSegmentRepresentation(segmentationNode, segmentID,
    vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName(), polyData, applyParentTransform);
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::SetBinaryLabelmapToSegment(
  vtkOrientedImageData* labelmap, vtkDMMLSegmentationNode* segmentationNode, std::string segmentID, int mergeMode/*=MODE_REPLACE*/, const int extent[6]/*=0*/,
    bool minimumOfAllSegments/*=false*/, const std::vector<std::string>& segmentIdsToOverwrite/*={}*/)
{
  if (!segmentationNode)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::SetBinaryLabelmapToSegment: Invalid input");
    return false;
    }

  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  if (!segmentation)
    {
    vtkGenericWarningMacro("vtkCjyxSegmentationsModuleLogic::SetBinaryLabelmapToSegment: Invalid segmentation");
    return false;
    }

  std::vector<std::string> modifiedSegmentIDs;
  bool result = vtkSegmentationModifier::ModifyBinaryLabelmap(labelmap, segmentation, segmentID, mergeMode, extent, minimumOfAllSegments,
    false, segmentIdsToOverwrite, &modifiedSegmentIDs);

  // Re-convert all other representations
  bool conversionHappened = false;
  std::vector<std::string> representationNames;
  vtkSegment* segment = segmentation->GetSegment(segmentID);
  if (segment)
    {
    segment->GetContainedRepresentationNames(representationNames);
    for (std::vector<std::string>::iterator reprIt = representationNames.begin();
      reprIt != representationNames.end(); ++reprIt)
      {
      std::string targetRepresentationName = (*reprIt);
      if (targetRepresentationName.compare(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()))
        {
        vtkSegmentationConverter::ConversionPathAndCostListType pathCosts;
        segmentation->GetPossibleConversions(targetRepresentationName, pathCosts);

        // Get cheapest path from found conversion paths
        vtkSegmentationConverter::ConversionPathType cheapestPath = vtkSegmentationConverter::GetCheapestPath(pathCosts);
        if (cheapestPath.empty())
          {
          continue;
          }
        conversionHappened |= segmentation->ConvertSegmentsUsingPath(modifiedSegmentIDs, cheapestPath, true);
        }
      }
    }

  if (conversionHappened)
    {
    const char* segmentIdChar = segmentID.c_str();
    segmentationNode->GetSegmentation()->InvokeEvent(vtkSegmentation::RepresentationModified, (void*)segmentIdChar);
    }

  return result;
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::SetTerminologyToSegmentationFromLabelmapNode(vtkDMMLSegmentationNode* segmentationNode,
  vtkDMMLLabelMapVolumeNode* labelmapNode, std::string terminologyContextName)
{
  if (terminologyContextName.empty())
    {
    return true;
    }

  vtkCjyxTerminologiesModuleLogic* terminologiesLogic =
    vtkCjyxTerminologiesModuleLogic::SafeDownCast(this->GetModuleLogic("Terminologies"));
  if (!terminologiesLogic)
    {
    vtkErrorMacro("SetTerminologyToSegmentationFromLabelmapNode: Terminology logic cannot be accessed");
    return false;
    }
  if (!segmentationNode)
    {
    vtkErrorMacro("SetTerminologyToSegmentationFromLabelmapNode: Invalid segmentation node");
    return false;
    }
  if (!labelmapNode || !labelmapNode->GetImageData())
    {
    vtkErrorMacro("SetTerminologyToSegmentationFromLabelmapNode: Invalid labelmap volume node");
    return false;
    }

  // Get color node
  if (!labelmapNode->GetDisplayNode())
    {
    vtkErrorMacro("SetTerminologyToSegmentationFromLabelmapNode: Segmentation node " << segmentationNode->GetName() << " has no display node");
    return false;
    }
  vtkDMMLColorTableNode* colorNode = vtkDMMLColorTableNode::SafeDownCast(labelmapNode->GetDisplayNode()->GetColorNode());
  if (!colorNode)
    {
    vtkErrorMacro("SetTerminologyToSegmentationFromLabelmapNode: Segmentation node " << segmentationNode->GetName() << " has no associated color table node");
    return false;
    }

  // Get first terminology entry. This is set to segments that cannot be matched to labels, and when
  // label names are not found in 3dCjyxLabel attributes in terminology types within the context.
  std::vector<vtkCjyxTerminologiesModuleLogic::CodeIdentifier> categories;
  terminologiesLogic->GetCategoriesInTerminology(terminologyContextName, categories);
  if (categories.empty())
    {
    vtkErrorMacro("SetTerminologyToSegmentationFromLabelmapNode: Terminology context " << terminologyContextName << " is empty");
    return false;
    }
  std::vector<vtkCjyxTerminologiesModuleLogic::CodeIdentifier> typesInFirstCategory;
  int firstNonEmptyCategoryIndex = -1;
  do
    {
    terminologiesLogic->GetTypesInTerminologyCategory(terminologyContextName, categories[++firstNonEmptyCategoryIndex], typesInFirstCategory);
    }
  while (typesInFirstCategory.empty() && firstNonEmptyCategoryIndex < static_cast<int>(categories.size()));
  if (typesInFirstCategory.empty())
    {
    vtkErrorMacro("SetTerminologyToSegmentationFromLabelmapNode: All categories in terminology context " << terminologyContextName << " are empty");
    return false;
    }

  vtkSmartPointer<vtkCjyxTerminologyEntry> firstTerminologyEntry = vtkSmartPointer<vtkCjyxTerminologyEntry>::New();
  firstTerminologyEntry->SetTerminologyContextName(terminologyContextName.c_str());
  vtkSmartPointer<vtkCjyxTerminologyCategory> firstCategory = vtkSmartPointer<vtkCjyxTerminologyCategory>::New();
  terminologiesLogic->GetCategoryInTerminology(
    terminologyContextName, categories[firstNonEmptyCategoryIndex], firstCategory );
  firstTerminologyEntry->GetCategoryObject()->Copy(firstCategory);
  vtkSmartPointer<vtkCjyxTerminologyType> firstType = vtkSmartPointer<vtkCjyxTerminologyType>::New();
  terminologiesLogic->GetTypeInTerminologyCategory(
    terminologyContextName, categories[firstNonEmptyCategoryIndex], typesInFirstCategory[0], firstType );
  firstTerminologyEntry->GetTypeObject()->Copy(firstType);
  std::string firstTerminologyString = terminologiesLogic->SerializeTerminologyEntry(firstTerminologyEntry);

  DMMLNodeModifyBlocker blocker(segmentationNode);

  // Assign terminology entry to each segment in the segmentation
  std::vector<std::string> segmentIDs;
  segmentationNode->GetSegmentation()->GetSegmentIDs(segmentIDs);
  vtkSmartPointer<vtkCjyxTerminologyEntry> foundTerminologyEntry = vtkSmartPointer<vtkCjyxTerminologyEntry>::New();
  for (std::vector<std::string>::iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt)
    {
    vtkSegment* segment = segmentationNode->GetSegmentation()->GetSegment(*segmentIdIt);

    // Check if label for the current segment exists in labelmap. If there were segments in the segmentation when
    // importing, then the label and so terminology will not be found. In this case terminology tag is left as is
    // (which may be the default GeneralAnatomy/Tissue/Tissue, or the one the user manually specified)
    int label = colorNode->GetColorIndexByName(segment->GetName());
    if (label == -1)
      {
      continue;
      }

    // Search for the 3dCjyxLabel attribute in the specified terminology context
    if (terminologiesLogic->FindTypeInTerminologyBy3dCjyxLabel(terminologyContextName, segment->GetName(), foundTerminologyEntry))
      {
      std::string foundTerminologyString = terminologiesLogic->SerializeTerminologyEntry(foundTerminologyEntry);
      segment->SetTag(vtkSegment::GetTerminologyEntryTagName(), foundTerminologyString);
      }
    else
      {
      // Set first entry if 3dCjyxLabel is not found
      segment->SetTag(vtkSegment::GetTerminologyEntryTagName(), firstTerminologyString);
      }
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::ExportSegmentsClosedSurfaceRepresentationToFiles(std::string destinationFolder,
  vtkDMMLSegmentationNode* segmentationNode, vtkStringArray* segmentIds /*=nullptr*/,
  std::string fileFormat /*="STL"*/, bool lps /*=true*/, double sizeScale /*=1.0*/, bool merge /*=false*/)
{
  if (!segmentationNode || !segmentationNode->GetSegmentation())
    {
    vtkGenericWarningMacro("ExportSegmentsClosedSurfaceRepresentationToFiles failed: invalid segmentationNode");
    return false;
    }

  std::vector<std::string> segmentIdsVector;
  if (segmentIds == nullptr)
    {
    segmentationNode->GetSegmentation()->GetSegmentIDs(segmentIdsVector);
    }
  else
    {
    for (int segmentIndex = 0; segmentIndex < segmentIds->GetNumberOfValues(); ++segmentIndex)
      {
      segmentIdsVector.push_back(segmentIds->GetValue(segmentIndex));
      }
    }

  std::string extension = vtksys::SystemTools::LowerCase(fileFormat);
  if (extension == "obj")
    {
    return ExportSegmentsClosedSurfaceRepresentationToObjFile(destinationFolder, segmentationNode, segmentIdsVector, lps, sizeScale);
    }
  if (extension != "stl")
    {
    vtkGenericWarningMacro("ExportSegmentsClosedSurfaceRepresentationToFiles: fileFormat "
      << fileFormat << " is unknown. Using STL.");
    }
  return ExportSegmentsClosedSurfaceRepresentationToStlFiles(destinationFolder, segmentationNode, segmentIdsVector, lps, sizeScale, merge);
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::ExportSegmentsClosedSurfaceRepresentationToStlFiles(std::string destinationFolder,
  vtkDMMLSegmentationNode* segmentationNode, const std::vector<std::string>& segmentIDs, bool lps, double sizeScale, bool merge)
{
  if (!segmentationNode)
    {
    vtkGenericWarningMacro("ExportSegmentsClosedSurfaceRepresentationToFiles failed: invalid segmentationNode");
    return false;
    }

  // We explicitly write the coordinate system into the file header.
  // See vtkDMMLModelStorageNode::WriteDataInternal.
  const std::string coordinateSystemValue = (lps ? "LPS" : "RAS");
  const std::string coordinateSytemSpecification = "SPACE=" + coordinateSystemValue;

  vtkNew<vtkTriangleFilter> triangulator;
  vtkNew<vtkSTLWriter> writer;
  writer->SetFileType(VTK_BINARY);
  writer->SetInputConnection(triangulator->GetOutputPort());
  std::string header = std::string("3D Cjyx output. ") + coordinateSytemSpecification;
  if (sizeScale != 1.0)
    {
    std::ostringstream strs;
    strs << sizeScale;
    header += ";SCALE=" + strs.str();
    }
  writer->SetHeader(header.c_str());

  if (merge)
    {
    vtkNew<vtkAppendPolyData> appendPolyData;

    for (std::vector<std::string>::const_iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt)
      {
      vtkNew<vtkPolyData> segmentPolyData;
      bool polyDataAvailable = vtkCjyxSegmentationsModuleLogic::GetSegmentClosedSurfaceRepresentation(
        segmentationNode, *segmentIdIt, segmentPolyData.GetPointer());
      if (!polyDataAvailable || segmentPolyData.GetPointer() == nullptr)
        {
        vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsClosedSurfaceRepresentationToFiles: Unable to convert segment "
          << (*segmentIdIt) << " to closed surface representation");
        continue;
        }
      appendPolyData->AddInputData(segmentPolyData.GetPointer());
      }
    vtkNew<vtkTransform> transformRasToLps;
    if (sizeScale != 1.0)
      {
      transformRasToLps->Scale(sizeScale, sizeScale, sizeScale);
      }
    if (lps)
      {
      transformRasToLps->Scale(-1, -1, 1);
      }
    vtkNew<vtkTransformPolyDataFilter> transformPolyDataToOutput;
    transformPolyDataToOutput->SetTransform(transformRasToLps.GetPointer());
    transformPolyDataToOutput->SetInputConnection(appendPolyData->GetOutputPort());
    std::string safeFileName = vtkCjyxSegmentationsModuleLogic::GetSafeFileName(segmentationNode->GetName());
    std::string filePath = destinationFolder + "/" + safeFileName + ".stl";
    triangulator->SetInputConnection(transformPolyDataToOutput->GetOutputPort());
    writer->SetFileName(filePath.c_str());
    try
      {
      writer->Write();
      }
    catch (...)
      {
      vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsClosedSurfaceRepresentationToFiles:"
        " Unable to write segmentation to " << filePath);
      return false;
      }
    }
  else
    {
    for (std::vector<std::string>::const_iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt)
      {
      vtkNew<vtkPolyData> segmentPolyData;
      bool polyDataAvailable = vtkCjyxSegmentationsModuleLogic::GetSegmentClosedSurfaceRepresentation(
        segmentationNode, *segmentIdIt, segmentPolyData.GetPointer());
      if (!polyDataAvailable || segmentPolyData.GetPointer() == nullptr)
        {
        vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsClosedSurfaceRepresentationToFiles: Unable to convert segment "
          << (*segmentIdIt) << " to closed surface representation");
        continue;
        }
      vtkNew<vtkTransform> transformRasToLps;
      if (sizeScale != 1.0)
        {
        transformRasToLps->Scale(sizeScale, sizeScale, sizeScale);
        }
      if (lps)
        {
        transformRasToLps->Scale(-1, -1, 1);
        }
      vtkNew<vtkTransformPolyDataFilter> transformPolyDataToOutput;
      transformPolyDataToOutput->SetTransform(transformRasToLps.GetPointer());
      transformPolyDataToOutput->SetInputData(segmentPolyData.GetPointer());
      std::string segmentName = segmentationNode->GetSegmentation()->GetSegment(*segmentIdIt)->GetName();
      std::string safeFileName = vtkCjyxSegmentationsModuleLogic::GetSafeFileName(segmentationNode->GetName());
      std::string filePath = destinationFolder + "/" + safeFileName + "_" + segmentName + ".stl";
      triangulator->SetInputConnection(transformPolyDataToOutput->GetOutputPort());
      writer->SetFileName(filePath.c_str());
      try
        {
        writer->Write();
        }
      catch (...)
        {
        vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsClosedSurfaceRepresentationToFiles:"
          " Unable to write segmentation to " << filePath);
        return false;
        }
      }
    }
  return true;
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::ExportSegmentsClosedSurfaceRepresentationToObjFile(std::string destinationFolder,
  vtkDMMLSegmentationNode* segmentationNode, const std::vector<std::string>& segmentIDs, bool lps, double sizeScale)
{
  if (!segmentationNode)
    {
    vtkGenericWarningMacro("ExportSegmentsClosedSurfaceRepresentationToFiles failed: invalid segmentationNode");
    return false;
    }

  vtkDMMLSegmentationDisplayNode* displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());

  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer.GetPointer());

  for (std::vector<std::string>::const_iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt)
    {
    vtkNew<vtkPolyData> segmentPolyData;
    bool polyDataAvailable = vtkCjyxSegmentationsModuleLogic::GetSegmentClosedSurfaceRepresentation(
      segmentationNode, *segmentIdIt, segmentPolyData.GetPointer());
    if (!polyDataAvailable || segmentPolyData.GetPointer() == nullptr)
      {
      vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsClosedSurfaceRepresentationToObjFile: Unable to convert segment "
        << (*segmentIdIt) << " to closed surface representation");
      continue;
      }
    vtkNew<vtkTransform> transformRasToLps;
    if (sizeScale != 1.0)
      {
      transformRasToLps->Scale(sizeScale, sizeScale, sizeScale);
      }
    if (lps)
      {
      transformRasToLps->Scale(-1, -1, 1);
      }
    vtkNew<vtkTransformPolyDataFilter> transformPolyDataToOutput;
    transformPolyDataToOutput->SetTransform(transformRasToLps.GetPointer());
    transformPolyDataToOutput->SetInputData(segmentPolyData.GetPointer());
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(transformPolyDataToOutput->GetOutputPort());
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper.GetPointer());

    if (displayNode)
      {
      double color[3] = { 0.5, 0.5, 0.5 };
      displayNode->GetSegmentColor(*segmentIdIt, color);
      // OBJ exporter sets the same color for ambient, diffuse, specular
      // so we scale it by 1/3 to avoid having too bright material.
      double colorScale = 1.0 / 3.0;
      actor->GetProperty()->SetColor(color[0] * colorScale, color[1] * colorScale, color[2] * colorScale);
      actor->GetProperty()->SetSpecularPower(3.0);
      actor->GetProperty()->SetOpacity(displayNode->GetSegmentOpacity3D(*segmentIdIt));
      }
    renderer->AddActor(actor.GetPointer());
    }

  vtkNew<vtkOBJExporter> exporter;
  exporter->SetRenderWindow(renderWindow.GetPointer());
  std::string safeFileName = vtkCjyxSegmentationsModuleLogic::GetSafeFileName(segmentationNode->GetName());
  std::string fullNameWithoutExtension = destinationFolder + "/" + safeFileName;
  exporter->SetFilePrefix(fullNameWithoutExtension.c_str());

  // We explicitly write the coordinate system into the file header.
  // See vtkDMMLModelStorageNode::WriteDataInternal.
  const std::string coordinateSystemValue = (lps ? "LPS" : "RAS");
  const std::string coordinateSytemSpecification = "SPACE=" + coordinateSystemValue;
  std::string header = std::string("3D Cjyx output. ") + coordinateSytemSpecification;
  if (sizeScale != 1.0)
    {
    std::ostringstream strs;
    strs << sizeScale;
    header += ";SCALE=" + strs.str();
    }
  exporter->SetOBJFileComment(header.c_str());

  try
    {
    exporter->Write();
    }
  catch (...)
    {
    vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsClosedSurfaceRepresentationToObjFile:"
      " Unable to write segmentation to " << fullNameWithoutExtension << ".obj");
    return false;
    }

  return true;
}

//-----------------------------------------------------------------------------
void vtkCjyxSegmentationsModuleLogic::GetLabelValuesFromColorNode(vtkDMMLSegmentationNode* segmentationNode, vtkDMMLColorTableNode* colorTableNode,
  vtkStringArray* inputSegmentIds, vtkIntArray* labelValues)
{
  if (!segmentationNode)
    {
    vtkErrorWithObjectMacro(nullptr, "GetLabelValuesFromColorNode: Invalid segmentation");
    return;
    }
  if (!colorTableNode)
    {
    vtkErrorWithObjectMacro(nullptr, "GetLabelValuesFromColorNode: Invalid color table node");
    return;
    }
  if (!labelValues)
    {
    vtkErrorWithObjectMacro(nullptr, "GetLabelValuesFromColorNode: Invalid labelValues");
    return;
    }

  vtkSmartPointer<vtkStringArray> segmentIds = inputSegmentIds;
  if (!segmentIds)
    {
    segmentIds = vtkSmartPointer<vtkStringArray>::New();
    segmentationNode->GetSegmentation()->GetSegmentIDs(segmentIds);
    }

  int extraColorCount = colorTableNode->GetNumberOfColors(); // Color for segments that are not in the table
  labelValues->SetNumberOfValues(segmentIds->GetNumberOfValues());
  for (int i = 0; i < segmentIds->GetNumberOfValues(); ++i)
    {
    vtkStdString segmentId = segmentIds->GetValue(i);
    const char* segmentName = segmentationNode->GetSegmentation()->GetSegment(segmentId)->GetName();
    int labelValue = colorTableNode->GetColorIndexByName(segmentName);
    if (labelValue < 0)
      {
      // Label value is not found in the color table
      // Use a label value that lies outside the table to prevent collisions with existing values.
      labelValue = extraColorCount;
      ++extraColorCount;
      }
    labelValues->SetValue(i, labelValue);
    }
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::ExportSegmentsBinaryLabelmapRepresentationToFiles(std::string destinationFolder,
  vtkDMMLSegmentationNode* segmentationNode, vtkStringArray* segmentIds/*=nullptr*/, std::string extension/*="NRRD"*/, bool useCompression/*=false*/,
  vtkDMMLVolumeNode* referenceVolumeNode /*=nullptr*/, int extentComputationMode /*=vtkSegmentation::EXTENT_REFERENCE_GEOMETRY*/,
  vtkDMMLColorTableNode* colorTableNode/*=nullptr*/)
{
  if (!segmentationNode)
    {
    vtkGenericWarningMacro("ExportSegmentsBinaryLabelmapRepresentationToFiles failed: invalid segmentationNode");
    return false;
    }

  vtkSmartPointer<vtkIntArray> labelValues = nullptr;
  if (colorTableNode)
    {
    labelValues = vtkSmartPointer<vtkIntArray>::New();
    vtkCjyxSegmentationsModuleLogic::GetLabelValuesFromColorNode(segmentationNode, colorTableNode, segmentIds, labelValues);
    }

  vtkNew<vtkOrientedImageData> mergedLabelmap_Reference;
  vtkCjyxSegmentationsModuleLogic::GenerateMergedLabelmapInReferenceGeometry(segmentationNode, referenceVolumeNode,
    segmentIds, extentComputationMode, mergedLabelmap_Reference, labelValues);

  vtkNew<vtkMatrix4x4> rasToIJKMatrix;
  mergedLabelmap_Reference->GetWorldToImageMatrix(rasToIJKMatrix);

  std::string safeFileName = vtkCjyxSegmentationsModuleLogic::GetSafeFileName(segmentationNode->GetName());
  std::string fullNameWithoutExtension = destinationFolder + "/" + safeFileName;
  std::string fileExtension = vtksys::SystemTools::LowerCase(extension);
  std::string fullNameWithExtension = fullNameWithoutExtension + "." + fileExtension;

  vtkNew<vtkITKImageWriter> writer;
  writer->SetInputData(mergedLabelmap_Reference);
  writer->SetRasToIJKMatrix(rasToIJKMatrix);
  writer->SetFileName(fullNameWithExtension.c_str());
  writer->SetUseCompression(useCompression);
  writer->Write();

  return true;
}

//-----------------------------------------------------------------------------
vtkDMMLSegmentationNode* vtkCjyxSegmentationsModuleLogic::GetDefaultSegmentationNode()
{
  vtkDMMLScene* scene = this->GetDMMLScene();
  if (!scene)
    {
    return nullptr;
    }

  // Setup a default segmentation node so that the default settings are propagated to all new segmentation nodes
  vtkSmartPointer<vtkDMMLNode> defaultNode = scene->GetDefaultNodeByClass("vtkDMMLSegmentationNode");
  if (!defaultNode)
    {
    defaultNode.TakeReference(scene->CreateNodeByClass("vtkDMMLSegmentationNode"));
    scene->AddDefaultNode(defaultNode);
    }
  return vtkDMMLSegmentationNode::SafeDownCast(defaultNode.GetPointer());
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::GetDefaultSurfaceSmoothingEnabled()
{
  vtkDMMLSegmentationNode* defaultSegmentationNode = this->GetDefaultSegmentationNode();
  if (!defaultSegmentationNode || !defaultSegmentationNode->GetSegmentation())
    {
    return false;
    }
  std::string smoothingFactorStr = defaultSegmentationNode->GetSegmentation()->GetConversionParameter(
      vtkBinaryLabelmapToClosedSurfaceConversionRule::GetSmoothingFactorParameterName());
  if (smoothingFactorStr.empty())
    {
    return true; // enabled by default
    }
  double smoothingFactor = vtkVariant(smoothingFactorStr).ToDouble();
  return (smoothingFactor > 0);
}

//-----------------------------------------------------------------------------
void vtkCjyxSegmentationsModuleLogic::SetDefaultSurfaceSmoothingEnabled(bool enabled)
{
  vtkDMMLSegmentationNode* defaultSegmentationNode = this->GetDefaultSegmentationNode();
  if (!defaultSegmentationNode || !defaultSegmentationNode->GetSegmentation())
    {
    vtkErrorMacro("vtkCjyxSegmentationsModuleLogic::SetSurfaceSmoothingEnabledByDefault failed: invalid default segmentation node");
    return;
    }
  std::string smoothingFactorStr = defaultSegmentationNode->GetSegmentation()->GetConversionParameter(
    vtkBinaryLabelmapToClosedSurfaceConversionRule::GetSmoothingFactorParameterName());
  double smoothingFactor = 0.5;
  if (smoothingFactorStr.empty())
    {
    smoothingFactor = vtkVariant(smoothingFactorStr).ToDouble();
    }
  if (smoothingFactor == 0.0)
    {
    smoothingFactor = (enabled ? 0.5 : -0.5);
    }
  else if ((smoothingFactor > 0.0) != enabled)
    {
    smoothingFactor = -smoothingFactor;
    }
  smoothingFactorStr = vtkVariant(smoothingFactor).ToString();
  defaultSegmentationNode->GetSegmentation()->SetConversionParameter(
    vtkBinaryLabelmapToClosedSurfaceConversionRule::GetSmoothingFactorParameterName(),
    smoothingFactorStr);
}

//-----------------------------------------------------------------------------
std::string vtkCjyxSegmentationsModuleLogic::GetSafeFileName(std::string originalName)
{
  // Remove characters from node name that cannot be used in file names
  // (same method as in qCjyxFileNameItemDelegate::fixupFileName)
  std::string safeName;
  vtksys::RegularExpression regExp("[A-Za-z0-9\\ \\-\\_\\.\\(\\)\\$\\!\\~\\#\\'\\%\\^\\{\\}]");
  for (size_t i=0; i<originalName.size(); ++i)
    {
    std::string currentCharStr;
    currentCharStr += originalName[i];
    if (regExp.find(currentCharStr))
      {
      safeName += currentCharStr;
      }
    }

  // trim whitespaces
  safeName.erase(safeName.find_last_not_of(" \t\r\n") + 1);
  safeName.erase(0, safeName.find_first_not_of(" \t\r\n"));

  return safeName;
}

//------------------------------------------------------------------------------
const char* vtkCjyxSegmentationsModuleLogic::GetSegmentStatusAsHumanReadableString(int segmentStatus)
{
  switch (segmentStatus)
    {
    case NotStarted:
      return "Not started";
    case InProgress:
      return "In progress";
    case Completed:
      return "Completed";
    case Flagged:
      return "Flagged";
    }
  return "Unknown";
};

//------------------------------------------------------------------------------
const char* vtkCjyxSegmentationsModuleLogic::GetSegmentStatusAsMachineReadableString(int segmentStatus)
{
  switch (segmentStatus)
  {
  case NotStarted:
    return "notstarted";
  case InProgress:
    return "inprogress";
  case Completed:
    return "completed";
  case Flagged:
    return "flagged";
  }
  return "unknown";
};

//------------------------------------------------------------------------------
int vtkCjyxSegmentationsModuleLogic::GetSegmentStatusFromMachineReadableString(std::string statusString)
{
  for (int i = 0; i < LastStatus; ++i)
    {
    std::string currentStatusString = vtkCjyxSegmentationsModuleLogic::GetSegmentStatusAsMachineReadableString(i);
    if (currentStatusString == statusString)
      {
      return i;
      }
    }
  return -1;
}

//------------------------------------------------------------------------------
const char* vtkCjyxSegmentationsModuleLogic::GetStatusTagName()
{
  return "Segmentation.Status";
}

//------------------------------------------------------------------------------
int vtkCjyxSegmentationsModuleLogic::GetSegmentStatus(vtkSegment* segment)
{
  if (!segment)
    {
    vtkErrorWithObjectMacro(nullptr, "Invalid segment");
    return -1;
    }
  std::string value;
  if (!segment->GetTag(vtkCjyxSegmentationsModuleLogic::GetStatusTagName(), value))
    {
    return NotStarted;
    }
  return vtkCjyxSegmentationsModuleLogic::GetSegmentStatusFromMachineReadableString(value);
}

//------------------------------------------------------------------------------
void vtkCjyxSegmentationsModuleLogic::SetSegmentStatus(vtkSegment* segment, int status)
{
  if (!segment)
    {
    vtkErrorWithObjectMacro(nullptr, "Invalid segment");
    return;
    }
  std::string currentStatusStr;
  if (status == vtkCjyxSegmentationsModuleLogic::NotStarted)
    {
    if (!segment->GetTag(vtkCjyxSegmentationsModuleLogic::GetStatusTagName(), currentStatusStr)
      || currentStatusStr.empty())
      {
      // Status information is not stored in the segment (which means that the segmentation is not started).
      // Avoid changing the tag, as it would trigger a modified event, which for example could interfere with undo/redo history
      // (a modified event on a segment clears all future undo/redo states).
      return;
      }
    }
  segment->SetTag(vtkCjyxSegmentationsModuleLogic::GetStatusTagName(), vtkCjyxSegmentationsModuleLogic::GetSegmentStatusAsMachineReadableString(status));
}

//------------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::ClearSegment(vtkDMMLSegmentationNode* segmentationNode, std::string segmentId)
{
  if (!segmentationNode)
    {
    vtkErrorWithObjectMacro(nullptr, "Invalid segmentation node");
    return false;
    }

  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  if (!segmentation)
    {
    vtkErrorWithObjectMacro(nullptr, "Invalid segmentation");
    return false;
    }

  vtkSegment* segment = segmentation->GetSegment(segmentId);
  if (!segment)
    {
    vtkErrorWithObjectMacro(nullptr, "Invalid segment");
    return false;
    }

  std::vector<std::string> representationNames;
  segmentation->GetContainedRepresentationNames(representationNames);

  bool wasMasterRepresentationModifiedEnabled = segmentationNode->GetSegmentation()->SetMasterRepresentationModifiedEnabled(false);
  segmentation->ClearSegment(segmentId);
  segmentationNode->GetSegmentation()->SetMasterRepresentationModifiedEnabled(wasMasterRepresentationModifiedEnabled);

  std::vector<std::string> segmentIDVector;
  segmentIDVector.push_back(segmentId);
  vtkCjyxSegmentationsModuleLogic::ReconvertAllRepresentations(segmentationNode, segmentIDVector);

  vtkCjyxSegmentationsModuleLogic::SetSegmentStatus(segment, vtkCjyxSegmentationsModuleLogic::NotStarted);
  segmentation->InvokeEvent(vtkSegmentation::RepresentationModified, (void*)segmentId.c_str());
  return true;
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::GetSharedSegmentIDsInMask(
  vtkDMMLSegmentationNode* segmentationNode, std::string sharedSegmentID, vtkOrientedImageData* maskLabelmap, const int extent[6],
  std::vector<std::string>& segmentIDs, int maskThreshold/*=0*/, bool includeInputSegmentID/*=false*/)
{
  segmentIDs.clear();
  if (!segmentationNode)
    {
    vtkErrorWithObjectMacro(nullptr, "Invalid segmentation node!");
    return false;
    }

  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  return vtkSegmentationModifier::GetSharedSegmentIDsInMask(segmentation, sharedSegmentID, maskLabelmap,
    extent, segmentIDs, maskThreshold, includeInputSegmentID);
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::ReconvertAllRepresentations(vtkDMMLSegmentationNode* segmentationNode,
  const std::vector<std::string>& segmentIDs/*={}*/)
{
  if (!segmentationNode)
    {
    vtkErrorWithObjectMacro(nullptr, "Invalid segmentation node!");
    return false;
    }

  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  if (!segmentation)
    {
    vtkErrorWithObjectMacro(nullptr, "Invalid segmentation!");
    return false;
    }

  std::vector<std::string> segmentIDsToConvert = segmentIDs;
  if (segmentIDsToConvert.empty())
    {
    segmentation->GetSegmentIDs(segmentIDsToConvert);
    }

  bool conversionHappened = false;
   std::vector<std::string> representationNames;
  segmentation->GetContainedRepresentationNames(representationNames);

  // Re-convert all other representations
  for (std::vector<std::string>::iterator reprIt = representationNames.begin();
    reprIt != representationNames.end(); ++reprIt)
    {
    std::string targetRepresentationName = (*reprIt);
    if (targetRepresentationName.compare(segmentation->MasterRepresentationName))
      {
      vtkSegmentationConverter::ConversionPathAndCostListType pathCosts;
      segmentation->GetPossibleConversions(targetRepresentationName, pathCosts);

      // Get cheapest path from found conversion paths
      vtkSegmentationConverter::ConversionPathType cheapestPath = vtkSegmentationConverter::GetCheapestPath(pathCosts);
      if (!cheapestPath.empty())
        {
        conversionHappened |= segmentationNode->GetSegmentation()->ConvertSegmentsUsingPath(segmentIDsToConvert, cheapestPath, true);
        }
      }
    }
  return conversionHappened;
}

//-----------------------------------------------------------------------------
void vtkCjyxSegmentationsModuleLogic::CollapseBinaryLabelmaps(vtkDMMLSegmentationNode* segmentationNode, bool forceToSingleLayer)
{
  if (!segmentationNode)
    {
    vtkErrorWithObjectMacro(nullptr, "Invalid segmentation node!");
    return;
    }

  DMMLNodeModifyBlocker blocker(segmentationNode);
  bool wasMasterRepresentationModifiedEnabled = segmentationNode->GetSegmentation()->SetMasterRepresentationModifiedEnabled(false);
  segmentationNode->GetSegmentation()->CollapseBinaryLabelmaps(forceToSingleLayer);
  segmentationNode->GetSegmentation()->SetMasterRepresentationModifiedEnabled(wasMasterRepresentationModifiedEnabled);
  vtkCjyxSegmentationsModuleLogic::ReconvertAllRepresentations(segmentationNode);
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::IsEffectiveExentOutsideReferenceVolume(
  vtkDMMLVolumeNode* referenceVolumeNode, vtkDMMLSegmentationNode* segmentationNode, vtkStringArray* segmentIDs/*=nullptr*/)
{
  if (!referenceVolumeNode)
    {
    vtkGenericWarningMacro("Invalid reference volume node");
    return false;
    }

  if (!segmentationNode)
    {
    vtkGenericWarningMacro("Invalid segmentation node");
    return false;
    }

  std::string segmentationGeometryString = segmentationNode->GetSegmentation()->DetermineCommonLabelmapGeometry(
    vtkSegmentation::EXTENT_UNION_OF_EFFECTIVE_SEGMENTS, segmentIDs);
  vtkNew<vtkOrientedImageData> segmentationGeometry;
  vtkSegmentationConverter::DeserializeImageGeometry(segmentationGeometryString, segmentationGeometry, false/*don't allocate*/);

  vtkNew<vtkMatrix4x4> ijkToRASMatrix;
  referenceVolumeNode->GetIJKToRASMatrix(ijkToRASMatrix);

  vtkNew<vtkOrientedImageData> referenceGeometry;
  referenceGeometry->SetExtent(referenceVolumeNode->GetImageData()->GetExtent());
  referenceGeometry->SetGeometryFromImageToWorldMatrix(ijkToRASMatrix);

  if (segmentationNode->GetParentTransformNode() != referenceVolumeNode->GetParentTransformNode())
    {
    vtkNew<vtkGeneralTransform> segmentationToReferenceTransform;
    vtkDMMLTransformNode::GetTransformBetweenNodes(segmentationNode->GetParentTransformNode(),
      referenceVolumeNode->GetParentTransformNode(), segmentationToReferenceTransform);
    vtkOrientedImageDataResample::TransformOrientedImage(segmentationGeometry, segmentationToReferenceTransform, true/*geometry only*/);
    }

  return vtkCjyxSegmentationsModuleLogic::IsSegmentationExentOutsideReferenceGeometry(referenceGeometry, segmentationGeometry);
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationsModuleLogic::IsSegmentationExentOutsideReferenceGeometry(
  vtkOrientedImageData* referenceGeometry, vtkOrientedImageData* segmentationGeometry)
{
  vtkNew<vtkTransform> segmentationGeometryToReferenceGeometryTransform;
  vtkOrientedImageDataResample::GetTransformBetweenOrientedImages(segmentationGeometry, referenceGeometry, segmentationGeometryToReferenceGeometryTransform);

  int transformedSegmentationExtent[6] = { 0, -1, 0, -1, 0, -1 };
  vtkOrientedImageDataResample::TransformExtent(segmentationGeometry->GetExtent(),
    segmentationGeometryToReferenceGeometryTransform, transformedSegmentationExtent);

  int referenceExtent[6] = { 0, -1, 0, -1, 0, -1 };
  referenceGeometry->GetExtent(referenceExtent);

  for (int i = 0; i < 3; ++i)
    {
    if (transformedSegmentationExtent[2 * i] < referenceExtent[2 * i]
      || transformedSegmentationExtent[2 * i + 1] > referenceExtent[2 * i + 1])
      {
      return true;
      }
    }
  return false;
}
