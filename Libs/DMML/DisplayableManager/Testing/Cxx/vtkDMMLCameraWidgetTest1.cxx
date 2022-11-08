/// Test if mouse gestures to move the camera works well
/// even if the camera is transformed with an anisotropic scaling transform.

// DMMLDisplayableManager includes
#include <vtkDMMLApplicationLogic.h>
#include <vtkDMMLCameraDisplayableManager.h>
#include <vtkDMMLCameraWidget.h>
#include <vtkDMMLDisplayableManagerGroup.h>
#include <vtkDMMLInteractionEventData.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLThreeDViewDisplayableManagerFactory.h>
#include <vtkDMMLThreeDViewInteractorStyle.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkErrorCode.h>
#include <vtkInteractorEventRecorder.h>
#include <vtkNew.h>
#include <vtkPNGWriter.h>
#include <vtkRegressionTestImage.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkWindowToImageFilter.h>
#include <vtkTransform.h>
#include <vtkCamera.h>

#include "vtkDMMLCoreTestingMacros.h"

namespace
{

// Simulate translation of the screen (left mouse click and drage)
void doTranslate(vtkRenderer* renderer, vtkDMMLViewNode* viewNode, vtkDMMLCameraWidget* cameraWidget)
{
  // Simulate click mouse left button
  vtkNew<vtkDMMLInteractionEventData> ev;
  ev->SetRenderer(renderer);
  ev->SetViewNode(viewNode);
  ev->SetType(12);
  ev->SetKeySym("Shift_L");
  ev->SetModifiers(1);  // Shift btn
  ev->SetMouseMovedSinceButtonDown(false);
  double wp[3];
  wp[0] = 0;
  wp[1] = 0;
  wp[2] = 0;
  int dp[2];
  dp[0] = 0;
  dp[1] = 0;
  ev->SetWorldPosition(wp);
  ev->SetDisplayPosition(dp);
  cameraWidget->ProcessInteractionEvent(ev);

  // Simulate drag
  ev->SetType(26);
  ev->SetMouseMovedSinceButtonDown(true);
  wp[0] = 10;
  wp[1] = 0;
  wp[2] = 0;
  dp[0] = 100;
  dp[1] = 0;
  ev->SetWorldPosition(wp);
  ev->SetDisplayPosition(dp);
  cameraWidget->ProcessInteractionEvent(ev);

  // Simulate button up
  ev->SetType(13);
  ev->SetMouseMovedSinceButtonDown(true);
  cameraWidget->ProcessInteractionEvent(ev);
}

}

//----------------------------------------------------------------------------
int vtkDMMLCameraWidgetTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  // Renderer, RenderWindow and Interactor
  vtkNew<vtkRenderer> rr;
  vtkNew<vtkRenderWindow> rw;
  vtkNew<vtkRenderWindowInteractor> ri;
  rw->SetSize(600, 600);

  rw->SetMultiSamples(0); // Ensure to have the same test image everywhere

  rw->AddRenderer(rr);
  rw->SetInteractor(ri.GetPointer());

  // Set Interactor Style
  vtkNew<vtkDMMLThreeDViewInteractorStyle> iStyle;
  ri->SetInteractorStyle(iStyle.GetPointer());

  // DMML scene
  vtkNew<vtkDMMLScene> scene;

  // Application logic - Handle creation of vtkDMMLSelectionNode and vtkDMMLInteractionNode
  vtkNew<vtkDMMLApplicationLogic> applicationLogic;
  applicationLogic->SetDMMLScene(scene.GetPointer());

  // Add ViewNode
  vtkNew<vtkDMMLViewNode> viewNode;
  viewNode->SetLayoutName("1");
  viewNode->SetLayoutLabel("1");
  vtkDMMLNode* nodeAdded = scene->AddNode(viewNode.GetPointer());
  CHECK_NOT_NULL(nodeAdded);

  // Factory
  vtkNew<vtkDMMLThreeDViewDisplayableManagerFactory> factory;
  CHECK_INT(factory->GetRegisteredDisplayableManagerCount(), 0);

  factory->RegisterDisplayableManager("vtkDMMLCameraDisplayableManager");
  factory->RegisterDisplayableManager("vtkDMMLViewDisplayableManager");
  CHECK_INT(factory->GetRegisteredDisplayableManagerCount(), 2);

  // Instantiate DisplayableManagerGroup
  vtkSmartPointer<vtkDMMLDisplayableManagerGroup> group = vtkSmartPointer<vtkDMMLDisplayableManagerGroup>::Take(
    factory->InstantiateDisplayableManagers(rr.GetPointer()));
  CHECK_NOT_NULL(group);

  vtkDMMLCameraDisplayableManager * cameraDisplayableManager =vtkDMMLCameraDisplayableManager::SafeDownCast(
    group->GetDisplayableManagerByClassName("vtkDMMLCameraDisplayableManager"));
  CHECK_NOT_NULL(cameraDisplayableManager);

  vtkDMMLCameraWidget* cameraWidget = cameraDisplayableManager->GetCameraWidget();
  CHECK_NOT_NULL(cameraWidget);

  vtkNew<vtkDMMLCameraNode> cameraNode;
  cameraNode->SetPosition(0, 50, 0);
  cameraWidget->SetRenderer(rr);
  cameraWidget->SetCameraNode(cameraNode);
  rr->SetActiveCamera(cameraWidget->GetCameraNode()->GetCamera());

  // Calculate camera shift before changing aspect ratio
  double cameraStartPos[3] = { 0.0, 0.0, 0.0 };
  double cameraEndPos[3] = { 0.0, 0.0, 0.0 };
  double cameraShift[3] = {0.0, 0.0, 0.0};
  cameraWidget->GetCameraNode()->GetPosition(cameraStartPos);
  doTranslate(rr, viewNode, cameraWidget);
  cameraWidget->GetCameraNode()->GetPosition(cameraEndPos);
  vtkMath::Subtract(cameraEndPos, cameraStartPos, cameraShift);

  // Change aspect ratio
  vtkNew<vtkTransform> transform;
  transform->Scale(2, 1, 1);
  cameraWidget->GetCameraNode()->GetCamera()->SetModelTransformMatrix(transform->GetMatrix());

  // Calculate camera shift after changing aspect ratio
  double cameraStartPosAnisotropic[3] = { 0.0, 0.0, 0.0 };
  double cameraEndPosAnisotropic[3] = { 0.0, 0.0, 0.0 };
  double cameraShiftAnisotropic[3] = { 0.0, 0.0, 0.0 };
  cameraWidget->GetCameraNode()->GetPosition(cameraStartPosAnisotropic);
  doTranslate(rr, viewNode, cameraDisplayableManager->GetCameraWidget());
  cameraWidget->GetCameraNode()->GetPosition(cameraEndPosAnisotropic);
  vtkMath::Subtract(cameraEndPosAnisotropic, cameraStartPosAnisotropic, cameraShiftAnisotropic);

  // Check if camera shift is the same even if the camera is anisotropically scaled
  for (int i = 0; i < 3; i++)
    {
    CHECK_DOUBLE_TOLERANCE(cameraShiftAnisotropic[i], cameraShift[i], 1.0e-5);
    }

  return EXIT_SUCCESS;
}
