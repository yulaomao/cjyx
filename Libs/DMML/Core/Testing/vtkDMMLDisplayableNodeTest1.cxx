/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

//DMML includes
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLDisplayableNode.h"
#include "vtkDMMLDisplayNode.h"
#include "vtkDMMLModelStorageNode.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

//----------------------------------------------------------------------------
class vtkDMMLDisplayableNodeTestHelper1 : public vtkDMMLDisplayableNode
{
public:
  // Provide a concrete New.
  static vtkDMMLDisplayableNodeTestHelper1 *New();

  vtkTypeMacro(vtkDMMLDisplayableNodeTestHelper1, vtkDMMLDisplayableNode);

  // Return the raw list of display nodes. Elements can be 0 even if the node
  // associated to the node ID exists in the scene.
  const std::vector<vtkDMMLDisplayNode*> GetInternalDisplayNodes()
    {
    std::vector<vtkDMMLDisplayNode*> InternalDisplayNodes;
    int ndnodes = this->GetNumberOfDisplayNodes();
    for (int i=0; i<ndnodes; i++)
      {
      InternalDisplayNodes.push_back(this->GetNthDisplayNode(i));
      }
    return InternalDisplayNodes;
    }
  vtkDMMLNode* CreateNodeInstance() override
    {
    return vtkDMMLDisplayableNodeTestHelper1::New();
    }
  const char* GetNodeTagName() override
    {
    return "vtkDMMLDisplayableNodeTestHelper1";
    }

  vtkDMMLStorageNode* CreateDefaultStorageNode() override
    {
    // just some random storage node to pass the storage node test of basic DMML node tests
    return vtkDMMLStorageNode::SafeDownCast(vtkDMMLModelStorageNode::New());
    }
};
vtkStandardNewMacro(vtkDMMLDisplayableNodeTestHelper1);

//----------------------------------------------------------------------------
class vtkDMMLDisplayNodeTestHelper : public vtkDMMLDisplayNode
{
public:
  // Provide a concrete New.
  static vtkDMMLDisplayNodeTestHelper *New();

  vtkTypeMacro(vtkDMMLDisplayNodeTestHelper, vtkDMMLDisplayNode);

  vtkDMMLNode* CreateNodeInstance() override
    {
    return vtkDMMLDisplayNodeTestHelper::New();
    }
  const char* GetNodeTagName() override
    {
    return "vtkDMMLDisplayNodeTestHelper";
    }

};
vtkStandardNewMacro(vtkDMMLDisplayNodeTestHelper);

int TestAddDisplayNodeID();
int TestAddDisplayNodeIDWithNoScene();
int TestAddDisplayNodeIDEventsWithNoScene();
int TestAddDelayedDisplayNode();
int TestRemoveDisplayNodeID();
int TestRemoveDisplayNode();
int TestRemoveDisplayableNode();
int TestDisplayModifiedEvent();
int TestReferences();
int TestImportIntoSceneWithNodeIdConflict();

//----------------------------------------------------------------------------
int vtkDMMLDisplayableNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLDisplayableNodeTestHelper1> node1;
  EXERCISE_ALL_BASIC_DMML_METHODS(node1);

  CHECK_EXIT_SUCCESS(TestAddDisplayNodeID());
  CHECK_EXIT_SUCCESS(TestAddDisplayNodeIDWithNoScene());
  //CHECK_EXIT_SUCCESS(TestAddDisplayNodeIDEventsWithNoScene());
  CHECK_EXIT_SUCCESS(TestAddDelayedDisplayNode());
  CHECK_EXIT_SUCCESS(TestRemoveDisplayNodeID());
  CHECK_EXIT_SUCCESS(TestRemoveDisplayNode());
  CHECK_EXIT_SUCCESS(TestRemoveDisplayableNode());
  CHECK_EXIT_SUCCESS(TestDisplayModifiedEvent());
  CHECK_EXIT_SUCCESS(TestReferences());
  CHECK_EXIT_SUCCESS(TestImportIntoSceneWithNodeIdConflict());

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestAddDisplayNodeID()
{
  vtkNew<vtkDMMLScene> scene;

  vtkNew<vtkDMMLDisplayableNodeTestHelper1> displayableNode;
  scene->AddNode(displayableNode);

  vtkNew<vtkDMMLDisplayNodeTestHelper> displayNode1;
  scene->AddNode(displayNode1);

  /// Add empty display node
  displayableNode->AddAndObserveDisplayNodeID(nullptr);
  CHECK_INT(displayableNode->GetNumberOfDisplayNodes(), 0);
  CHECK_NULL(displayableNode->GetNthDisplayNodeID(0));
  CHECK_NULL(displayableNode->GetNthDisplayNode(0));

  vtkSmartPointer<vtkCollection> referencedNodes;
  referencedNodes.TakeReference(scene->GetReferencedNodes(displayableNode));
  int referencedNodesCount = referencedNodes->GetNumberOfItems();

  /// Add display node ID
  displayableNode->AddAndObserveDisplayNodeID(displayNode1->GetID());

  referencedNodes.TakeReference(scene->GetReferencedNodes(displayableNode));
  int newReferencedNodesCount = referencedNodes->GetNumberOfItems();

  CHECK_NOT_NULL(displayableNode->GetNthDisplayNodeID(0));
  CHECK_STRING(displayableNode->GetNthDisplayNodeID(0), displayNode1->GetID());
  CHECK_POINTER(displayableNode->GetNthDisplayNode(0), displayNode1);
  CHECK_INT(newReferencedNodesCount, referencedNodesCount + 1);

  /// Add empty display node ID
  displayableNode->AddAndObserveDisplayNodeID(nullptr);
  CHECK_INT(displayableNode->GetNumberOfDisplayNodes(), 1);
  CHECK_NULL(displayableNode->GetNthDisplayNodeID(1));
  CHECK_NULL(displayableNode->GetNthDisplayNode(1));

  /// Change display node
  vtkNew<vtkDMMLDisplayNodeTestHelper> displayNode2;
  scene->AddNode(displayNode2);

  displayableNode->SetAndObserveDisplayNodeID(displayNode2->GetID());

  CHECK_NOT_NULL(displayableNode->GetNthDisplayNodeID(0));
  CHECK_STRING(displayableNode->GetNthDisplayNodeID(0), displayNode2->GetID());
  CHECK_POINTER(displayableNode->GetNthDisplayNode(0), displayNode2);

  /// Add display node
  vtkNew<vtkDMMLDisplayNodeTestHelper> displayNode3;
  scene->AddNode(displayNode3);

  displayableNode->SetAndObserveNthDisplayNodeID(1, displayNode3->GetID());

  CHECK_NOT_NULL(displayableNode->GetNthDisplayNodeID(1));
  CHECK_STRING(displayableNode->GetNthDisplayNodeID(1), displayNode3->GetID());
  CHECK_POINTER(displayableNode->GetNthDisplayNode(1), displayNode3);
  // make sure it didn't change the first display node ID
  CHECK_NOT_NULL(displayableNode->GetNthDisplayNodeID(0));
  CHECK_STRING(displayableNode->GetNthDisplayNodeID(0), displayNode2->GetID());
  CHECK_POINTER(displayableNode->GetNthDisplayNode(0), displayNode2);

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestAddDisplayNodeIDWithNoScene()
{
  vtkNew<vtkDMMLScene> scene;

  vtkNew<vtkDMMLDisplayableNodeTestHelper1> displayableNode;

  vtkNew<vtkDMMLDisplayNodeTestHelper> displayNode1;
  scene->AddNode(displayNode1);

  /// Add display node
  displayableNode->SetAndObserveDisplayNodeID(displayNode1->GetID());

  CHECK_NOT_NULL(displayableNode->GetNthDisplayNodeID(0));
  CHECK_STRING(displayableNode->GetNthDisplayNodeID(0), displayNode1->GetID());
  CHECK_NULL(displayableNode->GetNthDisplayNode(0));

  /// Change display node
  vtkNew<vtkDMMLDisplayNodeTestHelper> displayNode2;
  scene->AddNode(displayNode2);

  displayableNode->SetAndObserveDisplayNodeID(displayNode2->GetID());

  CHECK_NOT_NULL(displayableNode->GetNthDisplayNodeID(0));
  CHECK_STRING(displayableNode->GetNthDisplayNodeID(0), displayNode2->GetID());
  CHECK_NULL(displayableNode->GetNthDisplayNode(0));

  /// Add display node
  vtkNew<vtkDMMLDisplayNodeTestHelper> displayNode3;
  scene->AddNode(displayNode3);

  displayableNode->AddAndObserveDisplayNodeID(displayNode3->GetID());

  CHECK_NOT_NULL(displayableNode->GetNthDisplayNodeID(1));
  CHECK_STRING(displayableNode->GetNthDisplayNodeID(1), displayNode3->GetID());
  CHECK_NULL(displayableNode->GetNthDisplayNode(1));
  // make sure it didn't change the first display node ID
  CHECK_NOT_NULL(displayableNode->GetNthDisplayNodeID(0));
  CHECK_STRING(displayableNode->GetNthDisplayNodeID(0), displayNode2->GetID());
  CHECK_NULL(displayableNode->GetNthDisplayNode(0));

  // Finally, add the node into the scene so it can look for the display nodes
  // in the scene.
  scene->AddNode(displayableNode);
  const std::vector<vtkDMMLDisplayNode*> &internalNodes = displayableNode->GetInternalDisplayNodes();
  CHECK_INT(internalNodes.size(), 2);
  CHECK_NOT_NULL(internalNodes[0]);
  CHECK_NOT_NULL(internalNodes[1]);

  // Test the scanning of GetDisplayNode
  vtkDMMLDisplayNode* nthDisplayNode = displayableNode->GetNthDisplayNode(1);

  CHECK_INT(displayableNode->GetInternalDisplayNodes().size(), 2);
  CHECK_POINTER(nthDisplayNode, displayNode3);
  CHECK_POINTER(displayableNode->GetInternalDisplayNodes()[1], displayNode3);
  CHECK_NOT_NULL(displayableNode->GetInternalDisplayNodes()[0]);

  // Typically called by vtkDMMLScene::Import
  displayableNode->UpdateScene(scene);

  CHECK_INT(displayableNode->GetInternalDisplayNodes().size(), 2);
  CHECK_POINTER(displayableNode->GetInternalDisplayNodes()[0], displayNode2);
  CHECK_POINTER(displayableNode->GetInternalDisplayNodes()[1], displayNode3);
  CHECK_NOT_NULL(displayableNode->GetNthDisplayNodeID(1));
  CHECK_STRING(displayableNode->GetNthDisplayNodeID(1), displayNode3->GetID());
  CHECK_POINTER(displayableNode->GetNthDisplayNode(1), displayNode3);
  // make sure it didn't change the first display node ID
  CHECK_NOT_NULL(displayableNode->GetNthDisplayNodeID(0));
  CHECK_STRING(displayableNode->GetNthDisplayNodeID(0), displayNode2->GetID());
  CHECK_POINTER(displayableNode->GetNthDisplayNode(0), displayNode2);

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestAddDisplayNodeIDEventsWithNoScene()
{
  // Make sure that the DisplayableModifiedEvent is fired even when the
  // display node is observed when the displayable is not in the scene.
  vtkNew<vtkDMMLScene> scene;

  vtkNew<vtkDMMLDisplayableNodeTestHelper1> displayableNode;

  vtkNew<vtkDMMLDisplayNodeTestHelper> displayNode;
  scene->AddNode(displayNode);
  displayableNode->SetAndObserveDisplayNodeID(displayNode->GetID());

  vtkNew<vtkDMMLCoreTestingUtilities::vtkDMMLNodeCallback> callback;
  displayableNode->AddObserver(vtkCommand::AnyEvent, callback);

  scene->AddNode(displayableNode);

  CHECK_STD_STRING(callback->GetErrorString(), "");
  // called because added into the scene
  CHECK_INT(callback->GetNumberOfModified(), 1);
  // called because display node pointer is retrieved by scene and is
  // observed by displayable node
  CHECK_INT(callback->GetNumberOfEvents(vtkDMMLDisplayableNode::DisplayModifiedEvent), 1);
  callback->ResetNumberOfEvents();

  displayNode->Modified();
  CHECK_STD_STRING(callback->GetErrorString(), "");
  CHECK_INT(callback->GetNumberOfModified(), 0);
  CHECK_INT(callback->GetNumberOfEvents(vtkDMMLDisplayableNode::DisplayModifiedEvent), 1);

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestAddDelayedDisplayNode()
{
  vtkNew<vtkDMMLScene> scene;

  vtkNew<vtkDMMLDisplayableNodeTestHelper1> displayableNode;
  scene->AddNode(displayableNode);

  // Set a node ID that doesn't exist but will exist.
  vtkNew<vtkDMMLDisplayNodeTestHelper> displayNode1;
  displayableNode->SetAndObserveDisplayNodeID("vtkDMMLDisplayNodeTestHelper1");

  CHECK_NOT_NULL(displayableNode->GetNthDisplayNodeID(0));
  CHECK_STRING(displayableNode->GetNthDisplayNodeID(0), "vtkDMMLDisplayNodeTestHelper1");
  CHECK_NULL(displayableNode->GetNthDisplayNode(0));

  scene->AddNode(displayNode1);

  CHECK_NOT_NULL(displayableNode->GetNthDisplayNodeID(0));
  CHECK_STRING(displayableNode->GetNthDisplayNodeID(0), "vtkDMMLDisplayNodeTestHelper1");

  // Search for the node in the scene.
  vtkDMMLNode* displayNode = displayableNode->GetNthDisplayNode(0);

  CHECK_NOT_NULL(displayableNode->GetNthDisplayNodeID(0));
  CHECK_STRING(displayableNode->GetNthDisplayNodeID(0), displayNode1->GetID());
  CHECK_POINTER(displayNode, displayNode1);
  CHECK_POINTER(displayableNode->GetNthDisplayNode(0), displayNode1);

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestRemoveDisplayNodeID()
{
  vtkNew<vtkDMMLScene> scene;

  vtkNew<vtkDMMLDisplayableNodeTestHelper1> displayableNode;
  scene->AddNode(displayableNode);

  vtkNew<vtkDMMLDisplayNodeTestHelper> displayNode1;
  scene->AddNode(displayNode1);
  vtkNew<vtkDMMLDisplayNodeTestHelper> displayNode2;
  scene->AddNode(displayNode2);
  vtkNew<vtkDMMLDisplayNodeTestHelper> displayNode3;
  scene->AddNode(displayNode3);

  displayableNode->AddAndObserveDisplayNodeID(displayNode1->GetID());
  displayableNode->AddAndObserveDisplayNodeID(displayNode2->GetID());
  displayableNode->AddAndObserveDisplayNodeID(displayNode3->GetID());

  displayableNode->RemoveNthDisplayNodeID(1);

  CHECK_INT(displayableNode->GetNumberOfDisplayNodes(), 2);
  CHECK_NOT_NULL(displayableNode->GetNthDisplayNodeID(0));
  CHECK_STRING(displayableNode->GetNthDisplayNodeID(0), displayNode1->GetID());
  CHECK_POINTER(displayableNode->GetNthDisplayNode(0), displayNode1);
  CHECK_NOT_NULL(displayableNode->GetNthDisplayNodeID(1));
  CHECK_STRING(displayableNode->GetNthDisplayNodeID(1), displayNode3->GetID());
  CHECK_POINTER(displayableNode->GetNthDisplayNode(1), displayNode3);

  displayableNode->SetAndObserveNthDisplayNodeID(1, nullptr);

  CHECK_INT(displayableNode->GetNumberOfDisplayNodes(), 1);
  CHECK_NOT_NULL(displayableNode->GetNthDisplayNodeID(0));
  CHECK_STRING(displayableNode->GetNthDisplayNodeID(0), displayNode1->GetID());
  CHECK_POINTER(displayableNode->GetNthDisplayNode(0), displayNode1);

  displayableNode->RemoveAllDisplayNodeIDs();

  CHECK_INT(displayableNode->GetNumberOfDisplayNodes(), 0);
  CHECK_NULL(displayableNode->GetNthDisplayNodeID(0));
  CHECK_NULL(displayableNode->GetNthDisplayNode(0));

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestRemoveDisplayNode()
{
  vtkNew<vtkDMMLScene> scene;

  vtkNew<vtkDMMLDisplayableNodeTestHelper1> displayableNode;
  scene->AddNode(displayableNode);

  vtkNew<vtkDMMLDisplayNodeTestHelper> displayNode1;
  scene->AddNode(displayNode1);
  vtkNew<vtkDMMLDisplayNodeTestHelper> displayNode2;
  scene->AddNode(displayNode2);
  vtkNew<vtkDMMLDisplayNodeTestHelper> displayNode3;
  scene->AddNode(displayNode3);

  displayableNode->AddAndObserveDisplayNodeID(displayNode1->GetID());
  displayableNode->AddAndObserveDisplayNodeID(displayNode2->GetID());
  displayableNode->AddAndObserveDisplayNodeID(displayNode3->GetID());

  scene->RemoveNode(displayNode3);

  CHECK_INT(displayableNode->GetNumberOfDisplayNodes(), 2);
  CHECK_NOT_NULL(displayableNode->GetNthDisplayNodeID(0));
  CHECK_STRING(displayableNode->GetNthDisplayNodeID(0), displayNode1->GetID());
  CHECK_POINTER(displayableNode->GetNthDisplayNode(0), displayNode1);
  CHECK_NOT_NULL(displayableNode->GetNthDisplayNodeID(1));
  CHECK_STRING(displayableNode->GetNthDisplayNodeID(1), displayNode2->GetID());
  CHECK_POINTER(displayableNode->GetNthDisplayNode(1), displayNode2);

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestRemoveDisplayableNode()
{
  vtkNew<vtkDMMLScene> scene;

  vtkNew<vtkDMMLDisplayableNodeTestHelper1> displayableNode;
  scene->AddNode(displayableNode);

  vtkNew<vtkDMMLDisplayNodeTestHelper> displayNode1;
  scene->AddNode(displayNode1);
  vtkNew<vtkDMMLDisplayNodeTestHelper> displayNode2;
  scene->AddNode(displayNode2);
  vtkNew<vtkDMMLDisplayNodeTestHelper> displayNode3;
  scene->AddNode(displayNode3);

  displayableNode->AddAndObserveDisplayNodeID(displayNode1->GetID());
  displayableNode->AddAndObserveDisplayNodeID(displayNode2->GetID());
  displayableNode->AddAndObserveDisplayNodeID(displayNode3->GetID());

  scene->RemoveNode(displayableNode);
  // Removing the scene from the displayable node clear the cached display
  // nodes.
  vtkDMMLDisplayNode* displayNode = displayableNode->GetNthDisplayNode(0);
  std::vector<vtkDMMLDisplayNode*> displayNodes =
    displayableNode->GetInternalDisplayNodes();

  CHECK_INT(displayableNode->GetNumberOfDisplayNodes(), 3);
  CHECK_NULL(displayNode);
  CHECK_INT(displayNodes.size(), 3);
  CHECK_NULL(displayNodes[0]);
  CHECK_NULL(displayNodes[1]);
  CHECK_NULL(displayNodes[2]);
  CHECK_NOT_NULL(displayableNode->GetNthDisplayNodeID(0));
  CHECK_STRING(displayableNode->GetNthDisplayNodeID(0), displayNode1->GetID());
  CHECK_NULL(displayableNode->GetNthDisplayNode(0));
  CHECK_NOT_NULL(displayableNode->GetNthDisplayNodeID(1));
  CHECK_STRING(displayableNode->GetNthDisplayNodeID(1), displayNode2->GetID());
  CHECK_NULL(displayableNode->GetNthDisplayNode(1));
  CHECK_NOT_NULL(displayableNode->GetNthDisplayNodeID(2));
  CHECK_STRING(displayableNode->GetNthDisplayNodeID(2), displayNode3->GetID());
  CHECK_NULL(displayableNode->GetNthDisplayNode(2));

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestDisplayModifiedEvent()
{
  vtkNew<vtkDMMLScene> scene;

  vtkNew<vtkDMMLDisplayableNodeTestHelper1> displayableNode;
  scene->AddNode(displayableNode);

  vtkNew<vtkDMMLDisplayNodeTestHelper> displayNode1;
  scene->AddNode(displayNode1);

  vtkNew<vtkDMMLCoreTestingUtilities::vtkDMMLNodeCallback> spy;
  displayableNode->AddObserver(vtkCommand::AnyEvent, spy);

  displayableNode->SetAndObserveDisplayNodeID(displayNode1->GetID());

  CHECK_INT(spy->GetTotalNumberOfEvents(), 3);
  CHECK_INT(spy->GetNumberOfEvents(vtkCommand::ModifiedEvent), 1);
  CHECK_INT(spy->GetNumberOfEvents(vtkDMMLNode::ReferenceAddedEvent), 1);
  CHECK_INT(spy->GetNumberOfEvents(vtkDMMLDisplayableNode::DisplayModifiedEvent), 1);
  spy->ResetNumberOfEvents();

  displayNode1->Modified();

  CHECK_INT(spy->GetTotalNumberOfEvents(), 1);
  CHECK_INT(spy->GetNumberOfEvents(vtkDMMLDisplayableNode::DisplayModifiedEvent), 1);
  spy->ResetNumberOfEvents();

  vtkNew<vtkDMMLDisplayNodeTestHelper> displayNode2;
  scene->AddNode(displayNode2);
  displayableNode->SetAndObserveDisplayNodeID(displayNode2->GetID());

  CHECK_INT(spy->GetTotalNumberOfEvents(), 3);
  CHECK_INT(spy->GetNumberOfEvents(vtkCommand::ModifiedEvent), 1);
  CHECK_INT(spy->GetNumberOfEvents(vtkDMMLNode::ReferenceModifiedEvent), 1);
  CHECK_INT(spy->GetNumberOfEvents(vtkDMMLDisplayableNode::DisplayModifiedEvent), 1);
  spy->ResetNumberOfEvents();

  displayableNode->SetAndObserveDisplayNodeID(nullptr);

  CHECK_INT(spy->GetTotalNumberOfEvents(), 3);
  CHECK_INT(spy->GetNumberOfEvents(vtkCommand::ModifiedEvent), 1);
  CHECK_INT(spy->GetNumberOfEvents(vtkDMMLNode::ReferenceRemovedEvent), 1);
  CHECK_INT(spy->GetNumberOfEvents(vtkDMMLDisplayableNode::DisplayModifiedEvent), 1);
  spy->ResetNumberOfEvents();

  vtkNew<vtkDMMLDisplayNodeTestHelper> displayNode3;
  displayableNode->SetAndObserveDisplayNodeID("vtkDMMLDisplayNodeTestHelper3");

  CHECK_INT(spy->GetTotalNumberOfEvents(), 1);
  CHECK_INT(spy->GetNumberOfEvents(vtkCommand::ModifiedEvent), 1);
  spy->ResetNumberOfEvents();

  scene->AddNode(displayNode3);
  // update the reference of the node
  vtkDMMLDisplayNode* displayNode = displayableNode->GetDisplayNode();

  CHECK_INT(spy->GetTotalNumberOfEvents(), 2);
  CHECK_INT(spy->GetNumberOfEvents(vtkDMMLNode::ReferenceAddedEvent), 1);
  CHECK_INT(spy->GetNumberOfEvents(vtkDMMLDisplayableNode::DisplayModifiedEvent), 1);
  spy->ResetNumberOfEvents();

  CHECK_POINTER(displayNode, displayNode3);

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestReferences()
{
  vtkSmartPointer<vtkDMMLScene> scene = vtkSmartPointer<vtkDMMLScene>::New();

  vtkSmartPointer<vtkDMMLDisplayNodeTestHelper> displayNode1 = vtkSmartPointer<vtkDMMLDisplayNodeTestHelper>::New();
  scene->AddNode(displayNode1);

  vtkSmartPointer<vtkDMMLDisplayableNodeTestHelper1> displayableNode = vtkSmartPointer<vtkDMMLDisplayableNodeTestHelper1>::New();
  scene->AddNode(displayableNode);

  displayableNode->AddAndObserveDisplayNodeID(displayNode1->GetID());

  vtkSmartPointer<vtkCollection> referencedNodes;
  referencedNodes.TakeReference(
    scene->GetReferencedNodes(displayableNode));

  CHECK_INT(referencedNodes->GetNumberOfItems(), 2);
  CHECK_POINTER(referencedNodes->GetItemAsObject(0), displayableNode);
  CHECK_POINTER(referencedNodes->GetItemAsObject(1), displayNode1);

  // Observing a display node not yet in the scene should add the reference in
  // the dmml scene, however GetReferencedNodes can't return the node because
  // it is not yet in the scene.
  vtkSmartPointer<vtkDMMLDisplayNodeTestHelper> displayNode2 = vtkSmartPointer<vtkDMMLDisplayNodeTestHelper>::New();
  displayableNode->AddAndObserveDisplayNodeID("vtkDMMLDisplayNodeTestHelper2");

  referencedNodes.TakeReference(scene->GetReferencedNodes(displayableNode));
  CHECK_INT(referencedNodes->GetNumberOfItems(), 2);
  CHECK_POINTER(referencedNodes->GetItemAsObject(0), displayableNode);
  CHECK_POINTER(referencedNodes->GetItemAsObject(1), displayNode1);

  scene->AddNode(displayNode2);
  displayableNode->GetNthDisplayNode(1);

  referencedNodes.TakeReference(scene->GetReferencedNodes(displayableNode));
  CHECK_INT(referencedNodes->GetNumberOfItems(), 3);
  CHECK_POINTER(referencedNodes->GetItemAsObject(0), displayableNode);
  CHECK_POINTER(referencedNodes->GetItemAsObject(1), displayNode1);
  CHECK_POINTER(referencedNodes->GetItemAsObject(2), displayNode2);

  // Test if the reference removal works
  vtkSmartPointer<vtkDMMLDisplayNodeTestHelper> displayNode3 = vtkSmartPointer<vtkDMMLDisplayNodeTestHelper>::New();
  scene->AddNode(displayNode3);
  displayableNode->AddAndObserveDisplayNodeID(displayNode3->GetID());
  displayableNode->RemoveNthDisplayNodeID(2);

  referencedNodes.TakeReference(scene->GetReferencedNodes(displayableNode));
  CHECK_INT(referencedNodes->GetNumberOfItems(), 3);
  CHECK_POINTER(referencedNodes->GetItemAsObject(0), displayableNode);
  CHECK_POINTER(referencedNodes->GetItemAsObject(1), displayNode1);
  CHECK_POINTER(referencedNodes->GetItemAsObject(2), displayNode2);

  // Simulate scene deletion to see if it crashes or not.
  // When the displayable node is destroyed, it unreferences nodes. Make sure
  // it is ok for nodes already removed/deleted like displayNode1.
  displayNode1 = nullptr;
  displayableNode = nullptr;
  displayNode2 = nullptr;
  displayNode3 = nullptr;
  scene = nullptr;

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestImportIntoSceneWithNodeIdConflict()
{
  std::cout << "\n---TestImportIntoSceneWithNodeIdConflict---\n" << std::endl;

  // Create scene1
  vtkNew<vtkDMMLScene> scene1;

  vtkNew<vtkDMMLDisplayableNodeTestHelper1> scene1DisplayableNode1;
  scene1DisplayableNode1->SetName("Displayable1");
  scene1->AddNode(scene1DisplayableNode1);

  vtkNew<vtkDMMLDisplayNodeTestHelper> scene1DisplayNode1;
  scene1DisplayNode1->SetName("Display1");
  scene1->AddNode(scene1DisplayNode1);
  scene1DisplayableNode1->SetAndObserveDisplayNodeID(scene1DisplayNode1->GetID());

  scene1DisplayableNode1->AddAndObserveDisplayNodeID("vtkDMMLDisplayNodeTestHelper3");
  scene1DisplayableNode1->AddAndObserveDisplayNodeID("vtkDMMLDisplayNodeTestHelper5");

  // scene1:
  //   +-Displayable1 -> Display1, (vtkDMMLDisplayNodeTestHelper3), (vtkDMMLDisplayNodeTestHelper5)
  //   +-Display1

  // Write scene1 to string
  scene1->SetSaveToXMLString(1);
  scene1->Commit();
  std::string xmlScene1 = scene1->GetSceneXMLString();
  std::cout << "Scene1:\n\n" << xmlScene1 << std::endl;

  // Create scene2

  vtkNew<vtkDMMLScene> scene2;
  scene2->RegisterNodeClass(vtkSmartPointer<vtkDMMLDisplayableNodeTestHelper1>::New());
  scene2->RegisterNodeClass(vtkSmartPointer<vtkDMMLDisplayNodeTestHelper>::New());

  vtkNew<vtkDMMLDisplayableNodeTestHelper1> scene2DisplayableNode2;
  scene2DisplayableNode2->SetName("Displayable2");
  scene2->AddNode(scene2DisplayableNode2);

  vtkNew<vtkDMMLDisplayNodeTestHelper> scene2DisplayNode2;
  scene2DisplayNode2->SetName("Display2");
  scene2->AddNode(scene2DisplayNode2);
  scene2DisplayableNode2->SetAndObserveDisplayNodeID(scene2DisplayNode2->GetID());

  vtkNew<vtkDMMLDisplayNodeTestHelper> scene2DisplayNode3;
  scene2DisplayNode3->SetName("Display3");
  scene2->AddNode(scene2DisplayNode3);

  vtkNew<vtkDMMLDisplayNodeTestHelper> scene2DisplayNode4;
  scene2DisplayNode4->SetName("Display4");
  scene2->AddNode(scene2DisplayNode4);

  scene2->SetSaveToXMLString(1);
  scene2->Commit();
  std::cout << "\n\nScene2 before import:\n\n" << scene2->GetSceneXMLString() << std::endl;

  // scene2:
  //   +-Displayable2 -> Display2
  //   +-Display2
  //   +-Display3
  //   +-Display4

  // Import scene1 into scene2
  scene2->SetLoadFromXMLString(1);
  scene2->SetSceneXMLString(xmlScene1);
  scene2->Import();

  // scene2:
  //   +-Displayable2 -> Display2
  //   +-Display2
  //   +-Display3
  //   +-Display4
  //   +-Displayable1 -> Display1 (the stray additional references should not be here)
  //   +-Display1

  scene2->Commit();
  std::cout << "\n\nScene2 after import:\n\n" << scene2->GetSceneXMLString() << std::endl;

  // Check Display1
  vtkDMMLDisplayNodeTestHelper* scene2DisplayNode1 =
    vtkDMMLDisplayNodeTestHelper::SafeDownCast(scene2->GetFirstNodeByName("Display1"));
  CHECK_NOT_NULL(scene2DisplayNode1);
  CHECK_STRING_DIFFERENT(scene2DisplayNode1->GetID(), "vtkDMMLDisplayNodeTestHelper1");

  // Check Display1 references
  vtkDMMLDisplayableNode* scene2DisplayableNode1 =
    vtkDMMLDisplayableNode::SafeDownCast(scene2->GetFirstNodeByName("Displayable1"));
  CHECK_NOT_NULL(scene2DisplayableNode1);
  CHECK_STRING(scene2DisplayableNode1->GetDisplayNodeID(), scene2DisplayNode1->GetID());
  CHECK_INT(scene2DisplayableNode1->GetNumberOfDisplayNodes(), 1);

  // Check Display2
  CHECK_STRING(scene2DisplayNode2->GetID(), "vtkDMMLDisplayNodeTestHelper1");

  // Check Display2 references
  CHECK_STRING(scene2DisplayableNode2->GetDisplayNodeID(), scene2DisplayNode2->GetID());

  return EXIT_SUCCESS;
}