/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLDisplayableHierarchyNode.h"
#include "vtkDMMLModelDisplayNode.h"
#include "vtkDMMLModelNode.h"
#include "vtkDMMLScalarVolumeDisplayNode.h"
#include "vtkDMMLScalarVolumeNode.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkNew.h>

// test more general hierarchy uses, with different displayable node types
int vtkDMMLDisplayableHierarchyNodeTest2(int , char * [] )
{
  vtkNew<vtkDMMLScene> scene;

  vtkNew< vtkDMMLDisplayableHierarchyNode > hnode1;
  scene->AddNode(hnode1.GetPointer());

  vtkNew<vtkDMMLModelDisplayNode> hdnode1;
  scene->AddNode(hdnode1.GetPointer());

  if (hdnode1->GetID())
    {
    hnode1->SetAndObserveDisplayNodeID(hdnode1->GetID());
    }
  else
    {
    std::cerr << "Error setting up a display node for the first hierarchy node: "
              << "id is null on hierarchy display node" << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkDMMLDisplayableHierarchyNode> hnode2;
  scene->AddNode(hnode2.GetPointer());

  vtkNew<vtkDMMLScalarVolumeDisplayNode> hdnode2;
  scene->AddNode(hdnode2.GetPointer());

  if (hdnode2->GetID())
    {
    hnode2->SetAndObserveDisplayNodeID(hdnode2->GetID());
    }
  else
    {
    std::cerr << "Error setting up a display node for the second hierarchy node: "
              << "id is null on hierarchy display node" << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkDMMLModelNode> mnode1;
  scene->AddNode(mnode1.GetPointer());

  vtkNew<vtkDMMLModelDisplayNode> mdnode1;
  scene->AddNode(mdnode1.GetPointer());

  if (mdnode1->GetID())
    {
    mnode1->SetAndObserveDisplayNodeID(mdnode1->GetID());
    }
  else
    {
    std::cerr << "Error setting up a display node for the first model node\n";
    return EXIT_FAILURE;
    }

  vtkNew<vtkDMMLScalarVolumeNode> vnode1;
  scene->AddNode(vnode1.GetPointer());

  vtkNew<vtkDMMLScalarVolumeDisplayNode> vdnode1;
  scene->AddNode(vdnode1.GetPointer());

  if (vdnode1->GetID())
    {
    vnode1->SetAndObserveDisplayNodeID(vdnode1->GetID());
    }
  else
    {
    std::cerr << "Error setting up a display node for the first volume node\n";
    return EXIT_FAILURE;
    }

  // now set up a hierarchy
  hnode2->SetDisplayableNodeID(vnode1->GetID());
  hnode2->SetParentNodeID(hnode1->GetID());
  hnode1->SetDisplayableNodeID(mnode1->GetID());

  return EXIT_SUCCESS;
}
