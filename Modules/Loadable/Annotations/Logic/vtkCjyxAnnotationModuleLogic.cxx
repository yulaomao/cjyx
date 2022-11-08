// Annotation includes
#include "vtkCjyxAnnotationModuleLogic.h"

// Annotation/DMML includes
#include "vtkDMMLAnnotationRulerNode.h"
#include "vtkDMMLAnnotationRulerStorageNode.h"
#include "vtkDMMLAnnotationTextDisplayNode.h"
#include "vtkDMMLAnnotationLineDisplayNode.h"
#include "vtkDMMLAnnotationFiducialNode.h"
#include "vtkDMMLAnnotationFiducialsStorageNode.h"
#include "vtkDMMLAnnotationHierarchyNode.h"
#include "vtkDMMLAnnotationPointDisplayNode.h"
#include "vtkDMMLAnnotationTextNode.h"
#include "vtkDMMLAnnotationROINode.h"
#include "vtkDMMLAnnotationSnapshotNode.h"
#include "vtkDMMLAnnotationSnapshotStorageNode.h"
#include "vtkDMMLAnnotationLinesStorageNode.h"

// DMML includes
#include <vtkDMMLInteractionNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSelectionNode.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLUnitNode.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPNGWriter.h>
#include <vtkVersion.h>

// STD includes
#include <algorithm>
#include <string>
#include <iostream>
#include <sstream>

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkCjyxAnnotationModuleLogic)

//-----------------------------------------------------------------------------
// vtkCjyxAnnotationModuleLogic methods
//-----------------------------------------------------------------------------
vtkCjyxAnnotationModuleLogic::vtkCjyxAnnotationModuleLogic()
{
  this->m_LastAddedAnnotationNode = nullptr;
  this->ActiveHierarchyNodeID = nullptr;

  this->m_MeasurementFormat = new char[8];
  sprintf(this->m_MeasurementFormat, "%s", "%.1f");

  this->m_CoordinateFormat = new char[8];
  sprintf(this->m_CoordinateFormat, "%s", "%.1f");

}

//-----------------------------------------------------------------------------
vtkCjyxAnnotationModuleLogic::~vtkCjyxAnnotationModuleLogic()
{

  if (this->m_LastAddedAnnotationNode)
    {
    this->m_LastAddedAnnotationNode = nullptr;
    }

  if (this->ActiveHierarchyNodeID)
    {
    delete [] this->ActiveHierarchyNodeID;
    this->ActiveHierarchyNodeID = nullptr;
    }

  if (this->m_MeasurementFormat)
    {
    delete[] this->m_MeasurementFormat;
    this->m_MeasurementFormat = nullptr;
    }

  if (this->m_CoordinateFormat)
    {
    delete[] this->m_CoordinateFormat;
    this->m_CoordinateFormat = nullptr;
    }
}

//-----------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  os << indent << "MeasurementFormat = " << (this->m_MeasurementFormat ? this->m_MeasurementFormat : "NULL") << std::endl;
  os << indent << "CoordinateFormat = " << (this->m_CoordinateFormat ? this->m_CoordinateFormat : "NULL") << std::endl;
  os << indent << "ActiveHierarchyNodeID = " << (this->ActiveHierarchyNodeID ? this->ActiveHierarchyNodeID : "NULL") << std::endl;
  if (this->m_LastAddedAnnotationNode)
    {
    os << indent << "LastAddedAnnotationNode: " << (this->m_LastAddedAnnotationNode->GetID() ? this->m_LastAddedAnnotationNode->GetID() : "NULL ID") << std::endl;
    }
}


//-----------------------------------------------------------------------------
// Load an annotation from file
//-----------------------------------------------------------------------------
char *vtkCjyxAnnotationModuleLogic::LoadAnnotation(const char *filename, const char *name, int fileType)
{
  char *nodeID = nullptr;
  if (!filename)
    {
    vtkErrorMacro("LoadAnnotation: null filename, cannot load");
    return nodeID;
    }
  vtkDebugMacro("LoadAnnotation: filename = " << filename << ", fileType = " << fileType);
//  std::cout << "LoadAnnotation: filename = " << filename << ", fileType = " << fileType << std::endl;

  // turn on batch processing
  this->GetDMMLScene()->StartState(vtkDMMLScene::BatchProcessState);

  if (fileType == this->Fiducial)
    {
    vtkNew<vtkDMMLAnnotationFiducialsStorageNode> fStorageNode;
    vtkNew<vtkDMMLAnnotationFiducialNode> fnode;
    fnode->SetName(name);

    fStorageNode->SetFileName(filename);

    // add the storage node to the scene
    this->GetDMMLScene()->AddNode(fStorageNode.GetPointer());
    fnode->SetScene(this->GetDMMLScene());

    this->GetDMMLScene()->AddNode(fnode.GetPointer());
    fnode->SetAndObserveStorageNodeID(fStorageNode->GetID());

    if (fStorageNode->ReadData(fnode.GetPointer()))
      {
      vtkDebugMacro("LoadAnnotation: fiducial storage node read " << filename);
      nodeID =  fnode->GetID();
      }
    }
  else if (fileType == this->Ruler)
    {
    vtkNew<vtkDMMLAnnotationRulerStorageNode> rStorageNode;
    vtkNew<vtkDMMLAnnotationRulerNode> rNode;
    rNode->SetName(name);

    rStorageNode->SetFileName(filename);

    // add to the scene
    this->GetDMMLScene()->AddNode(rStorageNode.GetPointer());
    rNode->Initialize(this->GetDMMLScene());
    rNode->SetAndObserveStorageNodeID(rStorageNode->GetID());

    if (rStorageNode->ReadData(rNode.GetPointer()))
      {
      vtkDebugMacro("LoadAnnotation: ruler storage node read " << filename);
      nodeID = rNode->GetID();
      }
    }
  else if (fileType == this->ROI)
    {
    vtkNew<vtkDMMLAnnotationLinesStorageNode> roiStorageNode;
    vtkNew<vtkDMMLAnnotationROINode> roiNode;
    roiNode->SetName(name);

    roiStorageNode->SetFileName(filename);

    // add the storage node to the scene
    this->GetDMMLScene()->AddNode(roiStorageNode.GetPointer());
    roiNode->Initialize(this->GetDMMLScene());
    roiNode->SetAndObserveStorageNodeID(roiStorageNode->GetID());

    if (roiStorageNode->ReadData(roiNode.GetPointer()))
      {
      vtkDebugMacro("LoadAnnotation: fiducial storage node read " << filename);
      nodeID =  roiNode->GetID();
      }
    }
  else
    {
    vtkErrorMacro("LoadAnnotation: unknown file type " << fileType << ", cannot read " << filename);
    }
  // turn off batch processing
  this->GetDMMLScene()->EndState(vtkDMMLScene::BatchProcessState);

  return nodeID;
}

//-----------------------------------------------------------------------------
char *vtkCjyxAnnotationModuleLogic::AddFiducial(double r, double a, double s,
                                                  const char *label)
{
  char *nodeID = nullptr;
  vtkNew<vtkDMMLAnnotationFiducialNode> fnode;

  if (label != nullptr)
    {
    fnode->SetName(label);
    }
  fnode->SetFiducialCoordinates(r, a, s);
  fnode->Initialize(this->GetDMMLScene());

  nodeID = fnode->GetID();

  return nodeID;
}

//-----------------------------------------------------------------------------
//
//
// DMML event handling
//
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::ProcessDMMLNodesEvents(vtkObject *vtkNotUsed(caller),
                                                            unsigned long event,
                                                            void *callData)
{
  vtkDebugMacro("ProcessDMMLNodesEvents: Event "<< event);

  vtkDMMLNode* node = reinterpret_cast<vtkDMMLNode*> (callData);

  vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(
      node);
  if (annotationNode)
    {
    switch (event)
      {
      case vtkDMMLScene::NodeAddedEvent:
        this->OnDMMLSceneNodeAdded(annotationNode);
        break;
      case vtkCommand::ModifiedEvent:
        this->OnDMMLAnnotationNodeModifiedEvent(annotationNode);
        break;
      case vtkDMMLAnnotationControlPointsNode::ControlPointModifiedEvent:
        this->OnDMMLAnnotationNodeModifiedEvent(annotationNode);
        break;
      }
    return;
    }

}

//-----------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::OnDMMLSceneNodeAdded(vtkDMMLNode* node)
{
  vtkDebugMacro("OnDMMLSceneNodeAddedEvent");
  // don't respond if the scene is importing as the nodes will have hierarchy
  // nodes already defined
  if (this->GetDMMLScene() &&
      (this->GetDMMLScene()->IsImporting() ||
       this->GetDMMLScene()->IsRestoring()))
    {
    return;
    }

  vtkDMMLAnnotationNode * annotationNode = vtkDMMLAnnotationNode::SafeDownCast(
      node);
  if (!annotationNode)
    {
    return;
    }

  // check for missing display nodes (if we're not in batch processing mode)
  if (this->GetDMMLScene() &&
      !this->GetDMMLScene()->IsBatchProcessing())
    {
    // check if no display nodes have been added already via calls to Initialize
    if (annotationNode->GetDisplayNode() == nullptr)
      {
      // keep it down to one modify event from the node (will be node added
      // events from the new nodes)
      int modifyFlag = annotationNode->StartModify();
      vtkDebugMacro("OnDMMLSceneNodeAddedEvent: adding display nodes for " << annotationNode->GetName());
      if (vtkDMMLAnnotationLinesNode::SafeDownCast(annotationNode))
        {
        vtkDMMLAnnotationLinesNode::SafeDownCast(annotationNode)->CreateAnnotationLineDisplayNode();
        }
      if (vtkDMMLAnnotationControlPointsNode::SafeDownCast(annotationNode))
        {
        vtkDMMLAnnotationControlPointsNode::SafeDownCast(annotationNode)->CreateAnnotationPointDisplayNode();
        }
      annotationNode->CreateAnnotationTextDisplayNode();
      annotationNode->EndModify(modifyFlag);
      }
    }

  // set up the hierarchy for the new annotation node if necessary
  bool retval = this->AddHierarchyNodeForAnnotation(annotationNode);
  if (!retval)
    {
    vtkErrorMacro("OnDMMLSceneNodeAddedEvent: No hierarchyNode added.");
    return;
    }

  // we pass the hierarchy node along - it includes the pointer to the actual annotationNode
  this->AddNodeCompleted(annotationNode);
}

//-----------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::OnDMMLAnnotationNodeModifiedEvent(vtkDMMLNode* node)
{
  vtkDebugMacro("OnDMMLAnnotationNodeModifiedEvent " << node->GetID());

  vtkDMMLAnnotationNode * annotationNode = vtkDMMLAnnotationNode::SafeDownCast(
      node);
  if (!annotationNode)
    {
    return;
    }

  this->InvokeEvent(RefreshRequestEvent);
}

//-----------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::OnDMMLSceneEndClose()
{
  if (this->m_LastAddedAnnotationNode)
    {
    this->m_LastAddedAnnotationNode = nullptr;
    }

  if (this->GetActiveHierarchyNodeID())
    {
    this->SetActiveHierarchyNodeID(nullptr);
    }
}

//---------------------------------------------------------------------------
//
//
// Placement of Annotations
//
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Set the internal dmml scene and observe events on it
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::SetDMMLSceneInternal(vtkDMMLScene * newScene)
{
  // a good time to add the observed events!
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkDMMLScene::NodeAddedEvent);
//  events->InsertNextValue(vtkDMMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkDMMLScene::EndCloseEvent);
  this->SetAndObserveDMMLSceneEventsInternal(newScene, events.GetPointer());
}

//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::ObserveDMMLScene()
{
  // add known annotation types to the selection node
  vtkDMMLSelectionNode *selectionNode = vtkDMMLSelectionNode::SafeDownCast(
      this->GetDMMLScene()->GetNodeByID("vtkDMMLSelectionNodeSingleton"));
  if (selectionNode)
    {
    if (selectionNode->PlaceNodeClassNameInList("vtkDMMLAnnotationRulerNode") < 0
      || selectionNode->PlaceNodeClassNameInList("vtkDMMLAnnotationROINode") < 0)
      {
      // got into batch mode
      this->GetDMMLScene()->StartState(vtkDMMLScene::BatchProcessState);

      vtkDebugMacro("vtkCjyxAnnotationModuleLogic::ObserveDMMLScene(): adding new annotation class names to selection node place list");
      /// Markups handle placement of new fiducials
      // selectionNode->AddNewPlaceNodeClassNameToList("vtkDMMLAnnotationFiducialNode", ":/Icons/AnnotationPointWithArrow.png", "Fiducial");
      // selectionNode->AddNewPlaceNodeClassNameToList("vtkDMMLAnnotationTextNode",  ":/Icons/AnnotationTextWithArrow.png", "Text");
      selectionNode->AddNewPlaceNodeClassNameToList("vtkDMMLAnnotationRulerNode", ":/Icons/AnnotationDistanceWithArrow.png", "Ruler");
      selectionNode->AddNewPlaceNodeClassNameToList("vtkDMMLAnnotationROINode", ":/Icons/AnnotationROIWithArrow.png", "ROI");

      // stop batch add
      this->GetDMMLScene()->EndState(vtkDMMLScene::BatchProcessState);
      }
    }
  // Superclass::ObserveDMMLScene calls UpdateFromDMMLScene();
  this->Superclass::ObserveDMMLScene();
}

//-----------------------------------------------------------------------------
// Add Annotation Node
//-----------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::AddAnnotationNode(const char * nodeDescriptor, bool persistent)
{

  vtkDMMLSelectionNode *selectionNode = nullptr;
  if (this->GetDMMLScene())
    {
    selectionNode = vtkDMMLSelectionNode::SafeDownCast(this->GetDMMLScene()->GetNodeByID("vtkDMMLSelectionNodeSingleton"));
    }
  if (!selectionNode)
    {
    vtkErrorMacro("AddAnnotationNode: No selection node in the scene.");
    return;
    }

  selectionNode->SetActivePlaceNodeClassName(nodeDescriptor);

  this->StartPlaceMode(persistent);

}

//---------------------------------------------------------------------------
// Start the place mouse mode
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::StartPlaceMode(bool persistent, vtkDMMLInteractionNode* interactionNode)
{
  if (!interactionNode && this->GetDMMLScene())
    {
    interactionNode = vtkDMMLInteractionNode::SafeDownCast(this->GetDMMLScene()->GetNodeByID("vtkDMMLInteractionNodeSingleton"));
    }
  if (!interactionNode)
    {
    vtkErrorMacro ( "StartPlaceMode: No interaction node in the scene." );
    return;
    }

  interactionNode->SetCurrentInteractionMode(vtkDMMLInteractionNode::Place);

  interactionNode->SetPlaceModePersistence(persistent ? 1 : 0);

  if (interactionNode->GetCurrentInteractionMode()
      != vtkDMMLInteractionNode::Place)
    {

    vtkErrorMacro("StartPlaceMode: Could not set place mode!");
    return;

    }
}

//---------------------------------------------------------------------------
// called after a new annotation node was added, now add it to the table in the GUI
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::AddNodeCompleted(vtkDMMLAnnotationNode* annotationNode)
{

  if (!annotationNode)
    {
    return;
    }

  this->InvokeEvent(RefreshRequestEvent);

  this->m_LastAddedAnnotationNode = annotationNode;

}

//---------------------------------------------------------------------------
// Exit the place mode
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::StopPlaceMode(bool persistent, vtkDMMLInteractionNode* interactionNode)
{

  vtkDMMLSelectionNode *selectionNode = nullptr;
  if (this->GetDMMLScene())
    {
    selectionNode = vtkDMMLSelectionNode::SafeDownCast(
      this->GetDMMLScene()->GetNodeByID("vtkDMMLSelectionNodeSingleton"));
    }
  if (!selectionNode)
    {
    vtkErrorMacro("StopPlaceMode: No selection node in the scene.");
    return;
    }

  if (!interactionNode && this->GetDMMLScene())
    {
    interactionNode = vtkDMMLInteractionNode::SafeDownCast(this->GetDMMLScene()->GetNodeByID("vtkDMMLInteractionNodeSingleton"));
    }
  if (interactionNode == nullptr)
    {
    vtkErrorMacro ( "StopPlaceMode: No interaction node in the scene." );
    return;
    }

  if (persistent)
    {
    // if persistent placement was activated in the Annotation GUI, we do not want to reset it
    interactionNode->SetPlaceModePersistence(1);
    }
  else
    {
    // if persistent placement was not activated in the Annotation GUI, then we want to reset it
    interactionNode->SetPlaceModePersistence(0);
    }

  interactionNode->SwitchToViewTransformMode();
  if (interactionNode->GetCurrentInteractionMode()
      != vtkDMMLInteractionNode::ViewTransform)
    {

    vtkErrorMacro("StopPlaceMode: Could not set transform mode!");

    }
  // reset the active annotation id after switching to view transform mode,
  // since this is checked in the displayable managers
  selectionNode->SetActivePlaceNodeClassName("");
}

//---------------------------------------------------------------------------
// Cancel the current placement or remove the last placed node
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::CancelCurrentOrRemoveLastAddedAnnotationNode(vtkDMMLInteractionNode* interactionNode)
{
  if (!interactionNode && this->GetDMMLScene())
    {
    interactionNode = vtkDMMLInteractionNode::SafeDownCast(this->GetDMMLScene()->GetNodeByID("vtkDMMLInteractionNodeSingleton"));
    }
  if (!interactionNode)
    {
    vtkErrorMacro("CancelCurrentOrRemoveLastAddedAnnotationNode: No interaction node");
    return;
    }

  interactionNode->InvokeEvent(vtkDMMLInteractionNode::EndPlacementEvent);

}

//---------------------------------------------------------------------------
/// Remove an AnnotationNode and also its 1-1 IS-A hierarchyNode, if found.
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::RemoveAnnotationNode(vtkDMMLAnnotationNode* annotationNode)
{
  if (!annotationNode)
    {
    vtkErrorMacro("RemoveAnnotationNode: no node to remove.");
    return;
    }
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("RemoveAnnotationNode: No DMML Scene found.");
    return;
    }

  // remove the 1-1 IS-A hierarchy node first
  vtkDMMLDisplayableHierarchyNode* displayableHierarchyNode =
      vtkDMMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(
          annotationNode->GetScene(), annotationNode->GetID());
  if (displayableHierarchyNode)
    {
    // there is a parent
    this->GetDMMLScene()->RemoveNode(displayableHierarchyNode);

    }

  this->GetDMMLScene()->RemoveNode(annotationNode);

}

//---------------------------------------------------------------------------
//
//
// Annotation Properties as an interface to DMML
//
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Register the DMML node classes to the attached scene.
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::RegisterNodes()
{
  if (!this->GetDMMLScene())
    {
    vtkWarningMacro("RegisterNodes: no scene");
    return;
    }

  vtkDMMLScene *scene = this->GetDMMLScene();

  //
  // The core nodes
  //

  // base nodes
  vtkNew<vtkDMMLAnnotationNode> annotationNode;
  scene->RegisterNodeClass(annotationNode.GetPointer());

  vtkNew<vtkDMMLAnnotationDisplayNode> annotationDisplayNode;
  scene->RegisterNodeClass(annotationDisplayNode.GetPointer());

  vtkNew<vtkDMMLAnnotationStorageNode> annotationStorageNode;
  scene->RegisterNodeClass(annotationStorageNode.GetPointer());

  // Control Points
  vtkNew<vtkDMMLAnnotationControlPointsNode> annotationControlPointsNode;
  scene->RegisterNodeClass(annotationControlPointsNode.GetPointer());

  vtkNew<vtkDMMLAnnotationControlPointsStorageNode> annotationControlPointsStorageNode;
  scene->RegisterNodeClass(annotationControlPointsStorageNode.GetPointer());

  vtkNew<vtkDMMLAnnotationPointDisplayNode> annotationControlPointsDisplayNode;
  scene->RegisterNodeClass(annotationControlPointsDisplayNode.GetPointer());

  // Lines
  vtkNew<vtkDMMLAnnotationLinesNode> annotationLinesNode;
  scene->RegisterNodeClass(annotationLinesNode.GetPointer());

  vtkNew<vtkDMMLAnnotationLinesStorageNode> annotationLinesStorageNode;
  scene->RegisterNodeClass(annotationLinesStorageNode.GetPointer());

  vtkNew<vtkDMMLAnnotationLineDisplayNode> annotationLinesDisplayNode;
  scene->RegisterNodeClass(annotationLinesDisplayNode.GetPointer());

  // Text
  vtkNew<vtkDMMLAnnotationTextDisplayNode> annotationTextDisplayNode;
  scene->RegisterNodeClass(annotationTextDisplayNode.GetPointer());

  //
  // Now the actual Annotation tool nodes
  //

  // Snapshot annotation
  vtkNew<vtkDMMLAnnotationSnapshotNode> annotationSnapshotNode;
  scene->RegisterNodeClass(annotationSnapshotNode.GetPointer());

  vtkNew<vtkDMMLAnnotationSnapshotStorageNode> annotationSnapshotStorageNode;
  scene->RegisterNodeClass(annotationSnapshotStorageNode.GetPointer());

  // Text annotation
  vtkNew<vtkDMMLAnnotationTextNode> annotationTextNode;
  scene->RegisterNodeClass(annotationTextNode.GetPointer());

  // Ruler annotation
  vtkNew<vtkDMMLAnnotationRulerNode> annotationRulerNode;
  scene->RegisterNodeClass(annotationRulerNode.GetPointer());

  vtkNew<vtkDMMLAnnotationRulerStorageNode> annotationRulerStorageNode;
  scene->RegisterNodeClass(annotationRulerStorageNode.GetPointer());

  // ROI annotation
  vtkNew<vtkDMMLAnnotationROINode> annotationROINode;
  scene->RegisterNodeClass(annotationROINode.GetPointer());
  // ROI annotation backwards compatibility
#if DMML_APPLICATION_SUPPORT_VERSION < DMML_VERSION_CHECK(4, 0, 0)
  scene->RegisterNodeClass(annotationROINode.GetPointer(), "ROI");
#endif

  // Fiducial annotation
  vtkNew<vtkDMMLAnnotationFiducialNode> annotationFiducialNode;
  scene->RegisterNodeClass(annotationFiducialNode.GetPointer());

  vtkNew<vtkDMMLAnnotationFiducialsStorageNode> annotationFiducialsStorageNode;
  scene->RegisterNodeClass(annotationFiducialsStorageNode.GetPointer());

  //
  // Annotation hierarchies
  //
  vtkNew<vtkDMMLAnnotationHierarchyNode> annotationHierarchyNode;
  scene->RegisterNodeClass(annotationHierarchyNode.GetPointer());
}

//---------------------------------------------------------------------------
// Check if the id points to an annotation node
//---------------------------------------------------------------------------
bool vtkCjyxAnnotationModuleLogic::IsAnnotationNode(const char* id)
{
  if (!id ||
      !this->GetDMMLScene())
    {
    return false;
    }

  vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(
      this->GetDMMLScene()->GetNodeByID(id));

  if (annotationNode)
    {
    return true;
    }

  return false;

}

//---------------------------------------------------------------------------
// Check if the id points to an annotation hierarchy node
//---------------------------------------------------------------------------
bool vtkCjyxAnnotationModuleLogic::IsAnnotationHierarchyNode(const char* id)
{
  if (!id || !this->GetDMMLScene())
    {
    return false;
    }

  vtkDMMLAnnotationHierarchyNode* hierarchyNode = vtkDMMLAnnotationHierarchyNode::SafeDownCast(
      this->GetDMMLScene()->GetNodeByID(id));

  if (hierarchyNode)
    {
    return true;
    }

  return false;

}

//---------------------------------------------------------------------------
// Return the name of an annotation DMML Node
//---------------------------------------------------------------------------
const char * vtkCjyxAnnotationModuleLogic::GetAnnotationName(const char * id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationName: no id");
    return nullptr;
    }

  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("GetAnnotationName: Could not get the DMML node with id " << id);
    return nullptr;
    }

  vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(node);

  if (annotationNode)
    {
    return annotationNode->GetName();
    }

  vtkDMMLAnnotationHierarchyNode *hierarchyNode = vtkDMMLAnnotationHierarchyNode::SafeDownCast(node);
  if (hierarchyNode)
    {
    return hierarchyNode->GetName();
    }
  return nullptr;
}

//---------------------------------------------------------------------------
// Return the text of an annotation DMML Node
//---------------------------------------------------------------------------
vtkStdString vtkCjyxAnnotationModuleLogic::GetAnnotationText(const char* id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationText: no id supplied");
    return "";
    }

  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("GetAnnotationText: no dmml scene.");
    return "";
    }

  vtkDMMLNode* node = this->GetDMMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationText: Could not get the DMML node with id " << id);
    return "";
    }

  // special case for annotation snapShots
  vtkDMMLAnnotationSnapshotNode* snapshotNode =
      vtkDMMLAnnotationSnapshotNode::SafeDownCast(node);
  if (snapshotNode)
    {
    return snapshotNode->GetSnapshotDescription();
    }
  // end of special case for annotation snapShots

  vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationText: Could not get the annotationDMML node.");
    return "";
    }

  return annotationNode->GetText(0);

}

//---------------------------------------------------------------------------
// Set the text of an annotation DMML Node
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::SetAnnotationText(const char* id, const char * newtext)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationText: no id specified");
    return;
    }

  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("SetAnnotationText: Could not get the DMML node with id " << id);
    return;
    }

  // special case for snapShots
  vtkDMMLAnnotationSnapshotNode* snapshotNode =
      vtkDMMLAnnotationSnapshotNode::SafeDownCast(node);
  if (snapshotNode)
    {

    snapshotNode->SetSnapshotDescription(vtkStdString(newtext));

    // now bail out
    return;
    }
  // end of special case for snapShots

  vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationText: Could not get the annotationDMML node.");
    return;
    }

  if (!newtext)
    {
    vtkErrorMacro("SetAnnotationText: No text supplied, using an empty string.");
    annotationNode->SetText(0, "", 1, 1);
    return;
    }

  annotationNode->SetText(0, newtext, 1, 1);
}

//---------------------------------------------------------------------------
// Get the textScale of a DMML Annotation node
//---------------------------------------------------------------------------
double vtkCjyxAnnotationModuleLogic::GetAnnotationTextScale(const char* id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationTextScale: no id specified");
    return 0;
    }

  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("GetAnnotationTextScale: Could not get the DMML node with id " << id);
    return 0;
    }

  vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationTextScale: Could not get the annotation DMML node.");
    return 0;
    }

  return annotationNode->GetTextScale();

}

//---------------------------------------------------------------------------
// Set the textScale of a DMML Annotation node
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::SetAnnotationTextScale(const char* id, double textScale)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationTextScale: no id given");
    return;
    }
  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }
  if (!node)
    {
    vtkErrorMacro("SetAnnotationTextScale: Could not get the DMML node with id " << id);
    return;
    }

  vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationTextScale: Could not get the annotation DMML node.");
    return;
    }

  annotationNode->SetTextScale(textScale);

}

//---------------------------------------------------------------------------
// Get the selected text color of a DMML Annotation node
//---------------------------------------------------------------------------
double * vtkCjyxAnnotationModuleLogic::GetAnnotationTextSelectedColor(const char* id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationTextSelectedColor: no id specified");
    return nullptr;
    }
  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("GetAnnotationTextSelectedColor: Could not get the DMML node with id " << id);
    return nullptr;
    }

  vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationTextSelectedColor: Could not get the annotation DMML node.");
    return nullptr;
    }

  if (!annotationNode->GetAnnotationTextDisplayNode())
    {
    return nullptr;
    }

  return annotationNode->GetAnnotationTextDisplayNode()->GetSelectedColor();

}

//---------------------------------------------------------------------------
// Set the selected text color of a DMML Annotation node
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::SetAnnotationTextSelectedColor(const char* id, double * color)
{
  if (!id)
    {
    return;
    }
  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("SetAnnotationTextSelectedColor: Could not get the DMML node with id " << id);
    return;
    }

  vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationTextSelectedColor: Could not get the annotation DMML node.");
    return;
    }

  annotationNode->GetAnnotationTextDisplayNode()->SetSelectedColor(color);

  annotationNode->InvokeEvent(vtkCommand::ModifiedEvent);

}

//---------------------------------------------------------------------------
// Get the unselected text color of a DMML Annotation node
//---------------------------------------------------------------------------
double * vtkCjyxAnnotationModuleLogic::GetAnnotationTextUnselectedColor(const char* id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationTextUnselectedColor: no id given");
    return nullptr;
    }
  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("GetAnnotationTextUnselectedColor: Could not get the DMML node with id " << id);
    return nullptr;
    }

  vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationTextUnselectedColor: Could not get the annotation DMML node.");
    return nullptr;
    }

  if (!annotationNode->GetAnnotationTextDisplayNode())
    {
    return nullptr;
    }

  return annotationNode->GetAnnotationTextDisplayNode()->GetColor();

}

//---------------------------------------------------------------------------
// Set the unselected text color of a DMML Annotation node
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::SetAnnotationTextUnselectedColor(const char* id, double * color)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationTextUnselectedColor: no id given");
    return;
    }
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("SetAnnotationTextUnselectedColor: Could not get the DMML scene");
    return;
    }
  vtkDMMLNode* node = this->GetDMMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationTextUnselectedColor: Could not get the DMML node with id " << id);
    return;
    }

  vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationTextUnselectedColor: Could not get the annotation DMML node with id " << id);
    return;
    }

  if (!annotationNode->GetAnnotationTextDisplayNode())
    {
    vtkErrorMacro("SetAnnotationTextUnselectedColor: Could not get the text display node for the annotation DMML node with id " << id);
    return;
    }
  annotationNode->GetAnnotationTextDisplayNode()->SetColor(color);

  annotationNode->InvokeEvent(vtkCommand::ModifiedEvent);

}

//---------------------------------------------------------------------------
// Get the color of an annotation node
//---------------------------------------------------------------------------
double * vtkCjyxAnnotationModuleLogic::GetAnnotationColor(const char *id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationColor: no id given, cannot get color");
    return nullptr;
    }
  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("GetAnnotationColor: Could not get the DMML node for id " << id);
    return nullptr;
    }

  vtkDMMLDisplayableNode* annotationNode =
      vtkDMMLDisplayableNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationColor: Could not get the displayable DMML node for id " << id);
    return nullptr;
    }

  if (annotationNode->GetDisplayNode() == nullptr)
    {
    vtkErrorMacro("GetAnnotationColor: Could not get the display node for node " << id);
    return nullptr;
    }

  return annotationNode->GetDisplayNode()->GetSelectedColor();
}

//---------------------------------------------------------------------------
/// Set the color of an annotation dmml node
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::SetAnnotationColor(const char *id, double *color)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationColor: no id given, cannot set color");
    return;
    }
  if (!color)
    {
    vtkErrorMacro("SetAnnotationColor: no color given, cannot set color for node " << id);
    return;
    }

  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("SetAnnotationColor: Could not get the DMML node for id " << id);
    return;
    }

  vtkDMMLDisplayableNode* annotationNode =
      vtkDMMLDisplayableNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationColor: Could not get the displayable DMML node for id " << id);
    return;
    }

  if (annotationNode->GetDisplayNode() == nullptr)
    {
    vtkErrorMacro("SetAnnotationColor: Could not get the display node for node " << id);
    return;
    }

  annotationNode->GetDisplayNode()->SetSelectedColor(color);
  // this should trigger a display modified event, but it's not being caught
  annotationNode->InvokeEvent(vtkCommand::ModifiedEvent);
}

//---------------------------------------------------------------------------
// Get the unselected color of an annotation node
//---------------------------------------------------------------------------
double * vtkCjyxAnnotationModuleLogic::GetAnnotationUnselectedColor(const char *id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationUnselectedColor: no id given, cannot get color");
    return nullptr;
    }
  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("GetAnnotationUnselectedColor: Could not get the DMML node for id " << id);
    return nullptr;
    }

  vtkDMMLDisplayableNode* annotationNode =
      vtkDMMLDisplayableNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationUnselectedColor: Could not get the displayable DMML node for id " << id);
    return nullptr;
    }

  if (annotationNode->GetDisplayNode() == nullptr)
    {
    vtkErrorMacro("GetAnnotationUnselectedColor: Could not get the display node for node " << id);
    return nullptr;
    }

  return annotationNode->GetDisplayNode()->GetColor();
}

//---------------------------------------------------------------------------
/// Set the color of an annotation dmml node
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::SetAnnotationUnselectedColor(const char *id, double *color)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationUnselectedColor: no id given, cannot set color");
    return;
    }
  if (!color)
    {
    vtkErrorMacro("SetAnnotationUnselectedColor: no color given, cannot set color for node " << id);
    return;
    }

  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("SetAnnotationUnselectedColor: Could not get the DMML node for id " << id);
    return;
    }

  vtkDMMLDisplayableNode* annotationNode =
      vtkDMMLDisplayableNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationUnselectedColor: Could not get the displayable DMML node for id " << id);
    return;
    }

  if (annotationNode->GetDisplayNode() == nullptr)
    {
    vtkErrorMacro("SetAnnotationUnselectedColor: Could not get the display node for node " << id);
    return;
    }

  annotationNode->GetDisplayNode()->SetColor(color);
  // this should trigger a display modified event, but it's not being caught
  annotationNode->InvokeEvent(vtkCommand::ModifiedEvent);
}

//---------------------------------------------------------------------------
// Get the color of an annotation point node
//---------------------------------------------------------------------------
double * vtkCjyxAnnotationModuleLogic::GetAnnotationPointColor(const char *id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationPointColor: no id given, cannot get color");
    return nullptr;
    }
  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("GetAnnotationPointColor: Could not get the DMML node for id " << id);
    return nullptr;
    }

  vtkDMMLAnnotationControlPointsNode* annotationNode =
      vtkDMMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationPointColor: Could not get the displayable control points DMML node for id " << id);
    return nullptr;
    }

  if (annotationNode->GetAnnotationPointDisplayNode() == nullptr)
    {
    vtkErrorMacro("GetAnnotationPointColor: Could not get the point display node for node " << id);
    return nullptr;
    }

  return annotationNode->GetAnnotationPointDisplayNode()->GetSelectedColor();
}

//---------------------------------------------------------------------------
/// Set the color of an annotation point node
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::SetAnnotationPointColor(const char *id, double *color)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationPointColor: no id given, cannot set color");
    return;
    }
  if (!color)
    {
    vtkErrorMacro("SetAnnotationPointColor: no color given, cannot set color for node " << id);
    return;
    }

  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("SetAnnotationPointColor: Could not get the DMML node for id " << id);
    return;
    }

  vtkDMMLAnnotationControlPointsNode* annotationNode =
      vtkDMMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationPointColor: Could not get the displayable control points DMML node for id " << id);
    return;
    }

  if (annotationNode->GetAnnotationPointDisplayNode() == nullptr)
    {
    vtkErrorMacro("SetAnnotationPointColor: Could not get the display node for node " << id);
    return;
    }

  annotationNode->GetAnnotationPointDisplayNode()->SetSelectedColor(color);
  // this should trigger a display modified event, but it's not being caught
  annotationNode->InvokeEvent(vtkCommand::ModifiedEvent);
}

//---------------------------------------------------------------------------
// Get the unselected color of an annotation point node
//---------------------------------------------------------------------------
double * vtkCjyxAnnotationModuleLogic::GetAnnotationPointUnselectedColor(const char *id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationPointUnselectedColor: no id given, cannot get color");
    return nullptr;
    }
  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("GetAnnotationPointUnselectedColor: Could not get the DMML node for id " << id);
    return nullptr;
    }

  vtkDMMLAnnotationControlPointsNode* annotationNode =
      vtkDMMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationPointUnselectedColor: Could not get the displayable control points DMML node for id " << id);
    return nullptr;
    }

  if (annotationNode->GetAnnotationPointDisplayNode() == nullptr)
    {
    vtkErrorMacro("GetAnnotationPointUnselectedColor: Could not get the point display node for node " << id);
    return nullptr;
    }

  return annotationNode->GetAnnotationPointDisplayNode()->GetColor();
}

//---------------------------------------------------------------------------
/// Set the color of an annotation point node
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::SetAnnotationPointUnselectedColor(const char *id, double *color)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationPointUnselectedColor: no id given, cannot set color");
    return;
    }
  if (!color)
    {
    vtkErrorMacro("SetAnnotationPointUnselectedColor: no color given, cannot set color for node " << id);
    return;
    }

  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("SetAnnotationPointUnselectedColor: Could not get the DMML node for id " << id);
    return;
    }

  vtkDMMLAnnotationControlPointsNode* annotationNode =
      vtkDMMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationPointUnselectedColor: Could not get the displayable control points DMML node for id " << id);
    return;
    }

  if (annotationNode->GetAnnotationPointDisplayNode() == nullptr)
    {
    vtkErrorMacro("SetAnnotationPointUnselectedColor: Could not get the point display node for node " << id);
    return;
    }

  annotationNode->GetAnnotationPointDisplayNode()->SetColor(color);
}

//---------------------------------------------------------------------------
// Get the glyph type of the annotation point node as a string
//---------------------------------------------------------------------------
const char * vtkCjyxAnnotationModuleLogic::GetAnnotationPointGlyphTypeAsString(const char *id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationPointGlyphTypeAsString: no id given, cannot get glyph type");
    return nullptr;
    }

  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("GetAnnotationPointGlyphTypeAsString: Could not get the DMML node for id " << id);
    return nullptr;
    }

  vtkDMMLAnnotationControlPointsNode* annotationNode =
      vtkDMMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationPointGlyphTypeAsString: Could not get the displayable control points DMML node for id " << id);
    return nullptr;
    }

  if (annotationNode->GetAnnotationPointDisplayNode() == nullptr)
    {
    vtkErrorMacro("GetAnnotationPointGlyphTypeAsString: Could not get the point display node for node " << id);
    return nullptr;
    }

  return annotationNode->GetAnnotationPointDisplayNode()->GetGlyphTypeAsString();
}

//---------------------------------------------------------------------------
// Get the glyph type of the annotation point node
//---------------------------------------------------------------------------
int vtkCjyxAnnotationModuleLogic::GetAnnotationPointGlyphType(const char *id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationPointGlyphType: no id given, cannot get glyph type");
    return 0;
    }
  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("GetAnnotationPointGlyphTypeAsString: Could not get the DMML node for id " << id);
    return 0;
    }

  vtkDMMLAnnotationControlPointsNode* annotationNode =
      vtkDMMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationPointGlyphTypeAsString: Could not get the displayable control points DMML node for id " << id);
    return 0;
    }

  if (annotationNode->GetAnnotationPointDisplayNode() == nullptr)
    {
    vtkErrorMacro("GetAnnotationPointGlyphType: Could not get the point display node for node " << id);
    return 0;
    }

  return annotationNode->GetAnnotationPointDisplayNode()->GetGlyphType();
}

//---------------------------------------------------------------------------
// Set the glyph type of the annotation point node
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::SetAnnotationPointGlyphTypeFromString(const char *id, const char *glyphType)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationPointGlyphTypeFromString: no id given, cannot set glyph type");
    return;
    }
  if (!glyphType)
    {
    vtkErrorMacro("SetAnnotationPointGlyphTypeFromString: no glyph type given, cannot set glyph type for node " << id);
    return;
    }

  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("SetAnnotationPointGlyphTypeFromString: Could not get the DMML node for id " << id);
    return;
    }

  vtkDMMLAnnotationControlPointsNode* annotationNode =
      vtkDMMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationPointGlyphTypeFromString: Could not get the displayable control points DMML node for id " << id);
    return;
    }

  if (annotationNode->GetAnnotationPointDisplayNode() == nullptr)
    {
    vtkErrorMacro("SetAnnotationPointGlyphTypeFromString: Could not get the point display node for node " << id);
    return;
    }

  annotationNode->GetAnnotationPointDisplayNode()->SetGlyphTypeFromString(
      glyphType);
}

//---------------------------------------------------------------------------
// Set the glyph type of the annotation point node
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::SetAnnotationPointGlyphType(const char *id, int glyphType)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationPointGlyphType: no id given, cannot set glyph type to " << glyphType);
    return;
    }
  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("SetAnnotationPointGlyphType: Could not get the DMML node for id " << id);
    return;
    }

  vtkDMMLAnnotationControlPointsNode* annotationNode =
      vtkDMMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationPointGlyphType: Could not get the displayable control points DMML node for id " << id);
    return;
    }

  if (annotationNode->GetAnnotationPointDisplayNode() == nullptr)
    {
    vtkErrorMacro("SetAnnotationPointGlyphType: Could not get the point display node for node " << id);
    return;
    }

  annotationNode->GetAnnotationPointDisplayNode()->SetGlyphType(glyphType);
}

//---------------------------------------------------------------------------
// Get the color of an annotation line node
//---------------------------------------------------------------------------
double * vtkCjyxAnnotationModuleLogic::GetAnnotationLineColor(const char *id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationLineColor: no id given, cannot get color");
    return nullptr;
    }
  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }
  if (!node)
    {
    vtkErrorMacro("GetAnnotationLineColor: Could not get the DMML node for id " << id);
    return nullptr;
    }

  vtkDMMLAnnotationLinesNode* annotationNode =
      vtkDMMLAnnotationLinesNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationLineColor: Could not get the displayable control points DMML node for id " << id);
    return nullptr;
    }

  if (annotationNode->GetAnnotationLineDisplayNode() == nullptr)
    {
    vtkErrorMacro("GetAnnotationLineColor: Could not get the line display node for node " << id);
    return nullptr;
    }

  return annotationNode->GetAnnotationLineDisplayNode()->GetSelectedColor();
}

//---------------------------------------------------------------------------
/// Set the color of an annotation point node
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::SetAnnotationLineColor(const char *id, double *color)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationLineColor: no id given, cannot set color");
    return;
    }
  if (!color)
    {
    vtkErrorMacro("SetAnnotationLineColor: no color given, cannot set color for node " << id);
    return;
    }

  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("SetAnnotationLineColor: Could not get the DMML node for id " << id);
    return;
    }

  vtkDMMLAnnotationLinesNode* annotationNode =
      vtkDMMLAnnotationLinesNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationLineColor: Could not get the displayable control points DMML node for id " << id);
    return;
    }

  if (annotationNode->GetAnnotationPointDisplayNode() == nullptr)
    {
    vtkErrorMacro("SetAnnotationLineColor: Could not get the point display node for node " << id);
    return;
    }

  annotationNode->GetAnnotationLineDisplayNode()->SetSelectedColor(color);
  // this should trigger a display modified event, but it's not being caught
  annotationNode->InvokeEvent(vtkCommand::ModifiedEvent);
}

//---------------------------------------------------------------------------
// Get the unselected color of an annotation point node
//---------------------------------------------------------------------------
double * vtkCjyxAnnotationModuleLogic::GetAnnotationLineUnselectedColor(const char *id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationLineUnselectedColor: no id given, cannot get color");
    return nullptr;
    }
  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("GetAnnotationLineUnselectedColor: Could not get the DMML node for id " << id);
    return nullptr;
    }

  vtkDMMLAnnotationLinesNode* annotationNode =
      vtkDMMLAnnotationLinesNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationLineUnselectedColor: Could not get the displayable control points DMML node for id " << id);
    return nullptr;
    }

  if (annotationNode->GetAnnotationLineDisplayNode() == nullptr)
    {
    vtkErrorMacro("GetAnnotationLineUnselectedColor: Could not get the line display node for node " << id);
    return nullptr;
    }

  return annotationNode->GetAnnotationLineDisplayNode()->GetColor();
}

//---------------------------------------------------------------------------
/// Set the color of an annotation point node
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::SetAnnotationLineUnselectedColor(const char *id, double *color)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationLineUnselectedColor: no id given, cannot set color");
    return;
    }
  if (!color)
    {
    vtkErrorMacro("SetAnnotationLineUnselectedColor: no color given, cannot set color for node " << id);
    return;
    }

  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("SetAnnotationLineUnselectedColor: Could not get the DMML node for id " << id);
    return;
    }

  vtkDMMLAnnotationLinesNode* annotationNode =
      vtkDMMLAnnotationLinesNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationLineUnselectedColor: Could not get the displayable control points DMML node for id " << id);
    return;
    }

  if (annotationNode->GetAnnotationLineDisplayNode() == nullptr)
    {
    vtkErrorMacro("SetAnnotationLineUnselectedColor: Could not get the line display node for node " << id);
    return;
    }

  annotationNode->GetAnnotationLineDisplayNode()->SetColor(color);
}

//---------------------------------------------------------------------------
// Get the measurement value of a DMML Annotation node
//---------------------------------------------------------------------------
const char * vtkCjyxAnnotationModuleLogic::GetAnnotationMeasurement(const char* id, bool showUnits)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationMeasurement: no id given");
    return nullptr;
    }
  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("GetAnnotationMeasurement: Could not get the DMML node with id " << id);
    return nullptr;
    }

  // reset stringHolder
  this->m_StringHolder = "";

  // special case for annotation snapShots
  vtkDMMLAnnotationSnapshotNode* snapshotNode =
      vtkDMMLAnnotationSnapshotNode::SafeDownCast(node);
  if (snapshotNode)
    {
    return m_StringHolder.c_str();
    }
  // end of special case for annotation snapShots

  vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(
      node);
  vtkDMMLSelectionNode* selectionNode =  vtkDMMLSelectionNode::SafeDownCast(
    this->GetDMMLScene()->GetNodeByID("vtkDMMLSelectionNodeSingleton"));

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationMeasurement: Could not get the annotation DMML node.");
    return nullptr;
    }

  if (node->IsA("vtkDMMLAnnotationRulerNode"))
    {
    double length = vtkDMMLAnnotationRulerNode::SafeDownCast(annotationNode)->GetDistanceMeasurement();
    char string[512];
    sprintf(string, this->m_MeasurementFormat,length);

    std::string unit = string;
    if (showUnits)
      {
      vtkDMMLUnitNode* lengthUnit = selectionNode ? selectionNode->GetUnitNode("length") : nullptr;
      if (lengthUnit)
        {
        unit = lengthUnit->GetDisplayStringFromValue(length);
        }
      else
        {
        unit += " mm";
        }
      }
    std::ostringstream ss;
    ss << unit;

    this->m_StringHolder = ss.str();
    }
  else if (node->IsA("vtkDMMLAnnotationFiducialNode"))
    {
    std::ostringstream ss;
    double
        * tmpPtr =
            vtkDMMLAnnotationFiducialNode::SafeDownCast(annotationNode)->GetControlPointCoordinates(
                0);
    if (tmpPtr)
      {
      double coordinates[3] = { tmpPtr[0], tmpPtr[1], tmpPtr[2] };

      char string[512];
      sprintf(string, this->m_CoordinateFormat, coordinates[0]);
      char string2[512];
      sprintf(string2, this->m_CoordinateFormat, coordinates[1]);
      char string3[512];
      sprintf(string3, this->m_CoordinateFormat, coordinates[2]);

      ss << string << ", " << string2 << ", " << string3;

      this->m_StringHolder = ss.str();
      }
    }

  return this->m_StringHolder.c_str();

}

//---------------------------------------------------------------------------
// Return the icon of an annotation DMML Node
//---------------------------------------------------------------------------
const char * vtkCjyxAnnotationModuleLogic::GetAnnotationIcon(const char* id)
{
  if (!this->GetDMMLScene() || id == nullptr)
    {
    return nullptr;
    }
  vtkDMMLNode *dmmlNode = this->GetDMMLScene()->GetNodeByID(id);
  return this->GetAnnotationIcon(dmmlNode);
}

//---------------------------------------------------------------------------
// Return the icon of an annotation DMML Node
//---------------------------------------------------------------------------
const char * vtkCjyxAnnotationModuleLogic
::GetAnnotationIcon(vtkDMMLNode* dmmlNode)
{
  vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(
    dmmlNode);

  if (annotationNode)
    {
    return annotationNode->GetIcon();
    }

  vtkDMMLAnnotationHierarchyNode* hierarchyNode =
    vtkDMMLAnnotationHierarchyNode::SafeDownCast(dmmlNode);

  if (hierarchyNode)
    {
    return hierarchyNode->GetIcon();
    }

  vtkDMMLAnnotationSnapshotNode* snapshotNode =
    vtkDMMLAnnotationSnapshotNode::SafeDownCast(dmmlNode);

  if (snapshotNode)
    {
    return snapshotNode->GetIcon();
    }

  return nullptr;
}

//---------------------------------------------------------------------------
// Return the lock flag for this annotation
//---------------------------------------------------------------------------
int vtkCjyxAnnotationModuleLogic::GetAnnotationLockedUnlocked(const char * id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationLockedUnlocked: no id");
    return 0;
    }
  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }
  if (!node)
    {
    vtkErrorMacro("GetAnnotationLockedUnlocked: Could not get the DMML node with id " << id);
    return 0;
    }

  vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationLockedUnlocked: Could not get the annotationDMML node.");
    return 0;
    }

  // lock this annotation
  return annotationNode->GetLocked();

}

//---------------------------------------------------------------------------
// Toggles the lock on this annotation
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::SetAnnotationLockedUnlocked(const char * id)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationLockedUnlocked: no id");
    return;
    }
  vtkDMMLNode* node = nullptr;
  if (this->GetDMMLScene())
    {
    node = this->GetDMMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("SetAnnotationLockedUnlocked: Could not get the DMML node with id " << id);
    return;
    }

  vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationLockedUnlocked: Could not get the annotationDMML node.");
    return;
    }

  // lock this annotation
  annotationNode->SetLocked(!annotationNode->GetLocked());

}

//---------------------------------------------------------------------------
// Return the visibility flag for this annotation
//---------------------------------------------------------------------------
int vtkCjyxAnnotationModuleLogic::GetAnnotationVisibility(const char * id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationVisibility: no id given");
    return 0;
    }
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("No scene defined");
    return 0;
    }
  vtkDMMLNode* node = this->GetDMMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationVisibility: Could not get the DMML node with id " << id);
    return 0;
    }

  vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(
      node);

  if (annotationNode)
    {
    return annotationNode->GetDisplayVisibility();
    }

  // is it a hierarchy node?
  vtkDMMLAnnotationHierarchyNode *hnode = vtkDMMLAnnotationHierarchyNode::SafeDownCast(node);
  if (hnode && hnode->GetDisplayNode())
    {
    return hnode->GetDisplayNode()->GetVisibility();
    }

  return 0;

}

//---------------------------------------------------------------------------
// Toggles the visibility of this annotation
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::SetAnnotationVisibility(const char * id)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationVisibility: no id given");
    return;
    }
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("No scene defined");
    return;
    }
  vtkDMMLNode* node = this->GetDMMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationVisibility: Could not get the DMML node with id " << id);
    return;
    }

  vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(node);

  if (annotationNode)
    {
    // show/hide this annotation
    annotationNode->SetDisplayVisibility(!annotationNode->GetDisplayVisibility());
    return;
    }

  // or it might be a hierarchy node
  vtkDMMLAnnotationHierarchyNode *hnode = vtkDMMLAnnotationHierarchyNode::SafeDownCast(node);
  if (hnode && hnode->GetDisplayNode())
    {
    hnode->GetDisplayNode()->SetVisibility(!hnode->GetDisplayNode()->GetVisibility());
    }
}

//---------------------------------------------------------------------------
// Toggles the selected flag of this annotation
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::SetAnnotationSelected(const char * id, bool selected)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationSelected: no id given");
    return;
    }
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("No scene defined");
    return;
    }
  vtkDMMLNode* node = this->GetDMMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationSelected: Could not get the DMML node with id " << id);
    return;
    }

  // special case for snapshot and hierarchy nodes
  if (node->IsA("vtkDMMLAnnotationSnapshotNode") ||
      node->IsA("vtkDMMLAnnotationHierarchyNode"))
    {
    // directly bail out
    return;
    }
  // end of special case

  vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationSelected: Could not get the annotationDMML node.");
    return;
    }

  // select this annotation
  annotationNode->SetSelected(selected);

  annotationNode->InvokeEvent(vtkCommand::ModifiedEvent);

}

//---------------------------------------------------------------------------
// find all annotation nodes and annotation hierarchy nodes and set the selected flag on them
void vtkCjyxAnnotationModuleLogic::SetAllAnnotationsSelected(bool selected)
{
  if (this->GetDMMLScene() == nullptr)
    {
    return;
    }

  int numberOfHierarchyNodes =  this->GetDMMLScene()->GetNumberOfNodesByClass("vtkDMMLAnnotationHierarchyNode");
  for (int i = 0; i < numberOfHierarchyNodes; i++)
    {
    vtkDMMLNode *node = this->GetDMMLScene()->GetNthNodeByClass(i, "vtkDMMLAnnotationHierarchyNode");
    node->SetSelected(selected);
    }

  int numberOfAnnotationNodes =  this->GetDMMLScene()->GetNumberOfNodesByClass("vtkDMMLAnnotationNode");
  for (int i = 0; i < numberOfAnnotationNodes; i++)
    {
    vtkDMMLNode *node = this->GetDMMLScene()->GetNthNodeByClass(i, "vtkDMMLAnnotationNode");
    // use the helper method to localise special cases on annotation nodes
    this->SetAnnotationSelected(node->GetID(), selected);
    }


}

//---------------------------------------------------------------------------
// Backup an AnnotationDMML node
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::BackupAnnotationNode(const char * id)
{
  if (!id)
    {
    vtkErrorMacro("BackupAnnotationNode: no id given");
    return;
    }
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("No scene defined");
    return;
    }
  vtkDMMLNode* node = this->GetDMMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("BackupAnnotationNode: Could not get the DMML node with id " << id);
    return;
    }

  vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    if (!node->IsA("vtkDMMLAnnotationHierarchyNode"))
      {
      vtkErrorMacro("BackupAnnotationNode: Could not get the annotationDMML node.");
      }
    return;
    }

  annotationNode->CreateBackup();

  for (int i = 0; i < annotationNode->GetNumberOfDisplayNodes(); i++)
    {

    vtkDMMLAnnotationDisplayNode * displayNode =
        vtkDMMLAnnotationDisplayNode::SafeDownCast(
            annotationNode->GetNthDisplayNode(i));

    if (!displayNode)
      {
      vtkErrorMacro("BackupAnnotationNode: Could not get the annotationDMMLDisplay node number " << i << " with ID:" << annotationNode->GetID());
      }
    else
      {
      displayNode->CreateBackup();
      }

    }
}

//---------------------------------------------------------------------------
// Restore a backup version of a AnnotationDMML node
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::RestoreAnnotationNode(const char * id)
{
  if (!id)
    {
    vtkErrorMacro("RestoreAnnotationNode: no id given");
    return;
    }

  vtkDebugMacro("RestoreAnnotationNode: " << id);

  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("No scene defined");
    return;
    }

  vtkDMMLNode* node = this->GetDMMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("RestoreAnnotationNode: Could not get the DMML node with id " << id);
    return;
    }

  vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    if (!node->IsA("vtkDMMLAnnotationHierarchyNode"))
      {
      vtkErrorMacro("RestoreAnnotationNode: Could not get the annotationDMML node.");
      }
    return;
    }

  for (int i = 0; i < annotationNode->GetNumberOfDisplayNodes(); i++)
    {

    vtkDMMLAnnotationDisplayNode * displayNode =
        vtkDMMLAnnotationDisplayNode::SafeDownCast(
            annotationNode->GetNthDisplayNode(i));

    if (!displayNode)
      {
      vtkErrorMacro("RestoreAnnotationNode: Could not get the annotationDMMLDisplay node:" << displayNode);
      }
    else
      {
      // now restore
      displayNode->RestoreBackup();
      }

    } // end of displayNodes

  // now restore
  annotationNode->RestoreBackup();

  annotationNode->InvokeEvent(vtkCommand::ModifiedEvent);

}

//---------------------------------------------------------------------------
// Delete all backups of a AnnotationDMML node
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::DeleteBackupNodes(const char * id)
{
  if (!id)
    {
    vtkErrorMacro("DeleteBackupNodes: no id given");
    return;
    }

  vtkDebugMacro("DeleteBackupNodes: " << id);

  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("No scene defined");
    return;
    }
  vtkDMMLNode* node = this->GetDMMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("DeleteBackupNodes: Could not get the DMML node with id " << id);
    return;
    }

  vtkDMMLAnnotationNode* annotationNode =
      vtkDMMLAnnotationNode::SafeDownCast(node);

  if (!annotationNode)
    {
    if (!node->IsA("vtkDMMLAnnotationHierarchyNode"))
      {
      vtkErrorMacro("DeleteBackupNodes: Could not get the annotationDMML node.");
      }
    return;
    }

  for (int i = 0; i < annotationNode->GetNumberOfDisplayNodes(); i++)
    {

    vtkDMMLAnnotationDisplayNode * displayNode =
        vtkDMMLAnnotationDisplayNode::SafeDownCast(
            annotationNode->GetNthDisplayNode(i));

    if (!displayNode)
      {
      vtkErrorMacro("DeleteBackupNodes: Could not get the annotationDMMLDisplay node.");
      }
    else
      {
      displayNode->ClearBackup();
      }

    } // end of displayNodes

  annotationNode->ClearBackup();

}

//---------------------------------------------------------------------------
// Jump the 2d slices to the first control point location of an AnnotationDMML node
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::JumpSlicesToAnnotationCoordinate(const char * id)
{
  if (!id)
    {
    vtkErrorMacro("JumpSlicesToAnnotationCoordinate: no id given");
    return;
    }
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("No scene defined");
    return;
    }
  vtkDMMLNode* node = this->GetDMMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("JumpSlicesToAnnotationCoordinate: Could not get the DMML node with id " << id);
    return;
    }

  vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(node);

  if (!annotationNode)
    {
    vtkErrorMacro("JumpSlicesToAnnotationCoordinate: Could not get the annotationDMML node.");
      return;
    }

  // do not restore anything if this is a snapshot node
  if (annotationNode->IsA("vtkDMMLAnnotationSnapshotNode"))
    {
    return;
    }

  vtkDMMLAnnotationControlPointsNode* controlpointsNode = vtkDMMLAnnotationControlPointsNode::SafeDownCast(annotationNode);

  if (!controlpointsNode)
    {
    // we don't have a controlpointsNode so we can not jump the slices
    return;
    }

  // TODO for now only consider the first control point
  double *rasCoordinates = controlpointsNode->GetControlPointCoordinates(0);
  if (rasCoordinates)
    {
    double r = rasCoordinates[0];
    double a = rasCoordinates[1];
    double s = rasCoordinates[2];

    vtkDMMLSliceNode::JumpAllSlices(this->GetDMMLScene(), r, a, s);
    }
}

  //---------------------------------------------------------------------------
  const char * vtkCjyxAnnotationModuleLogic::MoveAnnotationUp(const char* id)
  {

    // reset stringHolder
    this->m_StringHolder = "";

    if (!id)
      {
      return this->m_StringHolder.c_str();
      }

    this->m_StringHolder = id;

    if (!this->GetDMMLScene())
      {
      vtkErrorMacro("No scene set.");
      return this->m_StringHolder.c_str();
      }

    vtkDMMLAnnotationNode* annotationNode =
        vtkDMMLAnnotationNode::SafeDownCast(this->GetDMMLScene()->GetNodeByID(
            id));

    if (!annotationNode)
      {
      vtkErrorMacro("MoveAnnotationUp: Could not get annotation node!");
      return this->m_StringHolder.c_str();
      }

    // get the corresponding hierarchy
    vtkDMMLAnnotationHierarchyNode* hNode =
        vtkDMMLAnnotationHierarchyNode::SafeDownCast(
            vtkDMMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(
                this->GetDMMLScene(), annotationNode->GetID()));

    if (!hNode)
      {
      vtkErrorMacro("MoveAnnotationUp: Could not get hierarchy node!");
      return this->m_StringHolder.c_str();
      }

    // where is it in the parent's list?
    int currentIndex = hNode->GetIndexInParent();
    vtkDebugMacro("MoveAnnotationUp: currentIndex = " << currentIndex);
    // now move it up one
    hNode->SetIndexInParent(currentIndex - 1);
    vtkDebugMacro("MoveAnnotationUp: after moving to " << currentIndex - 1 << ", current index is " << hNode->GetIndexInParent());
    // trigger an update on the q widget
    // done in the hierarchy node now when set the sorting value
    //annotationNode->Modified();
    // the id should be the same now
    this->m_StringHolder = annotationNode->GetID();
    return this->m_StringHolder.c_str();
  }

  //---------------------------------------------------------------------------
  const char* vtkCjyxAnnotationModuleLogic::MoveAnnotationDown(const char* id)
  {

    // reset stringHolder
    this->m_StringHolder = "";

    if (!id)
      {
      return this->m_StringHolder.c_str();
      }

    this->m_StringHolder = id;

    if (!this->GetDMMLScene())
      {
      vtkErrorMacro("MoveAnnotationDown: No scene set.");
      return this->m_StringHolder.c_str();
      }

    vtkDMMLAnnotationNode* annotationNode =
        vtkDMMLAnnotationNode::SafeDownCast(this->GetDMMLScene()->GetNodeByID(
            id));

    if (!annotationNode)
      {
      vtkErrorMacro("MoveAnnotationDown: Could not get annotation node!");
      return this->m_StringHolder.c_str();
      }

    // get the corresponding hierarchy
    vtkDMMLAnnotationHierarchyNode* hNode =
        vtkDMMLAnnotationHierarchyNode::SafeDownCast(
            vtkDMMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(
                this->GetDMMLScene(), annotationNode->GetID()));

    if (!hNode)
      {
      vtkErrorMacro("MoveAnnotationDown: Could not get hierarchy node!");
      return this->m_StringHolder.c_str();
      }

    // where is it in the parent's list?
    int currentIndex = hNode->GetIndexInParent();
    // now move it down one
    hNode->SetIndexInParent(currentIndex + 1);
    // trigger an update on the q widget
    annotationNode->Modified();
    // the id should be the same now
    this->m_StringHolder = annotationNode->GetID();
    return this->m_StringHolder.c_str();

  }

  //---------------------------------------------------------------------------
  //
  //
  // Annotation Hierarchy Functionality
  //
  //
  //---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Return the toplevel Annotation hierarchy node ID or create one if there is none.
// If an optional annotationNode is given, insert the toplevel hierarchy before it. If not,
// just add the toplevel hierarchy node.
//---------------------------------------------------------------------------
char * vtkCjyxAnnotationModuleLogic::GetTopLevelHierarchyNodeID(vtkDMMLNode* node)
{
  if (this->GetDMMLScene() == nullptr)
    {
    return nullptr;
    }
  const char *topLevelName = "All Annotations";
  char *toplevelNodeID = nullptr;
  vtkCollection *col = this->GetDMMLScene()->GetNodesByClass("vtkDMMLAnnotationHierarchyNode");
  vtkDMMLAnnotationHierarchyNode *toplevelNode = nullptr;
  unsigned int numNodes = col->GetNumberOfItems();
  if (numNodes != 0)
    {
    // iterate through the hierarchy nodes to find one with a name starting
    // with the top level name
    for (unsigned int n = 0; n < numNodes; n++)
      {
      vtkDMMLNode *thisNode = vtkDMMLNode::SafeDownCast(col->GetItemAsObject(n));
      if (thisNode && thisNode->GetName() &&
          strncmp(thisNode->GetName(), topLevelName, strlen(topLevelName)) == 0)
        {
        toplevelNode = vtkDMMLAnnotationHierarchyNode::SafeDownCast(col->GetItemAsObject(n));
        break;
        }
      }
    }

  if (!toplevelNode)
    {
    // no top level hierarchy node is currently in the scene, create a new one
    toplevelNode = vtkDMMLAnnotationHierarchyNode::New();

    toplevelNode->HideFromEditorsOff();
    toplevelNode->SetName(this->GetDMMLScene()->GetUniqueNameByString(topLevelName));

    if (!node)
      {
      this->GetDMMLScene()->AddNode(toplevelNode);
      }
    else
      {
      this->GetDMMLScene()->InsertBeforeNode(node, toplevelNode);
      }
    toplevelNodeID = toplevelNode->GetID();
    if (this->AddDisplayNodeForHierarchyNode(toplevelNode) == nullptr)
      {
      vtkErrorMacro("GetTopLevelHierarchyNodeID: error adding a display node for new top level node " << toplevelNodeID);
      }
    this->InvokeEvent(HierarchyNodeAddedEvent, toplevelNode);
    toplevelNode->Delete();
    }
  else
    {
    toplevelNodeID = toplevelNode->GetID();
    }
  col->RemoveAllItems();
  col->Delete();
  return toplevelNodeID;
}

//---------------------------------------------------------------------------
char * vtkCjyxAnnotationModuleLogic::GetTopLevelHierarchyNodeIDForNodeClass(vtkDMMLAnnotationNode *annotationNode)
{
  if (!annotationNode)
    {
    return nullptr;
    }
  if (this->GetDMMLScene() == nullptr)
    {
    return nullptr;
    }

  // get the set of hierarchy nodes to check through
  vtkCollection *col = nullptr;
  bool topLevelAnnotationIsActive = false;
  if (this->GetActiveHierarchyNode() && this->GetActiveHierarchyNode()->GetID() &&
      this->GetTopLevelHierarchyNodeID() &&
      !strcmp(this->GetActiveHierarchyNode()->GetID(), this->GetTopLevelHierarchyNodeID()))
    {
    topLevelAnnotationIsActive = true;
    }

  if (!topLevelAnnotationIsActive)
    {
    // just use the currently active annotation hierarchy if it exists
    if (this->GetActiveHierarchyNode())
      {
      return this->GetActiveHierarchyNode()->GetID();
      }
    }

  // find the per list annotation hierarchy nodes at the top level
  // look for any annotation hierarchy nodes in the scene
  col = this->GetDMMLScene()->GetNodesByClass("vtkDMMLAnnotationHierarchyNode");
  unsigned int numNodes = 0;
  if (col)
    {
    numNodes = col->GetNumberOfItems();
    }
  // iterate through the hierarchy nodes to find one with an attribute matching the input node's classname
  vtkDMMLAnnotationHierarchyNode *toplevelNode = nullptr;
  char *toplevelNodeID = nullptr;
  const char *attributeName = "MainChildType";
  for (unsigned int n = 0; n < numNodes; n++)
    {
    vtkDMMLNode *thisNode = vtkDMMLNode::SafeDownCast(col->GetItemAsObject(n));
    if (thisNode && thisNode->GetAttribute(attributeName) &&
        strcmp(thisNode->GetAttribute(attributeName), annotationNode->GetClassName()) == 0)
      {
      toplevelNode = vtkDMMLAnnotationHierarchyNode::SafeDownCast(col->GetItemAsObject(n));
      break;
      }
    }
  if (!toplevelNode)
    {
    // no hierarchy node mainly for this node class, create a new one
    toplevelNode = vtkDMMLAnnotationHierarchyNode::New();
    toplevelNode->SetAttribute(attributeName, annotationNode->GetClassName());
    toplevelNode->HideFromEditorsOff();
    // get the node tag name, remove the Annotation string, append List
    std::string nodeName = std::string(annotationNode->GetNodeTagName()).replace(0, strlen("Annotation"), "") + std::string(" List");
    toplevelNode->SetName(this->GetDMMLScene()->GetUniqueNameByString(nodeName.c_str()));
    // make it a child of the top annotation hierarchy
    toplevelNode->SetParentNodeID(this->GetTopLevelHierarchyNodeID());
    this->GetDMMLScene()->AddNode(toplevelNode);
    this->InvokeEvent(HierarchyNodeAddedEvent, toplevelNode);
    toplevelNodeID = toplevelNode->GetID();
    if (this->AddDisplayNodeForHierarchyNode(toplevelNode) == nullptr)
      {
      vtkErrorMacro("GetTopLevelHierarchyNodeIDForNodeClass: error adding a display node for hierarchy node " << toplevelNodeID);
      }
    toplevelNode->Delete();
    }
  else
    {
    toplevelNodeID = toplevelNode->GetID();
    }
  col->RemoveAllItems();
  col->Delete();
  return toplevelNodeID;
}

  //---------------------------------------------------------------------------
  // Add a new annotation hierarchy node for a given annotationNode.
  // The active hierarchy node will be the parent. If there is no
  // active hierarchy node, use the top-level annotation hierarchy node as the parent.
  // If there is no top-level annotation hierarchy node, create additionally a top-level hierarchy node which serves as
  // a parent to the new hierarchy node. Return true on success, false on failure.
  //---------------------------------------------------------------------------
  bool vtkCjyxAnnotationModuleLogic::AddHierarchyNodeForAnnotation(vtkDMMLAnnotationNode* annotationNode)
  {

    // check that there isn't already a hierarchy node for this node
    if (annotationNode && annotationNode->GetScene() && annotationNode->GetID())
      {
      vtkDMMLHierarchyNode *hnode =
          vtkDMMLHierarchyNode::GetAssociatedHierarchyNode(
              annotationNode->GetScene(), annotationNode->GetID());
      if (hnode != nullptr)
        {
        vtkDMMLAnnotationHierarchyNode *ahnode =
            vtkDMMLAnnotationHierarchyNode::SafeDownCast(hnode);
        if (ahnode != nullptr)
          {
          vtkWarningMacro("AddHierarchyNodeForAnnotation: annotation node " << annotationNode->GetID() << " already has a hierarchy node, returning.");
          return true;
          }
        else
          {
          vtkWarningMacro("AddHierarchyNodeForAnnotation: found a hierarchy node for this annotation node, but it's not an annotation hierarchy node, so adding a new one");
          }
        }
      }
    else
      {
      if (annotationNode)
        {
        vtkErrorMacro("AddHierarchyNodeForAnnotation: annotation node has no scene or id, not checking for existing hierarchy node");
        }
      }
  // is there an associated node?
/*  if (annotationNode->GetAttribute("AssociatedNodeID"))
    {
    // add a hierarchy for that node
    // add/get another displayable hierarchy that encapsulates both the associated node and the new fid node
    }
*/
    if (!this->GetActiveHierarchyNodeID())
      {
      // no active hierarchy node, this means we create the new node directly under the top-level hierarchy node
      char * toplevelHierarchyNodeID = nullptr;
      if (!annotationNode)
        {
        // we just add a new toplevel hierarchy node
        toplevelHierarchyNodeID = this->GetTopLevelHierarchyNodeID(nullptr);
        }
      else
        {
        // we need to insert the new toplevel hierarchy before the given annotationNode
        toplevelHierarchyNodeID = this->GetTopLevelHierarchyNodeID(annotationNode);
        }

      if (!toplevelHierarchyNodeID)
        {
        vtkErrorMacro("AddNewHierarchyNode: Toplevel hierarchy node was nullptr.");
        return false;
        }
      this->SetActiveHierarchyNodeID(toplevelHierarchyNodeID);
      }

    char *toplevelIDForThisClass = this->GetTopLevelHierarchyNodeIDForNodeClass(annotationNode);

    // Create a hierarchy node
    vtkDMMLAnnotationHierarchyNode* hierarchyNode =
        vtkDMMLAnnotationHierarchyNode::New();

    //hierarchyNode->SetScene(this->GetDMMLScene());

    if (!annotationNode)
      {
      // this is a user created hierarchy!

      // we want to see that!
      hierarchyNode->HideFromEditorsOff();

      hierarchyNode->SetName(
          this->GetDMMLScene()->GetUniqueNameByString("List"));

      this->GetDMMLScene()->AddNode(hierarchyNode);
      this->InvokeEvent(HierarchyNodeAddedEvent, hierarchyNode);

      }
    else
      {
      // this is the 1-1 hierarchy node for a given annotation node
      hierarchyNode->AllowMultipleChildrenOff();

      // we do not want to see that!
      hierarchyNode->HideFromEditorsOn();


      hierarchyNode->SetName(this->GetDMMLScene()->GetUniqueNameByString(
          "AnnotationHierarchy"));

      this->GetDMMLScene()->InsertBeforeNode(annotationNode, hierarchyNode);

      // set the displayable node id to point to this annotation node
      annotationNode->SetDisableModifiedEvent(1);
      hierarchyNode->SetDisplayableNodeID(annotationNode->GetID());
      annotationNode->SetDisableModifiedEvent(0);
      //annotationNode->Modified();
      }

    if (toplevelIDForThisClass)
      {
      hierarchyNode->SetParentNodeID(toplevelIDForThisClass);
      }
    else
      {
      hierarchyNode->SetParentNodeID(this->GetActiveHierarchyNodeID());
      }

    if (!annotationNode)
      {
      // we want it to be the active hierarchy from now on (do this after
      // setting the parent node id)
      this->SetActiveHierarchyNodeID(hierarchyNode->GetID());
      }
    // it's been added to the scene, delete this pointer
    hierarchyNode->Delete();
    return true;

  }

//---------------------------------------------------------------------------
// Add a new visible annotation hierarchy.
// The active hierarchy node will be the parent. If there is no
// active hierarchy node, use the top-level annotation hierarchy node as the parent.
// If there is no top-level annotation hierarchy node, create additionally a top-level hierarchy node which serves as
// a parent to the new hierarchy node. The newly added hierarchy node will be the
// active hierarchy node. Return true on success, false on failure.
//---------------------------------------------------------------------------
bool vtkCjyxAnnotationModuleLogic::AddHierarchy()
{
  return this->AddHierarchyNodeForAnnotation(nullptr);
}



//---------------------------------------------------------------------------
vtkDMMLAnnotationHierarchyNode *vtkCjyxAnnotationModuleLogic::GetActiveHierarchyNode()
{
  if (!this->GetActiveHierarchyNodeID())
    {
    // there was no active hierarchy
    // we then use the toplevel hierarchyNode
    char* toplevelNodeID = this->GetTopLevelHierarchyNodeID();

    if (!toplevelNodeID)
      {
      vtkErrorMacro("GetActiveHierarchyNode: Could not find or create any hierarchy.");
      return nullptr;
      }

    this->SetActiveHierarchyNodeID(toplevelNodeID);
    }
  if (this->GetDMMLScene()->GetNodeByID(this->GetActiveHierarchyNodeID()) == nullptr)
    {
    // try finding the top level hierarchy
    char* toplevelNodeID = this->GetTopLevelHierarchyNodeID();
    if (!toplevelNodeID)
      {
      vtkErrorMacro("GetActiveHierarchyNode: the active hierarchy node id was invalid and can't find or make a top level hierarchy node");
      // if the node with the active id can't be found in the scene, reset it to
      // null
      this->SetActiveHierarchyNodeID(nullptr);
      return nullptr;
      }
    else
      {
      this->SetActiveHierarchyNodeID(toplevelNodeID);
      }
    }
  return vtkDMMLAnnotationHierarchyNode::SafeDownCast(
    this->GetDMMLScene()->GetNodeByID(this->GetActiveHierarchyNodeID()));
}

  //---------------------------------------------------------------------------
  //
  //
  // Annotation SnapShot Functionality
  //
  //
  //---------------------------------------------------------------------------

  //---------------------------------------------------------------------------
  // Create a snapShot. This includes a screenshot of a specific view (see \ref GrabScreenShot(int screenshotWindow)),
  // a multiline text description.
  //---------------------------------------------------------------------------
  void vtkCjyxAnnotationModuleLogic::CreateSnapShot(const char* name, const char* description, int screenshotType, double scaleFactor, vtkImageData* screenshot)
  {
    if (!screenshot)
      {
      vtkErrorMacro("CreateSnapShot: No screenshot was set.");
      return;
      }
    if (!this->GetDMMLScene())
      {
      vtkErrorMacro("No scene defined");
      return;
      }

    vtkStdString nameString = vtkStdString(name);

    vtkDMMLAnnotationSnapshotNode * newSnapshotNode =
        vtkDMMLAnnotationSnapshotNode::New();
    newSnapshotNode->SetScene(this->GetDMMLScene());
    if (strcmp(nameString, ""))
      {
      // a name was specified
      newSnapshotNode->SetName(nameString.c_str());
      }
    else
      {
      // if no name is specified, generate a new unique one
      newSnapshotNode->SetName(this->GetDMMLScene()->GetUniqueNameByString(
          "Screenshot"));
      }
    newSnapshotNode->SetSnapshotDescription(description);
    newSnapshotNode->SetScreenShotType(screenshotType);
    newSnapshotNode->SetScreenShot(screenshot);
    newSnapshotNode->SetScaleFactor(scaleFactor);
    newSnapshotNode->HideFromEditorsOff();
    this->GetDMMLScene()->AddNode(newSnapshotNode);
    newSnapshotNode->Delete();
  }

  //---------------------------------------------------------------------------
  // Modify an existing annotation snapShot.
  //---------------------------------------------------------------------------
  void vtkCjyxAnnotationModuleLogic::ModifySnapShot(vtkStdString id, const char* name, const char* description, int screenshotType, double scaleFactor, vtkImageData* screenshot)
  {

    if (!screenshot)
      {
      vtkErrorMacro("ModifySnapShot: No screenshot was set.");
      return;
      }
    if (!this->GetDMMLScene())
      {
      vtkErrorMacro("No scene defined");
      return;
      }
    vtkDMMLNode* node = this->GetDMMLScene()->GetNodeByID(id.c_str());

    if (!node)
      {
      vtkErrorMacro("ModifySnapShot: Could not get node: " << id.c_str());
      return;
      }

    vtkDMMLAnnotationSnapshotNode* snapshotNode =
        vtkDMMLAnnotationSnapshotNode::SafeDownCast(node);

    if (!snapshotNode)
      {
      vtkErrorMacro("ModifySnapShot: Could not get snapshot node.");
      return;
      }

    vtkStdString nameString = vtkStdString(name);

    if (strcmp(nameString, ""))
      {
      // a name was specified
      snapshotNode->SetName(nameString.c_str());
      }
    else
      {
      // if no name is specified, generate a new unique one
      snapshotNode->SetName(this->GetDMMLScene()->GetUniqueNameByString(
          "Snapshot"));
      }
    snapshotNode->SetSnapshotDescription(description);
    snapshotNode->SetScreenShotType(screenshotType);
    snapshotNode->SetScreenShot(screenshot);
    snapshotNode->SetScaleFactor(scaleFactor);
    snapshotNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent,
        snapshotNode);

  }

//---------------------------------------------------------------------------
// Return the description of an existing Annotation snapShot node.
//---------------------------------------------------------------------------
vtkStdString vtkCjyxAnnotationModuleLogic::GetSnapShotName(const char* id)
{
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("No scene defined");
    return nullptr;
    }
  vtkDMMLNode* node = this->GetDMMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetSnapShotDescription: Could not get dmml node!");
      return nullptr;
    }

  vtkDMMLAnnotationSnapshotNode* snapshotNode =
    vtkDMMLAnnotationSnapshotNode::SafeDownCast(node);

  if (!snapshotNode)
    {
    vtkErrorMacro("GetSnapShotDescription: Could not get snapshot node!");
      return nullptr;
    }

  return snapshotNode->GetName();
}


//---------------------------------------------------------------------------
// Return the description of an existing Annotation snapShot node.
//---------------------------------------------------------------------------
vtkStdString vtkCjyxAnnotationModuleLogic::GetSnapShotDescription(const char* id)
{
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("No scene defined");
    return nullptr;
    }
  vtkDMMLNode* node = this->GetDMMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetSnapShotDescription: Could not get dmml node!");
      return nullptr;
    }

  vtkDMMLAnnotationSnapshotNode* snapshotNode =
    vtkDMMLAnnotationSnapshotNode::SafeDownCast(node);

  if (!snapshotNode)
    {
    vtkErrorMacro("GetSnapShotDescription: Could not get snapshot node!");
      return nullptr;
    }

  return snapshotNode->GetSnapshotDescription();
}

//---------------------------------------------------------------------------
// Return the screenshotType of an existing Annotation snapShot node.
//---------------------------------------------------------------------------
int vtkCjyxAnnotationModuleLogic::GetSnapShotScreenshotType(const char* id)
{
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("No scene defined");
    return 0;
    }
  vtkDMMLNode* node = this->GetDMMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetSnapShotScreenshotType: Could not get dmml node!");
      return 0;
    }

  vtkDMMLAnnotationSnapshotNode* snapshotNode =
    vtkDMMLAnnotationSnapshotNode::SafeDownCast(node);

  if (!snapshotNode)
    {
    vtkErrorMacro("GetSnapShotScreenshotType: Could not get snapshot node!");
      return 0;
    }

  return snapshotNode->GetScreenShotType();
}

//---------------------------------------------------------------------------
// Return the screenshotType of an existing Annotation snapShot node.
//---------------------------------------------------------------------------
double vtkCjyxAnnotationModuleLogic::GetSnapShotScaleFactor(const char* id)
{
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("No scene defined");
    return 0;
    }
  vtkDMMLNode* node = this->GetDMMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetSnapShotScaleFactor: Could not get dmml node!");
      return 0;
    }

  vtkDMMLAnnotationSnapshotNode* snapshotNode =
    vtkDMMLAnnotationSnapshotNode::SafeDownCast(node);

  if (!snapshotNode)
    {
    vtkErrorMacro("GetSnapShotScaleFactor: Could not get snapshot node!");
      return 0;
    }

  return snapshotNode->GetScaleFactor();
}

//---------------------------------------------------------------------------
// Return the screenshot of an existing Annotation snapShot node.
//---------------------------------------------------------------------------
vtkImageData* vtkCjyxAnnotationModuleLogic::GetSnapShotScreenshot(const char* id)
{
  vtkDMMLNode* node = this->GetDMMLScene()->GetNodeByID(id);
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("No scene defined");
    return nullptr;
    }
  if (!node)
    {
    vtkErrorMacro("GetSnapShotScreenshot: Could not get dmml node!");
      return nullptr;
    }

  vtkDMMLAnnotationSnapshotNode* snapshotNode =
    vtkDMMLAnnotationSnapshotNode::SafeDownCast(node);

  if (!snapshotNode)
    {
    vtkErrorMacro("GetSnapShotScreenshot: Could not get snapshot node!");
      return nullptr;
    }

  return snapshotNode->GetScreenShot();
}

//---------------------------------------------------------------------------
// Check if node id corresponds to a snapShot node.
//---------------------------------------------------------------------------
bool vtkCjyxAnnotationModuleLogic::IsSnapshotNode(const char* id)
{

  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("No scene defined");
    return false;
    }
  vtkDMMLNode* node = this->GetDMMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("IsSnapshotNode: Invalid node.");
    return false;
    }

  return node->IsA("vtkDMMLAnnotationSnapshotNode");

}
//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::SetHierarchyAnnotationsVisibleFlag(vtkDMMLAnnotationHierarchyNode* hierarchyNode, bool flag)
{
  if (hierarchyNode == nullptr)
    {
    // use the active one
    hierarchyNode = this->GetActiveHierarchyNode();
    }
  if (!hierarchyNode)
    {
    vtkErrorMacro("SetHierarchyAnnotationsVisibleFlag: no hierarchy node");
    return;
    }
  vtkNew<vtkCollection> children;
  hierarchyNode->GetChildrenDisplayableNodes(children);

  children->InitTraversal();
  for (int i=0; i<children->GetNumberOfItems(); ++i)
    {
    vtkDMMLAnnotationNode* childNode = vtkDMMLAnnotationNode::SafeDownCast(children->GetItemAsObject(i));
    if (childNode)
      {
      // this is a valid annotation child node
      childNode->SetDisplayVisibility((flag ? 1 : 0));
      }
    }
}

//---------------------------------------------------------------------------
void vtkCjyxAnnotationModuleLogic::SetHierarchyAnnotationsLockFlag(vtkDMMLAnnotationHierarchyNode* hierarchyNode, bool flag)
{
  if (hierarchyNode == nullptr)
    {
    // use the active one
    hierarchyNode = this->GetActiveHierarchyNode();
    }
  if (!hierarchyNode)
    {
    vtkErrorMacro("SetHierarchyAnnotationsLockFlag: no hierarchy node");
    return;
    }
  vtkNew<vtkCollection> children;
  hierarchyNode->GetChildrenDisplayableNodes(children);

  children->InitTraversal();
  for (int i=0; i<children->GetNumberOfItems(); ++i)
    {
    vtkDMMLAnnotationNode* childNode = vtkDMMLAnnotationNode::SafeDownCast(children->GetItemAsObject(i));
    if (childNode)
      {
      // this is a valid annotation child node
      childNode->SetLocked((flag ? 1 : 0));
      }
    }
}

//---------------------------------------------------------------------------
//
//
// Place Annotations programmatically
//
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//
//
// Report functionality
//
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
const char* vtkCjyxAnnotationModuleLogic::GetHTMLRepresentation(vtkDMMLAnnotationHierarchyNode* hierarchyNode, int level)
{
  if (!hierarchyNode)
    {
    vtkErrorMacro("GetHTMLRepresentation: We need a hierarchy Node here.");
    return nullptr;
    }

  vtkStdString html =
    vtkStdString("<tr bgcolor=#E0E0E0><td valign='middle'>");

  // level
  for (int i = 0; i < level; ++i)
    {
    html += "&nbsp;&nbsp;&nbsp;&nbsp;";
    }

  // icon
  html += "<img src='";
  const char *icon = this->GetAnnotationIcon(hierarchyNode->GetID());
  html += (icon ? icon : "");
  html += "'>";

  html += "</td><td valign='middle'>";
  html += "&nbsp;";
  html += "</td><td valign='middle'>";

  // text
  html += hierarchyNode->GetName();

  html += "</td></tr>";

  this->m_StringHolder = html;

  return this->m_StringHolder.c_str();
}

//---------------------------------------------------------------------------
const char* vtkCjyxAnnotationModuleLogic::GetHTMLRepresentation(vtkDMMLAnnotationNode* annotationNode, int level)
{
  if (!annotationNode)
    {
    vtkErrorMacro("GetHTMLRepresentation: We need an annotation Node here.");
    return nullptr;
    }

  vtkStdString html = vtkStdString("<tr><td valign='middle'>");

  // level
  for (int i = 0; i < level; ++i)
    {
    html += "&nbsp;&nbsp;&nbsp;&nbsp;";
    }

  // icon
  html += "<img src='";
  const char *icon = this->GetAnnotationIcon(annotationNode->GetID());
  if (icon)
    {
    html += icon;
    }
  html += "'>";

  html += "</td><td valign='middle'>";
  // if this is a snapshotNode, we want to include the image here
  if (annotationNode->IsA("vtkDMMLAnnotationSnapshotNode"))
    {
    vtkImageData* image =
      this->GetSnapShotScreenshot(annotationNode->GetID());

    if (image)
      {

      vtkStdString tempPath = vtkStdString(this->GetApplicationLogic()->GetTemporaryPath());
      tempPath.append(annotationNode->GetID());
      tempPath.append(".png");

      vtkNew<vtkPNGWriter> w;
      w->SetInputData(image);
      w->SetFileName(tempPath.c_str());
      w->Write();

      html += "<img src='";
      html += tempPath;
      html += "' width='400'>";

      }
    }
  else
    {
    const char *measurement =  this->GetAnnotationMeasurement(annotationNode->GetID(), true);
    if (measurement)
      {
      html += measurement;
      }
    }
  html += "</td><td valign='middle'>";

  // text
  vtkStdString txt = this->GetAnnotationText(annotationNode->GetID());
  if (txt)
    {
    html += txt;
    }
  // if this is a snapshotNode, we want to include the image here
  if (annotationNode->IsA("vtkDMMLAnnotationSnapshotNode"))
    {
    html += "<br><br>";
    vtkStdString desc =  this->GetSnapShotDescription(annotationNode->GetID());
    if (desc)
      {
      html += desc;
      }
    }

  html += "</td></tr>";

  this->m_StringHolder = html;

  return this->m_StringHolder.c_str();

}

//---------------------------------------------------------------------------
vtkDMMLAnnotationTextDisplayNode *vtkCjyxAnnotationModuleLogic::GetTextDisplayNode(const char *id)
{
  if (!id)
    {
    return nullptr;
    }
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("No scene defined");
    return nullptr;
    }
  vtkDMMLNode* node = this->GetDMMLScene()->GetNodeByID(id);
  if (!node)
    {
    return nullptr;
    }
  vtkDMMLAnnotationNode *textNode = vtkDMMLAnnotationNode::SafeDownCast(node);
  if (!textNode)
    {
    return nullptr;
    }
  return textNode->GetAnnotationTextDisplayNode();
}

//---------------------------------------------------------------------------
vtkDMMLAnnotationPointDisplayNode *vtkCjyxAnnotationModuleLogic::GetPointDisplayNode(const char *id)
{
  if (!id)
    {
    return nullptr;
    }
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("No scene defined");
    return nullptr;
    }
  vtkDMMLNode* node = this->GetDMMLScene()->GetNodeByID(id);
  if (!node)
    {
    return nullptr;
    }
  vtkDMMLAnnotationControlPointsNode *pointsNode =
    vtkDMMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!pointsNode)
    {
    return nullptr;
    }
  // get the point display node
  return pointsNode->GetAnnotationPointDisplayNode();
}

//---------------------------------------------------------------------------
vtkDMMLAnnotationLineDisplayNode *vtkCjyxAnnotationModuleLogic::GetLineDisplayNode(const char *id)
{
  if (!id)
    {
    return nullptr;
    }
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("No scene defined");
    return nullptr;
    }
  vtkDMMLNode* node = this->GetDMMLScene()->GetNodeByID(id);
  if (!node)
    {
    return nullptr;
    }
  vtkDMMLAnnotationLinesNode *linesNode =
    vtkDMMLAnnotationLinesNode::SafeDownCast(node);
  if (!linesNode)
    {
    return nullptr;
    }

  return linesNode->GetAnnotationLineDisplayNode();
}

//---------------------------------------------------------------------------
const char * vtkCjyxAnnotationModuleLogic::AddDisplayNodeForHierarchyNode(vtkDMMLAnnotationHierarchyNode *hnode)
{
  if (!hnode)
    {
    vtkErrorMacro("AddDisplayNodeForHierarchyNode: null input hierarchy node");
    return nullptr;
    }
  if (hnode->GetDisplayNode() && hnode->GetDisplayNodeID())
    {
    // it already has a display node
    return hnode->GetDisplayNodeID();
    }
  vtkDMMLAnnotationDisplayNode *dnode = vtkDMMLAnnotationDisplayNode::New();
  if (!dnode)
    {
    vtkErrorMacro("AddDisplayNodeForHierarchyNode: error creating a new display node");
    return nullptr;
    }
  dnode->SetVisibility(1);
  if (this->GetDMMLScene())
    {
    this->GetDMMLScene()->AddNode(dnode);
    hnode->SetAndObserveDisplayNodeID(dnode->GetID());
    }
  const char *id = dnode->GetID();
  dnode->Delete();
  return id;
}
