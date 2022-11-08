/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

// DMMLLogic includes
#include "vtkDMMLSliceLogic.h"
#include "vtkDMMLSliceLayerLogic.h"

// DMML includes
#include <vtkDMMLLinearTransformNode.h>
#include <vtkDMMLModelDisplayNode.h>
#include <vtkDMMLModelNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceCompositeNode.h>

// VTK includes
#include <vtkImageBlend.h>
#include <vtkNew.h>

#include "vtkDMMLCoreTestingMacros.h"

int vtkDMMLSliceLogicTest1(int , char * [] )
{
  vtkNew<vtkDMMLSliceLogic> logic;
  EXERCISE_BASIC_OBJECT_METHODS(logic.GetPointer());

  vtkNew<vtkDMMLScene> scene;

  // Add default slice orientation presets
  vtkDMMLSliceNode::AddDefaultSliceOrientationPresets(scene.GetPointer());

  logic->SetDMMLScene(scene.GetPointer());
  CHECK_NOT_NULL(logic->AddSliceNode("Green"));

  vtkNew<vtkDMMLSliceNode> SliceNode;
  TEST_SET_GET_VALUE(logic, SliceNode, SliceNode.GetPointer());

  vtkNew<vtkDMMLSliceLayerLogic> LabelLayer;
  TEST_SET_GET_VALUE(logic, LabelLayer, LabelLayer.GetPointer());

  vtkNew<vtkDMMLSliceCompositeNode> SliceCompositeNode;
  TEST_SET_GET_VALUE(logic, SliceCompositeNode, SliceCompositeNode.GetPointer());

  vtkNew<vtkDMMLSliceLayerLogic> ForegroundLayer;
  TEST_SET_GET_VALUE(logic, ForegroundLayer, ForegroundLayer.GetPointer());

  vtkNew<vtkDMMLSliceLayerLogic> BackgroundLayer;
  TEST_SET_GET_VALUE(logic, BackgroundLayer, BackgroundLayer.GetPointer());

  // TODO: need to fix the test.
  // The problem here is that the current node of the logic is wrong
  // it hasn't been added to the dmml scene. So when modified,
  // the logic realizes it and create a new node (loosing the props).
  //TEST_SET_GET_VALUE(logic, SliceOffset, 1);

  logic->DeleteSliceModel();
  logic->CreateSliceModel();
  TEST_GET_OBJECT(logic, SliceModelNode);
  TEST_GET_OBJECT(logic, SliceModelDisplayNode);
  TEST_GET_OBJECT(logic, SliceModelTransformNode);
  TEST_GET_OBJECT(logic, Blend);

  logic->Print(std::cout);
  return EXIT_SUCCESS;
}

