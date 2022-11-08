/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLBSplineTransformNode.h"
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLTransformNode.h"

// VTK includes
#include <vtkGeneralTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkTransform.h>
#include <vtkAddonMathUtilities.h>

vtkMatrix4x4* CreateTransformMatrix(double translateX, double translateY, double translateZ, double rotateX, double rotateY, double rotateZ)
{
  vtkNew<vtkTransform> tr;
  tr->Translate(translateX, translateY, translateZ);
  tr->RotateX(rotateX);
  tr->RotateY(rotateY);
  tr->RotateZ(rotateZ);
  vtkMatrix4x4* matrix = vtkMatrix4x4::New();
  tr->GetMatrix(matrix);
  return matrix;
}

int vtkDMMLTransformNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLTransformNode> node1;
  vtkNew<vtkDMMLScene> scene;
  scene->AddNode(node1.GetPointer());
  vtkNew<vtkMatrix4x4> linearTransform;
  node1->SetMatrixTransformToParent(linearTransform.GetPointer());
  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());

  /// Test matrix get/set functions

  // Create transform nodes
  vtkNew<vtkDMMLTransformNode> bTransform; // == b_to_w
  vtkNew<vtkDMMLTransformNode> cTransform; // == c_to_b
  vtkNew<vtkDMMLTransformNode> dTransform; // == d_to_c
  vtkNew<vtkDMMLTransformNode> eTransform; // == e_to_d
  vtkNew<vtkDMMLTransformNode> qTransform; // == q_to_b
  vtkNew<vtkDMMLTransformNode> rTransform; // == r_to_q
  vtkSmartPointer<vtkMatrix4x4> w_from_b_mx = vtkSmartPointer<vtkMatrix4x4>::Take(CreateTransformMatrix( 34,  23, -12,  44,  12,  78));
  vtkSmartPointer<vtkMatrix4x4> b_from_c_mx = vtkSmartPointer<vtkMatrix4x4>::Take(CreateTransformMatrix(-34,  11,  12, -22, 128,  18));
  vtkSmartPointer<vtkMatrix4x4> c_from_d_mx = vtkSmartPointer<vtkMatrix4x4>::Take(CreateTransformMatrix( 14, -23,  44,  11, -71,  38));
  vtkSmartPointer<vtkMatrix4x4> d_from_e_mx = vtkSmartPointer<vtkMatrix4x4>::Take(CreateTransformMatrix( 73,  81,  35,  22,  11, -98));
  vtkSmartPointer<vtkMatrix4x4> b_from_q_mx = vtkSmartPointer<vtkMatrix4x4>::Take(CreateTransformMatrix( 13, -71, 335, -42,  91, -28));
  vtkSmartPointer<vtkMatrix4x4> q_from_r_mx = vtkSmartPointer<vtkMatrix4x4>::Take(CreateTransformMatrix( 53, -11,  65, -12,  21,   8));
  bTransform->SetMatrixTransformToParent(w_from_b_mx.GetPointer());
  cTransform->SetMatrixTransformToParent(b_from_c_mx.GetPointer());
  dTransform->SetMatrixTransformToParent(c_from_d_mx.GetPointer());
  eTransform->SetMatrixTransformToParent(d_from_e_mx.GetPointer());
  qTransform->SetMatrixTransformToParent(b_from_q_mx.GetPointer());
  rTransform->SetMatrixTransformToParent(q_from_r_mx.GetPointer());

  // Create transform hierarchy in the scene
  //
  // WORLD -> w coordinate system
  //  |-- bTransform
  //         |-- cTransform
  //         |      |-- dTransform
  //         |             |-- eTransform
  //         |-- qTransform
  //                |-- rTransform
  //
  scene->AddNode(bTransform.GetPointer());
  scene->AddNode(cTransform.GetPointer());
  scene->AddNode(dTransform.GetPointer());
  scene->AddNode(eTransform.GetPointer());
  scene->AddNode(qTransform.GetPointer());
  scene->AddNode(rTransform.GetPointer());
  cTransform->SetAndObserveTransformNodeID(bTransform->GetID());
  dTransform->SetAndObserveTransformNodeID(cTransform->GetID());
  eTransform->SetAndObserveTransformNodeID(dTransform->GetID());
  qTransform->SetAndObserveTransformNodeID(bTransform->GetID());
  rTransform->SetAndObserveTransformNodeID(qTransform->GetID());

  // Test that child transform cannot be set as parent
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_BOOL(bTransform->SetAndObserveTransformNodeID(cTransform->GetID()), false);
  CHECK_BOOL(bTransform->SetAndObserveTransformNodeID(bTransform->GetID()), false);
  CHECK_BOOL(bTransform->SetAndObserveTransformNodeID(dTransform->GetID()), false);
  CHECK_BOOL(bTransform->SetAndObserveTransformNodeID(eTransform->GetID()), false);
  CHECK_BOOL(cTransform->SetAndObserveTransformNodeID(dTransform->GetID()), false);
  CHECK_BOOL(cTransform->SetAndObserveTransformNodeID(eTransform->GetID()), false);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  // Pre-compute transform matrices that will be used for testing
  vtkNew<vtkMatrix4x4> c_from_e_mx;
  vtkMatrix4x4::Multiply4x4(c_from_d_mx.GetPointer(), d_from_e_mx.GetPointer(), c_from_e_mx.GetPointer());
  vtkNew<vtkMatrix4x4> b_from_e_mx;
  vtkMatrix4x4::Multiply4x4(b_from_c_mx.GetPointer(), c_from_e_mx.GetPointer(), b_from_e_mx.GetPointer());
  vtkNew<vtkMatrix4x4> w_from_e_mx;
  vtkMatrix4x4::Multiply4x4(w_from_b_mx.GetPointer(), b_from_e_mx.GetPointer(), w_from_e_mx.GetPointer());
  vtkNew<vtkMatrix4x4> e_from_c_mx;
  vtkMatrix4x4::Invert(c_from_e_mx.GetPointer(), e_from_c_mx.GetPointer());
  vtkNew<vtkMatrix4x4> c_from_b_mx;
  vtkNew<vtkMatrix4x4> b_from_r_mx;
  vtkMatrix4x4::Multiply4x4(b_from_q_mx.GetPointer(), q_from_r_mx.GetPointer(), b_from_r_mx.GetPointer());
  vtkMatrix4x4::Invert(b_from_c_mx.GetPointer(), c_from_b_mx.GetPointer());
  vtkNew<vtkMatrix4x4> c_from_r_mx;
  vtkMatrix4x4::Multiply4x4(c_from_b_mx.GetPointer(), b_from_r_mx.GetPointer(), c_from_r_mx.GetPointer());

  // Test AreTransformsEqual
  vtkNew<vtkGeneralTransform> transform1;
  vtkNew<vtkGeneralTransform> transform2;
  vtkDMMLTransformNode::GetTransformBetweenNodes(bTransform.GetPointer(), eTransform.GetPointer(), transform1.GetPointer());
  vtkDMMLTransformNode::GetTransformBetweenNodes(bTransform.GetPointer(), eTransform.GetPointer(), transform2.GetPointer());
  // Test equal
  CHECK_BOOL(vtkDMMLTransformNode::AreTransformsEqual(transform1.GetPointer(), transform2.GetPointer()), true);
  vtkDMMLTransformNode::GetTransformBetweenNodes(bTransform.GetPointer(), cTransform.GetPointer(), transform2.GetPointer());
  // Test non-equal
  CHECK_BOOL(vtkDMMLTransformNode::AreTransformsEqual(transform1.GetPointer(), transform2.GetPointer()), false);
  // Check nullptr transforms
  CHECK_BOOL(vtkDMMLTransformNode::AreTransformsEqual(transform1.GetPointer(), nullptr), false);
  CHECK_BOOL(vtkDMMLTransformNode::AreTransformsEqual(nullptr, nullptr), true);
  // check identity transform is the same as nullptr transform
  vtkDMMLTransformNode::GetTransformBetweenNodes(eTransform.GetPointer(), eTransform.GetPointer(), transform1.GetPointer());
  CHECK_BOOL(vtkDMMLTransformNode::AreTransformsEqual(transform1.GetPointer(), nullptr), true);

  // Test GetMatrixTransformToNode computations
  vtkNew<vtkMatrix4x4> test_mx;

  // GetMatrixTransformToNode: target node is parent
  eTransform->GetMatrixTransformToNode(cTransform.GetPointer(), test_mx.GetPointer());
  CHECK_BOOL(vtkAddonMathUtilities::MatrixAreEqual(c_from_e_mx.GetPointer(), test_mx.GetPointer()), true);

  // GetMatrixTransformToNode: target node is child
  cTransform->GetMatrixTransformToNode(eTransform.GetPointer(), test_mx.GetPointer());
  CHECK_BOOL(vtkAddonMathUtilities::MatrixAreEqual(e_from_c_mx.GetPointer(), test_mx.GetPointer()), true);

  // GetMatrixTransformToNode: target node is world
  eTransform->GetMatrixTransformToNode(nullptr, test_mx.GetPointer());
  CHECK_BOOL(vtkAddonMathUtilities::MatrixAreEqual(w_from_e_mx.GetPointer(), test_mx.GetPointer()), true);

  // GetMatrixTransformToWorld
  eTransform->GetMatrixTransformToWorld(test_mx.GetPointer());
  CHECK_BOOL(vtkAddonMathUtilities::MatrixAreEqual(w_from_e_mx.GetPointer(), test_mx.GetPointer()), true);

  // GetMatrixTransformToNode: target node is in different branch
  rTransform->GetMatrixTransformToNode(cTransform.GetPointer(), test_mx.GetPointer());
  CHECK_BOOL(vtkAddonMathUtilities::MatrixAreEqual(c_from_r_mx.GetPointer(), test_mx.GetPointer()), true);

  // GetMatrixTransformToNode: target node is the same as the source
  eTransform->GetMatrixTransformToNode(eTransform.GetPointer(), test_mx.GetPointer());
  vtkNew<vtkMatrix4x4> identity;
  CHECK_BOOL(vtkAddonMathUtilities::MatrixAreEqual(identity.GetPointer(), test_mx.GetPointer()), true);

  // Test when there is a nonlinear transform above the common parent of two transform nodes.
  // Transform to world is nonlinear but the relative transform is linear.
  vtkNew<vtkDMMLBSplineTransformNode> nonlinearTransform;
  scene->AddNode(nonlinearTransform.GetPointer());
  bTransform->SetAndObserveTransformNodeID(nonlinearTransform->GetID());
  //
  // WORLD -> w coordinate system
  //  |-- nonlinearTransform
  //       |-- bTransform
  //              |-- cTransform
  //              |      |-- dTransform
  //              |             |-- eTransform
  //              |-- qTransform
  //                     |-- rTransform
  //
  rTransform->GetMatrixTransformToNode(cTransform.GetPointer(), test_mx.GetPointer());
  CHECK_BOOL(vtkAddonMathUtilities::MatrixAreEqual(c_from_r_mx.GetPointer(), test_mx.GetPointer()), true);
  CHECK_POINTER(rTransform->GetFirstCommonParent(dTransform.GetPointer()), bTransform.GetPointer());

  std::cout << "vtkDMMLTransformNodeTest1 successfully completed" << std::endl;
  return EXIT_SUCCESS;
}
