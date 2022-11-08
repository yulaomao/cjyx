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
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// DMMLDisplayableManager includes
#include <vtkDMMLThreeDViewDisplayableManagerFactory.h>
#include <vtkDMMLDisplayableManagerGroup.h>
#include <vtkDMMLAnnotationFiducialDisplayableManager.h>
#include <vtkDMMLThreeDViewInteractorStyle.h>

// DMMLLogic includes
#include <vtkDMMLApplicationLogic.h>

// DMML includes
#include <vtkDMMLCoreTestingMacros.h>
#include <vtkDMMLInteractionNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCamera.h>

// DisplayableManager initialization
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkCjyxAnnotationsModuleDMMLDisplayableManager)

//----------------------------------------------------------------------------
class vtkRenderRequestCallback : public vtkCommand
{
public:
  static vtkRenderRequestCallback *New()
    { return new vtkRenderRequestCallback; }
  void SetRenderer(vtkRenderer *renderer)
    { this->Renderer =  renderer; }
  int GetRenderRequestCount()
    { return this->RenderRequestCount; }
  void Execute(vtkObject*, unsigned long , void* ) override
    {
    this->Renderer->GetRenderWindow()->Render();
    this->RenderRequestCount++;
    //std::cout << "RenderRequestCount [" << this->RenderRequestCount << "]" << std::endl;
    }
protected:
  vtkRenderRequestCallback() = default;
  vtkRenderer * Renderer{nullptr};
  int           RenderRequestCount{0};
};

//----------------------------------------------------------------------------
int vtkDMMLAnnotationDisplayableManagerTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  // Renderer, RenderWindow and Interactor
  vtkRenderer* rr = vtkRenderer::New();
  vtkRenderWindow* rw = vtkRenderWindow::New();
  vtkRenderWindowInteractor* ri = vtkRenderWindowInteractor::New();
  rw->SetSize(600, 600);

  rw->SetMultiSamples(0); // Ensure to have the same test image everywhere

  rw->AddRenderer(rr);
  rw->SetInteractor(ri);

  // Set Interactor Style
  vtkDMMLThreeDViewInteractorStyle * iStyle = vtkDMMLThreeDViewInteractorStyle::New();
  ri->SetInteractorStyle(iStyle);
  iStyle->Delete();

  // DMML scene
  vtkDMMLScene* scene = vtkDMMLScene::New();

  // Application logic - Handle creation of vtkDMMLSelectionNode and vtkDMMLInteractionNode
  vtkDMMLApplicationLogic* applicationLogic = vtkDMMLApplicationLogic::New();
  applicationLogic->SetDMMLScene(scene);

  // Add ViewNode
  vtkDMMLViewNode * viewNode = vtkDMMLViewNode::New();
  vtkDMMLNode * nodeAdded = scene->AddNode(viewNode);
  viewNode->Delete();
  if (!nodeAdded)
    {
    std::cerr << "Failed to add vtkDMMLViewNode" << std::endl;
    return EXIT_FAILURE;
    }

  // Factory
  vtkDMMLThreeDViewDisplayableManagerFactory * factory = vtkDMMLThreeDViewDisplayableManagerFactory::New();

  factory->RegisterDisplayableManager("vtkDMMLCameraDisplayableManager");
  factory->RegisterDisplayableManager("vtkDMMLViewDisplayableManager");
  factory->RegisterDisplayableManager("vtkDMMLAnnotationDisplayableManager");
  factory->RegisterDisplayableManager("vtkDMMLModelDisplayableManager"); // Needed for the coord transform testing/picking

  vtkDMMLDisplayableManagerGroup * displayableManagerGroup =
      factory->InstantiateDisplayableManagers(rr);

  if (!displayableManagerGroup)
    {
    std::cerr << "Failed to instantiate Displayable Managers using "
        << "InstantiateDisplayableManagers" << std::endl;
    return EXIT_FAILURE;
    }

  // RenderRequest Callback
  vtkRenderRequestCallback * renderRequestCallback = vtkRenderRequestCallback::New();
  renderRequestCallback->SetRenderer(rr);
  displayableManagerGroup->AddObserver(vtkCommand::UpdateEvent, renderRequestCallback);

  // Assign ViewNode
  displayableManagerGroup->SetDMMLDisplayableNode(viewNode);

  // Check if RenderWindowInteractor has NOT been changed
  if (displayableManagerGroup->GetInteractor() != ri)
    {
    std::cerr << "Expected RenderWindowInteractor:" << ri << std::endl;
    std::cerr << "Current RenderWindowInteractor:"
        << displayableManagerGroup->GetInteractor() << std::endl;
    return EXIT_FAILURE;
    }

  double windowWidth = displayableManagerGroup->GetInteractor()->GetRenderWindow()->GetSize()[0];
  double windowHeight = displayableManagerGroup->GetInteractor()->GetRenderWindow()->GetSize()[1];
  std::cout << "Render window size: " << windowWidth << "x" << windowHeight << std::endl;

  // change to place mode
  vtkDMMLNode *dmmlNode = scene->GetNodeByID("vtkDMMLInteractionNode1");
  vtkDMMLInteractionNode *interactionNode = nullptr;
  if (dmmlNode)
    {
    interactionNode = vtkDMMLInteractionNode::SafeDownCast(dmmlNode);
    interactionNode->SwitchToPersistentPlaceMode();
    }

  // get the fiducial displayable manager so we can test the
  // OnClickInRenderWindow method
  vtkDMMLAbstractDisplayableManager * dm1 =
      displayableManagerGroup->GetDisplayableManagerByClassName("vtkDMMLAnnotationDisplayableManager");
  vtkDMMLAnnotationDisplayableManager *adm = nullptr;
  if (dm1)
    {
    adm = vtkDMMLAnnotationDisplayableManager::SafeDownCast(dm1);
    }
  else
    {
    std::cerr << "Unable to get the annotation displayable manager from the displayble manager group!" << std::endl;
    return EXIT_FAILURE;
    }
  if (adm ==  nullptr)
    {
    std::cerr << "Unable to cast the annotation displayable manager from the displayble manager!" << std::endl;
    return EXIT_FAILURE;
    }
  double worldCoords1[4], worldCoords2[4], worldCoords3[4], worldCoords4[4];
  worldCoords1[0] = worldCoords1[1] = worldCoords1[2] = worldCoords1[3] = 0.0;
  worldCoords2[0] = worldCoords2[1] = worldCoords2[2] = worldCoords2[3] = 0.0;
  worldCoords3[0] = worldCoords3[1] = worldCoords3[2] = worldCoords3[3] = 0.0;
  worldCoords4[0] = worldCoords4[1] = worldCoords4[2] = worldCoords4[3] = 0.0;

  double dispCoords[2];
  dispCoords[0] = dispCoords[1] = 100.0;
  adm->GetDisplayToWorldCoordinates(dispCoords, worldCoords1);
  std::cout << "Display: " << dispCoords[0] << "," << dispCoords[1] << ". World: " << worldCoords1[0] << "," << worldCoords1[1] << "," << worldCoords1[2] << "," << worldCoords1[3] << std::endl;

  dispCoords[0] = 900.0;
  dispCoords[1] = 900.0;
  adm->GetDisplayToWorldCoordinates(dispCoords, worldCoords2);
  std::cout << "Display: " << dispCoords[0] << "," << dispCoords[1] << ". World: " << worldCoords2[0] << "," << worldCoords2[1] << "," << worldCoords2[2] << "," << worldCoords2[3] << std::endl;

  dispCoords[0] = 250.0;
  dispCoords[1] = 750.0;
  adm->GetDisplayToWorldCoordinates(dispCoords, worldCoords3);
  std::cout << "Display: " << dispCoords[0] << "," << dispCoords[1] << ". World: " << worldCoords3[0] << "," << worldCoords3[1] << "," << worldCoords3[2] << "," << worldCoords3[3] << std::endl;

  dispCoords[0] = 800.0;
  dispCoords[1] = 55.0;
  adm->GetDisplayToWorldCoordinates(dispCoords, worldCoords4);
  std::cout << "Display: " << dispCoords[0] << "," << dispCoords[1] << ". World: " << worldCoords4[0] << "," << worldCoords4[1] << "," << worldCoords4[2] << "," << worldCoords4[3] << std::endl;

  // print out distances between resulting world coordinates
  double d12 = sqrt(vtkMath::Distance2BetweenPoints(worldCoords1, worldCoords2));
  std::cout << "Distance between world coords 1 and 2: " << d12 << std::endl;

  double d13 = sqrt(vtkMath::Distance2BetweenPoints(worldCoords1, worldCoords3));
  std::cout << "Distance between world coords 1 and 3: " << d13 << std::endl;

  double d14 = sqrt(vtkMath::Distance2BetweenPoints(worldCoords1, worldCoords4));
  std::cout << "Distance between world coords 1 and 4: " << d14 << std::endl;

  double d23 = sqrt(vtkMath::Distance2BetweenPoints(worldCoords2, worldCoords3));
  std::cout << "Distance between world coords 2 and 3: " << d23 << std::endl;

  double d24 = sqrt(vtkMath::Distance2BetweenPoints(worldCoords2, worldCoords4));
  std::cout << "Distance between world coords 2 and 4: " << d24 << std::endl;

  double d34 = sqrt(vtkMath::Distance2BetweenPoints(worldCoords3, worldCoords4));
  std::cout << "Distance between world coords 3 and 4: " << d34 << std::endl;

  // Interactor style should be vtkDMMLThreeDViewInteractorStyle
  vtkInteractorObserver * currentInteractoryStyle = ri->GetInteractorStyle();
  if (!vtkDMMLThreeDViewInteractorStyle::SafeDownCast(currentInteractoryStyle))
    {
    std::cerr << "Expected interactorStyle: vtkDMMLThreeDViewInteractorStyle" << std::endl;
    std::cerr << "Current RenderWindowInteractor: "
      << (currentInteractoryStyle ? currentInteractoryStyle->GetClassName() : "Null") << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    vtkRenderer *currentDefRenderer = currentInteractoryStyle->GetCurrentRenderer();
    if (!currentDefRenderer)
      {
      std::cerr << "ERROR: current interactor style doesn't have a current renderer! Trying to set it from the  displayable manager group..." << std::endl;
      //return EXIT_FAILURE;
      currentDefRenderer = displayableManagerGroup->GetRenderer();
      currentInteractoryStyle->SetCurrentRenderer(displayableManagerGroup->GetRenderer());
      currentDefRenderer = currentInteractoryStyle->GetCurrentRenderer();
      if (currentDefRenderer == nullptr)
        {
        std::cerr << "ERROR: unable to set the interactor style renderer from the displayble manager group's renderer!" << std::endl;
        return EXIT_FAILURE;
        }
      }
    int *currentSize = currentDefRenderer->GetSize();
    double *viewport = currentDefRenderer->GetViewport();
    std::cout << "Current interactor style's current renderer:" << std::endl;
    if (viewport)
      {
      std::cout << "\tviewport = " << viewport[0] << ", " << viewport[1] << ", " << viewport[2] << ", " << viewport[3] << std::endl;
      }
    if (currentSize)
      {
      std::cout << "\tcurrentSize = " << currentSize[0] << ", " << currentSize[1] << std::endl;
      }
    }

  // check out the camera
  vtkCamera *cam = displayableManagerGroup->GetRenderer()->GetActiveCamera();
  if (cam)
    {
    double *campos = cam->GetPosition();
    double *camfocal = cam->GetFocalPoint();
    double *camviewup = cam->GetViewUp();
    std::cout << "Current Camera:" << std::endl;
    if (campos)
      {
      std::cout << "\tPos = " << campos[0] << ", " << campos[1] << ", " << campos[2] << std::endl;
      }
    if (camfocal)
      {
      std::cout << "\tFocal = " << camfocal[0] << ", " << camfocal[1] << ", " << camfocal[2] << std::endl;
      }
    if (camviewup)
      {
      std::cout << "\tViewUp = " << camviewup[0] << ", " << camviewup[1] << ", " << camviewup[2] << std::endl;
      }

    // compare to the current camera node?
    dmmlNode = scene->GetNodeByID("vtkDMMLCameraNode1");
    vtkDMMLCameraNode *camnode = nullptr;
    if (dmmlNode)
      {
      camnode = vtkDMMLCameraNode::SafeDownCast(dmmlNode);
      if (camnode)
        {
        /*
          // try setting the position
        double newpos[3];
        newpos[0] = 0.0; newpos[1] = 605.0; newpos[3] = 0.0;
        camnode->SetPosition(newpos);
        */
        double *nodePos = camnode->GetPosition();
        if (nodePos)
          {
          std::cout << "Camera Node:\n\tPosition = " << nodePos[0] << ", " << nodePos[1] << ", " << nodePos[2] << std::endl;
          }
        double *nodeFoc = camnode->GetFocalPoint();
        if (nodeFoc)
          {
          std::cout << "\tFocalPoint = " << nodeFoc[0] << ", " << nodeFoc[1] << ", " << nodeFoc[2] << std::endl;
          }
        double *nodeViewup = camnode->GetViewUp();
        if (nodeViewup)
          {
          std::cout << "\tViewUp = " << nodeViewup[0] << ", " << nodeViewup[1] << ", " << nodeViewup[2] << std::endl;
          }
        }
      }
    }

  // try getting the world to display coords now for the last display point
  double worldCoords5[4];
  worldCoords5[0] = worldCoords5[1] = worldCoords5[2] = worldCoords5[3] = 0.0;
  adm->GetDisplayToWorldCoordinates(dispCoords, worldCoords5);
  std::cout << "Last Check: Display: " << dispCoords[0] << "," << dispCoords[1] << ". World: " << worldCoords5[0] << "," << worldCoords5[1] << "," << worldCoords5[2] << "," << worldCoords5[3] << std::endl;
  double d45 = sqrt(vtkMath::Distance2BetweenPoints(worldCoords4, worldCoords5));
  std::cout << "Distance between world coords 4 and 5 (should be < epsilon): " << d45 << std::endl;



  renderRequestCallback->Delete();
  if (displayableManagerGroup) { displayableManagerGroup->Delete(); }
  factory->Delete();
  applicationLogic->Delete();
  scene->Delete();
  rr->Delete();
  rw->Delete();
  ri->Delete();

//  return !retval;
  return EXIT_SUCCESS;
}

