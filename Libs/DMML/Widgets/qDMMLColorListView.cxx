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

// qDMML includes
#include "qDMMLColorListView.h"
#include "qDMMLColorModel.h"

// DMML includes
#include <vtkDMMLColorNode.h>

//------------------------------------------------------------------------------
class qDMMLColorListViewPrivate
{
  Q_DECLARE_PUBLIC(qDMMLColorListView);
protected:
  qDMMLColorListView* const q_ptr;
public:
  qDMMLColorListViewPrivate(qDMMLColorListView& object);
  void init();
};

//------------------------------------------------------------------------------
qDMMLColorListViewPrivate::qDMMLColorListViewPrivate(qDMMLColorListView& object)
  : q_ptr(&object)
{
}

//------------------------------------------------------------------------------
void qDMMLColorListViewPrivate::init()
{
  Q_Q(qDMMLColorListView);

  qDMMLColorModel* colorModel = new qDMMLColorModel(q);
  colorModel->setLabelColumn(0);
  QSortFilterProxyModel* sortFilterModel = new QSortFilterProxyModel(q);
  sortFilterModel->setSourceModel(colorModel);
  q->setModel(sortFilterModel);

  q->setEditTriggers(QAbstractItemView::NoEditTriggers);
  //q->setWrapping(true);
  //q->setResizeMode(QListView::Adjust);
  //q->setFlow(QListView::TopToBottom);
  //q->setRootIndex(sortFilterModel->mapFromSource(colorModel->dmmlColorNodeIndex()));

  //QObject::connect(q, SIGNAL(activated(QModelIndex)),
  //                 q, SLOT(onItemActivated(QModelIndex)));
}

//------------------------------------------------------------------------------
qDMMLColorListView::qDMMLColorListView(QWidget *_parent)
  : QListView(_parent)
  , d_ptr(new qDMMLColorListViewPrivate(*this))
{
  Q_D(qDMMLColorListView);
  d->init();
}

//------------------------------------------------------------------------------
qDMMLColorListView::~qDMMLColorListView() = default;

//------------------------------------------------------------------------------
qDMMLColorModel* qDMMLColorListView::colorModel()const
{
  return qobject_cast<qDMMLColorModel*>(this->sortFilterProxyModel()->sourceModel());
}

//------------------------------------------------------------------------------
QSortFilterProxyModel* qDMMLColorListView::sortFilterProxyModel()const
{
  return qobject_cast<QSortFilterProxyModel*>(this->model());
}

//------------------------------------------------------------------------------
void qDMMLColorListView::setDMMLColorNode(vtkDMMLNode* node)
{
  this->setDMMLColorNode(vtkDMMLColorNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qDMMLColorListView::setDMMLColorNode(vtkDMMLColorNode* node)
{
  qDMMLColorModel* dmmlModel = this->colorModel();
  Q_ASSERT(dmmlModel);
  dmmlModel->setDMMLColorNode(node);
  this->sortFilterProxyModel()->invalidate();
  this->setCurrentIndex(this->model()->index(-1,-1));
}

//------------------------------------------------------------------------------
vtkDMMLColorNode* qDMMLColorListView::dmmlColorNode()const
{
  qDMMLColorModel* dmmlModel = this->colorModel();
  Q_ASSERT(dmmlModel);
  return dmmlModel->dmmlColorNode();
}

//------------------------------------------------------------------------------
void qDMMLColorListView::setShowOnlyNamedColors(bool enable)
{
  if (enable)
    {
    this->sortFilterProxyModel()->setFilterRegExp(QRegExp("^(?!\\(none\\))"));
    }
  else
    {
    this->sortFilterProxyModel()->setFilterRegExp(QRegExp());
    }
}

//------------------------------------------------------------------------------
bool qDMMLColorListView::showOnlyNamedColors()const
{
  return this->sortFilterProxyModel()->filterRegExp().isEmpty();
}

//------------------------------------------------------------------------------
void qDMMLColorListView::currentChanged(const QModelIndex& current, const QModelIndex &previous)
{
  if (current.isValid())
    {
    QModelIndex colorIndex = this->sortFilterProxyModel()->mapToSource(current);
    int colorEntry = this->colorModel()->colorFromIndex(colorIndex);
    emit this->colorSelected(colorEntry);
    QColor color = this->colorModel()->qcolorFromColor(colorEntry);
    emit this->colorSelected(color);
    QString name = this->colorModel()->nameFromColor(colorEntry);
    emit this->colorSelected(name);
    }
  this->QListView::currentChanged(current, previous);
}
