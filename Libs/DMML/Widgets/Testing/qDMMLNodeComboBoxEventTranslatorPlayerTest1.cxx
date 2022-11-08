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
#include "ctkEventTranslatorPlayerWidget.h"
#include "ctkQtTestingUtility.h"

// qDMML includes
#include "qDMMLNodeComboBox.h"
#include "qDMMLNodeComboBoxEventPlayer.h"
#include "qDMMLNodeComboBoxEventTranslator.h"
#include "qDMMLSceneFactoryWidget.h"

// DMML includes
#include <vtkDMMLNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include "qDMMLWidget.h"

// STD includes
#include <cstdlib>
#include <iostream>

namespace
{
//-----------------------------------------------------------------------------
void checkFinalWidgetState(void* data)
  {
  qDMMLNodeComboBox* widget = reinterpret_cast<qDMMLNodeComboBox*>(data);

  Q_UNUSED(widget);
  }
}

//-----------------------------------------------------------------------------
int qDMMLNodeComboBoxEventTranslatorPlayerTest1(int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  QString xmlDirectory = QString(argv[1]) + "/Libs/DMML/Widgets/Testing/";

  // ------------------------
  ctkEventTranslatorPlayerWidget etpWidget;
  ctkQtTestingUtility* testUtility = new ctkQtTestingUtility(&etpWidget);
  testUtility->addTranslator(new qDMMLNodeComboBoxEventTranslator());
  testUtility->addPlayer(new qDMMLNodeComboBoxEventPlayer());
  etpWidget.setTestUtility(testUtility);

  // Test case 1
  qDMMLNodeComboBox* widget = new qDMMLNodeComboBox();
//  widget->setEditEnabled(true);
  widget->setRenameEnabled(true);
  qDMMLSceneFactoryWidget sceneFactory;

  sceneFactory.generateScene();
  sceneFactory.generateNode("vtkDMMLViewNode");
  widget->setNodeTypes(QStringList("vtkDMMLViewNode"));
  widget->setDMMLScene(sceneFactory.dmmlScene());

  etpWidget.addTestCase(widget,
                        xmlDirectory + "qDMMLNodeComboBoxEventTranslatorPlayerTest1.xml",
                        &checkFinalWidgetState);

//  sceneFactory.deleteScene();

  // ------------------------
  if (!app.arguments().contains("-I"))
    {
    QTimer::singleShot(0, &etpWidget, SLOT(play()));
    }

  etpWidget.show();
  return app.exec();
}

