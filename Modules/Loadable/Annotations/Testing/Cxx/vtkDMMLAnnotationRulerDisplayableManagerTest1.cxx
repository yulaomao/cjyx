/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

// DMMLDisplayableManager includes
#include <vtkDMMLCameraDisplayableManager.h>
#include <vtkDMMLDisplayableManagerGroup.h>
#include <vtkDMMLSliceViewDisplayableManagerFactory.h>
#include <vtkDMMLThreeDViewDisplayableManagerFactory.h>
#include <vtkDMMLViewDisplayableManager.h>
#include <vtkDMMLThreeDViewInteractorStyle.h>

// DMMLLogic includes
#include <vtkDMMLApplicationLogic.h>

// DMML includes
#include <vtkDMMLAnnotationRulerNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkErrorCode.h>
#include <vtkInteractorEventRecorder.h>
#include <vtkPNGWriter.h>
#include <vtkRegressionTestImage.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkWindowToImageFilter.h>

#include "DMMLDisplayableManager/vtkDMMLAnnotationDisplayableManager.h"

// STD includes
#include <cstdlib>
#include <iterator>

#include "vtkDMMLCoreTestingMacros.h"

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
  virtual void Execute(vtkObject*, unsigned long , void* )
    {
    this->Renderer->GetRenderWindow()->Render();
    this->RenderRequestCount++;
    //std::cout << "RenderRequestCount [" << this->RenderRequestCount << "]" << std::endl;
    }
protected:
  vtkRenderRequestCallback():Renderer(0), RenderRequestCount(0){}
  vtkRenderer * Renderer;
  int           RenderRequestCount;
};

//----------------------------------------------------------------------------
int vtkDMMLAnnotationRulerDisplayableManagerTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{


  /*
   *
   * Setup the test scenario.
   *
   */
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
  vtkDMMLThreeDViewDisplayableManagerFactory * factoryThreeDView = vtkDMMLThreeDViewDisplayableManagerFactory::New();
  vtkDMMLSliceViewDisplayableManagerFactory * factorySliceView = vtkDMMLSliceViewDisplayableManagerFactory::New();

  // Check if GetRegisteredDisplayableManagerCount returns 0
  if (factoryThreeDView->GetRegisteredDisplayableManagerCount() != 0)
    {
    std::cerr << "Expected RegisteredDisplayableManagerCount: 0" << std::endl;
    std::cerr << "Current RegisteredDisplayableManagerCount:"
        << factoryThreeDView->GetRegisteredDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }

  if (factorySliceView->GetRegisteredDisplayableManagerCount() != 0)
    {
    std::cerr << "Expected RegisteredDisplayableManagerCount: 0" << std::endl;
    std::cerr << "Current RegisteredDisplayableManagerCount:"
        << factorySliceView->GetRegisteredDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }


  factoryThreeDView->RegisterDisplayableManager(
      "vtkDMMLAnnotationRulerDisplayableManager");
  factorySliceView->RegisterDisplayableManager(
      "vtkDMMLAnnotationRulerDisplayableManager");


  /*
   *
   * Check if factory registered the displayable Managers
   *
   */
  // Check if GetRegisteredDisplayableManagerCount returns 1
  if (factoryThreeDView->GetRegisteredDisplayableManagerCount() != 1)
    {
    std::cerr << "Expected RegisteredDisplayableManagerCount: 1" << std::endl;
    std::cerr << "Current RegisteredDisplayableManagerCount:"
        << factoryThreeDView->GetRegisteredDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }
  // Check if GetRegisteredDisplayableManagerCount returns 1
  if (factorySliceView->GetRegisteredDisplayableManagerCount() != 1)
    {
    std::cerr << "Expected RegisteredDisplayableManagerCount: 1" << std::endl;
    std::cerr << "Current RegisteredDisplayableManagerCount:"
        << factorySliceView->GetRegisteredDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }


  vtkDMMLDisplayableManagerGroup * displayableManagerGroupThreeDView =
      factoryThreeDView->InstantiateDisplayableManagers(rr);

  if (!displayableManagerGroupThreeDView)
    {
    std::cerr << "Failed to instantiate Displayable Managers using "
        << "InstantiateDisplayableManagers" << std::endl;
    return EXIT_FAILURE;
    }

  //vtkDMMLDisplayableManagerGroup * displayableManagerGroupSliceView =
   //   factorySliceView->InstantiateDisplayableManagers(rr);

  /*
   *
   * Start testing the actual functionality.
   *
   * Testing plan
   * 1. Create ruler annotation and fire vtkDMMLScene::NodeAddedEvent events through DMMLScene->AddNode
   * 2. Delete ruler annotations and fire vtkDMMLScene::NodeRemovedEvent events through DMMLScene->RemoveNode
   *
   */

  //--------------------------------------------------------------------------------------
  // TEST 1:
  // Add one TextNode to scene and see if widget appears!
  //

  // fail if widgets are in Renderwindow
  if (rr->GetViewProps()->GetNumberOfItems()>0) {
    std::cerr << "Expected number of items in renderer: 0" << std::endl;
    std::cerr << "Current number of items in renderer: " << rr->GetViewProps()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
  }

  double worldCoordinates1[4];
  double worldCoordinates2[4];

  worldCoordinates1[0] = 10;
  worldCoordinates1[1] = 10;
  worldCoordinates1[2] = 0;
  worldCoordinates1[3] = 1;

  worldCoordinates1[0] = 30;
  worldCoordinates1[1] = 30;
  worldCoordinates1[2] = 0;
  worldCoordinates1[3] = 1;



  // create the DMML node
  double distance = sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2));

  vtkDMMLAnnotationRulerNode *rulerNode = vtkDMMLAnnotationRulerNode::New();

  rulerNode->SetPosition1(worldCoordinates1);
  rulerNode->SetPosition2(worldCoordinates2);
  rulerNode->SetDistanceMeasurement(distance);

  rulerNode->Initialize(scene);

  rulerNode->SetName(rulerNode->GetScene()->GetUniqueNameByString("AnnotationRuler"));

  // fail if widget did not appear
  if (rr->GetViewProps()->GetNumberOfItems()!=1) {
    std::cerr << "Expected number of items in renderer: 1" << std::endl;
    std::cerr << "Current number of items in renderer: " << rr->GetViewProps()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
  }

  //--------------------------------------------------------------------------------------
  // TEST 2:
  // Delete ruler annotations and fire vtkDMMLScene::NodeRemovedEvent events through DMMLScene->RemoveNode
  //
  scene->RemoveNode(rulerNode);

  // fail if widget did not disappear
  if (rr->GetViewProps()->GetNumberOfItems()!=0) {
    std::cerr << "Expected number of items in renderer: 0" << std::endl;
    std::cerr << "Current number of items in renderer: " << rr->GetViewProps()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
  }

  // cleanup
  rulerNode->Delete();

  applicationLogic->Delete();
  scene->RemoveNode(viewNode);
  viewNode->Delete();

  factoryThreeDView->Delete();
  factorySliceView->Delete();

  scene->Delete();
  rr->Delete();
  rw->Delete();
  ri->Delete();


  return EXIT_SUCCESS;
}

