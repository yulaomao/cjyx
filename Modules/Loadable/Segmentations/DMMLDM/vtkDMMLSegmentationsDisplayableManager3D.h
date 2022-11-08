/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __vtkDMMLSegmentationsDisplayableManager3D_h
#define __vtkDMMLSegmentationsDisplayableManager3D_h

// DMMLDisplayableManager includes
#include "vtkDMMLAbstractThreeDViewDisplayableManager.h"

#include "vtkCjyxSegmentationsModuleDMMLDisplayableManagerExport.h"

/// \brief Display segmentations in 3D views
///
/// Displays poly data representations of segmentations in 3D viewers
/// If master representation is a poly data then show master representation.
/// Otherwise show first poly data representation if any.
/// Otherwise try converting to closed surface representation
///
class VTK_CJYX_SEGMENTATIONS_MODULE_DMMLDISPLAYABLEMANAGER_EXPORT vtkDMMLSegmentationsDisplayableManager3D
  : public vtkDMMLAbstractThreeDViewDisplayableManager
{
public:

  static vtkDMMLSegmentationsDisplayableManager3D* New();
  vtkTypeMacro(vtkDMMLSegmentationsDisplayableManager3D,vtkDMMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Find display node managed by the displayable manager at a specified world RAS position.
  /// \return Non-zero in case a node is found at the position, 0 otherwise
  int Pick3D(double ras[3]) override;

  /// Get the DMML ID of the picked node, returns empty string if no pick
  const char* GetPickedNodeID() override;

  /// Get the ID of the picked segment, returns empty string if no pick
  virtual const char* GetPickedSegmentID();

protected:

  vtkDMMLSegmentationsDisplayableManager3D();
  ~vtkDMMLSegmentationsDisplayableManager3D() override;

  void UnobserveDMMLScene() override;
  void OnDMMLSceneNodeAdded(vtkDMMLNode* node) override;
  void OnDMMLSceneNodeRemoved(vtkDMMLNode* node) override;
  void ProcessDMMLNodesEvents(vtkObject* caller, unsigned long event, void* callData) override;

  /// Update actors based on segmentations in the scene
  void UpdateFromDMML() override;

  void OnDMMLSceneStartClose() override;
  void OnDMMLSceneEndClose() override;

  void OnDMMLSceneEndBatchProcess() override;

  /// Initialize the displayable manager
  void Create() override;

private:

  vtkDMMLSegmentationsDisplayableManager3D(const vtkDMMLSegmentationsDisplayableManager3D&) = delete;
  void operator=(const vtkDMMLSegmentationsDisplayableManager3D&) = delete;

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal;
};

#endif
