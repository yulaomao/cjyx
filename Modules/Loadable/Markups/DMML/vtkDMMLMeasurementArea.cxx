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
#include "vtkDMMLMeasurementArea.h"
#include "vtkDMMLUnitNode.h"

// Markups includes
#include "vtkDMMLMarkupsClosedCurveNode.h"
#include "vtkDMMLMarkupsPlaneNode.h"

vtkStandardNewMacro(vtkDMMLMeasurementArea);

//----------------------------------------------------------------------------
vtkDMMLMeasurementArea::vtkDMMLMeasurementArea()
{
  // Default unit is "cm2" because mm2 would be too small for most clinical values.
  // Accordingly, display coefficient is 0.01 because length unit is mm by default.
  this->SetUnits("cm2");
  this->SetDisplayCoefficient(0.01);
  this->SetPrintFormat("%-#4.4g%s");
}

//----------------------------------------------------------------------------
vtkDMMLMeasurementArea::~vtkDMMLMeasurementArea() = default;

//----------------------------------------------------------------------------
void vtkDMMLMeasurementArea::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//---------------------------------------------------------------------------
void vtkDMMLMeasurementArea::Compute()
{
  if (!this->InputDMMLNode)
    {
    this->ClearValue(vtkDMMLMeasurement::InsufficientInput);
    return;
    }

  // We derive area unit from length unit, but it may be better to introduce
  // an area unit node to be able to specify more human-friendly format.
  double area = 0.0;

  vtkDMMLMarkupsClosedCurveNode* curveNode = vtkDMMLMarkupsClosedCurveNode::SafeDownCast(this->InputDMMLNode);
  vtkDMMLMarkupsPlaneNode* planeNode = vtkDMMLMarkupsPlaneNode::SafeDownCast(this->InputDMMLNode);
  if (curveNode)
    {
    if (curveNode->GetNumberOfDefinedControlPoints(true) < 3)
      {
      vtkDebugMacro("Compute: Curve nodes must have more than one control points ("
        << curveNode->GetNumberOfDefinedControlPoints(true) << " found)");
      this->ClearValue(vtkDMMLMeasurement::InsufficientInput);
      return;
      }
    vtkPolyData* surfaceAreaMesh = this->GetMeshValue();
    if (!surfaceAreaMesh)
      {
      vtkNew<vtkPolyData> newMesh;
      this->SetMeshValue(newMesh);
      surfaceAreaMesh = this->GetMeshValue();
      }
    area = vtkDMMLMarkupsClosedCurveNode::GetClosedCurveSurfaceArea(curveNode, surfaceAreaMesh);
    }
  else if (planeNode)
    {
    double size_world[2] = { 0.0, 0.0 };
    planeNode->GetSizeWorld(size_world);
    area = size_world[0] * size_world[1];
    }
  else
    {
    vtkErrorMacro("Compute: Markup type not supported by this measurement: " << this->InputDMMLNode->GetClassName());
    this->ClearValue(vtkDMMLMeasurement::InsufficientInput);
    return;
    }

  this->SetValue(area, "area");
}
