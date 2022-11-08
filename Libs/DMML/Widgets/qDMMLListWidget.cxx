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
#include <QSortFilterProxyModel>

// CTK includes

// qDMML includes
#include "qDMMLListWidget.h"
//#include "qDMMLItemModel.h"
#include "qDMMLSceneTransformModel.h"

//------------------------------------------------------------------------------
class qDMMLListWidgetPrivate
{
  Q_DECLARE_PUBLIC(qDMMLListWidget);
protected:
  qDMMLListWidget* const q_ptr;
public:
  qDMMLListWidgetPrivate(qDMMLListWidget& object);
  void init();
};

//------------------------------------------------------------------------------
qDMMLListWidgetPrivate::qDMMLListWidgetPrivate(qDMMLListWidget& object)
  : q_ptr(&object)
{
}

//------------------------------------------------------------------------------
void qDMMLListWidgetPrivate::init()
{
  Q_Q(qDMMLListWidget);
  //p->QListView::setModel(new qDMMLItemModel(p));
  //p->QListView::setModel(new qDMMLSceneModel(p));
  ///new ctkModelTester(p->model(), p);

  qDMMLSceneTransformModel* sceneModel = new qDMMLSceneTransformModel(q);
  QSortFilterProxyModel* sortModel = new QSortFilterProxyModel(q);
  sortModel->setSourceModel(sceneModel);
  sortModel->setDynamicSortFilter(true);
  q->QListView::setModel(sortModel);
  q->setWrapping(true);
  q->setResizeMode(QListView::Adjust);
  q->setFlow(QListView::TopToBottom);
  // We have a problem when the model is reset (qDMMLSceneModel::setDMMLScene(0)),
  // the QSortFilterProxyModel doesn't realize that the rows have disappeared
  // and QSortFilterProxyModel::rowCount(QModelIndex) returns 1(the dmmlscene), which
  // is eventually called by the ctkModelTester slot connected to QSortFilterProxyModel
  // signal layoutAboutToBeChanged() which eventually calls testData on the valid QModelIndex
  //new ctkModelTester(p->model(), p);

  //ctkModelTester* tester = new ctkModelTester(p);
  //tester->setModel(transformModel);
}

//------------------------------------------------------------------------------
qDMMLListWidget::qDMMLListWidget(QWidget *_parent)
  : QListView(_parent)
  , d_ptr(new qDMMLListWidgetPrivate(*this))
{
  Q_D(qDMMLListWidget);
  d->init();
}

//------------------------------------------------------------------------------
qDMMLListWidget::~qDMMLListWidget() = default;

//------------------------------------------------------------------------------
void qDMMLListWidget::setDMMLScene(vtkDMMLScene* scene)
{
  QSortFilterProxyModel* sortModel = qobject_cast<QSortFilterProxyModel*>(this->model());
  qDMMLSceneModel* dmmlModel = qobject_cast<qDMMLSceneModel*>(sortModel->sourceModel());
  Q_ASSERT(dmmlModel);

  dmmlModel->setDMMLScene(scene);
  if (scene)
    {
    this->setRootIndex(sortModel->index(0, 0));
    sortModel->sort(0);
    sortModel->invalidate();
    }
}

//------------------------------------------------------------------------------
vtkDMMLScene* qDMMLListWidget::dmmlScene()const
{
  QSortFilterProxyModel* sortModel = qobject_cast<QSortFilterProxyModel*>(this->model());
  Q_ASSERT(qobject_cast<const qDMMLSceneModel*>(sortModel->sourceModel()));
  return qobject_cast<const qDMMLSceneModel*>(sortModel->sourceModel())->dmmlScene();
}
