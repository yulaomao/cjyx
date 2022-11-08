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
#include <QApplication>
#include <QMainWindow>
#include <QWindow>

// qDMML includes
#include "qDMMLSliceWidget_p.h"

// DMMLDisplayableManager includes
#include <vtkDMMLSliceViewInteractorStyle.h>

// DMML includes
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkCollection.h>
#include <vtkWeakPointer.h>


//--------------------------------------------------------------------------
// qDMMLSliceWidgetPrivate methods

//---------------------------------------------------------------------------
qDMMLSliceWidgetPrivate::qDMMLSliceWidgetPrivate(qDMMLSliceWidget& object)
  : q_ptr(&object)
{
}

//---------------------------------------------------------------------------
qDMMLSliceWidgetPrivate::~qDMMLSliceWidgetPrivate() = default;

//---------------------------------------------------------------------------
void qDMMLSliceWidgetPrivate::init()
{
  Q_Q(qDMMLSliceWidget);
  this->setupUi(q);

  this->SliceView->sliceViewInteractorStyle()
    ->SetSliceLogic(this->SliceController->sliceLogic());

  connect(this->SliceView, SIGNAL(resized(QSize)),
          this, SLOT(setSliceViewSize(QSize)));

  connect(this->SliceController, SIGNAL(imageDataConnectionChanged(vtkAlgorithmOutput*)),
          this, SLOT(setImageDataConnection(vtkAlgorithmOutput*)));
  connect(this->SliceController, SIGNAL(renderRequested()),
          this->SliceView, SLOT(scheduleRender()), Qt::QueuedConnection);
}

// --------------------------------------------------------------------------
void qDMMLSliceWidgetPrivate::setSliceViewSize(const QSize& size)
{
  QSizeF scaledSizeF = QSizeF(size) * this->SliceView->devicePixelRatioF();
  this->SliceController->setSliceViewSize(scaledSizeF.toSize());
}

// --------------------------------------------------------------------------
void qDMMLSliceWidgetPrivate::resetSliceViewSize()
{
  this->setSliceViewSize(this->SliceView->size());
}

// --------------------------------------------------------------------------
void qDMMLSliceWidgetPrivate::endProcessing()
{
  // When a scene is closed, we need to reconfigure the SliceNode to
  // the size of the widget.
  this->setSliceViewSize(this->SliceView->size());
}

// --------------------------------------------------------------------------
void qDMMLSliceWidgetPrivate::setImageDataConnection(vtkAlgorithmOutput * imageDataConnection)
{
  //qDebug() << "qDMMLSliceWidgetPrivate::setImageDataConnection";
  this->SliceView->setImageDataConnection(imageDataConnection);
}

// --------------------------------------------------------------------------
// qDMMLSliceView methods

// --------------------------------------------------------------------------
qDMMLSliceWidget::qDMMLSliceWidget(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qDMMLSliceWidgetPrivate(*this))
{
  Q_D(qDMMLSliceWidget);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLSliceWidget::qDMMLSliceWidget(qDMMLSliceWidgetPrivate* pimpl,
                                   QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(pimpl)
{
  // Note: You are responsible to call init() in the constructor of derived class.
}

// --------------------------------------------------------------------------
qDMMLSliceWidget::~qDMMLSliceWidget() = default;

//---------------------------------------------------------------------------
void qDMMLSliceWidget::setDMMLScene(vtkDMMLScene* newScene)
{
  Q_D(qDMMLSliceWidget);

  this->Superclass::setDMMLScene(newScene);

  d->qvtkReconnect(
    this->dmmlScene(), newScene,
    vtkDMMLScene::EndBatchProcessEvent, d, SLOT(endProcessing()));
}

//---------------------------------------------------------------------------
void qDMMLSliceWidget::setDMMLSliceNode(vtkDMMLSliceNode* newSliceNode)
{
  Q_D(qDMMLSliceWidget);
  d->SliceController->setDMMLSliceNode(newSliceNode);
  d->SliceView->setDMMLSliceNode(newSliceNode);
}

//---------------------------------------------------------------------------
vtkDMMLSliceCompositeNode* qDMMLSliceWidget::dmmlSliceCompositeNode()const
{
  Q_D(const qDMMLSliceWidget);
  return d->SliceController->dmmlSliceCompositeNode();
}

//---------------------------------------------------------------------------
void qDMMLSliceWidget::setSliceViewName(const QString& newSliceViewName)
{
  Q_D(qDMMLSliceWidget);
  d->SliceController->setSliceViewName(newSliceViewName);
}

//---------------------------------------------------------------------------
QString qDMMLSliceWidget::sliceViewName()const
{
  Q_D(const qDMMLSliceWidget);
  return d->SliceController->sliceViewName();
}

//---------------------------------------------------------------------------
void qDMMLSliceWidget::setSliceViewLabel(const QString& newSliceViewLabel)
{
  Q_D(qDMMLSliceWidget);
  d->SliceController->setSliceViewLabel(newSliceViewLabel);
}

//---------------------------------------------------------------------------
QString qDMMLSliceWidget::sliceViewLabel()const
{
  Q_D(const qDMMLSliceWidget);
  return d->SliceController->sliceViewLabel();
}

//---------------------------------------------------------------------------
void qDMMLSliceWidget::setSliceViewColor(const QColor& newSliceViewColor)
{
  Q_D(qDMMLSliceWidget);
  d->SliceController->setSliceViewColor(newSliceViewColor);
}

//---------------------------------------------------------------------------
QColor qDMMLSliceWidget::sliceViewColor()const
{
  Q_D(const qDMMLSliceWidget);
  return d->SliceController->sliceViewColor();
}

//---------------------------------------------------------------------------
void qDMMLSliceWidget::setSliceOrientation(const QString& orientation)
{
  Q_D(qDMMLSliceWidget);
  d->SliceController->setSliceOrientation(orientation);
}

//---------------------------------------------------------------------------
QString qDMMLSliceWidget::sliceOrientation()const
{
  Q_D(const qDMMLSliceWidget);
  return d->SliceController->sliceOrientation();
}

//---------------------------------------------------------------------------
void qDMMLSliceWidget::setImageDataConnection(vtkAlgorithmOutput* newImageDataConnection)
{
  Q_D(qDMMLSliceWidget);
  d->SliceController->setImageDataConnection(newImageDataConnection);
}

//---------------------------------------------------------------------------
vtkAlgorithmOutput* qDMMLSliceWidget::imageDataConnection() const
{
  Q_D(const qDMMLSliceWidget);
  return d->SliceController->imageDataConnection();
}

//---------------------------------------------------------------------------
vtkInteractorObserver* qDMMLSliceWidget::interactorStyle()const
{
  return this->sliceView()->interactorStyle();
}

//---------------------------------------------------------------------------
vtkCornerAnnotation* qDMMLSliceWidget::overlayCornerAnnotation()const
{
  return this->sliceView()->overlayCornerAnnotation();
}

//---------------------------------------------------------------------------
vtkDMMLSliceNode* qDMMLSliceWidget::dmmlSliceNode()const
{
  Q_D(const qDMMLSliceWidget);
  return d->SliceController->dmmlSliceNode();
}

//---------------------------------------------------------------------------
vtkDMMLSliceLogic* qDMMLSliceWidget::sliceLogic()const
{
  Q_D(const qDMMLSliceWidget);
  return d->SliceController->sliceLogic();
}

// --------------------------------------------------------------------------
void qDMMLSliceWidget::fitSliceToBackground()
{
  Q_D(qDMMLSliceWidget);
  d->SliceController->fitSliceToBackground();
}

// --------------------------------------------------------------------------
qDMMLSliceView* qDMMLSliceWidget::sliceView()const
{
  Q_D(const qDMMLSliceWidget);
  return d->SliceView;
}

// --------------------------------------------------------------------------
qDMMLSliceControllerWidget* qDMMLSliceWidget::sliceController()const
{
  Q_D(const qDMMLSliceWidget);
  return d->SliceController;
}

// --------------------------------------------------------------------------
void qDMMLSliceWidget::setSliceLogics(vtkCollection* logics)
{
  Q_D(qDMMLSliceWidget);
  d->SliceController->setSliceLogics(logics);
}

// --------------------------------------------------------------------------
void qDMMLSliceWidget::showEvent(QShowEvent* event)
{
  Superclass::showEvent(event);

  Q_D(qDMMLSliceWidget);

  // Reset slice view size when screen changes to account for a possible change
  // in the device pixel ratio.
  QWindow* window = nullptr;
  foreach(QWidget* widget, qApp->topLevelWidgets())
    {
    QMainWindow* mainWindow = qobject_cast<QMainWindow*>(widget);
    if (mainWindow)
      {
      window = mainWindow->windowHandle();
      break;
      }
    }
  if (window)
    {
    connect(window, SIGNAL(screenChanged(QScreen*)),
            d, SLOT(resetSliceViewSize()), Qt::UniqueConnection);
    }
}
