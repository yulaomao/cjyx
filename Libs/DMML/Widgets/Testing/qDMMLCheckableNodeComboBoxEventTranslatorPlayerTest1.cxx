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
#include "qDMMLCheckableNodeComboBox.h"
#include "qDMMLCheckableNodeComboBoxEventPlayer.h"
#include "qDMMLSceneFactoryWidget.h"
#include "qDMMLWidget.h"

// STD includes
#include <cstdlib>
#include <iostream>

namespace
{
//-----------------------------------------------------------------------------
void checkFinalWidgetState(void* data)
  {
  qDMMLCheckableNodeComboBox* widget = reinterpret_cast<qDMMLCheckableNodeComboBox*>(data);

  Q_UNUSED(widget);
  }
}

//-----------------------------------------------------------------------------
int qDMMLCheckableNodeComboBoxEventTranslatorPlayerTest1(int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();
  QString xmlDirectory = QString(argv[1]) + "/Libs/DMML/Widgets/Testing/";
  // ------------------------
  ctkEventTranslatorPlayerWidget etpWidget;
  ctkQtTestingUtility* testUtility = new ctkQtTestingUtility(&etpWidget);
  testUtility->addPlayer(new qDMMLCheckableNodeComboBoxEventPlayer());
  etpWidget.setTestUtility(testUtility);

  // Test case 1
  qDMMLCheckableNodeComboBox* widget = new qDMMLCheckableNodeComboBox();
  widget->setNodeTypes(QStringList("vtkDMMLViewNode"));

  qDMMLSceneFactoryWidget sceneFactory;
  sceneFactory.generateScene();
  sceneFactory.generateNode("vtkDMMLViewNode");
  sceneFactory.generateNode("vtkDMMLViewNode");

  widget->setDMMLScene(sceneFactory.dmmlScene());
  sceneFactory.generateNode("vtkDMMLViewNode");
  sceneFactory.generateNode("vtkDMMLViewNode");
  sceneFactory.generateNode("vtkDMMLViewNode");

  etpWidget.addTestCase(widget,
                        xmlDirectory + "qDMMLCheckableNodeComboBoxEventTranslatorPlayerTest1.xml",
                        &checkFinalWidgetState);

  // ------------------------
  if (!app.arguments().contains("-I"))
    {
    QTimer::singleShot(0, &etpWidget, SLOT(play()));
    }

  etpWidget.show();
  return app.exec();
}

