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
#include "qDMMLDisplayNodeViewComboBox.h"
#include <vtkDMMLModelDisplayNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkNew.h>

// ----------------------------------------------------------------------------
class qDMMLDisplayNodeViewComboBoxTester: public QObject
{
  Q_OBJECT
  qDMMLDisplayNodeViewComboBox* ComboBox;
  vtkDMMLNode* node(int i = 0);

private slots:
  void init();
  void cleanup();

  void testDefaults();
  void testSetScene();
  void testUncheck();
  void testObserveNode();
  void testAddNode();
  void testResetScene();
};

// ----------------------------------------------------------------------------
void qDMMLDisplayNodeViewComboBoxTester::init()
{
  this->ComboBox = new qDMMLDisplayNodeViewComboBox;
  vtkDMMLScene* scene = vtkDMMLScene::New();
  this->ComboBox->setDMMLScene(scene);
  vtkNew<vtkDMMLViewNode> node;
  scene->AddNode(node.GetPointer());
  vtkNew<vtkDMMLViewNode> node2;
  scene->AddNode(node2.GetPointer());
  vtkNew<vtkDMMLSliceNode> sliceNode;
  scene->AddNode(sliceNode.GetPointer());
  vtkNew<vtkDMMLModelDisplayNode> displayNode;
  scene->AddNode(displayNode.GetPointer());
  this->ComboBox->setDMMLDisplayNode(displayNode.GetPointer());

  this->ComboBox->show();
}

// ----------------------------------------------------------------------------
void qDMMLDisplayNodeViewComboBoxTester::cleanup()
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
vtkDMMLNode* qDMMLDisplayNodeViewComboBoxTester::node(int index)
{
  if (this->ComboBox == nullptr)
    {
    return nullptr;
    }
  vtkDMMLScene* scene = this->ComboBox->dmmlScene();
  return scene->GetNthNodeByClass(index, "vtkDMMLAbstractViewNode");
}

// ----------------------------------------------------------------------------
void qDMMLDisplayNodeViewComboBoxTester::testDefaults()
{
  qDMMLDisplayNodeViewComboBox comboBox;
  QCOMPARE(comboBox.isEnabled(), false);
  QCOMPARE(comboBox.checkedViewNodes().count(), 0);
  QCOMPARE(comboBox.allChecked(), true);
  QCOMPARE(comboBox.noneChecked(), true);
  QCOMPARE(comboBox.checkState(nullptr), Qt::Unchecked);
}

// ----------------------------------------------------------------------------
void qDMMLDisplayNodeViewComboBoxTester::testSetScene()
{
  QCOMPARE(this->ComboBox->checkedViewNodes().count(), 3);
  QCOMPARE(this->ComboBox->allChecked(), true);
  QCOMPARE(this->ComboBox->noneChecked(), false);
  QCOMPARE(this->ComboBox->checkState(this->node(0)), Qt::Checked);
  QCOMPARE(this->ComboBox->checkState(this->node(1)), Qt::Checked);
  QCOMPARE(this->ComboBox->checkState(this->node(2)), Qt::Checked);
}

// ----------------------------------------------------------------------------
void qDMMLDisplayNodeViewComboBoxTester::testUncheck()
{
  this->ComboBox->uncheck(this->node());
  QCOMPARE(this->ComboBox->allChecked(), false);
  QCOMPARE(this->ComboBox->noneChecked(), false);
  QCOMPARE(this->ComboBox->checkedViewNodes().count(), 2);
  if (this->ComboBox->checkedViewNodes().count())
    {
    QCOMPARE(this->ComboBox->checkedViewNodes()[0], this->node(1));
    QCOMPARE(this->ComboBox->checkedViewNodes()[1], this->node(2));
    }
  QCOMPARE(this->ComboBox->uncheckedViewNodes().count(), 1);
  if (this->ComboBox->uncheckedViewNodes().count())
    {
    QCOMPARE(this->ComboBox->uncheckedViewNodes()[0], this->node(0));
    }
}

// ----------------------------------------------------------------------------
void qDMMLDisplayNodeViewComboBoxTester::testObserveNode()
{
  vtkDMMLDisplayNode* displayNode = this->ComboBox->dmmlDisplayNode();
  displayNode->AddViewNodeID(this->node(0)->GetID());
  QCOMPARE(this->ComboBox->allChecked(), false);
  QCOMPARE(this->ComboBox->noneChecked(), false);
  QCOMPARE(this->ComboBox->checkedViewNodes().count(), 1);
  if (this->ComboBox->checkedViewNodes().count())
    {
    QCOMPARE(this->ComboBox->checkedViewNodes()[0], this->node(0));
    }
  QCOMPARE(this->ComboBox->uncheckedViewNodes().count(), 2);
  if (this->ComboBox->uncheckedViewNodes().count())
    {
    QCOMPARE(this->ComboBox->uncheckedViewNodes()[0], this->node(1));
    QCOMPARE(this->ComboBox->uncheckedViewNodes()[1], this->node(2));
    }
}

// ----------------------------------------------------------------------------
void qDMMLDisplayNodeViewComboBoxTester::testAddNode()
{
  vtkDMMLScene* scene = this->ComboBox->dmmlScene();
  vtkNew<vtkDMMLViewNode> viewNode;
  scene->AddNode(viewNode.GetPointer());
  QCOMPARE(this->ComboBox->allChecked(), true);
  QCOMPARE(this->ComboBox->noneChecked(), false);
  QCOMPARE(this->ComboBox->checkedViewNodes().count(), 4);
  for (int i = 0; i < this->ComboBox->checkedViewNodes().count(); ++i)
    {
    QCOMPARE(this->ComboBox->checkedViewNodes()[i], this->node(i));
    }
  QCOMPARE(this->ComboBox->uncheckedViewNodes().count(), 0);
}

// ----------------------------------------------------------------------------
void qDMMLDisplayNodeViewComboBoxTester::testResetScene()
{
  vtkDMMLScene* scene = this->ComboBox->dmmlScene();
  this->ComboBox->setDMMLScene(nullptr);
  QCOMPARE(this->ComboBox->checkedViewNodes().count(), 0);
  scene->Delete();
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qDMMLDisplayNodeViewComboBoxTest)
#include "moc_qDMMLDisplayNodeViewComboBoxTest.cxx"
