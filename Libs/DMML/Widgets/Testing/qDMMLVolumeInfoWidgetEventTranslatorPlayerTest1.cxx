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
#include "qDMMLVolumeInfoWidget.h"

// DMML includes
#include <vtkDMMLColorLogic.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLScalarVolumeDisplayNode.h>
#include <vtkDMMLScalarVolumeNode.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkVersion.h>
#include "qDMMLWidget.h"

// STD includes
#include <cstdlib>
#include <iostream>

namespace
{
//-----------------------------------------------------------------------------
void checkFinalWidgetState(void* data)
  {
  qDMMLVolumeInfoWidget* widget = reinterpret_cast<qDMMLVolumeInfoWidget*>(data);

  Q_UNUSED(widget);
  }
}

//-----------------------------------------------------------------------------
int qDMMLVolumeInfoWidgetEventTranslatorPlayerTest1(int argc, char * argv [] )
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
  vtkNew<vtkDMMLScalarVolumeNode> volumeNode;

  vtkNew<vtkImageData> imageData;
  imageData->SetDimensions(256, 256, 1);
  //imageData->SetSpacing(2., 2., 512.); not used by vtkDMMLVolumeNode
  //imageData->SetOrigin(0.0,0.0,0.0); not used by vtkDMMLVolumeNode
  imageData->AllocateScalars(VTK_UNSIGNED_SHORT, 1); // allocate storage for image data
  volumeNode->SetAndObserveImageData(imageData.GetPointer());
  volumeNode->SetSpacing(2., 2., 512.);
  volumeNode->SetOrigin(0, 0, 0);

  vtkNew<vtkDMMLScalarVolumeDisplayNode> displayNode;
  vtkNew<vtkDMMLScene> scene;

  // Add default color nodes
  vtkNew<vtkDMMLColorLogic> colorLogic;
  colorLogic->SetDMMLScene(scene.GetPointer());

  scene->AddNode(volumeNode.GetPointer());
  scene->AddNode(displayNode.GetPointer());

  volumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());

  qDMMLVolumeInfoWidget volumeInfo;
  volumeInfo.setVolumeNode(volumeNode.GetPointer());
  etpWidget.addTestCase(&volumeInfo,
                        xmlDirectory + "qDMMLVolumeInfoWidgetEventTranslatorPlayerTest1.xml",
                        &checkFinalWidgetState);

  // ------------------------
  if (!app.arguments().contains("-I"))
    {
    QTimer::singleShot(0, &etpWidget, SLOT(play()));
    }

  etpWidget.show();
  return app.exec();
}

