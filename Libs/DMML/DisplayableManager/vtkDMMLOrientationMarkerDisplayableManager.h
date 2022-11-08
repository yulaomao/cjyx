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

  This file was originally developed by Andras Lasso, PerkLab, Queen's University.

==============================================================================*/

#ifndef __vtkDMMLOrientationMarkerDisplayableManager_h
#define __vtkDMMLOrientationMarkerDisplayableManager_h

// DMMLDisplayableManager includes
#include "vtkDMMLAbstractDisplayableManager.h"
#include "vtkDMMLDisplayableManagerExport.h"

// DMML includes
class vtkDMMLCameraNode;

/// \brief Displayable manager that displays orientation marker in a slice or 3D view
class VTK_DMML_DISPLAYABLEMANAGER_EXPORT vtkDMMLOrientationMarkerDisplayableManager
  : public vtkDMMLAbstractDisplayableManager
{
  friend class vtkRendererUpdateObserver;

public:
  static vtkDMMLOrientationMarkerDisplayableManager* New();
  vtkTypeMacro(vtkDMMLOrientationMarkerDisplayableManager,vtkDMMLAbstractDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:

  vtkDMMLOrientationMarkerDisplayableManager();
  ~vtkDMMLOrientationMarkerDisplayableManager() override;

  /// Observe the View node and initialize the renderer accordingly.
  void Create() override;

  /// Called each time the view node is modified.
  /// Internally update the renderer from the view node.
  /// \sa UpdateFromDMMLViewNode()
  void OnDMMLDisplayableNodeModifiedEvent(vtkObject* caller) override;

  /// Update the renderer from the view node properties.
  void UpdateFromViewNode();

  /// Update the renderer based on the master renderer (the one that the orientation marker follows)
  void UpdateFromRenderer();

private:

  vtkDMMLOrientationMarkerDisplayableManager(const vtkDMMLOrientationMarkerDisplayableManager&) = delete;
  void operator=(const vtkDMMLOrientationMarkerDisplayableManager&) = delete;

  class vtkInternal;
  vtkInternal * Internal;
};

#endif
