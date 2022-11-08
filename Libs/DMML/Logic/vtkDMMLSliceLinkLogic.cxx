/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLSliceLinkLogic.cxx,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

// DMMLLogic includes
#include "vtkDMMLSliceLinkLogic.h"
#include "vtkDMMLSliceLogic.h"
#include "vtkDMMLApplicationLogic.h"

// DMML includes
#include <vtkEventBroker.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceCompositeNode.h>
#include <vtkDMMLSliceNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkFloatArray.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>

// STD includes
#include <cassert>


//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLSliceLinkLogic);

//----------------------------------------------------------------------------
vtkDMMLSliceLinkLogic::vtkDMMLSliceLinkLogic()
{
  this->BroadcastingEvents = 0;
}

//----------------------------------------------------------------------------
vtkDMMLSliceLinkLogic::~vtkDMMLSliceLinkLogic() = default;

//----------------------------------------------------------------------------
void vtkDMMLSliceLinkLogic::BroadcastingEventsOn()
{
  this->BroadcastingEvents++;
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLinkLogic::BroadcastingEventsOff()
{
  this->BroadcastingEvents--;
}

//----------------------------------------------------------------------------
int vtkDMMLSliceLinkLogic::GetBroadcastingEvents()
{
  return this->BroadcastingEvents;
}


//----------------------------------------------------------------------------
void vtkDMMLSliceLinkLogic::SetDMMLSceneInternal(vtkDMMLScene * newScene)
{
  // List of events the slice logics should listen
  vtkNew<vtkIntArray> events;
  vtkNew<vtkFloatArray> priorities;

  float normalPriority = 0.0;
  float lowPriority = -0.5;
  // float highPriority = 0.5;

  // Events that use the default priority.  Don't care the order they
  // are triggered
  events->InsertNextValue(vtkDMMLScene::NodeAddedEvent);
  priorities->InsertNextValue(normalPriority);
  events->InsertNextValue(vtkDMMLScene::NodeRemovedEvent);
  priorities->InsertNextValue(normalPriority);
  events->InsertNextValue(vtkDMMLScene::StartBatchProcessEvent);
  priorities->InsertNextValue(normalPriority);
  events->InsertNextValue(vtkDMMLScene::StartImportEvent);
  priorities->InsertNextValue(normalPriority);
  events->InsertNextValue(vtkDMMLScene::StartRestoreEvent);
  priorities->InsertNextValue(normalPriority);

  // Events that need to a lower priority than normal, in other words,
  // guaranteed to be be called after other triggers
  events->InsertNextValue(vtkDMMLScene::EndBatchProcessEvent);
  priorities->InsertNextValue(lowPriority);
  events->InsertNextValue(vtkDMMLScene::EndImportEvent);
  priorities->InsertNextValue(lowPriority);
  events->InsertNextValue(vtkDMMLScene::EndRestoreEvent);
  priorities->InsertNextValue(lowPriority);

  this->SetAndObserveDMMLSceneEventsInternal(newScene, events.GetPointer(), priorities.GetPointer());

  this->ProcessDMMLSceneEvents(newScene, vtkCommand::ModifiedEvent, nullptr);
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLinkLogic::OnDMMLSceneNodeAdded(vtkDMMLNode* node)
{
  if (!node)
    {
    return;
    }
  if (node->IsA("vtkDMMLSliceCompositeNode")
      || node->IsA("vtkDMMLSliceNode"))
    {
    vtkEventBroker::GetInstance()->AddObservation(
      node, vtkCommand::ModifiedEvent, this, this->GetDMMLNodesCallbackCommand());

    // If sliceNode we insert in our map the current status of the node
    vtkDMMLSliceNode* sliceNode = vtkDMMLSliceNode::SafeDownCast(node);
    SliceNodeStatusMap::iterator it = this->SliceNodeInteractionStatus.find(node->GetID());
    if (sliceNode && it == this->SliceNodeInteractionStatus.end())
      {
      this->SliceNodeInteractionStatus.insert(std::pair<std::string, SliceNodeInfos>
        (sliceNode->GetID(), SliceNodeInfos(sliceNode->GetInteracting())));
      }
    }
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLinkLogic::OnDMMLSceneNodeRemoved(vtkDMMLNode* node)
{
  if (!node)
    {
    return;
    }
  if (node->IsA("vtkDMMLSliceCompositeNode")
      || node->IsA("vtkDMMLSliceNode"))
    {
    vtkEventBroker::GetInstance()->RemoveObservations(
      node, vtkCommand::ModifiedEvent, this, this->GetDMMLNodesCallbackCommand());

    // Update the map
    vtkDMMLSliceNode* sliceNode = vtkDMMLSliceNode::SafeDownCast(node);
    if (sliceNode)
      {
      SliceNodeStatusMap::iterator it = this->SliceNodeInteractionStatus.find(node->GetID());
      if(it != this->SliceNodeInteractionStatus.end())
        {
        this->SliceNodeInteractionStatus.erase(it);
        }
      }
  }
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLinkLogic::OnDMMLNodeModified(vtkDMMLNode* node)
{
  // Update from SliceNode
  vtkDMMLSliceNode* sliceNode = vtkDMMLSliceNode::SafeDownCast(node);
  if ( sliceNode && sliceNode->GetID()
    && this->GetDMMLScene() && !this->GetDMMLScene()->IsBatchProcessing() )
    {
    SliceNodeStatusMap::iterator it = this->SliceNodeInteractionStatus.find(sliceNode->GetID());
    // if this is not the node that we are interacting with, short circuit

    if (!sliceNode->GetInteracting() || !sliceNode->GetInteractionFlags())
      {
      // We end up an interaction on the sliceNode
      if (it != this->SliceNodeInteractionStatus.end() && it->second.Interacting)
        {
        vtkDMMLSliceCompositeNode* compositeNode = this->GetCompositeNode(sliceNode);
        if (!compositeNode->GetHotLinkedControl() &&
            sliceNode->GetInteractionFlags() == vtkDMMLSliceNode::MultiplanarReformatFlag)
          {
          this->BroadcastSliceNodeEvent(sliceNode);
          }
        this->SliceNodeInteractionStatus.find(sliceNode->GetID())->second.Interacting =
          sliceNode->GetInteracting();
        }
      return;
      }

    // SliceNode was modified. Need to find the corresponding
    // SliceCompositeNode to check whether operations are linked
    vtkDMMLSliceCompositeNode* compositeNode = this->GetCompositeNode(sliceNode);

    if (compositeNode && compositeNode->GetLinkedControl())
      {
      // Slice node changed and slices are linked. Broadcast.
      //std::cout << "Slice node changed and slices are linked!" << std::endl;

      if (it != this->SliceNodeInteractionStatus.end() && !it->second.Interacting )
        {
        it->second.Interacting = sliceNode->GetInteracting();
        // Start Interaction event : we update the current sliceNodeNormal
        this->UpdateSliceNodeInteractionStatus(sliceNode);
        }

      if (compositeNode->GetHotLinkedControl() ||
          sliceNode->GetInteractionFlags() != vtkDMMLSliceNode::MultiplanarReformatFlag)
        {
        this->BroadcastSliceNodeEvent(sliceNode);
        }
      }
    else
      {
      // Slice node changed and slices are not linked. Do not broadcast.
      //std::cout << "Slice node changed and slices are NOT linked!" << std::endl;
      return;
      }
    }

  // Update from SliceCompositeNode
  vtkDMMLSliceCompositeNode* compositeNode
    = vtkDMMLSliceCompositeNode::SafeDownCast(node);
  if (compositeNode && this->GetDMMLScene() && !this->GetDMMLScene()->IsBatchProcessing())
    {

    // if this is not the node that we are interacting with, short circuit
    if (!compositeNode->GetInteracting()
        || !compositeNode->GetInteractionFlags())
      {
      return;
      }

    if (compositeNode && compositeNode->GetLinkedControl())
      {
      // Slice composite node changed and slices are linked. Broadcast.
      //std::cout << "SliceCompositeNode changed and slices are linked!" << std::endl;
      this->BroadcastSliceCompositeNodeEvent(compositeNode);
      }
    else
      {
      // Slice composite node changed and slices are not linked. Do
      // not broadcast.
      //std::cout << "SliceCompositeNode changed and slices are NOT linked!" << std::endl;
      }
    }
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLinkLogic::OnDMMLSceneStartBatchProcess()
{
  // Note the sense. Turning "on" tells the link logic that we are
  // already broadcasting an event, so don't rebroadcast.
  //std::cerr << "OnDMMLSceneStartBatchProcess" << std::endl;
  this->BroadcastingEventsOn();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLinkLogic::OnDMMLSceneEndBatchProcess()
{
  // Note the sense. Turning "off" tells the link logic that we are
  // not already broadcasting an event, so future events can be broadcast
  //std::cerr << "OnDMMLSceneEndBatchProcess" << std::endl;
  this->BroadcastingEventsOff();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLinkLogic::OnDMMLSceneStartImport()
{
  // Note the sense. Turning "on" tells the link logic that we are
  // already broadcasting an event, so don't rebroadcast.
  //std::cerr << "OnDMMLSceneStartImport" << std::endl;
  this->BroadcastingEventsOn();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLinkLogic::OnDMMLSceneEndImport()
{
  // Note the sense. Turning "off" tells the link logic that we are
  // not already broadcasting an event, so future events can be broadcast
  //std::cerr << "OnDMMLSceneEndImport" << std::endl;
  this->BroadcastingEventsOff();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLinkLogic::OnDMMLSceneStartRestore()
{
  // Note the sense. Turning "on" tells the link logic that we are
  // already broadcasting an event, so don't rebroadcast.
  //std::cerr << "OnDMMLSceneStartRestore" << std::endl;
  this->BroadcastingEventsOn();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLinkLogic::OnDMMLSceneEndRestore()
{
  // Note the sense. Turning "off" tells the link logic that we are
  // not already broadcasting an event, so future events can be broadcast
  //std::cerr << "OnDMMLSceneEndRestore" << std::endl;
  this->BroadcastingEventsOff();
}


//----------------------------------------------------------------------------
void vtkDMMLSliceLinkLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  vtkIndent nextIndent;
  nextIndent = indent.GetNextIndent();
}

//----------------------------------------------------------------------------
bool vtkDMMLSliceLinkLogic::IsOrientationMatching(vtkDMMLSliceNode *sliceNode1, vtkDMMLSliceNode *sliceNode2,
  double comparisonTolerance /* = 0.001 */)
{
  if (sliceNode1 == nullptr || sliceNode2 == nullptr)
    {
    vtkErrorMacro("vtkDMMLSliceLinkLogic::IsOrientationMatching failed: invalid input");
    return false;
    }
  vtkMatrix4x4* sliceToRAS1 = sliceNode1->GetSliceToRAS();
  vtkMatrix4x4* sliceToRAS2 = sliceNode2->GetSliceToRAS();
  for (int axisIndex = 0; axisIndex < 3; axisIndex++)
  {
    double axisVector1[3] = { sliceToRAS1->Element[0][axisIndex], sliceToRAS1->Element[1][axisIndex], sliceToRAS1->Element[2][axisIndex] };
    double axisVector2[3] = { sliceToRAS2->Element[0][axisIndex], sliceToRAS2->Element[1][axisIndex], sliceToRAS2->Element[2][axisIndex] };
    vtkMath::Normalize(axisVector1);
    vtkMath::Normalize(axisVector2);
    if ((fabs(axisVector1[0] - axisVector2[0]) > comparisonTolerance)
      || (fabs(axisVector1[1] - axisVector2[1]) > comparisonTolerance)
      || (fabs(axisVector1[2] - axisVector2[2]) > comparisonTolerance))
      {
      return false;
      }
  }
  return true;
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLinkLogic::BroadcastSliceNodeEvent(vtkDMMLSliceNode *sliceNode)
{
  // only broadcast a slice node event if we are not already actively
  // broadcasting events and we are actively interacting with the node
  // std::cout << "BroadcastingEvents: " << this->GetBroadcastingEvents()
  //           << ", Interacting: " << sliceNode->GetInteracting()
  //           << std::endl;
  if (!sliceNode)
    {
    return;
    }
  if (!this->GetBroadcastingEvents())
    {
    this->BroadcastingEventsOn();

    int requiredViewGroup = sliceNode->GetViewGroup();
    vtkDMMLSliceNode* sNode;
    vtkCollectionSimpleIterator it;
    vtkSmartPointer<vtkCollection> nodes;
    nodes.TakeReference(this->GetDMMLScene()->GetNodesByClass("vtkDMMLSliceNode"));
    for (nodes->InitTraversal(it);
        (sNode=vtkDMMLSliceNode::SafeDownCast(nodes->GetNextItemAsObject(it)));)
      {
      if (sNode == sliceNode || !sNode)
        {
        continue;
        }
      if (sNode->GetViewGroup() != requiredViewGroup)
        {
        continue;
        }

      // Link slice parameters whenever the reformation is consistent
      if (vtkDMMLSliceLinkLogic::IsOrientationMatching(sliceNode, sNode))
        {
        // std::cout << "Orientation match, flags = " << sliceNode->GetInteractionFlags() << std::endl;
        // std::cout << "Broadcasting SliceToRAS, SliceOrigin, and FieldOfView to "
        //            << sNode->GetName() << std::endl;
        //

        // Copy the slice to RAS information
        if (sliceNode->GetInteractionFlags() & sliceNode->GetInteractionFlagsModifier()
          & vtkDMMLSliceNode::SliceToRASFlag)
          {
          sNode->GetSliceToRAS()->DeepCopy( sliceNode->GetSliceToRAS() );
          }

        // Copy the slice origin information
        if (sliceNode->GetInteractionFlags() & sliceNode->GetInteractionFlagsModifier()
          & vtkDMMLSliceNode::XYZOriginFlag)
          {
          // Need to copy the SliceOrigin.
          double *xyzOrigin = sliceNode->GetXYZOrigin();
          sNode->SetXYZOrigin( xyzOrigin[0], xyzOrigin[1], xyzOrigin[2] );
          }

        // Forces the internal matrices to be updated which results
        // in this being modified so a Render can occur
        sNode->UpdateMatrices();
        }

      //
      // Some parameters and commands do not require the
      // orientations to match. These are handled here.
      //

      // Keeping zoom factor the same among all views (regardless of orientation)
      // is useful for reviewing a volume in multiple views.
      // Copy the field of view information. Use the new
      // prescribed x fov, aspect corrected y fov, and keep z fov
      // constant
      if (sliceNode->GetInteractionFlags() & sliceNode->GetInteractionFlagsModifier()
        & vtkDMMLSliceNode::FieldOfViewFlag)
      {
        sNode->SetFieldOfView(sliceNode->GetFieldOfView()[0],
          sliceNode->GetFieldOfView()[0]
          * sNode->GetFieldOfView()[1]
          / sNode->GetFieldOfView()[0],
          sNode->GetFieldOfView()[2]);
      }

      // need to manage prescribed spacing here as well?

      // Setting the orientation of the slice plane does not
      // require that the orientations initially match.
      if (sliceNode->GetInteractionFlags() & sliceNode->GetInteractionFlagsModifier()
        & vtkDMMLSliceNode::OrientationFlag)
        {
        // We could copy the orientation strings, but we really
        // want the slice to ras to match, so copy that
        sNode->GetSliceToRAS()->DeepCopy( sliceNode->GetSliceToRAS() );

        // Forces the internal matrices to be updated which results
        // in this being modified so a Render can occur
        sNode->UpdateMatrices();
        }

      // Setting field of view and orientation of slice views do not require the orientations to match

      // Order of operations are important: reset rotation, snap to volume axis, reset FOV

      if ((sliceNode->GetInteractionFlags() & sliceNode->GetInteractionFlagsModifier()
          & vtkDMMLSliceNode::ResetOrientationFlag))
        {
        sNode->SetOrientationToDefault();
        }

      if ((sliceNode->GetInteractionFlags() & sliceNode->GetInteractionFlagsModifier()
        & vtkDMMLSliceNode::RotateToBackgroundVolumePlaneFlag)
        && this->GetDMMLApplicationLogic()->GetSliceLogics())
        {
        vtkDMMLSliceLogic* logic = this->GetDMMLApplicationLogic()->GetSliceLogic(sNode);
        if (logic)
          {
          logic->RotateSliceToLowestVolumeAxes();
          }
        }

      // Resetting the field of view does not require the
      // orientations to match
      if ((sliceNode->GetInteractionFlags() & sliceNode->GetInteractionFlagsModifier()
        & vtkDMMLSliceNode::ResetFieldOfViewFlag)
        && this->GetDMMLApplicationLogic()->GetSliceLogics())
        {
        // need the logic for this slice (sNode)
        vtkDMMLSliceLogic* logic = this->GetDMMLApplicationLogic()->GetSliceLogic(sNode);
        if (logic)
          {
          logic->FitSliceToAll();
          sNode->UpdateMatrices();
          }
        }

      // Broadcasting the rotation from a ReformatWidget
      if (sliceNode->GetInteractionFlags() & sliceNode->GetInteractionFlagsModifier()
        & vtkDMMLSliceNode::MultiplanarReformatFlag)
        {
        this->BroadcastLastRotation(sliceNode,sNode);
        }

      // Setting the label outline mode
      if (sliceNode->GetInteractionFlags() & sliceNode->GetInteractionFlagsModifier()
        & vtkDMMLSliceNode::LabelOutlineFlag)
        {
        sNode->SetUseLabelOutline( sliceNode->GetUseLabelOutline() );
        }

      // Broadcasting the visibility of slice in 3D
      if (sliceNode->GetInteractionFlags() & sliceNode->GetInteractionFlagsModifier()
        & vtkDMMLSliceNode::SliceVisibleFlag)
        {
        std::string layoutName(sliceNode->GetLayoutName() ? sliceNode->GetLayoutName() : "");
        std::string lname(sNode->GetLayoutName() ? sNode->GetLayoutName() : "");
        if (layoutName.find("Compare") == 0)
          {
          // Compare view, only broadcast to compare views
          if (lname.find("Compare") == 0)
            {
            // Compare view, broadcast
            sNode->SetSliceVisible(sliceNode->GetSliceVisible());
            }
          }
        else
          {
          // Not a compare view, only broadcast to non compare views
          if (lname.find("Compare") != 0)
            {
            // not a Compare view, broadcast
            sNode->SetSliceVisible(sliceNode->GetSliceVisible());
            }
          }
        }

        // Setting the slice spacing
        if (sliceNode->GetInteractionFlags() & sliceNode->GetInteractionFlagsModifier()
          & vtkDMMLSliceNode::SliceSpacingFlag)
          {
          sNode->SetSliceSpacingMode( sliceNode->GetSliceSpacingMode() );
          sNode->SetPrescribedSliceSpacing( sliceNode->GetPrescribedSliceSpacing() );
          }
      //
      // End of the block for broadcasting parameters and commands
      // that do not require the orientation to match
      //
      }

    // Update SliceNodeInteractionStatus after MultiplanarReformat interaction
    this->UpdateSliceNodeInteractionStatus(sliceNode);
    this->BroadcastingEventsOff();
    }
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLinkLogic::BroadcastSliceCompositeNodeEvent(vtkDMMLSliceCompositeNode *sliceCompositeNode)
{
  // only broadcast a slice composite node event if we are not already actively
  // broadcasting events and we actively interacting with the node
  // std::cout << "BroadcastingEvents: " << this->GetBroadcastingEvents()
  //           << ", Interacting: " << sliceCompositeNode->GetInteracting()
  //           << std::endl;
  if (!sliceCompositeNode)
    {
    return;
    }
  if (!this->GetBroadcastingEvents() && sliceCompositeNode->GetInteracting())
    {
    this->BroadcastingEventsOn();

    int requiredViewGroup = -1;
    vtkDMMLSliceNode* sliceNode = vtkDMMLSliceLogic::GetSliceNode(sliceCompositeNode);
    if (sliceNode)
      {
      requiredViewGroup = sliceNode->GetViewGroup();
      }
    vtkDMMLSliceCompositeNode* cNode;
    vtkCollectionSimpleIterator it;
    vtkSmartPointer<vtkCollection> nodes;
    nodes.TakeReference(this->GetDMMLScene()->GetNodesByClass("vtkDMMLSliceCompositeNode"));

    for (nodes->InitTraversal(it);
        (cNode=vtkDMMLSliceCompositeNode::SafeDownCast(nodes->GetNextItemAsObject(it)));)
      {
      if (cNode == sliceCompositeNode || !cNode)
        {
        continue;
        }
      if (requiredViewGroup >= 0)
        {
        vtkDMMLSliceNode* sNode = vtkDMMLSliceLogic::GetSliceNode(cNode);
        if (sNode && sNode->GetViewGroup() != requiredViewGroup)
          {
          continue;
          }
        }
      // Foreground selection
      if (sliceCompositeNode->GetInteractionFlags() & sliceCompositeNode->GetInteractionFlagsModifier()
          & vtkDMMLSliceCompositeNode::ForegroundVolumeFlag)
        {
        //std::cerr << "Broadcasting Foreground Volume " << sliceCompositeNode->GetForegroundVolumeID() << std::endl;
        cNode->SetForegroundVolumeID(sliceCompositeNode->GetForegroundVolumeID());
        }

      // Background selection
      if (sliceCompositeNode->GetInteractionFlags() & sliceCompositeNode->GetInteractionFlagsModifier()
          & vtkDMMLSliceCompositeNode::BackgroundVolumeFlag)
        {
        cNode->SetBackgroundVolumeID(sliceCompositeNode->GetBackgroundVolumeID());
        }

      // Labelmap selection
      if (sliceCompositeNode->GetInteractionFlags() & sliceCompositeNode->GetInteractionFlagsModifier()
          & vtkDMMLSliceCompositeNode::LabelVolumeFlag)
        {
        cNode->SetLabelVolumeID(sliceCompositeNode->GetLabelVolumeID());
        }

      // Foreground opacity
      if (sliceCompositeNode->GetInteractionFlags() & sliceCompositeNode->GetInteractionFlagsModifier()
          & vtkDMMLSliceCompositeNode::ForegroundOpacityFlag)
        {
        cNode->SetForegroundOpacity(sliceCompositeNode->GetForegroundOpacity());
        }

      // Labelmap opacity
      if (sliceCompositeNode->GetInteractionFlags() & sliceCompositeNode->GetInteractionFlagsModifier()
          & vtkDMMLSliceCompositeNode::LabelOpacityFlag)
        {
        cNode->SetLabelOpacity(sliceCompositeNode->GetLabelOpacity());
        }

      }

    this->BroadcastingEventsOff();
    }
}

//----------------------------------------------------------------------------
vtkDMMLSliceCompositeNode* vtkDMMLSliceLinkLogic::GetCompositeNode(vtkDMMLSliceNode* sliceNode)
{
  vtkDMMLSliceCompositeNode* compositeNode = nullptr;

  vtkCollectionSimpleIterator it;
  vtkSmartPointer<vtkCollection> nodes;
  nodes.TakeReference(this->GetDMMLScene()->GetNodesByClass("vtkDMMLSliceCompositeNode"));

  for (nodes->InitTraversal(it);
      (compositeNode=vtkDMMLSliceCompositeNode::SafeDownCast(nodes->GetNextItemAsObject(it)));)
    {
    if (compositeNode->GetLayoutName()
        && !strcmp(compositeNode->GetLayoutName(), sliceNode->GetName()))
      {
      break;
      }

    compositeNode = nullptr;
    }

  return compositeNode;
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLinkLogic::BroadcastLastRotation(vtkDMMLSliceNode* sliceNode,
                                                  vtkDMMLSliceNode* sNode)
{
  SliceNodeStatusMap::iterator it = this->SliceNodeInteractionStatus.find(sliceNode->GetID());
  if (it == this->SliceNodeInteractionStatus.end())
    {
    return;
    }

  // Calculate the rotation applied to the sliceNode
  double cross[3], dot, rotation;
  vtkNew<vtkTransform> transform;
  vtkMatrix4x4* sNodeToRAS = sNode->GetSliceToRAS();
  double sliceNormal[3] = {sliceNode->GetSliceToRAS()->GetElement(0,2),
                           sliceNode->GetSliceToRAS()->GetElement(1,2),
                           sliceNode->GetSliceToRAS()->GetElement(2,2)};

  // Rotate the sliceNode to match the planeWidget normal
  vtkMath::Cross(it->second.LastNormal,sliceNormal, cross);
  dot = vtkMath::Dot(it->second.LastNormal, sliceNormal);
  // Clamp the dot product
  dot = (dot < -1.0) ? -1.0 : (dot > 1.0 ? 1.0 : dot);
  rotation = vtkMath::DegreesFromRadians(acos(dot));

  // Apply the rotation
  transform->PostMultiply();
  transform->SetMatrix(sNodeToRAS);
  transform->RotateWXYZ(rotation,cross);
  transform->GetMatrix(sNodeToRAS); // Update the changes

  sNode->UpdateMatrices();
}

//----------------------------------------------------------------------------
void vtkDMMLSliceLinkLogic::UpdateSliceNodeInteractionStatus(vtkDMMLSliceNode* sliceNode)
{
  SliceNodeStatusMap::iterator it = this->SliceNodeInteractionStatus.find(sliceNode->GetID());

  if (it != SliceNodeInteractionStatus.end())
    {
    it->second.LastNormal[0] = sliceNode->GetSliceToRAS()->GetElement(0,2);
    it->second.LastNormal[1] = sliceNode->GetSliceToRAS()->GetElement(1,2);
    it->second.LastNormal[2] = sliceNode->GetSliceToRAS()->GetElement(2,2);
    }
}

