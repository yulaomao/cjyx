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
#include "qDMMLVolumeThresholdWidget.h"

// DMML includes
#include <vtkDMMLApplicationLogic.h>
#include <vtkDMMLColorLogic.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLVolumeNode.h>

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
  qDMMLVolumeThresholdWidget* widget = reinterpret_cast<qDMMLVolumeThresholdWidget*>(data);

  Q_UNUSED(widget);
  }
}

//-----------------------------------------------------------------------------
int qDMMLVolumeThresholdWidgetEventTranslatorPlayerTest1(int argc, char * argv [] )
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
  vtkNew<vtkDMMLScene> scene;
  vtkNew<vtkDMMLApplicationLogic> applicationLogic;
  applicationLogic->SetDMMLScene(scene.GetPointer());
  scene->SetURL(argv[2]);
  scene->Connect();

  // Add default color nodes
  vtkNew<vtkDMMLColorLogic> colorLogic;
  colorLogic->SetDMMLScene(scene.GetPointer());
  // need to set it back to nullptr, otherwise the logic removes the nodes that it added when it is destructed
  colorLogic->SetDMMLScene(nullptr);

  vtkDMMLNode* node = scene->GetFirstNodeByClass("vtkDMMLScalarVolumeNode");
  vtkDMMLVolumeNode* volumeNode = vtkDMMLVolumeNode::SafeDownCast(node);

  qDMMLVolumeThresholdWidget volumeThreshold;
  volumeThreshold.setDMMLVolumeNode(volumeNode);

  etpWidget.addTestCase(&volumeThreshold,
                        xmlDirectory + "qDMMLVolumeThresholdWidgetEventTranslatorPlayerTest1.xml",
                        &checkFinalWidgetState);

  // ------------------------
  if (!app.arguments().contains("-I"))
    {
    QTimer::singleShot(0, &etpWidget, SLOT(play()));
    }

  etpWidget.show();
  return app.exec();
}

