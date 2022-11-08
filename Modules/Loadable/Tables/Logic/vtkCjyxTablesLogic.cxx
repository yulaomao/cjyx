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

// Tables Logic includes
#include "vtkCjyxTablesLogic.h"

// DMML includes
#include <vtkDMMLLayoutNode.h>
#include <vtkDMMLTableNode.h>
#include <vtkDMMLTableStorageNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkDMMLTableSQLiteStorageNode.h>
#include <vtkSmartPointer.h>
#include <vtkSQLiteDatabase.h>
#include <vtkStringArray.h>

// STD includes

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkCjyxTablesLogic);

//----------------------------------------------------------------------------
vtkCjyxTablesLogic::vtkCjyxTablesLogic() = default;

//----------------------------------------------------------------------------
vtkCjyxTablesLogic::~vtkCjyxTablesLogic() = default;

//----------------------------------------------------------------------------
void vtkCjyxTablesLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
vtkDMMLTableNode* vtkCjyxTablesLogic
::AddTable(const char* fileName, const char* name /*=nullptr*/, bool findSchema /*=true*/, const char* password /*=0*/)
{
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("vtkCjyxTablesLogic::AddTable failed: scene is invalid");
    return nullptr;
    }
  if (!fileName)
    {
    vtkErrorMacro("vtkCjyxTablesLogic::AddTable failed: fileName is invalid");
    return nullptr;
    }

  // Storable node
  vtkDMMLTableNode *tableNode = nullptr;

  // Check if the file is sqlite
  std::string extension = vtkDMMLStorageNode::GetLowercaseExtensionFromFileName(fileName);
  if( extension.empty() )
    {
    vtkErrorMacro("ReadData: no file extension specified: " << fileName);
    return nullptr;
    }
  if (   !extension.compare(".db")
      || !extension.compare(".db3")
      || !extension.compare(".sqlite")
      || !extension.compare(".sqlite3"))
    {
    // SQLite
    std::string dbname = std::string("sqlite://") + std::string(fileName);
    vtkSmartPointer<vtkSQLiteDatabase> database = vtkSmartPointer<vtkSQLiteDatabase>::Take(
                   vtkSQLiteDatabase::SafeDownCast( vtkSQLiteDatabase::CreateFromURL(dbname.c_str())));

    if (!database->Open(password?password:"", vtkSQLiteDatabase::USE_EXISTING))
      {
      vtkErrorMacro("Failed to read tables from " << fileName);
      return nullptr;
      }
    vtkStringArray *tables = database->GetTables();
    for (int i=0; i<tables->GetNumberOfTuples(); i++)
      {
      vtkStdString table = tables->GetValue(i);

      // Storage node
      vtkNew<vtkDMMLTableSQLiteStorageNode> tableStorageNode;
      tableStorageNode->SetFileName(fileName);
      tableStorageNode->SetTableName(table);
      this->GetDMMLScene()->AddNode(tableStorageNode.GetPointer());

      // Storable node
      vtkNew<vtkDMMLTableNode> tableNode1;
      std::string uname = std::string(table.c_str());
      tableNode1->SetName(uname.c_str());

      this->GetDMMLScene()->AddNode(tableNode1.GetPointer());
      // Read
      int res = tableStorageNode->ReadData(tableNode1.GetPointer());
      if (res == 0) // failed to read
        {
        vtkErrorMacro("Failed to read tables from " << fileName);
        this->GetDMMLScene()->RemoveNode(tableStorageNode.GetPointer());
        this->GetDMMLScene()->RemoveNode(tableNode1.GetPointer());
        return nullptr;
        }
      tableNode = tableNode1.GetPointer();
      }
    }
  else
    {
    // Storage node
    vtkNew<vtkDMMLTableStorageNode> tableStorageNode;
    tableStorageNode->SetFileName(fileName);
    tableStorageNode->SetAutoFindSchema(findSchema);
    this->GetDMMLScene()->AddNode(tableStorageNode.GetPointer());

    vtkNew<vtkDMMLTableNode> tableNode1;
    if (name)
      {
      tableNode1->SetName(name);
      }
    this->GetDMMLScene()->AddNode(tableNode1.GetPointer());

    // Read
    int res = tableStorageNode->ReadData(tableNode1.GetPointer());
    if (res == 0) // failed to read
      {
      vtkErrorMacro("vtkCjyxTablesLogic::AddTable failed: failed to read data from file: "<<fileName);
      this->GetDMMLScene()->RemoveNode(tableStorageNode.GetPointer());
      this->GetDMMLScene()->RemoveNode(tableNode1.GetPointer());
      return nullptr;
      }
    tableNode = tableNode1.GetPointer();
    }

  return tableNode;
}

//----------------------------------------------------------------------------
int vtkCjyxTablesLogic::GetLayoutWithTable(int currentLayout)
{
  switch (currentLayout)
    {
    case vtkDMMLLayoutNode::CjyxLayoutFourUpTableView:
    case vtkDMMLLayoutNode::CjyxLayout3DTableView:
    case vtkDMMLLayoutNode::CjyxLayoutFourUpPlotTableView:
      // table already shown, no need to change
      return currentLayout;
    case vtkDMMLLayoutNode::CjyxLayoutOneUp3DView:
      return vtkDMMLLayoutNode::CjyxLayout3DTableView;
    case vtkDMMLLayoutNode::CjyxLayoutConventionalPlotView:
    case vtkDMMLLayoutNode::CjyxLayoutFourUpPlotView:
    case vtkDMMLLayoutNode::CjyxLayoutOneUpPlotView:
    case vtkDMMLLayoutNode::CjyxLayoutThreeOverThreePlotView:
      return vtkDMMLLayoutNode::CjyxLayoutFourUpPlotTableView;
    default:
      return vtkDMMLLayoutNode::CjyxLayoutFourUpTableView;
    }
}
