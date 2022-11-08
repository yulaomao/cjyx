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
#include "vtkDMMLModelHierarchyNode.h"
#include "vtkDMMLScene.h"

// STD includes
#include <sstream>
#include <vector>

// VTK includes
#include <vtkNew.h>

// ITKSYS includes
#include <itksys/SystemTools.hxx>

using namespace vtkDMMLCoreTestingUtilities;

//---------------------------------------------------------------------------
int ImportIDModelHierarchyParentIDConflictTestXMLString();
int ImportIDModelHierarchyParentIDConflictTestFile();

//---------------------------------------------------------------------------
int vtkDMMLSceneImportIDModelHierarchyParentIDConflictTest(int vtkNotUsed(argc), char * vtkNotUsed(argv) [])
{
  bool res = true;
  res = res && (ImportIDModelHierarchyParentIDConflictTestXMLString() == EXIT_SUCCESS);
  res = res && (ImportIDModelHierarchyParentIDConflictTestFile() == EXIT_SUCCESS);
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//---------------------------------------------------------------------------
// add a 5 deep model hierarchy to a scene
int PopulateScene(vtkDMMLScene *scene)
{
  CHECK_INT(scene->GetNumberOfNodes(), 0);

  std::vector<vtkDMMLNode*> nodes;
  std::vector<vtkDMMLNode*> parentNodes;

  vtkDMMLNode* parentNode = nullptr;
  // add model hierarchy nodes
  for (int i = 0; i < 5; i++)
    {
    vtkNew<vtkDMMLModelHierarchyNode> mhn;
    nodes.push_back(mhn.GetPointer());
    scene->AddNode(mhn.GetPointer());
    std::string idNumberString = ToString(i);
    mhn->SetName(idNumberString.c_str());
    if (i > 0)
      {
      std::string parentNodeID = std::string("vtkDMMLModelHierarchyNode") + idNumberString;
//      std::cout << "Setting parent node id"
//                << " on node " << mhn->GetID()
//                << " to " << parentNodeID.c_str() << std::endl;
      mhn->SetParentNodeID(parentNodeID.c_str());
      }
    parentNodes.push_back(parentNode);
    parentNode = mhn.GetPointer();
    }

  CHECK_INT(scene->GetNumberOfNodes(), 5);

  for (int index = 0; index < 5; ++index)
    {
    std::string nodeID = std::string("vtkDMMLModelHierarchyNode") + ToString(index + 1);
    CHECK_NODE_IN_SCENE_BY_ID(scene, nodeID.c_str(), nodes[index]);
    }

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
// Print out the model hierarchy nodes in a scene
void PrintModelHierarchyNodes(int line, vtkDMMLScene *scene)
{
  int numNodes = scene->GetNumberOfNodesByClass("vtkDMMLModelHierarchyNode");

  std::cerr << "\nLine " << line << " - ModelHierarchyNodes:" << std::endl;

  for (int i = 0; i < numNodes; i++)
    {
    vtkDMMLNode *dmmlNode = scene->GetNthNodeByClass(i, "vtkDMMLModelHierarchyNode");
    if (dmmlNode && dmmlNode->IsA("vtkDMMLModelHierarchyNode"))
      {
      vtkDMMLModelHierarchyNode *hnode =
          vtkDMMLModelHierarchyNode::SafeDownCast(dmmlNode);
      std::cerr << "  " << i << ": Model Hierarchy node"
                << " named [" << (hnode->GetName() ? hnode->GetName() : "null")
                << "] with id [" << (hnode->GetID() ? hnode->GetID() : "null")
                << "] has parent node id of ["
                << (hnode->GetParentNodeID() ? hnode->GetParentNodeID() : "null")
                << "]"
                << std::endl;
      }
    }
}

//---------------------------------------------------------------------------
// The test makes sure the model hierarchy nodes correctly support node ID
// conflict in the parent node id. There are 2 steps in this test
// a) populates a scene with a 5 deep model hierarchy
// b) and imports a similar scene into the existing scene.
int ImportIDModelHierarchyParentIDConflictTestXMLString()
{
  vtkNew<vtkDMMLScene> scene;

  if (PopulateScene(scene.GetPointer())==EXIT_FAILURE)
    {
    return EXIT_FAILURE;
    }

  // At this point the scene should be:
  //
  //  Scene
  //    |---- vtkDMMLModelHierarchyNode1 (name: 0)
  //    |          |-- ref [parentNodeRef] to <null>
  //    |
  //    |---- vtkDMMLModelHierarchyNode2 (name: 1)
  //    |          |-- ref [parentNodeRef] to vtkDMMLModelHierarchyNode1
  //    |
  //    |---- vtkDMMLModelHierarchyNode3 (name: 2)
  //    |          |-- ref [parentNodeRef] to vtkDMMLModelHierarchyNode2
  //    |
  //    |---- vtkDMMLModelHierarchyNode4 (name: 3)
  //    |          |-- ref [parentNodeRef] to vtkDMMLModelHierarchyNode3
  //    |
  //    |---- vtkDMMLModelHierarchyNode5 (name: 4)
  //    |          |-- ref [parentNodeRef] to vtkDMMLModelHierarchyNode4

  //
  // Import
  //

  // Here is the scene that will be imported:

  const char scene1XML[] =
    "<DMML >"
    "  <ModelHierarchy id=\"vtkDMMLModelHierarchyNode1\" name=\"vtkDMMLModelHierarchyNode1\"></ModelHierarchy>"
    "  <ModelHierarchy id=\"vtkDMMLModelHierarchyNode2\" name=\"vtkDMMLModelHierarchyNode2\" parentNodeRef=\"vtkDMMLModelHierarchyNode1\"></ModelHierarchy>"
    "  <ModelHierarchy id=\"vtkDMMLModelHierarchyNode3\" name=\"vtkDMMLModelHierarchyNode2\" parentNodeRef=\"vtkDMMLModelHierarchyNode2\"></ModelHierarchy>"
    "  <ModelHierarchy id=\"vtkDMMLModelHierarchyNode4\" name=\"vtkDMMLModelHierarchyNode2\" parentNodeRef=\"vtkDMMLModelHierarchyNode3\"></ModelHierarchy>"
    "  <ModelHierarchy id=\"vtkDMMLModelHierarchyNode5\" name=\"vtkDMMLModelHierarchyNode2\" parentNodeRef=\"vtkDMMLModelHierarchyNode4\"></ModelHierarchy>"
    "</DMML>"
    ;

  scene->SetSceneXMLString(scene1XML);
  scene->SetLoadFromXMLString(1);

  // When importing the scene, there is conflict between the existing nodes
  // and added nodes. New IDs are set by Import to the added nodes and the
  // parent node refs should be updated

  scene->Import();  // adds Subject Hierarchy Node


  // At this point the scene should be:
  //
  //  Scene
  //    |---- vtkDMMLSubjectHierarchyNode1
  //    |---- vtkDMMLModelHierarchyNode1 (name: 0)
  //    |          |-- ref [parentNodeRef] to <null>
  //    |
  //    |---- vtkDMMLModelHierarchyNode2 (name: 1)
  //    |          |-- ref [parentNodeRef] to vtkDMMLModelHierarchyNode1
  //    |
  //    |---- vtkDMMLModelHierarchyNode3 (name: 2)
  //    |          |-- ref [parentNodeRef] to vtkDMMLModelHierarchyNode2
  //    |
  //    |---- vtkDMMLModelHierarchyNode4 (name: 3)
  //    |          |-- ref [parentNodeRef] to vtkDMMLModelHierarchyNode3
  //    |
  //    |---- vtkDMMLModelHierarchyNode5 (name: 4)
  //    |          |-- ref [parentNodeRef] to vtkDMMLModelHierarchyNode4
  //    |
  //    |---- vtkDMMLModelHierarchyNode6 (name: vtkDMMLModelHierarchyNode1)  [was vtkDMMLModelHierarchyNode1]
  //    |          |-- ref [parentNodeRef] to <null>
  //    |
  //    |---- vtkDMMLModelHierarchyNode7 (name: vtkDMMLModelHierarchyNode2)  [was vtkDMMLModelHierarchyNode1]
  //    |          |-- ref [parentNodeRef] to vtkDMMLModelHierarchyNode6
  //    |
  //    |---- vtkDMMLModelHierarchyNode8 (name: vtkDMMLModelHierarchyNode2)  [was vtkDMMLModelHierarchyNode3]
  //    |          |-- ref [parentNodeRef] to vtkDMMLModelHierarchyNode7
  //    |
  //    |---- vtkDMMLModelHierarchyNode9 (name: vtkDMMLModelHierarchyNode2)  [was vtkDMMLModelHierarchyNode4]
  //    |          |-- ref [parentNodeRef] to vtkDMMLModelHierarchyNode8
  //    |
  //    |---- vtkDMMLModelHierarchyNode10 (name: vtkDMMLModelHierarchyNode2) [was vtkDMMLModelHierarchyNode5]
  //    |          |-- ref [parentNodeRef] to vtkDMMLModelHierarchyNode9


  CHECK_INT_ADD_REPORT(scene->GetNumberOfNodes(), 11, PrintModelHierarchyNodes(__LINE__, scene.GetPointer()));
  CHECK_INT_ADD_REPORT(scene->GetNumberOfNodesByClass("vtkDMMLModelHierarchyNode"), 10, PrintModelHierarchyNodes(__LINE__, scene.GetPointer()));

  for (int index = 0; index < 10; ++index)
    {
    std::string nodeID = std::string("vtkDMMLModelHierarchyNode") + ToString(index + 1);
    vtkDMMLModelHierarchyNode* hierarchyNode =
        vtkDMMLModelHierarchyNode::SafeDownCast(scene->GetNodeByID(nodeID.c_str()));

    std::string expectedParentID = std::string("vtkDMMLModelHierarchyNode") + ToString(index);
    std::cout << "expectedParentID = " << expectedParentID << std::endl;

    CHECK_NOT_NULL_ADD_REPORT(hierarchyNode, PrintModelHierarchyNodes(__LINE__, scene.GetPointer()));
    CHECK_STRING_ADD_REPORT(hierarchyNode->GetParentNodeID(), (index == 0 || index == 5) ? nullptr : expectedParentID.c_str(), PrintModelHierarchyNodes(__LINE__, scene.GetPointer()));
    }

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
// The test makes sure the model hierarchy nodes correctly support node ID
// conflict in the parent node id.
// - populate a scene with one hierarchy node, save to disk
// - populates a scene with a 5 deep model hierarchy, save to disk
// - create new scene
// - import file with one hierarchy node
// - import file with 5 hierarchy nodes
int ImportIDModelHierarchyParentIDConflictTestFile()
{
  vtkNew<vtkDMMLScene> scene1;

  // add a single hierarchy node
  vtkNew<vtkDMMLModelHierarchyNode> mhn;
  scene1->AddNode(mhn.GetPointer());

  // At this point the scene1 should be:
  //
  //  Scene
  //    |---- vtkDMMLModelHierarchyNode1 (name: 0)
  //    |          |-- ref [parentNodeRef] to <null>

  CHECK_INT(scene1->GetNumberOfNodes(), 1);

  //
  // Save scene1
  //

  std::string filename1 = "ImportIDModelHierarchyParentIDConflictTestFile1.dmml";
  scene1->SetURL(filename1.c_str());
  scene1->Commit();

  // make a second scene file on disk with 5 nodes, the first one conflicting with the one node in the first scene
  vtkNew<vtkDMMLScene> scene2;

  if (PopulateScene(scene2.GetPointer())==EXIT_FAILURE)
    {
    return EXIT_FAILURE;
    }

  //
  // Save scene2
  //

  std::string filename2 = "ImportIDModelHierarchyParentIDConflictTestFile2.dmml";
  scene2->SetURL(filename2.c_str());
  scene2->Commit();

  //
  // Load scene1 and scene2 into the same scene
  //

  // now read into a new scene
  vtkNew<vtkDMMLScene> scene3;

  scene3->SetURL(filename1.c_str());
  scene3->Import();  // adds Subject Hierarchy Node

  // At this point the scene3 should be:
  //
  //  Scene
  //    |---- vtkDMMLSubjectHierarchyNode1
  //    |---- vtkDMMLModelHierarchyNode1 (name: 0)
  //    |          |-- ref [parentNodeRef] to <null>

  CHECK_INT(scene3->GetNumberOfNodes(), 2);

  std::cerr << "After first import, new scene has " << scene3->GetNumberOfNodes() << " nodes" << std::endl;
  PrintModelHierarchyNodes(__LINE__, scene3.GetPointer());

  // now import file 2
  scene3->SetURL(filename2.c_str());
  scene3->Import();

  // At this point the scene3 should be:
  //
  //  Scene
  //    |---- vtkDMMLSubjectHierarchyNode1
  //    |---- vtkDMMLModelHierarchyNode1 (name: 0)
  //    |          |-- ref [parentNodeRef] to <null>
  //    |
  //    |---- vtkDMMLModelHierarchyNode2 (name: 0)                        [was vtkDMMLModelHierarchyNode1]
  //    |          |-- ref [parentNodeRef] to <null>
  //    |
  //    |---- vtkDMMLModelHierarchyNode3 (name: 1)                        [was vtkDMMLModelHierarchyNode2]
  //    |          |-- ref [parentNodeRef] to vtkDMMLModelHierarchyNode2
  //    |
  //    |---- vtkDMMLModelHierarchyNode4 (name: 2)                        [was vtkDMMLModelHierarchyNode3]
  //    |          |-- ref [parentNodeRef] to vtkDMMLModelHierarchyNode3
  //    |
  //    |---- vtkDMMLModelHierarchyNode5 (name: 3)                        [was vtkDMMLModelHierarchyNode4]
  //    |          |-- ref [parentNodeRef] to vtkDMMLModelHierarchyNode4
  //    |
  //    |---- vtkDMMLModelHierarchyNode6 (name: 4)                        [was vtkDMMLModelHierarchyNode5]
  //    |          |-- ref [parentNodeRef] to vtkDMMLModelHierarchyNode5

  // but now expect the scene to have
  // vtkDMMLModelHierarchyNode1 has parent node id of null
  // vtkDMMLModelHierarchyNode6 has parent node id of null
  // vtkDMMLModelHierarchyNode2 has parent node id of vtkDMMLModelHierarchyNode6
  // vtkDMMLModelHierarchyNode3 has parent node id of vtkDMMLModelHierarchyNode2
  // vtkDMMLModelHierarchyNode4 has parent node id of vtkDMMLModelHierarchyNode3
  // vtkDMMLModelHierarchyNode5 has parent node id of vtkDMMLModelHierarchyNode4

  CHECK_INT(scene3->GetNumberOfNodes(), 7);

  vtkDMMLModelHierarchyNode *hierarchyNode2 =
      vtkDMMLModelHierarchyNode::SafeDownCast(scene3->GetNodeByID("vtkDMMLModelHierarchyNode2"));

  CHECK_NOT_NULL_ADD_REPORT(hierarchyNode2, PrintModelHierarchyNodes(__LINE__, scene3.GetPointer()));
  CHECK_STRING_ADD_REPORT(hierarchyNode2->GetParentNodeID(), "vtkDMMLModelHierarchyNode6", PrintModelHierarchyNodes(__LINE__, scene3.GetPointer()));

  // clean up
  int removed1 = static_cast<bool>(itksys::SystemTools::RemoveFile(filename1.c_str()));
  if (!removed1)
    {
    std::cerr << "Unable to remove file " << filename1.c_str() << std::endl;
    return EXIT_FAILURE;
    }
  int removed2 = static_cast<bool>(itksys::SystemTools::RemoveFile(filename2.c_str()));
  if (!removed2)
    {
    std::cerr << "Unable to remove file " << filename2.c_str() << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
