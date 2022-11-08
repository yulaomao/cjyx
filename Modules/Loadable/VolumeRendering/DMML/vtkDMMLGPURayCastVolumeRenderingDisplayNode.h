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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __vtkDMMLGPURayCastVolumeRenderingDisplayNode_h
#define __vtkDMMLGPURayCastVolumeRenderingDisplayNode_h

// Volume Rendering includes
#include "vtkDMMLVolumeRenderingDisplayNode.h"

/// \ingroup Cjyx_QtModules_VolumeRendering
/// \name vtkDMMLGPURayCastGPURayCastVolumeRenderingDisplayNode
/// \brief DMML node for storing information for GPU Raycast Volume Rendering
class VTK_CJYX_VOLUMERENDERING_MODULE_DMML_EXPORT vtkDMMLGPURayCastVolumeRenderingDisplayNode
  : public vtkDMMLVolumeRenderingDisplayNode
{
public:
  static vtkDMMLGPURayCastVolumeRenderingDisplayNode *New();
  vtkTypeMacro(vtkDMMLGPURayCastVolumeRenderingDisplayNode,vtkDMMLVolumeRenderingDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  // Description:
  // Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  // Description:
  // Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkDMMLNode::CopyContent
  vtkDMMLCopyContentDefaultMacro(vtkDMMLGPURayCastVolumeRenderingDisplayNode);

  // Description:
  // Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "GPURayCastVolumeRendering";}

protected:
  vtkDMMLGPURayCastVolumeRenderingDisplayNode();
  ~vtkDMMLGPURayCastVolumeRenderingDisplayNode() override;
  vtkDMMLGPURayCastVolumeRenderingDisplayNode(const vtkDMMLGPURayCastVolumeRenderingDisplayNode&);
  void operator=(const vtkDMMLGPURayCastVolumeRenderingDisplayNode&);
};

#endif
