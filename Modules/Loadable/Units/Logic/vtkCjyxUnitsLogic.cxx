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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Units Logic includes
#include "vtkCjyxUnitsLogic.h"

// DMML includes
#include <vtkDMMLScene.h>
#include <vtkDMMLSelectionNode.h>
#include <vtkDMMLUnitNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkCjyxUnitsLogic);

//----------------------------------------------------------------------------
vtkCjyxUnitsLogic::vtkCjyxUnitsLogic()
{
  this->UnitsScene = vtkDMMLScene::New();
  this->RestoringDefaultUnits = false;
  this->AddBuiltInUnits(this->UnitsScene);
}

//----------------------------------------------------------------------------
vtkCjyxUnitsLogic::~vtkCjyxUnitsLogic()
{
  if (this->UnitsScene)
    {
    this->UnitsScene->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkCjyxUnitsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
vtkDMMLUnitNode* vtkCjyxUnitsLogic
::AddUnitNode(const char* name, const char* quantity, const char* prefix,
              const char* suffix, int precision, double min, double max)
{
  return this->AddUnitNodeToScene(this->GetDMMLScene(), name, quantity,
    prefix, suffix, precision, min, max);
}

//----------------------------------------------------------------------------
vtkDMMLScene* vtkCjyxUnitsLogic::GetUnitsScene() const
{
  return this->UnitsScene;
}

//----------------------------------------------------------------------------
double vtkCjyxUnitsLogic::
GetSIPrefixCoefficient(const char* prefix)
{
  if (!prefix)
    {
    return 1.;
    }
  if (strcmp("yotta", prefix) == 0) { return 1000000000000000000000000.; }
  else if (strcmp("zetta", prefix) == 0) { return 1000000000000000000000.; }
  else if (strcmp("exa", prefix) == 0) { return 1000000000000000000.; }
  else if (strcmp("peta", prefix) == 0) { return 1000000000000000.; }
  else if (strcmp("tera", prefix) == 0) { return 1000000000000.; }
  else if (strcmp("giga", prefix) == 0) { return 1000000000.; }
  else if (strcmp("mega", prefix) == 0) { return 1000000.; }
  else if (strcmp("kilo", prefix) == 0) { return 1000.; }
  else if (strcmp("hecto", prefix) == 0) { return 100.; }
  else if (strcmp("deca", prefix) == 0) { return 10.; }
  else if (strcmp("", prefix) == 0) { return 1.; }
  else if (strcmp("deci", prefix) == 0) { return 0.1; }
  else if (strcmp("centi", prefix) == 0) { return 0.01; }
  else if (strcmp("milli", prefix) == 0) { return 0.001; }
  else if (strcmp("micro", prefix) == 0) { return 0.000001; }
  else if (strcmp("nano", prefix) == 0) { return 0.000000001; }
  else if (strcmp("pico", prefix) == 0) { return 0.000000000001; }
  else if (strcmp("femto", prefix) == 0) { return 0.000000000000001; }
  else if (strcmp("atto", prefix) == 0) { return 0.000000000000000001; }
  else if (strcmp("zepto", prefix) == 0) { return 0.000000000000000000001; }
  else if (strcmp("yocto", prefix) == 0) { return 0.000000000000000000000001; }
  else { return 1.; }
}

//----------------------------------------------------------------------------
double vtkCjyxUnitsLogic::GetDisplayCoefficient(const char* prefix, const char* basePrefix, double power/*=1*/)
{
  return pow(GetSIPrefixCoefficient(basePrefix) / GetSIPrefixCoefficient(prefix), power);
}

//----------------------------------------------------------------------------
vtkDMMLUnitNode* vtkCjyxUnitsLogic
::AddUnitNodeToScene(vtkDMMLScene* scene, const char* name,
                     const char* quantity, const char* prefix,
                     const char* suffix, int precision,
                     double min, double max,
                     double displayCoeff, double displayOffset)
{
  if (!scene)
    {
    return nullptr;
    }

  vtkDMMLUnitNode* unitNode = vtkDMMLUnitNode::New();
  unitNode->SetName(name);
  unitNode->SetQuantity(quantity);
  unitNode->SetPrefix(prefix);
  unitNode->SetSuffix(suffix);
  unitNode->SetPrecision(precision);
  unitNode->SetMinimumValue(min);
  unitNode->SetMaximumValue(max);
  unitNode->SetDisplayCoefficient(displayCoeff);
  unitNode->SetDisplayOffset(displayOffset);

  scene->AddNode(unitNode);
  unitNode->Delete();
  return unitNode;
}

//---------------------------------------------------------------------------
void vtkCjyxUnitsLogic::SetDMMLSceneInternal(vtkDMMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkDMMLScene::StartBatchProcessEvent);
  events->InsertNextValue(vtkDMMLScene::EndBatchProcessEvent);
  this->SetAndObserveDMMLSceneEventsInternal(newScene, events.GetPointer());
}

//---------------------------------------------------------------------------
void vtkCjyxUnitsLogic::ObserveDMMLScene()
{
  this->AddDefaultsUnits();
  this->Superclass::ObserveDMMLScene();
}

//---------------------------------------------------------------------------
void vtkCjyxUnitsLogic::UpdateFromDMMLScene()
{
  // called on EndBatchProcessEvent (e.g., scene close).
  if (this->GetDMMLScene())
    {
    this->Modified();
    }
}

//---------------------------------------------------------------------------
void vtkCjyxUnitsLogic::AddDefaultsUnits()
{
  vtkDMMLUnitNode* node =
    this->AddUnitNode("ApplicationLength", "length", "", "mm", 4);
  node->SetSaveWithScene(false);
  this->SetDefaultUnit(node->GetQuantity(), node->GetID());

  node = this->AddUnitNode("ApplicationArea", "area", "", "cm2", 4);
  node->SetDisplayCoefficient(Self::GetDisplayCoefficient("centi", "milli", 2));
  node->SetSaveWithScene(false);
  this->SetDefaultUnit(node->GetQuantity(), node->GetID());

  node = this->AddUnitNode("ApplicationVolume", "volume", "", "cm3", 5);
  node->SetDisplayCoefficient(Self::GetDisplayCoefficient("centi", "milli", 3));
  node->SetSaveWithScene(false);
  this->SetDefaultUnit(node->GetQuantity(), node->GetID());

  node = this->AddUnitNode("ApplicationTime", "time", "", "s", 3);
  node->SetSaveWithScene(false);
  this->SetDefaultUnit(node->GetQuantity(), node->GetID());

  node = this->AddUnitNode("ApplicationFrequency", "frequency", "", "Hz", 3);
  node->SetSaveWithScene(false);
  this->SetDefaultUnit(node->GetQuantity(), node->GetID());

  node = this->AddUnitNode("ApplicationVelocity", "velocity", "", "m/s", 3);
  node->SetSaveWithScene(false);
  this->SetDefaultUnit(node->GetQuantity(), node->GetID());

  node = this->AddUnitNode("ApplicationIntensity", "intensity", "", "W/m^2", 3);
  node->SetSaveWithScene(false);
  this->SetDefaultUnit(node->GetQuantity(), node->GetID());
}

//---------------------------------------------------------------------------
void vtkCjyxUnitsLogic::AddBuiltInUnits(vtkDMMLScene* scene)
{
  if (!scene)
    {
    return;
    }

  this->RegisterNodesInternal(scene);

  // Add defaults nodes here

  // in Cjyx, "length" quantity values are always expressed in millimeters.
  this->AddUnitNodeToScene(scene,
    "Meter", "length", "", "m", 4, -10000., 10000., Self::GetDisplayCoefficient("", "milli"), 0.);
  this->AddUnitNodeToScene(scene,
    "Centimeter", "length", "", "cm", 4, -10000., 10000., Self::GetDisplayCoefficient("centi", "milli"), 0.);
  this->AddUnitNodeToScene(scene,
    "Millimeter", "length", "", "mm", 4, -10000., 10000., Self::GetDisplayCoefficient("milli", "milli"), 0.);
  this->AddUnitNodeToScene(scene,
    "Micrometer", "length", "", u8"\u00b5m", 4, -10000., 10000., Self::GetDisplayCoefficient("micro", "milli"), 0.);
  this->AddUnitNodeToScene(scene,
    "Nanometer", "length", "", "nm", 4, -10000., 10000., Self::GetDisplayCoefficient("nano", "milli"), 0.);

  this->AddUnitNodeToScene(scene,
    "Square Meter", "area", "", "m2", 4, -10000., 10000., Self::GetDisplayCoefficient("", "milli", 2), 0.);
  this->AddUnitNodeToScene(scene,
    "Square Centimeter", "area", "", "cm2", 4, -10000., 10000., Self::GetDisplayCoefficient("centi", "milli", 2), 0.);
  this->AddUnitNodeToScene(scene,
    "Square Millimeter", "area", "", "mm2", 4, -10000., 10000., Self::GetDisplayCoefficient("milli", "milli", 2), 0.);
  this->AddUnitNodeToScene(scene,
    "Square Micrometer", "area", "", u8"\u00b5m2", 4, -10000., 10000., Self::GetDisplayCoefficient("micro", "milli", 2), 0.);
  this->AddUnitNodeToScene(scene,
    "Square Nanometer", "area", "", "nm2", 4, -10000., 10000., Self::GetDisplayCoefficient("nano", "milli", 2), 0.);

  this->AddUnitNodeToScene(scene,
    "Cubic Meter", "volume", "", "m3", 5, -10000., 10000., Self::GetDisplayCoefficient("", "milli", 3), 0.);
  this->AddUnitNodeToScene(scene,
    "Cubic Centimeter", "volume", "", "cm3", 5, -10000., 10000., Self::GetDisplayCoefficient("centi", "milli", 3), 0.);
  this->AddUnitNodeToScene(scene,
    "Cubic Millimeter", "volume", "", "mm3", 5, -10000., 10000., Self::GetDisplayCoefficient("milli", "milli", 3), 0.);
  this->AddUnitNodeToScene(scene,
    "Cubic Micrometer", "volume", "", u8"\u00b5m3", 5, -10000., 10000., Self::GetDisplayCoefficient("micro", "milli", 3), 0.);
  this->AddUnitNodeToScene(scene,
    "Cubic Nanometer", "volume", "", "nm3", 5, -10000., 10000., Self::GetDisplayCoefficient("nano", "milli", 3), 0.);

  // 30.436875 is average number of days in a month
  this->AddUnitNodeToScene(scene,
    "Year", "time", "", "year", 2, -10000., 10000., 1.0 / 12.0*30.436875*24.0*60.0*60.0, 0.);
  this->AddUnitNodeToScene(scene,
    "Month", "time", "", "month", 2, -10000., 10000., 1.0 / 30.436875*24.0*60.0*60.0, 0.);
  this->AddUnitNodeToScene(scene,
    "Day", "time", "", "day", 2, -10000., 10000., 1.0 / 24.0*60.0*60.0, 0.);
  this->AddUnitNodeToScene(scene,
    "Hour", "time", "", "h", 2, -10000., 10000., 1.0 / 60.0*60.0, 0.);
  this->AddUnitNodeToScene(scene,
    "Minute", "time", "", "min", 2, -10000., 10000., 1.0/60.0, 0.);
  this->AddUnitNodeToScene(scene,
    "Second", "time", "", "s", 3, -10000., 10000., Self::GetDisplayCoefficient(""), 0.);
  this->AddUnitNodeToScene(scene,
    "Millisecond", "time", "", "ms", 3, -10000., 10000., Self::GetDisplayCoefficient("milli"), 0.);
  this->AddUnitNodeToScene(scene,
    "Microsecond", "time", "", u8"\u00b5s", 3, -10000., 10000., Self::GetDisplayCoefficient("micro"), 0.);

  this->AddUnitNodeToScene(scene,
    "Hertz", "frequency", "", "Hz", 3, -10000., 10000., Self::GetDisplayCoefficient(""), 0.);
  this->AddUnitNodeToScene(scene,
    "Decahertz", "frequency", "", "daHz", 3, -10000., 10000., Self::GetDisplayCoefficient("deca"), 0.);
  this->AddUnitNodeToScene(scene,
    "Hectohertz", "frequency", "", "hHz", 3, -10000., 10000., Self::GetDisplayCoefficient("hecto"), 0.);
  this->AddUnitNodeToScene(scene,
    "Kilohertz", "frequency", "", "kHz", 3, -10000., 10000., Self::GetDisplayCoefficient("kilo"), 0.);
  this->AddUnitNodeToScene(scene,
    "Megahertz", "frequency", "", "MHz", 3, -10000., 10000., Self::GetDisplayCoefficient("mega"), 0.);
  this->AddUnitNodeToScene(scene,
    "Gigahertz", "frequency", "", "GHz", 3, -10000., 10000., Self::GetDisplayCoefficient("giga"), 0.);
  this->AddUnitNodeToScene(scene,
    "Terahertz", "frequency", "", "THz", 3, -10000., 10000., Self::GetDisplayCoefficient("tera"), 0.);

  this->AddUnitNodeToScene(scene,
    "Meter per second", "velocity", "", "m/s", 3, -10000., 10000., Self::GetDisplayCoefficient(""), 0.);
  this->AddUnitNodeToScene(scene,
    "Kilometer per second", "velocity", "", "km/s", 3, -10000., 10000., Self::GetDisplayCoefficient("kilo"), 0.);

  this->AddUnitNodeToScene(scene,
    "Intensity", "intensity", "", "W/m\xB2", 3, -10000., 10000., 1., 0.);
}

//-----------------------------------------------------------------------------
void vtkCjyxUnitsLogic::SetDefaultUnit(const char* quantity, const char* id)
{
  if (!quantity || !this->GetDMMLScene())
    {
    return;
    }

  vtkDMMLSelectionNode* selectionNode =  vtkDMMLSelectionNode::SafeDownCast(
    this->GetDMMLScene()->GetNodeByID("vtkDMMLSelectionNodeSingleton"));
  if (selectionNode)
    {
    selectionNode->SetUnitNodeID(quantity, id);
    if (!vtkIsObservedDMMLNodeEventMacro(selectionNode,
                                         vtkCommand::ModifiedEvent))
      {
      vtkObserveDMMLNodeMacro(selectionNode);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkCjyxUnitsLogic::RegisterNodes()
{
  this->RegisterNodesInternal(this->GetDMMLScene());
}

//-----------------------------------------------------------------------------
void vtkCjyxUnitsLogic::RegisterNodesInternal(vtkDMMLScene* scene)
{
  assert(scene != nullptr);

  vtkNew<vtkDMMLUnitNode> unitNode;
  scene->RegisterNodeClass(unitNode.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkCjyxUnitsLogic::OnDMMLSceneStartBatchProcess()
{
  // We save the units so that they can be restored when the singleton gets
  // reset (scene is cleared/closed...).
  this->SaveDefaultUnits();
  this->Superclass::OnDMMLSceneStartBatchProcess();
}

//-----------------------------------------------------------------------------
void vtkCjyxUnitsLogic::OnDMMLNodeModified(vtkDMMLNode* node)
{
  if (vtkDMMLSelectionNode::SafeDownCast(node) &&
      !this->RestoringDefaultUnits)
    {
    this->RestoreDefaultUnits();
    }
  this->Superclass::OnDMMLNodeModified(node);
}

//-----------------------------------------------------------------------------
void vtkCjyxUnitsLogic::SaveDefaultUnits()
{
  // Save selection node units.
  vtkDMMLSelectionNode* selectionNode =  vtkDMMLSelectionNode::SafeDownCast(
    this->GetDMMLScene()->GetNodeByID("vtkDMMLSelectionNodeSingleton"));
  std::vector<const char *> quantities;
  std::vector<const char *> unitIDs;
  if (selectionNode)
    {
    selectionNode->GetUnitNodeIDs(quantities, unitIDs);
    }
  this->CachedDefaultUnits.clear();
  std::vector<const char*>::const_iterator qIt;
  std::vector<const char*>::const_iterator uIt;
  for (qIt = quantities.begin(), uIt = unitIDs.begin();
       uIt != unitIDs.end(); ++qIt, ++uIt)
    {
    assert(qIt != quantities.end());
    const char* quantity = *qIt;
    const char* unitID = *uIt;
    assert( (quantity != nullptr) == (unitID != nullptr) );
    if (quantity && unitID)
      {
      this->CachedDefaultUnits[quantity] = unitID;
      }
    }
}

//-----------------------------------------------------------------------------
void vtkCjyxUnitsLogic::RestoreDefaultUnits()
{
  this->RestoringDefaultUnits = true;
  vtkDMMLSelectionNode* selectionNode =  vtkDMMLSelectionNode::SafeDownCast(
    this->GetDMMLScene()->GetNodeByID("vtkDMMLSelectionNodeSingleton"));
  std::vector<vtkDMMLUnitNode*> units;
  int wasModifying = 0;
  if (selectionNode)
    {
    wasModifying = selectionNode->StartModify();
    }
  // Restore selection node units.
  std::map<std::string, std::string>::const_iterator it;
  for ( it = this->CachedDefaultUnits.begin() ;
        it != this->CachedDefaultUnits.end();
        ++it )
    {
    this->SetDefaultUnit(it->first.c_str(), it->second.c_str());
    }
  if (selectionNode)
    {
    selectionNode->EndModify(wasModifying);
    }
  this->RestoringDefaultUnits = false;
}