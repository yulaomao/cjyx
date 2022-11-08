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

#include <sstream>
#include <map>
#include <string>

// VTK includes
#include <vtkCollection.h>
#include <vtkCommand.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPlot.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTable.h>

// DMML includes
#include "vtkDMMLPlotChartNode.h"
#include "vtkDMMLPlotSeriesNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLTableNode.h"

const char* vtkDMMLPlotChartNode::PlotSeriesNodeReferenceRole = "plotSeries";

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLPlotChartNode);

//----------------------------------------------------------------------------
vtkDMMLPlotChartNode::vtkDMMLPlotChartNode()
{
  this->HideFromEditors = 0;

  this->SetFontType("Arial");

  this->XAxisRange[0] = 0.0;
  this->XAxisRange[1] = 1.0;
  this->YAxisRange[0] = 0.0;
  this->YAxisRange[1] = 1.0;

  vtkNew<vtkIntArray>  events;
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkDMMLPlotChartNode::PlotModifiedEvent);
  events->InsertNextValue(vtkDMMLPlotSeriesNode::TableModifiedEvent);
  this->AddNodeReferenceRole(this->GetPlotSeriesNodeReferenceRole(), nullptr, events.GetPointer());
}


//----------------------------------------------------------------------------
vtkDMMLPlotChartNode::~vtkDMMLPlotChartNode()
{
  if (this->Title)
    {
    delete [] this->Title;
    this->Title = nullptr;
    }
  if (this->XAxisTitle)
    {
    delete [] this->XAxisTitle;
    this->XAxisTitle = nullptr;
    }
  if (this->YAxisTitle)
    {
    delete [] this->YAxisTitle;
    this->YAxisTitle = nullptr;
    }
  if (this->FontType)
    {
    delete [] this->FontType;
    this->FontType = nullptr;
    }
}

//----------------------------------------------------------------------------
const char* vtkDMMLPlotChartNode::GetPlotSeriesNodeReferenceRole()
{
  return vtkDMMLPlotChartNode::PlotSeriesNodeReferenceRole;
}

//----------------------------------------------------------------------------
void vtkDMMLPlotChartNode::OnNodeReferenceAdded(vtkDMMLNodeReference *reference)
{
  this->Superclass::OnNodeReferenceAdded(reference);
  if (std::string(reference->GetReferenceRole()) == this->PlotSeriesNodeReferenceRole)
    {
    this->InvokeEvent(vtkDMMLPlotChartNode::PlotModifiedEvent, reference->GetReferencedNode());
    }
}

//----------------------------------------------------------------------------
void vtkDMMLPlotChartNode::OnNodeReferenceModified(vtkDMMLNodeReference *reference)
{
  this->Superclass::OnNodeReferenceModified(reference);
  if (std::string(reference->GetReferenceRole()) == this->PlotSeriesNodeReferenceRole)
    {
    this->InvokeEvent(vtkDMMLPlotChartNode::PlotModifiedEvent, reference->GetReferencedNode());
    }
}

//----------------------------------------------------------------------------
void vtkDMMLPlotChartNode::OnNodeReferenceRemoved(vtkDMMLNodeReference *reference)
{
  this->Superclass::OnNodeReferenceRemoved(reference);
  if (std::string(reference->GetReferenceRole()) == this->PlotSeriesNodeReferenceRole)
    {
    this->InvokeEvent(vtkDMMLPlotChartNode::PlotModifiedEvent, reference->GetReferencedNode());
    }
}

//----------------------------------------------------------------------------
void vtkDMMLPlotChartNode::WriteXML(ostream& of, int nIndent)
{
  // Start by having the superclass write its information
  Superclass::WriteXML(of, nIndent);

  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLStringMacro(title, Title);
  vtkDMMLWriteXMLIntMacro(titleFontSize, TitleFontSize);
  vtkDMMLWriteXMLBooleanMacro(titleVisibility, TitleVisibility);
  vtkDMMLWriteXMLBooleanMacro(gridVisibility, GridVisibility);
  vtkDMMLWriteXMLBooleanMacro(legendVisibility, LegendVisibility);
  vtkDMMLWriteXMLIntMacro(legendFontSize, LegendFontSize);
  vtkDMMLWriteXMLBooleanMacro(xAxisRangeAuto, XAxisRangeAuto);
  vtkDMMLWriteXMLBooleanMacro(yAxisRangeAuto, YAxisRangeAuto);
  vtkDMMLWriteXMLVectorMacro(xAxisRange, XAxisRange, double, 2);
  vtkDMMLWriteXMLVectorMacro(yAxisRange, YAxisRange, double, 2);
  vtkDMMLWriteXMLBooleanMacro(xAxisLogScale, XAxisLogScale);
  vtkDMMLWriteXMLBooleanMacro(yAxisLogScale, YAxisLogScale);
  vtkDMMLWriteXMLStringMacro(xAxisTitle, XAxisTitle);
  vtkDMMLWriteXMLBooleanMacro(xAxisTitleVisibility, XAxisTitleVisibility);
  vtkDMMLWriteXMLStringMacro(yAxisTitle, YAxisTitle);
  vtkDMMLWriteXMLBooleanMacro(yAxisTitleVisibility, YAxisTitleVisibility);
  vtkDMMLWriteXMLIntMacro(axisTitleFontSize, AxisTitleFontSize);
  vtkDMMLWriteXMLIntMacro(axisLabelFontSize, AxisLabelFontSize);
  vtkDMMLWriteXMLStringMacro(fontType, FontType);
  vtkDMMLWriteXMLBooleanMacro(enablePointMoveAlongX, EnablePointMoveAlongX);
  vtkDMMLWriteXMLBooleanMacro(enablePointMoveAlongY, EnablePointMoveAlongY);
  vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLPlotChartNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  vtkDMMLNode::ReadXMLAttributes(atts);

  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLStringMacro(title, Title);
  vtkDMMLReadXMLIntMacro(titleFontSize, TitleFontSize);
  vtkDMMLReadXMLBooleanMacro(titleVisibility, TitleVisibility);
  vtkDMMLReadXMLBooleanMacro(gridVisibility, GridVisibility);
  vtkDMMLReadXMLBooleanMacro(legendVisibility, LegendVisibility);
  vtkDMMLReadXMLIntMacro(legendFontSize, LegendFontSize);
  vtkDMMLReadXMLBooleanMacro(xAxisRangeAuto, XAxisRangeAuto);
  vtkDMMLReadXMLBooleanMacro(yAxisRangeAuto, YAxisRangeAuto);
  vtkDMMLReadXMLVectorMacro(xAxisRange, XAxisRange, double, 2);
  vtkDMMLReadXMLVectorMacro(yAxisRange, YAxisRange, double, 2);
  vtkDMMLReadXMLBooleanMacro(xAxisLogScale, XAxisLogScale);
  vtkDMMLReadXMLBooleanMacro(yAxisLogScale, YAxisLogScale);
  vtkDMMLReadXMLStringMacro(xAxisTitle, XAxisTitle);
  vtkDMMLReadXMLBooleanMacro(xAxisTitleVisibility, XAxisTitleVisibility);
  vtkDMMLReadXMLStringMacro(yAxisTitle, YAxisTitle);
  vtkDMMLReadXMLBooleanMacro(yAxisTitleVisibility, YAxisTitleVisibility);
  vtkDMMLReadXMLIntMacro(axisTitleFontSize, AxisTitleFontSize);
  vtkDMMLReadXMLIntMacro(axisLabelFontSize, AxisLabelFontSize);
  vtkDMMLReadXMLStringMacro(fontType, FontType);
  vtkDMMLReadXMLBooleanMacro(enablePointMoveAlongX, EnablePointMoveAlongX);
  vtkDMMLReadXMLBooleanMacro(enablePointMoveAlongY, EnablePointMoveAlongY);
  vtkDMMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLPlotChartNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkDMMLCopyBeginMacro(anode);
  vtkDMMLCopyStringMacro(Title);
  vtkDMMLCopyIntMacro(TitleFontSize);
  vtkDMMLCopyBooleanMacro(TitleVisibility);
  vtkDMMLCopyBooleanMacro(GridVisibility);
  vtkDMMLCopyBooleanMacro(LegendVisibility);
  vtkDMMLCopyIntMacro(LegendFontSize);
  vtkDMMLCopyBooleanMacro(XAxisRangeAuto);
  vtkDMMLCopyBooleanMacro(YAxisRangeAuto);
  vtkDMMLCopyVectorMacro(XAxisRange, double, 2);
  vtkDMMLCopyVectorMacro(YAxisRange, double, 2);
  vtkDMMLCopyBooleanMacro(XAxisLogScale);
  vtkDMMLCopyBooleanMacro(YAxisLogScale);
  vtkDMMLCopyStringMacro(XAxisTitle);
  vtkDMMLCopyBooleanMacro(XAxisTitleVisibility);
  vtkDMMLCopyStringMacro(YAxisTitle);
  vtkDMMLCopyBooleanMacro(YAxisTitleVisibility);
  vtkDMMLCopyIntMacro(AxisTitleFontSize);
  vtkDMMLCopyIntMacro(AxisLabelFontSize);
  vtkDMMLCopyStringMacro(FontType);
  vtkDMMLCopyBooleanMacro(EnablePointMoveAlongX);
  vtkDMMLCopyBooleanMacro(EnablePointMoveAlongY);
  vtkDMMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLPlotChartNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDMMLNode::PrintSelf(os,indent);

  vtkDMMLPrintBeginMacro(os, indent);
  vtkDMMLPrintStringMacro(Title);
  vtkDMMLPrintIntMacro(TitleFontSize);
  vtkDMMLPrintBooleanMacro(TitleVisibility);
  vtkDMMLPrintBooleanMacro(GridVisibility);
  vtkDMMLPrintBooleanMacro(LegendVisibility);
  vtkDMMLPrintIntMacro(LegendFontSize);
  vtkDMMLPrintBooleanMacro(XAxisRangeAuto);
  vtkDMMLPrintBooleanMacro(YAxisRangeAuto);
  vtkDMMLPrintVectorMacro(XAxisRange, double, 2);
  vtkDMMLPrintVectorMacro(YAxisRange, double, 2);
  vtkDMMLPrintBooleanMacro(XAxisLogScale);
  vtkDMMLPrintBooleanMacro(YAxisLogScale);
  vtkDMMLPrintStringMacro(XAxisTitle);
  vtkDMMLPrintBooleanMacro(XAxisTitleVisibility);
  vtkDMMLPrintStringMacro(YAxisTitle);
  vtkDMMLPrintBooleanMacro(YAxisTitleVisibility);
  vtkDMMLPrintIntMacro(AxisTitleFontSize);
  vtkDMMLPrintIntMacro(AxisLabelFontSize);
  vtkDMMLPrintStringMacro(FontType);
  vtkDMMLPrintBooleanMacro(EnablePointMoveAlongX);
  vtkDMMLPrintBooleanMacro(EnablePointMoveAlongY);
  vtkDMMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLPlotChartNode::SetAndObservePlotSeriesNodeID(const char *plotSeriesNodeID)
{
  this->SetAndObserveNodeReferenceID(this->GetPlotSeriesNodeReferenceRole(), plotSeriesNodeID);
}

//----------------------------------------------------------------------------
void vtkDMMLPlotChartNode::AddAndObservePlotSeriesNodeID(const char *plotSeriesNodeID)
{
  this->AddAndObserveNodeReferenceID(this->GetPlotSeriesNodeReferenceRole(), plotSeriesNodeID);
}

//----------------------------------------------------------------------------
void vtkDMMLPlotChartNode::RemovePlotSeriesNodeID(const char *plotSeriesNodeID)
{
  if (!plotSeriesNodeID)
    {
    return;
    }

  this->RemoveNthPlotSeriesNodeID(this->GetPlotSeriesNodeIndexFromID(plotSeriesNodeID));
}

//----------------------------------------------------------------------------
void vtkDMMLPlotChartNode::RemoveNthPlotSeriesNodeID(int n)
{
  this->RemoveNthNodeReferenceID(this->GetPlotSeriesNodeReferenceRole(), n);
}

//----------------------------------------------------------------------------
void vtkDMMLPlotChartNode::RemoveAllPlotSeriesNodeIDs()
{
  this->RemoveNodeReferenceIDs(this->GetPlotSeriesNodeReferenceRole());
}

//----------------------------------------------------------------------------
void vtkDMMLPlotChartNode::SetAndObserveNthPlotSeriesNodeID(int n, const char *plotSeriesNodeID)
{
  this->SetAndObserveNthNodeReferenceID(this->GetPlotSeriesNodeReferenceRole(), n, plotSeriesNodeID);
}

//----------------------------------------------------------------------------
bool vtkDMMLPlotChartNode::HasPlotSeriesNodeID(const char* plotSeriesNodeID)
{
  return this->HasNodeReferenceID(this->GetPlotSeriesNodeReferenceRole(), plotSeriesNodeID);
}

//----------------------------------------------------------------------------
int vtkDMMLPlotChartNode::GetNumberOfPlotSeriesNodes()
{
  return this->GetNumberOfNodeReferences(this->GetPlotSeriesNodeReferenceRole());
}

//----------------------------------------------------------------------------
const char* vtkDMMLPlotChartNode::GetNthPlotSeriesNodeID(int n)
{
    return this->GetNthNodeReferenceID(this->GetPlotSeriesNodeReferenceRole(), n);
}

//----------------------------------------------------------------------------
int vtkDMMLPlotChartNode::GetPlotSeriesNodeIndexFromID(const char *plotSeriesNodeID)
{
  if (!plotSeriesNodeID)
    {
    return -1;
    }

  int numPlotSeriesNodes = this->GetNumberOfNodeReferences(
    this->GetPlotSeriesNodeReferenceRole());

  for (int plotIndex = 0; plotIndex < numPlotSeriesNodes; plotIndex++)
    {
    const char* id = this->GetNthNodeReferenceID(
      this->GetPlotSeriesNodeReferenceRole(), plotIndex);
    if (id && !strcmp(plotSeriesNodeID, id))
      {
      return plotIndex;
      break;
      }
    }

  return -1;
}

//----------------------------------------------------------------------------
const char* vtkDMMLPlotChartNode::GetPlotSeriesNodeID()
{
  return this->GetNthPlotSeriesNodeID(0);
}

//----------------------------------------------------------------------------
vtkDMMLPlotSeriesNode* vtkDMMLPlotChartNode::GetNthPlotSeriesNode(int n)
{
  return vtkDMMLPlotSeriesNode::SafeDownCast(
    this->GetNthNodeReference(this->GetPlotSeriesNodeReferenceRole(), n));
}

//----------------------------------------------------------------------------
vtkDMMLPlotSeriesNode* vtkDMMLPlotChartNode::GetPlotSeriesNode()
{
  return this->GetNthPlotSeriesNode(0);
}

//----------------------------------------------------------------------------
void vtkDMMLPlotChartNode::ProcessDMMLEvents(vtkObject *caller,
                                              unsigned long event,
                                              void *callData)
{
  Superclass::ProcessDMMLEvents(caller, event, callData);

  int numPlotSeriesNodes = this->GetNumberOfNodeReferences(this->GetPlotSeriesNodeReferenceRole());
  for (int plotIndex = 0; plotIndex < numPlotSeriesNodes; plotIndex++)
    {
    vtkDMMLPlotSeriesNode *pnode = this->GetNthPlotSeriesNode(plotIndex);
    if (pnode != nullptr && pnode == vtkDMMLPlotSeriesNode::SafeDownCast(caller) &&
       (event ==  vtkCommand::ModifiedEvent || event == vtkDMMLPlotSeriesNode::TableModifiedEvent))
      {
      this->InvokeEvent(vtkDMMLPlotChartNode::PlotModifiedEvent, pnode);
      this->Modified();
      }
    }

  return;
}

//----------------------------------------------------------------------------
int vtkDMMLPlotChartNode::GetPlotSeriesNodeNames(std::vector<std::string> &plotSeriesNodeNames)
{
  plotSeriesNodeNames.clear();
  int numPlotSeriesNodes = this->GetNumberOfNodeReferences(this->GetPlotSeriesNodeReferenceRole());
  for (int plotIndex = 0; plotIndex < numPlotSeriesNodes; plotIndex++)
    {
    vtkDMMLPlotSeriesNode *pnode = this->GetNthPlotSeriesNode(plotIndex);
    if (!pnode)
      {
      continue;
      }
    plotSeriesNodeNames.emplace_back(pnode->GetName());
    }

  return static_cast<int>(plotSeriesNodeNames.size());
}

//----------------------------------------------------------------------------
int vtkDMMLPlotChartNode::GetPlotSeriesNodeIDs(std::vector<std::string> &plotSeriesNodeIDs)
{
  plotSeriesNodeIDs.clear();
  int numPlotSeriesNodes = this->GetNumberOfNodeReferences(this->GetPlotSeriesNodeReferenceRole());
  for (int plotIndex = 0; plotIndex < numPlotSeriesNodes; plotIndex++)
    {
    plotSeriesNodeIDs.emplace_back(this->GetNthPlotSeriesNodeID(plotIndex));
    }

  return static_cast<int>(plotSeriesNodeIDs.size());
}

// --------------------------------------------------------------------------
void vtkDMMLPlotChartNode::SetPropertyToAllPlotSeriesNodes(PlotSeriesNodeProperty plotProperty, const char* value)
{
  if (!this->GetScene())
    {
    vtkErrorMacro("vtkDMMLPlotChartNode::SetPropertyToAllPlotSeriesNodes failed: invalid scene");
    return;
    }

  int numPlotSeriesNodes = this->GetNumberOfNodeReferences(this->GetPlotSeriesNodeReferenceRole());

  std::vector<int> plotSeriesNodesWasModifying(numPlotSeriesNodes, 0);

  // Update all plot nodes and invoke modified events at the end

  for (int plotIndex = 0; plotIndex < numPlotSeriesNodes; plotIndex++)
    {
    vtkDMMLPlotSeriesNode *plotSeriesNode = vtkDMMLPlotSeriesNode::SafeDownCast(this->GetNthNodeReference(this->GetPlotSeriesNodeReferenceRole(), plotIndex));
    if (!plotSeriesNode)
      {
      continue;
      }
    plotSeriesNodesWasModifying[plotIndex] = plotSeriesNode->StartModify();

    if (plotProperty == PlotType)
      {
      plotSeriesNode->SetPlotType(value);
      }
    else if (plotProperty == PlotXColumnName)
      {
      plotSeriesNode->SetXColumnName(value);
      }
    else if (plotProperty == PlotYColumnName)
      {
      plotSeriesNode->SetYColumnName(value);
      }
    else if (plotProperty == PlotMarkerStyle)
      {
      plotSeriesNode->SetMarkerStyle(plotSeriesNode->GetMarkerStyleFromString(value));
      }
    }

  for (int plotIndex = 0; plotIndex < numPlotSeriesNodes; plotIndex++)
    {
    vtkDMMLPlotSeriesNode *plotSeriesNode = vtkDMMLPlotSeriesNode::SafeDownCast(this->GetNthNodeReference(this->GetPlotSeriesNodeReferenceRole(), plotIndex));
    if (!plotSeriesNode)
      {
      continue;
      }
    plotSeriesNode->EndModify(plotSeriesNodesWasModifying[plotIndex]);
  }
}

// --------------------------------------------------------------------------
bool vtkDMMLPlotChartNode::GetPropertyFromAllPlotSeriesNodes(PlotSeriesNodeProperty plotProperty, std::string& value)
{
  value.clear();
  if (!this->GetScene())
    {
    vtkErrorMacro("vtkDMMLPlotChartNode::GetPropertyFromAllPlotSeriesNodes failed: invalid scene");
    return false;
    }

  int numPlotSeriesNodes = this->GetNumberOfNodeReferences(this->GetPlotSeriesNodeReferenceRole());

  if (numPlotSeriesNodes < 1)
    {
    return false;
    }

  bool commonPropertyDefined = false;

  for (int plotIndex = 0; plotIndex < numPlotSeriesNodes; plotIndex++)
    {
    vtkDMMLPlotSeriesNode *plotSeriesNode = vtkDMMLPlotSeriesNode::SafeDownCast(this->GetNthNodeReference(this->GetPlotSeriesNodeReferenceRole(), plotIndex));
    if (!plotSeriesNode)
      {
      continue;
      }

    // Get property value
    std::string propertyValue;
    if (plotProperty == PlotType)
      {
      propertyValue = plotSeriesNode->GetPlotTypeAsString(plotSeriesNode->GetPlotType());
      }
    else if (plotProperty == PlotXColumnName)
      {
      propertyValue = plotSeriesNode->GetXColumnName();
      }
    else if (plotProperty == PlotYColumnName)
      {
      propertyValue = plotSeriesNode->GetYColumnName();
      }
    else if (plotProperty == PlotMarkerStyle)
      {
      propertyValue = plotSeriesNode->GetMarkerStyleAsString(plotSeriesNode->GetMarkerStyle());
      }

    if (commonPropertyDefined)
      {
      if (propertyValue != value)
        {
        // not all plot nodes have the same property value
        return false;
        }
      }
    else
      {
      commonPropertyDefined = true;
      value = propertyValue;
      }
    }

  return true;
}
