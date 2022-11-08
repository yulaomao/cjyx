/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QPalette>

// qDMML includes
#include "qDMMLUtils.h"
#include "qDMMLTableModel.h"

// DMML includes
#include <vtkDMMLTableNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkSmartPointer.h>
#include <vtkTable.h>
#include <vtkBitArray.h>

static int UserRoleValueType = Qt::UserRole + 1;

//------------------------------------------------------------------------------
// qDMMLTableModelPrivate
//------------------------------------------------------------------------------
class qDMMLTableModelPrivate
{
  Q_DECLARE_PUBLIC(qDMMLTableModel);
protected:
  qDMMLTableModel* const q_ptr;
public:
  qDMMLTableModelPrivate(qDMMLTableModel& object);
  virtual ~qDMMLTableModelPrivate();
  void init();

  // Returns Excel-style column names from index (A, B, C, ..., Z, AA, AB, AC, ..., AZ, AAA, AAB, ...)
  static QString columnNameFromIndex(int index);

  // Generate tooltip text
  QString columnTooltipText(int tableCol);

  vtkSmartPointer<vtkCallbackCommand> CallBack;
  vtkSmartPointer<vtkDMMLTableNode>   DMMLTableNode;
  bool Transposed;
};

//------------------------------------------------------------------------------
qDMMLTableModelPrivate::qDMMLTableModelPrivate(qDMMLTableModel& object)
  : q_ptr(&object)
{
  this->CallBack = vtkSmartPointer<vtkCallbackCommand>::New();
  this->Transposed = false;
}

//------------------------------------------------------------------------------
qDMMLTableModelPrivate::~qDMMLTableModelPrivate()
{
  if (this->DMMLTableNode)
    {
    this->DMMLTableNode->RemoveObserver(this->CallBack);
    }
}

//------------------------------------------------------------------------------
void qDMMLTableModelPrivate::init()
{
  Q_Q(qDMMLTableModel);
  this->CallBack->SetClientData(q);
  this->CallBack->SetCallback(qDMMLTableModel::onDMMLNodeEvent);
  q->setColumnCount(0);
  QObject::connect(q, SIGNAL(itemChanged(QStandardItem*)), q, SLOT(onItemChanged(QStandardItem*)), Qt::UniqueConnection);
}

//------------------------------------------------------------------------------
QString qDMMLTableModelPrivate::columnNameFromIndex(int index)
{
  static const char base26Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  QString returnValue;
  while (index>=0)
  {
    returnValue.prepend(base26Chars[index % 26]);
    index = index/26 - 1;
  };
  return returnValue;
}

//------------------------------------------------------------------------------
QString qDMMLTableModelPrivate::columnTooltipText(int tableCol)
{
  Q_Q(qDMMLTableModel);
  vtkDMMLTableNode* tableNode = q->dmmlTableNode();
  if (tableNode == nullptr)
    {
    return QString();
    }

  std::string columnName = tableNode->GetColumnName(tableCol);
  QString longName = QString(tableNode->GetColumnLongName(columnName).c_str());
  QString description = QString(tableNode->GetColumnDescription(columnName).c_str());
  QString unitLabel = QString(tableNode->GetColumnUnitLabel(columnName).c_str());

  QStringList textLines;

  // Long name
  if (!longName.isEmpty())
    {
    textLines << QString("<b>") + longName + QString("</b>");
    }

  // Unit
  if (!unitLabel.isEmpty())
    {
    textLines << QString("Unit: ") + unitLabel;
    }

  // Description
  if (!description.isEmpty())
    {
    textLines << description;
    }

  return textLines.join("<p>");
}

//------------------------------------------------------------------------------
// qDMMLTableModel
//------------------------------------------------------------------------------
qDMMLTableModel::qDMMLTableModel(QObject *_parent)
  : QStandardItemModel(_parent)
  , d_ptr(new qDMMLTableModelPrivate(*this))
{
  Q_D(qDMMLTableModel);
  d->init();
}

//------------------------------------------------------------------------------
qDMMLTableModel::qDMMLTableModel(qDMMLTableModelPrivate* pimpl, QObject *parentObject)
  : QStandardItemModel(parentObject)
  , d_ptr(pimpl)
{
  Q_D(qDMMLTableModel);
  d->init();
}

//------------------------------------------------------------------------------
qDMMLTableModel::~qDMMLTableModel() = default;

//------------------------------------------------------------------------------
void qDMMLTableModel::setDMMLTableNode(vtkDMMLTableNode* tableNode)
{
  Q_D(qDMMLTableModel);
  if (d->DMMLTableNode)
    {
    d->DMMLTableNode->RemoveObserver(d->CallBack);
    }
  if (tableNode)
    {
    tableNode->AddObserver(vtkCommand::ModifiedEvent, d->CallBack);
    }
  d->DMMLTableNode = tableNode;
  this->updateModelFromDMML();
}

//------------------------------------------------------------------------------
vtkDMMLTableNode* qDMMLTableModel::dmmlTableNode()const
{
  Q_D(const qDMMLTableModel);
  return d->DMMLTableNode;
}

// --------------------------------------------------------------------------
void qDMMLTableModel::updateModelFromDMML()
{
  Q_D(qDMMLTableModel);

  QObject::disconnect(this, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(onItemChanged(QStandardItem*)));

  vtkDMMLTableNode* tableNode = vtkDMMLTableNode::SafeDownCast(d->DMMLTableNode);
  vtkTable* table = (tableNode ? tableNode->GetTable() : nullptr);
  if (table==nullptr || table->GetNumberOfColumns()==0)
    {
    beginResetModel();
    // setRowCount and setColumnCount to 0 would not be enough, it's necessary to remove the header as well
    setRowCount(0);
    setColumnCount(0);
    endResetModel();
    QObject::connect(this, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(onItemChanged(QStandardItem*)), Qt::UniqueConnection);
    return;
    }

  bool tableLocked = tableNode->GetLocked();
  bool labelInFirstTableColumn = tableNode->GetUseFirstColumnAsRowHeader();
  bool useColumnNameAsColumnHeader = tableNode->GetUseColumnNameAsColumnHeader();

  vtkIdType numberOfTableColumns = table->GetNumberOfColumns();
  vtkIdType numberOfTableRows = table->GetNumberOfRows();
  // offset: modelIndex = dmmlIndex - offset
  vtkIdType tableColOffset = labelInFirstTableColumn ? 1 : 0;
  vtkIdType tableRowOffset = useColumnNameAsColumnHeader ? 0 : -1;
  if (d->Transposed)
    {
    setRowCount(static_cast<int>(numberOfTableColumns-tableColOffset));
    setColumnCount(static_cast<int>(numberOfTableRows-tableRowOffset));
    }
  else
    {
    setRowCount(static_cast<int>(numberOfTableRows-tableRowOffset));
    setColumnCount(static_cast<int>(numberOfTableColumns-tableColOffset));
    }

  // Setup items for each table column
  for (vtkIdType tableCol = tableColOffset; tableCol < numberOfTableColumns; ++tableCol)
    {
    int modelCol = static_cast<int>(tableCol - tableColOffset);

    QString columnName(table->GetColumnName(tableCol));
    vtkAbstractArray* columnArray = table->GetColumn(tableCol);

    if (useColumnNameAsColumnHeader)
      {
      setHeaderData(modelCol, d->Transposed ? Qt::Vertical : Qt::Horizontal, columnName);
      }
    else
      {
      QString autoColumnHeader = d->columnNameFromIndex(modelCol);
      setHeaderData(modelCol, d->Transposed ? Qt::Vertical : Qt::Horizontal, autoColumnHeader);
      }

    // Go through the rows of the current column
    for (vtkIdType tableRow = tableRowOffset; tableRow < numberOfTableRows; ++tableRow)
      {
      int modelRow = static_cast<int>(tableRow - tableRowOffset);

      // Use existing item if already created
      QStandardItem* existingItem = nullptr;
      if (d->Transposed)
        {
        existingItem = this->item(modelCol, modelRow);
        }
      else
        {
        existingItem = this->item(modelRow, modelCol);
        }
      QStandardItem* item = existingItem;
      // Create item if did not exist
      if (item==nullptr)
        {
        item = new QStandardItem();
        }

      // Items in first row use bold font, the others regular
      if (tableRow>=0)
        {
        vtkVariant variant = table->GetValue(tableRow, tableCol);
        if (tableRow==0)
          {
          // the first row might have been bold earlier, make sure
          // it is reset to non-bold
          QFont font;
          item->setData(font, Qt::FontRole);
          }

        int dataType = columnArray->GetDataType();

        // Set item property for known types.
        // Special types are defined to be displayed differently, handled by qDMMLTableItemDelegate.
        // NOTE: The data type itself can be enough, but in future types it will be necessary to define display role
        //       as well, e.g. double array can be both color and position.
        if (vtkBitArray::SafeDownCast(columnArray))
          {
          // Boolean values indicated by a column of vtkBitArray type are displayed as checkboxes
          item->setData(VTK_BIT, UserRoleValueType);
          item->setCheckable(true);
          item->setCheckState(variant.ToInt() ? Qt::Checked : Qt::Unchecked);
          item->setText(QString()); // No text is supposed to be in the cell
          }
        // Default display as text
        else
          {
          if (dataType == VTK_CHAR || dataType == VTK_UNSIGNED_CHAR || dataType == VTK_SIGNED_CHAR)
            {
            // vtkVariant converts char type to string as a single letter, therefore we need to use
            // custom converter
            item->setText(QString::number(variant.ToInt()));
            }
          else
            {
            item->setText(QString(variant.ToString()));
            }
          item->setData(QVariant(), UserRoleValueType);
          item->setCheckable(false);
          item->setData(QVariant(), Qt::CheckStateRole);
          }
        }
      else
        {
        item->setText(columnName);
        QFont font;
        font.setBold(true);
        item->setData(font, Qt::FontRole);
        }

      // Handle locked flag
      if (item->isCheckable())
        {
        if (tableLocked)
          {
          // Item is view-only, clear the ItemIsUserCheckable flag
          item->setFlags(item->flags() & (~Qt::ItemIsUserCheckable));
          }
        else
          {
          // Item is editable, set the ItemIsUserCheckable flag
          item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
          }
        // Item text is empty and should not be editable
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        }
      else
        {
        if (tableLocked)
          {
          // Item is view-only, clear the ItemIsEditable flag
          item->setFlags(item->flags() & (~Qt::ItemIsEditable));
          }
        else
          {
          // Item is editable, set the ItemIsEditable flag
          item->setFlags(item->flags() | Qt::ItemIsEditable);
          }
        }

      // Add item if just created
      if (item!=existingItem)
        {
        if (d->Transposed)
          {
          setItem(modelCol, modelRow, item);
          }
        else
          {
          setItem(modelRow, modelCol, item);
          }
        }
      }
    }
  // Set row label: either simply 1, 2, ... or values of the first column
  for (vtkIdType tableRow = tableRowOffset; tableRow < numberOfTableRows; ++tableRow)
    {
    int modelRow = static_cast<int>(tableRow - tableRowOffset);
    QString rowLabel;
    if(labelInFirstTableColumn)
      {
      if (tableRow>=0)
        {
        rowLabel = QString(table->GetValue(tableRow, 0).ToString());
        }
      else
        {
        rowLabel = QString(table->GetColumnName(0));;
        }
      }
    else
      {
      rowLabel = QString::number(modelRow+1);
      }
    setHeaderData(modelRow, d->Transposed ? Qt::Horizontal : Qt::Vertical, rowLabel);
    }

  // Add tooltip text
  for (vtkIdType tableCol = tableColOffset; tableCol < numberOfTableColumns; ++tableCol)
    {
    int modelCol = static_cast<int>(tableCol - tableColOffset);
    QString tooltipText = d->columnTooltipText(tableCol);
    for (vtkIdType tableRow = tableRowOffset; tableRow < numberOfTableRows; ++tableRow)
      {
      int modelRow = static_cast<int>(tableRow - tableRowOffset);
      QStandardItem* existingItem = nullptr;
      if (d->Transposed)
        {
        existingItem = this->item(modelCol, modelRow);
        }
      else
        {
        existingItem = this->item(modelRow, modelCol);
        }
      existingItem->setToolTip(tooltipText);
      }
    }

  QObject::connect(this, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(onItemChanged(QStandardItem*)), Qt::UniqueConnection);
}

//------------------------------------------------------------------------------
void qDMMLTableModel::updateDMMLFromModel(QStandardItem* item)
{
  Q_D(qDMMLTableModel);
  if (item == nullptr)
    {
    qCritical("qDMMLTableModel::updateDMMLFromModel failed: item is invalid");
    return;
    }
  vtkDMMLTableNode* tableNode = vtkDMMLTableNode::SafeDownCast(d->DMMLTableNode);
  if (tableNode==nullptr)
    {
    qCritical("qDMMLTableModel::updateDMMLFromModel failed: tableNode is invalid");
    return;
    }
  vtkTable* table = tableNode->GetTable();
  if (table==nullptr)
    {
    qCritical("qDMMLTableModel::updateDMMLFromModel failed: table is invalid");
    return;
    }

  int tableRow = dmmlTableRowIndex(item->index());
  int tableCol = dmmlTableColumnIndex(item->index());

  if (tableRow>=0)
    {
    // Get item value according to type
    int widgetType = item->data(UserRoleValueType).toInt();
    if (widgetType == VTK_BIT)
      {
      // Cell bool value changed
      int checked = item->checkState();
      int valueBefore = table->GetValue(tableRow, tableCol).ToInt();
      if (checked == valueBefore)
        {
        // The value is not changed, this means that the table cannot store this value - revert the value in the table
        this->blockSignals(true);
        item->setCheckState(valueBefore ? Qt::Checked : Qt::Unchecked);
        item->setText(QString()); // No text is supposed to be in the cell
        this->blockSignals(false);
        }
      else
        {
        table->SetValue(tableRow, tableCol, vtkVariant(checked));
        table->GetColumn(tableCol)->Modified(); // Enable observation of checked state changed separately
        table->Modified();
        }
      }
    else
      {
      // Cell text value changed
      int dataType = table->GetColumn(tableCol)->GetDataType();
      if (dataType == VTK_CHAR || dataType == VTK_UNSIGNED_CHAR || dataType == VTK_SIGNED_CHAR)
        {
        // vtkVariant would convert char to a letter, so we need custom conversion here
        bool valid = false;
        int newValue = item->text().toInt(&valid);
        if (dataType == VTK_UNSIGNED_CHAR)
          {
          if (newValue < VTK_UNSIGNED_CHAR_MIN || newValue > VTK_UNSIGNED_CHAR_MAX)
            {
            valid = false;
            }
          }
        else
          {
          if (newValue < VTK_SIGNED_CHAR_MIN || newValue > VTK_SIGNED_CHAR_MAX)
            {
            valid = false;
            }
          }
        if (valid)
          {
          table->SetValue(tableRow, tableCol, newValue);
          table->Modified();
          }
        else
          {
          vtkVariant valueInTableBefore = table->GetValue(tableRow, tableCol); // restore this value if new value is invalid
          this->blockSignals(true);
          item->setText(QString::number(valueInTableBefore.ToInt()));
          this->blockSignals(false);
          }
        }
      else
        {
        vtkVariant valueInTableBefore = table->GetValue(tableRow, tableCol); // restore this value if new value is invalid
        vtkVariant itemText(item->text().toUtf8().constData()); // the vtkVariant constructor makes a copy of the input buffer, so using constData is safe
        table->SetValue(tableRow, tableCol, itemText);
        vtkVariant valueInTableAfter = table->GetValue(tableRow, tableCol);
        if (valueInTableBefore == valueInTableAfter)
          {
          // The value is not changed then it means it is invalid,
          // restore previous value
          this->blockSignals(true);
          item->setText(QString(valueInTableBefore.ToString()));
          this->blockSignals(false);
          }
        else
          {
          table->Modified();
          }
        }
      }
    }
  else
    {
    // Column header changed
    vtkAbstractArray* column = table->GetColumn(tableCol);
    if (column)
      {
      QString valueBefore = QString::fromStdString(column->GetName()?column->GetName():"");
      if (valueBefore!=item->text())
        {
        tableNode->RenameColumn(tableCol, item->text().toUtf8().constData());
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qDMMLTableModel::onDMMLNodeEvent(vtkObject* vtk_obj, unsigned long event,
                                      void* client_data, void* vtkNotUsed(call_data))
{
  vtkDMMLTableNode* tableNode = reinterpret_cast<vtkDMMLTableNode*>(vtk_obj);
  qDMMLTableModel* tableModel = reinterpret_cast<qDMMLTableModel*>(client_data);
  Q_ASSERT(tableNode);
  Q_ASSERT(tableModel);
  switch(event)
    {
    default:
    case vtkCommand::ModifiedEvent:
      tableModel->onDMMLTableNodeModified(tableNode);
      break;
    }
}

//------------------------------------------------------------------------------
void qDMMLTableModel::onDMMLTableNodeModified(vtkObject* node)
{
  Q_D(qDMMLTableModel);
  vtkDMMLTableNode* tableNode = vtkDMMLTableNode::SafeDownCast(node);
  Q_UNUSED(tableNode);
  Q_UNUSED(d);
  Q_ASSERT(tableNode == d->DMMLTableNode);
  this->updateModelFromDMML();
}

//------------------------------------------------------------------------------
void qDMMLTableModel::onItemChanged(QStandardItem * item)
{
  if (item == this->invisibleRootItem())
    {
    return;
    }
  this->updateDMMLFromModel(item);
}

//------------------------------------------------------------------------------
void qDMMLTableModel::setTransposed(bool transposed)
{
  Q_D(qDMMLTableModel);
  if (d->Transposed == transposed)
    {
    return;
    }
  d->Transposed = transposed;
  this->updateModelFromDMML();
}

//------------------------------------------------------------------------------
bool qDMMLTableModel::transposed()const
{
  Q_D(const qDMMLTableModel);
  return d->Transposed;
}

//------------------------------------------------------------------------------
int qDMMLTableModel::dmmlTableRowIndex(QModelIndex modelIndex)const
{
  Q_D(const qDMMLTableModel);
  if (!d->DMMLTableNode)
    {
    qWarning("qDMMLTableModel::dmmlTableRowIndex failed: invalid table node");
    return -1;
    }
  if (d->Transposed)
    {
    return d->DMMLTableNode->GetUseColumnNameAsColumnHeader() ? modelIndex.column() : modelIndex.column()-1;
    }
  else
    {
    return d->DMMLTableNode->GetUseColumnNameAsColumnHeader() ? modelIndex.row() : modelIndex.row()-1;
    }
}

//------------------------------------------------------------------------------
int qDMMLTableModel::dmmlTableColumnIndex(QModelIndex modelIndex)const
{
  Q_D(const qDMMLTableModel);
  if (!d->DMMLTableNode)
    {
    qWarning("qDMMLTableModel::dmmlTableColumnIndex failed: invalid table node");
    return -1;
    }
  if (d->Transposed)
    {
    return d->DMMLTableNode->GetUseFirstColumnAsRowHeader() ? modelIndex.row()+1 : modelIndex.row();
    }
  else
    {
    return d->DMMLTableNode->GetUseFirstColumnAsRowHeader() ? modelIndex.column()+1 : modelIndex.column();
    }
}

//------------------------------------------------------------------------------
int qDMMLTableModel::removeSelectionFromDMML(QModelIndexList selection, bool removeModelRow)
{
  Q_D(const qDMMLTableModel);
  if (!d->DMMLTableNode)
    {
    return 0;
    }
  bool removeDMMLRows = d->Transposed ? !removeModelRow : removeModelRow;

  QModelIndex index;
  QList<int> dmmlIndexList; // list of DMML table columns or rows that will be removed
  foreach(index, selection)
    {
    int dmmlIndex = removeDMMLRows ? dmmlTableRowIndex(index) : dmmlTableColumnIndex(index);
    if (!dmmlIndexList.contains(dmmlIndex))
      {
      // insert unique row/column index only
      dmmlIndexList.push_back(dmmlIndex);
      }
    }
  // reverse sort to start removing last index first to keep remaining indices valid
  std::sort(dmmlIndexList.begin(), dmmlIndexList.end(), std::greater<int>());

  // block modified events to prevent updating of the table during processing
  int wasModified = d->DMMLTableNode->StartModify();
  foreach(int dmmlIndex, dmmlIndexList)
    {
    if (removeDMMLRows)
      {
      if (dmmlIndex==-1)
        {
        // the header row is deleted, move up the first line to header
        vtkTable* table = d->DMMLTableNode->GetTable();
        if (table)
          {
          for (int columnIndex=0; columnIndex<table->GetNumberOfColumns(); columnIndex++)
            {
              vtkAbstractArray* column = table->GetColumn(columnIndex);
              if (!column)
                {
                qCritical("qDMMLTableModel::updateDMMLFromModel failed: column %d is invalid", columnIndex);
                continue;
                }
              d->DMMLTableNode->RenameColumn(columnIndex, table->GetValue(0,columnIndex).ToString());
            }
          d->DMMLTableNode->RemoveRow(0);
          }
        else
          {
          qCritical("qDMMLTableModel::updateDMMLFromModel failed: table is invalid");
          }
        }
      else
        {
        d->DMMLTableNode->RemoveRow(dmmlIndex);
        }
      }
    else
      {
      d->DMMLTableNode->RemoveColumn(dmmlIndex);
      }
    }
  d->DMMLTableNode->EndModify(wasModified);

  return dmmlIndexList.size();
}
