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

#ifndef __vtkDMMLTextStorageNode_h
#define __vtkDMMLTextStorageNode_h

// DMML includes
#include <vtkDMMLStorageNode.h>

/// \ingroup Cjyx_QtModules_Sequences
class VTK_DMML_EXPORT vtkDMMLTextStorageNode : public vtkDMMLStorageNode
{
public:

  static vtkDMMLTextStorageNode* New();
  vtkTypeMacro(vtkDMMLTextStorageNode, vtkDMMLStorageNode);

  vtkDMMLNode* CreateNodeInstance() override;

  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override { return "TextStorage"; };

  /// Return true if the node can be read in.
  bool CanReadInReferenceNode(vtkDMMLNode* refNode) override;

  /// Return true if the node can be written by using the writer.
  bool CanWriteFromReferenceNode(vtkDMMLNode* refNode) override;
  int WriteDataInternal(vtkDMMLNode* refNode) override;

  /// Return a default file extension for writing
  const char* GetDefaultWriteFileExtension() override;

protected:
  vtkDMMLTextStorageNode();
  ~vtkDMMLTextStorageNode() override;
  vtkDMMLTextStorageNode(const vtkDMMLTextStorageNode&);
  void operator=(const vtkDMMLTextStorageNode&);

  /// Does the actual reading. Returns 1 on success, 0 otherwise.
  /// Returns 0 by default (read not supported).
  /// This implementation delegates most everything to the superclass
  /// but it has an early exit if the file to be read is incompatible.
  int ReadDataInternal(vtkDMMLNode* refNode) override;

  /// Initialize all the supported write file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;
};

#endif
