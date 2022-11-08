
// DMML includes
#include "vtkDMMLAnnotationRulerNode.h"
#include "vtkDMMLAnnotationPointDisplayNode.h"
#include "vtkDMMLAnnotationRulerStorageNode.h"
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLScene.h"

// STD includes
#include <sstream>

int vtkDMMLAnnotationRulerNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLAnnotationRulerNode> node1;
  vtkNew<vtkDMMLScene> dmmlScene;
  dmmlScene->AddNode(node1.GetPointer());
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());
  TESTING_OUTPUT_ASSERT_ERRORS(4); // write XML fails for a ruler without points defined
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  dmmlScene->RemoveNode(node1.GetPointer());

  // ======================
  // Basic Setup
  // ======================
  dmmlScene->RegisterNodeClass(vtkSmartPointer<vtkDMMLAnnotationRulerNode>::New());
  dmmlScene->RegisterNodeClass(vtkSmartPointer<vtkDMMLAnnotationPointDisplayNode>::New());

  // ======================
  // Modify Properties
  // ======================
  vtkNew<vtkDMMLAnnotationRulerNode> node2;
  node2->Reset(nullptr);

  dmmlScene->AddNode(node2.GetPointer());
  node2->CreateAnnotationPointDisplayNode();
  CHECK_NOT_NULL(node2->GetAnnotationPointDisplayNode());

  {
    double ctp[3] = { 1, 1, 1};
    node2->SetPosition1(ctp);
  }
  {
    double ctp[3] = { 1, 2, 3};
    node2->SetPosition2(ctp);
  }

  double *ctrlPointID = node2->GetPosition2();
  CHECK_NOT_NULL(ctrlPointID);
  CHECK_INT(ctrlPointID[0], 1);
  CHECK_INT(ctrlPointID[1], 2);
  CHECK_INT(ctrlPointID[2], 3);

  vtkIndent ind;
  node2->PrintAnnotationInfo(cout,ind);

  node2->Modified();

  // Test Copy
  vtkNew< vtkDMMLAnnotationRulerNode> cNode;
  cNode->Copy(node2.GetPointer());

  CHECK_INT(cNode->GetPosition1()[0], node2->GetPosition1()[0]);
  CHECK_INT(cNode->GetPosition1()[1], node2->GetPosition1()[1]);
  CHECK_INT(cNode->GetPosition1()[2], node2->GetPosition1()[2]);

  CHECK_INT(cNode->GetPosition2()[0], node2->GetPosition2()[0]);
  CHECK_INT(cNode->GetPosition2()[1], node2->GetPosition2()[1]);
  CHECK_INT(cNode->GetPosition2()[2], node2->GetPosition2()[2]);

  cout << endl << "===== After Copy ====" << endl;
  cNode->Print( std::cout );
  cNode->PrintAnnotationInfo(cout,ind);

  // Test CopyWithoutModifiedEvent
  vtkNew< vtkDMMLAnnotationRulerNode> c2Node;
  int oldMode = c2Node->GetDisableModifiedEvent();
  c2Node->DisableModifiedEventOn();
  c2Node->Copy(node2.GetPointer());
  c2Node->SetDisableModifiedEvent(oldMode);

  CHECK_INT(c2Node->GetPosition1()[0], node2->GetPosition1()[0]);
  CHECK_INT(c2Node->GetPosition1()[1], node2->GetPosition1()[1]);
  CHECK_INT(c2Node->GetPosition1()[2], node2->GetPosition1()[2]);

  CHECK_INT(c2Node->GetPosition2()[0], node2->GetPosition2()[0]);
  CHECK_INT(c2Node->GetPosition2()[1], node2->GetPosition2()[1]);
  CHECK_INT(c2Node->GetPosition2()[2], node2->GetPosition2()[2]);

  cout << endl << "===== After Second Copy ====" << endl;
  c2Node->Print( std::cout );
  c2Node->PrintAnnotationInfo(cout,ind);

  // ======================
  // Test WriteXML and ReadXML
  // ======================

  dmmlScene->SetURL("AnnotationRulerNodeTest.dmml");
  dmmlScene->Commit(); // write

  vtkNew<vtkDMMLScene> dmmlScene2;
  dmmlScene2->RegisterNodeClass(vtkSmartPointer<vtkDMMLAnnotationRulerNode>::New());
  dmmlScene2->RegisterNodeClass(vtkSmartPointer<vtkDMMLAnnotationPointDisplayNode>::New());
  dmmlScene2->SetURL("AnnotationRulerNodeTest.dmml");
  dmmlScene2->Connect(); // read

  CHECK_INT(dmmlScene2->GetNumberOfNodesByClass("vtkDMMLAnnotationRulerNode"),1);

  vtkDMMLAnnotationRulerNode *node3 = vtkDMMLAnnotationRulerNode::SafeDownCast(dmmlScene->GetFirstNodeByClass("vtkDMMLAnnotationRulerNode"));
  CHECK_NOT_NULL(node3);

  std::stringstream initialAnnotation, afterAnnotation;

  node2->PrintAnnotationInfo(initialAnnotation,ind);
  node3->PrintAnnotationInfo(afterAnnotation,ind);
  CHECK_STRING(initialAnnotation.str().c_str(), afterAnnotation.str().c_str());

  std::cout << "Test passed" << std::endl;

  return EXIT_SUCCESS;
}
