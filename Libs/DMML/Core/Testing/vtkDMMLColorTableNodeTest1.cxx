/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLColorTableNode.h"
#include "vtkDMMLColorTableStorageNode.h"
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLParser.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>

using namespace vtkDMMLCoreTestingUtilities;

//---------------------------------------------------------------------------
int vtkDMMLColorTableNodeTest1(int argc, char * argv[])
{
  vtkNew<vtkDMMLColorTableNode> node1;
  {
    vtkNew<vtkDMMLScene> scene;
    scene->AddNode(node1.GetPointer());
    EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());
  }

  if (argc != 2)
    {
    std::cerr << "Line " << __LINE__
              << " - Missing parameters !\n"
              << "Usage: " << argv[0] << " /path/to/temp"
              << std::endl;
    return EXIT_FAILURE;
    }

  const char* tempDir = argv[1];

  std::string sceneFileName = std::string(tempDir) + "/vtkDMMLColorTableNodeTest1.dmml";
  std::string colorTableFileName = std::string(tempDir) + "/vtkDMMLColorTableNodeTest1.ctbl";

  // check that extra single quotes don't appear in color names via round trip
  // to xml

  std::string expectedColorNodeId;

  {
    vtkNew<vtkDMMLColorTableNode> colorNode;
    colorNode->SetTypeToUser();
    colorNode->SetNumberOfColors(3);
    colorNode->SetColor(0, "zero", 0.0, 0.0, 0.0, 1.0);
    colorNode->SetColor(1, "one", 1.0, 0.0, 0.0, 1.0);
    colorNode->SetColor(2, "two", 0.0, 1.0, 0.0, 1.0);
    colorNode->NamesInitialisedOn();

    // add node to the scene
    vtkNew<vtkDMMLScene> scene;
    scene->SetRootDirectory(tempDir);
    CHECK_NOT_NULL(scene->AddNode(colorNode.GetPointer()));

    // add storage node to the scene
    vtkSmartPointer<vtkDMMLStorageNode> colorStorageNode =
        vtkSmartPointer<vtkDMMLStorageNode>::Take(colorNode->CreateDefaultStorageNode());

    scene->AddNode(colorStorageNode);
    colorNode->SetAndObserveStorageNodeID(colorStorageNode->GetID());

    // keep track of the id
    expectedColorNodeId = std::string(colorNode->GetID());

    // write color table file
    colorStorageNode->SetFileName(colorTableFileName.c_str());
    colorStorageNode->WriteData(colorNode.GetPointer());
    colorNode->SetName("CustomColorTable");

    // write DMML file
    scene->SetURL(sceneFileName.c_str());
    CHECK_INT(scene->Commit(), 1);
  }

  {
    vtkNew<vtkDMMLScene> scene;
    scene->SetRootDirectory(tempDir);
    vtkNew<vtkDMMLParser> parser;
    parser->SetDMMLScene(scene.GetPointer());
    parser->SetFileName(sceneFileName.c_str());
    CHECK_INT(parser->Parse(), 1);

    // test the color node
    vtkDMMLColorTableNode *colorNode =
        vtkDMMLColorTableNode::SafeDownCast(scene->GetNodeByID(expectedColorNodeId.c_str()));
    CHECK_NOT_NULL(colorNode);

    CHECK_INT(colorNode->GetStorageNode()->ReadData(colorNode),1);

    CHECK_STRING(colorNode->GetColorName(0), "zero")
    CHECK_STRING(colorNode->GetColorName(1), "one")
    CHECK_STRING(colorNode->GetColorName(2), "two")
  }

  return EXIT_SUCCESS;
}
