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

// Qt includes
#include <QButtonGroup>
#include <QDebug>

// DMMLWidgets includes
#include <qDMMLWidgetsConfigure.h> // For DMML_WIDGETS_HAVE_WEBENGINE_SUPPORT
#include <qDMMLLayoutManager_p.h>
#include <qDMMLSliceView.h>
#include <qDMMLSliceWidget.h>
#include <qDMMLSliceControllerWidget.h>
#include <qDMMLTableView.h>
#include <qDMMLTableWidget.h>
#include <qDMMLPlotView.h>
#include <qDMMLPlotWidget.h>
#include <qDMMLThreeDView.h>
#include <qDMMLThreeDWidget.h>

// DMMLLogic includes
#include <vtkDMMLApplicationLogic.h>
#include <vtkDMMLColorLogic.h>
#include <vtkDMMLSliceLogic.h>
#include <vtkDMMLViewLogic.h>
#include <vtkDMMLSliceViewDisplayableManagerFactory.h>

// DMML includes
#include <vtkDMMLLayoutNode.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLTableViewNode.h>
#include <vtkDMMLPlotViewNode.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkCollection.h>

//------------------------------------------------------------------------------
// Factory methods

//------------------------------------------------------------------------------
// qDMMLLayoutThreeDViewFactory
//------------------------------------------------------------------------------
qDMMLLayoutThreeDViewFactory::qDMMLLayoutThreeDViewFactory(QObject* parent)
  : qDMMLLayoutViewFactory(parent)
{
  this->ViewLogics = vtkCollection::New();
}

//------------------------------------------------------------------------------
qDMMLLayoutThreeDViewFactory::~qDMMLLayoutThreeDViewFactory()
{
  this->ViewLogics->Delete();
  this->ViewLogics = nullptr;
}

//------------------------------------------------------------------------------
QString qDMMLLayoutThreeDViewFactory::viewClassName()const
{
  return "vtkDMMLViewNode";
}

//------------------------------------------------------------------------------
vtkCollection* qDMMLLayoutThreeDViewFactory::viewLogics() const
{
  return this->ViewLogics;
}

//------------------------------------------------------------------------------
QWidget* qDMMLLayoutThreeDViewFactory::createViewFromNode(vtkDMMLAbstractViewNode* viewNode)
{
  if (!viewNode || !this->layoutManager() || !this->layoutManager()->viewport())
    {
    Q_ASSERT(viewNode);
    return nullptr;
    }

  // There must be a unique ThreeDWidget per node
  Q_ASSERT(!this->viewWidget(viewNode));

  qDMMLThreeDWidget* threeDWidget = new qDMMLThreeDWidget(this->layoutManager()->viewport());
  threeDWidget->setObjectName(QString("ThreeDWidget%1").arg(viewNode->GetLayoutName()));
  threeDWidget->setDMMLScene(this->dmmlScene());
  threeDWidget->setDMMLViewNode(vtkDMMLViewNode::SafeDownCast(viewNode));

  this->viewLogics()->AddItem(threeDWidget->viewLogic());

  return threeDWidget;
}

//------------------------------------------------------------------------------
void qDMMLLayoutThreeDViewFactory::deleteView(vtkDMMLAbstractViewNode* viewNode)
{
  qDMMLThreeDWidget* threeDWidget =
    qobject_cast<qDMMLThreeDWidget*>(this->viewWidget(viewNode));
  if (threeDWidget)
    {
    this->viewLogics()->RemoveItem(threeDWidget->viewLogic());
    }
  this->Superclass::deleteView(viewNode);
}

//------------------------------------------------------------------------------
// qDMMLLayoutTableViewFactory
//------------------------------------------------------------------------------
qDMMLLayoutTableViewFactory::qDMMLLayoutTableViewFactory(QObject* parent)
  : qDMMLLayoutViewFactory(parent)
{
}

//------------------------------------------------------------------------------
QString qDMMLLayoutTableViewFactory::viewClassName()const
{
  return "vtkDMMLTableViewNode";
}

//------------------------------------------------------------------------------
QWidget* qDMMLLayoutTableViewFactory::createViewFromNode(vtkDMMLAbstractViewNode* viewNode)
{
  if (!this->layoutManager() || !viewNode || !this->layoutManager()->viewport())
    {
    Q_ASSERT(viewNode);
    return nullptr;
    }

  // There must be a unique TableWidget per node
  Q_ASSERT(!this->viewWidget(viewNode));

  qDMMLTableWidget* tableWidget = new qDMMLTableWidget(this->layoutManager()->viewport());
  tableWidget->setObjectName(QString("qDMMLTableWidget%1").arg(viewNode->GetLayoutName()));
  tableWidget->setDMMLScene(this->dmmlScene());
  tableWidget->setDMMLTableViewNode(vtkDMMLTableViewNode::SafeDownCast(viewNode));

  return tableWidget;
}

//------------------------------------------------------------------------------
// qDMMLLayoutPlotViewFactory
//------------------------------------------------------------------------------
qDMMLLayoutPlotViewFactory::qDMMLLayoutPlotViewFactory(QObject* parent)
  : qDMMLLayoutViewFactory(parent)
{
}

//------------------------------------------------------------------------------
QString qDMMLLayoutPlotViewFactory::viewClassName() const
{
  return "vtkDMMLPlotViewNode";
}

//------------------------------------------------------------------------------
vtkDMMLColorLogic* qDMMLLayoutPlotViewFactory::colorLogic()const
{
  return this->ColorLogic;
}

//------------------------------------------------------------------------------
void qDMMLLayoutPlotViewFactory::setColorLogic(vtkDMMLColorLogic* colorLogic)
{
  this->ColorLogic = colorLogic;
  /*
  foreach(QWidget* view, this->registeredViews())
    {
    qDMMLPlotWidget* plotWidget = qobject_cast<qDMMLPlotWidget*>(view);
    plotWidget->setColorLogic(this->colorLogic());
    }
  */
}

//------------------------------------------------------------------------------
QWidget* qDMMLLayoutPlotViewFactory::createViewFromNode(vtkDMMLAbstractViewNode* viewNode)
{
  if (!this->layoutManager() || !viewNode || !this->layoutManager()->viewport())
    {
    Q_ASSERT(viewNode);
    return nullptr;
    }

  // There must be a unique plot widget per node
  Q_ASSERT(!this->viewWidget(viewNode));

  qDMMLPlotWidget* plotWidget = new qDMMLPlotWidget(this->layoutManager()->viewport());
  plotWidget->setObjectName(QString("qDMMLPlotWidget%1").arg(viewNode->GetLayoutName()));
  plotWidget->setDMMLScene(this->dmmlScene());
  plotWidget->setDMMLPlotViewNode(vtkDMMLPlotViewNode::SafeDownCast(viewNode));

  return plotWidget;
}

//------------------------------------------------------------------------------
// qDMMLLayoutSliceViewFactory
//------------------------------------------------------------------------------
qDMMLLayoutSliceViewFactory::qDMMLLayoutSliceViewFactory(QObject* parent)
  : qDMMLLayoutViewFactory(parent)
{
  this->SliceControllerButtonGroup = new QButtonGroup(nullptr);
  this->SliceControllerButtonGroup->setParent(this);
  this->SliceControllerButtonGroup->setExclusive(false);
  this->SliceLogics = vtkCollection::New();
}

//------------------------------------------------------------------------------
qDMMLLayoutSliceViewFactory::~qDMMLLayoutSliceViewFactory()
{
  this->SliceLogics->Delete();
  this->SliceLogics = nullptr;
}

// --------------------------------------------------------------------------
QString qDMMLLayoutSliceViewFactory::viewClassName()const
{
  return "vtkDMMLSliceNode";
}

//------------------------------------------------------------------------------
vtkCollection* qDMMLLayoutSliceViewFactory::sliceLogics()const
{
  return this->SliceLogics;
}

// --------------------------------------------------------------------------
QWidget* qDMMLLayoutSliceViewFactory::createViewFromNode(vtkDMMLAbstractViewNode* viewNode)
{
  if (!this->layoutManager() || !viewNode || !this->layoutManager()->viewport())
    {// can't create a slice widget if there is no parent widget
    Q_ASSERT(viewNode);
    return nullptr;
    }

  // there is a unique slice widget per node
  Q_ASSERT(!this->viewWidget(viewNode));

  qDMMLSliceWidget * sliceWidget = new qDMMLSliceWidget(this->layoutManager()->viewport());
  sliceWidget->sliceController()->setControllerButtonGroup(this->SliceControllerButtonGroup);
  sliceWidget->setObjectName(QString("qDMMLSliceWidget%1").arg(viewNode->GetLayoutName()));
  // set slice node before setting the scene to allow using slice node names in the slice transform, display, and model nodes
  sliceWidget->setDMMLSliceNode(vtkDMMLSliceNode::SafeDownCast(viewNode));
  sliceWidget->setDMMLScene(this->dmmlScene());
  sliceWidget->setSliceLogics(this->sliceLogics());
  this->sliceLogics()->AddItem(sliceWidget->sliceLogic());

  return sliceWidget;
}

// --------------------------------------------------------------------------
void qDMMLLayoutSliceViewFactory::deleteView(vtkDMMLAbstractViewNode* viewNode)
{
  qDMMLSliceWidget* sliceWidget =
    qobject_cast<qDMMLSliceWidget*>(this->viewWidget(viewNode));
  if (sliceWidget)
    {
    this->sliceLogics()->RemoveItem(sliceWidget->sliceLogic());
    }
  this->Superclass::deleteView(viewNode);
}

//------------------------------------------------------------------------------
// qDMMLLayoutManagerPrivate methods

//------------------------------------------------------------------------------
qDMMLLayoutManagerPrivate::qDMMLLayoutManagerPrivate(qDMMLLayoutManager& object)
  : q_ptr(&object)
{
  this->Enabled = true;
  this->DMMLScene = nullptr;
  this->DMMLLayoutNode = nullptr;
  this->DMMLLayoutLogic = vtkDMMLLayoutLogic::New();
  this->ActiveDMMLThreeDViewNode = nullptr;
  this->ActiveDMMLTableViewNode = nullptr;
  this->ActiveDMMLPlotViewNode = nullptr;
  //this->SavedCurrentViewArrangement = vtkDMMLLayoutNode::CjyxLayoutNone;
}

//------------------------------------------------------------------------------
qDMMLLayoutManagerPrivate::~qDMMLLayoutManagerPrivate()
{
  this->DMMLLayoutLogic->Delete();
  this->DMMLLayoutLogic = nullptr;
}

//------------------------------------------------------------------------------
void qDMMLLayoutManagerPrivate::init()
{
  Q_Q(qDMMLLayoutManager);

  q->setSpacing(1);

  qDMMLLayoutThreeDViewFactory* threeDViewFactory =
    new qDMMLLayoutThreeDViewFactory;
  q->registerViewFactory(threeDViewFactory);

  qDMMLLayoutSliceViewFactory* sliceViewFactory =
    new qDMMLLayoutSliceViewFactory;
  q->registerViewFactory(sliceViewFactory);

  qDMMLLayoutTableViewFactory* tableViewFactory =
    new qDMMLLayoutTableViewFactory;
  q->registerViewFactory(tableViewFactory);

  qDMMLLayoutPlotViewFactory* plotViewFactory =
    new qDMMLLayoutPlotViewFactory;
  q->registerViewFactory(plotViewFactory);
}

//------------------------------------------------------------------------------
qDMMLThreeDWidget* qDMMLLayoutManagerPrivate::threeDWidget(vtkDMMLViewNode* node)const
{
  Q_Q(const qDMMLLayoutManager);
  return qobject_cast<qDMMLThreeDWidget*>(
        q->dmmlViewFactory("vtkDMMLViewNode")->viewWidget(node));
}

//------------------------------------------------------------------------------
qDMMLSliceWidget* qDMMLLayoutManagerPrivate::sliceWidget(vtkDMMLSliceNode* node)const
{
  Q_Q(const qDMMLLayoutManager);
  return qobject_cast<qDMMLSliceWidget*>(
        q->dmmlViewFactory("vtkDMMLSliceNode")->viewWidget(node));
}

//------------------------------------------------------------------------------
qDMMLTableWidget* qDMMLLayoutManagerPrivate::tableWidget(vtkDMMLTableViewNode* node)const
{
  Q_Q(const qDMMLLayoutManager);
  return qobject_cast<qDMMLTableWidget*>(
              q->dmmlViewFactory("vtkDMMLTableViewNode")->viewWidget(node));
}

//------------------------------------------------------------------------------
qDMMLPlotWidget *qDMMLLayoutManagerPrivate::plotWidget(vtkDMMLPlotViewNode *node) const
{
  Q_Q(const qDMMLLayoutManager);
  return qobject_cast<qDMMLPlotWidget*>(
              q->dmmlViewFactory("vtkDMMLPlotViewNode")->viewWidget(node));
}

//------------------------------------------------------------------------------
vtkDMMLNode* qDMMLLayoutManagerPrivate::viewNode(QWidget* widget)const
{
  if (qobject_cast<qDMMLSliceWidget*>(widget))
    {
    return qobject_cast<qDMMLSliceWidget*>(widget)->dmmlSliceNode();
    }
  if (qobject_cast<qDMMLThreeDWidget*>(widget))
    {
    return qobject_cast<qDMMLThreeDWidget*>(widget)->dmmlViewNode();
    }
  if (qobject_cast<qDMMLTableWidget*>(widget))
    {
    return qobject_cast<qDMMLTableWidget*>(widget)->dmmlTableViewNode();
    }
  if (qobject_cast<qDMMLPlotWidget*>(widget))
    {
    return qobject_cast<qDMMLPlotWidget*>(widget)->dmmlPlotViewNode();
    }
  return nullptr;
}

//------------------------------------------------------------------------------
QWidget* qDMMLLayoutManagerPrivate::viewWidget(vtkDMMLNode* viewNode)const
{
  Q_Q(const qDMMLLayoutManager);
  if (!vtkDMMLAbstractViewNode::SafeDownCast(viewNode))
    {
    return nullptr;
    }
  QWidget* widget = nullptr;
  if (vtkDMMLSliceNode::SafeDownCast(viewNode))
    {
    widget = this->sliceWidget(vtkDMMLSliceNode::SafeDownCast(viewNode));
    }
  if (vtkDMMLViewNode::SafeDownCast(viewNode))
    {
    widget = this->threeDWidget(vtkDMMLViewNode::SafeDownCast(viewNode));
    }
  if (vtkDMMLTableViewNode::SafeDownCast(viewNode))
    {
    widget = this->tableWidget(vtkDMMLTableViewNode::SafeDownCast(viewNode));
    }
  if (vtkDMMLPlotViewNode::SafeDownCast(viewNode))
    {
    widget = this->plotWidget(vtkDMMLPlotViewNode::SafeDownCast(viewNode));
    }
  return widget ? widget : q->dmmlViewFactory(
        QString::fromUtf8(viewNode->GetClassName()))->viewWidget(
        vtkDMMLAbstractViewNode::SafeDownCast(viewNode));
}

// --------------------------------------------------------------------------
void qDMMLLayoutManagerPrivate::setDMMLLayoutNode(vtkDMMLLayoutNode* layoutNode)
{
  this->qvtkReconnect(this->DMMLLayoutNode, layoutNode, vtkCommand::ModifiedEvent,
                    this, SLOT(onLayoutNodeModifiedEvent(vtkObject*)));
  this->DMMLLayoutNode = layoutNode;
  this->onLayoutNodeModifiedEvent(layoutNode);
}

// --------------------------------------------------------------------------
void qDMMLLayoutManagerPrivate::setActiveDMMLThreeDViewNode(vtkDMMLViewNode* node)
{
  Q_Q(qDMMLLayoutManager);
  QObject::connect(q->dmmlViewFactory("vtkDMMLViewNode"),
                   SIGNAL(activeViewNodeChanged(vtkDMMLAbstractViewNode*)),
                   this, SLOT(onActiveThreeDViewNodeChanged(vtkDMMLAbstractViewNode*)),
                   Qt::UniqueConnection);

  q->dmmlViewFactory("vtkDMMLViewNode")->setActiveViewNode(node);
}

// --------------------------------------------------------------------------
void qDMMLLayoutManagerPrivate
::onActiveThreeDViewNodeChanged(vtkDMMLAbstractViewNode* node)
{
  Q_Q(qDMMLLayoutManager);
  emit q->activeThreeDRendererChanged(
    q->dmmlViewFactory("vtkDMMLViewNode")->activeRenderer());
  emit q->activeDMMLThreeDViewNodeChanged(
    vtkDMMLViewNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qDMMLLayoutManagerPrivate::setActiveDMMLTableViewNode(vtkDMMLTableViewNode* node)
{
  Q_Q(qDMMLLayoutManager);
  QObject::connect(q->dmmlViewFactory("vtkDMMLTableViewNode"),
                   SIGNAL(activeViewNodeChanged(vtkDMMLAbstractViewNode*)),
                   this, SLOT(onActiveTableViewNodeChanged(vtkDMMLAbstractViewNode*)),
                   Qt::UniqueConnection);
  q->dmmlViewFactory("vtkDMMLTableViewNode")->setActiveViewNode(node);
}

// --------------------------------------------------------------------------
void qDMMLLayoutManagerPrivate
::onActiveTableViewNodeChanged(vtkDMMLAbstractViewNode* node)
{
  Q_Q(qDMMLLayoutManager);
  emit q->activeTableRendererChanged(
    q->dmmlViewFactory("vtkDMMLTableViewNode")->activeRenderer());
  emit q->activeDMMLTableViewNodeChanged(
                vtkDMMLTableViewNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qDMMLLayoutManagerPrivate::setActiveDMMLPlotViewNode(vtkDMMLPlotViewNode* node)
{
  Q_Q(qDMMLLayoutManager);
  QObject::connect(q->dmmlViewFactory("vtkDMMLPlotViewNode"),
                   SIGNAL(activeViewNodeChanged(vtkDMMLAbstractViewNode*)),
                   this, SLOT(onActivePlotViewNodeChanged(vtkDMMLAbstractViewNode*)),
                   Qt::UniqueConnection);
  q->dmmlViewFactory("vtkDMMLPlotViewNode")->setActiveViewNode(node);
}

// --------------------------------------------------------------------------
void qDMMLLayoutManagerPrivate::
onActivePlotViewNodeChanged(vtkDMMLAbstractViewNode* node)
{
  Q_Q(qDMMLLayoutManager);
  emit q->activePlotRendererChanged(
    q->dmmlViewFactory("vtkDMMLPlotViewNode")->activeRenderer());
  emit q->activeDMMLPlotViewNodeChanged(
                vtkDMMLPlotViewNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qDMMLLayoutManagerPrivate::onNodeAddedEvent(vtkObject* scene, vtkObject* node)
{
  Q_Q(qDMMLLayoutManager);
  Q_UNUSED(scene);
  Q_ASSERT(scene);
  Q_ASSERT(scene == this->DMMLScene);
  if (!this->DMMLScene || this->DMMLScene->IsBatchProcessing())
    {
    return;
    }

  // Layout node added
  vtkDMMLLayoutNode* layoutNode = vtkDMMLLayoutNode::SafeDownCast(node);
  if (layoutNode)
    {
    //qDebug() << "qDMMLLayoutManagerPrivate::onLayoutNodeAddedEvent";
    // Only one Layout node is expected
    Q_ASSERT(this->DMMLLayoutNode == nullptr);
    if (this->DMMLLayoutNode != nullptr)
      {
      return;
      }
    this->setDMMLLayoutNode(layoutNode);
    }

  // View node added
  vtkDMMLAbstractViewNode* viewNode =
    vtkDMMLAbstractViewNode::SafeDownCast(node);
  if (viewNode)
    {
    // No explicit parent layout node means that view is handled by the main Cjyx layout
    if (!viewNode->GetParentLayoutNode())
      {
      foreach(qDMMLLayoutViewFactory* dmmlViewFactory, q->dmmlViewFactories())
        {
        dmmlViewFactory->onViewNodeAdded(viewNode);
        }
      }
    }
  else if (node->IsA("vtkDMMLSegmentationNode"))
    {
    // Show segmentation section in slice view controller if the first segmentation
    // node has been added to the scene
    this->updateSegmentationControls();
    }
}

// --------------------------------------------------------------------------
void qDMMLLayoutManagerPrivate::onNodeRemovedEvent(vtkObject* scene, vtkObject* node)
{
  Q_Q(qDMMLLayoutManager);
  Q_UNUSED(scene);
  Q_ASSERT(scene);
  Q_ASSERT(scene == this->DMMLScene);
  // Layout node
  vtkDMMLLayoutNode* layoutNode = vtkDMMLLayoutNode::SafeDownCast(node);
  if (layoutNode)
    {
    // The layout to be removed should be the same as the stored one
    Q_ASSERT(this->DMMLLayoutNode == layoutNode);
    this->setDMMLLayoutNode(nullptr);
    }
  vtkDMMLAbstractViewNode* viewNode =
    vtkDMMLAbstractViewNode::SafeDownCast(node);
  if (viewNode)
    {
    foreach(qDMMLLayoutViewFactory* dmmlViewFactory, q->dmmlViewFactories())
      {
      dmmlViewFactory->onViewNodeRemoved(viewNode);
      }
    }
  else if (node->IsA("vtkDMMLSegmentationNode"))
    {
    this->updateSegmentationControls();
    }
}

//------------------------------------------------------------------------------
void qDMMLLayoutManagerPrivate::onSceneRestoredEvent()
{
  //qDebug() << "qDMMLLayoutManagerPrivate::onSceneRestoredEvent";

  if (this->DMMLLayoutNode)
    {
    // trigger an update to the layout
    this->DMMLLayoutNode->Modified();
    }
}

//------------------------------------------------------------------------------
void qDMMLLayoutManagerPrivate::onSceneAboutToBeClosedEvent()
{
  Q_Q(qDMMLLayoutManager);
  if (!this->Enabled)
    {
    return;
    }
  // remove the layout during closing.
  q->clearLayout();
}

//------------------------------------------------------------------------------
void qDMMLLayoutManagerPrivate::onSceneClosedEvent()
{
  //qDebug() << "qDMMLLayoutManagerPrivate::onSceneClosedEvent";
  if (this->DMMLScene->IsBatchProcessing())
    {
    // Some more processing on the scene is happening, let's just wait until it
    // finishes.
    return;
    }

  // Since the loaded scene may not contain the required nodes, calling
  // initialize will make sure the LayoutNode, DMMLViewNode,
  // DMMLSliceNode exists.
  this->updateLayoutFromDMMLScene();
  Q_ASSERT(this->DMMLLayoutNode);
}

//------------------------------------------------------------------------------
void qDMMLLayoutManagerPrivate::onLayoutNodeModifiedEvent(vtkObject* vtkNotUsed(layoutNode))
{
  if (!this->DMMLScene ||
      this->DMMLScene->IsBatchProcessing() ||
      !this->Enabled)
    {
    return;
    }
  this->updateLayoutInternal();
}

//------------------------------------------------------------------------------
void qDMMLLayoutManagerPrivate::updateLayoutFromDMMLScene()
{
  Q_Q(qDMMLLayoutManager);
  foreach(qDMMLLayoutViewFactory* viewFactory, q->dmmlViewFactories())
    {
    viewFactory->onSceneModified();
    }
  this->setDMMLLayoutNode(this->DMMLLayoutLogic->GetLayoutNode());
}

/*
//------------------------------------------------------------------------------
bool qDMMLLayoutManagerPrivate::startUpdateLayout()
{
  Q_Q(qDMMLLayoutManager);
  if (!q->viewport())
    {
    return false;
    }
  bool updatesEnabled = q->viewport()->updatesEnabled();
  q->viewport()->setUpdatesEnabled(false);
  return updatesEnabled;
}

//------------------------------------------------------------------------------
void qDMMLLayoutManagerPrivate::endUpdateLayout(bool updatesEnabled)
{
  Q_Q(qDMMLLayoutManager);
  if (!q->viewport())
    {
    return;
    }
  q->viewport()->setUpdatesEnabled(updatesEnabled);
}
*/

//------------------------------------------------------------------------------
void qDMMLLayoutManagerPrivate::updateLayoutInternal()
{
  Q_Q(qDMMLLayoutManager);
  int layout = this->DMMLLayoutNode ?
    this->DMMLLayoutNode->GetViewArrangement() :
    vtkDMMLLayoutNode::CjyxLayoutNone;

  if (layout == vtkDMMLLayoutNode::CjyxLayoutCustomView)
    {
    return;
    }

  QDomDocument newLayout;

  vtkDMMLAbstractViewNode* maximizedViewNode = (this->DMMLLayoutNode ? this->DMMLLayoutNode->GetMaximizedViewNode() : nullptr);
  if (maximizedViewNode)
    {
    // Maximized view
    std::string maximizedLayoutDescription = this->DMMLLayoutLogic->GetMaximizedViewLayoutDescription(maximizedViewNode);
    newLayout.setContent(QString::fromStdString(maximizedLayoutDescription));
    }
  else
    {
    // Normal (non-maximized view)
    // TBD: modify the dom doc manually, don't create a new one
    newLayout.setContent(QString(
      this->DMMLLayoutNode ?
      this->DMMLLayoutNode->GetCurrentLayoutDescription() : ""));
    }

  q->setLayout(newLayout);
  emit q->layoutChanged(layout);
}

//------------------------------------------------------------------------------
void qDMMLLayoutManagerPrivate::setLayoutNumberOfCompareViewRowsInternal(int num)
{
  // Set the number of viewers on the layout node. This will trigger a
  // callback to in qDMMLLayoutLogic to redefine the layouts for the
  // comparison modes.

  // Update LayoutNode
  if (this->DMMLLayoutNode)
    {
    this->DMMLLayoutNode->SetNumberOfCompareViewRows(num);
    }
}

//------------------------------------------------------------------------------
void qDMMLLayoutManagerPrivate::setLayoutNumberOfCompareViewColumnsInternal(int num)
{
  // Set the number of viewers on the layout node. This will trigger a
  // callback to in qDMMLLayoutLogic to redefine the layouts for the
  // comparison modes.

  // Update LayoutNode
  if (this->DMMLLayoutNode)
    {
    this->DMMLLayoutNode->SetNumberOfCompareViewColumns(num);
    }
}

//------------------------------------------------------------------------------
void qDMMLLayoutManagerPrivate::updateSegmentationControls()
{
  Q_Q(qDMMLLayoutManager);

  foreach(const QString& viewName, q->sliceViewNames())
    {
    q->sliceWidget(viewName)->sliceController()->updateSegmentationControlsVisibility();
    }
}


//------------------------------------------------------------------------------
// qDMMLLayoutManager methods

// --------------------------------------------------------------------------
qDMMLLayoutManager::qDMMLLayoutManager(QObject* parentObject)
  : Superclass(nullptr, parentObject)
  , d_ptr(new qDMMLLayoutManagerPrivate(*this))
{
  Q_D(qDMMLLayoutManager);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLLayoutManager::qDMMLLayoutManager(QWidget* viewport, QObject* parentObject)
  : Superclass(viewport, parentObject)
  , d_ptr(new qDMMLLayoutManagerPrivate(*this))
{
  Q_D(qDMMLLayoutManager);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLLayoutManager::qDMMLLayoutManager(qDMMLLayoutManagerPrivate* pimpl,
                                       QWidget* viewport, QObject* parentObject)
  : Superclass(viewport, parentObject)
  , d_ptr(pimpl)
{
  Q_D(qDMMLLayoutManager);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLLayoutManager::~qDMMLLayoutManager() = default;

// --------------------------------------------------------------------------
bool qDMMLLayoutManager::isEnabled()const
{
  Q_D(const qDMMLLayoutManager);
  return d->Enabled;
}

// --------------------------------------------------------------------------
void qDMMLLayoutManager::setEnabled(bool enable)
{
  Q_D(qDMMLLayoutManager);
  d->Enabled = enable;
}

// --------------------------------------------------------------------------
QList<qDMMLLayoutViewFactory*> qDMMLLayoutManager::dmmlViewFactories()const
{
  QList<qDMMLLayoutViewFactory*> res;
  foreach(ctkLayoutViewFactory* viewFactory, this->registeredViewFactories())
    {
    qDMMLLayoutViewFactory* dmmlViewFactory = qobject_cast<qDMMLLayoutViewFactory*>(viewFactory);
    if (dmmlViewFactory)
      {
      res << dmmlViewFactory;
      }
    }
  return res;
}

// --------------------------------------------------------------------------
qDMMLLayoutViewFactory* qDMMLLayoutManager
::dmmlViewFactory(const QString& viewClassName)const
{
  foreach(qDMMLLayoutViewFactory* viewFactory, this->dmmlViewFactories())
    {
    if (viewFactory->viewClassName() == viewClassName)
      {
      return viewFactory;
      }
    }
  return nullptr;
}

// --------------------------------------------------------------------------
void qDMMLLayoutManager
::registerViewFactory(ctkLayoutViewFactory* viewFactory)
{
  this->Superclass::registerViewFactory(viewFactory);
  qDMMLLayoutViewFactory* dmmlViewFactory = qobject_cast<qDMMLLayoutViewFactory*>(viewFactory);
  if (dmmlViewFactory)
    {
    dmmlViewFactory->setLayoutManager(this);
    dmmlViewFactory->setDMMLScene(this->dmmlScene());
    }
}

// --------------------------------------------------------------------------
void qDMMLLayoutManager::onViewportChanged()
{
  Q_D(qDMMLLayoutManager);
  d->updateLayoutFromDMMLScene();
  this->Superclass::onViewportChanged();
}

//------------------------------------------------------------------------------
qDMMLSliceWidget* qDMMLLayoutManager::sliceWidget(const QString& name)const
{
  qDMMLLayoutViewFactory* viewFactory = this->dmmlViewFactory("vtkDMMLSliceNode");
  if (!viewFactory)
    {
    return nullptr;
    }
  return qobject_cast<qDMMLSliceWidget*>(viewFactory->viewWidget(name));
}

//------------------------------------------------------------------------------
QStringList qDMMLLayoutManager::sliceViewNames() const
{
  qDMMLLayoutViewFactory* viewFactory = this->dmmlViewFactory("vtkDMMLSliceNode");
  if (!viewFactory)
    {
    return QStringList();
    }
  return viewFactory->viewNodeNames();
}

//------------------------------------------------------------------------------
int qDMMLLayoutManager::threeDViewCount()const
{
  qDMMLLayoutViewFactory* viewFactory = this->dmmlViewFactory("vtkDMMLViewNode");
  if (!viewFactory)
    {
    return 0;
    }
  return viewFactory->viewCount();
}

//------------------------------------------------------------------------------
int qDMMLLayoutManager::tableViewCount()const
{
  qDMMLLayoutViewFactory* viewFactory = this->dmmlViewFactory("vtkDMMLTableViewNode");
  if (!viewFactory)
    {
    return 0;
    }
  return viewFactory->viewCount();
}

//------------------------------------------------------------------------------
int qDMMLLayoutManager::plotViewCount() const
{
  qDMMLLayoutViewFactory* viewFactory = this->dmmlViewFactory("vtkDMMLPlotViewNode");
  if (!viewFactory)
    {
    return 0;
    }
  return viewFactory->viewCount();
}

//------------------------------------------------------------------------------
qDMMLThreeDWidget* qDMMLLayoutManager::threeDWidget(int id)const
{
  qDMMLLayoutViewFactory* viewFactory = this->dmmlViewFactory("vtkDMMLViewNode");
  if (!viewFactory)
    {
    return nullptr;
    }
  return qobject_cast<qDMMLThreeDWidget*>(viewFactory->viewWidget(id));
}

//------------------------------------------------------------------------------
qDMMLThreeDWidget* qDMMLLayoutManager::threeDWidget(const QString& name)const
{
  qDMMLLayoutViewFactory* viewFactory = this->dmmlViewFactory("vtkDMMLViewNode");
  if (!viewFactory)
    {
    return nullptr;
    }
  return qobject_cast<qDMMLThreeDWidget*>(viewFactory->viewWidget(name));
}

//------------------------------------------------------------------------------
qDMMLTableWidget* qDMMLLayoutManager::tableWidget(int id)const
{
  qDMMLLayoutViewFactory* viewFactory = this->dmmlViewFactory("vtkDMMLTableViewNode");
  if (!viewFactory)
    {
    return nullptr;
    }
  return qobject_cast<qDMMLTableWidget*>(viewFactory->viewWidget(id));
}

//------------------------------------------------------------------------------
qDMMLPlotWidget *qDMMLLayoutManager::plotWidget(int id)const
{
  qDMMLLayoutViewFactory* viewFactory = this->dmmlViewFactory("vtkDMMLPlotViewNode");
  if (!viewFactory)
    {
    return nullptr;
    }
  return qobject_cast<qDMMLPlotWidget*>(viewFactory->viewWidget(id));
}

//------------------------------------------------------------------------------
vtkCollection* qDMMLLayoutManager::dmmlSliceLogics()const
{
  qDMMLLayoutSliceViewFactory* viewFactory =
    qobject_cast<qDMMLLayoutSliceViewFactory*>(this->dmmlViewFactory("vtkDMMLSliceNode"));
  if (!viewFactory)
    {
    return nullptr;
    }
  return viewFactory->sliceLogics();
}

//------------------------------------------------------------------------------
vtkCollection *qDMMLLayoutManager::dmmlViewLogics() const
{
  qDMMLLayoutThreeDViewFactory* viewFactory =
    qobject_cast<qDMMLLayoutThreeDViewFactory*>(this->dmmlViewFactory("vtkDMMLViewNode"));
  if (!viewFactory)
    {
    return nullptr;
    }
  return viewFactory->viewLogics();
}

//------------------------------------------------------------------------------
void qDMMLLayoutManager::setDMMLColorLogic(vtkDMMLColorLogic* colorLogic)
{
  qDMMLLayoutPlotViewFactory* viewFactory =
    qobject_cast<qDMMLLayoutPlotViewFactory*>(this->dmmlViewFactory("vtkDMMLPlotViewNode"));
  if (!viewFactory)
    {
    return;
    }
  viewFactory->setColorLogic(colorLogic);
}

//------------------------------------------------------------------------------
vtkDMMLColorLogic* qDMMLLayoutManager::dmmlColorLogic()const
{
  qDMMLLayoutPlotViewFactory* viewFactory =
    qobject_cast<qDMMLLayoutPlotViewFactory*>(this->dmmlViewFactory("vtkDMMLPlotViewNode"));
  if (!viewFactory)
    {
    return nullptr;
    }
  return viewFactory->colorLogic();
}

//------------------------------------------------------------------------------
vtkDMMLLayoutLogic* qDMMLLayoutManager::layoutLogic()const
{
  Q_D(const qDMMLLayoutManager);
  return d->DMMLLayoutLogic;
}

//------------------------------------------------------------------------------
void qDMMLLayoutManager::setDMMLScene(vtkDMMLScene* scene)
{
  Q_D(qDMMLLayoutManager);
  if (d->DMMLScene == scene)
    {
    return;
    }

  vtkDMMLScene* oldScene = d->DMMLScene;
  d->DMMLScene = scene;
  d->DMMLLayoutNode = nullptr;

  // We want to connect the logic to the scene first (before the following
  // qvtkReconnect); that way, anytime the scene is modified, the logic
  // callbacks will be called  before qDMMLLayoutManager and keep the scene
  // in a good state
  d->DMMLLayoutLogic->SetDMMLScene(d->DMMLScene);

  d->qvtkReconnect(oldScene, scene, vtkDMMLScene::NodeAddedEvent,
                   d, SLOT(onNodeAddedEvent(vtkObject*,vtkObject*)));

  d->qvtkReconnect(oldScene, scene, vtkDMMLScene::NodeRemovedEvent,
                   d, SLOT(onNodeRemovedEvent(vtkObject*,vtkObject*)));

  d->qvtkReconnect(oldScene, scene, vtkDMMLScene::EndBatchProcessEvent,
                   d, SLOT(updateLayoutFromDMMLScene()));

  d->qvtkReconnect(oldScene, scene, vtkDMMLScene::EndBatchProcessEvent,
                   d, SLOT(updateSegmentationControls()));

  d->qvtkReconnect(oldScene, scene, vtkDMMLScene::EndRestoreEvent,
                   d, SLOT(onSceneRestoredEvent()));

  d->qvtkReconnect(oldScene, scene, vtkDMMLScene::StartCloseEvent,
                   d, SLOT(onSceneAboutToBeClosedEvent()));

  d->qvtkReconnect(oldScene, scene, vtkDMMLScene::EndCloseEvent,
                   d, SLOT(onSceneClosedEvent()));

  foreach(qDMMLLayoutViewFactory* viewFactory, this->dmmlViewFactories())
    {
    viewFactory->setDMMLScene(d->DMMLScene);
    }
  d->updateLayoutFromDMMLScene();
}

//------------------------------------------------------------------------------
vtkDMMLScene* qDMMLLayoutManager::dmmlScene()const
{
  Q_D(const qDMMLLayoutManager);
  return d->DMMLScene;
}

//------------------------------------------------------------------------------
vtkDMMLViewNode* qDMMLLayoutManager::activeDMMLThreeDViewNode()const
{
  return vtkDMMLViewNode::SafeDownCast(
    this->dmmlViewFactory("vtkDMMLViewNode")->activeViewNode());
}

//------------------------------------------------------------------------------
vtkDMMLTableViewNode* qDMMLLayoutManager::activeDMMLTableViewNode()const
{
  return vtkDMMLTableViewNode::SafeDownCast(
    this->dmmlViewFactory("vtkDMMLTableViewNode")->activeViewNode());
}

//------------------------------------------------------------------------------
vtkDMMLPlotViewNode *qDMMLLayoutManager::activeDMMLPlotViewNode()const
{
  return vtkDMMLPlotViewNode::SafeDownCast(
    this->dmmlViewFactory("vtkDMMLPlotViewNode")->activeViewNode());
}

//------------------------------------------------------------------------------
vtkRenderer* qDMMLLayoutManager::activeThreeDRenderer()const
{
  return this->dmmlViewFactory("vtkDMMLViewNode")->activeRenderer();
}

//------------------------------------------------------------------------------
vtkRenderer* qDMMLLayoutManager::activeTableRenderer()const
{
  return this->dmmlViewFactory("vtkDMMLTableViewNode")->activeRenderer();
}

//------------------------------------------------------------------------------
vtkRenderer* qDMMLLayoutManager::activePlotRenderer()const
{
  return this->dmmlViewFactory("vtkDMMLPlotViewNode")->activeRenderer();
}

//------------------------------------------------------------------------------
int qDMMLLayoutManager::layout()const
{
  Q_D(const qDMMLLayoutManager);
  return d->DMMLLayoutNode ?
    d->DMMLLayoutNode->GetViewArrangement() : vtkDMMLLayoutNode::CjyxLayoutNone;
}

//------------------------------------------------------------------------------
void qDMMLLayoutManager::setLayout(int layout)
{
  Q_D(qDMMLLayoutManager);
  if (this->layout() == layout)
    {
    return;
    }
  // Update LayoutNode
  if (d->DMMLLayoutNode)
    {
    if (!d->DMMLLayoutNode->IsLayoutDescription(layout))
      {
      layout = vtkDMMLLayoutNode::CjyxLayoutConventionalView;
      }
    d->DMMLLayoutNode->SetMaximizedViewNode(nullptr);
    d->DMMLLayoutNode->SetViewArrangement(layout);
    }
}

//------------------------------------------------------------------------------
void qDMMLLayoutManager::setMaximizedViewNode(vtkDMMLAbstractViewNode* viewNode)
{
  Q_D(qDMMLLayoutManager);
  if (!d->DMMLLayoutNode)
    {
    return;
    }
  d->DMMLLayoutNode->SetMaximizedViewNode(viewNode);
}

//------------------------------------------------------------------------------
vtkDMMLAbstractViewNode* qDMMLLayoutManager::maximizedViewNode()
{
  Q_D(qDMMLLayoutManager);
  if (!d->DMMLLayoutNode)
    {
    return nullptr;
    }
  return d->DMMLLayoutNode->GetMaximizedViewNode();
}

//------------------------------------------------------------------------------
void qDMMLLayoutManager::setLayoutNumberOfCompareViewRows(int num)
{
  Q_D(qDMMLLayoutManager);

  d->setLayoutNumberOfCompareViewRowsInternal(num);
}

//------------------------------------------------------------------------------
void qDMMLLayoutManager::setLayoutNumberOfCompareViewColumns(int num)
{
  Q_D(qDMMLLayoutManager);

  d->setLayoutNumberOfCompareViewColumnsInternal(num);
}

//------------------------------------------------------------------------------
void qDMMLLayoutManager::resetThreeDViews()
{
  for(int idx = 0; idx < this->threeDViewCount(); ++idx)
    {
    qDMMLThreeDView* threeDView = this->threeDWidget(idx)->threeDView();
    threeDView->resetFocalPoint();
    threeDView->resetCamera();
    }
}

//------------------------------------------------------------------------------
void qDMMLLayoutManager::resetSliceViews()
{
  foreach(const QString& viewName, this->sliceViewNames())
    {
    this->sliceWidget(viewName)->sliceController()->fitSliceToBackground();
    }
}

//------------------------------------------------------------------------------
QWidget* qDMMLLayoutManager::viewWidget(vtkDMMLNode* viewNode) const
{
  Q_D(const qDMMLLayoutManager);

  return d->viewWidget(viewNode);
}

//------------------------------------------------------------------------------
void qDMMLLayoutManager::setRenderPaused(bool pause)
{
  // Note: views that are instantiated between pauseRender() calls will not be affected
  // by the specified pause state
  Q_D(qDMMLLayoutManager);
  qDMMLLayoutViewFactory* sliceViewFactory = this->dmmlViewFactory("vtkDMMLSliceNode");
  foreach(const QString& viewName, sliceViewFactory->viewNodeNames())
    {
    ctkVTKAbstractView* view = this->sliceWidget(viewName)->sliceView();
    if (!pause && !view->isRenderPaused())
      {
      // This view is already resumed and a resume is requested.
      // This is probably because the view has just been added
      // and so it missed a few PauseRender requests.
      // Do not try to resume render to avoid logging of a warning,
      // and just log a debug message here to help with troubleshooting if
      // any problem comes up related to pausing rendering.
      qDebug() << Q_FUNC_INFO << "Resume render request is ignored for view "
        << viewName << ", probably the view has just been created";
      }
    else
      {
      view->setRenderPaused(pause);
      }
    }

  qDMMLLayoutViewFactory* threeDViewFactory = this->dmmlViewFactory("vtkDMMLViewNode");
  foreach(const QString& viewName, threeDViewFactory->viewNodeNames())
    {
    ctkVTKAbstractView* view = this->threeDWidget(viewName)->threeDView();
    view->setRenderPaused(pause);
    }
}

//------------------------------------------------------------------------------
void qDMMLLayoutManager::pauseRender()
{
  this->setRenderPaused(true);
}

//------------------------------------------------------------------------------
void qDMMLLayoutManager::resumeRender()
{
  this->setRenderPaused(false);
}
