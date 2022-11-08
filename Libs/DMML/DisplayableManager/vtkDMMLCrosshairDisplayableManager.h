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

#ifndef __vtkDMMLCrosshairDisplayableManager_h
#define __vtkDMMLCrosshairDisplayableManager_h

// DMMLDisplayableManager includes
#include "vtkDMMLAbstractSliceViewDisplayableManager.h"
#include "vtkDMMLDisplayableManagerExport.h"

class vtkDMMLCrosshairNode;
class vtkDMMLScene;
class vtkDMMLSliceIntersectionWidget;

/// \brief Displayable manager for the crosshair on slice (2D) views
///
/// Responsible for any display of the crosshair on Slice views.
class VTK_DMML_DISPLAYABLEMANAGER_EXPORT vtkDMMLCrosshairDisplayableManager :
  public vtkDMMLAbstractSliceViewDisplayableManager
{
public:
  static vtkDMMLCrosshairDisplayableManager* New();
  vtkTypeMacro(vtkDMMLCrosshairDisplayableManager,
                       vtkDMMLAbstractSliceViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Utility functions (used by 2D and 3D crosshair displayable managers)
  static vtkDMMLCrosshairNode* FindCrosshairNode(vtkDMMLScene* scene);

  bool CanProcessInteractionEvent(vtkDMMLInteractionEventData* eventData, double &closestDistance2) override;
  bool ProcessInteractionEvent(vtkDMMLInteractionEventData* eventData) override;

  void SetActionsEnabled(int actions);
  int GetActionsEnabled();

  vtkDMMLSliceIntersectionWidget* GetSliceIntersectionWidget();

  /// Displayable manager returns ID of the mouse cursor shape that should be displayed
  virtual int GetMouseCursor();

  // Called to notify the displayable manager that it has got or lost the focus.
  void SetHasFocus(bool hasFocus, vtkDMMLInteractionEventData* eventData) override;

protected:
  vtkDMMLCrosshairDisplayableManager();
  ~vtkDMMLCrosshairDisplayableManager() override;

  /// Initialize the displayable manager based on its associated
  /// vtkDMMLSliceNode
  void Create() override;

  /// Called when the SliceNode is modified. May cause Crosshair to remap its position on screen.
  void OnDMMLSliceNodeModifiedEvent() override;

  /// Method to perform additional initialization
  void AdditionalInitializeStep() override;

private:
  vtkDMMLCrosshairDisplayableManager(const vtkDMMLCrosshairDisplayableManager&) = delete;
  void operator=(const vtkDMMLCrosshairDisplayableManager&) = delete;

  void UnobserveDMMLScene() override;
  void ObserveDMMLScene() override;
  void UpdateFromDMMLScene() override;
  void OnDMMLNodeModified(vtkDMMLNode* node) override;

  class vtkInternal;
  vtkInternal * Internal;
};

#endif
