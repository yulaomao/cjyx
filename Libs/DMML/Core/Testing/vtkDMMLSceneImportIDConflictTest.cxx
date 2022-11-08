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
#include "vtkDMMLModelDisplayNode.h"
#include "vtkDMMLModelNode.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkPolyData.h>

// STD includes
#include <vtkNew.h>

using namespace vtkDMMLCoreTestingUtilities;

//---------------------------------------------------------------------------
int vtkDMMLSceneImportIDConflictTest(int vtkNotUsed(argc), char * vtkNotUsed(argv) [])
{
  vtkNew<vtkDMMLScene> scene;

  // Add displayable node
  vtkNew<vtkDMMLModelNode> modelNode;
  scene->AddNode(modelNode.GetPointer());

  // add poly data
  vtkNew<vtkPolyData> polyData;
  modelNode->SetAndObservePolyData(polyData.GetPointer());
  std::cout << "Polydata pointer = " << polyData.GetPointer() << std::endl;

  // Add display node
  vtkNew<vtkDMMLModelDisplayNode> modelDisplayNode;
  scene->AddNode(modelDisplayNode.GetPointer());
  modelNode->SetAndObserveDisplayNodeID(modelDisplayNode->GetID());

  // At this point the scene should be:
  //
  //  Scene
  //    |---- vtkDMMLModelNode1  (valid polydata)
  //    |          |-- ref [displayNodeRef] to vtkDMMLModelDisplayNode1
  //    |
  //    |---- vtkDMMLModelDisplayNode1 (valid polydata)

  CHECK_INT(scene->GetNumberOfNodes(), 2);
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(), "vtkDMMLModelNode1",  modelNode.GetPointer());
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(), "vtkDMMLModelDisplayNode1",  modelDisplayNode.GetPointer());
  CHECK_POINTER(modelNode->GetDisplayNode(), modelDisplayNode.GetPointer());

  //
  // Import
  //

  // Here is the scene that will be imported:
  //
  //  Scene
  //    |---- vtkDMMLModelNode1  (null polydata / New Model1)
  //    |          |-- ref [displayNodeRef] to vtkDMMLModelDisplayNode1
  //    |
  //    |---- vtkDMMLModelDisplayNode1 (null polydata / New Display 1)
  //    |
  //    |---- vtkDMMLModelDisplayNode2 (null polydata / New Display 2)
  //    |
  //    |---- vtkDMMLModelNode2  (null polydata / New Model2)
  //               |-- ref [displayNodeRef] to vtkDMMLModelDisplayNode2

  const char scene1XML[] =
    "<DMML  version=\"18916\" userTags=\"\">"
    "  <Model id=\"vtkDMMLModelNode1\" name=\"New Model1\" displayNodeRef=\"vtkDMMLModelDisplayNode1\" ></Model>"
    "  <ModelDisplay id=\"vtkDMMLModelDisplayNode1\" name=\"New Display 1\" ></ModelDisplay>"
    "  <ModelDisplay id=\"vtkDMMLModelDisplayNode2\" name=\"New Display 2\" ></ModelDisplay>"
    "  <Model id=\"vtkDMMLModelNode2\" name=\"New Model2\" displayNodeRef=\"vtkDMMLModelDisplayNode2\" ></Model>"
    "</DMML>"
    ;

  scene->SetSceneXMLString(scene1XML);
  scene->SetLoadFromXMLString(1);
  scene->Import();  // adds Subject Hierarchy Node

  // When importing the scene, there is conflict between the existing nodes
  // and added nodes. New IDs are set by Import to the added nodes.

  // At this point the scene should be:
  //
  //  Scene
  //    |---- vtkDMMLSubjectHierarchyNode1
  //    |---- vtkDMMLModelNode1  (valid polydata)
  //    |          |-- ref [displayNodeRef] to vtkDMMLModelDisplayNode1
  //    |
  //    |---- vtkDMMLModelDisplayNode1 (valid polydata)
  //    |
  //    |---- vtkDMMLModelNode2  (null polydata / New Model1)            [was vtkDMMLModelNode1]
  //    |          |-- ref [displayNodeRef] to vtkDMMLModelDisplayNode2
  //    |
  //    |---- vtkDMMLModelDisplayNode2 (null polydata / New Display 1)   [was vtkDMMLModelDisplayNode1]
  //    |
  //    |---- vtkDMMLModelDisplayNode3 (null polydata / New Display 2)   [was vtkDMMLModelDisplayNode2]
  //    |
  //    |---- vtkDMMLModelNode3  (null polydata / New Model2)            [was vtkDMMLModelNode2]
  //               |-- ref [displayNodeRef] to vtkDMMLModelDisplayNode3

  //
  // Check scene contains original nodes
  //

  CHECK_INT(scene->GetNumberOfNodes(), 7);
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkDMMLModelNode1", modelNode.GetPointer());
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkDMMLModelDisplayNode1", modelDisplayNode.GetPointer());
  CHECK_POINTER(modelNode->GetDisplayNode(), modelDisplayNode.GetPointer());

  //
  // Part 1
  //

  vtkDMMLModelNode* modelNode2 =
      vtkDMMLModelNode::SafeDownCast(scene->GetNodeByID("vtkDMMLModelNode3"));

  CHECK_NOT_NULL(modelNode2);
  CHECK_NODE_ID_AND_NAME(modelNode2, "vtkDMMLModelNode3", "New Model1");
  CHECK_NODE_ID_AND_NAME(modelNode2->GetDisplayNode(), "vtkDMMLModelDisplayNode3", "New Display 1");

  //
  // Part2
  //

  vtkDMMLModelNode* modelNode3 =
      vtkDMMLModelNode::SafeDownCast(scene->GetNodeByID("vtkDMMLModelNode2"));

  CHECK_NOT_NULL(modelNode3);
  CHECK_NODE_ID_AND_NAME(modelNode3, "vtkDMMLModelNode2", "New Model2");
  CHECK_NODE_ID_AND_NAME(modelNode3->GetDisplayNode(), "vtkDMMLModelDisplayNode2", "New Display 2");

  //
  // Check PolyData / InputPolyData
  //

  vtkDMMLModelDisplayNode* modelDisplayNode2 =
      vtkDMMLModelDisplayNode::SafeDownCast(modelNode2->GetDisplayNode());

  // check that the model nodes and model display nodes point to the right poly data

  CHECK_NULL(modelNode2->GetPolyData()); // new model node should have null polydata
  CHECK_NULL(modelDisplayNode2->GetInputPolyData()); // new model node's display node should have null polydata
  CHECK_NOT_NULL(modelNode->GetPolyData()); // original model node should not have null polydata
  CHECK_NOT_NULL(modelDisplayNode->GetInputPolyData()); // original model display node should not have null polydata
  CHECK_POINTER(modelNode->GetPolyData(), modelDisplayNode->GetInputPolyData()); // original model node and display node don't have the same poly data

  return EXIT_SUCCESS;
}
