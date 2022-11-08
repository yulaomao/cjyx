/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLHierarchyNode.h"

// VTK includes
#include <vtkObjectFactory.h>

//---------------------------------------------------------------------------
class vtkDMMLHierarchyNodeTestHelper1 : public vtkDMMLHierarchyNode
{
public:
  // Provide a concrete New.
  static vtkDMMLHierarchyNodeTestHelper1 *New();

  vtkTypeMacro(vtkDMMLHierarchyNodeTestHelper1,vtkDMMLHierarchyNode);

  vtkDMMLNode* CreateNodeInstance() override
    {
    return vtkDMMLHierarchyNodeTestHelper1::New();
    }
  const char* GetNodeTagName() override
    {
    return "vtkDMMLHierarchyNodeTestHelper1";
    }
  virtual bool CanApplyNonLinearTransforms() { return false; }
};
vtkStandardNewMacro(vtkDMMLHierarchyNodeTestHelper1);

//---------------------------------------------------------------------------
int vtkDMMLHierarchyNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLHierarchyNode> node1;
  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());

  TEST_SET_GET_DOUBLE_RANGE(node1.GetPointer(), SortingValue, 0.0, 10.0);

  TEST_SET_GET_STRING(node1.GetPointer(), ParentNodeID);

  vtkDMMLHierarchyNode* pnode = node1->GetParentNode();
  std::cout << "GetParentNode returned " << (pnode == nullptr ? "null" : "not null") << std::endl;

  node1->SetParentNodeID("testingID");
  pnode = node1->GetParentNode();
  std::cout << "GetParentNode returned " << (pnode == nullptr ? "null" : "not null") << std::endl;

  // test the parent node id
  node1->SetParentNodeID(nullptr);
  if (node1->GetParentNodeID() != nullptr)
    {
    std::cerr << "Error setting parent node id to null" << std::endl;
    return EXIT_FAILURE;
    }
  node1->SetParentNodeID(nullptr);
  if (node1->GetParentNodeID() != nullptr)
    {
    std::cerr << "Error setting parent node id to null a second time" << std::endl;
    return EXIT_FAILURE;
    }
  node1->SetParentNodeID("testingValidStringID");
  if (node1->GetParentNodeID() == nullptr ||
      strcmp(node1->GetParentNodeID(), "testingValidStringID") != 0)
    {
    std::cerr << "Error setting parent node id not null" << std::endl;
    return EXIT_FAILURE;
    }
  node1->SetParentNodeID("testingValidStringID");
  if (node1->GetParentNodeID() == nullptr ||
      strcmp(node1->GetParentNodeID(), "testingValidStringID") != 0)
    {
    std::cerr << "Error setting parent node id to a valid string a second time" << std::endl;
    return EXIT_FAILURE;
    }
  node1->SetParentNodeID("anotherID");
  if (node1->GetParentNodeID() == nullptr ||
      strcmp(node1->GetParentNodeID(), "anotherID") != 0)
    {
    std::cerr << "Error setting parent node id to a different valid string" << std::endl;
    return EXIT_FAILURE;
    }



  TEST_SET_GET_BOOLEAN(node1.GetPointer(), AllowMultipleChildren);

  return EXIT_SUCCESS;
}
