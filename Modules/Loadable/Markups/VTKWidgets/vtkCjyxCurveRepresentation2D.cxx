/*=========================================================================

 Copyright (c) ProxSim ltd., Kwun Tong, Hong Kong. All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 This file was originally developed by Davide Punzo, punzodavide@hotmail.it,
 and development was supported by ProxSim ltd.

=========================================================================*/

// VTK includes
#include "vtkCellLocator.h"
#include "vtkCleanPolyData.h"
#include "vtkDiscretizableColorTransferFunction.h"
#include "vtkLine.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPlane.h"
#include "vtkPoints.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkProperty2D.h"
#include "vtkRenderer.h"
#include "vtkSampleImplicitFunctionFilter.h"
#include "vtkCjyxCurveRepresentation2D.h"
#include "vtkTextActor.h"
#include "vtkTextProperty.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTubeFilter.h"

// DMML includes
#include "vtkDMMLInteractionEventData.h"
#include "vtkDMMLMarkupsDisplayNode.h"
#include "vtkDMMLProceduralColorNode.h"

vtkStandardNewMacro(vtkCjyxCurveRepresentation2D);

//----------------------------------------------------------------------
vtkCjyxCurveRepresentation2D::vtkCjyxCurveRepresentation2D()
{
  this->Line = vtkSmartPointer<vtkPolyData>::New();

  this->SliceDistance = vtkSmartPointer<vtkSampleImplicitFunctionFilter>::New();
  this->SliceDistance->SetImplicitFunction(this->SlicePlane);

  this->WorldToSliceTransformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->WorldToSliceTransformer->SetTransform(this->WorldToSliceTransform);
  this->WorldToSliceTransformer->SetInputConnection(this->SliceDistance->GetOutputPort());

  this->TubeFilter = vtkSmartPointer<vtkTubeFilter>::New();

  vtkNew<vtkCleanPolyData> cleaner;
  cleaner->PointMergingOn();
  cleaner->SetInputConnection(this->WorldToSliceTransformer->GetOutputPort());
  this->TubeFilter->SetInputConnection(cleaner->GetOutputPort());
  this->TubeFilter->SetNumberOfSides(6);
  this->TubeFilter->SetRadius(1);

  this->LineColorMap = vtkSmartPointer<vtkDiscretizableColorTransferFunction>::New();

  this->LineMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  this->LineMapper->SetInputConnection(this->TubeFilter->GetOutputPort());
  this->LineMapper->SetLookupTable(this->LineColorMap);
  this->LineMapper->SetScalarVisibility(true);

  this->LineActor = vtkSmartPointer<vtkActor2D>::New();
  this->LineActor->SetMapper(this->LineMapper);
  this->LineActor->SetProperty(this->GetControlPointsPipeline(Unselected)->Property);

  this->SliceCurvePointLocator = vtkSmartPointer<vtkCellLocator>::New();
}

//----------------------------------------------------------------------
vtkCjyxCurveRepresentation2D::~vtkCjyxCurveRepresentation2D() = default;

//----------------------------------------------------------------------
void vtkCjyxCurveRepresentation2D::UpdateFromDMML(vtkDMMLNode* caller, unsigned long event, void *callData /*=nullptr*/)
{
  Superclass::UpdateFromDMML(caller, event, callData);

  this->NeedToRenderOn();

  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || !this->IsDisplayable())
    {
    this->VisibilityOff();
    return;
    }
  this->VisibilityOn();

  // Line display

  double diameter = ( this->MarkupsDisplayNode->GetCurveLineSizeMode() == vtkDMMLMarkupsDisplayNode::UseLineDiameter ?
    this->MarkupsDisplayNode->GetLineDiameter() / this->ViewScaleFactorMmPerPixel : this->ControlPointSize * this->MarkupsDisplayNode->GetLineThickness() );
  this->TubeFilter->SetRadius(diameter * 0.5);

  this->LineActor->SetVisibility(markupsNode->GetNumberOfControlPoints() >= 2);

  // Hide the line actor if it doesn't intersect the current slice
  this->SliceDistance->Update();
  if (!this->IsRepresentationIntersectingSlice(vtkPolyData::SafeDownCast(this->SliceDistance->GetOutput()), this->SliceDistance->GetScalarArrayName()))
    {
    this->LineActor->SetVisibility(false);
    }

  bool allControlPointsSelected = this->GetAllControlPointsSelected();
  int controlPointType = Active;
  if (this->MarkupsDisplayNode->GetActiveComponentType() != vtkDMMLMarkupsDisplayNode::ComponentLine)
    {
    controlPointType = allControlPointsSelected ? Selected : Unselected;
    }
  this->LineActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->Property);
  this->TextActor->SetTextProperty(this->GetControlPointsPipeline(controlPointType)->TextProperty);

  if (this->MarkupsDisplayNode->GetLineColorNode() && this->MarkupsDisplayNode->GetLineColorNode()->GetColorTransferFunction())
    {
    // Update the line color mapping from the colorNode stored in the markups display node
    this->LineMapper->SetLookupTable(this->MarkupsDisplayNode->GetLineColorNode()->GetColorTransferFunction());
    }
  else
    {
    // if there is no line color node, build the color mapping from few variables
    // (color, opacity, distance fading, saturation and hue offset) stored in the display node
    this->UpdateDistanceColorMap(this->LineColorMap, this->LineActor->GetProperty()->GetColor());
    this->LineMapper->SetLookupTable(this->LineColorMap);
    }

  bool allNodesHidden = true;
  for (int controlPointIndex = 0; controlPointIndex < markupsNode->GetNumberOfControlPoints(); controlPointIndex++)
    {
    if (markupsNode->GetNthControlPointPositionVisibility(controlPointIndex)
      && (markupsNode->GetNthControlPointVisibility(controlPointIndex)))
      {
      allNodesHidden = false;
      break;
      }
    }

  // Display center position
  // It would be cleaner to use a dedicated actor for this instead of using the control points actors
  // as it would give flexibility in what glyph we use, it would not interfere with active control point display, etc.
  if (this->CurveClosed && markupsNode->GetNumberOfControlPoints() > 2 && this->CenterVisibilityOnSlice && !allNodesHidden)
    {
    double centerPosWorld[3], centerPosDisplay[3], orient[3] = { 0 };
    markupsNode->GetCenterOfRotationWorld(centerPosWorld);
    this->GetWorldToSliceCoordinates(centerPosWorld, centerPosDisplay);
    int centerControlPointType = allControlPointsSelected ? Selected : Unselected;
    if (this->MarkupsDisplayNode->GetActiveComponentType() == vtkDMMLMarkupsDisplayNode::ComponentCenterPoint)
      {
      centerControlPointType = Active;
      this->GetControlPointsPipeline(centerControlPointType)->ControlPoints->SetNumberOfPoints(0);
      this->GetControlPointsPipeline(centerControlPointType)->ControlPointsPolyData->GetPointData()->GetNormals()->SetNumberOfTuples(0);
      }
    this->GetControlPointsPipeline(centerControlPointType)->ControlPoints->InsertNextPoint(centerPosDisplay);
    this->GetControlPointsPipeline(centerControlPointType)->ControlPointsPolyData->GetPointData()->GetNormals()->InsertNextTuple(orient);

    this->GetControlPointsPipeline(centerControlPointType)->ControlPoints->Modified();
    this->GetControlPointsPipeline(centerControlPointType)->ControlPointsPolyData->GetPointData()->GetNormals()->Modified();
    this->GetControlPointsPipeline(centerControlPointType)->ControlPointsPolyData->Modified();
    if (centerControlPointType == Active)
      {
      this->GetControlPointsPipeline(centerControlPointType)->Actor->VisibilityOn();
      this->GetControlPointsPipeline(centerControlPointType)->LabelsActor->VisibilityOff();
      }
    }

    // Properties label display

  if (this->MarkupsDisplayNode->GetPropertiesLabelVisibility()
    && this->AnyPointVisibilityOnSlice
    && markupsNode->GetNumberOfDefinedControlPoints(true) > 0) // including preview
    {
    int controlPointIndex = 0;
    int numberOfDefinedControlPoints = markupsNode->GetNumberOfDefinedControlPoints(); // excluding previewed point
    if (numberOfDefinedControlPoints > 0)
      {
      // there is at least one placed point
      controlPointIndex = markupsNode->GetNthControlPointIndexByPositionStatus((numberOfDefinedControlPoints - 1) / 2, vtkDMMLMarkupsNode::PositionDefined);
      }
    else
      {
      // we only have a preview point
      controlPointIndex = markupsNode->GetNthControlPointIndexByPositionStatus(0, vtkDMMLMarkupsNode::PositionPreview);
      }
    double textPos[3] = { 0.0,  0.0, 0.0 };
    this->GetNthControlPointDisplayPosition(controlPointIndex, textPos);
    this->TextActor->SetDisplayPosition(static_cast<int>(textPos[0]), static_cast<int>(textPos[1]));
    this->TextActor->SetVisibility(true);
    }
  else
    {
    this->TextActor->SetVisibility(false);
    }
}

//----------------------------------------------------------------------
void vtkCjyxCurveRepresentation2D::CanInteract(
  vtkDMMLInteractionEventData* interactionEventData,
  int &foundComponentType, int &foundComponentIndex, double &closestDistance2)
{
  foundComponentType = vtkDMMLMarkupsDisplayNode::ComponentNone;
  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if ( !markupsNode || markupsNode->GetLocked() || markupsNode->GetNumberOfControlPoints() < 1
    || !interactionEventData )
    {
    return;
    }
  Superclass::CanInteract(interactionEventData, foundComponentType, foundComponentIndex, closestDistance2);
  if (foundComponentType != vtkDMMLMarkupsDisplayNode::ComponentNone)
    {
    // if mouse is near a control point then select that (ignore the line)
    return;
    }

  this->CanInteractWithCurve(interactionEventData, foundComponentType, foundComponentIndex, closestDistance2);
}

//----------------------------------------------------------------------
void vtkCjyxCurveRepresentation2D::GetActors(vtkPropCollection *pc)
{
  this->LineActor->GetActors(pc);
  this->Superclass::GetActors(pc);
}

//----------------------------------------------------------------------
void vtkCjyxCurveRepresentation2D::ReleaseGraphicsResources(
  vtkWindow *win)
{
  this->LineActor->ReleaseGraphicsResources(win);
  this->Superclass::ReleaseGraphicsResources(win);
}

//----------------------------------------------------------------------
int vtkCjyxCurveRepresentation2D::RenderOverlay(vtkViewport *viewport)
{
  int count=0;
  if (this->LineActor->GetVisibility())
    {
    count +=  this->LineActor->RenderOverlay(viewport);
    }
  count += this->Superclass::RenderOverlay(viewport);

  return count;
}

//-----------------------------------------------------------------------------
int vtkCjyxCurveRepresentation2D::RenderOpaqueGeometry(
  vtkViewport *viewport)
{
  int count=0;
  if (this->LineActor->GetVisibility())
    {
    count += this->LineActor->RenderOpaqueGeometry(viewport);
    }
  count += this->Superclass::RenderOpaqueGeometry(viewport);

  return count;
}

//-----------------------------------------------------------------------------
int vtkCjyxCurveRepresentation2D::RenderTranslucentPolygonalGeometry(
  vtkViewport *viewport)
{
  int count=0;
  if (this->LineActor->GetVisibility())
    {
    count += this->LineActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  count += this->Superclass::RenderTranslucentPolygonalGeometry(viewport);

  return count;
}

//-----------------------------------------------------------------------------
vtkTypeBool vtkCjyxCurveRepresentation2D::HasTranslucentPolygonalGeometry()
{
  if (this->Superclass::HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  if (this->LineActor->GetVisibility() && this->LineActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------
double *vtkCjyxCurveRepresentation2D::GetBounds()
{
  return nullptr;
}

//-----------------------------------------------------------------------------
void vtkCjyxCurveRepresentation2D::PrintSelf(ostream& os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);

  if (this->LineActor)
    {
    os << indent << "Line Actor Visibility: " << this->LineActor->GetVisibility() << "\n";
    }
  else
    {
    os << indent << "Line Actor: (none)\n";
    }
}

//-----------------------------------------------------------------------------
void vtkCjyxCurveRepresentation2D::SetMarkupsNode(vtkDMMLMarkupsNode *markupsNode)
{
  if (this->MarkupsNode != markupsNode)
    {
    if (markupsNode)
      {
      this->SliceDistance->SetInputConnection(markupsNode->GetCurveWorldConnection());
      }
    else
      {
      this->SliceDistance->SetInputData(this->Line);
      }
    }
  this->Superclass::SetMarkupsNode(markupsNode);
}

//----------------------------------------------------------------------
void vtkCjyxCurveRepresentation2D::CanInteractWithCurve(
  vtkDMMLInteractionEventData* interactionEventData,
  int &foundComponentType, int &componentIndex, double &closestDistance2)
{
  vtkDMMLSliceNode *sliceNode = this->GetSliceNode();
  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if ( !sliceNode || !markupsNode || markupsNode->GetLocked() || markupsNode->GetNumberOfControlPoints() < 2
    || !this->GetVisibility() || !interactionEventData
    || !this->MarkupsDisplayNode)
    {
    return;
    }

  this->SliceDistance->Update();
  // No points to find. Trying to run the locator would result in a crash.
  vtkDataSet* sliceDistanceData = this->SliceDistance->GetOutput();
  if (!sliceDistanceData || sliceDistanceData->GetNumberOfPoints() == 0)
    {
    return;
    }

  this->SliceCurvePointLocator->SetDataSet(this->SliceDistance->GetOutput());
  this->SliceCurvePointLocator->Update();

  double closestPointWorld[3] = { 0.0 };
  vtkIdType cellId = -1;
  int subId = -1;
  double dist2World = VTK_DOUBLE_MAX;
  if (interactionEventData->IsWorldPositionValid())
    {
    const double* worldPosition = interactionEventData->GetWorldPosition();
    this->SliceCurvePointLocator->FindClosestPoint(worldPosition, closestPointWorld, cellId, subId, dist2World);
    double lineVisibilityDistance = this->MarkupsDisplayNode->GetLineColorFadingEnd() * 1.2;
    if (dist2World > lineVisibilityDistance * lineVisibilityDistance)
      {
      // line not visible at this distance
      return;
      }
    }

  double closestPointDisplay[3] = { 0.0 };
  this->GetWorldToSliceCoordinates(closestPointWorld, closestPointDisplay);

  double maxPickingDistanceFromControlPoint2 = this->GetMaximumControlPointPickingDistance2();
  const int* displayPosition = interactionEventData->GetDisplayPosition();
  double displayPosition3[3] = { static_cast<double>(displayPosition[0]), static_cast<double>(displayPosition[1]), 0.0 };
  double dist2 = vtkMath::Distance2BetweenPoints(displayPosition3, closestPointDisplay);
  if (dist2 < maxPickingDistanceFromControlPoint2)
    {
    closestDistance2 = dist2;
    foundComponentType = vtkDMMLMarkupsDisplayNode::ComponentLine;
    componentIndex = markupsNode->GetControlPointIndexFromInterpolatedPointIndex(subId);
    }
}
