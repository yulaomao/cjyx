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

#ifndef __qDMMLLayoutWidget_h
#define __qDMMLLayoutWidget_h

// Qt includes
#include <QWidget>

// qDMML includes
#include "qDMMLWidgetsExport.h"

// DMML includes
#include <vtkDMMLLayoutNode.h>

class qDMMLLayoutWidgetPrivate;
class qDMMLLayoutManager;
class vtkDMMLScene;

class QDMML_WIDGETS_EXPORT qDMMLLayoutWidget : public QWidget
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef QWidget Superclass;

  /// Constructors
  explicit qDMMLLayoutWidget(QWidget* widget);
  ~qDMMLLayoutWidget() override;

  /// Layout manager
  Q_INVOKABLE qDMMLLayoutManager* layoutManager()const;
  /// Set layout manager (useful for specifying a specialized layout manager class)
  Q_INVOKABLE void setLayoutManager(qDMMLLayoutManager* layoutManager);

  /// Utility function that returns the dmml scene of the layout manager
  vtkDMMLScene* dmmlScene()const;
  /// Utility function that returns the current layout of the layout manager
  int layout()const;

public slots:
  /// Set the DMML \a scene to the layout manager
  void setDMMLScene(vtkDMMLScene* scene);

  /// Propagate to the layoutmanager
  void setLayout(int);

protected:
  qDMMLLayoutWidget(qDMMLLayoutWidgetPrivate* obj, QWidget* widget);
  QScopedPointer<qDMMLLayoutWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLLayoutWidget);
  Q_DISABLE_COPY(qDMMLLayoutWidget);
};

#endif
