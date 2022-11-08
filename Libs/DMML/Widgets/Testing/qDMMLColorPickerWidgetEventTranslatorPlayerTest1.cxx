/*=========================================================================

  Program: 3D Cjyx

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Benjamin LONG, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

=========================================================================*/

// Qt includes
#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QStandardItemModel>
#include <QTimer>
#include <QTreeView>

// Cjyx includes
#include "vtkCjyxConfigure.h"

// CTK includes
#include "ctkCallback.h"
#include "ctkColorDialog.h"
#include "ctkEventTranslatorPlayerWidget.h"
#include "ctkQtTestingUtility.h"

// qDMML includes
#include "qDMMLColorPickerWidget.h"

// DMML includes
#include <vtkDMMLColorTableNode.h>
#include <vtkDMMLPETProceduralColorNode.h>
#include <vtkDMMLScene.h>

// DMMLLogic includes
#include <vtkDMMLColorLogic.h>

// VTK includes
#include <vtkNew.h>
#include "qDMMLWidget.h"

// STD includes
#include <cstdlib>
#include <iostream>

namespace
{
//-----------------------------------------------------------------------------
void checkFinalWidgetState(void* data)
  {
  qDMMLColorPickerWidget* widget = reinterpret_cast<qDMMLColorPickerWidget*>(data);

  Q_UNUSED(widget);
  }
}

//-----------------------------------------------------------------------------
int qDMMLColorPickerWidgetEventTranslatorPlayerTest1(int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  QString xmlDirectory = QString(argv[1]) + "/Libs/DMML/Widgets/Testing/";

  // ------------------------
  ctkEventTranslatorPlayerWidget etpWidget;
  ctkQtTestingUtility* testUtility = new ctkQtTestingUtility(&etpWidget);
  etpWidget.setTestUtility(testUtility);

  // Test case 1
  qDMMLColorPickerWidget* widget = new qDMMLColorPickerWidget();
  widget->setObjectName("ColorPickerWidget1");

  vtkNew<vtkDMMLScene> scene;

  widget->setDMMLScene(scene.GetPointer());
  vtkNew<vtkDMMLColorLogic> colorLogic;
  colorLogic->SetDMMLScene(scene.GetPointer());

  etpWidget.addTestCase(widget,
                        xmlDirectory + "qDMMLColorPickerWidgetEventTranslatorPlayerTest1.xml",
                        &checkFinalWidgetState);

  // Test case 2
  qDMMLColorPickerWidget* widget2 = new qDMMLColorPickerWidget();
  widget2->setObjectName("ColorPickerWidget2");

  vtkNew<vtkDMMLScene> scene2;

  vtkNew<vtkDMMLColorTableNode> colorTableNode;
  colorTableNode->SetType(vtkDMMLColorTableNode::Labels);
  scene2->AddNode(colorTableNode.GetPointer());

  widget2->setDMMLScene(scene2.GetPointer());

  // for some reasons it generate a warning if the type is changed.
  colorTableNode->NamesInitialisedOff();
  colorTableNode->SetTypeToCool1();

  vtkNew<vtkDMMLPETProceduralColorNode> colorPETNode;
  colorPETNode->SetTypeToRainbow();
  scene2->AddNode(colorPETNode.GetPointer());

  etpWidget.addTestCase(widget2,
                        xmlDirectory + "qDMMLColorPickerWidgetEventTranslatorPlayerTest2.xml",
                        &checkFinalWidgetState);

  // ------------------------
  if (!app.arguments().contains("-I"))
    {
    QTimer::singleShot(0, &etpWidget, SLOT(play()));
    }

  etpWidget.show();
  return app.exec();
}

