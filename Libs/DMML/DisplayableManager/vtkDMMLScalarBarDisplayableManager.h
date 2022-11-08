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

==============================================================================*/

#ifndef __vtkDMMLScalarBarDisplayableManager_h
#define __vtkDMMLScalarBarDisplayableManager_h

// DMMLDisplayableManager includes
#include "vtkDMMLAbstractSliceViewDisplayableManager.h"
#include "vtkDMMLDisplayableManagerExport.h"

class vtkDMMLScalarBarNode;
class vtkDMMLScene;
class vtkDMMLWindowLevelWidget;

/// \brief Displayable manager for window/level adjustment of volumes.
///
/// This displayable manager only manages window/level adjustment events, does not display
/// "scalar bar" (color legend). Its name was given because orinally it was intended for
/// displaying color legend as well, but later a dedicated displayable manager was added for that purpose
/// (that can be used in any view types, for any displayable node types).
class VTK_DMML_DISPLAYABLEMANAGER_EXPORT vtkDMMLScalarBarDisplayableManager :
  public vtkDMMLAbstractSliceViewDisplayableManager
{
public:
  static vtkDMMLScalarBarDisplayableManager* New();
  vtkTypeMacro(vtkDMMLScalarBarDisplayableManager,
                       vtkDMMLAbstractSliceViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  bool CanProcessInteractionEvent(vtkDMMLInteractionEventData* eventData, double &closestDistance2) override;
  bool ProcessInteractionEvent(vtkDMMLInteractionEventData* eventData) override;

  void SetAdjustForegroundWindowLevelEnabled(bool enabled);
  bool GetAdjustForegroundWindowLevelEnabled();
  void SetAdjustBackgroundWindowLevelEnabled(bool enabled);
  bool GetAdjustBackgroundWindowLevelEnabled();

  vtkDMMLWindowLevelWidget* GetWindowLevelWidget();

protected:
  vtkDMMLScalarBarDisplayableManager();
  ~vtkDMMLScalarBarDisplayableManager() override;

  /// Initialize the displayable manager based on its associated
  /// vtkDMMLSliceNode
  void Create() override;

  /// Called when the SliceNode is modified. May cause ScalarBar to remap its position on screen.
  void OnDMMLSliceNodeModifiedEvent() override;

  /// Method to perform additional initialization
  void AdditionalInitializeStep() override;

private:
  vtkDMMLScalarBarDisplayableManager(const vtkDMMLScalarBarDisplayableManager&) = delete;
  void operator=(const vtkDMMLScalarBarDisplayableManager&) = delete;

  void UnobserveDMMLScene() override;
  void UpdateFromDMMLScene() override;

  class vtkInternal;
  vtkInternal * Internal;
};

#endif
