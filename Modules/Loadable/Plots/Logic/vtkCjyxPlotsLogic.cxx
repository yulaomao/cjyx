/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

// Plots Logic includes
#include "vtkCjyxPlotsLogic.h"

// DMML includes
#include <vtkDMMLPlotChartNode.h>
#include <vtkDMMLLayoutNode.h>
#include <vtkDMMLPlotSeriesNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSelectionNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>

// STD includes

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkCjyxPlotsLogic);

//----------------------------------------------------------------------------
vtkCjyxPlotsLogic::vtkCjyxPlotsLogic() = default;

//----------------------------------------------------------------------------
vtkCjyxPlotsLogic::~vtkCjyxPlotsLogic() = default;

//----------------------------------------------------------------------------
void vtkCjyxPlotsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
int vtkCjyxPlotsLogic::GetLayoutWithPlot(int currentLayout)
{
  switch (currentLayout)
    {
    case vtkDMMLLayoutNode::CjyxLayoutFourUpPlotView:
    case vtkDMMLLayoutNode::CjyxLayoutFourUpPlotTableView:
    case vtkDMMLLayoutNode::CjyxLayoutOneUpPlotView:
    case vtkDMMLLayoutNode::CjyxLayoutConventionalPlotView:
    case vtkDMMLLayoutNode::CjyxLayoutThreeOverThreePlotView:
      // plot already shown, no need to change
      return currentLayout;
    case vtkDMMLLayoutNode::CjyxLayoutFourUpTableView:
      return vtkDMMLLayoutNode::CjyxLayoutFourUpPlotTableView;
    case vtkDMMLLayoutNode::CjyxLayoutConventionalView:
      return vtkDMMLLayoutNode::CjyxLayoutConventionalPlotView;
    default:
      return vtkDMMLLayoutNode::CjyxLayoutFourUpPlotView;
    }
}

// --------------------------------------------------------------------------
vtkDMMLPlotSeriesNode* vtkCjyxPlotsLogic::CloneSeries(vtkDMMLPlotSeriesNode* source, const char * vtkNotUsed(name))
{
  if (!source || source->GetScene() == nullptr)
    {
    vtkErrorMacro("vtkCjyxPlotsLogic::CloneSeries failed: source is nullptr or not added to a a scene");
    return nullptr;
    }

  vtkSmartPointer<vtkDMMLNode> clonedNode = vtkSmartPointer<vtkDMMLNode>::Take(
    source->GetScene()->CreateNodeByClass("vtkDMMLPlotSeriesNode"));
  vtkDMMLPlotSeriesNode *clonedSeriesNode = vtkDMMLPlotSeriesNode::SafeDownCast(clonedNode);
  clonedSeriesNode->CopyWithScene(source);
  std::string nodeName(source->GetName() ? source->GetName() : "");
  nodeName += "_Copy";

  clonedSeriesNode->SetName(source->GetScene()->GetUniqueNameByString(nodeName.c_str()));
  source->GetScene()->AddNode(clonedSeriesNode);
  return clonedSeriesNode;
}

// --------------------------------------------------------------------------
void vtkCjyxPlotsLogic::ShowChartInLayout(vtkDMMLPlotChartNode* chartNode)
{
  // Switch to a layout that contains plot
  vtkDMMLLayoutNode* layoutNode = vtkDMMLLayoutNode::SafeDownCast(this->GetDMMLScene()->GetFirstNodeByClass("vtkDMMLLayoutNode"));
  if (layoutNode)
    {
    int currentLayout = layoutNode->GetViewArrangement();
    int layoutWithPlot = vtkCjyxPlotsLogic::GetLayoutWithPlot(currentLayout);
    if (currentLayout != layoutWithPlot)
      {
      layoutNode->SetViewArrangement(layoutWithPlot);
      }
    }

  // Show plot in viewers
  vtkCjyxApplicationLogic* appLogic = this->GetApplicationLogic();
  if (appLogic)
    {
    vtkDMMLSelectionNode* selectionNode = appLogic->GetSelectionNode();
    if (selectionNode)
      {
      const char* chartNodeID = (chartNode ? chartNode->GetID() : nullptr);
      selectionNode->SetActivePlotChartID(chartNodeID);
      }
    appLogic->PropagatePlotChartSelection();
    }
}

// --------------------------------------------------------------------------
vtkDMMLPlotChartNode* vtkCjyxPlotsLogic::GetFirstPlotChartForSeries(vtkDMMLPlotSeriesNode* seriesNode)
{
  if (seriesNode == nullptr)
    {
    return nullptr;
    }
  std::vector<vtkDMMLNode*> chartNodes;
  unsigned int numberOfNodes = this->GetDMMLScene()->GetNodesByClass("vtkDMMLPlotChartNode", chartNodes);
  for (unsigned int chartNodeIndex=0; chartNodeIndex<numberOfNodes; chartNodeIndex++)
    {
    vtkDMMLPlotChartNode* chartNode = vtkDMMLPlotChartNode::SafeDownCast(chartNodes[chartNodeIndex]);
    if (!chartNode)
      {
      continue;
      }
    if (chartNode->HasPlotSeriesNodeID(seriesNode->GetID()))
      {
      return chartNode;
      }
    }
  // not found
  return nullptr;
}
