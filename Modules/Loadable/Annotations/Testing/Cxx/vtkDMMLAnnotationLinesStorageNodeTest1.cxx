
// DMML includes
#include "vtkDMMLAnnotationLinesStorageNode.h"
#include "vtkDMMLAnnotationLinesNode.h"
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLCoreTestingMacros.h"

// STD includes
#include <sstream>


int vtkDMMLAnnotationLinesStorageNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLAnnotationLinesStorageNode> node2;
  EXERCISE_ALL_BASIC_DMML_METHODS(node2.GetPointer());

  vtkDMMLAnnotationLinesStorageNode* node1 = dynamic_cast <  vtkDMMLAnnotationLinesStorageNode *> (node2->CreateNodeInstance());
  if( node1 == nullptr )
    {
      std::cerr << "Error in CreateNodeInstance()" << std::endl;
      return EXIT_FAILURE;
    }
  node1->Delete();


  vtkNew<vtkDMMLAnnotationLinesNode> annNode;
  annNode->StartModify();
  annNode->SetName("AnnotationLinesStorageNodeTest") ;
  std::string nodeTagName = annNode->GetNodeTagName();
  std::cout << "Node Tag Name = " << nodeTagName << std::endl;

  annNode->AddText("TESTING 1",1,1);
  annNode->AddText("TESTING 2",1,1);

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


  annNode->AddLine(0,1,1,0);
  annNode->AddLine(0,2,0,1);
  annNode->AddLine(1,2,0,0);


  vtkNew<vtkDMMLScene> dmmlScene;
  dmmlScene->AddNode(annNode.GetPointer());

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
  node2->SetFileName("AnnotationLinesStorageNodeTest.acsv");
  node2->WriteData(annNode.GetPointer());
  vtkIndent in;
  std::stringstream initialAnnotation, afterAnnotation;
  annNode->PrintAnnotationInfo(initialAnnotation,in);
  annNode->ResetAnnotations();
  if (!node2->ReadData(annNode.GetPointer()))
    {
      std::cerr << "Error in vtkDMMLAnnotationLinesStorageNode::ReadData() " << std::endl;
      return EXIT_FAILURE;
    }
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
