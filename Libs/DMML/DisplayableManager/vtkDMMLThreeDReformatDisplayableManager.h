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

  This file was originally developed by Michael Jeulin-L, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __vtkDMMLThreeDReformatDisplayableManager_h
#define __vtkDMMLThreeDReformatDisplayableManager_h

// DMMLDisplayableManager includes
#include "vtkDMMLAbstractThreeDViewDisplayableManager.h"
#include "vtkDMMLDisplayableManagerExport.h"

/// \brief Displayable manager for ImplicitPlaneWidget2 in 3D views.
///
/// Responsible for any display based on the reformat widgets.
class VTK_DMML_DISPLAYABLEMANAGER_EXPORT vtkDMMLThreeDReformatDisplayableManager :
  public vtkDMMLAbstractThreeDViewDisplayableManager
{

public:
  static vtkDMMLThreeDReformatDisplayableManager* New();
  vtkTypeMacro(vtkDMMLThreeDReformatDisplayableManager,
                       vtkDMMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:
  vtkDMMLThreeDReformatDisplayableManager();
  ~vtkDMMLThreeDReformatDisplayableManager() override;

  /// Initialize the displayable manager based on its associated
  /// vtkDMMLSliceNode
  void Create() override;

  /// WidgetCallback is a static function to relay modified events from the Logic
  void ProcessWidgetsEvents(vtkObject *caller, unsigned long event, void *callData) override;

  void UnobserveDMMLScene() override;
  void UpdateFromDMMLScene() override;
  void OnDMMLSceneNodeAdded(vtkDMMLNode* node) override;
  void OnDMMLSceneNodeRemoved(vtkDMMLNode* node) override;
  void OnDMMLNodeModified(vtkDMMLNode* node) override;

private:
  vtkDMMLThreeDReformatDisplayableManager(const vtkDMMLThreeDReformatDisplayableManager&) = delete;
  void operator=(const vtkDMMLThreeDReformatDisplayableManager&) = delete;

  class vtkInternal;
  vtkInternal* Internal;
};

#endif // vtkDMMLThreeDReformatDisplayableManager_h
