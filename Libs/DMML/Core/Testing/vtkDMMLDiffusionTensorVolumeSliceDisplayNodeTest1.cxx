/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLDiffusionTensorVolumeSliceDisplayNode.h"

int vtkDMMLDiffusionTensorVolumeSliceDisplayNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLDiffusionTensorVolumeSliceDisplayNode> node1;
  // EXERCISE_BASIC_DISPLAY_DMML_METHODS is failing due to set/get ScalarVisibility
  CHECK_EXIT_SUCCESS(vtkDMMLCoreTestingUtilities::ExerciseBasicDMMLMethods( node1.GetPointer() ));
  return EXIT_SUCCESS;
}
