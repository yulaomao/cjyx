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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __vtkDMMLModelSliceDisplayableManager_h
#define __vtkDMMLModelSliceDisplayableManager_h

// DMMLDisplayableManager includes
#include "vtkDMMLAbstractSliceViewDisplayableManager.h"
#include "vtkDMMLDisplayableManagerExport.h"

class vtkDMMLDisplayableNode;

/// \brief Displayable manager for slice (2D) views.
///
/// Responsible for any display on Slice views that is not the slice themselves
/// nor the annotations.
/// Currently support only glyph display for Diffusion Tensor volumes.
class VTK_DMML_DISPLAYABLEMANAGER_EXPORT vtkDMMLModelSliceDisplayableManager
  : public vtkDMMLAbstractSliceViewDisplayableManager
{

public:
  static vtkDMMLModelSliceDisplayableManager* New();
  vtkTypeMacro(vtkDMMLModelSliceDisplayableManager,
                       vtkDMMLAbstractSliceViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // DisplayableNode handling customizations
  void AddDisplayableNode(vtkDMMLDisplayableNode* displayableNode);
  void RemoveDisplayableNode(vtkDMMLDisplayableNode* displayableNode);

protected:

  vtkDMMLModelSliceDisplayableManager();
  ~vtkDMMLModelSliceDisplayableManager() override;

  void UnobserveDMMLScene() override;
  void OnDMMLSceneNodeAdded(vtkDMMLNode* node) override;
  void OnDMMLSceneNodeRemoved(vtkDMMLNode* node) override;
  void ProcessDMMLNodesEvents(vtkObject* caller, unsigned long event, void* callData) override;

  void UpdateFromDMML() override;
  void OnDMMLSceneStartClose() override;
  void OnDMMLSceneEndClose() override;
  void OnDMMLSceneEndBatchProcess() override;

  /// Initialize the displayable manager based on its associated
  /// vtkDMMLSliceNode
  void Create() override;
  int AddingDisplayableNode;

private:

  vtkDMMLModelSliceDisplayableManager(const vtkDMMLModelSliceDisplayableManager&) = delete;
  void operator=(const vtkDMMLModelSliceDisplayableManager&) = delete;

  class vtkInternal;
  vtkInternal * Internal;
  friend class vtkInternal;

};

#endif
