/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

// Segmentations includes
#include "qDMMLSegmentsModel.h"
#include "qDMMLSortFilterSegmentsProxyModel.h"

// Segmentations logic includes
#include "vtkCjyxSegmentationsModuleLogic.h"

// DMML include
#include "vtkDMMLSegmentationNode.h"

// Qt includes
#include <QDebug>
#include <QStandardItem>

// -----------------------------------------------------------------------------
// qDMMLSortFilterSegmentsProxyModelPrivate

// -----------------------------------------------------------------------------
/// \ingroup Cjyx_DMMLWidgets
class qDMMLSortFilterSegmentsProxyModelPrivate
{
public:
  qDMMLSortFilterSegmentsProxyModelPrivate();

  bool FilterEnabled{false};
  QString NameFilter;
  QString TextFilter;
  bool ShowStatus[vtkCjyxSegmentationsModuleLogic::LastStatus];
  QStringList HideSegments;
};

// -----------------------------------------------------------------------------
qDMMLSortFilterSegmentsProxyModelPrivate::qDMMLSortFilterSegmentsProxyModelPrivate()
  : NameFilter(QString())
  , TextFilter(QString())
{
  for (int i = 0; i < vtkCjyxSegmentationsModuleLogic::LastStatus; ++i)
    {
    this->ShowStatus[i] = false;
    }
}

// -----------------------------------------------------------------------------
// qDMMLSortFilterSegmentsProxyModel

// -----------------------------------------------------------------------------
CTK_GET_CPP(qDMMLSortFilterSegmentsProxyModel, bool, filterEnabled, FilterEnabled);
CTK_GET_CPP(qDMMLSortFilterSegmentsProxyModel, QString, nameFilter, NameFilter);
CTK_GET_CPP(qDMMLSortFilterSegmentsProxyModel, QString, textFilter, TextFilter);

//------------------------------------------------------------------------------
qDMMLSortFilterSegmentsProxyModel::qDMMLSortFilterSegmentsProxyModel(QObject *vparent)
 : QSortFilterProxyModel(vparent)
 , d_ptr(new qDMMLSortFilterSegmentsProxyModelPrivate)
{
  this->setDynamicSortFilter(true);
}

//------------------------------------------------------------------------------
qDMMLSortFilterSegmentsProxyModel::~qDMMLSortFilterSegmentsProxyModel() = default;

//-----------------------------------------------------------------------------
vtkDMMLSegmentationNode* qDMMLSortFilterSegmentsProxyModel::segmentationNode()const
{
  qDMMLSegmentsModel* model = qobject_cast<qDMMLSegmentsModel*>(this->sourceModel());
  if (!model)
    {
    return nullptr;
    }
  return model->segmentationNode();
}

//-----------------------------------------------------------------------------
void qDMMLSortFilterSegmentsProxyModel::setFilterEnabled(bool filterEnabled)
{
  Q_D(qDMMLSortFilterSegmentsProxyModel);
  if (d->FilterEnabled == filterEnabled)
  {
    return;
  }
  d->FilterEnabled = filterEnabled;
  this->invalidateFilter();
  emit filterModified();
}

//-----------------------------------------------------------------------------
void qDMMLSortFilterSegmentsProxyModel::setNameFilter(QString filter)
{
  Q_D(qDMMLSortFilterSegmentsProxyModel);
  if (d->NameFilter == filter)
    {
    return;
    }
  d->NameFilter = filter;
  this->invalidateFilter();
  emit filterModified();
}

//-----------------------------------------------------------------------------
void qDMMLSortFilterSegmentsProxyModel::setTextFilter(QString filter)
{
  Q_D(qDMMLSortFilterSegmentsProxyModel);
  if (d->TextFilter == filter)
    {
    return;
    }
  d->TextFilter = filter;
  this->invalidateFilter();
  emit filterModified();
}

//-----------------------------------------------------------------------------
bool qDMMLSortFilterSegmentsProxyModel::showStatus(int status) const
{
  Q_D(const qDMMLSortFilterSegmentsProxyModel);
  if (status < 0 || status >= vtkCjyxSegmentationsModuleLogic::LastStatus)
    {
    return false;
    }
  return d->ShowStatus[status];
}

//-----------------------------------------------------------------------------
void qDMMLSortFilterSegmentsProxyModel::setShowStatus(int status, bool shown)
{
  Q_D(qDMMLSortFilterSegmentsProxyModel);
  if (status < 0 || status >= vtkCjyxSegmentationsModuleLogic::LastStatus)
    {
    return;
    }
  if (d->ShowStatus[status] == shown)
    {
    return;
    }

  d->ShowStatus[status] = shown;
  this->invalidateFilter();
  emit filterModified();
}

//-----------------------------------------------------------------------------
QString qDMMLSortFilterSegmentsProxyModel::segmentIDFromIndex(const QModelIndex& index)const
{
  qDMMLSegmentsModel* segmentsModel = qobject_cast<qDMMLSegmentsModel*>(this->sourceModel());
  return segmentsModel->segmentIDFromIndex( this->mapToSource(index) );
}

//-----------------------------------------------------------------------------
QModelIndex qDMMLSortFilterSegmentsProxyModel::indexFromSegmentID(QString segmentID, int column)const
{
  qDMMLSegmentsModel* segmentsModel = qobject_cast<qDMMLSegmentsModel*>(this->sourceModel());
  return this->mapFromSource(segmentsModel->indexFromSegmentID(segmentID, column));
}

//-----------------------------------------------------------------------------
QStandardItem* qDMMLSortFilterSegmentsProxyModel::sourceItem(const QModelIndex& sourceIndex)const
{
  qDMMLSegmentsModel* model = qobject_cast<qDMMLSegmentsModel*>(this->sourceModel());
  if (!model)
    {
    return nullptr;
    }
  return sourceIndex.isValid() ? model->itemFromIndex(sourceIndex) : model->invisibleRootItem();
}

//------------------------------------------------------------------------------
bool qDMMLSortFilterSegmentsProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent)const
{
  QStandardItem* parentItem = this->sourceItem(sourceParent);
  if (!parentItem)
    {
    return false;
    }
  QStandardItem* item = nullptr;

  // Sometimes the row is not complete (DnD), search for a non null item
  for (int childIndex=0; childIndex < parentItem->columnCount(); ++childIndex)
    {
    item = parentItem->child(sourceRow, childIndex);
    if (item)
      {
      break;
      }
    }
  if (item == nullptr)
    {
    return false;
    }

  qDMMLSegmentsModel* model = qobject_cast<qDMMLSegmentsModel*>(this->sourceModel());
  QString segmentID = model->segmentIDFromItem(item);
  return this->filterAcceptsItem(segmentID);
}

//------------------------------------------------------------------------------
bool qDMMLSortFilterSegmentsProxyModel::filterAcceptsItem(QString segmentID)const
{
  Q_D(const qDMMLSortFilterSegmentsProxyModel);

  // Filter if segment is hidden
  if (d->HideSegments.contains(segmentID))
    {
    return false;
    }

  if (!d->FilterEnabled)
    {
    return true;
    }

  qDMMLSegmentsModel* model = qobject_cast<qDMMLSegmentsModel*>(this->sourceModel());
  if (!model)
    {
    return false;
    }
  vtkDMMLSegmentationNode* segmentationNode = model->segmentationNode();
  if (!segmentationNode)
    {
    return false;
    }
  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  if (!segmentation)
    {
    return false;
    }
  vtkSegment* segment = segmentation->GetSegment(segmentID.toStdString());
  if (!segment)
    {
    return false;
    }

  // Filter by segment name
  if (!d->NameFilter.isEmpty())
    {
    QString segmentName(segment->GetName());
    if (!segmentName.contains(d->NameFilter, Qt::CaseInsensitive))
      {
        return false;
      }
    }

  // Filter by segment text (name and tag value)
  if (!d->TextFilter.isEmpty())
    {
    bool matchFound = false;
    QString segmentName = segment->GetName();
    if (segmentName.contains(d->TextFilter, Qt::CaseInsensitive))
      {
      matchFound = true;
      }
    if (!matchFound)
      {
      std::map<std::string, std::string> tags;
      segment->GetTags(tags);
      for (const auto& keyValue : tags)
        {
        QString value = keyValue.second.c_str();
        if (value.contains(d->TextFilter))
          {
          matchFound = true;
          break;
          }
        }
      }
    if (!matchFound)
      {
      return false;
      }
    }

  // Filter if segment state does not match one of the shown states
  bool statusFilterEnabled = false;
  for (int i = 0; i < vtkCjyxSegmentationsModuleLogic::LastStatus; ++i)
    {
    statusFilterEnabled = d->ShowStatus[i];
    if (statusFilterEnabled)
      {
      break;
      }
    }

  if (statusFilterEnabled)
    {
    int status = vtkCjyxSegmentationsModuleLogic::GetSegmentStatus(segment);
    if (status >= 0 && status < vtkCjyxSegmentationsModuleLogic::LastStatus && !d->ShowStatus[status])
      {
      return false;
      }
    }

  // All criteria were met
  return true;
}

//------------------------------------------------------------------------------
Qt::ItemFlags qDMMLSortFilterSegmentsProxyModel::flags(const QModelIndex & index)const
{
  QString segmentID = this->segmentIDFromIndex(index);
  bool isSelectable = this->filterAcceptsItem(segmentID);
  qDMMLSegmentsModel* segmentsModel = qobject_cast<qDMMLSegmentsModel*>(this->sourceModel());
  QStandardItem* item = segmentsModel->itemFromSegmentID(segmentID, index.column());
  if (!item)
    {
    return Qt::ItemFlags();
    }

  QFlags<Qt::ItemFlag> flags = item->flags();
  if (isSelectable)
    {
    return flags | Qt::ItemIsSelectable;
    }
  else
    {
    return flags & ~Qt::ItemIsSelectable;
    }
}

// --------------------------------------------------------------------------
void qDMMLSortFilterSegmentsProxyModel::setHideSegments(const QStringList& segmentIDs)
{
  Q_D(qDMMLSortFilterSegmentsProxyModel);
  d->HideSegments = segmentIDs;
  this->invalidateFilter();
  emit filterModified();
}

// --------------------------------------------------------------------------
QStringList qDMMLSortFilterSegmentsProxyModel::hideSegments()const
{
  Q_D(const qDMMLSortFilterSegmentsProxyModel);
  return d->HideSegments;
}
