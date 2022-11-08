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
#include "vtkDMMLCameraNode.h"
#include "vtkDMMLInteractionNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSceneEventRecorder.h"
#include "vtkDMMLSceneViewNode.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

//---------------------------------------------------------------------------
int vtkDMMLSceneViewNodeEventsTest(
  int vtkNotUsed(argc), char * vtkNotUsed(argv)[])
{
  vtkNew<vtkDMMLScene> scene;

  vtkNew<vtkDMMLCameraNode> cameraNode;
  scene->AddNode(cameraNode.GetPointer());
  vtkNew<vtkDMMLInteractionNode> interactionNode;
  interactionNode->SetPlaceModePersistence(0);
  scene->AddNode(interactionNode.GetPointer());

  vtkNew<vtkDMMLSceneViewNode> sceneViewNode;
  sceneViewNode->SetScene(scene.GetPointer());
  sceneViewNode->StoreScene();

  // Remove the cameraNode but keep the interaction node (singleton)
  scene->Clear(0);
  // Change the interaction node
  interactionNode->SetPlaceModePersistence(1);

  if (scene->GetNumberOfNodesByClass("vtkDMMLCameraNode") != 0 ||
      scene->GetNumberOfNodesByClass("vtkDMMLInteractionNode") != 1)
    {
    std::cerr << "Camera node not removed after vtkDMMLScene::Clear()"
              << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkDMMLSceneEventRecorder> callback;
  scene->AddObserver(vtkCommand::AnyEvent, callback.GetPointer());

  // Add the camera node back
  // The following (high level) happens in the scene:
  // vtkDMMLScene::StartState(RestoreState);
  // vtkDMMLScene::AddNode(savedCameraNode);
  // vtkDMMLScene::EndState(RestoreState);
  sceneViewNode->RestoreScene();

  if (scene->GetNumberOfNodesByClass("vtkDMMLCameraNode") != 1 ||
      scene->GetNumberOfNodesByClass("vtkDMMLInteractionNode") != 1)
    {
    std::cerr << "Camera or interaction nodes not restored " << std::endl;
    return EXIT_FAILURE;
    }

  if (callback->CalledEvents.size() != 6 ||
      callback->CalledEvents[vtkDMMLScene::StartBatchProcessEvent] != 1 ||
      callback->CalledEvents[vtkDMMLScene::StartRestoreEvent] != 1 ||
      callback->CalledEvents[vtkDMMLScene::NodeAboutToBeAddedEvent] != 1 ||
      callback->CalledEvents[vtkDMMLScene::NodeAddedEvent] != 1 ||
      callback->CalledEvents[vtkDMMLScene::EndRestoreEvent] != 1 ||
      callback->CalledEvents[vtkDMMLScene::EndBatchProcessEvent] != 1)
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkDMMLScene::StartBatchProcessEvent] << " "
              << callback->CalledEvents[vtkDMMLScene::StartRestoreEvent] << " "
              << callback->CalledEvents[vtkDMMLScene::NodeAboutToBeAddedEvent] << " "
              << callback->CalledEvents[vtkDMMLScene::NodeAddedEvent] << " "
              << callback->CalledEvents[vtkDMMLScene::EndRestoreEvent] << " "
              << callback->CalledEvents[vtkDMMLScene::EndBatchProcessEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
