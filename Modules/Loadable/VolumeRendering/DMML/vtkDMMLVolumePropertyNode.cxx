// DMML includes
#include "vtkDMMLScene.h"
#include "vtkDMMLVolumePropertyNode.h"
#include "vtkDMMLVolumePropertyStorageNode.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkColorTransferFunction.h>
#include <vtkIntArray.h>
#include <vtkObjectFactory.h>
#include <vtkNew.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>

// STD includes
#include <algorithm>
#include <cassert>
#include <limits>
#include <sstream>

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLVolumePropertyNode);

//----------------------------------------------------------------------------
vtkDMMLVolumePropertyNode::vtkDMMLVolumePropertyNode()
  : EffectiveRange{0.0,-1.0}
{
  this->ObservedEvents = vtkIntArray::New();
  this->ObservedEvents->InsertNextValue(vtkCommand::StartEvent);
  this->ObservedEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  this->ObservedEvents->InsertNextValue(vtkCommand::EndEvent);
  this->ObservedEvents->InsertNextValue(vtkCommand::StartInteractionEvent);
  this->ObservedEvents->InsertNextValue(vtkCommand::InteractionEvent);
  this->ObservedEvents->InsertNextValue(vtkCommand::EndInteractionEvent);

  vtkVolumeProperty* property = vtkVolumeProperty::New();
  property->SetUseClippedVoxelIntensity(true);
  // Clipped voxel intensity has to be a different enough so that it the computed gradient
  // will change primarily between the two sides of the clipping plane.
  // Default value is -10e37, which does not result in smooth clipped surface (most probably
  // due to numerical errors), but -1e10 works robustly.
  property->SetClippedVoxelIntensity(-1e10);
  vtkSetAndObserveDMMLObjectEventsMacro(this->VolumeProperty, property, this->ObservedEvents);
  property->Delete();

  // Observe the transfer functions
  this->SetColor(property->GetRGBTransferFunction());
  this->SetScalarOpacity(property->GetScalarOpacity());
  this->SetGradientOpacity(property->GetGradientOpacity());

  this->SetHideFromEditors(0);
}

//----------------------------------------------------------------------------
vtkDMMLVolumePropertyNode::~vtkDMMLVolumePropertyNode()
{
  if(this->VolumeProperty)
    {
    vtkUnObserveDMMLObjectMacro(this->VolumeProperty->GetScalarOpacity());
    vtkUnObserveDMMLObjectMacro(this->VolumeProperty->GetGradientOpacity());
    vtkUnObserveDMMLObjectMacro(this->VolumeProperty->GetRGBTransferFunction());
    vtkSetAndObserveDMMLObjectMacro(this->VolumeProperty, nullptr);
    }
  this->ObservedEvents->Delete();
}

//----------------------------------------------------------------------------
void vtkDMMLVolumePropertyNode::SetEffectiveRange(double min, double max)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting EffectiveRange to (" << min << "," << max << ")");
  if ((this->EffectiveRange[0] != min) || (this->EffectiveRange[1] != max))
    {
    this->EffectiveRange[0] = min;
    this->EffectiveRange[1] = max;
    this->Modified();
    this->InvokeCustomModifiedEvent(EffectiveRangeModified);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLVolumePropertyNode::SetEffectiveRange(double range[2])
{
  this->SetEffectiveRange(range[0], range[1]);
}

//----------------------------------------------------------------------------
void vtkDMMLVolumePropertyNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  this->Superclass::WriteXML(of, nIndent);

  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLIntMacro(interpolation, InterpolationType);
  vtkDMMLWriteXMLIntMacro(shade, Shade);
  vtkDMMLWriteXMLFloatMacro(diffuse, Diffuse);
  vtkDMMLWriteXMLFloatMacro(ambient, Ambient);
  vtkDMMLWriteXMLFloatMacro(specular, Specular);
  vtkDMMLWriteXMLFloatMacro(specularPower, SpecularPower);
  vtkDMMLWriteXMLStdStringMacro(scalarOpacity, ScalarOpacityAsString);
  vtkDMMLWriteXMLStdStringMacro(gradientOpacity, GradientOpacityAsString);
  vtkDMMLWriteXMLStdStringMacro(colorTransfer, RGBTransferFunctionAsString);
  vtkDMMLWriteXMLVectorMacro(effectiveRange, EffectiveRange, double, 2);
  vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLVolumePropertyNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  this->Superclass::ReadXMLAttributes(atts);

  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLIntMacro(interpolation, InterpolationType);
  vtkDMMLReadXMLIntMacro(shade, Shade);
  vtkDMMLReadXMLFloatMacro(diffuse, Diffuse);
  vtkDMMLReadXMLFloatMacro(ambient, Ambient);
  vtkDMMLReadXMLFloatMacro(specular, Specular);
  vtkDMMLReadXMLFloatMacro(specularPower, SpecularPower);
  vtkDMMLReadXMLStdStringMacro(scalarOpacity, ScalarOpacityAsString);
  vtkDMMLReadXMLStdStringMacro(gradientOpacity, GradientOpacityAsString);
  vtkDMMLReadXMLStdStringMacro(colorTransfer, RGBTransferFunctionAsString);
  vtkDMMLReadXMLVectorMacro(effectiveRange, EffectiveRange, double, 2);
  vtkDMMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLVolumePropertyNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);
  this->CopyParameterSet(anode);
}

//----------------------------------------------------------------------------
void vtkDMMLVolumePropertyNode::CopyParameterSet(vtkDMMLNode *anode)
{
  vtkDMMLVolumePropertyNode *node = vtkDMMLVolumePropertyNode::SafeDownCast(anode);
  if (!node)
    {
    vtkErrorMacro("CopyParameterSet: Invalid input DMML node");
    return;
    }

  vtkDMMLCopyBeginMacro(anode);
  vtkDMMLCopyVectorMacro(EffectiveRange, double, 2);
  vtkDMMLCopyEndMacro();

  // VolumeProperty
  // VolumeProperty->IndependentComponents is determined automatically from the input volume,
  // therefore it is not necessary to copy its.
  this->VolumeProperty->SetInterpolationType(node->VolumeProperty->GetInterpolationType());
  this->VolumeProperty->SetUseClippedVoxelIntensity(node->VolumeProperty->GetUseClippedVoxelIntensity());

  for (int i=0; i<VTK_MAX_VRCOMP; i++)
    {
    this->VolumeProperty->SetComponentWeight(i,node->GetVolumeProperty()->GetComponentWeight(i));
    //TODO: No set method for GrayTransferFunction, ColorChannels, and DefaultGradientOpacity

    // Transfer functions
    vtkColorTransferFunction* rgbTransfer = vtkColorTransferFunction::New();
    rgbTransfer->DeepCopy(node->GetVolumeProperty()->GetRGBTransferFunction(i));
    this->SetColor(rgbTransfer, i);
    rgbTransfer->Delete();

    vtkPiecewiseFunction* scalar = vtkPiecewiseFunction::New();
    scalar->DeepCopy(node->GetVolumeProperty()->GetScalarOpacity(i));
    this->SetScalarOpacity(scalar, i);
    scalar->Delete();
    this->VolumeProperty->SetScalarOpacityUnitDistance(i,this->VolumeProperty->GetScalarOpacityUnitDistance(i));

    vtkPiecewiseFunction* gradient = vtkPiecewiseFunction::New();
    gradient->DeepCopy(node->GetVolumeProperty()->GetGradientOpacity(i));
    this->SetGradientOpacity(gradient, i);
    gradient->Delete();

    // Lighting
    this->VolumeProperty->SetDisableGradientOpacity(i,node->GetVolumeProperty()->GetDisableGradientOpacity(i));
    this->VolumeProperty->SetShade(i,node->GetVolumeProperty()->GetShade(i));
    this->VolumeProperty->SetAmbient(i, node->VolumeProperty->GetAmbient(i));
    this->VolumeProperty->SetDiffuse(i, node->VolumeProperty->GetDiffuse(i));
    this->VolumeProperty->SetSpecular(i, node->VolumeProperty->GetSpecular(i));
    this->VolumeProperty->SetSpecularPower(i, node->VolumeProperty->GetSpecularPower(i));
    }
}

//----------------------------------------------------------------------------
void vtkDMMLVolumePropertyNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  vtkDMMLPrintBeginMacro(os, indent);
  vtkDMMLPrintVectorMacro(EffectiveRange, double, 2);
  vtkDMMLPrintEndMacro();

  os << indent << "VolumeProperty: ";
  this->VolumeProperty->PrintSelf(os,indent.GetNextIndent());
}

//---------------------------------------------------------------------------
void vtkDMMLVolumePropertyNode::ProcessDMMLEvents( vtkObject *caller,
                                                   unsigned long event,
                                                   void *callData )
{
  this->Superclass::ProcessDMMLEvents(caller, event, callData);
  switch (event)
    {
    case vtkCommand::StartEvent:
    case vtkCommand::EndEvent:
    case vtkCommand::StartInteractionEvent:
    case vtkCommand::InteractionEvent:
    case vtkCommand::EndInteractionEvent:
      this->InvokeEvent(event);
      break;
    case vtkCommand::ModifiedEvent:
      this->Modified();
      break;
    }
}

//---------------------------------------------------------------------------
std::string vtkDMMLVolumePropertyNode::DataToString(double* data, int size)
{
  std::stringstream resultStream;
  double *it = data;
  // Write header
  resultStream << size;
  resultStream.precision(std::numeric_limits<double>::digits10);
  for (int i=0; i < size; ++i)
    {
    resultStream << " ";
    resultStream << *it;
    it++;
    }
  return resultStream.str();
}

//---------------------------------------------------------------------------
int vtkDMMLVolumePropertyNode::DataFromString(const std::string& dataString, double* &data)
{
  std::stringstream stream;
  stream << dataString;

  int size=0;
  stream >> size;
  if (size==0)
    {
    return 0;
    }
  data = new double[size];
  for(int i=0; i < size; ++i)
    {
    std::string s;
    stream >> s;
    data[i] = atof(s.c_str());
    }
  return size;
}

//---------------------------------------------------------------------------
int vtkDMMLVolumePropertyNode::NodesFromString(const std::string& dataString, double* &nodes, int nodeSize)
{
  int size = vtkDMMLVolumePropertyNode::DataFromString(dataString, nodes);
  if (size % nodeSize)
    {
    vtkGenericWarningMacro("vtkDMMLVolumePropertyNode::NodesFromString: Error parsing data string");
    return 0;
    }
  // Ensure uniqueness
  double previous = VTK_DOUBLE_MIN;
  for (int i = 0; i < size; i+= nodeSize)
    {
    nodes[i] = vtkDMMLVolumePropertyNode::HigherAndUnique(nodes[i], previous);
    }
  return size / nodeSize;
}

//---------------------------------------------------------------------------
std::string vtkDMMLVolumePropertyNode::GetPiecewiseFunctionString(vtkPiecewiseFunction* function)
{
  return vtkDMMLVolumePropertyNode::DataToString(function->GetDataPointer(), function->GetSize() * 2);
}

//---------------------------------------------------------------------------
std::string vtkDMMLVolumePropertyNode::GetColorTransferFunctionString(vtkColorTransferFunction* function)
{
  return vtkDMMLVolumePropertyNode::DataToString(function->GetDataPointer(), function->GetSize() * 4);
}

//---------------------------------------------------------------------------
void vtkDMMLVolumePropertyNode::GetPiecewiseFunctionFromString(
  const std::string& str,vtkPiecewiseFunction* result)
{
  double* data = nullptr;
  int size = vtkDMMLVolumePropertyNode::NodesFromString(str, data, 2);
  if (size)
    {
    result->FillFromDataPointer(size, data);
    }
  delete [] data;
}

//---------------------------------------------------------------------------
void vtkDMMLVolumePropertyNode::GetColorTransferFunctionFromString(
  const std::string& str, vtkColorTransferFunction* result)
{
  double* data = nullptr;
  int size = vtkDMMLVolumePropertyNode::NodesFromString(str, data, 4);
  if (size)
    {
    result->FillFromDataPointer(size, data);
    }
  delete [] data;
}

//----------------------------------------------------------------------------
double vtkDMMLVolumePropertyNode::NextHigher(double value)
{
  if (value == 0.)
    {
    // special case to avoid denormalized numbers
    return std::numeric_limits<double>::min();
    }
  // Increment the value by the smallest offset possible
  // The challenge here is to find the offset, if the value is 100000000., an
  // offset of epsilon won't work.
  typedef union {
      long long i64;
      double d64;
    } dbl_64;
  dbl_64 d;
  d.d64 = value;
  d.i64 += (value < 0.) ? -1 : 1;
  return d.d64;
}

//----------------------------------------------------------------------------
double vtkDMMLVolumePropertyNode::HigherAndUnique(double value, double &previousValue)
{
  value = std::max(value, previousValue);
  if (value == previousValue)
    {
    value = vtkDMMLVolumePropertyNode::NextHigher(value);
    }
  assert (value != previousValue);
  previousValue = value;
  return value;
}

//---------------------------------------------------------------------------
vtkDMMLStorageNode* vtkDMMLVolumePropertyNode::CreateDefaultStorageNode()
{
  vtkDMMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkDMMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkDMMLVolumePropertyStorageNode"));
}

//---------------------------------------------------------------------------
void vtkDMMLVolumePropertyNode::SetScalarOpacity(vtkPiecewiseFunction* newScalarOpacity, int component)
{
  vtkUnObserveDMMLObjectMacro(this->VolumeProperty->GetScalarOpacity(component));
  this->VolumeProperty->SetScalarOpacity(component, newScalarOpacity);
  vtkObserveDMMLObjectEventsMacro(this->VolumeProperty->GetScalarOpacity(component), this->ObservedEvents);
}

//---------------------------------------------------------------------------
vtkPiecewiseFunction* vtkDMMLVolumePropertyNode::GetScalarOpacity(int component)
{
  return this->VolumeProperty->GetScalarOpacity(component);
}

//---------------------------------------------------------------------------
void vtkDMMLVolumePropertyNode::SetGradientOpacity(
  vtkPiecewiseFunction* newGradientOpacity, int component)
{
  vtkUnObserveDMMLObjectMacro(this->VolumeProperty->GetGradientOpacity(component));
  this->VolumeProperty->SetGradientOpacity(component, newGradientOpacity);
  vtkObserveDMMLObjectEventsMacro(this->VolumeProperty->GetGradientOpacity(component), this->ObservedEvents);
}

//---------------------------------------------------------------------------
vtkPiecewiseFunction* vtkDMMLVolumePropertyNode::GetGradientOpacity(int component)
{
  return this->VolumeProperty->GetGradientOpacity(component);
}

//---------------------------------------------------------------------------
void vtkDMMLVolumePropertyNode::SetColor(vtkColorTransferFunction* newColorFunction, int component)
{
  vtkUnObserveDMMLObjectMacro(this->VolumeProperty->GetRGBTransferFunction(component));
  this->VolumeProperty->SetColor(component, newColorFunction);
  vtkObserveDMMLObjectEventsMacro(this->VolumeProperty->GetRGBTransferFunction(component), this->ObservedEvents);
}

//---------------------------------------------------------------------------
vtkColorTransferFunction* vtkDMMLVolumePropertyNode::GetColor(int component)
{
  return this->VolumeProperty->GetRGBTransferFunction(component);
}

//---------------------------------------------------------------------------
bool vtkDMMLVolumePropertyNode::GetModifiedSinceRead()
{
  return this->Superclass::GetModifiedSinceRead() ||
    (this->VolumeProperty &&
     this->VolumeProperty->GetMTime() > this->GetStoredTime());
}

//---------------------------------------------------------------------------
bool vtkDMMLVolumePropertyNode::CalculateEffectiveRange()
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("CalculateEffectiveRange: Invalid volume property");
    return false;
    }

  vtkColorTransferFunction* colorTransferFunction = this->VolumeProperty->GetRGBTransferFunction();
  vtkPiecewiseFunction* opacityFunction = this->VolumeProperty->GetScalarOpacity();
  vtkPiecewiseFunction* gradientFunction = this->VolumeProperty->GetGradientOpacity();
  if (!colorTransferFunction || !opacityFunction || !gradientFunction)
    {
    vtkErrorMacro("CalculateEffectiveRange: Invalid transfer functions in volume property");
    return false;
    }

  double effectiveRange[2] = {0.0};

  double colorRange[2] = {0.0};
  colorTransferFunction->GetRange(colorRange);
  effectiveRange[0] = std::min(effectiveRange[0], colorRange[0]);
  effectiveRange[1] = std::max(effectiveRange[1], colorRange[1]);

  double opacityRange[2] = {0.0};
  opacityFunction->GetRange(opacityRange);
  effectiveRange[0] = std::min(effectiveRange[0], opacityRange[0]);
  effectiveRange[1] = std::max(effectiveRange[1], opacityRange[1]);

  double gradientRange[2] = {0.0};
  gradientFunction->GetRange(gradientRange);
  effectiveRange[0] = std::min(effectiveRange[0], gradientRange[0]);
  effectiveRange[1] = std::max(effectiveRange[1], gradientRange[1]);

  this->SetEffectiveRange(effectiveRange);
  return true;
}

//---------------------------------------------------------------------------
int vtkDMMLVolumePropertyNode::GetInterpolationType()
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("GetInterpolationType: Invalid volume property");
    return 0;
    }
  return this->VolumeProperty->GetInterpolationType();
}

//---------------------------------------------------------------------------
void vtkDMMLVolumePropertyNode::SetInterpolationType(int interpolationType)
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("SetInterpolationType: Invalid volume property");
    return;
    }
  this->VolumeProperty->SetInterpolationType(interpolationType);
  this->Modified();
}

//---------------------------------------------------------------------------
int vtkDMMLVolumePropertyNode::GetShade()
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("GetShade: Invalid volume property");
    return 0;
    }
  return this->VolumeProperty->GetShade();
}

//---------------------------------------------------------------------------
void vtkDMMLVolumePropertyNode::SetShade(int shade)
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("SetShade: Invalid volume property");
    return;
    }
  this->VolumeProperty->SetShade(shade);
  this->Modified();
}

//---------------------------------------------------------------------------
double vtkDMMLVolumePropertyNode::GetDiffuse()
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("GetDiffuse: Invalid volume property");
    return 0.0;
    }
  return this->VolumeProperty->GetDiffuse();
}

//---------------------------------------------------------------------------
void vtkDMMLVolumePropertyNode::SetDiffuse(double diffuse)
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("SetDiffuse: Invalid volume property");
    return;
    }
  this->VolumeProperty->SetDiffuse(diffuse);
  this->Modified();
}

//---------------------------------------------------------------------------
double vtkDMMLVolumePropertyNode::GetAmbient()
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("GetAmbient: Invalid volume property");
    return 0.0;
    }
  return this->VolumeProperty->GetAmbient();
}

//---------------------------------------------------------------------------
void vtkDMMLVolumePropertyNode::SetAmbient(double ambient)
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("SetAmbient: Invalid volume property");
    return;
    }
  this->VolumeProperty->SetAmbient(ambient);
  this->Modified();
}

//---------------------------------------------------------------------------
double vtkDMMLVolumePropertyNode::GetSpecular()
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("GetSpecular: Invalid volume property");
    return 0.0;
    }
  return this->VolumeProperty->GetSpecular();
}

//---------------------------------------------------------------------------
void vtkDMMLVolumePropertyNode::SetSpecular(double specular)
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("SetSpecular: Invalid volume property");
    return;
    }
  this->VolumeProperty->SetSpecular(specular);
  this->Modified();
}

//---------------------------------------------------------------------------
double vtkDMMLVolumePropertyNode::GetSpecularPower()
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("GetSpecularPower: Invalid volume property");
    return 0.0;
    }
  return this->VolumeProperty->GetSpecularPower();
}

//---------------------------------------------------------------------------
void vtkDMMLVolumePropertyNode::SetSpecularPower(double specularPower)
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("SetSpecularPower: Invalid volume property");
    return;
    }
  this->VolumeProperty->SetSpecularPower(specularPower);
  this->Modified();
}

//---------------------------------------------------------------------------
std::string vtkDMMLVolumePropertyNode::GetScalarOpacityAsString()
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("GetScalarOpacityAsString: Invalid volume property");
    return "";
    }
  return this->GetPiecewiseFunctionString(this->VolumeProperty->GetScalarOpacity());
}

//---------------------------------------------------------------------------
void vtkDMMLVolumePropertyNode::SetScalarOpacityAsString(std::string scalarOpacityFunctionStr)
{
  vtkPiecewiseFunction* scalarOpacity = vtkPiecewiseFunction::New();
  this->GetPiecewiseFunctionFromString(scalarOpacityFunctionStr.c_str(), scalarOpacity);
  this->SetScalarOpacity(scalarOpacity);
  scalarOpacity->Delete();
  this->Modified();
}

//---------------------------------------------------------------------------
std::string vtkDMMLVolumePropertyNode::GetGradientOpacityAsString()
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("GetGradientOpacityAsString: Invalid volume property");
    return "";
    }
  return this->GetPiecewiseFunctionString(this->VolumeProperty->GetGradientOpacity());
}

//---------------------------------------------------------------------------
void vtkDMMLVolumePropertyNode::SetGradientOpacityAsString(std::string gradientOpacityFunctionStr)
{
  vtkPiecewiseFunction* gradientOpacity = vtkPiecewiseFunction::New();
  this->GetPiecewiseFunctionFromString(gradientOpacityFunctionStr.c_str(), gradientOpacity);
  this->SetGradientOpacity(gradientOpacity);
  gradientOpacity->Delete();
  this->Modified();
}

//---------------------------------------------------------------------------
std::string vtkDMMLVolumePropertyNode::GetRGBTransferFunctionAsString()
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("GetRGBTransferFunctionAsString: Invalid volume property");
    return "";
    }
  return this->GetColorTransferFunctionString(this->VolumeProperty->GetRGBTransferFunction());
}

//---------------------------------------------------------------------------
void vtkDMMLVolumePropertyNode::SetRGBTransferFunctionAsString(std::string rgbTransferFunctionStr)
{
  vtkColorTransferFunction* colorTransfer = vtkColorTransferFunction::New();
  this->GetColorTransferFunctionFromString(rgbTransferFunctionStr.c_str(), colorTransfer);
  this->SetColor(colorTransfer);
  colorTransfer->Delete();
  this->Modified();
}
