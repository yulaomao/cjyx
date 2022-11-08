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

#ifndef __qDMMLSceneViewMenu_h
#define __qDMMLSceneViewMenu_h

// Qt includes
#include <QMenu>

// CTK includes
#include <ctkVTKObject.h>

#include "qDMMLWidgetsExport.h"

class qDMMLSceneViewMenuPrivate;
class vtkDMMLScene;
class vtkDMMLNode;

class QDMML_WIDGETS_EXPORT qDMMLSceneViewMenu : public QMenu
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(QString NoSceneViewText READ noSceneViewText WRITE setNoSceneViewText)
public:
  /// Superclass typedef
  typedef QMenu Superclass;

  /// Constructors
  explicit qDMMLSceneViewMenu(QWidget* newParent = nullptr);
  ~qDMMLSceneViewMenu() override;

  /// Return a pointer on the current DMML scene
  vtkDMMLScene* dmmlScene() const;

  /// This property holds the menu's text displayed when there are no scene views
  QString noSceneViewText()const;
  void setNoSceneViewText(const QString& newText);

public slots:

  /// Set the DMML \a scene associated with the widget
  virtual void setDMMLScene(vtkDMMLScene* scene);

signals:
  void dmmlSceneChanged(vtkDMMLScene*);

protected:
  QScopedPointer<qDMMLSceneViewMenuPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLSceneViewMenu);
  Q_DISABLE_COPY(qDMMLSceneViewMenu);

};

#endif
