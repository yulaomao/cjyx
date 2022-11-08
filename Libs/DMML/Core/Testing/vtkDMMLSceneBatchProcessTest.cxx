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
#include "vtkDMMLModelNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSceneEventRecorder.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkNew.h>

// STD includes
#include <iostream>

//---------------------------------------------------------------------------
int vtkDMMLSceneBatchProcessTest(
  int vtkNotUsed(argc), char * vtkNotUsed(argv) [] )
{
  /*
  if( argc < 2 )
    {
    std::cerr << "Error: missing arguments" << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << "  inputURL_scene.dmml " << std::endl;
    return EXIT_FAILURE;
    }
  */
  // Instantiate scene
  vtkNew<vtkDMMLScene> scene;

  // Configure dmml event callback
  vtkNew<vtkDMMLSceneEventRecorder> callback;
  scene->AddObserver(vtkCommand::AnyEvent, callback.GetPointer());

  //---------------------------------------------------------------------------
  // BatchProcess
  //---------------------------------------------------------------------------
  // Fires:
  // 1) StartBatchProcessEvent
  scene->StartState(vtkDMMLScene::BatchProcessState);

  if (callback->CalledEvents.size() != 1 ||
      callback->CalledEvents[vtkDMMLScene::StartBatchProcessEvent] != 1)
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkDMMLScene::StartBatchProcessEvent]
              << std::endl;
    return EXIT_FAILURE;
    }

  callback->CalledEvents.clear();

  // Fires:
  // 1) EndBatchProcessEvent
  scene->EndState(vtkDMMLScene::BatchProcessState);

  if (callback->CalledEvents.size() != 1 ||
      callback->CalledEvents[vtkDMMLScene::EndBatchProcessEvent] != 1)
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkDMMLScene::StartBatchProcessEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  //---------------------------------------------------------------------------
  // Nested BatchProcess
  //---------------------------------------------------------------------------
  // Fires:
  // 1) StartBatchProcessEvent
  scene->StartState(vtkDMMLScene::BatchProcessState);
  scene->StartState(vtkDMMLScene::BatchProcessState);

  if (scene->IsBatchProcessing() != true||
      callback->CalledEvents.size() != 1 ||
      callback->CalledEvents[vtkDMMLScene::StartBatchProcessEvent] != 1)
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkDMMLScene::StartBatchProcessEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  scene->EndState(vtkDMMLScene::BatchProcessState);
  if (scene->IsBatchProcessing() != true ||
      callback->CalledEvents.size() != 0)
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkDMMLScene::EndBatchProcessEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  // Fires:
  // 2) EndBatchProcessEvent
  scene->EndState(vtkDMMLScene::BatchProcessState);

  if (scene->IsBatchProcessing() != false ||
      callback->CalledEvents.size() != 1 ||
      callback->CalledEvents[vtkDMMLScene::EndBatchProcessEvent] != 1)
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkDMMLScene::EndBatchProcessEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  //---------------------------------------------------------------------------
  // Import
  //---------------------------------------------------------------------------
  // Fires:
  // 1) StartBatchProcessEvent
  // 2) StartImportProcessEvent
  scene->StartState(vtkDMMLScene::ImportState);

  if (scene->IsBatchProcessing() != true ||
      scene->IsImporting() != true ||
      callback->CalledEvents.size() != 2 ||
      callback->CalledEvents[vtkDMMLScene::StartBatchProcessEvent] != 1 ||
      callback->CalledEvents[vtkDMMLScene::StartImportEvent] != 1 ||
      callback->LastEventMTime[vtkDMMLScene::StartBatchProcessEvent] >
      callback->LastEventMTime[vtkDMMLScene::StartImportEvent])
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkDMMLScene::StartBatchProcessEvent] << " "
              << callback->CalledEvents[vtkDMMLScene::StartImportEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  // 3) NodeAboutToBeAddedEvent
  // 4) NodeAddedEvent
  vtkNew<vtkDMMLModelNode> modelNode;
  scene->AddNode(modelNode.GetPointer());

  if (scene->IsBatchProcessing() != true ||
      scene->IsImporting() != true ||
      callback->CalledEvents.size() != 2 ||
      callback->CalledEvents[vtkDMMLScene::NodeAboutToBeAddedEvent] != 1 ||
      callback->CalledEvents[vtkDMMLScene::NodeAddedEvent] != 1 ||
      callback->LastEventMTime[vtkDMMLScene::NodeAboutToBeAddedEvent] >
      callback->LastEventMTime[vtkDMMLScene::NodeAddedEvent])
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkDMMLScene::NodeAboutToBeAddedEvent] << " "
              << callback->CalledEvents[vtkDMMLScene::NodeAddedEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  // 5) EndImportProcessEvent
  // 6) EndBatchProcessEvent
  scene->EndState(vtkDMMLScene::ImportState);

  if (scene->IsBatchProcessing() != false ||
      scene->IsImporting() != false ||
      callback->CalledEvents.size() != 2 ||
      callback->CalledEvents[vtkDMMLScene::EndImportEvent] != 1 ||
      callback->CalledEvents[vtkDMMLScene::EndBatchProcessEvent] != 1 ||
      callback->LastEventMTime[vtkDMMLScene::EndImportEvent] >
      callback->LastEventMTime[vtkDMMLScene::EndBatchProcessEvent])
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkDMMLScene::EndImportEvent] << " "
              << callback->CalledEvents[vtkDMMLScene::EndBatchProcessEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  //---------------------------------------------------------------------------
  // BatchProcess + Import
  //---------------------------------------------------------------------------
  // Fires:
  // 1) StartBatchProcessEvent
  // 2) StartImportProcessEvent
  scene->StartState(vtkDMMLScene::BatchProcessState);
  scene->StartState(vtkDMMLScene::ImportState);

  if (scene->IsBatchProcessing() != true ||
      scene->IsImporting() != true ||
      callback->CalledEvents.size() != 2 ||
      callback->CalledEvents[vtkDMMLScene::StartBatchProcessEvent] != 1 ||
      callback->CalledEvents[vtkDMMLScene::StartImportEvent] != 1 ||
      callback->LastEventMTime[vtkDMMLScene::StartBatchProcessEvent] >
      callback->LastEventMTime[vtkDMMLScene::StartImportEvent])
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkDMMLScene::StartBatchProcessEvent] << " "
              << callback->CalledEvents[vtkDMMLScene::StartImportEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  // 3) EndImportProcessEvent
  // 4) EndBatchProcessEvent
  scene->EndState(vtkDMMLScene::ImportState);
  if (scene->IsBatchProcessing() != true ||
      scene->IsImporting() != false ||
      callback->CalledEvents.size() != 1 ||
      callback->CalledEvents[vtkDMMLScene::EndImportEvent] != 1)
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkDMMLScene::EndImportEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  scene->EndState(vtkDMMLScene::BatchProcessState);

  if (scene->IsBatchProcessing() != false ||
      scene->IsImporting() != false ||
      callback->CalledEvents.size() != 2 ||
      callback->CalledEvents[vtkDMMLScene::EndImportEvent] != 1 ||
      callback->CalledEvents[vtkDMMLScene::EndBatchProcessEvent] != 1 ||
      callback->LastEventMTime[vtkDMMLScene::EndImportEvent] >
      callback->LastEventMTime[vtkDMMLScene::EndBatchProcessEvent])
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkDMMLScene::EndImportEvent] << " "
              << callback->CalledEvents[vtkDMMLScene::EndBatchProcessEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  //---------------------------------------------------------------------------
  // Close
  //---------------------------------------------------------------------------
  // Fires:
  // 1) StartBatchProcessEvent
  // 2) StartCloseProcessEvent
  scene->StartState(vtkDMMLScene::CloseState);

  if (scene->IsBatchProcessing() != true ||
      scene->IsClosing() != true ||
      callback->CalledEvents.size() != 2 ||
      callback->CalledEvents[vtkDMMLScene::StartCloseEvent] != 1 ||
      callback->CalledEvents[vtkDMMLScene::StartBatchProcessEvent] != 1 ||
      callback->LastEventMTime[vtkDMMLScene::StartCloseEvent] >
      callback->LastEventMTime[vtkDMMLScene::StartBatchProcessEvent])
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkDMMLScene::StartBatchProcessEvent] << " "
              << callback->CalledEvents[vtkDMMLScene::StartCloseEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  // 3) NodeAboutToBeRemovedEvent
  // 4) NodeRemovedEvent
  scene->RemoveNode(modelNode.GetPointer());

  if (scene->IsBatchProcessing() != true ||
      scene->IsClosing() != true ||
      callback->CalledEvents.size() != 2 ||
      callback->CalledEvents[vtkDMMLScene::NodeAboutToBeRemovedEvent] != 1 ||
      callback->CalledEvents[vtkDMMLScene::NodeRemovedEvent] != 1 ||
      callback->LastEventMTime[vtkDMMLScene::NodeAboutToBeRemovedEvent] >
      callback->LastEventMTime[vtkDMMLScene::NodeRemovedEvent])
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkDMMLScene::NodeAboutToBeRemovedEvent] << " "
              << callback->CalledEvents[vtkDMMLScene::NodeRemovedEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  // 5) EndCloseProcessEvent
  // 6) EndBatchProcessEvent
  scene->EndState(vtkDMMLScene::CloseState);

  if (scene->IsBatchProcessing() != false ||
      scene->IsClosing() != false ||
      callback->CalledEvents.size() != 2 ||
      callback->CalledEvents[vtkDMMLScene::EndCloseEvent] != 1 ||
      callback->CalledEvents[vtkDMMLScene::EndBatchProcessEvent] != 1 ||
      callback->LastEventMTime[vtkDMMLScene::EndCloseEvent] >
      callback->LastEventMTime[vtkDMMLScene::EndBatchProcessEvent])
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkDMMLScene::EndCloseEvent] << " "
              << callback->CalledEvents[vtkDMMLScene::EndBatchProcessEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  //---------------------------------------------------------------------------
  // Restore within Restore
  //---------------------------------------------------------------------------
  // Fires:
  // 1) StartBatchProcessEvent
  // 2) StartRestoreProcessEvent
  scene->StartState(vtkDMMLScene::RestoreState);
  scene->StartState(vtkDMMLScene::RestoreState);

  if (scene->IsBatchProcessing() != true ||
      scene->IsRestoring() != true ||
      callback->CalledEvents.size() != 2 ||
      callback->CalledEvents[vtkDMMLScene::StartBatchProcessEvent] != 1 ||
      callback->CalledEvents[vtkDMMLScene::StartRestoreEvent] != 1 ||
      callback->LastEventMTime[vtkDMMLScene::StartBatchProcessEvent] >
      callback->LastEventMTime[vtkDMMLScene::StartRestoreEvent])
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkDMMLScene::StartBatchProcessEvent] << " "
              << callback->CalledEvents[vtkDMMLScene::StartRestoreEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();


  // 5) EndRestoreProcessEvent
  // 6) EndBatchProcessEvent
  scene->EndState(vtkDMMLScene::RestoreState);
  scene->EndState(vtkDMMLScene::RestoreState);

  if (scene->IsBatchProcessing() != false ||
      scene->IsRestoring() != false ||
      callback->CalledEvents.size() != 2 ||
      callback->CalledEvents[vtkDMMLScene::EndRestoreEvent] != 1 ||
      callback->CalledEvents[vtkDMMLScene::EndBatchProcessEvent] != 1 ||
      callback->LastEventMTime[vtkDMMLScene::EndRestoreEvent] >
      callback->LastEventMTime[vtkDMMLScene::EndBatchProcessEvent])
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkDMMLScene::EndRestoreEvent] << " "
              << callback->CalledEvents[vtkDMMLScene::EndBatchProcessEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();


  //---------------------------------------------------------------------------
  // Import within Restore
  //---------------------------------------------------------------------------
  // Fires:
  // 1) StartBatchProcessEvent
  // 2) StartRestoreProcessEvent
  // 3) StartImportProcessEvent
  scene->StartState(vtkDMMLScene::RestoreState);
  scene->StartState(vtkDMMLScene::ImportState);

  if (scene->IsBatchProcessing() != true ||
      scene->IsRestoring() != true ||
      scene->IsImporting() != true ||
      callback->CalledEvents.size() != 3 ||
      callback->CalledEvents[vtkDMMLScene::StartBatchProcessEvent] != 1 ||
      callback->CalledEvents[vtkDMMLScene::StartRestoreEvent] != 1 ||
      callback->CalledEvents[vtkDMMLScene::StartImportEvent] != 1 ||
      callback->LastEventMTime[vtkDMMLScene::StartBatchProcessEvent] >
      callback->LastEventMTime[vtkDMMLScene::StartRestoreEvent] ||
      callback->LastEventMTime[vtkDMMLScene::StartRestoreEvent] >
      callback->LastEventMTime[vtkDMMLScene::StartImportEvent])
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkDMMLScene::StartBatchProcessEvent] << " "
              << callback->CalledEvents[vtkDMMLScene::StartRestoreEvent] << " "
              << callback->CalledEvents[vtkDMMLScene::StartImportEvent] << " "
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  // 4) EndImportProcessEvent
  // 5) EndRestoreProcessEvent
  // 6) EndBatchProcessEvent
  scene->EndState(vtkDMMLScene::ImportState);
  scene->EndState(vtkDMMLScene::RestoreState);

  if (scene->IsBatchProcessing() != false ||
      scene->IsRestoring() != false ||
      scene->IsImporting() != false ||
      callback->CalledEvents.size() != 3 ||
      callback->CalledEvents[vtkDMMLScene::EndImportEvent] != 1 ||
      callback->CalledEvents[vtkDMMLScene::EndRestoreEvent] != 1 ||
      callback->CalledEvents[vtkDMMLScene::EndBatchProcessEvent] != 1 ||
      callback->LastEventMTime[vtkDMMLScene::EndImportEvent] >
      callback->LastEventMTime[vtkDMMLScene::EndRestoreEvent] ||
      callback->LastEventMTime[vtkDMMLScene::EndRestoreEvent] >
      callback->LastEventMTime[vtkDMMLScene::EndBatchProcessEvent])
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkDMMLScene::EndImportEvent] << " "
              << callback->CalledEvents[vtkDMMLScene::EndRestoreEvent] << " "
              << callback->CalledEvents[vtkDMMLScene::EndBatchProcessEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  return EXIT_SUCCESS;
}
