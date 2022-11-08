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
#include <QTimer>
#include <QWidget>

// Cjyx includes
#include "vtkCjyxConfigure.h"

// Cjyx includes
#include <qCjyxAbstractModuleRepresentation.h>
#include <qCjyxApplication.h>

// Volumes includes
#include "qCjyxVolumesModule.h"
#include "vtkCjyxVolumesLogic.h"

// VTK includes
#include <vtkNew.h>
#include "qDMMLWidget.h"

// ITK includes
#include <itkConfigure.h>
#include <itkFactoryRegistration.h>

//-----------------------------------------------------------------------------
int qCjyxVolumesModuleWidgetTest1( int argc, char * argv[] )
{
  itk::itkFactoryRegistration();

  qDMMLWidget::preInitializeApplication();
  qCjyxApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  if (argc < 2)
    {
    std::cerr << "Usage: qCjyxVolumesModuleWidgetTest1 volumeName [-I]" << std::endl;
    return EXIT_FAILURE;
    }

  qCjyxVolumesModule module;
  module.initialize(nullptr);

  vtkNew<vtkDMMLScene> scene;
  vtkNew<vtkCjyxVolumesLogic> volumesLogic;
  volumesLogic->SetDMMLScene(scene.GetPointer());

  vtkDMMLVolumeNode* volumeNode = volumesLogic->AddArchetypeVolume(argv[1], "volume");
  if (!volumeNode)
    {
    std::cerr << "Bad volume file:" << argv[1] << std::endl;
    return EXIT_FAILURE;
    }
  module.setDMMLScene(scene.GetPointer());

  dynamic_cast<QWidget*>(module.widgetRepresentation())->show();

  if (argc < 3 || QString(argv[2]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}
