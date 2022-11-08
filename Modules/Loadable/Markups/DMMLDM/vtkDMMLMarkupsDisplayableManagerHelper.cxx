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

// MarkupsModule/DMML includes
#include <vtkDMMLMarkupsNode.h>
#include <vtkDMMLMarkupsDisplayNode.h>

// MarkupsModule/DMMLDisplayableManager includes
#include "vtkDMMLMarkupsDisplayableManagerHelper.h"
#include "vtkDMMLMarkupsDisplayableManager.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkDMMLInteractionNode.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkProperty.h>
#include <vtkPickingManager.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCjyxMarkupsWidgetRepresentation.h>
#include <vtkCjyxMarkupsWidget.h>
#include <vtkCjyxPointsWidget.h>
#include <vtkCjyxLineWidget.h>
#include <vtkCjyxAngleWidget.h>
#include <vtkSmartPointer.h>
#include <vtkSphereHandleRepresentation.h>

// DMML includes
#include <vtkEventBroker.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLAbstractDisplayableManager.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLViewNode.h>

// STD includes
#include <algorithm>
#include <map>
#include <vector>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkDMMLMarkupsDisplayableManagerHelper);

//---------------------------------------------------------------------------
vtkDMMLMarkupsDisplayableManagerHelper::vtkDMMLMarkupsDisplayableManagerHelper()
{
  this->DisplayableManager = nullptr;
  this->AddingMarkupsNode = false;
  this->ObservedMarkupNodeEvents.push_back(vtkCommand::ModifiedEvent);
  this->ObservedMarkupNodeEvents.push_back(vtkDMMLTransformableNode::TransformModifiedEvent);
  this->ObservedMarkupNodeEvents.push_back(vtkDMMLDisplayableNode::DisplayModifiedEvent);
  this->ObservedMarkupNodeEvents.push_back(vtkDMMLMarkupsNode::PointModifiedEvent);
  this->ObservedMarkupNodeEvents.push_back(vtkDMMLMarkupsNode::PointAddedEvent);
  this->ObservedMarkupNodeEvents.push_back(vtkDMMLMarkupsNode::PointRemovedEvent);
  this->ObservedMarkupNodeEvents.push_back(vtkDMMLMarkupsNode::LockModifiedEvent);
  this->ObservedMarkupNodeEvents.push_back(vtkDMMLMarkupsNode::CenterOfRotationModifiedEvent);
  this->ObservedMarkupNodeEvents.push_back(vtkDMMLMarkupsNode::FixedNumberOfControlPointsModifiedEvent);
}

//---------------------------------------------------------------------------
vtkDMMLMarkupsDisplayableManagerHelper::~vtkDMMLMarkupsDisplayableManagerHelper()
{
  this->RemoveAllWidgetsAndNodes();
  this->SetDisplayableManager(nullptr);
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManagerHelper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "MarkupsDisplayNodeList:" << std::endl;

  os << indent << "MarkupsDisplayNodesToWidgets map:" << std::endl;
  DisplayNodeToWidgetIt widgetIterator = this->MarkupsDisplayNodesToWidgets.begin();
  for (widgetIterator = this->MarkupsDisplayNodesToWidgets.begin();
    widgetIterator != this->MarkupsDisplayNodesToWidgets.end();
    ++widgetIterator)
    {
    os << indent.GetNextIndent() << widgetIterator->first->GetID() << " : widget is " << (widgetIterator->second ? "not null" : "null") << std::endl;
    if (widgetIterator->second &&
      widgetIterator->second->GetRepresentation())
      {
      vtkCjyxMarkupsWidgetRepresentation * rep =
        vtkCjyxMarkupsWidgetRepresentation::SafeDownCast(widgetIterator->second->GetRepresentation());
      int numberOfNodes = 0;
      if (rep)
        {
        numberOfNodes = rep->GetMarkupsNode()->GetNumberOfControlPoints();
        }
      else
        {
        vtkWarningMacro("PrintSelf: no representation for widget assoc with markups node " << widgetIterator->first->GetID());
        }
      os << indent.GetNextIndent().GetNextIndent() << "number of nodes = " << numberOfNodes << std::endl;
      }
    }
};


//---------------------------------------------------------------------------
vtkCjyxMarkupsWidget* vtkDMMLMarkupsDisplayableManagerHelper::GetWidget(vtkDMMLMarkupsNode * markupsNode)
{
  if (!markupsNode)
    {
    return nullptr;
    }
  vtkDMMLMarkupsDisplayableManagerHelper::MarkupsNodesIt displayableIt =
    this->MarkupsNodes.find(markupsNode);
  if (displayableIt == this->MarkupsNodes.end())
    {
    // we do not manage this markup
    return nullptr;
    }

  // Return first widget found for a markups node
  for (vtkDMMLMarkupsDisplayableManagerHelper::DisplayNodeToWidgetIt widgetIterator = this->MarkupsDisplayNodesToWidgets.begin();
    widgetIterator != this->MarkupsDisplayNodesToWidgets.end();
    ++widgetIterator)
    {
    vtkDMMLMarkupsDisplayNode *markupsDisplayNode = widgetIterator->first;
    if (markupsDisplayNode->GetDisplayableNode() == markupsNode)
      {
      return widgetIterator->second;
      }
    }

  return nullptr;
}

//---------------------------------------------------------------------------
vtkCjyxMarkupsWidget * vtkDMMLMarkupsDisplayableManagerHelper::GetWidget(vtkDMMLMarkupsDisplayNode * node)
{
  if (!node)
    {
    return nullptr;
    }

  // Make sure the map contains a vtkWidget associated with this node
  DisplayNodeToWidgetIt it = this->MarkupsDisplayNodesToWidgets.find(node);
  if (it == this->MarkupsDisplayNodesToWidgets.end())
    {
    return nullptr;
    }

  return it->second;
}


//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManagerHelper::RemoveAllWidgetsAndNodes()
{
  DisplayNodeToWidgetIt widgetIterator = this->MarkupsDisplayNodesToWidgets.begin();
  for (widgetIterator =  this->MarkupsDisplayNodesToWidgets.begin();
       widgetIterator != this->MarkupsDisplayNodesToWidgets.end();
       ++widgetIterator)
    {
    widgetIterator->second->Delete();
    }
  this->MarkupsDisplayNodesToWidgets.clear();

  MarkupsNodesIt markupsIterator = this->MarkupsNodes.begin();
  for (markupsIterator = this->MarkupsNodes.begin();
    markupsIterator != this->MarkupsNodes.end();
    ++markupsIterator)
    {
    this->RemoveObservations(*markupsIterator);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManagerHelper::AddMarkupsNode(vtkDMMLMarkupsNode* node)
{
  if (!node)
    {
    return;
    }
  vtkDMMLAbstractViewNode* viewNode = vtkDMMLAbstractViewNode::SafeDownCast(this->DisplayableManager->GetDMMLDisplayableNode());
  if (!viewNode)
    {
    return;
    }

  if (this->AddingMarkupsNode)
    {
    return;
    }
  this->AddingMarkupsNode = true;

  this->AddObservations(node);
  this->MarkupsNodes.insert(node);

  // Add Display Nodes
  int nnodes = node->GetNumberOfDisplayNodes();
  for (int i = 0; i<nnodes; i++)
    {
    vtkDMMLMarkupsDisplayNode *displayNode = vtkDMMLMarkupsDisplayNode::SafeDownCast(node->GetNthDisplayNode(i));

    // Check whether DisplayNode should be shown in this view
    if (!displayNode
      || !displayNode->IsA("vtkDMMLMarkupsDisplayNode")
      || !displayNode->IsDisplayableInView(viewNode->GetID()))
      {
      continue;
      }

    this->AddDisplayNode(displayNode);
    }

  this->AddingMarkupsNode = false;
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManagerHelper::RemoveMarkupsNode(vtkDMMLMarkupsNode* node)
{
  if (!node)
    {
    return;
    }

  vtkDMMLMarkupsDisplayableManagerHelper::MarkupsNodesIt displayableIt =
    this->MarkupsNodes.find(node);

  if (displayableIt == this->MarkupsNodes.end())
    {
    // we do not manage this markup
    return;
    }

  // Remove display nodes corresponding to this markups node
  for (vtkDMMLMarkupsDisplayableManagerHelper::DisplayNodeToWidgetIt widgetIterator = this->MarkupsDisplayNodesToWidgets.begin();
    widgetIterator != this->MarkupsDisplayNodesToWidgets.end();
    /*upon deletion the increment is done already, so don't increment here*/)
    {
    vtkDMMLMarkupsDisplayNode *markupsDisplayNode = widgetIterator->first;
    if (markupsDisplayNode->GetDisplayableNode() != node)
      {
      ++widgetIterator;
      }
    else
      {
      // display node of the node that is being removed
      vtkDMMLMarkupsDisplayableManagerHelper::DisplayNodeToWidgetIt widgetIteratorToRemove = widgetIterator;
      ++widgetIterator;
      vtkCjyxMarkupsWidget* widgetToRemove = widgetIteratorToRemove->second;
      this->DeleteWidget(widgetToRemove);
      this->MarkupsDisplayNodesToWidgets.erase(widgetIteratorToRemove);
      }
    }

  this->RemoveObservations(node);
  this->MarkupsNodes.erase(displayableIt);
}


//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManagerHelper::AddDisplayNode(vtkDMMLMarkupsDisplayNode* markupsDisplayNode)
{
  if (!markupsDisplayNode)
    {
    return;
    }

  // Do not add the display node if displayNodeIt is already associated with a widget object.
  // This happens when a segmentation node already associated with a display node
  // is copied into an other (using vtkDMMLNode::Copy()) and is added to the scene afterward.
  // Related issue are #3428 and #2608
  vtkDMMLMarkupsDisplayableManagerHelper::DisplayNodeToWidgetIt displayNodeIt
    = this->MarkupsDisplayNodesToWidgets.find(markupsDisplayNode);
  if (displayNodeIt != this->MarkupsDisplayNodesToWidgets.end())
    {
    return;
    }

  // There should not be a widget for the new node
  if (this->GetWidget(markupsDisplayNode) != nullptr)
    {
    vtkErrorMacro("vtkDMMLMarkupsDisplayableManager2D: A widget is already associated to this node");
    return;
    }

  vtkCjyxMarkupsWidget* newWidget = this->DisplayableManager->CreateWidget(markupsDisplayNode);
  if (!newWidget)
    {
    vtkErrorMacro("vtkDMMLMarkupsDisplayableManager2D: Failed to create widget");
    return;
    }

  // record the mapping between node and widget in the helper
  this->MarkupsDisplayNodesToWidgets[markupsDisplayNode] = newWidget;

  // Build representation
  newWidget->UpdateFromDMML(markupsDisplayNode, 0); // no specific event triggers full rebuild

  this->DisplayableManager->RequestRender();

  // Update cached matrices. Calls UpdateWidget
  //this->UpdateDisplayableTransforms(mNode);
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManagerHelper::RemoveDisplayNode(vtkDMMLMarkupsDisplayNode* markupsDisplayNode)
{
  if (!markupsDisplayNode)
    {
    return;
    }

  vtkDMMLMarkupsDisplayableManagerHelper::DisplayNodeToWidgetIt displayNodeIt
    = this->MarkupsDisplayNodesToWidgets.find(markupsDisplayNode);
  if (displayNodeIt == this->MarkupsDisplayNodesToWidgets.end())
    {
    // no widget found for this display node
    return;
    }

  vtkCjyxMarkupsWidget* widget = (displayNodeIt->second);
  this->DeleteWidget(widget);

  this->MarkupsDisplayNodesToWidgets.erase(markupsDisplayNode);
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManagerHelper::DeleteWidget(vtkCjyxMarkupsWidget* widget)
{
  if (!widget)
    {
    return;
    }
  widget->SetRenderer(nullptr);
  widget->SetRepresentation(nullptr);
  widget->Delete();
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManagerHelper::AddObservations(vtkDMMLMarkupsNode* node)
{
  vtkCallbackCommand* callbackCommand = this->DisplayableManager->GetDMMLNodesCallbackCommand();
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  for (auto observedMarkupNodeEvent : this->ObservedMarkupNodeEvents)
    {
    if (!broker->GetObservationExist(node, observedMarkupNodeEvent, this->DisplayableManager, callbackCommand))
      {
      broker->AddObservation(node, observedMarkupNodeEvent, this->DisplayableManager, callbackCommand);
      }
    }
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManagerHelper::RemoveObservations(vtkDMMLMarkupsNode* node)
{
  vtkCallbackCommand* callbackCommand = this->DisplayableManager->GetDMMLNodesCallbackCommand();
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  for (auto observedMarkupNodeEvent : this->ObservedMarkupNodeEvents)
    {
    vtkEventBroker::ObservationVector observations;
    observations = broker->GetObservations(node, observedMarkupNodeEvent, this, callbackCommand);
    broker->RemoveObservations(observations);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLMarkupsDisplayableManagerHelper::SetDisplayableManager(vtkDMMLMarkupsDisplayableManager* displayableManager)
{
  this->DisplayableManager = displayableManager;
}
