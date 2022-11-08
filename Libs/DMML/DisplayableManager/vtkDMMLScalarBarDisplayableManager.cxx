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

// DMMLDisplayableManager includes
#include "vtkDMMLScalarBarDisplayableManager.h"

// DMML includes
#include <vtkDMMLApplicationLogic.h>
#include <vtkDMMLColorNode.h>
#include <vtkDMMLDisplayNode.h>
#include <vtkDMMLInteractionEventData.h>
#include <vtkDMMLInteractionNode.h>
#include <vtkDMMLLightBoxRendererManagerProxy.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceLogic.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLWindowLevelWidget.h>

// VTK includes
#include <vtkActor2D.h>
#include <vtkCallbackCommand.h>
#include <vtkCellArray.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPickingManager.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProp.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>
#include <vtkVersion.h>

// STD includes
#include <algorithm>
#include <cassert>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLScalarBarDisplayableManager );

//---------------------------------------------------------------------------
class vtkDMMLScalarBarDisplayableManager::vtkInternal
{
public:
  vtkInternal(vtkDMMLScalarBarDisplayableManager * external);
  ~vtkInternal();

  vtkObserverManager* GetDMMLNodesObserverManager();
  void Modified();

  // Slice
  vtkDMMLSliceNode* GetSliceNode();
  void UpdateSliceNode();

  // Build the crosshair representation
  void BuildScalarBar();

  vtkDMMLScalarBarDisplayableManager*        External;

  vtkSmartPointer<vtkDMMLWindowLevelWidget> WindowLevelWidget;
};


//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkDMMLScalarBarDisplayableManager::vtkInternal
::vtkInternal(vtkDMMLScalarBarDisplayableManager * external)
{
  this->External = external;
  this->WindowLevelWidget = vtkSmartPointer<vtkDMMLWindowLevelWidget>::New();
}

//---------------------------------------------------------------------------
vtkDMMLScalarBarDisplayableManager::vtkInternal::~vtkInternal()
{
  this->WindowLevelWidget->SetDMMLApplicationLogic(nullptr);
  this->WindowLevelWidget->SetRenderer(nullptr);
  this->WindowLevelWidget->SetSliceNode(nullptr);
}

//---------------------------------------------------------------------------
vtkObserverManager* vtkDMMLScalarBarDisplayableManager::vtkInternal::GetDMMLNodesObserverManager()
{
  return this->External->GetDMMLNodesObserverManager();
}

//---------------------------------------------------------------------------
void vtkDMMLScalarBarDisplayableManager::vtkInternal::Modified()
{
  return this->External->Modified();
}

//---------------------------------------------------------------------------
vtkDMMLSliceNode* vtkDMMLScalarBarDisplayableManager::vtkInternal
::GetSliceNode()
{
  return this->External->GetDMMLSliceNode();
}

//---------------------------------------------------------------------------
void vtkDMMLScalarBarDisplayableManager::vtkInternal::UpdateSliceNode()
{
  if (this->External->GetDMMLScene() == nullptr)
    {
    this->WindowLevelWidget->SetSliceNode(nullptr);
    return;
    }

  if (!this->WindowLevelWidget->GetRenderer())
    {
    vtkDMMLApplicationLogic *dmmlAppLogic = this->External->GetDMMLApplicationLogic();
    this->WindowLevelWidget->SetDMMLApplicationLogic(dmmlAppLogic);
    this->WindowLevelWidget->CreateDefaultRepresentation();
    this->WindowLevelWidget->SetRenderer(this->External->GetRenderer());
    }
  this->WindowLevelWidget->SetSliceNode(this->GetSliceNode());
}

//---------------------------------------------------------------------------
// vtkDMMLScalarBarDisplayableManager methods

//---------------------------------------------------------------------------
vtkDMMLScalarBarDisplayableManager::vtkDMMLScalarBarDisplayableManager()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkDMMLScalarBarDisplayableManager::~vtkDMMLScalarBarDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkDMMLScalarBarDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkDMMLScalarBarDisplayableManager::UpdateFromDMMLScene()
{
  this->Internal->UpdateSliceNode();
}

//---------------------------------------------------------------------------
void vtkDMMLScalarBarDisplayableManager::UnobserveDMMLScene()
{
  this->Internal->WindowLevelWidget->SetSliceNode(nullptr);
}

//---------------------------------------------------------------------------
void vtkDMMLScalarBarDisplayableManager::Create()
{
  // Setup the SliceNode, ScalarBarNode
  this->Internal->UpdateSliceNode();
}

//---------------------------------------------------------------------------
void vtkDMMLScalarBarDisplayableManager::AdditionalInitializeStep()
{
  // Build the initial crosshair representation
  //this->Internal->BuildScalarBar();
}

//---------------------------------------------------------------------------
void vtkDMMLScalarBarDisplayableManager::OnDMMLSliceNodeModifiedEvent()
{
}

//---------------------------------------------------------------------------
bool vtkDMMLScalarBarDisplayableManager::CanProcessInteractionEvent(vtkDMMLInteractionEventData* eventData, double &closestDistance2)
{
  int eventid = eventData->GetType();
  if (eventid == vtkCommand::LeaveEvent)
    {
    this->Internal->WindowLevelWidget->Leave(eventData);
    }

  return this->Internal->WindowLevelWidget->CanProcessInteractionEvent(eventData, closestDistance2);
}

//---------------------------------------------------------------------------
bool vtkDMMLScalarBarDisplayableManager::ProcessInteractionEvent(vtkDMMLInteractionEventData* eventData)
{
  bool processed = this->Internal->WindowLevelWidget->ProcessInteractionEvent(eventData);
  if (this->Internal->WindowLevelWidget->GetNeedToRender())
    {
    this->RequestRender();
    this->Internal->WindowLevelWidget->NeedToRenderOff();
    }
  return processed;
}

//---------------------------------------------------------------------------
void vtkDMMLScalarBarDisplayableManager::SetAdjustForegroundWindowLevelEnabled(bool enabled)
{
  this->Internal->WindowLevelWidget->SetForegroundVolumeEditable(enabled);
}

//---------------------------------------------------------------------------
bool vtkDMMLScalarBarDisplayableManager::GetAdjustForegroundWindowLevelEnabled()
{
  return this->Internal->WindowLevelWidget->GetForegroundVolumeEditable();
}

//---------------------------------------------------------------------------
void vtkDMMLScalarBarDisplayableManager::SetAdjustBackgroundWindowLevelEnabled(bool enabled)
{
  this->Internal->WindowLevelWidget->SetBackgroundVolumeEditable(enabled);
}

//---------------------------------------------------------------------------
bool vtkDMMLScalarBarDisplayableManager::GetAdjustBackgroundWindowLevelEnabled()
{
  return this->Internal->WindowLevelWidget->GetBackgroundVolumeEditable();
}


//---------------------------------------------------------------------------
vtkDMMLWindowLevelWidget* vtkDMMLScalarBarDisplayableManager::GetWindowLevelWidget()
{
  return this->Internal->WindowLevelWidget;
}
