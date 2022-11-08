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

// Annotations includes
#include <vtkDMMLAnnotationFiducialDisplayableManager.h>
#include <vtkDMMLAnnotationFiducialNode.h>
#include <vtkCjyxAnnotationModuleLogic.h>

// DMMLDisplayableManager includes
#include <vtkDMMLDisplayableManagerGroup.h>

// DMMLLogic includes
#include <vtkDMMLApplicationLogic.h>

// DMML includes
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkCamera.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>

// STD includes

//----------------------------------------------------------------------------
int vtkDMMLAnnotationDisplayableManagerTest2(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  // Renderer, RenderWindow and Interactor
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindow->SetSize(600, 600);
  renderWindow->SetMultiSamples(0); // Ensure to have the same test image everywhere

  vtkNew<vtkCamera> camera;
  renderer->SetActiveCamera(camera.GetPointer());

  renderWindow->AddRenderer(renderer.GetPointer());
  renderWindow->SetInteractor(renderWindowInteractor.GetPointer());

  // DMML scene
  vtkSmartPointer<vtkDMMLScene> scene =
    vtkSmartPointer<vtkDMMLScene>::New();

  // Application logic - Handle creation of vtkDMMLSelectionNode and vtkDMMLInteractionNode
  vtkSmartPointer<vtkDMMLApplicationLogic> applicationLogic =
    vtkSmartPointer<vtkDMMLApplicationLogic>::New();
  applicationLogic->SetDMMLScene(scene);

  // Add slice node
  vtkNew<vtkDMMLSliceNode> viewNode;
  scene->AddNode(viewNode.GetPointer());

  vtkSmartPointer<vtkCjyxAnnotationModuleLogic> annotationLogic =
    vtkSmartPointer<vtkCjyxAnnotationModuleLogic>::New();
  annotationLogic->SetDMMLScene(scene);

  vtkNew<vtkDMMLDisplayableManagerGroup> displayableManagerGroup;
  displayableManagerGroup->SetRenderer(renderer.GetPointer());
  displayableManagerGroup->SetDMMLDisplayableNode(viewNode.GetPointer());

  vtkNew<vtkDMMLAnnotationFiducialDisplayableManager> annotationDM;
  annotationDM->SetDMMLApplicationLogic(applicationLogic.GetPointer());
  displayableManagerGroup->AddDisplayableManager(annotationDM.GetPointer());

  // Start test
  vtkNew<vtkDMMLAnnotationFiducialNode> fiducialNode;
  double controlPoint[3]={0,0,0};
  fiducialNode->AddControlPoint(controlPoint,0,1);
  /// Initializing a node should automatically create a display node
  fiducialNode->Initialize(scene);
  if (fiducialNode->GetNumberOfDisplayNodes() == 0)
    {
    std::cerr << "No displayNode created for fiducialNode" << std::endl;
    return EXIT_FAILURE;
    }
  /* FIXME
   * Instead of Initialize. AddNode should be enough.
   * The displayable manager would listen to the scene, and create a display
   * node automatically.
  scene->AddNode(fiducialNode.GetPointer());
  if (fiducialNode->GetNumberOfDisplayNodes() == 0)
    {
    std::cerr << "No displayNode created for fiducialNode" << std::endl;
    return EXIT_FAILURE;
    }
  */

  /* FIXME
  int numberOfNodes = scene->GetNumberOfNodes();
  vtkNew<vtkDMMLAnnotationFiducialNode> fiducialNode2;
  /// Initializing a node should automatically create a display node
  fiducialNode2->Initialize(scene);
  scene->RemoveNode(fiducialNode2.GetPointer());
  if (numberOfNodes != scene->GetNumberOfNodes())
    {
    std::cerr << "Cleaning up a fiducial node leaves side nodes" << std::endl
              << "  - Number of nodes expected: " << numberOfNodes << std::endl
              << "  - Actual number of nodes:" << scene->GetNumberOfNodes()
              << std::endl;
    return EXIT_FAILURE;
    }
  */

  annotationDM->SetDMMLApplicationLogic(nullptr);
  applicationLogic = nullptr;
  scene = nullptr;

  return EXIT_SUCCESS;
}

