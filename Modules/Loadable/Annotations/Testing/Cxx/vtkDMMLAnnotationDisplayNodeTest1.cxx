/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

#include "vtkDMMLAnnotationDisplayNode.h"
#include "vtkDMMLCoreTestingMacros.h"

int vtkDMMLAnnotationDisplayNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLAnnotationDisplayNode> node1;
  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());
  return EXIT_SUCCESS;
}
