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

#ifndef __qDMMLLayoutManager_p_h
#define __qDMMLLayoutManager_p_h

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

/// Qt includes
#include <QHash>
#include <QObject>

/// CTK includes
#include <ctkVTKObject.h>
#include <ctkLayoutManager_p.h>

// qDMML includes
#include "qDMMLWidgetsConfigure.h" // For DMML_WIDGETS_HAVE_WEBENGINE_SUPPORT
#include "qDMMLLayoutManager.h"
#include "qDMMLLayoutViewFactory.h"

// DMMLLogic includes
#include <vtkDMMLLayoutLogic.h>

// VTK includes
#include <vtkSmartPointer.h>

class QLayout;
class QGridLayout;
class QButtonGroup;
class qDMMLSliceWidget;
class qDMMLTableView;
class qDMMLTableWidget;
class qDMMLPlotView;
class qDMMLPlotWidget;
class qDMMLThreeDView;
class qDMMLThreeDWidget;
class vtkCollection;
class vtkObject;
class vtkDMMLLayoutLogic;
class vtkDMMLLayoutNode;
class vtkDMMLTableViewNode;
class vtkDMMLPlotViewNode;
class vtkDMMLViewNode;
class vtkDMMLSliceNode;
class vtkXMLDataElement;

//-----------------------------------------------------------------------------
class QDMML_WIDGETS_EXPORT qDMMLLayoutManagerPrivate
  : public QObject
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qDMMLLayoutManager);
protected:
  qDMMLLayoutManager* const q_ptr;

public:
  qDMMLLayoutManagerPrivate(qDMMLLayoutManager& object);
  ~qDMMLLayoutManagerPrivate() override;

  virtual void init();

  void setDMMLLayoutNode(vtkDMMLLayoutNode* node);
  void setActiveDMMLThreeDViewNode(vtkDMMLViewNode * node);
  void setActiveDMMLTableViewNode(vtkDMMLTableViewNode * node);
  void setActiveDMMLPlotViewNode(vtkDMMLPlotViewNode * node);

  /// Enable/disable paint event associated with the TargetWidget
  //bool startUpdateLayout();
  //void endUpdateLayout(bool updateEnabled);

  /// Refresh the viewport with the current layout from the layout
  /// layout node. Empty the view if there is no layout node.
  void updateLayoutInternal();

  void setLayoutNumberOfCompareViewRowsInternal(int num);
  void setLayoutNumberOfCompareViewColumnsInternal(int num);

  /// Convenient function allowing to get a reference to the renderView widget
  /// identified by \a renderViewName.
  qDMMLThreeDWidget* threeDWidget(vtkDMMLViewNode* node)const;
  qDMMLTableWidget* tableWidget(vtkDMMLTableViewNode* node)const;
  qDMMLPlotWidget* plotWidget(vtkDMMLPlotViewNode* node)const;

  /// Convenient function allowing to get a reference to the sliceView widget
  /// identified by \a sliceViewName
  qDMMLSliceWidget* sliceWidget(vtkDMMLSliceNode* node)const;

  vtkDMMLNode* viewNode(QWidget* )const;
  QWidget* viewWidget(vtkDMMLNode* )const;

public slots:
  /// Handle DMML scene event
  void onNodeAddedEvent(vtkObject* scene, vtkObject* node);
  void onNodeRemovedEvent(vtkObject* scene, vtkObject* node);
  void onSceneAboutToBeClosedEvent();
  void onSceneClosedEvent();
  void onSceneRestoredEvent();

  /// Handle Layout node event
  void onLayoutNodeModifiedEvent(vtkObject* layoutNode);
  void updateLayoutFromDMMLScene();

  void onActiveThreeDViewNodeChanged(vtkDMMLAbstractViewNode*);
  void onActiveTableViewNodeChanged(vtkDMMLAbstractViewNode*);
  void onActivePlotViewNodeChanged(vtkDMMLAbstractViewNode*);

  /// Show segmentation controls in slice widgets only if there is at
  /// least one segmentation node in the scene
  void updateSegmentationControls();

public:
  bool                    Enabled;
  vtkDMMLScene*           DMMLScene;
  vtkDMMLLayoutNode*      DMMLLayoutNode;
  vtkDMMLLayoutLogic*     DMMLLayoutLogic;
  vtkDMMLViewNode*        ActiveDMMLThreeDViewNode;
  vtkDMMLTableViewNode*   ActiveDMMLTableViewNode;
  vtkDMMLPlotViewNode*    ActiveDMMLPlotViewNode;
protected:
  void showWidget(QWidget* widget);
};

//------------------------------------------------------------------------------
class QDMML_WIDGETS_EXPORT qDMMLLayoutThreeDViewFactory
  : public qDMMLLayoutViewFactory
{
  Q_OBJECT
public:
  typedef qDMMLLayoutViewFactory Superclass;
  qDMMLLayoutThreeDViewFactory(QObject* parent = nullptr);
  ~qDMMLLayoutThreeDViewFactory() override;

  QString viewClassName()const override;

  vtkCollection* viewLogics()const;

protected:
  QWidget* createViewFromNode(vtkDMMLAbstractViewNode* viewNode) override;
  void deleteView(vtkDMMLAbstractViewNode* viewNode) override;

  vtkCollection* ViewLogics;
};

//------------------------------------------------------------------------------
class QDMML_WIDGETS_EXPORT qDMMLLayoutTableViewFactory
  : public qDMMLLayoutViewFactory
{
  Q_OBJECT
public:
  typedef qDMMLLayoutViewFactory Superclass;
  qDMMLLayoutTableViewFactory(QObject* parent = nullptr);

  QString viewClassName()const override;

protected:
  QWidget* createViewFromNode(vtkDMMLAbstractViewNode* viewNode) override;
};

//------------------------------------------------------------------------------
class QDMML_WIDGETS_EXPORT qDMMLLayoutPlotViewFactory
  : public qDMMLLayoutViewFactory
{
  Q_OBJECT
public:
  typedef qDMMLLayoutViewFactory Superclass;
  qDMMLLayoutPlotViewFactory(QObject* parent = nullptr);

  QString viewClassName()const override;

  vtkDMMLColorLogic* colorLogic()const;
  void setColorLogic(vtkDMMLColorLogic* colorLogic);

protected:
  QWidget* createViewFromNode(vtkDMMLAbstractViewNode* viewNode) override;
  vtkDMMLColorLogic* ColorLogic{nullptr};
};

//------------------------------------------------------------------------------
class QDMML_WIDGETS_EXPORT qDMMLLayoutSliceViewFactory
  : public qDMMLLayoutViewFactory
{
  Q_OBJECT
public:
  typedef qDMMLLayoutViewFactory Superclass;
  qDMMLLayoutSliceViewFactory(QObject* parent = nullptr);
  ~qDMMLLayoutSliceViewFactory() override;

  QString viewClassName()const override;

  vtkCollection* sliceLogics()const;

protected:
  QWidget* createViewFromNode(vtkDMMLAbstractViewNode* viewNode) override;
  void deleteView(vtkDMMLAbstractViewNode* viewNode) override;

  QButtonGroup* SliceControllerButtonGroup;
  vtkCollection* SliceLogics;
};

#endif
