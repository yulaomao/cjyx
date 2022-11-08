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

// qDMML includes
#include "qDMMLNodeFactory.h"

// DMML includes
#include <vtkDMMLNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

//------------------------------------------------------------------------------
class qDMMLNodeFactoryPrivate
{
public:
  qDMMLNodeFactoryPrivate()
    {
    this->DMMLScene = nullptr;
    }
  vtkDMMLScene * DMMLScene;
  QHash<QString, QString> BaseNames;
  QHash<QString, QString> Attributes;
};

//------------------------------------------------------------------------------
qDMMLNodeFactory::qDMMLNodeFactory(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qDMMLNodeFactoryPrivate)
{
}

//------------------------------------------------------------------------------
qDMMLNodeFactory::~qDMMLNodeFactory() = default;

//------------------------------------------------------------------------------
CTK_SET_CPP(qDMMLNodeFactory, vtkDMMLScene*, setDMMLScene, DMMLScene);
CTK_GET_CPP(qDMMLNodeFactory, vtkDMMLScene*, dmmlScene, DMMLScene);

//------------------------------------------------------------------------------
vtkDMMLNode* qDMMLNodeFactory::createNode(const QString& className)
{
  Q_D(qDMMLNodeFactory);

  if (!d->DMMLScene || className.isEmpty())
    {
    return nullptr;
    }
  vtkSmartPointer<vtkDMMLNode> node;
  node.TakeReference( d->DMMLScene->CreateNodeByClass( className.toUtf8() ) );

  Q_ASSERT_X(node, "createNode",
             QString("Failed to create node of type [%1]").arg(className).toUtf8());

  if (node == nullptr)
    {
    return nullptr;
    }

  emit this->nodeInstantiated(node);

  QString baseName;
  if (d->BaseNames.contains(className) &&
      !d->BaseNames[className].isEmpty())
    {
    baseName = d->BaseNames[className];
    }
  else
    {
    baseName = d->DMMLScene->GetTagByClassName(className.toUtf8());
    }
  node->SetName(d->DMMLScene->GetUniqueNameByString(baseName.toUtf8()));

  // Set node attributes
  // Attributes must be set before adding the node into the scene as the node
  // combobox filter might hide the node if the attributes are not set yet.
  // Ideally the qDMMLSortFilterProxyModel should listen the all the nodes and
  // when the attribute property is changed, make sure that it doesn't change
  // it's visibility
  foreach (const QString& attributeName, d->Attributes.keys())
    {
    node->SetAttribute(attributeName.toUtf8(),
                       d->Attributes[attributeName].toUtf8());
    }

  emit this->nodeInitialized(node);
  // maybe the node has been added into the scene by slots connected
  // to nodeInitialized.
  // If node is initialized from a default node then scene may be set but the node
  // is not added to the scene yet, therefore we check if the node is actually present
  // in the scene (and add it if it is not present).
  if (!node->GetScene() || !node->GetScene()->IsNodePresent(node))
    {
    vtkDMMLNode* nodeAdded = d->DMMLScene->AddNode(node);
    Q_ASSERT(nodeAdded == node ||
             node->GetSingletonTag() != nullptr);
    node = nodeAdded;
    }
  emit this->nodeAdded(node);

  return node;
}

//------------------------------------------------------------------------------
vtkDMMLNode* qDMMLNodeFactory::createNode(vtkDMMLScene* scene, const QString& className,
                                          const QHash<QString,QString>& attributes)
{
  Q_ASSERT(scene);
  QScopedPointer<qDMMLNodeFactory> factory(new qDMMLNodeFactory());
  factory->setDMMLScene(scene);
  // Loop over attribute map and update the factory
  foreach(const QString& key, attributes.keys())
    {
    factory->addAttribute(key, attributes.value(key));
    }
  // Instantiate and return the requested node
  return factory->createNode(className);
}

//------------------------------------------------------------------------------
void qDMMLNodeFactory::addAttribute(const QString& name, const QString& value)
{
  Q_D(qDMMLNodeFactory);
  d->Attributes.insert(name, value);
}

//------------------------------------------------------------------------------
void qDMMLNodeFactory::removeAttribute(const QString& name)
{
  Q_D(qDMMLNodeFactory);
  d->Attributes.remove(name);
}

//------------------------------------------------------------------------------
QString qDMMLNodeFactory::attribute(const QString& name)const
{
  Q_D(const qDMMLNodeFactory);
  return d->Attributes[name];
}

//------------------------------------------------------------------------------
void qDMMLNodeFactory::setBaseName(const QString& className, const QString& baseName)
{
  Q_D(qDMMLNodeFactory);
  d->BaseNames[className] = baseName;
}

//------------------------------------------------------------------------------
QString qDMMLNodeFactory::baseName(const QString& className)const
{
  Q_D(const qDMMLNodeFactory);
  if (!d->BaseNames.contains(className))
    {
    qWarning("qDMMLNodeFactory::baseName failed: class name %s not found", qPrintable(className));
    return QString();
    }
  return d->BaseNames[className];
}
