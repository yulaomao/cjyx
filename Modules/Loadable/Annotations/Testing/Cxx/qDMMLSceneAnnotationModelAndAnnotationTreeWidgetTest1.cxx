
// Qt includes
#include <QApplication>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QTreeView>

// Cjyx includes
#include "vtkCjyxConfigure.h"

// CTK includes
#include <ctkModelTester.h>

// qDMML includes
#include "qDMMLSceneFactoryWidget.h"
#include "qDMMLSceneDisplayableModel.h"
#include "qDMMLUtils.h"

// VTK includes
#include "vtkDMMLCoreTestingMacros.h"
#include <vtkEventBroker.h>
#include "qDMMLWidget.h"

#include "GUI/qDMMLAnnotationTreeView.h"
#include "Logic/vtkCjyxAnnotationModuleLogic.h"
#include "DMML/vtkDMMLAnnotationRulerNode.h"


// STD includes
#include <cstdlib>
#include <iostream>

// DMML includes
#include <vtkDMMLDisplayableHierarchyNode.h>

int qDMMLSceneAnnotationModelAndAnnotationTreeViewTest1(int argc, char * argv [])
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  qDMMLSceneFactoryWidget sceneFactory(0);

  sceneFactory.generateScene();

  qDMMLAnnotationTreeView* view = new qDMMLAnnotationTreeView(0);

  //view->setSelectionBehavior(QAbstractItemView::SelectRows);

  vtkCjyxAnnotationModuleLogic* logic = vtkCjyxAnnotationModuleLogic::New();

  logic->SetDMMLScene(sceneFactory.dmmlScene());

  view->setAndObserveLogic(logic);
  view->setDMMLScene(sceneFactory.dmmlScene());
  view->hideScene();

  view->show();
  view->resize(500, 500);

  QTreeView view3;
  view3.setModel(view->sceneModel());
  view3.show();
/*
  qDMMLTreeView view2;
  view2.setSceneModelType("Displayable");
  view2.sceneModel()->setDMMLScene(sceneFactory.dmmlScene());
  view2.show();

  QTreeView view3;
  view3.setModel(view->sceneModel());
  view3.show();
*/
  double worldCoordinates1[3] = {0,0,0};
  double worldCoordinates2[3] = {50,50,50};

  double distance = sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2));

  vtkDMMLAnnotationRulerNode *rulerNode = vtkDMMLAnnotationRulerNode::New();

  rulerNode->SetPosition1(worldCoordinates1);
  rulerNode->SetPosition2(worldCoordinates2);
  rulerNode->SetDistanceMeasurement(distance);

  rulerNode->SetName(sceneFactory.dmmlScene()->GetUniqueNameByString("AnnotationRuler"));

  rulerNode->Initialize(sceneFactory.dmmlScene());

  rulerNode->Delete();

  std::cout << "Measurement in rulerNode: " << rulerNode->GetDistanceMeasurement() << std::endl;
/*
  QModelIndex index = view->d_func()->SceneModel->indexFromNode(sceneFactory.dmmlScene()->GetFirstNodeByClass("vtkDMMLAnnotationRulerNode"));

  qDMMLAbstractItemHelper* helper = view->d_func()->SceneModel->itemFromIndex(index);
  std::cout << helper->data(Qt::DisplayRole) << std::endl;
*/

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}

