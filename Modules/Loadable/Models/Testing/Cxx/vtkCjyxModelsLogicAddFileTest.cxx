/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Models logic
#include "vtkCjyxModelsLogic.h"

// DMML includes
#include "vtkDMMLCoreTestingMacros.h"
#include <vtkDMMLModelNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkNew.h>
#include <vtkPolyData.h>

//-----------------------------------------------------------------------------
int testAddEmptyFile(const char* filePath);
int testAddFile(const char* filePath);
int testAddModelWithPolyData(bool withPolyData);

//-----------------------------------------------------------------------------
int vtkCjyxModelsLogicAddFileTest( int argc, char * argv[] )
{
  // Test failure cases

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_EXIT_SUCCESS(testAddEmptyFile(0));
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_EXIT_SUCCESS(testAddEmptyFile(""));
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_EXIT_SUCCESS(testAddEmptyFile("non existing file.badextension"));
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_EXIT_SUCCESS(testAddEmptyFile("non existing file.vtk"));
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  // Test success cases

  if (argc > 1)
    {
    CHECK_EXIT_SUCCESS(testAddFile(argv[1]));
    }
  CHECK_EXIT_SUCCESS(testAddModelWithPolyData(false));
  CHECK_EXIT_SUCCESS(testAddModelWithPolyData(true));

  return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
int testAddEmptyFile(const char * filePath)
{
  vtkNew<vtkCjyxModelsLogic> modelsLogic;
  vtkDMMLModelNode* model = modelsLogic->AddModel(filePath);

  // Adding an invalid file shall not return a valid model
  CHECK_NULL(model);

  vtkNew<vtkDMMLScene> scene;
  modelsLogic->SetDMMLScene(scene);
  int nodeCountBefore = scene->GetNumberOfNodes();
  model = modelsLogic->AddModel(filePath);
  int nodeCountAfter = scene->GetNumberOfNodes();

  // Adding an invalid file shall not return a valid model
  CHECK_NULL(model);
  // Adding an invalid file shall not leave any nodes in the scene
  CHECK_INT(nodeCountAfter - nodeCountBefore, 0);

  return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
int testAddFile(const char * filePath)
{
  vtkNew<vtkCjyxModelsLogic> modelsLogic;
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  vtkDMMLModelNode* model = modelsLogic->AddModel(filePath);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  // File can't be loaded if no scene is set.
  CHECK_NULL(model);

  vtkNew<vtkDMMLScene> scene;
  modelsLogic->SetDMMLScene(scene);
  int nodeCountBefore = scene->GetNumberOfNodes();
  model = modelsLogic->AddModel(filePath);
  int nodeCountAfter = scene->GetNumberOfNodes();

  // Valid model node is expected
  CHECK_NOT_NULL(model);
  // 3 new nodes are expected: model, display, storage node
  CHECK_INT(nodeCountAfter - nodeCountBefore, 3);

  return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
int testAddModelWithPolyData(bool withPolyData)
{
  vtkNew<vtkPolyData> polyData;
  vtkPolyData* poly = (withPolyData ? polyData.GetPointer() : nullptr);
  vtkNew<vtkCjyxModelsLogic> modelsLogic;

  // Empty input should fail
  CHECK_NULL(modelsLogic->AddModel(poly));

  vtkNew<vtkDMMLScene> scene;
  modelsLogic->SetDMMLScene(scene);
  vtkDMMLModelNode* model = modelsLogic->AddModel(poly);
  CHECK_POINTER(model->GetPolyData(), poly);
  CHECK_NOT_NULL(model->GetModelDisplayNode());

  return EXIT_SUCCESS;
}

