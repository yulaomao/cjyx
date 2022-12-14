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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// DMMLDisplayableManager includes
#include "vtkDMMLAbstractThreeDViewDisplayableManager.h"

// DMML includes
#include "vtkDMMLThreeDViewInteractorStyle.h"
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>

// STD includes
#include <cassert>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLAbstractThreeDViewDisplayableManager);

//----------------------------------------------------------------------------
// vtkDMMLAbstractThreeDViewDisplayableManager methods

//----------------------------------------------------------------------------
vtkDMMLAbstractThreeDViewDisplayableManager::vtkDMMLAbstractThreeDViewDisplayableManager() = default;

//----------------------------------------------------------------------------
vtkDMMLAbstractThreeDViewDisplayableManager::~vtkDMMLAbstractThreeDViewDisplayableManager() = default;

//----------------------------------------------------------------------------
void vtkDMMLAbstractThreeDViewDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractThreeDViewDisplayableManager::OnDMMLDisplayableNodeModifiedEvent(
    vtkObject* caller)
{
  assert(vtkDMMLViewNode::SafeDownCast(caller));
#ifndef _DEBUG
  (void)caller;
#endif
  this->OnDMMLViewNodeModifiedEvent();
}

//---------------------------------------------------------------------------
vtkDMMLViewNode * vtkDMMLAbstractThreeDViewDisplayableManager::GetDMMLViewNode()
{
  return vtkDMMLViewNode::SafeDownCast(this->GetDMMLDisplayableNode());
}

//---------------------------------------------------------------------------
void vtkDMMLAbstractThreeDViewDisplayableManager::PassThroughInteractorStyleEvent(int eventid)
{
  vtkDMMLThreeDViewInteractorStyle* interactorStyle = vtkDMMLThreeDViewInteractorStyle::SafeDownCast(
              this->GetInteractor()->GetInteractorStyle());

  if (interactorStyle)
  {

  switch(eventid)
    {
    case vtkCommand::ExposeEvent:
      interactorStyle->OnExpose();
      break;

    case vtkCommand::ConfigureEvent:
      interactorStyle->OnConfigure();
      break;

    case vtkCommand::EnterEvent:
      interactorStyle->OnEnter();
      break;

    case vtkCommand::LeaveEvent:
        interactorStyle->OnLeave();
      break;

    case vtkCommand::TimerEvent:
        interactorStyle->OnTimer();
      break;

    case vtkCommand::MouseMoveEvent:
        interactorStyle->OnMouseMove();
      break;

    case vtkCommand::LeftButtonPressEvent:
        interactorStyle->OnLeftButtonDown();
      break;

    case vtkCommand::LeftButtonReleaseEvent:
        interactorStyle->OnLeftButtonUp();
        break;

    case vtkCommand::MiddleButtonPressEvent:
        interactorStyle->OnMiddleButtonDown();
      break;

    case vtkCommand::MiddleButtonReleaseEvent:
        interactorStyle->OnMiddleButtonUp();
      break;

    case vtkCommand::RightButtonPressEvent:
        interactorStyle->OnRightButtonDown();
      break;

    case vtkCommand::RightButtonReleaseEvent:
        interactorStyle->OnRightButtonUp();
      break;

    case vtkCommand::MouseWheelForwardEvent:
        interactorStyle->OnMouseWheelForward();
      break;

    case vtkCommand::MouseWheelBackwardEvent:
        interactorStyle->OnMouseWheelBackward();
      break;

    case vtkCommand::KeyPressEvent:
        interactorStyle->OnKeyDown();
        interactorStyle->OnKeyPress();
      break;

    case vtkCommand::KeyReleaseEvent:
        interactorStyle->OnKeyUp();
        interactorStyle->OnKeyRelease();
      break;

    case vtkCommand::CharEvent:
        interactorStyle->OnChar();
      break;

    case vtkCommand::DeleteEvent:
      interactorStyle->SetInteractor(nullptr);
      break;

    case vtkCommand::TDxMotionEvent:
    case vtkCommand::TDxButtonPressEvent:
    case vtkCommand::TDxButtonReleaseEvent:
      interactorStyle->DelegateTDxEvent(eventid,nullptr);
      break;
    }

    return;
  }
}

