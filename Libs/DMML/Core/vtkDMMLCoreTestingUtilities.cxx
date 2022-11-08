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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 1U24CA194354-01

==============================================================================*/

// DMML includes
#include "vtkDMMLCoreTestingUtilities.h"
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLDisplayableNode.h"
#include "vtkDMMLDisplayNode.h"
#include "vtkDMMLMessageCollection.h"
#include "vtkDMMLNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSliceNode.h"
#include "vtkDMMLStorableNode.h"
#include "vtkDMMLStorageNode.h"
#include "vtkDMMLTransformableNode.h"
#include "vtkDMMLTransformNode.h"

// vtkAddon includes
#include <vtkAddonTestingUtilities.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkGeneralTransform.h>
#include <vtkStringArray.h>
#include <vtkTestErrorObserver.h>
#include <vtkURIHandler.h>
#include <vtkXMLDataParser.h>

namespace vtkDMMLCoreTestingUtilities
{

//----------------------------------------------------------------------------
bool CheckNodeInSceneByID(int line, vtkDMMLScene* scene,
                          const char* nodeID, vtkDMMLNode* expected)
{
  std::string testName = "CheckNodeInSceneByID";

  if (!scene)
    {
    std::cerr << "\nLine " << line << " - scene is NULL"
              << " : " << testName << " failed" << std::endl;
    return false;
    }
  if (!nodeID)
    {
    std::cerr << "\nLine " << line << " - nodeID is NULL"
              << " : " << testName << " failed" << std::endl;
    return false;
    }
  if (nodeID[0] == '\0')
    {
    std::cerr << "\nLine " << line << " - nodeID is an empty string"
              << " : " << testName << " failed" << std::endl;
    return false;
    }
  if (!expected)
    {
    std::cerr << "\nLine " << line << " - expected node is NULL"
              << " : " << testName << " failed" << std::endl;
    return false;
    }
  vtkDMMLNode* current = scene->GetNodeByID(nodeID);
  if (current != expected)
    {
    const char* currentID = (current ? current->GetID() : nullptr);
    const char* expectedID = (expected ? expected->GetID() : nullptr);
    std::cerr << "\nLine " << line << " - GetNodeByID(\"" << nodeID << "\")"
              << " : " << testName << " failed"

              << "\n\tcurrent :" << current
              << ", ID: " << (currentID ? currentID : "(null)")

              << "\n\texpected:" << expected
              << ", ID: " << (expectedID ? expectedID : "(null)")
              << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool CheckNodeIdAndName(int line, vtkDMMLNode* node,
                        const char* expectedID, const char* expectedName)
{
  std::string testName = "CheckNodeIdAndName";
  if (!node)
    {
    std::cerr << "\nLine " << line << " - node is NULL"
              << " : " << testName << " failed" << std::endl;
    return false;
    }
  if (!vtkAddonTestingUtilities::CheckString(
        line, std::string(testName) + ": Unexpected nodeID",
        node->GetID(), expectedID)

      ||!vtkAddonTestingUtilities::CheckString(
        line, std::string(testName) + ": Unexpected nodeName",
        node->GetName(), expectedName))
    {
    return false;
    }
  return true;
}

// ----------------------------------------------------------------------------
int GetExpectedNodeAddedClassNames(const char * sceneFilePath, std::vector<std::string>& expectedNodeAddedClassNames)
{
  vtkNew<vtkDMMLScene> scene;
  vtkNew<vtkXMLDataParser> xmlParser;
  xmlParser->SetFileName(sceneFilePath);
  CHECK_BOOL(xmlParser->Parse() !=0 , true);
  int expectedNumberOfNode = xmlParser->GetRootElement()->GetNumberOfNestedElements();
  CHECK_BOOL(expectedNumberOfNode > 0, true);

  // Loop though all exepcted node and populate expectedNodeAddedClassNames vector
  // Note that node that can't be instantiated using CreateNodeByClass are not expected
  for(int i=0; i < xmlParser->GetRootElement()->GetNumberOfNestedElements(); ++i)
    {
    std::string className = "vtkDMML";
    className += xmlParser->GetRootElement()->GetNestedElement(i)->GetName();
    // Append 'Node' prefix only if required
    if (className.find("Node") != className.size() - 4)
      {
      className += "Node";
      }
    vtkSmartPointer<vtkDMMLNode> nodeSmartPointer;
    nodeSmartPointer.TakeReference(scene->CreateNodeByClass(className.c_str()));
    if (!nodeSmartPointer)
      {
      std::cout << "className:" << className << std::endl;
      --expectedNumberOfNode;
      }
    else
      {
      expectedNodeAddedClassNames.push_back(className);
      }
    }
  CHECK_BOOL(expectedNumberOfNode == static_cast<int>(expectedNodeAddedClassNames.size()), true);
  return EXIT_SUCCESS;
}

// ----------------------------------------------------------------------------
int ExerciseBasicObjectMethods(vtkObject* object)
{
  CHECK_NOT_NULL(object);
  object->Print( std::cout );
  std::cout << "Name of Class = " << object->GetClassName() << std::endl;
  std::cout << "Name of Superclass = " << object->Superclass::GetClassName() << std::endl;
  return EXIT_SUCCESS;
}

// ----------------------------------------------------------------------------
int ExerciseBasicDMMLMethods(vtkDMMLNode* node)
{
  CHECK_EXIT_SUCCESS(ExerciseBasicObjectMethods(node));

  //  Test CreateNodeInstance()
  vtkDMMLNode * newNode = node->CreateNodeInstance();
  CHECK_NOT_NULL(newNode);
  newNode->Delete();

  //  Test UpdateScene()
  node->UpdateScene(nullptr);

  //  Test New()
  vtkSmartPointer < vtkDMMLNode > node1 = vtkSmartPointer < vtkDMMLNode >::Take(node->CreateNodeInstance());

  //  Test GetID()
  CHECK_NULL(node1->GetID());

  //  Test GetNodeTagName()
  CHECK_NOT_NULL(node1->GetNodeTagName());
  CHECK_STRING_DIFFERENT(node1->GetNodeTagName(), "");

  //  Test Copy()
  node1->Copy(node);
  node->Reset(nullptr);

  //  Test SetAttribute() / GetAttribute()
  int mod = node->StartModify();
  std::string attributeName = std::string("attName");
  std::string attributeValue = std::string("attValue");
  node->SetAttribute(attributeName.c_str(), attributeValue.c_str() );
  CHECK_STRING(node->GetAttribute( attributeName.c_str() ), attributeValue.c_str());
  node->EndModify(mod);

  //  Test getters
  TEST_SET_GET_BOOLEAN( node, HideFromEditors );
  TEST_SET_GET_BOOLEAN( node, Selectable );
  TEST_SET_GET_STRING( node, Description );
  TEST_SET_GET_STRING( node, Name );
  TEST_SET_GET_STRING( node, SingletonTag );
  TEST_SET_GET_BOOLEAN( node, SaveWithScene );
  TEST_SET_GET_BOOLEAN( node, AddToScene );
  TEST_SET_GET_BOOLEAN( node, DisableModifiedEvent);
  TEST_SET_GET_BOOLEAN( node, Selected );

  //  Test singleton utility methods
  node->SetSingletonOff();
  CHECK_BOOL(node->IsSingleton(), false);
  node->SetSingletonOn();
  CHECK_BOOL(node->IsSingleton(), true);

  node->Modified();
  node->InvokePendingModifiedEvent();
  node1->SetName("copywithscene");
  node->CopyWithScene(node1);

  //  Test UpdateReferences()
  node->UpdateReferences();
  node->UpdateReferenceID("oldID", "newID");

  //  Test URLEncodeString()
  CHECK_STRING(node1->URLEncodeString("Thou Shall Test !"), "Thou%20Shall%20Test%20!");
  CHECK_STRING(node1->URLDecodeString("Thou%20Shall%20Test%20!"), "Thou Shall Test !");

  //  Test ReadXMLAttributes()
  const char *atts[] = {
            "name", "MyName",
            "description", "Testing a dmml node",
            "hideFromEditors", "false",
            "selectable", "true",
            "selected", "true",
            nullptr};
  node->ReadXMLAttributes(atts);

  //  Test WriteXML
  std::cout << "WriteXML output:" << std::endl << "------------------" << std::endl;
  node->WriteXML(std::cout, 0);
  std::cout << std::endl << "------------------" << std::endl;
  return EXIT_SUCCESS;
}

// ----------------------------------------------------------------------------
int ExerciseBasicStorableDMMLMethods(vtkDMMLStorableNode* node)
  {
  CHECK_EXIT_SUCCESS(ExerciseBasicDMMLMethods(node));

  CHECK_INT(node->GetNumberOfStorageNodes(), 0);

  node->SetAndObserveStorageNodeID("noid");
  node->AddAndObserveStorageNodeID("badid");
  node->SetAndObserveNthStorageNodeID(2, "nothing");

  node->SetCjyxDataType("testing");
  CHECK_STRING(node->GetCjyxDataType(), "testing");

  CHECK_STRING(node->GetNthStorageNodeID(0), "noid");
  CHECK_NULL(node->GetNthStorageNode(0));

  vtkDMMLStorageNode* defaultStorageNode = node->CreateDefaultStorageNode();
  if (defaultStorageNode)
    {
    std::cout << "Default storage node created" << std::endl;
    defaultStorageNode->Delete();
    }

  CHECK_NOT_NULL(node->GetUserTagTable());

  return EXIT_SUCCESS;
  }

// ----------------------------------------------------------------------------
int ExerciseBasicTransformableDMMLMethods(vtkDMMLTransformableNode* node)
{
  CHECK_EXIT_SUCCESS(ExerciseBasicStorableDMMLMethods(node));

  CHECK_NULL(node->GetParentTransformNode());

  node->SetAndObserveTransformNodeID(nullptr);
  CHECK_NULL(node->GetTransformNodeID());

  bool canApplyNonLinear = node->CanApplyNonLinearTransforms();
  std::cout << "Node can apply non linear transforms? " << (canApplyNonLinear == true ? "yes" : "no") << std::endl;
  return EXIT_SUCCESS;
}

// ----------------------------------------------------------------------------
int ExerciseBasicDisplayableDMMLMethods(vtkDMMLDisplayableNode* node)
  {
  CHECK_EXIT_SUCCESS(ExerciseBasicStorableDMMLMethods(node));

  CHECK_INT(node->GetNumberOfDisplayNodes(), 0);

  node->SetAndObserveDisplayNodeID("noid");
  node->AddAndObserveDisplayNodeID("badid");
  node->SetAndObserveNthDisplayNodeID(2, "nothing");
  CHECK_STRING(node->GetNthDisplayNodeID(0), "noid");
  CHECK_NULL(node->GetNthDisplayNode(0));

  return EXIT_SUCCESS;
  }

// ----------------------------------------------------------------------------
int ExerciseBasicDisplayDMMLMethods(vtkDMMLDisplayNode* node)
  {
  CHECK_EXIT_SUCCESS(ExerciseBasicDMMLMethods(node));

  CHECK_NULL(node->GetDisplayableNode());
  node->SetTextureImageDataConnection(nullptr);
  CHECK_NULL(node->GetTextureImageDataConnection());

  node->SetAndObserveColorNodeID(nullptr);
  CHECK_NULL(node->GetColorNodeID());
  CHECK_NULL(node->GetColorNode());

  node->SetActiveScalarName("testingScalar");
  CHECK_STRING(node->GetActiveScalarName(), "testingScalar");

  TEST_SET_GET_VECTOR3_DOUBLE_RANGE(node, Color, 0.0, 1.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANGE(node, SelectedColor, 0.0, 1.0);
  TEST_SET_GET_DOUBLE_RANGE(node, SelectedAmbient, 0.0, 1.0);
  TEST_SET_GET_DOUBLE_RANGE(node, SelectedSpecular, 0.0, 1.0);
  TEST_SET_GET_DOUBLE_RANGE(node, Opacity, 0.0, 1.0);
  TEST_SET_GET_DOUBLE_RANGE(node, Ambient, 0.0, 1.0);
  TEST_SET_GET_DOUBLE_RANGE(node, Diffuse, 0.0, 1.0);
  TEST_SET_GET_DOUBLE_RANGE(node, Specular, 0.0, 1.0);
  TEST_SET_GET_DOUBLE_RANGE(node, Power, 0.0, 1.0);
  TEST_SET_GET_BOOLEAN(node, Visibility);
  TEST_SET_GET_BOOLEAN(node, Visibility2D);
  TEST_SET_GET_BOOLEAN(node, Visibility3D);
  TEST_SET_GET_BOOLEAN(node, Clipping);
  TEST_SET_GET_INT_RANGE(node, SliceIntersectionThickness, 0, 10);
  TEST_SET_GET_BOOLEAN(node, BackfaceCulling);
  TEST_SET_GET_BOOLEAN(node, ScalarVisibility);
  TEST_SET_GET_BOOLEAN(node, VectorVisibility);
  TEST_SET_GET_BOOLEAN(node, TensorVisibility);
  TEST_SET_GET_BOOLEAN(node, AutoScalarRange);
  double expectedRange[2] = {-10, 10};
  node->SetScalarRange(expectedRange);
  double *scalarRange = node->GetScalarRange();
  CHECK_NOT_NULL(scalarRange);
  if (scalarRange[0] != expectedRange[0] || scalarRange[1] != expectedRange[1])
    {
    std::cerr << __LINE__ << " ERROR getting scalar range" << std::endl;
    return EXIT_FAILURE;
    }

  const char *red = "vtkDMMLSliceNodeRed";
  const char *green = "vtkDMMLSliceNodeGreen";
  const char *yellow = "vtkDMMLSliceNodeYellow";
  const char *threeD = "vtkDMMLViewNode1";
  CHECK_INT(node->GetNumberOfViewNodeIDs(), 0);
  CHECK_BOOL(node->IsDisplayableInView(red), true);
  CHECK_BOOL(node->IsDisplayableInView(green), true);
  CHECK_BOOL(node->IsDisplayableInView(yellow), true);
  CHECK_BOOL(node->IsDisplayableInView(threeD), true);

  node->AddViewNodeID(green);
  CHECK_BOOL(node->IsDisplayableInView(red), false);
  CHECK_BOOL(node->IsDisplayableInView(green), true);
  CHECK_BOOL(node->IsDisplayableInView(yellow), false);
  CHECK_BOOL(node->IsDisplayableInView(threeD), false);

  node->SetDisplayableOnlyInView(red);
  CHECK_BOOL(node->IsDisplayableInView(red), true);
  CHECK_BOOL(node->IsDisplayableInView(green), false);
  CHECK_BOOL(node->IsDisplayableInView(yellow), false);
  CHECK_BOOL(node->IsDisplayableInView(threeD), false);

  return EXIT_SUCCESS;
  }

// ----------------------------------------------------------------------------
int ExerciseBasicStorageDMMLMethods(vtkDMMLStorageNode* node)
  {
  CHECK_EXIT_SUCCESS(ExerciseBasicDMMLMethods(node));

  vtkNew<vtkTest::ErrorObserver> errorWarningObserver;
  int errorObserverTag = node->AddObserver(vtkCommand::WarningEvent, errorWarningObserver.GetPointer());
  int warningObserverTag = node->AddObserver(vtkCommand::ErrorEvent, errorWarningObserver.GetPointer());

  node->ReadData(nullptr);
  CHECK_BOOL(errorWarningObserver->GetError(), true);
  errorWarningObserver->Clear();

  node->WriteData(nullptr);
  CHECK_BOOL(errorWarningObserver->GetError(), true);
  errorWarningObserver->Clear();

  TEST_SET_GET_STRING(node, FileName);
  const char *f0 = node->GetNthFileName(0);
  std::cout << "Filename 0 = " << (f0 == nullptr ? "NULL" : f0) << std::endl;
  TEST_SET_GET_BOOLEAN(node, UseCompression);
  TEST_SET_GET_STRING(node, URI);

  vtkURIHandler *handler = vtkURIHandler::New();
  node->SetURIHandler(nullptr);
  CHECK_NULL(node->GetURIHandler());
  node->SetURIHandler(handler);
  CHECK_NOT_NULL(node->GetURIHandler());
  node->SetURIHandler(nullptr);
  handler->Delete();

  TEST_SET_GET_INT_RANGE(node, ReadState, 0, 5);
  const char *rstate = node->GetReadStateAsString();
  std::cout << "Read state, after int test = " << rstate << std::endl;
  node->SetReadStatePending();
  rstate = node->GetReadStateAsString();
  std::cout << "Read state, Pending = " << rstate << std::endl;
  node->SetReadStateIdle();
  rstate = node->GetReadStateAsString();
  std::cout << "Read state, Idle = " << rstate << std::endl;
  node->SetReadStateScheduled();
  rstate = node->GetReadStateAsString();
  std::cout << "Read state, Scheduled = " << rstate << std::endl;
  node->SetReadStateTransferring();
  rstate = node->GetReadStateAsString();
  std::cout << "Read state, Transferring = " << rstate << std::endl;
  node->SetReadStateTransferDone();
  rstate = node->GetReadStateAsString();
  std::cout << "Read state, TransfrerDone = " << rstate << std::endl;
  node->SetReadStateCancelled();
  rstate = node->GetReadStateAsString();
  std::cout << "Read state, Cancelled = " << rstate << std::endl;

  TEST_SET_GET_INT_RANGE(node, WriteState, 0, 5);
  const char *wstate = node->GetWriteStateAsString();
  std::cout << "Write state, after int test = " << wstate << std::endl;
  node->SetWriteStatePending();
  wstate = node->GetWriteStateAsString();
  std::cout << "Write state, Pending = " << wstate << std::endl;
  node->SetWriteStateIdle();
  wstate = node->GetWriteStateAsString();
  std::cout << "Write state, Idle = " << wstate << std::endl;
  node->SetWriteStateScheduled();
  wstate = node->GetWriteStateAsString();
  std::cout << "Write state, Scheduled = " << wstate << std::endl;
  node->SetWriteStateTransferring();
  wstate = node->GetWriteStateAsString();
  std::cout << "Write state, Transferring = " << wstate << std::endl;
  node->SetWriteStateTransferDone();
  wstate = node->GetWriteStateAsString();
  std::cout << "Write state, TransfrerDone = " << wstate << std::endl;
  node->SetWriteStateCancelled();
  wstate = node->GetWriteStateAsString();
  std::cout << "Write state, Cancelled = " << wstate << std::endl;

  std::string fullName = node->GetFullNameFromFileName();
  std::cout << "fullName = " << fullName.c_str() << std::endl;
  std::string fullName0 = node->GetFullNameFromNthFileName(0);
  std::cout << "fullName0 = " << fullName0.c_str() << std::endl;

  vtkStringArray *types = node->GetSupportedWriteFileTypes();
  std::cout << "Supported write types:" << std::endl;
  for (vtkIdType i = 0; i < types->GetNumberOfValues(); i++)
    {
    std::cout << "\t" << types->GetValue(i).c_str() << std::endl;
    }
  int sup = node->SupportedFileType(nullptr);
  CHECK_BOOL(errorWarningObserver->GetWarning(), true);
  errorWarningObserver->Clear();

  std::cout << "Filename or uri supported? " << sup << std::endl;
  sup = node->SupportedFileType("testing.vtk");
  std::cout << ".vtk supported?  " << sup << std::endl;
  sup = node->SupportedFileType("testing.nrrd");
  std::cout << ".nrrd supported?  " << sup << std::endl;
  sup = node->SupportedFileType("testing.mgz");
  std::cout << ".mgz supported?  " << sup << std::endl;

  TEST_SET_GET_STRING(node, WriteFileFormat);
  node->AddFileName("testing.txt");
  std::cout << "Number of file names = " << node->GetNumberOfFileNames() << std::endl;
  CHECK_BOOL(node->FileNameIsInList("testing.txt"), true);

  node->ResetNthFileName(0, "moretesting.txt");
  node->ResetNthFileName(100, "notinlist.txt");
  node->ResetNthFileName(0, nullptr);
  CHECK_BOOL(node->FileNameIsInList("notinlist"), false);

  node->ResetFileNameList();
  CHECK_INT(node->GetNumberOfFileNames(), 0);

  node->ResetURIList();
  std::cout << "Number of uri's after resetting list = " << node->GetNumberOfURIs() << std::endl;
  node->AddURI("https://www.nowhere.com/filename.txt");
  CHECK_INT(node->GetNumberOfURIs(),1);

  CHECK_STRING(node->GetNthURI(0), "https://www.nowhere.com/filename.txt");

  node->ResetNthURI(0, "https://www.nowhere.com/newfilename.txt");
  node->ResetNthURI(100, "ftp://not.in.list");
  node->ResetNthURI(100, nullptr);
  const char *dataDirName = "/test-ing/a/dir ect.ory";
  node->SetDataDirectory(dataDirName);
  node->SetFileName("/tmp/file.txt");
  node->SetDataDirectory(dataDirName);
  CHECK_STRING(node->GetFileName(), "/test-ing/a/dir ect.ory/file.txt");

  std::cout << "Resetting Data Directory to " << dataDirName << " succeeded, got new file name of " << node->GetFileName() << std::endl;
  const char *uriPrefix = "https://www.somewhere.com/";
  node->SetURIPrefix(uriPrefix);

  const char *defaultExt = node->GetDefaultWriteFileExtension();
  std::cout << "Default write extension = " << (defaultExt == nullptr ? "null" : defaultExt) << std::endl;

  std::cout << "Is null file path relative? " << node->IsFilePathRelative(nullptr) << std::endl;
  std::cout << "Is absolute file path relative? " << node->IsFilePathRelative("/spl/tmp/file.txt") << std::endl;
  std::cout << "Is relative file path relative? " << node->IsFilePathRelative("tmp/file.txt") << std::endl;
  node->RemoveObserver(errorObserverTag);
  node->RemoveObserver(warningObserverTag);

  // Remove any error/warning messages, just in case the storage node is later used in some more tests
  node->GetUserMessages()->ClearMessages();

  return EXIT_SUCCESS;
  }

// ----------------------------------------------------------------------------
int ExerciseBasicTransformDMMLMethods(vtkDMMLTransformNode* node)
  {
  CHECK_EXIT_SUCCESS(ExerciseBasicStorableDMMLMethods(node));

  std::cout << "IsLinear = " << node->IsLinear()<< std:: endl;
  CHECK_NOT_NULL(node->GetTransformToParent());
  bool isTransformToWorldLinear = node->IsTransformToWorldLinear();
  std::cout << "IsTransformToWorldLinear = " << isTransformToWorldLinear << std::endl;
  vtkSmartPointer < vtkDMMLTransformNode > t = vtkSmartPointer < vtkDMMLTransformNode >::Take(vtkDMMLTransformNode::SafeDownCast(node->CreateNodeInstance()));
  std::cout << "IsTransformToNodeLinear = " << node->IsTransformToNodeLinear(t) << std::endl;
  vtkSmartPointer<vtkGeneralTransform> g =  vtkSmartPointer<vtkGeneralTransform>::New();
  node->GetTransformToWorld(g);
  node->GetTransformToNode(t, g);
  vtkSmartPointer<vtkMatrix4x4> m =  vtkSmartPointer<vtkMatrix4x4>::New();
  if (!isTransformToWorldLinear)
    {
    TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
    }
  int getMatrixTransformToWorldResult = node->GetMatrixTransformToWorld(m);
  if (!isTransformToWorldLinear)
    {
    TESTING_OUTPUT_ASSERT_ERRORS_END();
    }
  else
    {
    CHECK_BOOL(getMatrixTransformToWorldResult!=0, true);
    }
  if (!isTransformToWorldLinear)
    {
    TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
    }
  int getMatrixTransformToNodeResult = node->GetMatrixTransformToNode(t, m);
  if (!isTransformToWorldLinear)
    {
    TESTING_OUTPUT_ASSERT_ERRORS_END();
    }
  else
    {
    CHECK_BOOL(getMatrixTransformToNodeResult!=0, true);
    }
  std::cout << "IsTransformNodeMyParent = " << node->IsTransformNodeMyParent(t) << std::endl;
  std::cout << "IsTransformNodeMyChild = " << node->IsTransformNodeMyChild(t) << std::endl;
  return EXIT_SUCCESS;
  }

// ----------------------------------------------------------------------------
int ExerciseAllBasicDMMLMethods(vtkDMMLNode* node)
{
  if (vtkDMMLDisplayNode::SafeDownCast(node))
    {
    return ExerciseBasicDisplayDMMLMethods(vtkDMMLDisplayNode::SafeDownCast(node));
    }
  if (vtkDMMLStorageNode::SafeDownCast(node))
    {
    return ExerciseBasicStorageDMMLMethods(vtkDMMLStorageNode::SafeDownCast(node));
    }

  // Go from specific to general types, test the most specific interface
  if (vtkDMMLTransformNode::SafeDownCast(node))
    {
    return ExerciseBasicTransformDMMLMethods(vtkDMMLTransformNode::SafeDownCast(node));
    }
  if (vtkDMMLDisplayableNode::SafeDownCast(node))
    {
    return ExerciseBasicDisplayableDMMLMethods(vtkDMMLDisplayableNode::SafeDownCast(node));
    }
  if (vtkDMMLTransformableNode::SafeDownCast(node))
    {
    return ExerciseBasicTransformableDMMLMethods(vtkDMMLTransformableNode::SafeDownCast(node));
    }
  if (vtkDMMLStorableNode::SafeDownCast(node))
    {
    return ExerciseBasicStorableDMMLMethods(vtkDMMLStorableNode::SafeDownCast(node));
    }
  return ExerciseBasicDMMLMethods(node);
}

// ----------------------------------------------------------------------------
int ExerciseSceneLoadingMethods(const char * sceneFilePath, vtkDMMLScene* inputScene /* = nullptr */)
{
  vtkSmartPointer<vtkDMMLScene> scene;
  if (inputScene)
    {
    scene = inputScene;
    }
  else
    {
      scene = vtkSmartPointer<vtkDMMLScene>::New();
    }

  // Add default slice orientation presets
  vtkDMMLSliceNode::AddDefaultSliceOrientationPresets(scene);

  scene->SetURL(sceneFilePath);
  scene->Connect();
  int numberOfNodes = scene->GetNumberOfNodes();
  scene->Connect();
  int numberOfNodesAfterReconnect = scene->GetNumberOfNodes();
  CHECK_INT(numberOfNodes, numberOfNodesAfterReconnect);

  scene->Import();
  int numberOfNodesAfterImport = scene->GetNumberOfNodes();
  CHECK_BOOL(numberOfNodesAfterImport>numberOfNodes, true);

  scene->Import();

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
vtkDMMLNodeCallback::vtkDMMLNodeCallback()
{
  this->ResetNumberOfEvents();
}

//---------------------------------------------------------------------------
vtkDMMLNodeCallback::~vtkDMMLNodeCallback() = default;

//---------------------------------------------------------------------------
void vtkDMMLNodeCallback::ResetNumberOfEvents()
{
  this->ReceivedEvents.clear();
}

//---------------------------------------------------------------------------
void vtkDMMLNodeCallback::SetDMMLNode(vtkDMMLNode* node)
{
  this->Node = node;
}

//---------------------------------------------------------------------------
std::string vtkDMMLNodeCallback::GetErrorString()
{
  return this->ErrorString;
}

//---------------------------------------------------------------------------
void vtkDMMLNodeCallback::SetErrorString(const char* error)
{
  this->ErrorString = std::string(error?error:"");
}

//---------------------------------------------------------------------------
void vtkDMMLNodeCallback::SetErrorString(int line, const char* error)
{
  std::stringstream ss;
  ss << "line " << line << " - " << (error?error:"(undefined)");
  this->ErrorString = ss.str();
}

//---------------------------------------------------------------------------
int vtkDMMLNodeCallback::GetNumberOfModified()
{
  return this->GetNumberOfEvents(vtkCommand::ModifiedEvent);
}

//---------------------------------------------------------------------------
int vtkDMMLNodeCallback::GetNumberOfEvents(unsigned long event)
{
  return this->ReceivedEvents[event];
}

//---------------------------------------------------------------------------
int vtkDMMLNodeCallback::GetTotalNumberOfEvents()
{
  int eventsCount = 0;
  for (std::map<unsigned long, unsigned int>::const_iterator it = this->ReceivedEvents.begin(),
       end = this->ReceivedEvents.end(); it != end; ++it)
    {
    eventsCount += it->second;
    }
  return eventsCount;
}

//---------------------------------------------------------------------------
std::vector<unsigned long> vtkDMMLNodeCallback::GetReceivedEvents()
{
  std::vector<unsigned long> receivedEvents;
  for(std::map<unsigned long,unsigned int>::iterator it = this->ReceivedEvents.begin();
      it != this->ReceivedEvents.end();
      ++it)
    {
    unsigned long event = it->first;
    if (this->GetNumberOfEvents(event) > 0)
      {
      receivedEvents.push_back(event);
      }
    }
  return receivedEvents;
}

//---------------------------------------------------------------------------
void vtkDMMLNodeCallback::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkCallbackCommand::PrintSelf(os,indent);

  os << indent << "ErrorString: " << this->GetErrorString() << "\n";
  os << indent << "TotalNumberOfEvents: " << this->GetTotalNumberOfEvents() << "\n";
  os << indent << "NumberOfModified: " << this->GetNumberOfModified() << "\n";
  std::vector<unsigned long> receivedEvent = this->GetReceivedEvents();
  os << indent << "ReceivedEvents: \n";
  for(std::vector<unsigned long>::iterator it = receivedEvent.begin();
      it != receivedEvent.end();
      ++it)
    {
    os << indent.GetNextIndent() << *it << " \n";
    }
}

//---------------------------------------------------------------------------
void vtkDMMLNodeCallback::Execute(vtkObject *vtkcaller,
  unsigned long eid, void *vtkNotUsed(calldata))
{
  // Let's return if an error already occurred
  if (this->ErrorString.size() > 0)
    {
    return;
    }
  if (this->Node &&
      this->Node != vtkDMMLNode::SafeDownCast(vtkcaller))
    {
    this->SetErrorString("vtkDMMLNodeCallback::Execute - node != vtkcaller");
    return;
    }

  ++this->ReceivedEvents[eid];
}

//---------------------------------------------------------------------------
int vtkDMMLNodeCallback::CheckStatus()
{
  if (!this->ErrorString.empty())
    {
    std::cerr << this->ErrorString << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

} // namespace vtkDMMLCoreTestingUtilities
