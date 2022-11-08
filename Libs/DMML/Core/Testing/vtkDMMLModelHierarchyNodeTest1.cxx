/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLModelDisplayNode.h"
#include "vtkDMMLModelHierarchyNode.h"
#include "vtkDMMLModelNode.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>

int vtkDMMLModelHierarchyNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLModelHierarchyNode> node1;
  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());

  TEST_SET_GET_STRING(node1.GetPointer(), ModelNodeID);
  //TEST_SET_GET_STRING(node1, DisplayNodeID);

  vtkNew<vtkDMMLScene> scene;
  vtkNew<vtkDMMLModelDisplayNode> dnode;
  scene->AddNode(node1.GetPointer());
  scene->AddNode(dnode.GetPointer());
  node1->SetAndObserveDisplayNodeID(dnode->GetID());
  TEST_SET_GET_BOOLEAN(node1.GetPointer(), Expanded);

  vtkNew<vtkDMMLModelNode> mnode;
  scene->AddNode(mnode.GetPointer());
  node1->SetModelNodeID(mnode->GetID());

  vtkDMMLModelNode * mnode2 = node1->GetModelNode();
  if (mnode2 != mnode.GetPointer())
    {
    std::cerr << "ERROR setting/getting model node" << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkCollection> col;
  node1->GetChildrenModelNodes(col.GetPointer());
  int numChildren =  col->GetNumberOfItems();
  if (numChildren != 1)
    {
    std::cerr << "Expected 1 child, got " << numChildren << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
