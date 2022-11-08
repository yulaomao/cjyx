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

  This file was originally developed by Andras Lasso (PerkLab, Queen's University).

==============================================================================*/

#ifndef __vtkDMMLCrosshairDisplayableManager3D_h
#define __vtkDMMLCrosshairDisplayableManager3D_h

// DMMLDisplayableManager includes
#include "vtkDMMLAbstractThreeDViewDisplayableManager.h"
#include "vtkDMMLDisplayableManagerExport.h"

/// \brief Displayable manager for the crosshair on 3D views
///
/// Responsible for any display of the crosshair on 3D views.
class VTK_DMML_DISPLAYABLEMANAGER_EXPORT vtkDMMLCrosshairDisplayableManager3D :
  public vtkDMMLAbstractThreeDViewDisplayableManager
{
public:
  static vtkDMMLCrosshairDisplayableManager3D* New();
  vtkTypeMacro(vtkDMMLCrosshairDisplayableManager3D,
    vtkDMMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:
  vtkDMMLCrosshairDisplayableManager3D();
  ~vtkDMMLCrosshairDisplayableManager3D() override;

  /// Initialize the displayable manager based on its associated
  /// vtkDMMLSliceNode
  void Create() override;

  /// Method to perform additional initialization
  void AdditionalInitializeStep() override;

private:
  vtkDMMLCrosshairDisplayableManager3D(const vtkDMMLCrosshairDisplayableManager3D&) = delete;
  void operator=(const vtkDMMLCrosshairDisplayableManager3D&) = delete;

  void UnobserveDMMLScene() override;
  void ObserveDMMLScene() override;
  void UpdateFromDMMLScene() override;
  void OnDMMLNodeModified(vtkDMMLNode* node) override;

  class vtkInternal;
  vtkInternal * Internal;
};

#endif
