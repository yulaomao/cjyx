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

#ifndef __vtkDMMLCPURayCastVolumeRenderingDisplayNode_h
#define __vtkDMMLCPURayCastVolumeRenderingDisplayNode_h

// Volume Rendering includes
#include "vtkDMMLVolumeRenderingDisplayNode.h"

/// \ingroup Cjyx_QtModules_VolumeRendering
/// \name vtkDMMLCPURayCastCPURayCastVolumeRenderingDisplayNode
/// \brief DMML node for storing information for CPU Raycast Volume Rendering
class VTK_CJYX_VOLUMERENDERING_MODULE_DMML_EXPORT vtkDMMLCPURayCastVolumeRenderingDisplayNode
  : public vtkDMMLVolumeRenderingDisplayNode
{
public:
  static vtkDMMLCPURayCastVolumeRenderingDisplayNode *New();
  vtkTypeMacro(vtkDMMLCPURayCastVolumeRenderingDisplayNode,vtkDMMLVolumeRenderingDisplayNode);
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
  vtkDMMLCopyContentDefaultMacro(vtkDMMLCPURayCastVolumeRenderingDisplayNode);

  // Description:
  // Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "CPURayCastVolumeRendering";}

protected:
  vtkDMMLCPURayCastVolumeRenderingDisplayNode();
  ~vtkDMMLCPURayCastVolumeRenderingDisplayNode() override;
  vtkDMMLCPURayCastVolumeRenderingDisplayNode(const vtkDMMLCPURayCastVolumeRenderingDisplayNode&);
  void operator=(const vtkDMMLCPURayCastVolumeRenderingDisplayNode&);
};

#endif

