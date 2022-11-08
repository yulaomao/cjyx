/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __qCjyxSubjectHierarchyLabelMapsPlugin_h
#define __qCjyxSubjectHierarchyLabelMapsPlugin_h

// SubjectHierarchy includes
#include "qCjyxSubjectHierarchyAbstractPlugin.h"

#include "qCjyxVolumesSubjectHierarchyPluginsExport.h"

class qCjyxSubjectHierarchyLabelMapsPluginPrivate;
class vtkDMMLLabelMapVolumeNode;

/// \ingroup Cjyx_QtModules_SubjectHierarchy_Plugins
class Q_CJYX_VOLUMES_SUBJECT_HIERARCHY_PLUGINS_EXPORT qCjyxSubjectHierarchyLabelMapsPlugin : public qCjyxSubjectHierarchyAbstractPlugin
{
public:
  Q_OBJECT

public:
  typedef qCjyxSubjectHierarchyAbstractPlugin Superclass;
  qCjyxSubjectHierarchyLabelMapsPlugin(QObject* parent = nullptr);
  ~qCjyxSubjectHierarchyLabelMapsPlugin() override;

public:
  /// Determines if a data node can be placed in the hierarchy using the actual plugin,
  /// and gets a confidence value for a certain DMML node (usually the type and possibly attributes are checked).
  /// \param node Node to be added to the hierarchy
  /// \param parentItemID Prospective parent of the node to add.
  ///   Default value is invalid. In that case the parent will be ignored, the confidence numbers are got based on the to-be child node alone.
  /// \return Floating point confidence number between 0 and 1, where 0 means that the plugin cannot handle the
  ///   node, and 1 means that the plugin is the only one that can handle the node (by node type or identifier attribute)
  double canAddNodeToSubjectHierarchy(
    vtkDMMLNode* node,
    vtkIdType parentItemID=vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID )const override;

  /// Determines if the actual plugin can handle a subject hierarchy item. The plugin with
  /// the highest confidence number will "own" the item in the subject hierarchy (set icon, tooltip,
  /// set context menu etc.)
  /// \param item Item to handle in the subject hierarchy tree
  /// \return Floating point confidence number between 0 and 1, where 0 means that the plugin cannot handle the
  ///   item, and 1 means that the plugin is the only one that can handle the item (by node type or identifier attribute)
  double canOwnSubjectHierarchyItem(vtkIdType itemID)const override;

  /// Get role that the plugin assigns to the subject hierarchy item.
  ///   Each plugin should provide only one role.
  Q_INVOKABLE const QString roleForPlugin()const override;

  /// Get icon of an owned subject hierarchy item
  /// \return Icon to set, empty icon if nothing to set
  QIcon icon(vtkIdType itemID) override;

  /// Get visibility icon for a visibility state
  QIcon visibilityIcon(int visible) override;

  /// Generate tooltip for a owned subject hierarchy item
  QString tooltip(vtkIdType itemID)const override;

  /// Set display visibility of a owned subject hierarchy item
  void setDisplayVisibility(vtkIdType itemID, int visible) override;

  /// Get display visibility of a owned subject hierarchy item
  /// \return Display visibility (0: hidden, 1: shown, 2: partially shown)
  int getDisplayVisibility(vtkIdType itemID)const override;

  /// Get visibility context menu item actions to add to tree view.
  /// These item visibility context menu actions can be shown in the implementations of \sa showVisibilityContextMenuActionsForItem
  QList<QAction*> visibilityContextMenuActions()const override;

  /// Show visibility context menu actions valid for a given subject hierarchy item.
  /// \param itemID Subject Hierarchy item to show the visibility context menu items for
  void showVisibilityContextMenuActionsForItem(vtkIdType itemID) override;

  /// Show an item in a selected view.
  /// Calls Volumes plugin's showItemInView implementation.
  bool showItemInView(vtkIdType itemID, vtkDMMLAbstractViewNode* viewNode, vtkIdList* allItemsToShow) override;

public:
  /// Show labelmap in all slice views. The argument node replaces any labelmap shown on the label layer
  void showLabelMapInAllViews(vtkDMMLLabelMapVolumeNode* node);

protected slots:
  /// Toggle labelmap outline visibility in the slice views
  void toggle2DOutlineVisibility(bool checked);

protected:
  QScopedPointer<qCjyxSubjectHierarchyLabelMapsPluginPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qCjyxSubjectHierarchyLabelMapsPlugin);
  Q_DISABLE_COPY(qCjyxSubjectHierarchyLabelMapsPlugin);
};

#endif
