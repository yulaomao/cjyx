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

// STD includes
#include <algorithm>
#include <sstream>

#include "vtkDMMLMessageCollection.h"
#include <vtkDMMLTextNode.h>
#include "vtkDMMLTextStorageNode.h"

// DMML includes
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkNew.h>
#include <vtkStringArray.h>
#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLTextStorageNode);

//----------------------------------------------------------------------------
vtkDMMLTextStorageNode::vtkDMMLTextStorageNode() = default;

//----------------------------------------------------------------------------
vtkDMMLTextStorageNode::~vtkDMMLTextStorageNode() = default;

//----------------------------------------------------------------------------
bool vtkDMMLTextStorageNode::CanReadInReferenceNode(vtkDMMLNode* refNode)
{
  return refNode->IsA("vtkDMMLTextNode");
}

//----------------------------------------------------------------------------
int vtkDMMLTextStorageNode::ReadDataInternal(vtkDMMLNode * refNode)
{
  if (!this->CanReadInReferenceNode(refNode))
    {
    return 0;
    }

  vtkDMMLTextNode* textNode = dynamic_cast<vtkDMMLTextNode*>(refNode);
  if (!textNode)
    {
    vtkErrorMacro("ReadDataInternal: not a text node.");
    return 0;
    }

  std::string fullName = this->GetFullNameFromFileName();

  // check that the file exists
  if (vtksys::SystemTools::FileExists(fullName.c_str()) == false)
    {
    vtkErrorMacro("ReadDataInternal: text file '" << fullName.c_str() << "' not found.");
    return 0;
    }

  // compute file extension
  std::string extension = vtkDMMLStorageNode::GetLowercaseExtensionFromFileName(fullName);
  if (extension.empty())
    {
    vtkErrorMacro("ReadData: no file extension specified: " << fullName.c_str());
    return 0;
    }

  vtkDebugMacro("ReadDataInternal: extension = " << extension.c_str());

  std::ifstream inputFile;
  inputFile.open(fullName);
  if (inputFile.fail())
    {
    vtkErrorMacro("vtkDMMLTextStorageNode::ReadDataInternal: Could not read file");
    return false;
    }

  std::stringstream ss;
  ss << inputFile.rdbuf();
  std::string inputString = ss.str();
  textNode->SetText(inputString.c_str());

  // success
  return 1;
}

//----------------------------------------------------------------------------
bool vtkDMMLTextStorageNode::CanWriteFromReferenceNode(vtkDMMLNode * refNode)
{
  vtkDMMLTextNode* textNode = vtkDMMLTextNode::SafeDownCast(refNode);
  if (textNode == nullptr)
    {
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Only text nodes can written in this format."));
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
int vtkDMMLTextStorageNode::WriteDataInternal(vtkDMMLNode * refNode)
{
  vtkDMMLTextNode* textNode = vtkDMMLTextNode::SafeDownCast(refNode);
  if (textNode == nullptr)
    {
    vtkErrorMacro(<< "vtkDMMLTextStorageNode::WriteDataInternal: Do not recognize node type " << refNode->GetClassName());
    return 0;
    }

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string(""))
    {
    vtkErrorMacro("WriteData: File name not specified");
    return 0;
    }

  // check if the file exists
  if (vtksys::SystemTools::FileExists(fullName.c_str()))
    {
    if (!vtksys::SystemTools::RemoveFile(fullName.c_str()))
      {
      vtkErrorMacro("WriteData: Could not overwrite existing file");
      }
    }

  std::ofstream file;
  file.open(fullName);
  file << textNode->GetText();
  this->StageWriteData(refNode);
  return true;
}

//----------------------------------------------------------------------------
void vtkDMMLTextStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("Text file (.txt)");
  this->SupportedReadFileTypes->InsertNextValue("XML document (.xml)");
  this->SupportedReadFileTypes->InsertNextValue("JSON document (.json)");
}

//----------------------------------------------------------------------------
void vtkDMMLTextStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Text file (.txt)");
  this->SupportedWriteFileTypes->InsertNextValue("XML document (.xml)");
  this->SupportedWriteFileTypes->InsertNextValue("JSON document (.json)");
}

//----------------------------------------------------------------------------
const char* vtkDMMLTextStorageNode::GetDefaultWriteFileExtension()
{
  return "txt";
}
