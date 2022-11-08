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

#ifndef __vtkDMMLLinearTransformsDisplayableManager3D_h
#define __vtkDMMLLinearTransformsDisplayableManager3D_h

// DMMLDisplayableManager includes
#include "vtkDMMLAbstractThreeDViewDisplayableManager.h"

#include "vtkCjyxTransformsModuleDMMLDisplayableManagerExport.h"

class vtkAbstractWidget;
class vtkDMMLTransformDisplayNode;


/// \brief Display transforms in 3D views
///
/// Displays transforms in 3D viewers as glyphs, deformed grid, or
/// contour surfaces
///
class VTK_CJYX_TRANSFORMS_MODULE_DMMLDISPLAYABLEMANAGER_EXPORT vtkDMMLLinearTransformsDisplayableManager3D
  : public vtkDMMLAbstractThreeDViewDisplayableManager
{
public:

  static vtkDMMLLinearTransformsDisplayableManager3D* New();
  vtkTypeMacro(vtkDMMLLinearTransformsDisplayableManager3D,vtkDMMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// \internal
  /// For testing purposes only:
  /// Return the widget associated with the given transform, if any.
  vtkAbstractWidget* GetWidget(vtkDMMLTransformDisplayNode* displayNode);

protected:

  vtkDMMLLinearTransformsDisplayableManager3D();
  ~vtkDMMLLinearTransformsDisplayableManager3D() override;

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

  void ProcessWidgetsEvents(vtkObject* caller, unsigned long event, void* callData) override;

private:

  vtkDMMLLinearTransformsDisplayableManager3D(const vtkDMMLLinearTransformsDisplayableManager3D&) = delete;
  void operator=(const vtkDMMLLinearTransformsDisplayableManager3D&) = delete;

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal;
};

#endif
