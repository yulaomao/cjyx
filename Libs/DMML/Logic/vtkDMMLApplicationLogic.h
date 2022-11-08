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

#ifndef __vtkDMMLApplicationLogic_h
#define __vtkDMMLApplicationLogic_h

// DMMLLogic includes
#include "vtkDMMLAbstractLogic.h"

#include "vtkDMMLLogicExport.h"
#include "vtkDMMLSliceCompositeNode.h"

class vtkDMMLColorLogic;
class vtkDMMLModelDisplayNode;
class vtkDMMLSliceNode;
class vtkDMMLSliceLogic;
class vtkDMMLSelectionNode;
class vtkDMMLStorableNode;
class vtkDMMLStorageNode;
class vtkDMMLInteractionNode;
class vtkDMMLViewLogic;
class vtkDMMLViewNode;

// VTK includes
class vtkCollection;
class vtkImageData;

// STD includes
#include <vector>

class VTK_DMML_LOGIC_EXPORT vtkDMMLApplicationLogic
  : public vtkDMMLAbstractLogic
{
public:

  static vtkDMMLApplicationLogic *New();
  void PrintSelf(ostream& os, vtkIndent indent) override;
  vtkTypeMacro(vtkDMMLApplicationLogic, vtkDMMLAbstractLogic);

  /// Get default Selection node
  vtkDMMLSelectionNode* GetSelectionNode()const;

  /// Get default Interaction node
  vtkDMMLInteractionNode* GetInteractionNode()const;

  /// All the slice logics in the application
  void SetSliceLogics(vtkCollection* sliceLogics);
  vtkCollection* GetSliceLogics()const;
  vtkDMMLSliceLogic* GetSliceLogic(vtkDMMLSliceNode* sliceNode) const;
  vtkDMMLSliceLogic* GetSliceLogicByLayoutName(const char* layoutName) const;
  /// Get slice logic from slice model display node
  vtkDMMLSliceLogic* GetSliceLogicByModelDisplayNode(vtkDMMLModelDisplayNode* displayNode) const;

  /// All the view logics in the application
  void SetViewLogics(vtkCollection* viewLogics);
  vtkCollection* GetViewLogics()const;
  vtkDMMLViewLogic* GetViewLogic(vtkDMMLViewNode* viewNode) const;
  vtkDMMLViewLogic* GetViewLogicByLayoutName(const char* layoutName) const;

  /// Set/Get color logic.
  /// The application typically sets a custom color logic (i.e.
  /// vtkCjyxColorLogic) that contains default color nodes.
  /// By default, a vtkDMMLColorLogic is instantiated.
  void SetColorLogic(vtkDMMLColorLogic* newColorLogic);
  vtkDMMLColorLogic* GetColorLogic()const;

  /// Apply the active volumes in the SelectionNode to the slice composite nodes
  /// Perform the default behavior related to selecting a volume
  /// (in this case, making it the background for all SliceCompositeNodes)
  /// \sa vtkInternal::PropagateVolumeSelection()
  /// \sa FitSliceToAll()
  /// \sa vtkDMMLSelectionNode::SetActiveVolumeID()
  /// \sa vtkDMMLSelectionNode::SetSecondaryVolumeID()
  /// \sa vtkDMMLSelectionNode::SetActiveLabelVolumeID()
  void PropagateVolumeSelection(int fit = 1);

  /// Propagate only active background volume in the SelectionNode to slice composite
  /// nodes
  /// \sa FitSliceToAll()
  /// \sa vtkDMMLSelectionNode::SetActiveVolumeID()
  /// \sa Layers::BackgroundLayer
  void PropagateBackgroundVolumeSelection(int fit = 1);

  /// Propagate only active foreground volume in the SelectionNode to slice composite
  /// nodes
  /// \sa FitSliceToAll()
  /// \sa vtkDMMLSelectionNode::SetSecondaryVolumeID()
  /// \sa Layers::ForegroundLayer
  void PropagateForegroundVolumeSelection(int fit = 1);

  /// Propagate only active label volume in the SelectionNode to slice composite
  /// nodes
  /// \sa FitSliceToAll()
  /// \sa vtkDMMLSelectionNode::SetActiveLabelVolumeID()
  /// \sa Layers::LabelLayer
  void PropagateLabelVolumeSelection(int fit = 1);

  enum Layers
  {
    LabelLayer = 0x1,
    ForegroundLayer = 0x2,
    BackgroundLayer = 0x4,
    AllLayers = LabelLayer | ForegroundLayer | BackgroundLayer
  };

  /// Propagate selected volume layer in the SelectionNode to the slice composite
  /// nodes.
  /// \sa Layers
  void PropagateVolumeSelection(int layer, int fit);

  /// Fit all the volumes into their views
  /// If onlyIfPropagateVolumeSelectionAllowed is true then field of view will be reset on
  /// only those slices where propagate volume selection is allowed
  /// If resetOrientation is true then slice orientation can be modified during function call
  void FitSliceToAll(bool onlyIfPropagateVolumeSelectionAllowed=false, bool resetOrientation=true);

  /// Propagate selected table in the SelectionNode to table view nodes.
  void PropagateTableSelection();

  /// Propagate selected PlotChart in the SelectionNode to Plot view nodes.
  void PropagatePlotChartSelection();

  /// zip the directory into a zip file
  /// Returns success or failure.
  bool Zip(const char* zipFileName, const char* directoryToZip);

  /// unzip the zip file to the current working directory
  /// Returns success or failure.
  bool Unzip(const char* zipFileName, const char* destinationDirectory);

  /// Convert reserved characters into percent notation to avoid issues with filenames
  /// containing things that might be mistaken, for example, for
  /// windows drive letters.  Used internally by SaveSceneToCjyxDataBundleDirectory.
  /// This is not a general purpose implementation; it preserves commonly used
  /// characters for filenames but avoids known issue like slashes or colons.
  /// Ideally a version from vtksys
  /// or similar should be used, but nothing seems to be available.
  /// https://en.wikipedia.org/wiki/Percent-encoding
  /// See https://github.com/Slicer/Slicer/issues/2605
  static std::string PercentEncode(std::string s);

  /// Save the scene into a self contained directory, sdbDir
  /// Called by the qCjyxSceneWriter, which can be accessed via
  /// \sa qCjyxCoreIOManager::saveScene
  /// If screenShot is not null, use it as the screen shot for a scene view
  /// Returns false if the save failed
  bool SaveSceneToCjyxDataBundleDirectory(const char* sdbDir, vtkImageData* screenShot = nullptr);

  /// Open the file into a temp directory and load the scene file
  /// inside.  Note that the first dmml file found in the extracted
  /// directory will be used.
  bool OpenCjyxDataBundle(const char* sdbFilePath, const char* temporaryDirectory);

  /// Unpack the file into a temp directory and return the scene file
  /// inside.  Note that the first dmml file found in the extracted
  /// directory will be used.
  std::string UnpackCjyxDataBundle(const char* sdbFilePath, const char* temporaryDirectory);

  /// Load any default parameter sets into the specified scene
  /// Returns the total number of loaded parameter sets
  static int LoadDefaultParameterSets(vtkDMMLScene* scene,
                                      const std::vector<std::string>& directories);

  /// Creates a unique (non-existent) file name by adding an index after base file name.
  /// knownExtension specifies the extension the index should be inserted before.
  /// It is necessary to provide extension, because there is no reliable way of correctly
  /// determining extension automatically (for example, file extension of some.file.nii.gz
  /// could be gz, nii.gz, or file.nii.gz and only one of them is correct).
  static std::string CreateUniqueFileName(const std::string &filename, const std::string& knownExtension="");

  /// List of custom events fired by the class.
  enum Events{
    RequestInvokeEvent = vtkCommand::UserEvent + 1,
    PauseRenderEvent = vtkCommand::UserEvent + 101,
    ResumeRenderEvent,
    EditNodeEvent,
    ShowViewContextMenuEvent,
  };
  /// Structure passed as calldata pointer in the RequestEvent invoked event.
  struct InvokeRequest{
    InvokeRequest();
    /// 100ms by default.
    unsigned int Delay{100};
    /// the caller to call InvokeEvent() on.
    vtkObject* Caller{nullptr};
    /// The event id of InvokeEvent. ModifiedEvent by default.
    unsigned long EventID{vtkCommand::ModifiedEvent};
    /// Optional call data. 0 by default.
    void* CallData{nullptr};
  };
  /// Conveniently calls an InvokeEvent on an object with a delay.
  void InvokeEventWithDelay(unsigned int delayInMs,
                            vtkObject* caller,
                            unsigned long eventID = vtkCommand::ModifiedEvent,
                            void* callData = nullptr);

  /// Return the temporary path that was set by the application
  const char* GetTemporaryPath();

  /// Set the temporary path the logics can use. The path should be set by the application
  void SetTemporaryPath(const char* path);

  /// Saves the provided image as screenshot of the scene (same filepath as the scene URL but extension is .png instead of .dmml).
  /// Uses current scene's URL property, so the URL must be up-to-date when calling this method.
  void SaveSceneScreenshot(vtkImageData* screenshot);

  /// Pauses rendering for all views in the current layout.
  /// It should be used in situations where multiple nodes are modified, and it is undesirable to display an intermediate
  /// state.
  /// The caller is responsible for making sure that each PauseRender() is paired with
  /// ResumeRender().
  /// \sa vtkDMMLApplicationLogic::ResumeRender()
  /// \sa qCjyxApplication::setRenderPaused()
  virtual void PauseRender();

  /// Resumes rendering for all views in the current layout.
  /// \sa vtkDMMLApplicationLogic::PauseRender()
  /// \sa qCjyxApplication::setRenderPaused()
  virtual void ResumeRender();

  /// Requests the application to show user interface for editing a node.
  virtual void EditNode(vtkDMMLNode* node);

  /// Sets a module with its corresponding logic to the application logic.
  /// \param moduleName name of the module.
  /// \param moduleLogic pointer to logic to be associated to the module. If this
  /// parameter is nullptr, then the module logic will be removed from the application
  /// logic.
  void SetModuleLogic(const char* moduleName, vtkDMMLAbstractLogic* moduleLogic);

  /// Gets a constant pointer to module logic associated with a given module
  /// \param moduleName name of the module associated to the logic
  /// \return constant pointer to vtkDMMLAbstractLogic corresponding to the
  /// logic associated to th logic
  vtkDMMLAbstractLogic* GetModuleLogic(const char* moduleName) const;

  enum IntersectingSlicesOperation
  {
    IntersectingSlicesVisibility,
    IntersectingSlicesInteractive,
    IntersectingSlicesTranslation,
    IntersectingSlicesRotation,
  };

  void SetIntersectingSlicesEnabled(IntersectingSlicesOperation operation, bool enabled);
  bool GetIntersectingSlicesEnabled(IntersectingSlicesOperation operation);

  void SetIntersectingSlicesIntersectionMode(int mode);
  int GetIntersectingSlicesIntersectionMode();

  void SetIntersectingSlicesLineThicknessMode(int mode);
  int GetIntersectingSlicesLineThicknessMode();

protected:

  vtkDMMLApplicationLogic();
  ~vtkDMMLApplicationLogic() override;

  void SetDMMLSceneInternal(vtkDMMLScene *newScene) override;

  void SetSelectionNode(vtkDMMLSelectionNode* );
  void SetInteractionNode(vtkDMMLInteractionNode* );

  void ProcessDMMLNodesEvents(vtkObject* caller, unsigned long event, void* callData) override;

  void OnDMMLSceneStartBatchProcess() override;
  void OnDMMLSceneEndBatchProcess() override;
  void OnDMMLSceneStartImport() override;
  void OnDMMLSceneEndImport() override;
  void OnDMMLSceneStartRestore() override;
  void OnDMMLSceneEndRestore() override;

private:

  vtkDMMLApplicationLogic(const vtkDMMLApplicationLogic&) = delete;
  void operator=(const vtkDMMLApplicationLogic&) = delete;

  class vtkInternal;
  vtkInternal* Internal;

};


#endif

