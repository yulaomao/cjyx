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
#ifndef __qDMMLMarkupsToolBar_p_h
#define __qDMMLMarkupsToolBar_p_h

// Qt includes
#include <QDebug>
#include <QToolButton>
#include <QMenu>
#include <QCheckBox>

// CTK includes
#include <ctkPimpl.h>
#include <ctkSignalMapper.h>
#include <ctkVTKObject.h>
#include "qCjyxBaseQTGUIExport.h"

// Cjyx includes
#include "qCjyxCoreApplication.h"
#include "qCjyxApplication.h"
#include "qCjyxLayoutManager.h"

// DMML includes
#include "qDMMLNodeComboBox.h"
#include "qDMMLThreeDView.h"
#include "qDMMLThreeDWidget.h"
#include "qDMMLSliceView.h"
#include "qDMMLSliceWidget.h"
#include <vtkDMMLScene.h>
#include <vtkDMMLInteractionNode.h>
#include <vtkDMMLLayoutLogic.h>
#include <vtkDMMLSelectionNode.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLViewNode.h>
#include <vtkDMMLWindowLevelWidget.h>
#include <qDMMLMarkupsToolBar.h>
#include <qCjyxMarkupsPlaceWidget.h>

//Logic includes
#include <vtkCjyxApplicationLogic.h>
#include <vtkDMMLApplicationLogic.h>
#include <vtkCjyxMarkupsLogic.h>

// VTK includes
#include <vtkWeakPointer.h>
#include <vtkSmartPointer.h>

#include "qCjyxMouseModeToolBar_p.h"

class qDMMLMarkupsToolBarPrivate;
class QAction;
class QActionGroup;
class QToolButton;

//-----------------------------------------------------------------------------
class qDMMLMarkupsToolBarPrivate : public QObject
{
  Q_OBJECT
    QVTK_OBJECT
    Q_DECLARE_PUBLIC(qDMMLMarkupsToolBar);

protected:
  qDMMLMarkupsToolBar* const q_ptr;

public:
  qDMMLMarkupsToolBarPrivate(qDMMLMarkupsToolBar& object);
  void init();
  void setDMMLScene(vtkDMMLScene* newScene);
  QCursor cursorFromIcon(QIcon& icon);
  void addSetModuleButton(vtkCjyxMarkupsLogic* markupsLogic, const QString& moduleName);

public slots:
  void onDMMLSceneStartBatchProcess();
  void onDMMLSceneEndBatchProcess();
  void updateWidgetFromDMML();
  void onActivePlaceNodeClassNameChangedEvent();
  void onPlaceNodeClassNameListModifiedEvent();
  void onSetModule(const QString& moduleName);

public:
  vtkSmartPointer<vtkDMMLScene>            DMMLScene;
  vtkSmartPointer<vtkDMMLApplicationLogic> DMMLAppLogic;
  vtkWeakPointer<vtkDMMLInteractionNode>   InteractionNode;
  vtkWeakPointer<vtkDMMLSelectionNode>     SelectionNode;

  QString DefaultPlaceClassName;
  QAction* NodeSelectorAction{nullptr};
  qDMMLNodeComboBox* MarkupsNodeSelector{nullptr};
  qCjyxMarkupsPlaceWidget* MarkupsPlaceWidget{nullptr};
};

#endif