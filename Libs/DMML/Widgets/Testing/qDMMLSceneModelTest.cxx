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

// CTK includes
#include "ctkTest.h"

// DMML includes
#include "qDMMLSceneFactoryWidget.h"
#include "qDMMLSceneModel.h"
#include <vtkDMMLScene.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkNew.h>

// ----------------------------------------------------------------------------
class qDMMLSceneModelTester: public QObject
{
  Q_OBJECT
  void setColumns(qDMMLSceneModel& model);
private slots:
  void testDefaults();
  void testSetsAndGets();
  void testSetScene();
  void testSetColumns();
  void testSetColumns_data();
  void testSetColumnsWithScene();
  void testSetColumnsWithScene_data();
};

// ----------------------------------------------------------------------------
void qDMMLSceneModelTester::testDefaults()
{
  qDMMLSceneModel sceneModel;
  QCOMPARE(sceneModel.listenNodeModifiedEvent(), qDMMLSceneModel::OnlyVisibleNodes);
  QCOMPARE(sceneModel.lazyUpdate(), false);
  QCOMPARE(sceneModel.nameColumn(), 0);
  QCOMPARE(sceneModel.idColumn(), -1);
  QCOMPARE(sceneModel.checkableColumn(), -1);
  QCOMPARE(sceneModel.visibilityColumn(), -1);
  QCOMPARE(sceneModel.toolTipNameColumn(), -1);

  QVERIFY(sceneModel.dmmlScene() == nullptr);
  QVERIFY(sceneModel.dmmlSceneItem() == nullptr);
  QVERIFY(!sceneModel.dmmlSceneIndex().isValid());
}

// ----------------------------------------------------------------------------
void qDMMLSceneModelTester::testSetsAndGets()
{
  qDMMLSceneModel sceneModel;

  sceneModel.setListenNodeModifiedEvent(qDMMLSceneModel::NoNodes);
  QCOMPARE(sceneModel.listenNodeModifiedEvent(), qDMMLSceneModel::NoNodes);

  sceneModel.setListenNodeModifiedEvent(qDMMLSceneModel::AllNodes);
  QCOMPARE(sceneModel.listenNodeModifiedEvent(), qDMMLSceneModel::AllNodes);

  sceneModel.setLazyUpdate(true);
  QCOMPARE(sceneModel.lazyUpdate(), true);

  sceneModel.setLazyUpdate(false);
  QCOMPARE(sceneModel.lazyUpdate(), false);

  vtkNew<vtkDMMLScene> scene;
  sceneModel.setDMMLScene(scene.GetPointer());
  QCOMPARE(sceneModel.dmmlScene(), scene.GetPointer());
}


// ----------------------------------------------------------------------------
void qDMMLSceneModelTester::testSetScene()
{
  qDMMLSceneModel sceneModel;
  vtkNew<vtkDMMLScene> scene;
  sceneModel.setDMMLScene(scene.GetPointer());
  QVERIFY(sceneModel.dmmlSceneItem() != nullptr);
  QVERIFY(sceneModel.dmmlSceneIndex().isValid());
  QCOMPARE(sceneModel.dmmlSceneIndex().row(), 0);
  QCOMPARE(sceneModel.dmmlSceneIndex().column(), 0);
  QVERIFY(!sceneModel.dmmlSceneIndex().parent().isValid());
  QCOMPARE(sceneModel.columnCount(), 1);
  QCOMPARE(sceneModel.columnCount(sceneModel.dmmlSceneIndex()), 1);
  vtkNew<vtkDMMLViewNode> node;
  scene->AddNode(node.GetPointer());
  QCOMPARE(sceneModel.columnCount(), 1);
  QCOMPARE(sceneModel.columnCount(sceneModel.dmmlSceneIndex()), 1);
}

// ----------------------------------------------------------------------------
void qDMMLSceneModelTester::testSetColumns()
{
  qDMMLSceneModel sceneModel;
  this->setColumns(sceneModel);
}

// ----------------------------------------------------------------------------
void qDMMLSceneModelTester::setColumns(qDMMLSceneModel& sceneModel)
{
  QFETCH(int, nameColumn);
  QFETCH(int, idColumn);
  QFETCH(int, checkableColumn);
  QFETCH(int, visibilityColumn);
  QFETCH(int, toolTipNameColumn);

  sceneModel.setNameColumn(nameColumn);
  QCOMPARE(sceneModel.nameColumn(), nameColumn);

  sceneModel.setIDColumn(idColumn);
  QCOMPARE(sceneModel.idColumn(), idColumn);

  sceneModel.setCheckableColumn(checkableColumn);
  QCOMPARE(sceneModel.checkableColumn(), checkableColumn);

  sceneModel.setVisibilityColumn(visibilityColumn);
  QCOMPARE(sceneModel.visibilityColumn(), visibilityColumn);

  sceneModel.setToolTipNameColumn(toolTipNameColumn);
  QCOMPARE(sceneModel.toolTipNameColumn(), toolTipNameColumn);
}

// ----------------------------------------------------------------------------
void qDMMLSceneModelTester::testSetColumns_data()
{
  QTest::addColumn<int>("nameColumn");
  QTest::addColumn<int>("idColumn");
  QTest::addColumn<int>("checkableColumn");
  QTest::addColumn<int>("visibilityColumn");
  QTest::addColumn<int>("toolTipNameColumn");
  QTest::addColumn<int>("extraItemColumn");
  QTest::addColumn<int>("columnCount");


  QTest::newRow("name") << 0 << -1 << -1 << -1 << -1 << -1 << 1;
  QTest::newRow("checkable name") << 0 << -1 << 0 << -1 << -1 << -1 << 1;
  QTest::newRow("visible name") << 0 << -1 << -1 << 0 << -1 << -1 << 1;
  QTest::newRow("none") << -1 << -1 << -1 << -1 << -1 << -1 << 1;
  QTest::newRow("all 0") << 0 << 0 << 0 << 0 << 0 << 0 << 1;
  QTest::newRow("id") << -1 << 0 << -1 << -1 << -1 << -1 << 1;
  QTest::newRow("id + tooltip") << -1 << 0 << -1 << -1 << 0 << -1 << 1;
  QTest::newRow("checkable id") << -1 << 0 << 0 << -1 << -1 << -1 << 1;
  QTest::newRow("visible id") << -1 << 0 << -1 << 0 << -1 << -1 << 1;
  QTest::newRow("name + id") << 0 << 1 << -1 << -1 << -1 << -1 << 2;
  QTest::newRow("checkable name + id") << 0 << 1 << 0 << -1 << -1 << -1 << 2;
  QTest::newRow("checkable + name + id") << 1 << 2 << 0 << -1 << -1 << -1 << 3;
}

// ----------------------------------------------------------------------------
void qDMMLSceneModelTester::testSetColumnsWithScene()
{
  qDMMLSceneModel sceneModel;

  qDMMLSceneFactoryWidget sceneFactory;
  sceneFactory.generateScene();
  sceneModel.setDMMLScene(sceneFactory.dmmlScene());

  for (int i=0; i < 100; ++i)
    {
    sceneFactory.generateNode();
    }

  this->setColumns(sceneModel);
  QFETCH(int, columnCount);
  QCOMPARE(sceneModel.columnCount(), columnCount);
  QCOMPARE(sceneModel.columnCount(sceneModel.dmmlSceneIndex()), columnCount);
}

// ----------------------------------------------------------------------------
void qDMMLSceneModelTester::testSetColumnsWithScene_data()
{
  this->testSetColumns_data();
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qDMMLSceneModelTest)
#include "moc_qDMMLSceneModelTest.cxx"
