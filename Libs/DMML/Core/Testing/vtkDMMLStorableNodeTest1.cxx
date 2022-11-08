/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLModelStorageNode.h"
#include "vtkDMMLStorableNode.h"

// VTK includes
#include <vtkObjectFactory.h>

//---------------------------------------------------------------------------
class vtkDMMLStorableNodeTestHelper1 : public vtkDMMLStorableNode
{
public:
  // Provide a concrete New.
  static vtkDMMLStorableNodeTestHelper1 *New();

  vtkTypeMacro(vtkDMMLStorableNodeTestHelper1,vtkDMMLStorableNode);

  vtkDMMLNode* CreateNodeInstance() override
    {
    return vtkDMMLStorableNodeTestHelper1::New();
    }
  const char* GetNodeTagName() override
    {
    return "vtkDMMLStorableNodeTestHelper1";
    }

  // for testing purposes, return a valid storage node,
  // vtkDMMLStorageNode::New returns nullptr
  vtkDMMLStorageNode* CreateDefaultStorageNode() override { return vtkDMMLModelStorageNode::New(); }
};
vtkStandardNewMacro(vtkDMMLStorableNodeTestHelper1);

//---------------------------------------------------------------------------
int vtkDMMLStorableNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLStorableNodeTestHelper1> node1;
  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());
  return EXIT_SUCCESS;
}
