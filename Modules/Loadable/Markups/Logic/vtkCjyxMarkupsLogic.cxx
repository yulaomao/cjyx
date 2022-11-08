/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  ==============================================================================*/

// Markups includes
#include "vtkCjyxMarkupsLogic.h"
#include "vtkCjyxMarkupsWidget.h"

// Markups DMML includes
#include "vtkDMMLInteractionEventData.h"
#include "vtkDMMLMarkupsAngleNode.h"
#include "vtkDMMLMarkupsClosedCurveNode.h"
#include "vtkDMMLMarkupsCurveNode.h"
#include "vtkDMMLMarkupsDisplayNode.h"
#include "vtkDMMLMarkupsFiducialDisplayNode.h"
#include "vtkDMMLMarkupsFiducialNode.h"
#include "vtkDMMLMarkupsFiducialStorageNode.h"
#include "vtkDMMLMarkupsJsonStorageNode.h"
#include "vtkDMMLMarkupsLineNode.h"
#include "vtkDMMLMarkupsNode.h"
#include "vtkDMMLMarkupsPlaneDisplayNode.h"
#include "vtkDMMLMarkupsPlaneJsonStorageNode.h"
#include "vtkDMMLMarkupsPlaneNode.h"
#include "vtkDMMLMarkupsROIDisplayNode.h"
#include "vtkDMMLMarkupsROIJsonStorageNode.h"
#include "vtkDMMLMarkupsROINode.h"
#include "vtkDMMLMarkupsStorageNode.h"
#include "vtkDMMLTableStorageNode.h"

// Markups vtk widgets includes
#include "vtkCjyxAngleWidget.h"
#include "vtkCjyxCurveWidget.h"
#include "vtkCjyxLineWidget.h"
#include "vtkCjyxPlaneWidget.h"
#include "vtkCjyxPointsWidget.h"
#include "vtkCjyxROIWidget.h"

// Annotation DMML includes
#include "vtkDMMLAnnotationFiducialNode.h"
#include "vtkDMMLAnnotationPointDisplayNode.h"
#include "vtkDMMLAnnotationTextDisplayNode.h"

// DMML includes
#include "vtkDMMLCameraNode.h"
#include "vtkDMMLColorTableNode.h"
#include "vtkDMMLHierarchyNode.h"
#include "vtkDMMLInteractionNode.h"
#include "vtkDMMLMessageCollection.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSelectionNode.h"
#include "vtkDMMLSliceCompositeNode.h"
#include "vtkDMMLSliceNode.h"
#include "vtkDMMLSceneViewNode.h"
#include "vtkDMMLTableNode.h"

// vtkAddon includes
#include "vtkAddonMathUtilities.h"

// VTK includes
#include <vtkBitArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkTable.h>

// STD includes
#include <cassert>
#include <list>

//----------------------------------------------------------------------------
class vtkCjyxMarkupsLogic::vtkInternal
{
public:

  void UpdatePlacementValidInSelectionNode()
    {
    if (!this->SelectionNode)
      {
      return;
      }
    bool activePlaceNodePlacementValid = false;
    if (this->ActiveMarkupsNode)
      {
      activePlaceNodePlacementValid = !this->ActiveMarkupsNode->GetControlPointPlacementComplete();
      }
    this->SelectionNode->SetActivePlaceNodePlacementValid(activePlaceNodePlacementValid);
    }

  // This keeps the elements that can be registered to a node type
  struct MarkupEntry
    {
    vtkSmartPointer<vtkCjyxMarkupsWidget> MarkupsWidget;
    vtkSmartPointer<vtkDMMLMarkupsNode> MarkupsNode;
    bool CreatePushButton;
    };

  std::map<std::string, std::string> MarkupsTypeStorageNodes;
  vtkDMMLSelectionNode* SelectionNode{ nullptr };

  vtkWeakPointer<vtkDMMLMarkupsNode> ActiveMarkupsNode;

  /// Keeps track of the registered nodes and corresponding widgets
  std::map<std::string, MarkupEntry> MarkupTypeToMarkupEntry;

  /// Keeps track of the order in which the markups were registered
  std::list<std::string> RegisteredMarkupsOrder;

  /// Counter used by GenerateUniqueColor for creating new colors from a color table.
  int UniqueColorNextColorTableIndex{ 0 };
};

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkCjyxMarkupsLogic);

//----------------------------------------------------------------------------
// call back to be triggered when the default display node is changed, so that
// changes to it can be linked to a modified event on the logic
// \ingroup
class vtkCjyxMarkupsLogicCallback : public vtkCommand
{
public:
  static vtkCjyxMarkupsLogicCallback *New()
  { return new vtkCjyxMarkupsLogicCallback; }

  vtkCjyxMarkupsLogicCallback() = default;

  void Execute (vtkObject *vtkNotUsed(caller), unsigned long event, void*) override
  {
    if (event == vtkCommand::ModifiedEvent)
      {
      if (!this->markupsLogic)
        {
        return;
        }
      // trigger a modified event on the logic so that settings panel
      // observers can update
      this->markupsLogic->InvokeEvent(vtkCommand::ModifiedEvent);
      }
  }
  void SetLogic(vtkCjyxMarkupsLogic *logic)
  {
    this->markupsLogic = logic;
  }
  vtkCjyxMarkupsLogic * markupsLogic;
};

//----------------------------------------------------------------------------
vtkCjyxMarkupsLogic::vtkCjyxMarkupsLogic()
{
  this->Internal = new vtkInternal();
  this->AutoCreateDisplayNodes = true;
  this->RegisterJsonStorageNodeForMarkupsType("ROI", "vtkDMMLMarkupsROIJsonStorageNode");
  this->RegisterJsonStorageNodeForMarkupsType("Plane", "vtkDMMLMarkupsPlaneJsonStorageNode");
}

//----------------------------------------------------------------------------
vtkCjyxMarkupsLogic::~vtkCjyxMarkupsLogic()
{
  this->SetAndObserveSelectionNode(nullptr);
  this->Internal->MarkupsTypeStorageNodes.clear();
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::ProcessDMMLNodesEvents(vtkObject *caller,
                                                   unsigned long event,
                                                   void *callData)
{
  vtkDebugMacro("ProcessDMMLNodesEvents: Event " << event);

  vtkDMMLMarkupsDisplayNode *markupsDisplayNode = vtkDMMLMarkupsDisplayNode::SafeDownCast(caller);
  if (markupsDisplayNode)
    {
    if (event == vtkDMMLMarkupsDisplayNode::ResetToDefaultsEvent)
      {
      vtkDebugMacro("ProcessDMMLNodesEvents: calling SetDisplayNodeToDefaults");
      this->SetDisplayNodeToDefaults(markupsDisplayNode);
      }
    else if (event == vtkDMMLMarkupsDisplayNode::JumpToPointEvent)
      {
      int componentIndex = -1;
      int componentType = -1;
      int viewGroup = -1;
      vtkDMMLSliceNode* sliceNode = nullptr;
      if (callData != nullptr)
        {
        vtkDMMLInteractionEventData* eventData = reinterpret_cast<vtkDMMLInteractionEventData*>(callData);
        componentIndex = eventData->GetComponentIndex();
        componentType = eventData->GetComponentType();
        if (eventData->GetViewNode())
          {
          viewGroup = eventData->GetViewNode()->GetViewGroup();
          sliceNode = vtkDMMLSliceNode::SafeDownCast(eventData->GetViewNode());
          }
        }
      if (componentType == vtkDMMLMarkupsDisplayNode::ComponentControlPoint)
        {
        // Jump current slice node to the plane of the control point (do not center)
        if (sliceNode)
          {
          vtkDMMLMarkupsNode* markupsNode = vtkDMMLMarkupsNode::SafeDownCast(markupsDisplayNode->GetDisplayableNode());
          if (markupsNode)
            {
            double worldPos[3] = { 0.0 };
            markupsNode->GetNthControlPointPositionWorld(componentIndex, worldPos);
            sliceNode->JumpSliceByOffsetting(worldPos[0], worldPos[1], worldPos[2]);
            }
          }
          // Jump centered in all other slices in the view group
          this->JumpSlicesToNthPointInMarkup(markupsDisplayNode->GetDisplayableNode()->GetID(), componentIndex,
            true /* centered */, viewGroup, sliceNode);
        }
      else if (callData != nullptr && (componentType == vtkDMMLMarkupsDisplayNode::ComponentRotationHandle
        || componentType == vtkDMMLMarkupsDisplayNode::ComponentTranslationHandle
        || componentType == vtkDMMLMarkupsDisplayNode::ComponentScaleHandle))
        {
        // Jump to the location of the current handle position.
        vtkDMMLInteractionEventData* eventData = reinterpret_cast<vtkDMMLInteractionEventData*>(callData);
        double position_World[3] = { 0.0, 0.0, 0.0 };
        eventData->GetWorldPosition(position_World);
        this->JumpSlicesToLocation(position_World[0], position_World[1], position_World[2], true /* centered */, viewGroup, sliceNode);
        }
      }
    }

  // Update all measurements if units changed.
  // This makes display format, precision, etc. changes reflected immediately.
  if (event == vtkDMMLSelectionNode::UnitModifiedEvent && this->GetDMMLScene())
    {
    // units modified, update all measurements
    std::vector<vtkDMMLNode*> nodes;
    this->GetDMMLScene()->GetNodesByClass("vtkDMMLMarkupsNode", nodes);
    for (vtkDMMLNode* node : nodes)
      {
      vtkDMMLMarkupsNode* markupsNode = vtkDMMLMarkupsNode::SafeDownCast(node);
      if (!markupsNode)
        {
        continue;
        }
      markupsNode->UpdateAllMeasurements();
      }
    }

  // Update the observer to the active place node.
  if (caller == this->Internal->SelectionNode && event == vtkDMMLSelectionNode::ActivePlaceNodeIDChangedEvent && this->GetDMMLScene())
    {
    vtkDMMLMarkupsNode* activeMarkupsNode = nullptr;
    std::string activeMarkupsNodeID = this->GetActiveListID();
    if (!activeMarkupsNodeID.empty())
      {
      activeMarkupsNode = vtkDMMLMarkupsNode::SafeDownCast(this->GetDMMLScene()->GetNodeByID(activeMarkupsNodeID.c_str()));
      }
    if (this->Internal->ActiveMarkupsNode.GetPointer() != activeMarkupsNode)
      {
      // Active placement mode changed, add an observer to the markups node so that
      // we get notified about any control point number or state changes,
      // so that we can update the PlacementValid value in the selection node.
      vtkUnObserveDMMLNodeMacro(this->Internal->ActiveMarkupsNode);
      vtkNew<vtkIntArray> events;
      events->InsertNextValue(vtkCommand::ModifiedEvent);
      events->InsertNextValue(vtkDMMLMarkupsNode::PointPositionDefinedEvent);
      events->InsertNextValue(vtkDMMLMarkupsNode::PointPositionUndefinedEvent);
      events->InsertNextValue(vtkDMMLMarkupsNode::PointPositionMissingEvent);
      events->InsertNextValue(vtkDMMLMarkupsNode::PointPositionNonMissingEvent);
      vtkObserveDMMLNodeEventsMacro(activeMarkupsNode, events.GetPointer());
      this->Internal->ActiveMarkupsNode = activeMarkupsNode;

      this->Internal->UpdatePlacementValidInSelectionNode();
      }
    }

  if (caller == this->Internal->ActiveMarkupsNode.GetPointer() && this->GetDMMLScene())
    {
    // Markup control points are placed, update the selection node to indicate if placement of more control points is allowed.
    this->Internal->UpdatePlacementValidInSelectionNode();
    }
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::SetDMMLSceneInternal(vtkDMMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkDMMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkDMMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkDMMLScene::EndBatchProcessEvent);
  this->SetAndObserveDMMLSceneEventsInternal(newScene, events.GetPointer());

  vtkDMMLSelectionNode* selectionNode = nullptr;
  if (this->GetDMMLScene())
    {
    selectionNode = vtkDMMLSelectionNode::SafeDownCast(this->GetDMMLScene()->GetNodeByID(this->GetSelectionNodeID().c_str()));
    }
  this->SetAndObserveSelectionNode(selectionNode);
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::ObserveDMMLScene()
{
  if (this->GetDMMLScene())
    {
    this->UpdatePlaceNodeClassNamesInSelectionNode();
    }
  this->Superclass::ObserveDMMLScene();
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::UpdatePlaceNodeClassNamesInSelectionNode()
{
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("vtkCjyxMarkupsLogic::UpdatePlaceNodeClassNamesInSelectionNode failed: invalid scene");
    return;
    }
  vtkDMMLSelectionNode *selectionNode = vtkDMMLSelectionNode::SafeDownCast(this->GetDMMLScene()->GetNodeByID(this->GetSelectionNodeID().c_str()));
  if (!selectionNode)
    {
    vtkErrorMacro("vtkCjyxMarkupsLogic::UpdatePlaceNodeClassNamesInSelectionNode failed: invalid selection node");
    return;
    }

  for (const std::string& markupType : this->Internal->RegisteredMarkupsOrder)
    {
    this->Internal->MarkupTypeToMarkupEntry[markupType];
    auto markupEntryIt = this->Internal->MarkupTypeToMarkupEntry.find(markupType);
    if (markupEntryIt == this->Internal->MarkupTypeToMarkupEntry.end())
      {
      vtkWarningMacro("vtkCjyxMarkupsLogic::UpdatePlaceNodeClassNamesInSelectionNode failed to add " << markupType << " to selection node");
      continue;
      }
    const char* markupsClassName = markupEntryIt->second.MarkupsNode->GetClassName();
    if (selectionNode->PlaceNodeClassNameInList(markupsClassName) < 0)
      {
      vtkSmartPointer<vtkDMMLMarkupsNode> markupsNode = vtkSmartPointer<vtkDMMLMarkupsNode>::Take(
        vtkDMMLMarkupsNode::SafeDownCast(this->GetDMMLScene()->CreateNodeByClass(markupsClassName)));
      if (!markupsNode)
        {
        vtkErrorMacro("vtkCjyxMarkupsLogic::ObserveDMMLScene: Failed to create markups node by class " << markupsClassName);
        continue;
        }
      selectionNode->AddNewPlaceNodeClassNameToList(markupsNode->GetClassName(), markupsNode->GetAddIcon(), markupsNode->GetMarkupType());
      }
    }
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::SetAndObserveSelectionNode(vtkDMMLSelectionNode* selectionNode)
{
  vtkNew<vtkIntArray> selectionEvents;
  selectionEvents->InsertNextValue(vtkDMMLSelectionNode::UnitModifiedEvent);
  selectionEvents->InsertNextValue(vtkDMMLSelectionNode::ActivePlaceNodeIDChangedEvent);
  vtkSetAndObserveDMMLNodeEventsMacro(this->Internal->SelectionNode, selectionNode, selectionEvents.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::RegisterNodes()
{
  assert(this->GetDMMLScene() != nullptr);

  vtkDMMLScene *scene = this->GetDMMLScene();
  if (!scene)
    {
    vtkErrorMacro("vtkCjyxMarkupsLogic::RegisterNodes failed: invalid scene");
    return;
    }

  // Generic markups nodes
  scene->RegisterAbstractNodeClass("vtkDMMLMarkupsNode", "Markup");
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLMarkupsDisplayNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLMarkupsJsonStorageNode>::New());

  // NOTE: the order of registration determines the order of the create push buttons in the GUI

  vtkNew<vtkDMMLMarkupsFiducialNode> fiducialNode;
  vtkNew<vtkCjyxPointsWidget> pointsWidget;
  this->RegisterMarkupsNode(fiducialNode, pointsWidget);
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLMarkupsFiducialDisplayNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLMarkupsFiducialStorageNode>::New());

  vtkNew<vtkDMMLMarkupsLineNode> lineNode;
  vtkNew<vtkCjyxLineWidget> lineWidget;
  this->RegisterMarkupsNode(lineNode, lineWidget);

  vtkNew<vtkDMMLMarkupsAngleNode> angleNode;
  vtkNew<vtkCjyxAngleWidget> angleWidget;
  this->RegisterMarkupsNode(angleNode, angleWidget);

  vtkNew<vtkDMMLMarkupsCurveNode> curveNode;
  vtkNew<vtkCjyxCurveWidget> curveWidget;
  this->RegisterMarkupsNode(curveNode, curveWidget);

  vtkNew<vtkDMMLMarkupsClosedCurveNode> closedCurveNode;
  vtkNew<vtkCjyxCurveWidget> closedCurveWidget;
  this->RegisterMarkupsNode(closedCurveNode, closedCurveWidget);

  vtkNew<vtkDMMLMarkupsPlaneNode> planeNode;
  vtkNew<vtkCjyxPlaneWidget> planeWidget;
  this->RegisterMarkupsNode(planeNode, planeWidget);
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLMarkupsPlaneDisplayNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLMarkupsPlaneJsonStorageNode>::New());

  vtkNew<vtkDMMLMarkupsROINode> roiNode;
  vtkNew<vtkCjyxROIWidget> roiWidget;
  this->RegisterMarkupsNode(roiNode, roiWidget);
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLMarkupsROIDisplayNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLMarkupsROIJsonStorageNode>::New());
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::UpdateFromDMMLScene()
{
  assert(this->GetDMMLScene() != nullptr);
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::OnDMMLSceneNodeAdded(vtkDMMLNode* node)
{
  if (!node)
    {
    return;
    }
  if (node->IsA("vtkDMMLMarkupsDisplayNode"))
    {
    vtkDebugMacro("OnDMMLSceneNodeAdded: Have a markups display node");
    vtkNew<vtkIntArray> events;
    events->InsertNextValue(vtkDMMLMarkupsDisplayNode::ResetToDefaultsEvent);
    events->InsertNextValue(vtkDMMLMarkupsDisplayNode::JumpToPointEvent);
    vtkUnObserveDMMLNodeMacro(node);
    vtkObserveDMMLNodeEventsMacro(node, events.GetPointer());
    }
  // a node could have been added by a node selector's create new node method,
  // but make sure that the scene is not batch processing before responding to
  // the event
  if (!node->IsA("vtkDMMLMarkupsNode"))
    {
    return;
    }
  if (this->GetDMMLScene() &&
      (this->GetDMMLScene()->IsImporting() ||
       this->GetDMMLScene()->IsRestoring() ||
       this->GetDMMLScene()->IsBatchProcessing()))
    {
    return;
    }
  vtkDMMLMarkupsNode *markupsNode = vtkDMMLMarkupsNode::SafeDownCast(node);
  if (!markupsNode)
    {
    return;
    }

  if (markupsNode->GetDisplayNode() == nullptr && this->AutoCreateDisplayNodes)
    {
    // add a display node
    int modifyFlag = markupsNode->StartModify();
    std::string displayNodeID = this->AddNewDisplayNodeForMarkupsNode(markupsNode);
    markupsNode->EndModify(modifyFlag);
    vtkDebugMacro("Added a display node with id " << displayNodeID.c_str()
                  << " for markups node with id " << markupsNode->GetID());
    }
  // make it active for adding to via the mouse
  this->SetActiveList(markupsNode);
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::OnDMMLSceneNodeRemoved(vtkDMMLNode* node)
{
  // remove observer
  if (!node)
    {
    return;
    }
  if (node->IsA("vtkDMMLMarkupsDisplayNode"))
    {
    vtkDebugMacro("OnDMMLSceneNodeRemoved: Have a markups display node");
    vtkUnObserveDMMLNodeMacro(node);
    }
}

//---------------------------------------------------------------------------
std::string vtkCjyxMarkupsLogic::GetSelectionNodeID()
{
  std::string selectionNodeID = std::string("");

  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("GetSelectionNodeID: no scene defined!");
    return selectionNodeID;
    }

  // try the application logic first
  vtkDMMLApplicationLogic *dmmlAppLogic = this->GetDMMLApplicationLogic();
  if (dmmlAppLogic)
    {
    vtkDMMLSelectionNode *selectionNode = dmmlAppLogic->GetSelectionNode();
    if (selectionNode)
      {
      char *id = selectionNode->GetID();
      if (id)
        {
        selectionNodeID = std::string(id);
        }
      }
    }
  else
    {
    // try a default string
    selectionNodeID = std::string("vtkDMMLSelectionNodeSingleton");
    // check if it's in the scene
    if (this->GetDMMLScene()->GetNodeByID(selectionNodeID.c_str()) == nullptr)
      {
      vtkErrorMacro("GetSelectionNodeID: no selection node in scene with id " << selectionNodeID);
      // reset it
      selectionNodeID = std::string("");
      }
    }
  return selectionNodeID;
}

//---------------------------------------------------------------------------
std::string vtkCjyxMarkupsLogic::GetActiveListID()
{
  std::string listID = std::string("");

  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("GetActiveListID: no scene defined!");
    return listID;
    }

  // get the selection node
  vtkDMMLSelectionNode *selectionNode = vtkDMMLSelectionNode::SafeDownCast(this->GetDMMLScene()->GetNodeByID(this->GetSelectionNodeID().c_str()));
  if (!selectionNode)
    {
    vtkErrorMacro("GetActiveListID: unable to get the selection node that governs active lists.");
    return listID;
    }

  const char *activePlaceNodeID = selectionNode->GetActivePlaceNodeID();
  // is there no active fiducial list?
  if (activePlaceNodeID == nullptr)
    {
    vtkDebugMacro("GetListID: no active place node");
    return listID;
    }

  listID = std::string(activePlaceNodeID);
  return listID;
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::SetActiveListID(vtkDMMLMarkupsNode* markupsNode)
{
  this->SetActiveList(markupsNode);
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::SetActiveList(vtkDMMLMarkupsNode *markupsNode)
{
  vtkDMMLSelectionNode *selectionNode = vtkDMMLSelectionNode::SafeDownCast(this->GetDMMLScene()->GetNodeByID(this->GetSelectionNodeID().c_str()));
  if (!selectionNode)
    {
    vtkErrorMacro("vtkCjyxMarkupsLogic::SetActiveList: No selection node in the scene.");
    return;
    }

  if (markupsNode == nullptr)
    {
    // If fiducial node was placed then reset node ID and deactivate placement
    const char *activePlaceNodeClassName = selectionNode->GetActivePlaceNodeClassName();
    if (activePlaceNodeClassName && strcmp(activePlaceNodeClassName, "vtkDMMLMarkupsFiducialNode") == 0)
      {
      selectionNode->SetReferenceActivePlaceNodeID(nullptr);
      vtkSmartPointer<vtkCollection> interactionNodes = vtkSmartPointer<vtkCollection>::Take
        (this->GetDMMLScene()->GetNodesByClass("vtkDMMLInteractionNode"));
      for(int interactionNodeIndex = 0; interactionNodeIndex < interactionNodes->GetNumberOfItems(); ++interactionNodeIndex)
        {
        vtkDMMLInteractionNode *interactionNode = vtkDMMLInteractionNode::SafeDownCast(interactionNodes->GetItemAsObject(interactionNodeIndex));
        if (interactionNode->GetCurrentInteractionMode() == vtkDMMLInteractionNode::Place)
          {
          interactionNode->SetCurrentInteractionMode(vtkDMMLInteractionNode::ViewTransform);
          }
        }
      }
    return;
    }

  // check if need to update the current type of node that's being placed
  const char *activePlaceNodeClassName = selectionNode->GetActivePlaceNodeClassName();
  if (!activePlaceNodeClassName ||
      (activePlaceNodeClassName &&
       strcmp(activePlaceNodeClassName, markupsNode->GetClassName()) != 0))
    {
    // call the set reference to make sure the event is invoked
    selectionNode->SetReferenceActivePlaceNodeClassName(markupsNode->GetClassName());
    }
  // set this markup node active if it's not already
  const char *activePlaceNodeID = selectionNode->GetActivePlaceNodeID();
  if (!activePlaceNodeID ||
      (activePlaceNodeID && strcmp(activePlaceNodeID, markupsNode->GetID()) != 0))
    {
    selectionNode->SetReferenceActivePlaceNodeID(markupsNode->GetID());
    }
}

//---------------------------------------------------------------------------
std::string vtkCjyxMarkupsLogic::AddNewDisplayNodeForMarkupsNode(vtkDMMLNode *dmmlNode)
{
  std::string id;
  if (!dmmlNode || !dmmlNode->GetScene())
    {
    vtkErrorMacro("AddNewDisplayNodeForMarkupsNode: unable to add a markups display node!");
    return id;
    }

  // is there already a display node?
  vtkDMMLDisplayableNode *displayableNode = vtkDMMLDisplayableNode::SafeDownCast(dmmlNode);
  if (displayableNode && displayableNode->GetDisplayNode() != nullptr)
    {
    return displayableNode->GetDisplayNodeID();
    }

  // create the display node
  displayableNode->CreateDefaultDisplayNodes();
  vtkDMMLMarkupsDisplayNode* displayNode = vtkDMMLMarkupsDisplayNode::SafeDownCast(displayableNode->GetDisplayNode());
  if (!displayNode)
    {
    vtkErrorMacro("AddNewDisplayNodeForMarkupsNode: error creating new display node");
    return id;
    }

  // set it from the defaults
  this->SetDisplayNodeToDefaults(displayNode);
  vtkDebugMacro("AddNewDisplayNodeForMarkupsNode: set display node to defaults");

  // get the node id to return
  id = std::string(displayNode->GetID());

  return id;
}

//---------------------------------------------------------------------------
std::string vtkCjyxMarkupsLogic::AddNewFiducialNode(const char *name, vtkDMMLScene *scene)
{
  vtkDMMLMarkupsNode* markupsNode = this->AddNewMarkupsNode("vtkDMMLMarkupsFiducialNode", name ? name : "", scene);
  if (!markupsNode)
    {
    return "";
    }

  // If adding to this scene (could be adding to a scene view during conversion)
  // make it active so mouse mode tool bar clicks will add new fids to this list.
  if (scene == nullptr || scene == this->GetDMMLScene())
    {
    this->SetActiveList(markupsNode);
    }

  const char* nodeId = markupsNode->GetID();
  return (nodeId ? nodeId : "");
}

//---------------------------------------------------------------------------
vtkDMMLMarkupsNode* vtkCjyxMarkupsLogic::AddNewMarkupsNode(
  std::string className, std::string nodeName/*=std::string()*/, vtkDMMLScene* scene/*=nullptr*/)
{
  if (!scene)
    {
    scene = this->GetDMMLScene();
    }
  if (!scene)
    {
    vtkErrorMacro("AddNewMarkupsNode: no scene to add a markups node to");
    return nullptr;
    }

  vtkSmartPointer<vtkDMMLNode> node = vtkSmartPointer<vtkDMMLNode>::Take(
    scene->CreateNodeByClass(className.c_str()));
  vtkDMMLMarkupsNode* markupsNode = vtkDMMLMarkupsNode::SafeDownCast(node);
  if (!markupsNode)
    {
    vtkErrorMacro("AddNewMarkupsNode: failed to instantiate class " << className);
    return nullptr;
    }

  // Set node name
  if (nodeName.empty())
    {
    nodeName = scene->GenerateUniqueName(markupsNode->GetDefaultNodeNamePrefix());
    }
  markupsNode->SetName(nodeName.c_str());

  // Add the new node and display node to the scene
  scene->AddNode(markupsNode);
  markupsNode->CreateDefaultDisplayNodes();

  // Special case: for ROI nodes, we create alternating colors.
  // If it turns out to be a well-liked feature then we can enable this for all markup types
  if (className == "vtkDMMLMarkupsROINode")
    {
    markupsNode->GetDisplayNode()->SetSelectedColor(this->GenerateUniqueColor().GetData());
    }

  return markupsNode;
}

//---------------------------------------------------------------------------
int vtkCjyxMarkupsLogic::AddControlPoint(double r, double a, double s)
{
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("AddControlPoint: no scene defined!");
    return -1;
    }

  // get the active list id
  std::string listID = this->GetActiveListID();

  // is there no active point list?
  if (listID.size() == 0)
    {
    vtkDebugMacro("AddControlPoint: no point list is active, adding one first!");
    std::string newListID = this->AddNewFiducialNode();
    if (newListID.size() == 0)
      {
      vtkErrorMacro("AddControlPoint: failed to add a new point list to the scene.");
      return -1;
      }
    // try to get the id again
    listID = this->GetActiveListID();
    if (listID.size() == 0)
      {
      vtkErrorMacro("AddControlPoint: failed to create a new point list to add to!");
      return -1;
      }
    }

  // get the active list
  vtkDMMLNode *listNode = this->GetDMMLScene()->GetNodeByID(listID.c_str());
  if (!listNode)
    {
    vtkErrorMacro("AddControlPoint: failed to get the active point list with id " << listID);
    return -1;
    }
  vtkDMMLMarkupsFiducialNode *fiducialNode = vtkDMMLMarkupsFiducialNode::SafeDownCast(listNode);
  if (!fiducialNode)
    {
    vtkErrorMacro("AddControlPoint: active list is not a point list: " << listNode->GetClassName());
    return -1;
    }
  vtkDebugMacro("AddControlPoint: adding a control point to the list " << listID);
  // add a control point to the active point list
  return fiducialNode->AddControlPoint(vtkVector3d(r,a,s), std::string());
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::JumpSlicesToLocation(double x, double y, double z, bool centered,
                                                 int viewGroup /* =-1 */, vtkDMMLSliceNode* exclude /* =nullptr */)
{
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("JumpSlicesToLocation: No scene defined");
    return;
    }

  // save the whole state as iterating over all slice nodes
  int jumpMode = centered ? vtkDMMLSliceNode::CenteredJumpSlice: vtkDMMLSliceNode::OffsetJumpSlice;
  vtkDMMLSliceNode::JumpAllSlices(this->GetDMMLScene(), x, y, z, jumpMode, viewGroup, exclude);
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::JumpSlicesToNthPointInMarkup(const char *id, int n, bool centered,
                                                         int viewGroup /* =-1 */, vtkDMMLSliceNode* exclude /* =nullptr */)
{
  if (!id)
    {
    return;
    }
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("JumpSlicesToNthPointInMarkup: No scene defined");
    return;
    }
  // get the markups node
  vtkDMMLNode *dmmlNode = this->GetDMMLScene()->GetNodeByID(id);
  if (dmmlNode == nullptr)
    {
    return;
    }
  vtkDMMLMarkupsNode *markupNode = vtkDMMLMarkupsNode::SafeDownCast(dmmlNode);
  if (markupNode)
    {
    double point[4];
    markupNode->GetNthControlPointPositionWorld(n, point);
    this->JumpSlicesToLocation(point[0], point[1], point[2], centered, viewGroup, exclude);
    }
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::FocusCamerasOnNthPointInMarkup(const char *id, int n)
{

  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("FocusCamerasOnNthPointInMarkup: No scene defined");
    return;
    }

  std::vector<vtkDMMLNode *> cameraNodes;
  this->GetDMMLScene()->GetNodesByClass("vtkDMMLCameraNode", cameraNodes);
  vtkDMMLNode *node;
  for (unsigned int i = 0; i < cameraNodes.size(); ++i)
    {
    node = cameraNodes[i];
    if (node)
      {
      this->FocusCameraOnNthPointInMarkup(node->GetID(), id, n);
      }
    }
}
//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::FocusCameraOnNthPointInMarkup(
  const char *cameraNodeID, const char *markupNodeID, int n)
{
  if (!cameraNodeID || !markupNodeID)
    {
    return;
    }
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("FocusCameraOnNthPointInMarkup: No scene defined");
    return;
    }

  // get the camera node
  vtkDMMLNode *dmmlNode1 = this->GetDMMLScene()->GetNodeByID(cameraNodeID);
  if (dmmlNode1 == nullptr)
    {
    vtkErrorMacro("FocusCameraOnNthPointInMarkup: unable to find node with id " << cameraNodeID);
    return;
    }
  vtkDMMLCameraNode *cameraNode = vtkDMMLCameraNode::SafeDownCast(dmmlNode1);
  if (!cameraNode)
    {
    vtkErrorMacro("FocusCameraOnNthPointInMarkup: unable to find camera with id " << cameraNodeID);
    return;
    }

  // get the markups node
  vtkDMMLNode *dmmlNode2 = this->GetDMMLScene()->GetNodeByID(markupNodeID);
  if (dmmlNode2 == nullptr)
    {
    vtkErrorMacro("FocusCameraOnNthPointInMarkup: unable to find node with id " << markupNodeID);
    return;
    }
  vtkDMMLMarkupsNode *markup = vtkDMMLMarkupsNode::SafeDownCast(dmmlNode2);
  if (!markup)
    {
    vtkErrorMacro("FocusCameraOnNthPointInMarkup: unable to find markup with id " << markupNodeID);
    return;
    }

  double point[4];
  markup->GetNthControlPointPositionWorld(n, point);
  // and focus the camera there
  cameraNode->SetFocalPoint(point[0], point[1], point[2]);
}

//---------------------------------------------------------------------------
char* vtkCjyxMarkupsLogic::LoadMarkups(const char* fileName, const char* nodeName/*=nullptr*/, vtkDMMLMessageCollection* userMessages/*=nullptr*/)
{
  if (!fileName)
    {
    vtkErrorMacro("vtkCjyxMarkupsLogic::LoadMarkups failed: invalid fileName");
    return nullptr;
    }

  // get file extension
  std::string extension = vtkDMMLStorageNode::GetLowercaseExtensionFromFileName(fileName);
  if( extension.empty() )
    {
    vtkErrorMacro("vtkCjyxMarkupsLogic::LoadMarkups failed: no file extension specified: " << fileName);
    return nullptr;
    }

  //
  if (extension == std::string(".json"))
    {
    return this->LoadMarkupsFromJson(fileName, nodeName, userMessages);
    }
  else if (extension == std::string(".fcsv"))
    {
    return this->LoadMarkupsFromFcsv(fileName, nodeName, userMessages);
    }
  else
    {
    vtkErrorMacro("vtkCjyxMarkupsLogic::LoadMarkups failed: unrecognized file extension in " << fileName);
    return nullptr;
    }
}

//---------------------------------------------------------------------------
char* vtkCjyxMarkupsLogic::LoadMarkupsFiducials(const char* fileName, const char* fidsName/*=nullptr*/, vtkDMMLMessageCollection* userMessages/*=nullptr*/)
{
  return this->LoadMarkups(fileName, fidsName, userMessages);
}

//---------------------------------------------------------------------------
char* vtkCjyxMarkupsLogic::LoadMarkupsFromJson(const char* fileName, const char* nodeName/*=nullptr*/, vtkDMMLMessageCollection* userMessages/*=nullptr*/)
{
  if (!fileName)
    {
    vtkErrorMacro("LoadMarkups: null file or markups class name, cannot load");
    return nullptr;
    }

  vtkDebugMacro("LoadMarkups, file name = " << fileName << ", nodeName = " << (nodeName ? nodeName : "null"));

  std::vector<std::string> markupsTypes;
  // make a storage node and fiducial node and set the file name
  vtkDMMLMarkupsJsonStorageNode* tempStorageNode = vtkDMMLMarkupsJsonStorageNode::SafeDownCast(
    this->GetDMMLScene()->AddNewNodeByClass("vtkDMMLMarkupsJsonStorageNode"));
  if (!tempStorageNode)
    {
    vtkErrorMacro("LoadMarkups: failed to instantiate markups storage node by class vtkDMMLMarkupsJsonStorageNode");
    return nullptr;
    }
  tempStorageNode->GetMarkupsTypesInFile(fileName, markupsTypes);
  if (userMessages)
    {
    userMessages->AddMessages(tempStorageNode->GetUserMessages());
    }
  this->GetDMMLScene()->RemoveNode(tempStorageNode);

  vtkDMMLMarkupsNode* importedMarkupsNode = nullptr;
  for(unsigned int markupsIndex = 0; markupsIndex < markupsTypes.size(); ++markupsIndex)
    {
    std::string markupsType = markupsTypes[markupsIndex];
    vtkDMMLMarkupsJsonStorageNode* storageNode = this->AddNewJsonStorageNodeForMarkupsType(markupsType);
    if (!storageNode)
      {
      vtkErrorMacro("LoadMarkupsFromJson: Could not create storage node for markup type: " << markupsType);
      continue;
      }

    if (markupsTypes.size() == 1)
      {
      // If a single markups node is stored in this file then save the filename in the storage node.
      // (If multiple markups node are loaded from the same file then the filename should not be saved
      // because that would make multiple nodes overwrite the same file during saving.)
      storageNode->SetFileName(fileName);
      }

    vtkDMMLMarkupsNode* markupsNode = storageNode->AddNewMarkupsNodeFromFile(fileName, nodeName, markupsIndex);
    if (!importedMarkupsNode)
      {
      importedMarkupsNode = markupsNode;
      }
    if (!markupsNode)
      {
      if (userMessages)
        {
        userMessages->AddMessages(storageNode->GetUserMessages());
        }
      this->GetDMMLScene()->RemoveNode(storageNode);
      }
    }

  if (!importedMarkupsNode)
    {
    return nullptr;
    }

  return importedMarkupsNode->GetID();
}

//---------------------------------------------------------------------------
char * vtkCjyxMarkupsLogic::LoadMarkupsFromFcsv(const char* fileName, const char* nodeName/*=nullptr*/, vtkDMMLMessageCollection* userMessages/*=nullptr*/)
{
  if (!fileName)
    {
    vtkErrorMacro("LoadMarkups: null file or markups class name, cannot load");
    return nullptr;
    }

  vtkDebugMacro("LoadMarkups, file name = " << fileName << ", nodeName = " << (nodeName ? nodeName : "null"));
  // make a storage node and fiducial node and set the file name
  vtkDMMLStorageNode* storageNode = vtkDMMLStorageNode::SafeDownCast(this->GetDMMLScene()->AddNewNodeByClass("vtkDMMLMarkupsFiducialStorageNode"));
  if (!storageNode)
    {
    vtkErrorMacro("LoadMarkups: failed to instantiate markups storage node by class vtkDMMLMarkupsFiducialNode");
    return nullptr;
    }

  std::string newNodeName;
  if (nodeName && strlen(nodeName)>0)
    {
    newNodeName = nodeName;
    }
  else
    {
    newNodeName = this->GetDMMLScene()->GetUniqueNameByString(storageNode->GetFileNameWithoutExtension(fileName).c_str());
    }
  vtkDMMLMarkupsNode* markupsNode = vtkDMMLMarkupsNode::SafeDownCast(this->GetDMMLScene()->AddNewNodeByClass("vtkDMMLMarkupsFiducialNode", newNodeName));
  if (!markupsNode)
    {
    vtkErrorMacro("LoadMarkups: failed to instantiate markups node by class vtkDMMLMarkupsFiducialNode");
    if (userMessages)
      {
      userMessages->AddMessages(storageNode->GetUserMessages());
      }
    this->GetDMMLScene()->RemoveNode(storageNode);
    return nullptr;
    }

  storageNode->SetFileName(fileName);
  // add the nodes to the scene and set up the observation on the storage node
  markupsNode->SetAndObserveStorageNodeID(storageNode->GetID());

  // read the file
  char* nodeID = nullptr;
  if (storageNode->ReadData(markupsNode))
    {
    nodeID = markupsNode->GetID();
    }
  else
    {
    if (userMessages)
      {
      userMessages->AddMessages(storageNode->GetUserMessages());
      }
    this->GetDMMLScene()->RemoveNode(storageNode);
    this->GetDMMLScene()->RemoveNode(markupsNode);
    }

  return nodeID;
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::SetAllControlPointsVisibility(vtkDMMLMarkupsNode *node, bool flag)
{
  if (!node)
    {
    vtkDebugMacro("SetAllControlPointsVisibility: No list");
    return;
    }

  for (int i = 0; i < node->GetNumberOfControlPoints(); i++)
    {
    node->SetNthControlPointVisibility(i, flag);
    }
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::ToggleAllControlPointsVisibility(vtkDMMLMarkupsNode *node)
{
  if (!node)
    {
    vtkDebugMacro("ToggleAllControlPointsVisibility: No list");
    return;
    }

  for (int i = 0; i < node->GetNumberOfControlPoints(); i++)
    {
    node->SetNthControlPointVisibility(i, !(node->GetNthControlPointVisibility(i)));
    }
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::SetAllControlPointsLocked(vtkDMMLMarkupsNode *node, bool flag)
{
  if (!node)
    {
    vtkDebugMacro("SetAllControlPointsLocked: No list");
    return;
    }

  for (int i = 0; i < node->GetNumberOfControlPoints(); i++)
    {
    node->SetNthControlPointLocked(i, flag);
    }
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::ToggleAllControlPointsLocked(vtkDMMLMarkupsNode *node)
{
  if (!node)
    {
    vtkDebugMacro("ToggleAllControlPointsLocked: No list");
    return;
    }

  for (int i = 0; i < node->GetNumberOfControlPoints(); i++)
    {
    node->SetNthControlPointLocked(i, !(node->GetNthControlPointLocked(i)));
    }
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::SetAllControlPointsSelected(vtkDMMLMarkupsNode *node, bool flag)
{
  if (!node)
    {
    vtkDebugMacro("SetAllControlPointsSelected: No list");
    return;
    }

  for (int i = 0; i < node->GetNumberOfControlPoints(); i++)
    {
    node->SetNthControlPointSelected(i, flag);
    }
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::ToggleAllControlPointsSelected(vtkDMMLMarkupsNode *node)
{
  if (!node)
    {
    vtkDebugMacro("ToggleAllControlPointsSelected: No list");
    return;
    }

  for (int i = 0; i < node->GetNumberOfControlPoints(); i++)
    {
    node->SetNthControlPointSelected(i, !(node->GetNthControlPointSelected(i)));
    }
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::CopyBasicDisplayProperties(vtkDMMLMarkupsDisplayNode *sourceDisplayNode,
                                                       vtkDMMLMarkupsDisplayNode *targetDisplayNode)
{
  if (!sourceDisplayNode || !targetDisplayNode)
    {
    vtkErrorMacro("vtkCjyxMarkupsLogic::CopyBasicDisplayProperties failed: invalid input nodes");
    return;
    }
  DMMLNodeModifyBlocker blocker(targetDisplayNode);
  targetDisplayNode->SetSnapMode(sourceDisplayNode->GetSnapMode());
  targetDisplayNode->SetFillVisibility(sourceDisplayNode->GetFillVisibility());
  targetDisplayNode->SetOutlineVisibility(sourceDisplayNode->GetOutlineVisibility());
  targetDisplayNode->SetFillOpacity(sourceDisplayNode->GetFillOpacity());
  targetDisplayNode->SetOutlineOpacity(sourceDisplayNode->GetOutlineOpacity());
  targetDisplayNode->SetTextScale(sourceDisplayNode->GetTextScale());

  targetDisplayNode->SetGlyphType(sourceDisplayNode->GetGlyphType());
  targetDisplayNode->SetGlyphScale(sourceDisplayNode->GetGlyphScale());
  targetDisplayNode->SetGlyphSize(sourceDisplayNode->GetGlyphSize());
  targetDisplayNode->SetUseGlyphScale(sourceDisplayNode->GetUseGlyphScale());

  targetDisplayNode->SetSliceProjection(sourceDisplayNode->GetSliceProjection());
  targetDisplayNode->SetSliceProjectionUseFiducialColor(sourceDisplayNode->GetSliceProjectionUseFiducialColor());
  targetDisplayNode->SetSliceProjectionOutlinedBehindSlicePlane(sourceDisplayNode->GetSliceProjectionOutlinedBehindSlicePlane());
  targetDisplayNode->SetSliceProjectionColor(sourceDisplayNode->GetSliceProjectionColor());
  targetDisplayNode->SetSliceProjectionOpacity(sourceDisplayNode->GetSliceProjectionOpacity());

  targetDisplayNode->SetCurveLineSizeMode(sourceDisplayNode->GetCurveLineSizeMode());
  targetDisplayNode->SetLineThickness(sourceDisplayNode->GetLineThickness());
  targetDisplayNode->SetLineDiameter(sourceDisplayNode->GetLineDiameter());

  targetDisplayNode->SetLineColorFadingStart(sourceDisplayNode->GetLineColorFadingStart());
  targetDisplayNode->SetLineColorFadingEnd(sourceDisplayNode->GetLineColorFadingEnd());
  targetDisplayNode->SetLineColorFadingSaturation(sourceDisplayNode->GetLineColorFadingSaturation());
  targetDisplayNode->SetLineColorFadingHueOffset(sourceDisplayNode->GetLineColorFadingHueOffset());

  targetDisplayNode->SetOccludedVisibility(sourceDisplayNode->GetOccludedVisibility());
  targetDisplayNode->SetOccludedOpacity(sourceDisplayNode->GetOccludedOpacity());
  std::string textPropertyStr = vtkDMMLDisplayNode::GetTextPropertyAsString(sourceDisplayNode->GetTextProperty());
  vtkDMMLMarkupsDisplayNode::UpdateTextPropertyFromString(textPropertyStr, targetDisplayNode->GetTextProperty());

  targetDisplayNode->SetSelectedColor(sourceDisplayNode->GetSelectedColor());
  targetDisplayNode->SetColor(sourceDisplayNode->GetColor());
  targetDisplayNode->SetActiveColor(sourceDisplayNode->GetActiveColor());
  targetDisplayNode->SetOpacity(sourceDisplayNode->GetOpacity());

  targetDisplayNode->SetInteractionHandleScale(sourceDisplayNode->GetInteractionHandleScale());
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::SetDisplayNodeToDefaults(vtkDMMLMarkupsDisplayNode *displayNode)
{
  if (!displayNode)
    {
    return;
    }
  if (!this->GetDMMLScene())
    {
    return;
    }
  vtkDMMLMarkupsDisplayNode* defaultNode = this->GetDefaultMarkupsDisplayNode();
  if (!defaultNode)
    {
    return;
    }
  this->CopyBasicDisplayProperties(defaultNode, displayNode);
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::SetDisplayDefaultsFromNode(vtkDMMLMarkupsDisplayNode *displayNode)
{
  if (!displayNode)
    {
    return;
    }
  if (!this->GetDMMLScene())
    {
    return;
    }
  vtkDMMLMarkupsDisplayNode* defaultNode = this->GetDefaultMarkupsDisplayNode();
  if (!defaultNode)
    {
    return;
    }
  this->CopyBasicDisplayProperties(displayNode, defaultNode);
}

//---------------------------------------------------------------------------
bool vtkCjyxMarkupsLogic::MoveNthControlPointToNewListAtIndex(int n, vtkDMMLMarkupsNode *markupsNode,
                                                                vtkDMMLMarkupsNode *newMarkupsNode, int newIndex)
{
  if (!markupsNode || !newMarkupsNode)
    {
    vtkErrorMacro("MoveNthControlPointToNewListAtIndex: at least one of the markup list nodes are null!");
    return false;
    }

  if (!markupsNode->ControlPointExists(n))
    {
    vtkErrorMacro("MoveNthControlPointToNewListAtIndex: source index n " << n
                  << " is not in list of size " << markupsNode->GetNumberOfControlPoints());
    return false;
    }

  // get the control point
  vtkDMMLMarkupsNode::ControlPoint *newControlPoint = new vtkDMMLMarkupsNode::ControlPoint;
  *newControlPoint = *markupsNode->GetNthControlPoint(n);

  // add it to the destination list
  bool insertVal = newMarkupsNode->InsertControlPoint(newControlPoint, newIndex);
  if (!insertVal)
    {
    vtkErrorMacro("MoveNthControlPointToNewListAtIndex: failed to insert new control point at " << newIndex <<
                  ", control point is still on source list.");
    return false;
    }

  // remove it from the source list
  markupsNode->RemoveNthControlPoint(n);

  return true;
}

//---------------------------------------------------------------------------
bool vtkCjyxMarkupsLogic::CopyNthControlPointToNewList(int n, vtkDMMLMarkupsNode *markupsNode,
                                                         vtkDMMLMarkupsNode *newMarkupsNode)
{
  if (!markupsNode || !newMarkupsNode)
    {
    vtkErrorMacro("CopyNthControlPointToNewList: at least one of the markup list nodes are null!");
    return false;
    }

  if (!markupsNode->ControlPointExists(n))
    {
    vtkErrorMacro("CopyNthControlPointToNewList: source index n " << n
                  << " is not in list of size " << markupsNode->GetNumberOfControlPoints());
    return false;
    }

  // get the control point
  vtkDMMLMarkupsNode::ControlPoint *newControlPoint = new vtkDMMLMarkupsNode::ControlPoint;
  *newControlPoint = *markupsNode->GetNthControlPoint(n);

  // add it to the destination list
  newMarkupsNode->AddControlPoint(newControlPoint, false);

  return true;
}


//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::ConvertAnnotationFiducialsToMarkups()
{
  if (!this->GetDMMLScene())
    {
    return;
    }

  // there can be annotation fiducials in the main scene, as well as in scene
  // view scenes, so collect all of those in one vector to iterate over
  std::vector<vtkDMMLScene *> scenes;
  scenes.push_back(this->GetDMMLScene());

  vtkCollection *sceneViews = this->GetDMMLScene()->GetNodesByClass("vtkDMMLSceneViewNode");
  int numberOfSceneViews = sceneViews->GetNumberOfItems();
  for (int n = 0; n < numberOfSceneViews; ++n)
    {
    vtkDMMLSceneViewNode *sceneView =
      vtkDMMLSceneViewNode::SafeDownCast(sceneViews->GetItemAsObject(n));
    if (sceneView && sceneView->GetStoredScene())
      {
      scenes.push_back(sceneView->GetStoredScene());
      }
    }

  vtkDebugMacro("ConvertAnnotationFiducialsToMarkups: Have " << scenes.size()
                << " scenes to check for annotation fiducial hierarchies");

  // now iterate over this scene and the scene view scenes to get out the
  // annotation fiducials that need to be converted
  for (unsigned int s = 0; s < scenes.size(); ++s)
    {
    vtkDMMLScene *scene = scenes[s];

    vtkCollection *annotationFiducials = scene->GetNodesByClass("vtkDMMLAnnotationFiducialNode");
    int numberOfAnnotationFids = annotationFiducials->GetNumberOfItems();

    if (numberOfAnnotationFids == 0)
      {
      annotationFiducials->Delete();
      continue;
      }


    // go through all the annotation fiducials and collect their hierarchies
    vtkStringArray *hierarchyNodeIDs = vtkStringArray::New();

    for (int n = 0; n < numberOfAnnotationFids; ++n)
      {
      vtkDMMLNode *dmmlNode =
        vtkDMMLNode::SafeDownCast(annotationFiducials->GetItemAsObject(n));
      if (!dmmlNode)
        {
        continue;
        }
      vtkDMMLHierarchyNode *oneToOneHierarchyNode =
        vtkDMMLHierarchyNode::GetAssociatedHierarchyNode(dmmlNode->GetScene(),
                                                         dmmlNode->GetID());
      if (!oneToOneHierarchyNode)
        {
        continue;
        }
      char * parentNodeID = oneToOneHierarchyNode->GetParentNodeID();
      // is it not already in the list of annotation hierarchy node ids?
      vtkIdType id = hierarchyNodeIDs->LookupValue(parentNodeID);
      if (id == -1)
        {
        vtkDebugMacro("Found unique annotation hierarchy node, id = " << parentNodeID);
        hierarchyNodeIDs->InsertNextValue(parentNodeID);
        }
      }

    annotationFiducials->RemoveAllItems();
    annotationFiducials->Delete();

    if (hierarchyNodeIDs->GetNumberOfValues() == 0)
      {
      hierarchyNodeIDs->Delete();
      return;
      }
    else
      {
      vtkDebugMacro("Converting " << hierarchyNodeIDs->GetNumberOfValues()
                    << " annotation hierarchies to markup lists");
      }
    // now iterate over the hierarchies that have fiducials in them and convert
    // them to markups lists
    for (int i = 0; i < hierarchyNodeIDs->GetNumberOfValues(); ++i)
      {
      vtkDMMLNode *dmmlNode = nullptr;
      vtkDMMLHierarchyNode *hierarchyNode = nullptr;
      dmmlNode = scene->GetNodeByID(hierarchyNodeIDs->GetValue(i));
      if (!dmmlNode)
        {
        continue;
        }

      hierarchyNode = vtkDMMLHierarchyNode::SafeDownCast(dmmlNode);
      if (!hierarchyNode)
        {
        continue;
        }

      // create a markups fiducial list with this name
      std::string markupsListID = this->AddNewFiducialNode(hierarchyNode->GetName(), scene);
      vtkDMMLMarkupsFiducialNode *markupsNode = vtkDMMLMarkupsFiducialNode::SafeDownCast(scene->GetNodeByID(markupsListID.c_str()));
      if (!markupsNode)
        {
        continue;
        }
      // now get the fiducials in this annotation hierarchy
      vtkCollection *children = vtkCollection::New();
      hierarchyNode->GetAssociatedChildrenNodes(children, "vtkDMMLAnnotationFiducialNode");
      vtkDebugMacro("Found " << children->GetNumberOfItems() << " annot fids in this hierarchy");
      for (int c = 0; c < children->GetNumberOfItems(); ++c)
        {
        vtkDMMLAnnotationFiducialNode *annotNode = vtkDMMLAnnotationFiducialNode::SafeDownCast(children->GetItemAsObject(c));
        if (!annotNode)
          {
          continue;
          }
        double coord[3];
        annotNode->GetFiducialCoordinates(coord);
        int fidIndex = markupsNode->AddControlPoint(vtkVector3d(coord), std::string(annotNode->GetName()));
        vtkDebugMacro("Added a control point at index " << fidIndex);
        char *desc = annotNode->GetDescription();
        if (desc)
          {
          markupsNode->SetNthControlPointDescription(fidIndex,std::string(desc));
          }
        markupsNode->SetNthControlPointSelected(fidIndex, annotNode->GetSelected());
        markupsNode->SetNthControlPointVisibility(fidIndex,
                                                  annotNode->GetDisplayVisibility());
        markupsNode->SetNthControlPointLocked(fidIndex, annotNode->GetLocked());
        const char *assocNodeID = annotNode->GetAttribute("AssociatedNodeID");
        if (assocNodeID)
          {
          markupsNode->SetNthControlPointAssociatedNodeID(fidIndex, assocNodeID);
          }

        // get the display nodes
        vtkDMMLAnnotationPointDisplayNode *pointDisplayNode = annotNode->GetAnnotationPointDisplayNode();
        vtkDMMLAnnotationTextDisplayNode *textDisplayNode = annotNode->GetAnnotationTextDisplayNode();

        if (c == 0)
          {
          // use the first display node to get display settings
          vtkDMMLMarkupsDisplayNode *markupDisplayNode = markupsNode->GetMarkupsDisplayNode();
          if (!markupDisplayNode || !pointDisplayNode || !textDisplayNode)
            {
            continue;
            }
          markupDisplayNode->SetColor(pointDisplayNode->GetColor());
          markupDisplayNode->SetSelectedColor(pointDisplayNode->GetSelectedColor());
          markupDisplayNode->SetGlyphScale(pointDisplayNode->GetGlyphScale());
          markupDisplayNode->SetTextScale(textDisplayNode->GetTextScale());
          markupDisplayNode->SetOpacity(pointDisplayNode->GetOpacity());
          markupDisplayNode->SetPower(pointDisplayNode->GetPower());
          markupDisplayNode->SetAmbient(pointDisplayNode->GetAmbient());
          markupDisplayNode->SetDiffuse(pointDisplayNode->GetDiffuse());
          markupDisplayNode->SetSpecular(pointDisplayNode->GetSpecular());
          markupDisplayNode->SetSliceProjection(pointDisplayNode->GetSliceProjection());
          markupDisplayNode->SetSliceProjectionColor(pointDisplayNode->GetProjectedColor());
          markupDisplayNode->SetSliceProjectionOpacity(pointDisplayNode->GetProjectedOpacity());
          }
        //
        // clean up the no longer needed annotation nodes
        //
        // remove the 1:1 hierarchy node
        vtkDMMLHierarchyNode *oneToOneHierarchyNode =
          vtkDMMLHierarchyNode::GetAssociatedHierarchyNode(annotNode->GetScene(), annotNode->GetID());
        if (oneToOneHierarchyNode)
          {
          scene->RemoveNode(oneToOneHierarchyNode);
          }

        // remove the display nodes
        if (pointDisplayNode)
          {
          scene->RemoveNode(pointDisplayNode);
          }
        if (textDisplayNode)
          {
          scene->RemoveNode(textDisplayNode);
          }
        // is there a storage node?
        vtkDMMLStorageNode *storageNode = annotNode->GetStorageNode();
        if (storageNode)
          {
          scene->RemoveNode(storageNode);
          }
        // now remove the annotation node
        scene->RemoveNode(annotNode);
        }
      children->RemoveAllItems();
      children->Delete();
      }
    hierarchyNodeIDs->Delete();
    } // end of looping over the scene
  sceneViews->RemoveAllItems();
  sceneViews->Delete();
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::RenameAllControlPointsFromCurrentFormat(vtkDMMLMarkupsNode *markupsNode)
{
  if (!markupsNode)
    {
    return;
    }

  int numberOfControlPoints = markupsNode->GetNumberOfControlPoints();
  // get the format string with the list name replaced
  std::string formatString = markupsNode->ReplaceListNameInControlPointLabelFormat();
  bool numberInFormat = false;
  std::vector<char> buffVector(vtkDMMLMarkupsFiducialStorageNode::GetMaximumLineLength());
  char* buff = &(buffVector[0]);
  if (formatString.find("%d") != std::string::npos ||
      formatString.find("%g") != std::string::npos ||
      formatString.find("%f") != std::string::npos)
    {
    numberInFormat = true;
    }
  for (int n = 0; n < numberOfControlPoints; ++n)
    {
    std::string oldLabel = markupsNode->GetNthControlPointLabel(n);
    std::string oldNumber;
    if (numberInFormat)
      {
      // extract any number from the old label
      // is there more than one number in the old label?
      // - find the start of the first number
      std::string numbers = std::string("0123456789.");
      size_t firstNumber = oldLabel.find_first_of(numbers);
      size_t secondNumber = std::string::npos;
      size_t endOfFirstNumber = std::string::npos;
      size_t keepNumberStart = std::string::npos;
      size_t keepNumberEnd = std::string::npos;
      if (firstNumber != std::string::npos)
        {
        endOfFirstNumber = oldLabel.find_first_not_of(numbers, firstNumber);
        secondNumber = oldLabel.find_first_of(numbers, endOfFirstNumber);
        }
      if (secondNumber != std::string::npos)
        {
        vtkWarningMacro("RenameAllControlPointsFromCurrentFormat: more than one number in markup " << n << ", keeping second one: " << oldLabel.c_str());
        keepNumberStart = secondNumber;
        keepNumberEnd = oldLabel.find_first_not_of(numbers, keepNumberStart);
        }
      else
        {
        // use the first number
        keepNumberStart = firstNumber;
        keepNumberEnd = endOfFirstNumber;
        }
      if (keepNumberStart != std::string::npos)
        {
        oldNumber = oldLabel.substr(keepNumberStart, keepNumberEnd - keepNumberStart);
        if (formatString.find("%d") != std::string::npos)
          {
          // integer
          sprintf(buff,formatString.c_str(),atoi(oldNumber.c_str()));
          }
        else
          {
          // float
          sprintf(buff,formatString.c_str(),atof(oldNumber.c_str()));
          }
        }
      else
        {
        // no number found, use n
        sprintf(buff,formatString.c_str(),n);
        }
      markupsNode->SetNthControlPointLabel(n, std::string(buff));
      }
    else
      {
      // no number in the format, so just rename it
      markupsNode->SetNthControlPointLabel(n, formatString);
      }
    }
}

//---------------------------------------------------------------------------
bool vtkCjyxMarkupsLogic::StartPlaceMode(bool persistent, vtkDMMLInteractionNode* interactionNode)
{
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("StartPlaceMode: no scene");
    return false;
    }

  // set up to place markups fiducials
  vtkDMMLSelectionNode *selectionNode =
    vtkDMMLSelectionNode::SafeDownCast(
      this->GetDMMLScene()->GetNodeByID("vtkDMMLSelectionNodeSingleton"));
  if (!selectionNode)
    {
    vtkErrorMacro ("StartPlaceMode: No selection node in the scene." );
    return false;
    }
  selectionNode->SetReferenceActivePlaceNodeClassName("vtkDMMLMarkupsFiducialNode");

  // now go into place mode with the persistece flag set
  if (!interactionNode)
    {
    interactionNode = vtkDMMLInteractionNode::SafeDownCast(
      this->GetDMMLScene()->GetNodeByID("vtkDMMLInteractionNodeSingleton"));
    }
  if (!interactionNode)
    {
    vtkErrorMacro ("StartPlaceMode: No interaction node in the scene." );
    return false;
    }

  interactionNode->SetCurrentInteractionMode(vtkDMMLInteractionNode::Place);
  interactionNode->SetPlaceModePersistence(persistent ? 1 : 0);

  if (interactionNode->GetCurrentInteractionMode()
      != vtkDMMLInteractionNode::Place)
    {
    vtkErrorMacro("StartPlaceMode: Could not set place mode! "
                  << "Tried to set the interaction mode to "
                  << vtkDMMLInteractionNode::Place
                  << ", but it's now "
                  << interactionNode->GetCurrentInteractionMode());
    return false;
    }

  return true;
}

//---------------------------------------------------------------------------
vtkDMMLMarkupsDisplayNode* vtkCjyxMarkupsLogic::GetDefaultMarkupsDisplayNode()
{
  if (!this->GetDMMLScene())
    {
    return nullptr;
    }
  vtkDMMLMarkupsDisplayNode* defaultNode = vtkDMMLMarkupsDisplayNode::SafeDownCast(
    this->GetDMMLScene()->GetDefaultNodeByClass("vtkDMMLMarkupsDisplayNode"));
  if (defaultNode)
    {
    return defaultNode;
    }
  vtkSmartPointer<vtkDMMLMarkupsDisplayNode> newDefaultNode =
    vtkSmartPointer<vtkDMMLMarkupsDisplayNode>::Take(vtkDMMLMarkupsDisplayNode::SafeDownCast(
                                                       this->GetDMMLScene()->CreateNodeByClass("vtkDMMLMarkupsDisplayNode")));
  if (!newDefaultNode)
    {
    return nullptr;
    }
  this->GetDMMLScene()->AddDefaultNode(newDefaultNode);
  return newDefaultNode;
}

//---------------------------------------------------------------------------
double vtkCjyxMarkupsLogic::GetClosedCurveSurfaceArea(vtkDMMLMarkupsClosedCurveNode* curveNode,
                                                        vtkPolyData* inputSurface /*=nullptr*/, bool projectWarp /*=true*/)
{
  return vtkDMMLMarkupsClosedCurveNode::GetClosedCurveSurfaceArea(curveNode, inputSurface, projectWarp);
}

//---------------------------------------------------------------------------
bool vtkCjyxMarkupsLogic::FitSurfaceProjectWarp(vtkPoints* curvePoints,
  vtkPolyData* surface, double radiusScalingFactor/*=1.0*/, vtkIdType numberOfInternalGridPoints/*=225*/)
{
  return vtkDMMLMarkupsClosedCurveNode::FitSurfaceProjectWarp(curvePoints, surface, radiusScalingFactor, numberOfInternalGridPoints);
}

//---------------------------------------------------------------------------
bool vtkCjyxMarkupsLogic::IsPolygonClockwise(vtkPoints* points, vtkIdList* pointIds/*nullptr*/)
{
  return vtkDMMLMarkupsClosedCurveNode::IsPolygonClockwise(points, pointIds);
}

//---------------------------------------------------------------------------
bool vtkCjyxMarkupsLogic::FitSurfaceDiskWarp(vtkPoints* curvePoints, vtkPolyData* surface, double radiusScalingFactor/*=1.0*/)
{
  return vtkDMMLMarkupsClosedCurveNode::FitSurfaceDiskWarp(curvePoints, surface, radiusScalingFactor);
}

//---------------------------------------------------------------------------
bool vtkCjyxMarkupsLogic::GetBestFitPlane(vtkDMMLMarkupsNode* curveNode, vtkPlane* plane)
{
  if (!curveNode || !plane)
    {
    return false;
    }
  vtkPoints* curvePointsWorld = curveNode->GetCurvePointsWorld();
  if (curvePointsWorld == nullptr || curvePointsWorld->GetNumberOfPoints() < 3)
    {
    // not enough points for computing a plane
    return false;
    }
  return vtkAddonMathUtilities::FitPlaneToPoints(curvePointsWorld, plane);
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::RegisterJsonStorageNodeForMarkupsType(std::string markupsType, std::string storageNodeClassName)
{
  this->Internal->MarkupsTypeStorageNodes[markupsType] = storageNodeClassName;
}

//---------------------------------------------------------------------------
std::string vtkCjyxMarkupsLogic::GetJsonStorageNodeClassNameForMarkupsType(std::string markupsType)
{
  auto markupsStorageNodeIt = this->Internal->MarkupsTypeStorageNodes.find(markupsType);
  if (markupsStorageNodeIt == this->Internal->MarkupsTypeStorageNodes.end())
    {
    return "vtkDMMLMarkupsJsonStorageNode";
    }
  return markupsStorageNodeIt->second;
}

//---------------------------------------------------------------------------
vtkDMMLMarkupsJsonStorageNode* vtkCjyxMarkupsLogic::AddNewJsonStorageNodeForMarkupsType(std::string markupsType)
{
  return vtkDMMLMarkupsJsonStorageNode::SafeDownCast(this->GetDMMLScene()->AddNewNodeByClass(this->GetJsonStorageNodeClassNameForMarkupsType(markupsType)));
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::RegisterMarkupsNode(vtkDMMLMarkupsNode* markupsNode,
                                                vtkCjyxMarkupsWidget* markupsWidget,
                                                bool createPushButton)
{
  // Check for nullptr
  if (markupsNode == nullptr)
    {
    vtkErrorMacro("RegisterMarkupsNode failed: Invalid node.");
    return;
    }

  // Register node class, if has not been registered already.
  // This is just a convenience function so that markups node registration can be
  // accomplished with a single registration method.
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("RegisterMarkupsNode failed: Invalid scene.");
    return;
    }
  if (!this->GetDMMLScene()->IsNodeClassRegistered(markupsNode->GetClassName()))
    {
    this->GetDMMLScene()->RegisterNodeClass(markupsNode);
    }

  // Check for nullptr
  if (markupsWidget == nullptr)
    {
    vtkErrorMacro("RegisterMarkup: Invalid widget.");
    return;
    }

  // Check that the class is not already registered
  if (this->GetNodeByMarkupsType(markupsNode->GetMarkupType()))
    {
    vtkWarningMacro("RegisterMarkup: Markups node " << markupsNode->GetMarkupType() << " is already registered.");
    return;
    }

  vtkCjyxMarkupsLogic::vtkInternal::MarkupEntry markup;
  markup.MarkupsWidget = markupsWidget;
  markup.MarkupsNode = markupsNode;
  markup.CreatePushButton = createPushButton;

  // Register the markup internally
  this->Internal->MarkupTypeToMarkupEntry[markupsNode->GetMarkupType()] = markup;
  this->Internal->RegisteredMarkupsOrder.push_back(markupsNode->GetMarkupType());

  this->UpdatePlaceNodeClassNamesInSelectionNode();

  this->InvokeEvent(vtkCjyxMarkupsLogic::MarkupRegistered);
}

//---------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::UnregisterMarkupsNode(vtkDMMLMarkupsNode* markupsNode)
{
  // Check for nullptr
  if (markupsNode == nullptr)
    {
    vtkErrorMacro("RegisterMarkup: Invalid node.");
    return;
    }

  // Check that the class is not already registered
  if (!this->GetNodeByMarkupsType(markupsNode->GetMarkupType()))
    {
    vtkWarningMacro("UnregisterMarkup: Markups node " << markupsNode->GetMarkupType() << " is not registered.");
    return;
    }

  // Remove the markup
  this->Internal->MarkupTypeToMarkupEntry.erase(markupsNode->GetMarkupType());
  this->Internal->RegisteredMarkupsOrder.remove(markupsNode->GetMarkupType());

  this->InvokeEvent(vtkCjyxMarkupsLogic::MarkupUnregistered);
}

//----------------------------------------------------------------------------
bool vtkCjyxMarkupsLogic::IsMarkupsNodeRegistered(const char* nodeType) const
{
  if (!nodeType)
    {
    return false;
    }
  for (auto markupTypToMarkupEntryIt : this->Internal->MarkupTypeToMarkupEntry)
    {
    if (strcmp(markupTypToMarkupEntryIt.second.MarkupsNode->GetClassName(), nodeType) == 0)
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
vtkCjyxMarkupsWidget* vtkCjyxMarkupsLogic::GetWidgetByMarkupsType(const char* markupName) const
{
  if (!markupName)
    {
    vtkErrorMacro("GetWidgetByMarkupsType: Invalid node.");
    return nullptr;
    }

  const auto& markupIt = this->Internal->MarkupTypeToMarkupEntry.find(markupName);
  if (markupIt == this->Internal->MarkupTypeToMarkupEntry.end())
    {
    return nullptr;
    }

  return markupIt->second.MarkupsWidget;
}

//----------------------------------------------------------------------------
vtkDMMLMarkupsNode* vtkCjyxMarkupsLogic::GetNodeByMarkupsType(const char* markupName) const
{
  if (!markupName)
    {
    vtkErrorMacro("GetNodeByMarkupsType: Invalid node.");
    return nullptr;
    }

  const auto& markupIt = this->Internal->MarkupTypeToMarkupEntry.find(markupName);
  if (markupIt == this->Internal->MarkupTypeToMarkupEntry.end())
    {
    return nullptr;
    }

  return markupIt->second.MarkupsNode;
}

//----------------------------------------------------------------------------
bool vtkCjyxMarkupsLogic::GetCreateMarkupsPushButton(const char* markupName) const
{
  if (!markupName)
    {
    vtkErrorMacro("GetCreateMarkupsPushButton: Invalid node.");
    return false;
    }

  const auto& markupIt = this->Internal->MarkupTypeToMarkupEntry.find(markupName);
  if (markupIt == this->Internal->MarkupTypeToMarkupEntry.end())
    {
    return false;
    }

  return markupIt->second.CreatePushButton;
}

//----------------------------------------------------------------------------
const std::list<std::string>& vtkCjyxMarkupsLogic::GetRegisteredMarkupsTypes() const
{
  return this->Internal->RegisteredMarkupsOrder;
}

//----------------------------------------------------------------------------
bool vtkCjyxMarkupsLogic::ImportControlPointsFromTable(vtkDMMLMarkupsNode* markupsNode, vtkDMMLTableNode* tableNode,
  int startRow/*=0*/, int numberOfRows/*=-1*/)
{
  if (!markupsNode || !tableNode || !tableNode->GetTable() || startRow < 0)
    {
    vtkGenericWarningMacro("vtkCjyxMarkupsLogic::ImportControlPointsFromTable failed: Invalid markupsNode or tableNode or startRow.");
    return false;
    }
  if (numberOfRows < 0 || numberOfRows > tableNode->GetNumberOfRows() - startRow)
    {
    numberOfRows = tableNode->GetNumberOfRows() - startRow;
    }
  DMMLNodeModifyBlocker blocker(markupsNode);

  vtkTable* table = tableNode->GetTable();

  vtkAbstractArray* arrayX = nullptr;
  vtkAbstractArray* arrayY = nullptr;
  vtkAbstractArray* arrayZ = nullptr;
  bool rasCoordinates = true;
  if (table->GetColumnByName("r") && table->GetColumnByName("a") && table->GetColumnByName("s"))
    {
    arrayX = table->GetColumnByName("r");
    arrayY = table->GetColumnByName("a");
    arrayZ = table->GetColumnByName("s");
    }
  else if (table->GetColumnByName("l") && table->GetColumnByName("p") && table->GetColumnByName("s"))
    {
    rasCoordinates = false;
    arrayX = table->GetColumnByName("l");
    arrayY = table->GetColumnByName("p");
    arrayZ = table->GetColumnByName("s");
    }

  vtkAbstractArray* arrayLabel = table->GetColumnByName("label");
  vtkAbstractArray* arrayDescription = table->GetColumnByName("description");
  vtkAbstractArray* arraySelected = table->GetColumnByName("selected");
  vtkAbstractArray* arrayVisible = table->GetColumnByName("visible");
  vtkAbstractArray* arrayLocked = table->GetColumnByName("locked");
  vtkAbstractArray* arrayDefined= table->GetColumnByName("defined");

  for (int row = startRow; row < startRow + numberOfRows; row++)
    {
    // vtkVariant cannot convert values from VTK_BIT type, therefore we need to handle it
    // as a special case here.
    int positionStatus = vtkDMMLMarkupsNode::PositionDefined;
    if (vtkBitArray::SafeDownCast(arrayDefined))
      {
      if (vtkBitArray::SafeDownCast(arrayDefined)->GetValue(row) == 0)
        {
        positionStatus = vtkDMMLMarkupsNode::PositionUndefined;
        }
      }
    else if (arrayDefined)
      {
      if (arrayDefined->GetVariantValue(row).ToInt() == 0)
        {
        positionStatus = vtkDMMLMarkupsNode::PositionUndefined;
        }
      }

    vtkDMMLMarkupsNode::ControlPoint* controlPoint = new vtkDMMLMarkupsNode::ControlPoint;

    bool validX = false;
    bool validY = false;
    bool validZ = false;
    if (arrayX && arrayY && arrayZ)
      {
      controlPoint->Position[0] = arrayX->GetVariantValue(row).ToDouble(&validX);
      controlPoint->Position[1] = arrayY->GetVariantValue(row).ToDouble(&validY);
      controlPoint->Position[2] = arrayZ->GetVariantValue(row).ToDouble(&validZ);
      if (!rasCoordinates)
        {
        controlPoint->Position[0] = -controlPoint->Position[0];
        controlPoint->Position[1] = -controlPoint->Position[1];
        }
      }
    if (!validX || !validY || !validZ)
      {
      controlPoint->PositionStatus = vtkDMMLMarkupsNode::PositionUndefined;
      }

    if (arrayLabel)
      {
      controlPoint->Label = arrayLabel->GetVariantValue(row).ToString();
      }
    if (arrayDescription)
      {
      controlPoint->Description = arrayDescription->GetVariantValue(row).ToString();
      }

    if (vtkBitArray::SafeDownCast(arraySelected))
      {
      controlPoint->Selected = (vtkBitArray::SafeDownCast(arraySelected)->GetValue(row) != 0);
      }
    else if (arraySelected)
      {
      controlPoint->Selected = (arraySelected->GetVariantValue(row).ToInt() != 0);
      }

    if (vtkBitArray::SafeDownCast(arrayVisible))
      {
      controlPoint->Visibility = (vtkBitArray::SafeDownCast(arrayVisible)->GetValue(row) != 0);
      }
    else if (arrayVisible)
      {
      controlPoint->Visibility = (arrayVisible->GetVariantValue(row).ToInt() != 0);
      }

    if (vtkBitArray::SafeDownCast(arrayLocked))
      {
      controlPoint->Locked = (vtkBitArray::SafeDownCast(arrayLocked)->GetValue(row) != 0);
      }
    else if (arrayLocked)
      {
      controlPoint->Locked = (arrayLocked->GetVariantValue(row).ToInt() != 0);
      }

    controlPoint->PositionStatus = positionStatus;
    controlPoint->AutoCreated = false;
    markupsNode->AddControlPoint(controlPoint);
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkCjyxMarkupsLogic::ExportControlPointsToTable(vtkDMMLMarkupsNode* markupsNode, vtkDMMLTableNode* tableNode,
  int coordinateSystem/*=vtkDMMLStorageNode::CoordinateSystemRAS*/)
{
  if (!markupsNode || !tableNode || !tableNode->GetTable())
    {
    vtkGenericWarningMacro("vtkCjyxMarkupsLogic::ExportControlPointsToTable failed: Invalid markupsNode or tableNode.");
    return false;
    }

  bool rasCoordinates = (coordinateSystem != vtkDMMLStorageNode::CoordinateSystemLPS);

  vtkTable* table = tableNode->GetTable();

  vtkAbstractArray* arrayLabel = table->GetColumnByName("label");
  if (!arrayLabel)
    {
    arrayLabel = vtkStringArray::New();
    arrayLabel->SetName("label");
    table->AddColumn(arrayLabel);
    arrayLabel->UnRegister(nullptr);
    }

  // Get/create coordinate arrays
  vtkAbstractArray* arrayCoordinates[3] = { nullptr, nullptr, nullptr };
  std::string columnNames[3] =
    {
    rasCoordinates ? "r" : "l",
    rasCoordinates ? "a" : "p",
    "s"
    };
  for (int coordIndex = 0; coordIndex < 3; coordIndex++)
    {
    arrayCoordinates[coordIndex] = table->GetColumnByName(columnNames[coordIndex].c_str());
    if (arrayCoordinates[coordIndex])
      {
      continue;
      }
    arrayCoordinates[coordIndex] = vtkDoubleArray::New();
    arrayCoordinates[coordIndex]->SetName(columnNames[coordIndex].c_str());
    tableNode->AddColumn(arrayCoordinates[coordIndex]);
    arrayCoordinates[coordIndex]->UnRegister(nullptr);
    }

  vtkAbstractArray* arrayDefined = table->GetColumnByName("defined");
  if (!arrayDefined)
    {
    arrayDefined = vtkBitArray::New();
    arrayDefined->SetName("defined");
    table->AddColumn(arrayDefined);
    arrayDefined->UnRegister(nullptr);
    }

  vtkAbstractArray* arraySelected = table->GetColumnByName("selected");
  if (!arraySelected)
    {
    arraySelected = vtkBitArray::New();
    arraySelected->SetName("selected");
    table->AddColumn(arraySelected);
    arraySelected->UnRegister(nullptr);
    }

  vtkAbstractArray* arrayVisible = table->GetColumnByName("visible");
  if (!arrayVisible)
    {
    arrayVisible = vtkBitArray::New();
    arrayVisible->SetName("visible");
    table->AddColumn(arrayVisible);
    arrayVisible->UnRegister(nullptr);
    }

  vtkAbstractArray* arrayLocked = table->GetColumnByName("locked");
  if (!arrayLocked)
    {
    arrayLocked = vtkBitArray::New();
    arrayLocked->SetName("locked");
    table->AddColumn(arrayLocked);
    arrayLocked->UnRegister(nullptr);
    }

  vtkAbstractArray* arrayDescription = table->GetColumnByName("description");
  if (!arrayDescription)
    {
    arrayDescription = vtkStringArray::New();
    arrayDescription->SetName("description");
    table->AddColumn(arrayDescription);
    arrayDescription->UnRegister(nullptr);
    }

  for (int controlPointIndex = 0; controlPointIndex < markupsNode->GetNumberOfControlPoints(); controlPointIndex++)
    {
    int row = tableNode->AddEmptyRow();
    vtkDMMLMarkupsNode::ControlPoint* controlPoint = markupsNode->GetNthControlPoint(controlPointIndex);
    if (rasCoordinates)
      {
      arrayCoordinates[0]->SetVariantValue(row, vtkVariant(controlPoint->Position[0]));
      arrayCoordinates[1]->SetVariantValue(row, vtkVariant(controlPoint->Position[1]));
      }
    else
      {
      arrayCoordinates[0]->SetVariantValue(row, vtkVariant(-controlPoint->Position[0]));
      arrayCoordinates[1]->SetVariantValue(row, vtkVariant(-controlPoint->Position[1]));
      }
    arrayCoordinates[2]->SetVariantValue(row, vtkVariant(controlPoint->Position[2]));

    arrayLabel->SetVariantValue(row, controlPoint->Label.c_str());
    arrayDescription->SetVariantValue(row, controlPoint->Description.c_str());
    arraySelected->SetVariantValue(row, controlPoint->Selected);
    arrayVisible->SetVariantValue(row, controlPoint->Visibility);
    arrayLocked->SetVariantValue(row, controlPoint->Locked);
    arrayDefined->SetVariantValue(row, controlPoint->PositionStatus==vtkDMMLMarkupsNode::PositionDefined);
    }

  return true;
}

//------------------------------------------------------------------------------
vtkVector3d vtkCjyxMarkupsLogic::GenerateUniqueColor()
{
  double color[3] = { 0.5, 0.5, 0.5 };
  this->GenerateUniqueColor(color);
  return vtkVector3d(color[0], color[1], color[2]);
}

//------------------------------------------------------------------------------
void vtkCjyxMarkupsLogic::GenerateUniqueColor(double color[3])
{
  double rgba[4] = { 1.0, 1.0, 0.0, 1.0 }; // default is yellow
  vtkDMMLColorTableNode* colorTable = nullptr;
  vtkDMMLScene* scene = this->GetDMMLScene();
    {
    colorTable = vtkDMMLColorTableNode::SafeDownCast(
      scene->GetNodeByID("vtkDMMLColorTableNodeFileMediumChartColors.txt"));
    }
  if (colorTable)
    {
    colorTable->GetColor(this->Internal->UniqueColorNextColorTableIndex, rgba);
    this->Internal->UniqueColorNextColorTableIndex++;
    if (this->Internal->UniqueColorNextColorTableIndex >= colorTable->GetNumberOfColors())
      {
      // reached the end of the color table, start from the beginning
      // (the result is not completely unique colors, but at least enough variety that is
      // sufficient for most cases)
      this->Internal->UniqueColorNextColorTableIndex = 0;
      }
    }
  color[0] = rgba[0];
  color[1] = rgba[1];
  color[2] = rgba[2];
}

//------------------------------------------------------------------------------
bool vtkCjyxMarkupsLogic::ExportControlPointsToCSV(vtkDMMLMarkupsNode* markupsNode,
  const std::string filename, bool lps/*=true*/)
{
  if (!markupsNode)
    {
    vtkGenericWarningMacro("vtkCjyxMarkupsLogic::ExportControlPointsToCSV failed: invalid input markupsNode");
    return false;
    }
  vtkNew<vtkDMMLTableNode> tableNode;
  if (!vtkCjyxMarkupsLogic::ExportControlPointsToTable(markupsNode, tableNode,
    lps ? vtkDMMLStorageNode::CoordinateSystemLPS : vtkDMMLStorageNode::CoordinateSystemRAS))
    {
    return false;
    }
  vtkNew<vtkDMMLTableStorageNode> tableStorageNode;
  tableStorageNode->SetFileName(filename.c_str());
  if (!tableStorageNode->WriteData(tableNode))
    {
    return false;
    }
  return true;
}

//------------------------------------------------------------------------------
bool vtkCjyxMarkupsLogic::ImportControlPointsFromCSV(
  vtkDMMLMarkupsNode* markupsNode, const std::string filename)
{
  if (!markupsNode)
    {
    vtkGenericWarningMacro("vtkCjyxMarkupsLogic::ImportControlPointsFromCSV failed: invalid markupsNode");
    return false;
    }
  vtkNew<vtkDMMLTableNode> tableNode;
  vtkNew<vtkDMMLTableStorageNode> tableStorageNode;
  tableStorageNode->SetFileName(filename.c_str());
  if (!tableStorageNode->ReadData(tableNode))
    {
    return false;
    }
  if (!vtkCjyxMarkupsLogic::ImportControlPointsFromTable(markupsNode, tableNode))
    {
    return false;
    }
  return true;
}
