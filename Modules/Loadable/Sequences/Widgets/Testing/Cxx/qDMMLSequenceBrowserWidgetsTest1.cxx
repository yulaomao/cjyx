/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QTimer>
#include <QVBoxLayout>

// qDMML includes
#include "qCjyxCoreApplication.h"
#include "qDMMLSequenceBrowserPlayWidget.h"
#include "qDMMLSequenceBrowserSeekWidget.h"

// DMML includes
#include "vtkDMMLScene.h"
#include "vtkDMMLSequenceNode.h"
#include "vtkDMMLSequenceBrowserNode.h"
#include "vtkDMMLTransformNode.h"

// VTK includes
#include <vtkNew.h>

int qDMMLSequenceBrowserWidgetsTest1( int argc, char * argv [] )
{
  QApplication app(argc, argv);

  vtkNew<vtkDMMLScene> scene;

  vtkNew<vtkDMMLSequenceNode> sequenceNode;
  const int numberOfDataNodes = 135;
  for (int i=0; i<numberOfDataNodes; i++)
    {
    vtkNew<vtkDMMLTransformNode> transform;
    QString indexValue = QString::number(i*1322.345);
    sequenceNode->SetDataNodeAtValue(transform.GetPointer(), indexValue.toLatin1().constData());
    }
  scene->AddNode(sequenceNode.GetPointer());

  vtkNew<vtkDMMLSequenceBrowserNode> browserNode;
  scene->AddNode(browserNode.GetPointer());
  browserNode->SetAndObserveMasterSequenceNodeID(sequenceNode->GetID());

  //
  // Create a simple gui with non-tranposed and transposed table view
  //
  QWidget parentWidget;
  parentWidget.setWindowTitle("qDMMLSequenceBrowserWidgetsTest1");
  QVBoxLayout vbox;
  parentWidget.setLayout(&vbox);

  qDMMLSequenceBrowserPlayWidget* playWidget = new qDMMLSequenceBrowserPlayWidget();
  playWidget->setParent(&parentWidget);
  playWidget->setDMMLSequenceBrowserNode(browserNode.GetPointer());
  vbox.addWidget(playWidget);

  qDMMLSequenceBrowserSeekWidget* seekWidget = new qDMMLSequenceBrowserSeekWidget();
  seekWidget->setParent(&parentWidget);
  seekWidget->setDMMLSequenceBrowserNode(browserNode.GetPointer());
  vbox.addWidget(seekWidget);

  parentWidget.show();
  parentWidget.raise();

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}