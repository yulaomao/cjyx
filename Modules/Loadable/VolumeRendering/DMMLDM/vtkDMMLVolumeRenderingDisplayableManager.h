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

#ifndef __vtkDMMLVolumeRenderingDisplayableManager_h
#define __vtkDMMLVolumeRenderingDisplayableManager_h

// VolumeRendering includes
#include "vtkCjyxVolumeRenderingModuleDMMLDisplayableManagerExport.h"

// DMML DisplayableManager includes
#include <vtkDMMLAbstractThreeDViewDisplayableManager.h>

class vtkCjyxVolumeRenderingLogic;
class vtkDMMLVolumeNode;
class vtkVolumeMapper;
class vtkVolume;

#define VTKIS_VOLUME_PROPS 100

/// \ingroup Cjyx_QtModules_VolumeRendering
class VTK_CJYX_VOLUMERENDERING_MODULE_DMMLDISPLAYABLEMANAGER_EXPORT vtkDMMLVolumeRenderingDisplayableManager
  : public vtkDMMLAbstractThreeDViewDisplayableManager
{
public:
  static vtkDMMLVolumeRenderingDisplayableManager *New();
  vtkTypeMacro(vtkDMMLVolumeRenderingDisplayableManager, vtkDMMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void OnDMMLSceneStartClose() override;
  void OnDMMLSceneEndClose() override;
  void OnDMMLSceneEndImport() override;
  void OnDMMLSceneEndRestore() override;
  void OnDMMLSceneEndBatchProcess() override;
  void OnDMMLSceneNodeAdded(vtkDMMLNode* node) override;
  void OnDMMLSceneNodeRemoved(vtkDMMLNode* node) override;
  void UnobserveDMMLScene() override;

  /// Update actors based on volumes in the scene
  void UpdateFromDMML() override;

  /// Utility functions mainly used for testing
  vtkVolumeMapper* GetVolumeMapper(vtkDMMLVolumeNode* volumeNode);
  vtkVolume* GetVolumeActor(vtkDMMLVolumeNode* volumeNode);

  /// Find display node managed by the displayable manager at a specified world RAS position.
  /// \return Non-zero in case a node is found at the position, 0 otherwise
  int Pick3D(double ras[3]) override;

  /// Get the DMML ID of the picked node, returns empty string if no pick
  const char* GetPickedNodeID() override;

public:
  static int DefaultGPUMemorySize;

protected:
  vtkDMMLVolumeRenderingDisplayableManager();
  ~vtkDMMLVolumeRenderingDisplayableManager() override;

  /// Initialize the displayable manager
  void Create() override;

  /// Observe graphical resources created event
  void ObserveGraphicalResourcesCreatedEvent();

  int ActiveInteractionModes() override;

  void ProcessDMMLNodesEvents(vtkObject * caller, unsigned long event, void * callData) override;

  void OnInteractorStyleEvent(int eventID) override;

protected:
  vtkCjyxVolumeRenderingLogic *VolumeRenderingLogic{nullptr};

protected:
  vtkDMMLVolumeRenderingDisplayableManager(const vtkDMMLVolumeRenderingDisplayableManager&); // Not implemented
  void operator=(const vtkDMMLVolumeRenderingDisplayableManager&); // Not implemented

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal;
};

#endif
