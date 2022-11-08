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

  This file was originally developed by Luis Ibanez, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QTimer>
#include <QTreeView>

// Cjyx includes
#include "vtkCjyxConfigure.h"

// qDMML includes
#include "qDMMLSceneModel.h"
#include "qDMMLSceneFactoryWidget.h"

// CTK includes
#include <ctkCoreTestingMacros.h>

// DMML includes
#include "vtkDMMLScene.h"

// VTK includes
#include "qDMMLWidget.h"

// STD includes

int qDMMLSceneModelTest1( int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  qDMMLSceneModel   sceneModel;
  CHECK_INT(sceneModel.rowCount(), 0);

  qDMMLSceneFactoryWidget sceneFactory;
  sceneFactory.generateScene();
  sceneModel.setDMMLScene(sceneFactory.dmmlScene());

  // Check if root item is shown
  CHECK_INT(sceneModel.rowCount(), 1);

  sceneFactory.deleteScene();
  CHECK_INT(sceneModel.rowCount(), 0);

  sceneFactory.generateScene();
  CHECK_INT(sceneModel.rowCount(), 0);

  sceneModel.setDMMLScene(sceneFactory.dmmlScene());
  CHECK_INT(sceneModel.rowCount(), 1);
  sceneFactory.generateNode();
  sceneFactory.generateNode();
  sceneFactory.generateNode();
  sceneFactory.generateNode();
  sceneFactory.generateNode();
  sceneFactory.generateNode();
  sceneFactory.generateNode();
  sceneFactory.generateNode();
  sceneFactory.generateNode();
  sceneFactory.generateNode();

  // We add 10 nodes but some nodes (such as folder display node) may trigger creation of a subject
  // hierarchy node.
  CHECK_BOOL(sceneFactory.dmmlScene()->GetNumberOfNodes() >= 10, true);
  CHECK_INT(sceneModel.rowCount(sceneModel.dmmlSceneIndex()), sceneFactory.dmmlScene()->GetNumberOfNodes());

  QStringList postNodes;
  postNodes << QString("temporary item");
  sceneModel.setPostItems(postNodes, sceneModel.dmmlSceneItem());
  CHECK_INT(sceneModel.rowCount(sceneModel.dmmlSceneIndex()), sceneFactory.dmmlScene()->GetNumberOfNodes() + 1);

  // test if it can be replaced
  postNodes.clear();
  postNodes << "separator" << "post item 1" << "post item 2" << "post item 3";
  sceneModel.setPostItems(postNodes, sceneModel.dmmlSceneItem());

  QStringList preNodes;
  preNodes << "pre item 1" << "pre item 2"  << "separator";
  sceneModel.setPreItems(preNodes, sceneModel.dmmlSceneItem());

  QStringList postScene;
  postScene << "post scene item";
  sceneModel.setPostItems(postScene, nullptr);

  CHECK_INT(sceneModel.rowCount(sceneModel.dmmlSceneIndex()), sceneFactory.dmmlScene()->GetNumberOfNodes() + 7);

  sceneFactory.generateNode();
  sceneFactory.generateNode();
  sceneFactory.generateNode();
  CHECK_BOOL(sceneFactory.dmmlScene()->GetNumberOfNodes() >= 13, true);

  CHECK_INT(sceneModel.rowCount(sceneModel.dmmlSceneIndex()), sceneFactory.dmmlScene()->GetNumberOfNodes() + 7);

  CHECK_INT(sceneModel.columnCount(), 1);
  CHECK_INT(sceneModel.columnCount(sceneModel.dmmlSceneIndex()), 1);

  QTreeView* view = new QTreeView(nullptr);
  view->setSelectionMode(QAbstractItemView::SingleSelection);
  view->setSelectionBehavior(QAbstractItemView::SelectRows);
  view->setDragDropMode(QAbstractItemView::InternalMove);
  view->setModel(&sceneModel);
  view->show();

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}
