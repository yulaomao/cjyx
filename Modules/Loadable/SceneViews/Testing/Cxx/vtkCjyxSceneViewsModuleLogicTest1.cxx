//#include <vtkCjyxConfigure.h>
#include <vtkCjyxSceneViewsModuleLogic.h>

// DMML includes
#include "vtkDMMLCoreTestingMacros.h"
#include <vtkDMMLScene.h>
#include <vtkDMMLSceneViewNode.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkTestingOutputWindow.h>


int vtkCjyxSceneViewsModuleLogicTest1(int , char * [] )
{

  vtkNew<vtkDMMLScene> scene;

  vtkNew<vtkCjyxSceneViewsModuleLogic> logic;
  EXERCISE_BASIC_OBJECT_METHODS(logic.GetPointer());

  // should fail, no scene
  std::cout << "CreateSceneView with no dmml scene or screen shot" << std::endl;
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  logic->CreateSceneView("SceneViewTest0", "this is a scene view", 0, nullptr);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  logic->SetDMMLScene(scene.GetPointer());
  CHECK_EXIT_SUCCESS(vtkDMMLCoreTestingUtilities::ExerciseBasicObjectMethods( logic.GetPointer() ));

  std::cout << "CreateSceneView with no screenshot" << std::endl;
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  logic->CreateSceneView("SceneViewTest1", "this is a scene view", 0, nullptr);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  // should pass w/screen shot
  vtkSmartPointer< vtkImageData > screenShot = vtkSmartPointer< vtkImageData >::New();
  std::cout << "CreateSceneView with no name or description, with screen shot" << std::endl;
  logic->CreateSceneView("", "", 1, screenShot);

  // give a name
  std::cout << "CreateSceneView with name, no description, with screen shot" << std::endl;
  logic->CreateSceneView("SceneViewTest2", "", 2, screenShot);

  std::cout << "DMML Scene has " << scene->GetNumberOfNodesByClass("vtkDMMLSceneViewNode") << " scene view nodes" << std::endl;

  std::string url = std::string("SceneViewsModuleTest.dmml");
  scene->SetURL(url.c_str());
  std::cout << "Writing DMML scene " << url.c_str() << std::endl;
  scene->Commit();

  // now reload it
  scene->SetURL(url.c_str());
  std::cout << "Reading DMML scene " << scene->GetURL() << std::endl;
  scene->Connect();
  std::cout << "After reading in DMML Scene " << url.c_str() << std::endl;
  std::cout << "\tscene has " << scene->GetNumberOfNodesByClass("vtkDMMLSceneViewNode") << " scene view nodes" << std::endl;

  logic->SetDMMLScene(scene.GetPointer());
  // test trying to remove a null node
  std::cout << "Trying to remove a null node." << std::endl;
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  logic->RemoveSceneViewNode(nullptr);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  // add a node to remove
  logic->CreateSceneView("SceneViewTestToRemove", "this is a scene view to remove", 0, screenShot);
  vtkCollection *col = scene->GetNodesByClassByName("vtkDMMLSceneViewNode", "SceneViewTestToRemove");
  if (col && col->GetNumberOfItems() > 0)
    {
    vtkDMMLSceneViewNode *nodeToRemove = vtkDMMLSceneViewNode::SafeDownCast(col->GetItemAsObject(0));
    if (nodeToRemove)
      {
      // now remove one of the nodes
      logic->RemoveSceneViewNode(nodeToRemove);
      std::cout << "After adding and removing a scene view node, scene has " << scene->GetNumberOfNodesByClass("vtkDMMLSceneViewNode") << " scene view nodes" << std::endl;

      }
     else
      {
      std::cerr << "Error getting a scene view node to remove" << std::endl;
      return EXIT_FAILURE;
      }
    }
  else
    {
    std::cerr << "Error adding and finding a node to remove" << std::endl;
    return EXIT_FAILURE;
    }
   col->RemoveAllItems();
   col->Delete();
  return EXIT_SUCCESS;
}



