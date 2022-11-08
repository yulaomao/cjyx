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
#include "qDMMLThreeDWidget.h"
#include "qDMMLThreeDViewControllerWidget.h"

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
  qDMMLThreeDWidget* widget = reinterpret_cast<qDMMLThreeDWidget*>(data);

  Q_UNUSED(widget);
  }
}

//-----------------------------------------------------------------------------
int qDMMLThreeDWidgetEventTranslatorPlayerTest1(int argc, char * argv [] )
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
  qDMMLThreeDWidget widget;
  widget.show();

  vtkNew<vtkDMMLScene> scene;

  // vtkDMMLAbstractDisplayableManager requires selection and interaction nodes
  vtkNew<vtkDMMLSelectionNode> selectionNode;
  scene->AddNode(selectionNode.GetPointer());
  vtkNew<vtkDMMLInteractionNode> interactionNode;
  scene->AddNode(interactionNode.GetPointer());

  vtkNew<vtkDMMLViewNode> viewNode;
  scene->AddNode(viewNode.GetPointer());

  widget.setViewLabel("View1");
  widget.setDMMLScene(scene.GetPointer());
  widget.setDMMLViewNode(viewNode.GetPointer());
  etpWidget.addTestCase(&widget,
                        xmlDirectory + "qDMMLThreeDWidgetEventTranslatorPlayerTest1.xml",
                        &checkFinalWidgetState);

  // ------------------------
  if (!app.arguments().contains("-I"))
    {
    QTimer::singleShot(0, &etpWidget, SLOT(play()));
    }

  etpWidget.show();
  return app.exec();
}

