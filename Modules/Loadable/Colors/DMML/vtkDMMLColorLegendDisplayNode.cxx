/*==============================================================================

  Program: 3D Cjyx

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#include "vtkDMMLColorLegendDisplayNode.h"

// DMML includes
#include <vtkDMMLScalarVolumeDisplayNode.h>
#include <vtkDMMLModelDisplayNode.h>
#include <vtkDMMLDisplayableNode.h>
#include <vtkDMMLProceduralColorNode.h>
#include <vtkDMMLColorTableNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkTextProperty.h>

// STD includes
#include <cstring>
#include <sstream>

namespace
{
const char* PRIMARY_DISPLAY_NODE_REFERENCE_ROLE = "primaryDisplay";
}

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLColorLegendDisplayNode);

//-----------------------------------------------------------------------------
vtkDMMLColorLegendDisplayNode::vtkDMMLColorLegendDisplayNode()
{
  this->LabelFormat = this->GetDefaultNumericLabelFormat();

  vtkNew<vtkTextProperty> titleTextProperty;
  titleTextProperty->SetBackgroundOpacity(0.0);
  titleTextProperty->SetFontSize(12);
  titleTextProperty->ShadowOn();
  vtkSetAndObserveDMMLObjectMacro(this->TitleTextProperty, titleTextProperty);

  vtkNew<vtkTextProperty> labelTextProperty;
  labelTextProperty->SetBackgroundOpacity(0.0);
  labelTextProperty->SetFontSize(12);
  // Used fixed spaced font family (Courier) so that decimal point in numbers
  // appear at the same position in right-aligned numbers.
  labelTextProperty->SetFontFamilyToCourier();
  labelTextProperty->ShadowOn();
  vtkSetAndObserveDMMLObjectMacro(this->LabelTextProperty, labelTextProperty);

  // Observe primary display node events (like change of scalar range)
  vtkNew<vtkIntArray> primaryDisplayNodeEvents;
  primaryDisplayNodeEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  primaryDisplayNodeEvents->InsertNextValue(vtkDMMLNode::ReferencedNodeModifiedEvent);
  primaryDisplayNodeEvents->InsertNextValue(vtkDMMLNode::ReferenceModifiedEvent);
  this->AddNodeReferenceRole(PRIMARY_DISPLAY_NODE_REFERENCE_ROLE, nullptr, primaryDisplayNodeEvents);

  this->Visibility2DOn();
  this->Visibility3DOn();
}

//---------------------------------------------------------------------------
std::string vtkDMMLColorLegendDisplayNode::GetDefaultNumericLabelFormat()
{
  return "%.1f";
}

//---------------------------------------------------------------------------
std::string vtkDMMLColorLegendDisplayNode::GetDefaultTextLabelFormat()
{
  return "%s";
}

//-----------------------------------------------------------------------------
vtkDMMLColorLegendDisplayNode::~vtkDMMLColorLegendDisplayNode()
{
  this->SetAndObservePrimaryDisplayNode(nullptr);
  vtkSetAndObserveDMMLObjectMacro(this->TitleTextProperty, nullptr);
  vtkSetAndObserveDMMLObjectMacro(this->LabelTextProperty, nullptr);
}

//----------------------------------------------------------------------------
void vtkDMMLColorLegendDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  vtkDMMLPrintBeginMacro(os, indent);
  vtkDMMLPrintEnumMacro(Orientation);
  vtkDMMLPrintVectorMacro(Position, double, 2);
  vtkDMMLPrintVectorMacro(Size, double, 2);
  vtkDMMLPrintStdStringMacro(TitleText);
  vtkDMMLPrintStdStringMacro(TitleTextPropertyAsString);
  vtkDMMLPrintStdStringMacro(LabelTextPropertyAsString);
  vtkDMMLPrintStdStringMacro(LabelFormat);
  vtkDMMLPrintIntMacro(MaxNumberOfColors);
  vtkDMMLPrintIntMacro(NumberOfLabels);
  vtkDMMLPrintBooleanMacro(UseColorNamesForLabels);
  vtkDMMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLColorLegendDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  this->Superclass::WriteXML(of, nIndent);

  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLEnumMacro(orientation, Orientation);
  vtkDMMLWriteXMLVectorMacro(position, Position, double, 2);
  vtkDMMLWriteXMLVectorMacro(size, Size, double, 2);
  vtkDMMLWriteXMLStdStringMacro(titleText, TitleText);
  vtkDMMLWriteXMLStdStringMacro(titleTextProperty, TitleTextPropertyAsString);
  vtkDMMLWriteXMLStdStringMacro(labelTextProperty, LabelTextPropertyAsString);
  vtkDMMLWriteXMLStdStringMacro(labelFormat, LabelFormat);
  vtkDMMLWriteXMLIntMacro(maxNumberOfColors, MaxNumberOfColors);
  vtkDMMLWriteXMLIntMacro(numberOfLabels, NumberOfLabels);
  vtkDMMLWriteXMLBooleanMacro(useColorNamesForLabels, UseColorNamesForLabels);
  vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLColorLegendDisplayNode::ReadXMLAttributes(const char** atts)
{
  // Read all DMML node attributes
  this->Superclass::ReadXMLAttributes(atts);

  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLEnumMacro(orientation, Orientation);
  vtkDMMLReadXMLVectorMacro(position, Position, double, 2);
  vtkDMMLReadXMLVectorMacro(size, Size, double, 2);
  vtkDMMLReadXMLStdStringMacro(titleText, TitleText);
  vtkDMMLReadXMLStdStringMacro(titleTextProperty, TitleTextPropertyFromString);
  vtkDMMLReadXMLStdStringMacro(labelTextProperty, LabelTextPropertyFromString);
  vtkDMMLReadXMLStdStringMacro(labelFormat, LabelFormat);
  vtkDMMLReadXMLIntMacro(maxNumberOfColors, MaxNumberOfColors);
  vtkDMMLReadXMLIntMacro(numberOfLabels, NumberOfLabels);
  vtkDMMLReadXMLBooleanMacro(useColorNamesForLabels, UseColorNamesForLabels);
  vtkDMMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLColorLegendDisplayNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  this->Superclass::CopyContent(anode, deepCopy);

  vtkDMMLColorLegendDisplayNode* node = vtkDMMLColorLegendDisplayNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  vtkDMMLCopyBeginMacro(anode);
  vtkDMMLCopyEnumMacro(Orientation);
  vtkDMMLCopyVectorMacro(Position, double, 2);
  vtkDMMLCopyVectorMacro(Size, double, 2);
  vtkDMMLCopyStdStringMacro(TitleText);
  // The name is misleading, this ShallowCopy method actually creates a deep copy
  this->TitleTextProperty->ShallowCopy(this->SafeDownCast(copySourceNode)->GetTitleTextProperty());
  this->LabelTextProperty->ShallowCopy(this->SafeDownCast(copySourceNode)->GetLabelTextProperty());
  vtkDMMLCopyStdStringMacro(LabelFormat);
  vtkDMMLCopyIntMacro(MaxNumberOfColors);
  vtkDMMLCopyIntMacro(NumberOfLabels);
  vtkDMMLCopyBooleanMacro(UseColorNamesForLabels);
  vtkDMMLCopyEndMacro();
}

//---------------------------------------------------------------------------
void vtkDMMLColorLegendDisplayNode::SetOrientation(int id)
{
  switch (id)
    {
    case 0:
      this->SetOrientation(vtkDMMLColorLegendDisplayNode::Horizontal);
      break;
    case 1:
    default:
      this->SetOrientation(vtkDMMLColorLegendDisplayNode::Vertical);
      break;
    }
}

//---------------------------------------------------------------------------
const char* vtkDMMLColorLegendDisplayNode::GetOrientationAsString(int id)
{
  switch (id)
    {
    case vtkDMMLColorLegendDisplayNode::Horizontal:
      return "Horizontal";
    case vtkDMMLColorLegendDisplayNode::Vertical:
    default:
      return "Vertical";
    }
}

//---------------------------------------------------------------------------
int vtkDMMLColorLegendDisplayNode::GetOrientationFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int i = 0; i < vtkDMMLColorLegendDisplayNode::Orientation_Last; i++)
    {
    if (std::strcmp(name, vtkDMMLColorLegendDisplayNode::GetOrientationAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // unknown name
  return -1;
}

//----------------------------------------------------------------------------
vtkDMMLDisplayNode* vtkDMMLColorLegendDisplayNode::GetPrimaryDisplayNode()
{
  return vtkDMMLDisplayNode::SafeDownCast(this->GetNodeReference(PRIMARY_DISPLAY_NODE_REFERENCE_ROLE));
}

//----------------------------------------------------------------------------
void vtkDMMLColorLegendDisplayNode::SetAndObservePrimaryDisplayNode(vtkDMMLDisplayNode* node)
{
  if (node && this->Scene != node->GetScene())
    {
    vtkErrorMacro("vtkDMMLColorLegendDisplayNode: Cannot set reference, the referenced and referencing node are not in the same scene");
    return;
    }

  this->SetAndObserveNodeReferenceID(PRIMARY_DISPLAY_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

//----------------------------------------------------------------------------
void vtkDMMLColorLegendDisplayNode::ProcessDMMLEvents(vtkObject *caller, unsigned long eventID, void *callData)
{
  Superclass::ProcessDMMLEvents(caller, eventID, callData);

  // Propagate primary display node and text properties modification events
  if (eventID == vtkCommand::ModifiedEvent
    && (caller == this->TitleTextProperty || caller == this->LabelTextProperty
     || caller == this->GetPrimaryDisplayNode()))
    {
    this->Modified();
    }
}

//---------------------------------------------------------------------------
void vtkDMMLColorLegendDisplayNode::SetTitleTextPropertyFromString(std::string textPropertyString)
{
  if (textPropertyString.empty())
    {
    vtkErrorMacro("SetTitleTextPropertyFromString: Invalid text property string");
    return;
    }

  std::string currentTextPropertyString = vtkDMMLDisplayNode::GetTextPropertyAsString(this->TitleTextProperty);
  if (textPropertyString == currentTextPropertyString)
    {
    return;
    }

  DMMLNodeModifyBlocker blocker(this);
  this->UpdateTextPropertyFromString(textPropertyString, this->TitleTextProperty);
  this->Modified();
}

//---------------------------------------------------------------------------
std::string vtkDMMLColorLegendDisplayNode::GetTitleTextPropertyAsString()
{
  return vtkDMMLDisplayNode::GetTextPropertyAsString(this->TitleTextProperty);
}

//---------------------------------------------------------------------------
void vtkDMMLColorLegendDisplayNode::SetLabelTextPropertyFromString(std::string textPropertyString)
{
  if (textPropertyString.empty())
    {
    vtkErrorMacro("SetLabelTextPropertyFromString: Invalid text property string");
    return;
    }

  std::string currentTextPropertyString = vtkDMMLDisplayNode::GetTextPropertyAsString(this->LabelTextProperty);
  if (textPropertyString == currentTextPropertyString)
    {
    return;
    }

  DMMLNodeModifyBlocker blocker(this);
  this->UpdateTextPropertyFromString(textPropertyString, this->LabelTextProperty);
  this->Modified();
}

//---------------------------------------------------------------------------
std::string vtkDMMLColorLegendDisplayNode::GetLabelTextPropertyAsString()
{
  return vtkDMMLDisplayNode::GetTextPropertyAsString(this->LabelTextProperty);
}
