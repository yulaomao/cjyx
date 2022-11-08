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
#include <QHeaderView>
#include <QSortFilterProxyModel>

// CTK includes
#include <ctkColorDialog.h>

// qDMML includes
#include "qDMMLColorTableView.h"
#include "qDMMLColorModel.h"
#include "qDMMLItemDelegate.h"

// DMML includes
#include <vtkDMMLColorTableNode.h>

//------------------------------------------------------------------------------
class qDMMLColorTableViewPrivate
{
  Q_DECLARE_PUBLIC(qDMMLColorTableView);
protected:
  qDMMLColorTableView* const q_ptr;
public:
  qDMMLColorTableViewPrivate(qDMMLColorTableView& object);
  void init();
};

//------------------------------------------------------------------------------
qDMMLColorTableViewPrivate::qDMMLColorTableViewPrivate(qDMMLColorTableView& object)
  : q_ptr(&object)
{
}

//------------------------------------------------------------------------------
void qDMMLColorTableViewPrivate::init()
{
  Q_Q(qDMMLColorTableView);

  qDMMLColorModel* colorModel = new qDMMLColorModel(q);
  QSortFilterProxyModel* sortFilterModel = new QSortFilterProxyModel(q);
  sortFilterModel->setFilterKeyColumn(colorModel->labelColumn());
  sortFilterModel->setSourceModel(colorModel);
  q->setModel(sortFilterModel);

  q->setSelectionBehavior(QAbstractItemView::SelectRows);
  q->horizontalHeader()->setStretchLastSection(false);
  q->horizontalHeader()->setSectionResizeMode(colorModel->colorColumn(), QHeaderView::ResizeToContents);
  q->horizontalHeader()->setSectionResizeMode(colorModel->labelColumn(), QHeaderView::Stretch);
  q->horizontalHeader()->setSectionResizeMode(colorModel->opacityColumn(), QHeaderView::ResizeToContents);
  q->setItemDelegate(new qDMMLItemDelegate(q));
}

//------------------------------------------------------------------------------
qDMMLColorTableView::qDMMLColorTableView(QWidget *_parent)
  : QTableView(_parent)
  , d_ptr(new qDMMLColorTableViewPrivate(*this))
{
  Q_D(qDMMLColorTableView);
  d->init();
}

//------------------------------------------------------------------------------
qDMMLColorTableView::~qDMMLColorTableView() = default;

//------------------------------------------------------------------------------
qDMMLColorModel* qDMMLColorTableView::colorModel()const
{
  return qobject_cast<qDMMLColorModel*>(this->sortFilterProxyModel()->sourceModel());
}

//------------------------------------------------------------------------------
QSortFilterProxyModel* qDMMLColorTableView::sortFilterProxyModel()const
{
  return qobject_cast<QSortFilterProxyModel*>(this->model());
}

//------------------------------------------------------------------------------
void qDMMLColorTableView::setDMMLColorNode(vtkDMMLNode* node)
{
  this->setDMMLColorNode(vtkDMMLColorNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qDMMLColorTableView::setDMMLColorNode(vtkDMMLColorNode* node)
{
  qDMMLColorModel* dmmlModel = this->colorModel();
  Q_ASSERT(dmmlModel);

  dmmlModel->setDMMLColorNode(node);
  this->sortFilterProxyModel()->invalidate();

  this->setEditTriggers( (node && node->GetType() == vtkDMMLColorTableNode::User) ?
      QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed :
      QAbstractItemView::NoEditTriggers);
}

//------------------------------------------------------------------------------
vtkDMMLColorNode* qDMMLColorTableView::dmmlColorNode()const
{
  qDMMLColorModel* dmmlModel = this->colorModel();
  Q_ASSERT(dmmlModel);
  return dmmlModel->dmmlColorNode();
}

//------------------------------------------------------------------------------
void qDMMLColorTableView::setShowOnlyNamedColors(bool enable)
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
bool qDMMLColorTableView::showOnlyNamedColors()const
{
  return this->sortFilterProxyModel()->filterRegExp().isEmpty();
}

//------------------------------------------------------------------------------
int qDMMLColorTableView::rowFromColorName(const QString& colorName)const
{
  int index = this->colorModel()->colorFromName(colorName);
  return this->rowFromColorIndex(index);
}

//------------------------------------------------------------------------------
int qDMMLColorTableView::rowFromColorIndex(int colorIndex)const
{
  QModelIndexList indexes = this->colorModel()->indexes(colorIndex);
  if (indexes.isEmpty())
    {
    return -1;
    }
  QModelIndex sortedIndex = this->sortFilterProxyModel()->mapFromSource(indexes[0]);
  return sortedIndex.row();
}
