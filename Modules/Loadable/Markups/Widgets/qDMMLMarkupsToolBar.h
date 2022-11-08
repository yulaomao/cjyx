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

==============================================================================*/

#ifndef __qDMMLMarkupsToolBar_h
#define __qDMMLMarkupsToolBar_h

// Qt includes
#include <QSignalMapper>
#include <QToolBar>
#include <QMenu>
#include <QCheckBox>


// CTK includes
#include <ctkPimpl.h>
// no ui begin
#include <ctkVTKObject.h>
// no ui end

// qDMMLWidget includes
#include "qDMMLWidget.h"
#include "qCjyxMarkupsModuleWidgetsExport.h"

// DMML includes
#include <vtkDMMLScene.h>

// DMML includes
#include <vtkCjyxMarkupsLogic.h>

class qDMMLMarkupsToolBarPrivate;
class vtkDMMLNode;
class vtkDMMLScene;
class vtkDMMLMarkupsNode;
class vtkDMMLInteractionNode;
class vtkDMMLSelectionNode;
class vtkCjyxApplicationLogic;

class QAction;
class QActionGroup;
class QToolButton;

class Q_CJYX_MODULE_MARKUPS_WIDGETS_EXPORT qDMMLMarkupsToolBar : public QToolBar
{
  Q_OBJECT
  QVTK_OBJECT
public:
  typedef QToolBar Superclass;

  /// Constructor
  /// Title is the name of the toolbar (can appear using right click on the toolbar area)
  qDMMLMarkupsToolBar(const QString& title, QWidget* parent = 0);
  qDMMLMarkupsToolBar(QWidget* parent = 0);
  ~qDMMLMarkupsToolBar() override;

  vtkDMMLMarkupsNode* activeMarkupsNode();
  Q_INVOKABLE vtkDMMLInteractionNode* interactionNode()const;
  Q_INVOKABLE vtkDMMLSelectionNode* selectionNode()const;

  void initializeToolBarLayout();
  void addCreateNodeShortcut(QString keySequence);
  void addTogglePersistenceShortcut(QString keySequence);
  void addPlacePointShortcut(QString keySequence);

public slots:
  void setApplicationLogic(vtkCjyxApplicationLogic* logic);
  virtual void setDMMLScene(vtkDMMLScene* newScene);
  void updateToolBarLayout();

  /// Set the currently selected markups node.
  void setActiveMarkupsNode(vtkDMMLMarkupsNode* newActiveMarkupsNode);
  void setPersistence(bool persistent);
  void setInteractionNode(vtkDMMLInteractionNode* interactionNode);
  void interactionModeActionTriggered(bool);
  void setSelectionNode(vtkDMMLSelectionNode* selectionNode);

    /// Create markup by class.
  void onAddNewMarkupsNodeByClass(const QString& className);
  void onAddNewAnnotationNodeByClass(const QString& className);

  // Keyboard shortcuts for Markups node interactions
  void onCreateNodeShortcut();
  void onTogglePersistenceShortcut();
  void onPlacePointShortcut();

signals:
  void dmmlSceneChanged(vtkDMMLScene*);
  void activeMarkupsNodeChanged(vtkDMMLNode* activeMarkupsNode);
  void activeMarkupsPlaceModeChanged(bool enabled);

protected slots:
  /// Update the widget when a different markups node is selected by the combo box.
  void onMarkupsNodeChanged(vtkDMMLNode*);

protected:
  QScopedPointer<qDMMLMarkupsToolBarPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLMarkupsToolBar);
  Q_DISABLE_COPY(qDMMLMarkupsToolBar);
};

#endif
