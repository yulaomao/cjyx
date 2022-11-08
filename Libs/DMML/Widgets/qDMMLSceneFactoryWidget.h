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

#ifndef __qDMMLSceneFactoryWidget_h
#define __qDMMLSceneFactoryWidget_h

// Qt includes
#include <QPushButton>
#include <QString>

// CTK includes
#include <ctkPimpl.h>

#include "qDMMLWidgetsExport.h"

class vtkDMMLScene;
class vtkDMMLNode;
class qDMMLSceneFactoryWidgetPrivate;

class QDMML_WIDGETS_EXPORT qDMMLSceneFactoryWidget : public QWidget
{
  Q_OBJECT
public:
  qDMMLSceneFactoryWidget(QWidget* parent = nullptr);
  ~qDMMLSceneFactoryWidget() override;

  vtkDMMLScene* dmmlScene()const;

public slots:
  void generateScene();
  void deleteScene();
  ///
  /// Create and add a node given its classname to the scene associated with the factory
  /// Note: The scene has the ownership of the node and is responsible to delete it.
  vtkDMMLNode* generateNode(const QString& dmmlNodeClassName);
  void deleteNode(const QString& dmmlNodeID);

  vtkDMMLNode* generateNode();
  void deleteNode();

signals:
  void dmmlSceneChanged(vtkDMMLScene* scene);
  void dmmlNodeAdded(vtkDMMLNode* node);
  void dmmlNodeRemoved(vtkDMMLNode* node);

protected:
  QScopedPointer<qDMMLSceneFactoryWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLSceneFactoryWidget);
  Q_DISABLE_COPY(qDMMLSceneFactoryWidget);
};

#endif
