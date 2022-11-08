/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

#ifndef __vtkDMMLTableViewNode_h
#define __vtkDMMLTableViewNode_h

#include "vtkDMMLAbstractViewNode.h"

class vtkDMMLTableNode;

/// \brief DMML node to represent table view parameters.
///
/// TableViewNodes are associated one to one with a TableWidget.
class VTK_DMML_EXPORT vtkDMMLTableViewNode : public vtkDMMLAbstractViewNode
{
public:
  static vtkDMMLTableViewNode *New();
  vtkTypeMacro(vtkDMMLTableViewNode, vtkDMMLAbstractViewNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //--------------------------------------------------------------------------
  /// DMMLNode methods
  //--------------------------------------------------------------------------

  vtkDMMLNode* CreateNodeInstance() override;

  ///
  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Copy the node's attributes to this object
  void Copy(vtkDMMLNode *node) override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override;

  ///
  /// Set the Table node id displayed in this Table View
  void SetTableNodeID(const char *);

  ///
  /// Get the Table node id displayed in this Table View
  const char * GetTableNodeID();

  ///
  /// Get the Table node displayed in this Table View
  vtkDMMLTableNode* GetTableNode();

  ///
  /// Configures the behavior of PropagateTableSelection().
  /// If DoPropagateTableSelection set to false then this
  /// view will not be affected by PropagateTableSelection.
  /// Default value is true.
  vtkSetMacro (DoPropagateTableSelection, bool );
  vtkGetMacro (DoPropagateTableSelection, bool );

  virtual const char* GetTableNodeReferenceRole();

protected:
  vtkDMMLTableViewNode();
  ~vtkDMMLTableViewNode() override;
  vtkDMMLTableViewNode(const vtkDMMLTableViewNode&);
  void operator=(const vtkDMMLTableViewNode&);

  virtual const char* GetTableNodeReferenceDMMLAttributeName();

  static const char* TableNodeReferenceRole;
  static const char* TableNodeReferenceDMMLAttributeName;

  bool DoPropagateTableSelection{true};
};

#endif
