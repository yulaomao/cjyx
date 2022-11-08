/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkDMMLSliceViewInteractorStyle.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or https://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkDMMLSliceViewInteractorStyle.h"

// DMML includes
#include "vtkDMMLAbstractSliceViewDisplayableManager.h"
#include "vtkDMMLApplicationLogic.h"
#include "vtkDMMLCrosshairDisplayableManager.h"
#include "vtkDMMLCrosshairNode.h"
#include "vtkDMMLDisplayableManagerGroup.h"
#include "vtkDMMLInteractionEventData.h"
#include "vtkDMMLInteractionNode.h"
#include "vtkDMMLScalarBarDisplayableManager.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSliceLayerLogic.h"
#include "vtkDMMLSliceLogic.h"
#include "vtkDMMLSliceNode.h"
#include "vtkDMMLVolumeNode.h"

// VTK includes
#include "vtkEvent.h"
#include "vtkGeneralTransform.h"
#include "vtkImageData.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkNew.h"

//STL includes
#include <algorithm>

//----------------------------------------------------------------------------
vtkCxxSetObjectMacro(vtkDMMLSliceViewInteractorStyle, SliceLogic, vtkDMMLSliceLogic);

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLSliceViewInteractorStyle);

//----------------------------------------------------------------------------
vtkDMMLSliceViewInteractorStyle::vtkDMMLSliceViewInteractorStyle()
{
  this->EnableCursorUpdate = true;
  this->SliceLogic = nullptr;
}

//----------------------------------------------------------------------------
vtkDMMLSliceViewInteractorStyle::~vtkDMMLSliceViewInteractorStyle()
{
  this->SetSliceLogic(nullptr);
}

//----------------------------------------------------------------------------
void vtkDMMLSliceViewInteractorStyle::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "\nSlice Logic:\n";
  if (this->SliceLogic)
    {
    this->SliceLogic->PrintSelf(os, indent.GetNextIndent());
    }
}

//----------------------------------------------------------------------------
bool vtkDMMLSliceViewInteractorStyle::DelegateInteractionEventToDisplayableManagers(vtkEventData* inputEventData)
{
  if (!this->SliceLogic || !this->DisplayableManagers || !inputEventData)
    {
    //this->SetMouseCursor(VTK_CURSOR_DEFAULT);
    return false;
    }
  vtkDMMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();
  if (!sliceNode)
    {
    return false;
    }
  vtkDMMLApplicationLogic* appLogic = nullptr;
  if (this->GetSliceLogic())
    {
    appLogic = this->GetSliceLogic()->GetDMMLApplicationLogic();
    }
  if (appLogic)
    {
    appLogic->PauseRender();
    }
  // Get display and world position
  int* displayPositionInt = this->GetInteractor()->GetEventPosition();
  vtkRenderer* pokedRenderer = this->GetInteractor()->FindPokedRenderer(displayPositionInt[0], displayPositionInt[1]);
  double displayPosition[4] =
    {
    static_cast<double>(displayPositionInt[0] - pokedRenderer->GetOrigin()[0]),
    static_cast<double>(displayPositionInt[1] - pokedRenderer->GetOrigin()[1]),
    0.0,
    1.0
    };
  vtkMatrix4x4 * xyToRasMatrix = sliceNode->GetXYToRAS();
  double worldPosition[4] = { 0.0, 0.0, 0.0, 1.0 };
  xyToRasMatrix->MultiplyPoint(displayPosition, worldPosition);

  vtkNew<vtkDMMLInteractionEventData> ed;
  ed->SetType(inputEventData->GetType());
  int displayPositionCorrected[2] = { displayPositionInt[0] - pokedRenderer->GetOrigin()[0], displayPositionInt[1] - pokedRenderer->GetOrigin()[1] };
  ed->SetDisplayPosition(displayPositionCorrected);
  ed->SetWorldPosition(worldPosition);
  ed->SetMouseMovedSinceButtonDown(this->MouseMovedSinceButtonDown);
  ed->SetAttributesFromInteractor(this->GetInteractor());

  // Update cursor position
  if (this->EnableCursorUpdate)
    {
    if (inputEventData->GetType() == vtkCommand::MouseMoveEvent)
      {
      // Update the cursor position (show coordinates of current position in the data probe, etc.)
      vtkDMMLScene* scene = this->SliceLogic->GetDMMLScene();
      vtkDMMLCrosshairNode* crosshairNode = vtkDMMLCrosshairDisplayableManager::FindCrosshairNode(scene);
      if (crosshairNode)
        {
        double xyz[3] = { 0.0 };
        vtkDMMLAbstractSliceViewDisplayableManager::ConvertDeviceToXYZ(this->GetInteractor(),
          sliceNode, displayPositionInt[0], displayPositionInt[1], xyz);
        crosshairNode->SetCursorPositionXYZ(xyz, sliceNode);
        }
      }
    else if (inputEventData->GetType() == vtkCommand::LeaveEvent)
      {
      vtkDMMLScene* scene = this->SliceLogic->GetDMMLScene();
      vtkDMMLCrosshairNode* crosshairNode = vtkDMMLCrosshairDisplayableManager::FindCrosshairNode(scene);
      if (crosshairNode)
        {
        crosshairNode->SetCursorPositionInvalid();
        }
      }
    }

  bool result = this->DelegateInteractionEventDataToDisplayableManagers(ed);
  if (appLogic)
    {
    appLogic->ResumeRender();
    }
  return result;
}

//----------------------------------------------------------------------------
void vtkDMMLSliceViewInteractorStyle::SetActionEnabled(int actionsMask, bool enable /*=true*/)
{
  if (actionsMask & SetCursorPosition)
    {
    this->EnableCursorUpdate = enable;
    }

  vtkDMMLCrosshairDisplayableManager* crosshairDisplayableManager = this->GetCrosshairDisplayableManager();
  if (crosshairDisplayableManager)
    {
    int actionsEnabled = crosshairDisplayableManager->GetActionsEnabled();
    if (enable)
      {
      actionsEnabled |= actionsMask;
      }
    else
      {
      actionsEnabled  &= (~actionsMask);
      }
    crosshairDisplayableManager->SetActionsEnabled(actionsEnabled);
    }

  vtkDMMLScalarBarDisplayableManager* scalarBarDisplayableManager = this->GetScalarBarDisplayableManager();
  if (scalarBarDisplayableManager)
    {
    scalarBarDisplayableManager->SetAdjustBackgroundWindowLevelEnabled((actionsMask & AdjustWindowLevelBackground) != 0);
    scalarBarDisplayableManager->SetAdjustForegroundWindowLevelEnabled((actionsMask & AdjustWindowLevelForeground) != 0);
    }
}

//----------------------------------------------------------------------------
bool vtkDMMLSliceViewInteractorStyle::GetActionEnabled(int actionsMask)
{
  int actionsEnabled = 0;

  if (this->EnableCursorUpdate)
    {
    actionsEnabled |= SetCursorPosition;
    }

  vtkDMMLCrosshairDisplayableManager* crosshairDisplayableManager = this->GetCrosshairDisplayableManager();
  if (crosshairDisplayableManager)
    {
    actionsEnabled |= crosshairDisplayableManager->GetActionsEnabled();
    }

   vtkDMMLScalarBarDisplayableManager* scalarBarDisplayableManager = this->GetScalarBarDisplayableManager();
  if (scalarBarDisplayableManager)
    {
    if (scalarBarDisplayableManager->GetAdjustBackgroundWindowLevelEnabled())
      {
      actionsEnabled |= AdjustWindowLevelBackground;
      }
    if (scalarBarDisplayableManager->GetAdjustForegroundWindowLevelEnabled())
      {
      actionsEnabled |= AdjustWindowLevelForeground;
      }
    }

  return (actionsEnabled & actionsMask) == actionsMask;
}

//----------------------------------------------------------------------------
vtkDMMLCrosshairDisplayableManager* vtkDMMLSliceViewInteractorStyle::GetCrosshairDisplayableManager()
{
  int numberOfDisplayableManagers = this->DisplayableManagers->GetDisplayableManagerCount();
  for (int displayableManagerIndex = 0; displayableManagerIndex < numberOfDisplayableManagers; ++displayableManagerIndex)
    {
    vtkDMMLCrosshairDisplayableManager* displayableManager = vtkDMMLCrosshairDisplayableManager::SafeDownCast(
      this->DisplayableManagers->GetNthDisplayableManager(displayableManagerIndex));
    if (displayableManager)
      {
      return displayableManager;
      }
    }
  return nullptr;
}

//----------------------------------------------------------------------------
vtkDMMLScalarBarDisplayableManager* vtkDMMLSliceViewInteractorStyle::GetScalarBarDisplayableManager()
{
  int numberOfDisplayableManagers = this->DisplayableManagers->GetDisplayableManagerCount();
  for (int displayableManagerIndex = 0; displayableManagerIndex < numberOfDisplayableManagers; ++displayableManagerIndex)
    {
    vtkDMMLScalarBarDisplayableManager* displayableManager = vtkDMMLScalarBarDisplayableManager::SafeDownCast(
      this->DisplayableManagers->GetNthDisplayableManager(displayableManagerIndex));
    if (displayableManager)
      {
      return displayableManager;
      }
    }
  return nullptr;
}
