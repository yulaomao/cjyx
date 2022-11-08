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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// DMMLDisplayableManager includes
#include "vtkDMMLCrosshairDisplayableManager.h"

// DMML includes
#include <vtkDMMLApplicationLogic.h>
#include <vtkDMMLColorNode.h>
#include <vtkDMMLCrosshairNode.h>
#include <vtkDMMLDisplayNode.h>
#include <vtkDMMLInteractionNode.h>
#include <vtkDMMLLightBoxRendererManagerProxy.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceIntersectionWidget.h>
#include <vtkDMMLSliceLogic.h>
#include <vtkDMMLSliceNode.h>

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
vtkStandardNewMacro(vtkDMMLCrosshairDisplayableManager );

//---------------------------------------------------------------------------
class vtkDMMLCrosshairDisplayableManager::vtkInternal
{
public:
  vtkInternal(vtkDMMLCrosshairDisplayableManager * external);
  ~vtkInternal();

  vtkObserverManager* GetDMMLNodesObserverManager();
  void Modified();

  // Slice
  vtkDMMLSliceNode* GetSliceNode();
  void UpdateSliceNode();
  void UpdateIntersectingSliceNodes();
  // Crosshair
  void SetCrosshairNode(vtkDMMLCrosshairNode* crosshairNode);

  // Actors
  void SetActor(vtkActor2D* prop) {Actor = prop;};

  // Build the crosshair representation
  void BuildCrosshair();

  // Add a line to the crosshair in display coordinates (needs to be
  // passed the points and cellArray to manipulate).
  void AddCrosshairLine(vtkPoints *pts, vtkCellArray *cellArray,
                        int p1x, int p1y, int p2x, int p2y);

  // Has crosshair position changed?
  bool HasCrosshairPositionChanged();

  // Has crosshair property changed?
  bool HasCrosshairPropertyChanged();

  vtkDMMLCrosshairDisplayableManager*        External;
  int                                        PickState;
  int                                        ActionState;
  vtkSmartPointer<vtkActor2D>                Actor;
  vtkWeakPointer<vtkRenderer>                LightBoxRenderer;

  vtkWeakPointer<vtkDMMLCrosshairNode>       CrosshairNode;
  int CrosshairMode;
  int CrosshairThickness;
  double CrosshairPosition[3];

  vtkSmartPointer<vtkDMMLSliceIntersectionWidget> SliceIntersectionWidget;
};


//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkDMMLCrosshairDisplayableManager::vtkInternal
::vtkInternal(vtkDMMLCrosshairDisplayableManager * external)
{
  this->External = external;
  this->CrosshairNode = nullptr;
  this->Actor = nullptr;
  this->LightBoxRenderer = nullptr;
  this->CrosshairMode = -1;
  this->CrosshairThickness = -1;
  this->CrosshairPosition[0] = 0.0;
  this->CrosshairPosition[1] = 0.0;
  this->CrosshairPosition[2] = 0.0;
  this->SliceIntersectionWidget = vtkSmartPointer<vtkDMMLSliceIntersectionWidget>::New();
}

//---------------------------------------------------------------------------
vtkDMMLCrosshairDisplayableManager::vtkInternal::~vtkInternal()
{
  this->SetCrosshairNode(nullptr);
  this->LightBoxRenderer = nullptr;

  if (this->SliceIntersectionWidget)
    {
    this->SliceIntersectionWidget->SetDMMLApplicationLogic(nullptr);
    this->SliceIntersectionWidget->SetRenderer(nullptr);
    this->SliceIntersectionWidget->SetSliceNode(nullptr);
    }

  // everything should be empty
  assert(this->CrosshairNode == nullptr);
}

//---------------------------------------------------------------------------
vtkObserverManager* vtkDMMLCrosshairDisplayableManager::vtkInternal::GetDMMLNodesObserverManager()
{
  return this->External->GetDMMLNodesObserverManager();
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager::vtkInternal::Modified()
{
  return this->External->Modified();
}

//---------------------------------------------------------------------------
bool vtkDMMLCrosshairDisplayableManager::vtkInternal::HasCrosshairPositionChanged()
{
  if (this->CrosshairNode.GetPointer() == nullptr)
    {
    return false;
    }

  // update the position of the actor
  double *ras = this->CrosshairNode->GetCrosshairRAS();
  double *lastRas = this->CrosshairPosition;
  double eps = 1.0e-12;
  if (fabs(lastRas[0] - ras[0]) > eps
    || fabs(lastRas[1] - ras[1]) > eps
    || fabs(lastRas[2] - ras[2]) > eps)
    {
    return true;
    }
  else
    {
    return false;
    }
}

//---------------------------------------------------------------------------
bool vtkDMMLCrosshairDisplayableManager::vtkInternal::HasCrosshairPropertyChanged()
{
  if (this->CrosshairNode.GetPointer() == nullptr)
    {
    return false;
    }

  if (this->CrosshairMode != this->CrosshairNode->GetCrosshairMode()
    || this->CrosshairThickness != this->CrosshairNode->GetCrosshairThickness())
    {
    return true;
    }
  else
    {
    return false;
    }
}

//---------------------------------------------------------------------------
vtkDMMLSliceNode* vtkDMMLCrosshairDisplayableManager::vtkInternal::GetSliceNode()
{
  return this->External->GetDMMLSliceNode();
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager::vtkInternal::UpdateSliceNode()
{
  assert(!this->GetSliceNode() || this->GetSliceNode()->GetLayoutName());

  // search for the Crosshair node
  vtkDMMLCrosshairNode* crosshairNode = vtkDMMLCrosshairDisplayableManager::FindCrosshairNode(this->External->GetDMMLScene());
  this->SetCrosshairNode(crosshairNode);
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager::vtkInternal::UpdateIntersectingSliceNodes()
{
  if (this->External->GetDMMLScene() == nullptr)
    {
    this->SliceIntersectionWidget->SetSliceNode(nullptr);
    return;
    }

  vtkDMMLApplicationLogic *dmmlAppLogic = this->External->GetDMMLApplicationLogic();

  if (!this->SliceIntersectionWidget->GetRenderer())
    {
    this->SliceIntersectionWidget->SetDMMLApplicationLogic(dmmlAppLogic);
    this->SliceIntersectionWidget->CreateDefaultRepresentation();
    this->SliceIntersectionWidget->SetRenderer(this->External->GetRenderer());
    this->SliceIntersectionWidget->SetSliceNode(this->GetSliceNode());
    }
  else
    {
    this->SliceIntersectionWidget->SetSliceNode(this->GetSliceNode());
    }
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager::vtkInternal
::SetCrosshairNode(vtkDMMLCrosshairNode* crosshairNode)
{
  if (this->CrosshairNode == crosshairNode)
    {
    return;
    }
  vtkSetAndObserveDMMLNodeMacro(this->CrosshairNode, crosshairNode);
}

//---------------------------------------------------------------------------
vtkDMMLCrosshairNode* vtkDMMLCrosshairDisplayableManager::FindCrosshairNode(vtkDMMLScene* scene)
{
  if (scene == nullptr)
    {
    return nullptr;
    }

  vtkDMMLNode* node;
  vtkCollectionSimpleIterator it;
  vtkSmartPointer<vtkCollection> crosshairs;
  crosshairs.TakeReference(scene->GetNodesByClass("vtkDMMLCrosshairNode"));
  for (crosshairs->InitTraversal(it);
       (node = (vtkDMMLNode*)crosshairs->GetNextItemAsObject(it)) ;)
    {
    vtkDMMLCrosshairNode* crosshairNode =
      vtkDMMLCrosshairNode::SafeDownCast(node);
    if (crosshairNode
        && crosshairNode->GetCrosshairName() == std::string("default"))
      {
      return crosshairNode;
      }
    }
  // no matching crosshair node is found
  //assert(0);
  return nullptr;
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager::vtkInternal::BuildCrosshair()
{
  // Remove the old actor is any
  if (this->Actor.GetPointer())
    {
    if (this->LightBoxRenderer)
      {
      this->LightBoxRenderer->RemoveActor(this->Actor);
      }
    this->Actor = nullptr;
    }

  if (!this->CrosshairNode.GetPointer())
    {
    return;
    }

  // Get the size of the window
  const int *screenSize = this->External->GetInteractor()->GetRenderWindow()->GetScreenSize();

  // Constants in display coordinates to define the crosshair
  int negW = -1.0*screenSize[0];
  int negWminus = -5;
  int negWminus2 = -10;
  int posWplus = 5;
  int posWplus2 = 10;
  int posW = screenSize[0];

  int negH = -1.0*screenSize[1];
  int negHminus = -5;
  int negHminus2 = -10;
  int posHplus = 5;
  int posHplus2 = 10;
  int posH = screenSize[1];

  // Set up the VTK data structures
  vtkNew<vtkPolyData> polyData;
  vtkNew<vtkCellArray> cellArray;
  vtkNew<vtkPoints> points;
  polyData->SetLines(cellArray.GetPointer());
  polyData->SetPoints(points.GetPointer());

  vtkNew<vtkPolyDataMapper2D> mapper;
  vtkNew<vtkActor2D> actor;
  mapper->SetInputData(polyData.GetPointer());
  actor->SetMapper(mapper.GetPointer());

  if (this->LightBoxRenderer)
    {
    this->LightBoxRenderer->AddActor(actor.GetPointer());
    }

  // Cache the actor
  this->SetActor(actor.GetPointer());

  // Define the geometry
  switch (this->CrosshairNode->GetCrosshairMode())
    {
    case vtkDMMLCrosshairNode::NoCrosshair:
      break;
    case vtkDMMLCrosshairNode::ShowBasic:
      this->AddCrosshairLine(points.GetPointer(), cellArray.GetPointer(),
                             0, negH, 0, negHminus);
      this->AddCrosshairLine(points.GetPointer(), cellArray.GetPointer(),
                             0, posHplus, 0, posH);
      this->AddCrosshairLine(points.GetPointer(), cellArray.GetPointer(),
                             negW, 0, negWminus, 0);
      this->AddCrosshairLine(points.GetPointer(), cellArray.GetPointer(),
                             posWplus, 0, posW, 0);
      break;
    case vtkDMMLCrosshairNode::ShowIntersection:
      this->AddCrosshairLine(points.GetPointer(), cellArray.GetPointer(),
                             negW, 0, posW, 0);
      this->AddCrosshairLine(points.GetPointer(), cellArray.GetPointer(),
                             0, negH, 0, posH);
      break;
    case vtkDMMLCrosshairNode::ShowSmallBasic:
      this->AddCrosshairLine(points.GetPointer(), cellArray.GetPointer(),
                             0, negHminus2, 0, negHminus);
      this->AddCrosshairLine(points.GetPointer(), cellArray.GetPointer(),
                             0, posHplus, 0, posHplus2);
      this->AddCrosshairLine(points.GetPointer(), cellArray.GetPointer(),
                             negWminus2, 0, negWminus, 0);
      this->AddCrosshairLine(points.GetPointer(), cellArray.GetPointer(),
                             posWplus, 0, posWplus2, 0);
      break;
    case vtkDMMLCrosshairNode::ShowSmallIntersection:
      this->AddCrosshairLine(points.GetPointer(), cellArray.GetPointer(),
                             0, negHminus2, 0, posHplus2);
      this->AddCrosshairLine(points.GetPointer(), cellArray.GetPointer(),
                             negWminus2, 0, posWplus2, 0);
      break;
    default:
      break;
    }

  // Set the properties
  //

  // Line Width
  if (this->CrosshairNode->GetCrosshairThickness() == vtkDMMLCrosshairNode::Fine)
    {
    actor->GetProperty()->SetLineWidth(1);
    }
  else if (this->CrosshairNode->GetCrosshairThickness() == vtkDMMLCrosshairNode::Medium)
    {
    actor->GetProperty()->SetLineWidth(3);
    }
  else if (this->CrosshairNode->GetCrosshairThickness() == vtkDMMLCrosshairNode::Thick)
    {
    actor->GetProperty()->SetLineWidth(5);
    }

  // Color
  actor->GetProperty()->SetColor(1.0, 0.8, 0.1);
  actor->GetProperty()->SetOpacity(1.0);


  // Set the visibility
  if (this->CrosshairNode->GetCrosshairMode() == vtkDMMLCrosshairNode::NoCrosshair)
    {
    actor->VisibilityOff();
    }
  else
    {
    actor->VisibilityOn();
    }

  this->CrosshairMode = this->CrosshairNode->GetCrosshairMode();
  this->CrosshairThickness = this->CrosshairNode->GetCrosshairThickness();
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager::vtkInternal::AddCrosshairLine(vtkPoints *pts, vtkCellArray *cellArray, int p1x, int p1y, int p2x, int p2y)
{
  vtkIdType p1 = pts->InsertNextPoint(p1x, p1y, 0);
  vtkIdType p2 = pts->InsertNextPoint(p2x, p2y, 0);

  cellArray->InsertNextCell(2);
  cellArray->InsertCellPoint(p1);
  cellArray->InsertCellPoint(p2);
}

//---------------------------------------------------------------------------
// vtkDMMLCrosshairDisplayableManager methods

//---------------------------------------------------------------------------
vtkDMMLCrosshairDisplayableManager::vtkDMMLCrosshairDisplayableManager()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkDMMLCrosshairDisplayableManager::~vtkDMMLCrosshairDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager::ObserveDMMLScene()
{
  this->Internal->BuildCrosshair();
  this->Superclass::ObserveDMMLScene();
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager::UpdateFromDMMLScene()
{
  this->Internal->UpdateSliceNode();
  this->Internal->UpdateIntersectingSliceNodes();
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager::UnobserveDMMLScene()
{
  this->Internal->SliceIntersectionWidget->SetSliceNode(nullptr);
  this->Internal->SetCrosshairNode(nullptr);
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager::OnDMMLNodeModified(
    vtkDMMLNode* vtkNotUsed(node))
{
  // update the properties and style of the crosshair
  bool builtCrosshair = false;
  if (this->Internal->HasCrosshairPropertyChanged())
    {
    this->Internal->BuildCrosshair();
    builtCrosshair = true;
    }

  // update the position of the actor
  if ((this->Internal->HasCrosshairPositionChanged() || builtCrosshair)
      && this->Internal->Actor)
    {
    double xyz[3];
    double *ras = this->Internal->CrosshairNode->GetCrosshairRAS();
    this->ConvertRASToXYZ(ras, xyz);

    this->Internal->Actor->SetPosition(xyz[0], xyz[1]);

    // put the actor in the right lightbox
    if (this->GetLightBoxRendererManagerProxy())
      {
      int id = (int) (floor(xyz[2] + 0.5)); // round to find the lightbox
      vtkRenderer *renderer
        = this->GetLightBoxRendererManagerProxy()->GetRenderer(id);
      if (renderer == nullptr)
        {
        // crosshair must not be displayed in this view
        this->Internal->Actor->SetVisibility(false);
        }
      else
        {
        // crosshair must be displayed in this view
        if (this->Internal->LightBoxRenderer == renderer)
          {
          this->Internal->Actor->SetVisibility(true);
          }
        else
          {
          if (this->Internal->LightBoxRenderer)
            {
            this->Internal->LightBoxRenderer->RemoveActor(this->Internal->Actor);
            }
          this->Internal->Actor->SetVisibility(true);
          renderer->AddActor(this->Internal->Actor);
          this->Internal->LightBoxRenderer = renderer;
          }
        }
      }

    double *lastRas = this->Internal->CrosshairPosition;
    lastRas[0] = ras[0];
    lastRas[1] = ras[1];
    lastRas[2] = ras[2];
    }

  // Request a render
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager::Create()
{
  // Setup the SliceNode, CrosshairNode
  this->Internal->UpdateSliceNode();
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager::AdditionalInitializeStep()
{
  // Build the initial crosshair representation
  this->Internal->BuildCrosshair();
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager::OnDMMLSliceNodeModifiedEvent()
{
  if (this->Internal->CrosshairNode)
    {
    // slice position may have changed

    // change last crosshair position to force a position update
    this->Internal->CrosshairPosition[0] += 100;

    // update cursor RAS position from XYZ (normalized screen) position
    double xyz[3] = { 0.0 };
    vtkDMMLSliceNode *crosshairSliceNode = this->Internal->CrosshairNode->GetCursorPositionXYZ(xyz);
    if (crosshairSliceNode != nullptr && crosshairSliceNode == this->Internal->GetSliceNode())
      {
      this->Internal->CrosshairNode->SetCursorPositionXYZ(xyz, crosshairSliceNode);
      }

    this->OnDMMLNodeModified(this->Internal->CrosshairNode);
    }
}

//---------------------------------------------------------------------------
bool vtkDMMLCrosshairDisplayableManager::CanProcessInteractionEvent(vtkDMMLInteractionEventData* eventData, double &closestDistance2)
{
  if (!this->Internal->SliceIntersectionWidget)
    {
    return false;
    }
  return this->Internal->SliceIntersectionWidget->CanProcessInteractionEvent(eventData, closestDistance2);
}

//---------------------------------------------------------------------------
bool vtkDMMLCrosshairDisplayableManager::ProcessInteractionEvent(vtkDMMLInteractionEventData* eventData)
{
  if (!this->Internal->SliceIntersectionWidget)
    {
    return false;
    }
  bool processed = this->Internal->SliceIntersectionWidget->ProcessInteractionEvent(eventData);
  if (this->Internal->SliceIntersectionWidget && this->Internal->SliceIntersectionWidget->GetNeedToRender())
    {
    this->Internal->SliceIntersectionWidget->NeedToRenderOff();
    this->RequestRender();
    }
  return processed;
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager::SetActionsEnabled(int actions)
{
  this->Internal->SliceIntersectionWidget->SetActionsEnabled(actions);
}

//---------------------------------------------------------------------------
int vtkDMMLCrosshairDisplayableManager::GetActionsEnabled()
{
  return this->Internal->SliceIntersectionWidget->GetActionsEnabled();
}

//---------------------------------------------------------------------------
vtkDMMLSliceIntersectionWidget* vtkDMMLCrosshairDisplayableManager::GetSliceIntersectionWidget()
{
  return this->Internal->SliceIntersectionWidget;
}

//---------------------------------------------------------------------------
int vtkDMMLCrosshairDisplayableManager::GetMouseCursor()
{
  if (!this->Internal->SliceIntersectionWidget)
    {
    return VTK_CURSOR_DEFAULT;
    }
  return this->Internal->SliceIntersectionWidget->GetMouseCursor();
}

//---------------------------------------------------------------------------
void vtkDMMLCrosshairDisplayableManager::SetHasFocus(bool hasFocus, vtkDMMLInteractionEventData* eventData)
{
  if (!hasFocus && this->Internal->SliceIntersectionWidget)
    {
    this->Internal->SliceIntersectionWidget->Leave(eventData);
    }
}