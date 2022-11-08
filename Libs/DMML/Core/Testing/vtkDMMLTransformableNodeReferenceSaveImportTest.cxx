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
#include "vtkDMMLLinearTransformNode.h"
#include "vtkDMMLTransformableNode.h"
#include "vtkDMMLScalarVolumeNode.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkNew.h>

namespace
{

int TestImportIntoEmptyScene();
int TestImportIntoSceneWithNodeIdConflict();

} // end of anonymous namespace

//---------------------------------------------------------------------------
int vtkDMMLTransformableNodeReferenceSaveImportTest(int vtkNotUsed(argc),
                                       char * vtkNotUsed(argv)[] )
{
  CHECK_EXIT_SUCCESS(TestImportIntoEmptyScene());
  CHECK_EXIT_SUCCESS(TestImportIntoSceneWithNodeIdConflict());
  return EXIT_SUCCESS;
}

namespace
{

//---------------------------------------------------------------------------
int TestImportIntoEmptyScene()
{
  // Create scene1

  vtkNew<vtkDMMLScene> scene1;

  vtkNew<vtkDMMLScalarVolumeNode> scene1TransformableNode1;
  scene1TransformableNode1->SetName("Transformable1");
  scene1->AddNode(scene1TransformableNode1);

  vtkNew<vtkDMMLLinearTransformNode> scene1TransformNode1;
  scene1TransformNode1->SetName("Transform1");
  scene1->AddNode(scene1TransformNode1);
  scene1TransformableNode1->SetAndObserveTransformNodeID(scene1TransformNode1->GetID());

  // scene1:
  //   +-Transformable1 -> Transform1
  //   +-Transform1

  // Write scene1 to string
  scene1->SetSaveToXMLString(1);
  scene1->Commit();
  std::string xmlScene1 = scene1->GetSceneXMLString();
  std::cout << xmlScene1 << std::endl;

  // Import scene1 into scene2
  vtkNew<vtkDMMLScene> scene2;
  scene2->SetLoadFromXMLString(1);
  scene2->SetSceneXMLString(xmlScene1);
  scene2->Import();

  // Check transform node IDs
  vtkDMMLLinearTransformNode* scene2TransformNode1 =
    vtkDMMLLinearTransformNode::SafeDownCast(scene2->GetFirstNodeByName("Transform1"));
  CHECK_NOT_NULL(scene2TransformNode1);
  CHECK_STRING(scene2TransformNode1->GetID(), "vtkDMMLLinearTransformNode1");

  // Check references
  vtkDMMLTransformableNode* scene2TransformableNode1 =
    vtkDMMLTransformableNode::SafeDownCast(scene2->GetFirstNodeByName("Transformable1"));
  CHECK_NOT_NULL(scene2TransformableNode1);
  CHECK_STRING(scene2TransformableNode1->GetTransformNodeID(), scene2TransformNode1->GetID());

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestImportIntoSceneWithNodeIdConflict()
{
  // Create scene1

  vtkNew<vtkDMMLScene> scene1;

  vtkNew<vtkDMMLScalarVolumeNode> scene1TransformableNode1;
  scene1TransformableNode1->SetName("Transformable1");
  scene1->AddNode(scene1TransformableNode1);

  vtkNew<vtkDMMLLinearTransformNode> scene1TransformNode1;
  scene1TransformNode1->SetName("Transform1");
  scene1->AddNode(scene1TransformNode1);
  scene1TransformableNode1->SetAndObserveTransformNodeID(scene1TransformNode1->GetID());

  // scene1:
  //   +-Transformable1 -> Transform1
  //   +-Transform1

  // Write scene1 to string
  scene1->SetSaveToXMLString(1);
  scene1->Commit();
  std::string xmlScene1 = scene1->GetSceneXMLString();

  // Create scene2

  vtkNew<vtkDMMLScene> scene2;

  vtkNew<vtkDMMLScalarVolumeNode> scene2TransformableNode2;
  scene2TransformableNode2->SetName("Transformable2");
  scene2->AddNode(scene2TransformableNode2);

  vtkNew<vtkDMMLLinearTransformNode> scene2TransformNode2;
  scene2TransformNode2->SetName("Transform2");
  scene2->AddNode(scene2TransformNode2);
  scene2TransformableNode2->SetAndObserveTransformNodeID(scene2TransformNode2->GetID());

  // scene2:
  //   +-Transformable2 -> Transform2
  //   +-Transform2

  // Import scene1 into scene2
  scene2->SetLoadFromXMLString(1);
  scene2->SetSceneXMLString(xmlScene1);
  scene2->Import();

  // scene2:
  //   +-Transformable2 -> Transform2
  //   +-Transform2
  //   +-Transformable1 -> Transform1
  //   +-Transform1

  // Check transform1
  vtkDMMLLinearTransformNode* scene2TransformNode1 =
    vtkDMMLLinearTransformNode::SafeDownCast(scene2->GetFirstNodeByName("Transform1"));
  CHECK_NOT_NULL(scene2TransformNode1);
  CHECK_STRING_DIFFERENT(scene2TransformNode1->GetID(), "vtkDMMLLinearTransformNode1");

  // Check transform1 references
  vtkDMMLTransformableNode* scene2TransformableNode1 =
    vtkDMMLTransformableNode::SafeDownCast(scene2->GetFirstNodeByName("Transformable1"));
  CHECK_NOT_NULL(scene2TransformableNode1);
  CHECK_STRING(scene2TransformableNode1->GetTransformNodeID(), scene2TransformNode1->GetID());

  // Check transform2
  CHECK_STRING(scene2TransformNode2->GetID(), "vtkDMMLLinearTransformNode1");

  // Check transform2 references
  CHECK_STRING(scene2TransformableNode2->GetTransformNodeID(), scene2TransformNode2->GetID());

  return EXIT_SUCCESS;
}

}
