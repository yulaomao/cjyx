/*=auto=========================================================================

  Portions (c) Copyright 2016 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

#include "vtkDMMLModelNode.h"
#include "vtkDMMLModelStorageNode.h"
#include "vtkDMMLScene.h"

#include "vtkDMMLCoreTestingMacros.h"

//------------------------------------------------------------------------------
int vtkDMMLSceneDefaultNodeTest(int , char * [] )
{
  vtkNew<vtkDMMLScene> scene1;

  // Test default node setting in scene
  vtkNew<vtkDMMLModelStorageNode> defaultStorageNode;
  defaultStorageNode->SetDefaultWriteFileExtension("stl");
  scene1->AddDefaultNode(defaultStorageNode.GetPointer());
  CHECK_POINTER(scene1->GetDefaultNodeByClass("vtkDMMLModelStorageNode"), defaultStorageNode.GetPointer());

  // Test if storage node created by AddDefaultStorage node
  // is overridden by default storage node set in the scene
  vtkNew<vtkDMMLModelNode> modelNode;
  scene1->AddNode(modelNode.GetPointer());
  CHECK_BOOL(modelNode->AddDefaultStorageNode(), true);
  vtkDMMLStorageNode* storageNode = modelNode->GetStorageNode();
  CHECK_NOT_NULL(storageNode);
  CHECK_STRING(storageNode->GetDefaultWriteFileExtension(), "stl");

  // Test if default node can be modified
  vtkDMMLModelStorageNode* defaultStorageNode2 = vtkDMMLModelStorageNode::SafeDownCast(scene1->GetDefaultNodeByClass("vtkDMMLModelStorageNode"));
  CHECK_NOT_NULL(defaultStorageNode2);
  defaultStorageNode2->SetDefaultWriteFileExtension("vtp");
  vtkNew<vtkDMMLModelNode> modelNode2;
  scene1->AddNode(modelNode2.GetPointer());
  CHECK_BOOL(modelNode2->AddDefaultStorageNode(), true);
  vtkDMMLStorageNode* storageNode2 = modelNode2->GetStorageNode();
  CHECK_NOT_NULL(storageNode2);
  CHECK_STRING(storageNode2->GetDefaultWriteFileExtension(), "vtp");

  return EXIT_SUCCESS;
}
