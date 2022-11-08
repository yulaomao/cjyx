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

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

==============================================================================*/

#ifndef __vtkDMMLTransformsDisplayableManager3D_h
#define __vtkDMMLTransformsDisplayableManager3D_h

// DMMLDisplayableManager includes
#include "vtkDMMLAbstractThreeDViewDisplayableManager.h"

#include "vtkCjyxTransformsModuleDMMLDisplayableManagerExport.h"

/// \brief Display transforms in 3D views
///
/// Displays transforms in 3D viewers as glyphs, deformed grid, or
/// contour surfaces
///
class VTK_CJYX_TRANSFORMS_MODULE_DMMLDISPLAYABLEMANAGER_EXPORT vtkDMMLTransformsDisplayableManager3D
  : public vtkDMMLAbstractThreeDViewDisplayableManager
{
public:

  static vtkDMMLTransformsDisplayableManager3D* New();
  vtkTypeMacro(vtkDMMLTransformsDisplayableManager3D,vtkDMMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:

  vtkDMMLTransformsDisplayableManager3D();
  ~vtkDMMLTransformsDisplayableManager3D() override;

  void UnobserveDMMLScene() override;
  void OnDMMLSceneNodeAdded(vtkDMMLNode* node) override;
  void OnDMMLSceneNodeRemoved(vtkDMMLNode* node) override;
  void ProcessDMMLNodesEvents(vtkObject* caller, unsigned long event, void* callData) override;

  /// Update Actors based on transforms in the scene
  void UpdateFromDMML() override;

  void OnDMMLSceneStartClose() override;
  void OnDMMLSceneEndClose() override;

  void OnDMMLSceneEndBatchProcess() override;

  /// Initialize the displayable manager
  void Create() override;

private:

  vtkDMMLTransformsDisplayableManager3D(const vtkDMMLTransformsDisplayableManager3D&) = delete;
  void operator=(const vtkDMMLTransformsDisplayableManager3D&) = delete;

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal;
};

#endif
