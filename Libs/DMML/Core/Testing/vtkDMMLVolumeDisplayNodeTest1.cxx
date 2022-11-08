/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLVolumeDisplayNode.h"

// VTK includes
#include <vtkObjectFactory.h>

//---------------------------------------------------------------------------
class vtkDMMLVolumeDisplayNodeTestHelper1 : public vtkDMMLVolumeDisplayNode
{
public:
  // Provide a concrete New.
  static vtkDMMLVolumeDisplayNodeTestHelper1 *New();

  vtkTypeMacro(vtkDMMLVolumeDisplayNodeTestHelper1,vtkDMMLVolumeDisplayNode);

  vtkDMMLNode* CreateNodeInstance() override
    {
    return vtkDMMLVolumeDisplayNodeTestHelper1::New();
    }

  const char * GetTypeAsString()
    {
    return "vtkDMMLVolumeDisplayNodeTestHelper1";
    }

  int ReadFile()
    {
    std::cout << "vtkDMMLVolumeDisplayNodeTestHelper1 pretending to read a file " << std::endl;
    return EXIT_SUCCESS;
    }

  const char* GetNodeTagName() override
    {
    return "Testing is good";
    }
};
vtkStandardNewMacro(vtkDMMLVolumeDisplayNodeTestHelper1);

//---------------------------------------------------------------------------
int vtkDMMLVolumeDisplayNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLVolumeDisplayNodeTestHelper1> node1;
  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());
  return EXIT_SUCCESS;
}
