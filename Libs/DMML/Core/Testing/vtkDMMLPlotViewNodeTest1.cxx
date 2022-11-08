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

#include "vtkDMMLScene.h"
#include "vtkDMMLPlotChartNode.h"
#include "vtkDMMLPlotViewNode.h"

#include "vtkDMMLCoreTestingMacros.h"

int vtkDMMLPlotViewNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLPlotViewNode> node1;
  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());

  // Check if modified eventes are only fired if
  // and only if PlotChart node ID is changed

  vtkNew<vtkDMMLScene> scene;
  vtkNew<vtkDMMLPlotChartNode> plotChartNode1;
  vtkNew<vtkDMMLPlotChartNode> plotChartNode2;
  scene->AddNode(plotChartNode1.GetPointer());
  scene->AddNode(plotChartNode2.GetPointer());

  vtkNew<vtkDMMLCoreTestingUtilities::vtkDMMLNodeCallback> callback;
  node1->AddObserver(vtkCommand::AnyEvent, callback.GetPointer());

  callback->ResetNumberOfEvents();
  node1->SetPlotChartNodeID(plotChartNode1->GetID());
  CHECK_INT(callback->GetNumberOfModified(),1);

  callback->ResetNumberOfEvents();
  node1->SetPlotChartNodeID(plotChartNode2->GetID());
  CHECK_INT(callback->GetNumberOfModified(),1);

  callback->ResetNumberOfEvents();
  node1->SetPlotChartNodeID(plotChartNode2->GetID());
  CHECK_INT(callback->GetNumberOfModified(),0);

  callback->ResetNumberOfEvents();
  node1->SetPlotChartNodeID(nullptr);
  CHECK_INT(callback->GetNumberOfModified(),1);

  callback->ResetNumberOfEvents();
  node1->SetPlotChartNodeID(nullptr);
  CHECK_INT(callback->GetNumberOfModified(),0);

  std::cout << "vtkDMMLPlotViewNodeTest1 completed successfully" << std::endl;
  return EXIT_SUCCESS;
}
