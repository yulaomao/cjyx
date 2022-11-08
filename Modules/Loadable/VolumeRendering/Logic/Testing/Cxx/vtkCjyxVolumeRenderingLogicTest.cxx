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

// VolumeRendering includes
#include <vtkDMMLVolumeRenderingDisplayNode.h>
#include <vtkCjyxVolumeRenderingLogic.h>
#include <vtkDMMLVolumePropertyNode.h>

// DMML includes
#include <vtkDMMLCoreTestingMacros.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLVolumeNode.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

//----------------------------------------------------------------------------
int testDefaultRenderingMethod(const std::string& moduleShareDirectory);
int testPresets(const std::string &moduleShareDirectory);

//----------------------------------------------------------------------------
int vtkCjyxVolumeRenderingLogicTest(int argc, char* argv[])
{
  if (argc != 2)
    {
    std::cout << "Missing moduleShare Directory argument !" << std::endl;
    return EXIT_FAILURE;
    }
  std::string moduleShareDirectory(argv[1]);

  CHECK_EXIT_SUCCESS(testDefaultRenderingMethod(moduleShareDirectory));
  CHECK_EXIT_SUCCESS(testPresets(moduleShareDirectory));
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int testDefaultRenderingMethod(const std::string& moduleShareDirectory)
{
  vtkNew<vtkCjyxVolumeRenderingLogic> logic;
  logic->SetModuleShareDirectory(moduleShareDirectory);

  vtkDMMLVolumeRenderingDisplayNode* displayNode = logic->CreateVolumeRenderingDisplayNode();
  CHECK_NULL(logic->GetDefaultRenderingMethod());
  CHECK_NULL(displayNode);

  vtkNew<vtkDMMLScene> scene;
  logic->SetDMMLScene(scene.GetPointer());
  displayNode = logic->CreateVolumeRenderingDisplayNode();
  CHECK_NOT_NULL(displayNode);
  CHECK_BOOL(displayNode->IsA("vtkDMMLGPURayCastVolumeRenderingDisplayNode"), true);
  displayNode->Delete();

  logic->SetDefaultRenderingMethod("vtkDMMLCPURayCastVolumeRenderingDisplayNode");
  displayNode = logic->CreateVolumeRenderingDisplayNode();
  CHECK_NOT_NULL(displayNode);
  CHECK_BOOL(displayNode->IsA("vtkDMMLCPURayCastVolumeRenderingDisplayNode"), true);
  displayNode->Delete();

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int testPresets(const std::string& moduleShareDirectory)
{
  vtkNew<vtkCjyxVolumeRenderingLogic> logic;
  logic->SetModuleShareDirectory(moduleShareDirectory);

  CHECK_NOT_NULL(logic->GetPresetsScene());
  CHECK_NOT_NULL(logic->GetPresetByName("MR-Default"));

  vtkNew<vtkDMMLVolumePropertyNode> newPreset;
  newPreset->SetName("MyNewPreset");
  CHECK_NULL(logic->GetPresetByName("MyNewPreset"));
  logic->AddPreset(newPreset.GetPointer());
  CHECK_NOT_NULL(logic->GetPresetByName("MyNewPreset"));

  vtkNew<vtkImageData> iconImage;
  vtkNew<vtkDMMLVolumePropertyNode> newPresetWithIcon;
  newPresetWithIcon->SetName("MyNewPresetWithIcon");
  logic->AddPreset(newPresetWithIcon.GetPointer(), iconImage.GetPointer());
  vtkDMMLNode* newPresetWithIcon2 = logic->GetPresetByName("MyNewPresetWithIcon");
  CHECK_NOT_NULL(newPresetWithIcon2);
  vtkDMMLVolumeNode* iconNode = vtkDMMLVolumeNode::SafeDownCast(
    newPresetWithIcon2->GetNodeReference(vtkCjyxVolumeRenderingLogic::GetIconVolumeReferenceRole()));
  CHECK_NOT_NULL(iconNode);
  CHECK_POINTER(iconNode->GetImageData(), iconImage.GetPointer());

  return EXIT_SUCCESS;
}
