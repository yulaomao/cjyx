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
  and was partially funded by NIH grant 1U24CA194354-01

==============================================================================*/

#ifndef __vtkDMMLCoreTestingUtilities_h
#define __vtkDMMLCoreTestingUtilities_h

// DMML/Core includes
#include <vtkDMML.h>

// VTK includes
#include <vtkCallbackCommand.h>

// STD includes
#include <sstream>
#include <vector>
#include <map>

class vtkDMMLNode;
class vtkDMMLScene;
class vtkDMMLDisplayableNode;
class vtkDMMLDisplayNode;
class vtkDMMLStorableNode;
class vtkDMMLStorageNode;
class vtkDMMLTransformableNode;
class vtkDMMLTransformNode;

/// This module provides functions to facilitate writing tests.
///
/// Usually these test methods are used by single-line convenience macros
/// defined in vtkDMMLCoreTestingMacros.h.

namespace vtkDMMLCoreTestingUtilities
{

VTK_DMML_EXPORT
bool CheckNodeInSceneByID(int line, vtkDMMLScene* scene,
                          const char* nodeID, vtkDMMLNode* expected);

VTK_DMML_EXPORT
bool CheckNodeIdAndName(int line, vtkDMMLNode* node,
                        const char* expectedID, const char* expectedName);

template<typename Type>
std::string ToString(Type value);

/// Return list of node that should be added to the scene
VTK_DMML_EXPORT
int GetExpectedNodeAddedClassNames(
    const char * sceneFilePath, std::vector<std::string>& expectedNodeAddedClassNames);

/// Test basic VTK object methods (print, superclass, etc.)
VTK_DMML_EXPORT
int ExerciseBasicObjectMethods(vtkObject* object);

/// Tests all basic DMML methods available for the current class.
/// Internally it calls ExerciseBasicObjectMethods, ExerciseBasicDMMLMethods,
/// ExerciseBasicTransformableDMMLMethods, ExerciseBasicStorableDMMLMethods, etc.
VTK_DMML_EXPORT
int ExerciseAllBasicDMMLMethods(vtkDMMLNode* object);

/// Cjyx Libs/DMML/vtkDMMLNode exercises
VTK_DMML_EXPORT
int ExerciseBasicDMMLMethods(vtkDMMLNode* node);

/// For testing nodes in Libs/DMML that are storable. Calls the basic
/// dmml methods macro first.
VTK_DMML_EXPORT
int ExerciseBasicStorableDMMLMethods(vtkDMMLStorableNode* node);

/// For testing nodes in Libs/DMML that are transformable. Calls the basic
/// storable dmml methods macro first.
VTK_DMML_EXPORT
int ExerciseBasicTransformableDMMLMethods(vtkDMMLTransformableNode* node);

/// For testing nodes in Libs/DMML that are displayable. Calls the basic
/// transformable dmml methods macro first.
VTK_DMML_EXPORT
int ExerciseBasicDisplayableDMMLMethods(vtkDMMLDisplayableNode* node);

/// For testing nodes in Libs/DMML that are subclasses of the display node. Calls the basic
/// dmml methods macro first.
VTK_DMML_EXPORT
int ExerciseBasicDisplayDMMLMethods(vtkDMMLDisplayNode* node);

/// For testing nodes in Libs/DMML that are subclasses of the storage node. Calls the basic
/// dmml methods macro first.
VTK_DMML_EXPORT
int ExerciseBasicStorageDMMLMethods(vtkDMMLStorageNode* node);

/// For testing nodes in Libs/DMML that are transform nodes. Calls the basic
/// storable dmml methods macro first.
VTK_DMML_EXPORT
int ExerciseBasicTransformDMMLMethods(vtkDMMLTransformNode* node);

/// Test scene loading and import with a custom scene.
/// This is a utility function because scene import of custom DMML nodes cannot be tested
/// in the base DMML library.
/// If inputScene is provided then that scene will be used for testing scene loading. It is
/// needed when custom node registration is necessary in the scene.
VTK_DMML_EXPORT
int ExerciseSceneLoadingMethods(const char * sceneFilePath, vtkDMMLScene* inputScene = nullptr);

//---------------------------------------------------------------------------
class VTK_DMML_EXPORT vtkDMMLNodeCallback : public vtkCallbackCommand
{
public:
  static vtkDMMLNodeCallback *New() {return new vtkDMMLNodeCallback;}
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void Execute(vtkObject* caller, unsigned long eid, void *callData) override;
  virtual void ResetNumberOfEvents();

  void SetDMMLNode(vtkDMMLNode*);
  std::string GetErrorString();

  /// Returns EXIT_SUCCESS if string is empty, EXIT_FAILURE if string is non-empty
  int CheckStatus();

  int GetNumberOfModified();
  int GetNumberOfEvents(unsigned long event);
  int GetTotalNumberOfEvents();
  std::vector<unsigned long> GetReceivedEvents();

protected:
  vtkDMMLNodeCallback();
  ~vtkDMMLNodeCallback() override;

  void SetErrorString(const char* error);

  void SetErrorString(int line, const char* error);

  vtkDMMLNode* Node{nullptr};
  std::string ErrorString;
  std::map<unsigned long, unsigned int> ReceivedEvents;
};

} // namespace vtkDMMLCoreTestingUtilities

#include "vtkDMMLCoreTestingUtilities.txx"

#endif
