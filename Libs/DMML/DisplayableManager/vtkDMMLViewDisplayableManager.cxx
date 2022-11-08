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
#include "vtkDMMLViewDisplayableManager.h"
#include "vtkDMMLCameraDisplayableManager.h"
#include "vtkDMMLDisplayableManagerGroup.h"

// DMML includes
#include <vtkDMMLScene.h>
#include <vtkDMMLViewNode.h>
#include <vtkDMMLCameraNode.h>
#include <vtkDMMLDisplayNode.h>
#include <vtkDMMLDisplayableNode.h>
#include <vtkDMMLSliceLogic.h>

// VTK includes
#include <vtkBoundingBox.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkFollower.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkOutlineSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkVectorText.h>

// STD includes

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLViewDisplayableManager );

//---------------------------------------------------------------------------
class vtkDMMLViewDisplayableManager::vtkInternal
{
public:
  vtkInternal(vtkDMMLViewDisplayableManager * external);
  ~vtkInternal();

  vtkDMMLCameraNode* CameraNode;

  void CreateAxis();
  void AddAxis(vtkRenderer * renderer);
  void UpdateAxis(vtkRenderer * renderer, vtkDMMLViewNode * viewNode);
  void UpdateRASBounds(double bounds[6]);

  void UpdateAxisVisibility();
  void UpdateAxisLabelVisibility();
  void UpdateAxisLabelText();
  void SetAxisLabelColor(double newAxisLabelColor[3]);

  void UpdateRenderMode();

  void UpdateStereoType();

  void UpdateBackgroundColor();

  std::vector<vtkSmartPointer<vtkFollower> > AxisLabelActors;
  std::vector<vtkSmartPointer<vtkVectorText> > AxisLabelTexts;
  std::vector<vtkSmartPointer<vtkTransformPolyDataFilter> > CenterAxisLabelTexts;
  vtkSmartPointer<vtkActor>                  BoxAxisActor;
  vtkBoundingBox*                            BoxAxisBoundingBox;
  vtkDMMLViewDisplayableManager*             External;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkDMMLViewDisplayableManager::vtkInternal::vtkInternal(vtkDMMLViewDisplayableManager * external)
{
  this->External = external;
  this->BoxAxisBoundingBox = new vtkBoundingBox();
  this->CreateAxis();
  this->CameraNode = nullptr;
}

//---------------------------------------------------------------------------
vtkDMMLViewDisplayableManager::vtkInternal::~vtkInternal()
{
  delete this->BoxAxisBoundingBox;
}

//---------------------------------------------------------------------------
void vtkDMMLViewDisplayableManager::vtkInternal::CreateAxis()
{
  // Create the default bounding box
  vtkNew<vtkOutlineSource> boxSource;
  vtkNew<vtkPolyDataMapper> boxMapper;
  boxMapper->SetInputConnection(boxSource->GetOutputPort());

  this->BoxAxisActor = vtkSmartPointer<vtkActor>::New();
  this->BoxAxisActor->SetMapper(boxMapper.GetPointer());
  this->BoxAxisActor->SetScale(1.0, 1.0, 1.0);
  this->BoxAxisActor->GetProperty()->SetColor(1.0, 0.0, 1.0);
  this->BoxAxisActor->SetPickable(0);

  this->AxisLabelActors.clear();
  this->AxisLabelTexts.clear();
  this->CenterAxisLabelTexts.clear();

  // default labels, will be overridden by view node AxisLabels
  const char* labels[6] = {"R", "A", "S", "L", "P", "I"};

  for(int i = 0; i < 6; ++i)
    {
    vtkNew<vtkVectorText> axisText;
    axisText->SetText(labels[i]);
    this->AxisLabelTexts.emplace_back(axisText.GetPointer());

    vtkNew<vtkTransformPolyDataFilter> centerAxisText;
    this->CenterAxisLabelTexts.emplace_back(centerAxisText.GetPointer());
    centerAxisText->SetInputConnection(axisText->GetOutputPort());

    vtkNew<vtkPolyDataMapper> axisMapper;
    axisMapper->SetInputConnection(centerAxisText->GetOutputPort());

    vtkNew<vtkFollower> axisActor;
    axisActor->SetMapper(axisMapper.GetPointer());
    axisActor->SetPickable(0);
    this->AxisLabelActors.emplace_back(axisActor.GetPointer());

    axisActor->GetProperty()->SetColor(1, 1, 1); // White
    axisActor->GetProperty()->SetDiffuse(0.0);
    axisActor->GetProperty()->SetAmbient(1.0);
    axisActor->GetProperty()->SetSpecular(0.0);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLViewDisplayableManager::vtkInternal::AddAxis(vtkRenderer * renderer)
{
  assert(renderer);

  renderer->AddViewProp(this->BoxAxisActor);

  for(std::size_t i = 0; i < this->AxisLabelActors.size(); ++i)
    {
    vtkFollower* actor = this->AxisLabelActors[i];
    renderer->AddViewProp(actor);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLViewDisplayableManager::vtkInternal::UpdateRASBounds(double bounds[6])
{
  //Bounds is x-min, x-max, y-min, y-max, z-min, z-max
  vtkMath::UninitializeBounds(bounds);

  if (this->External->GetDMMLViewNode() == nullptr)
    {
    return;
    }
  vtkDMMLScene *scene = this->External->GetDMMLViewNode()->GetScene();
  if (scene == nullptr)
    {
    return;
    }

  std::vector<vtkDMMLNode *> nodes;
  int nnodes = scene->GetNodesByClass("vtkDMMLDisplayableNode", nodes);
  for (int n=0; n < nnodes; n++)
    {
    vtkDMMLDisplayableNode* displayableNode =
      vtkDMMLDisplayableNode::SafeDownCast(nodes[n]);
    if (!displayableNode || vtkDMMLSliceLogic::IsSliceModelNode(displayableNode))
      {
      continue;
      }
    bool isDisplayableNodeVisibleInView = false;
    for (int i = 0; i < displayableNode->GetNumberOfDisplayNodes(); ++i)
      {
      vtkDMMLDisplayNode* displayNode = displayableNode->GetNthDisplayNode(i);
      if (displayNode && displayNode->IsDisplayableInView(
                           this->External->GetDMMLViewNode()->GetID()))
        {
        isDisplayableNodeVisibleInView = true;
        break;
        }
      }
    if (!isDisplayableNodeVisibleInView)
      {
      continue;
      }
    double nodeBounds[6];
    displayableNode->GetRASBounds(nodeBounds);
    if (vtkMath::AreBoundsInitialized(nodeBounds))
      {
      if (!vtkMath::AreBoundsInitialized(bounds))
        {
        for (int i=0; i<6; i++)
          {
          bounds[i] = nodeBounds[i];
          }
        }
      else
        {
        for (int i=0; i<3; i++)
          {
          if (bounds[2*i] > nodeBounds[2*i])
            {
            bounds[2*i] = nodeBounds[2*i];
            }
          if (bounds[2*i+1] < nodeBounds[2*i+1])
            {
            bounds[2*i+1] = nodeBounds[2*i+1];
            }
          }
        }
      }
    }

}

//---------------------------------------------------------------------------
void vtkDMMLViewDisplayableManager::vtkInternal::UpdateAxis(vtkRenderer * renderer,
                                                            vtkDMMLViewNode * viewNode)
{
  if (!renderer || !renderer->IsActiveCameraCreated() || !viewNode)
    {
    return;
    }

  // Turn off box and axis labels to compute bounds
  int boxVisibility = this->BoxAxisActor->GetVisibility();
  this->BoxAxisActor->VisibilityOff();

  int axisLabelVisibility = 0;
  for(std::size_t i = 0; i < this->AxisLabelActors.size(); ++i)
    {
    vtkFollower* actor = this->AxisLabelActors[i];
    axisLabelVisibility = actor->GetVisibility();
    actor->VisibilityOff();
    }

  // Compute bounds
  double bounds[6];
  this->UpdateRASBounds(bounds);

  //renderer->ComputeVisiblePropBounds(bounds);

  // If there are no visible props, create a default set of bounds
  vtkBoundingBox newBBox;
  if (!vtkMath::AreBoundsInitialized(bounds))
    {
    newBBox.SetBounds(-100.0, 100.0,
                      -100.0, 100.0,
                      -100.0, 100.0);
    }
  else
    {
    newBBox.SetBounds(bounds);

    // Check for degenerate bounds
    double maxLength = newBBox.GetMaxLength();
    double minPoint[3], maxPoint[3];
    newBBox.GetMinPoint(minPoint[0], minPoint[1], minPoint[2]);
    newBBox.GetMaxPoint(maxPoint[0], maxPoint[1], maxPoint[2]);

    for (unsigned int i = 0; i < 3; i++)
      {
      if (newBBox.GetLength(i) == 0.0)
        {
        minPoint[i] = minPoint[i] - maxLength * .05;
        maxPoint[i] = maxPoint[i] + maxLength * .05;
        }
      }
    newBBox.SetMinPoint(minPoint);
    newBBox.SetMaxPoint(maxPoint);
    }

  // See if bounding box has changed. If not, no need to change the axis actors.
  if (newBBox != *(this->BoxAxisBoundingBox))
    {
    *(this->BoxAxisBoundingBox) = newBBox;

    double bounds[6];
    this->BoxAxisBoundingBox->GetBounds(bounds);

    vtkNew<vtkOutlineSource> boxSource;
    boxSource->SetBounds(bounds);

    vtkNew<vtkPolyDataMapper> boxMapper;
    boxMapper->SetInputConnection(boxSource->GetOutputPort());

    this->BoxAxisActor->SetMapper(boxMapper.GetPointer());
    this->BoxAxisActor->SetScale(1.0, 1.0, 1.0);

    // Letter size as fraction of bounding box size
    double letterSize = viewNode->GetLetterSize();
    // Letter size in world coordinate system
    double letterSizeWorld = this->BoxAxisBoundingBox->GetMaxLength() * letterSize;

    for(std::size_t i = 0; i < this->AxisLabelActors.size(); ++i)
      {
      vtkFollower* actor = this->AxisLabelActors[i];
      actor->SetScale(letterSizeWorld, letterSizeWorld, letterSizeWorld);
      // Apply a transform so that center of the letter is in (0,0,0) position.
      // This is needed because the actor's origin is only used for center of
      // actor scaling and rotation. vtkFollower always places the actor's
      // (0,0,0) position to the followed position.
      // Small errors may lead to huge misalignments when the view needs to be
      // zoomed in a lot (for example, for displaying a volume with 0.001 spacing).
      double offsetToCenter[3] = { 0.0, 0.0, 0.0 };
      this->AxisLabelTexts[i]->Update();
      vtkPolyData* textPoly = this->AxisLabelTexts[i]->GetOutput();
      if (textPoly)
        {
        double bounds[6] = { 0.0, 1.0, 0.0, 1.0, 0.0, 1.0 };
        textPoly->GetBounds(bounds);
        offsetToCenter[0] = -(bounds[0] + bounds[1]) / 2.0;
        offsetToCenter[1] = -(bounds[2] + bounds[3]) / 2.0;
        offsetToCenter[2] = -(bounds[4] + bounds[5]) / 2.0;
        }
      vtkNew<vtkTransform> transform;
      transform->Translate(offsetToCenter);
      this->CenterAxisLabelTexts[i]->SetTransform(transform);
      }

    // Position the axis labels
    double center[3];
    this->BoxAxisBoundingBox->GetCenter(center);

    // Offset the center of the label by 1.5-letter distance
    // (to have an approximately one-letter space between the label and the box).
    double offset = letterSizeWorld * 1.5;
    this->AxisLabelActors[0]->SetPosition(               // R
      bounds[1] + offset,
      center[1],
      center[2]);
    this->AxisLabelActors[1]->SetPosition(               // A
      center[0],
      bounds[3] + offset,
      center[2]);
    this->AxisLabelActors[2]->SetPosition(               // S
      center[0],
      center[1],
      bounds[5] + offset);

    this->AxisLabelActors[3]->SetPosition(               // L
      bounds[0] - offset,
      center[1],
      center[2]);
    this->AxisLabelActors[4]->SetPosition(               // P
      center[0],
      bounds[2] - offset,
      center[2]);
    this->AxisLabelActors[5]->SetPosition(               // I
      center[0],
      center[1],
      bounds[4] - offset);
    }

  // Update camera and make the axis visible again
  this->BoxAxisActor->SetVisibility(boxVisibility);
  for(std::size_t i = 0; i < this->AxisLabelActors.size(); ++i)
    {
    vtkFollower* actor = this->AxisLabelActors[i];
    actor->SetCamera(renderer->GetActiveCamera());
    actor->SetVisibility(axisLabelVisibility);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLViewDisplayableManager::vtkInternal::UpdateAxisVisibility()
{
  int visible = this->External->GetDMMLViewNode()->GetBoxVisible();
  vtkDebugWithObjectMacro(this->External, << "UpdateAxisVisibility:" << visible);
  this->BoxAxisActor->SetVisibility(visible);
  this->External->RequestRender();
}

//---------------------------------------------------------------------------
void vtkDMMLViewDisplayableManager::vtkInternal::UpdateAxisLabelVisibility()
{
  vtkCamera *camera = this->External->GetRenderer() ?
    this->External->GetRenderer()->GetActiveCamera() : nullptr;
  if (!camera ||
      !this->External->GetDMMLViewNode())
    {
    return;
    }

  double orient[] = {-1,1};
  double dir[4];
  camera->GetDirectionOfProjection(dir);
  vtkMath::Normalize(dir);

  int visible = this->External->GetDMMLViewNode()->GetAxisLabelsVisible();
  int cameraBasedVilibility = this->External->GetDMMLViewNode()->GetAxisLabelsCameraDependent();

  double s2 = 0.5*sqrt(2.0);
  for  (int j=0; j<2; j++)
    {
    for  (int i=0; i<3; i++)
      {
      vtkFollower* actor = this->AxisLabelActors[i+3*j];
      if (cameraBasedVilibility)
        {
        double axis[] = {0,0,0};
        axis[i] = orient[j];
        double dot = vtkMath::Dot(axis, dir);
        if (dot > s2)
          {
          actor->SetVisibility(false);
          }
        else
          {
          actor->SetVisibility(visible);
          }
        }
      else
        {
        actor->SetVisibility(visible);
        }
      }
    }
  this->External->RequestRender();
}

//---------------------------------------------------------------------------
void vtkDMMLViewDisplayableManager::vtkInternal::UpdateAxisLabelText()
{
  vtkDMMLViewNode* viewNode = this->External->GetDMMLViewNode();
  if (!viewNode || !viewNode->GetAxisLabelsVisible())
    {
    return;
    }

  bool updateNeeded = false;
  // In this displayable manager class axis labels are ordered as +X,+Y,+Z,-X,-Y-,-Z.
  // In the view node axis labels are ordered as -X,+X,-Y,+Y,-Z,+Z.
  // viewAxisToDmAxis converts from view to displayable manager axis order.
  const int viewAxisToDmAxis[6]={3,0,4,1,5,2};
  for (int labelIndexView=0; labelIndexView<6; labelIndexView++)
    {
    if (strcmp(this->AxisLabelTexts[viewAxisToDmAxis[labelIndexView]]->GetText(),viewNode->GetAxisLabel(labelIndexView))!=0)
      {
      this->AxisLabelTexts[viewAxisToDmAxis[labelIndexView]]->SetText(viewNode->GetAxisLabel(labelIndexView));
      updateNeeded = true;
      }
    }
  if (updateNeeded)
    {
    this->External->RequestRender();
    }
}

//---------------------------------------------------------------------------
void vtkDMMLViewDisplayableManager::vtkInternal::SetAxisLabelColor(double newAxisLabelColor[3])
{
  for(std::size_t i = 0; i < this->AxisLabelActors.size(); ++i)
    {
    vtkFollower* actor = this->AxisLabelActors[i];
    actor->GetProperty()->SetColor(newAxisLabelColor);
    }
  this->External->RequestRender();
}

//---------------------------------------------------------------------------
void vtkDMMLViewDisplayableManager::vtkInternal::UpdateRenderMode()
{
  vtkDebugWithObjectMacro(this->External, << "UpdateRenderMode:" <<
                this->External->GetDMMLViewNode()->GetRenderMode());

  assert(this->External->GetRenderer()->IsActiveCameraCreated());

  vtkCamera *cam = this->External->GetRenderer()->GetActiveCamera();
  if (this->External->GetDMMLViewNode()->GetRenderMode() == vtkDMMLViewNode::Perspective)
    {
    cam->ParallelProjectionOff();
    }
  else if (this->External->GetDMMLViewNode()->GetRenderMode() == vtkDMMLViewNode::Orthographic)
    {
    cam->ParallelProjectionOn();
    cam->SetParallelScale(this->External->GetDMMLViewNode()->GetFieldOfView());
    }
}

//---------------------------------------------------------------------------
void vtkDMMLViewDisplayableManager::vtkInternal::UpdateStereoType()
{
  vtkDebugWithObjectMacro(this->External, << "UpdateStereoType:" <<
                this->External->GetDMMLViewNode()->GetStereoType());

  vtkRenderWindow * renderWindow = this->External->GetRenderer()->GetRenderWindow();
  int stereoType = this->External->GetDMMLViewNode()->GetStereoType();
  if (stereoType == vtkDMMLViewNode::RedBlue)
    {
    renderWindow->SetStereoTypeToRedBlue();
    }
  else if (stereoType == vtkDMMLViewNode::Anaglyph)
    {
    renderWindow->SetStereoTypeToAnaglyph();
    //renderWindow->SetAnaglyphColorSaturation(0.1);
    }
  else if (stereoType == vtkDMMLViewNode::QuadBuffer)
    {
    renderWindow->SetStereoTypeToCrystalEyes();
    }
  else if (stereoType == vtkDMMLViewNode::Interlaced)
    {
    renderWindow->SetStereoTypeToInterlaced();
    }
  else if (stereoType == vtkDMMLViewNode::UserDefined_1)
    {
    renderWindow->SetStereoType(101);
    }
  else if (stereoType == vtkDMMLViewNode::UserDefined_2)
    {
    renderWindow->SetStereoType(102);
    }
  else if (stereoType == vtkDMMLViewNode::UserDefined_3)
    {
    renderWindow->SetStereoType(103);
    }

  renderWindow->SetStereoRender(stereoType != vtkDMMLViewNode::NoStereo);
  this->External->RequestRender();
}

//---------------------------------------------------------------------------
void vtkDMMLViewDisplayableManager::vtkInternal::UpdateBackgroundColor()
{
  double backgroundColor[3] = {0.0, 0.0, 0.0};
  this->External->GetDMMLViewNode()->GetBackgroundColor(backgroundColor);
  double backgroundColor2[3] = {0.0, 0.0, 0.0};
  this->External->GetDMMLViewNode()->GetBackgroundColor2(backgroundColor2);
  vtkDebugWithObjectMacro(this->External, << "UpdateBackgroundColor (" <<
                backgroundColor[0] << ", " << backgroundColor[1] << ", "
                << backgroundColor[2] << ")");
  this->External->GetRenderer()->SetBackground(backgroundColor);
  this->External->GetRenderer()->SetBackground2(backgroundColor2);
  bool gradient = backgroundColor[0] != backgroundColor2[0] ||
                  backgroundColor[1] != backgroundColor2[1] ||
                  backgroundColor[2] != backgroundColor2[2];
  this->External->GetRenderer()->SetGradientBackground(gradient);

  // If new background color is White, switch axis color label to black
  if (backgroundColor[0] == 1.0 && backgroundColor[1] == 1.0 && backgroundColor[2] == 1.0)
    {
    double black[3] = {0.0, 0.0, 0.0};
    this->SetAxisLabelColor(black);
    }
  else
    {
    double white[3] = {1.0, 1.0, 1.0};
    this->SetAxisLabelColor(white);
    }

  this->External->RequestRender();
}

//---------------------------------------------------------------------------
// vtkDMMLViewDisplayableManager methods

//---------------------------------------------------------------------------
vtkDMMLViewDisplayableManager::vtkDMMLViewDisplayableManager()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkDMMLViewDisplayableManager::~vtkDMMLViewDisplayableManager()
{
  this->SetAndObserveCameraNode(nullptr);
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkDMMLViewDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkDMMLViewDisplayableManager::AdditionalInitializeStep()
{
  // TODO: Listen to ModifiedEvent and update the box coords if needed
  this->AddDMMLDisplayableManagerEvent(vtkDMMLViewNode::ResetFocalPointRequestedEvent);
}

//---------------------------------------------------------------------------
void vtkDMMLViewDisplayableManager::Create()
{
  this->Superclass::Create();

  assert(this->GetRenderer());
  assert(this->GetDMMLViewNode());

  this->Internal->AddAxis(this->GetRenderer());

  // CameraNodeDisplayableManager is expected to be instantiated !
  vtkDMMLCameraDisplayableManager * cameraDisplayableManager =
      vtkDMMLCameraDisplayableManager::SafeDownCast(
          this->GetDMMLDisplayableManagerGroup()->GetDisplayableManagerByClassName(
              "vtkDMMLCameraDisplayableManager"));
  assert(cameraDisplayableManager);

  // Listen for ActiveCameraChangedEvent
  // \tbd active camera should be set to view node instead and only observing
  //  view node should be necessary.
  cameraDisplayableManager->AddObserver(vtkDMMLCameraDisplayableManager::ActiveCameraChangedEvent,
                                        this->GetWidgetsCallbackCommand());

  this->SetAndObserveCameraNode(cameraDisplayableManager->GetCameraNode());

  this->UpdateFromViewNode();
}

//---------------------------------------------------------------------------
void vtkDMMLViewDisplayableManager
::OnDMMLDisplayableNodeModifiedEvent(vtkObject* vtkNotUsed(caller))
{
  this->UpdateFromViewNode();
}

//---------------------------------------------------------------------------
void vtkDMMLViewDisplayableManager::UpdateFromViewNode()
{
  this->Internal->UpdateRenderMode();
  this->Internal->UpdateAxisLabelVisibility();
  this->Internal->UpdateAxisVisibility();
  this->Internal->UpdateAxisLabelText();
  this->Internal->UpdateStereoType();
  this->Internal->UpdateBackgroundColor();
}

//---------------------------------------------------------------------------
void vtkDMMLViewDisplayableManager
::SetAndObserveCameraNode(vtkDMMLCameraNode* cameraNode)
{
  vtkSetAndObserveDMMLNodeMacro(this->Internal->CameraNode, cameraNode);
  this->Internal->UpdateAxis(this->GetRenderer(), this->GetDMMLViewNode());
  this->UpdateFromCameraNode();
}

//---------------------------------------------------------------------------
void vtkDMMLViewDisplayableManager::UpdateFromCameraNode()
{
  this->Internal->UpdateAxisLabelVisibility();
}

//---------------------------------------------------------------------------
void vtkDMMLViewDisplayableManager
::ProcessDMMLNodesEvents(vtkObject * caller,
                         unsigned long event,
                         void * callData)
{
  if (vtkDMMLViewNode::SafeDownCast(caller))
    {
    if (event == vtkDMMLViewNode::ResetFocalPointRequestedEvent)
      {
      vtkDebugMacro(<< "ProcessDMMLNodesEvents - ResetFocalPointEvent");
      this->Internal->UpdateAxis(this->GetRenderer(), this->GetDMMLViewNode());
      this->Internal->UpdateAxisLabelVisibility();
      }
    // Note: event == ModifiedEvent is handled by superclass
    }
  else if(vtkDMMLCameraNode::SafeDownCast(caller))
    {
    if (event == vtkCommand::ModifiedEvent)
      {
      this->UpdateFromCameraNode();
      }
    }
  this->Superclass::ProcessDMMLNodesEvents(caller, event, callData);
}

//---------------------------------------------------------------------------
void vtkDMMLViewDisplayableManager
::ProcessWidgetsEvents(vtkObject * caller,
                       unsigned long event,
                       void *callData)
{
  if (vtkDMMLCameraDisplayableManager::SafeDownCast(caller))
    {
    if (event == vtkDMMLCameraDisplayableManager::ActiveCameraChangedEvent)
      {
      vtkDMMLCameraDisplayableManager* cameraDisplayableManager =
        vtkDMMLCameraDisplayableManager::SafeDownCast(caller);
      this->SetAndObserveCameraNode(cameraDisplayableManager->GetCameraNode());
      }
    }
  this->Superclass::ProcessWidgetsEvents(caller, event, callData);
}
