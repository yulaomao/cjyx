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

// DMML includes
#include <vtkCacheManager.h>
#include <vtkDataIOManager.h>
#include <vtkDMMLLinearTransformNode.h>
#include <vtkDMMLLinearTransformSequenceStorageNode.h>
#include <vtkDMMLModelNode.h>
#include <vtkDMMLScalarVolumeNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSequenceNode.h>
#include <vtkDMMLSequenceStorageNode.h>
#include <vtkDMMLTransformNode.h>
#include <vtkDMMLVolumeSequenceStorageNode.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPolyData.h>

#include "vtkDMMLCoreTestingMacros.h"

//-----------------------------------------------------------------------------
int TestWriteReadSequence(const std::string& tempDir, vtkDMMLSequenceNode* sequenceNode, vtkDMMLStorageNode* storageNode, std::string fileName)
{
  std::stringstream fullFilePathSS;
  fullFilePathSS << tempDir << "/" << fileName << "." << storageNode->GetDefaultWriteFileExtension();
  std::string fullFilePath = fullFilePathSS.str();
  if (vtksys::SystemTools::FileExists(fullFilePath.c_str(), true))
    {
    vtksys::SystemTools::RemoveFile(fullFilePath.c_str());
    }

  std::cout << "Testing sequence write: " << fullFilePath << std::endl;
  storageNode->SetFileName(fullFilePath.c_str());
  CHECK_BOOL(storageNode->WriteData(sequenceNode), true);

  vtkSmartPointer<vtkDMMLScene> scene = sequenceNode->GetScene();

  vtkSmartPointer<vtkDMMLSequenceNode> readSequenceNode = vtkSmartPointer<vtkDMMLSequenceNode>::Take(
    vtkDMMLSequenceNode::SafeDownCast(sequenceNode->CreateNodeInstance()));
  scene->AddNode(readSequenceNode);
  vtkSmartPointer<vtkDMMLStorageNode> readStorageNode = vtkSmartPointer<vtkDMMLStorageNode>::Take(
    vtkDMMLStorageNode::SafeDownCast(storageNode->CreateNodeInstance()));
  scene->AddNode(readStorageNode);

  std::cout << "Testing sequence read: " << fullFilePath << std::endl;
  readStorageNode->SetFileName(fullFilePath.c_str());
  CHECK_BOOL(readStorageNode->ReadData(readSequenceNode), true);
  CHECK_INT(readSequenceNode->GetNumberOfDataNodes(), sequenceNode->GetNumberOfDataNodes());
  return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
int vtkDMMLSequenceStorageNodeTest1( int argc, char * argv[] )
{
  std::string tempDir = ".";
  if (argc > 1)
    {
    tempDir = argv[1];
    }

  vtkNew<vtkDMMLScene> scene;
  scene->SetDataIOManager(vtkNew<vtkDataIOManager>());
  scene->GetDataIOManager()->SetCacheManager(vtkNew<vtkCacheManager>());
  scene->GetDataIOManager()->GetCacheManager()->SetRemoteCacheDirectory(tempDir.c_str());

  // Add generic node sequence
  {
    vtkSmartPointer<vtkDMMLSequenceNode> genericSequenceNode = vtkDMMLSequenceNode::SafeDownCast(scene->AddNewNodeByClass("vtkDMMLSequenceNode"));
    vtkSmartPointer<vtkDMMLModelNode> modelNode = vtkDMMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkDMMLModelNode"));
    genericSequenceNode->SetDataNodeAtValue(modelNode.GetPointer(), "0");
    genericSequenceNode->AddDefaultStorageNode();
    vtkSmartPointer<vtkDMMLSequenceStorageNode> addedGenericStorageNode = vtkDMMLSequenceStorageNode::SafeDownCast(genericSequenceNode->GetStorageNode());
    CHECK_NOT_NULL(addedGenericStorageNode);
    CHECK_EXIT_SUCCESS(TestWriteReadSequence(tempDir, genericSequenceNode, addedGenericStorageNode, "TestGenericSequence"));
  }

  // Add volume node sequence
  {
    vtkSmartPointer<vtkDMMLSequenceNode> imageSequenceNode = vtkDMMLSequenceNode::SafeDownCast(scene->AddNewNodeByClass("vtkDMMLSequenceNode"));
    vtkNew<vtkImageData> image;
    image->SetDimensions(10, 10, 1);
    image->AllocateScalars(VTK_CHAR, 1);
    vtkSmartPointer<vtkDMMLScalarVolumeNode> volumeNode = vtkDMMLScalarVolumeNode::SafeDownCast(scene->AddNewNodeByClass("vtkDMMLScalarVolumeNode"));
    volumeNode->SetAndObserveImageData(image);
    imageSequenceNode->SetDataNodeAtValue(volumeNode.GetPointer(), "0");
    imageSequenceNode->AddDefaultStorageNode();
    vtkSmartPointer<vtkDMMLVolumeSequenceStorageNode> addedVolumeStorageNode
      = vtkDMMLVolumeSequenceStorageNode::SafeDownCast(imageSequenceNode->GetStorageNode());
    CHECK_EXIT_SUCCESS(TestWriteReadSequence(tempDir, imageSequenceNode, addedVolumeStorageNode, "TestImageSequence"));
  }

  // Add transform node sequence
  {
    vtkSmartPointer<vtkDMMLSequenceNode> transformSequenceNode = vtkDMMLSequenceNode::SafeDownCast(scene->AddNewNodeByClass("vtkDMMLSequenceNode"));
    vtkSmartPointer<vtkDMMLLinearTransformNode> transformNode
      = vtkDMMLLinearTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkDMMLLinearTransformNode"));
    transformSequenceNode->SetDataNodeAtValue(transformNode.GetPointer(), "0");
    transformSequenceNode->AddDefaultStorageNode();
    vtkSmartPointer<vtkDMMLLinearTransformSequenceStorageNode> addedTransformStorageNode
      = vtkDMMLLinearTransformSequenceStorageNode::SafeDownCast(transformSequenceNode->GetStorageNode());
    CHECK_NOT_NULL(addedTransformStorageNode);
    CHECK_EXIT_SUCCESS(TestWriteReadSequence(tempDir, transformSequenceNode, addedTransformStorageNode, "TestTransformSequence"));
  }

  // Create generic node sequence
  {
    vtkSmartPointer<vtkDMMLSequenceNode> genericSequenceNode = vtkDMMLSequenceNode::SafeDownCast(scene->AddNewNodeByClass("vtkDMMLSequenceNode"));
    vtkSmartPointer<vtkDMMLModelNode> modelNode = vtkDMMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkDMMLModelNode"));
    genericSequenceNode->SetDataNodeAtValue(modelNode.GetPointer(), "0");
    vtkSmartPointer<vtkDMMLSequenceStorageNode> createdGenericStorageNode = vtkSmartPointer<vtkDMMLSequenceStorageNode>::Take(
      vtkDMMLSequenceStorageNode::SafeDownCast(genericSequenceNode->CreateDefaultStorageNode()));
    CHECK_NOT_NULL(createdGenericStorageNode);
  }

  // Create volume node sequence
  {
    vtkSmartPointer<vtkDMMLSequenceNode> imageSequenceNode = vtkDMMLSequenceNode::SafeDownCast(scene->AddNewNodeByClass("vtkDMMLSequenceNode"));
    vtkSmartPointer<vtkDMMLScalarVolumeNode> volumeNode = vtkDMMLScalarVolumeNode::SafeDownCast(scene->AddNewNodeByClass("vtkDMMLScalarVolumeNode"));
    imageSequenceNode->SetDataNodeAtValue(volumeNode.GetPointer(), "0");
    vtkSmartPointer<vtkDMMLVolumeSequenceStorageNode> createdVolumeStorageNode = vtkSmartPointer<vtkDMMLVolumeSequenceStorageNode>::Take(
      vtkDMMLVolumeSequenceStorageNode::SafeDownCast(imageSequenceNode->CreateDefaultStorageNode()));
    CHECK_NOT_NULL(createdVolumeStorageNode);
  }

  // Create transform node sequence
  {
    vtkSmartPointer<vtkDMMLSequenceNode> transformSequenceNode = vtkDMMLSequenceNode::SafeDownCast(scene->AddNewNodeByClass("vtkDMMLSequenceNode"));
    vtkSmartPointer<vtkDMMLLinearTransformNode> transformNode
      = vtkDMMLLinearTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkDMMLLinearTransformNode"));
    transformSequenceNode->SetDataNodeAtValue(transformNode.GetPointer(), "0");
    vtkSmartPointer<vtkDMMLLinearTransformSequenceStorageNode> createdTransformStorageNode
      = vtkSmartPointer<vtkDMMLLinearTransformSequenceStorageNode>::Take(
      vtkDMMLLinearTransformSequenceStorageNode::SafeDownCast(transformSequenceNode->CreateDefaultStorageNode()));
    CHECK_NOT_NULL(createdTransformStorageNode);
  }

  return EXIT_SUCCESS;
}
