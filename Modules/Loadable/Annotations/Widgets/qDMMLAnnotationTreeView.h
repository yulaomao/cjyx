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

#ifndef __qDMMLAnnotationTreeView_h
#define __qDMMLAnnotationTreeView_h

// qDMML includes
#include "qDMMLTreeView.h"

// Annotations includes
#include "qCjyxAnnotationsModuleWidgetsExport.h"
class qDMMLAnnotationTreeViewPrivate;
class qDMMLSceneAnnotationModel;

// Logic includes
class vtkCjyxAnnotationModuleLogic;

// DMML includes
class vtkDMMLNode;
class vtkDMMLScene;

// VTK includes
class vtkCollection;

/// \ingroup Cjyx_QtModules_Annotation
class Q_CJYX_MODULE_ANNOTATIONS_WIDGETS_EXPORT qDMMLAnnotationTreeView
  : public qDMMLTreeView
{
  Q_OBJECT

public:
  typedef qDMMLTreeView Superclass;
  qDMMLAnnotationTreeView(QWidget *parent=nullptr);
  ~qDMMLAnnotationTreeView() override;

  void hideScene();

  const char* firstSelectedNode();

  // Register the logic
  void setLogic(vtkCjyxAnnotationModuleLogic* logic);


  void toggleLockForSelected();

  void toggleVisibilityForSelected();

  void deleteSelected();

  void selectedAsCollection(vtkCollection* collection);

  qDMMLSceneAnnotationModel* annotationModel()const;

signals:
  void onPropertyEditButtonClicked(QString id);

protected slots:
  void onClicked(const QModelIndex& index);

protected:
  QScopedPointer<qDMMLAnnotationTreeViewPrivate> d_ptr;
#ifndef QT_NO_CURSOR
  void mouseMoveEvent(QMouseEvent* e) override;
  bool viewportEvent(QEvent* e) override;
#endif
  void mousePressEvent(QMouseEvent* event) override;

  void toggleVisibility(const QModelIndex& index) override;

  /// Reimplemented to also set the active hierarchy node when the current
  /// index changes.
  /// \sa qDMMLTreeView::onSelectionChanged(),
  /// vtkCjyxAnnotationModuleLogic::SetActiveHierarchyNodeID()
  void onSelectionChanged(const QItemSelection & selected,
                                  const QItemSelection & deselected) override;

private:
  Q_DECLARE_PRIVATE(qDMMLAnnotationTreeView);
  Q_DISABLE_COPY(qDMMLAnnotationTreeView);

  vtkCjyxAnnotationModuleLogic* m_Logic;

  // toggle un-/lock of an annotation
  void onLockColumnClicked(vtkDMMLNode* node);

};

#endif
