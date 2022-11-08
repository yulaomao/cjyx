/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women’s Hospital through NIH grant R01MH112748.

==============================================================================*/

// VTK includes
#include <vtkActor2D.h>
#include <vtkCellLocator.h>
#include <vtkContourTriangulator.h>
#include <vtkCubeSource.h>
#include <vtkCutter.h>
#include <vtkDoubleArray.h>
#include <vtkLine.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkOutlineFilter.h>
#include <vtkPassThroughFilter.h>
#include <vtkPlane.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

// Markups VTK widgets includes
#include <vtkCjyxROIRepresentation2D.h>

// DMML includes
#include "vtkDMMLInteractionEventData.h"
#include "vtkDMMLMarkupsROIDisplayNode.h"
#include "vtkDMMLMarkupsROINode.h"

vtkStandardNewMacro(vtkCjyxROIRepresentation2D);
//----------------------------------------------------------------------
vtkCjyxROIRepresentation2D::vtkCjyxROIRepresentation2D()
{
  this->ROISource = nullptr;

  this->ROIPipelineInputFilter = vtkSmartPointer<vtkPassThroughFilter>::New();

  this->ROIToWorldTransform = vtkSmartPointer<vtkTransform>::New();
  this->ROIToWorldTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->ROIToWorldTransformFilter->SetInputConnection(this->ROIPipelineInputFilter->GetOutputPort());
  this->ROIToWorldTransformFilter->SetTransform(this->ROIToWorldTransform);

  this->ROIOutlineCutter = vtkSmartPointer<vtkCutter>::New();
  this->ROIOutlineCutter->SetInputConnection(this->ROIToWorldTransformFilter->GetOutputPort());
  this->ROIOutlineCutter->SetCutFunction(this->SlicePlane);

  this->ROIOutlineWorldToSliceTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->ROIOutlineWorldToSliceTransformFilter->SetInputConnection(this->ROIOutlineCutter->GetOutputPort());
  this->ROIOutlineWorldToSliceTransformFilter->SetTransform(this->WorldToSliceTransform);

  this->ROIOutlineMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  this->ROIOutlineMapper->SetInputConnection(this->ROIOutlineWorldToSliceTransformFilter->GetOutputPort());
  this->ROIOutlineProperty = vtkSmartPointer<vtkProperty2D>::New();
  this->ROIOutlineProperty->DeepCopy(this->GetControlPointsPipeline(Unselected)->Property);
  this->ROIOutlineActor = vtkSmartPointer<vtkActor2D>::New();
  this->ROIOutlineActor->SetMapper(this->ROIOutlineMapper);
  this->ROIOutlineActor->SetProperty(this->ROIOutlineProperty);

  this->ROIIntersectionTriangulator = vtkSmartPointer<vtkContourTriangulator>::New();
  this->ROIIntersectionTriangulator->SetInputConnection(this->ROIOutlineWorldToSliceTransformFilter->GetOutputPort());

  this->ROIMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  this->ROIMapper->SetInputConnection(this->ROIIntersectionTriangulator->GetOutputPort());
  this->ROIProperty = vtkSmartPointer<vtkProperty2D>::New();
  this->ROIProperty->DeepCopy(this->GetControlPointsPipeline(Unselected)->Property);
  this->ROIActor = vtkSmartPointer<vtkActor2D>::New();
  this->ROIActor->SetMapper(this->ROIMapper);
  this->ROIActor->SetProperty(this->ROIProperty);
}

//----------------------------------------------------------------------
vtkCjyxROIRepresentation2D::~vtkCjyxROIRepresentation2D() = default;

//----------------------------------------------------------------------
void vtkCjyxROIRepresentation2D::UpdateFromDMML(vtkDMMLNode* caller, unsigned long event, void *callData /*=nullptr*/)
{
  Superclass::UpdateFromDMML(caller, event, callData);

  vtkDMMLMarkupsROINode* roiNode = vtkDMMLMarkupsROINode::SafeDownCast(this->MarkupsNode);
  if (!roiNode)
    {
    return;
    }

  if (!roiNode || !this->MarkupsDisplayNode || !this->IsDisplayable())
    {
    this->VisibilityOff();
    return;
    }

  switch (roiNode->GetROIType())
    {
    case vtkDMMLMarkupsROINode::ROITypeBox:
    case vtkDMMLMarkupsROINode::ROITypeBoundingBox:
      this->UpdateCubeSourceFromDMML(roiNode);
      break;
    default:
      this->VisibilityOff();
      return;
    }

  this->ROIActor->SetVisibility(roiNode->GetNumberOfControlPoints() > 0 && this->MarkupsDisplayNode->GetFillVisibility());
  this->ROIOutlineActor->SetVisibility(roiNode->GetNumberOfControlPoints() > 0 && this->MarkupsDisplayNode->GetOutlineVisibility());

  this->ROIToWorldTransform->SetMatrix(roiNode->GetInteractionHandleToWorldMatrix());

  int controlPointType = Active;
  if (this->MarkupsDisplayNode->GetActiveComponentType() != vtkDMMLMarkupsROIDisplayNode::ComponentROI)
    {
    controlPointType = this->GetAllControlPointsSelected() ? vtkCjyxMarkupsWidgetRepresentation::Selected : vtkCjyxMarkupsWidgetRepresentation::Unselected;
    }

  double opacity = this->MarkupsDisplayNode->GetOpacity();

  double fillOpacity = opacity * this->MarkupsDisplayNode->GetFillOpacity();
  this->ROIProperty->DeepCopy(this->GetControlPointsPipeline(controlPointType)->Property);
  this->ROIProperty->SetOpacity(fillOpacity);

  double outlineOpacity = opacity * this->MarkupsDisplayNode->GetOutlineOpacity();
  this->ROIOutlineProperty->DeepCopy(this->GetControlPointsPipeline(controlPointType)->Property);
  this->ROIOutlineProperty->SetOpacity(outlineOpacity);

  // Properties label display
  this->TextActor->SetTextProperty(this->GetControlPointsPipeline(controlPointType)->TextProperty);
  if (this->MarkupsDisplayNode->GetPropertiesLabelVisibility()
    && this->AnyPointVisibilityOnSlice
    && roiNode->GetNumberOfDefinedControlPoints(true) > 0) // including preview
    {
    double textPos[3] = { 0.0,  0.0, 0.0 };
    this->GetNthControlPointDisplayPosition(0, textPos);
    this->TextActor->SetDisplayPosition(static_cast<int>(textPos[0]), static_cast<int>(textPos[1]));
    this->TextActor->SetVisibility(true);
    }
  else
    {
    this->TextActor->SetVisibility(false);
    }
}

//----------------------------------------------------------------------
void vtkCjyxROIRepresentation2D::SetROISource(vtkPolyDataAlgorithm* roiSource)
{
  this->ROISource = roiSource;
  if (this->ROISource)
    {
    this->ROIPipelineInputFilter->SetInputConnection(roiSource->GetOutputPort());
    }
  else
    {
    this->ROIPipelineInputFilter->RemoveAllInputConnections(0);
    }
}


//----------------------------------------------------------------------
void vtkCjyxROIRepresentation2D::UpdateCubeSourceFromDMML(vtkDMMLMarkupsROINode* roiNode)
{
  if (!roiNode)
    {
    return;
    }

  vtkSmartPointer<vtkCubeSource> cubeSource = vtkCubeSource::SafeDownCast(this->ROISource);
  if (!cubeSource)
    {
    cubeSource = vtkSmartPointer<vtkCubeSource>::New();
    this->SetROISource(cubeSource);
    }

  double sideLengths[3] = { 0.0, 0.0, 0.0 };
  roiNode->GetSizeWorld(sideLengths);
  cubeSource->SetXLength(sideLengths[0]);
  cubeSource->SetYLength(sideLengths[1]);
  cubeSource->SetZLength(sideLengths[2]);

  this->ROIOutlineActor->SetVisibility(vtkMath::Norm(sideLengths) >= 1e-6);
}

//----------------------------------------------------------------------
void vtkCjyxROIRepresentation2D::GetActors(vtkPropCollection *pc)
{
  this->ROIActor->GetActors(pc);
  this->ROIOutlineActor->GetActors(pc);
  this->Superclass::GetActors(pc);
}

//----------------------------------------------------------------------
void vtkCjyxROIRepresentation2D::ReleaseGraphicsResources(
  vtkWindow *win)
{
  this->ROIActor->ReleaseGraphicsResources(win);
  this->ROIOutlineActor->ReleaseGraphicsResources(win);
  this->Superclass::ReleaseGraphicsResources(win);
}

//----------------------------------------------------------------------
int vtkCjyxROIRepresentation2D::RenderOverlay(vtkViewport *viewport)
{
  int count = 0;
  if (this->ROIActor->GetVisibility())
    {
    count +=  this->ROIActor->RenderOverlay(viewport);
    }
  if (this->ROIOutlineActor->GetVisibility())
    {
    count +=  this->ROIOutlineActor->RenderOverlay(viewport);
    }
  count += Superclass::RenderOverlay(viewport);
  return count;
}

//-----------------------------------------------------------------------------
int vtkCjyxROIRepresentation2D::RenderOpaqueGeometry(
  vtkViewport *viewport)
{
  int count = 0;
  if (this->ROIActor->GetVisibility())
    {
    count += this->ROIActor->RenderOpaqueGeometry(viewport);
    }
  if (this->ROIOutlineActor->GetVisibility())
    {
    count += this->ROIOutlineActor->RenderOpaqueGeometry(viewport);
    }
  count += Superclass::RenderOpaqueGeometry(viewport);
  return count;
}

//-----------------------------------------------------------------------------
int vtkCjyxROIRepresentation2D::RenderTranslucentPolygonalGeometry(
  vtkViewport *viewport)
{
  int count = 0;
  if (this->ROIActor->GetVisibility())
    {
    count += this->ROIActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  if (this->ROIOutlineActor->GetVisibility())
    {
    count += this->ROIOutlineActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  count += Superclass::RenderTranslucentPolygonalGeometry(viewport);
  return count;
}

//-----------------------------------------------------------------------------
vtkTypeBool vtkCjyxROIRepresentation2D::HasTranslucentPolygonalGeometry()
{
  if (this->Superclass::HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  if (this->ROIActor->GetVisibility() && this->ROIActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  if (this->ROIOutlineActor->GetVisibility() && this->ROIOutlineActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------
double* vtkCjyxROIRepresentation2D::GetBounds()
{
  return nullptr;
}

//-----------------------------------------------------------------------------
void vtkCjyxROIRepresentation2D::PrintSelf(ostream& os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------
void vtkCjyxROIRepresentation2D::CanInteract(
  vtkDMMLInteractionEventData* interactionEventData,
  int &foundComponentType, int &foundComponentIndex, double &closestDistance2)
{
  foundComponentType = vtkDMMLMarkupsDisplayNode::ComponentNone;
  vtkDMMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || markupsNode->GetLocked() || !interactionEventData)
    {
    return;
    }

  Superclass::CanInteract(interactionEventData, foundComponentType, foundComponentIndex, closestDistance2);
  if (foundComponentType != vtkDMMLMarkupsDisplayNode::ComponentNone)
    {
    return;
    }

  this->CanInteractWithROI(interactionEventData, foundComponentType, foundComponentIndex, closestDistance2);
}

//-----------------------------------------------------------------------------
void vtkCjyxROIRepresentation2D::CanInteractWithROI(
  vtkDMMLInteractionEventData* interactionEventData,
  int& foundComponentType, int& foundComponentIndex, double& closestDistance2)
{
  vtkDMMLMarkupsROINode* roiNode = vtkDMMLMarkupsROINode::SafeDownCast(this->MarkupsNode);
  if (!roiNode
    || !this->Visibility
    || !this->ROIActor->GetVisibility()
    || !this->ROISource)
    {
    return;
    }

  this->ROIOutlineCutter->Update();
  if (this->ROIOutlineCutter->GetOutput() && this->ROIOutlineCutter->GetOutput()->GetNumberOfPoints() == 0)
    {
    return;
    }

  double closestPointWorld[3] = { 0.0, 0.0, 0.0 };
  if (interactionEventData->IsWorldPositionValid())
    {
    const double* worldPosition = interactionEventData->GetWorldPosition();

    double dist2World = VTK_DOUBLE_MAX;

    vtkPolyData* roiSliceIntersection = this->ROIOutlineCutter->GetOutput();
    for (int lineIndex = 0; lineIndex < roiSliceIntersection->GetNumberOfCells(); ++lineIndex)
      {
      vtkLine* line = vtkLine::SafeDownCast(roiSliceIntersection->GetCell(lineIndex));
      if (!line)
        {
        continue;
        }

      double edgePoint0World[3] = { 0.0, 0.0, 0.0 };
      line->GetPoints()->GetPoint(0, edgePoint0World);

      double edgePoint1World[3] = { 0.0, 0.0, 0.0 };
      line->GetPoints()->GetPoint(1, edgePoint1World);

      double t;
      double currentClosestPointWorld[3] = { 0.0, 0.0, 0.0 };
      double currentDist2World = vtkLine::DistanceToLine(worldPosition, edgePoint0World, edgePoint1World, t, currentClosestPointWorld);
      if (currentDist2World < dist2World)
        {
        dist2World = currentDist2World;
        closestPointWorld[0] = currentClosestPointWorld[0];
        closestPointWorld[1] = currentClosestPointWorld[1];
        closestPointWorld[2] = currentClosestPointWorld[2];
        }
      }
    }

  double closestPointDisplay[3] = { 0.0, 0.0, 0.0 };
  this->GetWorldToSliceCoordinates(closestPointWorld, closestPointDisplay);

  double pixelTolerance = this->PickingTolerance * this->ScreenScaleFactor;
  const int* displayPosition = interactionEventData->GetDisplayPosition();
  double displayPosition3[3] = { static_cast<double>(displayPosition[0]), static_cast<double>(displayPosition[1]), 0.0 };
  double dist2Display = vtkMath::Distance2BetweenPoints(displayPosition3, closestPointDisplay);
  if (dist2Display < pixelTolerance * pixelTolerance)
    {
    closestDistance2 = dist2Display;
    foundComponentType = vtkDMMLMarkupsROIDisplayNode::ComponentROI;
    foundComponentIndex = 0;
    }
}

//----------------------------------------------------------------------
void vtkCjyxROIRepresentation2D::SetupInteractionPipeline()
{
  this->InteractionPipeline = new MarkupsInteractionPipelineROI2D(this);
  this->InteractionPipeline->InitializePipeline();
}

//----------------------------------------------------------------------
void vtkCjyxROIRepresentation2D::UpdateInteractionPipeline()
{
  Superclass::UpdateInteractionPipeline();
  vtkDMMLMarkupsROINode* roiNode = vtkDMMLMarkupsROINode::SafeDownCast(this->GetMarkupsNode());
  if (!roiNode || !this->MarkupsDisplayNode || !this->ROISource)
    {
    this->InteractionPipeline->Actor->SetVisibility(false);
    return;
    }

  // To avoid "No input data" error messages from vtkTransformPolyDataFilter, we need to check that the input is not empty.
  this->ROIOutlineCutter->Update();
  vtkPolyData* roiPolyData = vtkPolyData::SafeDownCast(this->ROISource->GetOutput());
  vtkPolyData* outlinePolyData = this->ROIOutlineCutter->GetOutput();
  if ((roiPolyData && roiPolyData->GetNumberOfPoints() == 0) || (outlinePolyData && outlinePolyData->GetNumberOfPoints() == 0))
    {
    this->InteractionPipeline->Actor->SetVisibility(false);
    return;
    }

  this->InteractionPipeline->Actor->SetVisibility(this->MarkupsDisplayNode->GetVisibility()
    && roiNode->GetNumberOfControlPoints() > 0
    && this->MarkupsDisplayNode->GetVisibility2D()
    && this->MarkupsDisplayNode->GetHandlesInteractive());

  vtkNew<vtkTransform> handleToWorldTransform;
  handleToWorldTransform->SetMatrix(roiNode->GetInteractionHandleToWorldMatrix());
  this->InteractionPipeline->HandleToWorldTransform->DeepCopy(handleToWorldTransform);

  MarkupsInteractionPipelineROI2D* interactionPipeline = static_cast<MarkupsInteractionPipelineROI2D*>(this->InteractionPipeline);
  interactionPipeline->UpdateScaleHandles();
  interactionPipeline->WorldToSliceTransformFilter->SetTransform(this->WorldToSliceTransform);
}

//-----------------------------------------------------------------------------
vtkCjyxROIRepresentation2D::MarkupsInteractionPipelineROI2D::MarkupsInteractionPipelineROI2D(vtkCjyxMarkupsWidgetRepresentation* representation)
  : MarkupsInteractionPipelineROI(representation)
{
  this->WorldToSliceTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->WorldToSliceTransformFilter->SetTransform(vtkNew<vtkTransform>());
  this->WorldToSliceTransformFilter->SetInputConnection(this->HandleToWorldTransformFilter->GetOutputPort());
  this->Mapper->SetInputConnection(this->WorldToSliceTransformFilter->GetOutputPort());
  this->Mapper->SetTransformCoordinate(nullptr);
}

//----------------------------------------------------------------------
void vtkCjyxROIRepresentation2D::MarkupsInteractionPipelineROI2D::GetViewPlaneNormal(double viewPlaneNormal[3])
{
  if (!viewPlaneNormal)
    {
    return;
    }

  double viewPlaneNormal4[4] = { 0.0, 0.0, 1.0, 0.0 };
  if (this->Representation)
    {
    vtkDMMLSliceNode* sliceNode = vtkDMMLSliceNode::SafeDownCast(this->Representation->GetViewNode());
    if (sliceNode)
      {
      sliceNode->GetSliceToRAS()->MultiplyPoint(viewPlaneNormal4, viewPlaneNormal4);
      }
    }
  viewPlaneNormal[0] = viewPlaneNormal4[0];
  viewPlaneNormal[1] = viewPlaneNormal4[1];
  viewPlaneNormal[2] = viewPlaneNormal4[2];
}

//---------------------------------------------------------------------------
int IntersectWithFinitePlane(double n[3], double o[3],  double pOrigin[3], double px[3], double py[3],
  double x0[3], double x1[3])
{
  // Copied with fixes from vtkPlane in VTK9.
  // Can be replaced when we switch fully to VTK9.

  // Since we are dealing with convex shapes, if there is an intersection a
  // single line is produced as output. So all this is necessary is to
  // intersect the four bounding lines of the finite line and find the two
  // intersection points.
  int numInts = 0;
  double t, * x = x0;
  double xr0[3], xr1[3];

  // First line
  xr0[0] = pOrigin[0];
  xr0[1] = pOrigin[1];
  xr0[2] = pOrigin[2];
  xr1[0] = px[0];
  xr1[1] = px[1];
  xr1[2] = px[2];
  if (vtkPlane::IntersectWithLine(xr0, xr1, n, o, t, x))
    {
    numInts++;
    x = x1;
    }

  // Second line
  xr1[0] = py[0];
  xr1[1] = py[1];
  xr1[2] = py[2];
  if (vtkPlane::IntersectWithLine(xr0, xr1, n, o, t, x))
    {
    numInts++;
    x = x1;
    }
  if (numInts == 2)
    {
    return 1;
    }

  // Third line
  xr0[0] = -pOrigin[0] + px[0] + py[0];
  xr0[1] = -pOrigin[1] + px[1] + py[1];
  xr0[2] = -pOrigin[2] + px[2] + py[2];
  if (vtkPlane::IntersectWithLine(xr0, xr1, n, o, t, x))
    {
    numInts++;
    x = x1;
    }
  if (numInts == 2)
    {
    return 1;
    }

  // Fourth and last line
  xr1[0] = px[0];
  xr1[1] = px[1];
  xr1[2] = px[2];
  if (vtkPlane::IntersectWithLine(xr0, xr1, n, o, t, x))
    {
    numInts++;
    }
  if (numInts == 2)
    {
    return 1;
    }

  // No intersection has occurred, or a single degenerate point
  return 0;
}


//-----------------------------------------------------------------------------
void vtkCjyxROIRepresentation2D::MarkupsInteractionPipelineROI2D::UpdateScaleHandles()
{
  vtkDMMLMarkupsROINode* roiNode = vtkDMMLMarkupsROINode::SafeDownCast(
    vtkCjyxMarkupsWidgetRepresentation::SafeDownCast(this->Representation)->GetMarkupsNode());
  if (!roiNode)
    {
    return;
    }

  vtkDMMLMarkupsDisplayNode* displayNode = vtkDMMLMarkupsDisplayNode::SafeDownCast(roiNode->GetDisplayNode());

  double viewPlaneOrigin4[4] = { 0.0, 0.0, 0.0, 1.0 };
  double viewPlaneNormal4[4] = { 0.0, 0.0, 1.0, 0.0 };
  if (this->Representation)
    {
    vtkDMMLSliceNode* sliceNode = vtkDMMLSliceNode::SafeDownCast(this->Representation->GetViewNode());
    if (sliceNode)
      {
      sliceNode->GetSliceToRAS()->MultiplyPoint(viewPlaneNormal4, viewPlaneNormal4);
      sliceNode->GetSliceToRAS()->MultiplyPoint(viewPlaneOrigin4, viewPlaneOrigin4);
      }
    }

  vtkMatrix4x4* objectToWorldMatrix = roiNode->GetObjectToWorldMatrix();
  vtkNew<vtkTransform> worldToObjectTransform;
  worldToObjectTransform->Concatenate(objectToWorldMatrix);
  worldToObjectTransform->Inverse();

  double viewPlaneOrigin_Object[3] = { 0.0, 0.0, 0.0 };
  double viewPlaneNormal_Object[3] = { 0.0, 0.0, 0.0 };
  worldToObjectTransform->TransformPoint(viewPlaneOrigin4, viewPlaneOrigin_Object);
  worldToObjectTransform->TransformVector(viewPlaneNormal4, viewPlaneNormal_Object);

  double sideLengths[3] = { 0.0, 0.0, 0.0 };
  roiNode->GetSizeWorld(sideLengths);
  double sideRadius[3] = { sideLengths[0], sideLengths[1], sideLengths[2] };
  vtkMath::MultiplyScalar(sideRadius, 0.5);

  vtkNew<vtkPoints> scaleHandlePoints_Object;
  scaleHandlePoints_Object->SetNumberOfPoints(26);

  vtkIdTypeArray* visibilityArray = vtkIdTypeArray::SafeDownCast(this->ScaleHandlePoints->GetPointData()->GetArray("visibility"));
  visibilityArray->SetNumberOfValues(scaleHandlePoints_Object->GetNumberOfPoints());
  visibilityArray->Fill(displayNode ? displayNode->GetScaleHandleVisibility() : 1.0);
  this->UpdateHandleVisibility();

  // Corner handles are not visible in 2D
  for (int i = vtkDMMLMarkupsROIDisplayNode::HandleLPICorner; i <= vtkDMMLMarkupsROIDisplayNode::HandleRASCorner; ++i)
    {
    visibilityArray->SetValue(i, false);
    }

  vtkNew<vtkPlane> plane;
  plane->SetNormal(viewPlaneNormal_Object);
  plane->SetOrigin(viewPlaneOrigin_Object);

  // Left face handle
  double lFacePoint_Object[3] =  { -sideRadius[0], -sideRadius[1], -sideRadius[2] };
  double lFacePointX_Object[3] = { -sideRadius[0],  sideRadius[1], -sideRadius[2] };
  double lFacePointY_Object[3] = { -sideRadius[0], -sideRadius[1],  sideRadius[2] };
  double lFaceIntersection0_Object[3] = { 0.0, 0.0, 0.0 };
  double lFaceIntersection1_Object[3] = { 0.0, 0.0, 0.0 };
  if (IntersectWithFinitePlane(viewPlaneNormal_Object, viewPlaneOrigin_Object,
    lFacePoint_Object, lFacePointX_Object, lFacePointY_Object, lFaceIntersection0_Object, lFaceIntersection1_Object))
    {
    vtkMath::Add(lFaceIntersection0_Object, lFaceIntersection1_Object, lFacePoint_Object);
    vtkMath::MultiplyScalar(lFacePoint_Object, 0.5);
    }
  else
    {
    // Face does not intersect with the slice. Handle should not be visible.
    visibilityArray->SetValue(vtkDMMLMarkupsROIDisplayNode::HandleLFace, false);
    }
  scaleHandlePoints_Object->SetPoint(vtkDMMLMarkupsROIDisplayNode::HandleLFace, lFacePoint_Object);

  // Right face handle
  double rFacePoint_Object[3] =  { sideRadius[0], -sideRadius[1], -sideRadius[2] };
  double rFacePointX_Object[3] = { sideRadius[0],  sideRadius[1], -sideRadius[2] };
  double rFacePointY_Object[3] = { sideRadius[0], -sideRadius[1],  sideRadius[2] };
  double rFaceIntersection0_Object[3] = { 0.0, 0.0, 0.0 };
  double rFaceIntersection1_Object[3] = { 0.0, 0.0, 0.0 };
  if (IntersectWithFinitePlane(viewPlaneNormal_Object, viewPlaneOrigin_Object,
    rFacePoint_Object, rFacePointX_Object, rFacePointY_Object, rFaceIntersection0_Object, rFaceIntersection1_Object))
    {
    vtkMath::Add(rFaceIntersection0_Object, rFaceIntersection1_Object, rFacePoint_Object);
    vtkMath::MultiplyScalar(rFacePoint_Object, 0.5);
    }
  else
    {
    // Face does not intersect with the slice. Handle should not be visible.
    visibilityArray->SetValue(vtkDMMLMarkupsROIDisplayNode::HandleRFace, false);
    }
  scaleHandlePoints_Object->SetPoint(vtkDMMLMarkupsROIDisplayNode::HandleRFace, rFacePoint_Object);

  // Posterior face handle
  double pFacePoint_Object[3] =  { -sideRadius[0], -sideRadius[1], -sideRadius[2] };
  double pFacePointX_Object[3] = {  sideRadius[0], -sideRadius[1], -sideRadius[2] };
  double pFacePointY_Object[3] = { -sideRadius[0], -sideRadius[1],  sideRadius[2] };
  double pFaceIntersection0_Object[3] = { 0.0, 0.0, 0.0 };
  double pFaceIntersection1_Object[3] = { 0.0, 0.0, 0.0 };
  if (IntersectWithFinitePlane(viewPlaneNormal_Object, viewPlaneOrigin_Object,
    pFacePoint_Object, pFacePointX_Object, pFacePointY_Object, pFaceIntersection0_Object, pFaceIntersection1_Object))
    {
    vtkMath::Add(pFaceIntersection0_Object, pFaceIntersection1_Object, pFacePoint_Object);
    vtkMath::MultiplyScalar(pFacePoint_Object, 0.5);
    }
  else
    {
    // Face does not intersect with the slice. Handle should not be visible.
    visibilityArray->SetValue(vtkDMMLMarkupsROIDisplayNode::HandlePFace, false);
    }
  scaleHandlePoints_Object->SetPoint(vtkDMMLMarkupsROIDisplayNode::HandlePFace, pFacePoint_Object);

  // Anterior face handle
  double aFacePoint_Object[3] =  { -sideRadius[0],  sideRadius[1], -sideRadius[2] };
  double aFacePointX_Object[3] = {  sideRadius[0],  sideRadius[1], -sideRadius[2] };
  double aFacePointY_Object[3] = { -sideRadius[0],  sideRadius[1],  sideRadius[2] };
  double aFaceIntersection0_Object[3] = { 0.0, 0.0, 0.0 };
  double aFaceIntersection1_Object[3] = { 0.0, 0.0, 0.0 };
  if (IntersectWithFinitePlane(viewPlaneNormal_Object, viewPlaneOrigin_Object,
    aFacePoint_Object, aFacePointX_Object, aFacePointY_Object, aFaceIntersection0_Object, aFaceIntersection1_Object))
    {
    vtkMath::Add(aFaceIntersection0_Object, aFaceIntersection1_Object, aFacePoint_Object);
    vtkMath::MultiplyScalar(aFacePoint_Object, 0.5);
    }
  else
    {
    // Face does not intersect with the slice. Handle should not be visible.
    visibilityArray->SetValue(vtkDMMLMarkupsROIDisplayNode::HandleAFace, false);
    }
  scaleHandlePoints_Object->SetPoint(vtkDMMLMarkupsROIDisplayNode::HandleAFace, aFacePoint_Object);

  // Inferior face handle
  double iFacePoint_Object[3] =  { -sideRadius[0], -sideRadius[1], -sideRadius[2] };
  double iFacePointX_Object[3] = {  sideRadius[0], -sideRadius[1], -sideRadius[2] };
  double iFacePointY_Object[3] = { -sideRadius[0],  sideRadius[1], -sideRadius[2] };
  double iFaceIntersection0_Object[3] = { 0.0, 0.0, 0.0 };
  double iFaceIntersection1_Object[3] = { 0.0, 0.0, 0.0 };
  if (IntersectWithFinitePlane(viewPlaneNormal_Object, viewPlaneOrigin_Object,
    iFacePoint_Object, iFacePointX_Object, iFacePointY_Object, iFaceIntersection0_Object, iFaceIntersection1_Object))
    {
    vtkMath::Add(iFaceIntersection0_Object, iFaceIntersection1_Object, iFacePoint_Object);
    vtkMath::MultiplyScalar(iFacePoint_Object, 0.5);
    }
  else
    {
    // Face does not intersect with the slice. Handle should not be visible.
    visibilityArray->SetValue(vtkDMMLMarkupsROIDisplayNode::HandleIFace, false);
    }
  scaleHandlePoints_Object->SetPoint(vtkDMMLMarkupsROIDisplayNode::HandleIFace, iFacePoint_Object);

  // Superior face handle
  double sFacePoint_Object[3] =  { -sideRadius[0], -sideRadius[1], sideRadius[2] };
  double sFacePointX_Object[3] = {  sideRadius[0], -sideRadius[1], sideRadius[2] };
  double sFacePointY_Object[3] = { -sideRadius[0],  sideRadius[1], sideRadius[2] };
  double sFaceIntersection0_Object[3] = { 0.0, 0.0, 0.0 };
  double sFaceIntersection1_Object[3] = { 0.0, 0.0, 0.0 };
  if (IntersectWithFinitePlane(viewPlaneNormal_Object, viewPlaneOrigin_Object,
    sFacePoint_Object, sFacePointX_Object, sFacePointY_Object, sFaceIntersection0_Object, sFaceIntersection1_Object))
    {
    vtkMath::Add(sFaceIntersection0_Object, sFaceIntersection1_Object, sFacePoint_Object);
    vtkMath::MultiplyScalar(sFacePoint_Object, 0.5);
    }
  else
    {
    // Face does not intersect with the slice. Handle should not be visible.
    visibilityArray->SetValue(vtkDMMLMarkupsROIDisplayNode::HandleSFace, false);
    }
  scaleHandlePoints_Object->SetPoint(vtkDMMLMarkupsROIDisplayNode::HandleSFace, sFacePoint_Object);

  double sVector_Object[3] = { 0.0, 0.0, sideLengths[2] };
  double lpEdgePoint_Object[3] = { -sideRadius[0], -sideRadius[1], -sideRadius[2] };
  this->AddScaleEdgeIntersection(vtkDMMLMarkupsROIDisplayNode::HandleLPEdge,
    visibilityArray, scaleHandlePoints_Object, viewPlaneNormal_Object, viewPlaneOrigin_Object, lpEdgePoint_Object, sVector_Object);
  double rpEdgePoint_Object[3] = {  sideRadius[0], -sideRadius[1], -sideRadius[2] };
  this->AddScaleEdgeIntersection(vtkDMMLMarkupsROIDisplayNode::HandleRPEdge,
    visibilityArray, scaleHandlePoints_Object, viewPlaneNormal_Object, viewPlaneOrigin_Object, rpEdgePoint_Object, sVector_Object);
  double laEdgePoint_Object[3] = { -sideRadius[0],  sideRadius[1], -sideRadius[2] };
  this->AddScaleEdgeIntersection(vtkDMMLMarkupsROIDisplayNode::HandleLAEdge,
    visibilityArray, scaleHandlePoints_Object, viewPlaneNormal_Object, viewPlaneOrigin_Object, laEdgePoint_Object, sVector_Object);
  double raEdgePoint_Object[3] = {  sideRadius[0],  sideRadius[1], -sideRadius[2] };
  this->AddScaleEdgeIntersection(vtkDMMLMarkupsROIDisplayNode::HandleRAEdge,
    visibilityArray, scaleHandlePoints_Object, viewPlaneNormal_Object, viewPlaneOrigin_Object, raEdgePoint_Object, sVector_Object);

  double aVector_Object[3] = { 0.0, sideLengths[1], 0.0 };
  double liEdgePoint_Object[3] = { -sideRadius[0], -sideRadius[1], -sideRadius[2] };
  this->AddScaleEdgeIntersection(vtkDMMLMarkupsROIDisplayNode::HandleLIEdge,
    visibilityArray, scaleHandlePoints_Object, viewPlaneNormal_Object, viewPlaneOrigin_Object, liEdgePoint_Object, aVector_Object);
  double riEdgePoint_Object[3] = {  sideRadius[0], -sideRadius[1], -sideRadius[2] };
  this->AddScaleEdgeIntersection(vtkDMMLMarkupsROIDisplayNode::HandleRIEdge,
    visibilityArray, scaleHandlePoints_Object, viewPlaneNormal_Object, viewPlaneOrigin_Object, riEdgePoint_Object, aVector_Object);
  double lsEdgePoint_Object[3] = { -sideRadius[0], -sideRadius[1],  sideRadius[2] };
  this->AddScaleEdgeIntersection(vtkDMMLMarkupsROIDisplayNode::HandleLSEdge,
    visibilityArray, scaleHandlePoints_Object, viewPlaneNormal_Object, viewPlaneOrigin_Object, lsEdgePoint_Object, aVector_Object);
  double rsEdgePoint_Object[3] = {  sideRadius[0], -sideRadius[1],  sideRadius[2] };
  this->AddScaleEdgeIntersection(vtkDMMLMarkupsROIDisplayNode::HandleRSEdge,
    visibilityArray, scaleHandlePoints_Object, viewPlaneNormal_Object, viewPlaneOrigin_Object, rsEdgePoint_Object, aVector_Object);

  double rVector_Object[3] = { sideLengths[0], 0.0, 0.0 };
  double piEdgePoint_Object[3] = { -sideRadius[0], -sideRadius[1], -sideRadius[2] };
  this->AddScaleEdgeIntersection(vtkDMMLMarkupsROIDisplayNode::HandlePIEdge,
    visibilityArray, scaleHandlePoints_Object, viewPlaneNormal_Object, viewPlaneOrigin_Object, piEdgePoint_Object, rVector_Object);
  double aiEdgePoint_Object[3] = { -sideRadius[0],  sideRadius[1], -sideRadius[2] };
  this->AddScaleEdgeIntersection(vtkDMMLMarkupsROIDisplayNode::HandleAIEdge,
    visibilityArray, scaleHandlePoints_Object, viewPlaneNormal_Object, viewPlaneOrigin_Object, aiEdgePoint_Object, rVector_Object);
  double psEdgePoint_Object[3] = { -sideRadius[0], -sideRadius[1],  sideRadius[2] };
  this->AddScaleEdgeIntersection(vtkDMMLMarkupsROIDisplayNode::HandlePSEdge,
    visibilityArray, scaleHandlePoints_Object, viewPlaneNormal_Object, viewPlaneOrigin_Object, psEdgePoint_Object, rVector_Object);
  double asEdgePoint_Object[3] = { -sideRadius[0],  sideRadius[1],  sideRadius[2] };
  this->AddScaleEdgeIntersection(vtkDMMLMarkupsROIDisplayNode::HandleASEdge,
    visibilityArray, scaleHandlePoints_Object, viewPlaneNormal_Object, viewPlaneOrigin_Object, asEdgePoint_Object, rVector_Object);

  this->ScaleHandlePoints->SetPoints(scaleHandlePoints_Object);
}

//-----------------------------------------------------------------------------
void vtkCjyxROIRepresentation2D::MarkupsInteractionPipelineROI2D::AddScaleEdgeIntersection(
  int pointIndex, vtkIdTypeArray* visibilityArray, vtkPoints* scaleHandlePoints_Object,
  double viewPlaneNormal_Object[3], double viewPlaneOrigin_Object[3], double edgePoint1_Object[3], double edgeVector_Object[3])
{
  double edgePoint2_Object[3] = { 0.0, 0.0, 0.0 };
  vtkMath::Add(edgePoint1_Object, edgeVector_Object, edgePoint2_Object);

  double t = 0.0;
  double raEdgeIntersection_Object[3] = { 0.0, 0.0, 0.0 };
  if (!vtkPlane::IntersectWithLine(edgePoint1_Object, edgePoint2_Object, viewPlaneNormal_Object, viewPlaneOrigin_Object, t, raEdgeIntersection_Object))
    {
    visibilityArray->SetValue(pointIndex, false);
    }
  scaleHandlePoints_Object->SetPoint(pointIndex, raEdgeIntersection_Object);
}
