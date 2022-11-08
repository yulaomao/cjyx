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
#include "qDMMLCheckableNodeComboBox.h"
#include <vtkDMMLScene.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkNew.h>

// ----------------------------------------------------------------------------
class qDMMLCheckableNodeComboBoxTester: public QObject
{
  Q_OBJECT
  qDMMLCheckableNodeComboBox* ComboBox;
  vtkDMMLNode* node(int i = 0);
private slots:
  void init();
  void cleanup();

  void testDefaults();
  void testSetScene();
  void testCheck();
  void testRename();
  void testRename_data();

};


// ----------------------------------------------------------------------------
void qDMMLCheckableNodeComboBoxTester::init()
{
  this->ComboBox = new qDMMLCheckableNodeComboBox;
  vtkDMMLScene* scene = vtkDMMLScene::New();
  this->ComboBox->setDMMLScene(scene);
  vtkNew<vtkDMMLViewNode> node;
  scene->AddNode(node.GetPointer());
  vtkNew<vtkDMMLViewNode> node2;
  scene->AddNode(node2.GetPointer());
}

// ----------------------------------------------------------------------------
void qDMMLCheckableNodeComboBoxTester::cleanup()
{
  if (this->ComboBox == nullptr)
    {
    return;
    }
  if (this->ComboBox->dmmlScene())
    {
    this->ComboBox->dmmlScene()->Delete();
    }
  delete this->ComboBox;
  this->ComboBox = nullptr;
}

// ----------------------------------------------------------------------------
vtkDMMLNode* qDMMLCheckableNodeComboBoxTester::node(int index)
{
  if (this->ComboBox == nullptr)
    {
    return nullptr;
    }
  vtkDMMLScene* scene = this->ComboBox->dmmlScene();
  return scene->GetNthNodeByClass(index, "vtkDMMLViewNode");
}

// ----------------------------------------------------------------------------
void qDMMLCheckableNodeComboBoxTester::testDefaults()
{
  qDMMLCheckableNodeComboBox comboBox;
  QVERIFY(comboBox.checkedNodes().count() == 0);
  QCOMPARE(comboBox.allChecked(), true);
  QCOMPARE(comboBox.noneChecked(), true);
  QCOMPARE(comboBox.checkState(nullptr), Qt::Unchecked);
}

// ----------------------------------------------------------------------------
void qDMMLCheckableNodeComboBoxTester::testSetScene()
{
  QCOMPARE(this->ComboBox->checkedNodes().count(), 0);
  QCOMPARE(this->ComboBox->allChecked(), false);
  QCOMPARE(this->ComboBox->noneChecked(), true);
  QCOMPARE(this->ComboBox->checkState(this->node(0)), Qt::Unchecked);
  QCOMPARE(this->ComboBox->checkState(this->node(1)), Qt::Unchecked);
}

// ----------------------------------------------------------------------------
void qDMMLCheckableNodeComboBoxTester::testCheck()
{
  this->ComboBox->check(this->node());
  QCOMPARE(this->ComboBox->allChecked(), false);
  QCOMPARE(this->ComboBox->noneChecked(), false);
  QCOMPARE(this->ComboBox->checkedNodes().count(), 1);
  QCOMPARE(this->ComboBox->checkedNodes()[0], this->node());
}

// ----------------------------------------------------------------------------
void qDMMLCheckableNodeComboBoxTester::testRename()
{
  vtkNew<vtkDMMLViewNode> node3;
  this->ComboBox->dmmlScene()->AddNode(node3.GetPointer());

  this->ComboBox->check(this->node(0));
  this->ComboBox->check(this->node(1));

  this->ComboBox->show();
  QTimer::singleShot(100, qApp, SLOT(quit()));
  qApp->exec();

  QFETCH(int, index);
  QFETCH(QString, newName);

  this->node(index)->SetName(qPrintable(newName));

  QTimer::singleShot(100, qApp, SLOT(quit()));
  qApp->exec();
}

// ----------------------------------------------------------------------------
void qDMMLCheckableNodeComboBoxTester::testRename_data()
{
  QTest::addColumn<int>("index");
  QTest::addColumn<QString>("newName");

  QTest::newRow("0 aa") << 0 << QString("aa");
  QTest::newRow("0 zz") << 0 << QString("zz");
  QTest::newRow("1 aa") << 1 << QString("aa");
  QTest::newRow("1 zz") << 1 << QString("zz");
  QTest::newRow("2 aa") << 2 << QString("aa");
  QTest::newRow("2 zz") << 2 << QString("zz");
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qDMMLCheckableNodeComboBoxTest)
#include "moc_qDMMLCheckableNodeComboBoxTest.cxx"
