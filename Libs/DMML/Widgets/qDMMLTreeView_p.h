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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qDMMLTreeView_p_h
#define __qDMMLTreeView_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Cjyx API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// Qt includes
class QAction;
class QMenu;

// DMML includes
class vtkDMMLHierarchyNode;

// DMMLWidgets includes
#include "qDMMLTreeView.h"
class qDMMLSceneModel;
class qDMMLSortFilterProxyModel;

// VTK includes
class vtkCollection;

//------------------------------------------------------------------------------
class QDMML_WIDGETS_EXPORT qDMMLTreeViewPrivate
{
  Q_DECLARE_PUBLIC(qDMMLTreeView);
protected:
  qDMMLTreeView* const q_ptr;
public:
  qDMMLTreeViewPrivate(qDMMLTreeView& object);
  virtual ~qDMMLTreeViewPrivate();
  virtual void init();
  void setSceneModel(qDMMLSceneModel* newModel);
  void setSortFilterProxyModel(qDMMLSortFilterProxyModel* newSortModel);
  QSize sizeHint()const;
  void recomputeSizeHint(bool forceUpdate = false);
  /// Save the current expansion state of children nodes of a
  /// vtkDMMLDisplayableHierarchyNode
  void saveChildrenExpandState(QModelIndex& parentIndex);
  void scrollTo(const QString& name, bool next);

  qDMMLSceneModel*           SceneModel;
  qDMMLSortFilterProxyModel* SortFilterModel;
  QString                    SceneModelType;
  bool                       FitSizeToVisibleIndexes;
  mutable QSize              TreeViewSizeHint;
  QSize                      TreeViewMinSizeHint;
  bool                       ShowScene;
  bool                       ShowRootNode;
  QString                    LastScrollToName;

  QMenu*                     NodeMenu;
  QAction*                   RenameAction;
  QAction*                   DeleteAction;
  QAction*                   EditAction;
  QMenu*                     SceneMenu;

  vtkCollection*             ExpandedNodes;

};

#endif
