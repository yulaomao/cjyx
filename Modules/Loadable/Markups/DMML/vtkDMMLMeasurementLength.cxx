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
#include "vtkDMMLMeasurementLength.h"
#include "vtkDMMLUnitNode.h"

// Markups includes
#include "vtkDMMLMarkupsCurveNode.h"
#include "vtkDMMLMarkupsLineNode.h"

vtkStandardNewMacro(vtkDMMLMeasurementLength);

//----------------------------------------------------------------------------
vtkDMMLMeasurementLength::vtkDMMLMeasurementLength()
{
  this->SetUnits("mm");
  this->SetPrintFormat("%-#4.4g%s");
}

//----------------------------------------------------------------------------
vtkDMMLMeasurementLength::~vtkDMMLMeasurementLength() = default;

//----------------------------------------------------------------------------
void vtkDMMLMeasurementLength::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//---------------------------------------------------------------------------
void vtkDMMLMeasurementLength::Compute()
{
  if (!this->InputDMMLNode)
    {
    this->LastComputationResult = vtkDMMLMeasurement::InsufficientInput;
    return;
    }

  vtkDMMLMarkupsCurveNode* curveNode = vtkDMMLMarkupsCurveNode::SafeDownCast(this->InputDMMLNode);
  vtkDMMLMarkupsLineNode* lineNode = vtkDMMLMarkupsLineNode::SafeDownCast(this->InputDMMLNode);

  double length = 0.0;
  if (curveNode)
    {
    if (curveNode->GetNumberOfDefinedControlPoints(true) < 2)
      {
      vtkDebugMacro("Compute: Curve nodes must have more than one control points ("
        << curveNode->GetNumberOfDefinedControlPoints(true) << " found)");
      this->LastComputationResult = vtkDMMLMeasurement::InsufficientInput;
      return;
      }
    length = curveNode->GetCurveLengthWorld();
    }
  else if (lineNode)
    {
    if (lineNode->GetNumberOfDefinedControlPoints(true) < 2)
      {
      vtkDebugMacro("Compute: Line nodes must have exactly two control points ("
        << lineNode->GetNumberOfDefinedControlPoints(true) << " found)");
      this->LastComputationResult = vtkDMMLMeasurement::InsufficientInput;
      return;
      }
    length = lineNode->GetLineLengthWorld();
    }
  else
    {
    vtkErrorMacro("Compute: Markup type not supported by this measurement: " << this->InputDMMLNode->GetClassName());
    this->ClearValue(vtkDMMLMeasurement::InsufficientInput);
    return;
    }

  this->SetValue(length, "length");
}
