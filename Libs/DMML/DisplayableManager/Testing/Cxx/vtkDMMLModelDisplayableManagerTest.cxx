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

// DMMLDisplayableManager includes
#include <vtkDMMLDisplayableManagerGroup.h>
#include <vtkDMMLThreeDViewInteractorStyle.h>

// DMMLLogic includes
#include <vtkDMMLApplicationLogic.h>

// DMML includes
#include <vtkDMMLModelDisplayNode.h>
#include <vtkDMMLModelDisplayableManager.h>
#include <vtkDMMLModelNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkCamera.h>
#include <vtkErrorCode.h>
#include <vtkImageData.h>
#include <vtkInteractorEventRecorder.h>
#include <vtkNew.h>
#include <vtkPNGWriter.h>
#include <vtkRegressionTestImage.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSphereSource.h>
#include <vtkWindowToImageFilter.h>

// STD includes

const char vtkDMMLModelDisplayableManagerTest1EventLog[] =
"# StreamVersion 1\n";

//----------------------------------------------------------------------------
int vtkDMMLModelDisplayableManagerTest(int argc, char* argv[])
{
  // Renderer, RenderWindow and Interactor
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindow->SetSize(600, 600);
  renderWindow->SetMultiSamples(0); // Ensure to have the same test image everywhere

  renderWindow->AddRenderer(renderer.GetPointer());
  renderWindow->SetInteractor(renderWindowInteractor.GetPointer());

  // Set Interactor Style
  //vtkNew<vtkDMMLThreeDViewInteractorStyle> iStyle;
  //renderWindowInteractor->SetInteractorStyle(iStyle.GetPointer());

  // move back far enough to see the reformat widgets
  //renderer->GetActiveCamera()->SetPosition(0,0,-500.);

  // DMML scene
  vtkDMMLScene* scene = vtkDMMLScene::New();

  // Application logic - Handle creation of vtkDMMLSelectionNode and vtkDMMLInteractionNode
  vtkDMMLApplicationLogic* applicationLogic = vtkDMMLApplicationLogic::New();
  applicationLogic->SetDMMLScene(scene);

  // Add ViewNode
  vtkNew<vtkDMMLViewNode> viewNode;
  scene->AddNode(viewNode.GetPointer());

  vtkNew<vtkDMMLDisplayableManagerGroup> displayableManagerGroup;
  displayableManagerGroup->SetRenderer(renderer.GetPointer());
  displayableManagerGroup->SetDMMLDisplayableNode(viewNode.GetPointer());

  vtkNew<vtkDMMLModelDisplayableManager> vrDisplayableManager;
  vrDisplayableManager->SetDMMLApplicationLogic(applicationLogic);
  displayableManagerGroup->AddDisplayableManager(vrDisplayableManager.GetPointer());
  displayableManagerGroup->GetInteractor()->Initialize();

  vtkNew<vtkDMMLModelNode> modelNode;
  vtkNew<vtkSphereSource> sphereSource;
  sphereSource->SetRadius(10.);
  sphereSource->Update();
  modelNode->SetPolyDataConnection(sphereSource->GetOutputPort());

  scene->AddNode(modelNode.GetPointer());

  vtkNew<vtkDMMLModelDisplayNode> modelDisplayNode;
  scene->AddNode(modelDisplayNode.GetPointer());

  modelNode->AddAndObserveDisplayNodeID(modelDisplayNode->GetID());

  // TODO: Automatically move the camera (simulating movements)
  // to have a good screenshot.
  renderer->SetBackground(0, 169. / 255, 79. /255);
  renderer->SetBackground2(0, 83. / 255, 155. /255);
  renderer->SetGradientBackground(true);
  renderer->ResetCamera();

  // Event recorder
  bool disableReplay = false, record = false, screenshot = false;
  for (int i = 0; i < argc; i++)
    {
    disableReplay |= (strcmp("--DisableReplay", argv[i]) == 0);
    record        |= (strcmp("--Record", argv[i]) == 0);
    screenshot    |= (strcmp("--Screenshot", argv[i]) == 0);
    }
  vtkNew<vtkInteractorEventRecorder> recorder;
  recorder->SetInteractor(displayableManagerGroup->GetInteractor());
  if (!disableReplay)
    {
    if (record)
      {
      std::cout << "Recording ..." << std::endl;
      recorder->SetFileName("vtkInteractorEventRecorder.log");
      recorder->On();
      recorder->Record();
      }
    else
      {
      // Play
      recorder->ReadFromInputStringOn();
      recorder->SetInputString(vtkDMMLModelDisplayableManagerTest1EventLog);
      recorder->Play();
      }
    }

  int retval = vtkRegressionTestImageThreshold(renderWindow.GetPointer(), 85.0);
  if ( record || retval == vtkRegressionTester::DO_INTERACTOR)
    {
    displayableManagerGroup->GetInteractor()->Initialize();
    displayableManagerGroup->GetInteractor()->Start();
    }

  if (record || screenshot)
    {
    vtkNew<vtkWindowToImageFilter> windowToImageFilter;
    windowToImageFilter->SetInput(renderWindow.GetPointer());
    windowToImageFilter->SetScale(1, 1); //set the resolution of the output image
    windowToImageFilter->Update();

    vtkNew<vtkTesting> testHelper;
    testHelper->AddArguments(argc, const_cast<const char **>(argv));

    vtkStdString screenshootFilename = testHelper->GetDataRoot();
    screenshootFilename += "/Baseline/vtkDMMLCameraDisplayableManagerTest1.png";
    vtkNew<vtkPNGWriter> writer;
    writer->SetFileName(screenshootFilename.c_str());
    writer->SetInputConnection(windowToImageFilter->GetOutputPort());
    writer->Write();
    std::cout << "Saved screenshot: " << screenshootFilename << std::endl;
    }

  vrDisplayableManager->SetDMMLApplicationLogic(nullptr);
  applicationLogic->Delete();
  scene->Delete();

  return !retval;
}

