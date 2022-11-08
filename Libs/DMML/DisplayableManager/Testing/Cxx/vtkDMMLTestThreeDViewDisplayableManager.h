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

#ifndef __vtkDMMLTestThreeDViewDisplayableManager_h
#define __vtkDMMLTestThreeDViewDisplayableManager_h

// DMMLDisplayableManager includes
#include "vtkDMMLAbstractThreeDViewDisplayableManager.h"

class vtkDMMLCameraNode;

class vtkDMMLTestThreeDViewDisplayableManager :
  public vtkDMMLAbstractThreeDViewDisplayableManager
{

public:
  static vtkDMMLTestThreeDViewDisplayableManager* New();
  vtkTypeMacro(vtkDMMLTestThreeDViewDisplayableManager,vtkDMMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // For testing
  static int NodeAddedCount;

protected:

  vtkDMMLTestThreeDViewDisplayableManager();
  ~vtkDMMLTestThreeDViewDisplayableManager() override;

  virtual void AdditionnalInitializeStep();

  void Create() override;

//  virtual void OnDMMLSceneAboutToBeClosedEvent(){}
//  virtual void OnDMMLSceneClosedEvent(){}
//  virtual void OnDMMLSceneAboutToBeImportedEvent(){}
//  virtual void OnDMMLSceneImportedEvent(){}
//  virtual void OnDMMLSceneRestoredEvent(){}
  void OnDMMLSceneNodeAdded(vtkDMMLNode* node) override;
//  virtual void OnDMMLSceneNodeRemovedEvent(vtkDMMLNode* /*node*/){}

private:

  vtkDMMLTestThreeDViewDisplayableManager(const vtkDMMLTestThreeDViewDisplayableManager&) = delete;
  void operator=(const vtkDMMLTestThreeDViewDisplayableManager&) = delete;

  class vtkInternal;
  vtkInternal * Internal;

};

#endif
