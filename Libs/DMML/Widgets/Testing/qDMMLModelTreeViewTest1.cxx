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
#include <QApplication>
#include <QTimer>

// Cjyx includes
#include "vtkCjyxConfigure.h"

// DMMLWidgets includes
#include <qDMMLTreeView.h>

// DMML includes
#include <vtkDMMLScene.h>
#include <vtkDMMLModelNode.h>
#include <vtkDMMLModelDisplayNode.h>

// VTK includes
#include <vtkNew.h>
#include "qDMMLWidget.h"

// STD includes

int qDMMLModelTreeViewTest1( int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  vtkNew<vtkDMMLModelNode> modelNode;
  vtkNew<vtkDMMLModelDisplayNode> displayModelNode;

  vtkNew<vtkDMMLScene> scene;
  scene->AddNode(modelNode.GetPointer());
  scene->AddNode(displayModelNode.GetPointer());

  modelNode->SetAndObserveDisplayNodeID(displayModelNode->GetID());

  qDMMLTreeView modelView;
  modelView.setSceneModelType("ModelHierarchy");
  modelView.setDMMLScene(scene.GetPointer());

  modelView.show();
  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(100, &app, SLOT(quit()));
    }

  return app.exec();
}
