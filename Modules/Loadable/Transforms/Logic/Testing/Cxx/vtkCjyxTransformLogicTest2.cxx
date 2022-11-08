/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

// Logic includes
#include "vtkCjyxTransformLogic.h"

// DMML includes
#include "vtkDMMLModelNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLTransformNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkPolyDataReader.h>

namespace
{
//-----------------------------------------------------------------------------
vtkSmartPointer<vtkDMMLModelNode> LoadModelInScene
(const char* filepath, vtkDMMLScene* scene)
{
  vtkNew<vtkPolyDataReader> reader;
  reader->SetFileName(filepath);
  reader->Update();

  assert(reader->GetOutput());
  vtkSmartPointer<vtkDMMLModelNode> model =
    vtkSmartPointer<vtkDMMLModelNode>::New();
  model->SetAndObservePolyData(reader->GetOutput());
  scene->AddNode(model);
  return model;
}

} // end namespace

//-----------------------------------------------------------------------------
int vtkCjyxTransformLogicTest2(int argc, char * argv [])
{
  if(argc < 2)
    {
    std::cerr << "Missing transform file name." << std::endl;
    return EXIT_FAILURE;
    }

  //
  // Scene setup
  vtkDMMLScene* scene = vtkDMMLScene::New();

  // argv1 == cube.vtk
  vtkSmartPointer<vtkDMMLModelNode> singleChild = LoadModelInScene(argv[1], scene);

  vtkSmartPointer<vtkDMMLModelNode> child1 = LoadModelInScene(argv[1], scene);
  vtkSmartPointer<vtkDMMLModelNode> child2 = LoadModelInScene(argv[1], scene);

  vtkNew<vtkDMMLTransformNode> childTransform;
  scene->AddNode(childTransform.GetPointer());
  vtkSmartPointer<vtkDMMLModelNode> grandChild1 = LoadModelInScene(argv[1], scene);
  vtkSmartPointer<vtkDMMLModelNode> grandChild2 = LoadModelInScene(argv[1], scene);
  vtkSmartPointer<vtkDMMLModelNode> child3 = LoadModelInScene(argv[1], scene);

  // Transform with no descendant
  vtkNew<vtkDMMLTransformNode> childlessTransform;
  scene->AddNode(childlessTransform.GetPointer());

  // Transform with one child:
  // oneChildTransform -> singleChild
  vtkNew<vtkDMMLTransformNode> oneChildTransform;
  scene->AddNode(oneChildTransform.GetPointer());
  singleChild->SetAndObserveTransformNodeID(oneChildTransform->GetID());

  // Transform with two children:
  // twoChildrenTransform -> child1
  //                      -> child2
  vtkNew<vtkDMMLTransformNode> twoChildrenTransform;
  scene->AddNode(twoChildrenTransform.GetPointer());
  child1->SetAndObserveTransformNodeID(twoChildrenTransform->GetID());
  child2->SetAndObserveTransformNodeID(twoChildrenTransform->GetID());

  // Transform tree:
  // transformTree -> childTransform -> grandChild1
  //                                 -> grandChild2
  //               -> child3
  vtkNew<vtkDMMLTransformNode> transformTree;
  scene->AddNode(transformTree.GetPointer());
  childTransform->SetAndObserveTransformNodeID(transformTree->GetID());
  child3->SetAndObserveTransformNodeID(transformTree->GetID());
  grandChild1->SetAndObserveTransformNodeID(childTransform->GetID());
  grandChild2->SetAndObserveTransformNodeID(childTransform->GetID());

  vtkNew<vtkDMMLTransformNode> notInSceneTransform;

  // Test childlessTransform
  std::vector<vtkDMMLDisplayableNode*> results;
  vtkCjyxTransformLogic::GetTransformedNodes(
    scene, childlessTransform.GetPointer(), results);
  if (results.size() != 0)
    {
    std::cout << "Error, expected results.size() == 0, got "
      << results.size() << std::endl;
    return EXIT_FAILURE;
    }
  results.clear();

  // Test oneChildTransform
  vtkCjyxTransformLogic::GetTransformedNodes(
    scene, oneChildTransform.GetPointer(), results);
  if (results.size() != 1)
    {
    std::cout << "Error, expected results.size() == 1, got "
      << results.size() << std::endl;
    return EXIT_FAILURE;
    }
  if (strcmp(results[0]->GetID(), singleChild->GetID()) != 0)
    {
    std::cout << "Error, expected results[0] == singleChild " << std::endl;
    return EXIT_FAILURE;
    }
  results.clear();

  // Test twoChildrenTransform
  vtkCjyxTransformLogic::GetTransformedNodes(
    scene, twoChildrenTransform.GetPointer(), results);
  if (results.size() != 2)
    {
    std::cout << "Error, expected results.size() == 2, got "
      << results.size() << std::endl;
    return EXIT_FAILURE;
    }
  if (strcmp(results[0]->GetID(), child1->GetID()) != 0)
    {
    std::cout << "Error, expected results[0] == child1 " << std::endl;
    return EXIT_FAILURE;
    }
  if (strcmp(results[1]->GetID(), child2->GetID()) != 0)
    {
    std::cout << "Error, expected results[1] == child2 " << std::endl;
    return EXIT_FAILURE;
    }
  results.clear();

  // Try node not in the scene
  vtkCjyxTransformLogic::GetTransformedNodes(
    scene, notInSceneTransform.GetPointer(), results);
  if (results.size() != 0)
    {
    std::cout << "Error, expected results.size() == 0, got "
      << results.size() << std::endl;
    return EXIT_FAILURE;
    }
  results.clear();

  // Test transform tree
  // Not recursively first
  vtkCjyxTransformLogic::GetTransformedNodes(
    scene, transformTree.GetPointer(), results, false);
  if (results.size() != 2)
    {
    std::cout << "Error, expected results.size() == 2, got "
      << results.size() << std::endl;
    return EXIT_FAILURE;
    }
  if (strcmp(results[0]->GetID(), childTransform->GetID()) != 0)
    {
    std::cout << "Error, expected results[0] == childTransform " << std::endl;
    return EXIT_FAILURE;
    }
  if (strcmp(results[1]->GetID(), child3->GetID()) != 0)
    {
    std::cout << "Error, expected results[1] == child3 " << std::endl;
    return EXIT_FAILURE;
    }
  results.clear();

  // Now recursively
  vtkCjyxTransformLogic::GetTransformedNodes(
    scene, transformTree.GetPointer(), results);
  if (results.size() != 4)
    {
    std::cout << "Error, expected results.size() == 4, got "
      << results.size() << std::endl;
    return EXIT_FAILURE;
    }
  if (strcmp(results[0]->GetID(), childTransform->GetID()) != 0)
    {
    std::cout << "Error, expected results[0] == childTransform " << std::endl;
    return EXIT_FAILURE;
    }
  if (strcmp(results[1]->GetID(), grandChild1->GetID()) != 0)
    {
    std::cout << "Error, expected results[1] == grandChild1 " << std::endl;
    return EXIT_FAILURE;
    }
  if (strcmp(results[2]->GetID(), grandChild2->GetID()) != 0)
    {
    std::cout << "Error, expected results[2] == grandChild2 " << std::endl;
    return EXIT_FAILURE;
    }
  if (strcmp(results[3]->GetID(), child3->GetID()) != 0)
    {
    std::cout << "Error, expected results[3] == child3 " << std::endl;
    return EXIT_FAILURE;
    }
  results.clear();

  // Clean-up
  scene->Delete();

  return EXIT_SUCCESS;
}
