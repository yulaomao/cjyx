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
#include "qDMMLNodeComboBox.h"
#include "qDMMLSliceControllerWidget.h"
#include <vtkDMMLColorLogic.h>
#include <vtkDMMLLabelMapVolumeNode.h>
#include <vtkDMMLScalarVolumeNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceCompositeNode.h>
#include <vtkDMMLSliceLogic.h>
#include <vtkDMMLSliceNode.h>

// VTK includes
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkStringArray.h>

// ----------------------------------------------------------------------------
class qDMMLSliceControllerWidgetTester: public QObject
{
  Q_OBJECT
  vtkDMMLScene* DMMLScene;
  vtkDMMLSliceNode* DMMLSliceNode;

private slots:
  /// Run before each test
  void init();

  /// Run after each test
  void cleanup();

  void testDefaults();
  void testSetDMMLSliceNode();

  void testSetBackgroundVolume();
  void testSetBackgroundVolume_data();

  void testSetForegroundVolume();
  void testSetForegroundVolume_data();

  void testSetLabelVolume();
  void testSetLabelVolume_data();

  void testSetLabelVolumeWithNoLinkedControl();

  void testUpdateSliceOrientationSelector();
};

// ----------------------------------------------------------------------------
void qDMMLSliceControllerWidgetTester::init()
{
  this->DMMLScene = vtkDMMLScene::New();

  // Add default color nodes
  vtkNew<vtkDMMLColorLogic> colorLogic;
  colorLogic->SetDMMLScene(this->DMMLScene);
  // need to set it back to nullptr, otherwise the logic removes the nodes that it added when it is destructed
  colorLogic->SetDMMLScene(nullptr);

  vtkNew<vtkDMMLSliceNode> sliceNode;
  sliceNode->SetLayoutName("Red");
  vtkNew<vtkMatrix3x3> axialSliceToRAS;
  vtkDMMLSliceNode::GetAxialSliceToRASMatrix(axialSliceToRAS.GetPointer());

  sliceNode->AddSliceOrientationPreset("Axial", axialSliceToRAS.GetPointer());
  sliceNode->SetOrientation("Axial");
  this->DMMLScene->AddNode(sliceNode.GetPointer());

  vtkNew<vtkDMMLScalarVolumeNode> volumeNode1;
  volumeNode1->SetName("Volume 1");
  this->DMMLScene->AddNode(volumeNode1.GetPointer());

  vtkNew<vtkDMMLScalarVolumeNode> volumeNode2;
  volumeNode2->SetName("Volume 2");
  this->DMMLScene->AddNode(volumeNode2.GetPointer());

  vtkNew<vtkDMMLScalarVolumeNode> volumeNode3;
  volumeNode3->SetName("Volume 3");
  this->DMMLScene->AddNode(volumeNode3.GetPointer());

  vtkNew<vtkDMMLLabelMapVolumeNode> labelmapNode1;
  labelmapNode1->SetName("Labelmap 1");
  this->DMMLScene->AddNode(labelmapNode1.GetPointer());

  this->DMMLSliceNode = sliceNode.GetPointer();
}

// ----------------------------------------------------------------------------
void qDMMLSliceControllerWidgetTester::cleanup()
{
  this->DMMLScene->Delete();
}

// ----------------------------------------------------------------------------
void qDMMLSliceControllerWidgetTester::testDefaults()
{
  qDMMLSliceControllerWidget sliceControllerWidget;
  void* nullPtr = nullptr;
  QCOMPARE(reinterpret_cast<void*>(sliceControllerWidget.dmmlScene()), nullPtr);
  QCOMPARE(reinterpret_cast<void*>(sliceControllerWidget.dmmlSliceNode()), nullPtr);

  QCOMPARE(sliceControllerWidget.sliceViewName(), QString());
  QCOMPARE(sliceControllerWidget.sliceViewLabel(), QString());
  QCOMPARE(sliceControllerWidget.sliceViewColor(), QColor());

  QVERIFY(sliceControllerWidget.sliceLogic() != nullptr);
  QCOMPARE(sliceControllerWidget.imageDataConnection(), nullPtr);
  QCOMPARE(sliceControllerWidget.dmmlSliceCompositeNode(), nullPtr);

  QCOMPARE(sliceControllerWidget.isLinked(), false);
  QCOMPARE(sliceControllerWidget.isCompareView(), false);
  QCOMPARE(sliceControllerWidget.sliceOrientation(), QString("Axial"));
}

// ----------------------------------------------------------------------------
void qDMMLSliceControllerWidgetTester::testSetDMMLSliceNode()
{
  qDMMLSliceControllerWidget sliceControllerWidget;
  void* nullPtr = nullptr;

  sliceControllerWidget.setSliceViewLabel("R");
  QCOMPARE(sliceControllerWidget.sliceViewLabel(), QString(""));

  sliceControllerWidget.setSliceViewName("Red");
  QCOMPARE(sliceControllerWidget.sliceViewName(), QString(""));

  sliceControllerWidget.setSliceViewColor(Qt::red);
  QCOMPARE(sliceControllerWidget.sliceViewColor(), QColor());

  sliceControllerWidget.setDMMLScene(this->DMMLScene);
  sliceControllerWidget.setDMMLSliceNode(this->DMMLSliceNode);

  QCOMPARE(sliceControllerWidget.dmmlScene(), this->DMMLScene);
  QCOMPARE(sliceControllerWidget.dmmlSliceNode(), this->DMMLSliceNode);
  QVERIFY(sliceControllerWidget.sliceLogic() != nullptr);
  QCOMPARE(sliceControllerWidget.imageDataConnection(), nullPtr);

  QCOMPARE(sliceControllerWidget.dmmlSliceCompositeNode()->GetBackgroundVolumeID(), nullPtr);
  QCOMPARE(sliceControllerWidget.dmmlSliceCompositeNode()->GetForegroundVolumeID(), nullPtr);
  QCOMPARE(sliceControllerWidget.dmmlSliceCompositeNode()->GetLabelVolumeID(), nullPtr);

  sliceControllerWidget.setSliceViewName("Red");
  QCOMPARE(sliceControllerWidget.sliceViewName(), QString("Red"));

  sliceControllerWidget.setSliceViewLabel("R");
  QCOMPARE(sliceControllerWidget.sliceViewLabel(), QString("R"));

  sliceControllerWidget.setSliceViewColor(Qt::red);
  QCOMPARE(sliceControllerWidget.sliceViewColor(), QColor(Qt::red));

  QCOMPARE(sliceControllerWidget.isLinked(), false);
  QCOMPARE(sliceControllerWidget.isCompareView(), false);
  QCOMPARE(sliceControllerWidget.sliceOrientation(), QString("Axial"));
}

// ----------------------------------------------------------------------------
void qDMMLSliceControllerWidgetTester::testSetBackgroundVolume()
{
  qDMMLSliceControllerWidget sliceControllerWidget;
  sliceControllerWidget.setDMMLScene(this->DMMLScene);
  sliceControllerWidget.setDMMLSliceNode(this->DMMLSliceNode);

  QFETCH(QString, volumeNodeID);
  vtkDMMLNode* volumeNode = this->DMMLScene->GetNodeByID(volumeNodeID.toUtf8());
  sliceControllerWidget.dmmlSliceCompositeNode()->SetBackgroundVolumeID(volumeNode ? volumeNode->GetID() : nullptr);

  QFETCH(QString, expectedVolumeNodeID);
  qDMMLNodeComboBox* comboBox =
    qobject_cast<qDMMLNodeComboBox*>(sliceControllerWidget.findChild<qDMMLNodeComboBox*>("BackgroundComboBox"));
  QCOMPARE(comboBox->currentNodeID(), expectedVolumeNodeID);

  //sliceControllerWidget.show();
  //qApp->exec();
}

// ----------------------------------------------------------------------------
void qDMMLSliceControllerWidgetTester::testSetBackgroundVolume_data()
{
  QTest::addColumn<QString>("volumeNodeID");
  QTest::addColumn<QString>("expectedVolumeNodeID");

  QTest::newRow("no node") << QString() << QString();
  QTest::newRow("volume1") << "vtkDMMLScalarVolumeNode1" << "vtkDMMLScalarVolumeNode1";
  QTest::newRow("volume2") << "vtkDMMLScalarVolumeNode2" << "vtkDMMLScalarVolumeNode2";
  QTest::newRow("volume3") << "vtkDMMLScalarVolumeNode3" << "vtkDMMLScalarVolumeNode3";
  QTest::newRow("labelmap1") << "vtkDMMLLabelMapVolumeNode1" << "vtkDMMLLabelMapVolumeNode1";
}

// ----------------------------------------------------------------------------
void qDMMLSliceControllerWidgetTester::testSetForegroundVolume()
{
  qDMMLSliceControllerWidget sliceControllerWidget;
  sliceControllerWidget.setDMMLScene(this->DMMLScene);
  sliceControllerWidget.setDMMLSliceNode(this->DMMLSliceNode);

  QFETCH(QString, volumeNodeID);
  vtkDMMLNode* volumeNode = this->DMMLScene->GetNodeByID(volumeNodeID.toUtf8());
  sliceControllerWidget.dmmlSliceCompositeNode()->SetForegroundVolumeID(volumeNode ? volumeNode->GetID() : nullptr);

  QFETCH(QString, expectedVolumeNodeID);
  qDMMLNodeComboBox* comboBox =
    qobject_cast<qDMMLNodeComboBox*>(sliceControllerWidget.findChild<qDMMLNodeComboBox*>("ForegroundComboBox"));
  QCOMPARE(comboBox->currentNodeID(), expectedVolumeNodeID);

  //sliceControllerWidget.show();
  //qApp->exec();
}

// ----------------------------------------------------------------------------
void qDMMLSliceControllerWidgetTester::testSetForegroundVolume_data()
{
  QTest::addColumn<QString>("volumeNodeID");
  QTest::addColumn<QString>("expectedVolumeNodeID");

  QTest::newRow("no node") << QString() << QString();
  QTest::newRow("volume1") << "vtkDMMLScalarVolumeNode1" << "vtkDMMLScalarVolumeNode1";
  QTest::newRow("volume2") << "vtkDMMLScalarVolumeNode2" << "vtkDMMLScalarVolumeNode2";
  QTest::newRow("volume3") << "vtkDMMLScalarVolumeNode3" << "vtkDMMLScalarVolumeNode3";
  QTest::newRow("labelmap1") << "vtkDMMLLabelMapVolumeNode1" << "vtkDMMLLabelMapVolumeNode1";
}

// ----------------------------------------------------------------------------
void qDMMLSliceControllerWidgetTester::testSetLabelVolume()
{
  qDMMLSliceControllerWidget sliceControllerWidget;
  sliceControllerWidget.setDMMLScene(this->DMMLScene);
  sliceControllerWidget.setDMMLSliceNode(this->DMMLSliceNode);

  QFETCH(QString, volumeNodeID);
  vtkDMMLNode* volumeNode = this->DMMLScene->GetNodeByID(volumeNodeID.toUtf8());
  sliceControllerWidget.dmmlSliceCompositeNode()->SetLabelVolumeID(volumeNode ? volumeNode->GetID() : nullptr);

  QFETCH(QString, expectedVolumeNodeID);
  qDMMLNodeComboBox* comboBox =
    qobject_cast<qDMMLNodeComboBox*>(sliceControllerWidget.findChild<qDMMLNodeComboBox*>("LabelMapComboBox"));
  QCOMPARE(comboBox->currentNodeID(), expectedVolumeNodeID);

  //sliceControllerWidget.show();
  //qApp->exec();
}

// ----------------------------------------------------------------------------
void qDMMLSliceControllerWidgetTester::testSetLabelVolume_data()
{
  QTest::addColumn<QString>("volumeNodeID");
  QTest::addColumn<QString>("expectedVolumeNodeID");

  QTest::newRow("no node") << QString() << QString();
  QTest::newRow("volume1") << "vtkDMMLScalarVolumeNode1" << QString();
  QTest::newRow("volume2") << "vtkDMMLScalarVolumeNode2" << QString();
  QTest::newRow("volume3") << "vtkDMMLScalarVolumeNode3" << QString();
  QTest::newRow("labelmap1") << "vtkDMMLLabelMapVolumeNode1" << "vtkDMMLLabelMapVolumeNode1";
}

// ----------------------------------------------------------------------------
void qDMMLSliceControllerWidgetTester::testSetLabelVolumeWithNoLinkedControl()
{
  qDMMLSliceControllerWidget sliceControllerWidget;
  sliceControllerWidget.setDMMLScene(this->DMMLScene);
  sliceControllerWidget.setDMMLSliceNode(this->DMMLSliceNode);

  vtkDMMLScalarVolumeNode* scalarVolumeNode =
    vtkDMMLScalarVolumeNode::SafeDownCast(this->DMMLScene->GetNodeByID("vtkDMMLLabelMapVolumeNode1"));
  if (scalarVolumeNode)
    {
    sliceControllerWidget.dmmlSliceCompositeNode()->SetLabelVolumeID(scalarVolumeNode->GetID());
    }

  vtkNew<vtkDMMLLabelMapVolumeNode> labelmapNode2;
  labelmapNode2->SetName("Labelmap 2");
  this->DMMLScene->AddNode(labelmapNode2.GetPointer());

  vtkDMMLSliceCompositeNode* sliceCompositeNode =
    sliceControllerWidget.dmmlSliceCompositeNode();

  sliceCompositeNode->SetLinkedControl(0);
  sliceControllerWidget.sliceLogic()->StartSliceCompositeNodeInteraction(
    vtkDMMLSliceCompositeNode::LabelVolumeFlag);
  sliceCompositeNode->SetLabelVolumeID(labelmapNode2->GetID());
  sliceControllerWidget.sliceLogic()->EndSliceCompositeNodeInteraction();
  sliceCompositeNode->SetLinkedControl(1);
  sliceCompositeNode->Modified();

  qDMMLNodeComboBox* comboBox =
    qobject_cast<qDMMLNodeComboBox*>(sliceControllerWidget.findChild<qDMMLNodeComboBox*>("LabelMapComboBox"));
  QCOMPARE(comboBox->currentNodeID(), QString(labelmapNode2->GetID()));

  //sliceControllerWidget.show();
  //qApp->exec();
}

// ----------------------------------------------------------------------------
void qDMMLSliceControllerWidgetTester::testUpdateSliceOrientationSelector()
{
  qDMMLSliceControllerWidget sliceControllerWidget;
  sliceControllerWidget.setSliceViewLabel("R");
  sliceControllerWidget.setSliceViewColor(Qt::red);
  sliceControllerWidget.setDMMLScene(this->DMMLScene);
  sliceControllerWidget.setDMMLSliceNode(this->DMMLSliceNode);
  QCOMPARE(sliceControllerWidget.sliceOrientation(), QString("Axial"));

  // Update the sliceToRAS matrix
  vtkMatrix4x4* sliceToRAS =
      sliceControllerWidget.dmmlSliceNode()->GetSliceToRAS();
  sliceToRAS->SetElement(0, 0, 1.2);
  sliceControllerWidget.dmmlSliceNode()->UpdateMatrices();

  // Make sure the presets have not been updated
  vtkNew<vtkStringArray> orientationNames;
  sliceControllerWidget.dmmlSliceNode()->GetSliceOrientationPresetNames(orientationNames.GetPointer());
  QCOMPARE(orientationNames->GetNumberOfValues(), static_cast<vtkIdType>(1));
  QCOMPARE(orientationNames->GetValue(0).c_str(), "Axial");

  // Check that current orientation is updated
  QCOMPARE(sliceControllerWidget.dmmlSliceNode()->GetOrientation(), std::string("Reformat"));
  QCOMPARE(sliceControllerWidget.sliceOrientation(), QString("Reformat"));

  // Check that "Reformat" is the last item in the selector
  QComboBox* orientationSelector =
      sliceControllerWidget.findChild<QComboBox*>("SliceOrientationSelector");
  QVERIFY(orientationSelector != nullptr);
  QStringList items;
  for(int idx = 0; idx < orientationSelector->count(); ++idx)
    {
    items << orientationSelector->itemText(idx);
    }
  QCOMPARE(items, QStringList() << "Axial" << "Reformat");

  // Set orientation back to "Axial"
  sliceControllerWidget.dmmlSliceNode()->SetOrientation("Axial");
  QCOMPARE(sliceControllerWidget.sliceOrientation(), QString("Axial"));

}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qDMMLSliceControllerWidgetTest)
#include "moc_qDMMLSliceControllerWidgetTest.cxx"
