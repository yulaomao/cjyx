#include "vtkDMMLAnnotationRulerStorageNode.h"
#include "vtkDMMLAnnotationRulerNode.h"
#include "vtkDMMLScene.h"

#include <sstream>

#include "vtkDMMLCoreTestingMacros.h"

int vtkDMMLAnnotationRulerStorageNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLAnnotationRulerStorageNode> node2;
  EXERCISE_ALL_BASIC_DMML_METHODS(node2.GetPointer());

  vtkDMMLAnnotationRulerStorageNode* node1 = dynamic_cast <  vtkDMMLAnnotationRulerStorageNode *> (node2->CreateNodeInstance());
  if( node1 == nullptr )
    {
      std::cerr << "Error in CreateNodeInstance()" << std::endl;
      return EXIT_FAILURE;
    }
  node1->Delete();


  vtkSmartPointer< vtkDMMLAnnotationRulerNode > annNode = vtkSmartPointer< vtkDMMLAnnotationRulerNode >::New();
  annNode->StartModify();
  annNode->SetName("AnnotationRulerStorageNodeTest") ;
  std::string nodeTagName = annNode->GetNodeTagName();
  std::cout << "Node Tag Name = " << nodeTagName << std::endl;

 {
    double ctp[3] = { 1, 1, 1};
    annNode->SetPosition1(ctp);
  }
  {
    double ctp[3] = { 1, 2, 3};
    annNode->SetPosition2(ctp);
  }


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

  annNode->CreateAnnotationLineDisplayNode();
  if (!annNode->GetAnnotationLineDisplayNode())
    {
       std::cerr << "Error in vtkDMMLAnnotationLineNode::AnnotationPointDisplayNode() " << std::endl;
       return EXIT_FAILURE;
    }
  cout << "AnnotationPointDisplayNode Passed" << endl;


  annNode->Modified();

  // node2->SetDataDirectory("./log");
  node2->SetFileName("AnnotationRulerStorageNodeTest.acsv");
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
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;

}
