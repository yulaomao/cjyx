/*==============================================================================

Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
Queen's University, Kingston, ON, Canada. All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
and was supported through CANARIE's Research Software Program, and Cancer
Care Ontario.

==============================================================================*/

// DMML includes
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSegmentationNode.h"
#include "vtkDMMLSegmentationStorageNode.h"
#include "vtkSegmentationConverterFactory.h"

// Converter rules
#include "vtkClosedSurfaceToBinaryLabelmapConversionRule.h"
#include "vtkBinaryLabelmapToClosedSurfaceConversionRule.h"

#include "vtkFractionalLabelmapToClosedSurfaceConversionRule.h"
#include "vtkClosedSurfaceToFractionalLabelmapConversionRule.h"

int vtkDMMLSegmentationStorageNodeTest1(int argc, char * argv[] )
{
  vtkNew<vtkDMMLSegmentationStorageNode> node1;
  vtkNew<vtkDMMLScene> scene;
  scene->AddNode(node1.GetPointer());
  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());

  if (argc != 4)
    {
    std::cerr << "Line " << __LINE__
              << " - Missing parameters !\n"
              << "Usage: " << argv[0] << " /path/to/ITKSnapSegmentation.nii.gz /path/to/OldCjyxSegmentation.seg.nrrd /path/to/CjyxSegmentation.seg.nrrd"
              << std::endl;
    return EXIT_FAILURE;
    }

  vtkSegmentationConverterFactory* converterFactory = vtkSegmentationConverterFactory::GetInstance();
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkClosedSurfaceToBinaryLabelmapConversionRule>::New());
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkBinaryLabelmapToClosedSurfaceConversionRule>::New());
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkFractionalLabelmapToClosedSurfaceConversionRule>::New());
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkClosedSurfaceToFractionalLabelmapConversionRule>::New());

  const char* itkSnapSegmentationFilename = argv[1]; // ITKSnapSegmentation.nii.gz
  const char* oldCjyxSegmentationFilename = argv[2]; // OldCjyxSegmentation.seg.nrrd: Segmentation before shared labelmaps implemented.
  const char* cjyxSegmentationFilename = argv[3]; // CjyxSegmentation.seg.nrrd: Segmentation with shared labelmaps.

  // Test segmentation exported from ITK-SNAP
  std::cout << "Testing ITK-SNAP segmentation" << std::endl;
  {
    vtkNew<vtkDMMLSegmentationNode> segmentationNode;
    scene->AddNode(segmentationNode);
    vtkNew<vtkDMMLSegmentationStorageNode> segmentationStorageNode;
    scene->AddNode(segmentationStorageNode);
    segmentationStorageNode->SetFileName(itkSnapSegmentationFilename);
    segmentationStorageNode->ReadData(segmentationNode);
    vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
    CHECK_NOT_NULL(segmentation);

    int numberOfSegments = segmentation->GetNumberOfSegments();
    CHECK_INT(numberOfSegments, 4);

    int numberOfLayers = segmentation->GetNumberOfLayers(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName());
    CHECK_INT(numberOfLayers, 1);
  }

  // Test segmentation saved with Cjyx before shared labelmaps were implemented
  std::cout << "Testing pre shared labelmap segmentation" << std::endl;
  {
    vtkNew<vtkDMMLSegmentationNode> segmentationNode;
    scene->AddNode(segmentationNode);
    vtkNew<vtkDMMLSegmentationStorageNode> segmentationStorageNode;
    scene->AddNode(segmentationStorageNode);
    segmentationStorageNode->SetFileName(oldCjyxSegmentationFilename);
    segmentationStorageNode->ReadData(segmentationNode);
    vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
    CHECK_NOT_NULL(segmentation);

    int numberOfSegments = segmentation->GetNumberOfSegments();
    CHECK_INT(numberOfSegments, 3);

    int numberOfLayers = segmentation->GetNumberOfLayers(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName());
    CHECK_INT(numberOfLayers, 3);
  }

  std::cout << "Testing shared labelmap segmentation" << std::endl;
  {
    vtkNew<vtkDMMLSegmentationNode> segmentationNode;
    scene->AddNode(segmentationNode);
    vtkNew<vtkDMMLSegmentationStorageNode> segmentationStorageNode;
    scene->AddNode(segmentationStorageNode);
    segmentationStorageNode->SetFileName(cjyxSegmentationFilename);
    segmentationStorageNode->ReadData(segmentationNode);
    vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
    CHECK_NOT_NULL(segmentation);

    int numberOfSegments = segmentation->GetNumberOfSegments();
    CHECK_INT(numberOfSegments, 3);

    int numberOfLayers = segmentation->GetNumberOfLayers(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName());
    CHECK_INT(numberOfLayers, 2);
  }

  return EXIT_SUCCESS;
}
