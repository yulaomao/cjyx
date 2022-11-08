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

#ifndef __qCjyxMouseModeToolBar_p_h
#define __qCjyxMouseModeToolBar_p_h

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
#include <QToolBar>
#include <QMenu>
#include <QCheckBox>

// CTK includes
#include <ctkPimpl.h>
#include <ctkSignalMapper.h>
#include <ctkVTKObject.h>
#include "qCjyxBaseQTGUIExport.h"

// Cjyx includes
#include "qCjyxMouseModeToolBar.h"

// DMMLLogic includes
#include <vtkDMMLApplicationLogic.h>

// DMML includes
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

class qCjyxMouseModeToolBarPrivate;
class QAction;
class QActionGroup;
class QToolButton;

class qCjyxMouseModeToolBarPrivate: public QObject
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qCjyxMouseModeToolBar);

protected:
  qCjyxMouseModeToolBar* const q_ptr;

public:
  qCjyxMouseModeToolBarPrivate(qCjyxMouseModeToolBar& object);

  void init();
  void setDMMLScene(vtkDMMLScene* newScene);

  /// update mouse cursor shape according to current interaction mode and selection
  void updateCursor();

  void updatePlaceWidget();

  QCursor cursorFromIcon(QIcon& icon);

public slots:

  void onDMMLSceneStartBatchProcess();
  void onDMMLSceneEndBatchProcess();
  void updateWidgetFromDMML();

  void onActivePlaceNodeClassNameChangedEvent();
  void onPlaceNodeClassNameListModifiedEvent();

public:
  vtkSmartPointer<vtkDMMLScene>            DMMLScene;
  vtkSmartPointer<vtkDMMLApplicationLogic> DMMLAppLogic;
  vtkWeakPointer<vtkDMMLInteractionNode>   InteractionNode;

  QAction* AdjustViewAction;
  QAction* AdjustWindowLevelAction;
  QAction* PlaceWidgetAction;
  QAction* ToolBarAction;

  QMenu* PlaceWidgetMenu;

  QAction* AdjustWindowLevelAdjustModeAction;
  QAction* AdjustWindowLevelRegionModeAction;
  QAction* AdjustWindowLevelCenteredRegionModeAction;
  QMenu* AdjustWindowLevelMenu;

  ctkSignalMapper* AdjustWindowLevelModeMapper;

  /// Place Persistence
  QAction *PersistenceAction;

  /// Group interaction modes together so that they're exclusive
  QActionGroup* InteractionModesActionGroup;

  /// Group the place actions together so that they're exclusive
  QActionGroup* PlaceModesActionGroup;

  QString DefaultPlaceClassName;
};

#endif
