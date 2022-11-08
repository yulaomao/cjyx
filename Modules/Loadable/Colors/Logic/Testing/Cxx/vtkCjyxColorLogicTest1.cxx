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

// DMMLLogic includes
#include "vtkCjyxColorLogic.h"

// DMML includes
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkTimerLog.h>

// STD includes

#include "vtkDMMLCoreTestingMacros.h"

using namespace vtkAddonTestingUtilities;
using namespace vtkDMMLCoreTestingUtilities;

//----------------------------------------------------------------------------
namespace
{
  int TestDefaults();
}

int vtkCjyxColorLogicTest1(int vtkNotUsed(argc), char * vtkNotUsed(argv)[])
{
  CHECK_EXIT_SUCCESS(TestDefaults());
  return EXIT_SUCCESS;
}
namespace
{

//----------------------------------------------------------------------------
int TestDefaults()
{
  vtkNew<vtkDMMLScene> scene;
  vtkCjyxColorLogic* colorLogic = vtkCjyxColorLogic::New();

  vtkNew<vtkTimerLog> overallTimer;
  overallTimer->StartTimer();

  colorLogic->SetDMMLScene(scene.GetPointer());

  overallTimer->StopTimer();
  std::cout << "AddDefaultColorNodes: " << overallTimer->GetElapsedTime() << "s"
            << " " << 1. / overallTimer->GetElapsedTime() << "fps" << std::endl;
  overallTimer->StartTimer();

  colorLogic->Delete();

  std::cout << "RemoveDefaultColorNodes: " << overallTimer->GetElapsedTime() << "s"
            << " " << 1. / overallTimer->GetElapsedTime() << "fps" << std::endl;

  return EXIT_SUCCESS;
}

}
