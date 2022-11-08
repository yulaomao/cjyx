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

// DMML includes
#include <vtkDMMLAbstractViewNode.h>
#include <vtkDMMLInteractionEventData.h>
#include <vtkDMMLMarkupsDisplayNode.h>
#include <vtkDMMLProceduralColorNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkCommand.h>
#include <vtkDiscretizableColorTransferFunction.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPointData.h>
#include <vtkTextProperty.h>
#include <vtksys/SystemTools.hxx>

// STL includes
#include <sstream>

const char* vtkDMMLMarkupsDisplayNode::LineColorNodeReferenceRole = "lineColor";
const char* vtkDMMLMarkupsDisplayNode::LineColorNodeReferenceDMMLAttributeName = "lineColorNodeRef";

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLMarkupsDisplayNode);

//----------------------------------------------------------------------------
vtkDMMLMarkupsDisplayNode::vtkDMMLMarkupsDisplayNode()
{
  // Markups display node settings
  this->Visibility = 1;
  this->Visibility2D = 1;
  this->VectorVisibility = 0;
  this->ScalarVisibility = 0;
  this->TensorVisibility = 0;

  this->Color[0] = 0.4;
  this->Color[1] = 1.0;
  this->Color[2] = 1.0;

  this->SelectedColor[0] = 1.0;
  this->SelectedColor[1] = 0.5;
  this->SelectedColor[2] = 0.5;

  this->SetName("");
  this->Opacity = 1.0;
  this->Ambient = 0;
  this->Diffuse = 1.0;
  this->Specular = 0;
  this->Power = 1;

  // markup display node settings
  this->TextScale = 3;
  this->GlyphType = vtkDMMLMarkupsDisplayNode::Sphere3D;
  this->GlyphScale = 3.0; // relative to screen size
  this->GlyphSize = 5.0;  // size in world coordinate system (mm)
  this->UseGlyphScale = true; // relative size by default

  this->SnapMode = vtkDMMLMarkupsDisplayNode::SnapModeToVisibleSurface;

  // projection settings
  this->SliceProjection = false;
  this->SliceProjectionUseFiducialColor = true;
  this->SliceProjectionOutlinedBehindSlicePlane = false;
  this->SliceProjectionColor[0] = 1.0;
  this->SliceProjectionColor[1] = 1.0;
  this->SliceProjectionColor[2] = 1.0;
  this->SliceProjectionOpacity = 0.6;

  this->PropertiesLabelVisibility = true;
  this->PointLabelsVisibility = false;
  this->FillVisibility = true;
  this->OutlineVisibility = true;
  this->FillOpacity = 0.5;
  this->OutlineOpacity = 1.0;

  // Set active component defaults for mouse (identified by empty string)
  this->ActiveComponents[GetDefaultContextName()] = ComponentInfo();

  this->CurveLineSizeMode = vtkDMMLMarkupsDisplayNode::UseLineThickness;
  this->LineThickness = 0.2;
  this->LineDiameter = 1.0;

  // Line color variables
  this->LineColorFadingStart = 1.;
  this->LineColorFadingEnd = 10.;
  this->LineColorFadingSaturation = 1.;
  this->LineColorFadingHueOffset = 0.;

  this->OccludedVisibility = false;
  this->OccludedOpacity = 0.3;

  // Text apperarance
  this->TextProperty = nullptr;
  vtkNew<vtkTextProperty> textProperty;
  textProperty->SetBackgroundOpacity(0.0);
  textProperty->SetFontSize(5);
  textProperty->BoldOn();
  textProperty->ShadowOn();
  vtkSetAndObserveDMMLObjectMacro(this->TextProperty, textProperty);

  this->ActiveColor[0] = 0.4; // bright green
  this->ActiveColor[1] = 1.0;
  this->ActiveColor[2] = 0.0;

  this->HandlesInteractive = false;
  this->TranslationHandleVisibility = true;
  this->RotationHandleVisibility = true;
  this->ScaleHandleVisibility = true;
  this->InteractionHandleScale = 3.0; // size of the handles as percent in screen size

  // By default, all interaction handle axes are visible
  for (int i = 0; i < 4; ++i)
    {
    this->RotationHandleComponentVisibility[i] = true;
    this->ScaleHandleComponentVisibility[i] = true;
    this->TranslationHandleComponentVisibility[i] = true;
    }

  this->CanDisplayScaleHandles = false;

  // Line color node
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkCommand::ModifiedEvent);

  this->AddNodeReferenceRole(this->GetLineColorNodeReferenceRole(),
                             this->GetLineColorNodeReferenceDMMLAttributeName(),
                             events.GetPointer());
}

//----------------------------------------------------------------------------
vtkDMMLMarkupsDisplayNode::~vtkDMMLMarkupsDisplayNode()
{
  vtkSetAndObserveDMMLObjectMacro(this->TextProperty, nullptr);
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLBooleanMacro(propertiesLabelVisibility, PropertiesLabelVisibility);
  vtkDMMLWriteXMLBooleanMacro(pointLabelsVisibility, PointLabelsVisibility);
  vtkDMMLWriteXMLFloatMacro(textScale, TextScale);
  vtkDMMLWriteXMLFloatMacro(glyphScale, GlyphScale);
  vtkDMMLWriteXMLFloatMacro(glyphSize, GlyphSize);
  vtkDMMLWriteXMLBooleanMacro(useGlyphScale, UseGlyphScale);
  vtkDMMLWriteXMLEnumMacro(glyphType, GlyphType);
  vtkDMMLWriteXMLEnumMacro(snapMode, SnapMode);
  vtkDMMLWriteXMLBooleanMacro(sliceProjection, SliceProjection);
  vtkDMMLWriteXMLBooleanMacro(sliceProjectionUseFiducialColor, SliceProjectionUseFiducialColor);
  vtkDMMLWriteXMLBooleanMacro(sliceProjectionOutlinedBehindSlicePlane, SliceProjectionOutlinedBehindSlicePlane);
  vtkDMMLWriteXMLVectorMacro(sliceProjectionColor, SliceProjectionColor, double, 3);
  vtkDMMLWriteXMLFloatMacro(sliceProjectionOpacity, SliceProjectionOpacity);
  vtkDMMLWriteXMLEnumMacro(curveLineSizeMode, CurveLineSizeMode);
  vtkDMMLWriteXMLFloatMacro(lineThickness, LineThickness);
  vtkDMMLWriteXMLFloatMacro(lineDiameter, LineDiameter);
  vtkDMMLWriteXMLFloatMacro(lineColorFadingStart, LineColorFadingStart);
  vtkDMMLWriteXMLFloatMacro(lineColorFadingEnd, LineColorFadingEnd);
  vtkDMMLWriteXMLFloatMacro(lineColorFadingSaturation, LineColorFadingSaturation);
  vtkDMMLWriteXMLFloatMacro(lineColorFadingHueOffset, LineColorFadingHueOffset);
  vtkDMMLWriteXMLBooleanMacro(handlesInteractive, HandlesInteractive);
  vtkDMMLWriteXMLBooleanMacro(translationHandleVisibility, TranslationHandleVisibility);
  vtkDMMLWriteXMLBooleanMacro(rotationHandleVisibility, RotationHandleVisibility);
  vtkDMMLWriteXMLBooleanMacro(scaleHandleVisibility, ScaleHandleVisibility);
  vtkDMMLWriteXMLFloatMacro(interactionHandleScale, InteractionHandleScale);
  vtkDMMLWriteXMLBooleanMacro(fillVisibility, FillVisibility);
  vtkDMMLWriteXMLBooleanMacro(outlineVisibility, OutlineVisibility);
  vtkDMMLWriteXMLFloatMacro(fillOpacity, FillOpacity);
  vtkDMMLWriteXMLFloatMacro(outlineOpacity, OutlineOpacity);
  vtkDMMLWriteXMLBooleanMacro(occludedVisibility, OccludedVisibility);
  vtkDMMLWriteXMLFloatMacro(occludedOpacity, OccludedOpacity);
  vtkDMMLWriteXMLStdStringMacro(textProperty, TextPropertyAsString);
  vtkDMMLWriteXMLVectorMacro(activeColor, ActiveColor, double, 3);

  // Only write the handle axes properties if any of them are different from the default (all enabled).
  if (!this->TranslationHandleComponentVisibility[0] ||
      !this->TranslationHandleComponentVisibility[1] ||
      !this->TranslationHandleComponentVisibility[2] ||
      !this->TranslationHandleComponentVisibility[3])
    {
    vtkDMMLWriteXMLVectorMacro(translationHandleAxes, TranslationHandleComponentVisibility, bool, 4);
    }
  if (!this->RotationHandleComponentVisibility[0] ||
      !this->RotationHandleComponentVisibility[1] ||
      !this->RotationHandleComponentVisibility[2] ||
      !this->RotationHandleComponentVisibility[3])
    {
    vtkDMMLWriteXMLVectorMacro(rotationHandleAxes, RotationHandleComponentVisibility, bool, 4);
    }
  if (!this->ScaleHandleComponentVisibility[0] ||
      !this->ScaleHandleComponentVisibility[1] ||
      !this->ScaleHandleComponentVisibility[2] ||
      !this->ScaleHandleComponentVisibility[3])
    {
    vtkDMMLWriteXMLVectorMacro(scaleHandleAxes, ScaleHandleComponentVisibility, bool, 4);
    }

  vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayNode::ReadXMLAttributes(const char** atts)
{
  DMMLNodeModifyBlocker blocker(this);

  Superclass::ReadXMLAttributes(atts);

  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLBooleanMacro(propertiesLabelVisibility, PropertiesLabelVisibility);
  vtkDMMLReadXMLBooleanMacro(pointLabelsVisibility, PointLabelsVisibility);
  vtkDMMLReadXMLFloatMacro(textScale, TextScale);
  vtkDMMLReadXMLFloatMacro(glyphScale, GlyphScale);
  vtkDMMLReadXMLFloatMacro(glyphSize, GlyphSize);
  vtkDMMLReadXMLBooleanMacro(useGlyphScale, UseGlyphScale);
  vtkDMMLReadXMLEnumMacro(glyphType, GlyphType);
  vtkDMMLReadXMLEnumMacro(snapMode, SnapMode);
  vtkDMMLReadXMLBooleanMacro(sliceProjection, SliceProjection);
  vtkDMMLReadXMLBooleanMacro(sliceProjectionUseFiducialColor, SliceProjectionUseFiducialColor);
  vtkDMMLReadXMLBooleanMacro(sliceProjectionOutlinedBehindSlicePlane, SliceProjectionOutlinedBehindSlicePlane);
  vtkDMMLReadXMLVectorMacro(sliceProjectionColor, SliceProjectionColor, double, 3);
  vtkDMMLReadXMLFloatMacro(sliceProjectionOpacity, SliceProjectionOpacity);
  vtkDMMLReadXMLEnumMacro(curveLineSizeMode, CurveLineSizeMode);
  vtkDMMLReadXMLFloatMacro(lineThickness, LineThickness);
  vtkDMMLReadXMLFloatMacro(lineDiameter, LineDiameter);
  vtkDMMLReadXMLFloatMacro(lineColorFadingStart, LineColorFadingStart);
  vtkDMMLReadXMLFloatMacro(lineColorFadingEnd, LineColorFadingEnd);
  vtkDMMLReadXMLFloatMacro(lineColorFadingSaturation, LineColorFadingSaturation);
  vtkDMMLReadXMLFloatMacro(lineColorFadingHueOffset, LineColorFadingHueOffset);
  vtkDMMLReadXMLBooleanMacro(handlesInteractive, HandlesInteractive);
  vtkDMMLReadXMLBooleanMacro(translationHandleVisibility, TranslationHandleVisibility);
  vtkDMMLReadXMLBooleanMacro(rotationHandleVisibility, RotationHandleVisibility);
  vtkDMMLReadXMLBooleanMacro(scaleHandleVisibility, ScaleHandleVisibility);
  vtkDMMLReadXMLFloatMacro(interactionHandleScale, InteractionHandleScale);
  vtkDMMLReadXMLBooleanMacro(fillVisibility, FillVisibility);
  vtkDMMLReadXMLBooleanMacro(outlineVisibility, OutlineVisibility);
  vtkDMMLReadXMLFloatMacro(fillOpacity, FillOpacity);
  vtkDMMLReadXMLFloatMacro(outlineOpacity, OutlineOpacity);
  vtkDMMLReadXMLBooleanMacro(occludedVisibility, OccludedVisibility);
  vtkDMMLReadXMLFloatMacro(occludedOpacity, OccludedOpacity);
  vtkDMMLReadXMLStdStringMacro(textProperty, TextPropertyFromString);
  vtkDMMLReadXMLVectorMacro(activeColor, ActiveColor, double, 3);
  vtkDMMLReadXMLVectorMacro(rotationHandleAxes, RotationHandleComponentVisibility, bool, 4);
  vtkDMMLReadXMLVectorMacro(scaleHandleAxes, ScaleHandleComponentVisibility, bool, 4);
  vtkDMMLReadXMLVectorMacro(translationHandleAxes, TranslationHandleComponentVisibility, bool, 4);
  vtkDMMLReadXMLEndMacro();

  // Fix up legacy markups fiducial nodes
  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    // Glyph type used to be saved as an integer (not as a string enum as it is done now),
    // therefore we can use it to detect legacy scenes.
    if (!strcmp(attName, "glyphType"))
      {
      std::stringstream ss;
      int val = 0;
      ss << attValue;
      ss >> val;
      if (val > 0)
        {
        // Se glyph type from integer
        this->SetGlyphType(val);
        // Point label visibility attribute was not present in legacy scenes,
        // therefore we need to set it here.
        this->SetPointLabelsVisibility(true);
        }
      }
    else if (!strcmp(attName, "sliceIntersectionVisibility"))
      {
      // Presence of this attribute means that this is an old scene (created with Cjyx version before September 2019).
      // In these Cjyx versions, markups were always displayed in slice view (regardless of
      // sliceIntersectionVisibility value), but the vtkDMMLDisplayNode base class sets 2D visibility
      // based on sliceIntersectionVisibility value. This would result markups fiducial points being
      // hidden in 2D views by default. To prevent this, we restore visibility now.
      this->SetVisibility2D(true);
      }
    }
}


//----------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkDMMLCopyBeginMacro(anode);
  vtkDMMLCopyBooleanMacro(PropertiesLabelVisibility);
  vtkDMMLCopyBooleanMacro(PointLabelsVisibility);
  vtkDMMLCopyFloatMacro(TextScale);
  vtkDMMLCopyFloatMacro(GlyphScale);
  vtkDMMLCopyFloatMacro(GlyphSize);
  vtkDMMLCopyBooleanMacro(UseGlyphScale);
  vtkDMMLCopyEnumMacro(GlyphType);
  vtkDMMLCopyEnumMacro(SnapMode);
  vtkDMMLCopyBooleanMacro(SliceProjection);
  vtkDMMLCopyBooleanMacro(SliceProjectionUseFiducialColor);
  vtkDMMLCopyBooleanMacro(SliceProjectionOutlinedBehindSlicePlane);
  vtkDMMLCopyVectorMacro(SliceProjectionColor, double, 3);
  vtkDMMLCopyFloatMacro(SliceProjectionOpacity);
  vtkDMMLCopyEnumMacro(CurveLineSizeMode);
  vtkDMMLCopyFloatMacro(LineThickness);
  vtkDMMLCopyFloatMacro(LineDiameter);
  vtkDMMLCopyFloatMacro(LineColorFadingStart);
  vtkDMMLCopyFloatMacro(LineColorFadingEnd);
  vtkDMMLCopyFloatMacro(LineColorFadingSaturation);
  vtkDMMLCopyFloatMacro(LineColorFadingHueOffset);
  vtkDMMLCopyBooleanMacro(HandlesInteractive);
  vtkDMMLCopyBooleanMacro(TranslationHandleVisibility);
  vtkDMMLCopyBooleanMacro(RotationHandleVisibility);
  vtkDMMLCopyBooleanMacro(ScaleHandleVisibility);
  vtkDMMLCopyFloatMacro(InteractionHandleScale);
  vtkDMMLCopyBooleanMacro(FillVisibility);
  vtkDMMLCopyBooleanMacro(OutlineVisibility);
  vtkDMMLCopyFloatMacro(FillOpacity);
  vtkDMMLCopyFloatMacro(OutlineOpacity);
  vtkDMMLCopyBooleanMacro(OccludedVisibility);
  vtkDMMLCopyFloatMacro(OccludedOpacity);
  // The name is misleading, this ShallowCopy method actually creates a deep copy
  this->TextProperty->ShallowCopy(this->SafeDownCast(copySourceNode)->GetTextProperty());
  vtkDMMLCopyVectorMacro(ActiveColor, double, 3);
  vtkDMMLCopyVectorMacro(RotationHandleComponentVisibility, bool, 4);
  vtkDMMLCopyVectorMacro(ScaleHandleComponentVisibility, bool, 4);
  vtkDMMLCopyVectorMacro(TranslationHandleComponentVisibility, bool, 4);
  vtkDMMLCopyEndMacro();
}


//----------------------------------------------------------------------------
const char* vtkDMMLMarkupsDisplayNode::GetGlyphTypeAsString()
{
  return vtkDMMLMarkupsDisplayNode::GetGlyphTypeAsString(this->GlyphType);
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayNode::SetGlyphTypeFromString(const char *glyphString)
{
  this->SetGlyphType(vtkDMMLMarkupsDisplayNode::GetGlyphTypeFromString(glyphString));
}

//-----------------------------------------------------------
int vtkDMMLMarkupsDisplayNode::GetGlyphTypeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return 0;
    }
  for (int ii = 0; ii < GlyphType_Last; ii++)
    {
    if (strcmp(name, GetGlyphTypeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return GlyphTypeInvalid;
}

//---------------------------------------------------------------------------
const char* vtkDMMLMarkupsDisplayNode::GetGlyphTypeAsString(int id)
{
  switch (id)
  {
  case Vertex2D: return "Vertex2D";
  case Dash2D: return "Dash2D";
  case Cross2D: return "Cross2D";
  case CrossDot2D: return "CrossDot2D";
  case ThickCross2D: return "ThickCross2D";
  case Triangle2D: return "Triangle2D";
  case Square2D: return "Square2D";
  case Circle2D: return "Circle2D";
  case Diamond2D: return "Diamond2D";
  case Arrow2D: return "Arrow2D";
  case ThickArrow2D: return "ThickArrow2D";
  case HookedArrow2D: return "HookedArrow2D";
  case StarBurst2D: return "StarBurst2D";
  case Sphere3D: return "Sphere3D";
  case GlyphTypeInvalid:
  default:
    // invalid id
    return "Invalid";
  }
}

//-----------------------------------------------------------
int vtkDMMLMarkupsDisplayNode::GetSnapModeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < SnapMode_Last; ii++)
    {
    if (strcmp(name, GetSnapModeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//---------------------------------------------------------------------------
const char* vtkDMMLMarkupsDisplayNode::GetSnapModeAsString(int id)
{
  switch (id)
  {
  case SnapModeUnconstrained: return "unconstrained";
  case SnapModeToVisibleSurface: return "toVisibleSurface";
  default:
    // invalid id
    return "invalid";
  }
}

//----------------------------------------------------------------------------
const char* vtkDMMLMarkupsDisplayNode::GetCurveLineSizeModeAsString()
{
  return vtkDMMLMarkupsDisplayNode::GetCurveLineSizeModeAsString(this->CurveLineSizeMode);
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayNode::SetCurveLineSizeModeFromString(const char* modeString)
{
  this->SetCurveLineSizeMode(vtkDMMLMarkupsDisplayNode::GetCurveLineSizeModeFromString(modeString));
}

//-----------------------------------------------------------
int vtkDMMLMarkupsDisplayNode::GetCurveLineSizeModeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return 0;
    }
  for (int ii = 0; ii < CurveLineSizeMode_Last; ii++)
    {
    if (strcmp(name, GetCurveLineSizeModeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//---------------------------------------------------------------------------
const char* vtkDMMLMarkupsDisplayNode::GetCurveLineSizeModeAsString(int id)
{
  switch (id)
  {
  case UseLineThickness: return "UseLineThickness";
  case UseLineDiameter: return "UseLineDiameter";
  default:
    // invalid id
    return "Invalid";
  }
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  vtkDMMLPrintBeginMacro(os,indent);
  vtkDMMLPrintBooleanMacro(PropertiesLabelVisibility);
  vtkDMMLPrintBooleanMacro(PointLabelsVisibility);
  vtkDMMLPrintFloatMacro(TextScale);
  vtkDMMLPrintFloatMacro(GlyphScale);
  vtkDMMLPrintFloatMacro(GlyphSize);
  vtkDMMLPrintBooleanMacro(UseGlyphScale);
  vtkDMMLPrintEnumMacro(GlyphType);
  vtkDMMLPrintEnumMacro(SnapMode);
  vtkDMMLPrintBooleanMacro(SliceProjection);
  vtkDMMLPrintBooleanMacro(SliceProjectionUseFiducialColor);
  vtkDMMLPrintBooleanMacro(SliceProjectionOutlinedBehindSlicePlane);
  vtkDMMLPrintVectorMacro(SliceProjectionColor, double, 3);
  vtkDMMLPrintFloatMacro(SliceProjectionOpacity);
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
  vtkDMMLPrintEnumMacro(CurveLineSizeMode);
  vtkDMMLPrintFloatMacro(LineThickness);
  vtkDMMLPrintFloatMacro(LineDiameter);
  vtkDMMLPrintFloatMacro(LineColorFadingStart);
  vtkDMMLPrintFloatMacro(LineColorFadingEnd);
  vtkDMMLPrintFloatMacro(LineColorFadingSaturation);
  vtkDMMLPrintFloatMacro(LineColorFadingHueOffset);
  vtkDMMLPrintBooleanMacro(HandlesInteractive);
  vtkDMMLPrintBooleanMacro(TranslationHandleVisibility);
  vtkDMMLPrintBooleanMacro(RotationHandleVisibility);
  vtkDMMLPrintBooleanMacro(ScaleHandleVisibility);
  vtkDMMLPrintFloatMacro(InteractionHandleScale);
  vtkDMMLPrintBooleanMacro(FillVisibility);
  vtkDMMLPrintBooleanMacro(OutlineVisibility);
  vtkDMMLPrintFloatMacro(FillOpacity);
  vtkDMMLPrintFloatMacro(OutlineOpacity);
  vtkDMMLPrintBooleanMacro(OccludedVisibility);
  vtkDMMLPrintFloatMacro(OccludedOpacity);
  vtkDMMLPrintStdStringMacro(TextPropertyAsString);
  vtkDMMLPrintVectorMacro(ActiveColor, double, 3);
  vtkDMMLPrintVectorMacro(RotationHandleComponentVisibility, bool, 4);
  vtkDMMLPrintVectorMacro(ScaleHandleComponentVisibility, bool, 4);
  vtkDMMLPrintVectorMacro(TranslationHandleComponentVisibility, bool, 4);
  vtkDMMLPrintEndMacro();
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayNode::ProcessDMMLEvents(vtkObject *caller,
                                                  unsigned long event,
                                                  void *callData)
{
  Superclass::ProcessDMMLEvents(caller, event, callData);
  if (caller == this->TextProperty)
    {
    switch (event)
      {
      case vtkCommand::ModifiedEvent:
        this->Modified();
      default:
        break;
      }
    }
  return;
}

//-----------------------------------------------------------
void vtkDMMLMarkupsDisplayNode::UpdateScene(vtkDMMLScene *scene)
{
   Superclass::UpdateScene(scene);
}

//---------------------------------------------------------------------------
int  vtkDMMLMarkupsDisplayNode::GlyphTypeIs3D(int glyphType)
{
  if (glyphType == vtkDMMLMarkupsDisplayNode::Sphere3D)
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayNode::SetLineColorNodeID(const char *lineColorNodeID)
{
  this->SetNodeReferenceID(this->GetLineColorNodeReferenceRole(), lineColorNodeID);
}

//---------------------------------------------------------------------------
const char *vtkDMMLMarkupsDisplayNode::GetLineColorNodeID()
{
  return this->GetNodeReferenceID(this->GetLineColorNodeReferenceRole());
}

//---------------------------------------------------------------------------
vtkDMMLProceduralColorNode *vtkDMMLMarkupsDisplayNode::GetLineColorNode()
{
  return vtkDMMLProceduralColorNode::SafeDownCast(this->GetNodeReference(this->GetLineColorNodeReferenceRole()));
}

//---------------------------------------------------------------------------
const char *vtkDMMLMarkupsDisplayNode::GetLineColorNodeReferenceRole()
{
  return vtkDMMLMarkupsDisplayNode::LineColorNodeReferenceRole;
}

//----------------------------------------------------------------------------
const char *vtkDMMLMarkupsDisplayNode::GetLineColorNodeReferenceDMMLAttributeName()
{
  return vtkDMMLMarkupsDisplayNode::LineColorNodeReferenceDMMLAttributeName;
}

//---------------------------------------------------------------------------
int vtkDMMLMarkupsDisplayNode::GetActiveComponentType(std::string context/*=GetDefaultContextName()*/)
{
  if (this->ActiveComponents.find(context) == this->ActiveComponents.end())
    {
    vtkErrorMacro("GetActiveComponentType: No interaction context with identifier '" << context << "' was found");
    return ComponentNone;
    }

  return this->ActiveComponents[context].Type;
}

//---------------------------------------------------------------------------
int vtkDMMLMarkupsDisplayNode::GetActiveComponentIndex(std::string context/*=GetDefaultContextName()*/)
{
  if (this->ActiveComponents.find(context) == this->ActiveComponents.end())
    {
    vtkErrorMacro("GetActiveComponentIndex: No interaction context with identifier '" << context << "' was found");
    return -1;
    }

  return this->ActiveComponents[context].Index;
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayNode::SetActiveComponent(int componentType, int componentIndex, std::string context/*=GetDefaultContextName()*/)
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
bool vtkDMMLMarkupsDisplayNode::HasActiveComponent()
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
std::vector<std::string> vtkDMMLMarkupsDisplayNode::GetActiveComponentInteractionContexts()
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

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayNode::SetActiveControlPoint(int controlPointIndex)
{
  this->SetActiveComponent(ComponentControlPoint, controlPointIndex);
}

//---------------------------------------------------------------------------
int vtkDMMLMarkupsDisplayNode::UpdateActiveControlPointWorld(
  int controlPointIndex, vtkDMMLInteractionEventData* eventData,
  double orientationMatrixWorld[9], const char* viewNodeID,
  const char* associatedNodeID, int positionStatus)
{
  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || !eventData)
    {
    return -1;
    }

  bool addNewControlPoint = false;
  // Get index of point to update. If active index is not valid, use the next undefined point,
  // if none, create new point.
  int numberOfControlPoints = markupsNode->GetNumberOfControlPoints();
  if (controlPointIndex < 0 || controlPointIndex >= numberOfControlPoints
    || (markupsNode->GetNthControlPointPositionStatus(controlPointIndex) == vtkDMMLMarkupsNode::PositionDefined)
    || (markupsNode->GetNthControlPointPositionStatus(controlPointIndex) == vtkDMMLMarkupsNode::PositionMissing))
    {
    if (controlPointIndex < 0 || controlPointIndex >= numberOfControlPoints)
      {
      controlPointIndex = 0;
      }
    int undefinedIndex = -1;
    for (int offset = 0; offset < markupsNode->GetNumberOfControlPoints(); offset++)
      {
      int i = (controlPointIndex + offset) % numberOfControlPoints; // check all points, starting from controlPointIndex and wrap around
      int pointStatus = markupsNode->GetNthControlPointPositionStatus(i);
      if (pointStatus == vtkDMMLMarkupsNode::PositionUndefined)
        {
        undefinedIndex = i;
        break;
        }
      }
    if (undefinedIndex >= 0)
      {
      controlPointIndex = undefinedIndex;
      }
    else
      {
      controlPointIndex = markupsNode->GetNumberOfControlPoints();
      addNewControlPoint = true;
      }
    }

  // Update active component but not yet fire modified event because the control
  // point is not created/updated yet in the markups node.
  //TODO: Allow other interaction contexts to place markups
  bool activeComponentChanged = false;
  std::string interactionContext = eventData->GetInteractionContextName();
  if ( this->ActiveComponents[interactionContext].Index != controlPointIndex
    || this->ActiveComponents[interactionContext].Type != ComponentControlPoint )
    {
    this->ActiveComponents[interactionContext].Type = ComponentControlPoint;
    this->ActiveComponents[interactionContext].Index = controlPointIndex;
    activeComponentChanged = true;
    }

  // AddControlPoint will fire modified events anyway, so we temporarily disable events
  // to add a new point with a minimum number of events.
  bool wasDisabled = markupsNode->GetDisableModifiedEvent();
  markupsNode->DisableModifiedEventOn();
  if (positionStatus == vtkDMMLMarkupsNode::PositionPreview)
    {
    const char* layoutName = nullptr;
    if (this->GetScene())
      {
      vtkDMMLAbstractViewNode* viewNode = vtkDMMLAbstractViewNode::SafeDownCast(this->GetScene()->GetNodeByID(viewNodeID));
      if (viewNode)
        {
        layoutName = viewNode->GetLayoutName();
        }
      }
    markupsNode->SetAttribute("Markups.MovingInSliceView", layoutName ? layoutName : "");
    std::ostringstream controlPointIndexStr;
    controlPointIndexStr << controlPointIndex;
    markupsNode->SetAttribute("Markups.MovingMarkupIndex", controlPointIndexStr.str().c_str());
    }
  else
    {
    markupsNode->SetAttribute("Markups.MovingInSliceView", "");
    markupsNode->SetAttribute("Markups.MovingMarkupIndex", "");
    }
  markupsNode->SetDisableModifiedEvent(wasDisabled);

  double pointWorld[3] = { 0.0 };
  eventData->GetWorldPosition(pointWorld);

  if (addNewControlPoint)
    {
    // Add new control point
    vtkDMMLMarkupsNode::ControlPoint* controlPoint = new vtkDMMLMarkupsNode::ControlPoint;
    markupsNode->TransformPointFromWorld(pointWorld, controlPoint->Position);
    // TODO: transform orientation to world before copying
    std::copy_n(orientationMatrixWorld, 9, controlPoint->OrientationMatrix);
    if (associatedNodeID)
      {
      controlPoint->AssociatedNodeID = associatedNodeID;
      }
    controlPoint->PositionStatus = positionStatus;
    controlPoint->AutoCreated = true;
    markupsNode->AddControlPoint(controlPoint);
  }
  else
    {
    // Update existing control point
    markupsNode->SetNthControlPointPositionOrientationWorld(controlPointIndex,
      pointWorld, orientationMatrixWorld, associatedNodeID, positionStatus);
    if (positionStatus == vtkDMMLMarkupsNode::PositionUndefined)
      {
      markupsNode->SetNthControlPointAutoCreated(controlPointIndex, false);
      }
    }

  if (activeComponentChanged)
    {
    this->Modified();
    }

  return controlPointIndex;
}


//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayNode::GetActiveControlPoints(std::vector<int>& controlPointIndices)
{
  controlPointIndices.clear();
  for (std::map<std::string, ComponentInfo>::iterator it = this->ActiveComponents.begin(); it != this->ActiveComponents.end(); ++it)
    {
    if (it->second.Type == ComponentControlPoint)
      {
      controlPointIndices.push_back(it->second.Index);
      }
    }
}

//---------------------------------------------------------------------------
int vtkDMMLMarkupsDisplayNode::GetActiveControlPoint(std::string context)
{
  if ( this->ActiveComponents.find(context) != this->ActiveComponents.end()
    && this->ActiveComponents[context].Type == ComponentControlPoint )
    {
    return this->ActiveComponents[context].Index;
    }
  else
    {
    return -1;
    }
}

//---------------------------------------------------------------------------
vtkDMMLMarkupsNode* vtkDMMLMarkupsDisplayNode::GetMarkupsNode()
{
  return vtkDMMLMarkupsNode::SafeDownCast(this->GetDisplayableNode());
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayNode::SetTextPropertyFromString(std::string textPropertyString)
{
  if (textPropertyString.empty())
    {
    vtkErrorMacro("SetTextPropertyFromString: Invalid text property string");
    return;
    }

  std::string currentTextPropertyString = vtkDMMLDisplayNode::GetTextPropertyAsString(this->TextProperty);
  if (textPropertyString == currentTextPropertyString)
    {
    return;
    }

  DMMLNodeModifyBlocker blocker(this);
  this->UpdateTextPropertyFromString(textPropertyString, this->TextProperty);
  this->Modified();
}

//---------------------------------------------------------------------------
std::string vtkDMMLMarkupsDisplayNode::GetTextPropertyAsString()
{
  return vtkDMMLDisplayNode::GetTextPropertyAsString(this->TextProperty);
}

//-----------------------------------------------------------
vtkDataSet* vtkDMMLMarkupsDisplayNode::GetScalarDataSet()
{
  if (this->GetMarkupsNode())
    {
    return this->GetMarkupsNode()->GetCurveWorld();
    }
  return nullptr;
}

//-----------------------------------------------------------
vtkDataArray* vtkDMMLMarkupsDisplayNode::GetActiveScalarArray()
{
  if (this->GetActiveScalarName() == nullptr || strcmp(this->GetActiveScalarName(),"") == 0)
    {
    return nullptr;
    }
  if (!this->GetMarkupsNode())
    {
    return nullptr;
    }
  if (!this->GetMarkupsNode()->GetCurveWorld())
    {
    return nullptr;
    }

  return this->GetMarkupsNode()->GetCurveWorld()->GetPointData()->GetArray(this->GetActiveScalarName());
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayNode::UpdateAssignedAttribute()
{
  this->UpdateScalarRange();
  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    vtkWarningMacro("UpdateAssignedAttribute() failed: assign markupsNode before calling this method.");
    return;
    }
  markupsNode->UpdateAssignedAttribute();
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayNode::SetScalarVisibility(int visibility)
{
  if (visibility == this->GetScalarVisibility())
    {
    return;
    }
  DMMLNodeModifyBlocker blocker(this);
  Superclass::SetScalarVisibility(visibility);
  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    vtkWarningMacro("UpdateAssignedAttribute() failed: assign markupsNode before calling this method.");
    return;
    }
  // Markups uses a different filter output when scalar visibility is enabled therefore
  // we need to update assigned attribute each time the scalar visibility is changed.
  markupsNode->UpdateAssignedAttribute();
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayNode::SetHandleVisibility(int componentType, bool visibility)
{
  switch (componentType)
    {
    case vtkDMMLMarkupsDisplayNode::ComponentTranslationHandle:
      this->SetTranslationHandleVisibility(visibility);
      break;
    case vtkDMMLMarkupsDisplayNode::ComponentRotationHandle:
      this->SetRotationHandleVisibility(visibility);
      break;
    case vtkDMMLMarkupsDisplayNode::ComponentScaleHandle:
      this->SetScaleHandleVisibility(visibility);
      break;
    default:
      vtkErrorMacro("Unknown handle type");
      break;
    }
}

//---------------------------------------------------------------------------
bool vtkDMMLMarkupsDisplayNode::GetHandleVisibility(int componentType)
{
  switch (componentType)
    {
    case vtkDMMLMarkupsDisplayNode::ComponentTranslationHandle:
      return this->GetTranslationHandleVisibility();
    case vtkDMMLMarkupsDisplayNode::ComponentRotationHandle:
      return this->GetRotationHandleVisibility();
    case vtkDMMLMarkupsDisplayNode::ComponentScaleHandle:
      return this->GetScaleHandleVisibility();
    default:
      vtkErrorMacro("Unknown handle type");
    }
  return false;
}
