/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSceneViewNode.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkImageData.h>
#include <vtkNew.h>

int vtkDMMLSceneViewNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLSceneViewNode> node1;

  // test with null scene
  node1->StoreScene();
  node1->SetAbsentStorageFileNames();
  vtkCollection *col = node1->GetNodesByClass(nullptr);
  CHECK_NULL(col);

  // make a scene and test again
  vtkNew<vtkDMMLScene> scene;
  scene->AddNode(node1.GetPointer());
  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());
  node1->StoreScene();

  vtkDMMLScene *storedScene = node1->GetStoredScene();
  std::cout << "GetStoredScene returned " << (storedScene == nullptr ? "null" : "not null") << std::endl;

  node1->SetAbsentStorageFileNames();

  TEST_SET_GET_STRING( node1.GetPointer(), SceneViewDescription);

  node1->SetScreenShot(nullptr);
  vtkImageData *nullImage = node1->GetScreenShot();
  CHECK_NULL(nullImage);

  vtkImageData *imageData = vtkImageData::New();
  node1->SetScreenShot(imageData);
  imageData->Delete();
  imageData = node1->GetScreenShot();

  TEST_SET_GET_INT_RANGE( node1.GetPointer(), ScreenShotType, 0, 4);

  col = node1->GetNodesByClass("vtkDMMLNode");
  CHECK_NOT_NULL(col);

  col->RemoveAllItems();
  col->Delete();

  return EXIT_SUCCESS;
}
