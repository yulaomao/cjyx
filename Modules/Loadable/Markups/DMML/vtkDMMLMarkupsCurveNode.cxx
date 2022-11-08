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

#include "vtkDMMLMarkupsCurveNode.h"

// DMML includes
#include "vtkCurveGenerator.h"
#include "vtkCurveMeasurementsCalculator.h"
#include "vtkDMMLMarkupsDisplayNode.h"
#include "vtkDMMLMeasurementLength.h"
#include "vtkDMMLStaticMeasurement.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLTransformNode.h"
#include "vtkDMMLUnitNode.h"
#include "vtkProjectMarkupsCurvePointsFilter.h"
#include "vtkCjyxDijkstraGraphGeodesicPath.h"

// VTK includes
#include <vtkArrayCalculator.h>
#include <vtkAssignAttribute.h>
#include <vtkBoundingBox.h>
#include <vtkCallbackCommand.h>
#include <vtkCellLocator.h>
#include <vtkCleanPolyData.h>
#include <vtkCommand.h>
#include <vtkCutter.h>
#include <vtkDoubleArray.h>
#include <vtkGeneralTransform.h>
#include <vtkGenericCell.h>
#include <vtkLine.h>
#include <vtkMathUtilities.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkOBBTree.h>
#include <vtkObjectFactory.h>
#include <vtkParallelTransportFrame.h>
#include <vtkPassThroughFilter.h>
#include <vtkPlane.h>
#include <vtkPointData.h>
#include <vtkPointLocator.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkStringArray.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTriangleFilter.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLMarkupsCurveNode);


//----------------------------------------------------------------------------
vtkDMMLMarkupsCurveNode::vtkDMMLMarkupsCurveNode()
{
  // Set RequiredNumberOfControlPoints to a very high number to remain
  // in place mode after placing a curve point.
  this->RequiredNumberOfControlPoints = 1e6;

  this->CleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();

  this->TriangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
  this->TriangleFilter->SetInputConnection(this->CleanFilter->GetOutputPort());

  this->SurfaceToLocalTransformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->SurfaceToLocalTransformer->SetTransform(vtkNew<vtkGeneralTransform>());
  this->SurfaceToLocalTransformer->SetInputConnection(this->TriangleFilter->GetOutputPort());

  this->SurfaceScalarCalculator = vtkSmartPointer<vtkArrayCalculator>::New();
  this->SurfaceScalarCalculator->SetInputConnection(this->SurfaceToLocalTransformer->GetOutputPort());
  this->SurfaceScalarCalculator->AddObserver(vtkCommand::ModifiedEvent, this->DMMLCallbackCommand);
  this->SurfaceScalarCalculator->SetAttributeTypeToPointData();
  this->SurfaceScalarCalculator->SetResultArrayName("weights");
  this->SurfaceScalarCalculator->SetResultArrayType(VTK_FLOAT);
  this->SetSurfaceDistanceWeightingFunction("activeScalar");

  this->SurfaceScalarPassThroughFilter = vtkSmartPointer<vtkPassThroughFilter>::New();
  this->SurfaceScalarPassThroughFilter->SetInputConnection(this->SurfaceToLocalTransformer->GetOutputPort());

  this->CurveGenerator->SetCurveTypeToCardinalSpline();
  this->CurveGenerator->SetNumberOfPointsPerInterpolatingSegment(10);
  this->CurveGenerator->SetSurfaceCostFunctionType(vtkCjyxDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_DISTANCE);

  this->CurvePolyToWorldTransformer->SetInputConnection(this->CurveGenerator->GetOutputPort());

  this->ProjectPointsFilter = vtkSmartPointer<vtkProjectMarkupsCurvePointsFilter>::New();
  this->ProjectPointsFilter->SetInputCurveNode(this);
  this->ProjectPointsFilter->SetInputConnection(this->CurvePolyToWorldTransformer->GetOutputPort());

  vtkNew<vtkIntArray> events;
  events->InsertNextTuple1(vtkCommand::ModifiedEvent);
  events->InsertNextTuple1(vtkDMMLModelNode::MeshModifiedEvent);
  events->InsertNextTuple1(vtkDMMLTransformableNode::TransformModifiedEvent);
  this->AddNodeReferenceRole(this->GetSurfaceConstraintNodeReferenceRole(), this->GetSurfaceConstraintNodeReferenceDMMLAttributeName(), events);

  this->CurveMeasurementsCalculator = vtkSmartPointer<vtkCurveMeasurementsCalculator>::New();
  this->CurveMeasurementsCalculator->SetMeasurements(this->Measurements);
  this->CurveMeasurementsCalculator->SetInputConnection(this->ProjectPointsFilter->GetOutputPort());
  this->CurveMeasurementsCalculator->AddObserver(vtkCommand::ModifiedEvent, this->DMMLCallbackCommand);

  this->WorldOutput = vtkSmartPointer<vtkPassThroughFilter>::New();
  this->WorldOutput->SetInputConnection(this->CurveMeasurementsCalculator->GetOutputPort());

  this->CurveCoordinateSystemGeneratorWorld->SetInputConnection(this->WorldOutput->GetOutputPort());

  this->ScalarDisplayAssignAttribute = vtkSmartPointer<vtkAssignAttribute>::New();

  this->ShortestDistanceSurfaceActiveScalar = "";

  // Setup measurements calculated for this markup type
  vtkNew<vtkDMMLMeasurementLength> lengthMeasurement;
  lengthMeasurement->SetEnabled(false); // Length measurement is off by default to only show curve name
  lengthMeasurement->SetName("length");
  lengthMeasurement->SetInputDMMLNode(this);
  this->Measurements->AddItem(lengthMeasurement);

  vtkNew<vtkDMMLStaticMeasurement> curvatureMeanMeasurement;
  curvatureMeanMeasurement->SetName(this->CurveMeasurementsCalculator->GetMeanCurvatureName());
  curvatureMeanMeasurement->SetEnabled(false); // Curvature calculation is off by default
  this->Measurements->AddItem(curvatureMeanMeasurement);

  vtkNew<vtkDMMLStaticMeasurement> curvatureMaxMeasurement;
  curvatureMaxMeasurement->SetName(this->CurveMeasurementsCalculator->GetMaxCurvatureName());
  curvatureMaxMeasurement->SetEnabled(false); // Curvature calculation is off by default
  this->Measurements->AddItem(curvatureMaxMeasurement);

  this->CurvatureMeasurementModifiedCallbackCommand = vtkCallbackCommand::New();
  this->CurvatureMeasurementModifiedCallbackCommand->SetClientData( reinterpret_cast<void *>(this) );
  this->CurvatureMeasurementModifiedCallbackCommand->SetCallback( vtkDMMLMarkupsCurveNode::OnCurvatureMeasurementModified );
  curvatureMeanMeasurement->AddObserver(vtkCommand::ModifiedEvent, this->CurvatureMeasurementModifiedCallbackCommand);
  curvatureMaxMeasurement->AddObserver(vtkCommand::ModifiedEvent, this->CurvatureMeasurementModifiedCallbackCommand);
}

//----------------------------------------------------------------------------
vtkDMMLMarkupsCurveNode::~vtkDMMLMarkupsCurveNode()
{
  if (this->CurvatureMeasurementModifiedCallbackCommand)
    {
    this->CurvatureMeasurementModifiedCallbackCommand->SetClientData(nullptr);
    this->CurvatureMeasurementModifiedCallbackCommand->Delete();
    this->CurvatureMeasurementModifiedCallbackCommand = nullptr;
    }
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);

  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLEnumMacro(curveType, CurveType);
  vtkDMMLWriteXMLIntMacro(numberOfPointsPerInterpolatingSegment, NumberOfPointsPerInterpolatingSegment);
  vtkDMMLWriteXMLEnumMacro(surfaceCostFunctionType, SurfaceCostFunctionType);
  vtkDMMLWriteXMLStringMacro(surfaceDistanceWeightingFunction, SurfaceDistanceWeightingFunction);
  vtkDMMLWriteXMLFloatMacro(surfaceConstraintMaximumSearchRadiusTolerance, SurfaceConstraintMaximumSearchRadiusTolerance);
  vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::ReadXMLAttributes(const char** atts)
{
  DMMLNodeModifyBlocker blocker(this);

  this->Superclass::ReadXMLAttributes(atts);

  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLEnumMacro(curveType, CurveType);
  vtkDMMLReadXMLIntMacro(numberOfPointsPerInterpolatingSegment, NumberOfPointsPerInterpolatingSegment);
  vtkDMMLReadXMLEnumMacro(surfaceCostFunctionType, SurfaceCostFunctionType);
  vtkDMMLReadXMLStringMacro(surfaceDistanceWeightingFunction, SurfaceDistanceWeightingFunction);
  vtkDMMLReadXMLFloatMacro(surfaceConstraintMaximumSearchRadiusTolerance, SurfaceConstraintMaximumSearchRadiusTolerance);
  vtkDMMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkDMMLCopyBeginMacro(anode);
  vtkDMMLCopyEnumMacro(CurveType);
  vtkDMMLCopyIntMacro(NumberOfPointsPerInterpolatingSegment);
  vtkDMMLCopyEnumMacro(SurfaceCostFunctionType);
  vtkDMMLCopyStringMacro(SurfaceDistanceWeightingFunction);
  vtkDMMLCopyFloatMacro(SurfaceConstraintMaximumSearchRadiusTolerance);
  vtkDMMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkDMMLPrintBeginMacro(os, indent);
  vtkDMMLPrintEnumMacro(CurveType);
  vtkDMMLPrintIntMacro(NumberOfPointsPerInterpolatingSegment);
  vtkDMMLPrintEnumMacro(SurfaceCostFunctionType);
  vtkDMMLPrintStringMacro(SurfaceDistanceWeightingFunction);
  vtkDMMLPrintFloatMacro(SurfaceConstraintMaximumSearchRadiusTolerance);
  vtkDMMLPrintEndMacro();
}

//---------------------------------------------------------------------------
vtkPoints* vtkDMMLMarkupsCurveNode::GetCurvePointsWorld()
{
  vtkPolyData* curvePoly = this->GetCurveWorld();
  if (!curvePoly)
    {
    return nullptr;
    }
  return curvePoly->GetPoints();
}

//---------------------------------------------------------------------------
vtkPolyData* vtkDMMLMarkupsCurveNode::GetCurveWorld()
{
  if (this->GetNumberOfControlPoints() < 1)
    {
    return nullptr;
    }
  this->WorldOutput->Update();
  auto* curvePolyDataWorld = vtkPolyData::SafeDownCast(this->WorldOutput->GetOutput());
  this->TransformedCurvePolyLocator->SetDataSet(curvePolyDataWorld);
  return curvePolyDataWorld;
}

//----------------------------------------------------------------------
vtkAlgorithmOutput* vtkDMMLMarkupsCurveNode::GetCurveWorldConnection()
{
  return this->WorldOutput->GetOutputPort();
}

//---------------------------------------------------------------------------
double vtkDMMLMarkupsCurveNode::GetCurveLength(vtkPoints* curvePoints, bool closedCurve,
  vtkIdType startCurvePointIndex /*=0*/, vtkIdType numberOfCurvePoints /*=-1*/)
{
  if (!curvePoints || curvePoints->GetNumberOfPoints() < 2)
    {
    return 0.0;
    }
  if (startCurvePointIndex < 0)
    {
    vtkGenericWarningMacro("Invalid startCurvePointIndex=" << startCurvePointIndex << ", using 0 instead");
    startCurvePointIndex = 0;
    }
  vtkIdType lastCurvePointIndex = curvePoints->GetNumberOfPoints()-1;
  if (numberOfCurvePoints >= 0 && startCurvePointIndex + numberOfCurvePoints - 1 < lastCurvePointIndex)
    {
    lastCurvePointIndex = startCurvePointIndex + numberOfCurvePoints - 1;
    }

  double length = 0.0;
  double previousPoint[3] = { 0.0 };
  double nextPoint[3] = { 0.0 };
  curvePoints->GetPoint(startCurvePointIndex, previousPoint);
  for (vtkIdType curvePointIndex = startCurvePointIndex + 1; curvePointIndex <= lastCurvePointIndex; curvePointIndex++)
    {
    curvePoints->GetPoint(curvePointIndex, nextPoint);
    length += sqrt(vtkMath::Distance2BetweenPoints(previousPoint, nextPoint));
    previousPoint[0] = nextPoint[0];
    previousPoint[1] = nextPoint[1];
    previousPoint[2] = nextPoint[2];
    }
  // Add length of closing segment
  if (closedCurve && (numberOfCurvePoints < 0 || numberOfCurvePoints >= curvePoints->GetNumberOfPoints()))
    {
    curvePoints->GetPoint(0, nextPoint);
    length += sqrt(vtkMath::Distance2BetweenPoints(previousPoint, nextPoint));
    }
  return length;
}

//---------------------------------------------------------------------------
double vtkDMMLMarkupsCurveNode::GetCurveLengthWorld(
  vtkIdType startCurvePointIndex /*=0*/, vtkIdType numberOfCurvePoints /*=-1*/)
{
  vtkPoints* points = this->GetCurvePointsWorld();
  return vtkDMMLMarkupsCurveNode::GetCurveLength(points, this->CurveClosed,
    startCurvePointIndex, numberOfCurvePoints);
}

//---------------------------------------------------------------------------
double vtkDMMLMarkupsCurveNode::GetCurveLengthBetweenStartEndPointsWorld(vtkIdType startCurvePointIndex, vtkIdType endCurvePointIndex)
{
  if (startCurvePointIndex <= endCurvePointIndex)
  {
    return this->GetCurveLengthWorld(startCurvePointIndex, endCurvePointIndex - startCurvePointIndex + 1);
  }
  else
  {
    // wrap around
    return this->GetCurveLengthWorld(0, endCurvePointIndex + 1) + this->GetCurveLengthWorld(startCurvePointIndex, -1);
  }
}
//---------------------------------------------------------------------------
bool vtkDMMLMarkupsCurveNode::SetControlPointLabels(vtkStringArray* labels, vtkPoints* points)
{
  return this->SetControlPointLabelsWorld(labels, points);
}

//---------------------------------------------------------------------------
bool vtkDMMLMarkupsCurveNode::ConstrainPointsToSurface(vtkPoints* originalPoints, vtkPoints* normalVectors, vtkPolyData* surfacePolydata,
  vtkPoints* surfacePoints, double maximumSearchRadiusTolerance)
{
  // Convert normals from vtkPoints (the legacy interface) to vtkDoubleArray
  vtkNew<vtkDoubleArray> normalVectorsAsArray;
  normalVectorsAsArray->SetNumberOfComponents(3);
  for (vtkIdType i = 0; i < normalVectors->GetNumberOfPoints(); ++i)
    {
    normalVectorsAsArray->InsertNextTuple(normalVectors->GetPoint(i));
    }

  return vtkProjectMarkupsCurvePointsFilter::ConstrainPointsToSurface(originalPoints, normalVectorsAsArray, surfacePolydata,
    surfacePoints, maximumSearchRadiusTolerance);
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::ResampleCurveWorld(double controlPointDistance)
{
  vtkPoints* points = this->GetCurvePointsWorld();
  if (!points || points->GetNumberOfPoints() < 2)
    {
    return;
    }

  vtkNew<vtkPoints> interpolatedPoints;
  vtkNew<vtkDoubleArray> pedigreeIdsArray;
  vtkDMMLMarkupsCurveNode::ResamplePoints(points, interpolatedPoints, controlPointDistance, this->CurveClosed, pedigreeIdsArray);
  vtkDMMLMarkupsCurveNode::ResampleStaticControlPointMeasurements(this->Measurements, pedigreeIdsArray,
    this->CurveGenerator->GetNumberOfPointsPerInterpolatingSegment());

  vtkNew<vtkPoints> originalPoints;
  this->GetControlPointPositionsWorld(originalPoints);
  vtkNew<vtkStringArray> originalLabels;
  this->GetControlPointLabels(originalLabels);

  this->SetControlPointPositionsWorld(interpolatedPoints);
  this->SetControlPointLabelsWorld(originalLabels, originalPoints);
}

//---------------------------------------------------------------------------
bool vtkDMMLMarkupsCurveNode::ResampleStaticControlPointMeasurements(vtkCollection* measurements,
  vtkDoubleArray* curvePointsPedigreeIdsArray, int curvePointsPerControlPoint)
{
  if (!measurements || !curvePointsPedigreeIdsArray)
    {
    vtkGenericWarningMacro("vtkDMMLMarkupsCurveNode::ResampleCurveWorld: Invalid inputs");
    return false;
    }

  bool success = true;
  for (int index = 0; index < measurements->GetNumberOfItems(); ++index)
    {
    vtkDMMLStaticMeasurement* currentMeasurement = vtkDMMLStaticMeasurement::SafeDownCast(measurements->GetItemAsObject(index));
    if (!currentMeasurement)
      {
      continue;
      }
    vtkDoubleArray* controlPointValues = currentMeasurement->GetControlPointValues();
    if (!controlPointValues || controlPointValues->GetNumberOfTuples() < 2)
      {
      // no need to interpolate
      continue;
      }
    if (controlPointValues->GetNumberOfComponents() != 1)
      {
      vtkGenericWarningMacro("vtkDMMLMarkupsCurveNode::ResampleCurveWorld: "
        << "Only the interpolation of single component control point measurements is implemented");
      success = false;
      continue;
      }
    vtkNew<vtkDoubleArray> interpolatedMeasurement;
    interpolatedMeasurement->SetName(controlPointValues->GetName());
    vtkCurveMeasurementsCalculator::InterpolateArray(controlPointValues, interpolatedMeasurement, curvePointsPedigreeIdsArray, 1.0/curvePointsPerControlPoint);
    controlPointValues->DeepCopy(interpolatedMeasurement);
    }

  return success;
}

//---------------------------------------------------------------------------
bool vtkDMMLMarkupsCurveNode::ResamplePoints(vtkPoints* originalPoints, vtkPoints* sampledPoints,
  double samplingDistance, bool closedCurve, vtkDoubleArray* pedigreeIdsArray/*=nullptr*/)
{
  if (!originalPoints || !sampledPoints || samplingDistance <= 0)
    {
    vtkGenericWarningMacro("vtkDMMLMarkupsCurveNode::ResamplePoints failed: invalid inputs");
    return false;
    }

  if (pedigreeIdsArray)
    {
    pedigreeIdsArray->Initialize();
    }

  if (originalPoints->GetNumberOfPoints() < 2)
    {
    sampledPoints->DeepCopy(originalPoints);
    if (pedigreeIdsArray)
      {
      pedigreeIdsArray->InsertNextValue(0.0);
      pedigreeIdsArray->InsertNextValue(1.0);
      }
    return true;
    }

  double distanceFromLastSampledPoint = 0;
  double remainingSegmentLength = 0;
  double previousCurvePoint[3] = { 0.0 };
  originalPoints->GetPoint(0, previousCurvePoint);
  sampledPoints->Reset();
  sampledPoints->InsertNextPoint(previousCurvePoint);
  if (pedigreeIdsArray)
    {
    pedigreeIdsArray->Initialize();
    pedigreeIdsArray->InsertNextValue(0.0);
    }
  vtkIdType numberOfOriginalPoints = originalPoints->GetNumberOfPoints();
  bool addClosingSegment = closedCurve; // for closed curves, add a closing segment that connects last and first points
  double* currentCurvePoint = nullptr;
  for (vtkIdType originalPointIndex = 0; originalPointIndex < numberOfOriginalPoints || addClosingSegment; originalPointIndex++)
    {
    if (originalPointIndex >= numberOfOriginalPoints)
      {
      // this is the closing segment
      addClosingSegment = false;
      currentCurvePoint = originalPoints->GetPoint(0);
      }
    else
      {
      currentCurvePoint = originalPoints->GetPoint(originalPointIndex);
      }

    double segmentLength = sqrt(vtkMath::Distance2BetweenPoints(currentCurvePoint, previousCurvePoint));
    if (segmentLength <= 0.0)
      {
      continue;
      }
    remainingSegmentLength = distanceFromLastSampledPoint + segmentLength;
    if (remainingSegmentLength >= samplingDistance)
      {
      double segmentDirectionVector[3] =
        {
        (currentCurvePoint[0] - previousCurvePoint[0]) / segmentLength,
        (currentCurvePoint[1] - previousCurvePoint[1]) / segmentLength,
        (currentCurvePoint[2] - previousCurvePoint[2]) / segmentLength
        };
      // distance of new sampled point from previous curve point
      double distanceFromLastInterpolatedPoint = samplingDistance - distanceFromLastSampledPoint;
      while (remainingSegmentLength >= samplingDistance)
        {
        double newSampledPoint[3] =
          {
          previousCurvePoint[0] + segmentDirectionVector[0] * distanceFromLastInterpolatedPoint,
          previousCurvePoint[1] + segmentDirectionVector[1] * distanceFromLastInterpolatedPoint,
          previousCurvePoint[2] + segmentDirectionVector[2] * distanceFromLastInterpolatedPoint
          };
        sampledPoints->InsertNextPoint(newSampledPoint);
        distanceFromLastSampledPoint = 0;
        distanceFromLastInterpolatedPoint += samplingDistance;
        if (pedigreeIdsArray)
          {
          pedigreeIdsArray->InsertNextValue(originalPointIndex + distanceFromLastInterpolatedPoint/samplingDistance);
          }
        remainingSegmentLength -= samplingDistance;
        }
      distanceFromLastSampledPoint = remainingSegmentLength;
      }
    else
      {
      distanceFromLastSampledPoint += segmentLength;
      }
    previousCurvePoint[0] = currentCurvePoint[0];
    previousCurvePoint[1] = currentCurvePoint[1];
    previousCurvePoint[2] = currentCurvePoint[2];
    }

  // Make sure the resampled curve has the same size as the original
  // but avoid having very long or very short line segments at the end.
  if (closedCurve)
    {
    // Closed curve
    // Ideally, remainingSegmentLength would be equal to samplingDistance.
    if (remainingSegmentLength < samplingDistance * 0.5)
      {
      // last segment would be too short, so remove the last point and adjust position of second last point
      double lastPointPosition[3] = { 0.0 };
      vtkIdType lastPointOriginalPointIndex = 0;
      if (vtkDMMLMarkupsCurveNode::GetPositionAndClosestPointIndexAlongCurve(lastPointPosition, lastPointOriginalPointIndex,
        0, -(2.0*samplingDistance+remainingSegmentLength)/2.0, originalPoints, closedCurve))
        {
        sampledPoints->SetNumberOfPoints(sampledPoints->GetNumberOfPoints() - 1);
        sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1, lastPointPosition);
        if (pedigreeIdsArray)
          {
          pedigreeIdsArray->SetNumberOfValues(pedigreeIdsArray->GetNumberOfValues() - 1);
          pedigreeIdsArray->SetValue(pedigreeIdsArray->GetNumberOfValues() - 1, lastPointOriginalPointIndex);
          }
        }
      else
        {
        // something went wrong, we could not add a point, therefore just remove the last point
        sampledPoints->SetNumberOfPoints(sampledPoints->GetNumberOfPoints() - 1);
        if (pedigreeIdsArray)
          {
          pedigreeIdsArray->SetNumberOfValues(pedigreeIdsArray->GetNumberOfValues() - 1);
          }
        }
      }
    else
      {
      // last segment is only slightly shorter than the sampling distance
      // so just adjust the position of the last point
      double lastPointPosition[3] = { 0.0 };
      vtkIdType lastPointOriginalPointIndex = 0;
      if (vtkDMMLMarkupsCurveNode::GetPositionAndClosestPointIndexAlongCurve(lastPointPosition, lastPointOriginalPointIndex,
        0, -(samplingDistance+remainingSegmentLength)/2.0, originalPoints, closedCurve))
        {
        sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1, lastPointPosition);
        if (pedigreeIdsArray)
          {
          pedigreeIdsArray->SetValue(pedigreeIdsArray->GetNumberOfValues() - 1, lastPointOriginalPointIndex);
          }
        }
      }
    }
  else
    {
    // Open curve
    // Ideally, remainingSegmentLength would be equal to 0.
    if (remainingSegmentLength > samplingDistance * 0.5)
      {
      // last segment would be much longer than the sampling distance, so add an extra point
      double secondLastPointPosition[3] = { 0.0 };
      vtkIdType secondLastPointOriginalPointIndex = 0;
      if (vtkDMMLMarkupsCurveNode::GetPositionAndClosestPointIndexAlongCurve(secondLastPointPosition, secondLastPointOriginalPointIndex,
        originalPoints->GetNumberOfPoints() - 1, -(samplingDistance+remainingSegmentLength) / 2.0, originalPoints, closedCurve))
        {
        sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1, secondLastPointPosition);
        sampledPoints->InsertNextPoint(originalPoints->GetPoint(originalPoints->GetNumberOfPoints() - 1));
        if (pedigreeIdsArray)
          {
          pedigreeIdsArray->SetValue(pedigreeIdsArray->GetNumberOfValues() - 1, secondLastPointOriginalPointIndex);
          pedigreeIdsArray->InsertNextValue(originalPoints->GetNumberOfPoints() - 1);
          }
        }
      else
        {
        // something went wrong, we could not add a point, therefore just adjust the last point position
        sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1,
          originalPoints->GetPoint(originalPoints->GetNumberOfPoints() - 1));
        if (pedigreeIdsArray)
          {
          pedigreeIdsArray->SetValue(pedigreeIdsArray->GetNumberOfValues() - 1, originalPoints->GetNumberOfPoints() - 1);
          }
        }
      }
    else
      {
      // last segment is only slightly longer than the sampling distance
      // so we just adjust the position of last point
      sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1,
        originalPoints->GetPoint(originalPoints->GetNumberOfPoints() - 1));
      if (pedigreeIdsArray)
        {
        pedigreeIdsArray->SetValue(pedigreeIdsArray->GetNumberOfValues() - 1, originalPoints->GetNumberOfPoints() - 1);
        }
      }
    }

  return true;
}

//---------------------------------------------------------------------------
bool vtkDMMLMarkupsCurveNode::GetPositionAndClosestPointIndexAlongCurve(double foundCurvePosition[3], vtkIdType& foundClosestPointIndex,
  vtkIdType startCurvePointId, double distanceFromStartPoint, vtkPoints* curvePoints, bool closedCurve)
{
  vtkIdType numberOfCurvePoints = (curvePoints != nullptr ? curvePoints->GetNumberOfPoints() : 0);
  if (numberOfCurvePoints == 0)
    {
    vtkGenericWarningMacro("vtkDMMLMarkupsCurveNode::GetPositionAlongCurve failed: invalid input points");
    foundClosestPointIndex = -1;
    return false;
    }
  if (startCurvePointId < 0 || startCurvePointId >= numberOfCurvePoints)
    {
    vtkGenericWarningMacro("vtkDMMLMarkupsCurveNode::GetPositionAlongCurve failed: startCurvePointId is out of range");
    foundClosestPointIndex = -1;
    return false;
    }
  if (numberOfCurvePoints == 1 || distanceFromStartPoint == 0)
    {
    curvePoints->GetPoint(startCurvePointId, foundCurvePosition);
    foundClosestPointIndex = startCurvePointId;
    if (distanceFromStartPoint > 0.0)
      {
      vtkGenericWarningMacro("vtkDMMLMarkupsCurveNode::GetPositionAlongCurve failed: non-zero distance"
        " is requested but only 1 point is available");
      return false;
      }
    else
      {
      return true;
      }
    }
  vtkIdType idIncrement = (distanceFromStartPoint > 0 ? 1 : -1);
  double remainingDistanceFromStartPoint = abs(distanceFromStartPoint);
  double previousPoint[3] = { 0.0 };
  curvePoints->GetPoint(startCurvePointId, previousPoint);
  vtkIdType pointId = startCurvePointId;
  bool curveConfirmedToBeNonZeroLength = false;
  double lastSegmentLength = 0;
  while (true)
    {
    pointId += idIncrement;

    // if reach the end then wrap around for closed curve, terminate search for open curve
    if (pointId < 0 || pointId >= numberOfCurvePoints)
      {
      if (closedCurve)
        {
        if (!curveConfirmedToBeNonZeroLength)
          {
          if (vtkDMMLMarkupsCurveNode::GetCurveLength(curvePoints, closedCurve) == 0.0)
            {
            foundClosestPointIndex = -1;
            return false;
            }
          curveConfirmedToBeNonZeroLength = true;
          }
        pointId = (pointId < 0 ? numberOfCurvePoints : -1);
        continue;
        }
      else
        {
        // reached end of curve before getting at the requested distance
        // return closest
        foundClosestPointIndex = (pointId < 0 ? 0 : numberOfCurvePoints - 1);
        curvePoints->GetPoint(foundClosestPointIndex, foundCurvePosition);
        return false;
        }
      }

    // determine how much closer we are now
    double* nextPoint = curvePoints->GetPoint(pointId);
    lastSegmentLength = sqrt(vtkMath::Distance2BetweenPoints(nextPoint, previousPoint));
    remainingDistanceFromStartPoint -= lastSegmentLength;

    if (remainingDistanceFromStartPoint <= 0)
      {
      // reached the requested distance (and probably a bit more)
      for (int i=0; i<3; i++)
        {
        foundCurvePosition[i] = nextPoint[i] +
          remainingDistanceFromStartPoint * (nextPoint[i] - previousPoint[i]) / lastSegmentLength;
        }
      if (fabs(remainingDistanceFromStartPoint) <= fabs(remainingDistanceFromStartPoint + lastSegmentLength))
        {
        foundClosestPointIndex = pointId;
        }
      else
        {
        foundClosestPointIndex = pointId-1;
        }
      break;
      }

    previousPoint[0] = nextPoint[0];
    previousPoint[1] = nextPoint[1];
    previousPoint[2] = nextPoint[2];
    }
  return true;
}

//---------------------------------------------------------------------------
bool vtkDMMLMarkupsCurveNode::GetSampledCurvePointsBetweenStartEndPointsWorld(vtkPoints* sampledPoints,
  double samplingDistance, vtkIdType startCurvePointIndex, vtkIdType endCurvePointIndex)
{
  if (!sampledPoints || samplingDistance <= 0)
    {
    vtkGenericWarningMacro("vtkDMMLMarkupsCurveNode::GetSampledCurvePointsBetweenStartEndPoints failed: invalid inputs");
    return false;
    }
  vtkPoints* allPoints = this->GetCurvePointsWorld();
  if (!allPoints)
    {
    return false;
    }
  if (startCurvePointIndex < 0 || endCurvePointIndex >= allPoints->GetNumberOfPoints())
    {
    vtkGenericWarningMacro("vtkDMMLMarkupsCurveNode::GetSampledCurvePointsBetweenStartEndPoints failed: invalid inputs ("
    << "requested " << startCurvePointIndex << ".." << endCurvePointIndex << " range, but there are "
    << allPoints->GetNumberOfPoints() << " curve points)");
    return false;
    }
  vtkNew<vtkPoints> points;
  if (startCurvePointIndex <= endCurvePointIndex)
    {
    points->InsertPoints(0, endCurvePointIndex - startCurvePointIndex + 1, startCurvePointIndex, allPoints);
    }
  else
    {
    // wrap around
    vtkNew<vtkPoints> points;
    points->InsertPoints(0, allPoints->GetNumberOfPoints() - startCurvePointIndex, startCurvePointIndex, allPoints);
    points->InsertPoints(points->GetNumberOfPoints(), endCurvePointIndex + 1, 0, allPoints);
    }
  return vtkDMMLMarkupsCurveNode::ResamplePoints(points, sampledPoints, samplingDistance, this->CurveClosed);
}

//---------------------------------------------------------------------------
vtkIdType vtkDMMLMarkupsCurveNode::GetClosestCurvePointIndexToPositionWorld(const double posWorld[3])
{
  vtkPoints* points = this->GetCurvePointsWorld();
  if (!points)
    {
    return -1;
    }
  this->TransformedCurvePolyLocator->Update(); // or ->BuildLocator()?
  return this->TransformedCurvePolyLocator->FindClosestPoint(posWorld);
}

//---------------------------------------------------------------------------
vtkIdType vtkDMMLMarkupsCurveNode::GetCurvePointIndexFromControlPointIndex(int controlPointIndex)
{
  if (this->CurveGenerator->IsInterpolatingCurve())
    {
    return controlPointIndex * this->CurveGenerator->GetNumberOfPointsPerInterpolatingSegment();
    }
  else
    {
    double controlPointPositionWorld[3] = { 0.0 };
    this->GetNthControlPointPositionWorld(controlPointIndex, controlPointPositionWorld);
    return GetClosestCurvePointIndexToPositionWorld(controlPointPositionWorld);
    }
}

//---------------------------------------------------------------------------
bool vtkDMMLMarkupsCurveNode::GetCurveDirectionAtPointIndexWorld(vtkIdType curvePointIndex, double directionVectorWorld[3])
{
  vtkNew<vtkMatrix4x4> curvePointToWorld;
  if (!this->GetCurvePointToWorldTransformAtPointIndex(curvePointIndex, curvePointToWorld))
    {
    return false;
    }
  directionVectorWorld[0] = curvePointToWorld->GetElement(0, 2);
  directionVectorWorld[1] = curvePointToWorld->GetElement(1, 2);
  directionVectorWorld[2] = curvePointToWorld->GetElement(2, 2);
  return true;
}

//---------------------------------------------------------------------------
vtkIdType vtkDMMLMarkupsCurveNode::GetFarthestCurvePointIndexToPositionWorld(const double posWorld[3])
{
  vtkPoints* points = this->GetCurvePointsWorld();
  if (!points || points->GetNumberOfPoints()<1)
    {
    return false;
    }

  double farthestPoint[3] = { 0.0 };
  points->GetPoint(0, farthestPoint);
  double farthestPointDistance2 = vtkMath::Distance2BetweenPoints(posWorld, farthestPoint);
  vtkIdType farthestPointId = 0;

  vtkIdType numberOfPoints = points->GetNumberOfPoints();
  for (vtkIdType pointIndex = 1; pointIndex < numberOfPoints; pointIndex++)
    {
    double* nextPoint = points->GetPoint(pointIndex);
    double nextPointDistance2 = vtkMath::Distance2BetweenPoints(posWorld, nextPoint);
    if (nextPointDistance2 > farthestPointDistance2)
      {
      farthestPoint[0] = nextPoint[0];
      farthestPoint[1] = nextPoint[1];
      farthestPoint[2] = nextPoint[2];
      farthestPointDistance2 = nextPointDistance2;
      farthestPointId = pointIndex;
      }
    }

  return farthestPointId;
}

//---------------------------------------------------------------------------
vtkIdType vtkDMMLMarkupsCurveNode::GetCurvePointIndexAlongCurveWorld(vtkIdType startCurvePointId, double distanceFromStartPoint)
{
  vtkPoints* points = this->GetCurvePointsWorld();
  double foundCurvePosition[3] = { 0.0 };
  vtkIdType foundClosestPointIndex = -1;
  vtkDMMLMarkupsCurveNode::GetPositionAndClosestPointIndexAlongCurve(foundCurvePosition, foundClosestPointIndex,
    startCurvePointId, distanceFromStartPoint, points, this->CurveClosed);
  return foundClosestPointIndex;
}

//---------------------------------------------------------------------------
bool vtkDMMLMarkupsCurveNode::GetPositionAlongCurveWorld(double foundCurvePosition[3], vtkIdType startCurvePointId, double distanceFromStartPoint)
{
  vtkPoints* points = this->GetCurvePointsWorld();
  vtkIdType foundClosestPointIndex = -1;
  return vtkDMMLMarkupsCurveNode::GetPositionAndClosestPointIndexAlongCurve(foundCurvePosition, foundClosestPointIndex,
    startCurvePointId, distanceFromStartPoint, points, this->CurveClosed);
}

//---------------------------------------------------------------------------
bool vtkDMMLMarkupsCurveNode::GetPointsOnPlaneWorld(vtkPlane* plane, vtkPoints* intersectionPoints)
{
  if (!intersectionPoints)
    {
    return false;
    }
  intersectionPoints->Reset();
  if (!plane)
    {
    return false;
    }
  this->CurveGenerator->Update();
  vtkPolyData* curvePoly = this->GetCurveWorld();
  if (!curvePoly)
    {
    return true;
    }

  vtkNew<vtkCutter> cutEdges;
  cutEdges->SetInputData(curvePoly);
  cutEdges->SetCutFunction(plane);
  cutEdges->GenerateCutScalarsOff();
  cutEdges->SetValue(0, 0);
  cutEdges->Update();
  if (!cutEdges->GetOutput())
    {
    return true;
    }
  vtkPoints* points = cutEdges->GetOutput()->GetPoints();
  if (!points)
    {
    return true;
    }
  intersectionPoints->DeepCopy(points);
  return true;
}

//---------------------------------------------------------------------------
bool vtkDMMLMarkupsCurveNode::GetCurvePointToWorldTransformAtPointIndex(vtkIdType curvePointIndex, vtkMatrix4x4* curvePointToWorld)
{
  if (!curvePointToWorld)
    {
    vtkErrorMacro("vtkDMMLMarkupsCurveNode::GetCurvePointToWorldTransformAtPointIndex failed: Invalid curvePointToWorld");
    return false;
    }
  this->CurveGenerator->Update();
  this->CurveCoordinateSystemGeneratorWorld->Update();
  vtkPolyData* curvePoly = this->CurveCoordinateSystemGeneratorWorld->GetOutput();
  if (!curvePoly)
    {
    return false;
    }
  vtkIdType n = curvePoly->GetNumberOfPoints();
  if (curvePointIndex < 0 || curvePointIndex >= n)
    {
    vtkErrorMacro("vtkDMMLMarkupsCurveNode::GetCurvePointToWorldTransformAtPointIndex failed: Invalid curvePointIndex "
      << curvePointIndex << " (number of curve points: " << n << ")");
    return false;
    }
  curvePointToWorld->Identity();
  vtkPointData* pointData = curvePoly->GetPointData();
  if (!pointData)
    {
    return false;
    }
  vtkDoubleArray* normals = vtkDoubleArray::SafeDownCast(
    pointData->GetAbstractArray(this->CurveCoordinateSystemGeneratorWorld->GetNormalsArrayName()));
  vtkDoubleArray* binormals = vtkDoubleArray::SafeDownCast(
    pointData->GetAbstractArray(this->CurveCoordinateSystemGeneratorWorld->GetBinormalsArrayName()));
  vtkDoubleArray* tangents = vtkDoubleArray::SafeDownCast(
    pointData->GetAbstractArray(this->CurveCoordinateSystemGeneratorWorld->GetTangentsArrayName()));
  if (!tangents || !normals || !binormals)
    {
    return false;
    }
  double* normal = normals->GetTuple3(curvePointIndex);
  double* binormal = binormals->GetTuple3(curvePointIndex);
  double* tangent = tangents->GetTuple3(curvePointIndex);
  double* position = curvePoly->GetPoint(curvePointIndex);
  for (int row=0; row<3; row++)
    {
    curvePointToWorld->SetElement(row, 0, normal[row]);
    curvePointToWorld->SetElement(row, 1, binormal[row]);
    curvePointToWorld->SetElement(row, 2, tangent[row]);
    curvePointToWorld->SetElement(row, 3, position[row]);
    }
  return true;
}

//---------------------------------------------------------------------------
int vtkDMMLMarkupsCurveNode::GetCurveType()
{
  return this->CurveGenerator->GetCurveType();
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::SetCurveType(int type)
{
  this->CurveGenerator->SetCurveType(type);
}

//-----------------------------------------------------------
const char* vtkDMMLMarkupsCurveNode::GetCurveTypeAsString(int id)
{
  return this->CurveGenerator->GetCurveTypeAsString(id);
}

//-----------------------------------------------------------
int vtkDMMLMarkupsCurveNode::GetCurveTypeFromString(const char* name)
{
  return this->CurveGenerator->GetCurveTypeFromString(name);
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::SetCurveTypeToLinear()
{
  this->SetCurveType(vtkCurveGenerator::CURVE_TYPE_LINEAR_SPLINE);
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::SetCurveTypeToCardinalSpline()
{
  this->SetCurveType(vtkCurveGenerator::CURVE_TYPE_CARDINAL_SPLINE);
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::SetCurveTypeToKochanekSpline()
{
  this->SetCurveType(vtkCurveGenerator::CURVE_TYPE_KOCHANEK_SPLINE);
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::SetCurveTypeToPolynomial()
{
  this->SetCurveType(vtkCurveGenerator::CURVE_TYPE_POLYNOMIAL);
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::SetCurveTypeToShortestDistanceOnSurface(vtkDMMLModelNode* modelNode)
{
  DMMLNodeModifyBlocker blocker(this);
  this->SetCurveType(vtkCurveGenerator::CURVE_TYPE_SHORTEST_DISTANCE_ON_SURFACE);
  if (modelNode)
    {
    this->SetAndObserveSurfaceConstraintNode(modelNode);
    }
}

//---------------------------------------------------------------------------
int vtkDMMLMarkupsCurveNode::GetNumberOfPointsPerInterpolatingSegment()
{
  return this->CurveGenerator->GetNumberOfPointsPerInterpolatingSegment();
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::SetNumberOfPointsPerInterpolatingSegment(int pointsPerSegment)
{
  if (pointsPerSegment < 1)
    {
    vtkErrorMacro("vtkDMMLMarkupsCurveNode::SetNumberOfPointsPerInterpolatingSegment failed: minimum value is 1, attempted to set value " << pointsPerSegment);
    return;
    }
  this->CurveGenerator->SetNumberOfPointsPerInterpolatingSegment(pointsPerSegment);
}

//---------------------------------------------------------------------------
vtkIdType vtkDMMLMarkupsCurveNode::GetClosestPointPositionAlongCurveWorld(const double posWorld[3], double closestPosWorld[3])
{
  vtkPoints* points = this->GetCurvePointsWorld();
  if (!points || points->GetNumberOfPoints() < 1)
    {
    return -1;
    }
  if (points->GetNumberOfPoints() == 1)
    {
    points->GetPoint(0, closestPosWorld);
    return -1;
    }

  // Find closest curve point
  vtkIdType closestCurvePointIndex = this->GetClosestCurvePointIndexToPositionWorld(posWorld);
  if (closestCurvePointIndex < 0)
    {
    return -1;
    }
  double closestCurvePoint[3] = { 0.0 };
  points->GetPoint(closestCurvePointIndex, closestCurvePoint);
  double closestDistance2 = vtkMath::Distance2BetweenPoints(posWorld, closestPosWorld);
  closestPosWorld[0] = closestCurvePoint[0];
  closestPosWorld[1] = closestCurvePoint[1];
  closestPosWorld[2] = closestCurvePoint[2];
  vtkIdType lineIndex = closestCurvePointIndex;

  // See if we can find any points closer along the curve
  double relativePositionAlongLine = -1.0; // between 0.0-1.0 if between the endpoints of the line segment
  double otherPoint[3] = { 0.0 };
  double closestPointOnLine[3] = { 0.0 };
  if (closestCurvePointIndex - 1 >= 0)
    {
    points->GetPoint(closestCurvePointIndex - 1, otherPoint);
    double distance2 = vtkLine::DistanceToLine(posWorld, closestCurvePoint, otherPoint, relativePositionAlongLine, closestPointOnLine);
    if (distance2 < closestDistance2 && relativePositionAlongLine >= 0 && relativePositionAlongLine <= 1)
      {
      closestDistance2 = distance2;
      closestPosWorld[0] = closestPointOnLine[0];
      closestPosWorld[1] = closestPointOnLine[1];
      closestPosWorld[2] = closestPointOnLine[2];
      lineIndex = closestCurvePointIndex - 1;
      }
    }
  if (closestCurvePointIndex + 1 < points->GetNumberOfPoints())
    {
    points->GetPoint(closestCurvePointIndex + 1, otherPoint);
    double distance2 = vtkLine::DistanceToLine(posWorld, closestCurvePoint, otherPoint, relativePositionAlongLine, closestPointOnLine);
    if (distance2 < closestDistance2 && relativePositionAlongLine >= 0 && relativePositionAlongLine <= 1)
      {
      closestDistance2 = distance2;
      closestPosWorld[0] = closestPointOnLine[0];
      closestPosWorld[1] = closestPointOnLine[1];
      closestPosWorld[2] = closestPointOnLine[2];
      lineIndex = closestCurvePointIndex;
      }
    }
  return lineIndex;
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::UpdateMeasurementsInternal()
{
  // Execute curve measurements calculator (curvature, interpolate control point measurements
  // and store the results in the curve poly data points as scalars for visualization)
  if (this->CurveMeasurementsCalculator && this->GetNumberOfControlPoints() > 1)
    {
    // Update curvature unit (only do it if a curve measurement is enabled)
    vtkDMMLMeasurement* curvatureMeanMeasurement = this->GetMeasurement(this->CurveMeasurementsCalculator->GetMeanCurvatureName());
    vtkDMMLMeasurement* curvatureMaxMeasurement = this->GetMeasurement(this->CurveMeasurementsCalculator->GetMaxCurvatureName());
    if ( (curvatureMeanMeasurement && curvatureMeanMeasurement->GetEnabled())
      || (curvatureMaxMeasurement && curvatureMaxMeasurement->GetEnabled()))
      {
      std::string inverseLengthUnit = "mm-1";
      vtkDMMLUnitNode* lengthUnitNode = this->GetUnitNode("length");
      if (lengthUnitNode && lengthUnitNode->GetSuffix())
        {
        inverseLengthUnit = std::string(lengthUnitNode->GetSuffix()) + "-1";
        }
      this->CurveMeasurementsCalculator->SetCurvatureUnits(inverseLengthUnit);
      }
    this->CurveMeasurementsCalculator->Update();
    }

  Superclass::UpdateMeasurementsInternal();
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::ProcessDMMLEvents(vtkObject* caller,
                                             unsigned long event,
                                             void* callData)
{
  if (event == vtkDMMLTransformableNode::TransformModifiedEvent)
    {
    this->OnSurfaceModelTransformChanged();
    }
  else if (caller == this->SurfaceScalarCalculator.GetPointer())
    {
    this->UpdateAllMeasurements();
    int n = -1;
    this->InvokeCustomModifiedEvent(vtkDMMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
    this->StorableModifiedTime.Modified();
    }
  else if (caller == this->CurveGenerator.GetPointer())
    {
    int surfaceCostFunctionType = this->CurveGenerator->GetSurfaceCostFunctionType();
    // Change the pass through filter input depending on if we need the scalar values.
    // Trying to run SurfaceScalarCalculator without an active scalar will result in an error message.
    if (surfaceCostFunctionType == vtkCjyxDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_DISTANCE)
      {
      this->SurfaceScalarPassThroughFilter->SetInputConnection(this->SurfaceToLocalTransformer->GetOutputPort());
      }
    else
      {
      this->SurfaceScalarPassThroughFilter->SetInputConnection(this->SurfaceScalarCalculator->GetOutputPort());
      }
    }
  else if (caller == this->CurveMeasurementsCalculator.GetPointer())
    {
    vtkDMMLMarkupsDisplayNode* displayNode = this->GetMarkupsDisplayNode();
    if (displayNode)
      {
      this->InvokeEvent(vtkDMMLDisplayableNode::DisplayModifiedEvent, displayNode);
      }
    }

  if (caller == this->GetNodeReference(this->GetSurfaceConstraintNodeReferenceRole()))
    {
    this->OnSurfaceModelNodeChanged();
    }

  Superclass::ProcessDMMLEvents(caller, event, callData);
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::OnNodeReferenceAdded(vtkDMMLNodeReference* reference)
{
  if (strcmp(reference->GetReferenceRole(), this->GetSurfaceConstraintNodeReferenceRole()) == 0 ||
      strcmp(reference->GetReferenceRole(), this->TransformNodeReferenceRole) == 0)
    {
    this->OnSurfaceModelTransformChanged();
    this->OnSurfaceModelNodeChanged();
    }

  Superclass::OnNodeReferenceAdded(reference);
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::OnNodeReferenceModified(vtkDMMLNodeReference* reference)
{
  if (strcmp(reference->GetReferenceRole(), this->GetSurfaceConstraintNodeReferenceRole()) == 0 ||
      strcmp(reference->GetReferenceRole(), this->TransformNodeReferenceRole) == 0)
    {
    this->OnSurfaceModelTransformChanged();
    this->OnSurfaceModelNodeChanged();
    }

  Superclass::OnNodeReferenceModified(reference);
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::OnNodeReferenceRemoved(vtkDMMLNodeReference* reference)
{
  if (strcmp(reference->GetReferenceRole(), this->GetSurfaceConstraintNodeReferenceRole()) == 0 ||
      strcmp(reference->GetReferenceRole(), this->TransformNodeReferenceRole) == 0)
    {
    this->OnSurfaceModelTransformChanged();
    this->OnSurfaceModelNodeChanged();
    }
  Superclass::OnNodeReferenceRemoved(reference);
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::SetAndObserveSurfaceConstraintNode(vtkDMMLModelNode* modelNode)
{
  this->SetAndObserveNodeReferenceID(this->GetSurfaceConstraintNodeReferenceRole(), modelNode ? modelNode->GetID() : nullptr);
}

//---------------------------------------------------------------------------
vtkDMMLModelNode* vtkDMMLMarkupsCurveNode::GetSurfaceConstraintNode()
{
  return vtkDMMLModelNode::SafeDownCast(this->GetNodeReference(this->GetSurfaceConstraintNodeReferenceRole()));
}

//---------------------------------------------------------------------------
int vtkDMMLMarkupsCurveNode::GetSurfaceCostFunctionType()
{
  return this->CurveGenerator->GetSurfaceCostFunctionType();
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::SetSurfaceCostFunctionType(int surfaceCostFunctionType)
{
  this->CurveGenerator->SetSurfaceCostFunctionType(surfaceCostFunctionType);
}

//---------------------------------------------------------------------------
const char* vtkDMMLMarkupsCurveNode::GetSurfaceCostFunctionTypeAsString(int surfaceCostFunctionType)
{
  return vtkCjyxDijkstraGraphGeodesicPath::GetCostFunctionTypeAsString(surfaceCostFunctionType);
}

//---------------------------------------------------------------------------
int vtkDMMLMarkupsCurveNode::GetSurfaceCostFunctionTypeFromString(const char* surfaceCostFunctionTypeName)
{
  return vtkCjyxDijkstraGraphGeodesicPath::GetCostFunctionTypeFromString(surfaceCostFunctionTypeName);
}

//---------------------------------------------------------------------------
const char* vtkDMMLMarkupsCurveNode::GetSurfaceDistanceWeightingFunction()
{
  return this->SurfaceScalarCalculator->GetFunction();
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::SetSurfaceDistanceWeightingFunction(const char* function)
{
  const char* currentFunction = this->SurfaceScalarCalculator->GetFunction();
  if ((currentFunction && function && strcmp(this->SurfaceScalarCalculator->GetFunction(), function) == 0) ||
    (currentFunction == nullptr && function == nullptr))
    {
    return;
    }
  this->SurfaceScalarCalculator->SetFunction(function);
  this->UpdateSurfaceScalarVariables();
  this->UpdateAllMeasurements();
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::OnSurfaceModelNodeChanged()
{
  this->UpdateSurfaceScalarVariables();

  vtkDMMLModelNode* modelNode = this->GetSurfaceConstraintNode();
  if (modelNode)
    {
    this->CleanFilter->SetInputConnection(modelNode->GetPolyDataConnection());
    this->CurveGenerator->SetInputConnection(1, this->SurfaceScalarPassThroughFilter->GetOutputPort());
    }
  else
    {
    this->CleanFilter->RemoveAllInputs();
    this->CurveGenerator->RemoveInputConnection(1, this->SurfaceScalarPassThroughFilter->GetOutputPort());
    }
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::OnSurfaceModelTransformChanged()
{
  vtkDMMLModelNode* modelNode = this->GetSurfaceConstraintNode();
  if (!modelNode)
    {
    return;
    }

  vtkSmartPointer<vtkGeneralTransform> surfaceToLocalTransform = vtkGeneralTransform::SafeDownCast(
    this->SurfaceToLocalTransformer->GetTransform());
  if (!surfaceToLocalTransform)
    {
    surfaceToLocalTransform = vtkSmartPointer<vtkGeneralTransform>::New();
    this->SurfaceToLocalTransformer->SetTransform(surfaceToLocalTransform);
    }

  vtkDMMLTransformNode::GetTransformBetweenNodes(modelNode->GetParentTransformNode(), this->GetParentTransformNode(), surfaceToLocalTransform);
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::UpdateSurfaceScalarVariables()
{
  vtkDMMLModelNode* modelNode = this->GetSurfaceConstraintNode();
  if (!modelNode)
    {
    return;
    }

  vtkPolyData* polyData = modelNode->GetPolyData();
  if (!polyData)
    {
    return;
    }

  vtkPointData* pointData = polyData->GetPointData();
  if (!pointData)
    {
    return;
    }

  const char* activeScalarName = modelNode->GetActivePointScalarName(vtkDataSetAttributes::SCALARS);
  bool activeScalarChanged = false;
  if (!activeScalarName && this->ShortestDistanceSurfaceActiveScalar)
    {
    activeScalarChanged = true;
    }
  else if (activeScalarName && !this->ShortestDistanceSurfaceActiveScalar)
    {
    activeScalarChanged = true;
    }
  else if (activeScalarName && this->ShortestDistanceSurfaceActiveScalar && strcmp(activeScalarName, this->ShortestDistanceSurfaceActiveScalar) != 0)
    {
    activeScalarChanged = true;
    }
  this->ShortestDistanceSurfaceActiveScalar = activeScalarName;

  int numberOfArraysInMesh = pointData->GetNumberOfArrays();
  int numberOfArraysInCalculator = this->SurfaceScalarCalculator->GetNumberOfScalarArrays();
  if (!activeScalarChanged && numberOfArraysInMesh + 1 == numberOfArraysInCalculator)
    {
    return;
    }

  this->SurfaceScalarCalculator->RemoveAllVariables();
  for (int i = -1; i < numberOfArraysInMesh; ++i)
    {
    const char* variableName = "activeScalar";
    vtkDataArray* array = nullptr;
    if (i >= 0)
      {
      array = pointData->GetArray(i);
      variableName = array->GetName();
      }
    else
      {
      if (!activeScalarName)
        {
        continue;
        }
      array = pointData->GetArray(activeScalarName);
      }

    if (!array)
      {
      vtkWarningMacro("UpdateSurfaceScalarVariables: Could not get array " << i);
      continue;
      }

    this->SurfaceScalarCalculator->AddScalarVariable(variableName, array->GetName());
    }

  // Changing the variables doesn't invoke modified, so we need to invoke it here.
  this->SurfaceScalarCalculator->Modified();
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::UpdateAssignedAttribute()
{
  // TODO: This method only works well if there is a single display node
  // (or all display nodes use the same scalar display settings).
  vtkDMMLMarkupsDisplayNode* displayNode = this->GetMarkupsDisplayNode();
  if (!displayNode)
    {
    return;
    }

  this->ScalarDisplayAssignAttribute->Assign(
    displayNode->GetActiveScalarName(),
    vtkDataSetAttributes::SCALARS,
    displayNode->GetActiveAttributeLocation() >= 0 ? displayNode->GetActiveAttributeLocation() : vtkAssignAttribute::POINT_DATA);

  // Connect assign attributes filter if scalar visibility is on
  if (displayNode->GetScalarVisibility())
    {
    this->ScalarDisplayAssignAttribute->SetInputConnection(this->CurveMeasurementsCalculator->GetOutputPort());
    this->WorldOutput->SetInputConnection(this->ScalarDisplayAssignAttribute->GetOutputPort());
    }
  else
    {
    this->ScalarDisplayAssignAttribute->RemoveAllInputConnections(0);
    this->WorldOutput->SetInputConnection(this->CurveMeasurementsCalculator->GetOutputPort());
    }
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::OnCurvatureMeasurementModified(
  vtkObject* caller, unsigned long vtkNotUsed(eid), void* clientData, void* vtkNotUsed(callData))
{
  vtkDMMLMarkupsCurveNode* self = reinterpret_cast<vtkDMMLMarkupsCurveNode*>(clientData);
  vtkDMMLStaticMeasurement* measurement = reinterpret_cast<vtkDMMLStaticMeasurement*>(caller);
  if (!self || !measurement)
    {
    return;
    }

  if (!measurement->GetEnabled())
    {
    // measurement is disabled
    measurement->ClearValue();
    if (!self->CurveMeasurementsCalculator->GetCalculateCurvature())
      {
      // no need to compute and it was not computed, nothing to do
      return;
      }
    // Disable curve measurement calculator if no curvature metric is needed anymore
    bool isCurvatureComputationNeeded = false;
    for (int index = 0; index < self->Measurements->GetNumberOfItems(); ++index)
      {
      vtkDMMLMeasurement* currentMeasurement = vtkDMMLMeasurement::SafeDownCast(self->Measurements->GetItemAsObject(index));
      if (currentMeasurement->GetEnabled()
        && (currentMeasurement->GetName() == self->CurveMeasurementsCalculator->GetMeanCurvatureName()
          || currentMeasurement->GetName() == self->CurveMeasurementsCalculator->GetMaxCurvatureName()))
        {
        isCurvatureComputationNeeded = true;
        break;
        }
      }
    if (!isCurvatureComputationNeeded)
      {
      self->CurveMeasurementsCalculator->SetCalculateCurvature(false);
      self->CurveMeasurementsCalculator->Update();
      }
    return;
    }

  // measurement is enabled
  if (self->CurveMeasurementsCalculator->GetCalculateCurvature() && measurement->GetValueDefined())
    {
    // measurement was already on, nothing to do
    return;
    }

  // trigger a recompute
  self->CurveMeasurementsCalculator->SetCalculateCurvature(true);
  self->CurveMeasurementsCalculator->Update();
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsCurveNode::SetSurfaceConstraintMaximumSearchRadiusTolerance(double tolerance)
{
  if (vtkMathUtilities::FuzzyCompare<double>(this->GetSurfaceConstraintMaximumSearchRadiusTolerance(), tolerance))
    {
    return;
    }
  this->ProjectPointsFilter->SetMaximumSearchRadiusTolerance(tolerance);
  this->Modified();
}

//---------------------------------------------------------------------------
double vtkDMMLMarkupsCurveNode::GetSurfaceConstraintMaximumSearchRadiusTolerance() const
{
  return this->ProjectPointsFilter->GetMaximumSearchRadiusTolerance();
}
