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
/// vtkDMMLShaderPropertyNode - DMML node to represent custom volume shader
/// variables and replacement code

#ifndef __vtkDMMLShaderPropertyNode_h
#define __vtkDMMLShaderPropertyNode_h

// VolumeRendering includes
#include "vtkCjyxVolumeRenderingModuleDMMLExport.h"

// DMML includes
#include "vtkDMMLStorableNode.h"

// VTK includes
class vtkShaderProperty;
class vtkUniforms;

/// \brief vtkDMMLShaderPropertyNode volume shader custom code and
/// custom uniform variables defined by users or specialized rendering
/// modules.
class VTK_CJYX_VOLUMERENDERING_MODULE_DMML_EXPORT vtkDMMLShaderPropertyNode
  : public vtkDMMLStorableNode
{
public:

  /// Create a new vtkDMMLShaderPropertyNode
  static vtkDMMLShaderPropertyNode *New();
  vtkTypeMacro(vtkDMMLShaderPropertyNode,vtkDMMLStorableNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Don't change its scalarOpacity, gradientOpacity or color on the volume property
  /// but use the methods below. It wouldn't observe them.
  vtkGetObjectMacro(ShaderProperty, vtkShaderProperty);

  /// Get the list of user-defined uniform variables.
  vtkUniforms * GetVertexUniforms();
  vtkUniforms * GetFragmentUniforms();
  vtkUniforms * GetGeometryUniforms();

  //--------------------------------------------------------------------------
  // DMMLNode methods
  //--------------------------------------------------------------------------
  vtkDMMLNode* CreateNodeInstance() override;

  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkDMMLNode::CopyContent
  vtkDMMLCopyContentMacro(vtkDMMLShaderPropertyNode);

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "ShaderProperty";}

  /// Reimplemented for internal reasons.
  void ProcessDMMLEvents ( vtkObject *caller, unsigned long event, void *callData) override;

  /// Create default storage node or nullptr if does not have one
  vtkDMMLStorageNode* CreateDefaultStorageNode() override;

  /// \sa vtkDMMLStorableNode::GetModifiedSinceRead()
  bool GetModifiedSinceRead() override;

protected:
  vtkDMMLShaderPropertyNode();
  ~vtkDMMLShaderPropertyNode() override;

protected:
  /// Events observed on the transfer functions
  vtkIntArray* ObservedEvents;

  /// Main parameters for visualization
  vtkShaderProperty* ShaderProperty{nullptr};

private:
  vtkDMMLShaderPropertyNode(const vtkDMMLShaderPropertyNode&) = delete;
  void operator=(const vtkDMMLShaderPropertyNode&) = delete;

};

#endif
