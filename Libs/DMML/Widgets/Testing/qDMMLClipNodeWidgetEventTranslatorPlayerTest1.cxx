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
#include "qDMMLClipNodeWidget.h"

// DMML includes
#include <vtkDMMLClipModelsNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include "qDMMLWidget.h"

// STD includes
#include <cstdlib>
#include <iostream>

namespace
{
//-----------------------------------------------------------------------------
void checkFinalWidgetState(void* data)
  {
  qDMMLClipNodeWidget* widget = reinterpret_cast<qDMMLClipNodeWidget*>(data);

  CTKCOMPARE(widget->redSliceClipState(), vtkDMMLClipModelsNode::ClipPositiveSpace);
  CTKCOMPARE(widget->greenSliceClipState(), vtkDMMLClipModelsNode::ClipNegativeSpace);
  CTKCOMPARE(widget->clipType(), vtkDMMLClipModelsNode::ClipUnion);
  CTKCOMPARE(widget->yellowSliceClipState(), vtkDMMLClipModelsNode::ClipOff);
  }
}

//-----------------------------------------------------------------------------
int qDMMLClipNodeWidgetEventTranslatorPlayerTest1(int argc, char * argv [] )
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
  qDMMLClipNodeWidget* widget = new qDMMLClipNodeWidget();

  vtkSmartPointer< vtkDMMLClipModelsNode > clipNode =
    vtkSmartPointer< vtkDMMLClipModelsNode >::New();

  widget->setDMMLClipNode(clipNode);

  etpWidget.addTestCase(widget,
                        xmlDirectory + "qDMMLClipNodeWidgetEventTranslatorPlayerTest1.xml",
                        &checkFinalWidgetState);

  // ------------------------
  if (!app.arguments().contains("-I"))
    {
    QTimer::singleShot(0, &etpWidget, SLOT(play()));
    }

  etpWidget.show();
  return app.exec();
}

