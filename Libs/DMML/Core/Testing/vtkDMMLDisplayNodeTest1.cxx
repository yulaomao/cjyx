/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLDisplayNode.h"

// VTK includes
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
class vtkDMMLDisplayNodeTestHelper1 : public vtkDMMLDisplayNode
{
public:
  // Provide a concrete New.
  static vtkDMMLDisplayNodeTestHelper1 *New();

  vtkTypeMacro(vtkDMMLDisplayNodeTestHelper1,vtkDMMLDisplayNode);

  vtkDMMLNode* CreateNodeInstance() override
    {
    return vtkDMMLDisplayNodeTestHelper1::New();
    }

  const char * GetTypeAsString()
    {
    return "vtkDMMLDisplayNodeTestHelper1";
    }

  int ReadFile()
    {
    std::cout << "vtkDMMLDisplayNodeTestHelper1 pretending to read a file " << std::endl;
    return EXIT_SUCCESS;
    }

  const char* GetNodeTagName() override
    {
    return "Testing is good";
    }
};
vtkStandardNewMacro(vtkDMMLDisplayNodeTestHelper1);

//----------------------------------------------------------------------------
int vtkDMMLDisplayNodeTest1(int , char * [])
{
  vtkNew<vtkDMMLDisplayNodeTestHelper1> node1;
  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());
  return EXIT_SUCCESS;
}
