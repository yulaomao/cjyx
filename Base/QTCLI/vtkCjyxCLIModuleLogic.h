/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Cjyx

=========================================================================auto=*/

#ifndef __vtkCjyxCLIModuleLogic_h
#define __vtkCjyxCLIModuleLogic_h

// Cjyx includes
#include "vtkCjyxModuleLogic.h"

// DMMLCLI includes
#include "vtkDMMLCommandLineModuleNode.h"
class ModuleDescription;
class ModuleParameter;

// DMML include
#include "vtkDMMLScene.h"
class vtkDMMLModelHierarchyNode;
class DMMLIDMap;

// STL includes
#include <string>

#include "qCjyxBaseQTCLIExport.h"

typedef enum { CommandLineModule, SharedObjectModule, PythonModule } CommandLineModuleType;

/// \brief Logic for running CLI
///
/// vtkCjyxCLIModuleLogic logic allows to run a either synchronously or asynchronously CLI
/// using parameters of a \a vtkDMMLCommandLineModuleNode.
/// While a CLI module logic can run any CLI node, the logic can uniquely be
/// assigned a specific CLI by setting a DefaultModuleDescription.
class Q_CJYX_BASE_QTCLI_EXPORT vtkCjyxCLIModuleLogic :
  public vtkCjyxModuleLogic
{
public:
  static vtkCjyxCLIModuleLogic *New();
  vtkTypeMacro(vtkCjyxCLIModuleLogic,vtkCjyxModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// The default module description is used when creating new nodes.
  /// \sa CreateNode()
  void SetDefaultModuleDescription(const ModuleDescription& moduleDescription);
  const ModuleDescription& GetDefaultModuleDescription()const;

  /// Set the value of the parameter \a name.
  /// Return true if the parameter is found and different than \a value,
  /// false otherwise.
  /// \sa GetDefaultParameterValue(),
  bool SetDefaultParameterValue(const std::string& name, const std::string& value);

  /// Return the default parameter value and an empty string if the parameter
  /// can not be found.
  /// \sa SetDefaultParameterValue()
  std::string GetDefaultParameterValue(const std::string& name) const;

  /// Instantiate a default command line module node.
  /// If a default module description has been given, the node is initialized
  /// with the description.
  /// Warning: The caller is responsible for deleting it.
  /// \sa CreateNodeInScene(), SetDefaultModuleDescription()
  vtkDMMLCommandLineModuleNode* CreateNode();

  /// Instantiate a default command line module node and add it into the
  /// scene.
  /// The caller is responsible for remove the node from the scene.
  /// \sa CreateNode(), SetDefaultModuleDescription()
  vtkDMMLCommandLineModuleNode* CreateNodeInScene();

  // TODO: do we need to observe DMML here?
  virtual void ProcessDmmlEvents(vtkObject * vtkNotUsed(caller),
                                 unsigned long vtkNotUsed(event),
                                 void * vtkNotUsed(callData)){}

  /// For debugging, control deletion of temp files
  virtual void DeleteTemporaryFilesOn();
  virtual void DeleteTemporaryFilesOff();
  void SetDeleteTemporaryFiles(int value);
  int GetDeleteTemporaryFiles() const;

  // Control use of in-memory data transfer by this specific CLI.
  void SetAllowInMemoryTransfer(int value);
  int GetAllowInMemoryTransfer() const;

  /// For debugging, control redirection of cout and cerr
  virtual void RedirectModuleStreamsOn();
  virtual void RedirectModuleStreamsOff();
  void SetRedirectModuleStreams(int value);
  int GetRedirectModuleStreams() const;

  /// Schedules the command line module to run.
  /// The CLI is scheduled to be run in a separate thread. This methods
  /// is non blocking and returns immediately.
  /// If \a updateDisplay is 'true' the selection node will be updated with the
  /// the created nodes, which would automatically select the created nodes
  /// in the node selectors.
  void Apply( vtkDMMLCommandLineModuleNode* node, bool updateDisplay = true );

  /// Don't start the CLI in a separate thread, but run it in the main thread.
  /// This methods is blocking until the CLI finishes to execute, the UI being
  /// frozen until that time.
  /// If \a updateDisplay is 'true' the selection node will be updated with the
  /// the created nodes, which would automatically select the created nodes
  /// in the node selectors.
  void ApplyAndWait ( vtkDMMLCommandLineModuleNode* node, bool updateDisplay = true);

  void KillProcesses();

//   void LazyEvaluateModuleTarget(ModuleDescription& moduleDescriptionObject);
//   void LazyEvaluateModuleTarget(vtkDMMLCommandLineModuleNode* node)
//     { this->LazyEvaluateModuleTarget(node->GetModuleDescription()); }

  /// Set the application logic
  void SetDMMLApplicationLogic(vtkDMMLApplicationLogic* logic) override;

protected:
  /// Reimplemented to observe NodeAddedEvent.
  void SetDMMLSceneInternal(vtkDMMLScene * newScene) override;
  /// Reimplemented for AutoRun mode.
  void OnDMMLSceneNodeAdded(vtkDMMLNode* node) override;
  /// Reimplemented to observe CLI node.
  void ProcessDMMLNodesEvents(vtkObject *caller, unsigned long event,
                                      void *callData) override;
  /// Reimplemented to observe vtkCjyxApplicationLogic.
  void ProcessDMMLLogicsEvents(vtkObject*, long unsigned int, void*) override;


  std::string ConstructTemporaryFileName(const std::string& tag,
                                         const std::string& type,
                                         const std::string& name,
                                     const std::vector<std::string>& extensions,
                                     CommandLineModuleType commandType);
  std::string ConstructTemporarySceneFileName(vtkDMMLScene *scene);
  std::string FindHiddenNodeID(const ModuleDescription& d,
                               const ModuleParameter& p);

  // The method that runs the command line module
  void ApplyTask(void *clientdata);

  // Communicate progress back to the node
  static void ProgressCallback(void *);

  /// Return true if the commandlinemodule node can update the
  /// selection node with the outputs of the CLI
  bool IsCommandLineModuleNodeUpdatingDisplay(
    vtkDMMLCommandLineModuleNode* commandLineModuleNode)const;

  /// Call apply because the node requests it.
  void AutoRun(vtkDMMLCommandLineModuleNode* cliNode);

    /// List of custom events fired by the class.
  enum Events{
    RequestHierarchyEditEvent = vtkCommand::UserEvent + 1
  };

  // Add a model hierarchy node and all its descendents to a scene (miniscene to sent to a CLI).
  // The mapping of ids from the original scene to the mini scene is put in (added to) sceneToMiniSceneMap.
  // Any files that will be created by writing out the miniscene are added to filesToDelete (i.e. models)
  void AddCompleteModelHierarchyToMiniScene(vtkDMMLScene*, vtkDMMLModelHierarchyNode*, DMMLIDMap* sceneToMiniSceneMap, std::set<std::string> &filesToDelete);

  int GetCoordinateSystemFromString(const char* coordinateSystemStr)const;

private:
  vtkCjyxCLIModuleLogic();
  ~vtkCjyxCLIModuleLogic() override;
  vtkCjyxCLIModuleLogic(const vtkCjyxCLIModuleLogic&) = delete;
  void operator=(const vtkCjyxCLIModuleLogic&) = delete;

  class vtkInternal;
  vtkInternal * Internal;
};

#endif

