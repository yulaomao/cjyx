/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx

=========================================================================auto=*/

#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLModelStorageNode.h"
#include "vtkDMMLModelNode.h"
#include "vtkDMMLScene.h"

#include <vtkCylinderSource.h>
#include <vtkGeometryFilter.h>
#include <vtkNew.h>
#include <vtkTriangleFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVoxel.h>

#include <array>

//---------------------------------------------------------------------------
int TestReadWriteData(vtkDMMLScene* scene, const char* extension, vtkPointSet* mesh, int coordinateSystem, bool cellsMayBeSubdivided = false);
void CreateVoxelMeshes(vtkUnstructuredGrid* ug, vtkPolyData* poly);

//---------------------------------------------------------------------------
int vtkDMMLModelStorageNodeTest1(int argc, char * argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " /path/to/temp" << std::endl;
    return EXIT_FAILURE;
  }

  vtkNew<vtkDMMLModelStorageNode> node1;
  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());

  vtkNew<vtkDMMLScene> scene;
  const char* tempDir = argv[1];
  scene->SetRootDirectory(tempDir);

  vtkNew<vtkUnstructuredGrid> ug;
  vtkNew<vtkPolyData> poly;
  CreateVoxelMeshes(ug.GetPointer(), poly.GetPointer());
  for (int coordinateSystem : { vtkDMMLStorageNode::CoordinateSystemRAS, vtkDMMLStorageNode::CoordinateSystemLPS })
    {
    CHECK_EXIT_SUCCESS(TestReadWriteData(scene.GetPointer(), ".vtk", poly.GetPointer(), coordinateSystem));
    CHECK_EXIT_SUCCESS(TestReadWriteData(scene.GetPointer(), ".vtp", poly.GetPointer(), coordinateSystem));
    CHECK_EXIT_SUCCESS(TestReadWriteData(scene.GetPointer(), ".stl", poly.GetPointer(), coordinateSystem, true));
    CHECK_EXIT_SUCCESS(TestReadWriteData(scene.GetPointer(), ".ply", poly.GetPointer(), coordinateSystem, true));
    CHECK_EXIT_SUCCESS(TestReadWriteData(scene.GetPointer(), ".obj", poly.GetPointer(), coordinateSystem));
    CHECK_EXIT_SUCCESS(TestReadWriteData(scene.GetPointer(), ".vtk", ug.GetPointer(), coordinateSystem));
    CHECK_EXIT_SUCCESS(TestReadWriteData(scene.GetPointer(), ".vtu", ug.GetPointer(), coordinateSystem));
    }

  std::cout << "Test passed." << std::endl;
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestReadWriteData(vtkDMMLScene* scene, const char *extension, vtkPointSet *mesh, int coordinateSystem, bool cellsMayBeSubdivided/*=false*/)
{
  std::string fileName = std::string(scene->GetRootDirectory()) +
    std::string("/vtkDMMLModelNodeTest1") +
    std::string(extension);

  int numberOfPoints = mesh->GetNumberOfPoints();
  CHECK_BOOL(numberOfPoints > 0, true);

  int numberOfCells = mesh->GetNumberOfCells();
  CHECK_BOOL(numberOfCells > 0, true);

  // Add model node
  vtkNew<vtkDMMLModelNode> modelNode;
  modelNode->SetAndObserveMesh(mesh);
  CHECK_NOT_NULL(modelNode->GetMesh());
  CHECK_NOT_NULL(scene->AddNode(modelNode.GetPointer()));

  bool isPoly = (modelNode->GetMeshType() == vtkDMMLModelNode::PolyDataMeshType);
  std::cout << "Testing " << extension << " for "
    << (isPoly ? "polydata" : "unstructured grid")
    << " mesh"
    << " in coordinate system: "
    << vtkDMMLStorageNode::GetCoordinateSystemTypeAsString(coordinateSystem) << std::endl;

  // Add storage node
  modelNode->AddDefaultStorageNode();
  vtkDMMLModelStorageNode* storageNode = vtkDMMLModelStorageNode::SafeDownCast(modelNode->GetStorageNode());
  CHECK_NOT_NULL(storageNode);
  storageNode->SetFileName(fileName.c_str());
  storageNode->SetCoordinateSystem(coordinateSystem);

  // Test writing
  CHECK_BOOL(storageNode->WriteData(modelNode.GetPointer()), true);

  // Clear data from model node
  modelNode->SetAndObservePolyData(nullptr);
  storageNode->SetCoordinateSystem(vtkDMMLStorageNode::CoordinateSystemLPS);
  CHECK_NULL(modelNode->GetMesh());

  // Test reading
  CHECK_BOOL(storageNode->ReadData(modelNode.GetPointer()), true);
  vtkPointSet* mesh2 = modelNode->GetMesh();
  CHECK_NOT_NULL(mesh2);
  CHECK_INT(mesh2->GetNumberOfPoints(), numberOfPoints);
  if (cellsMayBeSubdivided)
    {
    // cells may be subdivided when written to file, so we don't check for strict equality
    // but check if we have at least that many as in the original mesh
    CHECK_BOOL(mesh2->GetNumberOfCells() >= numberOfCells, true);
    }
  else
    {
    CHECK_INT(mesh2->GetNumberOfCells(), numberOfCells);
    }

  // Check extents to make sure there is no mirroring of the model due to coordinate system mismatch
  double originalBounds[6] = { 0.0 };
  mesh->GetBounds(originalBounds);
  double restoredBounds[6] = { 0.0 };
  mesh2->GetBounds(restoredBounds);
  for (int boundsIndex = 0; boundsIndex < 6; boundsIndex++)
    {
    CHECK_DOUBLE_TOLERANCE(restoredBounds[boundsIndex], originalBounds[boundsIndex], 1e-3);
    }

  // Coordinate system after writing must be the one that was requested
  CHECK_INT(storageNode->GetCoordinateSystem(), coordinateSystem);

  // Check if coordinate system hint is overridden by coordinate system specified in file
  int someDifferentCoordinateSystem =
    coordinateSystem == vtkDMMLStorageNode::CoordinateSystemRAS ? vtkDMMLStorageNode::CoordinateSystemLPS : vtkDMMLStorageNode::CoordinateSystemRAS;
  storageNode->SetCoordinateSystem(someDifferentCoordinateSystem);
  CHECK_BOOL(storageNode->ReadData(modelNode.GetPointer()), true);
  CHECK_INT(storageNode->GetCoordinateSystem(), coordinateSystem);

  return EXIT_SUCCESS;
}
