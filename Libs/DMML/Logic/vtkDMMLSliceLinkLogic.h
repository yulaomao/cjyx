/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLSliceLinkLogic.h,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

///  vtkDMMLSliceLinkLogic - cjyx logic class for linked slice manipulation
///
/// This class manages the logic associated with linking the controls
/// of multiple slice and slice composite nodes. It listens to the
/// DMML scene for new slice and slice composite nodes and observes
/// these nodes for ModifiedEvents. When notified of a ModifiedEvent
/// on a slice or slice composite node, this logic class will
/// propagate state to other slice and slice composite nodes. A
/// critical component of the design is that slice and slice composite
/// nodes "know" when they are be changed interactively verses when
/// their state is being updated programmatically.

#ifndef __vtkDMMLSliceLinkLogic_h
#define __vtkDMMLSliceLinkLogic_h

// DMMLLogic includes
#include "vtkDMMLAbstractLogic.h"

// STD includes
#include <vector>

class vtkDMMLSliceNode;
class vtkDMMLSliceCompositeNode;

class VTK_DMML_LOGIC_EXPORT vtkDMMLSliceLinkLogic : public vtkDMMLAbstractLogic
{
public:

  /// The Usual VTK class functions
  static vtkDMMLSliceLinkLogic *New();
  vtkTypeMacro(vtkDMMLSliceLinkLogic,vtkDMMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:

  vtkDMMLSliceLinkLogic();
  ~vtkDMMLSliceLinkLogic() override;

  // On a change in scene, we need to manage the observations.
  void SetDMMLSceneInternal(vtkDMMLScene * newScene) override;

  void OnDMMLSceneNodeAdded(vtkDMMLNode* node) override;
  void OnDMMLSceneNodeRemoved(vtkDMMLNode* node) override;
  void OnDMMLNodeModified(vtkDMMLNode* node) override;
  void OnDMMLSceneStartBatchProcess() override;
  void OnDMMLSceneEndBatchProcess() override;
  void OnDMMLSceneStartImport() override;
  void OnDMMLSceneEndImport() override;
  void OnDMMLSceneStartRestore() override;
  void OnDMMLSceneEndRestore() override;

  // Used internally to control whether we are in the process of
  // broadcasting events. PIMPL it?
  void BroadcastingEventsOn();
  void BroadcastingEventsOff();
  int GetBroadcastingEvents();

  /// Broadcast a slice node to other slice nodes.
  void BroadcastSliceNodeEvent(vtkDMMLSliceNode *sliceNode);

  /// Broadcast a slice composite node to other slice composite nodes
  void BroadcastSliceCompositeNodeEvent(vtkDMMLSliceCompositeNode *compositeNode);

  /// Returns true if orientation of the slices match. Slice position and scaling is ignored.
  bool IsOrientationMatching(vtkDMMLSliceNode *sliceNode1, vtkDMMLSliceNode *sliceNode2, double comparisonTolerance = 0.001);

private:

  vtkDMMLSliceLinkLogic(const vtkDMMLSliceLinkLogic&) = delete;
  void operator=(const vtkDMMLSliceLinkLogic&) = delete;

  vtkDMMLSliceCompositeNode* GetCompositeNode(vtkDMMLSliceNode*);
  void BroadcastLastRotation(vtkDMMLSliceNode*, vtkDMMLSliceNode*);
  void UpdateSliceNodeInteractionStatus(vtkDMMLSliceNode*);

  // Counter on nested requests for Broadcasting events. Counter is
  // used as scene restores and scene view restores issue several
  // nested events and we want to block from the first Start to the
  // last End event (StartBatchProcess, StartImport, StartRestore).
  int BroadcastingEvents;

  struct SliceNodeInfos
    {
    SliceNodeInfos(int interacting) : Interacting(interacting) {}
    double LastNormal[3];
    int Interacting;
    };

  typedef std::map<std::string, SliceNodeInfos> SliceNodeStatusMap;
  SliceNodeStatusMap SliceNodeInteractionStatus;

};

#endif
