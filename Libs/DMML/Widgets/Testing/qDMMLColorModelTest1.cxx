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
#include <QTreeView>

// Cjyx includes
#include "vtkCjyxConfigure.h"

// CTK includes

// qDMML includes
#include "qDMMLNodeFactory.h"
#include "qDMMLColorModel.h"

// DMML includes
#include <vtkDMMLColorTableNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkNew.h>
#include "qDMMLWidget.h"

int qDMMLColorModelTest1(int argc, char * argv [])
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  qDMMLColorModel model;

  vtkNew<vtkDMMLScene> scene;
  qDMMLNodeFactory nodeFactory(nullptr);
  nodeFactory.setDMMLScene(scene.GetPointer());
  vtkDMMLNode* node = nodeFactory.createNode("vtkDMMLColorTableNode");
  vtkDMMLColorTableNode* colorNode = vtkDMMLColorTableNode::SafeDownCast(node);
  if (colorNode)
    {
    colorNode->SetTypeToWarmShade1();
    }
  model.setDMMLColorNode(colorNode);
  colorNode->SetTypeToCool1();

  QTreeView* view = new QTreeView(nullptr);
  view->setModel(&model);
  view->show();
  view->resize(500, 800);

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}

