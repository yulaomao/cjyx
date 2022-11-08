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
#include "vtkDMMLSegmentationNode.h"
#include "vtkDMMLSegmentationDisplayNode.h"
#include "vtkDMMLSegmentationStorageNode.h"

// SegmentationCore includes
#include "vtkOrientedImageData.h"
#include "vtkOrientedImageDataResample.h"
#include "vtkCalculateOversamplingFactor.h"

// DMML includes
#include <vtkEventBroker.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLTransformNode.h>
#include <vtkDMMLStorageNode.h>
#include <vtkDMMLSubjectHierarchyNode.h>
#include <vtkDMMLScalarVolumeNode.h>

// VTK includes
#include <vtkBoundingBox.h>
#include <vtkCallbackCommand.h>
#include <vtkGeneralTransform.h>
#include <vtkHomogeneousTransform.h>
#include <vtkImageCast.h>
#include <vtkImageThreshold.h>
#include <vtkIntArray.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>

// vtkITK includes
#include <vtkITKIslandMath.h>

// STD includes
#include <algorithm>

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLSegmentationNode);

//----------------------------------------------------------------------------
vtkDMMLSegmentationNode::vtkDMMLSegmentationNode()
{
  this->SegmentationModifiedCallbackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
  this->SegmentationModifiedCallbackCommand->SetClientData(reinterpret_cast<void *>(this));
  this->SegmentationModifiedCallbackCommand->SetCallback(vtkDMMLSegmentationNode::SegmentationModifiedCallback);

  this->SegmentCenterTmp[0] = 0.0;
  this->SegmentCenterTmp[1] = 0.0;
  this->SegmentCenterTmp[2] = 0.0;
  this->SegmentCenterTmp[3] = 1.0;

  this->SegmentListFilterEnabled = false;

  // Create empty segmentations object
  this->Segmentation = nullptr;
  vtkSmartPointer<vtkSegmentation> segmentation = vtkSmartPointer<vtkSegmentation>::New();
  this->SetAndObserveSegmentation(segmentation);

  this->ContentModifiedEvents->InsertNextValue(vtkSegmentation::MasterRepresentationModified);
  this->ContentModifiedEvents->InsertNextValue(vtkSegmentation::ContainedRepresentationNamesModified);
  this->ContentModifiedEvents->InsertNextValue(vtkSegmentation::SegmentAdded);
  this->ContentModifiedEvents->InsertNextValue(vtkSegmentation::SegmentRemoved);
  this->ContentModifiedEvents->InsertNextValue(vtkSegmentation::SegmentModified);
  this->ContentModifiedEvents->InsertNextValue(vtkSegmentation::SegmentsOrderModified);

  this->AddNodeReferenceRole(this->GetLabelmapConversionColorTableNodeReferenceRole(),
    this->GetLabelmapConversionColorTableNodeReferenceDMMLAttributeName());
}

//----------------------------------------------------------------------------
vtkDMMLSegmentationNode::~vtkDMMLSegmentationNode()
{
  this->SetAndObserveSegmentation(nullptr);

  // Make sure this callback cannot call this object
  this->SegmentationModifiedCallbackCommand->SetClientData(nullptr);
}

//----------------------------------------------------------------------------
void vtkDMMLSegmentationNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  if (this->Segmentation)
    {
    this->Segmentation->WriteXML(of, nIndent);
    }
  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLBooleanMacro(segmentListFilterEnabled, SegmentListFilterEnabled);
  vtkDMMLWriteXMLStdStringMacro(segmentListFilterOptions, SegmentListFilterOptions);
  vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLSegmentationNode::ReadXMLAttributes(const char** atts)
{
  // Read all DMML node attributes from two arrays of names and values
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  if (!this->Segmentation)
    {
    vtkSmartPointer<vtkSegmentation> segmentation = vtkSmartPointer<vtkSegmentation>::New();
    this->SetAndObserveSegmentation(segmentation);
    }
  this->Segmentation->ReadXMLAttributes(atts);
  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLBooleanMacro(segmentListFilterEnabled, SegmentListFilterEnabled);
  vtkDMMLReadXMLStdStringMacro(segmentListFilterOptions, SegmentListFilterOptions);
  vtkDMMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLSegmentationNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkDMMLSegmentationNode* node = vtkDMMLSegmentationNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }
  if (deepCopy)
    {
    if (node->GetSegmentation())
      {
      if (this->GetSegmentation())
        {
        this->GetSegmentation()->DeepCopy(node->GetSegmentation());
        }
      else
        {
        vtkSmartPointer<vtkSegmentation> newSegmentation
          = vtkSmartPointer<vtkSegmentation>::Take(node->GetSegmentation()->NewInstance());
        newSegmentation->DeepCopy(node->GetSegmentation());
        this->SetAndObserveSegmentation(newSegmentation);
        }
      }
    else
      {
      // input was nullptr
      this->SetAndObserveSegmentation(nullptr);
      }
    }
  else
    {
    // shallow-copy
    this->SetAndObserveSegmentation(node->GetSegmentation());
    }
  vtkDMMLCopyBeginMacro(anode);
  vtkDMMLCopyBooleanMacro(SegmentListFilterEnabled);
  vtkDMMLCopyStdStringMacro(SegmentListFilterOptions);
  vtkDMMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLSegmentationNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "Segmentation:";
  if (this->Segmentation)
    {
    this->Segmentation->PrintSelf(os, indent.GetNextIndent());
    }
  else
    {
    os << " (invalid)\n";
    }
  vtkDMMLPrintBeginMacro(os, indent);
  vtkDMMLPrintBooleanMacro(SegmentListFilterEnabled);
  vtkDMMLPrintStdStringMacro(SegmentListFilterOptions);
  vtkDMMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLSegmentationNode::SetAndObserveSegmentation(vtkSegmentation* segmentation)
{
  if (segmentation == this->Segmentation)
    {
    return;
    }

  // Remove segment event observations from previous segmentation
  if (this->Segmentation)
    {
    vtkEventBroker::GetInstance()->RemoveObservations(
      this->Segmentation, 0, this, this->SegmentationModifiedCallbackCommand);
    }

  this->SetSegmentation(segmentation);

  // Observe segment events in new segmentation
  if (this->Segmentation)
    {
    vtkEventBroker::GetInstance()->AddObservation(
      this->Segmentation, vtkSegmentation::MasterRepresentationModified, this, this->SegmentationModifiedCallbackCommand);
    vtkEventBroker::GetInstance()->AddObservation(
      this->Segmentation, vtkSegmentation::SegmentAdded, this, this->SegmentationModifiedCallbackCommand);
    vtkEventBroker::GetInstance()->AddObservation(
      this->Segmentation, vtkSegmentation::SegmentRemoved, this, this->SegmentationModifiedCallbackCommand);
    vtkEventBroker::GetInstance()->AddObservation(
      this->Segmentation, vtkSegmentation::SegmentModified, this, this->SegmentationModifiedCallbackCommand);
    vtkEventBroker::GetInstance()->AddObservation(
      this->Segmentation, vtkSegmentation::ContainedRepresentationNamesModified, this, this->SegmentationModifiedCallbackCommand);
    vtkEventBroker::GetInstance()->AddObservation(
      this->Segmentation, vtkSegmentation::RepresentationModified, this, this->SegmentationModifiedCallbackCommand);
    vtkEventBroker::GetInstance()->AddObservation(
      this->Segmentation, vtkSegmentation::SegmentsOrderModified, this, this->SegmentationModifiedCallbackCommand);
    }

  this->InvokeCustomModifiedEvent(vtkDMMLSegmentationNode::SegmentationChangedEvent);
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationNode::SegmentationModifiedCallback(vtkObject* vtkNotUsed(caller), unsigned long eid, void* clientData, void* callData)
{
  vtkDMMLSegmentationNode* self = reinterpret_cast<vtkDMMLSegmentationNode*>(clientData);
  if (!self)
    {
    return;
    }
  if (!self->Segmentation)
    {
    // this object is being deleted
    return;
    }
  switch (eid)
    {
    case vtkSegmentation::MasterRepresentationModified:
      self->OnMasterRepresentationModified();
      self->InvokeCustomModifiedEvent(eid, callData);
      break;
    case vtkSegmentation::RepresentationModified:
      self->StorableModifiedTime.Modified();
      self->InvokeCustomModifiedEvent(eid, callData);
      break;
    case vtkSegmentation::ContainedRepresentationNamesModified:
      self->StorableModifiedTime.Modified();
      self->InvokeCustomModifiedEvent(eid);
      break;
    case vtkSegmentation::SegmentAdded:
      self->StorableModifiedTime.Modified();
      self->OnSegmentAdded(reinterpret_cast<char*>(callData));
      self->InvokeCustomModifiedEvent(eid, callData);
      break;
    case vtkSegmentation::SegmentRemoved:
      self->StorableModifiedTime.Modified();
      self->OnSegmentRemoved(reinterpret_cast<char*>(callData));
      self->InvokeCustomModifiedEvent(eid, callData);
      break;
    case vtkSegmentation::SegmentModified:
      self->StorableModifiedTime.Modified();
      self->OnSegmentModified(reinterpret_cast<char*>(callData));
      self->InvokeCustomModifiedEvent(eid, callData);
      break;
    case vtkSegmentation::SegmentsOrderModified:
      self->StorableModifiedTime.Modified();
      self->InvokeCustomModifiedEvent(eid);
      break;
    default:
      vtkErrorWithObjectMacro(self, "vtkDMMLSegmentationNode::SegmentationModifiedCallback: Unknown event id "<<eid);
      return;
    }
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationNode::OnMasterRepresentationModified()
{
  // Reset supported write file types
  vtkDMMLSegmentationStorageNode* storageNode =  vtkDMMLSegmentationStorageNode::SafeDownCast(this->GetStorageNode());
  if (storageNode)
    {
    storageNode->ResetSupportedWriteFileTypes();
    }
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationNode::OnSegmentAdded(const char* vtkNotUsed(segmentId))
{
  vtkDMMLSegmentationDisplayNode* displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(this->GetDisplayNode());
  if (displayNode)
    {
    // Make sure the properties of the new segment are as expected even before the first update is triggered (e.g. by slice controller widget).
    // removeUnusedDisplayProperties is set to false to prevent removing of display properties of segments
    // that are not added to the segmentation node yet (this occurs during scene loading).
    displayNode->UpdateSegmentList(false);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationNode::OnSegmentRemoved(const char* vtkNotUsed(segmentId))
{
  vtkDMMLSegmentationDisplayNode* displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(this->GetDisplayNode());
  if (displayNode)
    {
    // Make sure the segment is removed from the display properties as well, so that when a new segment is added
    // in its place it is properly populated (it will have the same segment ID, so it would simply claim it)
    displayNode->UpdateSegmentList();
    }
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationNode::OnSegmentModified(const char* vtkNotUsed(segmentId))
{
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationNode::OnSubjectHierarchyUIDAdded(
  vtkDMMLSubjectHierarchyNode* shNode, vtkIdType itemWithNewUID )
{
  if (!shNode || !this->Segmentation || !itemWithNewUID)
    {
    return;
    }
  // If already has geometry, then do not look for a new one
  if (!this->Segmentation->GetConversionParameter(vtkSegmentationConverter::GetReferenceImageGeometryParameterName()).empty())
    {
    return;
    }
  // If the new UID is empty string, then do not look for the segmentation's referenced UID in its UID list
  std::string itemUidValueStr = shNode->GetItemUID(itemWithNewUID, vtkDMMLSubjectHierarchyConstants::GetDICOMInstanceUIDName());
  if (itemUidValueStr.empty())
    {
    return;
    }

  // Get volume node from subject hierarchy item with new UID
  vtkDMMLScalarVolumeNode* referencedVolumeNode = vtkDMMLScalarVolumeNode::SafeDownCast(
    shNode->GetItemDataNode(itemWithNewUID) );
  if (!referencedVolumeNode)
    {
    // If associated node is not a volume, then return
    return;
    }

  // Get associated subject hierarchy item
  vtkIdType segmentationShItemID = shNode->GetItemByDataNode(this);
  if (!segmentationShItemID)
    {
    // If segmentation is not in subject hierarchy, then we cannot find its DICOM references
    return;
    }

  // Get DICOM references from segmentation subject hierarchy node
  std::string referencedInstanceUIDsAttribute = shNode->GetItemAttribute(
    segmentationShItemID, vtkDMMLSubjectHierarchyConstants::GetDICOMReferencedInstanceUIDsAttributeName() );
  if (referencedInstanceUIDsAttribute.empty())
    {
    // No references
    return;
    }

  // If the subject hierarchy node that got a new UID has a DICOM instance UID referenced
  // from this segmentation, then use its geometry as image geometry conversion parameter
  std::vector<std::string> referencedSopInstanceUids;
  vtkDMMLSubjectHierarchyNode::DeserializeUIDList(referencedInstanceUIDsAttribute, referencedSopInstanceUids);
  bool referencedVolumeFound = false;
  bool warningLogged = false;
  std::vector<std::string>::iterator uidIt;
  for (uidIt = referencedSopInstanceUids.begin(); uidIt != referencedSopInstanceUids.end(); ++uidIt)
    {
    // If we find the instance UID, then we set the geometry
    if (itemUidValueStr.find(*uidIt) != std::string::npos)
      {
      // Only set the reference once, but check all UIDs
      if (!referencedVolumeFound)
        {
        // Set reference image geometry parameter if volume node is found
        this->SetReferenceImageGeometryParameterFromVolumeNode(referencedVolumeNode);
        referencedVolumeFound = true;
        }
      }
    // If referenced UID is not contained in found node, then warn user
    else if (referencedVolumeFound && !warningLogged)
      {
      vtkWarningMacro("vtkDMMLSegmentationNode::OnSubjectHierarchyUIDAdded: Referenced volume for segmentation '"
        << this->Name << "' found (" << referencedVolumeNode->GetName() << "), but some referenced UIDs are not present in it! (maybe only partial volume was loaded?)");
      // Only log warning once for this node
      warningLogged = true;
      }
    }
}

//----------------------------------------------------------------------------
vtkDMMLStorageNode* vtkDMMLSegmentationNode::CreateDefaultStorageNode()
{
  vtkDMMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkDMMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkDMMLSegmentationStorageNode"));
}

//----------------------------------------------------------------------------
void vtkDMMLSegmentationNode::CreateDefaultDisplayNodes()
{
  if (vtkDMMLSegmentationDisplayNode::SafeDownCast(this->GetDisplayNode()))
    {
    // Display node already exists
    return;
    }
  if (this->GetScene() == nullptr)
    {
    vtkErrorMacro("vtkDMMLSegmentationNode::CreateDefaultDisplayNodes failed: Scene is invalid");
    return;
    }
  vtkDMMLSegmentationDisplayNode* displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(
    this->GetScene()->AddNewNodeByClass("vtkDMMLSegmentationDisplayNode") );
  this->SetAndObserveDisplayNodeID(displayNode->GetID());
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationNode::ApplyTransformMatrix(vtkMatrix4x4* transformMatrix)
{
  vtkTransform* transform = vtkTransform::New();
  transform->SetMatrix(transformMatrix);
  this->ApplyTransform(transform);
  transform->Delete();
}

//----------------------------------------------------------------------------
void vtkDMMLSegmentationNode::ApplyTransform(vtkAbstractTransform* transform)
{
  if (!this->Segmentation)
    {
    return;
    }

  // Apply transform on segmentation
  bool wasEnabled = this->Segmentation->SetMasterRepresentationModifiedEnabled(false);
  vtkSmartPointer<vtkTransform> linearTransform = vtkSmartPointer<vtkTransform>::New();
  if (vtkOrientedImageDataResample::IsTransformLinear(transform, linearTransform))
    {
    this->Segmentation->ApplyLinearTransform(transform);
    }
  else
    {
    this->Segmentation->ApplyNonLinearTransform(transform);
    }
  this->Segmentation->SetMasterRepresentationModifiedEnabled(wasEnabled);
  this->Segmentation->InvalidateNonMasterRepresentations();

  // Make sure preferred display representations exist after transformation
  // (it is invalidated in the process unless it is the master representation)
  char* preferredDisplayRepresentation2D = nullptr;
  char* preferredDisplayRepresentation3D = nullptr;
  vtkDMMLSegmentationDisplayNode* displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(this->GetDisplayNode());
  if (displayNode)
    {
    preferredDisplayRepresentation2D = displayNode->GetPreferredDisplayRepresentationName2D();
    preferredDisplayRepresentation3D = displayNode->GetPreferredDisplayRepresentationName3D();
    }

  // Make sure preferred display representations exist after transformation
  // (it was invalidated in the process unless it is the master representation)
  if (displayNode)
    {
    if (preferredDisplayRepresentation2D)
      {
      this->Segmentation->CreateRepresentation(preferredDisplayRepresentation2D);
      }
    if (preferredDisplayRepresentation3D)
      {
      this->Segmentation->CreateRepresentation(preferredDisplayRepresentation3D);
      }
    // Need to set preferred representations again, as conversion sets them to the last converted one
    displayNode->SetPreferredDisplayRepresentationName2D(preferredDisplayRepresentation2D);
    displayNode->SetPreferredDisplayRepresentationName3D(preferredDisplayRepresentation3D);
    }
}

//----------------------------------------------------------------------------
bool vtkDMMLSegmentationNode::CanApplyNonLinearTransforms() const
{
  return true;
}

//---------------------------------------------------------------------------
// Global RAS in the form (Xmin, Xmax, Ymin, Ymax, Zmin, Zmax)
//---------------------------------------------------------------------------
void vtkDMMLSegmentationNode::GetRASBounds(double bounds[6])
{
  if (this->GetParentTransformNode() == nullptr)
    {
    // Segmentation is not transformed
    this->GetBounds(bounds);
    }
  else
    {
    // Segmentation is transformed
    vtkNew<vtkGeneralTransform> segmentationToRASTransform;
    vtkDMMLTransformNode::GetTransformBetweenNodes(this->GetParentTransformNode(), nullptr, segmentationToRASTransform.GetPointer());
    double bounds_Segmentation[6] = { 1, -1, 1, -1, 1, -1 };
    this->GetBounds(bounds_Segmentation);
    vtkOrientedImageDataResample::TransformBounds(bounds_Segmentation, segmentationToRASTransform.GetPointer(), bounds);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationNode::GetBounds(double bounds[6])
{
  vtkMath::UninitializeBounds(bounds);
  if (this->Segmentation)
    {
    this->Segmentation->GetBounds(bounds);
    }
}

//---------------------------------------------------------------------------
bool vtkDMMLSegmentationNode::GenerateMergedLabelmap(
  vtkOrientedImageData* mergedImageData,
  int extentComputationMode,
  vtkOrientedImageData* mergedLabelmapGeometry/*=nullptr*/,
  const std::vector<std::string>& segmentIDs/*=std::vector<std::string>()*/,
  vtkIntArray* labelValues/*=nullptr*/
  )
{
  return this->Segmentation->GenerateMergedLabelmap(mergedImageData, extentComputationMode, mergedLabelmapGeometry, segmentIDs, labelValues);
}

//---------------------------------------------------------------------------
bool vtkDMMLSegmentationNode::GenerateMergedLabelmapForAllSegments(vtkOrientedImageData* mergedImageData, int extentComputationMode,
  vtkOrientedImageData* mergedLabelmapGeometry /*=nullptr*/, vtkStringArray* segmentIDs /*=nullptr*/, vtkIntArray* labelValues/*nullptr*/)
{
  std::vector<std::string> segmentIDsVector;
  if (segmentIDs)
    {
    for (int i = 0; i < segmentIDs->GetNumberOfValues(); i++)
      {
      segmentIDsVector.push_back(segmentIDs->GetValue(i));
      }
    }
  return this->GenerateMergedLabelmap(mergedImageData, extentComputationMode, mergedLabelmapGeometry, segmentIDsVector, labelValues);
}

//-----------------------------------------------------------------------------
bool vtkDMMLSegmentationNode::GenerateEditMask(vtkOrientedImageData* maskImage, int editMode,
  vtkOrientedImageData* referenceGeometry,
  std::string editedSegmentID/*=""*/, std::string maskSegmentID/*=""*/,
  vtkOrientedImageData* masterVolume/*=nullptr*/, double editableIntensityRange[2]/*=nullptr*/,
  vtkDMMLSegmentationDisplayNode* displayNode/*=nullptr*/)
{
  if (!maskImage)
    {
    vtkErrorMacro("vtkDMMLSegmentationNode::GenerateEditMask: Invalid input mask image");
    return false;
    }
  int extent[6] = { 0, -1, 0, -1, 0, -1 };
  maskImage->SetExtent(extent);

  if (!referenceGeometry)
    {
    vtkErrorMacro("vtkDMMLSegmentationNode::GenerateEditMask: Invalid reference geometry");
    return false;
    }
  referenceGeometry->GetExtent(extent);
  if (extent[0] > extent[1]
    || extent[2] > extent[3]
    || extent[4] > extent[5])
    {
    // input reference geometry is empty, so we don't need to generate a mask
    return true;
    }

  std::vector<std::string> allSegmentIDs;
  this->GetSegmentation()->GetSegmentIDs(allSegmentIDs);

  std::vector<std::string> visibleSegmentIDs;
  if (editMode == vtkDMMLSegmentationNode::EditAllowedInsideVisibleSegments
    || editMode == vtkDMMLSegmentationNode::EditAllowedOutsideVisibleSegments)
    {
    if (!displayNode)
      {
      displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(this->GetDisplayNode());
      }
    if (displayNode)
      {
      for (std::vector<std::string>::iterator segmentIDIt = allSegmentIDs.begin(); segmentIDIt != allSegmentIDs.end(); ++segmentIDIt)
        {
        if (displayNode->GetSegmentVisibility(*segmentIDIt))
          {
          visibleSegmentIDs.push_back(*segmentIDIt);
          }
        }
      }
    else
      {
      vtkErrorMacro("vtkDMMLSegmentationNode::GenerateEditMask: Could not find valid display node");
      return false;
      }
    }

  std::vector<std::string> maskSegmentIDs;
  bool editInsideSegments = false;
  switch (editMode)
    {
  case vtkDMMLSegmentationNode::EditAllowedEverywhere:
    editInsideSegments = false;
    break;
  case vtkDMMLSegmentationNode::EditAllowedInsideAllSegments:
    editInsideSegments = true;
    maskSegmentIDs = allSegmentIDs;
    break;
  case vtkDMMLSegmentationNode::EditAllowedInsideVisibleSegments:
    editInsideSegments = true;
    maskSegmentIDs = visibleSegmentIDs;
    break;
  case vtkDMMLSegmentationNode::EditAllowedOutsideAllSegments:
    editInsideSegments = false;
    maskSegmentIDs = allSegmentIDs;
    break;
  case vtkDMMLSegmentationNode::EditAllowedOutsideVisibleSegments:
    editInsideSegments = false;
    maskSegmentIDs = visibleSegmentIDs;
    break;
  case vtkDMMLSegmentationNode::EditAllowedInsideSingleSegment:
    editInsideSegments = true;
    if (!maskSegmentID.empty())
      {
      maskSegmentIDs.push_back(maskSegmentID);
      }
    else
      {
      vtkWarningMacro("vtkDMMLSegmentationNode::GenerateEditMask: EditAllowedInsideSingleSegment selected but no mask segment is specified");
      }
    break;
  default:
    vtkErrorMacro("vtkDMMLSegmentationNode::GenerateEditMask: unknown mask mode");
    return false;
    }

  if (!editInsideSegments)
    {
    // Exclude edited segment from "outside" mask
    maskSegmentIDs.erase(std::remove(maskSegmentIDs.begin(), maskSegmentIDs.end(), editedSegmentID), maskSegmentIDs.end());
    }

  maskImage->SetExtent(extent);
  vtkNew<vtkMatrix4x4> referenceImageToWorldMatrix;
  referenceGeometry->GetImageToWorldMatrix(referenceImageToWorldMatrix.GetPointer());
  maskImage->SetImageToWorldMatrix(referenceImageToWorldMatrix.GetPointer());

  if (maskSegmentIDs.empty())
    {
    // If we passed empty segment list to GenerateSharedLabelmap then it would use all segment IDs,
    // instead of filling the volume with a single value. Therefore, we need to handle this special case separately here.
    maskImage->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
    vtkOrientedImageDataResample::FillImage(maskImage, editInsideSegments ? 1 : 0);
    }
  else
    {
    maskImage->AllocateScalars(VTK_SHORT, 1); // Change scalar type from unsigned int back to short for shared labelmap generation
    this->GenerateMergedLabelmap(maskImage, vtkSegmentation::EXTENT_UNION_OF_SEGMENTS, referenceGeometry, maskSegmentIDs);

    vtkNew<vtkImageThreshold> threshold;
    threshold->SetInputData(maskImage);
    threshold->SetInValue(editInsideSegments ? 1 : 0);
    threshold->SetOutValue(editInsideSegments ? 0 : 1);
    threshold->ReplaceInOn();
    threshold->ThresholdByLower(0);
    threshold->SetOutputScalarType(VTK_UNSIGNED_CHAR);
    threshold->Update();
    maskImage->DeepCopy(threshold->GetOutput());
    maskImage->SetImageToWorldMatrix(referenceImageToWorldMatrix.GetPointer());
    }

  // Apply threshold mask if paint threshold is turned on
  if (masterVolume != nullptr && editableIntensityRange != nullptr)
    {
    // Create threshold image
    vtkNew<vtkImageThreshold> threshold;
    threshold->SetInputData(masterVolume);
    threshold->ThresholdBetween(editableIntensityRange[0], editableIntensityRange[1]);
    threshold->SetInValue(1);
    threshold->SetOutValue(0);
    threshold->SetOutputScalarType(VTK_UNSIGNED_CHAR);
    threshold->Update();
    vtkNew<vtkOrientedImageData> thresholdMask; //  == 0 in editable region
    thresholdMask->ShallowCopy(threshold->GetOutput());
    vtkNew<vtkMatrix4x4> masterVolumeToWorldMatrix;
    masterVolume->GetImageToWorldMatrix(masterVolumeToWorldMatrix.GetPointer());
    thresholdMask->SetImageToWorldMatrix(masterVolumeToWorldMatrix.GetPointer());

    if (!vtkOrientedImageDataResample::ApplyImageMask(maskImage, thresholdMask.GetPointer(), 1))
      {
      vtkErrorMacro("vtkDMMLSegmentationNode::GenerateEditMask: failed to apply intensity mask");
      return false;
      }
    }

  return true;
}

//---------------------------------------------------------------------------
vtkIdType vtkDMMLSegmentationNode::GetSegmentSubjectHierarchyItem(std::string segmentID, vtkDMMLSubjectHierarchyNode* shNode)
{
  if (!shNode)
    {
    vtkErrorMacro("GetSegmentSubjectHierarchyItem: Invalid subject hierarchy");
    return vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }

  vtkIdType segmentationvtkIdType = shNode->GetItemByDataNode(this);
  if (!segmentationvtkIdType)
    {
    return vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }

  // Find child item of segmentation subject hierarchy item that has the requested segment ID
  std::vector<vtkIdType> segmentationChildItemIDs;
  shNode->GetItemChildren(segmentationvtkIdType, segmentationChildItemIDs, false);

  std::vector<vtkIdType>::iterator childIt;
  for (childIt=segmentationChildItemIDs.begin(); childIt!=segmentationChildItemIDs.end(); ++childIt)
    {
    vtkIdType childItemID = (*childIt);
    std::string childSegmentID = shNode->GetItemAttribute(childItemID, vtkDMMLSegmentationNode::GetSegmentIDAttributeName());
    if (!childSegmentID.empty() && !childSegmentID.compare(segmentID))
      {
      return childItemID;
      }
    }

  return vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID;
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationNode::SetReferenceImageGeometryParameterFromVolumeNode(vtkDMMLScalarVolumeNode* volumeNode)
{
  if (!volumeNode || !volumeNode->GetImageData())
    {
    return;
    }
  if (!this->Segmentation)
    {
    vtkSmartPointer<vtkSegmentation> segmentation = vtkSmartPointer<vtkSegmentation>::New();
    this->SetAndObserveSegmentation(segmentation);
    }

  // Get serialized geometry of selected volume
  vtkSmartPointer<vtkMatrix4x4> volumeIjkToRasMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  volumeNode->GetIJKToRASMatrix(volumeIjkToRasMatrix);

  // If there is a linear transform between the reference volume and segmentation then transform the geometry
  // to be aligned with the reference volume.
  if (volumeNode->GetParentTransformNode() != this->GetParentTransformNode())
    {
    vtkSmartPointer<vtkGeneralTransform> volumeToSegmentationTransform = vtkSmartPointer<vtkGeneralTransform>::New();
    vtkDMMLTransformNode::GetTransformBetweenNodes(volumeNode->GetParentTransformNode(), this->GetParentTransformNode(), volumeToSegmentationTransform);
    if (vtkDMMLTransformNode::IsGeneralTransformLinear(volumeToSegmentationTransform))
      {
      vtkNew<vtkMatrix4x4> volumeToSegmentationMatrix;
      vtkDMMLTransformNode::GetMatrixTransformBetweenNodes(volumeNode->GetParentTransformNode(), this->GetParentTransformNode(), volumeToSegmentationMatrix.GetPointer());
      vtkMatrix4x4::Multiply4x4(volumeToSegmentationMatrix.GetPointer(), volumeIjkToRasMatrix, volumeIjkToRasMatrix);
      }
    }

  std::string serializedImageGeometry = vtkSegmentationConverter::SerializeImageGeometry(
    volumeIjkToRasMatrix, volumeNode->GetImageData() );

  // Set parameter
  this->Segmentation->SetConversionParameter(
    vtkSegmentationConverter::GetReferenceImageGeometryParameterName(), serializedImageGeometry);

  // Set node reference from segmentation to reference geometry volume
  this->SetNodeReferenceID(
    vtkDMMLSegmentationNode::GetReferenceImageGeometryReferenceRole().c_str(), volumeNode->GetID() );
}

//---------------------------------------------------------------------------
std::string vtkDMMLSegmentationNode::AddSegmentFromClosedSurfaceRepresentation(vtkPolyData* polyData,
  std::string segmentName/* ="" */, double color[3] /* =nullptr */,
  std::string vtkNotUsed(segmentId)/* ="" */)
{
  if (!this->Segmentation)
    {
    vtkErrorMacro("AddSegmentFromClosedSurfaceRepresentation: Invalid segmentation");
    return "";
    }
  vtkNew<vtkSegment> newSegment;
  if (!segmentName.empty())
    {
    newSegment->SetName(segmentName.c_str());
    }
  if (color!=nullptr)
    {
    newSegment->SetColor(color);
    }
  newSegment->AddRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName(), polyData);
  if (!this->Segmentation->AddSegment(newSegment.GetPointer()))
    {
    return "";
    }
  return this->Segmentation->GetSegmentIdBySegment(newSegment.GetPointer());
}

//---------------------------------------------------------------------------
std::string vtkDMMLSegmentationNode::AddSegmentFromBinaryLabelmapRepresentation(vtkOrientedImageData* imageData,
  std::string segmentName/* ="" */, double color[3] /* =nullptr */,
  std::string vtkNotUsed(segmentId)/* ="" */)
{
  if (!this->Segmentation)
    {
    vtkErrorMacro("AddSegmentFromBinaryLabelmapRepresentation: Invalid segmentation");
    return "";
    }
  vtkNew<vtkSegment> newSegment;
  if (!segmentName.empty())
    {
    newSegment->SetName(segmentName.c_str());
    }
  if (color != nullptr)
    {
    newSegment->SetColor(color);
    }
  newSegment->AddRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName(), imageData);
  if (!this->Segmentation->AddSegment(newSegment.GetPointer()))
    {
    return "";
    }
  return this->Segmentation->GetSegmentIdBySegment(newSegment.GetPointer());
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationNode::RemoveSegment(const std::string& segmentID)
{
  if (!this->Segmentation)
    {
    vtkErrorMacro("RemoveSegment: Invalid segmentation");
    return;
    }
  this->Segmentation->RemoveSegment(segmentID);
}

//---------------------------------------------------------------------------
bool vtkDMMLSegmentationNode::CreateBinaryLabelmapRepresentation()
{
  if (!this->Segmentation)
    {
    vtkErrorMacro("CreateBinaryLabelmapRepresentation: Invalid segmentation");
    return false;
    }
  return this->Segmentation->CreateRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationNode::RemoveBinaryLabelmapRepresentation()
{
  if (!this->Segmentation)
    {
    vtkErrorMacro("RemoveBinaryLabelmapRepresentation: Invalid segmentation");
    return;
    }
  this->Segmentation->RemoveRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
}

//---------------------------------------------------------------------------
bool vtkDMMLSegmentationNode::GetBinaryLabelmapRepresentation(const std::string segmentId, vtkOrientedImageData* outputBinaryLabelmap)
{
  if (!this->Segmentation)
    {
    vtkErrorMacro("GetBinaryLabelmapRepresentation: Invalid segmentation");
    return false;
    }
  vtkSegment* segment = this->Segmentation->GetSegment(segmentId);
  if (!segment)
    {
    vtkErrorMacro("GetBinaryLabelmapRepresentation: Invalid segment");
    return false;
    }

  vtkOrientedImageData* binaryLabelmap = vtkOrientedImageData::SafeDownCast(
    segment->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()));
  if (!binaryLabelmap)
    {
    vtkErrorMacro("GetBinaryLabelmapRepresentation: No binary labelmap representation in segment");
    return false;
    }

  vtkNew<vtkImageThreshold> threshold;
  threshold->SetInputData(binaryLabelmap);
  threshold->ThresholdBetween(segment->GetLabelValue(), segment->GetLabelValue());
  threshold->SetInValue(1);
  threshold->SetOutValue(0);
  threshold->Update();

  outputBinaryLabelmap->ShallowCopy(threshold->GetOutput());
  outputBinaryLabelmap->CopyDirections(binaryLabelmap);
  return true;
}

//---------------------------------------------------------------------------
vtkOrientedImageData* vtkDMMLSegmentationNode::GetBinaryLabelmapInternalRepresentation(const std::string segmentId)
{
  if (!this->Segmentation)
    {
    vtkErrorMacro("GetBinaryLabelmapRepresentation: Invalid segmentation");
    return nullptr;
    }
  vtkSegment* segment = this->Segmentation->GetSegment(segmentId);
  if (!segment)
    {
    vtkErrorMacro("GetBinaryLabelmapRepresentation: Invalid segment");
    return nullptr;
    }
  return vtkOrientedImageData::SafeDownCast(segment->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()));
}

//---------------------------------------------------------------------------
bool vtkDMMLSegmentationNode::CreateClosedSurfaceRepresentation()
{
  if (!this->Segmentation)
    {
    vtkErrorMacro("CreateClosedSurfaceRepresentation: Invalid segmentation");
    return false;
    }
  return this->Segmentation->CreateRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationNode::RemoveClosedSurfaceRepresentation()
{
  if (!this->Segmentation)
    {
    vtkErrorMacro("RemoveClosedSurfaceRepresentation: Invalid segmentation");
    return;
    }
  this->Segmentation->RemoveRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
}

//---------------------------------------------------------------------------
bool vtkDMMLSegmentationNode::GetClosedSurfaceRepresentation(const std::string segmentId, vtkPolyData* outputClosedSurface)
{
  if (!this->Segmentation)
    {
    vtkErrorMacro("GetClosedSurfaceRepresentation: Invalid segmentation");
    return false;
    }
  vtkSegment* segment = this->Segmentation->GetSegment(segmentId);
  if (!segment)
    {
    vtkErrorMacro("GetClosedSurfaceRepresentation: Invalid segment");
    return false;
    }
  vtkDataObject* closedSurface = segment->GetRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
  if (!closedSurface)
    {
    vtkErrorMacro("GetClosedSurfaceRepresentation: No closed surface representation in segment");
    return false;
    }
  outputClosedSurface->DeepCopy(closedSurface);
  return true;
}

//---------------------------------------------------------------------------
vtkPolyData* vtkDMMLSegmentationNode::GetClosedSurfaceInternalRepresentation(const std::string segmentId)
{
  if (!this->Segmentation)
    {
    vtkErrorMacro("GetClosedSurfaceRepresentation: Invalid segmentation");
    return nullptr;
    }
  vtkSegment* segment = this->Segmentation->GetSegment(segmentId);
  if (!segment)
    {
    vtkErrorMacro("GetClosedSurfaceRepresentation: Invalid segment");
    return nullptr;
    }
  return vtkPolyData::SafeDownCast(segment->GetRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName()));
}

//---------------------------------------------------------------------------
bool vtkDMMLSegmentationNode::SetMasterRepresentationToBinaryLabelmap()
{
  if (!this->Segmentation)
    {
    vtkErrorMacro("SetMasterRepresentationToBinaryLabelmap: Invalid segmentation");
    return false;
    }
  this->Segmentation->SetMasterRepresentationName(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
  return true;
}

//---------------------------------------------------------------------------
bool vtkDMMLSegmentationNode::SetMasterRepresentationToClosedSurface()
{
  if (!this->Segmentation)
    {
    vtkErrorMacro("SetMasterRepresentationToClosedSurface: Invalid segmentation");
    return false;
    }
  this->Segmentation->SetMasterRepresentationName(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
  return true;
}

//---------------------------------------------------------------------------
double* vtkDMMLSegmentationNode::GetSegmentCenter(const std::string& segmentID)
{
  if (!this->Segmentation)
    {
    vtkErrorMacro("GetSegmentCenter: Invalid segmentation");
    return nullptr;
    }
  vtkSegment* currentSegment = this->Segmentation->GetSegment(segmentID);
  if (!currentSegment)
    {
    vtkErrorMacro("GetSegmentCenter: Segment not found: " << segmentID);
    return nullptr;
    }

  if (this->Segmentation->ContainsRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName()))
    {
    int labelOrientedImageDataEffectiveExtent[6] = { 0, -1, 0, -1, 0, -1 };
    vtkNew<vtkOrientedImageData> labelmap;
    this->GetBinaryLabelmapRepresentation(segmentID, labelmap);
    if (!vtkOrientedImageDataResample::CalculateEffectiveExtent(labelmap, labelOrientedImageDataEffectiveExtent))
      {
      vtkWarningMacro("GetSegmentCenter: segment " << segmentID << " is empty");
      return nullptr;
      }

    // Labelmap image to world
    vtkNew<vtkMatrix4x4> imageToWorldMatrix;
    labelmap->GetImageToWorldMatrix(imageToWorldMatrix.GetPointer());

    vtkNew<vtkOrientedImageData> effectiveExtentLabelmap;
    effectiveExtentLabelmap->SetImageToWorldMatrix(imageToWorldMatrix);
    effectiveExtentLabelmap->SetExtent(labelOrientedImageDataEffectiveExtent);
    vtkOrientedImageDataResample::ResampleOrientedImageToReferenceOrientedImage(labelmap, effectiveExtentLabelmap, effectiveExtentLabelmap);

    double resampledSpacing[3] = { 0 };
    effectiveExtentLabelmap->GetSpacing(resampledSpacing);

    double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
    effectiveExtentLabelmap->GetBounds(bounds);
    double volumeSizeInMm3 = (bounds[1] - bounds[0]) * (bounds[3] - bounds[2]) * (bounds[5] - bounds[4]);

    // set spacing to have an approximately 64^3 volume
    const double preferredVolumeSizeInVoxels = 64 * 64 * 64;

    double spacing = std::pow(volumeSizeInMm3 / preferredVolumeSizeInVoxels, 1 / 3.);
    for (int i = 0; i < 3; ++i)
      {
      double axisBoundsSize = bounds[2 * i + 1] - bounds[2 * i];
      int dimension = std::ceil(axisBoundsSize / spacing);
      resampledSpacing[i] = std::max(axisBoundsSize / dimension, resampledSpacing[i]);
      }

    vtkNew<vtkOrientedImageData> resampledLabelmap;
    resampledLabelmap->SetImageToWorldMatrix(imageToWorldMatrix);
    resampledLabelmap->SetSpacing(resampledSpacing);
    vtkOrientedImageDataResample::ResampleOrientedImageToReferenceOrientedImage(effectiveExtentLabelmap, resampledLabelmap, resampledLabelmap, false, true);

    // If the segmentation is very noisy then there can thousands of tiny islands
    // that would make vtkITKIslandMath fail if the label values are stored on unsigned char.
    vtkNew<vtkImageCast> castToUint;
    castToUint->SetInputData(resampledLabelmap);
    castToUint->SetOutputScalarTypeToUnsignedInt();

    vtkNew<vtkITKIslandMath> islandMath;
    islandMath->SetInputConnection(castToUint->GetOutputPort());

    vtkNew<vtkImageThreshold> largestIslandFilter;
    largestIslandFilter->SetInputConnection(islandMath->GetOutputPort());
    largestIslandFilter->ThresholdBetween(1.0, 1.0);
    largestIslandFilter->SetInValue(1.0);
    largestIslandFilter->SetOutValue(0.0);
    largestIslandFilter->Update();

    vtkNew<vtkOrientedImageData> largestResampledIsland;
    largestResampledIsland->ShallowCopy(largestIslandFilter->GetOutput());
    largestResampledIsland->CopyDirections(resampledLabelmap);

    int resampledLabelEffectiveExtent[6] = { 0, -1, 0, -1, 0, -1 };
    if (!vtkOrientedImageDataResample::CalculateEffectiveExtent(largestResampledIsland, resampledLabelEffectiveExtent))
      {
      vtkWarningMacro("GetSegmentCenter: segment " << segmentID << " is empty");
      return nullptr;
      }

    // segmentCenter_Image is floored to put the center exactly in the center of a voxel
    // (otherwise center position would be set at the boundary between two voxels when extent size is an even number)
    double segmentCenter_Image[4] =
      {
      floor((resampledLabelEffectiveExtent[0] + resampledLabelEffectiveExtent[1]) / 2.0),
      floor((resampledLabelEffectiveExtent[2] + resampledLabelEffectiveExtent[3]) / 2.0),
      floor((resampledLabelEffectiveExtent[4] + resampledLabelEffectiveExtent[5]) / 2.0),
      1.0
      };

    // Resampled image to world
    vtkNew<vtkMatrix4x4> resampledImageToWorldMatrix;
    resampledLabelmap->GetImageToWorldMatrix(resampledImageToWorldMatrix.GetPointer());
    resampledImageToWorldMatrix->MultiplyPoint(segmentCenter_Image, this->SegmentCenterTmp);
    }
  else
    {
    double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
    currentSegment->GetBounds(bounds);
    if (bounds[0]>bounds[1] || bounds[2]>bounds[3] || bounds[4]>bounds[5])
      {
      vtkWarningMacro("GetSegmentCenter: segment " << segmentID << " is empty");
      return nullptr;
      }
    this->SegmentCenterTmp[0] = (bounds[0] + bounds[1]) / 2.0;
    this->SegmentCenterTmp[1] = (bounds[2] + bounds[3]) / 2.0;
    this->SegmentCenterTmp[2] = (bounds[4] + bounds[5]) / 2.0;
    }

  return this->SegmentCenterTmp;
}


//---------------------------------------------------------------------------
double* vtkDMMLSegmentationNode::GetSegmentCenterRAS(const std::string& segmentID)
{
  double* segmentCenter_Segment = this->GetSegmentCenter(segmentID);
  if (!segmentCenter_Segment)
    {
    return nullptr;
    }

  // If segmentation node is transformed, apply that transform to get RAS coordinates
  vtkNew<vtkGeneralTransform> transformSegmentToRas;
  vtkDMMLTransformNode::GetTransformBetweenNodes(this->GetParentTransformNode(), nullptr, transformSegmentToRas.GetPointer());
  transformSegmentToRas->TransformPoint(segmentCenter_Segment, this->SegmentCenterTmp);

  return this->SegmentCenterTmp;
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationNode::GetSegmentCenter(const std::string& segmentID, double center[3])
{
  double* segmentCenterPosition = this->GetSegmentCenter(segmentID);
  if (segmentCenterPosition)
    {
    center[0] = segmentCenterPosition[0];
    center[1] = segmentCenterPosition[1];
    center[2] = segmentCenterPosition[2];
    }
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationNode::GetSegmentCenterRAS(const std::string& segmentID, double centerRAS[3])
{
  double* segmentCenterPositionRAS = this->GetSegmentCenterRAS(segmentID);
  if (segmentCenterPositionRAS)
    {
    centerRAS[0] = segmentCenterPositionRAS[0];
    centerRAS[1] = segmentCenterPositionRAS[1];
    centerRAS[2] = segmentCenterPositionRAS[2];
    }
}

//---------------------------------------------------------------------------
void vtkDMMLSegmentationNode::SetLabelmapConversionColorTableNodeID(const char* labelmapConversionColorTableNodeID)
{
  this->SetNodeReferenceID(this->GetLabelmapConversionColorTableNodeReferenceRole(), labelmapConversionColorTableNodeID);
}

//---------------------------------------------------------------------------
vtkDMMLColorTableNode* vtkDMMLSegmentationNode::GetLabelmapConversionColorTableNode()
{
  return vtkDMMLColorTableNode::SafeDownCast(this->GetNodeReference(this->GetLabelmapConversionColorTableNodeReferenceRole()));
}

//----------------------------------------------------------------------------
void vtkDMMLSegmentationNode::OnNodeReferenceAdded(vtkDMMLNodeReference* reference)
{
  this->Superclass::OnNodeReferenceAdded(reference);
  if (std::string(reference->GetReferenceRole()) == this->GetReferenceImageGeometryReferenceRole())
    {
    this->InvokeCustomModifiedEvent(vtkDMMLSegmentationNode::ReferenceImageGeometryChangedEvent, reference->GetReferencedNode());
    }
}

//----------------------------------------------------------------------------
void vtkDMMLSegmentationNode::OnNodeReferenceModified(vtkDMMLNodeReference* reference)
{
  this->Superclass::OnNodeReferenceModified(reference);
  if (std::string(reference->GetReferenceRole()) == this->GetReferenceImageGeometryReferenceRole())
    {
    this->InvokeCustomModifiedEvent(vtkDMMLSegmentationNode::ReferenceImageGeometryChangedEvent, reference->GetReferencedNode());
    }
}

//----------------------------------------------------------------------------
void vtkDMMLSegmentationNode::OnNodeReferenceRemoved(vtkDMMLNodeReference* reference)
{
  this->Superclass::OnNodeReferenceRemoved(reference);
  if (std::string(reference->GetReferenceRole()) == this->GetReferenceImageGeometryReferenceRole())
    {
    this->InvokeCustomModifiedEvent(vtkDMMLSegmentationNode::ReferenceImageGeometryChangedEvent, reference->GetReferencedNode());
    }
}

//----------------------------------------------------------------------------
const char* vtkDMMLSegmentationNode::ConvertMaskModeToString(int mode)
{
  switch (mode)
  {
    case EditAllowedEverywhere: return "EditAllowedEverywhere";
    case EditAllowedInsideAllSegments: return "EditAllowedInsideAllSegments";
    case EditAllowedInsideVisibleSegments: return "EditAllowedInsideVisibleSegments";
    case EditAllowedOutsideAllSegments: return "EditAllowedOutsideAllSegments";
    case EditAllowedOutsideVisibleSegments: return "EditAllowedOutsideVisibleSegments";
    case EditAllowedInsideSingleSegment: return "EditAllowedInsideSingleSegment";
    default: return "";
  }
}

//----------------------------------------------------------------------------
int vtkDMMLSegmentationNode::ConvertMaskModeFromString(const char* modeStr)
{
  if (!modeStr)
  {
    return -1;
  }
  for (int i=0; i<EditAllowed_Last; i++)
  {
    if (strcmp(modeStr, vtkDMMLSegmentationNode::ConvertMaskModeToString(i)) == 0)
    {
      return i;
    }
  }
  return -1;
}
