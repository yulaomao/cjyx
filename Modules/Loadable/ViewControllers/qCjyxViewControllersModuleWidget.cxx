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

// Qt includes
#include <QDebug>

// DMMLWidgets includes
#include <qDMMLSliceWidget.h>
#include <qDMMLSliceControllerWidget.h>
#include <qDMMLThreeDWidget.h>
#include <qDMMLThreeDViewControllerWidget.h>
#include <qDMMLPlotWidget.h>
#include <qDMMLPlotViewControllerWidget.h>

// Cjyx includes
#include "qCjyxViewControllersModuleWidget.h"
#include "ui_qCjyxViewControllersModuleWidget.h"
#include "qCjyxApplication.h"
#include "qCjyxLayoutManager.h"

// DMML includes
#include "vtkDMMLPlotViewNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSliceNode.h"
#include "vtkDMMLViewNode.h"

// DMMLLogic includes
#include "vtkDMMLLayoutLogic.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qCjyxViewControllersModuleWidgetPrivate:
    public Ui_qCjyxViewControllersModuleWidget
{
  Q_DECLARE_PUBLIC(qCjyxViewControllersModuleWidget);
protected:
  qCjyxViewControllersModuleWidget* const q_ptr;

public:
  qCjyxViewControllersModuleWidgetPrivate(qCjyxViewControllersModuleWidget& obj);
  virtual ~qCjyxViewControllersModuleWidgetPrivate();

  /// Create a Controller for a Node and pack in the widget
  void createController(vtkDMMLNode *n, qCjyxLayoutManager *lm);

  /// Remove the Controller for a Node from the widget
  void removeController(vtkDMMLNode *n);

  typedef std::map<vtkSmartPointer<vtkDMMLNode>, qDMMLViewControllerBar* > ControllerMapType;
  ControllerMapType ControllerMap;

protected:
};

//-----------------------------------------------------------------------------
qCjyxViewControllersModuleWidgetPrivate::qCjyxViewControllersModuleWidgetPrivate(qCjyxViewControllersModuleWidget& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qCjyxViewControllersModuleWidgetPrivate::~qCjyxViewControllersModuleWidgetPrivate() = default;

//-----------------------------------------------------------------------------
void
qCjyxViewControllersModuleWidgetPrivate::createController(vtkDMMLNode *n, qCjyxLayoutManager *layoutManager)
{
  Q_Q(qCjyxViewControllersModuleWidget);

  if (this->ControllerMap.find(n) != this->ControllerMap.end())
    {
    qDebug() << "qCjyxViewControllersModuleWidgetPrivate::createController - Node already added to module";
    return;
    }

  // create the ControllerWidget and wire it to the appropriate node
  qDMMLViewControllerBar *barWidget = nullptr;
  vtkDMMLSliceNode *sn = vtkDMMLSliceNode::SafeDownCast(n);
  if (sn)
    {
    qDMMLSliceControllerWidget *widget =
      new qDMMLSliceControllerWidget(this->SliceControllersCollapsibleButton);
    widget->setSliceViewName( sn->GetName() ); // call before setting slice node
    widget->setSliceViewLabel( sn->GetLayoutLabel() );
    QColor layoutColor = QColor::fromRgbF(sn->GetLayoutColor()[0],
                                          sn->GetLayoutColor()[1],
                                          sn->GetLayoutColor()[2]);
    widget->setSliceViewColor( layoutColor );
    widget->setDMMLSliceNode( sn );
    widget->setLayoutBehavior( qDMMLViewControllerBar::Panel );

    // SliceControllerWidget needs to know the SliceLogic(s)
    qDMMLSliceWidget *sliceWidget = layoutManager->sliceWidget(sn->GetLayoutName());
    widget->setSliceLogics(layoutManager->dmmlSliceLogics());
    widget->setSliceLogic(sliceWidget->sliceController()->sliceLogic());

    // add the widget to the display
    this->SliceControllersLayout->addWidget(widget);

    barWidget = widget;
    }

  vtkDMMLViewNode *vn = vtkDMMLViewNode::SafeDownCast(n);
  if (vn)
    {
    // ThreeDViewController needs to now the ThreeDView
    qDMMLThreeDWidget *viewWidget = dynamic_cast<qDMMLThreeDWidget*>(layoutManager->viewWidget( vn ));
    if (viewWidget)
      {
      qDMMLThreeDViewControllerWidget* widget =
        new qDMMLThreeDViewControllerWidget(this->ThreeDViewControllersCollapsibleButton);
      widget->setLayoutBehavior(qDMMLViewControllerBar::Panel);
      widget->setDMMLScene(q->dmmlScene());
      widget->setThreeDView( viewWidget->threeDView() );
      // qDMMLThreeDViewControllerWidget needs to know the ViewLogic(s)
      widget->setViewLogic(viewWidget->threeDController()->viewLogic());
      // add the widget to the display
      this->ThreeDViewControllersLayout->addWidget(widget);
      barWidget = widget;
      }
    }

  vtkDMMLPlotViewNode *pn = vtkDMMLPlotViewNode::SafeDownCast(n);
  if (pn)
    {
    qDMMLPlotViewControllerWidget *widget =
      new qDMMLPlotViewControllerWidget(this->PlotViewControllersCollapsibleButton);
    widget->setDMMLPlotViewNode( pn );
    widget->setLayoutBehavior( qDMMLViewControllerBar::Panel );

    // PlotViewController needs to now the PlotView
    qDMMLPlotWidget *viewWidget = dynamic_cast<qDMMLPlotWidget*>(layoutManager->viewWidget( pn ));
    if (viewWidget)
      {
      widget->setPlotView( viewWidget->plotView() ) ;
      widget->setDMMLPlotViewNode(pn);
      widget->setDMMLScene(q->dmmlScene());
      }

    // add the widget to the display
    this->PlotViewControllersLayout->addWidget(widget);

    barWidget = widget;
    }

  // cache the widget. we'll clean this up on the NodeRemovedEvent
  this->ControllerMap[n] = barWidget;
}

//-----------------------------------------------------------------------------
void
qCjyxViewControllersModuleWidgetPrivate::removeController(vtkDMMLNode *n)
{
  // find the widget for the SliceNode
  ControllerMapType::iterator cit = this->ControllerMap.find(n);
  if (cit == this->ControllerMap.end())
    {
    qDebug() << "qCjyxViewControllersModuleWidgetPrivate::removeController - Node has no Controller managed by this module.";
    return;
    }

  // unpack the widget
  vtkDMMLSliceNode *sn = vtkDMMLSliceNode::SafeDownCast(n);
  if (sn)
    {
    SliceControllersLayout->removeWidget((*cit).second);
    }

  vtkDMMLViewNode *vn = vtkDMMLViewNode::SafeDownCast(n);
  if (vn)
    {
    ThreeDViewControllersLayout->removeWidget((*cit).second);
    }

  vtkDMMLPlotViewNode *pn = vtkDMMLPlotViewNode::SafeDownCast(n);
  if (pn)
    {
    PlotViewControllersLayout->removeWidget((*cit).second);
    }

  // delete the widget
  delete (*cit).second;

  // remove entry from the map
  this->ControllerMap.erase(cit);
}


//-----------------------------------------------------------------------------
qCjyxViewControllersModuleWidget::qCjyxViewControllersModuleWidget(QWidget* _parentWidget)
  : Superclass(_parentWidget),
    d_ptr(new qCjyxViewControllersModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qCjyxViewControllersModuleWidget::~qCjyxViewControllersModuleWidget() = default;

//-----------------------------------------------------------------------------
void qCjyxViewControllersModuleWidget::setup()
{
  Q_D(qCjyxViewControllersModuleWidget);
  d->setupUi(this);

  connect(d->DMMLViewNodeComboBox, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
    this, SLOT(onAdvancedViewNodeChanged(vtkDMMLNode*)));

  d->AdvancedCollapsibleButton->setCollapsed(true);
}

//-----------------------------------------------------------------------------
void qCjyxViewControllersModuleWidget::setDMMLScene(vtkDMMLScene *newScene)
{
  Q_D(qCjyxViewControllersModuleWidget);

  vtkDMMLScene* oldScene = this->dmmlScene();

  this->Superclass::setDMMLScene(newScene);

  qCjyxApplication * app = qCjyxApplication::application();
  if (!app)
    {
    return;
    }
  qCjyxLayoutManager * layoutManager = app->layoutManager();
  if (!layoutManager)
    {
    return;
    }

  // Search the scene for the available view nodes and create a
  // Controller and connect it up
  std::vector<vtkDMMLNode*> sliceNodes;
  newScene->GetNodesByClass("vtkDMMLSliceNode", sliceNodes);
  for (std::vector< vtkDMMLNode* >::iterator sliceNodeIt = sliceNodes.begin(); sliceNodeIt != sliceNodes.end(); ++sliceNodeIt)
    {
    vtkDMMLSliceNode *snode = vtkDMMLSliceNode::SafeDownCast(*sliceNodeIt);
    if (snode)
      {
      d->createController(snode, layoutManager);
      }
    }

  std::vector<vtkDMMLNode*> threeDNodes;
  newScene->GetNodesByClass("vtkDMMLViewNode", threeDNodes);
  for (std::vector< vtkDMMLNode* >::iterator threeDNodeIt = threeDNodes.begin(); threeDNodeIt != threeDNodes.end(); ++threeDNodeIt)
    {
    vtkDMMLViewNode *vnode = vtkDMMLViewNode::SafeDownCast(*threeDNodeIt);
    if (vnode)
      {
      d->createController(vnode, layoutManager);
      }
    }

  std::vector<vtkDMMLNode*> plotNodes;
  newScene->GetNodesByClass("vtkDMMLPlotViewNode", plotNodes);
  for (std::vector< vtkDMMLNode* >::iterator plotNodeIt = plotNodes.begin(); plotNodeIt != plotNodes.end(); ++plotNodeIt)
    {
    vtkDMMLPlotViewNode *pnode = vtkDMMLPlotViewNode::SafeDownCast(*plotNodeIt);
    if (pnode)
      {
      d->createController(pnode, layoutManager);
      }
    }

  // Need to listen for any new slice or view nodes being added
  this->qvtkReconnect(oldScene, newScene, vtkDMMLScene::NodeAddedEvent,
                      this, SLOT(onNodeAddedEvent(vtkObject*,vtkObject*)));

  // Need to listen for any slice or view nodes being removed
  this->qvtkReconnect(oldScene, newScene, vtkDMMLScene::NodeRemovedEvent,
                      this, SLOT(onNodeRemovedEvent(vtkObject*,vtkObject*)));

  // Listen to changes in the Layout so we only show controllers for
  // the visible nodes
  QObject::connect(layoutManager, SIGNAL(layoutChanged(int)), this,
                   SLOT(onLayoutChanged(int)));

}

// --------------------------------------------------------------------------
void qCjyxViewControllersModuleWidget::onNodeAddedEvent(vtkObject*, vtkObject* node)
{
  Q_D(qCjyxViewControllersModuleWidget);

  if (!this->dmmlScene() || this->dmmlScene()->IsBatchProcessing())
    {
    return;
    }

  qCjyxApplication * app = qCjyxApplication::application();
  if (!app)
    {
    return;
    }
  qCjyxLayoutManager * layoutManager = app->layoutManager();
  if (!layoutManager)
    {
    return;
    }

  vtkDMMLSliceNode* sliceNode = vtkDMMLSliceNode::SafeDownCast(node);
  if (sliceNode)
    {
    //QString layoutName = sliceNode->GetLayoutName();
    //qDebug() << "qCjyxViewControllersModuleWidget::onNodeAddedEvent - layoutName:" << layoutName;

    // create the slice controller
    d->createController(sliceNode, layoutManager);
    }

  vtkDMMLViewNode* viewNode = vtkDMMLViewNode::SafeDownCast(node);
  if (viewNode)
    {
    //QString layoutName = viewNode->GetName();
    //qDebug() << "qCjyxViewControllersModuleWidget::onNodeAddedEvent - layoutName:" << layoutName;

    // create the view controller
    d->createController(viewNode, layoutManager);
    }

  vtkDMMLPlotViewNode* plotViewNode = vtkDMMLPlotViewNode::SafeDownCast(node);
  if (plotViewNode)
    {
    //QString layoutName = viewNode->GetName();
    //qDebug() << "qCjyxViewControllersModuleWidget::onNodeAddedEvent - layoutName:" << layoutName;

    // create the view controller
    d->createController(plotViewNode, layoutManager);
    }
}

// --------------------------------------------------------------------------
void qCjyxViewControllersModuleWidget::onNodeRemovedEvent(vtkObject*, vtkObject* node)
{
  Q_D(qCjyxViewControllersModuleWidget);

  if (!this->dmmlScene() || this->dmmlScene()->IsBatchProcessing())
    {
    return;
    }

  vtkDMMLSliceNode* sliceNode = vtkDMMLSliceNode::SafeDownCast(node);
  if (sliceNode)
    {
    QString layoutName = sliceNode->GetLayoutName();
    qDebug() << "qCjyxViewControllersModuleWidget::onNodeRemovedEvent - layoutName:" << layoutName;

    // destroy the slice controller
    d->removeController(sliceNode);
    }

  vtkDMMLViewNode* viewNode = vtkDMMLViewNode::SafeDownCast(node);
  if (viewNode)
    {
    QString layoutName = viewNode->GetName();
    qDebug() << "qCjyxViewControllersModuleWidget::onNodeRemovedEvent - layoutName:" << layoutName;

    // destroy the view controller
    d->removeController(viewNode);
    }

  vtkDMMLPlotViewNode* plotViewNode = vtkDMMLPlotViewNode::SafeDownCast(node);
  if (plotViewNode)
    {
    QString layoutName = plotViewNode->GetName();
    qDebug() << "qCjyxViewControllersModuleWidget::onNodeRemovedEvent - layoutName:" << layoutName;

    // destroy the view controller
    d->removeController(plotViewNode);
    }
}

// --------------------------------------------------------------------------
void qCjyxViewControllersModuleWidget::onLayoutChanged(int)
{
  Q_D(qCjyxViewControllersModuleWidget);

  if (!this->dmmlScene() || this->dmmlScene()->IsBatchProcessing())
    {
    return;
    }

  //qDebug() << "qCjyxViewControllersModuleWidget::onLayoutChanged";

  // add the controllers for any newly visible SliceNodes and remove
  // the controllers for any SliceNodes no longer visible

  qCjyxApplication * app = qCjyxApplication::application();
  if (!app)
    {
    return;
    }
  qCjyxLayoutManager * layoutManager = app->layoutManager();
  if (!layoutManager)
    {
    return;
    }

  vtkDMMLLayoutLogic *layoutLogic = layoutManager->layoutLogic();
  vtkCollection *visibleViews = layoutLogic->GetViewNodes();

  // hide Controllers for Nodes not currently visible in
  // the layout
  qCjyxViewControllersModuleWidgetPrivate::ControllerMapType::iterator cit;
  for (cit = d->ControllerMap.begin(); cit != d->ControllerMap.end(); ++cit)
    {
    // is mananaged Node not currently displayed in the layout?
    if (!visibleViews->IsItemPresent((*cit).first))
      {
      // hide it
      (*cit).second->hide();
      }
    }

  // show Controllers for Nodes not currently being managed
  // by this widget
  vtkObject *v = nullptr;
  vtkCollectionSimpleIterator it;
  for (visibleViews->InitTraversal(it);
    (v = visibleViews->GetNextItemAsObject(it));)
    {
    vtkDMMLNode *vn = vtkDMMLNode::SafeDownCast(v);
    if (vn)
      {
      // find the controller
      qCjyxViewControllersModuleWidgetPrivate::ControllerMapType::iterator cit = d->ControllerMap.find(vn);
      if (cit != d->ControllerMap.end())
        {
        // show it
        (*cit).second->show();
        }
      }
    }
}

// --------------------------------------------------------------------------
void qCjyxViewControllersModuleWidget::onAdvancedViewNodeChanged(vtkDMMLNode* viewNode)
{
  Q_D(qCjyxViewControllersModuleWidget);
  // Only show widget corresponding to selected view node type
  d->DMMLSliceInformationWidget->setVisible(vtkDMMLSliceNode::SafeDownCast(viewNode) != nullptr);
  d->DMMLThreeDViewInformationWidget->setVisible(vtkDMMLViewNode::SafeDownCast(viewNode) != nullptr);
}
