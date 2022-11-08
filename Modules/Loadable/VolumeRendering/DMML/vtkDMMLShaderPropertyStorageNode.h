/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Simon Drouin, Brigham and Women's
  Hospital, Boston, MA.

==============================================================================*/
///  vtkDMMLShaderPropertyStorageNode - DMML node for transform storage on disk
///
/// Storage nodes has methods to read/write transforms to/from disk

#ifndef __vtkDMMLShaderPropertyStorageNode_h
#define __vtkDMMLShaderPropertyStorageNode_h

// VolumeRendering includes
#include "vtkCjyxVolumeRenderingModuleDMMLExport.h"

// DMML includes
#include "vtkDMMLStorageNode.h"

class vtkImageData;

class VTK_CJYX_VOLUMERENDERING_MODULE_DMML_EXPORT vtkDMMLShaderPropertyStorageNode
  : public vtkDMMLStorageNode
{
public:
  static vtkDMMLShaderPropertyStorageNode *New();
  vtkTypeMacro(vtkDMMLShaderPropertyStorageNode,vtkDMMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  ///
  /// Get node XML tag name (like Storage, Transform)
  const char* GetNodeTagName() override {return "ShaderPropertyStorage";}

  /// Return true if the node can be read in
  bool CanReadInReferenceNode(vtkDMMLNode *refNode) override;

protected:
  vtkDMMLShaderPropertyStorageNode();
  ~vtkDMMLShaderPropertyStorageNode() override;
  vtkDMMLShaderPropertyStorageNode(const vtkDMMLShaderPropertyStorageNode&);
  void operator=(const vtkDMMLShaderPropertyStorageNode&);

  /// Initialize all the supported read file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  /// Read data and set it in the referenced node
  int ReadDataInternal(vtkDMMLNode *refNode) override;

  /// Write data from a  referenced node
  int WriteDataInternal(vtkDMMLNode *refNode) override;

};

#endif
