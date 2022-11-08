/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __vtkDMMLMarkupsDisplayableManager_h
#define __vtkDMMLMarkupsDisplayableManager_h

// MarkupsModule includes
#include "vtkCjyxMarkupsModuleDMMLDisplayableManagerExport.h"

// MarkupsModule/DMMLDisplayableManager includes
#include "vtkDMMLMarkupsDisplayableManagerHelper.h"

// DMMLDisplayableManager includes
#include <vtkDMMLAbstractDisplayableManager.h>

// VTK includes
#include <vtkCjyxMarkupsWidget.h>

// STD includes
#include <map>

class vtkDMMLMarkupsNode;
class vtkCjyxViewerWidget;
class vtkDMMLMarkupsDisplayNode;
class vtkAbstractWidget;

/// \ingroup Cjyx_QtModules_Markups
class  VTK_CJYX_MARKUPS_MODULE_DMMLDISPLAYABLEMANAGER_EXPORT vtkDMMLMarkupsDisplayableManager :
    public vtkDMMLAbstractDisplayableManager
{
public:

  // Allow the helper to call protected methods of displayable manager
  friend class vtkDMMLMarkupsDisplayableManagerHelper;

  static vtkDMMLMarkupsDisplayableManager *New();
  vtkTypeMacro(vtkDMMLMarkupsDisplayableManager, vtkDMMLAbstractDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Check if this is a 2d SliceView displayable manager, returns true if so,
  /// false otherwise. Checks return from GetSliceNode for non null, which means
  /// it's a 2d displayable manager
  virtual bool Is2DDisplayableManager();
  /// Get the sliceNode, if registered. This would mean it is a 2D SliceView displayableManager.
  vtkDMMLSliceNode * GetDMMLSliceNode();

  vtkDMMLMarkupsDisplayableManagerHelper *  GetHelper() { return this->Helper; };

  bool CanProcessInteractionEvent(vtkDMMLInteractionEventData* eventData, double &closestDistance2) override;
  bool ProcessInteractionEvent(vtkDMMLInteractionEventData* eventData) override;

  void SetHasFocus(bool hasFocus, vtkDMMLInteractionEventData* eventData) override;
  bool GetGrabFocus() override;
  bool GetInteractive() override;
  int GetMouseCursor() override;

  // Updates markup point preview position.
  // Returns true if the event is processed.
  vtkCjyxMarkupsWidget* GetWidgetForPlacement();

  vtkDMMLMarkupsNode* GetActiveMarkupsNodeForPlacement();

  int GetCurrentInteractionMode();

  // Methods from vtkDMMLAbstractSliceViewDisplayableManager

  /// Convert device coordinates (display) to XYZ coordinates (viewport).
  /// Parameter \a xyz is double[3]
  /// \sa ConvertDeviceToXYZ(vtkRenderWindowInteractor *, vtkDMMLSliceNode *, double x, double y, double xyz[3])
  void ConvertDeviceToXYZ(double x, double y, double xyz[3]);

  /// Get the widget of a node.
  vtkCjyxMarkupsWidget* GetWidget(vtkDMMLMarkupsDisplayNode * node);

protected:

  vtkDMMLMarkupsDisplayableManager();
  ~vtkDMMLMarkupsDisplayableManager() override;

  vtkCjyxMarkupsWidget* FindClosestWidget(vtkDMMLInteractionEventData *callData, double &closestDistance2);

  void ProcessDMMLNodesEvents(vtkObject *caller, unsigned long event, void *callData) override;

  /// Wrap the superclass render request in a check for batch processing
  virtual void RequestRender();

  /// Called from RequestRender method if UpdateFromDMMLRequested is true
  /// \sa RequestRender() SetUpdateFromDMMLRequested()
  void UpdateFromDMML() override;

  void SetDMMLSceneInternal(vtkDMMLScene* newScene) override;

  /// Called after the corresponding DMML event is triggered, from AbstractDisplayableManager
  /// \sa ProcessDMMLSceneEvents
  void UpdateFromDMMLScene() override;
  void OnDMMLSceneEndClose() override;
  void OnDMMLSceneEndImport() override;
  void OnDMMLSceneNodeAdded(vtkDMMLNode* node) override;
  void OnDMMLSceneNodeRemoved(vtkDMMLNode* node) override;

  /// Create a widget.
  vtkCjyxMarkupsWidget* CreateWidget(vtkDMMLMarkupsDisplayNode* node);

  /// Called after the corresponding DMML View container was modified
  void OnDMMLDisplayableNodeModifiedEvent(vtkObject* caller) override;

  /// Handler for specific SliceView actions, iterate over the widgets in the helper
  virtual void OnDMMLSliceNodeModifiedEvent();

  /// Observe the interaction node.
  void AddObserversToInteractionNode();
  void RemoveObserversFromInteractionNode();

  /// Check if it is the right displayManager
  virtual bool IsCorrectDisplayableManager();

  /// Return true if this displayable manager supports(can manage) that node,
  /// false otherwise.
  /// Can be reimplemented to add more conditions.
  /// \sa IsManageable(const char*), IsCorrectDisplayableManager()
  virtual bool IsManageable(vtkDMMLNode* node);
  /// Return true if this displayable manager supports(can manage) that node class,
  /// false otherwise.
  /// Can be reimplemented to add more conditions.
  /// \sa IsManageable(vtkDMMLNode*), IsCorrectDisplayableManager()
  virtual bool IsManageable(const char* nodeClassName);

  /// Respond to interactor style events
  void OnInteractorStyleEvent(int eventid) override;

  /// Accessor for internal flag that disables interactor style event processing
  vtkGetMacro(DisableInteractorStyleEventsProcessing, int);

  vtkSmartPointer<vtkDMMLMarkupsDisplayableManagerHelper> Helper;

  double LastClickWorldCoordinates[4];

  vtkWeakPointer<vtkCjyxMarkupsWidget> LastActiveWidget;

private:
  vtkDMMLMarkupsDisplayableManager(const vtkDMMLMarkupsDisplayableManager&) = delete;
  void operator=(const vtkDMMLMarkupsDisplayableManager&) = delete;

  int DisableInteractorStyleEventsProcessing;

  // by default, this displayableManager handles a 2d view, so the SliceNode
  // must be set when it's assigned to a viewer
  vtkWeakPointer<vtkDMMLSliceNode> SliceNode;
};

#endif
