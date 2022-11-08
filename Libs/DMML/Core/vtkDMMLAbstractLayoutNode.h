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
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

#ifndef __vtkDMMLAbstractLayoutNode_h
#define __vtkDMMLAbstractLayoutNode_h

// DMML includes
#include "vtkDMMLNode.h"

class vtkXMLDataElement;

/// \brief Node that describes the view layout of the application.
///
/// When the scene is closing (vtkDMMLScene::Clear), the view arrangement is
/// set to none due to the Copy() call on an empty node.
class VTK_DMML_EXPORT vtkDMMLAbstractLayoutNode : public vtkDMMLNode
{
public:
  vtkTypeMacro(vtkDMMLAbstractLayoutNode,vtkDMMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object
  void Copy(vtkDMMLNode *node) override;

protected:
  vtkDMMLAbstractLayoutNode();
  ~vtkDMMLAbstractLayoutNode() override;

  vtkDMMLAbstractLayoutNode(const vtkDMMLAbstractLayoutNode&);
  void operator=(const vtkDMMLAbstractLayoutNode&);
};

#endif
