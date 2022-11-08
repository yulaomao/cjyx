/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

#include "vtkDMMLClipModelsNode.h"
#include "vtkDMMLCoreTestingMacros.h"

int vtkDMMLClipModelsNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLClipModelsNode> node1;
  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());
  return EXIT_SUCCESS;
}
