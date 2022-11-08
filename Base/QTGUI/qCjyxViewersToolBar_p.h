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

#ifndef __qCjyxViewersToolBar_p_h
#define __qCjyxViewersToolBar_p_h

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
#include "qCjyxViewersToolBar.h"

// DMMLLogic includes
#include <vtkDMMLApplicationLogic.h>

// DMML includes
#include <vtkDMMLScene.h>
#include <vtkDMMLCrosshairNode.h>

// VTK includes
#include <vtkSmartPointer.h>

class qCjyxViewersToolBarPrivate;
class QAction;
class QActionGroup;
class QToolButton;

class qCjyxViewersToolBarPrivate: public QObject
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qCjyxViewersToolBar);

protected:
  qCjyxViewersToolBar* const q_ptr;

public:
  qCjyxViewersToolBarPrivate(qCjyxViewersToolBar& object);

  void init();
  void setDMMLScene(vtkDMMLScene* newScene);

public slots:

  void OnDMMLSceneStartClose();
  void OnDMMLSceneEndImport();
  void OnDMMLSceneEndClose();
  void onCrosshairNodeModeChangedEvent();
  void onSliceDisplayNodeChangedEvent();
  void updateWidgetFromDMML();

  void setCrosshairMode(int);
  void setCrosshairEnabled(bool); // used to toggle between last style and off
  void setCrosshairThickness(int);
  void setCrosshairJumpSlicesMode(int);

  void setIntersectingSlicesIntersectionMode(int);
  void setIntersectingSlicesVisibility(bool);
  void setIntersectingSlicesLineThicknessMode(int);
  void setIntersectingSlicesInteractive(bool);
  void setIntersectingSlicesRotationEnabled(bool);
  void setIntersectingSlicesTranslationEnabled(bool);

public:
  vtkSmartPointer<vtkDMMLScene> DMMLScene;
  vtkSmartPointer<vtkDMMLApplicationLogic> DMMLAppLogic;

  /// Crosshair
  QToolButton* CrosshairToolButton{nullptr};
  QMenu* CrosshairMenu{nullptr};

  ctkSignalMapper* CrosshairJumpSlicesMapper{nullptr};
  QAction* CrosshairJumpSlicesDisabledAction{nullptr};
  QAction* CrosshairJumpSlicesOffsetAction{nullptr};
  QAction* CrosshairJumpSlicesCenteredAction{nullptr};

  ctkSignalMapper* CrosshairMapper{nullptr};
  QAction* CrosshairNoAction{nullptr};
  QAction* CrosshairBasicAction{nullptr};
  QAction* CrosshairBasicIntersectionAction{nullptr};
  QAction* CrosshairSmallBasicAction{nullptr};
  QAction* CrosshairSmallBasicIntersectionAction{nullptr};

  ctkSignalMapper* CrosshairThicknessMapper{nullptr};
  QAction* CrosshairFineAction{nullptr};
  QAction* CrosshairMediumAction{nullptr};
  QAction* CrosshairThickAction{nullptr};

  QAction* CrosshairToggleAction{nullptr};


  QToolButton* SliceIntersectionsToolButton{nullptr};
  QMenu* SliceIntersectionsMenu{nullptr};

  ctkSignalMapper* SliceIntersectionsMapper{nullptr};
  QAction* SliceIntersectionsFullIntersectionAction{nullptr};
  QAction* SliceIntersectionsSkipIntersectionAction{nullptr};

  QAction* IntersectingSlicesVisibleAction{nullptr};
  QAction* IntersectingSlicesInteractiveAction{nullptr};
  QAction* IntersectingSlicesTranslationEnabledAction{nullptr};
  QAction* IntersectingSlicesRotationEnabledAction{nullptr};
  QMenu* IntersectingSlicesInteractionModesMenu{nullptr};

  ctkSignalMapper* SliceIntersectionsThicknessMapper{nullptr};
  QAction* SliceIntersectionsFineAction{nullptr};
  QAction* SliceIntersectionsMediumAction{nullptr};
  QAction* SliceIntersectionsThickAction{nullptr};

  int CrosshairLastMode{vtkDMMLCrosshairNode::ShowBasic};
};

#endif
