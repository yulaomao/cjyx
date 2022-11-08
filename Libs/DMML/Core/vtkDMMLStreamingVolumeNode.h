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

#ifndef __vtkDMMLStreamingVolumeNode_h
#define __vtkDMMLStreamingVolumeNode_h

// DMML includes
#include "vtkDMML.h"
#include "vtkDMMLNode.h"
#include "vtkDMMLStorageNode.h"
#include "vtkDMMLVectorVolumeDisplayNode.h"
#include "vtkDMMLVectorVolumeNode.h"
#include "vtkDMMLVolumeArchetypeStorageNode.h"

// vtkAddon includes
#include "vtkStreamingVolumeCodec.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkObject.h>
#include <vtkStdString.h>
#include <vtkUnsignedCharArray.h>

/// \brief DMML node for representing a single compressed video frame that can be decoded to an image representation
/// In this context, a frame is considered to be a compressed image that may require additional frames to decode to an image,
/// and an image is the uncompressed pixel based representation.
/// A video codec can be used to decode and encode between frame and image representations
class  VTK_DMML_EXPORT vtkDMMLStreamingVolumeNode : public vtkDMMLVectorVolumeNode
{
public:
  static vtkDMMLStreamingVolumeNode *New();
  vtkTypeMacro(vtkDMMLStreamingVolumeNode,vtkDMMLVectorVolumeNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  void ProcessDMMLEvents(vtkObject *caller, unsigned long event, void *callData) override;

  /// Set node attributes
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkDMMLNode::CopyContent
  vtkDMMLCopyContentMacro(vtkDMMLStreamingVolumeNode);

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override
  {return "StreamingVolume";}

  /// Set/Get the observed image data object and and image data connections
  /// \sa vtkDMMLVolumeNode::SetAndObserveImageData(), vtkDMMLVolumeNode::GetImageData(), vtkDMMLVolumeNode::GetImageDataConnection()
  void SetAndObserveImageData(vtkImageData* imageData) override;
  vtkImageData* GetImageData() override;
  vtkAlgorithmOutput* GetImageDataConnection() override;

  /// Set and observe the frame object containing the compressed image data
  /// \param frame Object containing the compressed video frame info
  void SetAndObserveFrame(vtkStreamingVolumeFrame* frame);

  /// Returns a pointer to the current frame
  vtkStreamingVolumeFrame* GetFrame(){return this->Frame.GetPointer();};

  /// Encodes the current vtkImageData as a compressed frame using the specified codec
  /// Returns true if the image is successfully encoded
  virtual bool EncodeImageData(bool forceKeyFrame = false);

  /// Decodes the current frame and stores the contents in the volume node as vtkImageData
  /// Returns true if the frame is successfully decoded
  virtual bool DecodeFrame();

  /// Returns true if the current frame is a keyframe
  /// Keyframes are not interpolated and don't require any additional frames in order to be decoded to an uncompressed image
  virtual bool IsKeyFrame();

  /// Callback that is called if the current frame is modified
  /// Invokes FrameModifiedEvent
  static void FrameModifiedCallback(vtkObject *caller, unsigned long eid, void* clientData, void* callData);
  enum
  {
    FrameModifiedEvent = 18002
  };

  /// Instance of the code that is
  vtkSetMacro(Codec, vtkStreamingVolumeCodec*);
  vtkStreamingVolumeCodec* GetCodec();

  /// The FourCC representing the codec that should be used
  /// See https://www.fourcc.org/codecs.php
  vtkGetMacro(CodecFourCC, std::string);
  vtkSetMacro(CodecFourCC, std::string);

  /// String representing the parameters that are currently being used by the codec
  /// Format is "ParameterName1:ParameterValue1;ParameterName2;ParameterValue2;ParameterNameN:ParameterValueN"
  void SetCodecParameterString(std::string parameterString);
  std::string GetCodecParameterString();

protected:
  vtkDMMLStreamingVolumeNode();
  ~vtkDMMLStreamingVolumeNode() override;
  vtkDMMLStreamingVolumeNode(const vtkDMMLStreamingVolumeNode&);
  void operator=(const vtkDMMLStreamingVolumeNode&);

  /// Allocates the vtkImageData so that the compressed image data can be decoded
  void AllocateImageForFrame(vtkImageData* imageData);

  /// Returns true if the number of observers on the ImageData or ImageDataConnection is greater than the default expected number
  bool HasExternalImageObserver();

protected:
  vtkSmartPointer<vtkStreamingVolumeCodec> Codec;
  std::string                              CodecFourCC;
  vtkSmartPointer<vtkStreamingVolumeFrame> Frame;
  bool                                     FrameDecoded{false};
  bool                                     FrameDecodingInProgress{false};
  vtkSmartPointer<vtkCallbackCommand>      FrameModifiedCallbackCommand;

};

#endif
