#include "vtkDMMLAnnotationLineDisplayNode.h"
#include "vtkDMMLAnnotationLinesNode.h"
#include "vtkDMMLAnnotationLinesStorageNode.h"
#include "vtkDMMLScene.h"

#include <sstream>

#include "vtkDMMLCoreTestingMacros.h"

void SetControlPointsAndText(vtkDMMLAnnotationLinesNode* node2)
{
  node2->AddText("TESTING 1",1,1);
  node2->AddText("TESTING 2",1,1);

  {
    double ctp[3] = { 1, 1, 1};
    node2->AddControlPoint(ctp,1,1);
  }
  {
    double ctp[3] = { 2, 2, 2};
    node2->AddControlPoint(ctp,0,1);
  }
  {
    double ctp[3] = { 1, 2, 3};
    node2->AddControlPoint(ctp,0,0);
  }
}

int vtkDMMLAnnotationLinesNodeTest1(int , char * [] )
{
  vtkNew< vtkDMMLAnnotationLinesNode > node1;
  vtkNew<vtkDMMLScene> dmmlScene;
  dmmlScene->AddNode(node1.GetPointer());
  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());
  dmmlScene->RemoveNode(node1.GetPointer());

  // ======================
  // Basic Setup
  // ======================
  dmmlScene->RegisterNodeClass(vtkSmartPointer<vtkDMMLAnnotationLinesNode>::New());
  dmmlScene->RegisterNodeClass(vtkSmartPointer<vtkDMMLAnnotationLineDisplayNode>::New());

  // ======================
  // Modify Properties
  // ======================
  vtkNew<vtkDMMLAnnotationLinesNode> node2;

  dmmlScene->AddNode(node2.GetPointer());
  node2->CreateAnnotationLineDisplayNode();
  CHECK_NOT_NULL(node2->GetAnnotationLineDisplayNode());

  SetControlPointsAndText(node2.GetPointer());
  CHECK_INT(node2->AddLine(0,1,1/*sel*/,0/*vis*/), 0);

  node2->ResetAnnotations();
  CHECK_INT(node2->GetNumberOfLines(), 0);

  SetControlPointsAndText(node2.GetPointer());
  CHECK_INT(node2->AddLine(0,1,1,0), 0);

  int sel = 0;
  int vis = 1;
  CHECK_INT(node2->AddLine(0,2,sel,vis), 1);

  vtkIdType ctrlPointID[2]={0};

  CHECK_BOOL(node2->GetEndPointsId(1,ctrlPointID), true);
  CHECK_INT(ctrlPointID[0], 0);
  CHECK_INT(ctrlPointID[1], 2);
  CHECK_INT(node2->GetAnnotationAttribute(1, vtkDMMLAnnotationLinesNode::LINE_SELECTED), sel);
  CHECK_INT(node2->GetAnnotationAttribute(1, vtkDMMLAnnotationLinesNode::LINE_VISIBLE), vis);

  node2->AddLine(1,2,0,0);
  CHECK_INT(node2->GetNumberOfLines(), 3);

  node2->DeleteLine(1);
  CHECK_INT(node2->GetNumberOfLines(), 2);

  // ======================
  // Test WriteXML and ReadXML
  // ======================

  dmmlScene->SetURL("AnnotationLineNodeTest.dmml");
  dmmlScene->Commit(); // write

  vtkNew<vtkDMMLScene> dmmlScene2;
  dmmlScene2->RegisterNodeClass(vtkSmartPointer<vtkDMMLAnnotationLinesNode>::New());
  dmmlScene2->RegisterNodeClass(vtkSmartPointer<vtkDMMLAnnotationLineDisplayNode>::New());
  dmmlScene2->SetURL("AnnotationLineNodeTest.dmml");
  dmmlScene2->Connect(); // read

  CHECK_INT(dmmlScene2->GetNumberOfNodesByClass("vtkDMMLAnnotationLinesNode"),1);

  vtkDMMLAnnotationLinesNode *node3 = vtkDMMLAnnotationLinesNode::SafeDownCast(dmmlScene->GetFirstNodeByClass("vtkDMMLAnnotationLinesNode"));
  CHECK_NOT_NULL(node3);

  vtkIndent ind;
  std::stringstream initialAnnotation, afterAnnotation;

  node2->PrintAnnotationInfo(initialAnnotation,ind);
  node3->PrintAnnotationInfo(afterAnnotation,ind);
  CHECK_STRING(initialAnnotation.str().c_str(), afterAnnotation.str().c_str());

  std::cout << "Test passed" << std::endl;

  return EXIT_SUCCESS;
}


