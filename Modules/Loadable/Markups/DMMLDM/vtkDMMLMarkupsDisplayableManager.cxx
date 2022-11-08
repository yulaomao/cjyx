#include "vtkDMMLMarkupsDisplayableManager.h"

#include "vtkDMMLAbstractSliceViewDisplayableManager.h"

// MarkupsModule/Logic includes
#include <vtkCjyxMarkupsLogic.h>

// DMMLDisplayableManager includes
#include <vtkDMMLDisplayableManagerGroup.h>
#include <vtkDMMLInteractionEventData.h>
#include <vtkDMMLModelDisplayableManager.h>

// DMML includes
#include <vtkEventBroker.h>
#include <vtkDMMLApplicationLogic.h>
#include <vtkDMMLFolderDisplayNode.h>
#include <vtkDMMLInteractionNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSelectionNode.h>
#include <vtkDMMLSliceLogic.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLTransformNode.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkAbstractWidget.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkEvent.h>
#include <vtkGeneralTransform.h>
#include <vtkMarkupsGlyphSource2D.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPropCollection.h>
#include <vtkProperty2D.h>
#include <vtkRendererCollection.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCjyxMarkupsWidgetRepresentation2D.h>
#include <vtkCjyxMarkupsWidget.h>
#include <vtkSphereSource.h>
#include <vtkTextProperty.h>
#include <vtkWidgetRepresentation.h>

// STD includes
#include <algorithm>
#include <map>
#include <vector>
#include <sstream>
#include <string>

typedef void (*fp)();

#define NUMERIC_ZERO 0.001

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkDMMLMarkupsDisplayableManager);

//---------------------------------------------------------------------------
// vtkDMMLMarkupsDisplayableManager methods

//---------------------------------------------------------------------------
vtkDMMLMarkupsDisplayableManager::vtkDMMLMarkupsDisplayableManager()
{
  this->Helper = vtkSmartPointer<vtkDMMLMarkupsDisplayableManagerHelper>::New();
  this->Helper->SetDisplayableManager(this);
  this->DisableInteractorStyleEventsProcessing = 0;

  this->LastClickWorldCoordinates[0]=0.0;
  this->LastClickWorldCoordinates[1]=0.0;
  this->LastClickWorldCoordinates[2]=0.0;
  this->LastClickWorldCoordinates[3]=1.0;

}

//---------------------------------------------------------------------------
vtkDMMLMarkupsDisplayableManager::~vtkDMMLMarkupsDisplayableManager()
{
  this->DisableInteractorStyleEventsProcessing = 0;
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "DisableInteractorStyleEventsProcessing = " << this->DisableInteractorStyleEventsProcessing << std::endl;
  if (this->SliceNode &&
      this->SliceNode->GetID())
    {
    os << indent << "Slice node id = " << this->SliceNode->GetID() << std::endl;
    }
  else
    {
    os << indent << "No slice node" << std::endl;
    }
}

//---------------------------------------------------------------------------
vtkDMMLSliceNode * vtkDMMLMarkupsDisplayableManager::GetDMMLSliceNode()
{
  return vtkDMMLSliceNode::SafeDownCast(this->GetDMMLDisplayableNode());
}

//---------------------------------------------------------------------------
bool vtkDMMLMarkupsDisplayableManager::Is2DDisplayableManager()
{
  return this->GetDMMLSliceNode() != nullptr;
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManager::RequestRender()
{
  if (!this->GetDMMLScene())
    {
    return;
    }
  if (!this->GetDMMLScene()->IsBatchProcessing())
    {
    this->Superclass::RequestRender();
    }
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManager::UpdateFromDMML()
{
  // this gets called from RequestRender, so make sure to jump out quickly if possible
  if (this->GetDMMLScene() == nullptr)
    {
    return;
    }

  // turn off update from dmml requested, as we're doing it now, and create
  // widget requests a render which checks this flag before calling update
  // from dmml again
  this->SetUpdateFromDMMLRequested(false);

  std::vector<vtkDMMLNode*> markupNodes;
  this->GetDMMLScene()->GetNodesByClass("vtkDMMLMarkupsNode", markupNodes);
  for (std::vector< vtkDMMLNode* >::iterator nodeIt = markupNodes.begin(); nodeIt != markupNodes.end(); ++nodeIt)
    {
    vtkDMMLMarkupsNode *markupsNode = vtkDMMLMarkupsNode::SafeDownCast(*nodeIt);
    if (!markupsNode)
      {
      continue;
      }
    if (this->GetHelper()->MarkupsNodes.find(markupsNode) != this->GetHelper()->MarkupsNodes.end())
      {
      // node added already
      continue;
      }
    this->GetHelper()->AddMarkupsNode(markupsNode);
    }

  std::vector<vtkDMMLNode*> markupsDisplayNodes;
  this->GetDMMLScene()->GetNodesByClass("vtkDMMLMarkupsDisplayNode", markupsDisplayNodes);
  for (std::vector< vtkDMMLNode* >::iterator nodeIt = markupsDisplayNodes.begin(); nodeIt != markupsDisplayNodes.end(); ++nodeIt)
    {
    vtkDMMLMarkupsDisplayNode *markupsDisplayNode = vtkDMMLMarkupsDisplayNode::SafeDownCast(*nodeIt);
    if (!markupsDisplayNode)
      {
      continue;
      }
    if (this->GetHelper()->MarkupsDisplayNodesToWidgets.find(markupsDisplayNode) != this->GetHelper()->MarkupsDisplayNodesToWidgets.end())
      {
      // node added already
      continue;
      }
    this->GetHelper()->AddDisplayNode(markupsDisplayNode);
    }

  // Remove observed markups nodes that have been deleted from the scene
  for (vtkDMMLMarkupsDisplayableManagerHelper::MarkupsNodesIt markupsIterator = this->Helper->MarkupsNodes.begin();
    markupsIterator != this->Helper->MarkupsNodes.end();
    /*upon deletion the increment is done already, so don't increment here*/)
    {
    vtkDMMLMarkupsNode *markupsNode = *markupsIterator;
    if (this->GetDMMLScene()->IsNodePresent(markupsNode))
      {
      ++markupsIterator;
      }
    else
      {
      // display node is not in the scene anymore, delete the widget
      vtkDMMLMarkupsDisplayableManagerHelper::MarkupsNodesIt markupsIteratorToRemove = markupsIterator;
      ++markupsIterator;
      this->Helper->RemoveMarkupsNode(*markupsIteratorToRemove);
      this->Helper->MarkupsNodes.erase(markupsIteratorToRemove);
      }
    }

  // Remove widgets corresponding deleted display nodes
  for (vtkDMMLMarkupsDisplayableManagerHelper::DisplayNodeToWidgetIt widgetIterator = this->Helper->MarkupsDisplayNodesToWidgets.begin();
    widgetIterator != this->Helper->MarkupsDisplayNodesToWidgets.end();
    /*upon deletion the increment is done already, so don't increment here*/)
    {
    vtkDMMLMarkupsDisplayNode *markupsDisplayNode = widgetIterator->first;
    if (this->GetDMMLScene()->IsNodePresent(markupsDisplayNode))
      {
      ++widgetIterator;
      }
    else
      {
      // display node is not in the scene anymore, delete the widget
      vtkDMMLMarkupsDisplayableManagerHelper::DisplayNodeToWidgetIt widgetIteratorToRemove = widgetIterator;
      ++widgetIterator;
      vtkCjyxMarkupsWidget* widgetToRemove = widgetIteratorToRemove->second;
      this->Helper->DeleteWidget(widgetToRemove);
      this->Helper->MarkupsDisplayNodesToWidgets.erase(widgetIteratorToRemove);
      }
    }

}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManager::SetDMMLSceneInternal(vtkDMMLScene* newScene)
{
  Superclass::SetDMMLSceneInternal(newScene);

  // after a new scene got associated, we want to make sure everything old is gone
  this->OnDMMLSceneEndClose();

  if (newScene)
    {
    this->AddObserversToInteractionNode();
    }
  else
    {
    // there's no scene to get the interaction node from, so this won't do anything
    this->RemoveObserversFromInteractionNode();
    }
  vtkDebugMacro("SetDMMLSceneInternal: add observer on interaction node now?");

  // clear out the map of glyph types
  //this->Helper->ClearNodeGlyphTypes();
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManager
::ProcessDMMLNodesEvents(vtkObject *caller, unsigned long event, void *callData)
{
  vtkDMMLMarkupsNode * markupsNode = vtkDMMLMarkupsNode::SafeDownCast(caller);
  vtkDMMLInteractionNode * interactionNode = vtkDMMLInteractionNode::SafeDownCast(caller);
  if (markupsNode)
    {
    bool renderRequested = false;

    for (int displayNodeIndex = 0; displayNodeIndex < markupsNode->GetNumberOfDisplayNodes(); displayNodeIndex++)
      {
      vtkDMMLMarkupsDisplayNode* displayNode = vtkDMMLMarkupsDisplayNode::SafeDownCast(markupsNode->GetNthDisplayNode(displayNodeIndex));
      vtkCjyxMarkupsWidget *widget = this->Helper->GetWidget(displayNode);
      if (!widget)
        {
        // if a new display node is added or display node view node IDs are changed then we may need to create a new widget
        this->Helper->AddDisplayNode(displayNode);
        widget = this->Helper->GetWidget(displayNode);
        }
      if (!widget)
        {
        continue;
        }
      widget->UpdateFromDMML(markupsNode, event, callData);
      if (widget->GetNeedToRender())
        {
        renderRequested = true;
        widget->NeedToRenderOff();
        }
      }

    if (renderRequested)
      {
      this->RequestRender();
      }
    }
  else if (interactionNode)
    {
    if (event == vtkDMMLInteractionNode::InteractionModeChangedEvent)
      {
      // loop through all widgets and update the widget status
      for (vtkDMMLMarkupsDisplayableManagerHelper::DisplayNodeToWidgetIt widgetIterator = this->Helper->MarkupsDisplayNodesToWidgets.begin();
        widgetIterator != this->Helper->MarkupsDisplayNodesToWidgets.end(); ++widgetIterator)
        {
        vtkCjyxMarkupsWidget* widget = widgetIterator->second;
        if (!widget)
          {
          continue;
          }
        vtkDMMLInteractionEventData* eventData = reinterpret_cast<vtkDMMLInteractionEventData*>(callData);
        widget->Leave(eventData);
        }
      }
    }
  else
    {
    this->Superclass::ProcessDMMLNodesEvents(caller, event, callData);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManager::OnDMMLSceneEndClose()
{
  vtkDebugMacro("OnDMMLSceneEndClose: remove observers?");
  // run through all nodes and remove node and widget
  this->Helper->RemoveAllWidgetsAndNodes();

  this->SetUpdateFromDMMLRequested(true);
  this->RequestRender();

}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManager::OnDMMLSceneEndImport()
{
  this->SetUpdateFromDMMLRequested(true);
  this->UpdateFromDMMLScene();
  //this->Helper->SetAllWidgetsToManipulate();
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManager::UpdateFromDMMLScene()
{
  if (this->GetDMMLDisplayableNode())
    {
    this->UpdateFromDMML();
    }
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManager::OnDMMLSceneNodeAdded(vtkDMMLNode* node)
{
  if (!node || !this->GetDMMLScene())
    {
    return;
    }

  // if the scene is still updating, jump out
  if (this->GetDMMLScene()->IsBatchProcessing())
    {
    this->SetUpdateFromDMMLRequested(true);
    return;
    }

  if (node->IsA("vtkDMMLInteractionNode"))
    {
    this->AddObserversToInteractionNode();
    return;
    }

  if (node->IsA("vtkDMMLMarkupsNode"))
  {
    this->Helper->AddMarkupsNode(vtkDMMLMarkupsNode::SafeDownCast(node));

    // and render again
    this->RequestRender();
  }
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManager::AddObserversToInteractionNode()
{
  if (!this->GetDMMLScene())
    {
    return;
    }
  // also observe the interaction node for changes
  vtkDMMLInteractionNode *interactionNode = this->GetInteractionNode();
  if (interactionNode)
    {
    vtkDebugMacro("AddObserversToInteractionNode: interactionNode found");
    vtkNew<vtkIntArray> interactionEvents;
    interactionEvents->InsertNextValue(vtkDMMLInteractionNode::InteractionModeChangedEvent);
    interactionEvents->InsertNextValue(vtkDMMLInteractionNode::InteractionModePersistenceChangedEvent);
    interactionEvents->InsertNextValue(vtkDMMLInteractionNode::EndPlacementEvent);
    vtkObserveDMMLNodeEventsMacro(interactionNode, interactionEvents.GetPointer());
    }
  else { vtkDebugMacro("AddObserversToInteractionNode: No interaction node!"); }
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManager::RemoveObserversFromInteractionNode()
{
  if (!this->GetDMMLScene())
    {
    return;
    }

  // find the interaction node
  vtkDMMLInteractionNode *interactionNode =  this->GetInteractionNode();
  if (interactionNode)
    {
    vtkUnObserveDMMLNodeMacro(interactionNode);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManager::OnDMMLSceneNodeRemoved(vtkDMMLNode* node)
{
  bool modified = false;

  vtkDMMLMarkupsNode *markupsNode = vtkDMMLMarkupsNode::SafeDownCast(node);
  if (markupsNode)
    {
    this->Helper->RemoveMarkupsNode(markupsNode);
    modified = true;
    }

  vtkDMMLMarkupsDisplayNode *markupsDisplayNode = vtkDMMLMarkupsDisplayNode::SafeDownCast(node);
  if (markupsDisplayNode)
    {
    this->Helper->RemoveDisplayNode(markupsDisplayNode);
    modified = true;
    }

  if (modified)
  {
    this->RequestRender();
  }
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManager::OnDMMLDisplayableNodeModifiedEvent(vtkObject* caller)
{
  vtkDebugMacro("OnDMMLDisplayableNodeModifiedEvent");

  if (!caller)
    {
    vtkErrorMacro("OnDMMLDisplayableNodeModifiedEvent: Could not get caller.");
    return;
    }

  vtkDMMLSliceNode * sliceNode = vtkDMMLSliceNode::SafeDownCast(caller);
  if (sliceNode)
    {
    // the associated renderWindow is a 2D SliceView
    // this is the entry point for all events fired by one of the three sliceviews
    // (e.g. change slice number, zoom etc.)

    // we remember that this instance of the displayableManager deals with 2D
    // this is important for widget creation etc. and save the actual SliceNode
    // because during Cjyx startup the SliceViews fire events, it will be always set correctly
    this->SliceNode = sliceNode;

    // now we call the handle for specific sliceNode actions
    this->OnDMMLSliceNodeModifiedEvent();

    // and exit
    return;
    }

  vtkDMMLViewNode * viewNode = vtkDMMLViewNode::SafeDownCast(caller);
  if (viewNode)
    {
    // the associated renderWindow is a 3D View
    vtkDebugMacro("OnDMMLDisplayableNodeModifiedEvent: This displayableManager handles a ThreeD view.");
    return;
    }
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManager::OnDMMLSliceNodeModifiedEvent()
{
  bool renderRequested = false;

  // run through all markup nodes in the helper
  vtkDMMLMarkupsDisplayableManagerHelper::DisplayNodeToWidgetIt it
    = this->Helper->MarkupsDisplayNodesToWidgets.begin();
  while(it != this->Helper->MarkupsDisplayNodesToWidgets.end())
    {
    // we loop through all widgets
    vtkCjyxMarkupsWidget* widget = (it->second);
    widget->UpdateFromDMML(this->SliceNode, vtkCommand::ModifiedEvent);
    if (widget->GetNeedToRender())
      {
      renderRequested = true;
      widget->NeedToRenderOff();
      }
    ++it;
    }

  if (renderRequested)
  {
    this->RequestRender();
  }
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManager::OnInteractorStyleEvent(int eventid)
{
  Superclass::OnInteractorStyleEvent(eventid);

}

//---------------------------------------------------------------------------
vtkCjyxMarkupsWidget* vtkDMMLMarkupsDisplayableManager::GetWidget(vtkDMMLMarkupsDisplayNode * node)
{
  return this->Helper->GetWidget(node);
}

//---------------------------------------------------------------------------
/// Check if it is the correct displayableManager
//---------------------------------------------------------------------------
bool vtkDMMLMarkupsDisplayableManager::IsCorrectDisplayableManager()
{
  vtkDMMLSelectionNode *selectionNode = this->GetDMMLApplicationLogic()->GetSelectionNode();
  if (selectionNode == nullptr)
    {
    vtkErrorMacro ("IsCorrectDisplayableManager: No selection node in the scene.");
    return false;
    }
  const char* placeNodeClassName = selectionNode->GetActivePlaceNodeClassName();
  if (!placeNodeClassName)
    {
    return false;
    }

  vtkSmartPointer<vtkDMMLNode> node =
    vtkSmartPointer<vtkDMMLNode>::Take(this->GetDMMLScene()->CreateNodeByClass(placeNodeClassName));
  vtkDMMLMarkupsNode* markupsNode = vtkDMMLMarkupsNode::SafeDownCast(node);
  if (!markupsNode)
    {
    return false;
    }

  // the purpose of the displayableManager is hardcoded
  return this->IsManageable(placeNodeClassName);

}
//---------------------------------------------------------------------------
bool vtkDMMLMarkupsDisplayableManager::IsManageable(vtkDMMLNode* node)
{
  if (node == nullptr)
    {
    vtkErrorMacro("Is Manageable: invalid node.");
    return false;
    }

  return this->IsManageable(node->GetClassName());
}

//---------------------------------------------------------------------------
bool vtkDMMLMarkupsDisplayableManager::IsManageable(const char* nodeClassName)
{
  if (nodeClassName == nullptr)
    {
   return false;
    }

  vtkCjyxMarkupsLogic* markupsLogic =
    vtkCjyxMarkupsLogic::SafeDownCast(this->GetDMMLApplicationLogic()->GetModuleLogic("Markups"));
  if (!markupsLogic)
    {
    vtkErrorMacro("Is Manageable: invalid Markups logic.");
    return false;
    }

  return markupsLogic->IsMarkupsNodeRegistered(nodeClassName);
}

//---------------------------------------------------------------------------
vtkCjyxMarkupsWidget* vtkDMMLMarkupsDisplayableManager::FindClosestWidget(vtkDMMLInteractionEventData *callData, double &closestDistance2)
{
  vtkCjyxMarkupsWidget* closestWidget = nullptr;
  closestDistance2 = VTK_DOUBLE_MAX;

  for (vtkDMMLMarkupsDisplayableManagerHelper::DisplayNodeToWidgetIt widgetIterator = this->Helper->MarkupsDisplayNodesToWidgets.begin();
    widgetIterator != this->Helper->MarkupsDisplayNodesToWidgets.end(); ++widgetIterator)
    {
    vtkCjyxMarkupsWidget* widget = widgetIterator->second;
    if (!widget)
      {
      continue;
      }
    double distance2FromWidget = VTK_DOUBLE_MAX;
    if (widget->CanProcessInteractionEvent(callData, distance2FromWidget))
      {
      if (!closestWidget || distance2FromWidget < closestDistance2)
        {
        closestDistance2 = distance2FromWidget;
        closestWidget = widget;
        }
      }
    }
  return closestWidget;
}

//---------------------------------------------------------------------------
bool vtkDMMLMarkupsDisplayableManager::CanProcessInteractionEvent(vtkDMMLInteractionEventData* eventData, double &closestDistance2)
{
  vtkDMMLInteractionNode* interactionNode = this->GetInteractionNode();
  // New point can be placed anywhere
  int eventid = eventData->GetType();
  // We allow mouse move with the shift modifier to be processed while in place mode so that we can continue to update the
  // preview position, even when using shift + mouse-move to adjust the crosshair position.
  if ((eventid == vtkCommand::MouseMoveEvent
       && (eventData->GetModifiers() == vtkEvent::NoModifier ||
          (eventData->GetModifiers() & vtkEvent::ShiftModifier &&
           interactionNode && interactionNode->GetCurrentInteractionMode() == vtkDMMLInteractionNode::Place)))
    || eventid == vtkCommand::Move3DEvent
    /*|| (eventid == vtkCommand::LeftButtonPressEvent && eventData->GetModifiers() == vtkEvent::NoModifier)
    || eventid == vtkCommand::LeftButtonReleaseEvent
    || eventid == vtkCommand::RightButtonReleaseEvent
    || eventid == vtkCommand::EnterEvent
    || eventid == vtkCommand::LeaveEvent*/)
    {
    vtkDMMLSelectionNode *selectionNode = this->GetSelectionNode();
    if (!interactionNode || !selectionNode)
      {
      return false;
      }
    if (interactionNode->GetCurrentInteractionMode() == vtkDMMLInteractionNode::Place
      && this->IsManageable(selectionNode->GetActivePlaceNodeClassName()))
      {

      // If there is a suitable markups node for placement but it is not available in current view
      // then we do not allow placement (placement would create a new markup node for this view,
      // which would probably not what users want - they would like to place using the current markups node)
      bool canPlaceInThisView = false;
      vtkDMMLMarkupsNode* markupsNode = this->GetActiveMarkupsNodeForPlacement();
      if (markupsNode)
        {
        int numberOfDisplayNodes = markupsNode->GetNumberOfDisplayNodes();
        vtkDMMLAbstractViewNode* viewNode = vtkDMMLAbstractViewNode::SafeDownCast(this->GetDMMLDisplayableNode());
        for (int displayNodeIndex = 0; displayNodeIndex < numberOfDisplayNodes; displayNodeIndex++)
          {
          vtkDMMLDisplayNode* displayNode = markupsNode->GetNthDisplayNode(displayNodeIndex);
          if (displayNode && displayNode->IsDisplayableInView(viewNode->GetID()))
            {
            canPlaceInThisView = true;
            break;
            }
          }
        }
      else
        {
        // a new markups node will be created
        canPlaceInThisView = true;
        }
      if (canPlaceInThisView)
        {
        closestDistance2 = 0.0;
        return true;
        }
      }
    }

  if (eventid == vtkCommand::LeaveEvent && this->LastActiveWidget != nullptr)
    {
    if (this->LastActiveWidget->GetMarkupsDisplayNode() && this->LastActiveWidget->GetMarkupsDisplayNode()->HasActiveComponent())
      {
      // this widget has active component, therefore leave event is relevant
      closestDistance2 = 0.0;
      return this->LastActiveWidget;
      }
    }

  // Other interactions
  bool canProcess = (this->FindClosestWidget(eventData, closestDistance2) != nullptr);

  if (!canProcess && this->LastActiveWidget != nullptr
    && (eventid == vtkCommand::MouseMoveEvent || eventid == vtkCommand::Move3DEvent) )
    {
    // interaction context (e.g. mouse) is moved away from the widget -> deactivate if it's the same context that activated it
    std::vector<std::string> contextsWithActiveComponents =
      this->LastActiveWidget->GetMarkupsDisplayNode()->GetActiveComponentInteractionContexts();
    if (std::find(contextsWithActiveComponents.begin(), contextsWithActiveComponents.end(), eventData->GetInteractionContextName())
        != contextsWithActiveComponents.end() )
      {
      this->LastActiveWidget->Leave(eventData);
      this->LastActiveWidget = nullptr;
      }
    }

  return canProcess;
}

//---------------------------------------------------------------------------
bool vtkDMMLMarkupsDisplayableManager::ProcessInteractionEvent(vtkDMMLInteractionEventData* eventData)
{
  if (this->GetDisableInteractorStyleEventsProcessing())
    {
    return false;
    }
  int eventid = eventData->GetType();

  if (eventid == vtkCommand::LeaveEvent)
    {
    if (this->LastActiveWidget != nullptr)
      {
      this->LastActiveWidget->Leave(eventData);
      this->LastActiveWidget = nullptr;
      }
    }

  // Find/create active widget. Using smart pointer instead of raw pointer to ensure activeWidget
  // object does not get fully deleted until we are done using it if the user deletes it as part
  // of an EndPlacementEvent
  vtkSmartPointer<vtkCjyxMarkupsWidget> activeWidget;
  if (this->GetInteractionNode()->GetCurrentInteractionMode() == vtkDMMLInteractionNode::Place)
    {
    activeWidget = this->GetWidgetForPlacement();
    if (activeWidget)
      {
      activeWidget->SetWidgetState(vtkCjyxMarkupsWidget::WidgetStateDefine);
      }
    }
  else
    {
    double closestDistance2 = VTK_DOUBLE_MAX;
    activeWidget = this->FindClosestWidget(eventData, closestDistance2);
    }

  // Deactivate previous widget
  if (this->LastActiveWidget != nullptr && this->LastActiveWidget != activeWidget.GetPointer())
    {
    this->LastActiveWidget->Leave(eventData);
    }
  this->LastActiveWidget = activeWidget;
  if (!activeWidget)
    {
    // deactivate widget if we move far from it
    if (eventid == vtkCommand::MouseMoveEvent && this->LastActiveWidget != nullptr)
      {
      this->LastActiveWidget->Leave(eventData);
      this->LastActiveWidget = nullptr;
      }
    return false;
    }

  // Pass on the interaction event to the active widget
  return activeWidget->ProcessInteractionEvent(eventData);
}

//---------------------------------------------------------------------------
vtkDMMLMarkupsNode* vtkDMMLMarkupsDisplayableManager::GetActiveMarkupsNodeForPlacement()
{
  vtkDMMLSelectionNode *selectionNode = this->GetSelectionNode();
  if (!selectionNode)
    {
    return nullptr;
    }
  const char *activeMarkupsID = selectionNode->GetActivePlaceNodeID();
  vtkDMMLMarkupsNode *markupsNode = vtkDMMLMarkupsNode::SafeDownCast(this->GetDMMLScene()->GetNodeByID(activeMarkupsID));
  if (!markupsNode)
    {
    return nullptr;
    }
  // Additional checks for placement
  const char* placeNodeClassName = selectionNode->GetActivePlaceNodeClassName();
  if (!placeNodeClassName)
    {
    return nullptr;
    }
  if (!this->IsManageable(placeNodeClassName))
    {
    return nullptr;
    }
  if (std::string(markupsNode->GetClassName()) != placeNodeClassName)
    {
    return nullptr;
    }
  return markupsNode;
}

//---------------------------------------------------------------------------
int vtkDMMLMarkupsDisplayableManager::GetCurrentInteractionMode()
{
  vtkDMMLInteractionNode *interactionNode = this->GetInteractionNode();
  if (!interactionNode)
    {
    return 0;
    }
  return interactionNode->GetCurrentInteractionMode();
}

//---------------------------------------------------------------------------
vtkCjyxMarkupsWidget* vtkDMMLMarkupsDisplayableManager::GetWidgetForPlacement()
{
  if (this->GetCurrentInteractionMode() != vtkDMMLInteractionNode::Place)
    {
    return nullptr;
    }
  vtkDMMLSelectionNode *selectionNode = this->GetSelectionNode();
  if (!selectionNode)
    {
    return nullptr;
    }
  std::string placeNodeClassName = (selectionNode->GetActivePlaceNodeClassName() ? selectionNode->GetActivePlaceNodeClassName() : nullptr);
  if (!this->IsManageable(placeNodeClassName.c_str()))
    {
    return nullptr;
    }

  // Check if the active markups node is already the right class, and if yes then use that
  vtkDMMLMarkupsNode *activeMarkupsNode = this->GetActiveMarkupsNodeForPlacement();

  // Do not create a new widget if the markup is not displayable in this view
  if (activeMarkupsNode)
    {
    bool canPlaceInThisView = false;
    int numberOfDisplayNodes = activeMarkupsNode->GetNumberOfDisplayNodes();
    vtkDMMLAbstractViewNode* viewNode = vtkDMMLAbstractViewNode::SafeDownCast(this->GetDMMLDisplayableNode());
    for (int displayNodeIndex = 0; displayNodeIndex < numberOfDisplayNodes; displayNodeIndex++)
      {
        vtkDMMLDisplayNode* displayNode = activeMarkupsNode->GetNthDisplayNode(displayNodeIndex);
        if (displayNode && displayNode->IsDisplayableInView(viewNode->GetID()))
        {
        canPlaceInThisView = true;
        break;
        }
      }
    if (!canPlaceInThisView)
      {
      return nullptr;
      }
    }

  if (activeMarkupsNode && activeMarkupsNode->GetMaximumNumberOfControlPoints() >= 0
    && activeMarkupsNode->GetNumberOfDefinedControlPoints() >= activeMarkupsNode->GetMaximumNumberOfControlPoints())
    {
    // maybe reached maximum number of points - if yes, then create a new widget
    if (activeMarkupsNode->GetNumberOfDefinedControlPoints() == activeMarkupsNode->GetMaximumNumberOfControlPoints())
      {
      // one more point than the maximum
      vtkCjyxMarkupsWidget *cjyxWidget = this->Helper->GetWidget(activeMarkupsNode);
      if (cjyxWidget && !cjyxWidget->IsPointPreviewed())
        {
        // no preview is shown, so the widget is actually complete
        activeMarkupsNode = nullptr;
        }
      }
    else
      {
      // clearly over the maximum number of points
      activeMarkupsNode = nullptr;
      }
    }

  // If there is no active markups node then create a new one
  if (!activeMarkupsNode)
    {
    vtkCjyxMarkupsLogic* markupsLogic =
      vtkCjyxMarkupsLogic::SafeDownCast(this->GetDMMLApplicationLogic()->GetModuleLogic("Markups"));
    if (markupsLogic)
      {
      activeMarkupsNode = markupsLogic->AddNewMarkupsNode(placeNodeClassName);
      }
    if (activeMarkupsNode)
      {
      selectionNode->SetReferenceActivePlaceNodeID(activeMarkupsNode->GetID());
      }
    else
      {
      vtkErrorMacro("GetWidgetForPlacement failed to create new markups node by class " << placeNodeClassName);
      }
    }

  if (!activeMarkupsNode)
    {
    return nullptr;
    }
  vtkCjyxMarkupsWidget *cjyxWidget = this->Helper->GetWidget(activeMarkupsNode);
  return cjyxWidget;
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManager::SetHasFocus(bool hasFocus, vtkDMMLInteractionEventData* eventData)
{
  if (!hasFocus && this->LastActiveWidget!=nullptr)
    {
    this->LastActiveWidget->Leave(eventData);
    this->LastActiveWidget = nullptr;
    }
}

//---------------------------------------------------------------------------
bool vtkDMMLMarkupsDisplayableManager::GetGrabFocus()
{
  return (this->LastActiveWidget != nullptr && this->LastActiveWidget->GetGrabFocus());
}

//---------------------------------------------------------------------------
bool vtkDMMLMarkupsDisplayableManager::GetInteractive()
{
  return (this->LastActiveWidget != nullptr && this->LastActiveWidget->GetInteractive());
}

//---------------------------------------------------------------------------
int vtkDMMLMarkupsDisplayableManager::GetMouseCursor()
{
  if (!this->LastActiveWidget)
    {
    return VTK_CURSOR_DEFAULT;
    }
  return this->LastActiveWidget->GetMouseCursor();
}

//---------------------------------------------------------------------------
vtkCjyxMarkupsWidget * vtkDMMLMarkupsDisplayableManager::CreateWidget(vtkDMMLMarkupsDisplayNode* markupsDisplayNode)
{
  vtkDMMLMarkupsNode* markupsNode = markupsDisplayNode->GetMarkupsNode();
  if (!markupsNode)
    {
    return nullptr;
    }

  vtkCjyxMarkupsLogic* markupsLogic =
    vtkCjyxMarkupsLogic::SafeDownCast(this->GetDMMLApplicationLogic()->GetModuleLogic("Markups"));
  if (!markupsLogic)
    {
    vtkErrorMacro("CreateWidget: invalid Markups logic.");
    return nullptr;
    }

  // Create a widget of the associated type if the node matches the registered nodes
  vtkCjyxMarkupsWidget* widgetForMarkup = vtkCjyxMarkupsWidget::SafeDownCast(
    markupsLogic->GetWidgetByMarkupsType(markupsNode->GetMarkupType()));
  vtkCjyxMarkupsWidget* widget = widgetForMarkup ? widgetForMarkup->CreateInstance() : nullptr;
  if (!widget)
    {
    vtkErrorMacro("vtkDMMLMarkupsDisplayableManager::CreateWidget failed: cannot instantiate widget for markup " << markupsNode->GetMarkupType());
    return nullptr;
    }

  vtkDMMLAbstractViewNode* viewNode = vtkDMMLAbstractViewNode::SafeDownCast(this->GetDMMLDisplayableNode());
  vtkRenderer* renderer = this->GetRenderer();
  widget->SetDMMLApplicationLogic(this->GetDMMLApplicationLogic());
  widget->CreateDefaultRepresentation(markupsDisplayNode, viewNode, renderer);
  return widget;
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManager::ConvertDeviceToXYZ(double x, double y, double xyz[3])
{
  vtkDMMLAbstractSliceViewDisplayableManager::ConvertDeviceToXYZ(this->GetInteractor(), this->GetDMMLSliceNode(), x, y, xyz);
}