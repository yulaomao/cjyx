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
#include "vtkDMMLModelNode.h"
#include "vtkDMMLParser.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSceneViewNode.h"

// STD includes
#include <vtkNew.h>

//---------------------------------------------------------------------------
int vtkDMMLSceneAddSingletonTest(int vtkNotUsed(argc), char * vtkNotUsed(argv) [])
{
  vtkNew<vtkDMMLScene> scene;

  // Add singleton node
  vtkNew<vtkDMMLModelNode> singleton1;
  singleton1->SetSingletonTag("Singleton");
  vtkDMMLNode* addedNode = scene->AddNode(singleton1.GetPointer());

  CHECK_POINTER(singleton1.GetPointer(), addedNode);
  CHECK_POINTER(singleton1.GetPointer(), scene->GetNodeByID("vtkDMMLModelNodeSingleton"));
  CHECK_POINTER(singleton1->GetScene(), scene.GetPointer());
  CHECK_STRING(singleton1->GetID(), "vtkDMMLModelNodeSingleton");
  CHECK_STRING(singleton1->GetName(), "Model");
  CHECK_INT(scene->GetNumberOfNodes(), 1);

  // AddNode of a new singleton shouldn't add an instance but copy the node
  // in the existing singleton
  vtkNew<vtkDMMLModelNode> singleton1Copy;
  singleton1Copy->SetSingletonTag("Singleton");
  addedNode = scene->AddNode(singleton1Copy.GetPointer());

  CHECK_NULL(singleton1Copy->GetScene());
  CHECK_NULL(singleton1Copy->GetID());
  CHECK_NULL(singleton1Copy->GetName());
  CHECK_POINTER(singleton1.GetPointer(), addedNode);
  CHECK_POINTER(singleton1.GetPointer(), scene->GetNodeByID("vtkDMMLModelNodeSingleton"));
  CHECK_STRING(singleton1->GetID(), "vtkDMMLModelNodeSingleton");
  CHECK_STRING(singleton1->GetName(), "Model");
  CHECK_INT(scene->GetNumberOfNodes(), 1);

  // Add a different singleton.
  vtkNew<vtkDMMLModelNode> singleton2;
  singleton2->SetSingletonTag("Singleton2");
  addedNode = scene->AddNode(singleton2.GetPointer());

  CHECK_POINTER(singleton2.GetPointer(), addedNode);
  CHECK_POINTER(singleton2.GetPointer(), scene->GetNodeByID("vtkDMMLModelNodeSingleton2"));
  CHECK_POINTER(singleton2->GetScene(), scene.GetPointer());
  CHECK_STRING(singleton2->GetID(), "vtkDMMLModelNodeSingleton2");
  CHECK_STRING(singleton2->GetName(), "Model_1");
  CHECK_INT(scene->GetNumberOfNodes(), 2);

  const char sceneXML[] =
    "<DMML  version=\"18916\" userTags=\"\">"
    "<SceneView id=\"vtkDMMLSceneSnapshotNode1\" name=\"sceneView\">"
    "  <Model id=\"vtkDMMLModelNodeSingleton\" name=\"Restored Model\" ></Model>"
    "  <Model id=\"vtkDMMLModelNode1\"      name=\"Old Model\" ></Model>"
    "  <Model id=\"vtkDMMLModelNodeSingleton1\" name=\"Restored Model2\" ></Model>"
    "</SceneView>"
    "</DMML>"
    ;

  vtkNew<vtkDMMLScene> tempScene;
  vtkNew<vtkDMMLSceneViewNode> registerNode;
  tempScene->RegisterNodeClass(registerNode.GetPointer());

  vtkNew<vtkDMMLParser> parser;
  parser->SetDMMLScene(tempScene.GetPointer());
  parser->Parse(sceneXML);

  vtkDMMLSceneViewNode* sceneViewNode = vtkDMMLSceneViewNode::SafeDownCast(
    tempScene->GetFirstNodeByName("sceneView"));

  // Test singleton loading/restoring.
  vtkDMMLNode* restoredSingleton1 =
    sceneViewNode->GetStoredScene()->GetNthNodeByClass(0, "vtkDMMLModelNode");
  restoredSingleton1->SetSingletonTag("Singleton");
  restoredSingleton1->SetAddToScene(1);
  addedNode = scene->AddNode(restoredSingleton1);

  CHECK_STRING(restoredSingleton1->GetID(), "vtkDMMLModelNodeSingleton");
  CHECK_POINTER(singleton1.GetPointer(), addedNode);
  CHECK_POINTER(singleton1.GetPointer(), scene->GetNodeByID("vtkDMMLModelNodeSingleton"));
  CHECK_STRING(singleton1->GetName(), "Restored Model");
  CHECK_INT(scene->GetNumberOfNodes(), 2);

  // Test compatibility with Cjyx 3 scenes.
  std::string singleton1ID = singleton1->GetID();
  restoredSingleton1 =
    sceneViewNode->GetStoredScene()->GetNthNodeByClass(1, "vtkDMMLModelNode");
  restoredSingleton1->SetSingletonTag("Singleton");
  restoredSingleton1->SetAddToScene(1);
  addedNode = scene->AddNode(restoredSingleton1);

  CHECK_POINTER_DIFFERENT(restoredSingleton1, addedNode);
  CHECK_STRING(restoredSingleton1->GetID(), "vtkDMMLModelNode1");
  // The node ID of singleton1 is kept (the node ID of restoredSingleton1 is changed and
  // nodes that are imported along with this singleton are notified about the ID change)
  CHECK_POINTER(singleton1.GetPointer(), scene->GetNodeByID("vtkDMMLModelNodeSingleton"));
  CHECK_STRING(singleton1->GetName(), "Old Model");
  CHECK_INT(scene->GetNumberOfNodes(), 2);

  // Test odd node ID. There is no reason why it could happen, but there is no
  // reason why it shouldn't be supported.
  restoredSingleton1 =
    sceneViewNode->GetStoredScene()->GetNthNodeByClass(2, "vtkDMMLModelNode");
  restoredSingleton1->SetSingletonTag("Singleton");
  restoredSingleton1->SetAddToScene(1);
  addedNode = scene->AddNode(restoredSingleton1);

  CHECK_POINTER_DIFFERENT(restoredSingleton1, addedNode);
  CHECK_STRING(restoredSingleton1->GetID(), "vtkDMMLModelNodeSingleton1");
  CHECK_POINTER(singleton1.GetPointer(), addedNode);
  // The node ID of singleton1 is kept (the node ID of restoredSingleton1 is changed and
  // nodes that are imported along with this singleton are notified about the ID change)
  CHECK_POINTER(singleton1.GetPointer(), scene->GetNodeByID("vtkDMMLModelNodeSingleton"));
  CHECK_STRING(singleton1->GetName(), "Restored Model2");
  CHECK_INT(scene->GetNumberOfNodes(), 2);

  ////////////////////////////
  // Check node references of imported singleton and regular nodes

  scene->Clear(true);

  // SingletonNodeA: singleton node referencing another singleton node and a regular node
  // vtkDMMLScriptedModuleNode1: regular node referencing a singleton node and another regular node
  const char scene1XML[] =
    "<DMML>"
    "<ScriptedModule id=\"vtkDMMLScriptedModuleNodeSingletonA\" name=\"Scene1SingletonNodeA\" singletonTag=\"SingletonA\""
      " references=\"ReferenceB:vtkDMMLScriptedModuleNodeSingletonB;Reference1:vtkDMMLScriptedModuleNode1;\" > </ScriptedModule>"
    "<ScriptedModule id=\"vtkDMMLScriptedModuleNodeSingletonB\" name=\"Scene1SingletonNodeB\" singletonTag=\"SingletonB\""
      " references=\"Reference1:vtkDMMLScriptedModuleNode2;\" > </ScriptedModule>"
    "<ScriptedModule id=\"vtkDMMLScriptedModuleNode1\" name=\"Scene1RegularNode1\""
      " references=\"Reference2:vtkDMMLScriptedModuleNode2;ReferenceA:vtkDMMLScriptedModuleNodeSingletonA;ReferenceB:vtkDMMLScriptedModuleNodeSingletonB;\" > </ScriptedModule>"
    "<ScriptedModule id=\"vtkDMMLScriptedModuleNode2\" name=\"Scene1RegularNode2\" > </ScriptedModule>"
    "</DMML>";

  scene->SetLoadFromXMLString(1);
  scene->SetSceneXMLString(scene1XML);
  scene->Import();  // adds Subject Hierarchy Node

  CHECK_INT(scene->GetNumberOfNodes(), 5);

  vtkDMMLNode* scene1SingletonA = scene->GetNodeByID("vtkDMMLScriptedModuleNodeSingletonA");
  vtkDMMLNode* scene1SingletonB = scene->GetNodeByID("vtkDMMLScriptedModuleNodeSingletonB");
  vtkDMMLNode* scene1Regular1 = scene->GetNodeByID("vtkDMMLScriptedModuleNode1");
  vtkDMMLNode* scene1Regular2 = scene->GetNodeByID("vtkDMMLScriptedModuleNode2");

  // Check node contents
  CHECK_NOT_NULL(scene1SingletonA);
  CHECK_NOT_NULL(scene1SingletonB);
  CHECK_NOT_NULL(scene1Regular1);
  CHECK_NOT_NULL(scene1Regular2);
  CHECK_STRING(scene1SingletonA->GetName(), "Scene1SingletonNodeA");
  CHECK_STRING(scene1SingletonB->GetName(), "Scene1SingletonNodeB");
  CHECK_STRING(scene1Regular1->GetName(), "Scene1RegularNode1");
  CHECK_STRING(scene1Regular2->GetName(), "Scene1RegularNode2");

  // Check node references
  CHECK_POINTER(scene1SingletonA->GetNodeReference("ReferenceB"), scene1SingletonB);
  CHECK_POINTER(scene1SingletonA->GetNodeReference("Reference1"), scene1Regular1);
  CHECK_POINTER(scene1Regular1->GetNodeReference("ReferenceA"), scene1SingletonA);
  CHECK_POINTER(scene1Regular1->GetNodeReference("ReferenceB"), scene1SingletonB);
  CHECK_POINTER(scene1Regular1->GetNodeReference("Reference2"), scene1Regular2);

  // SingletonNodeA: singleton node referencing another singleton node and a regular node
  // SingletonNodeB: singleton node, ID clash with a node in scene 1
  // vtkDMMLScriptedModuleNode1: regular node referencing a singleton node and another regular node; ID clash with a node in scene 1
  const char scene2XML[] =
    "<DMML>"
    "<ScriptedModule id=\"vtkDMMLScriptedModuleNodeSingletonA\" name=\"Scene2SingletonNodeA\" singletonTag=\"SingletonA\""
      " references=\"ReferenceB:vtkDMMLScriptedModuleNodeSingletonB;Reference1:vtkDMMLScriptedModuleNode1;\" > </ScriptedModule>"
    "<ScriptedModule id=\"vtkDMMLScriptedModuleNodeSingletonXB\" name=\"Scene2SingletonNodeB\" singletonTag=\"SingletonB\""
      " references=\"Reference1:vtkDMMLScriptedModuleNodeX2;\" > </ScriptedModule>"
    "<ScriptedModule id=\"vtkDMMLScriptedModuleNode1\" name=\"Scene2RegularNode1\""
      " references=\"Reference2:vtkDMMLScriptedModuleNodeX2;ReferenceA:vtkDMMLScriptedModuleNodeSingletonA;ReferenceB:vtkDMMLScriptedModuleNodeSingletonXB;\" > </ScriptedModule>"
    "<ScriptedModule id=\"vtkDMMLScriptedModuleNodeX2\" name=\"Scene2RegularNode2\" > </ScriptedModule>"
    "</DMML>";

  scene->SetLoadFromXMLString(1);
  scene->SetSceneXMLString(scene2XML);
  scene->Import();  // adds Subject Hierarchy Node

  CHECK_INT(scene->GetNumberOfNodes(), 7);

  vtkDMMLNode* scene2SingletonA = scene->GetNodeByID("vtkDMMLScriptedModuleNodeSingletonA");
  vtkDMMLNode* scene2SingletonB = scene->GetNodeByID("vtkDMMLScriptedModuleNodeSingletonB"); // ID changed (singleton is matched based on singleton tag)
  vtkDMMLNode* scene2Regular1 = scene->GetNodeByID("vtkDMMLScriptedModuleNode3"); // ID changed (due to clash with a regular node in scene1)
  vtkDMMLNode* scene2Regular2 = scene->GetNodeByID("vtkDMMLScriptedModuleNodeX2");

  CHECK_NULL(scene->GetNodeByID("vtkDMMLScriptedModuleNodeSingletonXB"));
  CHECK_NULL(scene->GetNodeByID("vtkDMMLScriptedModuleNode4"));
  CHECK_NOT_NULL(scene2SingletonA);
  CHECK_NOT_NULL(scene2SingletonB);
  CHECK_NOT_NULL(scene2Regular1);
  CHECK_NOT_NULL(scene2Regular2);

  // Check contents of newly imported nodes
  CHECK_STRING(scene2SingletonA->GetName(), "Scene2SingletonNodeA");
  CHECK_STRING(scene2SingletonB->GetName(), "Scene2SingletonNodeB");
  CHECK_STRING(scene2Regular1->GetName(), "Scene2RegularNode1");
  CHECK_STRING(scene2Regular2->GetName(), "Scene2RegularNode2");

  // Check that singleton ID is kept but contents is overwritten
  CHECK_STRING(scene1SingletonA->GetID(), "vtkDMMLScriptedModuleNodeSingletonA"); // same
  CHECK_STRING(scene1SingletonA->GetName(), "Scene2SingletonNodeA"); // changed
  CHECK_STRING(scene1SingletonB->GetID(), "vtkDMMLScriptedModuleNodeSingletonB"); // same
  CHECK_STRING(scene1SingletonB->GetName(), "Scene2SingletonNodeB"); // changed
  // Check that singleton node pointers remained the same
  CHECK_POINTER(scene1SingletonA, scene2SingletonA);
  CHECK_POINTER(scene1SingletonB, scene2SingletonB);
  // Check that regular nodes have not changed
  CHECK_STRING(scene1Regular1->GetID(), "vtkDMMLScriptedModuleNode1"); // same
  CHECK_STRING(scene1Regular1->GetName(), "Scene1RegularNode1"); // same
  CHECK_STRING(scene1Regular2->GetID(), "vtkDMMLScriptedModuleNode2"); // same
  CHECK_STRING(scene1Regular2->GetName(), "Scene1RegularNode2"); // same

  // Check that node references in regular nodes did not change
  CHECK_POINTER(scene1Regular1->GetNodeReference("ReferenceB"), scene1SingletonB);
  CHECK_POINTER(scene1Regular1->GetNodeReference("Reference2"), scene1Regular2);

  // Check that new node references are correct (same test repeated as with scene1 before the second scene import)
  CHECK_POINTER(scene2SingletonA->GetNodeReference("ReferenceB"), scene2SingletonB);
  CHECK_POINTER(scene2SingletonA->GetNodeReference("Reference1"), scene2Regular1);
  CHECK_POINTER(scene2Regular1->GetNodeReference("ReferenceA"), scene2SingletonA);
  CHECK_POINTER(scene2Regular1->GetNodeReference("ReferenceB"), scene2SingletonB);
  CHECK_POINTER(scene2Regular1->GetNodeReference("Reference2"), scene2Regular2);

  return EXIT_SUCCESS;
}
