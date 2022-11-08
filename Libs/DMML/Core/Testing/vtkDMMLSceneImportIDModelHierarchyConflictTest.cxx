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

==============================================================================*/

// DMML includes
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLModelDisplayNode.h"
#include "vtkDMMLModelHierarchyNode.h"
#include "vtkDMMLModelNode.h"
#include "vtkDMMLScene.h"

// STD includes
#include <vtkNew.h>
#include <vtkPolyData.h>

using namespace vtkDMMLCoreTestingUtilities;

//---------------------------------------------------------------------------
int ImportIDModelHierarchyConflictTest();
int ImportModelHierarchyTwiceTest();

//---------------------------------------------------------------------------
int vtkDMMLSceneImportIDModelHierarchyConflictTest(int vtkNotUsed(argc), char * vtkNotUsed(argv) [])
{
  bool res = true;
  res = res && (ImportIDModelHierarchyConflictTest() == EXIT_SUCCESS);
  res = res && (ImportModelHierarchyTwiceTest() == EXIT_SUCCESS);
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//---------------------------------------------------------------------------
// The test makes sure the model hierarchy nodes correctly support node ID
// conflict. There are 2 steps in this test
// a) populates a scene with a model with flat hierarchy node
// b) and imports a similar scene into the existing scene.
int ImportIDModelHierarchyConflictTest()
{
  vtkNew<vtkDMMLScene> scene;

  // Add model node
  vtkNew<vtkDMMLModelNode> modelNode;
  scene->AddNode(modelNode.GetPointer());

  // add poly data
  vtkNew<vtkPolyData> polyData;
  modelNode->SetAndObservePolyData(polyData.GetPointer());

  // Add display node
  vtkNew<vtkDMMLModelDisplayNode> modelDisplayNode;
  scene->AddNode(modelDisplayNode.GetPointer());
  modelNode->SetAndObserveDisplayNodeID(modelDisplayNode->GetID());

  CHECK_INT(scene->GetNumberOfNodes(), 2);
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkDMMLModelNode1", modelNode.GetPointer());
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkDMMLModelDisplayNode1", modelDisplayNode.GetPointer());
  CHECK_POINTER(modelNode->GetDisplayNode(), modelDisplayNode.GetPointer());

  // does the display node point to the correct polydata?
  CHECK_POINTER(modelDisplayNode->GetInputPolyData(), modelNode->GetPolyData());

  // add a model hierarchy node
  vtkNew<vtkDMMLModelDisplayNode> hierachyDisplayNode;
  scene->AddNode(hierachyDisplayNode.GetPointer());

  vtkNew<vtkDMMLModelHierarchyNode> hierarchyNode;
  scene->AddNode(hierarchyNode.GetPointer());

  hierarchyNode->SetAndObserveDisplayNodeID(hierachyDisplayNode->GetID());
  hierarchyNode->SetAssociatedNodeID(modelNode->GetID());

  CHECK_INT(scene->GetNumberOfNodes(), 4);

  // Note about vtkDMMLModelHierarchyNode reference role attribute names:
  //
  // For DisplayNodeID:
  //  - displayNodeID -> preferred
  //  - displayNodeRef -> deprecated
  //
  // For AssociatedNodeID:
  //  - associatedNodeRef -> preferred
  //  - modelNodeRef and modelNodeID -> deprecated

  // At this point the scene should be:
  //
  //  Scene
  //    |---- vtkDMMLModelNode1  (valid polydata)
  //    |          |-- ref [displayNodeRef] to vtkDMMLModelDisplayNode1
  //    |
  //    |---- vtkDMMLModelDisplayNode1 (valid polydata)
  //    |
  //    |---- vtkDMMLModelDisplayNode2 (null polydata)
  //    |
  //    |---- vtkDMMLModelHierarchyNode1
  //               |-- ref [displayNodeID] to vtkDMMLModelDisplayNode2
  //               |-- ref [associatedNodeRef] to vtkDMMLModelNode1


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
  //    |---- vtkDMMLModelHierarchyNode1  (null polydata / vtkDMMLModelHierarchyNode1)
  //               |-- ref [displayNodeID] to vtkDMMLModelDisplayNode2
  //               |-- ref [associatedNodeRef] to vtkDMMLModelNode1


  const char scene1XML[] =
    "<DMML  version=\"18916\" userTags=\"\">"
    "  <Model id=\"vtkDMMLModelNode1\" name=\"New Model1\" displayNodeRef=\"vtkDMMLModelDisplayNode1\" ></Model>"
    "  <ModelDisplay id=\"vtkDMMLModelDisplayNode1\" name=\"New Display 1\" ></ModelDisplay>"
    "  <ModelDisplay id=\"vtkDMMLModelDisplayNode2\" name=\"New Display 2\" ></ModelDisplay>"
    "  <ModelHierarchy id=\"vtkDMMLModelHierarchyNode1\" name=\"vtkDMMLModelHierarchyNode1\"  hideFromEditors=\"true\"  selectable=\"true\" displayNodeRef=\"vtkDMMLModelDisplayNode2\"  expanded=\"true\" modelNodeRef=\"vtkDMMLModelNode1\"></ModelHierarchy>"
    "</DMML>"
    ;

  scene->SetSceneXMLString(scene1XML);
  scene->SetLoadFromXMLString(1);

  // When importing the scene, there is conflict between the existing nodes
  // and added nodes. New IDs are set by Import to the added nodes.
  // The node ids in the scene after a proper import should be

  scene->Import();  // adds Subject Hierarchy Node

  // At this point the scene should be:
  //
  //  Scene
  //    |---- vtkDMMLSubjectHierarchyNode1
  //    |---- vtkDMMLModelNode1  (valid polydata)
  //    |          |-- ref [displayNodeRef] to vtkDMMLModelDisplayNode1
  //    |
  //    |---- vtkDMMLModelDisplayNode1 (valid polydata)
  //    |
  //    |---- vtkDMMLModelDisplayNode2 (null polydata)
  //    |
  //    |---- vtkDMMLModelHierarchyNode1
  //    |          |-- ref [displayNodeID] to vtkDMMLModelDisplayNode2
  //    |          |-- ref [associatedNodeRef] to vtkDMMLModelNode1
  //    |
  //    |---- vtkDMMLModelNode2  (null polydata / New Model1)             [was vtkDMMLModelNode1]
  //    |          |-- ref [displayNodeRef] to vtkDMMLModelDisplayNode3
  //    |
  //    |---- vtkDMMLModelDisplayNode3 (null polydata / New Display 1)    [was vtkDMMLModelDisplayNode1]
  //    |
  //    |---- vtkDMMLModelDisplayNode4 (null polydata / New Display 2)    [was vtkDMMLModelDisplayNode2]
  //    |
  //    |---- vtkDMMLModelHierarchyNode2  (null polydata / vtkDMMLModelHierarchyNode1) [was vtkDMMLModelHierarchyNode1]
  //               |-- ref [displayNodeID] to vtkDMMLModelDisplayNode4
  //               |-- ref [associatedNodeRef] to vtkDMMLModelNode2

  CHECK_INT(scene->GetNumberOfNodes(), 9);
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkDMMLModelNode1", modelNode.GetPointer());
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkDMMLModelDisplayNode1", modelDisplayNode.GetPointer());
  CHECK_POINTER(modelNode->GetDisplayNode(), modelDisplayNode.GetPointer());

  vtkDMMLModelNode* modelNode2 = vtkDMMLModelNode::SafeDownCast(
    scene->GetNodeByID("vtkDMMLModelNode2"));

  CHECK_NODE_ID_AND_NAME(modelNode2, "vtkDMMLModelNode2", "New Model1");
  CHECK_NODE_ID_AND_NAME(modelNode2->GetDisplayNode(), "vtkDMMLModelDisplayNode3", "New Display 1");

  // check that the hierarchies point to the right display nodes
  vtkDMMLModelHierarchyNode *hierarchyNode2 =
      vtkDMMLModelHierarchyNode::SafeDownCast(scene->GetNodeByID("vtkDMMLModelHierarchyNode2"));

  vtkDMMLModelDisplayNode* modelDisplayNode2 =
      vtkDMMLModelDisplayNode::SafeDownCast(modelNode2->GetDisplayNode());

  CHECK_NOT_NULL(hierarchyNode2);
  CHECK_STRING(hierarchyNode2->GetDisplayNodeID(), "vtkDMMLModelDisplayNode4");
  CHECK_STRING(hierarchyNode2->GetAssociatedNodeID(), "vtkDMMLModelNode2");

  // check that the model nodes and model display nodes point to the right poly data
  CHECK_NULL(modelNode2->GetPolyData()); // new model node should have null polydata
  CHECK_NULL(modelDisplayNode2->GetInputPolyData()); // new model node's display node should have null polydata
  CHECK_NOT_NULL(modelNode->GetPolyData()); // original model node should not have null polydata
  CHECK_NOT_NULL(modelDisplayNode->GetInputPolyData()); // original model display node should not have null polydata
  CHECK_POINTER(modelNode->GetPolyData(), modelDisplayNode->GetInputPolyData()); // original model node and display node don't have the same poly data

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int ImportModelHierarchyTwiceTest()
{
  vtkNew<vtkDMMLScene> scene;

  // Add model node
  vtkNew<vtkDMMLModelNode> modelNode;
  scene->AddNode(modelNode.GetPointer());
  vtkNew<vtkDMMLModelDisplayNode> hierachyDisplayNode;
  scene->AddNode(hierachyDisplayNode.GetPointer());

  // Add a model hierarchy node
  vtkNew<vtkDMMLModelHierarchyNode> modelHierarchyNode;
  scene->AddNode(modelHierarchyNode.GetPointer());
  modelHierarchyNode->SetAndObserveDisplayNodeID(hierachyDisplayNode->GetID());
  modelHierarchyNode->SetAssociatedNodeID(modelNode->GetID());

  vtkNew<vtkDMMLHierarchyNode> hierarchyNode;
  scene->AddNode(hierarchyNode.GetPointer());
  modelHierarchyNode->SetParentNodeID(hierarchyNode->GetID());

  // At this point the scene should be:
  //
  //  Scene
  //    |---- vtkDMMLModelNode1
  //    |
  //    |---- vtkDMMLModelDisplayNode1
  //    |
  //    |---- vtkDMMLModelHierarchyNode1
  //    |          |-- ref [displayNodeID] to vtkDMMLModelDisplayNode1
  //    |          |-- ref [associatedNodeRef] to vtkDMMLModelNode1
  //    |          |-- ref [parentNodeRef] to vtkDMMLHierarchyNode1
  //    |
  //    |---- vtkDMMLHierarchyNode1

  CHECK_INT(scene->GetNumberOfNodes(), 4);
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkDMMLModelNode1", modelNode.GetPointer());
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkDMMLModelDisplayNode1", hierachyDisplayNode.GetPointer());
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkDMMLModelHierarchyNode1", modelHierarchyNode.GetPointer());
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkDMMLHierarchyNode1", hierarchyNode.GetPointer());
  CHECK_POINTER(modelHierarchyNode->GetDisplayNode(), hierachyDisplayNode.GetPointer());
  CHECK_POINTER(modelHierarchyNode->GetAssociatedNode(), modelNode.GetPointer());
  CHECK_POINTER(modelHierarchyNode->GetParentNode(), hierarchyNode.GetPointer());

  //
  // Save
  //

  scene->SetSaveToXMLString(1);
  scene->Commit();
  std::string xmlScene = scene->GetSceneXMLString();
//  std::cerr << xmlScene << std::endl;

  // Load same scene into scene
  scene->SetSceneXMLString(xmlScene);
  scene->SetLoadFromXMLString(1);
  scene->Import();  // adds Subject Hierarchy Node

  // At this point the scene should be:
  //
  //  Scene
  //    |---- vtkDMMLSubjectHierarchyNode1
  //    |---- vtkDMMLModelNode1
  //    |
  //    |---- vtkDMMLModelDisplayNode1
  //    |
  //    |---- vtkDMMLModelHierarchyNode1
  //    |          |-- ref [displayNodeID] to vtkDMMLModelDisplayNode1
  //    |          |-- ref [associatedNodeRef] to vtkDMMLModelNode1
  //    |          |-- ref [parentNodeRef] to vtkDMMLHierarchyNode1
  //    |
  //    |---- vtkDMMLHierarchyNode1
  //    |
  //    |---- vtkDMMLModelNode2                                         [was vtkDMMLModelNode1]
  //    |
  //    |---- vtkDMMLModelDisplayNode2                                  [was vtkDMMLModelDisplayNode1]
  //    |
  //    |---- vtkDMMLModelHierarchyNode2
  //    |          |-- ref [displayNodeID] to vtkDMMLModelDisplayNode2
  //    |          |-- ref [associatedNodeRef] to vtkDMMLModelNode2
  //    |          |-- ref [parentNodeRef] to vtkDMMLHierarchyNode2
  //    |
  //    |---- vtkDMMLHierarchyNode2                                     [was vtkDMMLHierarchyNode1]

  CHECK_INT(scene->GetNumberOfNodes(), 9);
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkDMMLModelNode1", modelNode.GetPointer());
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkDMMLModelDisplayNode1", hierachyDisplayNode.GetPointer());
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkDMMLModelHierarchyNode1", modelHierarchyNode.GetPointer());
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkDMMLHierarchyNode1", hierarchyNode.GetPointer());
  CHECK_POINTER(modelHierarchyNode->GetDisplayNode(), hierachyDisplayNode.GetPointer());
  CHECK_POINTER(modelHierarchyNode->GetAssociatedNode(), modelNode.GetPointer());
  CHECK_POINTER(modelHierarchyNode->GetParentNode(), hierarchyNode.GetPointer());

  vtkDMMLModelHierarchyNode* modelHierarchyNode2 =
      vtkDMMLModelHierarchyNode::SafeDownCast(scene->GetNodeByID("vtkDMMLModelHierarchyNode2"));

  vtkDMMLHierarchyNode* hierarchyNode2 =
      vtkDMMLHierarchyNode::SafeDownCast(scene->GetNodeByID("vtkDMMLHierarchyNode2"));

  CHECK_NOT_NULL(modelHierarchyNode2);
  CHECK_NOT_NULL(hierarchyNode2);
  CHECK_POINTER(modelHierarchyNode2->GetParentNode(), hierarchyNode2);

  return EXIT_SUCCESS;
}
