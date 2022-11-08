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
#include <qDMMLThreeDWidget.h>
#include <qCjyxAbstractModuleRepresentation.h>
#include <qCjyxApplication.h>

// Volumes includes
#include "qCjyxModelsModule.h"
#include "vtkCjyxModelsLogic.h"

// DMML includes
#include <vtkDMMLModelHierarchyNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkNew.h>
#include "qDMMLWidget.h"

//-----------------------------------------------------------------------------
int qCjyxModelsModuleWidgetTestScene( int argc, char * argv[] )
{
  qDMMLWidget::preInitializeApplication();
  qCjyxApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  if (argc < 2)
    {
    std::cerr << "Usage: qCjyxModelsModuleWidgetTest1 sceneFilePath [-I]"
              << std::endl;
    return EXIT_FAILURE;
    }

  qCjyxModelsModule module;
  module.initialize(nullptr);

  vtkNew<vtkDMMLScene> scene;
  scene->SetURL(argv[1]);
  scene->Connect();

  module.setDMMLScene(scene.GetPointer());

  dynamic_cast<QWidget*>(module.widgetRepresentation())->show();

  qDMMLThreeDWidget view;
  view.setDMMLScene(scene.GetPointer());
  view.setDMMLViewNode(vtkDMMLViewNode::SafeDownCast(
    scene->GetFirstNodeByClass("vtkDMMLViewNode")));
  view.show();

  if (argc < 3 || QString(argv[2]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}
