/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

#include "vtkDMMLScene.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkObjectFactory.h>

#include "vtkDMMLCoreTestingMacros.h"

//------------------------------------------------------------------------------
class vtkDMMLCustomNode
  : public vtkDMMLNode
{
public:
  static vtkDMMLCustomNode *New();
  vtkTypeMacro(vtkDMMLCustomNode, vtkDMMLNode);

  vtkDMMLNode* CreateNodeInstance() override;
  const char* GetNodeTagName() override { return "Custom"; }

  void Reset(vtkDMMLNode* defaultNode) override
    {
    ++this->ResetCount;
    this->vtkDMMLNode::Reset(defaultNode);
    }

  int ResetCount{0};

protected:
  vtkDMMLCustomNode() = default;
  ~vtkDMMLCustomNode() override = default;
  vtkDMMLCustomNode(const vtkDMMLCustomNode&);
  void operator=(const vtkDMMLCustomNode&);
};

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLCustomNode);

//------------------------------------------------------------------------------
class vtkDMMLAnotherCustomNode
  : public vtkDMMLNode
{
public:
  static vtkDMMLAnotherCustomNode *New();
  vtkTypeMacro(vtkDMMLAnotherCustomNode, vtkDMMLNode);

  vtkDMMLNode* CreateNodeInstance() override;
  const char* GetNodeTagName() override { return "AnotherCustom"; }

protected:
  vtkDMMLAnotherCustomNode() = default;
  ~vtkDMMLAnotherCustomNode() override = default;
  vtkDMMLAnotherCustomNode(const vtkDMMLAnotherCustomNode&);
  void operator=(const vtkDMMLAnotherCustomNode&);
};

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLAnotherCustomNode);

//------------------------------------------------------------------------------
int vtkDMMLSceneTest1(int , char * [] )
{
  vtkNew<vtkDMMLScene> scene1;

  EXERCISE_BASIC_OBJECT_METHODS(scene1.GetPointer());

  std::cout << "GetNumberOfRegisteredNodeClasses() = ";
  std::cout << scene1->GetNumberOfRegisteredNodeClasses() << std::endl;

  TEST_SET_GET_STRING(scene1.GetPointer(), URL);
  TEST_SET_GET_STRING(scene1.GetPointer(), RootDirectory);

  // Test scene version parsing
  std::string applicationName;
  int major = -1;
  int minor = -1;
  int patch = -1;
  int revision = -1;
  // Old-style
  CHECK_BOOL(vtkDMMLScene::ParseVersion("Cjyx4.5.6", applicationName, major, minor, patch, revision), true);
  CHECK_STD_STRING(applicationName, "Cjyx");
  CHECK_INT(major, 4);
  CHECK_INT(minor, 5);
  CHECK_INT(patch, 6);
  // Invalid
  CHECK_BOOL(vtkDMMLScene::ParseVersion("Cjyx4.5.", applicationName, major, minor, patch, revision), false);
  CHECK_BOOL(vtkDMMLScene::ParseVersion("Some4.5.", applicationName, major, minor, patch, revision), false);
  // New-style
  CHECK_BOOL(vtkDMMLScene::ParseVersion("SomeApp 71.82.93 12345", applicationName, major, minor, patch, revision), true);
  CHECK_STD_STRING(applicationName, "SomeApp");
  CHECK_INT(major, 71);
  CHECK_INT(minor, 82);
  CHECK_INT(patch, 93);
  CHECK_INT(revision, 12345);

  //---------------------------------------------------------------------------
  // Test IsNodeClassRegistered
  //---------------------------------------------------------------------------

  {
  CHECK_BOOL(scene1->IsNodeClassRegistered(""), false);
  CHECK_BOOL(scene1->IsNodeClassRegistered("vtkDMMLScalarVolumeNode"), true);
  CHECK_BOOL(scene1->IsNodeClassRegistered("vtkDMMLInvalidNode"), false);
  }

  //---------------------------------------------------------------------------
  // Test ResetNodes
  //---------------------------------------------------------------------------

  {
    vtkNew<vtkDMMLCustomNode> node1;
    CHECK_INT(node1->ResetCount, 0);

    scene1->AddNode(node1.GetPointer());
    CHECK_INT(node1->ResetCount, 0);

    scene1->ResetNodes();

    CHECK_INT(node1->ResetCount, 1);

    scene1->Clear(/* removeSingletons= */ 1);
  }

  //---------------------------------------------------------------------------
  // Test GetFirstNode
  //---------------------------------------------------------------------------

  vtkDMMLNode* node1 =
    scene1->AddNode(vtkSmartPointer<vtkDMMLCustomNode>::New());
  node1->SetName("Node");
  node1->SetHideFromEditors(0);

  vtkDMMLNode* node2 =
    scene1->AddNode(vtkSmartPointer<vtkDMMLAnotherCustomNode>::New());
  node2->SetName("NodeWithSuffix");
  node2->SetHideFromEditors(0);

  vtkDMMLNode* node3 =
    scene1->AddNode(vtkSmartPointer<vtkDMMLAnotherCustomNode>::New());
  node3->SetName("Node");
  node3->SetHideFromEditors(1);

  vtkDMMLNode* node4 =
      scene1->AddNode(vtkSmartPointer<vtkDMMLCustomNode>::New());
  node4->SetName("NodeWithSuffix");
  node4->SetHideFromEditors(1);

  // Check if transform nodes have been added
  vtkSmartPointer<vtkCollection> transformNodes;
  transformNodes.TakeReference(scene1->GetNodesByClass("vtkDMMLCustomNode"));
  {
    int expectedTotalNodeCount = 2;
    int currentTotalNodeCount = transformNodes->GetNumberOfItems();
    if (currentTotalNodeCount != expectedTotalNodeCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentTotalNodeCount: " << currentTotalNodeCount << "\n"
                << "  expectedTotalNodeCount: " << expectedTotalNodeCount
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    vtkDMMLNode* expectedNode = node1;
    vtkDMMLNode* currentNode = vtkDMMLNode::SafeDownCast(transformNodes->GetItemAsObject(0));
    if (currentNode != expectedNode)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentNode: " << currentNode << "\n"
                << "  expectedNode: " << expectedNode
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    vtkDMMLNode* expectedNode = node4;
    vtkDMMLNode* currentNode = vtkDMMLNode::SafeDownCast(transformNodes->GetItemAsObject(1));
    if (currentNode != expectedNode)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentNode: " << currentNode << "\n"
                << "  expectedNode: " << expectedNode
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Check if selection nodes have been added in the expected order
  vtkSmartPointer<vtkCollection> selectionNodes;
  selectionNodes.TakeReference(scene1->GetNodesByClass("vtkDMMLAnotherCustomNode"));
  {
    int expectedTotalNodeCount = 2;
    int currentTotalNodeCount = selectionNodes->GetNumberOfItems();
    if (currentTotalNodeCount != expectedTotalNodeCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentTotalNodeCount: " << currentTotalNodeCount << "\n"
                << "  expectedTotalNodeCount: " << expectedTotalNodeCount
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    vtkDMMLNode* expectedNode = node2;
    vtkDMMLNode* currentNode = vtkDMMLNode::SafeDownCast(selectionNodes->GetItemAsObject(0));
    if (currentNode != expectedNode)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentNode: " << currentNode << "\n"
                << "  expectedNode: " << expectedNode
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    vtkDMMLNode* expectedNode = node3;
    vtkDMMLNode* currentNode = vtkDMMLNode::SafeDownCast(selectionNodes->GetItemAsObject(1));
    if (currentNode != expectedNode)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentNode: " << currentNode << "\n"
                << "  expectedNode: " << expectedNode
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Check that byClass works as expected
  {
    vtkDMMLNode* expectedFirstNodeByClass = node1;
    vtkDMMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ nullptr, /* byClass= */ "vtkDMMLCustomNode");
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }
  {
    vtkDMMLNode* expectedFirstNodeByClass = node2;
    vtkDMMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ nullptr, /* byClass= */ "vtkDMMLAnotherCustomNode");
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Check that byName works as expected
  {
    vtkDMMLNode* expectedFirstNodeByClass = node1;
    vtkDMMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ "Node");
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Check that byName + [exactNameMatch=false] works as expected
  {
    vtkDMMLNode* expectedFirstNodeByClass = node2;
    vtkDMMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ "Node.+",
          /* byClass= */ nullptr,
          /* byHideFromEditors= */ nullptr,
          /* exactNameMatch= */ false);
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Check that byHideFromEditors works as expected
  {
    int hideFromEditors = 1;
    vtkDMMLNode* expectedFirstNodeByClass = node3;
    vtkDMMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ nullptr, /* byClass= */ nullptr, /* byHideFromEditors= */ &hideFromEditors);
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Check that byClass + byName works as expected
  {
    vtkDMMLNode* expectedFirstNodeByClass = node3;
    vtkDMMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ "Node", /* byClass= */ "vtkDMMLAnotherCustomNode");
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }
  {
    vtkDMMLNode* expectedFirstNodeByClass = node1;
    vtkDMMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ "Node", /* byClass= */ "vtkDMMLCustomNode");
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Check that byClass + byName + [exactNameMatch=false] works as expected
  {
    vtkDMMLNode* expectedFirstNodeByClass = node4;
    vtkDMMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ "Node.+",
          /* byClass= */ "vtkDMMLCustomNode",
          /* byHideFromEditors= */ nullptr,
          /* exactNameMatch= */ false);
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Check that byClass + byName + byHideFromEditors works as expected
  {
    int hideFromEditors = 1;
    vtkDMMLNode* expectedFirstNodeByClass = node4;
    vtkDMMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ "NodeWithSuffix",
          /* byClass= */ "vtkDMMLCustomNode",
          /* byHideFromEditors= */ &hideFromEditors);
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }
  {
    int hideFromEditors = 1;
    vtkDMMLNode* expectedFirstNodeByClass = node3;
    vtkDMMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ "Node",
          /* byClass= */ "vtkDMMLAnotherCustomNode",
          /* byHideFromEditors= */ &hideFromEditors);
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Verify content of ReferencedIDChanges map
  {
    // Make sure IDs of nodes coming from private scenes are not stored in
    // the referenced ID changes map. It caused problems with AddArchetypeVoume,
    // because the volume (and related) nodes are tested in private scenes, they
    // have IDs when adding to the main scene, and the last node ID was stored
    // incorrectly as a changed ID
    vtkNew<vtkDMMLScene> privateScene;
    vtkDMMLCustomNode* nodeFromPrivateScene = vtkDMMLCustomNode::New();
    privateScene->AddNode(nodeFromPrivateScene);
    nodeFromPrivateScene->SetName("NodeFromPrivateScene");
    // Copy to std::string because pointer becomes invalid when adding to other scene
    std::string nodeInPrivateSceneID(nodeFromPrivateScene->GetID());
    privateScene->Clear(0);

    scene1->AddNode(nodeFromPrivateScene);
    const char* nodeAddedFromPrivateSceneID = nodeFromPrivateScene->GetID();
    const char* changedIDFromPrivateScene =
      scene1->GetChangedID(nodeInPrivateSceneID.c_str());
    if ( changedIDFromPrivateScene
      || nodeInPrivateSceneID.empty() || !nodeAddedFromPrivateSceneID ||
      !nodeInPrivateSceneID.compare(nodeAddedFromPrivateSceneID) )
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetChangedID()\n"
                << "  nodeFromPrivateSceneID: " << nodeInPrivateSceneID << "\n"
                << "  changedIDFromPrivateScene: " <<
                (changedIDFromPrivateScene?changedIDFromPrivateScene:"NULL") << "\n"
                << "  nodeAddedFromPrivateSceneID: " <<
                (nodeAddedFromPrivateSceneID?nodeAddedFromPrivateSceneID:"NULL")
                << std::endl;
      return EXIT_FAILURE;
      }

    // Check that IDs from imported scenes are indeed stored as changed if in
    // conflict with the main scene
    vtkNew<vtkDMMLScene> importedScene;
    vtkDMMLNode* importedNode =
      importedScene->AddNode(vtkSmartPointer<vtkDMMLCustomNode>::New());
    importedNode->SetName("ImportedNode");
    importedScene->SetSaveToXMLString(1);
    importedScene->Commit();
    std::string sceneXMLString = importedScene->GetSceneXMLString();
    const char* importedNodeID = importedNode->GetID();

    scene1->RegisterNodeClass(vtkSmartPointer<vtkDMMLCustomNode>::New());
    scene1->SetLoadFromXMLString(1);
    scene1->SetSceneXMLString(sceneXMLString);
    scene1->Import();
    const char* changedIDFromImportedScene =
      scene1->GetChangedID(importedNodeID);
    if ( !importedNodeID || !changedIDFromImportedScene ||
      !strcmp(changedIDFromImportedScene, importedNodeID) )
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetChangedID()\n"
                << "  importedNodeID: " <<
                (importedNodeID?importedNodeID:"NULL") << "\n"
                << "  changedIDFromImportedScene: " <<
                (changedIDFromImportedScene?changedIDFromImportedScene:"NULL")
                << std::endl;
      return EXIT_FAILURE;
      }

    // Needed to make sure the node is present after clearing the private scene
    nodeFromPrivateScene->Delete();
  }

  return EXIT_SUCCESS;
}
