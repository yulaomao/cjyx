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

int populateScene(vtkDMMLScene* scene, bool saveInSceneView)
{
  vtkNew<vtkDMMLSceneViewNode> sceneViewtoRegister;
  scene->RegisterNodeClass(sceneViewtoRegister.GetPointer());

  vtkNew<vtkDMMLScalarVolumeNode> displayableNode;
  scene->AddNode(displayableNode.GetPointer());

  vtkNew<vtkDMMLScalarVolumeDisplayNode> displayNode;
  scene->AddNode(displayNode.GetPointer());

  displayableNode->SetAndObserveDisplayNodeID(displayNode->GetID());

  if (saveInSceneView)
    {
    vtkNew<vtkDMMLSceneViewNode> sceneViewNode;
    scene->AddNode(sceneViewNode.GetPointer());

    sceneViewNode->StoreScene();
    }

  scene->RemoveNode(displayableNode.GetPointer());
  return EXIT_SUCCESS;
}

} // end of anonymous namespace

//---------------------------------------------------------------------------
int vtkDMMLSceneViewNodeImportSceneTest(int vtkNotUsed(argc),
                                       char * vtkNotUsed(argv)[] )
{
  // Save a scene containing a viewnode and a sceneview node.
  vtkNew<vtkDMMLScene> scene;
  CHECK_EXIT_SUCCESS(populateScene(scene.GetPointer(), true));

  // scene
  //   + vtkDMMLScalarVolumeDisplayNode1
  //   + vtkDMMLSceneViewNode1
  //       + vtkDMMLScalarVolumeNode1
  //            -> ref: vtkDMMLScalarVolumeDisplayNode1
  //       + vtkDMMLScalarVolumeDisplayNode1

  scene->SetSaveToXMLString(1);
  scene->Commit();
  std::string xmlScene = scene->GetSceneXMLString();
  std::cout << xmlScene << std::endl;

  // Simulate another scene
  vtkNew<vtkDMMLScene> scene2;
  CHECK_EXIT_SUCCESS(populateScene(scene2.GetPointer(), false));

  // scene2
  //   + vtkDMMLScalarVolumeDisplayNode1

  scene2->SetLoadFromXMLString(1);
  scene2->SetSceneXMLString(xmlScene);
  scene2->Import();

  // scene2
  //   + vtkDMMLScalarVolumeDisplayNode1 (original)
  //   + vtkDMMLScalarVolumeDisplayNode2 (imported)
  //   + vtkDMMLSceneViewNode1 (imported)
  //       + vtkDMMLScalarVolumeNode1
  //            -> ref: vtkDMMLScalarVolumeDisplayNode2
  //       + vtkDMMLScalarVolumeDisplayNode2

  // Check scene node IDs
  vtkDMMLNode* displayNode =
    scene2->GetFirstNodeByClass("vtkDMMLScalarVolumeDisplayNode");
  vtkDMMLNode* displayNode2 = scene2->GetNthNodeByClass(1, "vtkDMMLScalarVolumeDisplayNode");
  vtkDMMLSceneViewNode* sceneViewNode = vtkDMMLSceneViewNode::SafeDownCast(
    scene2->GetFirstNodeByClass("vtkDMMLSceneViewNode"));

  CHECK_NOT_NULL(displayNode);
  CHECK_NOT_NULL(displayNode2);
  CHECK_NOT_NULL(sceneViewNode);
  CHECK_STRING(displayNode->GetID(), "vtkDMMLScalarVolumeDisplayNode1");
  CHECK_STRING(displayNode2->GetID(), "vtkDMMLScalarVolumeDisplayNode2");
  CHECK_STRING(sceneViewNode->GetID(), "vtkDMMLSceneViewNode1");

  // Check sceneViewNode node IDs.
  vtkDMMLNode* sceneViewDisplayNode =
    sceneViewNode->GetStoredScene()->GetFirstNodeByClass("vtkDMMLScalarVolumeDisplayNode");
  vtkDMMLDisplayableNode* sceneViewDisplayableNode = vtkDMMLDisplayableNode::SafeDownCast(
    sceneViewNode->GetStoredScene()->GetFirstNodeByClass("vtkDMMLScalarVolumeNode"));

  CHECK_INT(sceneViewNode->GetStoredScene()->GetNumberOfNodes(), 3);
  CHECK_NOT_NULL(sceneViewDisplayNode);
  CHECK_NOT_NULL(sceneViewDisplayableNode);
  CHECK_STRING(sceneViewDisplayNode->GetID(), "vtkDMMLScalarVolumeDisplayNode2");
  CHECK_STRING(sceneViewDisplayableNode->GetID(), "vtkDMMLScalarVolumeNode1");

  // Check references
  CHECK_STRING(sceneViewDisplayableNode->GetNthDisplayNodeID(0), sceneViewDisplayNode->GetID());

  return EXIT_SUCCESS;
}
