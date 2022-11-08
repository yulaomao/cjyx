/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkDMMLModelNode.h>
#include <vtkDMMLModelStorageNode.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>
#include <vtkXMLDataElement.h>
#include <vtkXMLUtilities.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// vtkAddon includes
#include "vtkAddonSetGet.h"

// STD includes
#include <sstream>

using namespace vtkAddonTestingUtilities;
using namespace vtkDMMLCoreTestingUtilities;

#define DISPLAY_SCENE_NODE_REFERENCES 0

//---------------------------------------------------------------------------
class vtkDMMLNodeTestHelper1 : public vtkDMMLNode
{
public:
  // Provide a concrete New.
  static vtkDMMLNodeTestHelper1 *New();

  vtkTypeMacro(vtkDMMLNodeTestHelper1,vtkDMMLNode);

  NodeReferencesType& GetInternalReferencedNodes()
    {
    return this->NodeReferences;
    }

  vtkDMMLNode* CreateNodeInstance() override
    {
    return vtkDMMLNodeTestHelper1::New();
    }
  const char* GetNodeTagName() override
    {
    return "vtkDMMLNodeTestHelper1";
    }
  int GetNumberOfNodeReferenceObjects(const char* refrole)
    {
    if (!refrole)
      {
      return 0;
      }
    return this->NodeReferences[std::string(refrole)].size();
    }
  void ProcessDMMLEvents( vtkObject *caller, unsigned long event, void *callData ) override
    {
    Superclass::ProcessDMMLEvents(caller, event, callData);
    this->LastDMMLEventCaller = caller;
    this->LastDMMLEventId = event;
    }

  void SetOtherNodeID(const char* id);
  vtkGetStringMacro(OtherNodeID);

  vtkSetStdVectorMacro(TestingStringVector, std::vector<std::string>);
  vtkGetStdVectorMacro(TestingStringVector, std::vector<std::string>);

  vtkSetStdVectorMacro(TestingIntVector, std::vector<int>);
  vtkGetStdVectorMacro(TestingIntVector, std::vector<int>);

  vtkSetStdVectorMacro(TestingFloatVector, std::vector<float>);
  vtkGetStdVectorMacro(TestingFloatVector, std::vector<float>);

  void SetSceneReferences() override;
  void UpdateReferenceID(const char *oldID, const char *newID) override;
  void WriteXML(ostream& of, int nIndent) override;
  void ReadXMLAttributes(const char** atts) override;

  char*                     OtherNodeID;
  std::vector<std::string>  TestingStringVector;
  std::vector<int>          TestingIntVector;
  std::vector<float>        TestingFloatVector;

  vtkObject* LastDMMLEventCaller;
  unsigned long LastDMMLEventId;

private:
  vtkDMMLNodeTestHelper1()
    {
    this->OtherNodeID = nullptr;
    this->LastDMMLEventCaller = nullptr;
    this->LastDMMLEventId = 0;
    }
  ~vtkDMMLNodeTestHelper1() override
    {
    this->SetOtherNodeID(nullptr);
    }

};
vtkCxxSetReferenceStringMacro(vtkDMMLNodeTestHelper1, OtherNodeID);
vtkStandardNewMacro(vtkDMMLNodeTestHelper1);

//---------------------------------------------------------------------------
class vtkDMMLStorageNodeTestHelper : public vtkDMMLStorageNode
{
public:
  static vtkDMMLStorageNodeTestHelper *New();

  vtkTypeMacro(vtkDMMLStorageNodeTestHelper,vtkDMMLStorageNode);

  vtkDMMLNode* CreateNodeInstance() override
    {
    return vtkDMMLStorageNodeTestHelper::New();
    }
  const char* GetNodeTagName() override
    {
    return "vtkDMMLStorageNodeTestHelper";
    }

  void SetOtherNodeID(const char* id);
  vtkGetStringMacro(OtherNodeID);

  void SetSceneReferences() override;
  void UpdateReferenceID(const char *oldID, const char *newID) override;
  void WriteXML(ostream& of, int nIndent) override;
  void ReadXMLAttributes(const char** atts) override;

  // Implemented to satisfy the storage node interface
  bool CanReadInReferenceNode(vtkDMMLNode* refNode) override
    {
    return refNode->IsA("vtkDMMLNodeTestHelper1");
    }
  bool CanWriteFromReferenceNode(vtkDMMLNode* refNode) override
    {
    return refNode->IsA("vtkDMMLNodeTestHelper1");
    }
  void InitializeSupportedWriteFileTypes() override
    {
    this->SupportedWriteFileTypes->InsertNextValue(".noop");
    }
  int ReadDataInternal(vtkDMMLNode *refNode) override
    {
    vtkDMMLNodeTestHelper1 * node = vtkDMMLNodeTestHelper1::SafeDownCast(refNode);
    if(!node)
      {
      vtkErrorMacro("ReadData: Reference node is expected to be a vtkDMMLNodeTestHelper1");
      return 0;
      }
    return 1;
    }
  int WriteDataInternal(vtkDMMLNode *refNode) override
    {
    vtkDMMLNodeTestHelper1 * node = vtkDMMLNodeTestHelper1::SafeDownCast(refNode);
    if(!node)
      {
      vtkErrorMacro("WriteData: Reference node is expected to be a vtkDMMLNodeTestHelper1");
      return 0;
      }
    return 1;
    }

  char *OtherNodeID;

private:
  vtkDMMLStorageNodeTestHelper()
    {
    this->OtherNodeID = nullptr;
    this->DefaultWriteFileExtension = "noop";
    }
  ~vtkDMMLStorageNodeTestHelper() override
    {
    this->SetOtherNodeID(nullptr);
    }

};
vtkCxxSetReferenceStringMacro(vtkDMMLStorageNodeTestHelper, OtherNodeID);
vtkStandardNewMacro(vtkDMMLStorageNodeTestHelper);

//---------------------------------------------------------------------------
int TestBasicMethods();
int TestAttribute();
bool TestCopyWithScene();
int TestSetAndObserveNodeReferenceID();
bool TestAddRefrencedNodeIDWithNoScene();
bool TestAddDelayedReferenceNode();
bool TestRemoveReferencedNodeID();
bool TestRemoveReferencedNode();
bool TestRemoveReferencingNode();
bool TestNodeReferences();
bool TestReferenceModifiedEvent();
bool TestReferencesWithEvent();
bool TestMultipleReferencesToSameNodeWithEvent();
bool TestSingletonNodeReferencesUpdate();
bool TestAddReferencedNodeIDEventsWithNoScene();
int TestSetNodeReferenceID();
bool TestSetNodeReferenceIDToZeroOrEmptyString();
bool TestNodeReferenceSerialization();
bool TestClearScene();
bool TestImportSceneReferenceValidDuringImport();
int TestSaveLoadSpecialCharacters();
int TestReadWriteXMLProperties();

//---------------------------------------------------------------------------
int vtkDMMLNodeTest1(int , char * [] )
{
  bool res = true;
  res = res && (TestSaveLoadSpecialCharacters() == EXIT_SUCCESS);
  res = res && (TestBasicMethods() == EXIT_SUCCESS);
  res = res && (TestAttribute() == EXIT_SUCCESS);
  res = res && TestCopyWithScene();
  res = res && (TestSetAndObserveNodeReferenceID() == EXIT_SUCCESS);
  res = res && TestAddRefrencedNodeIDWithNoScene();
  res = res && TestAddDelayedReferenceNode();
  res = res && TestRemoveReferencedNodeID();
  res = res && TestRemoveReferencedNode();
  res = res && TestRemoveReferencingNode();
  res = res && TestNodeReferences();
  res = res && TestReferenceModifiedEvent();
  res = res && TestReferencesWithEvent();
  res = res && TestMultipleReferencesToSameNodeWithEvent();
  res = res && TestSingletonNodeReferencesUpdate();
  res = res && TestAddReferencedNodeIDEventsWithNoScene();
  res = res && (TestSetNodeReferenceID() == EXIT_SUCCESS);
  res = res && TestSetNodeReferenceIDToZeroOrEmptyString();
  res = res && TestNodeReferenceSerialization();
  res = res && TestClearScene();
  res = res && (TestReadWriteXMLProperties() == EXIT_SUCCESS);

  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//----------------------------------------------------------------------------
void vtkDMMLNodeTestHelper1::UpdateReferenceID(const char *oldID, const char *newID)
{
  this->Superclass::UpdateReferenceID(oldID, newID);
  if (this->OtherNodeID && !strcmp(oldID, this->OtherNodeID))
    {
    this->SetOtherNodeID(newID);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLNodeTestHelper1::SetSceneReferences()
{
  this->Superclass::SetSceneReferences();
  this->Scene->AddReferencedNodeID(this->OtherNodeID, this);
}

//----------------------------------------------------------------------------
void vtkDMMLNodeTestHelper1::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLStringMacro(OtherNodeRef, OtherNodeID);
  vtkDMMLWriteXMLStdStringVectorMacro(TestingStringVector, TestingStringVector, std::vector);
  vtkDMMLWriteXMLStdFloatVectorMacro(TestingFloatVector, TestingFloatVector, std::vector<float>);
  vtkDMMLWriteXMLStdIntVectorMacro(TestingIntVector, TestingIntVector, std::vector<int>);
  vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLNodeTestHelper1::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  Superclass::ReadXMLAttributes(atts);
  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLStringMacro(OtherNodeRef, OtherNodeID);
  vtkDMMLReadXMLStdStringVectorMacro(TestingStringVector, TestingStringVector, std::vector);
  vtkDMMLReadXMLStdFloatVectorMacro(TestingFloatVector, TestingFloatVector, std::vector<float>);
  vtkDMMLReadXMLStdIntVectorMacro(TestingIntVector, TestingIntVector, std::vector<int>);
  vtkDMMLReadXMLEndMacro();
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLStorageNodeTestHelper::UpdateReferenceID(const char *oldID, const char *newID)
{
  this->Superclass::UpdateReferenceID(oldID, newID);
  if (this->OtherNodeID && !strcmp(oldID, this->OtherNodeID))
    {
    this->SetOtherNodeID(newID);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLStorageNodeTestHelper::SetSceneReferences()
{
  this->Superclass::SetSceneReferences();
  this->Scene->AddReferencedNodeID(this->OtherNodeID, this);
}

//----------------------------------------------------------------------------
void vtkDMMLStorageNodeTestHelper::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLStringMacro(OtherNodeRef, OtherNodeID);
  vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLStorageNodeTestHelper::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  Superclass::ReadXMLAttributes(atts);
  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLStringMacro(OtherNodeRef, OtherNodeID);
  vtkDMMLReadXMLEndMacro();
  this->EndModify(disabledModify);
}

//---------------------------------------------------------------------------
bool TestSetAttribute(int line, const char* attribute, const char* value,
                      const char* expectedValue,
                      size_t expectedSize = 1,
                      int expectedModified = 0,
                      int totalNumberOfEvent = -1)
{
  vtkNew<vtkDMMLNodeTestHelper1> node;
  node->SetAttribute("Attribute0", "Value0");

  vtkNew<vtkDMMLNodeCallback> spy;
  node->AddObserver(vtkCommand::AnyEvent, spy.GetPointer());

  node->SetAttribute(attribute, value);

  if (!CheckString(line, "GetAttribute",
      node->GetAttribute(attribute), expectedValue))
    {
    return false;
    }
  if (!CheckInt(line, "GetAttributeNames",
      node->GetAttributeNames().size(), expectedSize))
    {
    return false;
    }
  if (totalNumberOfEvent == -1)
    {
    totalNumberOfEvent = expectedModified;
    }
  if (!CheckInt(line, "GetTotalNumberOfEvents",
      spy->GetTotalNumberOfEvents(), totalNumberOfEvent))
    {
    return false;
    }
  if (!CheckInt(line, "GetNumberOfEvents(vtkCommand::ModifiedEvent)",
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent), expectedModified))
    {
    return false;
    }
  spy->ResetNumberOfEvents();
  return true;
}

//---------------------------------------------------------------------------
int TestBasicMethods()
{
  vtkNew<vtkDMMLNodeTestHelper1> node1;
  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestAttribute()
{
  vtkNew<vtkDMMLNodeTestHelper1> node;
  // Test defaults and make sure it doesn't crash
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_NULL(node->GetAttribute(nullptr));
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  CHECK_INT(node->GetAttributeNames().size(), 0);
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_NULL(node->GetAttribute(""));
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  CHECK_NULL(node->GetAttribute("Attribute1"));

  // Test sets
  bool res = true;
  //                                                                 A: expectedSize
  //                                                                 B: expectedModified
  //                                                                 C: totalNumberOfEvent
  //                    (line    , attribute   , value   , expected, A, B, C
  res = TestSetAttribute(__LINE__, nullptr           , nullptr       , nullptr       , 1, 0, 2) && res;
  res = TestSetAttribute(__LINE__, nullptr           , ""      , nullptr       , 1, 0, 2) && res;
  res = TestSetAttribute(__LINE__, nullptr           , "Value1", nullptr       , 1, 0, 2) && res;
  res = TestSetAttribute(__LINE__, ""          , nullptr       , nullptr       , 1, 0, 2) && res;
  res = TestSetAttribute(__LINE__, ""          , ""      , nullptr       , 1, 0, 2) && res;
  res = TestSetAttribute(__LINE__, ""          , "Value1", nullptr       , 1, 0, 2) && res;
  res = TestSetAttribute(__LINE__, "Attribute1", nullptr       , nullptr) && res;
  res = TestSetAttribute(__LINE__, "Attribute1", ""      , ""      , 2, 1) && res;
  res = TestSetAttribute(__LINE__, "Attribute1", "Value1", "Value1", 2, 1) && res;
  res = TestSetAttribute(__LINE__, "Attribute0", nullptr       , nullptr       , 0, 1) && res;
  res = TestSetAttribute(__LINE__, "Attribute0", ""      , ""      , 1, 1) && res;
  res = TestSetAttribute(__LINE__, "Attribute0", "Value1", "Value1", 1, 1) && res;
  res = TestSetAttribute(__LINE__, "Attribute0", "Value0", "Value0", 1, 0) && res;
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

namespace
{

//----------------------------------------------------------------------------
bool TestCopyWithScene(
    int line,
    bool useSameClassNameForSourceAndCopy,
    bool useCopyWithSceneAfterAddingNode,
    const std::string& expectedSourceID,
    const std::string& expectedCopyID)
{
  struct ErrorWhenReturn
  {
    bool Opt1; bool Opt2; bool Opt3;
    int Line; bool Activated;
    ErrorWhenReturn(int line, bool opt1, bool opt2) :
      Opt1(opt1), Opt2(opt2), Line(line), Activated(true){}
    ~ErrorWhenReturn()
    {
      if (!this->Activated) { return; }
      std::cerr << "\nLine " << this->Line << " - TestCopyWithScene failed"
                << "\n\tuseSameClassNameForSourceAndCopy:" << this->Opt1
                << "\n\tuseCopyWithSceneAfterAddingNode:" << this->Opt2
                << std::endl;
    }
  };
  ErrorWhenReturn errorWhenReturn(
        line,
        useSameClassNameForSourceAndCopy,
        useCopyWithSceneAfterAddingNode);


  vtkNew<vtkDMMLScene> scene;
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLNodeTestHelper1>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLStorageNodeTestHelper>::New());

  vtkSmartPointer<vtkDMMLNode> source;

  // case: 1, x, x
  if (useSameClassNameForSourceAndCopy)
    {
    source = vtkSmartPointer<vtkDMMLNodeTestHelper1>::New();
    }
  // case: 0, x, x
  else
    {
    source = vtkSmartPointer<vtkDMMLStorageNodeTestHelper>::New();
    }

  source->SetAttribute("What", "TheSource");
  scene->AddNode(source.GetPointer());

  if (!CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        expectedSourceID.c_str(), source))
    {
    return false;
    }

  // Create a copy
  const char* name = "BetterThanTheOriginal";
  vtkNew<vtkDMMLNodeTestHelper1> copy;

  // case: x, 1, x
  if (useCopyWithSceneAfterAddingNode)
    {
    scene->AddNode(copy.GetPointer());
    }

  vtkNew<vtkDMMLNodeCallback> spy;
  copy->AddObserver(vtkCommand::ModifiedEvent, spy.GetPointer());

  copy->CopyWithScene(source);
  if (!CheckInt(
        __LINE__, "spy->GetNumberOfModified()",
        spy->GetNumberOfModified(), 1))
    {
    return false;
    }

  std::string uname = scene->GetUniqueNameByString(name);
  copy->SetName(uname.c_str());

  // case: x, 0, x
  if (!useCopyWithSceneAfterAddingNode)
    {
    scene->AddNode(copy.GetPointer());
    }

  if (!CheckInt(
        __LINE__, "scene->GetNumberOfNodes()",
        scene->GetNumberOfNodes(), 2)

      ||!CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        expectedSourceID.c_str(), source)

      ||!CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        expectedCopyID.c_str(), copy.GetPointer())

      ||!CheckNodeIdAndName(
        __LINE__, copy.GetPointer(),
        expectedCopyID.c_str(), "BetterThanTheOriginal")

      ||!CheckString(
        __LINE__, "copy->GetAttribute(\"What\")",
        copy->GetAttribute("What"), "TheSource"))
    {
    return false;
    }

  errorWhenReturn.Activated = false;
  return true;
}
}

//----------------------------------------------------------------------------
bool TestCopyWithScene()
{
  // vtkDMMLNodeTestHelper1, vtkDMMLStorageNodeTestHelper

  bool res = true;
  //                                  A: SameClass
  //                                  B: CopyAfterAdd
  //
  //                            (line    , A, B, C, expectedSrcID                  , expectedCopyID           )
  res = res && TestCopyWithScene(__LINE__, false, false, "vtkDMMLStorageNodeTestHelper1", "vtkDMMLNodeTestHelper11");
  res = res && TestCopyWithScene(__LINE__, false, false, "vtkDMMLStorageNodeTestHelper1", "vtkDMMLNodeTestHelper11");
//  res = res && TestCopyWithScene(__LINE__, 0, 1, "vtkDMMLStorageNodeTestHelper1", "vtkDMMLNodeTestHelper11"); // NOT SUPPORTED
//  res = res && TestCopyWithScene(__LINE__, 0, 1, "vtkDMMLStorageNodeTestHelper1", "vtkDMMLNodeTestHelper11"); // NOT SUPPORTED
  res = res && TestCopyWithScene(__LINE__, true, false, "vtkDMMLNodeTestHelper11"      , "vtkDMMLNodeTestHelper12");
  res = res && TestCopyWithScene(__LINE__, true, false, "vtkDMMLNodeTestHelper11"      , "vtkDMMLNodeTestHelper12");
//  res = res && TestCopyWithScene(__LINE__, 1, 1, "vtkDMMLNodeTestHelper11"      , "vtkDMMLNodeTestHelper12"); // NOT SUPPORTED
//  res = res && TestCopyWithScene(__LINE__, 1, 1, "vtkDMMLNodeTestHelper11"      , "vtkDMMLNodeTestHelper12"); // NOT SUPPORTED

  return res;
}

namespace
{

//----------------------------------------------------------------------------
bool CheckNthNodeReferenceID(int line, const char* function,
                             vtkDMMLNode* referencingNode, const char* role, int n,
                             const char* expectedNodeReferenceID,
                             bool referencingNodeAddedToScene = true,
                             vtkDMMLNode* expectedNodeReference = nullptr)
{
  const char* currentNodeReferenceID = referencingNode->GetNthNodeReferenceID(role, n);
  bool different = true;
  if (currentNodeReferenceID == nullptr || expectedNodeReferenceID == nullptr)
    {
    different = !(currentNodeReferenceID == nullptr && expectedNodeReferenceID == nullptr);
    }
  else if(strcmp(currentNodeReferenceID, expectedNodeReferenceID) == 0)
    {
    different = false;
    }
  if (different)
    {
    std::cerr << "Line " << line << " - " << function << " : CheckNthNodeReferenceID failed"
              << "\n\tcurrent " << n << "th NodeReferenceID:"
              << (currentNodeReferenceID ? currentNodeReferenceID : "<null>")
              << "\n\texpected " << n << "th NodeReferenceID:"
              << (expectedNodeReferenceID ? expectedNodeReferenceID : "<null>")
              << std::endl;
    return false;
    }

  if (!referencingNodeAddedToScene)
    {
    expectedNodeReference = nullptr;
    }
  vtkDMMLNode* currentNodeReference = referencingNode->GetNthNodeReference(role, n);
  if (currentNodeReference!= expectedNodeReference)
    {
    std::cerr << "Line " << line << " - " << function << " : CheckNthNodeReferenceID failed"
              << "\n\tcurrent " << n << "th NodeReference:" << currentNodeReference
              << "\n\texpected " << n << "th NodeReference:" << expectedNodeReference
              << std::endl;
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
int GetReferencedNodeCount(vtkDMMLScene* scene, vtkDMMLNode * referencingNode)
{
  vtkSmartPointer<vtkCollection> referencedNodes;
  referencedNodes.TakeReference(scene->GetReferencedNodes(referencingNode));
  return referencedNodes->GetNumberOfItems();
}

//----------------------------------------------------------------------------
int CheckNumberOfNodeReferences(int line, const char* function,
                                const char* role, vtkDMMLNode * referencingNode, int expected)
{
  int current = referencingNode->GetNumberOfNodeReferences(role);
  if (current != expected)
    {
    std::cerr << "Line " << line << " - " << function << " : CheckNumberOfNodeReferences failed"
              << "\n\tcurrent NumberOfNodeReferences:" << current
              << "\n\texpected NumberOfNodeReferences:" << expected
              << std::endl;
    return false;
    }
  vtkDMMLNodeTestHelper1 * referencingNodeTest = vtkDMMLNodeTestHelper1::SafeDownCast(referencingNode);
  if (referencingNodeTest)
    {
    current = referencingNodeTest->GetNumberOfNodeReferenceObjects(role);
    if (current != expected)
      {
      std::cerr << "Line " << line << " - " << function << " : CheckNumberOfNodeReferenceObjects failed"
                << "\n\tcurrent NumberOfNodeReferences:" << current
                << "\n\texpected NumberOfNodeReferences:" << expected
                << std::endl;
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
int CheckReferencedNodeCount(int line, const char* function,
                             vtkDMMLScene* scene, vtkDMMLNode * referencingNode, int expected)
{
  int current = GetReferencedNodeCount(scene, referencingNode);
  if (current != expected)
    {
    std::cerr << "Line " << line << " - " << function << " : CheckReferencedNodeCount failed"
              << "\n\tcurrent ReferencedNodesCount:" << current
              << "\n\texpected ReferencedNodesCount:" << expected
              << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool CheckReturnNode(int line, const char* function, vtkDMMLNode* current, vtkDMMLNode* expected)
{
  if (current != expected)
    {
    std::cerr << "Line " << line << " - " << function << " : CheckReturnNode failed"
              << "\n\tcurrent returnNode:" << current
              << "\n\texpected returnNode:" << expected
              << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool CheckNodeReferences(int line, const char* function, vtkDMMLScene* scene,
                         vtkDMMLNode * referencingNode, const char* role, int n,
                         vtkDMMLNode* expectedNodeReference,
                         int expectedNumberOfNodeReferences,
                         int expectedReferencedNodesCount,
                         vtkDMMLNode* currentReturnNode)
{
  if (!CheckNthNodeReferenceID(line, function, referencingNode, role,
                               /* n = */ n,
                               /* expectedNodeReferenceID = */
                                 (expectedNodeReference ? expectedNodeReference->GetID() : nullptr),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ expectedNodeReference
                               ))
    {
    return false;
    }
  if (!CheckNumberOfNodeReferences(line, function, role, referencingNode,
                                   expectedNumberOfNodeReferences))
    {
    return false;
    }
  if (!CheckReferencedNodeCount(line, function, scene, referencingNode,
                                expectedReferencedNodesCount))
    {
    return false;
    }
  if (!CheckReturnNode(line, function, currentReturnNode, expectedNodeReference))
    {
    return false;
    }
  return true;
}
}

//----------------------------------------------------------------------------
int TestSetAndObserveNodeReferenceID()
{
  vtkNew<vtkDMMLScene> scene;

  vtkDMMLNode *returnNode = nullptr;
  int referencedNodesCount = -1;

  std::string role1("refrole1");
  std::string role2("refrole2");
  std::string role3("refrole3");

  vtkNew<vtkDMMLNodeTestHelper1> referencingNode;
  scene->AddNode(referencingNode.GetPointer());

  vtkNew<vtkDMMLNodeTestHelper1> referencedNode1;
  scene->AddNode(referencedNode1.GetPointer() );

  /// Add empty referenced node with empty role
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  returnNode = referencingNode->AddAndObserveNodeReferenceID(nullptr, nullptr);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  if (!CheckNodeReferences(__LINE__, "AddAndObserveNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), nullptr,
                           /* n = */ 0,
                           /* expectedNodeReference = */ nullptr,
                           /* expectedNumberOfNodeReferences = */ 0,
                           /* expectedReferencedNodesCount = */ 1,
                           /* currentReturnNode = */ returnNode))
    {
    return EXIT_FAILURE;
    }

  /// Add empty referenced node with a role
  returnNode = referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), nullptr);
  if (!CheckNodeReferences(__LINE__, "AddAndObserveNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role1.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ nullptr,
                           /* expectedNumberOfNodeReferences = */ 0,
                           /* expectedReferencedNodesCount = */ 1,
                           /* currentReturnNode = */ returnNode))
    {
    return EXIT_FAILURE;
    }

  /// Add referenced node ID
  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode1->GetID());
  if (!CheckNodeReferences(__LINE__, "AddAndObserveNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role1.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ referencedNode1.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 1,
                           /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                           /* currentReturnNode = */ returnNode))
    {
    return EXIT_FAILURE;
    }

  /// Add empty referenced node ID
  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), nullptr);
  if (!CheckNodeReferences(__LINE__, "AddAndObserveNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role1.c_str(),
                           /* n = */ 1,
                           /* expectedNodeReference = */ nullptr,
                           /* expectedNumberOfNodeReferences = */ 1,
                           /* expectedReferencedNodesCount = */ referencedNodesCount,
                           /* currentReturnNode = */ returnNode))
    {
    return EXIT_FAILURE;
    }

  /// Change referenced node
  vtkNew<vtkDMMLNodeTestHelper1> referencedNode2;
  scene->AddNode(referencedNode2.GetPointer());

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetAndObserveNodeReferenceID(role1.c_str(), referencedNode2->GetID());

  if (!CheckNodeReferences(__LINE__, "SetAndObserveNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role1.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ referencedNode2.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 1,
                           /* expectedReferencedNodesCount = */ referencedNodesCount,
                           /* currentReturnNode = */ returnNode))
    {
    return EXIT_FAILURE;
    }

  /// Add referenced node
  vtkNew<vtkDMMLNodeTestHelper1> referencedNode3;
  scene->AddNode(referencedNode3.GetPointer());

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetAndObserveNthNodeReferenceID(role1.c_str(), 1, referencedNode3->GetID());

  if (!CheckNodeReferences(__LINE__, "SetAndObserveNthNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role1.c_str(),
                           /* n = */ 1,
                           /* expectedNodeReference = */ referencedNode3.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 2,
                           /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                           /* currentReturnNode = */ returnNode))
    {
    return EXIT_FAILURE;
    }

  // make sure it didn't change the first referenced node ID
  if (!CheckNthNodeReferenceID(__LINE__, "SetAndObserveNthNodeReferenceID", referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode2.GetPointer()))
    {
    return EXIT_FAILURE;
    }

  /// Add different role
  vtkNew<vtkDMMLNodeTestHelper1> referencedNode22;
  scene->AddNode(referencedNode22.GetPointer());

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetAndObserveNodeReferenceID(role2.c_str(), referencedNode22->GetID());

  if (!CheckNodeReferences(__LINE__, "SetAndObserveNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role2.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ referencedNode22.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 1,
                           /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                           /* currentReturnNode = */ returnNode))
    {
    return EXIT_FAILURE;
    }

  /// Add referenced node
  vtkNew<vtkDMMLNodeTestHelper1> referencedNode23;
  scene->AddNode(referencedNode23.GetPointer());

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetAndObserveNthNodeReferenceID(role2.c_str(), 1, referencedNode23->GetID());

  if (!CheckNodeReferences(__LINE__, "SetAndObserveNthNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role2.c_str(),
                           /* n = */ 1,
                           /* expectedNodeReference = */ referencedNode23.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 2,
                           /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                           /* currentReturnNode = */ returnNode))
    {
    return EXIT_FAILURE;
    }

  // make sure it didn't change the first referenced node ID
  if (!CheckNthNodeReferenceID(__LINE__, "SetAndObserveNthNodeReferenceID", referencingNode.GetPointer(),
                               role2.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode22->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode22.GetPointer()))
    {
    return EXIT_FAILURE;
    }

  // make sure it didn't change the first role references
  if (!CheckNthNodeReferenceID(__LINE__, "SetAndObserveNthNodeReferenceID", referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 1,
                               /* expectedNodeReferenceID = */ referencedNode3->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode3.GetPointer()))
    {
    return EXIT_FAILURE;
    }
  if (!CheckNumberOfNodeReferences(__LINE__, "SetAndObserveNthNodeReferenceID", role1.c_str(),
                                   referencingNode.GetPointer(),
                                   /* expectedNumberOfNodeReferences = */ 2))
    {
    return EXIT_FAILURE;
    }
  // make sure it didn't change the first referenced node ID associated with the first role
  if (!CheckNthNodeReferenceID(__LINE__, "SetAndObserveNthNodeReferenceID", referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode2.GetPointer()))
    {
    return EXIT_FAILURE;
    }

  /// change reference and check that it did
  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetAndObserveNthNodeReferenceID(role2.c_str(), 1, referencedNode3->GetID());

  if (!CheckNodeReferences(__LINE__, "SetAndObserveNthNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role2.c_str(),
                           /* n = */ 1,
                           /* expectedNodeReference = */ referencedNode3.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 2,
                           /* expectedReferencedNodesCount = */ referencedNodesCount - 1,
                           /* currentReturnNode = */ returnNode))
    {
    return EXIT_FAILURE;
    }
  // make sure it didn't change the first referenced node ID
  if (!CheckNthNodeReferenceID(__LINE__, "SetAndObserveNthNodeReferenceID", referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode2.GetPointer()))
    {
    return EXIT_FAILURE;
    }

  /// (1) set first reference, (2) set first reference to null and (3) set second reference
  vtkNew<vtkDMMLNodeTestHelper1> referencedNode31;
  scene->AddNode(referencedNode31.GetPointer());

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetAndObserveNthNodeReferenceID(role3.c_str(), 0, referencedNode31->GetID());

  if (!CheckNodeReferences(__LINE__, "SetAndObserveNthNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role3.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ referencedNode31.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 1,
                           /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                           /* currentReturnNode = */ returnNode))
    {
    return EXIT_FAILURE;
    }

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetAndObserveNthNodeReferenceID(role3.c_str(), 0, nullptr);

  if (!CheckNodeReferences(__LINE__, "SetAndObserveNthNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role3.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ nullptr,
                           /* expectedNumberOfNodeReferences = */ 0,
                           /* expectedReferencedNodesCount = */ referencedNodesCount -1,
                           /* currentReturnNode = */ returnNode))
    {
    return EXIT_FAILURE;
    }

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetAndObserveNthNodeReferenceID(role3.c_str(), 1, referencedNode31->GetID());

  if (!CheckNodeReferences(__LINE__, "SetAndObserveNthNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role3.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ referencedNode31.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 1,
                           /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                           /* currentReturnNode = */ returnNode))
    {
    return EXIT_FAILURE;
    }

  /// Set Nth reference to 0
  std::vector<int> referenceIndices;
  referenceIndices.push_back(20);
  referenceIndices.push_back(30);
  referenceIndices.push_back(31);
  referenceIndices.push_back(32);
  referenceIndices.push_back(21);
  referenceIndices.push_back(10);
  referenceIndices.push_back(3);
  referenceIndices.push_back(-1);
  for (std::vector<int>::iterator it = referenceIndices.begin();
       it != referenceIndices.end();
       ++it)
    {
    int nth = *it;
    referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
    TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_BEGIN(); // error is only returned if a negative index is provided and it's fine
    returnNode = referencingNode->SetAndObserveNthNodeReferenceID(role3.c_str(), nth, nullptr);
    TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_END();

    if (!CheckNodeReferences(__LINE__, "SetAndObserveNthNodeReferenceID", scene.GetPointer(),
                             referencingNode.GetPointer(), role3.c_str(),
                             /* n = */ nth,
                             /* expectedNodeReference = */ nullptr,
                             /* expectedNumberOfNodeReferences = */ 1,
                             /* expectedReferencedNodesCount = */ referencedNodesCount,
                             /* currentReturnNode = */ returnNode))
      {
      return EXIT_FAILURE;
      }
    }

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
bool TestAddRefrencedNodeIDWithNoScene()
{
  vtkNew<vtkDMMLScene> scene;

  std::string role1("refrole1");

  vtkNew<vtkDMMLNodeTestHelper1> referencingNode;

  vtkNew<vtkDMMLNodeTestHelper1> referencedNode1;
  scene->AddNode(referencedNode1.GetPointer());

  /// Add referenced node
  referencingNode->SetAndObserveNodeReferenceID(role1.c_str(), referencedNode1->GetID());

  if (!CheckNthNodeReferenceID(__LINE__, "SetAndObserveNodeReferenceID",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode1->GetID(),
                               /* referencingNodeAddedToScene = */ false,
                               /* expectedNodeReference = */ referencedNode1.GetPointer()))
    {
    return false;
    }

  /// Change referenced node
  vtkNew<vtkDMMLNodeTestHelper1> referencedNode2;
  scene->AddNode(referencedNode2.GetPointer());

  referencingNode->SetAndObserveNodeReferenceID(role1.c_str(), referencedNode2->GetID());

  if (!CheckNthNodeReferenceID(__LINE__, "SetAndObserveNodeReferenceID",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ false,
                               /* expectedNodeReference = */ referencedNode2.GetPointer()))
    {
    return false;
    }

  /// Add referenced node
  vtkNew<vtkDMMLNodeTestHelper1> referencedNode3;
  scene->AddNode(referencedNode3.GetPointer());

  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode3->GetID());

  if (!CheckNthNodeReferenceID(__LINE__, "AddAndObserveNodeReferenceID",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 1,
                               /* expectedNodeReferenceID = */ referencedNode3->GetID(),
                               /* referencingNodeAddedToScene = */ false,
                               /* expectedNodeReference = */ referencedNode3.GetPointer()))
    {
    return false;
    }

  // make sure it didn't change the first referenced node ID
  if (!CheckNthNodeReferenceID(__LINE__, "AddAndObserveNodeReferenceID",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ false,
                               /* expectedNodeReference = */ referencedNode2.GetPointer()))
    {
    return false;
    }

  // Finally, add the node into the scene so it can look for the referenced nodes
  // in the scene.
  scene->AddNode(referencingNode.GetPointer());

  if (!CheckNthNodeReferenceID(__LINE__, "vtkDMMLScene::AddNode",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 1,
                               /* expectedNodeReferenceID = */ referencedNode3->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode3.GetPointer()))
    {
    return false;
    }

  // make sure it didn't change the first referenced node ID
  if (!CheckNthNodeReferenceID(__LINE__, "vtkDMMLScene::AddNode",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode2.GetPointer()))
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestAddDelayedReferenceNode()
{
  vtkNew<vtkDMMLScene> scene;
  std::string role1("refrole1");
  std::string role2("refrole2");

  vtkNew<vtkDMMLNodeTestHelper1> referencingNode;
  scene->AddNode(referencingNode.GetPointer());

  // Set a node ID that doesn't exist but will exist.
  vtkNew<vtkDMMLNodeTestHelper1> referencedNode1;
  referencingNode->SetAndObserveNodeReferenceID(role1.c_str(),"vtkDMMLNodeTestHelper12");

  if (!CheckNthNodeReferenceID(__LINE__, "SetAndObserveNodeReferenceID",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ "vtkDMMLNodeTestHelper12",
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ nullptr))
    {
    return false;
    }

  scene->AddNode(referencedNode1.GetPointer());

  if (referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) == nullptr ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 0), "vtkDMMLNodeTestHelper12") ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->GetReferencedNode() != nullptr)
    {
    std::cerr << "Line " << __LINE__ << ": SetAndObserveNodeReferenceID failed" << std::endl;
    return false;
    }

  // Search for the node in the scene.
  if (!CheckNthNodeReferenceID(__LINE__, "SetAndObserveNodeReferenceID",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode1->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode1.GetPointer()))
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestRemoveReferencedNodeID()
{
  std::string role1("refrole1");
  std::string role2("refrole2");

  vtkNew<vtkDMMLScene> scene;

  vtkNew<vtkDMMLNodeTestHelper1> referencingNode;
  scene->AddNode(referencingNode.GetPointer());

  vtkNew<vtkDMMLNodeTestHelper1> referencedNode1;
  scene->AddNode(referencedNode1.GetPointer());
  vtkNew<vtkDMMLNodeTestHelper1> referencedNode2;
  scene->AddNode(referencedNode2.GetPointer());
  vtkNew<vtkDMMLNodeTestHelper1> referencedNode3;
  scene->AddNode(referencedNode3.GetPointer());

  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode1->GetID());
  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode2->GetID());
  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode3->GetID());

  referencingNode->RemoveNthNodeReferenceID(role1.c_str(), 1);

  if (!CheckNumberOfNodeReferences(__LINE__, "RemoveNthNodeReferenceID",
                                   role1.c_str(),
                                   referencingNode.GetPointer(),
                                   /* expectedNumberOfNodeReferences = */ 2))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "RemoveNthNodeReferenceID",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode1->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode1.GetPointer()
                               ))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "RemoveNthNodeReferenceID",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 1,
                               /* expectedNodeReferenceID = */ referencedNode3->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode3.GetPointer()))
    {
    return false;
    }

  referencingNode->SetAndObserveNthNodeReferenceID(role1.c_str(), 1, nullptr);

  if (!CheckNumberOfNodeReferences(__LINE__, "SetAndObserveNthNodeReferenceID",
                                   role1.c_str(),
                                   referencingNode.GetPointer(),
                                   /* expectedNumberOfNodeReferences = */ 1))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "SetAndObserveNthNodeReferenceID",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode1->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode1.GetPointer()
                               ))
    {
    return false;
    }

  // add second role refs
  referencingNode->AddAndObserveNodeReferenceID(role2.c_str(), referencedNode2->GetID());
  referencingNode->AddAndObserveNodeReferenceID(role2.c_str(), referencedNode3->GetID());

  referencingNode->RemoveNthNodeReferenceID(role2.c_str(), 1);

  if (!CheckNumberOfNodeReferences(__LINE__, "RemoveNthNodeReferenceID",
                                   role1.c_str(),
                                   referencingNode.GetPointer(),
                                   /* expectedNumberOfNodeReferences = */ 1))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "RemoveNthNodeReferenceID",
                               referencingNode.GetPointer(),
                               role2.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode2.GetPointer()
                               ))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "RemoveNthNodeReferenceID",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode1->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode1.GetPointer()
                               ))
    {
    return false;
    }

  referencingNode->RemoveNodeReferenceIDs(role1.c_str());

  if (!CheckNumberOfNodeReferences(__LINE__, "RemoveAllNodeReferenceIDs",
                                   role1.c_str(),
                                   referencingNode.GetPointer(),
                                   /* expectedNumberOfNodeReferences = */ 0))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "RemoveAllNodeReferenceIDs",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ nullptr,
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ nullptr
                               ))
    {
    return false;
    }

  if (!CheckNumberOfNodeReferences(__LINE__, "RemoveAllNodeReferenceIDs",
                                   role2.c_str(),
                                   referencingNode.GetPointer(),
                                   /* expectedNumberOfNodeReferences = */ 1))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "RemoveAllNodeReferenceIDs",
                               referencingNode.GetPointer(),
                               role2.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode2.GetPointer()
                               ))
    {
    return false;
    }

  referencingNode->RemoveNodeReferenceIDs(nullptr);

  if (!CheckNumberOfNodeReferences(__LINE__, "RemoveNodeReferenceIDs",
                                   role1.c_str(),
                                   referencingNode.GetPointer(),
                                   /* expectedNumberOfNodeReferences = */ 0))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "RemoveNodeReferenceIDs",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ nullptr,
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ nullptr
                               ))
    {
    return false;
    }

  if (!CheckNumberOfNodeReferences(__LINE__, "RemoveNodeReferenceIDs",
                                   role2.c_str(),
                                   referencingNode.GetPointer(),
                                   /* expectedNumberOfNodeReferences = */ 0))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "RemoveNodeReferenceIDs",
                               referencingNode.GetPointer(),
                               role2.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ nullptr,
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ nullptr
                               ))
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestRemoveReferencedNode()
{
  std::string role1("refrole1");

  vtkNew<vtkDMMLScene> scene;

  vtkNew<vtkDMMLNodeTestHelper1> referencingNode;
  scene->AddNode(referencingNode.GetPointer());

  vtkNew<vtkDMMLNodeTestHelper1> referencedNode1;
  scene->AddNode(referencedNode1.GetPointer());
  vtkNew<vtkDMMLNodeTestHelper1> referencedNode2;
  scene->AddNode(referencedNode2.GetPointer());
  vtkNew<vtkDMMLNodeTestHelper1> referencedNode3;
  scene->AddNode(referencedNode3.GetPointer());

  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode1->GetID());
  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode2->GetID());
  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode3->GetID());

  scene->RemoveNode(referencedNode3.GetPointer());

  if (!CheckNumberOfNodeReferences(__LINE__, "vtkDMMLScene::RemoveNode(referencedNode)",
                                   role1.c_str(),
                                   referencingNode.GetPointer(),
                                   /* expectedNumberOfNodeReferences = */ 2))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "vtkDMMLScene::RemoveNode(referencedNode)",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode1->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode1.GetPointer()
                               ))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "vtkDMMLScene::RemoveNode(referencedNode)",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 1,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode2.GetPointer()
                               ))
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestRemoveReferencingNode()
{
  std::string role1("refrole1");

  vtkNew<vtkDMMLScene> scene;

  vtkNew<vtkDMMLNodeTestHelper1> referencingNode;
  scene->AddNode(referencingNode.GetPointer());

  vtkNew<vtkDMMLNodeTestHelper1> referencedNode1;
  scene->AddNode(referencedNode1.GetPointer());
  vtkNew<vtkDMMLNodeTestHelper1> referencedNode2;
  scene->AddNode(referencedNode2.GetPointer());
  vtkNew<vtkDMMLNodeTestHelper1> referencedNode3;
  scene->AddNode(referencedNode3.GetPointer());

  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode1->GetID());
  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode2->GetID());
  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode3->GetID());

  scene->RemoveNode(referencingNode.GetPointer());
  // Removing the scene from the  node clear the cached referenced
  // nodes.
  vtkDMMLNode* referencedNode = referencingNode->GetNthNodeReference(role1.c_str(), 0);
  std::vector<vtkDMMLNode*> referencedNodes;
  referencingNode->GetNodeReferences(role1.c_str(), referencedNodes);

  if (!CheckNumberOfNodeReferences(__LINE__, "vtkDMMLScene::RemoveNode(referencingNode)",
                                   role1.c_str(),
                                   referencingNode.GetPointer(),
                                   /* expectedNumberOfNodeReferences = */ 3))
    {
    return false;
    }

  int expectedReferencedNodeVectorSize = 3;
  int currentReferencedNodeVectorSize = referencedNodes.size();
  if (currentReferencedNodeVectorSize != expectedReferencedNodeVectorSize)
    {
    std::cerr << "Line " << __LINE__ << " - " << "GetNodeReferences" << " failed"
              << "\n\tcurrent ReferencedNodeVectorSize:" << currentReferencedNodeVectorSize
              << "\n\texpected ReferencedNodeVectorSize:" << expectedReferencedNodeVectorSize
              << std::endl;
    }

  if (referencedNode != nullptr ||
      referencedNodes[0] != nullptr ||
      referencedNodes[1] != nullptr ||
      referencedNodes[2] != nullptr
      )
    {
    std::cerr << "Line " << __LINE__ << ": RemoveNode failed" << std::endl;
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "vtkDMMLScene::RemoveNode(referencingNode)",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode1->GetID(),
                               /* referencingNodeAddedToScene = */ false,
                               /* expectedNodeReference = */ nullptr
                               ))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "vtkDMMLScene::RemoveNode(referencingNode)",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 1,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ false,
                               /* expectedNodeReference = */ nullptr
                               ))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "vtkDMMLScene::RemoveNode(referencingNode)",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 2,
                               /* expectedNodeReferenceID = */ referencedNode3->GetID(),
                               /* referencingNodeAddedToScene = */ false,
                               /* expectedNodeReference = */ nullptr
                               ))
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestNodeReferences()
{
  std::string role1("refrole1");

  vtkSmartPointer<vtkDMMLScene> scene = vtkSmartPointer<vtkDMMLScene>::New();

  vtkSmartPointer<vtkDMMLNodeTestHelper1> referencedNode1 =
    vtkSmartPointer<vtkDMMLNodeTestHelper1>::New();
  scene->AddNode(referencedNode1);

  vtkSmartPointer<vtkDMMLNodeTestHelper1> referencingNode =
    vtkSmartPointer<vtkDMMLNodeTestHelper1>::New();
  scene->AddNode(referencingNode);

  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode1->GetID());

  vtkSmartPointer<vtkCollection> referencedNodes;
  referencedNodes.TakeReference(
    scene->GetReferencedNodes(referencingNode.GetPointer()));

  if (referencedNodes->GetNumberOfItems() != 2 ||
      referencedNodes->GetItemAsObject(0) != referencingNode.GetPointer() ||
      referencedNodes->GetItemAsObject(1) != referencedNode1.GetPointer())
    {
    std::cerr << "Line " << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
              << referencedNodes->GetNumberOfItems() << std::endl;
    return false;
    }

  // Observing a referenced node not yet in the scene should add the reference in
  // the dmml scene, however GetReferencedNodes can't return the node because
  // it is not yet in the scene.
  vtkSmartPointer<vtkDMMLNodeTestHelper1> referencedNode2 =
    vtkSmartPointer<vtkDMMLNodeTestHelper1>::New();
  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(),"vtkDMMLNodeTestHelper13");

  referencedNodes.TakeReference(
    scene->GetReferencedNodes(referencingNode.GetPointer()));
  if (referencedNodes->GetNumberOfItems() != 2 ||
      referencedNodes->GetItemAsObject(0) != referencingNode.GetPointer() ||
      referencedNodes->GetItemAsObject(1) != referencedNode1.GetPointer())
    {
    std::cerr << "Line " << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
              << referencedNodes->GetNumberOfItems() << std::endl;
    return false;
    }

  scene->AddNode(referencedNode2);
  //referencingNode->GetNthNodeReference(role1.c_str(), 1);

  referencedNodes.TakeReference(
    scene->GetReferencedNodes(referencingNode));
  if (referencedNodes->GetNumberOfItems() != 3 ||
      referencedNodes->GetItemAsObject(0) != referencingNode.GetPointer() ||
      referencedNodes->GetItemAsObject(1) != referencedNode1.GetPointer() ||
      referencedNodes->GetItemAsObject(2) != referencedNode2.GetPointer())
    {
    std::cerr << "Line " << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
              << referencedNodes->GetNumberOfItems() << std::endl;
    return false;
    }

  // Test if the reference removal works
  vtkSmartPointer<vtkDMMLNodeTestHelper1> referencedNode3 =
    vtkSmartPointer<vtkDMMLNodeTestHelper1>::New();
  scene->AddNode(referencedNode3);
  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode3->GetID());
  referencingNode->RemoveNthNodeReferenceID(role1.c_str(), 2);

  referencedNodes.TakeReference(
    scene->GetReferencedNodes(referencingNode));
  int expectedNumberOfItems = 3;
  if (referencedNodes->GetNumberOfItems() != expectedNumberOfItems ||
      referencedNodes->GetItemAsObject(0) != referencingNode.GetPointer() ||
      referencedNodes->GetItemAsObject(1) != referencedNode1.GetPointer() ||
      referencedNodes->GetItemAsObject(2) != referencedNode2.GetPointer())
    {
    std::cerr << "Line " << __LINE__ << " : SetAndObserveNodeReferenceID failed"
              << "\n\tcurrent NumberOfItems:" << referencedNodes->GetNumberOfItems()
              << "\n\texpected NumberOfItems:" << expectedNumberOfItems
              << std::endl;
    return false;
    }

  // Simulate scene deletion to see if it crashes or not.
  // When the  node is destroyed, it unreferences nodes. Make sure
  // it is ok for nodes already removed/deleted like referencedNode1.
  referencedNode1 = nullptr;
  referencingNode = nullptr;
  referencedNode2 = nullptr;
  referencedNode3 = nullptr;
  scene = nullptr;

  return true;
}

//----------------------------------------------------------------------------
bool TestReferenceModifiedEvent()
{
  std::string role1("refrole1");

  vtkNew<vtkDMMLScene> scene;

  vtkNew<vtkDMMLNodeTestHelper1> referencingNode;
  scene->AddNode(referencingNode.GetPointer());

  vtkNew<vtkDMMLNodeTestHelper1> referencedNode1;
  scene->AddNode(referencedNode1.GetPointer());

  vtkNew<vtkDMMLNodeCallback> spy;
  referencingNode->AddObserver(vtkCommand::AnyEvent, spy.GetPointer());

  referencingNode->SetAndObserveNodeReferenceID(role1.c_str(), referencedNode1->GetID());

  if (spy->GetTotalNumberOfEvents() != 2 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1 ||
      spy->GetNumberOfEvents(vtkDMMLNode::ReferenceAddedEvent) != 1)
    {
    std::cerr << "Line " << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkDMMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();

  vtkNew<vtkDMMLNodeTestHelper1> referencedNode2;
  scene->AddNode(referencedNode2.GetPointer());
  referencingNode->SetAndObserveNodeReferenceID(role1.c_str(), referencedNode2->GetID());

  if (spy->GetTotalNumberOfEvents() != 2 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1 ||
      spy->GetNumberOfEvents(vtkDMMLNode::ReferenceModifiedEvent) != 1)
    {
    std::cerr << "Line " << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkDMMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();

  referencingNode->SetAndObserveNodeReferenceID(role1.c_str(),  nullptr);

  if (spy->GetTotalNumberOfEvents() != 2 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1 ||
      spy->GetNumberOfEvents(vtkDMMLNode::ReferenceRemovedEvent) != 1)
    {
    std::cerr << "Line " << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkDMMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();

  vtkNew<vtkDMMLNodeTestHelper1> referencedNode3;
  referencingNode->SetAndObserveNodeReferenceID(role1.c_str(), "vtkDMMLNodeTestHelper14");

  // If the referenced node is not yet in the scene then vtkDMMLNode::ReferenceAddedEvent
  // should not be invoked yet.
  if (spy->GetTotalNumberOfEvents() != 1 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1)
    {
    std::cerr << "Line " << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkDMMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();

  scene->AddNode(referencedNode3.GetPointer());
  // update the reference of the node
  vtkDMMLNode* referencedNode = referencingNode->GetNodeReference(role1.c_str());

  if (spy->GetTotalNumberOfEvents() != 1 ||
      spy->GetNumberOfEvents(vtkDMMLNode::ReferenceAddedEvent) != 1 ||
      referencedNode != referencedNode3.GetPointer())
    {
    std::cerr << "Line " << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkDMMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();
  return true;
}

//----------------------------------------------------------------------------
bool TestReferencesWithEvent()
{
  std::string role1("refrole1");

  vtkNew<vtkDMMLScene> scene;

  vtkNew<vtkDMMLNodeTestHelper1> referencingNode;
  scene->AddNode(referencingNode.GetPointer());

  vtkNew<vtkDMMLNodeTestHelper1> referencedNode1;
  scene->AddNode(referencedNode1.GetPointer());

  vtkNew<vtkIntArray> events;
  events->InsertNextValue(777);
  events->InsertNextValue(888);

  vtkNew<vtkDMMLNodeCallback> spy;
  referencingNode->AddObserver(vtkCommand::AnyEvent, spy.GetPointer());

  referencingNode->SetAndObserveNodeReferenceID(role1.c_str(), referencedNode1->GetID(), events.GetPointer());

  if (spy->GetTotalNumberOfEvents() != 2 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1 ||
      spy->GetNumberOfEvents(vtkDMMLNode::ReferenceAddedEvent) != 1 )
    {
    std::cerr << "Line " << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkDMMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }

  if (referencingNode->GetInternalReferencedNodes()[role1][0]->GetReferencedNode() == nullptr ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->GetEvents()->GetNumberOfTuples() != 2 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->GetEvents()->GetValue(0) != 777 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->GetEvents()->GetValue(1) != 888)
    {
    std::cerr << "Line " << __LINE__ << ": SetAndObserveNodeReferenceID failed: events are incorrect" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkDMMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }

  spy->ResetNumberOfEvents();

  vtkNew<vtkDMMLNodeTestHelper1> referencedNode2;
  scene->AddNode(referencedNode2.GetPointer());
  referencingNode->SetAndObserveNodeReferenceID(role1.c_str(), referencedNode2->GetID(), events.GetPointer());

  if (spy->GetTotalNumberOfEvents() != 2 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1 ||
      spy->GetNumberOfEvents(vtkDMMLNode::ReferenceModifiedEvent) != 1)
    {
    std::cerr << "Line " << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkDMMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }
  if (referencingNode->GetInternalReferencedNodes()[role1][0]->GetReferencedNode() == nullptr ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->GetEvents()->GetNumberOfTuples() != 2 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->GetEvents()->GetValue(0) != 777 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->GetEvents()->GetValue(1) != 888)
    {
    std::cerr << "Line " << __LINE__ << ": SetAndObserveNodeReferenceID failed: events are incorrect" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkDMMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }

  spy->ResetNumberOfEvents();

  referencingNode->SetAndObserveNodeReferenceID(role1.c_str(),  nullptr);

  if (spy->GetTotalNumberOfEvents() != 2 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1 ||
      spy->GetNumberOfEvents(vtkDMMLNode::ReferenceRemovedEvent) != 1)
    {
    std::cerr << "Line " << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkDMMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();


  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode1->GetID(), events.GetPointer());

  if (referencingNode->GetInternalReferencedNodes()[role1][0]->GetReferencedNode() == nullptr ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->GetEvents()->GetNumberOfTuples() != 2 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->GetEvents()->GetValue(0) != 777 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->GetEvents()->GetValue(1) != 888)
    {
    std::cerr << "Line " << __LINE__ << ": SetAndObserveNodeReferenceID failed: events are incorrect" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkDMMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }


  spy->ResetNumberOfEvents();

  referencingNode->SetAndObserveNthNodeReferenceID(role1.c_str(), 0, referencedNode1->GetID(), events.GetPointer());

  if (referencingNode->GetInternalReferencedNodes()[role1][0]->GetReferencedNode() == nullptr ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->GetEvents()->GetNumberOfTuples() != 2 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->GetEvents()->GetValue(0) != 777 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->GetEvents()->GetValue(1) != 888)
    {
    std::cerr << "Line " << __LINE__ << ": SetAndObserveNodeReferenceID failed: events are incorrect" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkDMMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }

  // Check if we can successfully modify the events only
  events->SetNumberOfTuples(0);
  events->InsertNextValue(345);
  events->InsertNextValue(777);
  events->InsertNextValue(999);

  referencingNode->SetAndObserveNthNodeReferenceID(role1.c_str(), 0, referencedNode1->GetID(), events.GetPointer());

  if (referencingNode->GetInternalReferencedNodes()[role1][0]->GetReferencedNode() == nullptr ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->GetEvents()->GetNumberOfTuples() != 3 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->GetEvents()->GetValue(0) != 345 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->GetEvents()->GetValue(1) != 777 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->GetEvents()->GetValue(2) != 999)
    {
    std::cerr << "Line " << __LINE__ << ": SetAndObserveNodeReferenceID failed: events are incorrect" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkDMMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }

  referencingNode->LastDMMLEventId=0;
  referencedNode1->InvokeEvent(345);
  if (referencingNode->LastDMMLEventId!=345)
    {
    std::cerr << "Line " << __LINE__ << ": SetAndObserveNodeReferenceID failed: event 345 is not received after event list modified" << std::endl;
    return false;
    }

  referencingNode->LastDMMLEventId=0;
  referencedNode1->InvokeEvent(777);
  if (referencingNode->LastDMMLEventId!=777)
    {
    std::cerr << "Line " << __LINE__ << ": SetAndObserveNodeReferenceID failed: event 777 is not received after event list modified" << std::endl;
    return false;
    }

  referencingNode->LastDMMLEventId=0;
  referencedNode1->InvokeEvent(888);
  if (referencingNode->LastDMMLEventId==888)
    {
    std::cerr << "Line " << __LINE__ << ": SetAndObserveNodeReferenceID failed: event 888 is still received after event list modified" << std::endl;
    return false;
    }

  // Make sure everything works if we don't reuse the same event object
  vtkNew<vtkIntArray> events2;
  events2->InsertNextValue(345);
  events2->InsertNextValue(777);
  events2->InsertNextValue(111);

  referencingNode->SetAndObserveNthNodeReferenceID(role1.c_str(), 0, referencedNode1->GetID(), events2.GetPointer());

  if (referencingNode->GetInternalReferencedNodes()[role1][0]->GetReferencedNode() == nullptr ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->GetEvents()->GetNumberOfTuples() != 3 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->GetEvents()->GetValue(0) != 345 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->GetEvents()->GetValue(1) != 777 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->GetEvents()->GetValue(2) != 111)
    {
    std::cerr << "Line " << __LINE__ << ": SetAndObserveNodeReferenceID failed: events are incorrect" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkDMMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }

  referencingNode->LastDMMLEventId=0;
  referencedNode1->InvokeEvent(345);
  if (referencingNode->LastDMMLEventId!=345)
    {
    std::cerr << "Line " << __LINE__ << ": SetAndObserveNodeReferenceID failed: event 345 is not received after event list modified" << std::endl;
    return false;
    }

  referencingNode->LastDMMLEventId=0;
  referencedNode1->InvokeEvent(777);
  if (referencingNode->LastDMMLEventId!=777)
    {
    std::cerr << "Line " << __LINE__ << ": SetAndObserveNodeReferenceID failed: event 777 is not received after event list modified" << std::endl;
    return false;
    }

  referencingNode->LastDMMLEventId=0;
  referencedNode1->InvokeEvent(111);
  if (referencingNode->LastDMMLEventId!=111)
    {
    std::cerr << "Line " << __LINE__ << ": SetAndObserveNodeReferenceID failed: event 777 is not received after event list modified" << std::endl;
    return false;
    }

  referencingNode->LastDMMLEventId=0;
  referencedNode1->InvokeEvent(888);
  if (referencingNode->LastDMMLEventId==888)
    {
    std::cerr << "Line " << __LINE__ << ": SetAndObserveNodeReferenceID failed: event 888 is still received after event list modified" << std::endl;
    return false;
    }

  spy->ResetNumberOfEvents();

  return true;
}

//----------------------------------------------------------------------------
bool TestMultipleReferencesToSameNodeWithEvent()
{
  const char* role1="refrole1";
  const char* role2="refrole2";

  vtkNew<vtkDMMLScene> scene;

  vtkNew<vtkDMMLNodeTestHelper1> referencingNode;
  scene->AddNode(referencingNode.GetPointer());

  vtkNew<vtkDMMLNodeTestHelper1> referencedNode1;
  scene->AddNode(referencedNode1.GetPointer());

  vtkNew<vtkDMMLNodeTestHelper1> referencedNode2;
  scene->AddNode(referencedNode2.GetPointer());

  vtkNew<vtkIntArray> events;

  events->SetNumberOfTuples(0);
  events->InsertNextValue(777);
  events->InsertNextValue(888);
  referencingNode->SetAndObserveNodeReferenceID(role1, referencedNode1->GetID(), events.GetPointer());

  events->SetNumberOfTuples(0);
  events->InsertNextValue(888);
  events->InsertNextValue(999);

  TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_BEGIN(); // we may get a warning in debug mode
  referencingNode->SetAndObserveNodeReferenceID(role2, referencedNode1->GetID(), events.GetPointer());
  TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_END();

  // Test that the referencing node receives events all the requested referencedNode1 events from role 1 and 2

  referencingNode->LastDMMLEventId=0;
  referencedNode1->InvokeEvent(777);
  if (referencingNode->LastDMMLEventId!=777)
    {
    std::cerr << "Line " << __LINE__ << ": TestMultipleReferencesToSameNodeWithEvent failed: event 777 is not received" << std::endl;
    return false;
    }

  referencingNode->LastDMMLEventId=0;
  referencedNode1->InvokeEvent(888);
  if (referencingNode->LastDMMLEventId!=888)
    {
    std::cerr << "Line " << __LINE__ << ": TestMultipleReferencesToSameNodeWithEvent failed: event 888 is not received" << std::endl;
    return false;
    }

  referencingNode->LastDMMLEventId=0;
  referencedNode1->InvokeEvent(999);
  if (referencingNode->LastDMMLEventId!=999)
    {
    std::cerr << "Line " << __LINE__ << ": TestMultipleReferencesToSameNodeWithEvent failed: event 999 is not received" << std::endl;
    return false;
    }

  // Remove redefencedNode1 role1 observer, check if still receives events from role 2 but not from role 1
  // Test that the referencing node receives events all the requested referenceNode1 events from role 1 and 2

  events->SetNumberOfTuples(0);
  events->InsertNextValue(333);
  events->InsertNextValue(444);
  referencingNode->SetAndObserveNodeReferenceID(role1, referencedNode2->GetID(), events.GetPointer());

  referencingNode->LastDMMLEventId=0;
  referencedNode1->InvokeEvent(777);
  if (referencingNode->LastDMMLEventId==777)
    {
    std::cerr << "Line " << __LINE__ << ": TestMultipleReferencesToSameNodeWithEvent failed: event 777 is not received while it should not have been" << std::endl;
    return false;
    }

  referencingNode->LastDMMLEventId=0;
  referencedNode1->InvokeEvent(888);
  if (referencingNode->LastDMMLEventId!=888)
    {
    std::cerr << "Line " << __LINE__ << ": TestMultipleReferencesToSameNodeWithEvent failed: event 888 is not received" << std::endl;
    return false;
    }

  referencingNode->LastDMMLEventId=0;
  referencedNode1->InvokeEvent(999);
  if (referencingNode->LastDMMLEventId!=999)
    {
    std::cerr << "Line " << __LINE__ << ": TestMultipleReferencesToSameNodeWithEvent failed: event 999 is not received" << std::endl;
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestSingletonNodeReferencesUpdate()
{
  const char* role1="refrole1";

  vtkNew<vtkDMMLScene> scene;

  // Add a referencing/referenced singleton node pair to the scene twice
  // and verify there is no memory leaks.

  vtkNew<vtkDMMLNodeTestHelper1> referencingNode1;
  referencingNode1->SetName("referencingNode1");
  referencingNode1->SetScene(scene.GetPointer());
  vtkNew<vtkDMMLNodeTestHelper1> referencedNode1;
  referencingNode1->SetName("referencedNode1");
  scene->AddNode(referencedNode1.GetPointer());
  referencingNode1->SetAndObserveNodeReferenceID(role1, referencedNode1->GetID());
  referencingNode1->SetSingletonTag("other");
  scene->AddNode(referencingNode1.GetPointer());

  vtkNew<vtkDMMLNodeTestHelper1> referencingNode2;
  referencingNode1->SetName("referencingNode2");
  referencingNode2->SetScene(scene.GetPointer());
  vtkNew<vtkDMMLNodeTestHelper1> referencedNode2;
  referencingNode1->SetName("referencedNode2");
  scene->AddNode(referencedNode2.GetPointer());
  referencingNode2->SetAndObserveNodeReferenceID(role1, referencedNode2->GetID());
  referencingNode2->SetSingletonTag("other");
  scene->AddNode(referencingNode2.GetPointer());

  return true;
}

//----------------------------------------------------------------------------
bool TestAddReferencedNodeIDEventsWithNoScene()
{
  std::string role1("refrole1");

  // Make sure that the ReferenceAddedEvent is fired even when the
  // referenced node is observed when the referencing is not in the scene.
  vtkNew<vtkDMMLScene> scene;

  vtkNew<vtkDMMLNodeTestHelper1> referencingNode;

  vtkNew<vtkDMMLNodeTestHelper1> referencedNode;
  scene->AddNode(referencedNode.GetPointer());
  referencingNode->SetAndObserveNodeReferenceID(role1.c_str(), referencedNode->GetID());

  vtkNew<vtkDMMLNodeCallback> callback;
  referencingNode->AddObserver(vtkCommand::AnyEvent, callback.GetPointer());

  scene->AddNode(referencingNode.GetPointer());

  if (!callback->GetErrorString().empty() ||
      callback->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1 )
    {
    std::cerr << "ERROR line " << __LINE__ << ": " << std::endl
              << "vtkDMMLScene::AddNode(referencingNode) failed. "
              << callback->GetErrorString().c_str() << " "
              << "Number of ModifiedEvent: " << callback->GetNumberOfModified() << " "
              << "Number of ReferenceAddedEvent: "
              << callback->GetNumberOfEvents(vtkDMMLNode::ReferenceAddedEvent)
              << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
int TestSetNodeReferenceID()
{
  vtkNew<vtkDMMLScene> scene;

  vtkDMMLNode *returnNode = nullptr;
  int referencedNodesCount = -1;

  std::string role1("refrole1");
  std::string role2("refrole2");
  std::string role3("refrole3");

  vtkNew<vtkDMMLNodeTestHelper1> referencingNode;
  scene->AddNode(referencingNode.GetPointer());

  vtkNew<vtkDMMLNodeTestHelper1> referencedNode1;
  scene->AddNode(referencedNode1.GetPointer());

  /// Add empty referenced node with empty role
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  returnNode = referencingNode->AddNodeReferenceID(nullptr, nullptr);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  if (!CheckNodeReferences(__LINE__, "AddNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), nullptr,
                           /* n = */ 0,
                           /* expectedNodeReference = */ nullptr,
                           /* expectedNumberOfNodeReferences = */ 0,
                           /* expectedReferencedNodesCount = */ 1,
                           /* currentReturnNode = */ returnNode))
    {
    return EXIT_FAILURE;
    }

  /// Add empty referenced node with a role
  returnNode = referencingNode->AddNodeReferenceID(role1.c_str(), nullptr);
  if (!CheckNodeReferences(__LINE__, "AddNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role1.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ nullptr,
                           /* expectedNumberOfNodeReferences = */ 0,
                           /* expectedReferencedNodesCount = */ 1,
                           /* currentReturnNode = */ returnNode))
    {
    return EXIT_FAILURE;
    }

  /// Add referenced node ID
  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->AddNodeReferenceID(role1.c_str(), referencedNode1->GetID());
  if (!CheckNodeReferences(__LINE__, "AddNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role1.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ referencedNode1.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 1,
                           /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                           /* currentReturnNode = */ returnNode))
    {
    return EXIT_FAILURE;
    }

  /// Add empty referenced node ID
  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->AddNodeReferenceID(role1.c_str(), nullptr);
  if (!CheckNodeReferences(__LINE__, "AddNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role1.c_str(),
                           /* n = */ 1,
                           /* expectedNodeReference = */ nullptr,
                           /* expectedNumberOfNodeReferences = */ 1,
                           /* expectedReferencedNodesCount = */ referencedNodesCount,
                           /* currentReturnNode = */ returnNode))
    {
    return EXIT_FAILURE;
    }

  /// Change referenced node
  vtkNew<vtkDMMLNodeTestHelper1> referencedNode2;
  scene->AddNode(referencedNode2.GetPointer());

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetNodeReferenceID(role1.c_str(), referencedNode2->GetID());

  if (!CheckNodeReferences(__LINE__, "SetNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role1.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ referencedNode2.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 1,
                           /* expectedReferencedNodesCount = */ referencedNodesCount,
                           /* currentReturnNode = */ returnNode))
    {
    return EXIT_FAILURE;
    }

  /// Add referenced node
  vtkNew<vtkDMMLNodeTestHelper1> referencedNode3;
  scene->AddNode(referencedNode3.GetPointer());

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetNthNodeReferenceID(role1.c_str(), 1, referencedNode3->GetID());

  if (!CheckNodeReferences(__LINE__, "SetNthNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role1.c_str(),
                           /* n = */ 1,
                           /* expectedNodeReference = */ referencedNode3.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 2,
                           /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                           /* currentReturnNode = */ returnNode))
    {
    return EXIT_FAILURE;
    }

  // make sure it didn't change the first referenced node ID
  if (!CheckNthNodeReferenceID(__LINE__, "SetNthNodeReferenceID", referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode2.GetPointer()))
    {
    return EXIT_FAILURE;
    }

  /// Add different role
  vtkNew<vtkDMMLNodeTestHelper1> referencedNode22;
  scene->AddNode(referencedNode22.GetPointer());

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetNodeReferenceID(role2.c_str(), referencedNode22->GetID());

  if (!CheckNodeReferences(__LINE__, "SetNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role2.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ referencedNode22.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 1,
                           /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                           /* currentReturnNode = */ returnNode))
    {
    return EXIT_FAILURE;
    }

  /// Add referenced node
  vtkNew<vtkDMMLNodeTestHelper1> referencedNode23;
  scene->AddNode(referencedNode23.GetPointer());

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetNthNodeReferenceID(role2.c_str(), 1, referencedNode23->GetID());

  if (!CheckNodeReferences(__LINE__, "SetNthNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role2.c_str(),
                           /* n = */ 1,
                           /* expectedNodeReference = */ referencedNode23.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 2,
                           /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                           /* currentReturnNode = */ returnNode))
    {
    return EXIT_FAILURE;
    }

  // make sure it didn't change the first referenced node ID
  if (!CheckNthNodeReferenceID(__LINE__, "SetNthNodeReferenceID", referencingNode.GetPointer(),
                               role2.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode22->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode22.GetPointer()))
    {
    return EXIT_FAILURE;
    }

  // make sure it didn't change the first role references
  if (!CheckNthNodeReferenceID(__LINE__, "SetNthNodeReferenceID", referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 1,
                               /* expectedNodeReferenceID = */ referencedNode3->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode3.GetPointer()))
    {
    return EXIT_FAILURE;
    }
  if (!CheckNumberOfNodeReferences(__LINE__, "SetNthNodeReferenceID", role1.c_str(),
                                   referencingNode.GetPointer(),
                                   /* expectedNumberOfNodeReferences = */ 2))
    {
    return EXIT_FAILURE;
    }
  // make sure it didn't change the first referenced node ID associated with the first role
  if (!CheckNthNodeReferenceID(__LINE__, "SetNthNodeReferenceID", referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode2.GetPointer()))
    {
    return EXIT_FAILURE;
    }

  /// change reference and check that it did
  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetNthNodeReferenceID(role2.c_str(), 1, referencedNode3->GetID());

  if (!CheckNodeReferences(__LINE__, "SetNthNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role2.c_str(),
                           /* n = */ 1,
                           /* expectedNodeReference = */ referencedNode3.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 2,
                           /* expectedReferencedNodesCount = */ referencedNodesCount - 1,
                           /* currentReturnNode = */ returnNode))
    {
    return EXIT_FAILURE;
    }
  // make sure it didn't change the first referenced node ID
  if (!CheckNthNodeReferenceID(__LINE__, "SetNthNodeReferenceID", referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode2.GetPointer()))
    {
    return EXIT_FAILURE;
    }

  /// (1) set first reference, (2) set first reference to null and (3) set second reference
  vtkNew<vtkDMMLNodeTestHelper1> referencedNode31;
  scene->AddNode(referencedNode31.GetPointer());

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetNthNodeReferenceID(role3.c_str(), 0, referencedNode31->GetID());

  if (!CheckNodeReferences(__LINE__, "SetNthNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role3.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ referencedNode31.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 1,
                           /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                           /* currentReturnNode = */ returnNode))
    {
    return EXIT_FAILURE;
    }

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetNthNodeReferenceID(role3.c_str(), 0, nullptr);

  if (!CheckNodeReferences(__LINE__, "SetNthNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role3.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ nullptr,
                           /* expectedNumberOfNodeReferences = */ 0,
                           /* expectedReferencedNodesCount = */ referencedNodesCount -1,
                           /* currentReturnNode = */ returnNode))
    {
    return EXIT_FAILURE;
    }

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetNthNodeReferenceID(role3.c_str(), 1, referencedNode31->GetID());

  if (!CheckNodeReferences(__LINE__, "SetNthNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role3.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ referencedNode31.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 1,
                           /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                           /* currentReturnNode = */ returnNode))
    {
    return EXIT_FAILURE;
    }

  /// Set Nth reference to 0
  std::vector<int> referenceIndices;
  referenceIndices.push_back(20);
  referenceIndices.push_back(30);
  referenceIndices.push_back(31);
  referenceIndices.push_back(32);
  referenceIndices.push_back(21);
  referenceIndices.push_back(10);
  referenceIndices.push_back(3);
  referenceIndices.push_back(-1);
  for (std::vector<int>::iterator it = referenceIndices.begin();
       it != referenceIndices.end();
       ++it)
    {
    int nth = *it;
    referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
    TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_BEGIN(); // error is only returned if a negative index is provided and it's fine
    returnNode = referencingNode->SetNthNodeReferenceID(role3.c_str(), nth, nullptr);
    TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_END();

    if (!CheckNodeReferences(__LINE__, "SetNthNodeReferenceID", scene.GetPointer(),
                             referencingNode.GetPointer(), role3.c_str(),
                             /* n = */ nth,
                             /* expectedNodeReference = */ nullptr,
                             /* expectedNumberOfNodeReferences = */ 1,
                             /* expectedReferencedNodesCount = */ referencedNodesCount,
                             /* currentReturnNode = */ returnNode))
      {
      return EXIT_FAILURE;
      }
    }

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
bool TestSetNodeReferenceIDToZeroOrEmptyString()
{
  vtkNew<vtkDMMLScene> scene;

  vtkDMMLNode *returnNode = nullptr;
  int referencedNodesCount = -1;

  std::string role1("refrole1");

  vtkNew<vtkDMMLNodeTestHelper1> referencingNode;
  scene->AddNode(referencingNode.GetPointer());

  // The following code adds 8 referenced nodes

  std::vector< vtkWeakPointer<vtkDMMLNodeTestHelper1> > referencingNodes;

  int referencingNodeCount = 8;
  for (int idx = 0; idx < referencingNodeCount; idx++)
    {
    vtkNew<vtkDMMLNodeTestHelper1> referencingNode;
    scene->AddNode(referencingNode.GetPointer());
    referencingNodes.emplace_back(referencingNode.GetPointer());
    }

  for (int idx = 0; idx < referencingNodeCount; ++idx)
    {
    vtkDMMLNodeTestHelper1 * referencedNode = referencingNodes.at(idx);

    referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
    returnNode = referencingNode->AddNodeReferenceID(role1.c_str(), referencedNode->GetID());
    if (!CheckNodeReferences(__LINE__, "AddNodeReferenceID", scene.GetPointer(),
                             referencingNode.GetPointer(), role1.c_str(),
                             /* n = */ idx,
                             /* expectedNodeReference = */ referencedNode,
                             /* expectedNumberOfNodeReferences = */ idx + 1,
                             /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                             /* currentReturnNode = */ returnNode))
      {
      return false;
      }
    }

  int expectedReferencedNodesCount = referencedNodesCount;

  if (!CheckNumberOfNodeReferences(__LINE__, "TestSetNodeReferenceIDToZeroOrEmptyString", role1.c_str(),
                                   referencingNode.GetPointer(), expectedReferencedNodesCount))
    {
    return false;
    }

  // The code below checks that setting a reference to either 0 or an empty string removes
  // the reference from the underlying vector.

  expectedReferencedNodesCount = expectedReferencedNodesCount - 1;
  referencingNode->SetNthNodeReferenceID(role1.c_str(), 1, "");
  if (!CheckNumberOfNodeReferences(__LINE__, "TestSetNodeReferenceIDToZeroOrEmptyString", role1.c_str(),
                                   referencingNode.GetPointer(), expectedReferencedNodesCount))
    {
    return false;
    }

  expectedReferencedNodesCount = expectedReferencedNodesCount - 1;
  referencingNode->SetNthNodeReferenceID(role1.c_str(), 1, nullptr);
  if (!CheckNumberOfNodeReferences(__LINE__, "TestSetNodeReferenceIDToZeroOrEmptyString", role1.c_str(),
                                   referencingNode.GetPointer(), expectedReferencedNodesCount))
    {
    return false;
    }

  expectedReferencedNodesCount = expectedReferencedNodesCount - 1;
  referencingNode->SetAndObserveNthNodeReferenceID(role1.c_str(), 1, "");
  if (!CheckNumberOfNodeReferences(__LINE__, "TestSetNodeReferenceIDToZeroOrEmptyString", role1.c_str(),
                                   referencingNode.GetPointer(), expectedReferencedNodesCount))
    {
    return false;
    }

  expectedReferencedNodesCount = expectedReferencedNodesCount - 1;
  referencingNode->SetAndObserveNthNodeReferenceID(role1.c_str(), 1, nullptr);
  if (!CheckNumberOfNodeReferences(__LINE__, "TestSetNodeReferenceIDToZeroOrEmptyString", role1.c_str(),
                                   referencingNode.GetPointer(), expectedReferencedNodesCount))
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestNodeReferenceSerialization()
{
  std::string role1("refrole1");
  std::string role2("refrole2");

  vtkNew<vtkDMMLScene> scene;
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLNodeTestHelper1>::New());

  vtkNew<vtkDMMLNodeTestHelper1> referencingNode;
  scene->AddNode(referencingNode.GetPointer());

  vtkNew<vtkDMMLNodeTestHelper1> referencedNode11;
  scene->AddNode(referencedNode11.GetPointer());
  referencingNode->AddNodeReferenceID(role1.c_str(), referencedNode11->GetID());

  vtkNew<vtkDMMLNodeTestHelper1> referencedNode21;
  vtkNew<vtkDMMLNodeTestHelper1> referencedNode22;
  scene->AddNode(referencedNode21.GetPointer());
  scene->AddNode(referencedNode22.GetPointer());
  referencingNode->AddNodeReferenceID(role2.c_str(), referencedNode21->GetID());
  referencingNode->AddNodeReferenceID(role2.c_str(), referencedNode22->GetID());

  std::stringstream ss;

  // Write scene to XML string
  scene->SetSaveToXMLString(1);
  scene->Commit();
  std::string sceneXMLString = scene->GetSceneXMLString();

  vtkNew<vtkDMMLScene> scene2;
  scene2->RegisterNodeClass(vtkSmartPointer<vtkDMMLNodeTestHelper1>::New());
  scene2->SetLoadFromXMLString(1);
  scene2->SetSceneXMLString(sceneXMLString);
  scene2->Import();  // adds Subject Hierarchy Node

  if (!CheckInt(__LINE__,
                "Scene2-GetNumberOfNodes",
                scene2->GetNumberOfNodes(), 5))
    {
    return false;
    }

  vtkDMMLNode* referencingNodeImported =
      scene2->GetNodeByID(referencingNode->GetID());

  if (!CheckNotNull(__LINE__,
                    std::string("Scene2-GetNodeByID-") + referencingNode->GetID(),
                    referencingNodeImported))
    {
    return false;
    }

  if (!CheckInt(__LINE__,
                std::string("Scene2-referencingNodeImported-GetNumberOfNodeReferences-role:") + role1,
                referencingNodeImported->GetNumberOfNodeReferences(role1.c_str()),
                1))
    {
    return false;
    }

  if (!CheckInt(__LINE__,
                std::string("Scene2-referencingNodeImported-GetNumberOfNodeReferences-role:") + role2,
                referencingNodeImported->GetNumberOfNodeReferences(role2.c_str()),
                2))
    {
    return false;
    }

  if (!CheckNotNull(__LINE__,
                    std::string("Scene2-referencingNodeImported-GetNthNodeReferenceID-n:0-role:") + role1,
                    referencingNodeImported->GetNthNodeReferenceID(role1.c_str(), 0)))
    {
    return false;
    }

  if(!CheckString(
       __LINE__,
       std::string("Scene2-referencingNodeImported-GetNthNodeReferenceID-n:0-role:") + role1,
       referencingNodeImported->GetNthNodeReferenceID(role1.c_str(), 0),
       referencedNode11->GetID()))
    {
    return false;
    }

  if (!CheckNotNull(__LINE__,
                    std::string("Scene2-referencingNodeImported-GetNthNodeReferenceID-n:0-role:") + role2,
                    referencingNodeImported->GetNthNodeReferenceID(role2.c_str(), 0)))
    {
    return false;
    }

  if(!CheckString(
       __LINE__,
       std::string("Scene2-referencingNodeImported-GetNthNodeReferenceID-n:0-role:") + role2,
       referencingNodeImported->GetNthNodeReferenceID(role2.c_str(), 0),
       referencedNode21->GetID()))
    {
    return false;
    }

  if (!CheckNotNull(__LINE__,
                    std::string("Scene2-referencingNodeImported-GetNthNodeReferenceID-n:1-role:") + role2,
                    referencingNodeImported->GetNthNodeReferenceID(role2.c_str(), 1)))
    {
    return false;
    }

  if(!CheckString(
       __LINE__,
       std::string("Scene2-referencingNodeImported-GetNthNodeReferenceID-n:1-role:") + role2,
       referencingNodeImported->GetNthNodeReferenceID(role2.c_str(), 1),
       referencedNode22->GetID()))
    {
    return false;
    }
  return true;
}

namespace
{

//----------------------------------------------------------------------------
bool TestClearScene_CheckNumberOfEvents(const char* description,
                                        int removeSingleton,
                                        bool referencingNodeIsSingleton,
                                        bool referencedNodeIsSingleton,
                                        int expectedTotalNumberOfEventsForReferencingNode,
                                        int expectedNumberOfReferenceRemovedEventsForReferencingNode,
                                        int expectedTotalNumberOfEventsForReferencedNode,
                                        int expectedNumberOfReferenceRemovedEventsForReferencedNode)
{
  vtkNew<vtkDMMLScene> scene;
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLNodeTestHelper1>::New());

  std::string role1("refrole1");

  vtkNew<vtkDMMLNodeTestHelper1> referencingNode;
  if (referencingNodeIsSingleton)
    {
    std::string tag("ReferencingNodeSingleton-");
    tag += description;
    referencingNode->SetSingletonTag(tag.c_str());
    }
  scene->AddNode(referencingNode.GetPointer());

  vtkNew<vtkDMMLNodeTestHelper1> referencedNode;
  if (referencedNodeIsSingleton)
    {
    std::string tag("ReferencedNodeSingleton-");
    tag += description;
    referencedNode->SetSingletonTag(tag.c_str());
    }
  scene->AddNode(referencedNode.GetPointer());
  referencingNode->AddNodeReferenceID(role1.c_str(), referencedNode->GetID());

  vtkNew<vtkDMMLNodeCallback> referencingNodeSpy;
  vtkNew<vtkDMMLNodeCallback> referencedNodeSpy;

  referencingNode->AddObserver(vtkCommand::AnyEvent, referencingNodeSpy.GetPointer());
  referencedNode->AddObserver(vtkCommand::AnyEvent, referencedNodeSpy.GetPointer());

  if (!CheckInt(__LINE__,
                std::string("TestClearScene_AddNodes-TotalNumberOfEvents-ReferencingNode_") + description,
                referencingNodeSpy->GetTotalNumberOfEvents(), 0))
    {
    referencingNodeSpy->Print(std::cerr);
    return false;
    }

  if (!CheckInt(__LINE__,
                std::string("TestClearScene_AddNodes-TotalNumberOfEvents-ReferencedNode_") + description,
                referencedNodeSpy->GetTotalNumberOfEvents(), 0))
    {
    referencedNodeSpy->Print(std::cerr);
    return false;
    }

  scene->Clear(removeSingleton);

  // ReferencingNode

  if (!CheckInt(__LINE__,
                std::string("TestClearScene-TotalNumberOfEvents-for-ReferencingNode_") + description,
                referencingNodeSpy->GetTotalNumberOfEvents(),
                expectedTotalNumberOfEventsForReferencingNode))
    {
    referencingNodeSpy->Print(std::cerr);
    return false;
    }

  if (!CheckInt(__LINE__,
                std::string("TestClearScene-NumberOfReferenceRemovedEvents-for-ReferencingNode_") + description,
                referencingNodeSpy->GetNumberOfEvents(vtkDMMLNode::ReferenceRemovedEvent),
                expectedNumberOfReferenceRemovedEventsForReferencingNode))
    {
    referencingNodeSpy->Print(std::cerr);
    return false;
    }

  if (!CheckInt(__LINE__,

  // ReferencedNode

                std::string("TestClearScene-TotalNumberOfEvents-for-ReferencedNode_") + description,
                referencedNodeSpy->GetTotalNumberOfEvents(),
                expectedTotalNumberOfEventsForReferencedNode))
    {
    referencedNodeSpy->Print(std::cerr);
    return false;
    }

  if (!CheckInt(__LINE__,
                std::string("TestClearScene-NumberOfReferenceRemovedEvents-for-ReferencedNode_") + description,
                referencedNodeSpy->GetNumberOfEvents(vtkDMMLNode::ReferenceRemovedEvent),
                expectedNumberOfReferenceRemovedEventsForReferencedNode))
    {
    referencedNodeSpy->Print(std::cerr);
    return false;
    }

  referencingNodeSpy->ResetNumberOfEvents();
  referencedNodeSpy->ResetNumberOfEvents();

  return true;
}

} // end of anonymous namespace

//----------------------------------------------------------------------------
bool TestClearScene()
{

  // removeSingleton OFF
  int removeSingleton = 0;

  // "referencingNode" references "referencedNode"
  if (!TestClearScene_CheckNumberOfEvents(
        "[referencingNode-referencedNode]",
        /* removeSingleton = */ removeSingleton,
        /* referencingNodeIsSingleton = */ false,
        /* referencedNodeIsSingleton = */ false,
        /* expectedTotalNumberOfEventsForReferencingNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencingNode= */ 0,
        /* expectedTotalNumberOfEventsForReferencedNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencedNode= */ 0
        ))
    {
    return false;
    }

  // "referencingNode" references "singletonReferencedNode"
  if (!TestClearScene_CheckNumberOfEvents(
        "[referencingNode-singletonReferencedNode]",
        /* removeSingleton = */ removeSingleton,
        /* referencingNodeIsSingleton = */ false,
        /* referencedNodeIsSingleton = */ true,
        /* expectedTotalNumberOfEventsForReferencingNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencingNode= */ 0,
        /* expectedTotalNumberOfEventsForReferencedNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencedNode= */ 0
        ))
    {
    return false;
    }


  // "singletonReferencingNode" references "referencedNode"
  if (!TestClearScene_CheckNumberOfEvents(
        "[singletonReferencingNode-referencedNode]",
        /* removeSingleton = */ removeSingleton,
        /* referencingNodeIsSingleton = */ true,
        /* referencedNodeIsSingleton = */ false,
        /* expectedTotalNumberOfEventsForReferencingNode= */ 2,
        /* expectedNumberOfReferenceRemovedEventsForReferencingNode= */ 1,
        /* expectedTotalNumberOfEventsForReferencedNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencedNode= */ 0
        ))
    {
    return false;
    }

  // "singletonReferencingNode" references "singletonReferencedNode"
  if (!TestClearScene_CheckNumberOfEvents(
        "[singletonReferencingNode-singletonReferencedNode]",
        /* removeSingleton = */ removeSingleton,
        /* referencingNodeIsSingleton = */ true,
        /* referencedNodeIsSingleton = */ true,
        /* expectedTotalNumberOfEventsForReferencingNode= */ 2,
        /* expectedNumberOfReferenceRemovedEventsForReferencingNode= */ 1,
        /* expectedTotalNumberOfEventsForReferencedNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencedNode= */ 0
        ))
    {
    return false;
    }

  // removeSingleton ON
  removeSingleton = 1;

  // "referencingNode" references "referencedNode"
  if (!TestClearScene_CheckNumberOfEvents(
        "[referencingNode-referencedNode]",
        /* removeSingleton = */ removeSingleton,
        /* referencingNodeIsSingleton = */ false,
        /* referencedNodeIsSingleton = */ false,
        /* expectedTotalNumberOfEventsForReferencingNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencingNode= */ 0,
        /* expectedTotalNumberOfEventsForReferencedNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencedNode= */ 0
        ))
    {
    return false;
    }

  // "referencingNode" references "singletonReferencedNode"
  if (!TestClearScene_CheckNumberOfEvents(
        "[referencingNode-singletonReferencedNode]",
        /* removeSingleton = */ removeSingleton,
        /* referencingNodeIsSingleton = */ false,
        /* referencedNodeIsSingleton = */ true,
        /* expectedTotalNumberOfEventsForReferencingNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencingNode= */ 0,
        /* expectedTotalNumberOfEventsForReferencedNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencedNode= */ 0
        ))
    {
    return false;
    }


  // "singletonReferencingNode" references "referencedNode"
  if (!TestClearScene_CheckNumberOfEvents(
        "[singletonReferencingNode-referencedNode]",
        /* removeSingleton = */ removeSingleton,
        /* referencingNodeIsSingleton = */ true,
        /* referencedNodeIsSingleton = */ false,
        /* expectedTotalNumberOfEventsForReferencingNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencingNode= */ 0,
        /* expectedTotalNumberOfEventsForReferencedNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencedNode= */ 0
        ))
    {
    return false;
    }

  // "singletonReferencingNode" references "singletonReferencedNode"
  if (!TestClearScene_CheckNumberOfEvents(
        "[singletonReferencingNode-singletonReferencedNode]",
        /* removeSingleton = */ removeSingleton,
        /* referencingNodeIsSingleton = */ true,
        /* referencedNodeIsSingleton = */ true,
        /* expectedTotalNumberOfEventsForReferencingNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencingNode= */ 0,
        /* expectedTotalNumberOfEventsForReferencedNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencedNode= */ 0
        ))
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
int TestReadWriteXMLProperties()
{
  vtkNew<vtkDMMLNodeTestHelper1> node1;

  std::vector<std::string> testingStringsVector;
  testingStringsVector.emplace_back("Several;Semicolon;In;One;String");
  testingStringsVector.emplace_back("Percent%Sign");
  testingStringsVector.emplace_back("%25%3B;%");
  node1->SetTestingStringVector(testingStringsVector);

  std::vector<float> testingFloatVector;
  testingFloatVector.push_back(0.0);
  testingFloatVector.push_back(1.1);
  testingFloatVector.push_back(2.2);
  testingFloatVector.push_back(3.3);
  testingFloatVector.push_back(4.4);
  testingFloatVector.push_back(5.5);
  node1->SetTestingFloatVector(testingFloatVector);

  std::vector<int> testingIntVector;
  testingIntVector.push_back(0);
  testingIntVector.push_back(1);
  testingIntVector.push_back(2);
  testingIntVector.push_back(3);
  testingIntVector.push_back(4);
  testingIntVector.push_back(5);
  node1->SetTestingIntVector(testingIntVector);

  std::stringstream ss;
  ss << "<DMMLNode ";
  node1->WriteXML(ss, 0);
  ss << " />";
  std::cout << "Scene XML: " << ss.str() << std::endl;

  vtkSmartPointer<vtkXMLDataElement> element = vtkSmartPointer<vtkXMLDataElement>::Take(vtkXMLUtilities::ReadElementFromStream(ss));

  const char *atts[] = {
    "TestingStringVector", element->GetAttribute("TestingStringVector"),
    "TestingFloatVector", element->GetAttribute("TestingFloatVector"),
    "TestingIntVector", element->GetAttribute("TestingIntVector"),
    nullptr };

  vtkNew<vtkDMMLNodeTestHelper1> node2;
  node2->ReadXMLAttributes(atts);

  if (node1->GetTestingStringVector() != node2->GetTestingStringVector())
    {
    std::cout << "Node 1 string vector:" << std::endl;
    for (auto s : node1->GetTestingStringVector())
      {
      std::cout << "  " << s << std::endl;
      }
    std::cout << "Node 2 string vector:" << std::endl;
    for (auto s : node2->GetTestingStringVector())
      {
      std::cout << "  " << s << std::endl;
      }
    }

  if (node1->GetTestingFloatVector() != node2->GetTestingFloatVector())
    {
    return EXIT_FAILURE;
    }

  if (node1->GetTestingIntVector() != node2->GetTestingIntVector())
    {
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

namespace
{

//----------------------------------------------------------------------------
class vtkDMMLTestScene : public vtkDMMLScene
{
public:
  static vtkDMMLTestScene *New();
  typedef vtkDMMLTestScene Self;

  vtkTypeMacro(vtkDMMLTestScene, vtkDMMLScene);

  typedef NodeReferencesType TestNodeReferencesType;
  TestNodeReferencesType test_NodeReferences()
  {
    return this->NodeReferences;
  }

protected:
  vtkDMMLTestScene() = default;
};
vtkStandardNewMacro(vtkDMMLTestScene);

//----------------------------------------------------------------------------
#if DISPLAY_SCENE_NODE_REFERENCES
void DisplaySceneNodeReferences(
    int line, vtkDMMLTestScene::TestNodeReferencesType nodeReferences)
{
  vtkDMMLTestScene::TestNodeReferencesType::iterator referenceIt;
  vtkDMMLTestScene::TestNodeReferencesType::value_type::second_type::iterator referringNodesIt;

  std::cout << "\nLine " << line << " - Scene NodeReferences:" << std::endl;

  for (referenceIt = nodeReferences.begin(); referenceIt != nodeReferences.end(); ++referenceIt)
    {
    std::cout << "  " << referenceIt->first << " [";
    for (referringNodesIt = referenceIt->second.begin(); referringNodesIt != referenceIt->second.end(); ++referringNodesIt)
      {
      std::cout << *referringNodesIt << ", ";
      }
    std::cout << "]" << std::endl;
    }
}
#endif
}

//----------------------------------------------------------------------------
bool TestImportSceneReferenceValidDuringImport()
{
  //
  // Create scene and register node
  //

  std::string role1("refrole1");

  vtkNew<vtkDMMLTestScene> scene;
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLNodeTestHelper1>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLStorageNodeTestHelper>::New());

  //
  // Add nodes
  //

  vtkNew<vtkDMMLNodeTestHelper1> node1;
  scene->AddNode(node1.GetPointer()); // ID: vtkDMMLNodeTestHelper11

  vtkNew<vtkDMMLNodeTestHelper1> node2;
  scene->AddNode(node2.GetPointer()); // ID: vtkDMMLNodeTestHelper12
  node1->AddNodeReferenceID(role1.c_str(), node2->GetID());

  vtkNew<vtkDMMLNodeTestHelper1> node3;
  scene->AddNode(node3.GetPointer()); // ID: vtkDMMLNodeTestHelper13
  node2->SetOtherNodeID(node3->GetID());

  vtkNew<vtkDMMLStorageNodeTestHelper> node4;
  node4->SetOtherNodeID(node2->GetID());
  node4->AddNodeReferenceID(role1.c_str(), node1->GetID());
  scene->AddNode(node4.GetPointer()); // ID: vtkDMMLStorageNodeTestHelper1

  // At this point the scene should be:
  //
  //  Scene
  //    |---- vtkDMMLNodeTestHelper11
  //    |          |-- ref [refrole1] to vtkDMMLNodeTestHelper12
  //    |
  //    |---- vtkDMMLNodeTestHelper12
  //    |          |-- ref [otherNode] to vtkDMMLNodeTestHelper13
  //    |
  //    |---- vtkDMMLNodeTestHelper13
  //    |
  //    |---- vtkDMMLStorageNodeTestHelper1
  //    |          |-- ref [otherNode] to vtkDMMLNodeTestHelper12
  //    |          |-- ref [refrole1] to vtkDMMLNodeTestHelper11

#if DISPLAY_SCENE_NODE_REFERENCES
  DisplaySceneNodeReferences(__LINE__, scene->test_NodeReferences());
#endif

  if (!CheckInt(
        __LINE__, "GetNumberOfNodes",
        scene->GetNumberOfNodes(), 4)

      ||!CheckString(
        __LINE__, "node1->GetNodeReferenceID(\"refrole1\")",
        node1->GetNodeReferenceID("refrole1"), "vtkDMMLNodeTestHelper12")

      ||!CheckPointer(
        __LINE__, "node1->GetNodeReference(\"refrole1\")",
        node1->GetNodeReference("refrole1"), node2.GetPointer())

      ||!CheckString(
        __LINE__, "node2->GetOtherNodeID()",
        node2->GetOtherNodeID(), "vtkDMMLNodeTestHelper13")

      ||!CheckString(
        __LINE__, "node4->GetOtherNodeID()",
        node4->GetOtherNodeID(), "vtkDMMLNodeTestHelper12")

      ||!CheckString(
        __LINE__, "node4->GetNodeReferenceID(\"refrole1\")",
        node4->GetNodeReferenceID("refrole1"), "vtkDMMLNodeTestHelper11")

      ||!CheckPointer(
        __LINE__, "node4->GetNodeReference(\"refrole1\")",
        node4->GetNodeReference("refrole1"), node1.GetPointer())
      )
    {
    return false;
    }

  //
  // Write scene to XML string for importing later
  //
  scene->SetSaveToXMLString(1);
  scene->Commit();
  std::string sceneXMLString = scene->GetSceneXMLString();

//  std::cerr << sceneXMLString << std::endl;

  //
  // Add few more nodes and references
  //

  vtkNew<vtkDMMLNodeTestHelper1> node5;
  node5->AddNodeReferenceID(role1.c_str(), node3->GetID());
  scene->AddNode(node5.GetPointer()); // ID: vtkDMMLNodeTestHelper15

  vtkNew<vtkDMMLNodeTestHelper1> node6;
  scene->AddNode(node6.GetPointer()); // ID: vtkDMMLNodeTestHelper16
  node6->AddNodeReferenceID(role1.c_str(), node5->GetID());

  // At this point the scene should be:
  //
  //  Scene
  //    |---- vtkDMMLNodeTestHelper11
  //    |          |-- ref [refrole1] to vtkDMMLNodeTestHelper12
  //    |
  //    |---- vtkDMMLNodeTestHelper12
  //    |          |-- ref [otherNode] to vtkDMMLNodeTestHelper13
  //    |
  //    |---- vtkDMMLNodeTestHelper13
  //    |
  //    |---- vtkDMMLStorageNodeTestHelper1
  //    |          |-- ref [otherNode] to vtkDMMLNodeTestHelper12
  //    |          |-- ref [refrole1] to vtkDMMLNodeTestHelper11
  //    |
  //    |---- vtkDMMLNodeTestHelper14
  //    |          |-- ref [refrole1] to vtkDMMLNodeTestHelper13
  //    |
  //    |---- vtkDMMLNodeTestHelper15
  //               |-- ref [refrole1] to vtkDMMLNodeTestHelper14

#if DISPLAY_SCENE_NODE_REFERENCES
  DisplaySceneNodeReferences(__LINE__, scene->test_NodeReferences());
#endif

  if (!CheckInt(
        __LINE__, "GetNumberOfNodes",
        scene->GetNumberOfNodes(), 6)


      ||!CheckString(
        __LINE__, "node1->GetNodeReferenceID(\"refrole1\")",
        node1->GetNodeReferenceID("refrole1"), "vtkDMMLNodeTestHelper12")

      ||!CheckPointer(
        __LINE__, "node1->GetNodeReference(\"refrole1\")",
        node1->GetNodeReference("refrole1"), node2.GetPointer())

      ||!CheckString(
        __LINE__, "node2->GetOtherNodeID()",
        node2->GetOtherNodeID(), "vtkDMMLNodeTestHelper13")

      ||!CheckString(
        __LINE__, "node4->GetOtherNodeID()",
        node4->GetOtherNodeID(), "vtkDMMLNodeTestHelper12")

      ||!CheckString(
        __LINE__, "node4->GetNodeReferenceID(\"refrole1\")",
        node4->GetNodeReferenceID("refrole1"), "vtkDMMLNodeTestHelper11")

      ||!CheckPointer(
        __LINE__, "node4->GetNodeReference(\"refrole1\")",
        node4->GetNodeReference("refrole1"), node1.GetPointer())


      ||!CheckString(
        __LINE__, "node5->GetNodeReferenceID(\"refrole1\")",
        node5->GetNodeReferenceID("refrole1"), "vtkDMMLNodeTestHelper13")

      ||!CheckPointer(
        __LINE__, "node5->GetNodeReference(\"refrole1\")",
        node5->GetNodeReference("refrole1"), node3.GetPointer())

      ||!CheckString(
        __LINE__, "node6->GetNodeReferenceID(\"refrole1\")",
        node6->GetNodeReferenceID("refrole1"), "vtkDMMLNodeTestHelper14")

      ||!CheckPointer(
        __LINE__, "node6->GetNodeReference(\"refrole1\")",
        node6->GetNodeReference("refrole1"), node5.GetPointer())

      )
    {
    return false;
    }


  //
  // Import saved scene into existing one
  //

  scene->SetLoadFromXMLString(1);
  scene->SetSceneXMLString(sceneXMLString);
  scene->Import();

  // At this point the scene should be:
  //
  //  Scene
  //    |---- vtkDMMLNodeTestHelper11
  //    |          |-- ref [refrole1] to vtkDMMLNodeTestHelper12
  //    |
  //    |---- vtkDMMLNodeTestHelper12
  //    |          |-- ref [otherNode] to vtkDMMLNodeTestHelper13
  //    |
  //    |---- vtkDMMLNodeTestHelper13
  //    |
  //    |---- vtkDMMLStorageNodeTestHelper1
  //    |          |-- ref [otherNode] to vtkDMMLNodeTestHelper12
  //    |          |-- ref [refrole1] to vtkDMMLNodeTestHelper11
  //    |
  //    |---- vtkDMMLNodeTestHelper14
  //    |          |-- ref [refrole1] to vtkDMMLNodeTestHelper13
  //    |
  //    |---- vtkDMMLNodeTestHelper15
  //    |          |-- ref [refrole1] to vtkDMMLNodeTestHelper14
  //    |
  //    |---- vtkDMMLNodeTestHelper16                             [was vtkDMMLNodeTestHelper11]
  //    |          |-- ref [refrole1] to vtkDMMLNodeTestHelper17
  //    |
  //    |---- vtkDMMLNodeTestHelper17                             [was vtkDMMLNodeTestHelper12]
  //    |          |-- ref [otherNode] to vtkDMMLNodeTestHelper18
  //    |
  //    |---- vtkDMMLNodeTestHelper18                             [was vtkDMMLNodeTestHelper13]
  //    |
  //    |---- vtkDMMLStorageNodeTestHelper2                       [was vtkDMMLStorageNodeTestHelper1]
  //    |          |-- ref [otherNode] to vtkDMMLNodeTestHelper17
  //    |          |-- ref [refrole1] to vtkDMMLNodeTestHelper16

#if DISPLAY_SCENE_NODE_REFERENCES
  DisplaySceneNodeReferences(__LINE__, scene->test_NodeReferences());
#endif

  vtkDMMLNodeTestHelper1 *node7 =
      vtkDMMLNodeTestHelper1::SafeDownCast(scene->GetNodeByID("vtkDMMLNodeTestHelper16"));

  vtkDMMLNodeTestHelper1 *node8 =
      vtkDMMLNodeTestHelper1::SafeDownCast(scene->GetNodeByID("vtkDMMLNodeTestHelper17"));

  vtkDMMLStorageNodeTestHelper *node10 =
      vtkDMMLStorageNodeTestHelper::SafeDownCast(scene->GetNodeByID("vtkDMMLStorageNodeTestHelper2"));

  //
  // Check scene contains original and imported nodes
  //

  if (!CheckInt(
        __LINE__, "GetNumberOfNodes",
        scene->GetNumberOfNodes(), 10)

      ||!CheckString(
        __LINE__, "node1->GetNodeReferenceID(\"refrole1\")",
        node1->GetNodeReferenceID("refrole1"), "vtkDMMLNodeTestHelper12")

      ||!CheckPointer(
        __LINE__, "node1->GetNodeReference(\"refrole1\")",
        node1->GetNodeReference("refrole1"), node2.GetPointer())

      ||!CheckString(
        __LINE__, "node2->GetOtherNodeID()",
        node2->GetOtherNodeID(), "vtkDMMLNodeTestHelper13")

      ||!CheckString(
        __LINE__, "node4->GetOtherNodeID()",
        node4->GetOtherNodeID(), "vtkDMMLNodeTestHelper12")

      ||!CheckString(
        __LINE__, "node4->GetNodeReferenceID(\"refrole1\")",
        node4->GetNodeReferenceID("refrole1"), "vtkDMMLNodeTestHelper11")

      ||!CheckPointer(
        __LINE__, "node4->GetNodeReference(\"refrole1\")",
        node4->GetNodeReference("refrole1"), node1.GetPointer())


      ||!CheckString(
        __LINE__, "node5->GetNodeReferenceID(\"refrole1\")",
        node5->GetNodeReferenceID("refrole1"), "vtkDMMLNodeTestHelper13")

      ||!CheckPointer(
        __LINE__, "node5->GetNodeReference(\"refrole1\")",
        node5->GetNodeReference("refrole1"), node3.GetPointer())

      ||!CheckString(
        __LINE__, "node6->GetNodeReferenceID(\"refrole1\")",
        node6->GetNodeReferenceID("refrole1"), "vtkDMMLNodeTestHelper14")

      ||!CheckPointer(
        __LINE__, "node6->GetNodeReference(\"refrole1\")",
        node6->GetNodeReference("refrole1"), node5.GetPointer())
      )
    {
    return false;
    }

  if (!CheckString(
        __LINE__, "node7->GetNodeReferenceID(\"refrole1\")",
        node7->GetNodeReferenceID("refrole1"), "vtkDMMLNodeTestHelper17")

      ||!CheckPointer(
        __LINE__, "node7->GetNodeReference(\"refrole1\")",
        node7->GetNodeReference("refrole1"), node8)

      ||!CheckString(
        __LINE__, "node8->GetOtherNodeID()",
        node8->GetOtherNodeID(), "vtkDMMLNodeTestHelper18")

      ||!CheckString(
        __LINE__, "node10->GetOtherNodeID()",
        node10->GetOtherNodeID(), "vtkDMMLNodeTestHelper17")

      ||!CheckString(
        __LINE__, "node10->GetNodeReferenceID(\"refrole1\")",
        node10->GetNodeReferenceID("refrole1"), "vtkDMMLNodeTestHelper16")

      ||!CheckPointer(
        __LINE__, "node10->GetNodeReference(\"refrole1\")",
        node10->GetNodeReference("refrole1"), node7)
      )
    {
    return false;
    }

  return true;
}

//---------------------------------------------------------------------------
int TestSaveLoadSpecialCharacters()
{
  vtkNew<vtkDMMLScene> scene;

  const std::string special1 = "Test that special characters can be stored -- ampersand : (&) quote : (\") apostrophe: (') less: (<) greater: (>)";
  const std::string special2 = "Test that content is not double-decoded -- &amp; &quot; &apos; &lt; &gt; &nbsp;";
  const std::string special3 = "Test attribute percentage encoding -- % %3B %25 %% something";

  vtkNew<vtkDMMLModelNode> node;
  scene->AddNode(node.GetPointer());
  node->SetName(special1.c_str());
  node->SetDescription(special2.c_str());
  node->SetAttribute("special1Att", special1.c_str());
  node->SetAttribute("special2Att", special2.c_str());
  node->SetAttribute("special3Att", special3.c_str());

  std::stringstream ss;

  // Write scene to XML string
  scene->SetSaveToXMLString(1);
  CHECK_BOOL(scene->Commit() != 0, true);
  std::string sceneXMLString = scene->GetSceneXMLString();

  std::cout << sceneXMLString << std::endl;

  vtkNew<vtkDMMLScene> sceneReloaded;
  sceneReloaded->SetLoadFromXMLString(1);
  sceneReloaded->SetSceneXMLString(sceneXMLString);
  CHECK_BOOL(sceneReloaded->Import() != 0, true);

  vtkDMMLNode* nodeReloaded = sceneReloaded->GetFirstNodeByClass("vtkDMMLModelNode");
  CHECK_NOT_NULL(nodeReloaded);

  CHECK_STRING(node->GetName(), nodeReloaded->GetName());
  CHECK_STRING(node->GetDescription(), nodeReloaded->GetDescription());
  CHECK_STRING(node->GetAttribute("special1Att"), nodeReloaded->GetAttribute("special1Att"));
  CHECK_STRING(node->GetAttribute("special2Att"), nodeReloaded->GetAttribute("special2Att"));
  CHECK_STRING(node->GetAttribute("special3Att"), nodeReloaded->GetAttribute("special3Att"));

  return EXIT_SUCCESS;
}
