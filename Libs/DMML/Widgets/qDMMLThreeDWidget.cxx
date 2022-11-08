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
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QToolButton>

// CTK includes
#include <ctkPopupWidget.h>

// qDMML includes
#include "qDMMLThreeDViewControllerWidget.h"
#include "qDMMLThreeDView.h"
#include "qDMMLThreeDWidget.h"

// DMML includes
#include <vtkDMMLScene.h>
#include <vtkDMMLViewLogic.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkCollection.h>

//--------------------------------------------------------------------------
// qDMMLSliceViewPrivate
class qDMMLThreeDWidgetPrivate
  : public QObject
{
  Q_DECLARE_PUBLIC(qDMMLThreeDWidget);
protected:
  qDMMLThreeDWidget* const q_ptr;
public:
  qDMMLThreeDWidgetPrivate(qDMMLThreeDWidget& object);
  ~qDMMLThreeDWidgetPrivate() override;

  void init();

  qDMMLThreeDView* ThreeDView;
  qDMMLThreeDViewControllerWidget* ThreeDController;
};


//---------------------------------------------------------------------------
qDMMLThreeDWidgetPrivate::qDMMLThreeDWidgetPrivate(qDMMLThreeDWidget& object)
  : q_ptr(&object)
{
  this->ThreeDView = nullptr;
  this->ThreeDController = nullptr;
}

//---------------------------------------------------------------------------
qDMMLThreeDWidgetPrivate::~qDMMLThreeDWidgetPrivate() = default;

//---------------------------------------------------------------------------
void qDMMLThreeDWidgetPrivate::init()
{
  Q_Q(qDMMLThreeDWidget);

  QVBoxLayout* layout = new QVBoxLayout(q);
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);

  this->ThreeDController = new qDMMLThreeDViewControllerWidget;
  layout->addWidget(this->ThreeDController);

  this->ThreeDView = new qDMMLThreeDView;
  this->ThreeDView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  layout->addWidget(this->ThreeDView);

  this->ThreeDController->setThreeDView(this->ThreeDView);

  QObject::connect(q, SIGNAL(dmmlSceneChanged(vtkDMMLScene*)),
                   this->ThreeDView, SLOT(setDMMLScene(vtkDMMLScene*)));

  QObject::connect(q, SIGNAL(dmmlSceneChanged(vtkDMMLScene*)),
                   this->ThreeDController, SLOT(setDMMLScene(vtkDMMLScene*)));
}

// --------------------------------------------------------------------------
// qDMMLThreeDWidget methods

// --------------------------------------------------------------------------
qDMMLThreeDWidget::qDMMLThreeDWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qDMMLThreeDWidgetPrivate(*this))
{
  Q_D(qDMMLThreeDWidget);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLThreeDWidget::~qDMMLThreeDWidget() = default;

// --------------------------------------------------------------------------
void qDMMLThreeDWidget::addDisplayableManager(const QString& dManager)
{
  Q_D(qDMMLThreeDWidget);
  d->ThreeDView->addDisplayableManager(dManager);
}

// --------------------------------------------------------------------------
void qDMMLThreeDWidget::setDMMLViewNode(vtkDMMLViewNode* newViewNode)
{
  Q_D(qDMMLThreeDWidget);
  if (!newViewNode)
    {
    qWarning() << Q_FUNC_INFO << " failed: view node is invalid";
    return;
    }
  vtkDMMLViewLogic* viewLogic = this->viewLogic();
  if (!viewLogic)
    {
    qWarning() << Q_FUNC_INFO << " failed: view logic is invalid";
    return;
    }
  d->ThreeDView->setDMMLViewNode(newViewNode);
  viewLogic->SetName(newViewNode->GetLayoutName());
}

// --------------------------------------------------------------------------
vtkDMMLViewNode* qDMMLThreeDWidget::dmmlViewNode()const
{
  Q_D(const qDMMLThreeDWidget);
  return d->ThreeDView->dmmlViewNode();
}

// --------------------------------------------------------------------------
vtkDMMLViewLogic* qDMMLThreeDWidget::viewLogic() const
{
  Q_D(const qDMMLThreeDWidget);

  return d->ThreeDController->viewLogic();
}

// --------------------------------------------------------------------------
qDMMLThreeDView* qDMMLThreeDWidget::threeDView()const
{
  Q_D(const qDMMLThreeDWidget);
  return d->ThreeDView;
}

// --------------------------------------------------------------------------
qDMMLThreeDViewControllerWidget* qDMMLThreeDWidget::threeDController() const
{
  Q_D(const qDMMLThreeDWidget);
  return d->ThreeDController;
}

//---------------------------------------------------------------------------
void qDMMLThreeDWidget::setViewLabel(const QString& newViewLabel)
{
  Q_D(qDMMLThreeDWidget);
  d->ThreeDController->setViewLabel(newViewLabel);
}

//---------------------------------------------------------------------------
QString qDMMLThreeDWidget::viewLabel()const
{
  Q_D(const qDMMLThreeDWidget);
  return d->ThreeDController->viewLabel();
}

//---------------------------------------------------------------------------
void qDMMLThreeDWidget::setQuadBufferStereoSupportEnabled(bool value)
{
  Q_D(qDMMLThreeDWidget);
  d->ThreeDController->setQuadBufferStereoSupportEnabled(value);
}

//---------------------------------------------------------------------------
void qDMMLThreeDWidget::setViewColor(const QColor& newViewColor)
{
  Q_D(qDMMLThreeDWidget);
  if (!this->viewLogic() || !this->viewLogic()->GetViewNode())
    {
    qWarning() << Q_FUNC_INFO << " failed: view node is invalid";
    return;
    }

  double layoutColor[3] = { newViewColor.redF(), newViewColor.greenF(), newViewColor.blueF() };
  this->viewLogic()->GetViewNode()->SetLayoutColor(layoutColor);
}

//---------------------------------------------------------------------------
QColor qDMMLThreeDWidget::viewColor()const
{
  Q_D(const qDMMLThreeDWidget);
  if (!this->viewLogic() || !this->viewLogic()->GetViewNode())
    {
    qWarning() << Q_FUNC_INFO << " failed: view node is invalid";
    return QColor(127, 127, 127);
    }
  double* layoutColorVtk = this->viewLogic()->GetViewNode()->GetLayoutColor();
  QColor layoutColor = QColor::fromRgbF(layoutColorVtk[0], layoutColorVtk[1], layoutColorVtk[2]);
  return layoutColor;
}

//---------------------------------------------------------------------------
void qDMMLThreeDWidget::setDMMLScene(vtkDMMLScene* newScene)
{
  Q_D(qDMMLThreeDWidget);

  this->Superclass::setDMMLScene(newScene);
}

//------------------------------------------------------------------------------
void qDMMLThreeDWidget::getDisplayableManagers(vtkCollection* displayableManagers)
{
  Q_D(qDMMLThreeDWidget);
  d->ThreeDView->getDisplayableManagers(displayableManagers);
}
