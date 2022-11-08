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

#ifndef __vtkDMMLDisplayableManagerFactory_h
#define __vtkDMMLDisplayableManagerFactory_h

// VTK includes
#include <vtkObject.h>

#include "vtkDMMLDisplayableManagerExport.h"

class vtkRenderer;
class vtkDMMLApplicationLogic;
class vtkDMMLDisplayableManagerGroup;

/// \brief Factory where displayable manager classes are registered.
///
/// A displayable manager class is responsible to represent a
/// DMMLDisplayable node in a renderer.
class VTK_DMML_DISPLAYABLEMANAGER_EXPORT vtkDMMLDisplayableManagerFactory : public vtkObject
{
public:
  static vtkDMMLDisplayableManagerFactory* New();
  vtkTypeMacro(vtkDMMLDisplayableManagerFactory,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  enum
  {
    DisplayableManagerFactoryRegisteredEvent     = 30100,
    DisplayableManagerFactoryUnRegisteredEvent   = 30101
  };

  /// Return True if Displayable Manager identified by \a vtkClassName
  /// is already registered.
  bool IsDisplayableManagerRegistered(const char* vtkClassName);

  /// Register Displayable Manager identified by \a vtkClassOrScriptName
  /// Returns True if displayable manager was successfully registered
  /// \a vtkClassOrScriptName should be either:
  /// a VTK class registered using the CMake macro VTK_MAKE_INSTANTIATOR3
  /// a python script having the extension .py
  /// Internally for each registered python script, a scripted displayable manager
  /// will be instantiated.
  bool RegisterDisplayableManager(const char* vtkClassOrScriptName);

  /// UnRegister Displayable Manager identified by \a vtkClassOrScriptName.
  /// Returns True if displayable manager was successfully unregistered
  /// \a vtkClassOrScriptName should be either a VTK class registered using the
  /// CMake macro VTK_MAKE_INSTANTIATOR3 or a python script name.
  bool UnRegisterDisplayableManager(const char* vtkClassOrScriptName);

  /// Return number of registered displayable managers
  int GetRegisteredDisplayableManagerCount();

  /// Return name of the nth registered displayable manager
  std::string GetRegisteredDisplayableManagerName(int n);

  /// Instantiate registered DisplayableManagers
  /// It returns a vtkDMMLDisplayableManagerGroup representing a list of DisplayableManager
  /// Internally, the factory keep track of all the Group and will invoke the ModifiedEvent
  /// of each group.
  vtkDMMLDisplayableManagerGroup* InstantiateDisplayableManagers(vtkRenderer * newRenderer);

  /// Set the application logic. It will be passed to displayable
  /// managers and groups to be created.
  void SetDMMLApplicationLogic(vtkDMMLApplicationLogic* applicationLogic);
  vtkDMMLApplicationLogic* GetDMMLApplicationLogic()const;

protected:

  vtkDMMLDisplayableManagerFactory();
  ~vtkDMMLDisplayableManagerFactory() override;

  class vtkInternal;
  vtkInternal* Internal;

  typedef vtkDMMLDisplayableManagerFactory Self;

private:

  vtkDMMLDisplayableManagerFactory(const vtkDMMLDisplayableManagerFactory&) = delete;
  void operator=(const vtkDMMLDisplayableManagerFactory&) = delete;

};

#endif
