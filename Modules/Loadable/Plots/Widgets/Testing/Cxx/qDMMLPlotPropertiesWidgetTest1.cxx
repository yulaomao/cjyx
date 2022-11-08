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
#include <QGridLayout>

// qDMML includes
#include "qDMMLPlotView.h"
#include "qCjyxCoreApplication.h"
#include "qDMMLPlotChartPropertiesWidget.h"
#include "qDMMLPlotSeriesPropertiesWidget.h"

// DMML includes
#include "vtkFloatArray.h"
#include "vtkDMMLPlotChartNode.h"
#include "vtkDMMLPlotSeriesNode.h"
#include "vtkDMMLPlotViewNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSelectionNode.h"
#include "vtkDMMLTableNode.h"

// DMMLLogic includes
#include <vtkDMMLColorLogic.h>

// Cjyx includes
#include "vtkCjyxConfigure.h"

// VTK includes
#include <vtkNew.h>
#include <vtkTable.h>
#include "qDMMLWidget.h"

int qDMMLPlotPropertiesWidgetTest1( int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  vtkNew<vtkDMMLScene> scene;
  vtkNew<vtkDMMLColorLogic> colorLogic;
  colorLogic->SetDMMLScene(scene.GetPointer());

  // Create a vtkTable
  vtkNew<vtkTable> table;

  vtkNew<vtkFloatArray> arrX;
  arrX->SetName("X Axis");
  table->AddColumn(arrX.GetPointer());

  vtkNew<vtkFloatArray> arrC;
  arrC->SetName("Cosine");
  table->AddColumn(arrC.GetPointer());

  vtkNew<vtkFloatArray> arrS;
  arrS->SetName("Sine");
  table->AddColumn(arrS.GetPointer());

  // Fill in the table with some example values
  int numPoints = 69;
  float inc = 7.5 / (numPoints - 1);
  table->SetNumberOfRows(numPoints);
  for (int i = 0; i < numPoints; ++i)
  {
    table->SetValue(i, 0, i * inc);
    table->SetValue(i, 1, cos(i * inc));
    table->SetValue(i, 2, sin(i * inc));
  }

  // Create a DMMLTableNode
  vtkDMMLTableNode* tableNode = vtkDMMLTableNode::SafeDownCast(scene->AddNewNodeByClass("vtkDMMLTableNode"));
  tableNode->SetAndObserveTable(table.GetPointer());

  // Create two plotSeriesNodes

  vtkDMMLPlotSeriesNode* plotSeriesNode1 = vtkDMMLPlotSeriesNode::SafeDownCast(scene->AddNewNodeByClass("vtkDMMLPlotSeriesNode", "My cosine"));
  plotSeriesNode1->SetAndObserveTableNodeID(tableNode->GetID());
  plotSeriesNode1->SetXColumnName(tableNode->GetColumnName(0));
  plotSeriesNode1->SetYColumnName(tableNode->GetColumnName(1));

  vtkDMMLPlotSeriesNode* plotSeriesNode2 = vtkDMMLPlotSeriesNode::SafeDownCast(scene->AddNewNodeByClass("vtkDMMLPlotSeriesNode", "My sine"));
  plotSeriesNode2->SetAndObserveTableNodeID(tableNode->GetID());
  plotSeriesNode2->SetXColumnName(tableNode->GetColumnName(0));
  plotSeriesNode2->SetYColumnName(tableNode->GetColumnName(2));
  plotSeriesNode2->SetUniqueColor();
  plotSeriesNode2->SetLineStyle(vtkDMMLPlotSeriesNode::LineStyleSolid);
  plotSeriesNode2->SetMarkerStyle(vtkDMMLPlotSeriesNode::MarkerStyleNone);

  // Create a PlotChart node
  vtkNew<vtkDMMLPlotChartNode> plotChartNode;
  scene->AddNode(plotChartNode.GetPointer());
  plotSeriesNode1->SetName(arrC->GetName());
  plotChartNode->AddAndObservePlotSeriesNodeID(plotSeriesNode1->GetID());
  plotSeriesNode2->SetName(arrS->GetName());
  plotChartNode->AddAndObservePlotSeriesNodeID(plotSeriesNode2->GetID());

  // Create PlotView node
  vtkNew<vtkDMMLPlotViewNode> plotViewNode;
  scene->AddNode(plotViewNode.GetPointer());
  // Set PlotChart ID in PlotView
  plotViewNode->SetPlotChartNodeID(plotChartNode->GetID());

  //
  // Create a simple GUI
  //
  QWidget parentWidget;
  parentWidget.setWindowTitle("qDMMLPlotSeriesPropertiesWidgetTest1");
  QGridLayout gridLayout;
  parentWidget.setLayout(&gridLayout);

  qDMMLPlotView* plotView = new qDMMLPlotView();
  plotView->setMinimumSize(100, 100);
  plotView->setDMMLScene(scene.GetPointer());
  plotView->setDMMLPlotViewNode(plotViewNode.GetPointer());
  gridLayout.addWidget(plotView, 0, 0, 1, 2);

  qDMMLPlotChartPropertiesWidget* chartPropertiesWidget = new qDMMLPlotChartPropertiesWidget();
  chartPropertiesWidget->setDMMLScene(scene.GetPointer());
  chartPropertiesWidget->setDMMLPlotChartNode(plotChartNode.GetPointer());
  gridLayout.addWidget(chartPropertiesWidget, 1, 0);

  qDMMLPlotSeriesPropertiesWidget* seriesPropertiesWidget = new qDMMLPlotSeriesPropertiesWidget();
  seriesPropertiesWidget->setDMMLScene(scene.GetPointer());
  seriesPropertiesWidget->setDMMLPlotSeriesNode(plotSeriesNode1);
  gridLayout.addWidget(seriesPropertiesWidget, 1, 1);

  parentWidget.show();
  parentWidget.raise();

  if (argc < 2 || QString(argv[1]) != "-I")
  {
    QTimer::singleShot(200, &app, SLOT(quit()));
  }

  return app.exec();
}
