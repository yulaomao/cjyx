/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLLinearTransformNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLTransformableNode.h"

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

//---------------------------------------------------------------------------
class vtkDMMLTransformableNodeTestHelper1 : public vtkDMMLTransformableNode
{
public:
  // Provide a concrete New.
  static vtkDMMLTransformableNodeTestHelper1 *New();

  vtkTypeMacro(vtkDMMLTransformableNodeTestHelper1,vtkDMMLTransformableNode);

  vtkDMMLNode* CreateNodeInstance() override
    {
    return vtkDMMLTransformableNodeTestHelper1::New();
    }
  const char* GetNodeTagName() override
    {
    return "vtkDMMLTransformableNodeTestHelper1";
    }
};
vtkStandardNewMacro(vtkDMMLTransformableNodeTestHelper1);

//---------------------------------------------------------------------------
int TestSetAndObserveTransformNodeID();

//---------------------------------------------------------------------------
int vtkDMMLTransformableNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLTransformableNodeTestHelper1> node1;
  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());
  CHECK_EXIT_SUCCESS(TestSetAndObserveTransformNodeID());
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestSetAndObserveTransformNodeID()
{
  vtkNew<vtkDMMLScene> scene;
  vtkNew<vtkDMMLTransformableNodeTestHelper1> transformable;
  scene->AddNode(transformable.GetPointer());

  vtkNew<vtkDMMLLinearTransformNode> transform;
  scene->AddNode(transform.GetPointer());
  vtkNew<vtkMatrix4x4> matrix;
  matrix->SetElement(0,3, 1.);
  transform->SetMatrixTransformToParent(matrix.GetPointer());

  transformable->SetAndObserveTransformNodeID(transform->GetID());
  CHECK_POINTER(transformable->GetParentTransformNode(), transform.GetPointer());
  double point[3] = {0., 0., 0.};
  double res[3] = {-1., -1., -1.};
  transformable->TransformPointToWorld(point, res);
  if (res[0] != 1. || res[1] != 0. || res[2] != 0.)
    {
    std::cout << __LINE__ << "TransformPointToWorld failed"
              << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
