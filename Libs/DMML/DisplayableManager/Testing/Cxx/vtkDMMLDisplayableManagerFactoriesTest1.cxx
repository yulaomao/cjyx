
// DMMLDisplayableManager includes
#include <vtkDMMLDisplayableManagerGroup.h>
#include <vtkDMMLSliceViewDisplayableManagerFactory.h>
#include <vtkDMMLThreeDViewDisplayableManagerFactory.h>
#include <vtkDMMLThreeDViewInteractorStyle.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLTestThreeDViewDisplayableManager.h>
#include <vtkDMMLTestSliceViewDisplayableManager.h>
#include <vtkDMMLTestCustomDisplayableManager.h>

// DMMLLogic includes
#include <vtkDMMLApplicationLogic.h>

// DMML includes
#include <vtkDMMLViewNode.h>
#include <vtkDMMLSliceNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTesting.h>

// STD includes

//----------------------------------------------------------------------------
int vtkDMMLDisplayableManagerFactoriesTest1(int argc, char* argv[])
{
  vtkNew<vtkTesting> testHelper;
  testHelper->AddArguments(argc, const_cast<const char **>(argv));

  vtkDMMLThreeDViewDisplayableManagerFactory * threeDViewFactory = vtkDMMLThreeDViewDisplayableManagerFactory::GetInstance();
  if (!threeDViewFactory)
    {
    std::cerr << "Line " << __LINE__
      << " - Problem with vtkDMMLThreeDViewDisplayableManagerFactory::GetInstance() method"
      << std::endl;
    return EXIT_FAILURE;
    }

  vtkDMMLSliceViewDisplayableManagerFactory * cjyxViewFactory = vtkDMMLSliceViewDisplayableManagerFactory::GetInstance();
  if (!cjyxViewFactory)
    {
    std::cerr << "Line " << __LINE__
      << " - Problem with vtkDMMLThreeDViewDisplayableManagerFactory::GetInstance() method"
      << std::endl;
    return EXIT_FAILURE;
    }

  // DMML Scene
  vtkNew<vtkDMMLScene> scene;

  // DMML Application logic (Add Interaction and Selection node)
  vtkNew<vtkDMMLApplicationLogic> dmmlAppLogic;
  dmmlAppLogic->SetDMMLScene(scene.GetPointer());

  int currentCount = threeDViewFactory->GetRegisteredDisplayableManagerCount();
  if (currentCount != 0)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with threeDViewFactory->GetRegisteredDisplayableManagerCount() method"
        << std::endl;
    std::cerr << "\tcurrentCount:" << currentCount << " - expected: 0" << std::endl;
    return EXIT_FAILURE;
    }

  if (threeDViewFactory->GetRegisteredDisplayableManagerName(-1) != "" ||
      threeDViewFactory->GetRegisteredDisplayableManagerName(0) != "")
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with threeDViewFactory->GetNthRegisteredDisplayableManagerName() method"
        << std::endl;
    }

  // Register displayable manager
  threeDViewFactory->RegisterDisplayableManager("vtkDMMLTestThreeDViewDisplayableManager");
  threeDViewFactory->RegisterDisplayableManager("vtkDMMLTestCustomDisplayableManager");

  cjyxViewFactory->RegisterDisplayableManager("vtkDMMLTestSliceViewDisplayableManager");
  cjyxViewFactory->RegisterDisplayableManager("vtkDMMLTestCustomDisplayableManager");

  if (threeDViewFactory->GetRegisteredDisplayableManagerName(0) != "vtkDMMLTestThreeDViewDisplayableManager" ||
      threeDViewFactory->GetRegisteredDisplayableManagerName(1) != "vtkDMMLTestCustomDisplayableManager")
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with threeDViewFactory->GetNthRegisteredDisplayableManagerName() method"
        << std::endl;
    }

  // Renderer, RenderWindow and Interactor
  vtkNew<vtkRenderer> rr;
  vtkNew<vtkRenderWindow> rw;
  vtkNew<vtkRenderWindowInteractor> ri;
  rw->SetSize(600, 600);
  rw->SetMultiSamples(0); // Ensure to have the same test image everywhere
  rw->AddRenderer(rr.GetPointer());
  rw->SetInteractor(ri.GetPointer());

  // Set Interactor Style
  vtkNew<vtkDMMLThreeDViewInteractorStyle> iStyle;
  ri->SetInteractorStyle(iStyle.GetPointer());

  // ThreeD - Instantiate displayable managers
  vtkDMMLDisplayableManagerGroup * threeDViewGroup = threeDViewFactory->InstantiateDisplayableManagers(rr.GetPointer());
  if (!threeDViewGroup)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with threeDViewFactory->InstantiateDisplayableManagers() method"
        << std::endl;
    std::cerr << "\tgroup should NOT be NULL" << std::endl;
    return EXIT_FAILURE;
    }

  currentCount = threeDViewGroup->GetDisplayableManagerCount();
  if (currentCount != 2)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with threeDViewGroup->GetDisplayableManagerCount() method"
        << std::endl;
    std::cerr << "\tcurrentCount:" << currentCount << " - expected: 2" << std::endl;
    return EXIT_FAILURE;
    }

  // ThreeD - Instantiate and add node to the scene
  vtkNew<vtkDMMLViewNode> viewNode;
  vtkDMMLNode * nodeAdded = scene->AddNode(viewNode.GetPointer());
  if (!nodeAdded)
    {
    std::cerr << "Line " << __LINE__ << " - Failed to add vtkDMMLViewNode" << std::endl;
    return EXIT_FAILURE;
    }

  // ThreeD - Associate displayable node to the group
  threeDViewGroup->SetDMMLDisplayableNode(viewNode.GetPointer());

  // Slice - Instantiate displayable managers
  vtkDMMLDisplayableManagerGroup * sliceViewGroup =
      cjyxViewFactory->InstantiateDisplayableManagers(rr.GetPointer());
  if (!sliceViewGroup)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with sliceViewFactory->InstantiateDisplayableManagers() method"
        << std::endl;
    std::cerr << "\tgroup should NOT be NULL" << std::endl;
    return EXIT_FAILURE;
    }

  currentCount = sliceViewGroup->GetDisplayableManagerCount();
  if (currentCount != 2)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with sliceViewGroup->GetDisplayableManagerCount() method"
        << std::endl;
    std::cerr << "\tcurrentCount:" << currentCount << " - expected: 2" << std::endl;
    return EXIT_FAILURE;
    }

  // Slice - Instantiate and add node to the scene
  vtkNew<vtkDMMLSliceNode> sliceNode;
  sliceNode->SetLayoutName("Red");
  sliceNode->SetName("Red-Axial");
  nodeAdded = scene->AddNode(sliceNode.GetPointer());
  if (!nodeAdded)
    {
    std::cerr << "Line " << __LINE__ << " - Failed to add vtkSliceViewNode" << std::endl;
    return EXIT_FAILURE;
    }

  // Slice - Associate displayable node to the group
  sliceViewGroup->SetDMMLDisplayableNode(sliceNode.GetPointer());

  // Add node to the scene
  vtkNew<vtkDMMLCameraNode> cameraNode;
  scene->AddNode(cameraNode.GetPointer());

  // Check if both displayable manager cought the event
  if (vtkDMMLTestThreeDViewDisplayableManager::NodeAddedCount != 1)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with vtkDMMLTestThreeDViewDisplayableManager::OnDMMLSceneNodeAddedEvent method"
        << std::endl;
    std::cerr << "\tNodeAddedCount - current:" <<
              vtkDMMLTestThreeDViewDisplayableManager::NodeAddedCount
              << "- expected: 1"<< std::endl;
    return EXIT_FAILURE;
    }
  if (vtkDMMLTestSliceViewDisplayableManager::NodeAddedCount != 1)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with vtkDMMLTestSliceViewDisplayableManager::OnDMMLSceneNodeAddedEvent method"
        << std::endl;
    std::cerr << "\tNodeAddedCount - current:" <<
              vtkDMMLTestSliceViewDisplayableManager::NodeAddedCount
              << "- expected: 1"<< std::endl;
    return EXIT_FAILURE;
    }
  if (vtkDMMLTestCustomDisplayableManager::NodeAddedCountSliceView != 1)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with vtkDMMLTestCustomDisplayableManager::OnDMMLSceneNodeAddedEvent method"
        << std::endl;
    std::cerr << "\tNodeAddedCount - current:" <<
              vtkDMMLTestCustomDisplayableManager::NodeAddedCountSliceView
              << "- expected: 1"<< std::endl;
    return EXIT_FAILURE;
    }
  if (vtkDMMLTestCustomDisplayableManager::NodeAddedCountThreeDView != 1)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with vtkDMMLTestCustomDisplayableManager::OnDMMLSceneNodeAddedEvent method"
        << std::endl;
    std::cerr << "\tNodeAddedCount - current:" <<
              vtkDMMLTestCustomDisplayableManager::NodeAddedCountThreeDView
              << "- expected: 1"<< std::endl;
    return EXIT_FAILURE;
    }

  // Reset
  vtkDMMLTestThreeDViewDisplayableManager::NodeAddedCount = 0;
  vtkDMMLTestSliceViewDisplayableManager::NodeAddedCount = 0;
  vtkDMMLTestCustomDisplayableManager::NodeAddedCountSliceView = 0;
  vtkDMMLTestCustomDisplayableManager::NodeAddedCountThreeDView = 0;


  // Load scene
  std::string dataRoot = testHelper->GetDataRoot();
  std::string dmmlFiletoLoad = dataRoot + "/Data/vtkDMMLDisplayableManagerFactoriesTest1-load.dmml";
  scene->SetURL(dmmlFiletoLoad.c_str());
  bool success = scene->Connect() != 0;
  if (!success)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with vtkDMMLScene::Connect" << std::endl;
    return EXIT_FAILURE;
    }

  // Check if scene contains the expected node
  std::vector<vtkDMMLNode*> cameraNodes;
  scene->GetNodesByClass("vtkDMMLCameraNode", cameraNodes);
  if (cameraNodes.size() != 1 || !vtkDMMLCameraNode::SafeDownCast(cameraNodes.at(0)))
    {
    std::cerr << "Line " << __LINE__ << " - Problem with vtkDMMLScene::Import"
              << std::endl << "\tScene CameraNode count - current:"
              << cameraNodes.size() << " - expected: 1" << std::endl;
    return EXIT_FAILURE;
    }

  // Check if both displayable manager cought the event
  if (vtkDMMLTestThreeDViewDisplayableManager::NodeAddedCount != 1)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with vtkDMMLTestThreeDViewDisplayableManager::OnDMMLSceneNodeAddedEvent method"
        << std::endl;
    std::cerr << "\tNodeAddedCount - current:" <<
              vtkDMMLTestThreeDViewDisplayableManager::NodeAddedCount
              << "- expected: 1"<< std::endl;
    return EXIT_FAILURE;
    }
  if (vtkDMMLTestSliceViewDisplayableManager::NodeAddedCount != 1)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with vtkDMMLTestSliceViewDisplayableManager::OnDMMLSceneNodeAddedEvent method"
        << std::endl;
    std::cerr << "\tNodeAddedCount - current:" <<
              vtkDMMLTestSliceViewDisplayableManager::NodeAddedCount
              << "- expected: 1"<< std::endl;
    return EXIT_FAILURE;
    }
  if (vtkDMMLTestCustomDisplayableManager::NodeAddedCountSliceView != 1)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with vtkDMMLTestCustomDisplayableManager::OnDMMLSceneNodeAddedEvent method"
        << std::endl;
    std::cerr << "\tNodeAddedCount - current:" <<
              vtkDMMLTestCustomDisplayableManager::NodeAddedCountSliceView
              << "- expected: 1"<< std::endl;
    return EXIT_FAILURE;
    }
  if (vtkDMMLTestCustomDisplayableManager::NodeAddedCountThreeDView != 1)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with vtkDMMLTestCustomDisplayableManager::OnDMMLSceneNodeAddedEvent method"
        << std::endl;
    std::cerr << "\tNodeAddedCount - current:" <<
              vtkDMMLTestCustomDisplayableManager::NodeAddedCountThreeDView
              << "- expected: 1"<< std::endl;
    return EXIT_FAILURE;
    }


  // Import scene
  std::string dmmlFiletoImport = dataRoot + "/Data/vtkDMMLDisplayableManagerFactoriesTest1-import.dmml";
  scene->SetURL(dmmlFiletoImport.c_str());
  success = scene->Import() != 0;
  if (!success)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with vtkDMMLScene::Import" << std::endl;
    return EXIT_FAILURE;
    }

  // Check if scene contains the expected node
  cameraNodes.clear();
  scene->GetNodesByClass("vtkDMMLCameraNode", cameraNodes);
  if (cameraNodes.size() != 2 || !vtkDMMLCameraNode::SafeDownCast(cameraNodes.at(0)))
    {
    std::cerr << "Line " << __LINE__ << " - Problem with vtkDMMLScene::Import"
              << std::endl << "\tScene CameraNode count - current:"
              << cameraNodes.size() << " - expected: 2" << std::endl;
    return EXIT_FAILURE;
    }

  // Check if both displayable manager cought the event
  if (vtkDMMLTestThreeDViewDisplayableManager::NodeAddedCount != 2)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with vtkDMMLTestThreeDViewDisplayableManager::OnDMMLSceneNodeAddedEvent method"
        << std::endl;
    std::cerr << "\tNodeAddedCount - current:" <<
              vtkDMMLTestThreeDViewDisplayableManager::NodeAddedCount
              << "- expected: 2"<< std::endl;
    return EXIT_FAILURE;
    }
  if (vtkDMMLTestSliceViewDisplayableManager::NodeAddedCount != 2)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with vtkDMMLTestSliceViewDisplayableManager::OnDMMLSceneNodeAddedEvent method"
        << std::endl;
    std::cerr << "\tNodeAddedCount - current:" <<
              vtkDMMLTestSliceViewDisplayableManager::NodeAddedCount
              << "- expected: 2"<< std::endl;
    return EXIT_FAILURE;
    }
  if (vtkDMMLTestCustomDisplayableManager::NodeAddedCountSliceView != 2)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with vtkDMMLTestCustomDisplayableManager::OnDMMLSceneNodeAddedEvent method"
        << std::endl;
    std::cerr << "\tNodeAddedCount - current:" <<
              vtkDMMLTestCustomDisplayableManager::NodeAddedCountSliceView
              << "- expected: 2"<< std::endl;
    return EXIT_FAILURE;
    }
  if (vtkDMMLTestCustomDisplayableManager::NodeAddedCountThreeDView != 2)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with vtkDMMLTestCustomDisplayableManager::OnDMMLSceneNodeAddedEvent method"
        << std::endl;
    std::cerr << "\tNodeAddedCount - current:" <<
              vtkDMMLTestCustomDisplayableManager::NodeAddedCountThreeDView
              << "- expected: 2"<< std::endl;
    return EXIT_FAILURE;
    }

  threeDViewGroup->Delete();
  sliceViewGroup->Delete();

  return EXIT_SUCCESS;
}
