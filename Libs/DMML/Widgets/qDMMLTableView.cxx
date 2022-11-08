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
#include <QClipboard>
#include <QDebug>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QKeyEvent>
#include <QSortFilterProxyModel>
#include <QString>
#include <QToolButton>

// CTK includes
#include <ctkPopupWidget.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkStringArray.h>
#include <vtkTable.h>

// qDMML includes
#include "qDMMLTableView.h"
#include "qDMMLTableView_p.h"
#include "qDMMLTableModel.h"

// DMML includes
#include <vtkDMMLLayoutNode.h>
#include <vtkDMMLPlotSeriesNode.h>
#include <vtkDMMLPlotChartNode.h>
#include <vtkDMMLPlotViewNode.h>
#include <vtkRenderingCoreEnums.h> // for VTK_MARKER_SQUARE
#include <vtkDMMLScene.h>
#include <vtkDMMLSelectionNode.h>
#include <vtkDMMLTableNode.h>
#include <vtkDMMLTableViewNode.h>

// STL includes
#include <algorithm>
#include <deque>

#define CTK_CHECK_AND_RETURN_IF_FAIL(FUNC) \
  if (!FUNC(Q_FUNC_INFO))       \
    {                              \
    return;                        \
    }

#define CTK_CHECK_AND_RETURN_FALSE_IF_FAIL(FUNC) \
  if (!FUNC(Q_FUNC_INFO))            \
    {                                   \
    return false;                       \
    }

//------------------------------------------------------------------------------
qDMMLTableViewPrivate::qDMMLTableViewPrivate(qDMMLTableView& object)
  : q_ptr(&object)
  , DMMLScene(nullptr)
  , DMMLTableViewNode(nullptr)
  , PinButton(nullptr)
  , PopupWidget(nullptr)
{
}

//---------------------------------------------------------------------------
qDMMLTableViewPrivate::~qDMMLTableViewPrivate() = default;

//------------------------------------------------------------------------------
void qDMMLTableViewPrivate::init()
{
  Q_Q(qDMMLTableView);

  qDMMLTableModel* tableModel = new qDMMLTableModel(q);
  QSortFilterProxyModel* sortFilterModel = new QSortFilterProxyModel(q);
  sortFilterModel->setSourceModel(tableModel);
  q->setModel(sortFilterModel);

  q->horizontalHeader()->setStretchLastSection(false);

  // Let the view expand in both directions
  q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  this->PopupWidget = new ctkPopupWidget;
  QHBoxLayout* popupLayout = new QHBoxLayout;
  popupLayout->addWidget(new QToolButton);
  this->PopupWidget->setLayout(popupLayout);
}

//---------------------------------------------------------------------------
void qDMMLTableViewPrivate::setDMMLScene(vtkDMMLScene* newScene)
{
  //Q_Q(qDMMLTableView);
  if (newScene == this->DMMLScene)
    {
    return;
    }
  this->qvtkReconnect(
    this->dmmlScene(), newScene,
    vtkDMMLScene::StartBatchProcessEvent, this, SLOT(startProcessing()));

  this->qvtkReconnect(
    this->dmmlScene(), newScene,
    vtkDMMLScene::EndBatchProcessEvent, this, SLOT(endProcessing()));
  this->DMMLScene = newScene;
}

// --------------------------------------------------------------------------
void qDMMLTableViewPrivate::startProcessing()
{
}

// --------------------------------------------------------------------------
void qDMMLTableViewPrivate::endProcessing()
{
  this->updateWidgetFromViewNode();
}

// --------------------------------------------------------------------------
vtkDMMLScene* qDMMLTableViewPrivate::dmmlScene()
{
  return this->DMMLScene;
}

// --------------------------------------------------------------------------
bool qDMMLTableViewPrivate::verifyTableModelAndNode(const char* methodName) const
{
  Q_Q(const qDMMLTableView);
  if (!q->tableModel())
    {
    qWarning() << "qDMMLTableView:: " << methodName << " failed: invalid model";
    return false;
    }
  if (!q->dmmlTableNode())
    {
    qWarning() << "qDMMLTableView::" << methodName << " failed: invalid node";
    return false;
    }
  return true;
}

// --------------------------------------------------------------------------
void qDMMLTableViewPrivate::updateWidgetFromViewNode()
{
  Q_Q(qDMMLTableView);
  if (!this->DMMLTableViewNode)
    {
    // this view is used without view node (table node is set directly)
    return;
    }
  if (!this->DMMLScene
    || this->DMMLScene != this->DMMLTableViewNode->GetScene())
    {
    // the view node is not in the scene anymore, do not show the table
    q->setDMMLTableNode((vtkDMMLNode*)nullptr);
    }
  // Update the TableNode
  q->setDMMLTableNode(this->DMMLTableViewNode->GetTableNode());
}

//------------------------------------------------------------------------------
qDMMLTableView::qDMMLTableView(QWidget *_parent)
  : QTableView(_parent)
  , d_ptr(new qDMMLTableViewPrivate(*this))
{
  Q_D(qDMMLTableView);
  d->init();
}

//------------------------------------------------------------------------------
qDMMLTableView::~qDMMLTableView() = default;

//------------------------------------------------------------------------------
qDMMLTableModel* qDMMLTableView::tableModel()const
{
  return qobject_cast<qDMMLTableModel*>(this->sortFilterProxyModel()->sourceModel());
}

//------------------------------------------------------------------------------
QSortFilterProxyModel* qDMMLTableView::sortFilterProxyModel()const
{
  return qobject_cast<QSortFilterProxyModel*>(this->model());
}

//------------------------------------------------------------------------------
void qDMMLTableView::setDMMLTableNode(vtkDMMLNode* node)
{
  this->setDMMLTableNode(vtkDMMLTableNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qDMMLTableView::setDMMLTableNode(vtkDMMLTableNode* node)
{
  qDMMLTableModel* dmmlModel = this->tableModel();
  if (!dmmlModel)
    {
    qCritical("qDMMLTableView::setDMMLTableNode failed: invalid model");
    return;
    }

  dmmlModel->setDMMLTableNode(node);
  this->sortFilterProxyModel()->invalidate();

  this->horizontalHeader()->setMinimumSectionSize(60);
  this->resizeColumnsToContents();

  emit selectionChanged();
}

//------------------------------------------------------------------------------
vtkDMMLTableNode* qDMMLTableView::dmmlTableNode()const
{
  qDMMLTableModel* dmmlModel = this->tableModel();
  if (!dmmlModel)
    {
    qCritical("qDMMLTableView::dmmlTableNode failed: model is invalid");
    return nullptr;
    }
  return dmmlModel->dmmlTableNode();
}

//------------------------------------------------------------------------------
bool qDMMLTableView::transposed()const
{
  Q_D(const qDMMLTableView);
  CTK_CHECK_AND_RETURN_FALSE_IF_FAIL(d->verifyTableModelAndNode)
  return tableModel()->transposed();
}

//------------------------------------------------------------------------------
void qDMMLTableView::setTransposed(bool transposed)
{
  Q_D(qDMMLTableView);
  CTK_CHECK_AND_RETURN_IF_FAIL(d->verifyTableModelAndNode)
  tableModel()->setTransposed(transposed);
}

//------------------------------------------------------------------------------
void qDMMLTableView::keyPressEvent(QKeyEvent *event)
{
  if(event->matches(QKeySequence::Copy) )
    {
    this->copySelection();
    return;
    }
  if(event->matches(QKeySequence::Paste) )
    {
    this->pasteSelection();
    return;
    }

  // Prevent giving the focus to the previous/next widget if arrow keys are used
  // at the edge of the table (without this: if the current cell is in the top
  // row and user press the Up key, the focus goes from the table to the previous
  // widget in the tab order)
  if (model() && (
    (event->key() == Qt::Key_Left && currentIndex().column() == 0)
    || (event->key() == Qt::Key_Up && currentIndex().row() == 0)
    || (event->key() == Qt::Key_Right && currentIndex().column() == model()->columnCount()-1)
    || (event->key() == Qt::Key_Down && currentIndex().row() == model()->rowCount()-1) ) )
    {
    return;
    }
  QTableView::keyPressEvent(event);
}

//-----------------------------------------------------------------------------
void qDMMLTableView::copySelection()
{
  Q_D(qDMMLTableView);
  CTK_CHECK_AND_RETURN_IF_FAIL(d->verifyTableModelAndNode)

  if (!selectionModel()->hasSelection())
    {
    return;
    }

  qDMMLTableModel* dmmlModel = tableModel();
  QItemSelectionModel* selection = selectionModel();
  QString textToCopy;
  bool firstLine = true;
  for (int rowIndex=0; rowIndex<dmmlModel->rowCount(); rowIndex++)
    {
    if (!selection->rowIntersectsSelection(rowIndex, QModelIndex()))
      {
      // no items are selected in this entire row, skip it
      continue;
      }
    if (firstLine)
      {
      firstLine = false;
      }
    else
      {
      textToCopy.append('\n');
      }
    bool firstItemInLine = true;
    for (int columnIndex=0; columnIndex<dmmlModel->columnCount(); columnIndex++)
      {
      if (!selection->columnIntersectsSelection(columnIndex, QModelIndex()))
        {
        // no items are selected in this entire column, skip it
        continue;
        }
      if (firstItemInLine)
        {
        firstItemInLine = false;
        }
      else
        {
        textToCopy.append('\t');
        }
      QStandardItem *item = dmmlModel->item(rowIndex, columnIndex);
      if (item->isCheckable())
        {
        textToCopy.append(item->checkState() == Qt::Checked ? "1" : "0");
        }
      else
        {
        textToCopy.append(item->text());
        }
      }
    }

  QApplication::clipboard()->setText(textToCopy);
}

//-----------------------------------------------------------------------------
void qDMMLTableView::pasteSelection()
{
  Q_D(qDMMLTableView);
  CTK_CHECK_AND_RETURN_IF_FAIL(d->verifyTableModelAndNode)

  QString text = QApplication::clipboard()->text();
  if (text.isEmpty())
    {
    return;
    }
  QStringList lines = text.split('\n');
  if (lines.empty())
    {
    // nothing to paste
    return;
    }
  if (lines.back().isEmpty())
    {
    // usually there is an extra empty line at the end
    // remove that to avoid adding an extra empty line to the table
    lines.pop_back();
    }
  if (lines.empty())
    {
    // nothing to paste
    return;
    }

  // If there is no selection then paste from top-left
  qDMMLTableModel* dmmlModel = tableModel();
  int rowIndex = currentIndex().row();
  if (rowIndex < 0)
    {
    rowIndex = 0;
    }
  int startColumnIndex = currentIndex().column();
  if (startColumnIndex < 0)
    {
    startColumnIndex = 0;
    }

  // If there are multiple table views then each cell modification would trigger
  // a table update, which may be very slow in case of large tables, therefore
  // we need to use StartModify/EndModify.
  vtkDMMLTableNode* tableNode = dmmlTableNode();
  int wasModified = tableNode->StartModify();

  // Pre-allocate new rows (to reduce number of updateModelFromDMML() calls
  if (tableNode->GetNumberOfColumns() == 0)
    {
    // insertRow() may insert two rows if the table is empty (one column header + one data item),
    // which could cause an extra row added to the table. To prevent this, we add a column instead,
    // which is just a single value.
    insertColumn();
    dmmlModel->updateModelFromDMML();
    }
  for (int i = lines.size() - (dmmlModel->rowCount() - rowIndex); i>0; i--)
    {
    insertRow();
    }
  dmmlModel->updateModelFromDMML();

  foreach(QString line, lines)
    {
    int columnIndex = startColumnIndex;
    QStringList cells = line.split('\t');
    foreach(QString cell, cells)
      {
      // Pre-allocate new columns (enough for at least for storing all the items in the current row)
      if (columnIndex >= dmmlModel->columnCount())
        {
        for (int i = cells.size() - (dmmlModel->columnCount() - startColumnIndex); i>0; i--)
          {
          insertColumn();
          }
        dmmlModel->updateModelFromDMML();
        }
      // Set values in items
      QStandardItem* item = dmmlModel->item(rowIndex,columnIndex);
      if (item != nullptr)
        {
        if (item->isCheckable())
          {
          item->setCheckState(cell.toInt() == 0 ? Qt::Unchecked : Qt::Checked);
          }
        else
          {
          item->setText(cell);
          }
        }
      else
        {
        qWarning() << "Failed to set " << cell << " in table cell (" << rowIndex << ", " << columnIndex << ")";
        }
      columnIndex++;
      }
    rowIndex++;
    }
  tableNode->EndModify(wasModified);
}

//-----------------------------------------------------------------------------
void qDMMLTableView::plotSelection()
{
  Q_D(qDMMLTableView);
  CTK_CHECK_AND_RETURN_IF_FAIL(d->verifyTableModelAndNode)

  vtkDMMLTableNode* tableNode = dmmlTableNode();

  if(!this->dmmlScene())
    {
    qWarning() << "qDMMLTableView::plotSelection failed: no dmmlScene available";
    return;
    }

  // Validate type of selected columns
  int stringColumnIndex = -1; // one string column is allowed (to be used as point labels)
  std::deque<int> columnIndices;
  QItemSelectionModel* selection = selectionModel();
  QModelIndexList selectedColumns = selection->selectedIndexes();
  for (int i = 0; i< selectedColumns.count(); i++)
    {
    QModelIndex index = selectedColumns.at(i);
    int columnIndex = index.column();
    if (std::find(columnIndices.begin(), columnIndices.end(), columnIndex) == columnIndices.end()
      && columnIndex != stringColumnIndex)
      {
      // found new column in selection
      vtkAbstractArray* column = tableNode->GetTable()->GetColumn(columnIndex);
      if (!column || !column->GetName())
        {
        QString message = QString("Column %1 is invalid. Failed to generate a plot").arg(columnIndex);
        qCritical() << Q_FUNC_INFO << ": " << message;
        QMessageBox::warning(nullptr, tr("Failed to create Plot"), message);
        return;
        }
      int columnDataType = column->GetDataType();
      if (columnDataType == VTK_BIT)
        {
        QString message = QString("Type of column %1 is 'bit'. Plotting of these types are currently not supported."
          " Please convert the data type of this column to numeric using Table module's Column properties section,"
          " or select different columns for plotting.").arg(column->GetName());
        qCritical() << Q_FUNC_INFO << ": " << message;
        QMessageBox::warning(nullptr, tr("Failed to create Plot"), message);
        return;
        }
      if (columnDataType == VTK_STRING)
        {
        if (stringColumnIndex < 0)
          {
          // no string columns so far, use this
          stringColumnIndex = columnIndex;
          }
        else
          {
          QString message = QString("Multiple 'string' type of columns are selected for plotting (%1, %2) but only one is allowed."
            " Please change selection or convert data type of this column to numeric using Table module's 'Column properties' section."
            ).arg(tableNode->GetColumnName(stringColumnIndex).c_str(), column->GetName());
          qCritical() << Q_FUNC_INFO << ": " << message;
          QMessageBox::warning(nullptr, tr("Failed to create Plot"), message);
          return;
          }
        }
      else
        {
        columnIndices.push_back(columnIndex);
        }
      }
    }
  if (columnIndices.size() == 0)
    {
    QString message = QString("A single 'string' type column is selected."
      " Please change selection or convert data type of this column to numeric using Table module's 'Column properties' section.");
    qCritical() << Q_FUNC_INFO << ": " << message;
    QMessageBox::warning(nullptr, tr("Failed to plot data"), message);
    return;
    }

  // Determine which column to be used as X axis
  int plotType = vtkDMMLPlotSeriesNode::PlotTypeLine;
  std::string xColumnName;
  if (stringColumnIndex >= 0)
    {
    // there was a string column, create a line plot
    xColumnName = tableNode->GetColumnName(stringColumnIndex);
    }
  else if (columnIndices.size()>1)
    {
    // there was no string column and there are at least two columns,
    // create scatter plot(s) using the first selected column as X axis
    plotType = vtkDMMLPlotSeriesNode::PlotTypeScatter;
    xColumnName = tableNode->GetColumnName(columnIndices[0]);
    columnIndices.pop_front();
    }

  // Make current plot chart active and visible
  vtkDMMLSelectionNode* selectionNode = vtkDMMLSelectionNode::SafeDownCast(
  this->dmmlScene()->GetNodeByID("vtkDMMLSelectionNodeSingleton"));
  if (!selectionNode)
    {
    qWarning() << "qDMMLTableView::plotSelection failed: invalid selection Node";
    return;
    }

  // Set a Plot Layout
  vtkDMMLLayoutNode* layoutNode = vtkDMMLLayoutNode::SafeDownCast(
    this->dmmlScene()->GetFirstNodeByClass("vtkDMMLLayoutNode"));
  if (!layoutNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to get layout node!";
    return;
    }
  int viewArray = layoutNode->GetViewArrangement();
  if (viewArray != vtkDMMLLayoutNode::CjyxLayoutConventionalPlotView  &&
      viewArray != vtkDMMLLayoutNode::CjyxLayoutFourUpPlotView        &&
      viewArray != vtkDMMLLayoutNode::CjyxLayoutFourUpPlotTableView   &&
      viewArray != vtkDMMLLayoutNode::CjyxLayoutOneUpPlotView         &&
      viewArray != vtkDMMLLayoutNode::CjyxLayoutThreeOverThreePlotView)
    {
    layoutNode->SetViewArrangement(vtkDMMLLayoutNode::CjyxLayoutConventionalPlotView);
    }

  vtkSmartPointer<vtkDMMLPlotChartNode> plotChartNode = vtkDMMLPlotChartNode::SafeDownCast(
    this->dmmlScene()->GetNodeByID(selectionNode->GetActivePlotChartID()));

  if (!plotChartNode)
    {
    plotChartNode = vtkSmartPointer<vtkDMMLPlotChartNode>::New();
    this->dmmlScene()->AddNode(plotChartNode);
    selectionNode->SetActivePlotChartID(plotChartNode->GetID());
    }

  vtkDMMLPlotViewNode* plotViewNode = vtkDMMLPlotViewNode::SafeDownCast(this->dmmlScene()->GetSingletonNode("PlotView1", "vtkDMMLPlotViewNode"));
  if (plotViewNode && plotViewNode->GetDoPropagatePlotChartSelection())
    {
    plotViewNode->SetPlotChartNodeID(plotChartNode->GetID());
    }

  std::string plotMarkerStyle;
  plotChartNode->GetPropertyFromAllPlotSeriesNodes(vtkDMMLPlotChartNode::PlotMarkerStyle, plotMarkerStyle);

  // Remove columns/plots not selected from plotChartNode
  plotChartNode->RemoveAllPlotSeriesNodeIDs();

  for (std::deque<int>::iterator columnIndexIt = columnIndices.begin(); columnIndexIt != columnIndices.end(); ++columnIndexIt)
    {
    std::string yColumnName = tableNode->GetColumnName(*columnIndexIt);

    // Check if there is already a PlotSeriesNode that has the same name as this Column and reuse that to avoid node duplication
    vtkSmartPointer<vtkCollection> colPlots = vtkSmartPointer<vtkCollection>::Take(
      this->dmmlScene()->GetNodesByClassByName("vtkDMMLPlotSeriesNode", yColumnName.c_str()));
    if (colPlots == nullptr)
      {
      continue;
      }
    vtkDMMLPlotSeriesNode *plotSeriesNode = nullptr;
    for (int plotIndex = 0; plotIndex < colPlots->GetNumberOfItems(); plotIndex++)
      {
      plotSeriesNode = vtkDMMLPlotSeriesNode::SafeDownCast(colPlots->GetItemAsObject(plotIndex));
      if (plotSeriesNode != nullptr)
        {
        break;
        }
      }

    // Create a PlotSeriesNode if a usable node has not been found
    if (plotSeriesNode == nullptr)
      {
      plotSeriesNode = vtkDMMLPlotSeriesNode::SafeDownCast(this->dmmlScene()->AddNewNodeByClass(
        "vtkDMMLPlotSeriesNode", yColumnName.c_str()));
      plotSeriesNode->SetUniqueColor();
      }
    if (plotType == vtkDMMLPlotSeriesNode::PlotTypeScatter)
      {
      plotSeriesNode->SetXColumnName(xColumnName);
      }
    else
      {
      plotSeriesNode->SetLabelColumnName(xColumnName);
      plotSeriesNode->SetMarkerStyle(VTK_MARKER_SQUARE);
      }
    plotSeriesNode->SetYColumnName(yColumnName);
    plotSeriesNode->SetAndObserveTableNodeID(tableNode->GetID());

    std::string namePlotSeriesNode = plotSeriesNode->GetName();
    std::size_t found = namePlotSeriesNode.find("Markups");
    if (found != std::string::npos)
      {
      plotChartNode->RemovePlotSeriesNodeID(plotSeriesNode->GetID());
      plotSeriesNode->GetNodeReference("Markups")->RemoveNodeReferenceIDs("Markups");
      this->dmmlScene()->RemoveNode(plotSeriesNode);
      continue;
      }

    // Set the type of the PlotSeriesNode
    plotSeriesNode->SetPlotType(plotType);

    if (!plotMarkerStyle.empty())
      {
      plotSeriesNode->SetMarkerStyle(plotSeriesNode->GetMarkerStyleFromString(plotMarkerStyle.c_str()));
      }

    // Add the reference of the PlotSeriesNode in the active PlotChartNode
    plotChartNode->AddAndObservePlotSeriesNodeID(plotSeriesNode->GetID());
    }
}

//-----------------------------------------------------------------------------
void qDMMLTableView::insertColumn()
{
  Q_D(qDMMLTableView);
  CTK_CHECK_AND_RETURN_IF_FAIL(d->verifyTableModelAndNode)
  if (tableModel()->transposed())
    {
    dmmlTableNode()->AddEmptyRow();
    }
  else
    {
    dmmlTableNode()->AddColumn();
    }
}

//-----------------------------------------------------------------------------
void qDMMLTableView::deleteColumn()
{
  Q_D(qDMMLTableView);
  CTK_CHECK_AND_RETURN_IF_FAIL(d->verifyTableModelAndNode)
  tableModel()->removeSelectionFromDMML(selectionModel()->selectedIndexes(), false);
  clearSelection();
}

//-----------------------------------------------------------------------------
void qDMMLTableView::insertRow()
{
  Q_D(qDMMLTableView);
  CTK_CHECK_AND_RETURN_IF_FAIL(d->verifyTableModelAndNode)
  if (tableModel()->transposed())
    {
    dmmlTableNode()->AddColumn();
    }
  else
    {
    dmmlTableNode()->AddEmptyRow();
    }
}

//-----------------------------------------------------------------------------
void qDMMLTableView::deleteRow()
{
  Q_D(qDMMLTableView);
  CTK_CHECK_AND_RETURN_IF_FAIL(d->verifyTableModelAndNode)
  tableModel()->removeSelectionFromDMML(selectionModel()->selectedIndexes(), true);
  clearSelection();
}

//-----------------------------------------------------------------------------
bool qDMMLTableView::firstRowLocked()const
{
  Q_D(const qDMMLTableView);
  CTK_CHECK_AND_RETURN_FALSE_IF_FAIL(d->verifyTableModelAndNode)
  if (tableModel()->transposed())
    {
    return dmmlTableNode()->GetUseFirstColumnAsRowHeader();
    }
  else
    {
    return dmmlTableNode()->GetUseColumnNameAsColumnHeader();
    }
}


//-----------------------------------------------------------------------------
void qDMMLTableView::setFirstRowLocked(bool locked)
{
  Q_D(qDMMLTableView);
  CTK_CHECK_AND_RETURN_IF_FAIL(d->verifyTableModelAndNode)
  if (tableModel()->transposed())
    {
    if (dmmlTableNode()->GetUseFirstColumnAsRowHeader()==locked)
      {
      //no change
      return;
      }
    dmmlTableNode()->SetUseFirstColumnAsRowHeader(locked);
    }
  else
    {
    if (dmmlTableNode()->GetUseColumnNameAsColumnHeader()==locked)
      {
      //no change
      return;
      }
    dmmlTableNode()->SetUseColumnNameAsColumnHeader(locked);
    }
  this->resizeColumnsToContents();
}

//-----------------------------------------------------------------------------
bool qDMMLTableView::firstColumnLocked()const
{
  Q_D(const qDMMLTableView);
  CTK_CHECK_AND_RETURN_FALSE_IF_FAIL(d->verifyTableModelAndNode)
  if (tableModel()->transposed())
    {
    return dmmlTableNode()->GetUseColumnNameAsColumnHeader();
    }
  else
    {
    return dmmlTableNode()->GetUseFirstColumnAsRowHeader();
    }
}

//-----------------------------------------------------------------------------
void qDMMLTableView::setFirstColumnLocked(bool locked)
{
  Q_D(qDMMLTableView);
  CTK_CHECK_AND_RETURN_IF_FAIL(d->verifyTableModelAndNode)
  if (tableModel()->transposed())
    {
    if (dmmlTableNode()->GetUseColumnNameAsColumnHeader()==locked)
      {
      //no change
      return;
      }
    dmmlTableNode()->SetUseColumnNameAsColumnHeader(locked);
    }
  else
    {
    if (dmmlTableNode()->GetUseFirstColumnAsRowHeader()==locked)
      {
      //no change
      return;
      }
    dmmlTableNode()->SetUseFirstColumnAsRowHeader(locked);
    }
  this->resizeColumnsToContents();
}

//------------------------------------------------------------------------------
void qDMMLTableView::setDMMLScene(vtkDMMLScene* newScene)
{
  Q_D(qDMMLTableView);
  if (newScene == d->DMMLScene)
    {
    return;
    }

  d->setDMMLScene(newScene);

  if (d->DMMLTableViewNode && newScene != d->DMMLTableViewNode->GetScene())
    {
    this->setDMMLTableViewNode(nullptr);
    }

  emit dmmlSceneChanged(newScene);
}

//---------------------------------------------------------------------------
void qDMMLTableView::setDMMLTableViewNode(vtkDMMLTableViewNode* newTableViewNode)
{
  Q_D(qDMMLTableView);
  if (d->DMMLTableViewNode == newTableViewNode)
    {
    return;
    }

  // connect modified event on TableViewNode to updating the widget
  d->qvtkReconnect(
    d->DMMLTableViewNode, newTableViewNode,
    vtkCommand::ModifiedEvent, d, SLOT(updateWidgetFromViewNode()));

  // cache the TableViewNode
  d->DMMLTableViewNode = newTableViewNode;

  // make sure the gui is up to date
  d->updateWidgetFromViewNode();
}

//---------------------------------------------------------------------------
vtkDMMLTableViewNode* qDMMLTableView::dmmlTableViewNode()const
{
  Q_D(const qDMMLTableView);
  return d->DMMLTableViewNode;
}

//---------------------------------------------------------------------------
vtkDMMLScene* qDMMLTableView::dmmlScene()const
{
  Q_D(const qDMMLTableView);
  return d->DMMLScene;
}

//---------------------------------------------------------------------------
QList<int> qDMMLTableView::selectedDMMLTableColumnIndices()const
{
  QList<int> dmmlColumnIndexList;
  QModelIndexList selection = selectionModel()->selectedIndexes();
  qDMMLTableModel* tableModel = this->tableModel();
  QModelIndex index;
  foreach(index, selection)
    {
    int dmmlColumnIndex = tableModel->dmmlTableColumnIndex(index);
    if (!dmmlColumnIndexList.contains(dmmlColumnIndex))
      {
      // insert unique row/column index only
      dmmlColumnIndexList.push_back(dmmlColumnIndex);
      }
    }
  return dmmlColumnIndexList;
}

//---------------------------------------------------------------------------
void qDMMLTableView::selectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
  QTableView::selectionChanged(selected, deselected);
  emit selectionChanged();
}
