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
#include "vtkDMMLColorLegendDisplayableManager.h"

// DMML includes
#include <vtkDMMLApplicationLogic.h>
#include <vtkDMMLColorLegendDisplayNode.h>
#include <vtkDMMLColorNode.h>
#include <vtkDMMLDisplayableNode.h>
#include <vtkDMMLScalarVolumeDisplayNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceCompositeNode.h>
#include <vtkDMMLSliceLogic.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLViewNode.h>
#include <vtkDMMLVolumeNode.h>

// VTK includes
#include <vtkLookupTable.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkTextProperty.h>

// Cjyx includes
#include <vtkCjyxScalarBarActor.h>

// STL includes
#include <algorithm>
#include <cstring>
#include <numeric>

namespace
{
const int RENDERER_LAYER = 1; // layer ID where the legent will be displayed
}

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLColorLegendDisplayableManager);

//---------------------------------------------------------------------------
class vtkDMMLColorLegendDisplayableManager::vtkInternal
{
public:

  vtkInternal(vtkDMMLColorLegendDisplayableManager * external);
  virtual ~vtkInternal();

  vtkObserverManager* GetDMMLNodesObserverManager();
  void Modified();

  vtkDMMLSliceCompositeNode* FindSliceCompositeNode();
  bool IsVolumeVisibleInSliceView(vtkDMMLVolumeNode* volumeNode);

  // Update color legend
  void UpdateColorLegend();

  // Update actor and widget representation.
  // Returns true if the actor has changed.
  bool UpdateActor(vtkDMMLColorLegendDisplayNode* dispNode);

  // Show/hide the actor by adding to the renderer and enabling visibility; or removing from the renderer.
  // Returns tru if visibility changed.
  bool ShowActor(vtkCjyxScalarBarActor* actor, bool show);

  void UpdateSliceNode();
  void SetSliceCompositeNode(vtkDMMLSliceCompositeNode* compositeNode);
  void UpdateActorsVisibilityFromSliceCompositeNode();

  /// Get lookup table as a copy of a LUT from table node without empty colors
  /// \param colorTableNode - input color table node
  /// \param validColorMaskVector - bool vector with true value for valid color
  /// \return a newly created LUT without empty colors or nullptr in case of an error
  /// \warning The returned LUT pointer must be deleted after usage or
  /// a vtkSmartPointer<vtkLookupTable> must take control over the pointer.
  /// If original color node doesn't have empty colors, then
  /// \sa vtkDMMLColorNode::CreateLookupTableCopy() is used
  vtkLookupTable* CreateLookupTableCopyWithoutEmptyColors( vtkDMMLColorNode* colorNode,
    std::vector<bool>& validColorMaskVector);

  vtkDMMLColorLegendDisplayableManager* External;

  /// Map stores color legend display node ID as a key, ScalarBarActor as a value
  std::map< std::string, vtkSmartPointer<vtkCjyxScalarBarActor> > ColorLegendActorsMap;

  /// For volume nodes we need to observe the slice composite node so that we can show color legend
  /// only for nodes that are visible in the slice view.
  vtkWeakPointer<vtkDMMLSliceCompositeNode> SliceCompositeNode;

  vtkSmartPointer<vtkRenderer> ColorLegendRenderer;
};


//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkDMMLColorLegendDisplayableManager::vtkInternal::vtkInternal(vtkDMMLColorLegendDisplayableManager* external)
: External(external)
{
  this->ColorLegendRenderer = vtkSmartPointer<vtkRenderer>::New();
  // Prevent erasing Z-buffer (important for quick picking and markup label visibility assessment)
  this->ColorLegendRenderer->EraseOff();
}

//---------------------------------------------------------------------------
vtkDMMLColorLegendDisplayableManager::vtkInternal::~vtkInternal()
{
  this->ColorLegendActorsMap.clear();
}

//---------------------------------------------------------------------------
vtkObserverManager* vtkDMMLColorLegendDisplayableManager::vtkInternal::GetDMMLNodesObserverManager()
{
  return this->External->GetDMMLNodesObserverManager();
}

//---------------------------------------------------------------------------
void vtkDMMLColorLegendDisplayableManager::vtkInternal::Modified()
{
  return this->External->Modified();
}

//---------------------------------------------------------------------------
bool vtkDMMLColorLegendDisplayableManager::vtkInternal::ShowActor(vtkCjyxScalarBarActor* actor, bool show)
{
  if (!this->ColorLegendRenderer.GetPointer())
    {
    return false;
    }
  bool wasInRenderer = this->ColorLegendRenderer->HasViewProp(actor);
  bool wasVisible = wasInRenderer && actor->GetVisibility();
  if (show && !wasInRenderer)
    {
    this->ColorLegendRenderer->AddActor2D(actor);
    }
  else if (!show && wasInRenderer)
    {
    this->ColorLegendRenderer->RemoveActor(actor);
    }
  actor->SetVisibility(show);
  return (wasVisible != show);
}

//---------------------------------------------------------------------------
bool vtkDMMLColorLegendDisplayableManager::vtkInternal::IsVolumeVisibleInSliceView(
  vtkDMMLVolumeNode* volumeNode)
{
  if (!volumeNode)
    {
    return false;
    }
  if (!this->SliceCompositeNode.GetPointer())
    {
    return false;
    }
  const char* volumeNodeID = volumeNode->GetID();
  if (!volumeNodeID)
    {
    return false;
    }
  if (this->SliceCompositeNode->GetBackgroundVolumeID())
    {
    if (strcmp(this->SliceCompositeNode->GetBackgroundVolumeID(), volumeNodeID) == 0)
      {
      return true;
      }
    }
  if (this->SliceCompositeNode->GetForegroundVolumeID())
    {
    if (strcmp(this->SliceCompositeNode->GetForegroundVolumeID(), volumeNodeID) == 0)
      {
      return true;
      }
    }
  if (this->SliceCompositeNode->GetLabelVolumeID())
    {
    if (strcmp(this->SliceCompositeNode->GetLabelVolumeID(), volumeNodeID) == 0)
      {
      return true;
      }
    }
  return false;
}

//---------------------------------------------------------------------------
bool vtkDMMLColorLegendDisplayableManager::vtkInternal::UpdateActor(vtkDMMLColorLegendDisplayNode* colorLegendDisplayNode)
{
  vtkCjyxScalarBarActor* actor = this->External->GetColorLegendActor(colorLegendDisplayNode);
  if (!actor || !colorLegendDisplayNode)
    {
    return false;
    }

  if (!colorLegendDisplayNode->GetVisibility())
    {
    return this->ShowActor(actor, false);
    }
  vtkDMMLDisplayableNode* displayableNode = colorLegendDisplayNode->GetDisplayableNode();
  if (!displayableNode)
    {
    return this->ShowActor(actor, false);
    }

  // Get primary display node
  vtkDMMLDisplayNode* primaryDisplayNode = colorLegendDisplayNode->GetPrimaryDisplayNode();
  if (!primaryDisplayNode && displayableNode)
    {
    // Primary display node is not set, fall back to the first non-color-legend display node of the displayable node
    for (int i = 0; i < displayableNode->GetNumberOfDisplayNodes(); i++)
      {
      if (!vtkDMMLColorLegendDisplayNode::SafeDownCast(displayableNode->GetDisplayNode()))
        {
        // found a suitable (non-color-legend) display node
        primaryDisplayNode = displayableNode->GetDisplayNode();
        break;
        }
      }
    }
  if (!primaryDisplayNode)
    {
    vtkErrorWithObjectMacro(this->External, "UpdateActor failed: No primary display node found");
    return this->ShowActor(actor, false);
    }

  // Setup/update color legend actor visibility
  // Color legend is only visible if the primary display node is visible as well, to reduce clutter in the views.
  vtkDMMLNode* viewNode = this->External->GetDMMLDisplayableNode();
  if (!viewNode)
    {
    return this->ShowActor(actor, false);
    }
  bool visible = colorLegendDisplayNode->GetVisibility(viewNode->GetID());

  if (visible)
    {
    // Only show the color legend if the primary display node is visible in the view, too.
    vtkDMMLVolumeDisplayNode* volumeDisplayNode = vtkDMMLVolumeDisplayNode::SafeDownCast(primaryDisplayNode);
    if (volumeDisplayNode)
      {
      // Volumes are special case, their visibility can be determined from slice view logics
      if (this->SliceCompositeNode)
        {
        // 2D view
        visible &= this->IsVolumeVisibleInSliceView(vtkDMMLVolumeNode::SafeDownCast(volumeDisplayNode->GetDisplayableNode()));
        }
      else
        {
        // 3D view
        // For now don't show color legends for volumes in 3D views.
        // In the future, color legend could be shown for volumes that are shown in slice views
        // that are visible in the 3D view.
        visible = false;
        }
      }
    else
      {
      // For all other nodes (models, markups, ...) visibilitly is determined from the display node.
      visible &= primaryDisplayNode->GetVisibility(viewNode->GetID());
      }
    }

  if (!visible)
    {
    return this->ShowActor(actor, false);
    }

  std::string title = colorLegendDisplayNode->GetTitleText();
  actor->SetTitle(title.c_str());

  actor->SetTitleTextProperty(colorLegendDisplayNode->GetTitleTextProperty());
  actor->SetLabelTextProperty(colorLegendDisplayNode->GetLabelTextProperty());

  std::string format = colorLegendDisplayNode->GetLabelFormat();
  actor->SetLabelFormat(format.c_str());

  double size[2] = { 0.5, 0.5 };
  colorLegendDisplayNode->GetSize(size);

  double position[3] = { 0.0, 0.0, 0.0 };
  colorLegendDisplayNode->GetPosition(position);

  // Set text position to the inner side of the legend
  // (using SetTextPositionTo...ScalarBar)
  // because the text overlapping with the image is typically
  // occludes less of the view contents.

  switch (colorLegendDisplayNode->GetOrientation())
    {
    case vtkDMMLColorLegendDisplayNode::Vertical:
      actor->SetOrientationToVertical();
      actor->SetPosition(position[0] * (1 - size[0]), position[1] * (1 - size[1]));
      actor->SetWidth(size[0]);
      actor->SetHeight(size[1]);
      if (position[0] < 0.5)
        {
        actor->SetTextPositionToSucceedScalarBar();
        actor->SetTextPad(2); // make some space between the bar and labels
        actor->GetTitleTextProperty()->SetJustificationToLeft();
        }
      else
        {
        actor->SetTextPositionToPrecedeScalarBar();
        actor->SetTextPad(-2); // make some space between the bar and labels
        actor->GetTitleTextProperty()->SetJustificationToRight();
        }
      break;
    case vtkDMMLColorLegendDisplayNode::Horizontal:
      actor->SetOrientationToHorizontal();
      actor->SetPosition(position[0] * (1 - size[1]), position[1] * (1 - size[0]));
      actor->SetWidth(size[1]);
      actor->SetHeight(size[0]);
      actor->SetTextPad(0);
      actor->GetTitleTextProperty()->SetJustificationToCentered();
      if (position[1] < 0.5)
        {
        actor->SetTextPositionToSucceedScalarBar();
        }
      else
        {
        actor->SetTextPositionToPrecedeScalarBar();
        }
      break;
    default:
      vtkErrorWithObjectMacro(this->External, "UpdateActor failed to set orientation: unknown orientation type " << colorLegendDisplayNode->GetOrientation());
      break;
    }

  // Get color node from the primary display node.
  // This is what determines the appearance of the displayable node, therefore it must be used
  // and not the color node and range that is set in the colorLegendDisplayNode.
  vtkDMMLColorNode* colorNode = primaryDisplayNode->GetColorNode();
  if (!colorNode)
    {
    vtkErrorWithObjectMacro(this->External, "UpdateActor failed: No color node is set in primary display node");
    return this->ShowActor(actor, false);
    }

  // Update displayed scalars range from primary display node
  double range[2] = { -1.0, -1.0 };
  vtkDMMLScalarVolumeDisplayNode* scalarVolumeDisplayNode = vtkDMMLScalarVolumeDisplayNode::SafeDownCast(primaryDisplayNode);
  if (scalarVolumeDisplayNode)
    {
    // Scalar volume display node
    double window = scalarVolumeDisplayNode->GetWindow();
    double level = scalarVolumeDisplayNode->GetLevel();
    range[0] = level - window / 2.0;
    range[1] = level + window / 2.0;
    }
  else
    {
    // Model or other display node
    primaryDisplayNode->GetScalarRange(range);
    }

  if (primaryDisplayNode->GetScalarRangeFlag() == vtkDMMLDisplayNode::UseDirectMapping)
    {
    // direct RGB color mapping, no LUT is used
    return this->ShowActor(actor, false);
    }

  if (!colorNode->GetLookupTable())
    {
    vtkErrorWithObjectMacro(this->External, "UpdateActor failed: No color node is set in primary display node");
    return this->ShowActor(actor, false);
    }

  // The look up table range, linear/log scale, etc. may need
  // to be changed to render the correct scalar values, thus
  // one lookup table can not be shared by multiple mappers
  // if any of those mappers needs to map using its scalar
  // values range. It is therefore necessary to make a copy
  // of the colorNode vtkLookupTable in order not to impact
  // that lookup table original range.
  vtkSmartPointer<vtkLookupTable> lut;
  std::vector<bool> validColorMask;
  if (colorLegendDisplayNode->GetUseColorNamesForLabels())
    {
    lut = vtkSmartPointer<vtkLookupTable>::Take(this->CreateLookupTableCopyWithoutEmptyColors(colorNode, validColorMask));
    }
  else
    {
    lut = vtkSmartPointer<vtkLookupTable>::Take(colorNode->CreateLookupTableCopy());
    }
  if (lut.GetPointer() == nullptr)
    {
    return false;
    }
  lut->SetTableRange(range);

  // Color name == label with valid number of colors (size of validColorMask vector in non zero)
  if (colorLegendDisplayNode->GetUseColorNamesForLabels() && !validColorMask.empty())
    {
    // When there are only a few colors (e.g., 5-10) in the LUT then it is important to build the
    // color table more color indices, otherwise centered labels would not be show up at the correct
    // position. We oversample the LUT to have approximately 256 color indices (newNumberOfColors)
    // regardless of how many items were in the original color table.

    // Only show colors and labels for valid colors in colorNode
    int numberOfValidColors = std::accumulate(validColorMask.begin(), validColorMask.end(), 0);
    actor->SetLookupTable(lut);
    actor->SetNumberOfLabels(numberOfValidColors);
    actor->SetMaximumNumberOfColors(256); // Without it colors on the ColorBar sometimes wrong
    actor->GetLookupTable()->ResetAnnotations();
    // Set color names of labels only for valid colors
    int i = 0;
    for (auto it = validColorMask.begin(); it != validColorMask.end(); ++it)
      {
      size_t validIndex = it - validColorMask.begin();
      if (*it && i < numberOfValidColors)
        {
        actor->GetLookupTable()->SetAnnotation(i++, vtkStdString(colorNode->GetColorName(validIndex)));
        }
      }
    actor->SetUseAnnotationAsLabel(true);
    actor->SetCenterLabel(true);
    }
  else if (!colorLegendDisplayNode->GetUseColorNamesForLabels()) // Color name == value ( default behaviour )
    {
    actor->SetNumberOfLabels(colorLegendDisplayNode->GetNumberOfLabels());
    actor->SetMaximumNumberOfColors(colorLegendDisplayNode->GetMaxNumberOfColors());
    actor->SetUseAnnotationAsLabel(false);
    actor->SetCenterLabel(false);
    actor->SetLookupTable(lut);
    }
  else
    {
    return false;
    }

  this->ShowActor(actor, true);

  // modified
  return true;
}

//---------------------------------------------------------------------------
vtkDMMLSliceCompositeNode* vtkDMMLColorLegendDisplayableManager::vtkInternal::FindSliceCompositeNode()
{
  vtkDMMLNode* viewNode = this->External->GetDMMLDisplayableNode();
  vtkDMMLSliceNode* sliceNode = vtkDMMLSliceNode::SafeDownCast(viewNode);
  if (!sliceNode)
    {
    // this displayable manager is not of a slice node
    return nullptr;
    }
  vtkDMMLApplicationLogic* dmmlAppLogic = this->External->GetDMMLApplicationLogic();
  if (!dmmlAppLogic)
    {
    vtkGenericWarningMacro("vtkDMMLColorLegendDisplayableManager::vtkInternal::FindSliceCompositeNode failed: invalid dmmlApplogic");
    return nullptr;
    }
  vtkDMMLSliceLogic* sliceLogic = dmmlAppLogic->GetSliceLogic(sliceNode);
  if (!sliceLogic)
    {
    return nullptr;
    }
  vtkDMMLSliceCompositeNode* sliceCompositeNode = sliceLogic->GetSliceCompositeNode();
  return sliceCompositeNode;
}

//---------------------------------------------------------------------------
void vtkDMMLColorLegendDisplayableManager::vtkInternal::UpdateSliceNode()
{
  vtkDMMLSliceCompositeNode* sliceCompositeNode = this->FindSliceCompositeNode();
  this->SetSliceCompositeNode(sliceCompositeNode);
}

//---------------------------------------------------------------------------
void vtkDMMLColorLegendDisplayableManager::vtkInternal::SetSliceCompositeNode(vtkDMMLSliceCompositeNode* compositeNode)
{
  if (this->SliceCompositeNode == compositeNode)
    {
    return;
    }
  vtkSetAndObserveDMMLNodeMacro(this->SliceCompositeNode, compositeNode);
  this->External->SetUpdateFromDMMLRequested(true);
  this->External->RequestRender();
}

//------------------------------------------------------------------------------
vtkLookupTable* vtkDMMLColorLegendDisplayableManager::vtkInternal::CreateLookupTableCopyWithoutEmptyColors( vtkDMMLColorNode* colorNode,
  std::vector<bool>& validColorIndex)
{
  if (!colorNode)
    {
    return nullptr;
    }

  validColorIndex.resize(colorNode->GetNumberOfColors());

  for (int i = 0; i < colorNode->GetNumberOfColors(); ++i)
    {
    const char* name = colorNode->GetColorName(i);
    if (name && std::strcmp( name, colorNode->GetNoName()) != 0)
      {
      validColorIndex[i] = true;
      }
    }
  int nofValidColors = std::accumulate( validColorIndex.begin(), validColorIndex.end(), 0);
  if (nofValidColors == 0)
    {
    validColorIndex.clear();
    return nullptr;
    }
  if (nofValidColors == colorNode->GetNumberOfColors())
    {
    return colorNode->CreateLookupTableCopy();
    }

  vtkLookupTable* newLUT = vtkLookupTable::New();
  newLUT->Allocate(nofValidColors);
  newLUT->SetNumberOfTableValues(nofValidColors);

  for (int i = 0, j = 0; i < colorNode->GetNumberOfColors(); ++i)
    {
    double rgba[4] = {0.5, 0.5, 0.5, 1.0};
    if (colorNode->GetColor( i, rgba) && validColorIndex[i])
      {
      newLUT->SetTableValue( j++, rgba);
      }
    }
  newLUT->Build();
  newLUT->BuildSpecialColors();

  return newLUT;
}

//---------------------------------------------------------------------------
// vtkDMMLColorLegendDisplayableManager methods

//---------------------------------------------------------------------------
vtkDMMLColorLegendDisplayableManager::vtkDMMLColorLegendDisplayableManager()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkDMMLColorLegendDisplayableManager::~vtkDMMLColorLegendDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkDMMLColorLegendDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
vtkCjyxScalarBarActor* vtkDMMLColorLegendDisplayableManager::GetColorLegendActor(vtkDMMLColorLegendDisplayNode* dispNode) const
{
  if (!dispNode)
    {
    vtkErrorMacro("GetColorLegendActor: display node is invalid");
    return nullptr;
    }
  const auto it = this->Internal->ColorLegendActorsMap.find(dispNode->GetID());
  if (it == this->Internal->ColorLegendActorsMap.end())
    {
    return nullptr;
    }
  return it->second;
}

//---------------------------------------------------------------------------
void vtkDMMLColorLegendDisplayableManager::Create()
{
  // Create a renderer in RENDERER_LAYER that will display the color legend
  // above the default layer (above images and markups).
  vtkRenderer* renderer = this->GetRenderer();
  if (!renderer)
    {
    vtkErrorMacro("vtkDMMLColorLegendDisplayableManager::Create() failed: renderer is invalid");
    return;
    }
  this->Internal->ColorLegendRenderer->InteractiveOff();
  vtkRenderWindow* renderWindow = renderer->GetRenderWindow();
  if (!renderer)
    {
    vtkErrorMacro("vtkDMMLColorLegendDisplayableManager::Create() failed: render window is invalid");
    return;
    }
  if (renderWindow->GetNumberOfLayers() < RENDERER_LAYER + 1)
    {
    renderWindow->SetNumberOfLayers(RENDERER_LAYER + 1);
    }
  this->Internal->ColorLegendRenderer->SetLayer(RENDERER_LAYER);
  renderWindow->AddRenderer(this->Internal->ColorLegendRenderer);

  // TODO: needed?
  // this->Internal->UpdateSliceNode();
}

//---------------------------------------------------------------------------
void vtkDMMLColorLegendDisplayableManager::AdditionalInitializeStep()
{
}

//---------------------------------------------------------------------------
void vtkDMMLColorLegendDisplayableManager::SetDMMLSceneInternal(vtkDMMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkDMMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkDMMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkDMMLScene::EndCloseEvent);
  events->InsertNextValue(vtkDMMLScene::EndBatchProcessEvent);
  this->SetAndObserveDMMLSceneEventsInternal(newScene, events.GetPointer());
}

//---------------------------------------------------------------------------
void vtkDMMLColorLegendDisplayableManager::OnDMMLDisplayableNodeModifiedEvent(vtkObject* vtkNotUsed(caller))
{
  // slice node has been updated, nothing to do
}

//---------------------------------------------------------------------------
void vtkDMMLColorLegendDisplayableManager::OnDMMLSceneNodeAdded(vtkDMMLNode* node)
{
  this->Superclass::OnDMMLSceneNodeAdded(node);

  if (!node || !this->GetDMMLScene())
    {
    vtkErrorMacro("OnDMMLSceneNodeAdded: Invalid DMML scene or input node");
    return;
    }

  if (node->IsA("vtkDMMLColorLegendDisplayNode"))
    {
    vtkNew<vtkIntArray> events;
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    vtkObserveDMMLNodeEventsMacro(node, events);

    vtkNew<vtkCjyxScalarBarActor> scalarBarActor;
    scalarBarActor->UnconstrainedFontSizeOn();

    // By default, color swatch is too wide (especially when showing long color names),
    // therefore, set it to a bit narrower
    scalarBarActor->SetBarRatio(0.2);

    std::string id(node->GetID());
    this->Internal->ColorLegendActorsMap[id] = scalarBarActor;

    this->ProcessDMMLNodesEvents(node, vtkCommand::ModifiedEvent, nullptr);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLColorLegendDisplayableManager::OnDMMLSceneNodeRemoved(vtkDMMLNode* node)
{
  this->Superclass::OnDMMLSceneNodeRemoved(node);

  if (!node || !this->GetDMMLScene())
    {
    vtkErrorMacro("OnDMMLSceneNodeRemoved: Invalid DMML scene or input node");
    return;
    }

  if (node->IsA("vtkDMMLColorLegendDisplayNode"))
    {
    vtkUnObserveDMMLNodeMacro(node);

    auto it = this->Internal->ColorLegendActorsMap.find(node->GetID());
    if (it != this->Internal->ColorLegendActorsMap.end())
      {
      vtkSmartPointer<vtkCjyxScalarBarActor> actor = it->second;
      this->Internal->ColorLegendActorsMap.erase(it);
      this->Internal->ColorLegendRenderer->RemoveActor(actor);
      }
    }
}

//---------------------------------------------------------------------------
void vtkDMMLColorLegendDisplayableManager::UpdateFromDMML()
{
  // this gets called from RequestRender, so make sure to jump out quickly if possible
  if (this->GetDMMLScene() == nullptr)
    {
    return;
    }

  // This is called when the view node is set. Update all actors.
  for (auto& colorBarNodeIdToActorIt : this->Internal->ColorLegendActorsMap)
    {
    vtkDMMLColorLegendDisplayNode* displayNode = vtkDMMLColorLegendDisplayNode::SafeDownCast(
      this->GetDMMLScene()->GetNodeByID(colorBarNodeIdToActorIt.first));
    if (!displayNode)
      {
      // orphan pipeline, it should have been deleted by the node removed event notification
      vtkWarningMacro("vtkDMMLColorLegendDisplayableManager::UpdateFromDMML: invalid node ID " << colorBarNodeIdToActorIt.first);
      continue;
      }
    this->Internal->UpdateActor(displayNode);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLColorLegendDisplayableManager::ProcessDMMLNodesEvents(vtkObject *caller, unsigned long event, void *callData)
{
  this->Superclass::ProcessDMMLNodesEvents(caller, event, callData);

  if (event != vtkCommand::ModifiedEvent)
    {
    return;
    }
  vtkDMMLColorLegendDisplayNode* dispNode = vtkDMMLColorLegendDisplayNode::SafeDownCast(caller);
  vtkDMMLSliceCompositeNode* sliceCompositeNode = vtkDMMLSliceCompositeNode::SafeDownCast(caller);
  if (dispNode)
    {
    if (this->Internal->UpdateActor(dispNode))
      {
      this->RequestRender();
      }
    }
  else if (sliceCompositeNode)
    {
    this->SetUpdateFromDMMLRequested(true);
    this->RequestRender();
    }
}

//---------------------------------------------------------------------------
void vtkDMMLColorLegendDisplayableManager::UpdateFromDMMLScene()
{
  this->Internal->UpdateSliceNode();
}

//---------------------------------------------------------------------------
void vtkDMMLColorLegendDisplayableManager::UnobserveDMMLScene()
{
  this->Internal->SetSliceCompositeNode(nullptr);
}
