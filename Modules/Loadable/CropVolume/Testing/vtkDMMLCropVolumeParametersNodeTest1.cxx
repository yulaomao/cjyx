/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLCropVolumeParametersNode.h"

int vtkDMMLCropVolumeParametersNodeTest1(int , char * [] )
{
  vtkNew< vtkDMMLCropVolumeParametersNode > node1;

  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());

  TEST_SET_GET_STRING(node1.GetPointer(), InputVolumeNodeID);
  TEST_SET_GET_STRING(node1.GetPointer(), OutputVolumeNodeID);
  TEST_SET_GET_STRING(node1.GetPointer(), ROINodeID);

  TEST_SET_GET_BOOLEAN(node1.GetPointer(), VoxelBased);
  TEST_SET_GET_INT_RANGE(node1.GetPointer(), InterpolationMode, 1, 4);
  TEST_SET_GET_BOOLEAN(node1.GetPointer(), IsotropicResampling);
  TEST_SET_GET_DOUBLE_RANGE(node1.GetPointer(), SpacingScalingConst, -10.0, 10.0);

  return EXIT_SUCCESS;
}
