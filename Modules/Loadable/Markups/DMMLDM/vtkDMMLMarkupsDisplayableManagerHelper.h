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
/// .NAME vtkDMMLMarkupsDisplayableManagerHelper - utility class to manage widgets
/// .SECTION Description
/// This class defines lists that are used to manage the widgets associated with markups.
/// A markup which is managed by a displayableManager consists of
///   a) the Markups DMML Node (MarkupsNodeList)
///   b) the vtkWidget to show this markup (Widgets)
///   c) a vtkWidget to represent sliceIntersections in the slice viewers (WidgetIntersections)
///


#ifndef VTKDMMLMARKUPSDISPLAYABLEMANAGERHELPER_H_
#define VTKDMMLMARKUPSDISPLAYABLEMANAGERHELPER_H_

// MarkupsModule includes
#include "vtkCjyxMarkupsModuleDMMLDisplayableManagerExport.h"

// MarkupsModule/DMML includes
#include <vtkDMMLMarkupsNode.h>

// VTK includes
#include <vtkCjyxMarkupsWidget.h>
#include <vtkSmartPointer.h>

// DMML includes
#include <vtkDMMLSliceNode.h>

// STL includes
#include <set>

class vtkDMMLMarkupsDisplayableManager;
class vtkDMMLMarkupsDisplayNode;
class vtkDMMLInteractionNode;

/// \ingroup Cjyx_QtModules_Markups
class VTK_CJYX_MARKUPS_MODULE_DMMLDISPLAYABLEMANAGER_EXPORT vtkDMMLMarkupsDisplayableManagerHelper :
    public vtkObject
{
public:

  static vtkDMMLMarkupsDisplayableManagerHelper *New();
  vtkTypeMacro(vtkDMMLMarkupsDisplayableManagerHelper, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkGetObjectMacro(DisplayableManager, vtkDMMLMarkupsDisplayableManager);
  void SetDisplayableManager(vtkDMMLMarkupsDisplayableManager*);

  /// Set all widget status to manipulate
  //void SetAllWidgetsToManipulate();

  /// Get a vtkCjyxMarkupsWidget* given a node
  vtkCjyxMarkupsWidget * GetWidget(vtkDMMLMarkupsDisplayNode * markupsDisplayNode);
  /// Get first visible widget for this markup
  vtkCjyxMarkupsWidget * GetWidget(vtkDMMLMarkupsNode * markupsNode);

  /// Remove all widgets, intersection widgets, nodes
  void RemoveAllWidgetsAndNodes();

  /// Map of vtkWidget indexed using associated node ID
  typedef std::map < vtkSmartPointer<vtkDMMLMarkupsDisplayNode>, vtkCjyxMarkupsWidget* > DisplayNodeToWidgetType;
  typedef std::map < vtkSmartPointer<vtkDMMLMarkupsDisplayNode>, vtkCjyxMarkupsWidget* >::iterator DisplayNodeToWidgetIt;
  DisplayNodeToWidgetType MarkupsDisplayNodesToWidgets;  // display nodes with widgets assigned

  typedef std::set < vtkSmartPointer<vtkDMMLMarkupsNode> > MarkupsNodesType;
  typedef std::set < vtkSmartPointer<vtkDMMLMarkupsNode> >::iterator MarkupsNodesIt;
  MarkupsNodesType MarkupsNodes; // observed markups nodes

  void AddMarkupsNode(vtkDMMLMarkupsNode* node);
  void RemoveMarkupsNode(vtkDMMLMarkupsNode* node);
  void AddDisplayNode(vtkDMMLMarkupsDisplayNode* displayNode);
  void RemoveDisplayNode(vtkDMMLMarkupsDisplayNode* displayNode);

  void DeleteWidget(vtkCjyxMarkupsWidget* widget);

  void AddObservations(vtkDMMLMarkupsNode* node);
  void RemoveObservations(vtkDMMLMarkupsNode* node);

protected:

  vtkDMMLMarkupsDisplayableManagerHelper();
  ~vtkDMMLMarkupsDisplayableManagerHelper() override;

private:

  vtkDMMLMarkupsDisplayableManagerHelper(const vtkDMMLMarkupsDisplayableManagerHelper&) = delete;
  void operator=(const vtkDMMLMarkupsDisplayableManagerHelper&) = delete;

  /// Keep a record of the current glyph type for the handles in the widget
  /// associated with this node, prevents changing them unnecessarily
  std::map<vtkDMMLNode*, std::vector<int> > NodeGlyphTypes;

  bool AddingMarkupsNode;

  std::vector<unsigned long> ObservedMarkupNodeEvents;

  vtkDMMLMarkupsDisplayableManager* DisplayableManager;
};

#endif /* VTKDMMLMARKUPSDISPLAYABLEMANAGERHELPER_H_ */
