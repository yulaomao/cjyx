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

#ifndef __qDMMLWidget_h
#define __qDMMLWidget_h

// Qt includes
#include <QWidget>

// DMMLWidgets includes
#include "qDMMLWidgetsExport.h"
class qDMMLWidgetPrivate;

// VTK includes
class vtkDMMLScene;

/// Base class for any widget that requires a DMML Scene.
class QDMML_WIDGETS_EXPORT qDMMLWidget : public QWidget
{
  Q_OBJECT

public:
  typedef QWidget Superclass;
  explicit qDMMLWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
  ~qDMMLWidget() override;

  /// Return a pointer on the current DMML scene
  Q_INVOKABLE vtkDMMLScene* dmmlScene() const;

  /// Initialization that needs to be performed before creating the Qt application object.
  /// Sets default application attributes related to hi-DPI, OpenGL surface format initialization, etc.
  Q_INVOKABLE static void preInitializeApplication();

  /// Initialization that needs to be performed after application object is created.
  Q_INVOKABLE static void postInitializeApplication();

  // Get the pixmap from the icon that is the most suitable for current screen resolution.
  // Useful for cases when a widget cannot take a QIcon as input only as QPixmap.
  Q_INVOKABLE static QPixmap pixmapFromIcon(const QIcon& icon);

public slots:
  /// Set the DMML \a scene associated with the widget
  virtual void setDMMLScene(vtkDMMLScene* newScene);

signals:
  /// When designing custom qDMMLWidget in the designer, you can connect the
  /// dmmlSceneChanged signal directly to the aggregated DMML widgets that
  /// have a setDMMLScene slot.
  void dmmlSceneChanged(vtkDMMLScene*);

protected:
  QScopedPointer<qDMMLWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLWidget);
  Q_DISABLE_COPY(qDMMLWidget);
};

#endif
