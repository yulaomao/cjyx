#include "vtkDMMLAnnotationControlPointsStorageNode.h"
#include "vtkDMMLAnnotationControlPointsNode.h"
#include "vtkDMMLScene.h"

#include <sstream>

#include "vtkDMMLCoreTestingMacros.h"

int vtkDMMLAnnotationControlPointsStorageNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLAnnotationControlPointsStorageNode> node2;
  EXERCISE_ALL_BASIC_DMML_METHODS(node2.GetPointer());

  vtkDMMLAnnotationControlPointsStorageNode* node1 = dynamic_cast <  vtkDMMLAnnotationControlPointsStorageNode *> (node2->CreateNodeInstance());
  if( node1 == nullptr )
    {
      std::cerr << "Error in CreateNodeInstance()" << std::endl;
      return EXIT_FAILURE;
    }
  node1->Delete();


  vtkSmartPointer< vtkDMMLAnnotationControlPointsNode > annNode = vtkSmartPointer< vtkDMMLAnnotationControlPointsNode >::New();
  annNode->StartModify();
  annNode->SetName("AnnotationControlPointsStorageNodeTest") ;
  std::string nodeTagName = annNode->GetNodeTagName();
  std::cout << "Node Tag Name = " << nodeTagName << std::endl;
  {
    double ctp[3] = { 1, 1, 1};
    annNode->AddControlPoint(ctp,1,1);
  }
  {
    double ctp[3] = { 2, 2, 2};
    annNode->AddControlPoint(ctp,0,1);
  }
  {
    double ctp[3] = { 1, 2, 3};
    annNode->AddControlPoint(ctp,0,0);
  }

  annNode->AddText("TESTING",1,1);
  vtkSmartPointer<vtkDMMLScene> dmmlScene = vtkSmartPointer<vtkDMMLScene>::New();
  dmmlScene->AddNode(annNode);

  annNode->CreateAnnotationTextDisplayNode();
  if (!annNode->GetAnnotationTextDisplayNode())
    {
       std::cerr << "Error in vtkDMMLAnnotationNode::AnnotationTextDisplayNode() " << std::endl;
       return EXIT_FAILURE;
    }

  annNode->CreateAnnotationPointDisplayNode();
  if (!annNode->GetAnnotationPointDisplayNode())
    {
       std::cerr << "Error in vtkDMMLAnnotationControlPointsNode::AnnotationPointDisplayNode() " << std::endl;
       return EXIT_FAILURE;
    }
  cout << "AnnotationPointDisplayNode Passed" << endl;

  annNode->Modified();

  // node2->SetDataDirectory("./log");
  node2->SetFileName("AnnotationControlPointsStorageNodeTest.acsv");
  node2->WriteData(annNode);

  vtkIndent in;
  std::stringstream initialAnnotation, afterAnnotation;
  annNode->PrintAnnotationInfo(initialAnnotation,in);
  annNode->ResetAnnotations();
  node2->ReadData(annNode);
  annNode->PrintAnnotationInfo(afterAnnotation,in);
  if (initialAnnotation.str().compare(afterAnnotation.str()))
  {
    std::cerr << endl << "Error in WriteData() or ReadData()" << std::endl;
    std::cerr << "Before:" << std::endl << initialAnnotation.str() <<std::endl;
    std::cerr << "After:" << std::endl << afterAnnotation.str() <<std::endl;
    std::cerr << "KP : need to fix annText field ones we have AnnotationFiducials defined" <<std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;

}
