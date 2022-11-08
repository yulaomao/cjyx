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

This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
and was supported through CANARIE's Research Software Program, and Cancer
Care Ontario.

==============================================================================*/

// DMML includes
#include "vtkDMMLStreamingVolumeNode.h"

// VTK includes
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkMatrix4x4.h>

// vtkAddon includes
#include <vtkStreamingVolumeCodecFactory.h>

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLStreamingVolumeNode);

const int NUMBER_OF_INTERNAL_IMAGEDATACONNECTION_OBSERVERS = 1;
const int NUMBER_OF_INTERNAL_IMAGEDATA_OBSERVERS = 2;

//----------------------------------------------------------------------------
// vtkDMMLStreamingVolumeNode methods

//-----------------------------------------------------------------------------
vtkDMMLStreamingVolumeNode::vtkDMMLStreamingVolumeNode()
  : Codec(nullptr)
  , Frame(nullptr)
  , FrameModifiedCallbackCommand(vtkSmartPointer<vtkCallbackCommand>::New())
{
  this->FrameModifiedCallbackCommand->SetClientData(reinterpret_cast<void *>(this));
  this->FrameModifiedCallbackCommand->SetCallback(vtkDMMLStreamingVolumeNode::FrameModifiedCallback);
}

//-----------------------------------------------------------------------------
vtkDMMLStreamingVolumeNode::~vtkDMMLStreamingVolumeNode() = default;

//---------------------------------------------------------------------------
void vtkDMMLStreamingVolumeNode::FrameModifiedCallback(vtkObject *caller, unsigned long vtkNotUsed(eid), void* clientData, void* vtkNotUsed(callData))
{
  vtkDMMLStreamingVolumeNode* self = reinterpret_cast<vtkDMMLStreamingVolumeNode*>(clientData);
  if (!self)
    {
    return;
    }

  if (vtkStreamingVolumeFrame::SafeDownCast(caller) == self->Frame)
    {
    if (self->HasExternalImageObserver())
      {
      self->DecodeFrame();
      }
    self->InvokeCustomModifiedEvent(vtkDMMLStreamingVolumeNode::FrameModifiedEvent);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLStreamingVolumeNode::ProcessDMMLEvents(vtkObject *caller, unsigned long event, void *callData)
{
  Superclass::ProcessDMMLEvents(caller, event, callData);
  if (this->ImageDataConnection != nullptr &&
    this->ImageDataConnection->GetProducer() == vtkAlgorithm::SafeDownCast(caller) &&
    event == vtkCommand::ModifiedEvent)
    {
    if (!this->FrameDecodingInProgress)
      {
      // The image data has been modified externally
      // This invalidates the contents of the current frame
      this->SetAndObserveFrame(nullptr);
      }
    }
}

//---------------------------------------------------------------------------
bool vtkDMMLStreamingVolumeNode::IsKeyFrame()
{
  if (this->Frame)
    {
    return this->Frame->IsKeyFrame();
    }
  return false;
}

//---------------------------------------------------------------------------
void vtkDMMLStreamingVolumeNode::SetAndObserveImageData(vtkImageData* imageData)
{
  if (!this->FrameDecodingInProgress)
    {
    // If no frame is being decoded, then this call is external
    // The current frame data is invalid
    this->SetAndObserveFrame(nullptr);
    }
  Superclass::SetAndObserveImageData(imageData);
}

//---------------------------------------------------------------------------
void vtkDMMLStreamingVolumeNode::AllocateImageForFrame(vtkImageData* imageData)
{
  if (imageData && this->Frame)
    {
    int frameDimensions[3] = { 0,0,0 };
    this->Frame->GetDimensions(frameDimensions);
    imageData->SetDimensions(frameDimensions);
    imageData->AllocateScalars(this->Frame->GetVTKScalarType(), this->Frame->GetNumberOfComponents());
    }
}

//---------------------------------------------------------------------------
vtkImageData* vtkDMMLStreamingVolumeNode::GetImageData()
{
  if (this->Frame)
    {
    this->DecodeFrame();
    }

  vtkImageData* imageData = Superclass::GetImageData();
  return imageData;
}

//---------------------------------------------------------------------------
vtkAlgorithmOutput* vtkDMMLStreamingVolumeNode::GetImageDataConnection()
{
  if (this->Frame)
    {
    this->DecodeFrame();
    }
  return Superclass::GetImageDataConnection();
}

//---------------------------------------------------------------------------
vtkStreamingVolumeCodec* vtkDMMLStreamingVolumeNode::GetCodec()
{
  if (!this->Codec ||
      (this->Codec &&
       this->Codec->GetFourCC() != this->GetCodecFourCC()))
    {
    this->Codec = vtkSmartPointer<vtkStreamingVolumeCodec>::Take(vtkStreamingVolumeCodecFactory::GetInstance()->CreateCodecByFourCC(this->GetCodecFourCC()));
    }
  return this->Codec;
}

//---------------------------------------------------------------------------
bool vtkDMMLStreamingVolumeNode::HasExternalImageObserver()
{
  vtkImageData* imageData = Superclass::GetImageData();
  if ((this->ImageDataConnection != nullptr &&
       this->ImageDataConnection->GetReferenceCount() > NUMBER_OF_INTERNAL_IMAGEDATACONNECTION_OBSERVERS) ||
      (imageData && imageData->GetReferenceCount() > NUMBER_OF_INTERNAL_IMAGEDATA_OBSERVERS))
    {
    return true;
    }
  return false;
}

//---------------------------------------------------------------------------
void vtkDMMLStreamingVolumeNode::SetAndObserveFrame(vtkStreamingVolumeFrame* frame)
{
  if (this->Frame == frame)
    {
    return;
    }

  if (this->Frame)
    {
    this->Frame->RemoveObservers(vtkCommand::ModifiedEvent, this->FrameModifiedCallbackCommand);
    }

  this->Frame = frame;
  this->FrameDecoded = false;

  if (this->Frame)
    {
    this->Frame->AddObserver(vtkCommand::ModifiedEvent, this->FrameModifiedCallbackCommand);
    }

  if (this->Frame)
    {
    this->CodecFourCC = this->Frame->GetCodecFourCC();

    // If the image is being observed beyond the default internal observations of the volume node, then the frame should be decoded
    // since some external class is observing the image data.
    if (this->HasExternalImageObserver())
      {
      this->DecodeFrame();
      }
    }
  this->Modified();
  this->InvokeCustomModifiedEvent(vtkDMMLStreamingVolumeNode::FrameModifiedEvent);
}

//---------------------------------------------------------------------------
bool vtkDMMLStreamingVolumeNode::DecodeFrame()
{
  if (this->FrameDecodingInProgress)
    {
    // Frame is already being decoded
    return true;
    }

  if (!this->Frame)
    {
    vtkErrorMacro("No frame to decode!");
    return false;
    }

  if (this->FrameDecoded)
    {
    // Frame is already decoded.
    // Doesn't need to be decoded twice.
    return true;
    }

  this->FrameDecodingInProgress = true;
  this->FrameDecoded = false;

  vtkSmartPointer<vtkImageData> imageData = Superclass::GetImageData();
  if (!imageData)
    {
    imageData = vtkSmartPointer<vtkImageData>::New();
    }
  this->AllocateImageForFrame(imageData);

  bool success = true;
  if (!imageData)
    {
    vtkErrorMacro("Cannot decode frame. No destination image data!");
    success = false;
    }
  else if (!this->GetCodec())
    {
    vtkErrorMacro("Could not find codec \"" << this->GetCodecFourCC() << "\"");
    success = false;
    }
  else if (!this->Codec->DecodeFrame(this->Frame, imageData))
    {
    vtkErrorMacro("Could not decode frame!");
    success = false;
    }

  if (success)
    {
    this->FrameDecoded = true;
    }
  this->SetAndObserveImageData(imageData);
  this->FrameDecodingInProgress = false;
  return success;
}

//---------------------------------------------------------------------------
bool vtkDMMLStreamingVolumeNode::EncodeImageData(bool forceKeyFrame/*=false*/)
{
  vtkImageData* imageData = Superclass::GetImageData();
  if (!imageData)
    {
    vtkErrorMacro("No image data to encode!");
    return false;
    }

  if (!this->GetCodec())
    {
    vtkErrorMacro("Could not find codec \"" << this->GetCodecFourCC() << "\"");
    return false;
    }

  vtkSmartPointer<vtkStreamingVolumeFrame> frame = vtkSmartPointer<vtkStreamingVolumeFrame>::New();
  if (!this->Codec->EncodeImageData(imageData, frame, forceKeyFrame))
    {
    vtkErrorMacro("Could not encode frame!");
    return false;
    }

  this->FrameDecoded = true;
  this->SetAndObserveFrame(frame);

  return true;
}

//----------------------------------------------------------------------------
void vtkDMMLStreamingVolumeNode::SetCodecParameterString(std::string parameterString)
{
  if (!this->GetCodec())
    {
    return;
    }
  this->Codec->SetParametersFromString(parameterString);
}

//----------------------------------------------------------------------------
std::string vtkDMMLStreamingVolumeNode::GetCodecParameterString()
{
  if (!this->GetCodec())
    {
    return "";
    }

  return this->Codec->GetParametersAsString();
}

//----------------------------------------------------------------------------
void vtkDMMLStreamingVolumeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLStdStringMacro(codecFourCC, CodecFourCC);
  vtkDMMLWriteXMLStdStringMacro(codecParameters, CodecParameterString);
  vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLStreamingVolumeNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  Superclass::ReadXMLAttributes(atts);
  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLStdStringMacro(codecFourCC, CodecFourCC);
  vtkDMMLReadXMLStdStringMacro(codecParameters, CodecParameterString);
  vtkDMMLReadXMLEndMacro();
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLStreamingVolumeNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);

  vtkDMMLStreamingVolumeNode* streamingVolumeNode = vtkDMMLStreamingVolumeNode::SafeDownCast(anode);
  if (!streamingVolumeNode)
    {
    // Other node is not a streaming volume node.
    // We don't need to worry about it trying to decode the frame.
    Superclass::CopyContent(anode, deepCopy);
    }
  else
    {
    // If the source node is a vtkDMMLStreamingVolumeNode, we should not call Superclass::CopyContent(),
    // since the vtkDMMLVolumeNode::CopyContent() function would cause the source node to decode its frame.

    vtkDMMLDisplayableNode::CopyContent(anode, deepCopy);

    /// Duplicated from vtkDMMLVolumeNode::CopyContent()
    vtkAlgorithm* producer = streamingVolumeNode->ImageDataConnection ?
      streamingVolumeNode->ImageDataConnection->GetProducer() : nullptr;
    vtkImageData* sourceImageData = vtkImageData::SafeDownCast(
      producer ? producer->GetOutputDataObject(
        streamingVolumeNode->ImageDataConnection->GetIndex()) : nullptr);

    vtkSmartPointer<vtkImageData> targetImageData = sourceImageData;
    if (deepCopy && sourceImageData)
      {
      targetImageData = vtkSmartPointer<vtkImageData>::Take(sourceImageData->NewInstance());
      targetImageData->DeepCopy(sourceImageData);
      }
    this->SetAndObserveImageData(targetImageData);
    this->CopyOrientation(streamingVolumeNode);

    /// Duplicated from vtkDMMLTensorVolumeNode::CopyContent()
    vtkNew<vtkMatrix4x4> measurementFrameMatrix;
    streamingVolumeNode->GetMeasurementFrameMatrix(measurementFrameMatrix);
    this->SetMeasurementFrameMatrix(measurementFrameMatrix);
    this->Order = streamingVolumeNode->GetOrder();
    }

  vtkDMMLCopyBeginMacro(anode);
  vtkDMMLCopyStdStringMacro(CodecFourCC);
  this->SetAndObserveFrame(this->SafeDownCast(copySourceNode)->GetFrame());
  vtkDMMLCopyStdStringMacro(CodecParameterString);
  vtkDMMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLStreamingVolumeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  vtkDMMLPrintBeginMacro(os, indent);
  os << indent << this->Frame << "\n";
  if (this->Codec)
    {
    os << indent << this->Codec << "\n";
    }
  else
    {
    vtkDMMLPrintStdStringMacro(CodecFourCC);
    }
  vtkDMMLPrintEndMacro();
}
