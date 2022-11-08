#include "vtkDMMLAnnotationPointDisplayNode.h"
#include "vtkDMMLAnnotationFiducialNode.h"
#include "vtkDMMLScene.h"

#include <sstream>

#include "vtkDMMLCoreTestingMacros.h"

int vtkDMMLAnnotationFiducialNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLAnnotationFiducialNode> node1;
  vtkNew<vtkDMMLScene> dmmlScene;
  dmmlScene->AddNode(node1.GetPointer());
  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());
  dmmlScene->RemoveNode(node1.GetPointer());

  // ======================
  // Basic Setup
  // ======================
  dmmlScene->RegisterNodeClass(vtkSmartPointer<vtkDMMLAnnotationFiducialNode>::New());
  dmmlScene->RegisterNodeClass(vtkSmartPointer<vtkDMMLAnnotationPointDisplayNode>::New());

  // ======================
  // Modify Properties
  // ======================
  vtkNew<vtkDMMLAnnotationFiducialNode> node2;

  dmmlScene->AddNode(node2.GetPointer());
  node2->CreateAnnotationPointDisplayNode();
  CHECK_NOT_NULL(node2->GetAnnotationPointDisplayNode());

  double ctp[3] = { 1, 2, 3};
  const char* text = "Test 1 2";
  node2->SetFiducialLabel(text);
  CHECK_BOOL(node2->SetFiducial(ctp,1,0), true);
  node2->SetSelected(1);
  node2->SetDisplayVisibility(0);

  CHECK_INT(node2->GetNumberOfTexts(), 1);
  CHECK_STRING(node2->GetFiducialLabel(), text);

  double *ctrlPointPos = node2->GetFiducialCoordinates();
  CHECK_NOT_NULL(ctrlPointPos);
  CHECK_INT(ctrlPointPos[0], 1);
  CHECK_INT(ctrlPointPos[1], 2);
  CHECK_INT(ctrlPointPos[2], 3);

  CHECK_BOOL(node2->GetSelected(), true);
  CHECK_BOOL(node2->GetDisplayVisibility(), false);

  vtkIndent ind;
  node2->PrintAnnotationInfo(cout,ind);

  node2->Modified();

  // ======================
  // Test WriteXML and ReadXML
  // ======================

  dmmlScene->SetURL("AnnotationFiducialNodeTest.dmml");
  dmmlScene->Commit(); // write

  vtkNew<vtkDMMLScene> dmmlScene2;
  dmmlScene2->RegisterNodeClass(vtkSmartPointer<vtkDMMLAnnotationFiducialNode>::New());
  dmmlScene2->RegisterNodeClass(vtkSmartPointer<vtkDMMLAnnotationPointDisplayNode>::New());
  dmmlScene2->SetURL("AnnotationFiducialNodeTest.dmml");
  dmmlScene2->Connect(); // read

  CHECK_INT(dmmlScene2->GetNumberOfNodesByClass("vtkDMMLAnnotationFiducialNode"),1);

  vtkDMMLAnnotationFiducialNode *node3 = vtkDMMLAnnotationFiducialNode::SafeDownCast(dmmlScene->GetFirstNodeByClass("vtkDMMLAnnotationFiducialNode"));
  CHECK_NOT_NULL(node3);

  std::stringstream initialAnnotation, afterAnnotation;

  node2->PrintAnnotationInfo(initialAnnotation,ind);
  node3->PrintAnnotationInfo(afterAnnotation,ind);
  CHECK_STRING(initialAnnotation.str().c_str(), afterAnnotation.str().c_str());

  std::cout << "Test passed" << std::endl;

  return EXIT_SUCCESS;
}
