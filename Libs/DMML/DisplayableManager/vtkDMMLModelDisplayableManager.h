/*=========================================================================

  Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/

#ifndef __vtkDMMLModelDisplayableManager_h
#define __vtkDMMLModelDisplayableManager_h

// DMMLDisplayableManager includes
#include "vtkDMMLAbstractThreeDViewDisplayableManager.h"
#include "vtkDMMLDisplayableManagerExport.h"

// DMML includes
#include <vtkDMMLModelNode.h>
class vtkDMMLClipModelsNode;
class vtkDMMLDisplayNode;
class vtkDMMLDisplayableNode;
class vtkDMMLTransformNode;

// VTK includes
#include "vtkRenderWindow.h"
class vtkActor;
class vtkAlgorithm;
class vtkCellPicker;
class vtkLookupTable;
class vtkMatrix4x4;
class vtkPlane;
class vtkPointPicker;
class vtkProp3D;
class vtkPropPicker;
class vtkWorldPointPicker;

/// \brief Manage display nodes with polydata in 3D views.
///
/// Any display node in the scene that contains a valid output polydata is
/// represented into the view renderer using configured synchronized vtkActors
/// and vtkMappers.
/// Note that the display nodes must be of type vtkDMMLModelDisplayNode
/// (to have an output polydata) but the displayable nodes don't necessarily
/// have to be of type vtkDMMLModelNode.
class VTK_DMML_DISPLAYABLEMANAGER_EXPORT vtkDMMLModelDisplayableManager
  : public vtkDMMLAbstractThreeDViewDisplayableManager
{
public:
  static vtkDMMLModelDisplayableManager* New();
  vtkTypeMacro(vtkDMMLModelDisplayableManager,vtkDMMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Get/Set the ClipModels Node
  vtkDMMLClipModelsNode* GetClipModelsNode();
  void SetClipModelsNode(vtkDMMLClipModelsNode *snode);

  /// Return the current model actor corresponding to a give DMML ID
  vtkProp3D *GetActorByID(const char *id);

  /// Return the current node ID corresponding to a given vtkProp3D
  const char *GetIDByActor(vtkProp3D *actor);

  /// Get world point picker
  vtkWorldPointPicker* GetWorldPointPicker();

  /// Get property picker
  vtkPropPicker* GetPropPicker();

  /// Get cell picker
  vtkCellPicker* GetCellPicker();

  /// Get point picker
  vtkPointPicker* GetPointPicker();

  /// Convert an x/y location to a DMML node, 3D RAS point, point ID, cell ID,
  /// as appropriate depending what's found under the xy.
  int Pick(int x, int y);

  /// Convert a RAS location to a DMML node, point ID, cell ID,
  /// as appropriate depending what's found under the position.
  int Pick3D(double ras[3]) override;

  /// Get tolerance for Pick() method. It will call vtkCellPicker.GetTolerance()
  double GetPickTolerance();
  /// Set tolerance for Pick() method. It will call vtkCellPicker.SetTolerance()
  void SetPickTolerance(double tolerance);

  /// Get the DMML ID of the picked node, returns empty string if no pick
  const char* GetPickedNodeID() override;

  /// Get the picked RAS point, returns 0,0,0 if no pick
  double* GetPickedRAS();
  /// Set the picked RAS point, returns 0,0,0 if no pick
  void SetPickedRAS(double* newPickedRAS);

  /// Get the picked cell id, returns -1 if no pick
  vtkIdType GetPickedCellID();
  /// Set the picked cell id, returns -1 if no pick
  void SetPickedCellID(vtkIdType newCellID);

  /// Get the picked point id, returns -1 if no pick
  vtkIdType GetPickedPointID();
  /// Set the picked point id, returns -1 if no pick
  void SetPickedPointID(vtkIdType newPointID);

  void SetClipPlaneFromMatrix(vtkMatrix4x4 *sliceMatrix,
                             int planeDirection,
                             vtkPlane *plane);

  /// Return true if the node can be represented as a model
  bool IsModelDisplayable(vtkDMMLDisplayableNode* node)const;
  /// Return true if the display node is a model
  bool IsModelDisplayable(vtkDMMLDisplayNode* node)const;

  /// Helper function for determining what type of scalar is active.
  /// \return True if attribute location in display node is vtkAssignAttribute::CELL_DATA
  ///   or active cell scalar name in the model node is vtkDataSetAttributes::SCALARS.
  ///   False otherwise.
  static bool IsCellScalarsActive(vtkDMMLDisplayNode* displayNode, vtkDMMLModelNode* model = nullptr);

protected:
  int ActiveInteractionModes() override;

  void UnobserveDMMLScene() override;

  void OnDMMLSceneStartClose() override;
  void OnDMMLSceneEndClose() override;
  void UpdateFromDMMLScene() override;
  void OnDMMLSceneNodeAdded(vtkDMMLNode* node) override;
  void OnDMMLSceneNodeRemoved(vtkDMMLNode* node) override;

  void OnInteractorStyleEvent(int eventId) override;
  void ProcessDMMLNodesEvents(vtkObject *caller, unsigned long event, void *callData) override;

  /// Returns true if something visible in modelNode has changed and would
  /// require a refresh.
  bool OnDMMLDisplayableModelNodeModifiedEvent(vtkDMMLDisplayableNode* modelNode);

  /// Updates Actors based on models in the scene
  void UpdateFromDMML() override;

  void RemoveDMMLObservers() override;

  void RemoveModelProps();
  void RemoveModelObservers(int clearCache);
  void RemoveDisplayable(vtkDMMLDisplayableNode* model);
  void RemoveDisplayableNodeObservers(vtkDMMLDisplayableNode* model);

  void UpdateModelsFromDMML();
  void UpdateModel(vtkDMMLDisplayableNode* model);
  void UpdateModelMesh(vtkDMMLDisplayableNode* model);
  void UpdateModifiedModel(vtkDMMLDisplayableNode* model);

  void SetModelDisplayProperty(vtkDMMLDisplayableNode* model);
  int GetDisplayedModelsVisibility(vtkDMMLDisplayNode* displayNode);

  const char* GetActiveScalarName(vtkDMMLDisplayNode* displayNode,
                                  vtkDMMLModelNode* model = nullptr);

  /// Returns not null if modified
  int UpdateClipSlicesFromDMML();
  vtkAlgorithm *CreateTransformedClipper(vtkDMMLTransformNode *tnode,
                                         vtkDMMLModelNode::MeshTypeHint type);

  void RemoveDisplayedID(std::string &id);

protected:
  vtkDMMLModelDisplayableManager();
  ~vtkDMMLModelDisplayableManager() override;

  friend class vtkDMMLThreeDViewInteractorStyle; // Access to RequestRender();

private:
  vtkDMMLModelDisplayableManager(const vtkDMMLModelDisplayableManager&) = delete;
  void operator=(const vtkDMMLModelDisplayableManager&) = delete;

  class vtkInternal;
  vtkInternal* Internal;
};

#endif
