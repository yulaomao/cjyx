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

// DMML includes
#include "vtkDMMLPlotChartNode.h"
#include "vtkDMMLPlotViewNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSelectionNode.h"

// VTK includes
#include <vtkAssignAttribute.h>
#include <vtkCommand.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// STD includes
#include <sstream>

const char* vtkDMMLPlotViewNode::PlotChartNodeReferenceRole = "plotChart";
const char* vtkDMMLPlotViewNode::PlotChartNodeReferenceDMMLAttributeName = "plotChartNodeRef";

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLPlotViewNode);

//----------------------------------------------------------------------------
vtkDMMLPlotViewNode::vtkDMMLPlotViewNode()
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkDMMLPlotViewNode::PlotChartNodeChangedEvent);
  events->InsertNextValue(vtkDMMLPlotChartNode::PlotModifiedEvent);

  this->AddNodeReferenceRole(this->GetPlotChartNodeReferenceRole(),
                             this->GetPlotChartNodeReferenceDMMLAttributeName(),
                             events.GetPointer());
}

//----------------------------------------------------------------------------
vtkDMMLPlotViewNode::~vtkDMMLPlotViewNode() = default;

//----------------------------------------------------------------------------
void vtkDMMLPlotViewNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLIntMacro(doPropagatePlotChartSelection, DoPropagatePlotChartSelection);
  vtkDMMLWriteXMLEnumMacro(interactionMode, InteractionMode);
  vtkDMMLWriteXMLBooleanMacro(enablePointMoveAlongX, EnablePointMoveAlongX);
  vtkDMMLWriteXMLBooleanMacro(enablePointMoveAlongY, EnablePointMoveAlongY);
  vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLPlotViewNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLIntMacro(doPropagatePlotChartSelection, DoPropagatePlotChartSelection);
  vtkDMMLReadXMLEnumMacro(interactionMode, InteractionMode);
  vtkDMMLReadXMLBooleanMacro(enablePointMoveAlongX, EnablePointMoveAlongX);
  vtkDMMLReadXMLBooleanMacro(enablePointMoveAlongY, EnablePointMoveAlongY);
  vtkDMMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLPlotViewNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkDMMLCopyBeginMacro(anode);
  vtkDMMLCopyIntMacro(DoPropagatePlotChartSelection);
  vtkDMMLCopyEnumMacro(InteractionMode);
  vtkDMMLCopyBooleanMacro(EnablePointMoveAlongX);
  vtkDMMLCopyBooleanMacro(EnablePointMoveAlongY);
  vtkDMMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLPlotViewNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  vtkDMMLPrintBeginMacro(os, indent);
  vtkDMMLPrintIntMacro(DoPropagatePlotChartSelection);
  vtkDMMLPrintEnumMacro(InteractionMode);
  vtkDMMLPrintBooleanMacro(EnablePointMoveAlongX);
  vtkDMMLPrintBooleanMacro(EnablePointMoveAlongY);
  vtkDMMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLPlotViewNode::SetPlotChartNodeID(const char* plotChartNodeId)
{
  this->SetNodeReferenceID(this->GetPlotChartNodeReferenceRole(), plotChartNodeId);
}

//----------------------------------------------------------------------------
const char* vtkDMMLPlotViewNode::GetPlotChartNodeID()
{
  return this->GetNodeReferenceID(this->GetPlotChartNodeReferenceRole());
}

//----------------------------------------------------------------------------
vtkDMMLPlotChartNode* vtkDMMLPlotViewNode::GetPlotChartNode()
{
  return vtkDMMLPlotChartNode::SafeDownCast(this->GetNodeReference(this->GetPlotChartNodeReferenceRole()));
}

//----------------------------------------------------------------------------
void vtkDMMLPlotViewNode::ProcessDMMLEvents(vtkObject *caller, unsigned long event, void *callData)
{
  Superclass::ProcessDMMLEvents(caller, event, callData);

  vtkDMMLPlotChartNode *pnode = this->GetPlotChartNode();
  if (pnode != nullptr && pnode == vtkDMMLPlotChartNode::SafeDownCast(caller) &&
     (event ==  vtkCommand::ModifiedEvent || event == vtkDMMLPlotChartNode::PlotModifiedEvent))
    {
    this->InvokeEvent(vtkDMMLPlotViewNode::PlotChartNodeChangedEvent, pnode);
    }

  return;
}

//----------------------------------------------------------------------------
const char* vtkDMMLPlotViewNode::GetPlotChartNodeReferenceRole()
{
  return vtkDMMLPlotViewNode::PlotChartNodeReferenceRole;
}

//----------------------------------------------------------------------------
const char* vtkDMMLPlotViewNode::GetPlotChartNodeReferenceDMMLAttributeName()
{
    return vtkDMMLPlotViewNode::PlotChartNodeReferenceDMMLAttributeName;
}

//----------------------------------------------------------------------------
void vtkDMMLPlotViewNode::OnNodeReferenceAdded(vtkDMMLNodeReference *reference)
{
  this->Superclass::OnNodeReferenceAdded(reference);
  if (std::string(reference->GetReferenceRole()) == this->PlotChartNodeReferenceRole)
    {
    this->InvokeEvent(vtkDMMLPlotViewNode::PlotChartNodeChangedEvent, reference->GetReferencedNode());
    }
}

//----------------------------------------------------------------------------
void vtkDMMLPlotViewNode::OnNodeReferenceModified(vtkDMMLNodeReference *reference)
{
  this->Superclass::OnNodeReferenceModified(reference);
  if (std::string(reference->GetReferenceRole()) == this->PlotChartNodeReferenceRole)
    {
    this->InvokeEvent(vtkDMMLPlotViewNode::PlotChartNodeChangedEvent, reference->GetReferencedNode());
    }
}

//----------------------------------------------------------------------------
void vtkDMMLPlotViewNode::OnNodeReferenceRemoved(vtkDMMLNodeReference *reference)
{
  this->Superclass::OnNodeReferenceRemoved(reference);
  if (std::string(reference->GetReferenceRole()) == this->PlotChartNodeReferenceRole)
    {
    this->InvokeEvent(vtkDMMLPlotViewNode::PlotChartNodeChangedEvent, reference->GetReferencedNode());
    }
}

//-----------------------------------------------------------
const char* vtkDMMLPlotViewNode::GetInteractionModeAsString(int id)
{
  switch (id)
  {
  case InteractionModePanView: return "PanView";
  case InteractionModeSelectPoints: return "SelectPoints";
  case InteractionModeFreehandSelectPoints: return "FreehandSelectPoints";
  case InteractionModeMovePoints: return "MovePoints";
  default:
    // invalid id
    return "";
  }
}

//-----------------------------------------------------------
int vtkDMMLPlotViewNode::GetInteractionModeFromString(const char* name)
{
  if (name == nullptr)
  {
    // invalid name
    return -1;
  }
  for (int ii = 0; ii < InteractionMode_Last; ii++)
  {
    if (strcmp(name, GetInteractionModeAsString(ii)) == 0)
    {
      // found a matching name
      return ii;
    }
  }
  // unknown name
  return -1;
}
