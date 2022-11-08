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
#include <QDebug>
#include <QRandomGenerator>

// qDMML includes
#include "qDMMLSceneFactoryWidget.h"
#include "qDMMLNodeFactory.h"
#include "ui_qDMMLSceneFactoryWidget.h"

// DMML includes
#include <vtkDMMLNode.h>
#include <vtkDMMLScene.h>

class qDMMLSceneFactoryWidgetPrivate: public Ui_qDMMLSceneFactoryWidget
{
  Q_DECLARE_PUBLIC(qDMMLSceneFactoryWidget);
protected:
  qDMMLSceneFactoryWidget* const q_ptr;
public:
  qDMMLSceneFactoryWidgetPrivate(qDMMLSceneFactoryWidget& object);
  void init();
  void setNodeActionsEnabled(bool enable);

  vtkDMMLScene*  DMMLScene;
  QRandomGenerator RandomGenerator;
};

// --------------------------------------------------------------------------
qDMMLSceneFactoryWidgetPrivate::qDMMLSceneFactoryWidgetPrivate(qDMMLSceneFactoryWidget& object)
  : q_ptr(&object)
{
  this->DMMLScene = nullptr;
  // RandomGenerator is not seeded with random number to make behavior reproducible
}

// --------------------------------------------------------------------------
void qDMMLSceneFactoryWidgetPrivate::init()
{
  Q_Q(qDMMLSceneFactoryWidget);
  this->setupUi(q);
  QObject::connect(this->NewSceneButton, SIGNAL(clicked()), q, SLOT(generateScene()));
  QObject::connect(this->DeleteSceneButton, SIGNAL(clicked()), q, SLOT(deleteScene()));
  QObject::connect(this->NewNodeButton, SIGNAL(clicked()), q, SLOT(generateNode()));
  QObject::connect(this->DeleteNodeButton, SIGNAL(clicked()), q, SLOT(deleteNode()));
}

// --------------------------------------------------------------------------
void qDMMLSceneFactoryWidgetPrivate::setNodeActionsEnabled(bool enable)
{
  this->NewNodeButton->setEnabled(enable);
  this->NewNodeLineEdit->setEnabled(enable);
  this->DeleteNodeButton->setEnabled(enable);
  this->DeleteNodeLineEdit->setEnabled(enable);
}

// --------------------------------------------------------------------------
// qDMMLSceneFactoryWidget methods

// --------------------------------------------------------------------------
qDMMLSceneFactoryWidget::qDMMLSceneFactoryWidget(QWidget* _parent)
  : QWidget(_parent)
  , d_ptr(new qDMMLSceneFactoryWidgetPrivate(*this))
{
  Q_D(qDMMLSceneFactoryWidget);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLSceneFactoryWidget::~qDMMLSceneFactoryWidget()
{
  this->deleteScene();
}

// --------------------------------------------------------------------------
void qDMMLSceneFactoryWidget::generateScene()
{
  Q_D(qDMMLSceneFactoryWidget);

  if (d->DMMLScene)
    {
    d->DMMLScene->Delete();
    }
  d->DMMLScene = vtkDMMLScene::New();
  d->setNodeActionsEnabled(true);
  d->DeleteSceneButton->setEnabled(true);
  emit dmmlSceneChanged(d->DMMLScene);
}

// --------------------------------------------------------------------------
void qDMMLSceneFactoryWidget::deleteScene()
{
  Q_D(qDMMLSceneFactoryWidget);
  if (!d->DMMLScene)
    {
    return;
    }
  d->setNodeActionsEnabled(false);
  d->DeleteSceneButton->setEnabled(false);
  emit this->dmmlSceneChanged(nullptr);
  d->DMMLScene->Delete();
  d->DMMLScene = nullptr;
}

// --------------------------------------------------------------------------
vtkDMMLScene* qDMMLSceneFactoryWidget::dmmlScene()const
{
  Q_D(const qDMMLSceneFactoryWidget);
  return d->DMMLScene;
}

// --------------------------------------------------------------------------
vtkDMMLNode* qDMMLSceneFactoryWidget::generateNode()
{
  Q_D(qDMMLSceneFactoryWidget);
  Q_ASSERT(d->DMMLScene != nullptr);
  QString nodeClassName = d->NewNodeLineEdit->text();
  if (nodeClassName.isEmpty())
    {
    int numClasses = d->DMMLScene->GetNumberOfRegisteredNodeClasses();
    int classNumber = 0;
    vtkDMMLNode* node = nullptr;
    while (!node || node->GetSingletonTag() || node->IsA("vtkDMMLSubjectHierarchyNode"))
      {
      classNumber = d->RandomGenerator.generate() % numClasses;
      node = d->DMMLScene->GetNthRegisteredNodeClass(classNumber);
      Q_ASSERT(node);
      }
    nodeClassName = QString::fromUtf8(node->GetClassName());
    if (nodeClassName.isEmpty())
      {
      qWarning() << "Class registered (#" << classNumber << "):"
                 << node << " has an empty classname";
      }
    }
  return this->generateNode(nodeClassName);
}

// --------------------------------------------------------------------------
vtkDMMLNode* qDMMLSceneFactoryWidget::generateNode(const QString& className)
{
  Q_D(qDMMLSceneFactoryWidget);
  Q_ASSERT(!className.isEmpty());
  Q_ASSERT(d->DMMLScene != nullptr);
  vtkDMMLNode* node = qDMMLNodeFactory::createNode(d->DMMLScene, className);
  Q_ASSERT(node);
  if (node)
    {
    emit dmmlNodeAdded(node);
    }
  return node;
}

// --------------------------------------------------------------------------
void qDMMLSceneFactoryWidget::deleteNode()
{
  Q_D(qDMMLSceneFactoryWidget);
  Q_ASSERT(d->DMMLScene != nullptr);
  QString nodeClassName = d->DeleteNodeLineEdit->text();
  if (!nodeClassName.isEmpty())
    {
    this->deleteNode(nodeClassName);
    return;
    }
  int numNodes = d->DMMLScene->GetNumberOfNodes();
  if (numNodes == 0)
    {
    return;
    }
  vtkDMMLNode* node = d->DMMLScene->GetNthNode(d->RandomGenerator.generate() % numNodes);
  d->DMMLScene->RemoveNode(node);
  // FIXME: disable delete button when there is no more nodes in the scene to delete
  emit dmmlNodeRemoved(node);
}

// --------------------------------------------------------------------------
void qDMMLSceneFactoryWidget::deleteNode(const QString& className)
{
  Q_D(qDMMLSceneFactoryWidget);
  Q_ASSERT(!className.isEmpty());
  Q_ASSERT(d->DMMLScene != nullptr);
  int numNodes = d->DMMLScene->GetNumberOfNodesByClass(className.toUtf8());
  if (numNodes == 0)
    {
    qDebug() << "qDMMLSceneFactoryWidget::deleteNode(" <<className <<") no node";
    return;
    }
  vtkDMMLNode* node = d->DMMLScene->GetNthNodeByClass(d->RandomGenerator.generate() % numNodes, className.toUtf8());
  qDebug() << "qDMMLSceneFactoryWidget::deleteNode(" <<className <<") ==" << node->GetClassName();
  d->DMMLScene->RemoveNode(node);
  // FIXME: disable delete button when there is no more nodes in the scene to delete
  emit dmmlNodeRemoved(node);
}
