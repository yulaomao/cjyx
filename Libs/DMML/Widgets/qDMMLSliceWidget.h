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

#ifndef __qDMMLSliceWidget_h
#define __qDMMLSliceWidget_h

// CTK includes
#include <ctkPimpl.h>
#include <vtkVersion.h>

// qDMMLWidget includes
#include "qDMMLWidget.h"

#include "qDMMLWidgetsExport.h"

class qDMMLSliceView;
class qDMMLSliceWidgetPrivate;
class qDMMLSliceControllerWidget;
class vtkCollection;
class vtkDMMLScene;
class vtkDMMLNode;
class vtkDMMLSliceLogic;
class vtkDMMLSliceNode;
class vtkDMMLSliceCompositeNode;

class vtkAlgorithmOutput;
class vtkImageData;
class vtkInteractorObserver;
class vtkCornerAnnotation;
class vtkCollection;

class QDMML_WIDGETS_EXPORT qDMMLSliceWidget : public qDMMLWidget
{
  Q_OBJECT
  Q_PROPERTY(QString sliceOrientation READ sliceOrientation WRITE setSliceOrientation)
  Q_PROPERTY(QString sliceViewName READ sliceViewName WRITE setSliceViewName)
  Q_PROPERTY(QString sliceViewLabel READ sliceViewLabel WRITE setSliceViewLabel)
  Q_PROPERTY(QColor sliceViewColor READ sliceViewColor WRITE setSliceViewColor)

public:
  /// Superclass typedef
  typedef qDMMLWidget Superclass;

  /// Constructors
  explicit qDMMLSliceWidget(QWidget* parent = nullptr);
  ~qDMMLSliceWidget() override;

  /// Get slice controller
  Q_INVOKABLE qDMMLSliceControllerWidget* sliceController()const;

  /// \sa qDMMLSliceControllerWidget::dmmlSliceNode()
  /// \sa setDMMLSliceNode()
  Q_INVOKABLE vtkDMMLSliceNode* dmmlSliceNode()const;

  /// \sa qDMMLSliceControllerWidget::sliceLogic()
  Q_INVOKABLE vtkDMMLSliceLogic* sliceLogic()const;

  /// \sa qDMMLSliceControllerWidget::sliceOrientation()
  /// \sa setSliceOrientation()
  Q_INVOKABLE QString sliceOrientation()const;

  /// \sa qDMMLSliceControllerWidget::imageData()
  /// \sa setImageData();
  Q_INVOKABLE vtkAlgorithmOutput* imageDataConnection()const;

  /// \sa qDMMLSliceControllerWidget::dmmlSliceCompositeNode()
  Q_INVOKABLE vtkDMMLSliceCompositeNode* dmmlSliceCompositeNode()const;

  /// \sa qDMMLSliceControllerWidget::sliceViewName()
  /// \sa setSliceViewName()
  QString sliceViewName()const;

  /// \sa qDMMLSliceControllerWidget::sliceViewName()
  /// \sa sliceViewName()
  void setSliceViewName(const QString& newSliceViewName);

  /// \sa qDMMLSliceControllerWidget::sliceViewLabel()
  /// \sa setSliceViewLabel()
  QString sliceViewLabel()const;

  /// \sa qDMMLSliceControllerWidget::sliceViewLabel()
  /// \sa sliceViewLabel()
  void setSliceViewLabel(const QString& newSliceViewLabel);

  /// \sa qDMMLSliceControllerWidget::sliceViewColor()
  /// \sa setSliceViewColor()
  QColor sliceViewColor()const;

  /// \sa qDMMLSliceControllerWidget::sliceViewColor()
  /// \sa sliceViewColor()
  void setSliceViewColor(const QColor& newSliceViewColor);

  /// Returns the interactor style of the view
  /// A const vtkInteractorObserver pointer is returned as you shouldn't
  /// mess too much with it. If you do, be aware that you are probably
  /// unsynchronizing the view from the nodes/logics.
  Q_INVOKABLE vtkInteractorObserver* interactorStyle()const;

  /// Return the overlay corner annotation of the view
  Q_INVOKABLE vtkCornerAnnotation* overlayCornerAnnotation()const;

  /// propagates the logics to the qDMMLSliceControllerWidget
  Q_INVOKABLE void setSliceLogics(vtkCollection* logics);

  /// Get a reference to the underlying slice view. It is the widget that
  /// renders the view (contains vtkRenderWindow).
  /// \sa sliceController()
  Q_INVOKABLE qDMMLSliceView* sliceView()const;

public slots:
  void setDMMLScene(vtkDMMLScene * newScene) override;

  /// \sa qDMMLSliceControllerWidget::setDMMLSliceNode()
  /// \sa dmmlSliceNode()
  void setDMMLSliceNode(vtkDMMLSliceNode* newSliceNode);

  /// \sa qDMMLSliceControllerWidget::setImageData()
  /// \sa imageData()
  void setImageDataConnection(vtkAlgorithmOutput* newImageDataConnection);

  /// \sa qDMMLSliceControllerWidget::setSliceOrientation()
  /// \sa sliceOrientation()
  void setSliceOrientation(const QString& orientation);

  /// Fit slices to background
  void fitSliceToBackground();

signals:
  /// Signal emitted when editing of a node is requested from within the slice widget
  void nodeAboutToBeEdited(vtkDMMLNode* node);

protected:
  void showEvent(QShowEvent *) override;

  QScopedPointer<qDMMLSliceWidgetPrivate> d_ptr;

  /// Constructor allowing derived class to specify a specialized pimpl.
  ///
  /// \note You are responsible to call init() in the constructor of
  /// derived class. Doing so ensures the derived class is fully
  /// instantiated in case virtual method are called within init() itself.
  qDMMLSliceWidget(qDMMLSliceWidgetPrivate* obj, QWidget* parent);

private:
  Q_DECLARE_PRIVATE(qDMMLSliceWidget);
  Q_DISABLE_COPY(qDMMLSliceWidget);
};

#endif
