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

// DMML includes
#include "vtkDMMLColorTableNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSelectionNode.h"
#include "vtkDMMLTableNode.h"

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkAssignAttribute.h>
#include <vtkBrush.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkContextMapper2D.h>
#include <vtkEventBroker.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>
#include <vtkPlot.h>
#include <vtkPlotBar.h>
#include <vtkPlotLine.h>
#include <vtkPlotPoints.h>
#include <vtkTable.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// STD includes
#include <algorithm>
#include <sstream>

const char* vtkDMMLPlotSeriesNode::TableNodeReferenceRole = "table";
const char* vtkDMMLPlotSeriesNode::TableNodeReferenceDMMLAttributeName = "tableNodeRef";

//------------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLPlotSeriesNode);

//----------------------------------------------------------------------------
vtkDMMLPlotSeriesNode::vtkDMMLPlotSeriesNode()
{
  this->HideFromEditors = 0;
  this->Color[0] = 0.0;
  this->Color[1] = 0.0;
  this->Color[2] = 0.0;

  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkDMMLPlotSeriesNode::TableModifiedEvent);
  this->AddNodeReferenceRole(this->GetTableNodeReferenceRole(),
                             this->GetTableNodeReferenceDMMLAttributeName(),
                             events.GetPointer());
}

//----------------------------------------------------------------------------
vtkDMMLPlotSeriesNode::~vtkDMMLPlotSeriesNode() = default;

//----------------------------------------------------------------------------
const char *vtkDMMLPlotSeriesNode::GetTableNodeReferenceRole()
{
  return vtkDMMLPlotSeriesNode::TableNodeReferenceRole;
}

//----------------------------------------------------------------------------
const char *vtkDMMLPlotSeriesNode::GetTableNodeReferenceDMMLAttributeName()
{
  return vtkDMMLPlotSeriesNode::TableNodeReferenceDMMLAttributeName;
}

//----------------------------------------------------------------------------
void vtkDMMLPlotSeriesNode::WriteXML(ostream& of, int nIndent)
{
  // Start by having the superclass write its information
  Superclass::WriteXML(of, nIndent);

  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLEnumMacro(plotType, PlotType);
  vtkDMMLWriteXMLStdStringMacro(xColumnName, XColumnName);
  vtkDMMLWriteXMLStdStringMacro(yColumnName, YColumnName);
  vtkDMMLWriteXMLStdStringMacro(labelColumnName, LabelColumnName);
  vtkDMMLWriteXMLEnumMacro(markerStyle, MarkerStyle);
  vtkDMMLWriteXMLFloatMacro(markerSize, MarkerSize);
  vtkDMMLWriteXMLEnumMacro(lineStyle, LineStyle);
  vtkDMMLWriteXMLFloatMacro(lineWidth, LineWidth);
  vtkDMMLWriteXMLVectorMacro(color, Color, double, 3);
  vtkDMMLWriteXMLFloatMacro(opacity, Opacity);
  vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLPlotSeriesNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLEnumMacro(plotType, PlotType);
  vtkDMMLReadXMLStdStringMacro(xColumnName, XColumnName);
  vtkDMMLReadXMLStdStringMacro(yColumnName, YColumnName);
  vtkDMMLReadXMLStdStringMacro(labelColumnName, LabelColumnName);
  vtkDMMLReadXMLEnumMacro(markerStyle, MarkerStyle);
  vtkDMMLReadXMLFloatMacro(markerSize, MarkerSize);
  vtkDMMLReadXMLEnumMacro(lineStyle, LineStyle);
  vtkDMMLReadXMLFloatMacro(lineWidth, LineWidth);
  vtkDMMLReadXMLVectorMacro(color, Color, double, 3);
  vtkDMMLReadXMLFloatMacro(opacity, Opacity);
  vtkDMMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLPlotSeriesNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkDMMLCopyBeginMacro(anode);
  vtkDMMLCopyEnumMacro(PlotType);
  vtkDMMLCopyStdStringMacro(XColumnName);
  vtkDMMLCopyStdStringMacro(YColumnName);
  vtkDMMLCopyStdStringMacro(LabelColumnName);
  vtkDMMLCopyEnumMacro(MarkerStyle);
  vtkDMMLCopyFloatMacro(MarkerSize);
  vtkDMMLCopyEnumMacro(LineStyle);
  vtkDMMLCopyFloatMacro(LineWidth);
  vtkDMMLCopyVectorMacro(Color, double, 3);
  vtkDMMLCopyFloatMacro(Opacity);
  vtkDMMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLPlotSeriesNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  vtkDMMLPrintBeginMacro(os, indent);
  vtkDMMLPrintEnumMacro(PlotType);
  vtkDMMLPrintStdStringMacro(XColumnName);
  vtkDMMLPrintStdStringMacro(YColumnName);
  vtkDMMLPrintStdStringMacro(LabelColumnName);
  vtkDMMLPrintEnumMacro(MarkerStyle);
  vtkDMMLPrintFloatMacro(MarkerSize);
  vtkDMMLPrintEnumMacro(LineStyle);
  vtkDMMLPrintFloatMacro(LineWidth);
  vtkDMMLPrintVectorMacro(Color, double, 3);
  vtkDMMLPrintFloatMacro(Opacity);
  vtkDMMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLPlotSeriesNode::ProcessDMMLEvents(vtkObject *caller, unsigned long event, void *callData)
{
  Superclass::ProcessDMMLEvents(caller, event, callData);

  if (caller == nullptr ||
      (event != vtkCommand::ModifiedEvent &&
       event != vtkDMMLPlotSeriesNode::TableModifiedEvent))
    {
    return;
    }

  vtkDMMLTableNode *tnode = this->GetTableNode();
  vtkDMMLTableNode *callerTable = vtkDMMLTableNode::SafeDownCast(caller);
  if (callerTable != nullptr && tnode != nullptr && tnode == callerTable &&
      event == vtkCommand::ModifiedEvent)
    {
    this->InvokeCustomModifiedEvent(vtkDMMLPlotSeriesNode::TableModifiedEvent, callerTable);
    }

  return;
}

//----------------------------------------------------------------------------
const char *vtkDMMLPlotSeriesNode::GetTableNodeID()
{
  return this->GetNodeReferenceID(this->GetTableNodeReferenceRole());
}

//----------------------------------------------------------------------------
void vtkDMMLPlotSeriesNode::SetAndObserveTableNodeID(const char *tableNodeID)
{
  // Set and Observe the DMMLTable reference
  this->SetAndObserveNodeReferenceID(this->GetTableNodeReferenceRole(), tableNodeID);
}

//----------------------------------------------------------------------------
void vtkDMMLPlotSeriesNode::SetAndObserveTableNodeID(const std::string &tableNodeID)
{
  return this->SetAndObserveTableNodeID(tableNodeID.c_str());
}

//----------------------------------------------------------------------------
vtkDMMLTableNode *vtkDMMLPlotSeriesNode::GetTableNode()
{
  return vtkDMMLTableNode::SafeDownCast(
    this->GetNodeReference(this->GetTableNodeReferenceRole()));
}

//----------------------------------------------------------------------------
void vtkDMMLPlotSeriesNode::SetPlotType(const char *type)
{
  this->SetPlotType(this->GetPlotTypeFromString(type));
}


//-----------------------------------------------------------
const char* vtkDMMLPlotSeriesNode::GetPlotTypeAsString(int id)
{
  switch (id)
    {
    case PlotTypeLine: return "Line";
    case PlotTypeBar: return "Bar";
    case PlotTypeScatter: return "Scatter";
    case PlotTypeScatterBar: return "ScatterBar";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkDMMLPlotSeriesNode::GetPlotTypeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < PlotType_Last; ii++)
    {
    if (strcmp(name, GetPlotTypeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
const char* vtkDMMLPlotSeriesNode::GetMarkerStyleAsString(int id)
{
  switch (id)
    {
    case MarkerStyleNone: return "none";
    case MarkerStyleCross: return "cross";
    case MarkerStylePlus: return "plus";
    case MarkerStyleSquare: return "square";
    case MarkerStyleCircle: return "circle";
    case MarkerStyleDiamond: return "diamond";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkDMMLPlotSeriesNode::GetMarkerStyleFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < MarkerStyle_Last; ii++)
    {
    if (strcmp(name, GetMarkerStyleAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
const char* vtkDMMLPlotSeriesNode::GetLineStyleAsString(int id)
{
  switch (id)
    {
    case LineStyleNone: return "none";
    case LineStyleSolid: return "solid";
    case LineStyleDash: return "dash";
    case LineStyleDot: return "dot";
    case LineStyleDashDot: return "dash-dot";
    case LineStyleDashDotDot: return "dash-dot-dot";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkDMMLPlotSeriesNode::GetLineStyleFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < LineStyle_Last; ii++)
    {
    if (strcmp(name, GetLineStyleAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
bool vtkDMMLPlotSeriesNode::IsXColumnRequired()
{
  return (this->PlotType == PlotTypeScatter || this->PlotType == PlotTypeScatterBar);
}

//-----------------------------------------------------------
void vtkDMMLPlotSeriesNode::SetUniqueColor(const char* colorTableNodeID)
{
  if (this->GetScene() == nullptr)
    {
    vtkGenericWarningMacro("vtkDMMLPlotSeriesNode::GenerateUniqueColor failed: node is not added to scene");
    return;
    }
  if (colorTableNodeID == nullptr)
    {
    colorTableNodeID = "vtkDMMLColorTableNodeRandom";
    }
  vtkDMMLColorTableNode* colorTableNode = vtkDMMLColorTableNode::SafeDownCast(this->GetScene()->GetNodeByID(colorTableNodeID));
  if (colorTableNode == nullptr)
    {
    vtkGenericWarningMacro("vtkDMMLPlotSeriesNode::GenerateUniqueColor failed: color table node by ID "
      << (colorTableNodeID ? colorTableNodeID : "(none)") << " not found in scene");
    return;
    }
  std::vector< vtkDMMLNode* > seriesNodes;
  this->GetScene()->GetNodesByClass("vtkDMMLPlotSeriesNode", seriesNodes);
  int numberOfColors = colorTableNode->GetNumberOfColors();
  if (numberOfColors < 1)
    {
    vtkGenericWarningMacro("vtkDMMLPlotSeriesNode::GenerateUniqueColor failed: color table node "
      << (colorTableNodeID ? colorTableNodeID : "(none)") << " is empty");
    return;
    }
  double color[4] = { 0,0,0,0 };
  bool isColorUnique = false;
  for (int colorIndex = 0; colorIndex < numberOfColors; colorIndex++)
    {
    colorTableNode->GetColor(colorIndex, color);
    isColorUnique = true;
    for (std::vector< vtkDMMLNode* >::iterator seriesNodeIt = seriesNodes.begin(); seriesNodeIt != seriesNodes.end(); ++seriesNodeIt)
      {
      vtkDMMLPlotSeriesNode* seriesNode = vtkDMMLPlotSeriesNode::SafeDownCast(*seriesNodeIt);
      if (!seriesNode)
        {
        continue;
        }
      if (seriesNode == this)
        {
        continue;
        }
      double* foundColor = seriesNode->GetColor();
      if (fabs(foundColor[0] - color[0]) < 0.1
        && fabs(foundColor[1] - color[1]) < 0.1
        && fabs(foundColor[2] - color[2]) < 0.1)
        {
        isColorUnique = false;
        break;
        }
      }
    if (isColorUnique)
      {
      break;
      }
    }
  if (!isColorUnique)
    {
    // Run out of colors, which means that there are more series than entries
    // in the color table. Use sequential indices to have approximately
    // uniform distribution.
    colorTableNode->GetColor(seriesNodes.size() % numberOfColors, color);
    }
  this->SetColor(color);
}
