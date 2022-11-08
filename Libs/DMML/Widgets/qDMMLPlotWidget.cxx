/*==============================================================================

  Copyright (c) Kapteyn Astronomical Institute
  University of Groningen, Groningen, Netherlands. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Davide Punzo, Kapteyn Astronomical Institute,
  and was supported through the European Research Council grant nr. 291531.

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
#include "qDMMLPlotViewControllerWidget.h"
#include "qDMMLPlotView.h"
#include "qDMMLPlotWidget.h"

//--------------------------------------------------------------------------
// qDMMLSliceViewPrivate
class qDMMLPlotWidgetPrivate
  : public QObject
{
  Q_DECLARE_PUBLIC(qDMMLPlotWidget);
protected:
  qDMMLPlotWidget* const q_ptr;
public:
  qDMMLPlotWidgetPrivate(qDMMLPlotWidget& object);
  ~qDMMLPlotWidgetPrivate() override;

  void init();

  qDMMLPlotView*       PlotView;
  qDMMLPlotViewControllerWidget* PlotController;
};


//---------------------------------------------------------------------------
qDMMLPlotWidgetPrivate::qDMMLPlotWidgetPrivate(qDMMLPlotWidget& object)
  : q_ptr(&object)
{
  this->PlotView = nullptr;
  this->PlotController = nullptr;
}

//---------------------------------------------------------------------------
qDMMLPlotWidgetPrivate::~qDMMLPlotWidgetPrivate() = default;

//---------------------------------------------------------------------------
void qDMMLPlotWidgetPrivate::init()
{
  Q_Q(qDMMLPlotWidget);

  QVBoxLayout* layout = new QVBoxLayout(q);
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);

  this->PlotController = new qDMMLPlotViewControllerWidget;
  layout->addWidget(this->PlotController);

  this->PlotView = new qDMMLPlotView;
  this->PlotView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->addWidget(this->PlotView);

  this->PlotController->setPlotView(this->PlotView);

  QObject::connect(q, SIGNAL(dmmlSceneChanged(vtkDMMLScene*)),
                   this->PlotView, SLOT(setDMMLScene(vtkDMMLScene*)));
  QObject::connect(q, SIGNAL(dmmlSceneChanged(vtkDMMLScene*)),
                   this->PlotController, SLOT(setDMMLScene(vtkDMMLScene*)));
}

// --------------------------------------------------------------------------
// qDMMLPlotWidget methods

// --------------------------------------------------------------------------
qDMMLPlotWidget::qDMMLPlotWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qDMMLPlotWidgetPrivate(*this))
{
  Q_D(qDMMLPlotWidget);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLPlotWidget::~qDMMLPlotWidget()
{
  Q_D(qDMMLPlotWidget);
  d->PlotView->setDMMLScene(nullptr);
  d->PlotController->setDMMLScene(nullptr);
}


// --------------------------------------------------------------------------
void qDMMLPlotWidget::setDMMLPlotViewNode(vtkDMMLPlotViewNode* newPlotViewNode)
{
  Q_D(qDMMLPlotWidget);
  d->PlotView->setDMMLPlotViewNode(newPlotViewNode);
  d->PlotController->setDMMLPlotViewNode(newPlotViewNode);
}

// --------------------------------------------------------------------------
vtkDMMLPlotViewNode* qDMMLPlotWidget::dmmlPlotViewNode()const
{
  Q_D(const qDMMLPlotWidget);
  return d->PlotView->dmmlPlotViewNode();
}

// --------------------------------------------------------------------------
qDMMLPlotView* qDMMLPlotWidget::plotView()const
{
  Q_D(const qDMMLPlotWidget);
  return d->PlotView;
}

// --------------------------------------------------------------------------
qDMMLPlotViewControllerWidget* qDMMLPlotWidget::plotController()const
{
  Q_D(const qDMMLPlotWidget);
  return d->PlotController;
}

//---------------------------------------------------------------------------
void qDMMLPlotWidget::setViewLabel(const QString& newPlotViewLabel)
{
  Q_D(qDMMLPlotWidget);
  d->PlotController->setViewLabel(newPlotViewLabel);
}

//---------------------------------------------------------------------------
QString qDMMLPlotWidget::viewLabel()const
{
  Q_D(const qDMMLPlotWidget);
  return d->PlotController->viewLabel();
}
