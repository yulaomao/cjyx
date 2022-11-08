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
#include "qDMMLSceneCategoryModel.h"

// DMML includes
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkNew.h>
#include "qDMMLWidget.h"

// STD includes

int qDMMLSceneCategoryModelTest1(int argc, char * argv [])
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  qDMMLSceneCategoryModel model;

  vtkNew<vtkDMMLScene> scene;
  qDMMLNodeFactory nodeFactory(nullptr);
  nodeFactory.setDMMLScene(scene.GetPointer());
  nodeFactory.createNode("vtkDMMLROINode");
  nodeFactory.addAttribute("Category", "First Category");
  nodeFactory.createNode("vtkDMMLCameraNode");
  nodeFactory.createNode("vtkDMMLViewNode");
  nodeFactory.createNode("vtkDMMLLinearTransformNode");
  nodeFactory.removeAttribute("Category");
  nodeFactory.createNode("vtkDMMLTableNode");
  model.setDMMLScene(scene.GetPointer());
  nodeFactory.createNode("vtkDMMLScalarVolumeNode");
  nodeFactory.addAttribute("Category", "Second Category");
  nodeFactory.createNode("vtkDMMLSliceNode");
  nodeFactory.createNode("vtkDMMLSliceNode");
  nodeFactory.addAttribute("Category", "Third Category");
  nodeFactory.createNode("vtkDMMLViewNode");
  nodeFactory.createNode("vtkDMMLViewNode");

  QStringList scenePreItems =
    QStringList() << "pre 1" << "pre 2" << "separator";
  model.setPreItems(scenePreItems, nullptr);
  model.setPreItems(scenePreItems, model.dmmlSceneItem());

  if (model.itemFromCategory("Second Category") == nullptr ||
      model.itemFromCategory("Second Category") == model.dmmlSceneItem())
    {
    std::cerr << "Wrong category. Item: "
              << model.itemFromCategory("Second Category")
              << " scene item: " << model.dmmlSceneItem() << std::endl;
    return EXIT_FAILURE;
    }
  model.setPreItems(scenePreItems, model.itemFromCategory("Second Category"));

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

