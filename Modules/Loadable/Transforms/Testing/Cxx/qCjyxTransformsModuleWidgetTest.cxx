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

// CTK includes
#include "ctkTest.h"

// DMML includes
#include "qCjyxTransformsModule.h"
#include "qCjyxTransformsModuleWidget.h"
#include <vtkDMMLScene.h>
#include <vtkDMMLTransformNode.h>

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkNew.h>

// ----------------------------------------------------------------------------
class qCjyxTransformsModuleWidgetTester: public QObject
{
  Q_OBJECT

private slots:

  void testIdentity();
  void testInvert();
};

// ----------------------------------------------------------------------------
void qCjyxTransformsModuleWidgetTester::testIdentity()
{
  vtkNew<vtkDMMLScene> scene;
  vtkNew<vtkDMMLTransformNode> transformNode;
  scene->AddNode(transformNode.GetPointer());

  qCjyxTransformsModule transformsModule;
  transformsModule.setDMMLScene(scene.GetPointer());
  transformsModule.logic();
  qCjyxTransformsModuleWidget* transformsWidget =
    dynamic_cast<qCjyxTransformsModuleWidget*>(transformsModule.widgetRepresentation());

  vtkNew<vtkMatrix4x4> matrix;
  transformNode->GetMatrixTransformToParent(matrix.GetPointer());
  matrix->SetElement(0,0, 10.);
  matrix->SetElement(1,0, 2.);
  transformNode->SetMatrixTransformToParent(matrix.GetPointer());
  transformsWidget->identity();
  transformNode->GetMatrixTransformToParent(matrix.GetPointer());
  QCOMPARE(matrix->GetElement(0,0), 1.);
  QCOMPARE(matrix->GetElement(1,0), 0.);
  //transformsWidget->show();
  //qApp->exec();
}

// ----------------------------------------------------------------------------
void qCjyxTransformsModuleWidgetTester::testInvert()
{
  vtkNew<vtkDMMLScene> scene;
  vtkNew<vtkDMMLTransformNode> transformNode;
  scene->AddNode(transformNode.GetPointer());

  qCjyxTransformsModule transformsModule;
  transformsModule.setDMMLScene(scene.GetPointer());
  transformsModule.logic();
  qCjyxTransformsModuleWidget* transformsWidget =
    dynamic_cast<qCjyxTransformsModuleWidget*>(transformsModule.widgetRepresentation());

  vtkNew<vtkMatrix4x4> matrix;
  transformNode->GetMatrixTransformToParent(matrix.GetPointer());
  matrix->SetElement(0,0, 10.);
  matrix->SetElement(1,0, 2.);
  transformNode->SetMatrixTransformToParent(matrix.GetPointer());
  transformsWidget->invert();
  transformNode->GetMatrixTransformToParent(matrix.GetPointer());
  QCOMPARE(matrix->GetElement(0,0), 0.1);
  QCOMPARE(matrix->GetElement(1,0), -0.2);
  //transformsWidget->show();
  //qApp->exec();
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qCjyxTransformsModuleWidgetTest)
#include "moc_qCjyxTransformsModuleWidgetTest.cxx"
