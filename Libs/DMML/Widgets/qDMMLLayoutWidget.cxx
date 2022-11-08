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

// qDMML includes
#include "qDMMLLayoutManager.h"
#include "qDMMLLayoutWidget.h"

class qDMMLLayoutWidgetPrivate
{
public:
  qDMMLLayoutManager* LayoutManager;
};

//------------------------------------------------------------------------------
// qDMMLLayoutWidget methods

// --------------------------------------------------------------------------
qDMMLLayoutWidget::qDMMLLayoutWidget(QWidget* widget)
  : Superclass(widget)
  , d_ptr(new qDMMLLayoutWidgetPrivate)
{
  this->setLayoutManager(new qDMMLLayoutManager);
}

// --------------------------------------------------------------------------
qDMMLLayoutWidget::~qDMMLLayoutWidget() = default;

//------------------------------------------------------------------------------
qDMMLLayoutManager* qDMMLLayoutWidget::layoutManager()const
{
  Q_D(const qDMMLLayoutWidget);
  return d->LayoutManager;
}

//------------------------------------------------------------------------------
void qDMMLLayoutWidget::setLayoutManager(qDMMLLayoutManager* layoutManager)
{
  Q_D(qDMMLLayoutWidget);
  d->LayoutManager = layoutManager;
  d->LayoutManager->setParent(this);
  d->LayoutManager->setViewport(this);
}

//------------------------------------------------------------------------------
void qDMMLLayoutWidget::setDMMLScene(vtkDMMLScene* scene)
{
  Q_D(qDMMLLayoutWidget);
  d->LayoutManager->setDMMLScene(scene);
}

//------------------------------------------------------------------------------
vtkDMMLScene* qDMMLLayoutWidget::dmmlScene()const
{
  Q_D(const qDMMLLayoutWidget);
  return d->LayoutManager->dmmlScene();
}

//------------------------------------------------------------------------------
int qDMMLLayoutWidget::layout()const
{
  Q_D(const qDMMLLayoutWidget);
  return d->LayoutManager->layout();
}

//------------------------------------------------------------------------------
void qDMMLLayoutWidget::setLayout(int layout)
{
  Q_D(qDMMLLayoutWidget);
  d->LayoutManager->setLayout(layout);
}
