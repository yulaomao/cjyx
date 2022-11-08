/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLCameraNode.h"
#include "vtkDMMLCoreTestingMacros.h"

namespace {

//---------------------------------------------------------------------------
int ExerciseBasicMethods();
int TestGetSetLayoutName();

}

//---------------------------------------------------------------------------
int vtkDMMLCameraNodeTest1(int , char * [] )
{
  CHECK_EXIT_SUCCESS(ExerciseBasicMethods());
  CHECK_EXIT_SUCCESS(TestGetSetLayoutName());
  return EXIT_SUCCESS;
}

namespace {

//---------------------------------------------------------------------------
int ExerciseBasicMethods()
{
  vtkNew<vtkDMMLCameraNode> node1;
  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestGetSetLayoutName()
{
  vtkNew<vtkDMMLCameraNode> node1;

  vtkNew<vtkDMMLCoreTestingUtilities::vtkDMMLNodeCallback> callback;
  node1->AddObserver(vtkCommand::AnyEvent, callback.GetPointer());

  CHECK_NULL(node1->GetLayoutName());

  node1->SetLayoutName(nullptr);
  CHECK_INT(callback->GetTotalNumberOfEvents(), 0);

  node1->SetLayoutName("1");
  CHECK_INT(callback->GetNumberOfEvents(vtkDMMLCameraNode::LayoutNameModifiedEvent), 1);
  CHECK_INT(callback->GetNumberOfModified(), 1);
  CHECK_INT(callback->GetTotalNumberOfEvents(), 2);

  callback->ResetNumberOfEvents();
  node1->SetLayoutName("1");
  CHECK_INT(callback->GetTotalNumberOfEvents(), 0);

  callback->ResetNumberOfEvents();
  node1->SetLayoutName("2");
  CHECK_INT(callback->GetNumberOfEvents(vtkDMMLCameraNode::LayoutNameModifiedEvent), 1);
  CHECK_INT(callback->GetNumberOfModified(), 1);
  CHECK_INT(callback->GetTotalNumberOfEvents(), 2);

  callback->ResetNumberOfEvents();
  node1->SetLayoutName(nullptr);
  CHECK_INT(callback->GetNumberOfEvents(vtkDMMLCameraNode::LayoutNameModifiedEvent), 1);
  CHECK_INT(callback->GetNumberOfModified(), 1);
  CHECK_INT(callback->GetTotalNumberOfEvents(), 2);

  return EXIT_SUCCESS;
}

}
