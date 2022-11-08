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
#include <QSignalSpy>
#include <QTimer>
#include <QTreeView>

// Cjyx includes
#include "vtkCjyxConfigure.h"

// CTK includes
#include "ctkCallback.h"
#include "ctkEventTranslatorPlayerWidget.h"
#include "ctkQtTestingUtility.h"

// qDMML includes
#include "qDMMLTreeView.h"
#include <qDMMLSceneTransformModel.h>
#include <qDMMLTreeViewEventPlayer.h>
#include <qDMMLTreeViewEventTranslator.h>

// DMML includes
#include <vtkDMMLApplicationLogic.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLModelNode.h>
#include <vtkDMMLModelDisplayNode.h>

// VTK includes
#include <vtkNew.h>
#include "qDMMLWidget.h"

// STD includes
#include <cstdlib>
#include <iostream>

namespace
{
QSignalSpy* SpyAction;
//-----------------------------------------------------------------------------
void checkFinalWidgetState(void* data)
  {
  qDMMLTreeView* widget = reinterpret_cast<qDMMLTreeView*>(data);

  CTKCOMPARE(widget->currentIndex().row(), 1);
  Q_UNUSED(widget);
  }
//-----------------------------------------------------------------------------
void checkFinalWidgetState2(void* data)
  {
  qDMMLTreeView* widget = reinterpret_cast<qDMMLTreeView*>(data);

  Q_UNUSED(widget);

  CTKCOMPARE(widget->currentIndex().row(), 1);
  }
}

//-----------------------------------------------------------------------------
int qDMMLTreeViewEventTranslatorPlayerTest1(int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  QString xmlDirectory = QString(argv[1]) + "/Libs/DMML/Widgets/Testing/";

  // ------------------------
  ctkEventTranslatorPlayerWidget etpWidget;
  ctkQtTestingUtility* testUtility = new ctkQtTestingUtility(&etpWidget);
  testUtility->addPlayer(new qDMMLTreeViewEventPlayer());
  testUtility->addTranslator(new qDMMLTreeViewEventTranslator());
  etpWidget.setTestUtility(testUtility);

  // Test case 1
  qDMMLTreeView widget;

  vtkNew<vtkDMMLScene> scene;
  vtkNew<vtkDMMLApplicationLogic> applicationLogic;
  applicationLogic->SetDMMLScene(scene.GetPointer());
  widget.setDMMLScene(scene.GetPointer());
  scene->SetURL(argv[2]);
  scene->Import();

  etpWidget.addTestCase(&widget,
                        xmlDirectory + "qDMMLTreeViewEventTranslatorPlayerTest1.xml",
                        &checkFinalWidgetState);

  // Test case 2
  qDMMLTreeView widget2;

  vtkNew<vtkDMMLModelNode> modelNode;
  vtkNew<vtkDMMLModelNode> modelNode2;
  vtkNew<vtkDMMLModelDisplayNode> displayModelNode;
  vtkNew<vtkDMMLModelDisplayNode> displayModelNode2;

  vtkNew<vtkDMMLScene> scene2;
  applicationLogic->SetDMMLScene(scene2.GetPointer());
  scene2->AddNode(modelNode.GetPointer());
  scene2->AddNode(modelNode2.GetPointer());
  scene2->AddNode(displayModelNode.GetPointer());
  scene2->AddNode(displayModelNode2.GetPointer());

  modelNode->SetAndObserveDisplayNodeID(displayModelNode->GetID());
  modelNode2->SetAndObserveDisplayNodeID(displayModelNode2->GetID());

  widget2.setSceneModelType("ModelHierarchy");
  widget2.setDMMLScene(scene2.GetPointer());

  QAction* insertTransformAction = new QAction("Insert transform", nullptr);
  widget2.prependNodeMenuAction(insertTransformAction);
  widget2.prependSceneMenuAction(insertTransformAction);

  QSignalSpy spyAction(insertTransformAction, SIGNAL(triggered()));
  SpyAction = &spyAction;

  etpWidget.addTestCase(&widget2,
                        xmlDirectory + "qDMMLTreeViewEventTranslatorPlayerTest2.xml",
                        &checkFinalWidgetState2);
  // ------------------------
  if (!app.arguments().contains("-I"))
    {
    QTimer::singleShot(0, &etpWidget, SLOT(play()));
    }

  etpWidget.show();

  return app.exec();
}

