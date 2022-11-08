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
#include <QApplication>

// qDMML includes
#include "qDMMLColorModel_p.h"
#include "qDMMLUtils.h"

// DMML includes
#include <vtkDMMLColorTableNode.h>

// VTK includes

//------------------------------------------------------------------------------
qDMMLColorModelPrivate::qDMMLColorModelPrivate(qDMMLColorModel& object)
  : q_ptr(&object)
{
  this->CallBack = vtkSmartPointer<vtkCallbackCommand>::New();
  this->NoneEnabled = false;
  this->ColorColumn = 0;
  this->LabelColumn = 1;
  this->OpacityColumn = 2;
  this->CheckableColumn = -1;
  this->IsUpdatingWidgetFromDMML = false;
}

//------------------------------------------------------------------------------
qDMMLColorModelPrivate::~qDMMLColorModelPrivate()
{
  if (this->DMMLColorNode)
    {
    this->DMMLColorNode->RemoveObserver(this->CallBack);
    }
}

//------------------------------------------------------------------------------
void qDMMLColorModelPrivate::init()
{
  Q_Q(qDMMLColorModel);
  this->CallBack->SetClientData(q);
  this->CallBack->SetCallback(qDMMLColorModel::onDMMLNodeEvent);
  this->updateColumnCount();
  QStringList headerLabels;
  for (int i = 0; i <= this->maxColumnId(); ++i)
    {
    headerLabels << "";
    }
  if (q->colorColumn() != -1)
    {
    headerLabels[q->colorColumn()] = "Color";
    }
  if (q->labelColumn() != -1)
    {
    headerLabels[q->labelColumn()] = "Label";
    }
  if (q->opacityColumn() != -1)
    {
    headerLabels[q->opacityColumn()] = "Opacity";
    }
  q->setHorizontalHeaderLabels(headerLabels);
  QObject::connect(q, SIGNAL(itemChanged(QStandardItem*)),
                   q, SLOT(onItemChanged(QStandardItem*)),
                   Qt::UniqueConnection);
}

//------------------------------------------------------------------------------
void qDMMLColorModelPrivate::updateColumnCount()
{
  Q_Q(qDMMLColorModel);
  const int max = this->maxColumnId();
  q->setColumnCount(max + 1);
}

//------------------------------------------------------------------------------
int qDMMLColorModelPrivate::maxColumnId()const
{
  int maxId = 0; // information (scene, node uid... ) are stored in the 1st column
  maxId = qMax(maxId, this->ColorColumn);
  maxId = qMax(maxId, this->LabelColumn);
  maxId = qMax(maxId, this->OpacityColumn);
  maxId = qMax(maxId, this->CheckableColumn);
  return maxId;
}

//------------------------------------------------------------------------------
// qDMMLColorModel
//------------------------------------------------------------------------------
qDMMLColorModel::qDMMLColorModel(QObject *_parent)
  : QStandardItemModel(_parent)
  , d_ptr(new qDMMLColorModelPrivate(*this))
{
  Q_D(qDMMLColorModel);
  d->init();
}

//------------------------------------------------------------------------------
qDMMLColorModel::qDMMLColorModel(qDMMLColorModelPrivate* pimpl, QObject *parentObject)
  : QStandardItemModel(parentObject)
  , d_ptr(pimpl)
{
  Q_D(qDMMLColorModel);
  d->init();
}

//------------------------------------------------------------------------------
qDMMLColorModel::~qDMMLColorModel() = default;

//------------------------------------------------------------------------------
void qDMMLColorModel::setDMMLColorNode(vtkDMMLColorNode* colorNode)
{
  Q_D(qDMMLColorModel);
  if (d->DMMLColorNode)
    {
    d->DMMLColorNode->RemoveObserver(d->CallBack);
    }
  if (colorNode)
    {
    colorNode->AddObserver(vtkCommand::ModifiedEvent, d->CallBack);
    }
  d->DMMLColorNode = colorNode;
  this->updateNode();
}

//------------------------------------------------------------------------------
vtkDMMLColorNode* qDMMLColorModel::dmmlColorNode()const
{
  Q_D(const qDMMLColorModel);
  return d->DMMLColorNode;
}

//------------------------------------------------------------------------------
void qDMMLColorModel::setNoneEnabled(bool enable)
{
  Q_D(qDMMLColorModel);
  if (this->noneEnabled() == enable)
    {
    return;
    }
  d->NoneEnabled = enable;
  if (enable)
    {
    this->insertRow(0, new QStandardItem(tr("None")));
    }
  else
    {
    this->removeRow(0);
    }
  this->updateNode();
}

//------------------------------------------------------------------------------
bool qDMMLColorModel::noneEnabled()const
{
  Q_D(const qDMMLColorModel);
  return d->NoneEnabled;
}

//------------------------------------------------------------------------------
int qDMMLColorModel::colorColumn()const
{
  Q_D(const qDMMLColorModel);
  return d->ColorColumn;
}

//------------------------------------------------------------------------------
void qDMMLColorModel::setColorColumn(int column)
{
  Q_D(qDMMLColorModel);
  d->ColorColumn = column;
  d->updateColumnCount();
}

//------------------------------------------------------------------------------
int qDMMLColorModel::labelColumn()const
{
  Q_D(const qDMMLColorModel);
  return d->LabelColumn;
}

//------------------------------------------------------------------------------
void qDMMLColorModel::setLabelColumn(int column)
{
  Q_D(qDMMLColorModel);
  d->LabelColumn = column;
  d->updateColumnCount();
}

//------------------------------------------------------------------------------
int qDMMLColorModel::opacityColumn()const
{
  Q_D(const qDMMLColorModel);
  return d->OpacityColumn;
}

//------------------------------------------------------------------------------
void qDMMLColorModel::setOpacityColumn(int column)
{
  Q_D(qDMMLColorModel);
  d->OpacityColumn = column;
  d->updateColumnCount();
}

//------------------------------------------------------------------------------
int qDMMLColorModel::checkableColumn()const
{
  Q_D(const qDMMLColorModel);
  return d->CheckableColumn;
}

//------------------------------------------------------------------------------
void qDMMLColorModel::setCheckableColumn(int column)
{
  Q_D(qDMMLColorModel);
  d->CheckableColumn = column;
  d->updateColumnCount();
}


//------------------------------------------------------------------------------
int qDMMLColorModel::colorFromItem(QStandardItem* colorItem)const
{
  Q_D(const qDMMLColorModel);
  // TODO: fasten by saving the pointer into the data
  if (d->DMMLColorNode == nullptr || colorItem == nullptr)
    {
    return -1;
    }
  QVariant colorIndex = colorItem->data(qDMMLColorModel::ColorEntryRole);
  if (!colorIndex.isValid())
    {
    return -1;
    }
  return colorIndex.toInt();
}

//------------------------------------------------------------------------------
QStandardItem* qDMMLColorModel::itemFromColor(int color, int column)const
{
  //Q_D(const qDMMLColorModel);
  if (color == -1)
    {
    return nullptr;
    }
  QModelIndexList indexes = this->match(this->index(0,0), qDMMLColorModel::ColorEntryRole,
                                      color, 1,
                                      Qt::MatchExactly | Qt::MatchRecursive);
  while (indexes.size())
    {
    if (indexes[0].column() == column)
      {
      return this->itemFromIndex(indexes[0]);
      }
    indexes = this->match(indexes[0], qDMMLColorModel::ColorEntryRole, color, 1,
                          Qt::MatchExactly | Qt::MatchRecursive);
    }
  return nullptr;
}

//------------------------------------------------------------------------------
QModelIndexList qDMMLColorModel::indexes(int color)const
{
  return this->match(this->index(0,0), qDMMLColorModel::ColorEntryRole, color, -1,
                     Qt::MatchExactly | Qt::MatchRecursive);
}

//------------------------------------------------------------------------------
QColor qDMMLColorModel::qcolorFromColor(int entry)const
{
  Q_D(const qDMMLColorModel);
  if (d->DMMLColorNode == nullptr || entry < 0)
    {
    return QColor();
    }
  double rgba[4];
  d->DMMLColorNode->GetColor(entry, rgba);
  return QColor::fromRgbF(rgba[0], rgba[1], rgba[2], rgba[3]);
}

//------------------------------------------------------------------------------
QString qDMMLColorModel::nameFromColor(int entry)const
{
  Q_D(const qDMMLColorModel);
  if (d->DMMLColorNode == nullptr || entry < 0)
    {
    return QString();
    }
  return QString(d->DMMLColorNode->GetColorName(entry));
}

//------------------------------------------------------------------------------
int qDMMLColorModel::colorFromName(const QString& name)const
{
  Q_D(const qDMMLColorModel);
  if (d->DMMLColorNode == nullptr)
    {
    return -1;
    }
  return d->DMMLColorNode->GetColorIndexByName(name.toUtf8());
}

//------------------------------------------------------------------------------
void qDMMLColorModel::updateNode()
{
  Q_D(qDMMLColorModel);

  if (d->IsUpdatingWidgetFromDMML)
    {
    // Updating widget from DMML is already in progress
    return;
    }
  d->IsUpdatingWidgetFromDMML = true;

  if (d->DMMLColorNode == nullptr)
    {
    this->setRowCount(this->noneEnabled() ? 1 : 0);
    d->IsUpdatingWidgetFromDMML = false;
    return;
    }

  this->setRowCount(
    d->DMMLColorNode->GetNumberOfColors() + (this->noneEnabled() ? 1 : 0));

  bool wasBlocked = this->blockSignals(true);
  int startIndex = (this->noneEnabled() ? 1 : 0);
  for (int color = 0; color < d->DMMLColorNode->GetNumberOfColors(); ++color)
    {
    for (int j= 0; j < this->columnCount(); ++j)
      {
      QStandardItem* colorItem = this->invisibleRootItem()->child(
        color + startIndex, j);
      if (!colorItem)
        {
        colorItem = new QStandardItem();
        this->invisibleRootItem()->setChild(color + startIndex,j,colorItem);
        }
      this->updateItemFromColor(colorItem, color, j);
      }
    }
  this->blockSignals(wasBlocked);

  d->IsUpdatingWidgetFromDMML = false;
}

//------------------------------------------------------------------------------
void qDMMLColorModel::updateItemFromColor(QStandardItem* item, int color, int column)
{
  Q_D(qDMMLColorModel);
  if (color < 0)
    {
    return;
    }
  item->setData(color, qDMMLColorModel::ColorEntryRole);

  QString colorName = d->DMMLColorNode->GetNamesInitialised() ?
    d->DMMLColorNode->GetColorName(color) : "";
  if (column == d->ColorColumn)
    {
    QPixmap pixmap;
    double rgba[4] = { 0., 0., 0., 1. };
    const bool validColor = d->DMMLColorNode->GetColor(color, rgba);
    if (validColor)
      {
      // It works to set just a QColor but if the model gets into a QComboBox,
      // the currently selected item doesn't get a decoration.
      // TODO: Cache the pixmap as it is expensive to compute and it is done
      // for ALL the colors of the node anytime a color is changed.
      pixmap = qDMMLUtils::createColorPixmap(
        qApp->style(), QColor::fromRgbF(rgba[0], rgba[1], rgba[2]));
      item->setData(pixmap, Qt::DecorationRole);
      item->setData(QColor::fromRgbF(rgba[0], rgba[1], rgba[2]), qDMMLColorModel::ColorRole);
      }
    else
      {
      item->setData(QVariant(), Qt::DecorationRole);
      item->setData(QColor(), qDMMLColorModel::ColorRole);
      }
    item->setData(validColor && column != d->LabelColumn ?
      pixmap.size() : QVariant(), Qt::SizeHintRole);
    item->setToolTip(colorName);
    }
  if (column == d->LabelColumn)
    {
    item->setText(colorName);
    item->setToolTip("");
    }
  if (column == d->OpacityColumn)
    {
    double rgba[4] = { 0., 0., 0., 1. };
    d->DMMLColorNode->GetColor(color, rgba);
    item->setData(QString::number(rgba[3],'f',2), Qt::DisplayRole);
    }
  if (column == d->CheckableColumn)
    {
    item->setCheckable(true);
    }
}

//------------------------------------------------------------------------------
void qDMMLColorModel::updateColorFromItem(int color, QStandardItem* item)
{
  Q_D(qDMMLColorModel);
  vtkDMMLColorTableNode* colorTableNode = vtkDMMLColorTableNode::SafeDownCast(d->DMMLColorNode);
  if (color < 0 || !colorTableNode)
    {
    return;
    }
  if (item->column() == d->ColorColumn)
    {
    QColor rgba(item->data(qDMMLColorModel::ColorRole).value<QColor>());
    colorTableNode->SetColor(color, rgba.redF(), rgba.greenF(), rgba.blueF());
    }
  else if (item->column() == d->LabelColumn)
    {
    colorTableNode->SetColorName(color, item->text().toUtf8());
    }
  else if (item->column() == d->OpacityColumn)
    {
    colorTableNode->SetOpacity(color, item->data(Qt::DisplayRole).toDouble());
    }
}

//-----------------------------------------------------------------------------
void qDMMLColorModel::onDMMLNodeEvent(vtkObject* vtk_obj, unsigned long event,
                                      void* client_data, void* vtkNotUsed(call_data))
{
  vtkDMMLColorNode* colorNode = reinterpret_cast<vtkDMMLColorNode*>(vtk_obj);
  qDMMLColorModel* colorModel = reinterpret_cast<qDMMLColorModel*>(client_data);
  Q_ASSERT(colorNode);
  Q_ASSERT(colorModel);
  switch(event)
    {
    default:
    case vtkCommand::ModifiedEvent:
      colorModel->onDMMLColorNodeModified(colorNode);
      break;
    }
}

//------------------------------------------------------------------------------
void qDMMLColorModel::onDMMLColorNodeModified(vtkObject* node)
{
  Q_D(qDMMLColorModel);
  vtkDMMLColorNode* colorNode = vtkDMMLColorNode::SafeDownCast(node);
  Q_UNUSED(colorNode);
  Q_UNUSED(d);
  Q_ASSERT(colorNode == d->DMMLColorNode);
  this->updateNode();
}

//------------------------------------------------------------------------------
void qDMMLColorModel::onItemChanged(QStandardItem * item)
{
  if (item == this->invisibleRootItem())
    {
    return;
    }
  int color = this->colorFromItem(item);
  this->updateColorFromItem(color, item);
}

//------------------------------------------------------------------------------
QVariant qDMMLColorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  QVariant retval =  QStandardItemModel::headerData(section, orientation, role);

  if (orientation == Qt::Vertical &&
      role == Qt::DisplayRole)
    {
    // for the vertical header, decrement the row number by one, since the
    // rows start from 1 and the indices start from 0 in the color look up
    // table.
    retval = QVariant(retval.toInt() - 1);
    }

  return retval;

}
