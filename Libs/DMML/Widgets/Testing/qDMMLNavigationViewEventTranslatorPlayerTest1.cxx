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
#include <QHBoxLayout>
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
#include "qDMMLNavigationView.h"
#include "qDMMLThreeDView.h"

// DMML includes
#include <vtkDMMLInteractionNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSelectionNode.h>
#include <vtkDMMLViewNode.h>

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
  qDMMLNavigationView* widget = reinterpret_cast<qDMMLNavigationView*>(data);

  Q_UNUSED(widget);
  }
}

//-----------------------------------------------------------------------------
int qDMMLNavigationViewEventTranslatorPlayerTest1(int argc, char * argv [] )
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
  QWidget topLevel;

  qDMMLNavigationView navigationView;
  navigationView.setWindowTitle("Navigation view");

  qDMMLThreeDView threeDView;
  threeDView.setWindowTitle("ThreeDView");

  QHBoxLayout* hboxLayout = new QHBoxLayout;
  hboxLayout->addWidget(&navigationView);
  hboxLayout->addWidget(&threeDView);
  topLevel.setLayout(hboxLayout);

  navigationView.setRendererToListen(threeDView.renderer());

  vtkNew<vtkDMMLScene> scene;

  // vtkDMMLAbstractDisplayableManager requires selection and interaction nodes
  vtkNew<vtkDMMLSelectionNode> selectionNode;
  scene->AddNode(selectionNode.GetPointer());
  vtkNew<vtkDMMLInteractionNode> interactionNode;
  scene->AddNode(interactionNode.GetPointer());

  navigationView.setDMMLScene(scene.GetPointer());
  threeDView.setDMMLScene(scene.GetPointer());

  vtkDMMLViewNode* viewNode = vtkDMMLViewNode::New();
  viewNode->SetBoxVisible(true);
  scene->AddNode(viewNode);
  viewNode->Delete();

  threeDView.setDMMLViewNode(viewNode);
  navigationView.setDMMLViewNode(viewNode);

  etpWidget.addTestCase(&topLevel,
                        xmlDirectory + "qDMMLNavigationViewEventTranslatorPlayerTest1.xml",
                        &checkFinalWidgetState);

  // ------------------------
  if (!app.arguments().contains("-I"))
    {
    QTimer::singleShot(0, &etpWidget, SLOT(play()));
    }

  etpWidget.show();
  return app.exec();
}

