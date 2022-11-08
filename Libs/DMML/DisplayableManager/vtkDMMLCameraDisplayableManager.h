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

  Based on Cjyx/Base/GUI/vtkCjyxViewerWidget.h,
  this file was developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __vtkDMMLCameraDisplayableManager_h
#define __vtkDMMLCameraDisplayableManager_h

// DMMLDisplayableManager includes
#include "vtkDMMLAbstractThreeDViewDisplayableManager.h"

#include "vtkDMMLDisplayableManagerExport.h"

class vtkDMMLCameraNode;
class vtkDMMLCameraWidget;

class VTK_DMML_DISPLAYABLEMANAGER_EXPORT vtkDMMLCameraDisplayableManager :
  public vtkDMMLAbstractThreeDViewDisplayableManager
{

public:
  static vtkDMMLCameraDisplayableManager* New();
  vtkTypeMacro(vtkDMMLCameraDisplayableManager,vtkDMMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void RemoveDMMLObservers() override;

  void UpdateCameraNode();

  vtkDMMLCameraNode* GetCameraNode();

  /// Events
  enum
  {
    ActiveCameraChangedEvent   = 30000
  };

  bool CanProcessInteractionEvent(vtkDMMLInteractionEventData* eventData, double &closestDistance2) override;
  bool ProcessInteractionEvent(vtkDMMLInteractionEventData* eventData) override;

  vtkDMMLCameraWidget* GetCameraWidget();

protected:

  vtkDMMLCameraDisplayableManager();
  ~vtkDMMLCameraDisplayableManager() override;

  void Create() override;

  void OnDMMLSceneEndClose() override;
  void OnDMMLSceneStartImport() override;
  void OnDMMLSceneEndImport() override;
  void OnDMMLSceneEndRestore() override;
  void OnDMMLSceneNodeAdded(vtkDMMLNode* node) override;
  void OnDMMLSceneNodeRemoved(vtkDMMLNode* node) override;

  void ProcessDMMLNodesEvents(vtkObject *caller,
                                      unsigned long event,
                                      void *callData) override;
  void OnDMMLNodeModified(vtkDMMLNode* node) override;

  void SetAndObserveCameraNode(vtkDMMLCameraNode * newCameraNode);
  void AdditionalInitializeStep() override;
  void SetCameraToRenderer();
  void SetCameraToInteractor();

private:

  vtkDMMLCameraDisplayableManager(const vtkDMMLCameraDisplayableManager&) = delete;
  void operator=(const vtkDMMLCameraDisplayableManager&) = delete;

  class vtkInternal;
  vtkInternal * Internal;

};

#endif
