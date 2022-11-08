/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Segmentations includes
#include "qDMMLSegmentsModel.h"
#include "qDMMLSegmentsTableView.h"
#include "qDMMLSortFilterSegmentsProxyModel.h"
#include "ui_qDMMLSegmentsTableView.h"
#include "vtkDMMLSegmentationNode.h"
#include "vtkDMMLSegmentationDisplayNode.h"
#include "vtkSegmentation.h"
#include "vtkSegment.h"

// Segmentations logic includes
#include "vtkCjyxSegmentationsModuleLogic.h"

// Terminologies includes
#include "qCjyxTerminologyItemDelegate.h"

// DMML includes
#include <vtkDMMLScene.h>
#include <vtkDMMLLabelMapVolumeNode.h>
#include <vtkDMMLModelNode.h>
#include <vtkDMMLSliceLogic.h>
#include <vtkDMMLSliceNode.h>

// Cjyx includes
#include <qCjyxApplication.h>
#include <qCjyxCoreApplication.h>
#include <qCjyxLayoutManager.h>
#include <qCjyxModuleManager.h>
#include <qCjyxAbstractCoreModule.h>
#include <qDMMLItemDelegate.h>
#include <qDMMLSliceWidget.h>

// VTK includes
#include <vtkWeakPointer.h>

// Qt includes
#include <QAction>
#include <QContextMenuEvent>
#include <QDebug>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QModelIndex>
#include <QStringList>
#include <QTimer>
#include <QToolButton>

#define ID_PROPERTY "ID"
#define VISIBILITY_PROPERTY "Visible"
#define STATUS_PROPERTY "Status"

//-----------------------------------------------------------------------------
struct SegmentListFilterParameters
{
  bool ShowStatusFilter[vtkCjyxSegmentationsModuleLogic::LastStatus];
  QString TextFilter;

  const char AttributeSeparator = ';';
  const char KeyValueSeparator = ':';
  const char ValueSeparator = ',';

  const char* TextFilterKey = "text";
  const char* StatusFilterKey = "status";

  SegmentListFilterParameters()
  {
    this->init();
  }

  void init()
  {
    this->TextFilter = "";
    for (int i = 0; i < vtkCjyxSegmentationsModuleLogic::LastStatus; ++i)
      {
      this->ShowStatusFilter[i] = false;
      }
  }

  /// Returns a string representation of the data in the SegmentListFilterParameters
  QString serializeStatusFilter()
  {
    std::stringstream statusFilterStringStream;
    statusFilterStringStream << this->TextFilterKey << this->KeyValueSeparator << this->TextFilter.toStdString() << this->AttributeSeparator;
    statusFilterStringStream << this->StatusFilterKey << this->KeyValueSeparator;
    bool firstElement = true;
    for (int i = 0; i < vtkCjyxSegmentationsModuleLogic::LastStatus; ++i)
      {
      if (!this->ShowStatusFilter[i])
        {
        continue;
        }

      if (!firstElement)
        {
        statusFilterStringStream << this->ValueSeparator;
        }
      firstElement = false;

      statusFilterStringStream << vtkCjyxSegmentationsModuleLogic::GetSegmentStatusAsMachineReadableString(i);
      }
    return QString::fromStdString(statusFilterStringStream.str());
  };

  /// Converts a string representation to the underlying values, and sets them in the SegmentListFilterParameters
  void deserializeStatusFilter(QString filterString)
  {
    this->init();

    QStringList attributes = filterString.split(this->AttributeSeparator);
    for (QString attribute : attributes)
      {
      QStringList keyValue = attribute.split(this->KeyValueSeparator);
      if (keyValue.size() != 2)
        {
        continue;
        }
      QString key = keyValue[0];
      QString value = keyValue[1];

      if (key == this->TextFilterKey)
        {
        this->TextFilter = value;
        }
      else if(key == this->StatusFilterKey)
        {
        QStringList statusFilters = value.split(this->ValueSeparator);
        for (QString statusString : statusFilters)
          {
          int status = vtkCjyxSegmentationsModuleLogic::GetSegmentStatusFromMachineReadableString(statusString.toStdString());
          if (status < 0 || status >= vtkCjyxSegmentationsModuleLogic::LastStatus)
            {
            continue;
            }
          this->ShowStatusFilter[status] = true;
          }
        }
      }
  }
};

//-----------------------------------------------------------------------------
class qDMMLSegmentsTableViewPrivate: public Ui_qDMMLSegmentsTableView
{
  Q_DECLARE_PUBLIC(qDMMLSegmentsTableView);

protected:
  qDMMLSegmentsTableView* const q_ptr;
public:
  qDMMLSegmentsTableViewPrivate(qDMMLSegmentsTableView& object);
  void init();

  /// Sets table message and takes care of the visibility of the label
  void setMessage(const QString& message);

public:
  /// Segmentation DMML node containing shown segments
  vtkWeakPointer<vtkDMMLSegmentationNode> SegmentationNode;

  /// Flag determining whether the long-press per-view segment visibility options are available
  bool AdvancedSegmentVisibility;

  /// Currently, if we are requesting segment display information from the
  /// segmentation display node,  the display node may emit modification events.
  /// We make sure these events do not interrupt the update process by setting
  /// IsUpdatingWidgetFromDMML to true when an update is already in progress.
  bool IsUpdatingWidgetFromDMML;

  bool IsFilterBarVisible;

  qDMMLSegmentsModel* Model;
  qDMMLSortFilterSegmentsProxyModel* SortFilterModel;

  QIcon StatusIcons[vtkCjyxSegmentationsModuleLogic::LastStatus];
  QPushButton* ShowStatusButtons[vtkCjyxSegmentationsModuleLogic::LastStatus];
  QTimer FilterParameterChangedTimer;

  bool JumpToSelectedSegmentEnabled;

  /// When the model is being reset, the blocking state and selected segment IDs are stored here.
  bool WasBlockingTableSignalsBeforeReset;
  QStringList SelectedSegmentIDsBeforeReset;
};

//-----------------------------------------------------------------------------
qDMMLSegmentsTableViewPrivate::qDMMLSegmentsTableViewPrivate(qDMMLSegmentsTableView& object)
  : q_ptr(&object)
  , SegmentationNode(nullptr)
  , AdvancedSegmentVisibility(false)
  , IsUpdatingWidgetFromDMML(false)
  , IsFilterBarVisible(false)
  , Model(nullptr)
  , SortFilterModel(nullptr)
  , JumpToSelectedSegmentEnabled(false)
  , WasBlockingTableSignalsBeforeReset(false)
{
  for (int status = 0; status < vtkCjyxSegmentationsModuleLogic::LastStatus; ++status)
    {
    this->ShowStatusButtons[status] = nullptr;
    }
}

//-----------------------------------------------------------------------------
void qDMMLSegmentsTableViewPrivate::init()
{
  Q_Q(qDMMLSegmentsTableView);

  this->setupUi(q);

  this->Model = new qDMMLSegmentsModel(this->SegmentsTable);
  this->SortFilterModel = new qDMMLSortFilterSegmentsProxyModel(this->SegmentsTable);
  this->SortFilterModel->setSourceModel(this->Model);
  this->SegmentsTable->setModel(this->SortFilterModel);

  for (int status = 0; status < vtkCjyxSegmentationsModuleLogic::LastStatus; ++status)
    {
    switch (status)
      {
      case vtkCjyxSegmentationsModuleLogic::NotStarted:
        this->ShowStatusButtons[status] = this->ShowNotStartedButton;
        this->StatusIcons[status] = QIcon(":Icons/NotStarted.png");
        break;
      case vtkCjyxSegmentationsModuleLogic::InProgress:
        this->ShowStatusButtons[status] = this->ShowInProgressButton;
        this->StatusIcons[status] = QIcon(":Icons/InProgress.png");
        break;
      case vtkCjyxSegmentationsModuleLogic::Completed:
        this->ShowStatusButtons[status] = this->ShowCompletedButton;
        this->StatusIcons[status] = QIcon(":Icons/Completed.png");
        break;
      case vtkCjyxSegmentationsModuleLogic::Flagged:
        this->ShowStatusButtons[status] = this->ShowFlaggedButton;
        this->StatusIcons[status] = QIcon(":Icons/Flagged.png");
        break;
      default:
        this->ShowStatusButtons[status] = nullptr;
        this->StatusIcons[status] = QIcon();
      }
    if (this->ShowStatusButtons[status])
      {
      this->ShowStatusButtons[status]->setProperty(STATUS_PROPERTY, status);
      }
    }

  // Hide filter bar to simplify default GUI. User can enable to handle many segments
  q->setFilterBarVisible(false);

  // Hide layer column
  q->setLayerColumnVisible(false);

  this->setMessage(QString());

  this->SegmentsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  this->SegmentsTable->horizontalHeader()->setSectionResizeMode(this->Model->nameColumn(), QHeaderView::Stretch);
  this->SegmentsTable->horizontalHeader()->setStretchLastSection(false);
  this->SegmentsTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

  // Select rows
  this->SegmentsTable->setSelectionBehavior(QAbstractItemView::SelectRows);

  // Unset read-only by default (edit triggers are double click and edit key press)
  q->setReadOnly(false);

  // Setup filter parameter changed timer
  this->FilterParameterChangedTimer.setInterval(500);
  this->FilterParameterChangedTimer.setSingleShot(true);

  // Make connections
  QObject::connect(&this->FilterParameterChangedTimer, &QTimer::timeout, q, &qDMMLSegmentsTableView::updateDMMLFromFilterParameters);
  QObject::connect(this->SegmentsTable->selectionModel(), &QItemSelectionModel::selectionChanged, q, &qDMMLSegmentsTableView::onSegmentSelectionChanged);
  QObject::connect(this->Model, &qDMMLSegmentsModel::segmentAboutToBeModified, q, &qDMMLSegmentsTableView::segmentAboutToBeModified);
  QObject::connect(this->Model, &QAbstractItemModel::modelAboutToBeReset, q, &qDMMLSegmentsTableView::modelAboutToBeReset);
  QObject::connect(this->Model, &QAbstractItemModel::modelReset, q, &qDMMLSegmentsTableView::modelReset);
  QObject::connect(this->SegmentsTable, &QTableView::clicked, q, &qDMMLSegmentsTableView::onSegmentsTableClicked);
  QObject::connect(this->FilterLineEdit, &ctkSearchBox::textEdited, this->SortFilterModel, &qDMMLSortFilterSegmentsProxyModel::setTextFilter);
  for (QPushButton* button : this->ShowStatusButtons)
    {
    if (!button)
      {
      continue;
      }
    QObject::connect(button, &QToolButton::clicked, q, &qDMMLSegmentsTableView::onShowStatusButtonClicked);
    }
  QObject::connect(this->SortFilterModel, &qDMMLSortFilterSegmentsProxyModel::filterModified, q, &qDMMLSegmentsTableView::onSegmentsFilterModified);

  // Set item delegate to handle color and opacity changes
  this->SegmentsTable->setItemDelegateForColumn(this->Model->colorColumn(), new qCjyxTerminologyItemDelegate(this->SegmentsTable));
  this->SegmentsTable->setItemDelegateForColumn(this->Model->opacityColumn(), new qDMMLItemDelegate(this->SegmentsTable));
  this->SegmentsTable->installEventFilter(q);
}

//-----------------------------------------------------------------------------
void qDMMLSegmentsTableViewPrivate::setMessage(const QString& message)
{
  this->SegmentsTableMessageLabel->setVisible(!message.isEmpty());
  this->SegmentsTableMessageLabel->setText(message);
}

//-----------------------------------------------------------------------------
// qDMMLSegmentsTableView methods

//-----------------------------------------------------------------------------
qDMMLSegmentsTableView::qDMMLSegmentsTableView(QWidget* _parent)
  : qDMMLWidget(_parent)
  , d_ptr(new qDMMLSegmentsTableViewPrivate(*this))
{
  Q_D(qDMMLSegmentsTableView);
  d->init();
}

//-----------------------------------------------------------------------------
qDMMLSegmentsTableView::~qDMMLSegmentsTableView() = default;

//-----------------------------------------------------------------------------
void qDMMLSegmentsTableView::setSegmentationNode(vtkDMMLNode* node)
{
  Q_D(qDMMLSegmentsTableView);

  vtkDMMLSegmentationNode* segmentationNode = vtkDMMLSegmentationNode::SafeDownCast(node);
  d->SegmentationNode = segmentationNode;
  d->Model->setSegmentationNode(d->SegmentationNode);

  // Connect segment added/removed and display modified events to population of the table
  qvtkReconnect(d->SegmentationNode, segmentationNode, vtkSegmentation::SegmentAdded,
    this, SLOT(onSegmentAddedOrRemoved()));
  qvtkReconnect(d->SegmentationNode, segmentationNode, vtkSegmentation::SegmentRemoved,
    this, SLOT(onSegmentAddedOrRemoved()));
  qvtkReconnect(d->SegmentationNode, segmentationNode, vtkCommand::ModifiedEvent,
    this, SLOT(updateWidgetFromDMML()));
  this->onSegmentAddedOrRemoved();
  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qDMMLSegmentsTableView::onShowStatusButtonClicked()
{
  Q_D(qDMMLSegmentsTableView);
  QPushButton* button = qobject_cast<QPushButton*>(sender());
  if (!button)
    {
    return;
    }
  int status = button->property(STATUS_PROPERTY).toInt();
  d->SortFilterModel->setShowStatus(status, button->isChecked());
}

//-----------------------------------------------------------------------------
void qDMMLSegmentsTableView::onSegmentsFilterModified()
{
  Q_D(qDMMLSegmentsTableView);

  QString textFilter = d->SortFilterModel->textFilter();
  if (d->FilterLineEdit->text() != textFilter)
    {
    d->FilterLineEdit->setText(textFilter);
    }

  for (int status = 0; status < vtkCjyxSegmentationsModuleLogic::LastStatus; ++status)
    {
    QPushButton* button = d->ShowStatusButtons[status];
    if (!button)
      {
      continue;
      }
    button->setChecked(d->SortFilterModel->showStatus(status));
    }

  if (d->SegmentationNode && !d->IsUpdatingWidgetFromDMML)
    {
    d->FilterParameterChangedTimer.start();
    }
}

//-----------------------------------------------------------------------------
void qDMMLSegmentsTableView::updateDMMLFromFilterParameters()
{
  Q_D(qDMMLSegmentsTableView);
  if (!d->SegmentationNode)
    {
    qCritical() << Q_FUNC_INFO << "Invalid segmentation node";
    return;
    }

  SegmentListFilterParameters filterParameters;
  filterParameters.TextFilter = d->SortFilterModel->textFilter();
  for (int status = 0; status < vtkCjyxSegmentationsModuleLogic::LastStatus; ++status)
    {
    bool showStatus = d->SortFilterModel->showStatus(status);
    filterParameters.ShowStatusFilter[status] = showStatus;
    }

  DMMLNodeModifyBlocker blocker(d->SegmentationNode);
  d->SegmentationNode->SetSegmentListFilterEnabled(d->IsFilterBarVisible);
  std::string filterString = filterParameters.serializeStatusFilter().toStdString();
  d->SegmentationNode->SetSegmentListFilterOptions(filterString);
}

//-----------------------------------------------------------------------------
void qDMMLSegmentsTableView::onSegmentsTableClicked(const QModelIndex& modelIndex)
{
  Q_D(qDMMLSegmentsTableView);
  QString segmentId = d->SortFilterModel->segmentIDFromIndex(modelIndex);
  QStandardItem* item = d->Model->itemFromSegmentID(segmentId);
  if (!d->SegmentationNode || !item)
    {
    return;
    }

  Qt::ItemFlags flags = item->flags();
  if (!flags.testFlag(Qt::ItemIsSelectable))
    {
    return;
    }

  vtkSegment* segment = d->SegmentationNode->GetSegmentation()->GetSegment(segmentId.toStdString());
  if (modelIndex.column() == d->Model->visibilityColumn())
    {
    // Set all visibility types to segment referenced by button toggled
    int visible = !item->data(qDMMLSegmentsModel::VisibilityRole).toInt();
    this->setSegmentVisibility(segmentId, visible, -1, -1, -1);
    }
  else if (modelIndex.column() == d->Model->statusColumn())
    {
    int status = vtkCjyxSegmentationsModuleLogic::GetSegmentStatus(segment);
    switch (status)
      {
      case vtkCjyxSegmentationsModuleLogic::SegmentStatus::Flagged:
        status = vtkCjyxSegmentationsModuleLogic::SegmentStatus::Completed;
        break;
      default:
        ++status;
        if (status >= vtkCjyxSegmentationsModuleLogic::SegmentStatus::LastStatus)
          {
          status = 0;
          }
        break;
      }
    vtkCjyxSegmentationsModuleLogic::SetSegmentStatus(segment, status);
    }
}

//---------------------------------------------------------------------------
void qDMMLSegmentsTableView::onSegmentAddedOrRemoved()
{
  Q_D(qDMMLSegmentsTableView);

  if (!d->SegmentationNode)
    {
    d->setMessage(tr("No node is selected"));
    return;
    }
  else if (d->SegmentationNode->GetSegmentation()->GetNumberOfSegments() == 0)
    {
    d->setMessage(tr("Empty segmentation"));
    return;
    }
  d->setMessage(QString());
}

//---------------------------------------------------------------------------
void qDMMLSegmentsTableView::updateWidgetFromDMML()
{
  Q_D(qDMMLSegmentsTableView);
  if (!d->SegmentationNode)
    {
    return;
    }

  bool wasUpdatingFromDMML = d->IsUpdatingWidgetFromDMML;
  d->IsUpdatingWidgetFromDMML = true;

  bool listFilterEnabled = d->SegmentationNode->GetSegmentListFilterEnabled();
  this->setFilterBarVisible(listFilterEnabled);

  QString filterOptions = QString::fromStdString(d->SegmentationNode->GetSegmentListFilterOptions());

  SegmentListFilterParameters filterParameters;
  filterParameters.deserializeStatusFilter(filterOptions);

  d->SortFilterModel->setTextFilter(filterParameters.TextFilter);
  for (int status = 0; status < vtkCjyxSegmentationsModuleLogic::LastStatus; ++status)
    {
    d->SortFilterModel->setShowStatus(status, filterParameters.ShowStatusFilter[status]);
    }

  d->IsUpdatingWidgetFromDMML = wasUpdatingFromDMML;
}

//---------------------------------------------------------------------------
void qDMMLSegmentsTableView::setDMMLScene(vtkDMMLScene* newScene)
{
  Q_D(qDMMLSegmentsTableView);
  if (newScene == this->dmmlScene())
    {
    return;
    }

  if (d->SegmentationNode && newScene != d->SegmentationNode->GetScene())
    {
    this->setSegmentationNode(nullptr);
    }

  Superclass::setDMMLScene(newScene);
}

//-----------------------------------------------------------------------------
vtkDMMLNode* qDMMLSegmentsTableView::segmentationNode()
{
  Q_D(qDMMLSegmentsTableView);

  return d->SegmentationNode;
}

//--------------------------------------------------------------------------
qDMMLSortFilterSegmentsProxyModel* qDMMLSegmentsTableView::sortFilterProxyModel()const
{
  Q_D(const qDMMLSegmentsTableView);
  if (!d->SortFilterModel)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid sort filter proxy model";
    return nullptr;
    }
  return d->SortFilterModel;
}

//--------------------------------------------------------------------------
qDMMLSegmentsModel* qDMMLSegmentsTableView::model()const
{
  Q_D(const qDMMLSegmentsTableView);
  if (!d->Model)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid data model";
    return nullptr;
    }
  return d->Model;
}

//-----------------------------------------------------------------------------
QTableView* qDMMLSegmentsTableView::tableWidget()
{
  Q_D(qDMMLSegmentsTableView);
  return d->SegmentsTable;
}

//-----------------------------------------------------------------------------
void qDMMLSegmentsTableView::onSegmentSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
  Q_D(qDMMLSegmentsTableView);
  if (d->JumpToSelectedSegmentEnabled)
    {
    this->jumpSlices();
    }
  if (d->SegmentsTable->signalsBlocked())
    {
    return;
    }
  emit selectionChanged(selected, deselected);
}

//-----------------------------------------------------------------------------
void qDMMLSegmentsTableView::onVisibility3DActionToggled(bool visible)
{
  QAction* senderAction = qobject_cast<QAction*>(sender());
  if (!senderAction)
    {
    return;
    }

  // Set 3D visibility to segment referenced by action toggled
  this->setSegmentVisibility(senderAction, -1, visible, -1, -1);
}

//-----------------------------------------------------------------------------
void qDMMLSegmentsTableView::onVisibility2DFillActionToggled(bool visible)
{
  QAction* senderAction = qobject_cast<QAction*>(sender());
  if (!senderAction)
    {
    return;
    }

  // Set 2D fill visibility to segment referenced by action toggled
  this->setSegmentVisibility(senderAction, -1, -1, visible, -1);
}

//-----------------------------------------------------------------------------
void qDMMLSegmentsTableView::onVisibility2DOutlineActionToggled(bool visible)
{
  QAction* senderAction = qobject_cast<QAction*>(sender());
  if (!senderAction)
    {
    return;
    }

  // Set 2D outline visibility to segment referenced by action toggled
  this->setSegmentVisibility(senderAction, -1, -1, -1, visible);
}

//-----------------------------------------------------------------------------
void qDMMLSegmentsTableView::setSegmentVisibility(QObject* senderObject, int visible, int visible3D, int visible2DFill, int visible2DOutline)
{
  Q_D(qDMMLSegmentsTableView);

  if (!d->SegmentationNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: segmentation node is not set";
    return;
    }

  QString segmentId = senderObject->property(ID_PROPERTY).toString();
  this->setSegmentVisibility(segmentId, visible, visible3D, visible2DFill, visible2DOutline);
}

//-----------------------------------------------------------------------------
void qDMMLSegmentsTableView::setSegmentVisibility(QString segmentId, int visible, int visible3D, int visible2DFill, int visible2DOutline)
{
  Q_D(qDMMLSegmentsTableView);

  if (!d->SegmentationNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: segmentation node is not set";
    return;
    }

  vtkDMMLSegmentationDisplayNode* displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(
    d->SegmentationNode->GetDisplayNode() );
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": No display node for segmentation!";
    return;
    }
  vtkDMMLSegmentationDisplayNode::SegmentDisplayProperties properties;
  displayNode->GetSegmentDisplayProperties(segmentId.toStdString(), properties);

  // Change visibility to all modes
  bool valueChanged = false;
  if (visible == 0 || visible == 1)
    {
    properties.Visible = (bool)visible;

    // If overall visibility is explicitly set to true then enable all visibility options
    // to make sure that something is actually visible.
    if (properties.Visible && !properties.Visible3D && !properties.Visible2DFill && !properties.Visible2DOutline)
      {
      properties.Visible3D = true;
      properties.Visible2DFill = true;
      properties.Visible2DOutline = true;
      }

    valueChanged = true;
    }
  if (visible3D == 0 || visible3D == 1)
    {
    properties.Visible3D = (bool)visible3D;
    valueChanged = true;
    }
  if (visible2DFill == 0 || visible2DFill == 1)
    {
    properties.Visible2DFill = (bool)visible2DFill;
    valueChanged = true;
    }
  if (visible2DOutline == 0 || visible2DOutline == 1)
    {
    properties.Visible2DOutline = (bool)visible2DOutline;
    valueChanged = true;
    }

  // Set visibility to display node
  if (valueChanged)
    {
    displayNode->SetSegmentDisplayProperties(segmentId.toStdString(), properties);
    }
}

//-----------------------------------------------------------------------------
int qDMMLSegmentsTableView::segmentCount() const
{
  Q_D(const qDMMLSegmentsTableView);

  return d->Model->rowCount();
}

//-----------------------------------------------------------------------------
QStringList qDMMLSegmentsTableView::selectedSegmentIDs()
{
  Q_D(qDMMLSegmentsTableView);
  if (!d->SegmentsTable->selectionModel()->hasSelection())
    {
    return QStringList();
    }

  QStringList selectedSegmentIds;
  for (int row = 0; row < d->SortFilterModel->rowCount(); ++row)
    {
    if (!d->SegmentsTable->selectionModel()->isRowSelected(row, QModelIndex()))
      {
      continue;
      }
    selectedSegmentIds << d->SortFilterModel->segmentIDFromIndex(d->SortFilterModel->index(row, 0));
    }
  return selectedSegmentIds;
}

//-----------------------------------------------------------------------------
void qDMMLSegmentsTableView::setSelectedSegmentIDs(QStringList segmentIDs)
{
  Q_D(qDMMLSegmentsTableView);

  if (!d->SegmentationNode && !segmentIDs.empty())
    {
    qCritical() << Q_FUNC_INFO << " failed: segmentation node is not set";
    return;
    }
  if (segmentIDs == this->selectedSegmentIDs())
    {
    return;
    }

  bool validSelection = false;
  DMMLNodeModifyBlocker blocker(d->SegmentationNode);
  // First segment selection should also clear other selections
  QItemSelectionModel::SelectionFlag itemSelectionFlag = QItemSelectionModel::ClearAndSelect;
  for (QString segmentID : segmentIDs)
    {
    QModelIndex index = d->SortFilterModel->indexFromSegmentID(segmentID);
    if (!index.isValid())
      {
      continue;
      }
    validSelection = true;
    QItemSelectionModel::QItemSelectionModel::SelectionFlags flags = QFlags<QItemSelectionModel::SelectionFlag>();
    flags.setFlag(itemSelectionFlag);
    flags.setFlag(QItemSelectionModel::Rows);
    d->SegmentsTable->selectionModel()->select(index, flags);
    // Afther the first segment, we append to the current selection
    itemSelectionFlag = QItemSelectionModel::Select;
    }

  if (!validSelection)
    {
    // The list of segment IDs was either empty, or all IDs were invalid.
    d->SegmentsTable->selectionModel()->clearSelection();
    }
}

//-----------------------------------------------------------------------------
void qDMMLSegmentsTableView::clearSelection()
{
  Q_D(qDMMLSegmentsTableView);
  d->SegmentsTable->clearSelection();
}

//------------------------------------------------------------------------------
bool qDMMLSegmentsTableView::eventFilter(QObject* target, QEvent* event)
{
  Q_D(qDMMLSegmentsTableView);
  if (target == d->SegmentsTable)
    {
    // Prevent giving the focus to the previous/next widget if arrow keys are used
    // at the edge of the table (without this: if the current cell is in the top
    // row and user press the Up key, the focus goes from the table to the previous
    // widget in the tab order)
    if (event->type() == QEvent::KeyPress)
      {
      QKeyEvent* keyEvent = static_cast<QKeyEvent *>(event);
      QAbstractItemModel* model = d->SegmentsTable->model();
      QModelIndex currentIndex = d->SegmentsTable->currentIndex();

      if (model && (
        (keyEvent->key() == Qt::Key_Left && currentIndex.column() == 0)
        || (keyEvent->key() == Qt::Key_Up && currentIndex.row() == 0)
        || (keyEvent->key() == Qt::Key_Right && currentIndex.column() == model->columnCount() - 1)
        || (keyEvent->key() == Qt::Key_Down && currentIndex.row() == model->rowCount() - 1)))
        {
        return true;
        }
      }
    }
  return this->QWidget::eventFilter(target, event);
}

//------------------------------------------------------------------------------
void qDMMLSegmentsTableView::endProcessing()
{
}

//------------------------------------------------------------------------------
void qDMMLSegmentsTableView::setSelectionMode(int mode)
{
  Q_D(qDMMLSegmentsTableView);
  d->SegmentsTable->setSelectionMode(static_cast<QAbstractItemView::SelectionMode>(mode));
}

//------------------------------------------------------------------------------
void qDMMLSegmentsTableView::setHeaderVisible(bool visible)
{
  Q_D(qDMMLSegmentsTableView);
  d->SegmentsTable->horizontalHeader()->setVisible(visible);
}

//------------------------------------------------------------------------------
void qDMMLSegmentsTableView::setVisibilityColumnVisible(bool visible)
{
  Q_D(qDMMLSegmentsTableView);
  d->SegmentsTable->setColumnHidden(d->Model->visibilityColumn(), !visible);
}

//------------------------------------------------------------------------------
void qDMMLSegmentsTableView::setColorColumnVisible(bool visible)
{
  Q_D(qDMMLSegmentsTableView);
  d->SegmentsTable->setColumnHidden(d->Model->colorColumn(), !visible);
}

//------------------------------------------------------------------------------
void qDMMLSegmentsTableView::setOpacityColumnVisible(bool visible)
{
  Q_D(qDMMLSegmentsTableView);
  d->SegmentsTable->setColumnHidden(d->Model->opacityColumn(), !visible);
}

//------------------------------------------------------------------------------
void qDMMLSegmentsTableView::setStatusColumnVisible(bool visible)
{
  Q_D(qDMMLSegmentsTableView);
  d->SegmentsTable->setColumnHidden(d->Model->statusColumn(), !visible);
}

//------------------------------------------------------------------------------
void qDMMLSegmentsTableView::setLayerColumnVisible(bool visible)
{
  Q_D(qDMMLSegmentsTableView);
  d->SegmentsTable->setColumnHidden(d->Model->layerColumn(), !visible);
}

//------------------------------------------------------------------------------
void qDMMLSegmentsTableView::setReadOnly(bool aReadOnly)
{
  Q_D(qDMMLSegmentsTableView);
  if (aReadOnly)
    {
    d->SegmentsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
  else
    {
    d->SegmentsTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    }
}

//------------------------------------------------------------------------------
void qDMMLSegmentsTableView::setFilterBarVisible(bool visible)
{
  Q_D(qDMMLSegmentsTableView);
  d->FilterBar->setVisible(visible);
  d->IsFilterBarVisible = visible;
  d->SortFilterModel->setFilterEnabled(visible);
}

//------------------------------------------------------------------------------
int qDMMLSegmentsTableView::selectionMode()
{
  Q_D(qDMMLSegmentsTableView);
  return d->SegmentsTable->selectionMode();
}

//------------------------------------------------------------------------------
bool qDMMLSegmentsTableView::headerVisible()
{
  Q_D(qDMMLSegmentsTableView);
  return d->SegmentsTable->horizontalHeader()->isVisible();
}

//------------------------------------------------------------------------------
bool qDMMLSegmentsTableView::visibilityColumnVisible()
{
  Q_D(qDMMLSegmentsTableView);
  return !d->SegmentsTable->isColumnHidden(d->Model->visibilityColumn());
}

//------------------------------------------------------------------------------
bool qDMMLSegmentsTableView::colorColumnVisible()
{
  Q_D(qDMMLSegmentsTableView);
  return !d->SegmentsTable->isColumnHidden(d->Model->colorColumn());
}

//------------------------------------------------------------------------------
bool qDMMLSegmentsTableView::opacityColumnVisible()
{
  Q_D(qDMMLSegmentsTableView);
  return !d->SegmentsTable->isColumnHidden(d->Model->opacityColumn());
}

//------------------------------------------------------------------------------
bool qDMMLSegmentsTableView::statusColumnVisible()
{
  Q_D(qDMMLSegmentsTableView);
  return !d->SegmentsTable->isColumnHidden(d->Model->statusColumn());
}

//------------------------------------------------------------------------------
bool qDMMLSegmentsTableView::layerColumnVisible()
{
  Q_D(qDMMLSegmentsTableView);
  return !d->SegmentsTable->isColumnHidden(d->Model->layerColumn());
}

//------------------------------------------------------------------------------
bool qDMMLSegmentsTableView::readOnly()
{
  Q_D(qDMMLSegmentsTableView);
  return (d->SegmentsTable->editTriggers() == QAbstractItemView::NoEditTriggers);
}

//------------------------------------------------------------------------------
bool qDMMLSegmentsTableView::filterBarVisible()
{
  Q_D(qDMMLSegmentsTableView);
  return d->FilterBar->isVisible();
}

//------------------------------------------------------------------------------
QString qDMMLSegmentsTableView::textFilter()
{
  Q_D(qDMMLSegmentsTableView);
  return d->SortFilterModel->textFilter();
}

//------------------------------------------------------------------------------
void qDMMLSegmentsTableView::setTextFilter(QString filter)
{
  Q_D(qDMMLSegmentsTableView);
  d->SortFilterModel->setTextFilter(filter);
}

//------------------------------------------------------------------------------
bool qDMMLSegmentsTableView::statusShown(int status)
{
  Q_D(qDMMLSegmentsTableView);
  if (status < 0 || status >= vtkCjyxSegmentationsModuleLogic::LastStatus)
    {
    return false;
    }

  QPushButton* button = d->ShowStatusButtons[status];
  if (!button)
    {
    return false;
    }
  return button->isChecked();
}

//------------------------------------------------------------------------------
void qDMMLSegmentsTableView::setStatusShown(int status, bool shown)
{
  Q_D(qDMMLSegmentsTableView);
  if (status < 0 || status >= vtkCjyxSegmentationsModuleLogic::LastStatus)
    {
    return;
    }

  QPushButton* button = d->ShowStatusButtons[status];
  if (!button)
    {
    return;
    }
  button->setChecked(shown);
}

//------------------------------------------------------------------------------
int qDMMLSegmentsTableView::rowForSegmentID(QString segmentID)
{
  return this->sortFilterProxyModel()->indexFromSegmentID(segmentID).row();
}

//------------------------------------------------------------------------------
QString qDMMLSegmentsTableView::segmentIDForRow(int row)
{
  QModelIndex index = this->sortFilterProxyModel()->index(row, 0);
  return this->sortFilterProxyModel()->segmentIDFromIndex(index);
}

//------------------------------------------------------------------------------
void qDMMLSegmentsTableView::contextMenuEvent(QContextMenuEvent* event)
{
  Q_D(qDMMLSegmentsTableView);

  QMenu* contextMenu = new QMenu(this);

  QStringList selectedSegmentIDs = this->selectedSegmentIDs();

  if (selectedSegmentIDs.size() > 0)
    {
    QAction* showOnlySelectedAction = new QAction("Show only selected segments", this);
    QObject::connect(showOnlySelectedAction, SIGNAL(triggered()), this, SLOT(showOnlySelectedSegments()));
    contextMenu->addAction(showOnlySelectedAction);

    contextMenu->addSeparator();

    QAction* jumpSlicesAction = new QAction("Jump slices", this);
    QObject::connect(jumpSlicesAction, SIGNAL(triggered()), this, SLOT(jumpSlices()));
    contextMenu->addAction(jumpSlicesAction);

    contextMenu->addSeparator();

    QAction* moveUpAction = new QAction("Move selected segments up", this);
    QObject::connect(moveUpAction, SIGNAL(triggered()), this, SLOT(moveSelectedSegmentsUp()));
    contextMenu->addAction(moveUpAction);

    QAction* moveDownAction = new QAction("Move selected segments down", this);
    QObject::connect(moveDownAction, SIGNAL(triggered()), this, SLOT(moveSelectedSegmentsDown()));
    contextMenu->addAction(moveDownAction);
    }

  contextMenu->addSeparator();

  QAction* showFilterAction = new QAction("Show filter bar", this);
  showFilterAction->setCheckable(true);
  showFilterAction->setChecked(d->FilterBar->isVisible());
  QObject::connect(showFilterAction, SIGNAL(triggered(bool)), this, SLOT(setFilterBarVisible(bool)));
  contextMenu->addAction(showFilterAction);

  QAction* showLayerColumnAction = new QAction("Show layer column", this);
  showLayerColumnAction->setCheckable(true);
  showLayerColumnAction->setChecked(this->layerColumnVisible());
  QObject::connect(showLayerColumnAction, SIGNAL(triggered(bool)), this, SLOT(setLayerColumnVisible(bool)));
  contextMenu->addAction(showLayerColumnAction);

  QModelIndex index = d->SegmentsTable->indexAt(d->SegmentsTable->viewport()->mapFromGlobal(event->globalPos()));
  if (d->AdvancedSegmentVisibility && index.isValid())
    {
    QString segmentID = d->SortFilterModel->segmentIDFromIndex(index);

    // Get segment display properties
    vtkDMMLSegmentationDisplayNode::SegmentDisplayProperties properties;
    vtkDMMLSegmentationDisplayNode* displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(d->SegmentationNode->GetDisplayNode());
    if (displayNode)
      {
      displayNode->GetSegmentDisplayProperties(segmentID.toUtf8().constData(), properties);
      }

    contextMenu->addSeparator();

    QAction* visibility3DAction = new QAction("Show in 3D", this);
    visibility3DAction->setCheckable(true);
    visibility3DAction->setChecked(properties.Visible3D);
    visibility3DAction->setProperty(ID_PROPERTY, segmentID);
    QObject::connect(visibility3DAction, SIGNAL(triggered(bool)), this, SLOT(onVisibility3DActionToggled(bool)));
    contextMenu->addAction(visibility3DAction);

    QAction* visibility2DFillAction = new QAction("Show in 2D as fill", this);
    visibility2DFillAction->setCheckable(true);
    visibility2DFillAction->setChecked(properties.Visible2DFill);
    visibility2DFillAction->setProperty(ID_PROPERTY, segmentID);
    connect(visibility2DFillAction, SIGNAL(triggered(bool)), this, SLOT(onVisibility2DFillActionToggled(bool)));
    contextMenu->addAction(visibility2DFillAction);

    QAction* visibility2DOutlineAction = new QAction("Show in 2D as outline", this);
    visibility2DOutlineAction->setCheckable(true);
    visibility2DOutlineAction->setChecked(properties.Visible2DOutline);
    visibility2DOutlineAction->setProperty(ID_PROPERTY, segmentID);
    connect(visibility2DOutlineAction, SIGNAL(triggered(bool)), this, SLOT(onVisibility2DOutlineActionToggled(bool)));
    contextMenu->addAction(visibility2DOutlineAction);
    }

  if (selectedSegmentIDs.size() > 0)
    {
    contextMenu->addSeparator();
    for (int status = 0; status < vtkCjyxSegmentationsModuleLogic::LastStatus; ++status)
      {
      QString name = vtkCjyxSegmentationsModuleLogic::GetSegmentStatusAsHumanReadableString(status);
      QIcon icon = d->StatusIcons[status];

      QAction* setStatusAction = new QAction(name);
      setStatusAction->setIcon(icon);
      setStatusAction->setProperty(STATUS_PROPERTY, status);
      QObject::connect(setStatusAction, SIGNAL(triggered()), this, SLOT(setSelectedSegmentsStatus()));
      contextMenu->addAction(setStatusAction);
      }

    contextMenu->addSeparator();
    QAction* clearSelectedSegmentAction = new QAction("Clear selected segments", this);
    QObject::connect(clearSelectedSegmentAction, SIGNAL(triggered()), this, SLOT(clearSelectedSegments()));
    contextMenu->addAction(clearSelectedSegmentAction);
    }

  contextMenu->popup(event->globalPos());
}

//------------------------------------------------------------------------------
void qDMMLSegmentsTableView::setSelectedSegmentsStatus(int aStatus)
{
  Q_D(qDMMLSegmentsTableView);

  int status = aStatus;
  if (status == -1)
    {
    QAction* setStatusAction = qobject_cast<QAction*>(sender());
    Q_ASSERT(setStatusAction);
    if (!setStatusAction)
      {
      return;
      }
    status = setStatusAction->property(STATUS_PROPERTY).toInt();
    }

  if (!d->SegmentationNode)
    {
    qCritical() << Q_FUNC_INFO << "Invalid segmentation node";
    return;
    }
  vtkSegmentation* segmentation = d->SegmentationNode->GetSegmentation();
  if (!segmentation)
    {
    qCritical() << Q_FUNC_INFO << "Invalid segmentation";
    return;
    }

  DMMLNodeModifyBlocker blocker(d->SegmentationNode);
  QStringList selectedSegmentIDs = this->selectedSegmentIDs();
  for (QString segmentID : selectedSegmentIDs)
    {
    vtkSegment* segment = segmentation->GetSegment(segmentID.toStdString());
    if (segment)
      {
      vtkCjyxSegmentationsModuleLogic::SetSegmentStatus(segment, status);
      }
    }
}

//------------------------------------------------------------------------------
void qDMMLSegmentsTableView::clearSelectedSegments()
{
  Q_D(qDMMLSegmentsTableView);

  QAction* clearSegmentAction = qobject_cast<QAction*>(sender());
  Q_ASSERT(clearSegmentAction);

  if (!d->SegmentationNode)
    {
    qCritical() << Q_FUNC_INFO << "Invalid segmentation node";
    return;
    }
  vtkSegmentation* segmentation = d->SegmentationNode->GetSegmentation();
  if (!segmentation)
    {
    qCritical() << Q_FUNC_INFO << "Invalid segmentation";
    return;
    }

  QMessageBox messageBox;
  messageBox.addButton("Clear", QMessageBox::ButtonRole::AcceptRole);
  QPushButton* cancelButton = messageBox.addButton("Cancel", QMessageBox::ButtonRole::RejectRole);
  messageBox.setDefaultButton(cancelButton);
  messageBox.setText("Are you sure you want to clear the contents of the selected segments?");
  if (messageBox.exec() == QMessageBox::ButtonRole::RejectRole)
    {
    return;
    }

  QStringList selectedSegmentIDs = this->selectedSegmentIDs();
  for (QString segmentID : selectedSegmentIDs)
    {
    vtkCjyxSegmentationsModuleLogic::ClearSegment(d->SegmentationNode, segmentID.toStdString());
    }
}

//------------------------------------------------------------------------------
void qDMMLSegmentsTableView::showOnlySelectedSegments()
{
  QStringList selectedSegmentIDs = this->selectedSegmentIDs();
  if (selectedSegmentIDs.size() == 0)
    {
    qWarning() << Q_FUNC_INFO << ": No segment selected";
    return;
    }

  Q_D(qDMMLSegmentsTableView);
  if (!d->SegmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": No current segmentation node";
    return;
    }
  vtkDMMLSegmentationDisplayNode* displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(
    d->SegmentationNode->GetDisplayNode() );
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": No display node for segmentation " << d->SegmentationNode->GetName();
    return;
    }

  vtkSegmentation* segmentation = d->SegmentationNode->GetSegmentation();
  if (!segmentation)
    {
    qCritical() << Q_FUNC_INFO << ": No segmentation";
    return;
    }

  std::vector<std::string> displayedSegmentIDs;
  segmentation->GetSegmentIDs(displayedSegmentIDs);

  QStringList hiddenSegmentIDs = d->SortFilterModel->hideSegments();

  // Hide all segments except the selected ones
  DMMLNodeModifyBlocker blocker(displayNode);
  for (std::string displayedID : displayedSegmentIDs)
    {
    QString segmentID = QString::fromStdString(displayedID);
    if (hiddenSegmentIDs.contains(segmentID))
      {
      continue;
      }

    bool visible = false;
    if (selectedSegmentIDs.contains(segmentID))
      {
      visible = true;
      }

    displayNode->SetSegmentVisibility(displayedID, visible);
    }
}

//------------------------------------------------------------------------------
void qDMMLSegmentsTableView::jumpSlices()
{
  QStringList selectedSegmentIDs = this->selectedSegmentIDs();
  if (selectedSegmentIDs.size() == 0)
    {
    // No segment selected
    return;
    }

  Q_D(qDMMLSegmentsTableView);
  if (!d->SegmentationNode)
    {
    // No current segmentation node
    return;
    }

  double* segmentCenterPosition = d->SegmentationNode->GetSegmentCenterRAS(selectedSegmentIDs[0].toUtf8().constData());
  if (!segmentCenterPosition)
    {
    return;
    }

  qCjyxLayoutManager* layoutManager = qCjyxApplication::application()->layoutManager();
  if (!layoutManager)
    {
    // application is closing
    return;
    }
  foreach(QString sliceViewName, layoutManager->sliceViewNames())
    {
    // Check if segmentation is visible in this view
    qDMMLSliceWidget* sliceWidget = layoutManager->sliceWidget(sliceViewName);
    vtkDMMLSliceNode* sliceNode = sliceWidget->dmmlSliceNode();
    if (!sliceNode || !sliceNode->GetID())
      {
      continue;
      }
    bool visibleInView = false;
    int numberOfDisplayNodes = d->SegmentationNode->GetNumberOfDisplayNodes();
    for (int displayNodeIndex = 0; displayNodeIndex < numberOfDisplayNodes; displayNodeIndex++)
      {
      vtkDMMLDisplayNode* segmentationDisplayNode = d->SegmentationNode->GetNthDisplayNode(displayNodeIndex);
      if (segmentationDisplayNode && segmentationDisplayNode->IsDisplayableInView(sliceNode->GetID()))
        {
        visibleInView = true;
        break;
        }
      }
    if (!visibleInView)
      {
      continue;
      }
    sliceNode->JumpSliceByCentering(segmentCenterPosition[0], segmentCenterPosition[1], segmentCenterPosition[2]);
    // snap to IJK to make sure slice is not positioned at the boundary of two voxels
    vtkCjyxApplicationLogic* appLogic = qCjyxApplication::application()->applicationLogic();
    if (appLogic)
      {
      vtkDMMLSliceLogic* sliceLogic = appLogic->GetSliceLogic(sliceNode);
      if (sliceLogic)
        {
        sliceLogic->SnapSliceOffsetToIJK();
        }
      }
    }
}

//------------------------------------------------------------------------------
void qDMMLSegmentsTableView::moveSelectedSegmentsUp()
{
  QStringList selectedSegmentIDs = this->selectedSegmentIDs();
  if (selectedSegmentIDs.size() == 0)
    {
    qWarning() << Q_FUNC_INFO << ": No segment selected";
    return;
    }

  Q_D(qDMMLSegmentsTableView);
  if (!d->SegmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": No current segmentation node";
    return;
    }
  vtkSegmentation* segmentation = d->SegmentationNode->GetSegmentation();

  QModelIndexList segmentModelIndices;
  QList<int> selectedRows;
  foreach (QString segmentID, selectedSegmentIDs)
    {
    QModelIndex index = d->SortFilterModel->indexFromSegmentID(segmentID);
    segmentModelIndices << index;
    selectedRows << index.row();
    }
  int minIndex = *(std::min_element(selectedRows.begin(), selectedRows.end()));
  if (minIndex == 0)
    {
    qDebug() << Q_FUNC_INFO << ": Cannot move top segment up";
    return;
    }

  for (int i = 0; i < selectedSegmentIDs.size(); ++i)
    {
    QModelIndex selectedModelIndex = segmentModelIndices[i];
    QModelIndex previousModelIndex = d->SortFilterModel->index(selectedModelIndex.row() - 1, 0);
    QString previousSegmentID = d->SortFilterModel->segmentIDFromIndex(previousModelIndex);
    int previousSegmentIndex = segmentation->GetSegmentIndex(previousSegmentID.toStdString());
    segmentation->SetSegmentIndex(selectedSegmentIDs[i].toUtf8().constData(), previousSegmentIndex);
    }
  this->setSelectedSegmentIDs(selectedSegmentIDs);
}

//------------------------------------------------------------------------------
void qDMMLSegmentsTableView::moveSelectedSegmentsDown()
{
  QStringList selectedSegmentIDs = this->selectedSegmentIDs();
  if (selectedSegmentIDs.size() == 0)
    {
    qWarning() << Q_FUNC_INFO << ": No segment selected";
    return;
    }

  Q_D(qDMMLSegmentsTableView);
  if (!d->SegmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": No current segmentation node";
    return;
    }
  vtkSegmentation* segmentation = d->SegmentationNode->GetSegmentation();

  QModelIndexList segmentModelIndices;
  QList<int> selectedRows;
  foreach(QString segmentID, selectedSegmentIDs)
    {
    QModelIndex index = d->SortFilterModel->indexFromSegmentID(segmentID);
    segmentModelIndices << index;
    selectedRows << index.row();
    }
  int maxIndex = *(std::max_element(selectedRows.begin(), selectedRows.end()));
  if (maxIndex == d->SortFilterModel->rowCount() - 1)
    {
    qDebug() << Q_FUNC_INFO << ": Cannot move bottom segment down";
    return;
    }

  for (int i = selectedSegmentIDs.count() - 1; i >= 0; --i)
    {
    QModelIndex selectedModelIndex = segmentModelIndices[i];
    QModelIndex nextModelIndex = d->SortFilterModel->index(selectedModelIndex.row() + 1, 0);
    QString nextSegmentID = d->SortFilterModel->segmentIDFromIndex(nextModelIndex);
    int nextSegmentIndex = segmentation->GetSegmentIndex(nextSegmentID.toStdString());
    segmentation->SetSegmentIndex(selectedSegmentIDs[i].toUtf8().constData(), nextSegmentIndex);
    }
  this->setSelectedSegmentIDs(selectedSegmentIDs);
}

// --------------------------------------------------------------------------
QString qDMMLSegmentsTableView::terminologyTooltipForSegment(vtkSegment* segment)
{
  return qDMMLSegmentsModel::terminologyTooltipForSegment(segment);
}

// --------------------------------------------------------------------------
void qDMMLSegmentsTableView::setHideSegments(const QStringList& segmentIDs)
{
  Q_D(qDMMLSegmentsTableView);
  d->SortFilterModel->setHideSegments(segmentIDs);
}

// --------------------------------------------------------------------------
QStringList qDMMLSegmentsTableView::hideSegments()const
{
  Q_D(const qDMMLSegmentsTableView);
  return d->SortFilterModel->hideSegments();
}

// --------------------------------------------------------------------------
QStringList qDMMLSegmentsTableView::displayedSegmentIDs()const
{
  Q_D(const qDMMLSegmentsTableView);

  QStringList displayedSegmentIDs;
  for (int row = 0; row < d->SortFilterModel->rowCount(); ++row)
    {
    displayedSegmentIDs << d->SortFilterModel->segmentIDFromIndex(d->SortFilterModel->index(row, 0));
    }
  return displayedSegmentIDs;
}

// --------------------------------------------------------------------------
void qDMMLSegmentsTableView::setJumpToSelectedSegmentEnabled(bool enable)
{
  Q_D(qDMMLSegmentsTableView);
  d->JumpToSelectedSegmentEnabled = enable;
}

// --------------------------------------------------------------------------
bool qDMMLSegmentsTableView::jumpToSelectedSegmentEnabled()const
{
  Q_D(const qDMMLSegmentsTableView);
  return d->JumpToSelectedSegmentEnabled;
}

// --------------------------------------------------------------------------
void qDMMLSegmentsTableView::modelAboutToBeReset()
{
  Q_D(qDMMLSegmentsTableView);
  d->WasBlockingTableSignalsBeforeReset = d->SegmentsTable->blockSignals(true);
  d->SelectedSegmentIDsBeforeReset = this->selectedSegmentIDs();
}

// --------------------------------------------------------------------------
void qDMMLSegmentsTableView::modelReset()
{
  Q_D(qDMMLSegmentsTableView);
  d->SegmentsTable->blockSignals(d->WasBlockingTableSignalsBeforeReset);
  this->setSelectedSegmentIDs(d->SelectedSegmentIDsBeforeReset);
  d->SelectedSegmentIDsBeforeReset.clear();
}
