/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLDiffusionTensorDisplayPropertiesNode.h"

// VTK includes
#include <vtkObjectFactory.h>

//---------------------------------------------------------------------------
class vtkDMMLDiffusionTensorDisplayPropertiesNodeTestHelper1 : public vtkDMMLDiffusionTensorDisplayPropertiesNode
{
public:
  // Provide a concrete New.
  static vtkDMMLDiffusionTensorDisplayPropertiesNodeTestHelper1 *New();

  vtkTypeMacro(vtkDMMLDiffusionTensorDisplayPropertiesNodeTestHelper1,vtkDMMLDiffusionTensorDisplayPropertiesNode);

  vtkDMMLNode* CreateNodeInstance() override
    {
    return vtkDMMLDiffusionTensorDisplayPropertiesNodeTestHelper1::New();
    }
  const char* GetNodeTagName() override
    {
    return "vtkDMMLDiffusionTensorDisplayPropertiesNodeTestHelper1";
    }

  virtual int ReadData(vtkDMMLNode *vtkNotUsed(refNode)) { return 0; }
  virtual int WriteData(vtkDMMLNode *vtkNotUsed(refNode)) { return 0; }
};
vtkStandardNewMacro(vtkDMMLDiffusionTensorDisplayPropertiesNodeTestHelper1);

//---------------------------------------------------------------------------
int vtkDMMLDiffusionTensorDisplayPropertiesNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLDiffusionTensorDisplayPropertiesNodeTestHelper1> node1;
  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());
  return EXIT_SUCCESS;
}
