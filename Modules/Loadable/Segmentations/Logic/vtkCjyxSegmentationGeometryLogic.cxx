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
  and was supported through CANARIE and the Applied Cancer Research Unit program
  of Cancer Care Ontario with funds provided by the Ontario Ministry of Health and
  Long-Term Care

==============================================================================*/

// Segmentations includes
#include "vtkCjyxSegmentationGeometryLogic.h"

// SegmentationCore includes
#include "vtkSegmentation.h"
#include "vtkSegmentationConverter.h"
#include "vtkOrientedImageDataResample.h"
#include "vtkCalculateOversamplingFactor.h"

// DMML includes
#include "vtkDMMLAnnotationROINode.h"
#include "vtkDMMLMarkupsROINode.h"
#include "vtkDMMLModelNode.h"
#include "vtkDMMLNodePropertyMacros.h"
#include "vtkDMMLScalarVolumeNode.h"
#include "vtkDMMLTransformNode.h"

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkTransform.h>
#include <vtkGeneralTransform.h>
#include <vtkAddonMathUtilities.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkCjyxSegmentationGeometryLogic);

//----------------------------------------------------------------------------
vtkCjyxSegmentationGeometryLogic::vtkCjyxSegmentationGeometryLogic()
{
  this->OutputGeometryImageData = vtkOrientedImageData::New();
}

//----------------------------------------------------------------------------
vtkCjyxSegmentationGeometryLogic::~vtkCjyxSegmentationGeometryLogic()
{
  this->SetInputSegmentationNode(nullptr);
  this->SetSourceGeometryNode(nullptr);
  if (this->OutputGeometryImageData)
    {
    this->OutputGeometryImageData->Delete();
    this->OutputGeometryImageData = nullptr;
    }
}

//----------------------------------------------------------------------------
void vtkCjyxSegmentationGeometryLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  vtkDMMLPrintBeginMacro(os, indent);
  vtkDMMLPrintBooleanMacro(IsotropicSpacing);
  vtkDMMLPrintFloatMacro(OversamplingFactor);
  vtkDMMLPrintVectorMacro(UserSpacing, double, 3);
  vtkDMMLPrintVectorMacro(InputAxisIndexForSourceAxis, int, 3);
  vtkDMMLPrintVectorMacro(SourceAxisIndexForInputAxis, int, 3);
  vtkDMMLPrintEndMacro();

  if (this->InputSegmentationNode)
    {
    os << indent << "InputSegmentationNode: " ;
    this->InputSegmentationNode->PrintSelf(os, indent.GetNextIndent());
    }

  if (this->SourceGeometryNode)
    {
    os << indent << "SourceGeometryNode: " ;
    this->SourceGeometryNode->PrintSelf(os, indent.GetNextIndent());
    }

  os << indent << "OutputGeometryImageData: " ;
  this->OutputGeometryImageData->PrintSelf(os, indent.GetNextIndent());
}

//-----------------------------------------------------------------------------
void vtkCjyxSegmentationGeometryLogic::SetSourceGeometryNode(vtkDMMLDisplayableNode* node)
{
  vtkSetObjectBodyMacro(SourceGeometryNode, vtkDMMLDisplayableNode, node);

  // Calculate axis permutation if necessary
  if (this->SourceGeometryNode && this->InputSegmentationNode)
    {
    this->ComputeSourceAxisIndexForInputAxis();
    }
}

//-----------------------------------------------------------------------------
std::string vtkCjyxSegmentationGeometryLogic::CalculateOutputGeometry()
{
  // Reset geometry
  this->ResetGeometryImageData();

  if (!this->InputSegmentationNode)
    {
    return "No input segmentation specified";
    }
  if (!this->SourceGeometryNode)
    {
    return "No source geometry specified";
    }

  // Determine source type
  vtkDMMLScalarVolumeNode* sourceVolumeNode = vtkDMMLScalarVolumeNode::SafeDownCast(this->SourceGeometryNode);
  vtkDMMLAnnotationROINode* sourceRoiNode = vtkDMMLAnnotationROINode::SafeDownCast(this->SourceGeometryNode);
  vtkDMMLMarkupsROINode* sourceMarkupsRoiNode = vtkDMMLMarkupsROINode::SafeDownCast(this->SourceGeometryNode);
  vtkDMMLSegmentationNode* sourceSegmentationNode = vtkDMMLSegmentationNode::SafeDownCast(this->SourceGeometryNode);

  if (sourceVolumeNode
      || (sourceSegmentationNode && this->IsSourceSegmentationWithBinaryLabelmapMaster()))
    {
    //TODO: Fractional labelmaps cannot be used yet as source, as DetermineCommonLabelmapGeometry only supports binary labelmaps
    return this->CalculateOutputGeometryFromImage();
    }
  else if (sourceRoiNode || sourceMarkupsRoiNode)
    {
    return this->CalculateOutputGeometryFromBounds(false); // use source axes
    }
  else
    {
    return this->CalculateOutputGeometryFromBounds(true); // use current axes
    }
}

//-----------------------------------------------------------------------------
void vtkCjyxSegmentationGeometryLogic::CalculatePaddedOutputGeometry()
{
  if (!this->InputSegmentationNode || !this->PadOutputGeometry)
    {
    return;
    }

  std::string segmentationGeometryString = this->InputSegmentationNode->GetSegmentation()->DetermineCommonLabelmapGeometry(
    vtkSegmentation::EXTENT_UNION_OF_EFFECTIVE_SEGMENTS);
  vtkNew<vtkOrientedImageData> inputGeometryImageData;
  vtkSegmentationConverter::DeserializeImageGeometry(segmentationGeometryString, inputGeometryImageData, false/*don't allocate*/);
  vtkNew<vtkTransform> segmentationGeometryToReferenceGeometryTransform;
  vtkOrientedImageDataResample::GetTransformBetweenOrientedImages(inputGeometryImageData,
    this->OutputGeometryImageData, segmentationGeometryToReferenceGeometryTransform);

  int transformedSegmentationExtent[6] = { 0, -1, 0, -1, 0, -1 };
  vtkOrientedImageDataResample::TransformExtent(inputGeometryImageData->GetExtent(),
    segmentationGeometryToReferenceGeometryTransform, transformedSegmentationExtent);

  int outputGeometryExtent[6] = { 0, -1, 0, -1, 0, -1 };
  this->OutputGeometryImageData->GetExtent(outputGeometryExtent);

  for (int i = 0; i < 3; ++i)
    {
    outputGeometryExtent[2*i] = std::min(outputGeometryExtent[2*i], transformedSegmentationExtent[2*i]);
    outputGeometryExtent[2*i+1] = std::max(outputGeometryExtent[2*i+1], transformedSegmentationExtent[2*i+1]);
    }
  this->OutputGeometryImageData->SetExtent(outputGeometryExtent);
}

//-----------------------------------------------------------------------------
std::string vtkCjyxSegmentationGeometryLogic::CalculateOutputGeometryFromImage()
{
  if (!this->InputSegmentationNode)
    {
    return "Invalid input segmentation node";
    }

  // Determine source type
  vtkDMMLScalarVolumeNode* sourceVolumeNode = vtkDMMLScalarVolumeNode::SafeDownCast(this->SourceGeometryNode);
  vtkDMMLSegmentationNode* sourceSegmentationNode = vtkDMMLSegmentationNode::SafeDownCast(this->SourceGeometryNode);

  // Set initial geometry from source volume
  vtkNew<vtkMatrix4x4> sourceImageToSourceMatrix;
  if (sourceVolumeNode)
    {
    if (!sourceVolumeNode->GetImageData())
      {
      return "Invalid source volume (empty image)";
      }
    sourceVolumeNode->GetIJKToRASMatrix(sourceImageToSourceMatrix);
    this->OutputGeometryImageData->SetExtent(sourceVolumeNode->GetImageData()->GetExtent());
    }
  else if (sourceSegmentationNode)
    {
    vtkNew<vtkOrientedImageData> sourceBinaryLabelmap;
    std::string geometryString = sourceSegmentationNode->GetSegmentation()->DetermineCommonLabelmapGeometry();
    if (!vtkSegmentationConverter::DeserializeImageGeometry(geometryString, sourceBinaryLabelmap, false))
      {
      return "Failed to determine labelmap geometry from segmentation node";
      }
    sourceBinaryLabelmap->GetImageToWorldMatrix(sourceImageToSourceMatrix);
    this->OutputGeometryImageData->SetExtent(sourceBinaryLabelmap->GetExtent());
    }
  else
    {
    return "Invalid source volume (volume or segmentation node is required)";
    }

  // Determine transform between source node and input segmentation
  vtkNew<vtkMatrix4x4> sourceToInputSegmentationMatrix;
  if (!vtkDMMLTransformNode::GetMatrixTransformBetweenNodes(this->SourceGeometryNode->GetParentTransformNode(),
    this->InputSegmentationNode->GetParentTransformNode(), sourceToInputSegmentationMatrix))
    {
    vtkWarningMacro("CalculateOutputGeometry: Ignoring parent transforms because non-linear components have been found");
    }

  vtkNew<vtkMatrix4x4> outputGeometryImageToSegmentationMatrix;
  vtkMatrix4x4::Multiply4x4(sourceToInputSegmentationMatrix, sourceImageToSourceMatrix, outputGeometryImageToSegmentationMatrix);
  this->OutputGeometryImageData->SetImageToWorldMatrix(outputGeometryImageToSegmentationMatrix);

  // Apply optional settings
  if (this->IsotropicSpacing)
    {
    double* spacing = this->OutputGeometryImageData->GetSpacing();
    double minSpacing = this->OutputGeometryImageData->GetMinSpacing();

    int newExtent[6] = {0,-1,0,-1,0,-1};
    int extent[6] = {0,-1,0,-1,0,-1};
    this->OutputGeometryImageData->GetExtent(extent);
    for (unsigned int axis=0; axis<3; ++axis)
      {
      double oversamplingForAxis = spacing[this->SourceAxisIndexForInputAxis[axis]] / minSpacing;
      int dimension = extent[axis*2+1] - extent[axis*2] + 1;
      int extentMin = static_cast<int>(ceil(oversamplingForAxis * extent[axis * 2]));
      int extentMax = extentMin + static_cast<int>(floor(oversamplingForAxis*dimension)) - 1;
      newExtent[axis*2] = extentMin;
      newExtent[axis*2+1] = extentMax;
      }
    this->OutputGeometryImageData->SetSpacing(minSpacing, minSpacing, minSpacing);
    this->OutputGeometryImageData->SetExtent(newExtent);
    }

  if (this->OversamplingFactor != 1.0)
    {
    vtkCalculateOversamplingFactor::ApplyOversamplingOnImageGeometry(this->OutputGeometryImageData, this->OversamplingFactor);
    }

  this->CalculatePaddedOutputGeometry();

  // success
  return "";
}

//-----------------------------------------------------------------------------
std::string vtkCjyxSegmentationGeometryLogic::CalculateOutputGeometryFromBounds(bool keepCurrentAxisDirections)
{
  if (!this->InputSegmentationNode)
    {
    return "Invalid input segmentation node";
    }

  // Get initial spacing
  double outputSpacing[3] = { 0 };
  outputSpacing[0] = this->UserSpacing[0];
  outputSpacing[1] = this->UserSpacing[1];
  outputSpacing[2] = this->UserSpacing[2];
  if (outputSpacing[0] <= 0 || outputSpacing[1] <= 0 || outputSpacing[2] <= 0)
    {
    return "All spacing values must be larger than 0";
    }

  // Get source bounds
  double sourceBounds[6] = { 0, -1, 0, -1, 0, -1 };
  this->SourceGeometryNode->GetBounds(sourceBounds);

  vtkDMMLMarkupsROINode* sourceMarkupsROINode = vtkDMMLMarkupsROINode::SafeDownCast(this->SourceGeometryNode);
  if (sourceMarkupsROINode)
    {
    // Set the bounds from the ROI in Object coordinate system.
    // Center of the ROI in "Object" is [0,0,0].
    double roiSize[3] = { 0.0, 0.0, 0.0 };
    sourceMarkupsROINode->GetSize(roiSize);
    double roiCenter[3] = { 0.0, 0.0, 0.0 };
    sourceMarkupsROINode->GetCenter(roiCenter);
    for (int i = 0; i < 3; ++i)
      {
      sourceBounds[2*i]     = - (roiSize[i] * 0.5);
      sourceBounds[2*i + 1] = + (roiSize[i] * 0.5);
      }
    }

  // Determine transform between source node and input segmentation
  vtkNew<vtkMatrix4x4> segmentationToSourceMatrix;
  if (!vtkDMMLTransformNode::GetMatrixTransformBetweenNodes(this->InputSegmentationNode->GetParentTransformNode(),
    this->SourceGeometryNode->GetParentTransformNode(), segmentationToSourceMatrix))
    {
    vtkWarningMacro("CalculateOutputGeometry: Ignoring parent transforms because non-linear components have been found");
    }

  if (sourceMarkupsROINode)
    {
    // If the source object is a markups ROI node, then we treat the ROI Object coordinate system as the "Source".
    vtkMatrix4x4* objectToNodeMatrix = sourceMarkupsROINode->GetObjectToNodeMatrix();
    vtkNew<vtkMatrix4x4> nodeToObjectMatrix;
    vtkMatrix4x4::Invert(objectToNodeMatrix, nodeToObjectMatrix);
    vtkMatrix4x4::Multiply4x4(nodeToObjectMatrix, segmentationToSourceMatrix, segmentationToSourceMatrix);
    }

  vtkNew<vtkMatrix4x4> outputGeometryImageToSourceMatrix;

  // If input segmentation has non-empty binary labelmap master that need to be resampled,
  // then match the axes of the labelmap to the axes of the transformed source node, and
  // determine directions and spacing according to that.
  // In this case, origin is also given by the input segmentation's labelmap
  if (keepCurrentAxisDirections && this->InputSegmentationCanBeResampled())
    {
    vtkNew<vtkOrientedImageData> inputBinaryLabelmap;
    std::string geometryString = this->InputSegmentationNode->GetSegmentation()->DetermineCommonLabelmapGeometry();
    vtkSegmentationConverter::DeserializeImageGeometry(geometryString, inputBinaryLabelmap, false);

    // Find which labelmap axis corresponds to each source axis, to get the correct spacing value for each source axis
    vtkNew<vtkMatrix4x4> inputLabelmapIJKToInputSegmentation;
    inputBinaryLabelmap->GetImageToWorldMatrix(inputLabelmapIJKToInputSegmentation);
    vtkNew<vtkMatrix4x4> inputLabelmapIJKToSource;
    vtkMatrix4x4::Multiply4x4(segmentationToSourceMatrix, inputLabelmapIJKToInputSegmentation, inputLabelmapIJKToSource);
    this->OutputGeometryImageData->SetImageToWorldMatrix(inputLabelmapIJKToSource);

    outputSpacing[0] = this->UserSpacing[this->InputAxisIndexForSourceAxis[0]];
    outputSpacing[1] = this->UserSpacing[this->InputAxisIndexForSourceAxis[1]];
    outputSpacing[2] = this->UserSpacing[this->InputAxisIndexForSourceAxis[2]];
    this->OutputGeometryImageData->SetSpacing(outputSpacing);

    vtkNew<vtkMatrix4x4> outputGeometryImageToWorld;
    this->OutputGeometryImageData->GetImageToWorldMatrix(outputGeometryImageToWorld);

    // outputGeometryImageToSource = SegmentationToSource * OutputGeometryImageToWorld
    vtkMatrix4x4::Multiply4x4(segmentationToSourceMatrix, outputGeometryImageToWorld, outputGeometryImageToSourceMatrix);
    }
  else
    {
    // Directions according to segmentation to source transformation
    // Spacing as specified on the UI
    outputGeometryImageToSourceMatrix->SetElement(0, 0, outputSpacing[0]);
    outputGeometryImageToSourceMatrix->SetElement(1, 1, outputSpacing[1]);
    outputGeometryImageToSourceMatrix->SetElement(2, 2, outputSpacing[2]);
    vtkNew<vtkMatrix4x4> sourceToSegmentationMatrix;
    vtkMatrix4x4::Invert(segmentationToSourceMatrix, sourceToSegmentationMatrix);
    vtkNew<vtkMatrix4x4> outputGeometryImageToSegmentationMatrix;
    vtkMatrix4x4::Multiply4x4(sourceToSegmentationMatrix, outputGeometryImageToSourceMatrix, outputGeometryImageToSegmentationMatrix);
    this->OutputGeometryImageData->SetImageToWorldMatrix(outputGeometryImageToSegmentationMatrix);
    }

  // Calculate extent
  const double corner1_Source[4] =
    {
    sourceBounds[0] + outputSpacing[0] / 2.0,
    sourceBounds[2] + outputSpacing[1] / 2.0,
    sourceBounds[4] + outputSpacing[2] / 2.0,
    1.0
    };
  const double corner2_Source[4] =
    {
    sourceBounds[1] - outputSpacing[0] / 2.0,
    sourceBounds[3] - outputSpacing[1] / 2.0,
    sourceBounds[5] - outputSpacing[2] / 2.0,
    1.0
    };
  vtkNew<vtkMatrix4x4> sourceToOutputGeometryImageMatrix;
  vtkMatrix4x4::Invert(outputGeometryImageToSourceMatrix, sourceToOutputGeometryImageMatrix);
  double corner1_OutputGeometryImage[4] = { 0.0, 0.0, 0.0, 1.0 };
  double corner2_OutputGeometryImage[4] = { 0.0, 0.0, 0.0, 1.0 };
  sourceToOutputGeometryImageMatrix->MultiplyPoint(corner1_Source, corner1_OutputGeometryImage);
  sourceToOutputGeometryImageMatrix->MultiplyPoint(corner2_Source, corner2_OutputGeometryImage);
  int outputExtent[6] =
    {
    vtkMath::Round(std::min(corner1_OutputGeometryImage[0], corner2_OutputGeometryImage[0])),
    vtkMath::Round(std::max(corner1_OutputGeometryImage[0], corner2_OutputGeometryImage[0])),
    vtkMath::Round(std::min(corner1_OutputGeometryImage[1], corner2_OutputGeometryImage[1])),
    vtkMath::Round(std::max(corner1_OutputGeometryImage[1], corner2_OutputGeometryImage[1])),
    vtkMath::Round(std::min(corner1_OutputGeometryImage[2], corner2_OutputGeometryImage[2])),
    vtkMath::Round(std::max(corner1_OutputGeometryImage[2], corner2_OutputGeometryImage[2]))
    };
  this->OutputGeometryImageData->SetExtent(outputExtent);

  this->CalculatePaddedOutputGeometry();

  // success
  return"";
}

//-----------------------------------------------------------------------------
void vtkCjyxSegmentationGeometryLogic::ResetGeometryImageData()
{
  vtkNew<vtkMatrix4x4> identityMatrix;
  identityMatrix->Identity();
  this->OutputGeometryImageData->SetImageToWorldMatrix(identityMatrix);
  this->OutputGeometryImageData->SetDimensions(0, 0, 0);
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationGeometryLogic::IsSourceSegmentationWithBinaryLabelmapMaster()
{
  vtkDMMLSegmentationNode* sourceSegmentationNode = vtkDMMLSegmentationNode::SafeDownCast(this->SourceGeometryNode);
  std::string binaryLabelmapName = vtkSegmentationConverter::GetBinaryLabelmapRepresentationName();
  vtkSmartPointer<vtkOrientedImageData> sourceBinaryLabelmap;
  if ( sourceSegmentationNode
    && sourceSegmentationNode->GetSegmentation()
    && sourceSegmentationNode->GetSegmentation()->GetNumberOfSegments() > 0
    && sourceSegmentationNode->GetSegmentation()->ContainsRepresentation(binaryLabelmapName)
    && sourceSegmentationNode->GetSegmentation()->GetMasterRepresentationName() == binaryLabelmapName )
    {
    return true;
    }

  return false;
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationGeometryLogic::InputSegmentationCanBeResampled()
{
  if (!this->InputSegmentationNode || !this->InputSegmentationNode->GetSegmentation())
    {
    return false;
    }
  if (this->InputSegmentationNode->GetSegmentation()->GetNumberOfSegments() == 0)
    {
    return false;
    }
  if (!this->InputSegmentationNode->GetSegmentation()->ContainsRepresentation(
      vtkSegmentationConverter::GetBinaryLabelmapRepresentationName())
    || this->InputSegmentationNode->GetSegmentation()->GetMasterRepresentationName()
      != vtkSegmentationConverter::GetBinaryLabelmapRepresentationName())
    {
    return false;
    }
  std::string geometryString = this->InputSegmentationNode->GetSegmentation()->DetermineCommonLabelmapGeometry();
  if (geometryString.empty())
    {
    // all segments are empty
    return false;
    }
  return true;
}

//-----------------------------------------------------------------------------
void vtkCjyxSegmentationGeometryLogic::ComputeSourceAxisIndexForInputAxis()
{
  // Initialize source axis permutation and inverse
  this->InputAxisIndexForSourceAxis[0] = 0;
  this->InputAxisIndexForSourceAxis[1] = 1;
  this->InputAxisIndexForSourceAxis[2] = 2;
  this->SourceAxisIndexForInputAxis[0] = 0;
  this->SourceAxisIndexForInputAxis[1] = 1;
  this->SourceAxisIndexForInputAxis[2] = 2;

  vtkDMMLTransformableNode* transformableSourceNode = vtkDMMLTransformableNode::SafeDownCast(this->SourceGeometryNode);
  if (!transformableSourceNode || !this->InputSegmentationNode)
    {
    vtkErrorMacro("ComputeSourceAxisIndexForInputAxis: Invalid input nodes");
    return;
    }
  if (transformableSourceNode->GetScene() != this->InputSegmentationNode->GetScene())
    {
    vtkErrorMacro("ComputeSourceAxisIndexForInputAxis: DMML scene of the given source node and the widget are different, cannot set node");
    return;
    }

  // If source is volume type and input segmentation has non-empty binary labelmap master that need to be resampled,
  // then match the axes of the input labelmap to the axes of the transformed source node.
  // Use this calculated permutation for updating spacing widget from geometry and interpreting spacing input
  if ( (transformableSourceNode->IsA("vtkDMMLScalarVolumeNode") || this->IsSourceSegmentationWithBinaryLabelmapMaster())
    && this->InputSegmentationCanBeResampled() )
    {
    // Determine transform between source node and input segmentation
    vtkNew<vtkGeneralTransform> segmentationToSourceTransform;
    vtkNew<vtkTransform> segmentationToSourceTransformLinear;
    vtkDMMLTransformNode::GetTransformBetweenNodes(this->InputSegmentationNode->GetParentTransformNode(),
      transformableSourceNode->GetParentTransformNode(), segmentationToSourceTransform);
    vtkNew<vtkMatrix4x4> segmentationToSourceMatrix;
    if (vtkDMMLTransformNode::IsGeneralTransformLinear(segmentationToSourceTransform, segmentationToSourceTransformLinear))
      {
      // Transformation between segmentation and source is linear
      segmentationToSourceTransformLinear->GetMatrix(segmentationToSourceMatrix);
      }
    else
      {
      vtkWarningMacro("ComputeSourceAxisIndexForInputAxis: Ignoring parent transforms because non-linear components have been found");
      segmentationToSourceMatrix->Identity();
      }

    vtkNew<vtkOrientedImageData> inputBinaryLabelmap;
    std::string geometryString = this->InputSegmentationNode->GetSegmentation()->DetermineCommonLabelmapGeometry();
    vtkSegmentationConverter::DeserializeImageGeometry(geometryString, inputBinaryLabelmap, false);

    // Find which labelmap axis corresponds to each source axis, to get the correct spacing value for each source axis
    vtkNew<vtkMatrix4x4> inputLabelmapIJKToInputSegmentation;
    inputBinaryLabelmap->GetImageToWorldMatrix(inputLabelmapIJKToInputSegmentation);
    vtkNew<vtkMatrix4x4> inputLabelmapIJKToSource;
    vtkMatrix4x4::Multiply4x4(segmentationToSourceMatrix, inputLabelmapIJKToInputSegmentation, inputLabelmapIJKToSource);

    // Find the axis that is best aligned with each source axis
    double scale[3] = { 1.0 };
    vtkAddonMathUtilities::NormalizeOrientationMatrixColumns(inputLabelmapIJKToSource, scale);
    for (int sourceAxisIndex=0; sourceAxisIndex<3; sourceAxisIndex++)
      {
      double largestComponentValue = 0.0;
      for (int labelmapIJKAxisIndex=0; labelmapIJKAxisIndex<3; labelmapIJKAxisIndex++)
        {
        double currentComponentValue = fabs(inputLabelmapIJKToSource->GetElement(sourceAxisIndex, labelmapIJKAxisIndex));
        if (currentComponentValue > largestComponentValue)
          {
          largestComponentValue = currentComponentValue;
          this->InputAxisIndexForSourceAxis[sourceAxisIndex] = labelmapIJKAxisIndex;
          }
        }
      }
    }

  // Calculate inverse permutation
  for (int i=0; i<3; ++i)
    {
    this->SourceAxisIndexForInputAxis[this->InputAxisIndexForSourceAxis[i]] = i;
    }
}

//-----------------------------------------------------------------------------
bool vtkCjyxSegmentationGeometryLogic::ResampleLabelmapsInSegmentationNode()
{
  if (!this->InputSegmentationNode || !this->InputSegmentationNode->GetSegmentation())
    {
    vtkErrorMacro("vtkCjyxSegmentationGeometryLogic::ResampleLabelmapsInSegmentationNode: invalid input segmentation node");
    return false;
    }

  // Check if master representation is binary or fractional labelmap (those are the only supported representations in segment editor)
  std::string masterRepresentationName = this->InputSegmentationNode->GetSegmentation()->GetMasterRepresentationName();
  if ( masterRepresentationName != vtkSegmentationConverter::GetBinaryLabelmapRepresentationName()
    && masterRepresentationName != vtkSegmentationConverter::GetFractionalLabelmapRepresentationName() )
    {
    vtkErrorMacro("vtkCjyxSegmentationGeometryLogic::ResampleLabelmapsInSegmentationNode: "
      << "Master representation needs to be a labelmap type, but '" << masterRepresentationName.c_str() << "' found");
    return false;
    }

  bool success = true;

  DMMLNodeModifyBlocker blocker(this->InputSegmentationNode);
  vtkOrientedImageData* geometryImageData = this->GetOutputGeometryImageData();
  std::vector< std::string > segmentIDs;
  this->InputSegmentationNode->GetSegmentation()->GetSegmentIDs(segmentIDs);
  for (std::vector< std::string >::const_iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt)
    {
    std::string currentSegmentID = *segmentIdIt;
    vtkSegment* currentSegment = this->InputSegmentationNode->GetSegmentation()->GetSegment(*segmentIdIt);

    // Get master labelmap from segment
    vtkOrientedImageData* currentLabelmap = vtkOrientedImageData::SafeDownCast(
      currentSegment->GetRepresentation(masterRepresentationName) );
    if (!currentLabelmap)
      {
      vtkErrorMacro("vtkCjyxSegmentationGeometryLogic::ResampleLabelmapsInSegmentationNode: "
        << "Failed to retrieve master representation from segment " << currentSegmentID.c_str());
      continue;
      }

    // Resample
    if (!vtkOrientedImageDataResample::ResampleOrientedImageToReferenceOrientedImage(
      currentLabelmap, geometryImageData, currentLabelmap, false, this->PadOutputGeometry))
      {
      vtkErrorMacro("vtkCjyxSegmentationGeometryLogic::ResampleLabelmapsInSegmentationNode: "
        << "Segment " << this->InputSegmentationNode->GetName() << "/" << currentSegmentID.c_str() << " failed to be resampled");
      success = false;
      continue;
      }
    }

  this->InputSegmentationNode->Modified();
  return success;
}
