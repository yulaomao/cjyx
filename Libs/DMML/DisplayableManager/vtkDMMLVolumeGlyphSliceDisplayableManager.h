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

#ifndef __vtkDMMLVolumeGlyphSliceDisplayableManager_h
#define __vtkDMMLVolumeGlyphSliceDisplayableManager_h

// DMMLDisplayableManager includes
#include "vtkDMMLAbstractSliceViewDisplayableManager.h"
#include "vtkDMMLDisplayableManagerExport.h"

/// \brief Displayable manager for slice (2D) views.
///
/// Responsible for any display on Slice views that is not the slice themselves
/// nor the annotations.
/// Currently support only glyph display for Diffusion Tensor volumes.
class VTK_DMML_DISPLAYABLEMANAGER_EXPORT vtkDMMLVolumeGlyphSliceDisplayableManager
  : public vtkDMMLAbstractSliceViewDisplayableManager
{
public:
  static vtkDMMLVolumeGlyphSliceDisplayableManager* New();
  vtkTypeMacro(vtkDMMLVolumeGlyphSliceDisplayableManager,
                       vtkDMMLAbstractSliceViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:

  vtkDMMLVolumeGlyphSliceDisplayableManager();
  ~vtkDMMLVolumeGlyphSliceDisplayableManager() override;

  void UnobserveDMMLScene() override;
  void UpdateFromDMMLScene() override;
  void ProcessDMMLNodesEvents(vtkObject* caller, unsigned long event, void* callData) override;
  void OnDMMLSceneStartClose() override;

  /// Initialize the displayable manager based on its associated
  /// vtkDMMLSliceNode
  void Create() override;

private:

  vtkDMMLVolumeGlyphSliceDisplayableManager(const vtkDMMLVolumeGlyphSliceDisplayableManager&) = delete;
  void operator=(const vtkDMMLVolumeGlyphSliceDisplayableManager&) = delete;

  class vtkInternal;
  vtkInternal * Internal;
  friend class vtkInternal;
};

#endif
