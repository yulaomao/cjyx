/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

// Logic includes
#include "vtkCjyxTransformLogic.h"

// DMML includes
#include "vtkDMMLScene.h"

// ITK includes
#include <itkConfigure.h>
#include <itkFactoryRegistration.h>

//-----------------------------------------------------------------------------
int vtkCjyxTransformLogicTest1(int argc, char * argv [])
{
  itk::itkFactoryRegistration();

  if(argc < 2)
    {
    std::cerr << "Missing transform file name." << std::endl;
    return EXIT_FAILURE;
    }

  vtkDMMLScene* scene = vtkDMMLScene::New();

  vtkCjyxTransformLogic* transformModuleLogic = vtkCjyxTransformLogic::New();
  transformModuleLogic->SetDMMLScene(scene);
  if (transformModuleLogic->GetDMMLScene() != scene)
    {
    std::cerr << "A DMML Scene must be set to go further." << std::endl;
    return EXIT_FAILURE;
    }

  vtkDMMLTransformNode* transform = transformModuleLogic->AddTransform(argv[1], scene);
  if (transform == nullptr)
    {
    std::cerr << "Could not read transform file: " << argv[1] << std::endl;
    return EXIT_FAILURE;
    }

  transformModuleLogic->Delete();
  scene->Delete();

  return EXIT_SUCCESS;
}
