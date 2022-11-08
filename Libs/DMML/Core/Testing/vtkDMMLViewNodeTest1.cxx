/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLInteractionNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLViewNode.h"

int vtkDMMLViewNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLViewNode> node1;
  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());

  // Test Set/GetInteractionNode without scene
  {
    vtkNew<vtkDMMLViewNode> viewNode;
    vtkNew<vtkDMMLInteractionNode> interactionNode;
    CHECK_NULL(viewNode->GetInteractionNode());
    viewNode->SetInteractionNode(interactionNode.GetPointer());
    CHECK_NULL(viewNode->GetInteractionNode());
  }

  // Test Set/GetInteractionNode with scene
  {
    vtkNew<vtkDMMLScene> scene;
    vtkNew<vtkDMMLViewNode> viewNode;
    vtkNew<vtkDMMLInteractionNode> interactionNode; // interaction node is a singleton by default
    scene->AddNode(viewNode.GetPointer());
    scene->AddNode(interactionNode.GetPointer());
    CHECK_POINTER(viewNode->GetInteractionNode(), interactionNode.GetPointer());
    CHECK_POINTER(viewNode->GetInteractionNode(), scene->GetNodeByID("vtkDMMLInteractionNodeSingleton"));

    vtkNew<vtkDMMLInteractionNode> otherInteractionNode;
    otherInteractionNode->SetSingletonOff();
    scene->AddNode(otherInteractionNode.GetPointer());
    CHECK_POINTER(viewNode->GetInteractionNode(), scene->GetNodeByID("vtkDMMLInteractionNodeSingleton"));
    viewNode->SetInteractionNode(otherInteractionNode.GetPointer());
    CHECK_POINTER(viewNode->GetInteractionNode(), otherInteractionNode.GetPointer());
    CHECK_POINTER_DIFFERENT(otherInteractionNode.GetPointer(), scene->GetNodeByID("vtkDMMLInteractionNodeSingleton"));
  }

  return EXIT_SUCCESS;
}
