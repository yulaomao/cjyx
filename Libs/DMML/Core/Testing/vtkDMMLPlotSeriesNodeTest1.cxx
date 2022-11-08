/*==============================================================================

  Copyright (c) Kapteyn Astronomical Institute
  University of Groningen, Groningen, Netherlands. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Davide Punzo, Kapteyn Astronomical Institute,
  and was supported through the European Research Council grant nr. 291531.

==============================================================================*/

#include "vtkDMMLPlotSeriesNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLTableNode.h"

#include "vtkFloatArray.h"
#include "vtkPlot.h"
#include "vtkTable.h"
#include "vtkTestErrorObserver.h"

#include "vtkDMMLCoreTestingMacros.h"

int vtkDMMLPlotSeriesNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLScene> scene;
  vtkNew<vtkDMMLPlotSeriesNode> plotSeriesNode;
  EXERCISE_ALL_BASIC_DMML_METHODS(plotSeriesNode.GetPointer());
  scene->AddNode(plotSeriesNode.GetPointer());

  // Create a vtkTable
  vtkNew<vtkTable> table;

  vtkNew<vtkFloatArray> arrX;
  arrX->SetName("X Axis");
  table->AddColumn(arrX.GetPointer());

  vtkNew<vtkFloatArray> arrC;
  arrC->SetName("Cosine");
  table->AddColumn(arrC.GetPointer());

  // Fill in the table with some example values
  int numPoints = 69;
  float inc = 7.5 / (numPoints-1);
  table->SetNumberOfRows(numPoints);
  for (int i = 0; i < numPoints; ++i)
    {
    table->SetValue(i, 0, i * inc);
    table->SetValue(i, 1, cos(i * inc));
    }

  // Create a DMMLTableNode
  vtkNew<vtkDMMLTableNode> tableNode;
  scene->AddNode(tableNode.GetPointer());
  tableNode->SetAndObserveTable(table.GetPointer());

  // Set and Observe the DMMLTableNode
  plotSeriesNode->SetAndObserveTableNodeID(tableNode->GetID());

  plotSeriesNode->SetPlotType(vtkDMMLPlotSeriesNode::PlotTypeBar);

  plotSeriesNode->SetXColumnName(arrX->GetName());
  plotSeriesNode->SetYColumnName(arrC->GetName());


  // Verify that Copy method creates a true independent copy
  vtkSmartPointer<vtkDMMLPlotSeriesNode> nodeCopy = vtkSmartPointer<vtkDMMLPlotSeriesNode>::New();
  nodeCopy->CopyWithScene(plotSeriesNode.GetPointer());

  CHECK_STD_STRING(plotSeriesNode->GetName(), nodeCopy->GetName());
  CHECK_STD_STRING(plotSeriesNode->GetXColumnName(), arrX->GetName());
  CHECK_STD_STRING(plotSeriesNode->GetYColumnName(), arrC->GetName());

  std::cout << "vtkDMMLPlotSeriesNodeTest1 completed successfully" << std::endl;
  return EXIT_SUCCESS;
}
