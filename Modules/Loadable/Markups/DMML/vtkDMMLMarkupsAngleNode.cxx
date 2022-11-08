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
#include "vtkDMMLMarkupsDisplayNode.h"
#include "vtkDMMLMarkupsAngleNode.h"
#include "vtkDMMLMeasurementAngle.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLMarkupsAngleNode);

//----------------------------------------------------------------------------
vtkDMMLMarkupsAngleNode::vtkDMMLMarkupsAngleNode()
  : OrientationRotationAxis{0.0, 0.0, 1.0}
  , AngleMeasurementMode(Minimal)
{
  this->MaximumNumberOfControlPoints = 3;
  this->RequiredNumberOfControlPoints = 3;

  // Setup measurements calculated for this markup type
  vtkNew<vtkDMMLMeasurementAngle> angleMeasurement;
  angleMeasurement->SetName("angle");
  angleMeasurement->SetInputDMMLNode(this);
  this->Measurements->AddItem(angleMeasurement);
}

//----------------------------------------------------------------------------
vtkDMMLMarkupsAngleNode::~vtkDMMLMarkupsAngleNode() = default;

//----------------------------------------------------------------------------
void vtkDMMLMarkupsAngleNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);

  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLEnumMacro(angleMeasurementMode, AngleMeasurementMode);
  vtkDMMLWriteXMLVectorMacro(orientationRotationAxis, OrientationRotationAxis, double, 3);
  vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsAngleNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);

  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLEnumMacro(angleMeasurementMode, AngleMeasurementMode);
  vtkDMMLReadXMLVectorMacro(orientationRotationAxis, OrientationRotationAxis, double, 3);
  vtkDMMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsAngleNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkDMMLCopyBeginMacro(anode);
  vtkDMMLCopyEnumMacro(AngleMeasurementMode);
  vtkDMMLCopyVectorMacro(OrientationRotationAxis, double, 3);
  vtkDMMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsAngleNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkDMMLPrintBeginMacro(os, indent);
  vtkDMMLPrintEnumMacro(AngleMeasurementMode);
  vtkDMMLPrintVectorMacro(OrientationRotationAxis, double, 3);
  vtkDMMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsAngleNode::SetAngleMeasurementMode(int type)
{
  this->AngleMeasurementMode = type;
  this->Modified();
  this->UpdateMeasurementsInternal();
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsAngleNode::SetOrientationRotationAxis(double r, double a, double s)
{
  this->OrientationRotationAxis[0] = r;
  this->OrientationRotationAxis[1] = a;
  this->OrientationRotationAxis[2] = s;
  this->Modified();
  this->UpdateMeasurementsInternal();
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsAngleNode::SetOrientationRotationAxis(double ras[3])
{
  this->SetOrientationRotationAxis(ras[0], ras[1], ras[2]);
}

//----------------------------------------------------------------------------
const char* vtkDMMLMarkupsAngleNode::GetAngleMeasurementModeAsString(int id)
{
  switch (id)
    {
    case vtkDMMLMarkupsAngleNode::Minimal:
      {
      return "minimal";
      }
    case vtkDMMLMarkupsAngleNode::OrientedPositive:
      {
      return "orientedPositive";
      }
    case vtkDMMLMarkupsAngleNode::OrientedSigned:
      {
      return "orientedSigned";
      }
    default:
      {
      vtkGenericWarningMacro("Unknown angle mode type: " << id);
      return "";
      }
    }
}

//----------------------------------------------------------------------------
int vtkDMMLMarkupsAngleNode::GetAngleMeasurementModeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    vtkGenericWarningMacro("Invalid angle mode name");
    return -1;
    }
  for (int i = 0; i < vtkDMMLMarkupsAngleNode::AngleMeasurementMode_Last; i++)
    {
    if (strcmp(name, vtkDMMLMarkupsAngleNode::GetAngleMeasurementModeAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // name not found
  vtkGenericWarningMacro("Unknown angle mode name: " << name);
  return -1;
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsAngleNode::SetAngleMeasurementModeToMinimal()
{
  this->SetAngleMeasurementMode(vtkDMMLMarkupsAngleNode::Minimal);
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsAngleNode::SetAngleMeasurementModeToOrientedSigned()
{
  this->SetAngleMeasurementMode(vtkDMMLMarkupsAngleNode::OrientedSigned);
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsAngleNode::SetAngleMeasurementModeToOrientedPositive()
{
  this->SetAngleMeasurementMode(vtkDMMLMarkupsAngleNode::OrientedPositive);
}

//---------------------------------------------------------------------------
double vtkDMMLMarkupsAngleNode::GetAngleDegrees()
{
  if (this->GetNumberOfDefinedControlPoints(true) != 3)
    {
    vtkErrorMacro("Angle markups require exactly three control points");
    return 0.0;
    }

  double p1[3] = { 0.0 };
  double c[3] = { 0.0 };
  double p2[3] = { 0.0 };
  this->GetNthControlPointPositionWorld(0, p1);
  this->GetNthControlPointPositionWorld(1, c);
  this->GetNthControlPointPositionWorld(2, p2);

  if ( vtkMath::Distance2BetweenPoints(p1, c) <= VTK_DBL_EPSILON
    && vtkMath::Distance2BetweenPoints(p2, c) <= VTK_DBL_EPSILON )
    {
    return 0.0;
    }

  double vector1[3] = { p1[0] - c[0], p1[1] - c[1], p1[2] - c[2] };
  double vector2[3] = { p2[0] - c[0], p2[1] - c[1], p2[2] - c[2] };
  vtkMath::Normalize(vector1);
  vtkMath::Normalize(vector2);
  double angle_Rad = acos(vtkMath::Dot(vector1, vector2));
  double angle_Deg = vtkMath::DegreesFromRadians(angle_Rad);
  if (this->AngleMeasurementMode == Minimal)
    {
    return angle_Deg;
    }
  else
    {
    double vector1_vector2_Cross[3] = {0.0, 0.0, 0.0};
    vtkMath::Cross(vector1, vector2, vector1_vector2_Cross);
    if (vtkMath::Dot(vector1_vector2_Cross, this->OrientationRotationAxis) >= 0)
      {
      return angle_Deg;
      }
    else
      {
      if (this->AngleMeasurementMode == OrientedSigned)
        {
        return (-1.0) * angle_Deg;
        }
      else if (this->AngleMeasurementMode == OrientedPositive)
        {
        return 360.0 - angle_Deg;
        }
      }
    }

  vtkErrorMacro("Invalid angle mode " << this->AngleMeasurementMode);
  return 0.0;
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsAngleNode::UpdateInteractionHandleToWorldMatrix()
{
  if (this->GetNumberOfControlPoints() < 3)
    {
    return;
    }

  double point0_World[3] = { 0.0 };
  double point1_World[3] = { 0.0 };
  double point2_World[3] = { 0.0 };
  this->GetNthControlPointPositionWorld(0, point0_World);
  this->GetNthControlPointPositionWorld(1, point1_World);
  this->GetNthControlPointPositionWorld(2, point2_World);

  double epsilon = 1e-5;
  double handleX_World[3] = { 1.0, 0.0, 0.0 };
  vtkMath::Subtract(point0_World, point1_World, handleX_World);
  if (vtkMath::Norm(handleX_World) < epsilon)
    {
    return;
    }
  vtkMath::Normalize(handleX_World);

  double vectorPoint1ToPoint2_World[3] = { 0.0 };
  vtkMath::Subtract(point2_World, point1_World, vectorPoint1ToPoint2_World);
  if (vtkMath::Norm(vectorPoint1ToPoint2_World) < epsilon)
    {
    return;
    }
  vtkMath::Normalize(vectorPoint1ToPoint2_World);

  if (std::abs(vtkMath::Dot(handleX_World, vectorPoint1ToPoint2_World)) > 1.0 - epsilon)
    {
    return;
    }

  double handleZ_World[3] = { 0.0 };
  vtkMath::Cross(handleX_World, vectorPoint1ToPoint2_World, handleZ_World);
  vtkMath::Normalize(handleZ_World);

  double handleY_World[3] = { 0.0 };
  vtkMath::Cross(handleZ_World, handleX_World, handleY_World);
  vtkMath::Normalize(handleY_World);

  vtkNew<vtkMatrix4x4> handleToWorldMatrix;
  for (int i = 0; i < 3; ++i)
    {
    handleToWorldMatrix->SetElement(i, 0, handleX_World[i]);
    handleToWorldMatrix->SetElement(i, 1, handleY_World[i]);
    handleToWorldMatrix->SetElement(i, 2, handleZ_World[i]);
    handleToWorldMatrix->SetElement(i, 3, point1_World[i]);
    }
  this->InteractionHandleToWorldMatrix->DeepCopy(handleToWorldMatrix);
}
