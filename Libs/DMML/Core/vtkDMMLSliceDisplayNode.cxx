/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Ebatinca S.L., Las Palmas de Gran Canaria, Spain

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// DMML includes
#include "vtkDMMLSliceDisplayNode.h"

// VTK includes

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLSliceDisplayNode);

//-----------------------------------------------------------------------------
vtkDMMLSliceDisplayNode::vtkDMMLSliceDisplayNode()
{
  // Set active component defaults for mouse (identified by empty string)
  this->ActiveComponents[GetDefaultContextName()] = ComponentInfo();
}

//-----------------------------------------------------------------------------
vtkDMMLSliceDisplayNode::~vtkDMMLSliceDisplayNode()
{
}

//----------------------------------------------------------------------------
void vtkDMMLSliceDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  vtkDMMLPrintBeginMacro(os, indent);
  vtkDMMLPrintBooleanMacro(IntersectingSlicesInteractive);
  vtkDMMLPrintBooleanMacro(IntersectingSlicesTranslationEnabled);
  vtkDMMLPrintBooleanMacro(IntersectingSlicesRotationEnabled);
  vtkDMMLPrintIntMacro(IntersectingSlicesInteractiveHandlesVisibilityMode);
  vtkDMMLPrintIntMacro(IntersectingSlicesIntersectionMode);
  vtkDMMLPrintIntMacro(IntersectingSlicesLineThicknessMode);
  {
  os << indent << "ActiveComponents:";
  for (std::map<std::string, ComponentInfo>::iterator it = this->ActiveComponents.begin(); it != this->ActiveComponents.end(); ++it)
    {
    os << indent << indent;
    if (it->first.empty())
      {
      os << "(default)";
      }
    else
      {
      os << it->first;
      }
    os << ": " << it->second.Type << ", " << it->second.Index;
    }
  os << "\n";
  }
  vtkDMMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  this->Superclass::WriteXML(of, nIndent);

  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLBooleanMacro(intersectingSlicesInteractive, IntersectingSlicesInteractive);
  vtkDMMLWriteXMLBooleanMacro(intersectingSlicesTranslationEnabled, IntersectingSlicesTranslationEnabled);
  vtkDMMLWriteXMLBooleanMacro(intersectingSlicesRotationEnabled, IntersectingSlicesRotationEnabled);
  vtkDMMLWriteXMLIntMacro(intersectingSlicesInteractiveHandlesVisibilityMode, IntersectingSlicesInteractiveHandlesVisibilityMode);
  vtkDMMLWriteXMLIntMacro(intersectingSlicesIntersectionMode, IntersectingSlicesIntersectionMode);
  vtkDMMLWriteXMLIntMacro(intersectingSlicesIntersectionMode, IntersectingSlicesLineThicknessMode);
  vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  this->Superclass::ReadXMLAttributes(atts);

  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLBooleanMacro(intersectingSlicesInteractive, IntersectingSlicesInteractive);
  vtkDMMLReadXMLBooleanMacro(intersectingSlicesTranslationEnabled, IntersectingSlicesTranslationEnabled);
  vtkDMMLReadXMLBooleanMacro(intersectingSlicesRotationEnabled, IntersectingSlicesRotationEnabled);
  vtkDMMLReadXMLIntMacro(intersectingSlicesInteractiveHandlesVisibilityMode, IntersectingSlicesInteractiveHandlesVisibilityMode);
  vtkDMMLReadXMLIntMacro(intersectingSlicesIntersectionMode, IntersectingSlicesIntersectionMode);
  vtkDMMLReadXMLIntMacro(intersectingSlicesLineThicknessMode, IntersectingSlicesLineThicknessMode);
  vtkDMMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLSliceDisplayNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkDMMLSliceDisplayNode* node = vtkDMMLSliceDisplayNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  vtkDMMLCopyBeginMacro(anode);
  vtkDMMLCopyBooleanMacro(IntersectingSlicesInteractive);
  vtkDMMLCopyBooleanMacro(IntersectingSlicesTranslationEnabled);
  vtkDMMLCopyBooleanMacro(IntersectingSlicesRotationEnabled);
  vtkDMMLCopyIntMacro(IntersectingSlicesInteractiveHandlesVisibilityMode);
  vtkDMMLCopyIntMacro(IntersectingSlicesIntersectionMode);
  vtkDMMLCopyIntMacro(IntersectingSlicesLineThicknessMode);
  vtkDMMLCopyEndMacro();
}

//---------------------------------------------------------------------------
void vtkDMMLSliceDisplayNode::SetIntersectingSlicesInteractiveModeEnabled(
  IntersectingSlicesInteractiveMode mode, bool enabled)
{
  switch (mode)
    {
    case vtkDMMLSliceDisplayNode::ModeTranslation:
      this->SetIntersectingSlicesTranslationEnabled(enabled);
      break;
    case vtkDMMLSliceDisplayNode::ModeRotation:
      this->SetIntersectingSlicesRotationEnabled(enabled);
      break;
    default:
      vtkErrorMacro("Unknown mode");
      break;
    }
}

//---------------------------------------------------------------------------
bool vtkDMMLSliceDisplayNode::GetIntersectingSlicesInteractiveModeEnabled(
  IntersectingSlicesInteractiveMode mode)
{
  switch (mode)
    {
    case vtkDMMLSliceDisplayNode::ModeTranslation:
      return this->GetIntersectingSlicesTranslationEnabled();
    case vtkDMMLSliceDisplayNode::ModeRotation:
      return this->GetIntersectingSlicesRotationEnabled();
    default:
      vtkErrorMacro("Unknown mode");
    }
  return false;
}

//-----------------------------------------------------------
int vtkDMMLSliceDisplayNode::GetIntersectingSlicesInteractiveHandlesVisibilityModeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < HandlesVisibilityMode_Last; ii++)
    {
    if (strcmp(name, GetIntersectingSlicesInteractiveHandlesVisibilityModeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//---------------------------------------------------------------------------
const char* vtkDMMLSliceDisplayNode::GetIntersectingSlicesInteractiveHandlesVisibilityModeAsString(int id)
{
  switch (id)
    {
    case NeverVisible: return "NeverVisible";
    case NearbyVisible: return "NearbyVisible";
    case AlwaysVisible: return "AlwaysVisible";
    //case FadingVisible: return "FadingVisible";
    default:
      // invalid id
      return "Invalid";
    }
}

//-----------------------------------------------------------
int vtkDMMLSliceDisplayNode::GetIntersectingSlicesIntersectionModeFromString(const char* name)
  {
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < IntersectionMode_Last; ii++)
    {
    if (strcmp(name, GetIntersectingSlicesIntersectionModeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
  }

//---------------------------------------------------------------------------
const char* vtkDMMLSliceDisplayNode::GetIntersectingSlicesIntersectionModeAsString(int id)
{
  switch (id)
    {
    case SkipLineCrossings: return "SkipLineCrossings";
    case FullLines: return "FullLines";
    default:
      // invalid id
      return "Invalid";
    }
}

//-----------------------------------------------------------
int vtkDMMLSliceDisplayNode::GetIntersectingSlicesLineThicknessModeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < LineThicknessMode_Last; ii++)
    {
    if (strcmp(name, GetIntersectingSlicesLineThicknessModeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//---------------------------------------------------------------------------
const char* vtkDMMLSliceDisplayNode::GetIntersectingSlicesLineThicknessModeAsString(int id)
{
  switch (id)
    {
    case FineLines: return "FineLines";
    case MediumLines: return "MediumLines";
    case ThickLines: return "ThickLines";
    default:
      // invalid id
      return "Invalid";
    }
}

//---------------------------------------------------------------------------
int vtkDMMLSliceDisplayNode::GetActiveComponentType(std::string context/*=GetDefaultContextName()*/)
{
  if (this->ActiveComponents.find(context) == this->ActiveComponents.end())
    {
    vtkErrorMacro("GetActiveComponentType: No interaction context with identifier '" << context << "' was found");
    return ComponentNone;
    }

  return this->ActiveComponents[context].Type;
}

//---------------------------------------------------------------------------
int vtkDMMLSliceDisplayNode::GetActiveComponentIndex(std::string context/*=GetDefaultContextName()*/)
{
  if (this->ActiveComponents.find(context) == this->ActiveComponents.end())
    {
    vtkErrorMacro("GetActiveComponentIndex: No interaction context with identifier '" << context << "' was found");
    return -1;
    }

  return this->ActiveComponents[context].Index;
}

//---------------------------------------------------------------------------
void vtkDMMLSliceDisplayNode::SetActiveComponent(int componentType, int componentIndex, std::string context/*=GetDefaultContextName()*/)
{
  if ( this->ActiveComponents.find(context) != this->ActiveComponents.end()
    && this->ActiveComponents[context].Type == componentType
    && this->ActiveComponents[context].Index == componentIndex )
    {
    // no change
    return;
    }
  this->ActiveComponents[context].Index = componentIndex;
  this->ActiveComponents[context].Type = componentType;
  this->Modified();
}

//---------------------------------------------------------------------------
bool vtkDMMLSliceDisplayNode::HasActiveComponent()
{
  for (std::map<std::string, ComponentInfo>::iterator it = this->ActiveComponents.begin(); it != this->ActiveComponents.end(); ++it)
    {
    if (it->second.Type != ComponentNone)
      {
      return true;
      }
    }
  return false;
}

//---------------------------------------------------------------------------
std::vector<std::string> vtkDMMLSliceDisplayNode::GetActiveComponentInteractionContexts()
{
  std::vector<std::string> interactionContextVector;
  for (std::map<std::string, ComponentInfo>::iterator it = this->ActiveComponents.begin(); it != this->ActiveComponents.end(); ++it)
    {
    if (it->second.Type != ComponentNone)
      {
      interactionContextVector.push_back(it->first);
      }
    }
  return interactionContextVector;
}
