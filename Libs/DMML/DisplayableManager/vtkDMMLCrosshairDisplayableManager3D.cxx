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

  This file was originally developed by Andras Lasso (PerkLab, Queen's University).

==============================================================================*/

// DMMLDisplayableManager includes
#include "vtkDMMLCrosshairDisplayableManager3D.h"
#include "vtkDMMLCrosshairDisplayableManager.h"

// DMML includes
#include <vtkDMMLApplicationLogic.h>
#include <vtkDMMLCrosshairNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkHandleWidget.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointHandleRepresentation3D.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

// STD includes
#include <algorithm>
#include <cassert>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLCrosshairDisplayableManager3D );

//---------------------------------------------------------------------------
class vtkDMMLCrosshairDisplayableManager3D::vtkInternal
{
public:
  vtkInternal(vtkDMMLCrosshairDisplayableManager3D * external);
  ~vtkInternal();

  vtkObserverManager* GetDMMLNodesObserverManager();
  void Modified();

  // Crosshair
  void SetCrosshairNode(vtkDMMLCrosshairNode* crosshairNode);

  // Build the crosshair representation
  void BuildCrosshair();

  vtkDMMLCrosshairDisplayableManager3D* External;

  vtkWeakPointer<vtkRenderWindowInteractor> RenderWindowInteractor;
  vtkSmartPointer<vtkPointHandleRepresentation3D> CrosshairRepresentation;
  vtkSmartPointer<vtkHandleWidget> CrosshairWidget;

  vtkWeakPointer<vtkDMMLCrosshairNode> CrosshairNode;
  int CrosshairMode;
  int CrosshairThickness;
  double CrosshairPosition[3];
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkDMMLCrosshairDisplayableManager3D::vtkInternal
::vtkInternal(vtkDMMLCrosshairDisplayableManager3D * external)
{
  this->External = external;
  this->CrosshairMode = -1;
  this->CrosshairThickness = -1;
  this->CrosshairPosition[0] = 0.0;
  this->CrosshairPosition[1] = 0.0;
  this->CrosshairPosition[2] = 0.0;

  this->CrosshairRepresentation = vtkSmartPointer<vtkPointHandleRepresentation3D>::New();
  this->CrosshairRepresentation->SetPlaceFactor(2.5);
  this->CrosshairRepresentation->SetHandleSize(30);
  this->CrosshairRepresentation->GetProperty()->SetColor(1.0, 0.8, 0.1);

  this->CrosshairWidget = vtkSmartPointer<vtkHandleWidget>::New();
  this->CrosshairWidget->SetRepresentation(this->CrosshairRepresentation);
  this->CrosshairWidget->EnabledOff();
  this->CrosshairWidget->ProcessEventsOff();
}

//---------------------------------------------------------------------------
vtkDMMLCrosshairDisplayableManager3D::vtkInternal::~vtkInternal()
{
  this->SetCrosshairNode(nullptr);
}

//---------------------------------------------------------------------------
vtkObserverManager* vtkDMMLCrosshairDisplayableManager3D::vtkInternal::GetDMMLNodesObserverManager()
{
  return this->External->GetDMMLNodesObserverManager();
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager3D::vtkInternal::Modified()
{
  return this->External->Modified();
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager3D::vtkInternal
::SetCrosshairNode(vtkDMMLCrosshairNode* crosshairNode)
{
  if (this->CrosshairNode == crosshairNode)
    {
    return;
    }
  vtkSetAndObserveDMMLNodeMacro(this->CrosshairNode, crosshairNode);
  this->BuildCrosshair();
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager3D::vtkInternal::BuildCrosshair()
{
  vtkRenderWindowInteractor* interactor = this->External->GetInteractor();
  if (!this->CrosshairNode.GetPointer() || !interactor)
    {
    this->CrosshairWidget->SetInteractor(nullptr);
    return;
    }

  this->CrosshairMode = this->CrosshairNode->GetCrosshairMode();
  if (this->CrosshairNode->GetCrosshairMode() == vtkDMMLCrosshairNode::NoCrosshair)
    {
    this->CrosshairWidget->EnabledOff();
    return;
    }
  this->CrosshairWidget->SetInteractor(interactor);
  this->CrosshairWidget->EnabledOn();

  const int *screenSize = interactor->GetRenderWindow()->GetScreenSize();

  // Handle size is defined a percentage of screen size to accommodate high-DPI screens
  double handleSizeInScreenSizePercent = 5;
  if (this->CrosshairNode->GetCrosshairMode() == vtkDMMLCrosshairNode::ShowSmallBasic
    || this->CrosshairNode->GetCrosshairMode() == vtkDMMLCrosshairNode::ShowSmallIntersection)
    {
    handleSizeInScreenSizePercent = 2.5;
    }
  double handleSizeInPixels = double(screenSize[1])*(0.01*handleSizeInScreenSizePercent);
  this->CrosshairRepresentation->SetHandleSize(handleSizeInPixels);

  // Line Width
  // Base width is 1 on a full HD display.
  double baseWidth = 1 + int(screenSize[1] / 1000);
  switch (this->CrosshairNode->GetCrosshairThickness())
    {
    case vtkDMMLCrosshairNode::Medium:
      this->CrosshairRepresentation->GetProperty()->SetLineWidth(baseWidth * 2);
      break;
    case vtkDMMLCrosshairNode::Thick:
      this->CrosshairRepresentation->GetProperty()->SetLineWidth(baseWidth * 3);
      break;
    case vtkDMMLCrosshairNode::Fine:
    default:
      this->CrosshairRepresentation->GetProperty()->SetLineWidth(baseWidth);
      break;
    }
  this->CrosshairThickness = this->CrosshairNode->GetCrosshairThickness();
}

//---------------------------------------------------------------------------
// vtkDMMLCrosshairDisplayableManager3D methods

//---------------------------------------------------------------------------
vtkDMMLCrosshairDisplayableManager3D::vtkDMMLCrosshairDisplayableManager3D()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkDMMLCrosshairDisplayableManager3D::~vtkDMMLCrosshairDisplayableManager3D()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager3D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager3D::ObserveDMMLScene()
{
  this->Internal->BuildCrosshair();
  this->Superclass::ObserveDMMLScene();
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager3D::UpdateFromDMMLScene()
{
  // search for the Crosshair node
  vtkDMMLCrosshairNode* crosshairNode = vtkDMMLCrosshairDisplayableManager::FindCrosshairNode(this->GetDMMLScene());
  this->Internal->SetCrosshairNode(crosshairNode);
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager3D::UnobserveDMMLScene()
{
  this->Internal->SetCrosshairNode(nullptr);
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager3D::OnDMMLNodeModified(vtkDMMLNode* node)
{
  if (!vtkDMMLCrosshairNode::SafeDownCast(node))
    {
    return;
    }

  // update the properties and style of the crosshair
  if (this->Internal->CrosshairMode != this->Internal->CrosshairNode->GetCrosshairMode()
    || (this->Internal->CrosshairMode != vtkDMMLCrosshairNode::NoCrosshair
      && this->Internal->CrosshairThickness != this->Internal->CrosshairNode->GetCrosshairThickness()))
    {
    this->Internal->BuildCrosshair();
    this->RequestRender();
    }

  // update the position of the actor
  double *ras = this->Internal->CrosshairNode->GetCrosshairRAS();
  double *lastRas = this->Internal->CrosshairPosition;
  double eps = 1.0e-12;
  if (fabs(lastRas[0] - ras[0]) > eps
    || fabs(lastRas[1] - ras[1]) > eps
    || fabs(lastRas[2] - ras[2]) > eps)
    {
    this->Internal->CrosshairRepresentation->SetWorldPosition(ras);
    lastRas[0] = ras[0];
    lastRas[1] = ras[1];
    lastRas[2] = ras[2];
    if (this->Internal->CrosshairMode != vtkDMMLCrosshairNode::NoCrosshair)
      {
      this->RequestRender();
      }
    }
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager3D::Create()
{
  this->UpdateFromDMMLScene();
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager3D::AdditionalInitializeStep()
{
  // Build the initial crosshair representation
  this->Internal->BuildCrosshair();
}
