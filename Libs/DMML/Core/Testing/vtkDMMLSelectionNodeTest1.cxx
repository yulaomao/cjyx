/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLSelectionNode.h"

// ---------------------------------------------------------------------------
int TestUnit(vtkDMMLSelectionNode* node1);

// ---------------------------------------------------------------------------
int vtkDMMLSelectionNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLSelectionNode> node1;

  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());

  TEST_SET_GET_STRING( node1.GetPointer(), ActiveVolumeID);
  TEST_SET_GET_STRING( node1.GetPointer(), SecondaryVolumeID);
  TEST_SET_GET_STRING( node1.GetPointer(), ActiveLabelVolumeID);
  TEST_SET_GET_STRING( node1.GetPointer(), ActivePlaceNodeID);
  TEST_SET_GET_STRING( node1.GetPointer(), ActivePlaceNodeClassName);
  TEST_SET_GET_STRING( node1.GetPointer(), ActiveROIListID);
  TEST_SET_GET_STRING( node1.GetPointer(), ActiveCameraID);
  TEST_SET_GET_STRING( node1.GetPointer(), ActiveTableID);
  TEST_SET_GET_STRING( node1.GetPointer(), ActiveViewID);
  TEST_SET_GET_STRING( node1.GetPointer(), ActiveLayoutID);

  // annotations
  node1->AddNewPlaceNodeClassNameToList(nullptr, nullptr);
  node1->AddNewPlaceNodeClassNameToList("invalid string", nullptr);
  node1->AddNewPlaceNodeClassNameToList("vtkDMMLAnnotationFiducialNode", nullptr);
  node1->AddNewPlaceNodeClassNameToList(nullptr, ":/Icons/AnnotationROI.png");
  node1->AddNewPlaceNodeClassNameToList("vtkDMMLAnnotationROINode", ":/Icons/AnnotationROI.png");
  node1->AddNewPlaceNodeClassNameToList("vtkDMMLAnnotationFiducialNode", ":/Icons/AnnotationPoint.png");

  std::string className;
  std::cout << "Checking for className '" << className.c_str() << "' in list, got index: " << node1->PlaceNodeClassNameInList(className) << std::endl;
  className = std::string("vtkDMMLAnnotationFiducialNode");
  int index = node1->PlaceNodeClassNameInList(className);
  std::cout << "Checking for className '" << className.c_str() << "' in list, got index: " << index << std::endl;
  if (index != -1)
    {
    std::string classNamestring = node1->GetPlaceNodeClassNameByIndex(index);
    if (classNamestring.compare(className) != 0)
      {
      std::cerr << "Error! Set className '" << className.c_str()
                << "' to list at index " << index << ", but got back '"
                << classNamestring.c_str() << "'" << std::endl;
      node1->Print(std::cout);
      return EXIT_FAILURE;
      }
    std::string resource = node1->GetPlaceNodeResourceByIndex(index);
    if (resource.compare(":/Icons/AnnotationPoint.png") != 0)
      {
      std::cerr << "ERROR! Got resource for index " << index << ": '" << resource.c_str() << "', but expected ':/Icons/AnnotationPoint.png'" << std::endl;
      node1->Print(std::cout);
      return EXIT_FAILURE;
      }
    std::cout << "Got resource for index " << index << ": " << resource.c_str() << std::endl;
    }
  std::string resource = node1->GetPlaceNodeResourceByClassName(className);
  if (resource.compare(":/Icons/AnnotationPoint.png") != 0)
    {
    std::cerr << "ERROR! Got resource for className " << className << ": '" << resource.c_str() << "', but expected ':/Icons/AnnotationPoint.png'" << std::endl;
    node1->Print(std::cout);
    return EXIT_FAILURE;
    }

  CHECK_EXIT_SUCCESS(TestUnit(node1.GetPointer()));

  return EXIT_SUCCESS;
}

// ---------------------------------------------------------------------------
int TestUnit(vtkDMMLSelectionNode* node1 )
{
  vtkNew<vtkDMMLCoreTestingUtilities::vtkDMMLNodeCallback> callback;
  node1->AddObserver(vtkDMMLSelectionNode::UnitModifiedEvent, callback.GetPointer());

  const char* quantity = "mass";
  const char* unit = "vtkDMMLUnitNodeKilogram";
  node1->SetUnitNodeID(quantity, unit);
  CHECK_STRING(node1->GetUnitNodeID(quantity), unit);

  node1->SetUnitNodeID(quantity, "");
  CHECK_NULL(node1->GetUnitNodeID(quantity));


  node1->SetUnitNodeID(quantity, nullptr);
  CHECK_NULL(node1->GetUnitNodeID(quantity));

  node1->SetUnitNodeID("", unit);
  CHECK_STRING(node1->GetUnitNodeID(""), unit);

  node1->SetUnitNodeID(nullptr, unit);
  CHECK_STRING(node1->GetUnitNodeID(nullptr), unit);

  CHECK_INT(callback->GetNumberOfEvents(vtkDMMLSelectionNode::UnitModifiedEvent), 3);

  return EXIT_SUCCESS;
}
