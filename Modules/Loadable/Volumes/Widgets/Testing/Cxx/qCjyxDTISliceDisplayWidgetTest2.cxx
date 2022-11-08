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

// Qt includes
#include <QApplication>
#include <QTimer>

// Cjyx includes
#include "vtkDMMLApplicationLogic.h"
#include "vtkDMMLColorLogic.h"
#include "vtkCjyxConfigure.h"

// Volumes includes
#include "qCjyxDTISliceDisplayWidget.h"
#include <vtkCjyxVolumesLogic.h>

// DMML includes
#include <vtkDMMLDiffusionTensorVolumeSliceDisplayNode.h>
#include <vtkDMMLDiffusionTensorDisplayPropertiesNode.h>
#include <qDMMLWidget.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkTrivialProducer.h>

// ITK includes
#include <itkConfigure.h>
#include <itkFactoryRegistration.h>

//-----------------------------------------------------------------------------
int qCjyxDTISliceDisplayWidgetTest2( int argc, char * argv[] )
{
  itk::itkFactoryRegistration();

  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  if (argc < 2)
    {
    std::cerr << "Usage: qCjyxDTISliceDisplayWidgetTest2 dtiFileName" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "file: " << argv[1] << std::endl;

  vtkNew<vtkDMMLScene> scene;

  vtkNew<vtkCjyxApplicationLogic> appLogic;

  // Add Color logic (used by volumes logic)
  vtkNew<vtkDMMLColorLogic> colorLogic;
  colorLogic->SetDMMLScene(scene.GetPointer());
  colorLogic->SetDMMLApplicationLogic(appLogic);
  appLogic->SetModuleLogic("Colors", colorLogic);

  // Add Volumes logic
  vtkNew<vtkCjyxVolumesLogic> volumesLogic;
  volumesLogic->SetDMMLScene(scene);
  volumesLogic->SetDMMLApplicationLogic(appLogic);
  appLogic->SetModuleLogic("Volumes", volumesLogic);

  vtkDMMLVolumeNode* volumeNode = volumesLogic->AddArchetypeVolume(argv[1], "dti");
  if (!volumeNode)
    {
    std::cerr << "Bad DTI file:" << argv[1] << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkDMMLDiffusionTensorDisplayPropertiesNode> propertiesNode;
  scene->AddNode(propertiesNode);
  vtkNew<vtkDMMLDiffusionTensorVolumeSliceDisplayNode> displayNode;
  displayNode->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(propertiesNode->GetID());
  scene->AddNode(displayNode);
  volumeNode->AddAndObserveDisplayNodeID(displayNode->GetID());
  vtkNew<vtkTrivialProducer> tp;
  tp->SetOutput(volumeNode->GetImageData());
  displayNode->SetSliceImagePort(tp->GetOutputPort());

  qCjyxDTISliceDisplayWidget widget;
  widget.setDMMLScene(scene);
  widget.setDMMLDTISliceDisplayNode(displayNode);

  widget.show();
  if (argc < 3 || QString(argv[2]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}
