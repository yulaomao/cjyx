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

#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLTextStorageNode.h"
#include "vtkDMMLTextNode.h"
#include "vtkDMMLScene.h"

#include <vtkNew.h>

//---------------------------------------------------------------------------
int TestReadWriteData(vtkDMMLScene* scene, const char* extension, std::string text, int encoding);

//---------------------------------------------------------------------------
int vtkDMMLTextStorageNodeTest1(int argc, char * argv[] )
{
  if (argc != 2)
    {
    std::cerr << "Usage: " << argv[0] << " /path/to/temp" << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkDMMLTextStorageNode> node1;
  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());

  vtkNew<vtkDMMLScene> scene;
  const char* tempDir = argv[1];
  scene->SetRootDirectory(tempDir);

  CHECK_EXIT_SUCCESS(TestReadWriteData(scene.GetPointer(), ".txt", "Hello world!", VTK_ENCODING_US_ASCII));
  CHECK_EXIT_SUCCESS(TestReadWriteData(scene.GetPointer(), "UTF8.txt", u8"Hell\u00F3 vil\u00E1g!", VTK_ENCODING_UTF_8));
  CHECK_EXIT_SUCCESS(TestReadWriteData(scene.GetPointer(), ".xml", "<Hello World=True/>", VTK_ENCODING_US_ASCII));
  CHECK_EXIT_SUCCESS(TestReadWriteData(scene.GetPointer(), ".json", "{\"Hello\":\"World\"}", VTK_ENCODING_US_ASCII));

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestReadWriteData(vtkDMMLScene* scene, const char *extension, std::string text, int encoding)
{
  std::string fileName = std::string(scene->GetRootDirectory()) +
                         std::string("/vtkDMMLTextNodeTest1") +
                         std::string(extension);

  // Add text node
  vtkNew<vtkDMMLTextNode> textNode;
  textNode->SetText(text, encoding);
  textNode->SetForceCreateStorageNode(vtkDMMLTextNode::CreateStorageNodeAlways);
  CHECK_NOT_NULL(scene->AddNode(textNode.GetPointer()));
  CHECK_BOOL(textNode->AddDefaultStorageNode(), true);
  CHECK_NOT_NULL(textNode->GetStorageNode());

  vtkDMMLStorageNode* storageNode = textNode->GetStorageNode();
  CHECK_NOT_NULL(storageNode);
  storageNode->SetFileName(fileName.c_str());

  // Test writing
  CHECK_BOOL(storageNode->WriteData(textNode.GetPointer()), true);

  // Clear data from text node
  textNode->SetText("");
  CHECK_STD_STRING(textNode->GetText(), "");

  // Test reading
  CHECK_BOOL(storageNode->ReadData(textNode.GetPointer()), true);
  CHECK_STD_STRING(textNode->GetText(), text);

  return EXIT_SUCCESS;
}
