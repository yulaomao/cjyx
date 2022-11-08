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

#ifndef __qDMMLThreeDWidget_h
#define __qDMMLThreeDWidget_h

// Qt includes
#include <QWidget>
class QResizeEvent;

// qDMMLWidget includes
#include "qDMMLWidget.h"
class qDMMLThreeDViewControllerWidget;
class qDMMLThreeDView;
class qDMMLThreeDWidgetPrivate;

// DMML includes
class vtkDMMLScene;
class vtkDMMLViewNode;

// DMMLLogic includes
class vtkDMMLViewLogic;

// VTK includes
class vtkCollection;

class QDMML_WIDGETS_EXPORT qDMMLThreeDWidget : public qDMMLWidget
{
  Q_OBJECT
  Q_PROPERTY(QString viewLabel READ viewLabel WRITE setViewLabel)
  Q_PROPERTY(QColor viewColor READ viewColor WRITE setViewColor)

public:
  /// Superclass typedef
  typedef qDMMLWidget Superclass;

  /// Constructors
  explicit qDMMLThreeDWidget(QWidget* parent = nullptr);
  ~qDMMLThreeDWidget() override;

  /// Get slice controller
  Q_INVOKABLE qDMMLThreeDViewControllerWidget* threeDController()const;

  /// Get the 3D View node observed by view.
  Q_INVOKABLE vtkDMMLViewNode* dmmlViewNode()const;

  /// \sa qDMMLSliceControllerWidget::viewLogic()
  Q_INVOKABLE vtkDMMLViewLogic* viewLogic()const;

  /// Get a reference to the underlying ThreeD View
  /// Becareful if you change the threeDView, you might
  /// unsynchronize the view from the nodes/logics.
  Q_INVOKABLE qDMMLThreeDView* threeDView()const;

  /// \sa qDMMLThreeDView::addDisplayableManager
  Q_INVOKABLE void addDisplayableManager(const QString& displayableManager);
  Q_INVOKABLE void getDisplayableManagers(vtkCollection* displayableManagers);

  /// \sa qDMMLThreeDViewControllerWidget::viewLabel()
  /// \sa setiewLabel()
  QString viewLabel()const;

  /// \sa qDMMLThreeDViewControllerWidget::viewLabel()
  /// \sa viewLabel()
  void setViewLabel(const QString& newViewLabel);

  /// \sa qDMMLThreeDViewControllerWidget::setQuadBufferStereoSupportEnabled
  Q_INVOKABLE void setQuadBufferStereoSupportEnabled(bool value);

  /// \sa qDMMLThreeDViewControllerWidget::viewColor()
  /// \sa setViewColor()
  QColor viewColor()const;

  /// \sa qDMMLThreeDViewControllerWidget::viewColor()
  /// \sa viewColor()
  void setViewColor(const QColor& newViewColor);

public slots:
  void setDMMLScene(vtkDMMLScene* newScene) override;

  /// Set the current \a viewNode to observe
  void setDMMLViewNode(vtkDMMLViewNode* newViewNode);

protected:
  QScopedPointer<qDMMLThreeDWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLThreeDWidget);
  Q_DISABLE_COPY(qDMMLThreeDWidget);
};

#endif
