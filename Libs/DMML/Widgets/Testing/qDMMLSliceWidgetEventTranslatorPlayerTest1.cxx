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
#include "qDMMLSliceWidget.h"
#include "qDMMLNodeObject.h"

// DMML includes
#include <vtkDMMLApplicationLogic.h>
#include <vtkDMMLColorLogic.h>
#include <vtkDMMLDisplayNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLSliceViewDisplayableManagerFactory.h>
#include <vtkDMMLVolumeNode.h>

// VTK includes
#include <vtkMultiThreader.h>
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
  qDMMLSliceWidget* widget = reinterpret_cast<qDMMLSliceWidget*>(data);

  Q_UNUSED(widget);
  }
}

//-----------------------------------------------------------------------------
int qDMMLSliceWidgetEventTranslatorPlayerTest1(int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  QString xmlDirectory = QString(argv[1]) + "/Libs/DMML/Widgets/Testing/";

  // ------------------------
  ctkEventTranslatorPlayerWidget etpWidget;
  ctkQtTestingUtility* testUtility = new ctkQtTestingUtility(&etpWidget);
  etpWidget.setTestUtility(testUtility);

  vtkNew<vtkDMMLApplicationLogic> applicationLogic;
  vtkDMMLSliceViewDisplayableManagerFactory::GetInstance()->SetDMMLApplicationLogic(applicationLogic);

  vtkNew<vtkDMMLColorLogic> colorLogic;

  // Test case 1
  vtkNew<vtkDMMLScene> scene;
  applicationLogic->SetDMMLScene(scene.GetPointer());
  colorLogic->SetDMMLScene(scene.GetPointer());

  scene->SetURL(argv[2]);
  scene->Connect();

  vtkDMMLSliceNode* redSliceNode = nullptr;
  // search for a red slice node
  std::vector<vtkDMMLNode*> sliceNodes;
  scene->GetNodesByClass("vtkDMMLSliceNode", sliceNodes);

  for (unsigned int i = 0; i < sliceNodes.size(); ++i)
    {
    vtkDMMLSliceNode* sliceNode = vtkDMMLSliceNode::SafeDownCast(sliceNodes[i]);
    if (!strcmp(sliceNode->GetLayoutName(), "Red") )
      {
      redSliceNode = sliceNode;
      break;
      }
    }
  if (!redSliceNode)
    {
    std::cerr << "Scene must contain a valid vtkDMMLSliceNode:" << redSliceNode << std::endl;
    return EXIT_FAILURE;
    }

  // "Red" slice by default
  qDMMLSliceWidget sliceWidget;
  sliceWidget.setDMMLScene(scene.GetPointer());

  sliceWidget.setDMMLSliceNode(redSliceNode);

  etpWidget.addTestCase(&sliceWidget,
                        xmlDirectory + "qDMMLSliceWidgetEventTranslatorPlayerTest1.xml",
                        &checkFinalWidgetState);

  // ------------------------
  if (!app.arguments().contains("-I"))
    {
    QTimer::singleShot(0, &etpWidget, SLOT(play()));
    }

  etpWidget.show();
  return app.exec();
}

