#include "vtkDMMLAnnotationStorageNode.h"
#include "vtkDMMLAnnotationNode.h"
#include "vtkDMMLScene.h"

#include <sstream>

#include "vtkDMMLCoreTestingMacros.h"

int vtkDMMLAnnotationStorageNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLAnnotationStorageNode> node2;
  EXERCISE_ALL_BASIC_DMML_METHODS(node2.GetPointer());

  vtkDMMLAnnotationStorageNode* node1 = dynamic_cast <  vtkDMMLAnnotationStorageNode *> (node2->CreateNodeInstance());
  if( node1 == nullptr )
    {
      std::cerr << "Error in CreateNodeInstance()" << std::endl;
      return EXIT_FAILURE;
    }
  node1->Delete();


  vtkSmartPointer< vtkDMMLAnnotationNode > annNode = vtkSmartPointer< vtkDMMLAnnotationNode >::New();
  annNode->StartModify();
  annNode->SetName("AnnotationStorageNodeTest") ;
  std::string nodeTagName = annNode->GetNodeTagName();
  std::cout << "Node Tag Name = " << nodeTagName << std::endl;
  annNode->AddText("Test 1" , 1, 0);
  annNode->AddText("Test ,2"  , 0, 1);
  annNode->AddText("Test3"  , 1, 1);
  annNode->Modified();

  vtkSmartPointer<vtkDMMLScene> dmmlScene = vtkSmartPointer<vtkDMMLScene>::New();
  dmmlScene->AddNode(annNode);

  annNode->CreateAnnotationTextDisplayNode();
  if (!annNode->GetAnnotationTextDisplayNode())
    {
       std::cerr << "Error in vtkDMMLAnnotationNode::AnnotationTextDisplayNode() " << std::endl;
       return EXIT_FAILURE;
    }

  annNode->Modified();

  // node2->SetDataDirectory("./log");
  node2->SetFileName("AnnotationStorageNodeTest.acsv");
  int writeResult = node2->WriteData(annNode);
  if (!writeResult)
    {
    std::cerr << "Error writing data to file " << node2->GetFileName() << ", return value = " << writeResult << std::endl;
    return EXIT_FAILURE;
    }

  vtkIndent in;
  std::stringstream initialAnnotation, afterAnnotation;
  annNode->PrintAnnotationInfo(initialAnnotation,in);
  std::cout << "initialAnnotation (after write data called):" << std::endl << initialAnnotation.str() << std::endl;
  annNode->ResetAnnotations();
  int readResult = node2->ReadData(annNode);
  if (!readResult)
    {
    std::cerr << "Error reading data from file " << node2->GetFileName() << ", result = " << readResult << std::endl;
    return EXIT_FAILURE;
    }

  annNode->PrintAnnotationInfo(afterAnnotation,in);
  std::cout << "afterAnnotation (after read data called):" << std::endl << afterAnnotation.str() << std::endl;

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
