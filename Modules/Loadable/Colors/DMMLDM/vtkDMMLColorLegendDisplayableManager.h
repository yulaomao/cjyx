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

#ifndef __vtkDMMLColorLegendDisplayableManager_h
#define __vtkDMMLColorLegendDisplayableManager_h

// DMMLDisplayableManager includes
#include "vtkDMMLAbstractDisplayableManager.h"
#include "vtkCjyxColorsModuleDMMLDisplayableManagerExport.h"

class vtkDMMLColorLegendDisplayNode;
class vtkScalarBarWidget;
class vtkCjyxScalarBarActor;
class vtkDMMLScene;

/// \brief Displayable manager for color legends.
///
/// This displayable manager implements color legend display in both 2D and 3D views.
class VTK_CJYX_COLORS_MODULE_DMMLDISPLAYABLEMANAGER_EXPORT vtkDMMLColorLegendDisplayableManager :
  public vtkDMMLAbstractDisplayableManager
{
public:
  static vtkDMMLColorLegendDisplayableManager* New();
  vtkTypeMacro(vtkDMMLColorLegendDisplayableManager, vtkDMMLAbstractDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// \brief Get scalar bar actor
  /// \param dispNode - color legend display node
  /// \return scalar bar actor pointer
  vtkCjyxScalarBarActor* GetColorLegendActor(vtkDMMLColorLegendDisplayNode* dispNode) const;

protected:
  vtkDMMLColorLegendDisplayableManager();
  ~vtkDMMLColorLegendDisplayableManager() override;

  /// Initialize the displayable manager based on its associated
  /// vtkDMMLSliceNode
  void Create() override;

  /// Called from RequestRender method if UpdateFromDMMLRequested is true
  /// \sa RequestRender() SetUpdateFromDMMLRequested()
  void UpdateFromDMML() override;

  void SetDMMLSceneInternal(vtkDMMLScene* newScene) override;
  void UpdateFromDMMLScene() override;
  void UnobserveDMMLScene() override;
  void OnDMMLSceneNodeAdded(vtkDMMLNode* node) override;
  void OnDMMLSceneNodeRemoved(vtkDMMLNode* node) override;
  void ProcessDMMLNodesEvents(vtkObject *caller,
                                      unsigned long event,
                                      void *callData) override;

  /// Called when the SliceNode or Three3DViewNode are modified. May cause ColorLegend to remap its position on screen.
  void OnDMMLDisplayableNodeModifiedEvent(vtkObject* caller) override;

  /// Method to perform additional initialization
  void AdditionalInitializeStep() override;

private:
  vtkDMMLColorLegendDisplayableManager(const vtkDMMLColorLegendDisplayableManager&) = delete;
  void operator=(const vtkDMMLColorLegendDisplayableManager&) = delete;

  class vtkInternal;
  vtkInternal* Internal;
};

#endif
