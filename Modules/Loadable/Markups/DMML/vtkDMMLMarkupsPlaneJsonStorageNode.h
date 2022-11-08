/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

/// Markups Module DMML storage nodes
///
/// vtkDMMLMarkupsPlaneJsonStorageNode - DMML node for storing markups in JSON file
///

#ifndef __vtkDMMLMarkupsPlaneJsonStorageNode_h
#define __vtkDMMLMarkupsPlaneJsonStorageNode_h

// Markups includes
#include "vtkCjyxMarkupsModuleDMMLExport.h"
#include "vtkDMMLMarkupsJsonStorageNode.h"

class vtkDMMLMarkupsNode;
class vtkDMMLMarkupsDisplayNode;

/// \ingroup Cjyx_QtModules_Markups
class VTK_CJYX_MARKUPS_MODULE_DMML_EXPORT vtkDMMLMarkupsPlaneJsonStorageNode : public vtkDMMLMarkupsJsonStorageNode
{
public:
  static vtkDMMLMarkupsPlaneJsonStorageNode* New();
  vtkTypeMacro(vtkDMMLMarkupsPlaneJsonStorageNode, vtkDMMLMarkupsJsonStorageNode);

  vtkDMMLNode* CreateNodeInstance() override;

  ///
  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override { return "MarkupsPlaneJsonStorage"; };

  bool CanReadInReferenceNode(vtkDMMLNode* refNode) override;

protected:
  vtkDMMLMarkupsPlaneJsonStorageNode();
  ~vtkDMMLMarkupsPlaneJsonStorageNode() override;
  vtkDMMLMarkupsPlaneJsonStorageNode(const vtkDMMLMarkupsPlaneJsonStorageNode&);
  void operator=(const vtkDMMLMarkupsPlaneJsonStorageNode&);

  class vtkInternalPlane;
  friend class vtkInternalPlane;
};

#endif
