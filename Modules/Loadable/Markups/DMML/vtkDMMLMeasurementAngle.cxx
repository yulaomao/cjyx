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
#include "vtkDMMLMeasurementAngle.h"
#include "vtkDMMLUnitNode.h"

// Markups includes
#include "vtkDMMLMarkupsAngleNode.h"

vtkStandardNewMacro(vtkDMMLMeasurementAngle);

//----------------------------------------------------------------------------
vtkDMMLMeasurementAngle::vtkDMMLMeasurementAngle()
{
  this->SetUnits("deg");
  this->SetPrintFormat("%3.1f%s");
}

//----------------------------------------------------------------------------
vtkDMMLMeasurementAngle::~vtkDMMLMeasurementAngle() = default;

//----------------------------------------------------------------------------
void vtkDMMLMeasurementAngle::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//---------------------------------------------------------------------------
void vtkDMMLMeasurementAngle::Compute()
{
  if (!this->InputDMMLNode)
    {
    this->ClearValue(vtkDMMLMeasurement::InsufficientInput);
    return;
    }

  vtkDMMLMarkupsAngleNode* angleNode = vtkDMMLMarkupsAngleNode::SafeDownCast(this->InputDMMLNode);
  if (!angleNode)
    {
    vtkErrorMacro("Compute: Markup type not supported by this measurement: " << this->InputDMMLNode->GetClassName());
    this->ClearValue(vtkDMMLMeasurement::InsufficientInput);
    return;
    }

  if (angleNode->GetNumberOfDefinedControlPoints(true) != 3)
    {
    vtkDebugMacro("Compute: Angle nodes must have exactly three control points ("
      << angleNode->GetNumberOfDefinedControlPoints(true) << " found)");
    this->ClearValue(vtkDMMLMeasurement::InsufficientInput);
    return;
    }

  double p1[3] = { 0.0 };
  double c[3] = { 0.0 };
  double p2[3] = { 0.0 };
  angleNode->GetNthControlPointPositionWorld(0, p1);
  angleNode->GetNthControlPointPositionWorld(1, c);
  angleNode->GetNthControlPointPositionWorld(2, p2);

  if ( vtkMath::Distance2BetweenPoints(p1, c) < VTK_DBL_EPSILON
    || vtkMath::Distance2BetweenPoints(p2, c) < VTK_DBL_EPSILON )
    {
    vtkErrorMacro("Compute: Control points are too close to each other to compute angle reliably");
    this->ClearValue(vtkDMMLMeasurement::InsufficientInput);
    return;
    }

  this->SetValue(angleNode->GetAngleDegrees(), "angle");
}
