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
//#include <QDomElement>
#include <QDebug>

// CTK includes
#include <ctkVTKAbstractView.h>

// DMMLWidgets includes
#include "qDMMLLayoutManager.h"
#include "qDMMLLayoutViewFactory.h"
#include "qDMMLWidget.h"

// DMML includes
#include <vtkDMMLAbstractViewNode.h>
#include <vtkDMMLLayoutLogic.h>
#include <vtkDMMLLayoutNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>

//------------------------------------------------------------------------------
// qDMMLLayoutViewFactoryPrivate methods
class qDMMLLayoutViewFactoryPrivate
{
  Q_DECLARE_PUBLIC(qDMMLLayoutViewFactory);
public:
  qDMMLLayoutViewFactoryPrivate(qDMMLLayoutViewFactory& obj);
  virtual void init();

  vtkDMMLAbstractViewNode* viewNodeByName(const QString& viewName)const;
  vtkDMMLAbstractViewNode* viewNodeByLayoutLabel(const QString& layoutLabel)const;

  vtkDMMLLayoutLogic::ViewAttributes attributesFromXML(QDomElement viewElement)const;
  vtkDMMLLayoutLogic::ViewProperties propertiesFromXML(QDomElement viewElement)const;
  vtkDMMLLayoutLogic::ViewProperty propertyFromXML(QDomElement propertyElement)const;

  QList<qDMMLWidget*> dmmlWidgets()const;

protected:
  qDMMLLayoutViewFactory* q_ptr;

  qDMMLLayoutManager* LayoutManager;
  QHash<vtkDMMLAbstractViewNode*, QWidget*> Views;

  vtkDMMLScene* DMMLScene;
  vtkDMMLAbstractViewNode* ActiveViewNode;
};

//------------------------------------------------------------------------------
qDMMLLayoutViewFactoryPrivate::qDMMLLayoutViewFactoryPrivate(qDMMLLayoutViewFactory& object)
  : q_ptr(&object)
  , LayoutManager(nullptr)
  , DMMLScene(nullptr)
  , ActiveViewNode(nullptr)
{
}

//------------------------------------------------------------------------------
void qDMMLLayoutViewFactoryPrivate::init()
{
  Q_Q(qDMMLLayoutViewFactory);
  q->setUseCachedViews(false);
}

//------------------------------------------------------------------------------
vtkDMMLAbstractViewNode* qDMMLLayoutViewFactoryPrivate
::viewNodeByName(const QString& viewName)const
{
  foreach(vtkDMMLAbstractViewNode* viewNode, this->Views.keys())
    {
    if (viewName == QString(viewNode->GetName()))
      {
      return viewNode;
      }
    }
  return nullptr;
}

//------------------------------------------------------------------------------
vtkDMMLAbstractViewNode* qDMMLLayoutViewFactoryPrivate
::viewNodeByLayoutLabel(const QString& layoutLabel)const
{
  foreach(vtkDMMLAbstractViewNode* viewNode, this->Views.keys())
    {
    if (layoutLabel == QString(viewNode->GetLayoutLabel()))
      {
      return viewNode;
      }
    }
  return nullptr;
}

//------------------------------------------------------------------------------
vtkDMMLLayoutLogic::ViewAttributes qDMMLLayoutViewFactoryPrivate
::attributesFromXML(QDomElement viewElement)const
{
  vtkDMMLLayoutLogic::ViewAttributes attributes;
  QDomNamedNodeMap elementAttributes = viewElement.attributes();
  const int attributeCount = elementAttributes.count();
  for (int i = 0; i < attributeCount; ++i)
    {
    QDomNode attribute = elementAttributes.item(i);
    attributes[attribute.nodeName().toStdString()] =
      viewElement.attribute(attribute.nodeName()).toStdString();
    }
  return attributes;
}

//------------------------------------------------------------------------------
vtkDMMLLayoutLogic::ViewProperties qDMMLLayoutViewFactoryPrivate
::propertiesFromXML(QDomElement viewElement)const
{
  vtkDMMLLayoutLogic::ViewProperties properties;
  for (QDomElement childElement = viewElement.firstChildElement();
       !childElement.isNull();
       childElement = childElement.nextSiblingElement())
    {
    properties.push_back(this->propertyFromXML(childElement));
    }
  return properties;
}

//------------------------------------------------------------------------------
vtkDMMLLayoutLogic::ViewProperty qDMMLLayoutViewFactoryPrivate
::propertyFromXML(QDomElement propertyElement)const
{
  vtkDMMLLayoutLogic::ViewProperty property;
  QDomNamedNodeMap elementAttributes = propertyElement.attributes();
  const int attributeCount = elementAttributes.count();
  for (int i = 0; i < attributeCount; ++i)
    {
    QDomNode attribute = elementAttributes.item(i);
    property[attribute.nodeName().toStdString()] =
      propertyElement.attribute(attribute.nodeName()).toStdString();
    }
  property["value"] = propertyElement.text().toStdString();
  return property;
}


//------------------------------------------------------------------------------
QList<qDMMLWidget*> qDMMLLayoutViewFactoryPrivate
::dmmlWidgets()const
{
  QList<qDMMLWidget*> res;
  foreach(QWidget* viewWidget, this->Views.values())
    {
    qDMMLWidget* dmmlWidget = qobject_cast<qDMMLWidget*>(viewWidget);
    if (dmmlWidget)
      {
      res << dmmlWidget;
      }
    }
  return res;
}

//------------------------------------------------------------------------------
// qDMMLLayoutViewFactory methods

// --------------------------------------------------------------------------
qDMMLLayoutViewFactory::qDMMLLayoutViewFactory(QObject* parentObject)
  : Superclass(parentObject)
  , d_ptr(new qDMMLLayoutViewFactoryPrivate(*this))
{
  Q_D(qDMMLLayoutViewFactory);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLLayoutViewFactory::~qDMMLLayoutViewFactory()
{
  Q_D(qDMMLLayoutViewFactory);
  while(this->viewCount())
    {
    this->deleteView(d->Views.keys()[0]);
    }
}

// --------------------------------------------------------------------------
QString qDMMLLayoutViewFactory::viewClassName()const
{
  return QString();
}

// --------------------------------------------------------------------------
bool qDMMLLayoutViewFactory::isElementSupported(QDomElement layoutElement)const
{
  if (!this->Superclass::isElementSupported(layoutElement))
    {
    return false;
    }
  vtkDMMLAbstractViewNode* viewNode = this->viewNodeFromXML(layoutElement);
  return this->isViewNodeSupported(viewNode);
}

// --------------------------------------------------------------------------
bool qDMMLLayoutViewFactory::isViewNodeSupported(vtkDMMLAbstractViewNode* viewNode)const
{
  return viewNode && viewNode->IsA(this->viewClassName().toUtf8());
}

// --------------------------------------------------------------------------
qDMMLLayoutManager* qDMMLLayoutViewFactory::layoutManager()const
{
  Q_D(const qDMMLLayoutViewFactory);
  return d->LayoutManager;
}

// --------------------------------------------------------------------------
void qDMMLLayoutViewFactory::setLayoutManager(qDMMLLayoutManager* layoutManager)
{
  Q_D(qDMMLLayoutViewFactory);
  d->LayoutManager = layoutManager;
}

// --------------------------------------------------------------------------
vtkDMMLScene* qDMMLLayoutViewFactory::dmmlScene()const
{
  Q_D(const qDMMLLayoutViewFactory);
  return d->DMMLScene;
}

// --------------------------------------------------------------------------
void qDMMLLayoutViewFactory::setDMMLScene(vtkDMMLScene* scene)
{
  Q_D(qDMMLLayoutViewFactory);

  if (d->DMMLScene == scene)
    {
    return;
    }
  while(this->viewCount())
    {
    this->deleteView(d->Views.keys()[0]);
    }
  this->qvtkReconnect(d->DMMLScene, scene, vtkDMMLScene::NodeAddedEvent,
                      this, SLOT(onNodeAdded(vtkObject*,vtkObject*)));

  this->qvtkReconnect(d->DMMLScene, scene, vtkDMMLScene::NodeRemovedEvent,
                     this, SLOT(onNodeRemoved(vtkObject*,vtkObject*)));

  this->qvtkReconnect(d->DMMLScene, scene, vtkDMMLScene::EndBatchProcessEvent,
                      this, SLOT(onSceneModified()));

  d->DMMLScene = scene;

  this->onSceneModified();
}

//------------------------------------------------------------------------------
QWidget* qDMMLLayoutViewFactory::viewWidget(int id)const
{
  Q_D(const qDMMLLayoutViewFactory);
  if (id < 0 || id >= d->Views.size())
    {
    return nullptr;
    }
  return d->Views.values().at(id);
}

//------------------------------------------------------------------------------
QWidget* qDMMLLayoutViewFactory::viewWidget(vtkDMMLAbstractViewNode* node)const
{
  Q_D(const qDMMLLayoutViewFactory);
  return d->Views.value(node, 0);
}

//------------------------------------------------------------------------------
vtkDMMLAbstractViewNode* qDMMLLayoutViewFactory::viewNode(QWidget* widget)const
{
  Q_D(const qDMMLLayoutViewFactory);
  return d->Views.key(widget, 0);
}

//------------------------------------------------------------------------------
QWidget* qDMMLLayoutViewFactory::viewWidget(const QString& name)const
{
  Q_D(const qDMMLLayoutViewFactory);
  vtkDMMLAbstractViewNode* viewNode = d->viewNodeByName(name);
  return this->viewWidget(viewNode);
}

//------------------------------------------------------------------------------
QWidget* qDMMLLayoutViewFactory::viewWidgetByLayoutLabel(const QString& layoutLabel)const
{
  Q_D(const qDMMLLayoutViewFactory);
  vtkDMMLAbstractViewNode* viewNode = d->viewNodeByLayoutLabel(layoutLabel);
  return this->viewWidget(viewNode);
}

//------------------------------------------------------------------------------
QStringList qDMMLLayoutViewFactory::viewNodeNames() const
{
  Q_D(const qDMMLLayoutViewFactory);

  QStringList res;
  foreach (vtkDMMLAbstractViewNode* viewNode, d->Views.keys())
    {
    res << viewNode->GetName();
    }

  return res;
}

//------------------------------------------------------------------------------
int qDMMLLayoutViewFactory::viewCount()const
{
  Q_D(const qDMMLLayoutViewFactory);
  return d->Views.size();
}

// --------------------------------------------------------------------------
void qDMMLLayoutViewFactory::beginSetupLayout()
{
  Q_D(qDMMLLayoutViewFactory);
  this->Superclass::beginSetupLayout();
  foreach(vtkDMMLAbstractViewNode* viewNode, d->Views.keys())
    {
    viewNode->SetMappedInLayout(0);
    }
}

// --------------------------------------------------------------------------
void qDMMLLayoutViewFactory::onNodeAdded(vtkObject* scene, vtkObject* node)
{
  vtkDMMLScene* dmmlScene = vtkDMMLScene::SafeDownCast(scene);
  if (!dmmlScene || dmmlScene->IsBatchProcessing())
    {
    // We have to leave because maybe all the nodes required by the view node
    // (e.g. composite slice node) have not yet been added to the scene.
    return;
    }

  Q_UNUSED(scene);
  vtkDMMLAbstractViewNode* viewNode = vtkDMMLAbstractViewNode::SafeDownCast(node);
  if (viewNode && !viewNode->GetParentLayoutNode())
    {
    // No explicit parent layout node means that view is handled by the main Cjyx layout
    this->onViewNodeAdded(viewNode);
    }
}

// --------------------------------------------------------------------------
void qDMMLLayoutViewFactory::onNodeRemoved(vtkObject* scene, vtkObject* node)
{
  Q_UNUSED(scene);
  vtkDMMLAbstractViewNode* viewNode = vtkDMMLAbstractViewNode::SafeDownCast(node);
  if (viewNode)
    {
    this->onViewNodeRemoved(viewNode);
    }
}

// --------------------------------------------------------------------------
void qDMMLLayoutViewFactory::onNodeModified(vtkObject* node)
{
  vtkDMMLAbstractViewNode* viewNode = vtkDMMLAbstractViewNode::SafeDownCast(node);
  if (viewNode)
    {
    this->onViewNodeModified(viewNode);
    }
}

// --------------------------------------------------------------------------
void qDMMLLayoutViewFactory::onViewNodeAdded(vtkDMMLAbstractViewNode* node)
{
  Q_D(qDMMLLayoutViewFactory);
  if (!this->isViewNodeSupported(node))
    {
    return;
    }
  if (this->viewWidget(node))
    { // The view already exists, no need to create it again.
    return;
    }
  QWidget* viewWidget = this->createViewFromNode(node);
  if (!viewWidget)
    { // The factory cannot create such view, do nothing about it
    return;
    }

  // Do not show until mapped into a view (the widget is shown/hidden only
  // if it is part of the layout, but if the widget was not yet part of any layout
  // then it would show up in the top-left corner of the viewport)
  viewWidget->setVisible(false);

  d->Views[node] = viewWidget;

  // For now, the active view is the first one
  if (this->viewCount() == 1)
    {
    this->setActiveViewNode(node);
    }
  this->qvtkConnect(node, vtkCommand::ModifiedEvent,
                    this, SLOT(onNodeModified(vtkObject*)));
  emit viewCreated(viewWidget);
}

// --------------------------------------------------------------------------
void qDMMLLayoutViewFactory::onViewNodeRemoved(vtkDMMLAbstractViewNode* node)
{
  this->deleteView(node);
}

// --------------------------------------------------------------------------
void qDMMLLayoutViewFactory::onViewNodeModified(vtkDMMLAbstractViewNode* node)
{
  this->viewWidget(node)->setVisible(node->IsViewVisibleInLayout());
}

// --------------------------------------------------------------------------
void qDMMLLayoutViewFactory::onSceneModified()
{
  Q_D(qDMMLLayoutViewFactory);
  if (!d->DMMLScene)
    {
    return;
    }
  std::vector<vtkDMMLNode*> viewNodes;
  d->DMMLScene->GetNodesByClass("vtkDMMLAbstractViewNode", viewNodes);
  for (unsigned int i = 0; i < viewNodes.size(); ++i)
    {
    vtkDMMLAbstractViewNode* viewNode =
      vtkDMMLAbstractViewNode::SafeDownCast(viewNodes[i]);
    Q_ASSERT(viewNode);
    this->onViewNodeAdded(viewNode);
    }
}

// --------------------------------------------------------------------------
QWidget* qDMMLLayoutViewFactory::createViewFromNode(vtkDMMLAbstractViewNode* viewNode)
{
  Q_UNUSED(viewNode);
  return nullptr;
}

// --------------------------------------------------------------------------
void qDMMLLayoutViewFactory::deleteView(vtkDMMLAbstractViewNode* viewNode)
{
  Q_D(qDMMLLayoutViewFactory);

  this->qvtkDisconnect(viewNode, vtkCommand::ModifiedEvent,
                       this, SLOT(onNodeModified(vtkObject*)));

  QWidget* widgetToDelete = this->viewWidget(viewNode);

  // Remove slice widget
  if (!widgetToDelete)
    {
    return;
    }
  qDMMLWidget* dmmlWidgetToDelete = qobject_cast<qDMMLWidget*>(widgetToDelete);
  if (dmmlWidgetToDelete)
    {
    dmmlWidgetToDelete->setDMMLScene(nullptr);
    }
  this->unregisterView(widgetToDelete);
  d->Views.remove(viewNode);
  widgetToDelete->deleteLater();
  if (this->activeViewNode() == viewNode)
    {
    this->setActiveViewNode(nullptr);
    }
}

// --------------------------------------------------------------------------
void qDMMLLayoutViewFactory::setActiveViewNode(vtkDMMLAbstractViewNode* node)
{
  Q_D(qDMMLLayoutViewFactory);
  if (d->ActiveViewNode == node)
    {
    return;
    }
  d->ActiveViewNode = node;

  emit this->activeViewNodeChanged(d->ActiveViewNode);
}

// --------------------------------------------------------------------------
vtkDMMLAbstractViewNode* qDMMLLayoutViewFactory::activeViewNode()const
{
  Q_D(const qDMMLLayoutViewFactory);
  return d->ActiveViewNode;
}

// --------------------------------------------------------------------------
vtkRenderer* qDMMLLayoutViewFactory::activeRenderer()const
{
  QWidget* activeViewWidget = this->viewWidget(this->activeViewNode());
  if (!activeViewWidget)
    {
    return nullptr;
    }
  ctkVTKAbstractView* view = activeViewWidget->findChild<ctkVTKAbstractView*>();
  vtkRenderWindow* renderWindow = view ? view->renderWindow() : nullptr;
  vtkRendererCollection* renderers =
    renderWindow ? renderWindow->GetRenderers() : nullptr;
  return renderers ? renderers->GetFirstRenderer() : nullptr;
}

//------------------------------------------------------------------------------
vtkDMMLAbstractViewNode* qDMMLLayoutViewFactory::viewNodeFromXML(QDomElement viewElement)const
{
  Q_D(const qDMMLLayoutViewFactory);
  vtkDMMLLayoutLogic::ViewAttributes attributes =
    d->attributesFromXML(viewElement);
  // convert Qt xml element into vtkDMMLLayoutLogic attributes
  vtkDMMLNode* viewNode = this->layoutManager()->layoutLogic()->GetViewFromAttributes(attributes);
  return vtkDMMLAbstractViewNode::SafeDownCast(viewNode);
}

//------------------------------------------------------------------------------
QWidget* qDMMLLayoutViewFactory::createViewFromXML(QDomElement viewElement)
{
  Q_D(qDMMLLayoutViewFactory);
  vtkDMMLAbstractViewNode* viewNode = this->viewNodeFromXML(viewElement);
  Q_ASSERT(viewNode);
  // Usually, the view is automatically created if:
  //  (1) a view node associated with the factory is added (See onViewNodeAdded)
  //  (2) a new Scene is set on the factory (See setDMMLScene / onSceneModified)
  //  (3) a batch process ends (See EndBatchProcessEvent / onSceneModified)
  QWidget* view = this->viewWidget(viewNode);
  if (!view)
    {
    // The following call will take care of creating the view when a sceneView is
    // restored. In that case, vtkDMMLLayoutLogic::OnDMMLSceneEndRestore was called first
    // without giving a chance to the factory to create the missing views.
    this->onViewNodeAdded(viewNode);
    view = this->viewWidget(viewNode);
    }
  Q_ASSERT(view);

  vtkDMMLLayoutLogic::ViewProperties properties = d->propertiesFromXML(viewElement);
  this->layoutManager()->layoutLogic()->ApplyProperties(properties, viewNode, "relayout");
  return view;
}

//------------------------------------------------------------------------------
QList<vtkDMMLAbstractViewNode*> qDMMLLayoutViewFactory
::viewNodesFromXML(QDomElement viewElement)const
{
  Q_D(const qDMMLLayoutViewFactory);
  vtkDMMLLayoutLogic::ViewAttributes attributes =
    d->attributesFromXML(viewElement);
  vtkCollection* viewNodes =
    this->layoutManager()->layoutLogic()->GetViewsFromAttributes(attributes);

  QList<vtkDMMLAbstractViewNode*> res;
  for (vtkDMMLAbstractViewNode* node = nullptr;
       (node = vtkDMMLAbstractViewNode::SafeDownCast(viewNodes->GetNextItemAsObject()));)
    {
    res << node;
    }
  viewNodes->Delete();
  return res;
}

//------------------------------------------------------------------------------
QList<QWidget*> qDMMLLayoutViewFactory::createViewsFromXML(QDomElement viewElement)
{
  Q_D(qDMMLLayoutViewFactory);
  QList<vtkDMMLAbstractViewNode*> viewNodes = this->viewNodesFromXML(viewElement);

  QList<QWidget*> res;
  foreach(vtkDMMLAbstractViewNode* viewNode, viewNodes)
    {
    res << this->viewWidget(viewNode);
    vtkDMMLLayoutLogic::ViewProperties properties = d->propertiesFromXML(viewElement);
    this->layoutManager()->layoutLogic()->ApplyProperties(properties, viewNode, "relayout");
    }
  return res;
}

//------------------------------------------------------------------------------
void qDMMLLayoutViewFactory::setupView(QDomElement viewElement, QWidget* view)
{
  this->Superclass::setupView(viewElement, view);
  vtkDMMLAbstractViewNode* viewNode = this->viewNode(view);
  Q_ASSERT(viewNode);
  viewNode->SetMappedInLayout(1);
  view->setVisible(viewNode->GetVisibility());
  view->setWindowTitle(viewNode->GetName());
}
