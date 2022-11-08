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

  This file was originally developed by Daniel Haehn, UPenn
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __vtkDMMLSceneViewsModuleLogic_h
#define __vtkDMMLSceneViewsModuleLogic_h

// CjyxLogic includes
#include "vtkCjyxBaseLogic.h"

// DMMLLogic includes
#include "vtkDMMLAbstractLogic.h"

#include "vtkCjyxSceneViewsModuleLogicExport.h"
//#include "qCjyxSceneViewsModuleExport.h"

#include "vtkCjyxModuleLogic.h"

// DMML includes
class vtkDMMLSceneViewNode;

// VTK includes
class vtkImageData;
#include <vtkStdString.h>

#include <string>

/// \ingroup Cjyx_QtModules_SceneViews
class VTK_CJYX_SCENEVIEWS_MODULE_LOGIC_EXPORT vtkCjyxSceneViewsModuleLogic :
  public vtkCjyxModuleLogic
{
public:

  static vtkCjyxSceneViewsModuleLogic *New();
  vtkTypeMacro(vtkCjyxSceneViewsModuleLogic,vtkCjyxModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Initialize listening to DMML events
  void SetDMMLSceneInternal(vtkDMMLScene * newScene) override;

  /// Register DMML Node classes to Scene. Gets called automatically when the DMMLScene is attached to this logic class.
  void RegisterNodes() override;

  /// Create a sceneView..
  void CreateSceneView(const char* name, const char* description, int screenshotType, vtkImageData* screenshot);

  /// Modify an existing sceneView.
  void ModifySceneView(vtkStdString id, const char* name, const char* description, int screenshotType, vtkImageData* screenshot);

  /// Return the name of an existing sceneView.
  vtkStdString GetSceneViewName(const char* id);

  /// Return the description of an existing sceneView.
  vtkStdString GetSceneViewDescription(const char* id);

  /// Return the screenshotType of an existing sceneView.
  int GetSceneViewScreenshotType(const char* id);

  /// Return the screenshot of an existing sceneView.
  vtkImageData* GetSceneViewScreenshot(const char* id);

  /// Restore a sceneView.
  /// If removeNodes flag is false, don't restore the scene if it will remove data.
  /// The method will return with false if restore failed because nodes were not allowed
  /// to be removed.
  /// RemoveNodes defaults to true for backward compatibility.
  bool RestoreSceneView(const char* id, bool removeNodes = true);

  /// Move sceneView up
  const char* MoveSceneViewUp(const char* id);

  /// Move sceneView up
  const char* MoveSceneViewDown(const char* id);

  /// Remove a scene view node
  void RemoveSceneViewNode(vtkDMMLSceneViewNode *sceneViewNode);

protected:

  vtkCjyxSceneViewsModuleLogic();

  ~vtkCjyxSceneViewsModuleLogic() override;

  void OnDMMLSceneNodeAdded(vtkDMMLNode* node) override;
  void OnDMMLSceneEndImport() override;
  void OnDMMLSceneEndRestore() override;
  void OnDMMLSceneEndClose() override;

  void OnDMMLNodeModified(vtkDMMLNode* node) override;

private:

  std::string m_StringHolder;

private:
  vtkCjyxSceneViewsModuleLogic(const vtkCjyxSceneViewsModuleLogic&) = delete;
  void operator=(const vtkCjyxSceneViewsModuleLogic&) = delete;
};

#endif
