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
#include "vtkDMMLMeasurementVolume.h"
#include "vtkDMMLUnitNode.h"

// Markups includes
#include "vtkDMMLMarkupsROINode.h"

vtkStandardNewMacro(vtkDMMLMeasurementVolume);

//----------------------------------------------------------------------------
vtkDMMLMeasurementVolume::vtkDMMLMeasurementVolume()
{
  // Default unit is cm3 because mm3 would be too small for most clinical values.
  // Accordingly, since length unit is mm by default, display coefficient for cm3 is 0.001.
  this->SetUnits("cm3");
  this->SetDisplayCoefficient(0.001);
  this->SetPrintFormat("%-#4.4g%s");
}

//----------------------------------------------------------------------------
vtkDMMLMeasurementVolume::~vtkDMMLMeasurementVolume() = default;

//----------------------------------------------------------------------------
void vtkDMMLMeasurementVolume::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//---------------------------------------------------------------------------
void vtkDMMLMeasurementVolume::Compute()
{
  if (!this->InputDMMLNode)
    {
    this->ClearValue(vtkDMMLMeasurement::InsufficientInput);
    return;
    }

  vtkDMMLMarkupsROINode* roiNode = vtkDMMLMarkupsROINode::SafeDownCast(this->InputDMMLNode);
  if (!roiNode)
    {
    vtkErrorMacro("Compute: Markup type not supported by this measurement: " << this->InputDMMLNode->GetClassName());
    this->ClearValue(vtkDMMLMeasurement::InsufficientInput);
    return;
    }

  double size[3] = { 0.0, 0.0, 0.0 };
  roiNode->GetSizeWorld(size);
  double volume = size[0] * size[1] * size[2];

  this->SetValue(volume, "volume");
}
