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

#ifndef __qDMMLLayoutViewFactory_h
#define __qDMMLLayoutViewFactory_h

// Qt includes
#include <QStringList>

// CTK includes
#include <ctkLayoutViewFactory.h>
#include <ctkVTKObject.h>

// DMML includes
#include "qDMMLWidgetsExport.h"
class qDMMLLayoutManager;
class qDMMLLayoutViewFactoryPrivate;
class vtkDMMLAbstractViewNode;
class vtkDMMLNode;
class vtkDMMLScene;

// VTK includes
class vtkCollection;
class vtkObject;
class vtkRenderer;

class QDMML_WIDGETS_EXPORT qDMMLLayoutViewFactory : public ctkLayoutViewFactory
{
  Q_OBJECT
  QVTK_OBJECT
  /// This property controls the class name of the view nodes that are
  /// supported by the factory.
  /// The accessor MUST BE reimplemented in the derived class.
  /// \sa viewClassName(), isElementSupported, isViewNodeSupported
  Q_PROPERTY(QString viewClassName READ viewClassName);
public:
  /// Superclass typedef
  typedef ctkLayoutViewFactory Superclass;

  /// Constructors
  explicit qDMMLLayoutViewFactory(QObject* parent=nullptr);
  ~qDMMLLayoutViewFactory() override;

  /// Returns the viewClassName property value.
  /// Returns QString() by default. \note To be reimplemented in derived
  /// classes.
  /// \sa viewClassName
  virtual QString viewClassName()const;

  /// Returns true if the layoutElement matches the viewClassName
  /// \sa viewClassName, isViewNodeSupported()
  bool isElementSupported(QDomElement layoutElement)const override;

  /// Returns true if the viewNode matches the viewClassName.
  /// \sa viewClassName, isElementSupported()
  virtual bool isViewNodeSupported(vtkDMMLAbstractViewNode* viewNode)const;

  /// Return the layout manager. It means the view factory
  /// can only be used in one layout manager.
  /// \sa setLayoutManager()
  Q_INVOKABLE qDMMLLayoutManager* layoutManager()const;

  /// Set the layout manager
  /// \sa layoutManager
  Q_INVOKABLE void setLayoutManager(qDMMLLayoutManager*);

  /// Return the dmml scene that was set.
  /// \sa setDMMLScene()
  Q_INVOKABLE vtkDMMLScene* dmmlScene()const;

  Q_INVOKABLE QWidget* viewWidget(int id)const;
  Q_INVOKABLE QWidget* viewWidget(vtkDMMLAbstractViewNode* node)const;
  Q_INVOKABLE QWidget* viewWidget(const QString& name)const;
  Q_INVOKABLE QWidget* viewWidgetByLayoutLabel(const QString& layoutLabel)const;
  Q_INVOKABLE int viewCount()const;

  void beginSetupLayout() override;

  vtkDMMLAbstractViewNode* viewNode(QWidget* widget)const;

  /// Return all the names of the created view nodes.
  QStringList viewNodeNames() const;

  void setActiveViewNode(vtkDMMLAbstractViewNode* viewNode);
  vtkDMMLAbstractViewNode* activeViewNode()const;
  virtual vtkRenderer* activeRenderer()const;

public Q_SLOTS:
  /// Set the DMML scene to the factory and all the created views that are
  /// of type qDMMLWidget. Can be reimplemented if the view is not a
  /// qDMMLWidget.
  /// \sa dmmlScene()
  virtual void setDMMLScene(vtkDMMLScene* scene);

  virtual void onNodeAdded(vtkObject* scene, vtkObject* node);
  virtual void onNodeRemoved(vtkObject* scene, vtkObject* node);
  virtual void onNodeModified(vtkObject* node);
  virtual void onViewNodeAdded(vtkDMMLAbstractViewNode* node);
  virtual void onViewNodeRemoved(vtkDMMLAbstractViewNode* node);
  virtual void onViewNodeModified(vtkDMMLAbstractViewNode* node);

  /// Populate widgets from the scene. The widgets will then be used by viewFromXML()
  /// \sa viewFromXML()
  virtual void onSceneModified();

Q_SIGNALS:
  /// This signal emitted whenever a new view is created.
  void viewCreated(QWidget* createdView);

  /// This signal is emitted when the active view node changed.
  /// \sa setActiveViewNode()
  void activeViewNodeChanged(vtkDMMLAbstractViewNode*);


protected:
  QScopedPointer<qDMMLLayoutViewFactoryPrivate> d_ptr;

  void setupView(QDomElement viewElement, QWidget* view) override;
  /// Reimplemented to search view node matching QDomElement.
  /// \sa viewFromXML(), createViewFromNode()
  QWidget* createViewFromXML(QDomElement layoutElement) override;
  /// Reimplemented to support
  QList<QWidget*> createViewsFromXML(QDomElement layoutElement) override;

  virtual vtkDMMLAbstractViewNode* viewNodeFromXML(QDomElement viewElement)const;
  virtual QList<vtkDMMLAbstractViewNode*> viewNodesFromXML(QDomElement viewElement)const;

  /// Instantiate a QWidget for a given abstract view node.
  /// To be reimplemented
  /// \sa createViewFromXML
  virtual QWidget* createViewFromNode(vtkDMMLAbstractViewNode* node);
  virtual void deleteView(vtkDMMLAbstractViewNode* node);

private:
  Q_DECLARE_PRIVATE(qDMMLLayoutViewFactory);
  Q_DISABLE_COPY(qDMMLLayoutViewFactory);
};

#endif
