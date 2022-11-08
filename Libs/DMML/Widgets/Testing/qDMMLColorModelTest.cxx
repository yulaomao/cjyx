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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// CTK includes
#include <ctkTest.h>

// qDMML includes
#include "qDMMLColorModel.h"

// DMML includes
#include <vtkDMMLColorTableNode.h>

// VTK includes
#include <vtkNew.h>

// --------------------------------------------------------------------------
class qDMMLColorModelTester: public QObject
{
  Q_OBJECT
private:

  qDMMLColorModel * ColorModel;

private slots:
  void init();
  void cleanup();

  void testSetDMMLColorNode();

  void testSetNoneEnabled();
};

// ----------------------------------------------------------------------------
void qDMMLColorModelTester::init()
{
  this->ColorModel = new qDMMLColorModel();
}

// ----------------------------------------------------------------------------
void qDMMLColorModelTester::cleanup()
{
  if (this->ColorModel == nullptr)
    {
    return;
    }
  delete this->ColorModel;
}

// ----------------------------------------------------------------------------
void qDMMLColorModelTester::testSetDMMLColorNode()
{
  QVERIFY(this->ColorModel->dmmlColorNode() == nullptr);

  vtkNew<vtkDMMLColorTableNode> colorTableNode;
  this->ColorModel->setDMMLColorNode(colorTableNode.GetPointer());
  QCOMPARE(this->ColorModel->dmmlColorNode(), colorTableNode.GetPointer());

  this->ColorModel->setDMMLColorNode(nullptr);
}

// ----------------------------------------------------------------------------
void qDMMLColorModelTester::testSetNoneEnabled()
{
  QVERIFY(!this->ColorModel->noneEnabled());

  this->ColorModel->setNoneEnabled(true);
  QVERIFY(this->ColorModel->noneEnabled());

  this->ColorModel->setNoneEnabled(false);
  QVERIFY(!this->ColorModel->noneEnabled());
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qDMMLColorModelTest)
#include "moc_qDMMLColorModelTest.cxx"
