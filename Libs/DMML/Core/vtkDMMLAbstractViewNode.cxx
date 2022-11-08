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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// VTK includes
#include <vtkStringArray.h>

// DMML includes
#include "vtkDMMLAbstractViewNode.h"
#include "vtkDMMLInteractionNode.h"
#include "vtkDMMLLayoutNode.h"
#include "vtkDMMLModelNode.h"
#include "vtkDMMLScene.h"

// STD includes
#include <sstream>

const char* vtkDMMLAbstractViewNode::OrientationMarkerHumanModelReferenceRole = "OrientationMarkerHumanModel";
const char* vtkDMMLAbstractViewNode::ParentLayoutNodeReferenceRole = "ParentLayoutNodeRef";
const char* vtkDMMLAbstractViewNode::InteractionNodeReferenceRole = "InteractionNodeRef";
const int vtkDMMLAbstractViewNode::AxisLabelsCount = 6;
static const char* DEFAULT_AXIS_LABELS[vtkDMMLAbstractViewNode::AxisLabelsCount] = {"L", "R", "P", "A", "I", "S"};

//----------------------------------------------------------------------------
vtkDMMLAbstractViewNode::vtkDMMLAbstractViewNode()
{
  this->BackgroundColor[0] = 0.0;
  this->BackgroundColor[1] = 0.0;
  this->BackgroundColor[2] = 0.0;
  this->BackgroundColor2[0] = 0.0;
  this->BackgroundColor2[1] = 0.0;
  this->BackgroundColor2[2] = 0.0;

  this->LayoutColor[0] = vtkDMMLAbstractViewNode::GetThreeDViewBlueColor()[0];
  this->LayoutColor[1] = vtkDMMLAbstractViewNode::GetThreeDViewBlueColor()[1];
  this->LayoutColor[2] = vtkDMMLAbstractViewNode::GetThreeDViewBlueColor()[2];

  this->SetLayoutLabel("1");
  this->SetHideFromEditors(0);

  this->AxisLabels = vtkSmartPointer<vtkStringArray>::New();
  for (int i=0; i<vtkDMMLAbstractViewNode::AxisLabelsCount; i++)
    {
    this->AxisLabels->InsertNextValue(DEFAULT_AXIS_LABELS[i]);
    }
 }

//----------------------------------------------------------------------------
vtkDMMLAbstractViewNode::~vtkDMMLAbstractViewNode()
{
  this->SetLayoutLabel(nullptr);
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractViewNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  this->Superclass::WriteXML(of, nIndent);

  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLStringMacro(layoutLabel, LayoutLabel);
  vtkDMMLWriteXMLStringMacro(layoutName, LayoutName);
  if (this->GetViewGroup() > 0)
    {
    vtkDMMLWriteXMLBooleanMacro(viewGroup, ViewGroup);
    }
  vtkDMMLWriteXMLBooleanMacro(active, Active);
  vtkDMMLWriteXMLBooleanMacro(visibility, Visibility);
  vtkDMMLWriteXMLVectorMacro(backgroundColor, BackgroundColor, double, 3);
  vtkDMMLWriteXMLVectorMacro(backgroundColor2, BackgroundColor2, double, 3);
  vtkDMMLWriteXMLVectorMacro(layoutColor, LayoutColor, double, 3);
  if (this->OrientationMarkerEnabled)
    {
    vtkDMMLWriteXMLEnumMacro(orientationMarkerType, OrientationMarkerType);
    vtkDMMLWriteXMLEnumMacro(orientationMarkerSize, OrientationMarkerSize);
    }
  if (this->RulerEnabled)
    {
    vtkDMMLWriteXMLEnumMacro(rulerType, RulerType);
    }
  vtkDMMLWriteXMLEnumMacro(rulerColor, RulerColor);
  vtkDMMLWriteXMLEndMacro();

  of << " AxisLabels=\"";
  for (int i=0; i<vtkDMMLAbstractViewNode::AxisLabelsCount; i++)
    {
    of << (i>0?";":"") << this->GetAxisLabel(i);
    }
  of << "\"";
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractViewNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  this->Superclass::ReadXMLAttributes(atts);

  const int backGroundColorInvalid = -1;
  this->BackgroundColor2[0] = backGroundColorInvalid;

  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLStringMacro(layoutLabel, LayoutLabel);
  vtkDMMLReadXMLStringMacro(layoutName, LayoutName);
  vtkDMMLReadXMLBooleanMacro(viewGroup, ViewGroup);
  vtkDMMLReadXMLBooleanMacro(active, Active);

  // XXX Do not read 'visibility' attribute and default to 1 because:
  // (1) commit r21034 (STYLE: Add abstract class for all view nodes)
  // changed the default value for 'visibility' attribute from 1 to 0. This
  // means there are a lot of already saved scene where visibility attribute
  // value is saved as 0.
  // (2) support for visibility attribute by the layout manager has been
  // added.
  // XXX Support for 'visibility' attribute could be restored by updating
  // the dmml version. Scene with a newer version number would consider the
  // serialized attribute whereas older scene would not.
  //
  // vtkDMMLReadXMLBooleanMacro(visibility, Visibility)

  vtkDMMLReadXMLVectorMacro(backgroundColor, BackgroundColor, double, 3);
  vtkDMMLReadXMLVectorMacro(backgroundColor2, BackgroundColor2, double, 3);
  vtkDMMLReadXMLVectorMacro(layoutColor, LayoutColor, double, 3);
  vtkDMMLReadXMLEnumMacro(orientationMarkerType, OrientationMarkerType);
  vtkDMMLReadXMLEnumMacro(orientationMarkerSize, OrientationMarkerSize);
  vtkDMMLReadXMLEnumMacro(rulerType, RulerType);
  vtkDMMLReadXMLEnumMacro(rulerColor, RulerColor);
  vtkDMMLReadXMLEndMacro();

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "AxisLabels"))
      {
      std::stringstream labels(attValue);
      std::string label;
      int labelIndex = 0;
      while (std::getline(labels, label, ';') && labelIndex<vtkDMMLAbstractViewNode::AxisLabelsCount)
        {
        this->SetAxisLabel(labelIndex, label.c_str());
        labelIndex++;
        }
      // If not all labels were defined set the missing ones to empty
      // to make sure all labels are consistently set.
      for (; labelIndex<vtkDMMLAbstractViewNode::AxisLabelsCount; labelIndex++)
        {
        this->SetAxisLabel(labelIndex, "");
        }
      }
    }
#if DMML_APPLICATION_SUPPORT_VERSION < DMML_VERSION_CHECK(4, 0, 0)
  if (this->BackgroundColor2[0] == backGroundColorInvalid)
    {
    // BackgroundColor2 has not been set
    this->BackgroundColor2[0] = this->BackgroundColor[0];
    this->BackgroundColor2[1] = this->BackgroundColor[1];
    this->BackgroundColor2[2] = this->BackgroundColor[2];
    }
#endif
#if DMML_APPLICATION_SUPPORT_VERSION < DMML_VERSION_CHECK(4, 2, 0)
  // vtkDMMLAbstractViewNodes where not singletons before 4.2
  if (!this->GetLayoutName() || strlen(this->GetLayoutName()))
    {
    const char* layoutName = nullptr;
    if (this->GetID() &&
        strncmp(this->GetID(), this->GetClassName(), strlen(this->GetClassName())) == 0)
      {
      layoutName = this->GetID() + strlen(this->GetClassName());
      }
    if (!layoutName || strlen(layoutName) == 0)
      {
      layoutName = "1";
      }
    this->SetLayoutName(layoutName);
    }
#endif

  // Do not restore MappedInLayout state, because the view may not be mapped into the layout just yet.
  // (the attribute tells that it was mapped into the layout when the scene was saved but the current
  // layout may be different, see issue #6284).
  this->SetAttribute("MappedInLayout", nullptr);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractViewNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkDMMLCopyBeginMacro(anode);
  vtkDMMLCopyStringMacro(LayoutLabel);
  vtkDMMLCopyIntMacro(ViewGroup);
  vtkDMMLCopyIntMacro(Active);
  vtkDMMLCopyIntMacro(Visibility);
  vtkDMMLCopyVectorMacro(BackgroundColor, double, 3);
  vtkDMMLCopyVectorMacro(BackgroundColor2, double, 3);
  vtkDMMLCopyVectorMacro(LayoutColor, double, 3);
  if (this->OrientationMarkerEnabled)
    {
    vtkDMMLCopyEnumMacro(OrientationMarkerType);
    vtkDMMLCopyEnumMacro(OrientationMarkerSize);
    }
  if (this->RulerEnabled)
    {
    vtkDMMLCopyEnumMacro(RulerType);
    }
  vtkDMMLCopyEnumMacro(RulerColor);
  vtkDMMLCopyEndMacro();

  vtkDMMLAbstractViewNode *node = vtkDMMLAbstractViewNode::SafeDownCast(anode);
  for (int i=0; i<vtkDMMLAbstractViewNode::AxisLabelsCount; i++)
    {
    this->SetAxisLabel(i,node->GetAxisLabel(i));
    }
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractViewNode::Reset(vtkDMMLNode* defaultNode)
{
  // The LayoutName is preserved by vtkDMMLNode::Reset, however the layout
  // label (typically associated with the layoutName) is not preserved
  // automatically.
  // This require a custom behavior implemented here.
  std::string layoutLabel = this->GetLayoutLabel() ? this->GetLayoutLabel() : "";
  int viewGroup = this->GetViewGroup();
  this->Superclass::Reset(defaultNode);
  this->DisableModifiedEventOn();
  this->SetLayoutLabel(layoutLabel.c_str());
  this->SetViewGroup(viewGroup);
  this->DisableModifiedEventOff();
}

//----------------------------------------------------------------------------
void vtkDMMLAbstractViewNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkDMMLPrintBeginMacro(os, indent);
  vtkDMMLPrintStringMacro(LayoutLabel);
  vtkDMMLPrintIntMacro(ViewGroup);
  vtkDMMLPrintIntMacro(Active);
  vtkDMMLPrintIntMacro(Visibility);
  vtkDMMLPrintVectorMacro(BackgroundColor, double, 3);
  vtkDMMLPrintVectorMacro(BackgroundColor2, double, 3);
  vtkDMMLPrintVectorMacro(LayoutColor, double, 3);
  if (this->OrientationMarkerEnabled)
    {
    vtkDMMLPrintEnumMacro(OrientationMarkerType);
    vtkDMMLPrintEnumMacro(OrientationMarkerSize);
    }
  if (this->RulerEnabled)
    {
    vtkDMMLPrintEnumMacro(RulerType);
    }
  vtkDMMLPrintEnumMacro(RulerColor);
  vtkDMMLPrintEndMacro();

  os << indent << " AxisLabels: ";
  for (int i=0; i<vtkDMMLAbstractViewNode::AxisLabelsCount; i++)
    {
    os << (i>0?";":"") << this->GetAxisLabel(i);
    }
  os << "\n";

}

//------------------------------------------------------------------------------
vtkDMMLInteractionNode* vtkDMMLAbstractViewNode::GetInteractionNode()
{
  vtkDMMLInteractionNode * interactionNode =
      vtkDMMLInteractionNode::SafeDownCast(this->GetNodeReference(this->InteractionNodeReferenceRole));
  if (this->GetScene() && !interactionNode)
    {
    interactionNode = vtkDMMLInteractionNode::SafeDownCast (
          this->GetScene()->GetNodeByID("vtkDMMLInteractionNodeSingleton"));
    }
  return interactionNode;
}

//------------------------------------------------------------------------------
bool vtkDMMLAbstractViewNode::SetInteractionNodeID(const char *interactionNodeId)
{
  if (!interactionNodeId)
    {
    return false;
    }
  this->SetNodeReferenceID(this->InteractionNodeReferenceRole, interactionNodeId);
  return true;
}

//------------------------------------------------------------------------------
bool vtkDMMLAbstractViewNode::SetInteractionNode(vtkDMMLNode* node)
{
  if (node && this->Scene != node->GetScene())
    {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return false;
    }
  return this->SetInteractionNodeID(node ? node->GetID() : nullptr);
}

int vtkDMMLAbstractViewNode::IsMappedInLayout()
{
  if (!this->GetAttribute("MappedInLayout"))
    {
    return 0;
    }
  return strcmp(this->GetAttribute("MappedInLayout"), "1") == 0;
}

//------------------------------------------------------------------------------
void vtkDMMLAbstractViewNode::SetMappedInLayout(int value)
{
  if (this->IsMappedInLayout() == value)
    {
    return;
    }
  this->SetAttribute("MappedInLayout", value ? "1" : "0");
}

//------------------------------------------------------------------------------
bool vtkDMMLAbstractViewNode::IsViewVisibleInLayout()
{
  return (this->IsMappedInLayout() && this->GetVisibility());
}

//-----------------------------------------------------------
const char* vtkDMMLAbstractViewNode::GetOrientationMarkerTypeAsString(int id)
{
  switch (id)
    {
    case OrientationMarkerTypeNone: return "none";
    case OrientationMarkerTypeCube: return "cube";
    case OrientationMarkerTypeHuman: return "human";
    case OrientationMarkerTypeAxes: return "axes";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkDMMLAbstractViewNode::GetOrientationMarkerTypeFromString(const char* name)
{
  if (name == nullptr)
  {
    // invalid name
    return -1;
  }
  for (int i=0; i<OrientationMarkerType_Last; i++)
    {
    if (strcmp(name, GetOrientationMarkerTypeAsString(i))==0)
      {
      // found a matching name
      return i;
      }
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
const char* vtkDMMLAbstractViewNode::GetOrientationMarkerSizeAsString(int id)
{
  switch (id)
    {
    case OrientationMarkerSizeSmall: return "small";
    case OrientationMarkerSizeMedium: return "medium";
    case OrientationMarkerSizeLarge: return "large";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkDMMLAbstractViewNode::GetOrientationMarkerSizeFromString(const char* name)
{
  if (name == nullptr)
  {
    // invalid name
    return -1;
  }
  for (int i=0; i<OrientationMarkerSize_Last; i++)
    {
    if (strcmp(name, GetOrientationMarkerSizeAsString(i))==0)
      {
      // found a matching name
      return i;
      }
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
const char* vtkDMMLAbstractViewNode::GetRulerTypeAsString(int id)
{
  switch (id)
    {
    case RulerTypeNone: return "none";
    case RulerTypeThin: return "thin";
    case RulerTypeThick: return "thick";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkDMMLAbstractViewNode::GetRulerTypeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int i=0; i<RulerType_Last; i++)
    {
    if (strcmp(name, GetRulerTypeAsString(i))==0)
      {
      // found a matching name
      return i;
      }
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
const char* vtkDMMLAbstractViewNode::GetRulerColorAsString(int id)
{
  switch (id)
    {
    case RulerColorWhite: return "white";
    case RulerColorBlack: return "black";
    case RulerColorYellow: return "yellow";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkDMMLAbstractViewNode::GetRulerColorFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int i=0; i<RulerColor_Last; i++)
    {
    if (strcmp(name, GetRulerColorAsString(i))==0)
      {
      // found a matching name
      return i;
      }
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
void vtkDMMLAbstractViewNode::SetOrientationMarkerHumanModelNodeID(const char* modelNodeId)
{
  if (!this->OrientationMarkerEnabled)
    {
    vtkErrorMacro("vtkDMMLAbstractViewNode::SetOrientationMarkerHumanModelID failed: orientation marker is disabled");
    return;
    }
  this->SetNodeReferenceID(OrientationMarkerHumanModelReferenceRole, modelNodeId);
}

//-----------------------------------------------------------
const char* vtkDMMLAbstractViewNode::GetOrientationMarkerHumanModelNodeID()
{
  if (!this->OrientationMarkerEnabled)
    {
    vtkErrorMacro("vtkDMMLAbstractViewNode::GetOrientationMarkerHumanModelID failed: orientation marker is disabled");
    return nullptr;
    }
  return this->GetNodeReferenceID(OrientationMarkerHumanModelReferenceRole);
}

//-----------------------------------------------------------
vtkDMMLModelNode* vtkDMMLAbstractViewNode::GetOrientationMarkerHumanModelNode()
{
  if (!this->OrientationMarkerEnabled)
    {
    vtkErrorMacro("vtkDMMLAbstractViewNode::GetOrientationMarkerHumanModel failed: orientation marker is disabled");
    return nullptr;
    }
  return vtkDMMLModelNode::SafeDownCast(this->GetNodeReference(OrientationMarkerHumanModelReferenceRole));
}

//-----------------------------------------------------------
const char* vtkDMMLAbstractViewNode::GetAxisLabel(int labelIndex)
{
  if (labelIndex<0 || labelIndex>=vtkDMMLAbstractViewNode::AxisLabelsCount)
    {
    vtkErrorMacro("vtkDMMLAbstractViewNode::GetAxisLabel labelIndex=" << labelIndex << " argument is invalid. Valid range: 0<=labelIndex<"
      << vtkDMMLAbstractViewNode::AxisLabelsCount);
    return "";
    }
  return this->AxisLabels->GetValue(labelIndex);
}

//-----------------------------------------------------------
void vtkDMMLAbstractViewNode::SetAxisLabel(int labelIndex, const char* label)
{
  if (labelIndex<0 || labelIndex>=vtkDMMLAbstractViewNode::AxisLabelsCount)
    {
    vtkErrorMacro("vtkDMMLAbstractViewNode::SetAxisLabel labelIndex=" << labelIndex << " argument is invalid. Valid range: 0<=labelIndex<"
      << vtkDMMLAbstractViewNode::AxisLabelsCount);
    return;
    }
  if (label==nullptr)
    {
    label = "";
    }
  if (this->AxisLabels->GetValue(labelIndex).compare(label)==0)
    {
    // no change
    return;
    }
  this->AxisLabels->SetValue(labelIndex, label);
  this->Modified();
}

//----------------------------------------------------------------------------
vtkDMMLNode* vtkDMMLAbstractViewNode::GetParentLayoutNode()
{
  return this->GetNodeReference(this->ParentLayoutNodeReferenceRole);
}

//----------------------------------------------------------------------------
bool vtkDMMLAbstractViewNode::SetAndObserveParentLayoutNodeID(const char *layoutNodeId)
{
  if (!layoutNodeId)
    {
    return false;
    }

  this->SetAndObserveNodeReferenceID(this->ParentLayoutNodeReferenceRole, layoutNodeId);
  return true;
}

//----------------------------------------------------------------------------
bool vtkDMMLAbstractViewNode::SetAndObserveParentLayoutNode(vtkDMMLNode* node)
{
  if (node && this->Scene != node->GetScene())
    {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return false;
    }

  return this->SetAndObserveParentLayoutNodeID(node ? node->GetID() : nullptr);
}

//----------------------------------------------------------------------------
double* vtkDMMLAbstractViewNode::GetRedColor()
{
  // #F34A33
  static double redColor[3] = {243. / 255.,
                                74. / 255.,
                                51. / 255.};
  return redColor;
}

//----------------------------------------------------------------------------
double* vtkDMMLAbstractViewNode::GetYellowColor()
{
  // #EDD54C
  static double yellowColor[3] = {237. / 255.,
                                  213. / 255.,
                                   76. / 255.};
  return yellowColor;
}

//----------------------------------------------------------------------------
double* vtkDMMLAbstractViewNode::GetGreenColor()
{
  // #6EB04B
  static double greenColor[3] = {110. / 255.,
                                 176. / 255.,
                                  75. / 255.};
  return greenColor;
}

//----------------------------------------------------------------------------
double* vtkDMMLAbstractViewNode::GetCompareColor()
{
  // #E17012
  static double compareColor[3] = {225. / 255.,
                                   112. / 255.,
                                    18. / 255.};
  return compareColor;
}

//----------------------------------------------------------------------------
double* vtkDMMLAbstractViewNode::GetGrayColor()
{
  // #8C8C8C
  static double grayColor[3] = {140. / 255.,
                                140. / 255.,
                                140. / 255.};
  return grayColor;
}

//------------------------------------------------------------------------------
double* vtkDMMLAbstractViewNode::GetThreeDViewBlueColor()
{
  // #7483E9
  static double blueColor[3] = {116. / 255.,
                                131. / 255.,
                                233. / 255.};
  return blueColor;
}

//------------------------------------------------------------------------------
vtkDMMLLayoutNode* vtkDMMLAbstractViewNode::GetMaximizedState(bool& maximized, bool& canBeMaximized)
{
  canBeMaximized = true;
  maximized = false;
  vtkDMMLLayoutNode* layoutNode = nullptr;
  if (this->GetParentLayoutNode())
    {
    layoutNode = vtkDMMLLayoutNode::SafeDownCast(this->GetParentLayoutNode());
    if (!layoutNode)
      {
      // the owner is not a real layout node, it means it is a standalone view, cannot be maximized
      canBeMaximized = false;
      }
    }
  if (!layoutNode && this->GetScene())
    {
   layoutNode = vtkDMMLLayoutNode::SafeDownCast(this->GetScene()->GetFirstNodeByClass("vtkDMMLLayoutNode"));
    }
  if (layoutNode)
    {
    maximized = (layoutNode->GetMaximizedViewNode() == this);
    }
  return layoutNode;
}
