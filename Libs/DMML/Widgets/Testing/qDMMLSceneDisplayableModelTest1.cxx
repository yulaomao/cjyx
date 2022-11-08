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
#include <ctkModelTester.h>

// qDMML includes
#include "qDMMLSceneFactoryWidget.h"
#include "qDMMLSceneDisplayableModel.h"

// VTK includes
#include "qDMMLWidget.h"

// STD includes

// DMML includes
#include <vtkDMMLDisplayableHierarchyNode.h>

int qDMMLSceneDisplayableModelTest1(int argc, char * argv [])
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  qDMMLSceneDisplayableModel model;
  qDMMLSceneFactoryWidget sceneFactory(nullptr);

  try
    {
    ctkModelTester tester(&model);
    tester.setTestDataEnabled(false);

    sceneFactory.generateScene();

    model.setDMMLScene(sceneFactory.dmmlScene());
    model.setDMMLScene(sceneFactory.dmmlScene());

    sceneFactory.generateNode();
    sceneFactory.deleteNode();

    sceneFactory.generateNode();
    sceneFactory.deleteNode();

    sceneFactory.generateNode();
    sceneFactory.generateNode();

    sceneFactory.deleteNode();
    sceneFactory.deleteNode();

    sceneFactory.generateNode();
    sceneFactory.deleteNode();

    sceneFactory.generateNode();
    sceneFactory.deleteNode();

    sceneFactory.generateNode();
    sceneFactory.generateNode();

    sceneFactory.deleteNode();
    sceneFactory.deleteNode();

    for( int i = 0; i < 100; ++i)
      {
      sceneFactory.deleteNode();
      }
    for( int i = 0; i < 100; ++i)
      {
      sceneFactory.generateNode();
      }
    for( int i = 0; i < 99; ++i)
      {
      sceneFactory.deleteNode();
      }
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
    sceneFactory.generateNode();
    sceneFactory.generateNode();
    sceneFactory.generateNode();

    vtkDMMLNode* node1 = sceneFactory.generateNode("vtkDMMLModelNode");
    vtkDMMLNode* node2 = sceneFactory.generateNode("vtkDMMLScalarVolumeNode");
    vtkDMMLNode* hnode1 = sceneFactory.generateNode("vtkDMMLModelHierarchyNode");
    vtkDMMLDisplayableHierarchyNode* h1 = vtkDMMLDisplayableHierarchyNode::SafeDownCast(hnode1);
    vtkDMMLNode* hnode2 = sceneFactory.generateNode("vtkDMMLDisplayableHierarchyNode");
    vtkDMMLDisplayableHierarchyNode* h2 = vtkDMMLDisplayableHierarchyNode::SafeDownCast(hnode2);
    if (node1 && node1->GetID() && h1)
      {
      std::cout << "Adding node1 to a hierarchy node" << std::endl;
      h1->SetDisplayableNodeID(node1->GetID());
      if (node2 && node2->GetID() && h2)
        {
        std::cout << "Adding node2 to a hierarchy node" << std::endl;
        h2->SetDisplayableNodeID(node2->GetID());
        }
      if (h2 && h2->GetID())
        {
        h1->SetParentNodeID(h2->GetID());
        }
      }
    std::cout << "start reparenting node" << std::endl;
    model.qDMMLSceneDisplayableModel::reparent(node1, node2);
    std::cout << "end reparenting node" << std::endl;
    }
  catch (const char* error)
    {
    std::cerr << error << std::endl;
    return EXIT_FAILURE;
    }
  /*
  QStandardItemModel m;
  m.setColumnCount(2);
  QStandardItem* item = new QStandardItem("titi");
  m.insertRow(0, item);
  QList<QStandardItem*> items;
  items << new QStandardItem("toto");
  items << new QStandardItem("tata");
  items[0]->setBackground(QLinearGradient());
  item->insertRow(0,items);
  */
  QTreeView* view = new QTreeView(nullptr);
  //view->setSelectionBehavior(QAbstractItemView::SelectRows);
  view->setDragDropMode(QAbstractItemView::InternalMove);
  view->setModel(&model);
  //view->setModel(&m);
  view->show();
  view->resize(500, 800);

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}

