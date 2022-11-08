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

#ifndef __qDMMLLayoutManager_h
#define __qDMMLLayoutManager_h

// Qt includes
#include <QStringList>
class QWidget;

// CTK includes
#include <ctkLayoutFactory.h>

// qDMML includes
#include "qDMMLWidgetsExport.h"

class qDMMLPlotWidget;
class qDMMLTableWidget;
class qDMMLThreeDWidget;
class qDMMLSliceWidget;
class qDMMLLayoutManagerPrivate;
class qDMMLLayoutViewFactory;

class vtkDMMLAbstractViewNode;
class vtkDMMLColorLogic;
class vtkDMMLLayoutLogic;
class vtkDMMLScene;
class vtkDMMLNode;
class vtkDMMLTableNode;
class vtkDMMLTableViewNode;
class vtkDMMLViewNode;
class vtkDMMLPlotViewNode;

class vtkCollection;
class vtkRenderer;

/// DMML layout manager that instantiates the Qt widgets and layouts from the
/// layout node and the view nodes in the scene.
/// The management (creation, configuration and deletion) of the view widgets
/// are delegated to qDMMLLayoutViewFactory.
/// Example to change a default factory:
/// \code
/// qDMMLLayoutSliceViewFactory* dmmlSliceViewFactory =
///   qobject_cast<qDMMLLayoutSliceViewFactory*>(
///     app->layoutManager()->dmmlViewFactory("vtkDMMLSliceNode"));
/// MySliceViewFactory* mySliceViewFactory =
///   new MySliceViewFactory(app->layoutManager());
/// mySliceViewFactory->setSliceLogics(dmmlSliceViewFactory->sliceLogics());
///
/// app->layoutManager()->unregisterViewFactory(dmmlSliceViewFactory);
/// app->layoutManager()->registerViewFactory(mySliceViewFactory);
/// \endcode
/// You can also register ctkLayoutViewFactories that are not related to DMML
/// view nodes.
/// \sa ctkLayoutFactory, ctkLayoutManager, qDMMLLayoutViewFactory
class QDMML_WIDGETS_EXPORT qDMMLLayoutManager : public ctkLayoutFactory
{
  Q_OBJECT

  /// This property controls whether the layout manager reacts to layout node
  /// changes or note. When enabled (default), the layout is updated each time
  /// the layout node is modified and when the scene leaves batch-process state.
  /// It can be useful to temporarily disable the manager when loading a scene,
  /// it could otherwise change the layout.
  /// \sa isEnabled(), setEnabled(), setDMMLScene()
  Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)
  // The following properties are exposed so that they are available within python
  Q_PROPERTY(int layout READ layout WRITE setLayout NOTIFY layoutChanged DESIGNABLE false)
  Q_PROPERTY(int threeDViewCount READ threeDViewCount DESIGNABLE false)
  Q_PROPERTY(int tableViewCount READ tableViewCount DESIGNABLE false)
  Q_PROPERTY(int plotViewCount READ plotViewCount DESIGNABLE false)

public:
  /// Superclass typedef
  typedef ctkLayoutFactory Superclass;

  /// Constructors
  explicit qDMMLLayoutManager(QObject* parent=nullptr);
  explicit qDMMLLayoutManager(QWidget* viewport, QObject* parent);
  ~qDMMLLayoutManager() override;

  /// Return the enabled property value.
  /// \sa enabled
  bool isEnabled()const;

  /// Reimplemented for internal reasons.
  /// If the registered view factory is a qDMMLLayoutViewFactory, then set
  /// its layoutManager and its dmmlScene.
  /// \sa ctkLayoutFactory::registerViewFactory(), unregisterViewFactory()
  Q_INVOKABLE virtual void registerViewFactory(ctkLayoutViewFactory* viewFactory);

  /// Return the list of registered DMML view factories.
  /// \sa registeredViewFactories(), registerViewFactory(),
  /// unregisterViewFactory()
  Q_INVOKABLE QList<qDMMLLayoutViewFactory*> dmmlViewFactories()const;

  /// Return the view factory that handles the viewClassName view nodes.
  /// This can be used to replace a view factory with another one.
  /// \sa dmmlViewFactories(), registerViewFactory(), unregisterViewFactory()
  Q_INVOKABLE qDMMLLayoutViewFactory* dmmlViewFactory(const QString& viewClassName)const;

  /// Return the dmml scene of the layout manager. It is the scene that is set
  /// by setDMMLScene().
  /// \sa setDMMLScene(), enabled
  Q_INVOKABLE vtkDMMLScene* dmmlScene()const;

  /// Get the view widget representing a particular node (can be used
  /// for SliceNodes or ViewNodes, returning qDMMLSliceWidget or
  /// qDMMLThreeDWidget respectively).
  Q_INVOKABLE QWidget* viewWidget(vtkDMMLNode* n) const;

  /// Get slice view widget identified by \a name
  Q_INVOKABLE qDMMLSliceWidget* sliceWidget(const QString& name)const;

  /// Get 3D widget identified by \a name
  Q_INVOKABLE qDMMLThreeDWidget* threeDWidget(const QString& name)const;

  /// Get the list of SliceWidgetNames
  /// All slice widget names are returned,
  /// even those from hidden and not currently used widgets.
  Q_INVOKABLE QStringList sliceViewNames() const;

  /// Return the number of instantiated ThreeDRenderView
  int threeDViewCount()const;
  int tableViewCount()const;
  int plotViewCount()const;

  /// Get ThreeDWidget identified by \a id
  /// where \a id is an integer ranging from 0 to N-1 with N being the number
  /// of instantiated qDMMLThreeDView (that should also be equal to the number
  /// of vtkDMMLViewNode)
  Q_INVOKABLE qDMMLThreeDWidget* threeDWidget(int id)const;
  Q_INVOKABLE qDMMLTableWidget* tableWidget(int id)const;
  Q_INVOKABLE qDMMLPlotWidget* plotWidget(int id)const;

  /// Return the up-to-date list of vtkDMMLSliceLogics associated to the slice views.
  /// The returned collection object is owned by the layout manager.
  Q_INVOKABLE vtkCollection* dmmlSliceLogics()const;

  /// Return the up-to-date list of vtkDMMLViewLogics associated to the threeD views.
  /// The returned collection object is owned by the layout manager.
  Q_INVOKABLE vtkCollection* dmmlViewLogics()const;

  Q_INVOKABLE void setDMMLColorLogic(vtkDMMLColorLogic* colorLogic);
  Q_INVOKABLE vtkDMMLColorLogic* dmmlColorLogic()const;

  /// Returns the current layout. it's the same value than
  /// vtkDMMLLayoutNode::ViewArrangement
  /// \sa vtkDMMLLayoutNode::CjyxLayout, layoutLogic()
  int layout()const;

  /// Return the view node that is temporarily shown maximized in the view layout.
  Q_INVOKABLE vtkDMMLAbstractViewNode* maximizedViewNode();

  /// Return the layout logic instantiated and used by the manager.
  /// \sa setLayout(), layout()
  Q_INVOKABLE vtkDMMLLayoutLogic* layoutLogic()const;

  /// Return the view node of the active 3D view.
  /// \todo For now the active view is the first 3D view.
  /// \sa activeThreeDRenderer(), activeDMMLPlotViewNode(),
  /// activePlotRenderer()
  Q_INVOKABLE vtkDMMLViewNode* activeDMMLThreeDViewNode()const;
  /// Return the renderer of the active 3D view.
  /// \todo For now the active view is the first 3D view.
  /// \sa  activeThreeDRenderer(), activeDMMLPlotViewNode(),
  /// activePlotRenderer()
  Q_INVOKABLE vtkRenderer* activeThreeDRenderer()const;
  /// Return the view node of the active table view.
  /// \todo For now the active view is the first table view.
  /// \sa  activeTableRenderer(), activeDMMLThreeDViewNode(),
  /// activeThreeDRenderer()
  Q_INVOKABLE vtkDMMLTableViewNode* activeDMMLTableViewNode()const;
  /// Return the renderer of the active table view.
  /// \todo For now the active view is the first table view.
  /// \sa  activeDMMLTableViewNode(), activeDMMLThreeDViewNode(),
  /// activeThreeDRenderer()
  Q_INVOKABLE vtkRenderer* activeTableRenderer()const;
  /// Return the view node of the active plot view.
  /// \todo For now the active view is the first plot view.
  /// \sa  activePlotRenderer(), activeDMMLThreeDViewNode(),
  /// activeThreeDRenderer()
  Q_INVOKABLE vtkDMMLPlotViewNode* activeDMMLPlotViewNode()const;
  /// Return the renderer of the active plot view.
  /// \todo For now the active view is the first plot view.
  /// \sa  activeDMMLPlotViewNode(), activeDMMLThreeDViewNode(),
  /// activeThreeDRenderer()
  Q_INVOKABLE vtkRenderer* activePlotRenderer()const;


public slots:
  /// Set the enabled property value
  /// \sa enabled
  void setEnabled(bool enable);

  /// Set the DMML \a scene that should be listened for events
  /// \sa dmmlScene(), enabled
  void setDMMLScene(vtkDMMLScene* scene);

  /// Change the current layout (see vtkDMMLLayoutNode::CjyxLayout)
  /// It creates views if needed.
  void setLayout(int newLayout);

  /// Makes a view displayed maximized (taking the entire area) of the view layout.
  /// Setting the value to nullptr restores the original view layout.
  void setMaximizedViewNode(vtkDMMLAbstractViewNode* viewNode);

  /// Change the number of viewers in comparison modes
  /// It creates views if needed.
  void setLayoutNumberOfCompareViewRows(int num);
  void setLayoutNumberOfCompareViewColumns(int num);

  /// Reset focal view around volumes
  /// \sa ctkVTKRenderView::resetFocalPoint(), ctkVTKRenderView::resetCamera()
  void resetThreeDViews();

  /// Reset focal view around volumes
  /// \sa qDMMLSliceControllerWidget::fitSliceToBackground(), vtkDMMLSliceLogic::FitSliceToAll()
  void resetSliceViews();

  /// Calls setPauseRender(pause) on all slice and 3D views
  /// Tracks the previous pause state which is restored using resumeRender()
  /// Each pauseRender(true) should always be accompanied by a corresponding pauseRender(false) call
  /// \sa pauseRender
  void setRenderPaused(bool pause);

  /// Equivalent to setRenderPaused(true)
  /// \sa setRenderPaused
  void pauseRender();

  /// Equivalent to setRenderPaused(false)
  /// \sa setRenderPaused
  void resumeRender();

signals:
  void activeDMMLThreeDViewNodeChanged(vtkDMMLViewNode* newActiveDMMLThreeDViewNode);
  void activeDMMLTableViewNodeChanged(vtkDMMLTableViewNode* newActiveDMMLTableViewNode);
  void activeDMMLPlotViewNodeChanged(vtkDMMLPlotViewNode* newActiveDMMLPlotViewNode);
  void activeThreeDRendererChanged(vtkRenderer* newRenderer);
  void activeTableRendererChanged(vtkRenderer* newRenderer);
  void activePlotRendererChanged(vtkRenderer* newRenderer);
  void layoutChanged(int);

  /// Signal emitted when editing of a node is requested from within the layout
  void nodeAboutToBeEdited(vtkDMMLNode* node);

protected:
  QScopedPointer<qDMMLLayoutManagerPrivate> d_ptr;
  qDMMLLayoutManager(qDMMLLayoutManagerPrivate* obj, QWidget* viewport, QObject* parent);

  void onViewportChanged() override;

  using ctkLayoutManager::setLayout;
private:
  Q_DECLARE_PRIVATE(qDMMLLayoutManager);
  Q_DISABLE_COPY(qDMMLLayoutManager);
};

#endif
