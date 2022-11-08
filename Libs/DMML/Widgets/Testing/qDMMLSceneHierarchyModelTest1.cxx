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
#include "qDMMLSceneHierarchyModel.h"
#include "qDMMLSortFilterHierarchyProxyModel.h"

// DMML includes
#include "vtkDMMLHierarchyNode.h"

// VTK includes
#include "qDMMLWidget.h"

// STD includes

int qDMMLSceneHierarchyModelTest1(int argc, char * argv [])
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  qDMMLSceneHierarchyModel model;
  qDMMLSceneFactoryWidget sceneFactory(nullptr);

  try
    {
    ctkModelTester tester(&model);
    tester.setTestDataEnabled(false);

    sceneFactory.generateScene();

    model.setDMMLScene(sceneFactory.dmmlScene());

    vtkDMMLNode* node1 = sceneFactory.generateNode("vtkDMMLViewNode");
    vtkDMMLHierarchyNode* hierarchyNode1 = vtkDMMLHierarchyNode::SafeDownCast(
      sceneFactory.generateNode("vtkDMMLHierarchyNode"));

    vtkDMMLHierarchyNode* hierarchyNode2 = vtkDMMLHierarchyNode::SafeDownCast(
      sceneFactory.generateNode("vtkDMMLHierarchyNode"));
    vtkDMMLNode* node2 = sceneFactory.generateNode("vtkDMMLViewNode");

    vtkDMMLHierarchyNode* hierarchyNode3 = vtkDMMLHierarchyNode::SafeDownCast(
      sceneFactory.generateNode("vtkDMMLHierarchyNode"));
    vtkDMMLNode* node3 = sceneFactory.generateNode("vtkDMMLViewNode");

    //vtkDMMLHierarchyNode* hierarchyNode4 =
    vtkDMMLHierarchyNode::SafeDownCast(
      sceneFactory.generateNode("vtkDMMLHierarchyNode"));

    hierarchyNode1->SetAssociatedNodeID(node1->GetID());
    hierarchyNode2->SetAssociatedNodeID(node3->GetID());
    hierarchyNode3->SetAssociatedNodeID(node2->GetID());

    //vtkDMMLHierarchyNode* hierarchyNode5 =
    vtkDMMLHierarchyNode::SafeDownCast(
      sceneFactory.generateNode("vtkDMMLHierarchyNode"));
    //hierarchyNode5->SetParentNodeID(hierarchyNode4->GetID());

    //vtkDMMLNode* node4 = sceneFactory.generateNode("vtkDMMLViewNode");
    //hierarchyNode5->SetAssociatedNodeID(node4->GetID());
    }
  catch (const char* error)
    {
    std::cerr << error << std::endl;
    return EXIT_FAILURE;
    }

  QTreeView view(nullptr);
  view.setDragDropMode(QAbstractItemView::InternalMove);
  view.setModel(&model);
  view.show();
  view.resize(500, 300);

  QTreeView view2(nullptr);
  view2.setWindowTitle("Filtered");

  qDMMLSortFilterHierarchyProxyModel sortFilterModel;
  sortFilterModel.setNodeTypes(
    QStringList() << "vtkDMMLHierarchyNode" << "vtkDMMLViewNode" );
  sortFilterModel.setSourceModel(&model);

  view2.setDragDropMode(QAbstractItemView::InternalMove);
  view2.setModel(&sortFilterModel);
  view2.show();
  view2.resize(500, 300);

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}

