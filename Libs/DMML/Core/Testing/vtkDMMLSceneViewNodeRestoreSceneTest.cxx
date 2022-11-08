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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// DMML includes
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLScalarVolumeDisplayNode.h"
#include "vtkDMMLScalarVolumeNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSceneViewNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>

namespace
{

vtkDMMLScene* createScene();
int restoreEditAndRestore();
int removeRestoreEditAndRestore();

} // end of anonymous namespace

//---------------------------------------------------------------------------
int vtkDMMLSceneViewNodeRestoreSceneTest(int vtkNotUsed(argc),
                                         char * vtkNotUsed(argv)[] )
{
  CHECK_EXIT_SUCCESS(restoreEditAndRestore());
  CHECK_EXIT_SUCCESS(removeRestoreEditAndRestore());
  return EXIT_SUCCESS;
}

namespace
{

//---------------------------------------------------------------------------
vtkDMMLScene* createScene()
{
  vtkDMMLScene* scene = vtkDMMLScene::New();

  vtkNew<vtkDMMLScalarVolumeDisplayNode> displayNode;
  scene->AddNode(displayNode.GetPointer());

  vtkNew<vtkDMMLScalarVolumeNode> volumeNode;
  volumeNode->SetScene(scene);
  volumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());
  scene->AddNode(volumeNode.GetPointer());

  return scene;
}

//---------------------------------------------------------------------------
int restoreEditAndRestore()
{
  vtkSmartPointer<vtkDMMLScene> scene;
  scene.TakeReference(createScene());

  vtkNew<vtkDMMLSceneViewNode> sceneViewNode;
  scene->AddNode(sceneViewNode.GetPointer());

  sceneViewNode->StoreScene();

  sceneViewNode->RestoreScene();

  vtkDMMLScalarVolumeNode* volumeNode = vtkDMMLScalarVolumeNode::SafeDownCast(
    scene->GetNodeByID("vtkDMMLScalarVolumeNode1"));
  volumeNode->SetAndObserveDisplayNodeID("vtkDMMLScalarVolumeDisplayNode2");

  sceneViewNode->RestoreScene();

  CHECK_STRING(volumeNode->GetDisplayNodeID(), "vtkDMMLScalarVolumeDisplayNode1");

  return EXIT_SUCCESS;
}


//---------------------------------------------------------------------------
int removeRestoreEditAndRestore()
{
  vtkSmartPointer<vtkDMMLScene> scene;
  scene.TakeReference(createScene());

  vtkNew<vtkDMMLSceneViewNode> sceneViewNode;
  scene->AddNode(sceneViewNode.GetPointer());

  sceneViewNode->StoreScene();

  vtkDMMLScalarVolumeNode* volumeNode = vtkDMMLScalarVolumeNode::SafeDownCast(
    scene->GetNodeByID("vtkDMMLScalarVolumeNode1"));
  scene->RemoveNode(volumeNode);

  // TODO: We expect errors here because of https://github.com/Slicer/Slicer/issues/2816 is not resolved.
  // Once that bug is fixed, RestoreScene() should not throw any errors, and so the
  // TESTING_OUTPUT_ASSERT_ERRORS_BEGIN/END macros should be removed.
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  sceneViewNode->RestoreScene();
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  vtkDMMLScalarVolumeNode* restoredVolumeNode = vtkDMMLScalarVolumeNode::SafeDownCast(
    scene->GetNodeByID("vtkDMMLScalarVolumeNode1"));
  restoredVolumeNode->SetAndObserveDisplayNodeID("vtkDMMLScalarVolumeDisplayNode2");

  sceneViewNode->RestoreScene();

  CHECK_NOT_NULL(restoredVolumeNode->GetDisplayNodeID());
  CHECK_STRING(restoredVolumeNode->GetDisplayNodeID(), "vtkDMMLScalarVolumeDisplayNode1");

  return EXIT_SUCCESS;
}

} // end of anonymous namespace
