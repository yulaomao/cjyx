/*=auto=========================================================================

Portions (c) Copyright 2017 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#include "vtkDMMLMeasurement.h"

// DMML include
#include "vtkDMMLScene.h"
#include "vtkDMMLSelectionNode.h"
#include "vtkDMMLUnitNode.h"

// VTK include
#include <vtkObjectFactory.h>

// STD include
#include <sstream>

//----------------------------------------------------------------------------
vtkDMMLMeasurement::vtkDMMLMeasurement()
{
  this->SetPrintFormat("%5.3f %s");
}

//----------------------------------------------------------------------------
vtkDMMLMeasurement::~vtkDMMLMeasurement()
{
  this->Clear();
}

//----------------------------------------------------------------------------
std::string vtkDMMLMeasurement::GetValueWithUnitsAsPrintableString()
{
  if (this->PrintFormat.empty())
    {
    return "";
    }
  if (!this->ValueDefined)
    {
    return "(undefined)";
    }
  char buf[80] = { 0 };
  snprintf(buf, sizeof(buf) - 1, this->PrintFormat.c_str(), this->GetDisplayValue(), this->Units.c_str());
  return buf;
}

//----------------------------------------------------------------------------
void vtkDMMLMeasurement::Clear()
{
  this->SetEnabled(true);
  this->ClearValue(InsufficientInput);
  this->ValueDefined = false;
  this->SetPrintFormat("%5.3f %s");
  this->SetDisplayCoefficient(1.0);
  this->SetQuantityCode(nullptr);
  this->SetDerivationCode(nullptr);
  this->SetUnitsCode(nullptr);
  this->SetMethodCode(nullptr);
  this->SetControlPointValues(nullptr);
  this->SetMeshValue(nullptr);
  this->SetInputDMMLNode(nullptr);
}

//----------------------------------------------------------------------------
void vtkDMMLMeasurement::ClearValue(ComputationResult computationResult/*=NoChange*/)
{
  bool modified = false;
  if (computationResult != NoChange)
    {
    if (this->LastComputationResult != computationResult)
      {
      this->LastComputationResult = computationResult;
      modified = true;
      }
    }
  if (this->Value != 0.0)
    {
    this->Value = 0.0;
    modified = true;
    }
  if (this->ValueDefined)
    {
    this->ValueDefined = false;
    modified = true;
    }
  if (modified)
    {
    this->Modified();
    }

  // Note: this->SetControlPointValues(nullptr); is not called here, because if we clear it here
  // then every time something in the markups node changes that calls curveGenerator->Modified()
  // that is supposed to use just these control point values, the UpdateMeasurementsInternal call
  // clears the value, thus deleting the control point data.
}

//----------------------------------------------------------------------------
void vtkDMMLMeasurement::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << indent << "Enabled: " << (this->Enabled ? "true" : "false") << "\n";
  os << indent << "Name: " << this->Name << "\n";
  os << indent << "PrintableValue: " << this->GetValueWithUnitsAsPrintableString();
  os << indent << "Value: " << this->Value << "\n";
  os << indent << "DisplayCoefficient: " << this->DisplayCoefficient << "\n";
  os << indent << "ValueDefined: " << (this->ValueDefined ? "true" : "false") << "\n";
  os << indent << "Units: " << this->Units << "\n";
  os << indent << "PrintFormat: " << this->PrintFormat << "\n";
  os << indent << "Description: " << this->Description << "\n";
  if (this->QuantityCode)
    {
    os << indent << "Quantity: " << this->QuantityCode->GetAsPrintableString() << std::endl;
    }
  if (this->DerivationCode)
    {
    os << indent << "Derivation: " << this->DerivationCode->GetAsPrintableString() << std::endl;
    }
  if (this->UnitsCode)
    {
    os << indent << "Units: " << this->UnitsCode->GetAsPrintableString() << std::endl;
    }
  if (this->MethodCode)
    {
    os << indent << "Method: " << this->MethodCode->GetAsPrintableString() << std::endl;
    }
}

//----------------------------------------------------------------------------
void vtkDMMLMeasurement::Copy(vtkDMMLMeasurement* src)
{
  if (!src)
    {
    return;
    }
  this->SetEnabled(src->Enabled);
  this->SetName(src->GetName());
  this->SetValue(src->GetValue());
  this->SetDisplayCoefficient(src->GetDisplayCoefficient());
  this->ValueDefined = src->GetValueDefined();
  this->SetUnits(src->GetUnits());
  this->SetPrintFormat(src->GetPrintFormat());
  this->SetDescription(src->GetDescription());
  if (src->QuantityCode)
    {
    if (!this->QuantityCode)
      {
      this->QuantityCode = vtkSmartPointer<vtkCodedEntry>::New();
      }
    this->QuantityCode->Copy(src->QuantityCode);
    }
  else
    {
    this->QuantityCode = nullptr;
    }
  if (src->DerivationCode)
    {
    if (!this->DerivationCode)
      {
      this->DerivationCode = vtkSmartPointer<vtkCodedEntry>::New();
      }
    this->DerivationCode->Copy(src->DerivationCode);
    }
  else
    {
    this->DerivationCode = nullptr;
    }
  if (src->UnitsCode)
    {
    if (!this->UnitsCode)
      {
      this->UnitsCode = vtkSmartPointer<vtkCodedEntry>::New();
      }
    this->UnitsCode->Copy(src->UnitsCode);
    }
  else
    {
    this->UnitsCode = nullptr;
    }
  if (src->MethodCode)
    {
    if (!this->MethodCode)
      {
      this->MethodCode = vtkSmartPointer<vtkCodedEntry>::New();
      }
    this->MethodCode->Copy(src->MethodCode);
    }
  else
    {
    this->MethodCode = nullptr;
    }
  if (src->ControlPointValues)
    {
    if (!this->ControlPointValues)
      {
      this->ControlPointValues = vtkSmartPointer<vtkDoubleArray>::New();
      }
    this->ControlPointValues->DeepCopy(src->ControlPointValues);
    }
  else
    {
    this->ControlPointValues = nullptr;
    }
  if (src->MeshValue)
    {
    if (!this->MeshValue)
      {
      this->MeshValue = vtkSmartPointer<vtkPolyData>::New();
      }
    this->MeshValue->DeepCopy(src->MeshValue);
    }
  else
    {
    this->MeshValue = nullptr;
    }

  this->SetInputDMMLNode(src->InputDMMLNode);
}

//----------------------------------------------------------------------------
void vtkDMMLMeasurement::SetEnabled(bool enabled)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Enabled to " << enabled);
  if (this->Enabled != enabled)
    {
    this->Enabled = enabled;
    this->Modified();
    this->InvokeEvent(InputDataModifiedEvent);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLMeasurement::EnabledOn()
{
  this->SetEnabled(true);
}

//----------------------------------------------------------------------------
void vtkDMMLMeasurement::EnabledOff()
{
  this->SetEnabled(false);
}

//----------------------------------------------------------------------------
void vtkDMMLMeasurement::SetUnits(std::string units)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Units to " << units);
  if (this->Units != units)
    {
    this->Units = units;
    this->Modified();
    this->InvokeEvent(InputDataModifiedEvent);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLMeasurement::SetPrintFormat(std::string format)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting PrintFormat to " << format);
  if (this->PrintFormat != format)
    {
    this->PrintFormat = format;
    this->Modified();
    this->InvokeEvent(InputDataModifiedEvent);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLMeasurement::SetQuantityCode(vtkCodedEntry* entry)
{
  if (!entry)
    {
    if (this->QuantityCode)
      {
      this->QuantityCode->Delete();
      this->QuantityCode = nullptr;
      }
    return;
    }
  if (!this->QuantityCode)
    {
    this->QuantityCode = vtkCodedEntry::New();
    }
  this->QuantityCode->Copy(entry);
}

//----------------------------------------------------------------------------
void vtkDMMLMeasurement::SetDerivationCode(vtkCodedEntry* entry)
{
  if (!entry)
    {
    if (this->DerivationCode)
      {
      this->DerivationCode->Delete();
      this->DerivationCode = nullptr;
      }
    return;
    }
  if (!this->DerivationCode)
    {
    this->DerivationCode = vtkCodedEntry::New();
    }
  this->DerivationCode->Copy(entry);
}

//----------------------------------------------------------------------------
void vtkDMMLMeasurement::SetUnitsCode(vtkCodedEntry* entry)
{
  if (!entry)
    {
    if (this->UnitsCode)
      {
      this->UnitsCode->Delete();
      this->UnitsCode = nullptr;
      }
    return;
    }
  if (!this->UnitsCode)
    {
    this->UnitsCode = vtkCodedEntry::New();
    }
  this->UnitsCode->Copy(entry);
}

//----------------------------------------------------------------------------
void vtkDMMLMeasurement::SetMethodCode(vtkCodedEntry* entry)
{
  if (!entry)
    {
    if (this->MethodCode)
      {
      this->MethodCode->Delete();
      this->MethodCode = nullptr;
      }
    return;
    }
  if (!this->MethodCode)
    {
    this->MethodCode = vtkCodedEntry::New();
    }
  this->MethodCode->Copy(entry);
}

//----------------------------------------------------------------------------
void vtkDMMLMeasurement::SetControlPointValues(vtkDoubleArray* inputValues)
{
  if (!inputValues)
    {
    this->ControlPointValues = nullptr;
    return;
    }
  if (!this->ControlPointValues)
    {
    this->ControlPointValues = vtkSmartPointer<vtkDoubleArray>::New();
    }
  this->ControlPointValues->DeepCopy(inputValues);
}

//----------------------------------------------------------------------------
void vtkDMMLMeasurement::SetMeshValue(vtkPolyData* meshValue)
{
  this->MeshValue = meshValue;
}

//----------------------------------------------------------------------------
void vtkDMMLMeasurement::SetInputDMMLNode(vtkDMMLNode* node)
{
  if (this->InputDMMLNode != node)
    {
    this->InputDMMLNode = node;
    this->Modified();
    this->InvokeEvent(InputDataModifiedEvent);
    }
}

//----------------------------------------------------------------------------
vtkDMMLNode* vtkDMMLMeasurement::GetInputDMMLNode()
{
  if (this->InputDMMLNode.GetPointer())
    {
    return this->InputDMMLNode.GetPointer();
    }

  return nullptr;
}

//----------------------------------------------------------------------------
const char* vtkDMMLMeasurement::GetLastComputationResultAsPrintableString()
{
  switch (this->LastComputationResult)
    {
    case OK: return "OK";
    case InsufficientInput: return "Insufficient input";
    case InternalError: return "Internal error";
    default:
      // invalid id
      return "";
    }
}

//----------------------------------------------------------------------------
void vtkDMMLMeasurement::SetDisplayValue(double displayValue, const char* units/*=nullptr*/, double displayCoefficient/*=0.0*/)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Value to " << displayValue);
  bool modified = false;

  // Update units and displayCoefficient if they are specified
  if (units)
    {
    if (this->Units != units)
      {
      this->Units = units;
      modified = true;
      }
    }
  if (displayCoefficient != 0.0)
    {
    // caller specified a new displayCoefficient
    if (this->DisplayCoefficient != displayCoefficient)
      {
      this->DisplayCoefficient = displayCoefficient;
      modified = true;
      }
    }
  else
    {
    // caller specified a new displayCoefficient
    displayCoefficient = this->DisplayCoefficient;
    }
  // Compute stored value
  if (displayCoefficient == 0.0)
    {
    vtkErrorMacro("vtkDMMLMeasurement::SetDisplayValue: invalid display coefficient == 0.0, using 1.0 instead");
    displayCoefficient = 1.0;
    }
  double value = displayValue / displayCoefficient;

  if (this->Value != value)
    {
    this->Value = value;
    modified = true;
    }
  if (this->ValueDefined != true)
    {
    this->ValueDefined = true;
    modified = true;
    }
  if (modified)
    {
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkDMMLMeasurement::SetValue(double value, const char* quantityName)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Value to " << value);
  bool modified = false;

  vtkDMMLUnitNode* unitNode = this->GetUnitNode(quantityName);
  if (unitNode)
    {
    std::string units;
    if (unitNode->GetSuffix())
      {
      units = unitNode->GetSuffix();
      }
    if (this->Units != units)
      {
      this->Units = units;
      modified = true;
      }

    std::string printFormat;
    if (unitNode->GetDisplayStringFormat())
      {
      printFormat = unitNode->GetDisplayStringFormat();
      }
    if (this->PrintFormat != printFormat)
      {
      this->PrintFormat = printFormat;
      modified = true;
      }

    double displayCoefficient = unitNode->GetDisplayCoefficient();
    if (this->DisplayCoefficient != displayCoefficient)
      {
      this->DisplayCoefficient = displayCoefficient;
      modified = true;
      }
    }

  if (this->Value != value)
    {
    this->Value = value;
    modified = true;
    }
  if (this->ValueDefined != true)
    {
    this->ValueDefined = true;
    modified = true;
    }
  ComputationResult computationResult = vtkDMMLMeasurement::OK;
  if (this->LastComputationResult != computationResult)
    {
    this->LastComputationResult = computationResult;
    modified = true;
    }
  if (modified)
    {
    this->Modified();
    }
}

//---------------------------------------------------------------------------
double vtkDMMLMeasurement::GetDisplayValue()
{
  return this->Value * this->DisplayCoefficient;
};

//---------------------------------------------------------------------------
vtkDMMLUnitNode* vtkDMMLMeasurement::GetUnitNode(const char* quantityName)
{
  if (!quantityName || strlen(quantityName)==0)
    {
    return nullptr;
    }
  if (!this->InputDMMLNode || !this->InputDMMLNode->GetScene())
    {
    vtkWarningMacro("vtkDMMLMeasurement::GetUnitNode failed: InputDMMLNode is required to get the unit node from the scene");
    return nullptr;
    }
  vtkDMMLScene* scene = this->InputDMMLNode->GetScene();
  if (!scene)
  {
    return nullptr;
  }
  vtkDMMLSelectionNode* selectionNode = vtkDMMLSelectionNode::SafeDownCast(
    scene->GetNodeByID("vtkDMMLSelectionNodeSingleton"));
  if (!selectionNode)
    {
    vtkWarningMacro("vtkDMMLMeasurement::GetUnitNode failed: selection node not found");
    return nullptr;
    }
  vtkDMMLUnitNode* unitNode = vtkDMMLUnitNode::SafeDownCast(scene->GetNodeByID(
    selectionNode->GetUnitNodeID(quantityName)));

  // Do not log warning if null, because for example there is no 'angle' unit node, and in
  // that case hundreds of warnings would be thrown in a non erroneous situation.
  return unitNode;
}

//----------------------------------------------------------------------------
vtkCodedEntry* vtkDMMLMeasurement::GetQuantityCode()
{
  return this->QuantityCode;
}

//----------------------------------------------------------------------------
vtkCodedEntry* vtkDMMLMeasurement::GetDerivationCode()
{
  return this->DerivationCode;
}

//----------------------------------------------------------------------------
vtkCodedEntry* vtkDMMLMeasurement::GetUnitsCode()
{
  return this->UnitsCode;
}

//----------------------------------------------------------------------------
vtkCodedEntry* vtkDMMLMeasurement::GetMethodCode()
{
  return this->MethodCode;
}

//----------------------------------------------------------------------------
vtkPolyData* vtkDMMLMeasurement::GetMeshValue()
{
  return this->MeshValue;
}
