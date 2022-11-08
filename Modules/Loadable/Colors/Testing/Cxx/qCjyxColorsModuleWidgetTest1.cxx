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

  This file was originally developed by Luis Ibanez, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QTimer>

// Cjyx includes
#include "vtkCjyxConfigure.h"

// Cjyx includes
#include "qCjyxApplication.h"
#include "qCjyxColorsModule.h"
#include "qCjyxColorsModuleWidget.h"

// DMMLLogic includes
#include <vtkDMMLColorLogic.h>

// DMML includes
#include <vtkDMMLScene.h>

// VTK includes
#include "qDMMLWidget.h"

// STD includes

#include "vtkDMMLCoreTestingMacros.h"

int qCjyxColorsModuleWidgetTest1(int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  vtkSmartPointer<vtkDMMLScene> scene = vtkSmartPointer<vtkDMMLScene>::New();
  vtkSmartPointer<vtkDMMLColorLogic> colorLogic = vtkSmartPointer<vtkDMMLColorLogic>::New();
  colorLogic->SetDMMLScene(scene);

  qCjyxColorsModule colorsModule;
  colorsModule.setDMMLScene(scene);
  colorsModule.initialize(nullptr);

  qCjyxColorsModuleWidget* colorsWidget =
    dynamic_cast<qCjyxColorsModuleWidget*>(colorsModule.widgetRepresentation());
  colorsWidget->show();

  std::vector< vtkDMMLNode* > nodes;
  scene->GetNodesByClass("vtkDMMLColorNode", nodes);
  for (std::vector< vtkDMMLNode* >::iterator nodeIt = nodes.begin(); nodeIt != nodes.end(); ++nodeIt)
    {
    colorsWidget->setCurrentColorNode(*nodeIt);
    }

  // colorsWidget->show();

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(100, qApp, SLOT(quit()));
    }

  return app.exec();
}

