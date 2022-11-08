#include "vtkDMMLAnnotationControlPointsNode.h"
#include "vtkDMMLAnnotationControlPointsStorageNode.h"
#include "vtkDMMLAnnotationPointDisplayNode.h"
#include "vtkDMMLScene.h"

#include <sstream>

#include "vtkDMMLCoreTestingMacros.h"

int vtkDMMLAnnotationControlPointsNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLAnnotationControlPointsNode> node1;
  vtkNew<vtkDMMLScene> dmmlScene;
  dmmlScene->AddNode(node1.GetPointer());
  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());
  dmmlScene->RemoveNode(node1.GetPointer());

  // ======================
  // Basic Setup
  // ======================
  dmmlScene->RegisterNodeClass(vtkSmartPointer<vtkDMMLAnnotationControlPointsNode>::New());
  dmmlScene->RegisterNodeClass(vtkSmartPointer<vtkDMMLAnnotationPointDisplayNode>::New());

  // ======================
  // Modify Properties
  // ======================
  vtkNew<vtkDMMLAnnotationControlPointsNode> node2;
  node2->Reset(nullptr);

  dmmlScene->AddNode(node2.GetPointer());
  node2->CreateAnnotationPointDisplayNode();
  CHECK_NOT_NULL(node2->GetAnnotationPointDisplayNode());

  {
    double ctp[3] = { 1, 1, 1};
    node2->AddControlPoint(ctp,1,1);
    CHECK_BOOL(node2->GetAnnotationAttribute(0, vtkDMMLAnnotationControlPointsNode::CP_VISIBLE), true);
    CHECK_BOOL(node2->GetAnnotationAttribute(0, vtkDMMLAnnotationControlPointsNode::CP_SELECTED), true);
  }
  {
    double ctp[3] = { 2, 2, 2};
    node2->AddControlPoint(ctp,0,1);
  }
  {
    double ctp[3] = { 1, 2, 3};
    node2->SetControlPoint(3,ctp,0,0);
    node2->SetControlPoint(2,ctp,0,0);
  }
  CHECK_INT(node2->GetNumberOfControlPoints(), 4);

  node2->DeleteControlPoint(3);
  CHECK_INT(node2->GetNumberOfControlPoints(), 3);

  node2->AddText("TESTING",1,1);
  CHECK_INT(node2->GetNumberOfTexts(), 1);
  CHECK_STRING(node2->GetText(0), "TESTING");

  node2->Modified();

  // ======================
  // Test WriteXML and ReadXML
  // ======================

  dmmlScene->SetURL("AnnotationControlPointNodeTest.dmml");
  dmmlScene->Commit(); // write

  vtkNew<vtkDMMLScene> dmmlScene2;
  dmmlScene2->RegisterNodeClass(vtkSmartPointer<vtkDMMLAnnotationControlPointsNode>::New());
  dmmlScene2->RegisterNodeClass(vtkSmartPointer<vtkDMMLAnnotationPointDisplayNode>::New());
  dmmlScene2->SetURL("AnnotationControlPointNodeTest.dmml");
  dmmlScene2->Connect(); // read

  CHECK_INT(dmmlScene2->GetNumberOfNodesByClass("vtkDMMLAnnotationControlPointsNode"),1);

  vtkDMMLAnnotationControlPointsNode *node3 = vtkDMMLAnnotationControlPointsNode::SafeDownCast(
    dmmlScene->GetFirstNodeByClass("vtkDMMLAnnotationControlPointsNode"));
  CHECK_NOT_NULL(node3);

  vtkIndent ind;
  std::stringstream initialAnnotation, afterAnnotation;

  node2->PrintAnnotationInfo(initialAnnotation,ind);
  node3->PrintAnnotationInfo(afterAnnotation,ind);
  CHECK_STRING(initialAnnotation.str().c_str(), afterAnnotation.str().c_str());

  std::cout << "Test passed" << std::endl;

  return EXIT_SUCCESS;
}
