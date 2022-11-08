/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#include "vtkDMMLMarkupsStorageNode.h"

#include "vtkDMMLScene.h"

#include "vtkObjectFactory.h"
#include "vtkStringArray.h"

#include <sstream>

//----------------------------------------------------------------------------
vtkDMMLMarkupsStorageNode::vtkDMMLMarkupsStorageNode()
{
  this->CoordinateSystem = vtkDMMLMarkupsStorageNode::LPS;
}

//----------------------------------------------------------------------------
vtkDMMLMarkupsStorageNode::~vtkDMMLMarkupsStorageNode() = default;

//----------------------------------------------------------------------------
void vtkDMMLMarkupsStorageNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  this->Superclass::ReadXMLAttributes(atts);

  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLEnumMacro(coordinateSystem, CoordinateSystem);
  vtkDMMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  vtkDMMLPrintBeginMacro(os, indent);
  vtkDMMLPrintEnumMacro(CoordinateSystem);
  vtkDMMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsStorageNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  this->Superclass::WriteXML(of, nIndent);

  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLEnumMacro(coordinateSystem, CoordinateSystem);
  vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsStorageNode::Copy(vtkDMMLNode *anode)
{
  int disabledModify = this->StartModify();

  this->Superclass::Copy(anode);

  vtkDMMLCopyBeginMacro(anode);
  vtkDMMLCopyEnumMacro(CoordinateSystem);
  vtkDMMLCopyEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
bool vtkDMMLMarkupsStorageNode::CanReadInReferenceNode(vtkDMMLNode *refNode)
{
  return refNode->IsA("vtkDMMLMarkupsNode");
}

//----------------------------------------------------------------------------
std::string vtkDMMLMarkupsStorageNode::GetCoordinateSystemAsString()
{
  return vtkDMMLStorageNode::GetCoordinateSystemTypeAsString(this->CoordinateSystem);
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsStorageNode::UseRASOn()
{
  this->SetCoordinateSystem(vtkDMMLStorageNode::CoordinateSystemRAS);
}

//----------------------------------------------------------------------------
bool vtkDMMLMarkupsStorageNode::GetUseRAS()
{
  return (this->GetCoordinateSystem() == vtkDMMLStorageNode::CoordinateSystemRAS);
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsStorageNode::UseLPSOn()
{
  this->SetCoordinateSystem(vtkDMMLStorageNode::CoordinateSystemLPS);
}

//----------------------------------------------------------------------------
bool vtkDMMLMarkupsStorageNode::GetUseLPS()
{
  return (this->GetCoordinateSystem() == vtkDMMLStorageNode::CoordinateSystemLPS);
}

//---------------------------------------------------------------------------
const char* vtkDMMLMarkupsStorageNode::GetCoordinateSystemAsString(int id)
{
  return vtkDMMLStorageNode::GetCoordinateSystemTypeAsString(id);
}

//-----------------------------------------------------------
int vtkDMMLMarkupsStorageNode::GetCoordinateSystemFromString(const char* name)
{
  // For backward-compatibility with old scenes (magic number was used instead of string)
  if (strcmp(name, "0") == 0)
    {
    return vtkDMMLStorageNode::CoordinateSystemRAS;
    }
  else if (strcmp(name, "1") == 0)
    {
    return vtkDMMLStorageNode::CoordinateSystemLPS;
    }

  // Current method, store coordinate system as string
  return vtkDMMLStorageNode::GetCoordinateSystemTypeFromString(name);
}
