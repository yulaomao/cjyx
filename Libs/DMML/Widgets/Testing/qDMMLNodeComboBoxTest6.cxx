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
#include <QDebug>
#include <QTimer>

// Cjyx includes
#include "vtkCjyxConfigure.h"

// qDMML includes
#include "qDMMLNodeComboBox.h"

// DMML includes
#include <vtkDMMLModelNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkNew.h>
#include "qDMMLWidget.h"


int qDMMLNodeComboBoxTest6( int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  qDMMLNodeComboBox nodeSelector;
  nodeSelector.setNodeTypes(QStringList("vtkDMMLModelNode"));
  nodeSelector.setNoneEnabled(true);
  nodeSelector.addAttribute("vtkDMMLModelNode", "foo", 1);
/*
  qDMMLNodeComboBox nodeSelector2;
  nodeSelector2.setNodeTypes(QStringList("vtkDMMLModelNode"));
  nodeSelector2.setNoneEnabled(false);
  nodeSelector2.addAttribute("vtkDMMLModelNode", "foo", 0);
*/
  vtkNew<vtkDMMLScene> scene;

  vtkNew<vtkDMMLModelNode> modelNode;
  modelNode->SetAttribute("foo", "1");
  scene->AddNode(modelNode.GetPointer());

  nodeSelector.setDMMLScene(scene.GetPointer());
  nodeSelector.setCurrentNode(modelNode.GetPointer());
  nodeSelector.show();
/*
  nodeSelector2.setDMMLScene(scene.GetPointer());
  nodeSelector2.setCurrentNode(modelNode.GetPointer());
  nodeSelector2.show();
*/
  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  app.processEvents();
  modelNode->SetAttribute("foo", "0");
  modelNode->Modified();
  qDebug() <<"modified";
  return app.exec();
}
