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

// DMML includes
#include "vtkDMMLTableViewNode.h"

#include "vtkDMMLScene.h"
#include "vtkDMMLTableNode.h"

// VTK includes
#include <vtkCommand.h> // for vtkCommand::ModifiedEvent
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

const char* vtkDMMLTableViewNode::TableNodeReferenceRole = "table";
const char* vtkDMMLTableViewNode::TableNodeReferenceDMMLAttributeName = "tableNodeRef";

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLTableViewNode);

//----------------------------------------------------------------------------
vtkDMMLTableViewNode::vtkDMMLTableViewNode()
{
  this->AddNodeReferenceRole(this->GetTableNodeReferenceRole(),
                             this->GetTableNodeReferenceDMMLAttributeName());
}

//----------------------------------------------------------------------------
vtkDMMLTableViewNode::~vtkDMMLTableViewNode() = default;

//----------------------------------------------------------------------------
const char* vtkDMMLTableViewNode::GetNodeTagName()
{
  return "TableView";
}


//----------------------------------------------------------------------------
void vtkDMMLTableViewNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  of << " doPropagateTableSelection=\"" << (int)this->DoPropagateTableSelection << "\"";
}

//----------------------------------------------------------------------------
void vtkDMMLTableViewNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  Superclass::ReadXMLAttributes(atts);
  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if(!strcmp (attName, "doPropagateTableSelection" ))
      {
      this->SetDoPropagateTableSelection(atoi(attValue)?true:false);
      }
    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, SliceID
void vtkDMMLTableViewNode::Copy(vtkDMMLNode *anode)
{
  int disabledModify = this->StartModify();
  Superclass::Copy(anode);
  vtkDMMLTableViewNode *node = vtkDMMLTableViewNode::SafeDownCast(anode);
  if (node)
    {
    this->SetDoPropagateTableSelection (node->GetDoPropagateTableSelection());
    }
  else
    {
    vtkErrorMacro("vtkDMMLTableViewNode::Copy failed: invalid input node");
    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLTableViewNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << indent << "DoPropagateTableSelection: " << this->DoPropagateTableSelection << "\n";
}

//----------------------------------------------------------------------------
void vtkDMMLTableViewNode::SetTableNodeID(const char* tableNodeId)
{
  this->SetNodeReferenceID(this->GetTableNodeReferenceRole(), tableNodeId);
}

//----------------------------------------------------------------------------
const char* vtkDMMLTableViewNode::GetTableNodeID()
{
  return this->GetNodeReferenceID(this->GetTableNodeReferenceRole());
}

//----------------------------------------------------------------------------
vtkDMMLTableNode* vtkDMMLTableViewNode::GetTableNode()
{
  return vtkDMMLTableNode::SafeDownCast(this->GetNodeReference(this->GetTableNodeReferenceRole()));
}

//----------------------------------------------------------------------------
const char* vtkDMMLTableViewNode::GetTableNodeReferenceRole()
{
  return vtkDMMLTableViewNode::TableNodeReferenceRole;
}

//----------------------------------------------------------------------------
const char* vtkDMMLTableViewNode::GetTableNodeReferenceDMMLAttributeName()
{
  return vtkDMMLTableViewNode::TableNodeReferenceDMMLAttributeName;
}
