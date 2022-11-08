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

// DMMLLogic includes
#include "vtkDMMLAbstractLogic.h"

// DMML includes
#include <vtkDMMLModelNode.h>
#include <vtkDMMLScalarVolumeNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <iostream>

//---------------------------------------------------------------------------
/// vtkDMMLTestLogic records what methods of vtkDMMLAbstractLogic are called
/// when vtkDMMLScene fires events.
class vtkDMMLTestLogic: public vtkDMMLAbstractLogic
{
public:
  vtkTypeMacro(vtkDMMLTestLogic, vtkDMMLAbstractLogic);
  static vtkDMMLTestLogic *New();

  void SetDMMLSceneInternal(vtkDMMLScene* scene) override;
  void UnobserveDMMLScene() override;
  void ObserveDMMLScene() override;
  void RegisterNodes() override;
  void UpdateFromDMMLScene() override;

  void OnDMMLSceneStartBatchProcess() override;
  void OnDMMLSceneEndBatchProcess() override;
  void OnDMMLSceneStartClose() override;
  void OnDMMLSceneEndClose() override;
  void OnDMMLSceneStartImport() override;
  void OnDMMLSceneEndImport() override;
  void OnDMMLSceneStartRestore() override;
  void OnDMMLSceneEndRestore() override;
  void OnDMMLSceneNew() override;
  void OnDMMLSceneNodeAdded(vtkDMMLNode* nodeAdded) override;
  void OnDMMLSceneNodeRemoved(vtkDMMLNode* nodeRemoved) override;

  enum MethodType{
    Set = 0,
    Unobserve,
    Observe,
    Register,
    Update
  };
  /// CalledMethods stores how many times a method is called.
  /// Methods are identified using vtkDMMLTestLogic::MethodType
  /// or vtkDMMLScene::SceneEventType
  std::map<unsigned long, int> CalledMethods;
protected:
  vtkDMMLTestLogic() = default;
  ~vtkDMMLTestLogic() override = default;
};

vtkStandardNewMacro(vtkDMMLTestLogic);

//---------------------------------------------------------------------------
void vtkDMMLTestLogic::SetDMMLSceneInternal(vtkDMMLScene* scene)
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkDMMLTestLogic::Set];

  // Listen to all the events
  vtkNew<vtkIntArray> sceneEvents;
  sceneEvents->InsertNextValue(vtkDMMLScene::StartBatchProcessEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::EndBatchProcessEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::StartCloseEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::EndCloseEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::StartImportEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::EndImportEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::StartRestoreEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::EndRestoreEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::NewSceneEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::NodeAddedEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::NodeRemovedEvent);

  this->SetAndObserveDMMLSceneEventsInternal(scene, sceneEvents.GetPointer());
}

//---------------------------------------------------------------------------
void vtkDMMLTestLogic::UnobserveDMMLScene()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkDMMLTestLogic::Unobserve];
  this->Superclass::UnobserveDMMLScene();
}

//---------------------------------------------------------------------------
void vtkDMMLTestLogic::ObserveDMMLScene()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkDMMLTestLogic::Observe];
  this->Superclass::ObserveDMMLScene();
}

//---------------------------------------------------------------------------
void vtkDMMLTestLogic::RegisterNodes()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkDMMLTestLogic::Register];
  this->Superclass::RegisterNodes();
}

//---------------------------------------------------------------------------
void vtkDMMLTestLogic::UpdateFromDMMLScene()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkDMMLTestLogic::Update];
  this->Superclass::UpdateFromDMMLScene();
}

//---------------------------------------------------------------------------
void vtkDMMLTestLogic::OnDMMLSceneStartBatchProcess()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkDMMLScene::StartBatchProcessEvent];
  this->Superclass::OnDMMLSceneStartBatchProcess();
}

//---------------------------------------------------------------------------
void vtkDMMLTestLogic::OnDMMLSceneEndBatchProcess()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkDMMLScene::EndBatchProcessEvent];
  this->Superclass::OnDMMLSceneEndBatchProcess();
}

//---------------------------------------------------------------------------
void vtkDMMLTestLogic::OnDMMLSceneStartClose()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkDMMLScene::StartCloseEvent];
  this->Superclass::OnDMMLSceneStartClose();
}

//---------------------------------------------------------------------------
void vtkDMMLTestLogic::OnDMMLSceneEndClose()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkDMMLScene::EndCloseEvent];
  this->Superclass::OnDMMLSceneEndClose();
}

//---------------------------------------------------------------------------
void vtkDMMLTestLogic::OnDMMLSceneStartImport()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkDMMLScene::StartImportEvent];
  this->Superclass::OnDMMLSceneStartImport();
}

//---------------------------------------------------------------------------
void vtkDMMLTestLogic::OnDMMLSceneEndImport()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkDMMLScene::EndImportEvent];
  this->Superclass::OnDMMLSceneEndImport();
}

//---------------------------------------------------------------------------
void vtkDMMLTestLogic::OnDMMLSceneStartRestore()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkDMMLScene::StartRestoreEvent];
  this->Superclass::OnDMMLSceneStartRestore();
}

//---------------------------------------------------------------------------
void vtkDMMLTestLogic::OnDMMLSceneEndRestore()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkDMMLScene::EndRestoreEvent];
  this->Superclass::OnDMMLSceneEndRestore();
}

//---------------------------------------------------------------------------
void vtkDMMLTestLogic::OnDMMLSceneNew()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkDMMLScene::NewSceneEvent];
  this->Superclass::OnDMMLSceneNew();
}

//---------------------------------------------------------------------------
void vtkDMMLTestLogic::OnDMMLSceneNodeAdded(vtkDMMLNode* nodeAdded)
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkDMMLScene::NodeAddedEvent];
  this->Superclass::OnDMMLSceneNodeAdded(nodeAdded);
}

//---------------------------------------------------------------------------
void vtkDMMLTestLogic::OnDMMLSceneNodeRemoved(vtkDMMLNode* nodeRemoved)
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkDMMLScene::NodeRemovedEvent];
  this->Superclass::OnDMMLSceneNodeRemoved(nodeRemoved);
}

//---------------------------------------------------------------------------
int vtkDMMLAbstractLogicSceneEventsTest(
  int vtkNotUsed(argc), char * vtkNotUsed(argv)[] )
{
  vtkNew<vtkDMMLScene> scene;
  vtkDMMLTestLogic* testLogic = vtkDMMLTestLogic::New();

  //---------------------------------------------------------------------------
  // SetDMMLScene
  //---------------------------------------------------------------------------
  testLogic->SetDMMLScene(scene.GetPointer());

  if (testLogic->GetDMMLScene() != scene.GetPointer() ||
      testLogic->CalledMethods.size() != 4 ||
      testLogic->CalledMethods[vtkDMMLTestLogic::Set] != 1 ||
      testLogic->CalledMethods[vtkDMMLTestLogic::Observe] != 1 ||
      testLogic->CalledMethods[vtkDMMLTestLogic::Register] != 1 ||
      testLogic->CalledMethods[vtkDMMLTestLogic::Update] != 1)
    {
    std::cerr << "Wrong fired events: "
              << testLogic->CalledMethods.size() << " event(s) fired." << std::endl
              << testLogic->CalledMethods[vtkDMMLTestLogic::Set] << " "
              << testLogic->CalledMethods[vtkDMMLTestLogic::Observe] << " "
              << testLogic->CalledMethods[vtkDMMLTestLogic::Register] << " "
              << testLogic->CalledMethods[vtkDMMLTestLogic::Update]
              << std::endl;
    return EXIT_FAILURE;
    }
  testLogic->CalledMethods.clear();

  //---------------------------------------------------------------------------
  // SetDMMLScene(other scene)
  //---------------------------------------------------------------------------
  vtkNew<vtkDMMLScene> scene2;
  testLogic->SetDMMLScene(scene2.GetPointer());

  if (testLogic->GetDMMLScene() != scene2.GetPointer() ||
      testLogic->CalledMethods.size() != 5 ||
      testLogic->CalledMethods[vtkDMMLTestLogic::Unobserve] != 1 ||
      testLogic->CalledMethods[vtkDMMLTestLogic::Set] != 1 ||
      testLogic->CalledMethods[vtkDMMLTestLogic::Observe] != 1 ||
      testLogic->CalledMethods[vtkDMMLTestLogic::Register] != 1 ||
      testLogic->CalledMethods[vtkDMMLTestLogic::Update] != 1)
    {
    std::cerr << "Wrong fired events: "
              << testLogic->CalledMethods.size() << " event(s) fired." << std::endl
              << testLogic->CalledMethods[vtkDMMLTestLogic::Unobserve] << " "
              << testLogic->CalledMethods[vtkDMMLTestLogic::Set] << " "
              << testLogic->CalledMethods[vtkDMMLTestLogic::Observe] << " "
              << testLogic->CalledMethods[vtkDMMLTestLogic::Register] << " "
              << testLogic->CalledMethods[vtkDMMLTestLogic::Update]
              << std::endl;
    return EXIT_FAILURE;
    }
  testLogic->CalledMethods.clear();

  //---------------------------------------------------------------------------
  // SetDMMLScene(0)
  //---------------------------------------------------------------------------
  testLogic->SetDMMLScene(nullptr);

  if (testLogic->GetDMMLScene() != nullptr ||
      testLogic->CalledMethods.size() != 2 ||
      testLogic->CalledMethods[vtkDMMLTestLogic::Unobserve] != 1 ||
      testLogic->CalledMethods[vtkDMMLTestLogic::Set] != 1)
    {
    std::cerr << "Wrong fired events: "
              << testLogic->CalledMethods.size() << " event(s) fired." << std::endl
              << testLogic->CalledMethods[vtkDMMLTestLogic::Unobserve] << " "
              << testLogic->CalledMethods[vtkDMMLTestLogic::Set]
              << std::endl;
    return EXIT_FAILURE;
    }
  testLogic->CalledMethods.clear();

  //---------------------------------------------------------------------------
  // SetDMMLScene(scene)
  //---------------------------------------------------------------------------
  testLogic->SetDMMLScene(scene.GetPointer());

  if (testLogic->GetDMMLScene() != scene.GetPointer() ||
      testLogic->CalledMethods.size() != 4 ||
      testLogic->CalledMethods[vtkDMMLTestLogic::Set] != 1 ||
      testLogic->CalledMethods[vtkDMMLTestLogic::Observe] != 1 ||
      testLogic->CalledMethods[vtkDMMLTestLogic::Register] != 1 ||
      testLogic->CalledMethods[vtkDMMLTestLogic::Update] != 1)
    {
    std::cerr << "Wrong fired events: "
              << testLogic->CalledMethods.size() << " event(s) fired." << std::endl
              << testLogic->CalledMethods[vtkDMMLTestLogic::Set] << " "
              << testLogic->CalledMethods[vtkDMMLTestLogic::Observe] << " "
              << testLogic->CalledMethods[vtkDMMLTestLogic::Register] << " "
              << testLogic->CalledMethods[vtkDMMLTestLogic::Update]
              << std::endl;
    return EXIT_FAILURE;
    }
  testLogic->CalledMethods.clear();

  //---------------------------------------------------------------------------
  // Import
  //---------------------------------------------------------------------------
  scene->StartState(vtkDMMLScene::ImportState);

  if (testLogic->CalledMethods.size() != 2 ||
      testLogic->CalledMethods[vtkDMMLScene::StartBatchProcessEvent] != 1 ||
      testLogic->CalledMethods[vtkDMMLScene::StartImportEvent] != 1)
    {
    std::cerr << "Wrong fired events: "
              << testLogic->CalledMethods.size() << " event(s) fired." << std::endl
              << testLogic->CalledMethods[vtkDMMLScene::StartBatchProcessEvent] << " "
              << testLogic->CalledMethods[vtkDMMLScene::StartImportEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  testLogic->CalledMethods.clear();

  // Add node into the scene during the import state
  vtkNew<vtkDMMLScalarVolumeNode> volumeNode;
  scene->AddNode(volumeNode.GetPointer());

  if (testLogic->CalledMethods.size() != 1 ||
      testLogic->CalledMethods[vtkDMMLScene::NodeAddedEvent] != 1)
    {
    std::cerr << "Wrong fired events: "
              << testLogic->CalledMethods.size() << " event(s) fired." << std::endl
              << testLogic->CalledMethods[vtkDMMLScene::NodeAddedEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  testLogic->CalledMethods.clear();

  // end of the import
  scene->EndState(vtkDMMLScene::ImportState);

  if (testLogic->CalledMethods.size() != 3 ||
      testLogic->CalledMethods[vtkDMMLScene::EndImportEvent] != 1 ||
      testLogic->CalledMethods[vtkDMMLScene::EndBatchProcessEvent] != 1 ||
      testLogic->CalledMethods[vtkDMMLTestLogic::Update] != 1)
    {
    std::cerr << "Wrong fired events: "
              << testLogic->CalledMethods.size() << " event(s) fired." << std::endl
              << testLogic->CalledMethods[vtkDMMLScene::EndBatchProcessEvent] << " "
              << testLogic->CalledMethods[vtkDMMLScene::EndImportEvent] << " "
              << testLogic->CalledMethods[vtkDMMLTestLogic::Update]
              << std::endl;
    return EXIT_FAILURE;
    }
  testLogic->CalledMethods.clear();

  //---------------------------------------------------------------------------
  // Add node (outside of batch processing)
  //---------------------------------------------------------------------------
  vtkNew<vtkDMMLModelNode> modelNode;
  scene->AddNode(modelNode.GetPointer());
  if (testLogic->CalledMethods.size() != 1 ||
      testLogic->CalledMethods[vtkDMMLScene::NodeAddedEvent] != 1)
    {
    std::cerr << "Wrong fired events: "
              << testLogic->CalledMethods.size() << " event(s) fired." << std::endl
              << testLogic->CalledMethods[vtkDMMLScene::NodeAddedEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  testLogic->CalledMethods.clear();

  //---------------------------------------------------------------------------
  // Close
  //---------------------------------------------------------------------------
  scene->Clear(false);
  if (testLogic->CalledMethods.size() != 6 ||
      testLogic->CalledMethods[vtkDMMLScene::StartBatchProcessEvent] != 1 ||
      testLogic->CalledMethods[vtkDMMLScene::StartCloseEvent] != 1 ||
      testLogic->CalledMethods[vtkDMMLScene::NodeRemovedEvent] != 2 ||
      testLogic->CalledMethods[vtkDMMLScene::EndCloseEvent] != 1 ||
      testLogic->CalledMethods[vtkDMMLScene::EndBatchProcessEvent] != 1 ||
      testLogic->CalledMethods[vtkDMMLTestLogic::Update] != 1)
    {
    std::cerr << "Wrong fired events: "
              << testLogic->CalledMethods.size() << " event(s) fired." << std::endl
              << testLogic->CalledMethods[vtkDMMLScene::StartBatchProcessEvent] << " "
              << testLogic->CalledMethods[vtkDMMLScene::StartCloseEvent] << " "
              << testLogic->CalledMethods[vtkDMMLScene::NodeRemovedEvent] << " "
              << testLogic->CalledMethods[vtkDMMLScene::EndCloseEvent] << " "
              << testLogic->CalledMethods[vtkDMMLScene::EndBatchProcessEvent] << " "
              << testLogic->CalledMethods[vtkDMMLTestLogic::Update] << " "
              << std::endl;
    return EXIT_FAILURE;
    }
  testLogic->CalledMethods.clear();

  //---------------------------------------------------------------------------
  // Add node again (outside of batch processing)
  //---------------------------------------------------------------------------
  scene->AddNode(modelNode.GetPointer());
  scene->AddNode(volumeNode.GetPointer());

  testLogic->Delete();

  return EXIT_SUCCESS;
}

