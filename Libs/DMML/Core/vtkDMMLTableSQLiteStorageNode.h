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

#ifndef __vtkDMMLTableSQLightStorageNode_h
#define __vtkDMMLTableSQLightStorageNode_h

#include "vtkDMMLStorageNode.h"

/// \brief DMML node for handling Table node storage
///
/// vtkDMMLTableSQLiteStorageNode allows reading/writing of table node from
/// SQLight database.
///
///

class vtkSQLiteDatabase;

class VTK_DMML_EXPORT vtkDMMLTableSQLiteStorageNode : public vtkDMMLStorageNode
{
public:
  static vtkDMMLTableSQLiteStorageNode *New();
  vtkTypeMacro(vtkDMMLTableSQLiteStorageNode,vtkDMMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "TableSQLightStorage";}

  /// Return true if the node can be read in
  bool CanReadInReferenceNode(vtkDMMLNode *refNode) override;

  /// SQLight Database password
  vtkSetStringMacro(Password);
  vtkGetStringMacro(Password);

  /// SQLight Database table name
  vtkSetStringMacro(TableName);
  vtkGetStringMacro(TableName);

  /// Drop a specified table from the database
  static int DropTable(char *tableName, vtkSQLiteDatabase* database);

protected:
  vtkDMMLTableSQLiteStorageNode();
  ~vtkDMMLTableSQLiteStorageNode() override;
  vtkDMMLTableSQLiteStorageNode(const vtkDMMLTableSQLiteStorageNode&);
  void operator=(const vtkDMMLTableSQLiteStorageNode&);

  /// Initialize all the supported write file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  /// Read data and set it in the referenced node. Returns 0 on failure.
  int ReadDataInternal(vtkDMMLNode *refNode) override;

  /// Write data from a  referenced node. Returns 0 on failure.
  int WriteDataInternal(vtkDMMLNode *refNode) override;

  char *TableName;
  char *Password;
};

#endif
