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

#ifndef __vtkDMMLAbstractThreeDViewDisplayableManager_h
#define __vtkDMMLAbstractThreeDViewDisplayableManager_h

// DMMLDisplayableManager includes
#include "vtkDMMLAbstractDisplayableManager.h"

#include "vtkDMMLDisplayableManagerExport.h"

class vtkDMMLViewNode;

/// \brief Superclass for displayable manager classes.
///
/// A displayable manager class is responsible to represent a
/// DMMLDisplayable node in a renderer.
class VTK_DMML_DISPLAYABLEMANAGER_EXPORT vtkDMMLAbstractThreeDViewDisplayableManager :
    public vtkDMMLAbstractDisplayableManager
{
public:

  static vtkDMMLAbstractThreeDViewDisplayableManager *New();
  void PrintSelf(ostream& os, vtkIndent indent) override;
  vtkTypeMacro(vtkDMMLAbstractThreeDViewDisplayableManager, vtkDMMLAbstractDisplayableManager);

  /// Get DMML ViewNode
  vtkDMMLViewNode * GetDMMLViewNode();


  /// Find display node managed by the displayable manager at a specified world RAS position.
  /// \return Non-zero in case a node is found at the position, 0 otherwise
  virtual int Pick3D(double vtkNotUsed(ras)[3]) { return 0; }

  /// Get the DMML ID of the picked node, returns empty string if no pick
  virtual const char* GetPickedNodeID() { return nullptr; }

protected:

  vtkDMMLAbstractThreeDViewDisplayableManager();
  ~vtkDMMLAbstractThreeDViewDisplayableManager() override;

  void OnDMMLDisplayableNodeModifiedEvent(vtkObject* caller) override;

  /// Could be overloaded in DisplayableManager subclass
  virtual void OnDMMLViewNodeModifiedEvent(){}

  virtual void PassThroughInteractorStyleEvent(int eventid);

private:

  vtkDMMLAbstractThreeDViewDisplayableManager(const vtkDMMLAbstractThreeDViewDisplayableManager&) = delete;
  void operator=(const vtkDMMLAbstractThreeDViewDisplayableManager&) = delete;
};

#endif
