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

==============================================================================*/

#include "vtkDMMLInteractionEventData.h"

// VTK includes
#include "vtkCamera.h"
#include "vtkCellPicker.h"
#include "vtkEvent.h"
#include "vtkMatrix4x4.h"
#include "vtkPoints.h"
#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkWindow.h"

#include <algorithm>

//---------------------------------------------------------------------------
vtkDMMLInteractionEventData::vtkDMMLInteractionEventData()
{
  this->Type = 0;
  this->Modifiers = 0;
  this->DisplayPosition[0] = 0;
  this->DisplayPosition[1] = 0;
  this->WorldPosition[0] = 0.0;
  this->WorldPosition[1] = 0.0;
  this->WorldPosition[2] = 0.0;
  this->WorldOrientation[0] = 0.0;
  this->WorldOrientation[1] = 0.0;
  this->WorldOrientation[2] = 0.0;
  this->WorldOrientation[3] = 1.0;
  this->WorldDirection[0] = 0.0;
  this->WorldDirection[1] = 0.0;
  this->WorldDirection[2] = 1.0;
  this->TrackPadPosition[0] = 0.0;
  this->TrackPadPosition[1] = 0.0;
  this->DisplayPositionValid = false;
  this->WorldPositionValid = false;
  this->WorldPositionAccurate = false;
  this->ComputeAccurateWorldPositionAttempted = false;
  this->AccuratePicker = nullptr;
  this->Renderer = nullptr;
  this->KeyRepeatCount = 0;
  this->KeyCode = 0;
  this->ViewNode = nullptr;
  this->Rotation = 0.0;
  this->LastRotation = 0.0;
  this->Scale = 1.0;
  this->LastScale = 1.0;
  this->Translation[0] = this->Translation[1] = 0.0;
  this->LastTranslation[0] = this->LastTranslation[1] = 0.0;
  this->WorldToPhysicalScale = 1.0;
  this->InteractionContextName = "";
  this->ComponentType = -1;
  this->ComponentIndex = -1;
  this->MouseMovedSinceButtonDown = true;
  this->WorldToViewTransformMatrixValid = false;
  std::fill(std::begin(this->WorldToViewTransformMatrix), std::end(this->WorldToViewTransformMatrix), 0.0);
}

//---------------------------------------------------------------------------
vtkDMMLInteractionEventData* vtkDMMLInteractionEventData::New()
{
  vtkDMMLInteractionEventData *ret = new vtkDMMLInteractionEventData;
  ret->InitializeObjectBase();
  return ret;
};

//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::SetType(unsigned long v)
{
  this->Type = v;
}

//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::SetModifiers(int v)
{
  this->Modifiers = v;
}

//---------------------------------------------------------------------------
int vtkDMMLInteractionEventData::GetModifiers()
{
  return this->Modifiers;
}

//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::SetWorldPosition(const double p[3], bool accurate/*=true*/)
{
  this->WorldPosition[0] = p[0];
  this->WorldPosition[1] = p[1];
  this->WorldPosition[2] = p[2];
  this->WorldPositionValid = true;
  this->WorldPositionAccurate = accurate;
}

//---------------------------------------------------------------------------
bool vtkDMMLInteractionEventData::IsWorldPositionValid()
{
  return this->WorldPositionValid;
}

//---------------------------------------------------------------------------
bool vtkDMMLInteractionEventData::IsWorldPositionAccurate()
{
  return this->WorldPositionValid && this->WorldPositionAccurate;
}

//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::SetWorldPositionInvalid()
{
  this->WorldPositionValid = false;
  this->WorldPositionAccurate = false;
}

//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::GetDisplayPosition(int v[2]) const
{
  std::copy(this->DisplayPosition, this->DisplayPosition + 2, v);
}

//---------------------------------------------------------------------------
const int* vtkDMMLInteractionEventData::GetDisplayPosition() const
{
  return this->DisplayPosition;
}
//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::SetDisplayPosition(const int p[2])
{
  this->DisplayPosition[0] = p[0];
  this->DisplayPosition[1] = p[1];
  this->DisplayPositionValid = true;
  this->ComputeAccurateWorldPositionAttempted = false;
}

//---------------------------------------------------------------------------
bool vtkDMMLInteractionEventData::IsDisplayPositionValid()
{
  return this->DisplayPositionValid;
}

//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::SetDisplayPositionInvalid()
{
  this->DisplayPositionValid = false;
}

//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::SetKeyCode(char v)
{
  this->KeyCode = v;
}

//---------------------------------------------------------------------------
char vtkDMMLInteractionEventData::GetKeyCode()
{
  return this->KeyCode;
}

//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::SetKeyRepeatCount(char v)
{
  this->KeyRepeatCount = v;
}

//---------------------------------------------------------------------------
int vtkDMMLInteractionEventData::GetKeyRepeatCount()
{
  return this->KeyRepeatCount;
}

//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::SetKeySym(const std::string &v)
{
  this->KeySym = v;
}

//---------------------------------------------------------------------------
const std::string& vtkDMMLInteractionEventData::GetKeySym()
{
  return this->KeySym;
}

//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::SetViewNode(vtkDMMLAbstractViewNode* viewNode)
{
  this->ViewNode = viewNode;
}

//---------------------------------------------------------------------------
vtkDMMLAbstractViewNode* vtkDMMLInteractionEventData::GetViewNode() const
{
  return this->ViewNode;
}

//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::SetComponentType(int componentType) { this->ComponentType = componentType; }

//---------------------------------------------------------------------------
int vtkDMMLInteractionEventData::GetComponentType() const
{
  return this->ComponentType;
}

//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::SetComponentIndex(int componentIndex)
{
  this->ComponentIndex = componentIndex;
}

//---------------------------------------------------------------------------
int vtkDMMLInteractionEventData::GetComponentIndex() const
{
  return this->ComponentIndex;
}

//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::SetMouseMovedSinceButtonDown(bool moved)
{
  this->MouseMovedSinceButtonDown = moved;
}

//---------------------------------------------------------------------------
bool vtkDMMLInteractionEventData::GetMouseMovedSinceButtonDown() const
{
  return this->MouseMovedSinceButtonDown;
}

//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::SetRotation(double v)
{
  this->Rotation = v;
}

//---------------------------------------------------------------------------
double vtkDMMLInteractionEventData::GetRotation() const
{
  return this->Rotation;
}

//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::SetLastRotation(double v)
{
  this->LastRotation = v;
}

//---------------------------------------------------------------------------
double vtkDMMLInteractionEventData::GetLastRotation() const
{
  return this->LastRotation;
}

//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::SetWorldToPhysicalScale(double v)
{
  this->WorldToPhysicalScale = v;
}

//---------------------------------------------------------------------------
double vtkDMMLInteractionEventData::GetWorldToPhysicalScale() const
{
  return this->WorldToPhysicalScale;
}

//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::SetAttributesFromInteractor(vtkRenderWindowInteractor* interactor)
{
  this->Modifiers = 0;
  if (interactor->GetShiftKey())
    {
    this->Modifiers |= vtkEvent::ShiftModifier;
    }
  if (interactor->GetControlKey())
    {
    this->Modifiers |= vtkEvent::ControlModifier;
    }
  if (interactor->GetAltKey())
    {
    this->Modifiers |= vtkEvent::AltModifier;
    }
  this->KeyCode = interactor->GetKeyCode();
  this->KeySym = (interactor->GetKeySym() ? interactor->GetKeySym() : "");
  this->KeyRepeatCount = interactor->GetRepeatCount();

  this->Rotation = interactor->GetRotation();
  this->LastRotation = interactor->GetLastRotation();
  this->Scale = interactor->GetScale();
  this->LastScale = interactor->GetLastScale();
  this->SetTranslation(interactor->GetTranslation());
  this->SetLastTranslation(interactor->GetLastTranslation());
}

//---------------------------------------------------------------------------
vtkRenderer* vtkDMMLInteractionEventData::GetRenderer() const
{
  return this->Renderer;
}

//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::SetRenderer(vtkRenderer* ren)
{
  this->Renderer = ren;
}

//---------------------------------------------------------------------------
vtkCellPicker* vtkDMMLInteractionEventData::GetAccuratePicker() const
{
  return this->AccuratePicker;
}

//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::SetAccuratePicker(vtkCellPicker* picker)
{
  this->AccuratePicker = picker;
}

//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::SetInteractionContextName(const std::string& contextName)
{
  this->InteractionContextName = contextName;
}

//---------------------------------------------------------------------------
const std::string& vtkDMMLInteractionEventData::GetInteractionContextName()
{
  return this->InteractionContextName;
}

//---------------------------------------------------------------------------
bool vtkDMMLInteractionEventData::ComputeAccurateWorldPosition(bool force/*=false*/)
{
  if (this->IsWorldPositionAccurate())
    {
    return true;
    }
  if (this->ComputeAccurateWorldPositionAttempted && !force)
    {
    // by default do not attempt to compute accurate position again
    // if it failed once
    return false;
    }
  if (!this->AccuratePicker || !this->Renderer || !this->DisplayPositionValid)
    {
    return false;
    }
  this->ComputeAccurateWorldPositionAttempted = true;
  if (!this->AccuratePicker->Pick(static_cast<double>(this->DisplayPosition[0]), static_cast<double>(this->DisplayPosition[1]), 0, this->Renderer))
    {
    return false;
    }
  vtkPoints* pickPositions = this->AccuratePicker->GetPickedPositions();
  int numberOfPickedPositions = pickPositions->GetNumberOfPoints();
  if (numberOfPickedPositions < 1)
    {
    return false;
    }
  // There may be multiple picked positions, choose the one closest to the camera
  double cameraPosition[3] = { 0,0,0 };
  this->Renderer->GetActiveCamera()->GetPosition(cameraPosition);
  pickPositions->GetPoint(0, this->WorldPosition);
  double minDist2 = vtkMath::Distance2BetweenPoints(this->WorldPosition, cameraPosition);
  for (int i = 1; i < numberOfPickedPositions; i++)
    {
    double currentMinDist2 = vtkMath::Distance2BetweenPoints(pickPositions->GetPoint(i), cameraPosition);
    if (currentMinDist2 < minDist2)
      {
      pickPositions->GetPoint(i, this->WorldPosition);
      minDist2 = currentMinDist2;
      }
    }
  this->WorldPositionValid = true;
  this->WorldPositionAccurate = true;
  return true;
}

//---------------------------------------------------------------------------
bool vtkDMMLInteractionEventData::Equivalent(const vtkEventData *e) const
{
  const vtkDMMLInteractionEventData *edd = static_cast<const vtkDMMLInteractionEventData *>(e);
  if (this->Type != edd->Type)
    {
    return false;
    }
  if (edd->Modifiers >= 0 && (this->Modifiers != edd->Modifiers))
    {
    return false;
    }
  return true;
};

//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::SetScale(double scale)
{
  this->Scale = scale;
}

//---------------------------------------------------------------------------
double vtkDMMLInteractionEventData::GetScale() const
{
  return this->Scale;
}

//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::SetLastScale(double lastScale)
{
  this->LastScale = lastScale;
}

//---------------------------------------------------------------------------
double vtkDMMLInteractionEventData::GetLastScale() const
{
  return this->LastScale;
}

//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::SetTranslation(const double translation[2])
{
  this->Translation[0] = translation[0];
  this->Translation[1] = translation[1];
}

//---------------------------------------------------------------------------
const double* vtkDMMLInteractionEventData::GetTranslation() const
{
  return this->Translation;
}

//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::SetLastTranslation(const double lastTranslation[2])
{
  this->LastTranslation[0] = lastTranslation[0];
  this->LastTranslation[1] = lastTranslation[1];
}

//---------------------------------------------------------------------------
const double* vtkDMMLInteractionEventData::GetLastTranslation() const
{
  return this->LastTranslation;
}

//---------------------------------------------------------------------------
void vtkDMMLInteractionEventData::WorldToDisplay(const double worldPosition[3], double displayPosition[3])
{
  if (!this->Renderer)
    {
    return;
    }
  if (!this->WorldToViewTransformMatrixValid)
    {
    vtkMatrix4x4::DeepCopy(this->WorldToViewTransformMatrix,
      this->Renderer->GetActiveCamera()->
      GetCompositeProjectionTransformMatrix(
        this->Renderer->GetTiledAspectRatio(),0,1));
    this->WorldToViewTransformMatrixValid = true;
    }

  //get view to display scaling and put in 4x4 matrix
  const double* viewport = this->Renderer->GetViewport();
  const int* displaySize = this->Renderer->GetVTKWindow()->GetSize();

  const double homogeneousWorldPosition[] = {worldPosition[0], worldPosition[1], worldPosition[2], 1.0};
  double viewPosition[4];

  vtkMatrix4x4::MultiplyPoint(this->WorldToViewTransformMatrix, homogeneousWorldPosition, viewPosition);
  if (viewPosition[3] != 0.0)
    {
    viewPosition[0] /= viewPosition[3];
    viewPosition[1] /= viewPosition[3];
    viewPosition[2] /= viewPosition[3];
    viewPosition[3] = 1.0;
    }

  // view to display
  displayPosition[0] = (viewPosition[0] + 1.0) * (displaySize[0] * (viewport[2] - viewport[0])) / 2.0
    + displaySize[0] * viewport[0];
  displayPosition[1] = (viewPosition[1] + 1.0) * (displaySize[1] * (viewport[3] - viewport[1])) / 2.0
    + displaySize[1] * viewport[1];
  displayPosition[2] = 0.0;
}