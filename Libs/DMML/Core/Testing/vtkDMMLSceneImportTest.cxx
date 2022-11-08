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

// DMML includes
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkNew.h>

//---------------------------------------------------------------------------
int vtkDMMLSceneImportTest(int argc, char * argv[] )
{
  if (argc < 2)
    {
    std::cout << "Usage: vtkDMMLSceneImportTest scene_file_path.dmml"
              << std::endl;
    return EXIT_FAILURE;
    }
  const char* sceneFilePath = argv[1];

  //---------------------------------------------------------------------------
  vtkNew<vtkDMMLScene> scene;

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  scene->SetURL(nullptr);
  scene->Connect();
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  scene->SetURL("");
  scene->Connect();
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  scene->SetURL(nullptr);
  scene->Import();
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  scene->SetURL("");
  scene->Import();
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  CHECK_EXIT_SUCCESS(vtkDMMLCoreTestingUtilities::ExerciseSceneLoadingMethods(sceneFilePath));

  return EXIT_SUCCESS;
}
