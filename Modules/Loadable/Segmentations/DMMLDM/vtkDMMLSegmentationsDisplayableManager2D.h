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

#ifndef __vtkDMMLSegmentationsDisplayableManager2D_h
#define __vtkDMMLSegmentationsDisplayableManager2D_h

// DMMLDisplayableManager includes
#include "vtkDMMLAbstractSliceViewDisplayableManager.h"

#include "vtkCjyxSegmentationsModuleDMMLDisplayableManagerExport.h"

class vtkDMMLSegmentationDisplayNode;
class vtkStringArray;
class vtkDoubleArray;

/// \brief Displayable manager for showing segmentations in slice (2D) views.
///
/// Displays segmentations in slice viewers as labelmaps or contour lines
///
class VTK_CJYX_SEGMENTATIONS_MODULE_DMMLDISPLAYABLEMANAGER_EXPORT vtkDMMLSegmentationsDisplayableManager2D
  : public vtkDMMLAbstractSliceViewDisplayableManager
{

public:
  static vtkDMMLSegmentationsDisplayableManager2D* New();
  vtkTypeMacro(vtkDMMLSegmentationsDisplayableManager2D, vtkDMMLAbstractSliceViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Assemble and return info string to display in Data probe for a given viewer XYZ position.
  /// \return Invalid string by default, meaning no information to display.
  std::string GetDataProbeInfoStringForPosition(double xyz[3]) override;

  /// Get list of segments visible at selected display position.
  /// segmentValues is optional, if not nullptr then it returns value for each segment for fractional representations
  virtual void GetVisibleSegmentsForPosition(double ras[3], vtkDMMLSegmentationDisplayNode* displayNode,
    vtkStringArray* segmentIDs, vtkDoubleArray* segmentValues = nullptr);

protected:
  void UnobserveDMMLScene() override;
  void OnDMMLSceneNodeAdded(vtkDMMLNode* node) override;
  void OnDMMLSceneNodeRemoved(vtkDMMLNode* node) override;
  void ProcessDMMLNodesEvents(vtkObject* caller, unsigned long event, void* callData) override;

  /// Update Actors based on transforms in the scene
  void UpdateFromDMML() override;

  void OnDMMLSceneStartClose() override;
  void OnDMMLSceneEndClose() override;

  void OnDMMLSceneEndBatchProcess() override;

  /// Initialize the displayable manager based on its associated vtkDMMLSliceNode
  void Create() override;

protected:
  vtkDMMLSegmentationsDisplayableManager2D();
  ~vtkDMMLSegmentationsDisplayableManager2D() override;

private:
  vtkDMMLSegmentationsDisplayableManager2D(const vtkDMMLSegmentationsDisplayableManager2D&) = delete;
  void operator=(const vtkDMMLSegmentationsDisplayableManager2D&) = delete;

  class vtkInternal;
  vtkInternal * Internal;
  friend class vtkInternal;
};

#endif
