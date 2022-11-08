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
#include "qDMMLColorTableComboBox.h"
#include "qDMMLNodeComboBox.h"
#include "qDMMLSceneModel.h"

// DMML includes
#include <vtkDMMLColorTableNode.h>
#include <vtkDMMLModelNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkNew.h>
#include "qDMMLWidget.h"

// STD includes

int qDMMLNodeComboBoxLazyUpdateTest1( int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  qDMMLNodeComboBox nodeSelector;
  qDMMLColorTableComboBox treeNodeSelector;

  nodeSelector.setNodeTypes(QStringList("vtkDMMLColorTableNode"));
  nodeSelector.setShowHidden(true);
  nodeSelector.setNoneEnabled(true);

  qobject_cast<qDMMLSceneModel*>(nodeSelector.sortFilterProxyModel()->sourceModel())
    ->setLazyUpdate(true);
  qobject_cast<qDMMLSceneModel*>(treeNodeSelector.sortFilterProxyModel()->sourceModel())
    ->setLazyUpdate(true);

  vtkNew<vtkDMMLScene> scene;
  nodeSelector.setDMMLScene(scene.GetPointer());
  treeNodeSelector.setDMMLScene(scene.GetPointer());

  scene->StartState(vtkDMMLScene::ImportState);
  vtkNew<vtkDMMLColorTableNode> node;
  scene->AddNode(node.GetPointer());

  if (nodeSelector.nodeCount() != 0 ||
      treeNodeSelector.nodeCount() != 0 )
    {
    std::cerr << "qDMMLSceneModel::LazyUpdate failed when adding a node"
              << std::endl;
    return EXIT_FAILURE;
    }

  scene->EndState(vtkDMMLScene::ImportState);

  if (nodeSelector.nodeCount() != 1 ||
      treeNodeSelector.nodeCount() != 1 )
    {
    std::cerr << "qDMMLSceneModel::LazyUpdate failed when updating the scene"
              << std::endl;
    return EXIT_FAILURE;
    }

  qDMMLNodeComboBox nodeSelector2;
  nodeSelector2.setNodeTypes(QStringList("vtkDMMLColorTableNode"));
  nodeSelector2.setShowHidden(true);
  nodeSelector2.setNoneEnabled(true);

  qDMMLColorTableComboBox treeNodeSelector2;

  qobject_cast<qDMMLSceneModel*>(nodeSelector2.sortFilterProxyModel()->sourceModel())
    ->setLazyUpdate(true);
  qobject_cast<qDMMLSceneModel*>(treeNodeSelector2.sortFilterProxyModel()->sourceModel())
    ->setLazyUpdate(true);

  nodeSelector2.setDMMLScene(scene.GetPointer());
  treeNodeSelector2.setDMMLScene(scene.GetPointer());

  if (nodeSelector2.nodeCount() != 1 ||
      treeNodeSelector2.nodeCount() != 1 )
    {
    std::cerr << "qDMMLSceneModel::LazyUpdate failed when updating the scene"
              << std::endl;
    return EXIT_FAILURE;
    }

  nodeSelector.show();
  nodeSelector2.show();

  treeNodeSelector.show();
  treeNodeSelector2.show();

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}
