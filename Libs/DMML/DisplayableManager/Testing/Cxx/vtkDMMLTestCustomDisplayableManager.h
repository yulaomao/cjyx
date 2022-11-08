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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __vtkDMMLTestCustomDisplayableManager_h
#define __vtkDMMLTestCustomDisplayableManager_h

// DMMLDisplayableManager includes
#include "vtkDMMLAbstractDisplayableManager.h"

class vtkDMMLCameraNode;

class vtkDMMLTestCustomDisplayableManager :
  public vtkDMMLAbstractDisplayableManager
{

public:
  static vtkDMMLTestCustomDisplayableManager* New();
  vtkTypeMacro(vtkDMMLTestCustomDisplayableManager,vtkDMMLAbstractDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // For testing
  static int NodeAddedCountThreeDView;
  static int NodeAddedCountSliceView;

protected:

  vtkDMMLTestCustomDisplayableManager();
  ~vtkDMMLTestCustomDisplayableManager() override;

  virtual void AdditionnalInitializeStep();
  void OnInteractorStyleEvent(int eventid) override;

  void Create() override;

  void OnDMMLSceneNodeAdded(vtkDMMLNode* node) override;

private:

  vtkDMMLTestCustomDisplayableManager(const vtkDMMLTestCustomDisplayableManager&) = delete;
  void operator=(const vtkDMMLTestCustomDisplayableManager&) = delete;

  class vtkInternal;
  vtkInternal * Internal;

};

#endif
