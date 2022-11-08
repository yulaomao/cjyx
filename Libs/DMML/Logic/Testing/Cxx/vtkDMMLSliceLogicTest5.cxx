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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/
// DMMLLogic includes
#include <vtkDMMLSliceLogic.h>
#include <vtkDMMLSliceLayerLogic.h>

// DMML includes
#include <vtkDMMLColorTableNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLScalarVolumeDisplayNode.h>
#include <vtkDMMLModelDisplayNode.h>
#include <vtkDMMLSliceCompositeNode.h>
#include <vtkDMMLVolumeArchetypeStorageNode.h>

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkImageData.h>
#include <vtkImageViewer2.h>
#include <vtkNew.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTimerLog.h>
#include <vtkVersion.h>

// ITK includes
#include <itkConfigure.h>
#include <itkFactoryRegistration.h>

//-----------------------------------------------------------------------------
vtkDMMLScalarVolumeNode* vtkDMMLSliceLogicTest5_loadVolume(const char* volume, vtkDMMLScene* scene)
{
  vtkNew<vtkDMMLScalarVolumeDisplayNode> displayNode;
  vtkNew<vtkDMMLScalarVolumeNode> scalarNode;
  vtkNew<vtkDMMLVolumeArchetypeStorageNode> storageNode;

  displayNode->SetAutoWindowLevel(false);
  displayNode->SetInterpolate(false);

  storageNode->SetFileName(volume);
  if (storageNode->SupportedFileType(volume) == 0)
    {
    return nullptr;
    }
  scalarNode->SetName("foo");
  scalarNode->SetScene(scene);
  displayNode->SetScene(scene);
  //vtkCjyxColorLogic *colorLogic = vtkCjyxColorLogic::New();
  //displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultVolumeColorNodeID());
  //colorLogic->Delete();
  scene->AddNode(storageNode.GetPointer());
  scene->AddNode(displayNode.GetPointer());
  scalarNode->SetAndObserveStorageNodeID(storageNode->GetID());
  scalarNode->SetAndObserveDisplayNodeID(displayNode->GetID());
  scene->AddNode(scalarNode.GetPointer());
  storageNode->ReadData(scalarNode.GetPointer());

  vtkDMMLColorTableNode* colorNode = vtkDMMLColorTableNode::New();
  colorNode->SetTypeToGrey();
  scene->AddNode(colorNode);
  colorNode->Delete();
  displayNode->SetAndObserveColorNodeID(colorNode->GetID());

  return scalarNode.GetPointer();
}

//-----------------------------------------------------------------------------
int vtkDMMLSliceLogicTest5(int argc, char * argv [] )
{
  itk::itkFactoryRegistration();

  if( argc < 2 )
    {
    std::cerr << "Error: missing arguments" << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << "  input_image " << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkDMMLScene> scene;

  // Add default slice orientation presets
  vtkDMMLSliceNode::AddDefaultSliceOrientationPresets(scene.GetPointer());

  vtkNew<vtkDMMLSliceLogic> sliceLogic;
  sliceLogic->SetDMMLScene(scene.GetPointer());
  sliceLogic->AddSliceNode("Green");
  sliceLogic->ResizeSliceNode(256, 256);

  vtkDMMLSliceNode* sliceNode =sliceLogic->GetSliceNode();
  sliceNode->SetSliceResolutionMode(vtkDMMLSliceNode::SliceResolutionMatch2DView);

  vtkDMMLSliceCompositeNode* sliceCompositeNode = sliceLogic->GetSliceCompositeNode();

  vtkNew<vtkDMMLSliceLayerLogic> sliceLayerLogic;

  sliceLogic->SetBackgroundLayer(sliceLayerLogic.GetPointer());

  vtkDMMLScalarVolumeNode* scalarNode = vtkDMMLSliceLogicTest5_loadVolume(argv[1], scene.GetPointer());
  if (scalarNode == nullptr || scalarNode->GetImageData() == nullptr)
    {
    std::cerr << "Not a valid volume: " << argv[1] << std::endl;
    return EXIT_FAILURE;
    }

  vtkDMMLDisplayNode* displayNode = scalarNode->GetDisplayNode();
  //sliceLayerLogic->SetVolumeNode(scalarNode);
  sliceCompositeNode->SetBackgroundVolumeID(scalarNode->GetID());

  vtkAlgorithmOutput* textureImagePort = sliceLogic->GetSliceModelDisplayNode()->GetTextureImageDataConnection();
  vtkImageData* textureImage = vtkImageData::SafeDownCast(textureImagePort->GetProducer()->GetOutputDataObject(textureImagePort->GetIndex()));
  int* tdims = textureImage->GetDimensions();
  std::cout << "Texture dimension"  << tdims[0] << " " << tdims[1] << " " << tdims[2] << std::endl;

  vtkAlgorithmOutput* imgPort = sliceLogic->GetImageDataConnection();
  vtkImageData* img = vtkImageData::SafeDownCast(imgPort->GetProducer()->GetOutputDataObject(0));
  int* dims = img->GetDimensions();
  std::cout << "Logic dimension"  << dims[0] << " " << dims[1] << " " << dims[2] << std::endl;
  // Not sure why sliceLayerLogic->GetVolumeDisplayNode() is different from displayNode
  //vtkDMMLScalarVolumeDisplayNode* displayNode2 = vtkDMMLScalarVolumeDisplayNode::SafeDownCast(sliceLayerLogic->GetVolumeDisplayNode());

  for (int i = 0; i < 30; ++i)
    {
    vtkNew<vtkTimerLog> timerLog;
    timerLog->StartTimer();
    displayNode->Modified();
    timerLog->StopTimer();
    std::cout << "vtkDMMLDisplayNode::Modified(): " << timerLog->GetElapsedTime()
              << " fps: " << 1. / timerLog->GetElapsedTime() << std::endl;
    }
  vtkNew<vtkImageViewer2> viewer;
  //viewer->SetInput(appendComponents->GetOutput());
  viewer->SetInputConnection(sliceLogic->GetSliceModelDisplayNode()->GetTextureImageDataConnection());
  //viewer->SetInputConnection(appendComponents->GetOutputPort());

  // Renderer, RenderWindow and Interactor
  vtkRenderWindow* rw = viewer->GetRenderWindow();
  rw->SetSize(dims[0], dims[1]);
  rw->SetMultiSamples(0); // Ensure to have the same test image everywhere

  vtkRenderWindowInteractor* ri = vtkRenderWindowInteractor::New();
  viewer->SetupInteractor(ri);

  rw->Render();

  for (int i = 0; i < 30; ++i)
    {
    vtkSmartPointer<vtkTimerLog> timerLog = vtkSmartPointer<vtkTimerLog>::New();
    timerLog->StartTimer();
    displayNode->Modified();
    rw->Render();
    timerLog->StopTimer();
    std::cout << "vtkDMMLDisplayNode::Modified() + render: " << timerLog->GetElapsedTime()
              << " fps: " << 1. / timerLog->GetElapsedTime() << std::endl;
    }

  if (argc > 2 && std::string(argv[2]) == "-I")
    {
    ri->Start();
    }

  ri->Delete();

  return EXIT_SUCCESS;
}

