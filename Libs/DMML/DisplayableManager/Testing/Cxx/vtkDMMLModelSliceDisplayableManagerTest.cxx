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
#include <vtkDMMLModelSliceDisplayableManager.h>
#include <vtkDMMLModelNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceNode.h>

// VTK includes
#include <vtkCamera.h>
#include <vtkErrorCode.h>
#include <vtkImageData.h>
#include <vtkInteractorEventRecorder.h>
#include <vtkNew.h>
#include <vtkPNGWriter.h>
#include <vtkRegressionTestImage.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkWindowToImageFilter.h>

// STD includes
bool TestBatchRemoveDisplayNode();

//----------------------------------------------------------------------------
int vtkDMMLModelSliceDisplayableManagerTest(int vtkNotUsed(argc),
                                            char* vtkNotUsed(argv)[])
{
  bool res = true;
  res = TestBatchRemoveDisplayNode() && res;
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkRenderWindow> CreateRenderWindow()
{
  // Renderer, RenderWindow and Interactor
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindow->SetSize(600, 600);
  renderWindow->SetMultiSamples(0); // Ensure to have the same test image everywhere

  renderWindow->AddRenderer(renderer.GetPointer());
  renderWindow->SetInteractor(renderWindowInteractor.GetPointer());
  return vtkSmartPointer<vtkRenderWindow>(renderWindow.GetPointer());
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkDMMLDisplayableManagerGroup> CreateDisplayableManager(
  vtkDMMLScene* scene, vtkRenderer* renderer)
{
  // Application logic - Handle creation of vtkDMMLSelectionNode and vtkDMMLInteractionNode
  vtkNew<vtkDMMLApplicationLogic> applicationLogic;
  applicationLogic->SetDMMLScene(scene);

  // Add ViewNode
  vtkNew<vtkDMMLSliceNode> sliceNode;
  sliceNode->SetLayoutName("Red");
  scene->AddNode(sliceNode.GetPointer());

  vtkNew<vtkDMMLDisplayableManagerGroup> displayableManagerGroup;
  displayableManagerGroup->SetRenderer(renderer);
  displayableManagerGroup->SetDMMLDisplayableNode(sliceNode.GetPointer());

  vtkNew<vtkDMMLModelSliceDisplayableManager> displayableManager;
  displayableManager->SetDMMLApplicationLogic(applicationLogic.GetPointer());
  displayableManagerGroup->AddDisplayableManager(displayableManager.GetPointer());
  displayableManagerGroup->GetInteractor()->Initialize();
  return displayableManagerGroup.GetPointer();
}

//----------------------------------------------------------------------------
bool TestBatchRemoveDisplayNode()
{
  vtkSmartPointer<vtkRenderWindow> renderWindow = CreateRenderWindow();
  vtkNew<vtkDMMLScene> scene;
  vtkSmartPointer<vtkDMMLDisplayableManagerGroup> displayableManagerGroup =
    CreateDisplayableManager(scene.GetPointer(),
                             renderWindow->GetRenderers()->GetFirstRenderer());

  // We observe the model display nodes and we make sure only the dmmlscene
  // has a reference on them (ref count is 1)
  vtkDMMLModelDisplayNode* modelDisplayNode =
    vtkDMMLModelDisplayNode::New();
  scene->AddNode(modelDisplayNode);
  modelDisplayNode->Delete();

  vtkDMMLModelNode* modelNode = vtkDMMLModelNode::New();
  vtkNew<vtkSphereSource> sphereSource;
  sphereSource->SetRadius(10.);
  sphereSource->Update();

  modelNode->SetPolyDataConnection(sphereSource->GetOutputPort());
  modelNode->AddAndObserveDisplayNodeID(modelDisplayNode->GetID());
  scene->AddNode(modelNode);
  modelNode->Delete();

  // and we delete them
  scene->StartState(vtkDMMLScene::BatchProcessState);
  scene->RemoveNode(modelNode);
  scene->RemoveNode(modelDisplayNode);
  scene->EndState(vtkDMMLScene::BatchProcessState);

  // We trigger an update here (UpdateSliceNode) to make sure the displayable
  // manager doesn't use the deleted nodes.
  vtkDMMLNode* sliceNode = scene->GetNodeByID("vtkDMMLSliceNodeRed");
  sliceNode->Modified();
  return true;
}

