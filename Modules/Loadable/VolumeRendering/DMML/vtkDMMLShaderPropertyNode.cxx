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

  This file was originally developed by Simon Drouin, Brigham and Women's
  Hospital, Boston, MA.

==============================================================================*/

// DMML includes
#include "vtkDMMLScene.h"
#include "vtkDMMLShaderPropertyNode.h"
#include "vtkDMMLVolumePropertyStorageNode.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkIntArray.h>
#include <vtkObjectFactory.h>
#include <vtkShaderProperty.h>
#include <vtkUniforms.h>

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLShaderPropertyNode);

//----------------------------------------------------------------------------
vtkDMMLShaderPropertyNode::vtkDMMLShaderPropertyNode()
{
  this->ObservedEvents = vtkIntArray::New();
  this->ObservedEvents->InsertNextValue(vtkCommand::ModifiedEvent);

  vtkShaderProperty* property = vtkShaderProperty::New();
  vtkSetAndObserveDMMLObjectEventsMacro(this->ShaderProperty, property, this->ObservedEvents);
  property->Delete();

  // Observe uniform variables
  vtkObserveDMMLObjectEventsMacro(this->ShaderProperty->GetVertexCustomUniforms(), this->ObservedEvents);
  vtkObserveDMMLObjectEventsMacro(this->ShaderProperty->GetFragmentCustomUniforms(), this->ObservedEvents);

  this->SetHideFromEditors(0);
}

//----------------------------------------------------------------------------
vtkDMMLShaderPropertyNode::~vtkDMMLShaderPropertyNode()
{
  if(this->ShaderProperty)
    {
    vtkUnObserveDMMLObjectMacro(this->ShaderProperty->GetVertexCustomUniforms());
    vtkUnObserveDMMLObjectMacro(this->ShaderProperty->GetFragmentCustomUniforms());
    vtkSetAndObserveDMMLObjectMacro(this->ShaderProperty, nullptr);
    }
  this->ObservedEvents->Delete();
}

//----------------------------------------------------------------------------
vtkUniforms * vtkDMMLShaderPropertyNode::GetVertexUniforms()
{
  return this->ShaderProperty->GetVertexCustomUniforms();
}

//----------------------------------------------------------------------------
vtkUniforms * vtkDMMLShaderPropertyNode::GetFragmentUniforms()
{
  return this->ShaderProperty->GetFragmentCustomUniforms();
}

//----------------------------------------------------------------------------
vtkUniforms * vtkDMMLShaderPropertyNode::GetGeometryUniforms()
{
  return this->ShaderProperty->GetGeometryCustomUniforms();
}

//----------------------------------------------------------------------------
void vtkDMMLShaderPropertyNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  this->Superclass::WriteXML(of, nIndent);

//  vtkDMMLWriteXMLBeginMacro(of);
//  vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLShaderPropertyNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  this->Superclass::ReadXMLAttributes(atts);

//  vtkDMMLReadXMLBeginMacro(atts);
//  vtkDMMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLShaderPropertyNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkDMMLShaderPropertyNode* node = vtkDMMLShaderPropertyNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  this->ShaderProperty->DeepCopy( node->ShaderProperty);
}

//----------------------------------------------------------------------------
void vtkDMMLShaderPropertyNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "ShaderProperty: ";
  this->ShaderProperty->PrintSelf(os,indent.GetNextIndent());
}

//---------------------------------------------------------------------------
void vtkDMMLShaderPropertyNode::ProcessDMMLEvents( vtkObject *caller,
                                                   unsigned long event,
                                                   void *callData)
{
  this->Superclass::ProcessDMMLEvents(caller, event, callData);
  switch (event)
    {
    case vtkCommand::ModifiedEvent:
      this->Modified();
      break;
    }
}

//---------------------------------------------------------------------------
vtkDMMLStorageNode* vtkDMMLShaderPropertyNode::CreateDefaultStorageNode()
{
  vtkDMMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkDMMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkDMMLShaderPropertyStorageNode"));
}

//---------------------------------------------------------------------------
bool vtkDMMLShaderPropertyNode::GetModifiedSinceRead()
{
  return this->Superclass::GetModifiedSinceRead() ||
    (this->ShaderProperty &&
     this->ShaderProperty->GetMTime() > this->GetStoredTime());
}
