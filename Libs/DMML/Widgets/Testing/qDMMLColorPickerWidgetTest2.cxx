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
#include "vtkCjyxConfigure.h"

// CTK includes
#include <ctkColorDialog.h>

// qDMML includes
#include "qDMMLColorPickerWidget.h"

// DMML includes
#include <vtkDMMLColorTableNode.h>
#include <vtkDMMLPETProceduralColorNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkNew.h>
#include "qDMMLWidget.h"

int qDMMLColorPickerWidgetTest2(int argc, char * argv [])
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  qDMMLColorPickerWidget colorPickerWidget;

  vtkNew<vtkDMMLScene> scene;

  vtkNew<vtkDMMLColorTableNode> colorTableNode;
  colorTableNode->SetType(vtkDMMLColorTableNode::Labels);
  scene->AddNode(colorTableNode.GetPointer());

  colorPickerWidget.setDMMLScene(scene.GetPointer());

  // for some reasons it generate a warning if the type is changed.
  colorTableNode->NamesInitialisedOff();
  colorTableNode->SetTypeToCool1();

  vtkNew<vtkDMMLPETProceduralColorNode> colorPETNode;
  colorPETNode->SetTypeToRainbow();
  scene->AddNode(colorPETNode.GetPointer());

  ctkColorDialog::addDefaultTab(&colorPickerWidget, "Extra", SIGNAL(colorSelected(QColor)));

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    // quits the getColor dialog event loop.
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  ctkColorDialog::getColor(Qt::red, nullptr, "", ctkColorDialog::ColorDialogOptions());
  return EXIT_SUCCESS;
}

