/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QButtonGroup>
#include <QClipboard>
#include <QDebug>
#include <QInputDialog>
#include <QList>
#include <QMenu>
#include <QMessageBox>
#include <QModelIndex>
#include <QMouseEvent>
#include <QSettings>
#include <QShortcut>
#include <QSignalMapper>
#include <QStringList>
#include <QTableWidgetItem>
#include <QSharedPointer>
#include <QSpinBox>

// CjyxQt includes
#include <qCjyxCoreApplication.h>
#include <qCjyxModuleManager.h>
#include <qCjyxAbstractCoreModule.h>

// CTK includes
#include "ctkMessageBox.h"

// Cjyx includes
#include "qDMMLNodeFactory.h"
#include "qDMMLSceneModel.h"
#include "qDMMLSortFilterSubjectHierarchyProxyModel.h"
#include "qDMMLSubjectHierarchyModel.h"
#include "qDMMLUtils.h"
#include "qCjyxApplication.h"

// DMML includes
#include "vtkDMMLColorLegendDisplayNode.h"
#include "vtkDMMLInteractionNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSelectionNode.h"
#include "vtkDMMLSliceLogic.h"
#include "vtkDMMLSliceNode.h"
#include "vtkDMMLSubjectHierarchyNode.h"
#include "vtkDMMLTableNode.h"

// DMMLDM includes
#include "vtkDMMLMarkupsDisplayableManager.h"
#include "vtkDMMLMarkupsDisplayableManagerHelper.h"

// Module logic includes
#include <vtkCjyxColorLogic.h>

// Markups includes
#include "qDMMLMarkupsAbstractOptionsWidget.h"
#include "qDMMLMarkupsOptionsWidgetsFactory.h"
#include "qCjyxMarkupsModule.h"
#include "qCjyxMarkupsModuleWidget.h"
#include "ui_qCjyxMarkupsModule.h"
#include "vtkDMMLMarkupsCurveNode.h"
#include "vtkDMMLMarkupsDisplayNode.h"
#include "vtkDMMLMarkupsFiducialStorageNode.h"
#include "vtkDMMLMarkupsNode.h"
#include "vtkCjyxMarkupsLogic.h"
#include "qDMMLMarkupsToolBar.h"

// VTK includes
#include <vtkMath.h>
#include <vtkNew.h>
#include "vtkPoints.h"
#include <math.h>

static const int JUMP_MODE_COMBOBOX_INDEX_IGNORE = 0;
static const int JUMP_MODE_COMBOBOX_INDEX_OFFSET = 1;
static const int JUMP_MODE_COMBOBOX_INDEX_CENTERED = 2;
static const char* NAME_PROPERTY = "name";

static const int COORDINATE_COMBOBOX_INDEX_WORLD = 0;
static const int COORDINATE_COMBOBOX_INDEX_LOCAL = 1;
static const int COORDINATE_COMBOBOX_INDEX_HIDE = 2;


//extern qCjyxMarkupsOptionsWidgetsFactory* qCjyxMarkupsOptionsWidgetsFactory::Instance = nullptr;
//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_Markups
class qCjyxMarkupsModuleWidgetPrivate: public Ui_qCjyxMarkupsModule
{
  Q_DECLARE_PUBLIC(qCjyxMarkupsModuleWidget);

protected:
  qCjyxMarkupsModuleWidget* const q_ptr;

public:
  qCjyxMarkupsModuleWidgetPrivate(qCjyxMarkupsModuleWidget& object);
  ~qCjyxMarkupsModuleWidgetPrivate();

  void setupUi(qCjyxWidget* widget);

  qDMMLMarkupsToolBar* toolBar();

  /// the number of columns matches the column labels by using the size of the QStringList
  int numberOfColumns();

  vtkDMMLSelectionNode* selectionNode();

  vtkDMMLNode* selectionNodeActivePlaceNode();
  void setSelectionNodeActivePlaceNode(vtkDMMLNode* activePlaceNode);
  void setDMMLMarkupsNodeFromSelectionNode();

  void setPlaceModeEnabled(bool placeEnable);
  bool getPersistanceModeEnabled();
  vtkDMMLMarkupsDisplayNode* markupsDisplayNode();

  // update the markups creation buttons.
  void createMarkupsPushButtons();

  // update the list of markups widgets from qDMMLMarkupsOptionsWidgetsFactory
  void updateMarkupsOptionsWidgets();

  // place the markups options widgets.
  void placeMarkupsOptionsWidgets();

  enum ControlPointColumnsIds
    {
    SelectedColumn = 0,
    LockedColumn,
    VisibleColumn,
    NameColumn,
    DescriptionColumn,
    RColumn,
    AColumn,
    SColumn,
    PositionColumn
    };

private:
  vtkWeakPointer<vtkDMMLMarkupsNode> MarkupsNode;

  QStringList columnLabels;

  QAction*    newMarkupWithCurrentDisplayPropertiesAction;

  QMenu*      visibilityMenu;
  QAction*    visibilityOnAllControlPointsInListAction;
  QAction*    visibilityOffAllControlPointsInListAction;

  QMenu*      selectedMenu;
  QAction*    selectedOnAllControlPointsInListAction;
  QAction*    selectedOffAllControlPointsInListAction;

  QMenu*      lockMenu;
  QAction*    lockAllControlPointsInListAction;
  QAction*    unlockAllControlPointsInListAction;

  QAction*    cutAction;
  QAction*    copyAction;
  QAction*    pasteAction;

  QPixmap     CjyxLockIcon;
  QPixmap     CjyxUnlockIcon;
  QPixmap     CjyxVisibleIcon;
  QPixmap     CjyxInvisibleIcon;

  // Dynamic list of create markups push buttons
  QList<QPushButton*> ceateMarkupsPushButtons;
  unsigned int createMarkupsButtonsColumns;

  // Export/import section
  QButtonGroup* ImportExportOperationButtonGroup;
  QButtonGroup* ImportExportCoordinateSystemButtonGroup;

  // Markups options widgets
  QList<qDMMLMarkupsAbstractOptionsWidget*> MarkupsOptionsWidgets;
};

//-----------------------------------------------------------------------------
// qCjyxMarkupsModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qCjyxMarkupsModuleWidgetPrivate::qCjyxMarkupsModuleWidgetPrivate(qCjyxMarkupsModuleWidget& object)
  : q_ptr(&object)
{
  Q_Q(qCjyxMarkupsModuleWidget);

  this->columnLabels << q->tr("Selected") << q->tr("Locked") << q->tr("Visible")
    << q->tr("Name") << q->tr("Description") << q->tr("R") << q->tr("A") << q->tr("S") << q->tr("Position");

  this->newMarkupWithCurrentDisplayPropertiesAction = nullptr;
  this->visibilityMenu = nullptr;
  this->visibilityOnAllControlPointsInListAction = nullptr;
  this->visibilityOffAllControlPointsInListAction = nullptr;

  this->selectedMenu = nullptr;
  this->selectedOnAllControlPointsInListAction = nullptr;
  this->selectedOffAllControlPointsInListAction = nullptr;

  this->lockMenu = nullptr;
  this->lockAllControlPointsInListAction = nullptr;
  this->unlockAllControlPointsInListAction = nullptr;

  this->cutAction = nullptr;
  this->copyAction = nullptr;
  this->pasteAction = nullptr;

  this->CjyxLockIcon = QPixmap(":/Icons/Small/CjyxLock.png");
  this->CjyxUnlockIcon = QPixmap(":/Icons/Small/CjyxUnlock.png");
  this->CjyxVisibleIcon = QPixmap(":/Icons/Small/CjyxVisible.png");
  this->CjyxInvisibleIcon = QPixmap(":/Icons/Small/CjyxInvisible.png");

  this->createMarkupsButtonsColumns = 0;

  this->ImportExportOperationButtonGroup = nullptr;
  this->ImportExportCoordinateSystemButtonGroup = nullptr;

  this->updateMarkupsOptionsWidgets();
}

//-----------------------------------------------------------------------------
qCjyxMarkupsModuleWidgetPrivate::~qCjyxMarkupsModuleWidgetPrivate() = default;

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidgetPrivate::setupUi(qCjyxWidget* widget)
{
  Q_Q(qCjyxMarkupsModuleWidget);
  this->Ui_qCjyxMarkupsModule::setupUi(widget);

  QStringList registeredMarkups;
  for(const auto& name: q->markupsLogic()->GetRegisteredMarkupsTypes())
    {
    vtkDMMLMarkupsNode* markupsNode = q->markupsLogic()->GetNodeByMarkupsType(name.c_str());
    if (markupsNode)
      {
      registeredMarkups << markupsNode->GetClassName();
      }
    }

  this->activeMarkupTreeView->setNodeTypes(registeredMarkups);
  this->activeMarkupTreeView->setColumnHidden(this->activeMarkupTreeView->model()->idColumn(), true);
  this->activeMarkupTreeView->setColumnHidden(this->activeMarkupTreeView->model()->transformColumn(), true);
  this->activeMarkupTreeView->setColumnHidden(this->activeMarkupTreeView->model()->descriptionColumn(), false);

  // We need to disable the controlPointsCollapsibleButton here because doing so in the .ui file would lead to
  // "child widget is not accessible" warning in debug mode whenever a point is selected in the control point list.
  this->controlPointsCollapsibleButton->setEnabled(false);

  // set up the list buttons
  // visibility
  // first add actions to the menu, then hook them up
  visibilityMenu = new QMenu(qCjyxMarkupsModuleWidget::tr("Visibility"), this->visibilityAllControlPointsInListMenuButton);
  // visibility on
  this->visibilityOnAllControlPointsInListAction =
    new QAction(QIcon(":/Icons/Small/CjyxVisible.png"), "Visibility On", visibilityMenu);
  this->visibilityOnAllControlPointsInListAction->setToolTip("Set visibility flag to on for all control points in the active markup");
  this->visibilityOnAllControlPointsInListAction->setCheckable(false);
  QObject::connect(this->visibilityOnAllControlPointsInListAction, SIGNAL(triggered()),
                   q, SLOT(onVisibilityOnAllControlPointsInListPushButtonClicked()));

  // visibility off
  this->visibilityOffAllControlPointsInListAction =
    new QAction(QIcon(":/Icons/Small/CjyxInvisible.png"), "Visibility Off", visibilityMenu);
  this->visibilityOffAllControlPointsInListAction->setToolTip("Set visibility flag to off for all control points in the active markup");
  this->visibilityOffAllControlPointsInListAction->setCheckable(false);
  QObject::connect(this->visibilityOffAllControlPointsInListAction, SIGNAL(triggered()),
                   q, SLOT(onVisibilityOffAllControlPointsInListPushButtonClicked()));

  this->visibilityMenu->addAction(this->visibilityOnAllControlPointsInListAction);
  this->visibilityMenu->addAction(this->visibilityOffAllControlPointsInListAction);
  this->visibilityAllControlPointsInListMenuButton->setMenu(this->visibilityMenu);
  this->visibilityAllControlPointsInListMenuButton->setIcon(QIcon(":/Icons/VisibleOrInvisible.png"));

  // visibility toggle
  QObject::connect(this->visibilityAllControlPointsInListMenuButton, SIGNAL(clicked()),
                   q, SLOT(onVisibilityAllControlPointsInListToggled()));

  // lock
  // first add actions to the menu, then hook them up
  lockMenu = new QMenu(qCjyxMarkupsModuleWidget::tr("Lock"), this->lockAllControlPointsInListMenuButton);
  // lock
  this->lockAllControlPointsInListAction =
    new QAction(QIcon(":/Icons/Small/CjyxLock.png"), "Lock", lockMenu);
  this->lockAllControlPointsInListAction->setToolTip("Set lock flag to on for all control points in the active markup");
  this->lockAllControlPointsInListAction->setCheckable(false);
  QObject::connect(this->lockAllControlPointsInListAction, SIGNAL(triggered()),
                   q, SLOT(onLockAllControlPointsInListPushButtonClicked()));

  // lock off
  this->unlockAllControlPointsInListAction =
    new QAction(QIcon(":/Icons/Small/CjyxUnlock.png"), "Unlock", lockMenu);
  this->unlockAllControlPointsInListAction->setToolTip("Set lock flag to off for all control points in the active markup");
  this->unlockAllControlPointsInListAction->setCheckable(false);
  QObject::connect(this->unlockAllControlPointsInListAction, SIGNAL(triggered()),
                   q, SLOT(onUnlockAllControlPointsInListPushButtonClicked()));

  this->lockMenu->addAction(this->lockAllControlPointsInListAction);
  this->lockMenu->addAction(this->unlockAllControlPointsInListAction);
  this->lockAllControlPointsInListMenuButton->setMenu(this->lockMenu);
  this->lockAllControlPointsInListMenuButton->setIcon(QIcon(":/Icons/Small/CjyxLockUnlock.png"));

  // lock toggle
  QObject::connect(this->lockAllControlPointsInListMenuButton, SIGNAL(clicked()),
                   q, SLOT(onLockAllControlPointsInListToggled()));

  // selected
  // first add actions to the menu, then hook them up
  selectedMenu = new QMenu(qCjyxMarkupsModuleWidget::tr("Selected"), this->selectedAllControlPointsInListMenuButton);
  // selected on
  this->selectedOnAllControlPointsInListAction =
    new QAction(QIcon(":/Icons/MarkupsSelected.png"), "Selected On", selectedMenu);
  this->selectedOnAllControlPointsInListAction->setToolTip("Set selected flag to on for all control points in the active markup");
  this->selectedOnAllControlPointsInListAction->setCheckable(false);
  QObject::connect(this->selectedOnAllControlPointsInListAction, SIGNAL(triggered()),
                   q, SLOT(onSelectAllControlPointsInListPushButtonClicked()));

  // selected off
  this->selectedOffAllControlPointsInListAction =
    new QAction(QIcon(":/Icons/MarkupsUnselected.png"), "Selected Off", selectedMenu);
  this->selectedOffAllControlPointsInListAction->setToolTip("Set selected flag to off for all control points in the active markup");
  this->selectedOffAllControlPointsInListAction->setCheckable(false);
  QObject::connect(this->selectedOffAllControlPointsInListAction, SIGNAL(triggered()),
                   q, SLOT(onDeselectAllControlPointsInListPushButtonClicked()));

  this->selectedMenu->addAction(this->selectedOnAllControlPointsInListAction);
  this->selectedMenu->addAction(this->selectedOffAllControlPointsInListAction);
  this->selectedAllControlPointsInListMenuButton->setMenu(this->selectedMenu);
  this->selectedAllControlPointsInListMenuButton->setIcon(QIcon(":/Icons/MarkupsSelectedOrUnselected.png"));

  // selected toggle
  QObject::connect(this->selectedAllControlPointsInListMenuButton, SIGNAL(clicked()),
                   q, SLOT(onSelectedAllControlPointsInListToggled()));

  // add
  QObject::connect(this->addControlPointPushButton, SIGNAL(clicked()),
                   q, SLOT(onAddControlPointPushButtonClicked()));
  // move
  QObject::connect(this->moveControlPointUpPushButton, SIGNAL(clicked()),
                   q, SLOT(onMoveControlPointUpPushButtonClicked()));
  QObject::connect(this->moveControlPointDownPushButton, SIGNAL(clicked()),
                   q, SLOT(onMoveControlPointDownPushButtonClicked()));
  // position status
  QObject::connect(this->missingControlPointPushButton, SIGNAL(clicked()),
      q, SLOT(onMissingControlPointPushButtonClicked()));
  QObject::connect(this->unsetControlPointPushButton, SIGNAL(clicked()),
      q, SLOT(onUnsetControlPointPushButtonClicked()));
  // delete
  QObject::connect(this->deleteControlPointPushButton, SIGNAL(clicked()),
                   q, SLOT(onDeleteControlPointPushButtonClicked()));
  QObject::connect(this->deleteAllControlPointsInListPushButton, SIGNAL(clicked()),
      q, SLOT(onDeleteAllControlPointsInListPushButtonClicked()));

  this->cutAction = new QAction(q);
  this->cutAction->setText(qCjyxMarkupsModuleWidget::tr("Cut"));
  this->cutAction->setIcon(QIcon(":Icons/Medium/CjyxEditCut.png"));
  this->cutAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  this->cutAction->setShortcuts(QKeySequence::Cut);
  this->cutAction->setToolTip(qCjyxMarkupsModuleWidget::tr("Cut"));
  q->addAction(this->cutAction);
  this->CutControlPointsToolButton->setDefaultAction(this->cutAction);
  QObject::connect(this->cutAction, SIGNAL(triggered()), q, SLOT(cutSelectedToClipboard()));

  this->copyAction = new QAction(q);
  this->copyAction->setText(qCjyxMarkupsModuleWidget::tr("Copy"));
  this->copyAction->setIcon(QIcon(":Icons/Medium/CjyxEditCopy.png"));
  this->copyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  this->copyAction->setShortcuts(QKeySequence::Copy);
  this->copyAction->setToolTip(qCjyxMarkupsModuleWidget::tr("Copy"));
  q->addAction(this->copyAction);
  this->CopyControlPointsToolButton->setDefaultAction(this->copyAction);
  QObject::connect(this->copyAction, SIGNAL(triggered()), q, SLOT(copySelectedToClipboard()));

  this->pasteAction = new QAction(q);
  this->pasteAction->setText(qCjyxMarkupsModuleWidget::tr("Paste"));
  this->pasteAction->setIcon(QIcon(":Icons/Medium/CjyxEditPaste.png"));
  this->pasteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  this->pasteAction->setShortcuts(QKeySequence::Paste);
  this->pasteAction->setToolTip(qCjyxMarkupsModuleWidget::tr("Paste"));
  q->addAction(this->pasteAction);
  this->PasteControlPointsToolButton->setDefaultAction(this->pasteAction);
  QObject::connect(this->pasteAction, SIGNAL(triggered()), q, SLOT(pasteSelectedFromClipboard()));

  // set up the active markups node selector
  QObject::connect(this->activeMarkupTreeView, SIGNAL(currentItemChanged(vtkIdType)),
    q, SLOT(onActiveMarkupItemChanged(vtkIdType)));

  // Create the layout for the create markups group box.
  this->createMarkupsPushButtons();

  // update the checked state of showing the slice intersections
  // vtkDMMLApplicationLogic::GetIntersectingSlicesEnabled cannot be called, as the scene
  // is not yet set, so just set to the default value (slice intersections not visible).
  this->sliceIntersectionsVisibilityCheckBox->setChecked(false);
  QObject::connect(this->sliceIntersectionsVisibilityCheckBox,
                   SIGNAL(toggled(bool)),
                   q, SLOT(onSliceIntersectionsVisibilityToggled(bool)));

  //
  // set up the list visibility/locked buttons
  //
  QObject::connect(this->listLockedUnlockedPushButton, SIGNAL(clicked()),
                   q, SLOT(onListLockedUnlockedPushButtonClicked()));

  //
  // set up the control point number locked/unlocked button
  //
  QObject::connect(this->fixedNumberOfControlPointsPushButton, SIGNAL(clicked()),
    q, SLOT(onFixedNumberOfControlPointsPushButtonClicked()));

  //
  // set up the name format line edit
  //
  QObject::connect(this->nameFormatLineEdit, SIGNAL(textEdited(const QString &)),
                   q, SLOT(onNameFormatLineEditTextEdited(const QString &)));
  //
  // set up the reset format button
  //
  QObject::connect(this->resetNameFormatToDefaultPushButton, SIGNAL(clicked()),
                   q, SLOT(onResetNameFormatToDefaultPushButtonClicked()));
  //
  // set up the rename all button
  //
  QObject::connect(this->renameAllWithCurrentNameFormatPushButton, SIGNAL(clicked()),
                   q, SLOT(onRenameAllWithCurrentNameFormatPushButtonClicked()));
  //
  // set up the convert annotations button
  //
  QObject::connect(this->convertAnnotationFiducialsPushButton, SIGNAL(clicked()),
                   q, SLOT(convertAnnotationFiducialsToMarkups()));

  //
  // set up the table
  //

  // only select rows rather than cells
  this->activeMarkupTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  // allow multi select
  this->activeMarkupTableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

  // number of columns with headers
  this->activeMarkupTableWidget->setColumnCount(this->numberOfColumns());
  this->activeMarkupTableWidget->setHorizontalHeaderLabels(this->columnLabels);
  // We do not use QHeaderView::ResizeToContents, as it slows down table updates when there are many control points
  this->activeMarkupTableWidget->horizontalHeader()->setSectionResizeMode(qCjyxMarkupsModuleWidgetPrivate::NameColumn, QHeaderView::Stretch);
  this->activeMarkupTableWidget->horizontalHeader()->setStretchLastSection(false);

  // adjust the column widths
  this->activeMarkupTableWidget->setColumnWidth(qCjyxMarkupsModuleWidgetPrivate::NameColumn, 60);
  this->activeMarkupTableWidget->setColumnWidth(qCjyxMarkupsModuleWidgetPrivate::DescriptionColumn, 120);
  this->activeMarkupTableWidget->setColumnWidth(qCjyxMarkupsModuleWidgetPrivate::RColumn, 65);
  this->activeMarkupTableWidget->setColumnWidth(qCjyxMarkupsModuleWidgetPrivate::AColumn, 65);
  this->activeMarkupTableWidget->setColumnWidth(qCjyxMarkupsModuleWidgetPrivate::SColumn, 65);

  // show/hide the coordinate columns
  QObject::connect(this->coordinatesComboBox,
                   SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onHideCoordinateColumnsToggled(int)));

  // use an icon for some column headers
  // selected is a check box
  QTableWidgetItem *selectedHeader = this->activeMarkupTableWidget->horizontalHeaderItem(qCjyxMarkupsModuleWidgetPrivate::SelectedColumn);
  selectedHeader->setText("");
  selectedHeader->setIcon(QIcon(":/Icons/MarkupsSelected.png"));
  selectedHeader->setToolTip(QString("Click in this column to select/deselect control points for passing to CLI modules"));
  this->activeMarkupTableWidget->setColumnWidth(qCjyxMarkupsModuleWidgetPrivate::SelectedColumn, 30);
  // locked is an open and closed lock
  QTableWidgetItem *lockedHeader = this->activeMarkupTableWidget->horizontalHeaderItem(qCjyxMarkupsModuleWidgetPrivate::LockedColumn);
  lockedHeader->setText("");
  lockedHeader->setIcon(QIcon(":/Icons/Small/CjyxLockUnlock.png"));
  lockedHeader->setToolTip(QString("Click in this column to lock/unlock control points to prevent them from being moved by mistake"));
  this->activeMarkupTableWidget->setColumnWidth(qCjyxMarkupsModuleWidgetPrivate::LockedColumn, 30);
  // visible is an open and closed eye
  QTableWidgetItem *visibleHeader = this->activeMarkupTableWidget->horizontalHeaderItem(qCjyxMarkupsModuleWidgetPrivate::VisibleColumn);
  visibleHeader->setText("");
  visibleHeader->setIcon(QIcon(":/Icons/Small/CjyxVisibleInvisible.png"));
  visibleHeader->setToolTip(QString("Click in this column to show/hide control points in 2D and 3D"));
  this->activeMarkupTableWidget->setColumnWidth(qCjyxMarkupsModuleWidgetPrivate::VisibleColumn, 30);
  // position is a location bubble
  QTableWidgetItem *positionHeader = this->activeMarkupTableWidget->horizontalHeaderItem(qCjyxMarkupsModuleWidgetPrivate::PositionColumn);
  positionHeader->setText("");
  positionHeader->setIcon(QIcon(":/Icons/Large/MarkupsPositionStatus.png"));
  positionHeader->setToolTip(QString("Click in this column to modify the control point position state.\n\n"
                                     "- Edit: Enter place mode to modify the control point position in the slice views\n"
                                     "- Skip: 'Place multiple control points' mode skips over the control point entry\n"
                                     "- Restore: Set the control point position to its last known set position\n"
                                     "- Clear: Clear the defined control point position, but do not delete the control point"));
  this->activeMarkupTableWidget->setColumnWidth(qCjyxMarkupsModuleWidgetPrivate::PositionColumn, 10);

  // listen for changes so can update dmml node
  QObject::connect(this->activeMarkupTableWidget, SIGNAL(cellChanged(int, int)),
                   q, SLOT(onActiveMarkupTableCellChanged(int, int)));

  // listen for click on a markup
  QObject::connect(this->activeMarkupTableWidget, SIGNAL(itemClicked(QTableWidgetItem*)),
                   q, SLOT(onActiveMarkupTableCellClicked(QTableWidgetItem*)));
  // listen for the current cell changing (happens when arrows are used to navigate)
  QObject::connect(this->activeMarkupTableWidget, SIGNAL(currentCellChanged(int, int, int, int)),
                   q, SLOT(onActiveMarkupTableCurrentCellChanged(int, int, int, int)));
  // listen for a right click
  this->activeMarkupTableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  QObject::connect(this->activeMarkupTableWidget, SIGNAL(customContextMenuRequested(QPoint)),
                   q, SLOT(onRightClickActiveMarkupTableWidget(QPoint)));

  // set up the display properties buttons
  QObject::connect(this->resetToDefaultDisplayPropertiesPushButton, SIGNAL(clicked()),
    q, SLOT(onResetToDefaultDisplayPropertiesPushButtonClicked()));
  QObject::connect(this->saveToDefaultDisplayPropertiesPushButton, SIGNAL(clicked()),
    q, SLOT(onSaveToDefaultDisplayPropertiesPushButtonClicked()));

  // Place the options widgets
  this->placeMarkupsOptionsWidgets();
  QObject::connect(qDMMLMarkupsOptionsWidgetsFactory::instance(), SIGNAL(optionsWidgetRegistered()),
                   q, SLOT(onUpdateMarkupsOptionsWidgets()));
  QObject::connect(qDMMLMarkupsOptionsWidgetsFactory::instance(), SIGNAL(optionsWidgetUnregistered()),
                   q, SLOT(onUpdateMarkupsOptionsWidgets()));

  // hide measurement settings table until markups node containing measurement is set
  this->measurementSettingsTableWidget->setVisible(false);

  // Export/import
  this->ImportExportOperationButtonGroup = new QButtonGroup(this->exportImportCollapsibleButton);
  this->ImportExportOperationButtonGroup->addButton(this->tableExportRadioButton);
  this->ImportExportOperationButtonGroup->addButton(this->tableImportRadioButton);

  this->ImportExportCoordinateSystemButtonGroup = new QButtonGroup(this->exportImportCollapsibleButton);
  this->ImportExportCoordinateSystemButtonGroup->addButton(this->lpsExportRadioButton);
  this->ImportExportCoordinateSystemButtonGroup->addButton(this->rasExportRadioButton);

  this->tableExportRadioButton->setChecked(true);
  this->rasExportRadioButton->setChecked(true);

  QObject::connect(this->exportedImportedNodeComboBox, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
    q, SLOT(updateImportExportWidgets()));

  QObject::connect(this->ImportExportOperationButtonGroup, SIGNAL(buttonClicked(QAbstractButton*)),
    q, SLOT(updateImportExportWidgets()));

  QObject::connect(this->exportImportPushButton, SIGNAL(clicked()),
    q, SLOT(onImportExportApply()));

  q->updateImportExportWidgets();

  QObject::connect(this->ColorLegendCollapsibleGroupBox, SIGNAL(toggled(bool)),
    q, SLOT(onColorLegendCollapsibleGroupBoxToggled(bool)));

  // Add event observers for registration/unregistration of markups
  q->qvtkConnect(q->markupsLogic(), vtkCjyxMarkupsLogic::MarkupRegistered,
    q, SLOT(onCreateMarkupsPushButtons()));
  q->qvtkConnect(q->markupsLogic(), vtkCjyxMarkupsLogic::MarkupUnregistered,
    q, SLOT(onCreateMarkupsPushButtons()));
}

//-----------------------------------------------------------------------------
qDMMLMarkupsToolBar* qCjyxMarkupsModuleWidgetPrivate::toolBar()
{
  Q_Q(const qCjyxMarkupsModuleWidget);
  qCjyxMarkupsModule* module = dynamic_cast<qCjyxMarkupsModule*>(q->module());
  if (!module)
    {
    qWarning("qCjyxMarkupsModuleWidget::toolBar failed: module is not set");
    return nullptr;
    }
  return module->toolBar();
}

//-----------------------------------------------------------------------------
int qCjyxMarkupsModuleWidgetPrivate::numberOfColumns()
{
  return this->columnLabels.size();
}

//-----------------------------------------------------------------------------
vtkDMMLSelectionNode* qCjyxMarkupsModuleWidgetPrivate::selectionNode()
{
  Q_Q(qCjyxMarkupsModuleWidget);
  if (!q->dmmlScene() || !q->markupsLogic())
    {
    return nullptr;
    }
  vtkDMMLSelectionNode *selectionNode = vtkDMMLSelectionNode::SafeDownCast(
    q->dmmlScene()->GetNodeByID(q->markupsLogic()->GetSelectionNodeID().c_str()));
  return selectionNode;
}

//-----------------------------------------------------------------------------
vtkDMMLMarkupsDisplayNode* qCjyxMarkupsModuleWidgetPrivate::markupsDisplayNode()
{
  Q_Q(qCjyxMarkupsModuleWidget);
  if (!this->MarkupsNode)
    {
    return nullptr;
    }
  return vtkDMMLMarkupsDisplayNode::SafeDownCast(this->MarkupsNode->GetDisplayNode());
}

//-----------------------------------------------------------------------------
vtkDMMLNode* qCjyxMarkupsModuleWidgetPrivate::selectionNodeActivePlaceNode()
{
  Q_Q(qCjyxMarkupsModuleWidget);
  vtkDMMLSelectionNode *selNode = this->selectionNode();
  if (!selNode)
    {
    return nullptr;
    }

  const char *selectionNodeActivePlaceNodeID = selNode->GetActivePlaceNodeID();
  if (!selectionNodeActivePlaceNodeID)
    {
    return nullptr;
    }
  vtkDMMLNode* activePlaceNode = q->dmmlScene()->GetNodeByID(selectionNodeActivePlaceNodeID);
  return activePlaceNode;
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidgetPrivate::setSelectionNodeActivePlaceNode(vtkDMMLNode* node)
{
  Q_Q(qCjyxMarkupsModuleWidget);
  if (!q->markupsLogic())
    {
    return;
    }
  vtkDMMLMarkupsNode* activePlaceNode = vtkDMMLMarkupsNode::SafeDownCast(node);
  q->markupsLogic()->SetActiveList(activePlaceNode);
  q->updateToolBar(activePlaceNode);
  q->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidgetPrivate::setDMMLMarkupsNodeFromSelectionNode()
{
  Q_Q(qCjyxMarkupsModuleWidget);

  // Select current markups node
  vtkDMMLMarkupsNode* currentMarkupsNode = vtkDMMLMarkupsNode::SafeDownCast(this->selectionNodeActivePlaceNode());

  if (!currentMarkupsNode && q->dmmlScene() && this->activeMarkupTreeView->subjectHierarchyNode())
    {
    // Active place node is not a markups node then switch to the last markups node.
    vtkCollection* nodes = q->dmmlScene()->GetNodes();
    vtkDMMLSubjectHierarchyNode* shNode = this->activeMarkupTreeView->subjectHierarchyNode();
    for (int nodeIndex = nodes->GetNumberOfItems() - 1; nodeIndex >= 0; nodeIndex--)
    {
      vtkDMMLMarkupsNode* markupsNode = vtkDMMLMarkupsNode::SafeDownCast(nodes->GetItemAsObject(nodeIndex));
      if (!markupsNode)
        {
        continue;
        }
      vtkIdType itemID = shNode->GetItemByDataNode(markupsNode);
      if (!itemID)
        {
        continue;
        }
      QModelIndex itemIndex = this->activeMarkupTreeView->sortFilterProxyModel()->indexFromSubjectHierarchyItem(itemID);
      if (!itemIndex.isValid())
        {
        // not visible in current view
        continue;
        }
      currentMarkupsNode = vtkDMMLMarkupsNode::SafeDownCast(markupsNode);
      }
    if (currentMarkupsNode)
      {
      this->setSelectionNodeActivePlaceNode(currentMarkupsNode);
      }
    }
  q->setDMMLMarkupsNode(currentMarkupsNode);
  q->updateToolBar(currentMarkupsNode);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidgetPrivate::setPlaceModeEnabled(bool placeEnable)
{
  Q_Q(qCjyxMarkupsModuleWidget);
  vtkDMMLInteractionNode* interactionNode = nullptr;
  if (q->dmmlScene())
    {
    interactionNode = vtkDMMLInteractionNode::SafeDownCast(q->dmmlScene()->GetNodeByID("vtkDMMLInteractionNodeSingleton"));
    }
  if (interactionNode == nullptr)
    {
    if (placeEnable)
      {
      qCritical() << Q_FUNC_INFO << " setPlaceModeEnabled failed: invalid interaction node";
      }
    return;
    }

  if (placeEnable)
    {
    interactionNode->SetCurrentInteractionMode(vtkDMMLInteractionNode::Place);
    }
  else
    {
    if (interactionNode->GetCurrentInteractionMode() == vtkDMMLInteractionNode::Place)
      {
      interactionNode->SetCurrentInteractionMode(vtkDMMLInteractionNode::ViewTransform);
      }
    }
}

//-----------------------------------------------------------
bool qCjyxMarkupsModuleWidgetPrivate::getPersistanceModeEnabled()
{
  Q_Q(qCjyxMarkupsModuleWidget);
  vtkDMMLInteractionNode* interactionNode = nullptr;
  if (q->dmmlScene())
    {
    interactionNode = vtkDMMLInteractionNode::SafeDownCast(q->dmmlScene()->GetNodeByID("vtkDMMLInteractionNodeSingleton"));
    }
  if (interactionNode && interactionNode->GetPlaceModePersistence())
    {
    return true;
    }
  else
    {
    return false;
    }
}

//-----------------------------------------------------------
void qCjyxMarkupsModuleWidgetPrivate::updateMarkupsOptionsWidgets()
{
  foreach(auto widget, this->MarkupsOptionsWidgets)
    {
    widget->setParent(nullptr);
    }

  this->MarkupsOptionsWidgets.clear();

  // Create the markups options widgets registered in qDMMLMarkupsOptionsWidgetsFactory.
  auto factory = qDMMLMarkupsOptionsWidgetsFactory::instance();
  foreach(const auto& widgetClassName, factory->registeredOptionsWidgetsClassNames())
    {
    this->MarkupsOptionsWidgets.append(factory->createWidget(widgetClassName));
    }
}

//-----------------------------------------------------------
void qCjyxMarkupsModuleWidgetPrivate::placeMarkupsOptionsWidgets()
{
  Q_Q(qCjyxMarkupsModuleWidget);

  // Add the options widgets
  foreach(const auto& widget, this->MarkupsOptionsWidgets)
    {
    // If the parent is different from the qCjyxMarkupsModule widget, then add the widget.
    if (widget->parentWidget() != q)
      {
      this->markupsModuleVerticalLayout->addWidget(widget);
      widget->setVisible(false);
      }

      // Forward the dmmlSceneChanged signal
      QObject::connect(q, SIGNAL(dmmlSceneChanged(vtkDMMLScene*)),
                       widget, SLOT(setDMMLScene(vtkDMMLScene*)));
    }
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidgetPrivate::createMarkupsPushButtons()
{
  Q_Q(qCjyxMarkupsModuleWidget);

  QGridLayout *layout= new QGridLayout();

  vtkDMMLApplicationLogic* appLogic = q->appLogic();
  if (!appLogic)
    {
      qCritical() << Q_FUNC_INFO << "createMarkupsPushButtons: invalid application logic.";
      return;
    }

  vtkCjyxMarkupsLogic* markupsLogic =
    vtkCjyxMarkupsLogic::SafeDownCast(appLogic->GetModuleLogic("Markups"));
  if (!markupsLogic)
    {
    qCritical() << Q_FUNC_INFO << "createMarkupsPushButtons: invalid application logic.";
    return;
    }

  unsigned int i=0;

  for(const auto markupName: q->markupsLogic()->GetRegisteredMarkupsTypes())
    {
    vtkDMMLMarkupsNode* markupsNode =
      markupsLogic->GetNodeByMarkupsType(markupName.c_str());

    // Create markups add buttons.
    if (markupsNode && q->markupsLogic()->GetCreateMarkupsPushButton(markupName.c_str()))
      {
      QSignalMapper* mapper = new QSignalMapper(q);
      QPushButton *markupCreatePushButton = new QPushButton();
      //NOTE: We assign object name so we can test the dynamic creation of buttons in the tests.
      markupCreatePushButton->setObjectName(QString("Create") +
                                            QString(markupsNode->GetMarkupType()) +
                                            QString("PushButton"));
      markupCreatePushButton->setIcon(QIcon(markupsNode->GetPlaceAddIcon()));
      markupCreatePushButton->setToolTip(QString("Create ") +
                                         QString(markupsNode->GetTypeDisplayName()));
      markupCreatePushButton->setText(QString(markupsNode->GetTypeDisplayName()));
      layout->addWidget(markupCreatePushButton,
                        i / this->createMarkupsButtonsColumns,
                        i % this->createMarkupsButtonsColumns);

      QObject::connect(markupCreatePushButton, SIGNAL(clicked()), mapper, SLOT(map()));
      mapper->setMapping(markupCreatePushButton, markupsNode->GetClassName());
      QObject::connect(mapper, SIGNAL(mapped(const QString&)),
                       q, SLOT(onCreateMarkupByClass(const QString&)));

      // NOTE: Alternative connection using lambdas instead of QSignalMapper
      // QObject::connect(markupCreatePushButton, &QPushButton::clicked,
      //                  q, [q, markupsNode] {q->onCreateMarkupByClass(markupsNode->GetClassName());});

      ++i;
      }
    }

  // NOTE: this is a temporary widget to reparent the former layout (so it will get destroyed)
  if (createMarkupsGroupBox->layout())
    {
    QWidget tempWidget;
    tempWidget.setLayout(createMarkupsGroupBox->layout());
    }

  this->createMarkupsGroupBox->setLayout(layout);
}

//-----------------------------------------------------------------------------
// qCjyxMarkupsModuleWidget methods

//-----------------------------------------------------------------------------
qCjyxMarkupsModuleWidget::qCjyxMarkupsModuleWidget(QWidget* _parent)
  : Superclass( _parent )
    , d_ptr( new qCjyxMarkupsModuleWidgetPrivate(*this) )
{
  this->volumeSpacingScaleFactor = 10.0;
}

//-----------------------------------------------------------------------------
qCjyxMarkupsModuleWidget::~qCjyxMarkupsModuleWidget()
{
  Q_D(qCjyxMarkupsModuleWidget);
  this->setDMMLMarkupsNode(nullptr);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::setup()
{
  Q_D(qCjyxMarkupsModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::enter()
{
  Q_D(qCjyxMarkupsModuleWidget);

  this->Superclass::enter();

  this->checkForAnnotationFiducialConversion();

  d->setDMMLMarkupsNodeFromSelectionNode();

  // set up dmml scene observations so that the GUI gets updated
  this->qvtkConnect(this->dmmlScene(), vtkDMMLScene::NodeAddedEvent,
                    this, SLOT(onNodeAddedEvent(vtkObject*, vtkObject*)));
  this->qvtkConnect(this->dmmlScene(), vtkDMMLScene::NodeRemovedEvent,
                    this, SLOT(onNodeRemovedEvent(vtkObject*, vtkObject*)));
  this->qvtkConnect(this->dmmlScene(), vtkDMMLScene::EndImportEvent,
                    this, SLOT(onDMMLSceneEndImportEvent()));
  this->qvtkConnect(this->dmmlScene(), vtkDMMLScene::EndBatchProcessEvent,
                    this, SLOT(onDMMLSceneEndBatchProcessEvent()));
  this->qvtkConnect(this->dmmlScene(), vtkDMMLScene::EndCloseEvent,
                    this, SLOT(onDMMLSceneEndCloseEvent()));
  this->qvtkConnect(this->dmmlScene(), vtkDMMLScene::EndRestoreEvent,
                    this, SLOT(onDMMLSceneEndRestoreEvent()));

  vtkDMMLNode *selectionNode = d->selectionNode();
  if (selectionNode)
    {
    this->qvtkConnect(selectionNode, vtkDMMLSelectionNode::ActivePlaceNodeIDChangedEvent,
                      this, SLOT(onSelectionNodeActivePlaceNodeIDChanged()));
    }
  // Toolbar
  qDMMLMarkupsToolBar* toolBar = d->toolBar();
  if (toolBar)
    {
    connect(toolBar, SIGNAL(activeMarkupsNodeChanged(vtkDMMLNode*)), this, SLOT(onActiveMarkupDMMLNodeChanged(vtkDMMLNode*)));
    }

  // Add event observers to MarkupsNode
  if (d->MarkupsNode)
    {
    vtkDMMLMarkupsNode* markupsNode = d->MarkupsNode;
    d->MarkupsNode = nullptr; // this will force a reset
    this->setDMMLMarkupsNode(markupsNode);
    vtkIdType itemID = d->activeMarkupTreeView->subjectHierarchyNode()->GetItemByDataNode(markupsNode);
    QModelIndex itemIndex = d->activeMarkupTreeView->sortFilterProxyModel()->indexFromSubjectHierarchyItem(itemID);
    if(itemIndex.row()>=0)
      {
      d->activeMarkupTreeView->scrollTo(itemIndex);
      d->activeMarkupTreeView->setCurrentNode(markupsNode);
      }
    }

  // check the max scales against volume spacing, they might need to be updated
  this->updateMaximumScaleFromVolumes();
  this->enableMarkupTableButtons(d->MarkupsNode ? 1 : 0);

}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::checkForAnnotationFiducialConversion()
{
  // check to see if there are any annotation fiducial list nodes
  // and offer to import them as markups
  int numFids = this->dmmlScene()->GetNumberOfNodesByClass("vtkDMMLAnnotationFiducialNode");
  int numSceneViews = this->dmmlScene()->GetNumberOfNodesByClass("vtkDMMLSceneViewNode");
  if (numFids > 0)
    {
    ctkMessageBox convertMsgBox;
    convertMsgBox.setWindowTitle("Convert Annotation hierarchies to Markups point list nodes?");
    QString labelText = QString("Convert ")
      + QString::number(numFids)
      + QString(" Annotation fiducial lists to Markups point list nodes?")
      + QString(" Moves all Annotation fiducial lists out of hierarchies (deletes")
      + QString(" the nodes, but leaves the hierarchies in case rulers or")
      + QString(" ROIs are mixed in) and into Markups point list nodes.");
    if (numSceneViews > 0)
      {
      labelText += QString(" Iterates through ")
        + QString::number(numSceneViews)
        + QString(" Scene Views and converts any fiducial lists saved in those")
        + QString(" scenes into Markups point list nodes as well.");
      }
    // don't show again check box conflicts with informative text, so use
    // a long text
    convertMsgBox.setText(labelText);
    QPushButton *convertButton =
      convertMsgBox.addButton(tr("Convert"), QMessageBox::AcceptRole);
    convertMsgBox.addButton(QMessageBox::Cancel);
    convertMsgBox.setDefaultButton(convertButton);
    convertMsgBox.setDontShowAgainVisible(true);
    convertMsgBox.setDontShowAgainSettingsKey("Markups/AlwaysConvertAnnotationFiducials");
    convertMsgBox.exec();
    if (convertMsgBox.clickedButton() == convertButton)
      {
      this->convertAnnotationFiducialsToMarkups();
      }
    }
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::convertAnnotationFiducialsToMarkups()
{
  if (this->markupsLogic())
    {
    this->markupsLogic()->ConvertAnnotationFiducialsToMarkups();
    }
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::exit()
{
  this->Superclass::exit();

  // remove dmml scene observations, don't need to update the GUI while the
  // module is not showing
  this->qvtkDisconnectAll();

  // remove observations from measurements
  Q_D(qCjyxMarkupsModuleWidget);
  if (d->MarkupsNode)
    {
    for (int i=0; i<d->MarkupsNode->Measurements->GetNumberOfItems(); ++i)
      {
      vtkDMMLMeasurement* currentMeasurement = vtkDMMLMeasurement::SafeDownCast(
        d->MarkupsNode->Measurements->GetItemAsObject(i) );
      if (currentMeasurement)
        {
        qvtkDisconnect(currentMeasurement, vtkCommand::ModifiedEvent, this, SLOT(onMeasurementModified()));
        }
      }
    qvtkDisconnect(d->MarkupsNode->Measurements, vtkCommand::ModifiedEvent,
      this, SLOT(onMeasurementsCollectionModified()));
    }
}

//-----------------------------------------------------------------------------
vtkCjyxMarkupsLogic *qCjyxMarkupsModuleWidget::markupsLogic()
{
  if (this->logic() == nullptr)
    {
    return nullptr;
    }
  return vtkCjyxMarkupsLogic::SafeDownCast(this->logic());
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::updateWidgetFromDMML()
  {
  Q_D(qCjyxMarkupsModuleWidget);

  bool wasBlocked = d->activeMarkupTreeView->blockSignals(true);
  if (d->MarkupsNode)
    {
    vtkIdType itemID = d->activeMarkupTreeView->subjectHierarchyNode()->GetItemByDataNode(d->MarkupsNode);
    QModelIndex itemIndex = d->activeMarkupTreeView->sortFilterProxyModel()->indexFromSubjectHierarchyItem(itemID);
    if (itemIndex.row() >= 0)
      {
      d->activeMarkupTreeView->scrollTo(itemIndex);
      }
    }
  d->activeMarkupTreeView->setCurrentNode(d->MarkupsNode);
  d->activeMarkupTreeView->blockSignals(wasBlocked);
  d->markupsDisplayWidget->setDMMLMarkupsNode(d->MarkupsNode);

  // Color legend
  vtkDMMLColorLegendDisplayNode* colorLegendNode = nullptr;
  vtkDMMLDisplayNode* displayNode = d->markupsDisplayWidget->dmmlMarkupsDisplayNode();
  colorLegendNode = vtkCjyxColorLogic::GetColorLegendDisplayNode(displayNode);
  d->ColorLegendDisplayNodeWidget->setDMMLColorLegendDisplayNode(colorLegendNode);

  d->ColorLegendCollapsibleGroupBox->setCollapsed(!colorLegendNode);
  d->ColorLegendCollapsibleGroupBox->setEnabled(displayNode && displayNode->GetColorNode());

  if (!d->MarkupsNode)
    {
    d->activeMarkupTableWidget->clearContents();
    d->activeMarkupTableWidget->setRowCount(0);
    return;
    }

  if (d->MarkupsNode->GetLocked())
    {
    d->listLockedUnlockedPushButton->setIcon(QIcon(":Icons/Medium/CjyxLock.png"));
    d->listLockedUnlockedPushButton->setToolTip(QString("Click to unlock this control point list so points can be moved by the mouse"));
    }
  else
    {
    d->listLockedUnlockedPushButton->setIcon(QIcon(":Icons/Medium/CjyxUnlock.png"));
    d->listLockedUnlockedPushButton->setToolTip(QString("Click to lock this control point list so points cannot be moved by the mouse"));
    }

  if (d->MarkupsNode->GetFixedNumberOfControlPoints())
    {
    d->fixedNumberOfControlPointsPushButton->setIcon(QIcon(":Icons/Medium/CjyxPointNumberLock.png"));
    d->fixedNumberOfControlPointsPushButton->setToolTip(QString("Click to unlock the number of control points so points can be added or deleted"));
    d->deleteControlPointPushButton->setEnabled(false);
    d->deleteAllControlPointsInListPushButton->setEnabled(false);
    }
  else
    {
    d->fixedNumberOfControlPointsPushButton->setIcon(QIcon(":Icons/Medium/CjyxPointNumberUnlock.png"));
    d->fixedNumberOfControlPointsPushButton->setToolTip(QString("Click to lock the number of control points so no points can be added or deleted"));
    d->deleteControlPointPushButton->setEnabled(true);
    d->deleteAllControlPointsInListPushButton->setEnabled(true);
    }
  // update slice intersections
  d->sliceIntersectionsVisibilityCheckBox->setChecked(this->sliceIntersectionsVisible());

  // update the list name format
  QString nameFormat = QString(d->MarkupsNode->GetControlPointLabelFormat().c_str());
  d->nameFormatLineEdit->setText(nameFormat);

   // update the table
  int numberOfPoints = d->MarkupsNode->GetNumberOfControlPoints();
  if (d->activeMarkupTableWidget->rowCount() != numberOfPoints)
    {
    // force full update of the table
    // (after node change or batch update with multiple rows added or deleted)
    this->updateRows();
    }
  // Update options widgets
  foreach(const auto& widget, d->MarkupsOptionsWidgets)
    {
    widget->updateWidgetFromDMML();
    }
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::updateMaximumScaleFromVolumes()
{
  Q_D(qCjyxMarkupsModuleWidget);

  double maxSliceSpacing = 1.0;

  vtkDMMLSliceLogic *sliceLogic = nullptr;
  vtkDMMLApplicationLogic *dmmlAppLogic = this->logic()->GetDMMLApplicationLogic();
  if (!dmmlAppLogic)
    {
    return;
    }

  vtkDMMLNode *dmmlNode = this->dmmlScene()->GetNodeByID("vtkDMMLSliceNodeRed");
  if (!dmmlNode)
    {
    return;
    }
  vtkDMMLSliceNode *redSlice = vtkDMMLSliceNode::SafeDownCast(dmmlNode);
  if (!redSlice)
    {
    return;
    }
  sliceLogic = dmmlAppLogic->GetSliceLogic(redSlice);
  if (!sliceLogic)
    {
    return;
    }

  double *volumeSliceSpacing = sliceLogic->GetBackgroundSliceSpacing();
  if (volumeSliceSpacing != nullptr)
    {
    for (int i = 0; i < 3; ++i)
      {
      if (volumeSliceSpacing[i] > maxSliceSpacing)
        {
        maxSliceSpacing = volumeSliceSpacing[i];
        }
      }
    }
  double maxScale = maxSliceSpacing * this->volumeSpacingScaleFactor;
  // round it up to nearest multiple of 10
  maxScale = ceil(maxScale / 10.0) * 10.0;

  d->markupsDisplayWidget->setMaximumMarkupsScale(maxScale);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::updateRows()
{
  Q_D(qCjyxMarkupsModuleWidget);
  vtkDMMLMarkupsNode* markupsNode = this->dmmlMarkupsNode();
  if (!markupsNode)
    {
    return;
    }

  int numberOfPoints = d->MarkupsNode->GetNumberOfControlPoints();
  if (d->activeMarkupTableWidget->rowCount() != numberOfPoints)
    {
    d->activeMarkupTableWidget->setRowCount(numberOfPoints);
    }
  for (int m = 0; m < numberOfPoints; m++)
    {
    this->updateRow(m);
    }
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::updateRow(int controlPointIndex)
{
  Q_D(qCjyxMarkupsModuleWidget);
  vtkDMMLMarkupsNode* markupsNode = this->dmmlMarkupsNode();
  if (!markupsNode
    || controlPointIndex >= markupsNode->GetNumberOfControlPoints()) // markup point is already deleted (possible after batch update)
    {
    return;
    }

  if (controlPointIndex < 0)
    {
    qWarning() << Q_FUNC_INFO << " failed: invalid controlPointIndex";
    return;
    }

  // this is updating the qt widget from DMML, and should not trigger any updates on the node, so turn off events
  QSignalBlocker blocker(d->activeMarkupTableWidget);

  // selected
  int column = qCjyxMarkupsModuleWidgetPrivate::SelectedColumn;
  QTableWidgetItem* item = d->activeMarkupTableWidget->item(controlPointIndex, column);
  bool isNewItem = false;
  if (!item)
    {
    item = new QTableWidgetItem();
    // disable editing so that a double click won't bring up an entry box
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    isNewItem = true;
    }
  Qt::CheckState selectedState = (markupsNode->GetNthControlPointSelected(controlPointIndex) ? Qt::Checked : Qt::Unchecked);
  if (isNewItem || item->checkState() != selectedState)
    {
    item->setCheckState(selectedState);
    }
  if (isNewItem)
    {
    d->activeMarkupTableWidget->setItem(controlPointIndex, column, item);
    }

  // locked
  column = qCjyxMarkupsModuleWidgetPrivate::LockedColumn;
  item = d->activeMarkupTableWidget->item(controlPointIndex, column);
  isNewItem = false;
  if (!item)
    {
    item = new QTableWidgetItem();
    // disable checkable
    item->setData(Qt::CheckStateRole, QVariant());
    item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);
    // disable editing so that a double click won't bring up an entry box
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    isNewItem = true;
    }
  bool locked = markupsNode->GetNthControlPointLocked(controlPointIndex);
  if (isNewItem || item->data(Qt::UserRole).toBool() != locked)
    {
    item->setData(Qt::UserRole, QVariant(locked));
    item->setData(Qt::DecorationRole, locked ? d->CjyxLockIcon : d->CjyxUnlockIcon);
    }
  if (isNewItem)
    {
    d->activeMarkupTableWidget->setItem(controlPointIndex, column, item);
    }

  // visible
  column = qCjyxMarkupsModuleWidgetPrivate::VisibleColumn;
  item = d->activeMarkupTableWidget->item(controlPointIndex, column);
  isNewItem = false;
  if (!item)
    {
    item = new QTableWidgetItem();
    // disable checkable
    item->setData(Qt::CheckStateRole, QVariant());
    item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);
    // disable editing so that a double click won't bring up an entry box
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    isNewItem = true;
    }
  bool visible = markupsNode->GetNthControlPointVisibility(controlPointIndex);
  if (isNewItem || item->data(Qt::UserRole).toBool() != visible)
    {
    item->setData(Qt::UserRole, QVariant(visible));
    item->setData(Qt::DecorationRole, visible ? d->CjyxVisibleIcon : d->CjyxInvisibleIcon);
    }
  if (isNewItem)
    {
    d->activeMarkupTableWidget->setItem(controlPointIndex, column, item);
    }

  // name
  column = qCjyxMarkupsModuleWidgetPrivate::NameColumn;
  item = d->activeMarkupTableWidget->item(controlPointIndex, column);
  isNewItem = false;
  if (!item)
    {
    item = new QTableWidgetItem();
    isNewItem = true;
    }
  QString label = QString::fromStdString(markupsNode->GetNthControlPointLabel(controlPointIndex));
  if (isNewItem || item->text() != label)
    {
    item->setText(label);
    }
  if (isNewItem)
    {
    d->activeMarkupTableWidget->setItem(controlPointIndex, column, item);
    }

  // description
  column = qCjyxMarkupsModuleWidgetPrivate::DescriptionColumn;
  item = d->activeMarkupTableWidget->item(controlPointIndex, column);
  isNewItem = false;
  if (!item)
    {
    item = new QTableWidgetItem();
    isNewItem = true;
    }
  QString description = QString::fromStdString(markupsNode->GetNthControlPointDescription(controlPointIndex));
  if (isNewItem || item->text() != description)
    {
    item->setText(description);
    }
  if (isNewItem)
    {
    d->activeMarkupTableWidget->setItem(controlPointIndex, column, item);
    }

   // coordinates
   double point[3] = {0.0, 0.0, 0.0};
   if (d->coordinatesComboBox->currentIndex() == COORDINATE_COMBOBOX_INDEX_WORLD)
     {
     double worldPoint[4] = {0.0, 0.0, 0.0, 1.0};
     markupsNode->GetNthControlPointPositionWorld(controlPointIndex, worldPoint);
     for (int p = 0; p < 3; ++p)
       {
       point[p] = worldPoint[p];
       }
     }
   else
     {
     markupsNode->GetNthControlPointPosition(controlPointIndex, point);
     }
  int rColumnIndex = qCjyxMarkupsModuleWidgetPrivate::RColumn;
  int mPositionStatus = markupsNode->GetNthControlPointPositionStatus(controlPointIndex);
  bool showCoordinates = (mPositionStatus == vtkDMMLMarkupsNode::PositionDefined ||
    mPositionStatus == vtkDMMLMarkupsNode::PositionPreview);
  for (int p = 0; p < 3; p++)
    {
    column = rColumnIndex + p;
    item = d->activeMarkupTableWidget->item(controlPointIndex, column);
    isNewItem = false;
    if (!item)
      {
      item = new QTableWidgetItem();
      isNewItem = true;
      }
    QString coordinate;
    if (showCoordinates)
      {
      // last argument to number sets the precision
      coordinate = QString::number(point[p], 'f', 3);
      }
    if (isNewItem || item->text() != coordinate)
      {
      item->setText(coordinate);
      }
    if (isNewItem)
      {
      d->activeMarkupTableWidget->setItem(controlPointIndex, column, item);
      }
    }

  // position status
  column = qCjyxMarkupsModuleWidgetPrivate::PositionColumn;
  item = d->activeMarkupTableWidget->item(controlPointIndex, column);
  isNewItem = false;
  if (!item)
    {
    item = new QTableWidgetItem();
    // disable editing so that a double click won't bring up an entry box
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    isNewItem = true;
    }
  int positionStatus = markupsNode->GetNthControlPointPositionStatus(controlPointIndex);
  if (isNewItem
    || item->data(Qt::UserRole).toInt() != positionStatus)
    {
    item->setData(Qt::UserRole, positionStatus);
    switch (positionStatus)
      {
      case vtkDMMLMarkupsNode::PositionDefined: item->setData(Qt::DecorationRole, QPixmap(":/Icons/XSmall/MarkupsDefined.png")); break;
      case vtkDMMLMarkupsNode::PositionPreview: item->setData(Qt::DecorationRole, QPixmap(":/Icons/XSmall/MarkupsInProgress.png")); break;
      case vtkDMMLMarkupsNode::PositionMissing: item->setData(Qt::DecorationRole, QPixmap(":/Icons/XSmall/MarkupsMissing.png")); break;
      case vtkDMMLMarkupsNode::PositionUndefined: item->setData(Qt::DecorationRole, QPixmap(":/Icons/XSmall/MarkupsUndefined.png")); break;
      }
    }
  if (isNewItem)
    {
    d->activeMarkupTableWidget->setItem(controlPointIndex, column, item);
    }
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onNodeAddedEvent(vtkObject*, vtkObject* node)
{
  Q_D(qCjyxMarkupsModuleWidget);

  if (!this->dmmlScene() || this->dmmlScene()->IsBatchProcessing())
    {
    return;
    }

  vtkDMMLMarkupsNode* markupsNode = vtkDMMLMarkupsNode::SafeDownCast(node);
  if (!markupsNode)
    {
      return;
    }

  // make it active
  d->activeMarkupTreeView->setCurrentNode(markupsNode);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onNodeRemovedEvent(vtkObject* scene, vtkObject* node)
{
  Q_UNUSED(scene);
  Q_UNUSED(node);
  if (!this->dmmlScene() || this->dmmlScene()->IsBatchProcessing())
    {
    return;
    }
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onDMMLSceneEndImportEvent()
{
  this->checkForAnnotationFiducialConversion();
  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onDMMLSceneEndRestoreEvent()
{
  this->checkForAnnotationFiducialConversion();
  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onDMMLSceneEndBatchProcessEvent()
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!this->dmmlScene())
    {
    return;
    }
  this->checkForAnnotationFiducialConversion();
  d->setDMMLMarkupsNodeFromSelectionNode();
  // force update (clear GUI if no node is selected anymore)
  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onDMMLSceneEndCloseEvent()
{
  if (!this->dmmlScene() || this->dmmlScene()->IsBatchProcessing())
    {
    return;
    }
  this->setDMMLMarkupsNode(nullptr);
  // force update (clear GUI if no node is selected anymore)
  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onPKeyActivated()
{
  QPoint pos = QCursor::pos();

  // find out which widget it was over
  QWidget *widget = qCjyxApplication::application()->widgetAt(pos);

  // simulate a mouse press inside the widget
  QPoint widgetPos = widget->mapFromGlobal(pos);
  QMouseEvent click(QEvent::MouseButtonRelease, widgetPos, Qt::LeftButton, Qt::MouseButtons(), Qt::KeyboardModifiers());
  click.setAccepted(true);

  // and send it to the widget
  //qDebug() << "onPKeyActivated: sending event with pos " << widgetPos;
  QCoreApplication::sendEvent(widget, &click);
}


//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onVisibilityOnAllControlPointsInListPushButtonClicked()
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->SetAllControlPointsVisibility(d->MarkupsNode, true);
  d->MarkupsNode->SetDisplayVisibility(true);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onVisibilityOffAllControlPointsInListPushButtonClicked()
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->SetAllControlPointsVisibility(d->MarkupsNode, false);
  d->MarkupsNode->SetDisplayVisibility(false);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onVisibilityAllControlPointsInListToggled()
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->ToggleAllControlPointsVisibility(d->MarkupsNode);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onLockAllControlPointsInListPushButtonClicked()
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->SetAllControlPointsLocked(d->MarkupsNode, true);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onUnlockAllControlPointsInListPushButtonClicked()
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->SetAllControlPointsLocked(d->MarkupsNode, false);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onLockAllControlPointsInListToggled()
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->ToggleAllControlPointsLocked(d->MarkupsNode);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onSelectAllControlPointsInListPushButtonClicked()
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->SetAllControlPointsSelected(d->MarkupsNode, true);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onDeselectAllControlPointsInListPushButtonClicked()
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->SetAllControlPointsSelected(d->MarkupsNode, false);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onSelectedAllControlPointsInListToggled()
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->ToggleAllControlPointsSelected(d->MarkupsNode);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onAddControlPointPushButtonClicked()
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }
  // get the active node
  if ((d->MarkupsNode->GetNumberOfControlPoints() >= d->MarkupsNode->GetMaximumNumberOfControlPoints()) &&
       d->MarkupsNode->GetMaximumNumberOfControlPoints() >= 0)
    {
    return;
    }
  int index = d->MarkupsNode->AddControlPoint(vtkVector3d(0,0,0));
  d->MarkupsNode->UnsetNthControlPointPosition(index);
  d->setPlaceModeEnabled(false);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onMoveControlPointUpPushButtonClicked()
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  // get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, that only one is selected
  if ((selectedItems.size() / d->numberOfColumns()) != 1)
    {
    qDebug() << "Move up: only select one markup to move, current selected: " << selectedItems.size() << ", number of columns = " << d->numberOfColumns();
    return;
    }
  int thisIndex = selectedItems.at(0)->row();
  //qDebug() << "Swapping " << thisIndex << " and " << thisIndex - 1;
  d->MarkupsNode->SwapControlPoints(thisIndex, thisIndex - 1);
  // now make sure the new row is selected so a user can keep moving it up
  d->activeMarkupTableWidget->selectRow(thisIndex - 1);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onMoveControlPointDownPushButtonClicked()
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  // get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, that only one is selected
  if ((selectedItems.size() / d->numberOfColumns()) != 1)
    {
    return;
    }
  int thisIndex = selectedItems.at(0)->row();
  //qDebug() << "Swapping " << thisIndex << " and " << thisIndex + 1;
  d->MarkupsNode->SwapControlPoints(thisIndex, thisIndex + 1);
  // now make sure the new row is selected so a user can keep moving it down
  d->activeMarkupTableWidget->selectRow(thisIndex + 1);
}
//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onDeleteControlPointPushButtonClicked(bool confirm /*=true*/)
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  // get the selected rows
  QList<QTableWidgetItem*> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, check if nothing is selected
  if (selectedItems.isEmpty())
    {
    return;
    }

  // iterate over the selected items and save their row numbers (there are
  // selected indices for each column in a row, so jump by the number of
  // columns), so can delete without relying on the table
  QList<int> rows;
  for (int i = 0; i < selectedItems.size(); i += d->numberOfColumns())
    {
    // get the row
    int row = selectedItems.at(i)->row();
    // qDebug() << "Saving: i = " << i << ", row = " << row;
    rows << row;
    }
  // sort the list
  std::sort(rows.begin(), rows.end());
  if (confirm)
    {
    ctkMessageBox deleteAllMsgBox;
    deleteAllMsgBox.setWindowTitle("Delete control points in this list?");
    QString labelText = QString("Delete ")
      + QString::number(rows.size())
        + QString(" control points from this list?");
    // don't show again check box conflicts with informative text, so use
    // a long text
    deleteAllMsgBox.setText(labelText);

    QPushButton* deleteButton =
      deleteAllMsgBox.addButton(tr("Delete"), QMessageBox::AcceptRole);
      deleteAllMsgBox.addButton(QMessageBox::Cancel);
      deleteAllMsgBox.setDefaultButton(deleteButton);
      deleteAllMsgBox.setIcon(QMessageBox::Question);
      deleteAllMsgBox.setDontShowAgainVisible(true);
      deleteAllMsgBox.setDontShowAgainSettingsKey("Markups/AlwaysDeleteMarkups");
      deleteAllMsgBox.exec();
    if (deleteAllMsgBox.clickedButton() != deleteButton)
      {
      return;
      }
    }

  // delete from the end
  for (int i = rows.size() - 1; i >= 0; --i)
    {
    int index = rows.at(i);
    // qDebug() << "Deleting: i = " << i << ", index = " << index;
    d->MarkupsNode->RemoveNthControlPoint(index);
    }

  // clear the selection on the table
  d->activeMarkupTableWidget->clearSelection();
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onResetControlPointPushButtonClicked()
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  // get the selected rows
  QList<QTableWidgetItem*> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, check if nothing is selected
  if (selectedItems.isEmpty())
    {
    return;
    }

  // iterate over the selected items and save their row numbers (there are
  // selected indices for each column in a row, so jump by the number of
  // columns), so can delete without relying on the table
  QList<int> rows;
  for (int i = 0; i < selectedItems.size(); i += d->numberOfColumns())
    {
    // get the row
    int row = selectedItems.at(i)->row();
    rows << row;
    }
  // sort the list
  std::sort(rows.begin(), rows.end());

  // unplace from the end
  for (int i = rows.size() - 1; i >= 0; --i)
    {
    int index = rows.at(i);
    d->MarkupsNode->ResetNthControlPointPosition(index);
    d->setPlaceModeEnabled(true);
    }
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onRestoreControlPointPushButtonClicked()
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  // get the selected rows
  QList<QTableWidgetItem*> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, check if nothing is selected
  if (selectedItems.isEmpty())
    {
    return;
    }

  // iterate over the selected items and save their row numbers (there are
  // selected indices for each column in a row, so jump by the number of
  // columns), so can delete without relying on the table
  QList<int> rows;
  for (int i = 0; i < selectedItems.size(); i += d->numberOfColumns())
    {
    // get the row
    int row = selectedItems.at(i)->row();
    rows << row;
    }
  // sort the list
  std::sort(rows.begin(), rows.end());

  // unplace from the end
  for (int i = rows.size() - 1; i >= 0; --i)
    {
    int index = rows.at(i);
    d->MarkupsNode->RestoreNthControlPointPosition(index);
    }
}
//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onUnsetControlPointPushButtonClicked()
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  // get the selected rows
  QList<QTableWidgetItem*> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, check if nothing is selected
  if (selectedItems.isEmpty())
    {
    return;
    }

  // iterate over the selected items and save their row numbers (there are
  // selected indices for each column in a row, so jump by the number of
  // columns), so can delete without relying on the table
  QList<int> rows;
  for (int i = 0; i < selectedItems.size(); i += d->numberOfColumns())
    {
    // get the row
    int row = selectedItems.at(i)->row();
    rows << row;
    }
  // sort the list
  std::sort(rows.begin(), rows.end());

  // unplace from the end
  for (int i = rows.size() - 1; i >= 0; --i)
    {
    int index = rows.at(i);
    d->MarkupsNode->UnsetNthControlPointPosition(index);
    }
}
//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onMissingControlPointPushButtonClicked()
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  // get the selected rows
  QList<QTableWidgetItem*> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, check if nothing is selected
  if (selectedItems.isEmpty())
    {
    return;
    }

  // iterate over the selected items and save their row numbers (there are
  // selected indices for each column in a row, so jump by the number of
  // columns), so can delete without relying on the table
  QList<int> rows;
  for (int i = 0; i < selectedItems.size(); i += d->numberOfColumns())
    {
    // get the row
    int row = selectedItems.at(i)->row();
    rows << row;
    }
  // sort the list
  std::sort(rows.begin(), rows.end());

  // unplace from the end
  for (int i = rows.size() - 1; i >= 0; --i)
    {
    int index = rows.at(i);
    if (d->MarkupsNode->GetNthControlPointPositionStatus(index) == vtkDMMLMarkupsNode::PositionMissing)
      {
      d->MarkupsNode->UnsetNthControlPointPosition(index);
      }
    else
      {
      d->MarkupsNode->SetNthControlPointPositionMissing(index);
      }
  }
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onDeleteAllControlPointsInListPushButtonClicked()
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  ctkMessageBox deleteAllMsgBox;
  deleteAllMsgBox.setWindowTitle("Delete all control points in this list?");
  QString labelText = QString("Delete all ")
    + QString::number(d->MarkupsNode->GetNumberOfControlPoints())
    + QString(" control points in this list?");
  // don't show again check box conflicts with informative text, so use
  // a long text
  deleteAllMsgBox.setText(labelText);

  QPushButton *deleteButton =
    deleteAllMsgBox.addButton(tr("Delete All"), QMessageBox::AcceptRole);
  deleteAllMsgBox.addButton(QMessageBox::Cancel);
  deleteAllMsgBox.setDefaultButton(deleteButton);
  deleteAllMsgBox.setIcon(QMessageBox::Question);
  deleteAllMsgBox.setDontShowAgainVisible(true);
  deleteAllMsgBox.setDontShowAgainSettingsKey("Markups/AlwaysDeleteAllMarkups");
  deleteAllMsgBox.exec();
  if (deleteAllMsgBox.clickedButton() == deleteButton)
    {
    d->MarkupsNode->RemoveAllControlPoints();
    }
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onActiveMarkupItemChanged(vtkIdType)
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!this->isEntered())
    {
    // ignore any changes if the GUI is not shown
    return;
    }
  this->onActiveMarkupDMMLNodeChanged(d->activeMarkupTreeView->currentNode());
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onActiveMarkupDMMLNodeAdded(vtkDMMLNode * node)
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (this->markupsLogic())
    {
    this->markupsLogic()->AddNewDisplayNodeForMarkupsNode(node);
    }
  // make sure it's set up for the mouse mode tool bar to easily add points to
  // it by making it active in the selection node
  d->setSelectionNodeActivePlaceNode(node);
  d->setPlaceModeEnabled(true);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onActiveMarkupDMMLNodeChanged(vtkDMMLNode* node)
{
  Q_D(qCjyxMarkupsModuleWidget);

  if (!this->isEntered())
    {
    // ignore any changes if the GUI is not shown
    return;
    }

  vtkDMMLMarkupsNode* markupsNode = vtkDMMLMarkupsNode::SafeDownCast(node);

  // User changed the selected markup node.
  // We now make it the active place node in the scene.
  if (markupsNode)
    {
    d->setSelectionNodeActivePlaceNode(markupsNode);
    }
  this->enableMarkupTableButtons(markupsNode ? 1 : 0);
  this->setDMMLMarkupsNode(markupsNode);
}

//------------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onColorLegendCollapsibleGroupBoxToggled(bool toggled)
{
  Q_D(qCjyxMarkupsModuleWidget);

  // Make sure a legend display node exists if the color legend section is opened
  if (!toggled)
    {
    return;
    }

  vtkDMMLDisplayNode* displayNode = d->markupsDisplayWidget->dmmlMarkupsDisplayNode();
  vtkDMMLColorLegendDisplayNode* colorLegendNode = vtkCjyxColorLogic::GetColorLegendDisplayNode(displayNode);
  if (!colorLegendNode)
    {
    // color legend node does not exist, we need to create it now

    // Pause render to prevent the new Color legend displayed for a moment before it is hidden.
    vtkDMMLApplicationLogic* dmmlAppLogic = this->logic()->GetDMMLApplicationLogic();
    if (dmmlAppLogic)
      {
      dmmlAppLogic->PauseRender();
      }
    colorLegendNode = vtkCjyxColorLogic::AddDefaultColorLegendDisplayNode(displayNode);
    colorLegendNode->SetVisibility(false); // just because the groupbox is opened, don't show color legend yet
    if (dmmlAppLogic)
      {
      dmmlAppLogic->ResumeRender();
      }
    }
  d->ColorLegendDisplayNodeWidget->setDMMLColorLegendDisplayNode(colorLegendNode);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::enableMarkupTableButtons(bool enable)
{
  Q_D(qCjyxMarkupsModuleWidget);

  d->displayCollapsibleButton->setEnabled(enable);
  d->controlPointsCollapsibleButton->setEnabled(enable);
  d->measurementsCollapsibleButton->setEnabled(enable);
  d->exportImportCollapsibleButton->setEnabled(enable);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onCreateMarkupByClass(const QString& className)
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!this->markupsLogic())
    {
    qWarning() << Q_FUNC_INFO << " failed: invalid markups logic";
    return;
    }
  vtkDMMLMarkupsNode* markupsNode = this->markupsLogic()->AddNewMarkupsNode(className.toStdString());
  if (markupsNode)
    {
    this->onActiveMarkupDMMLNodeAdded(markupsNode);
    }
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onSelectionNodeActivePlaceNodeIDChanged()
{
  Q_D(qCjyxMarkupsModuleWidget);
  vtkDMMLMarkupsNode* markupsNode = vtkDMMLMarkupsNode::SafeDownCast(d->selectionNodeActivePlaceNode());
  this->setDMMLMarkupsNode(markupsNode);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onListVisibileInvisiblePushButtonClicked()
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  // toggle the visibility
  bool visibleFlag = d->MarkupsNode->GetDisplayVisibility();
  visibleFlag = !visibleFlag;
  d->MarkupsNode->SetDisplayVisibility(visibleFlag);

  if (this->markupsLogic())
    {
    this->markupsLogic()->SetAllControlPointsVisibility(d->MarkupsNode, visibleFlag);
    }
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onListLockedUnlockedPushButtonClicked()
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }
  bool locked = d->MarkupsNode->GetLocked();
  d->MarkupsNode->SetLocked(!locked);
  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onFixedNumberOfControlPointsPushButtonClicked()
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }
  d->MarkupsNode->SetFixedNumberOfControlPoints(!d->MarkupsNode->GetFixedNumberOfControlPoints());

// end point placement for locked node
  d->setPlaceModeEnabled(false);
  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onNameFormatLineEditTextEdited(const QString text)
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }
  d->MarkupsNode->SetControlPointLabelFormat(std::string(text.toUtf8()));
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onResetNameFormatToDefaultPushButtonClicked()
{
   Q_D(qCjyxMarkupsModuleWidget);
  if (!d->MarkupsNode || !this->dmmlScene())
    {
    return;
    }
   // make a new default markups node and use its value for the name format
  vtkSmartPointer<vtkDMMLMarkupsNode> defaultNode = vtkDMMLMarkupsNode::SafeDownCast(
    this->dmmlScene()->GetDefaultNodeByClass(d->MarkupsNode->GetClassName()));
  if (!defaultNode)
    {
    defaultNode = vtkSmartPointer<vtkDMMLMarkupsNode>::Take(vtkDMMLMarkupsNode::SafeDownCast(
      this->dmmlScene()->CreateNodeByClass(d->MarkupsNode->GetClassName())));
    }
  if (!defaultNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: invalid default markups node";
    }
  d->MarkupsNode->SetControlPointLabelFormat(defaultNode->GetControlPointLabelFormat());
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onRenameAllWithCurrentNameFormatPushButtonClicked()
{
   Q_D(qCjyxMarkupsModuleWidget);
   if (!d->MarkupsNode || !this->markupsLogic())
     {
     return;
     }
   this->markupsLogic()->RenameAllControlPointsFromCurrentFormat(d->MarkupsNode);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onActiveMarkupTableCellChanged(int row, int column)
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  // row corresponds to the index in the list
  int n = row;

  // now switch on the property
  QTableWidgetItem *item = d->activeMarkupTableWidget->item(row, column);
  if (!item)
    {
    qDebug() << QString("Unable to find item in table at ") + QString::number(row) + QString(", ") + QString::number(column);
    return;
    }
  if (column == qCjyxMarkupsModuleWidgetPrivate::SelectedColumn)
    {
    bool flag = (item->checkState() == Qt::Unchecked ? false : true);
    d->MarkupsNode->SetNthControlPointSelected(n, flag);
    }
  else if (column == qCjyxMarkupsModuleWidgetPrivate::LockedColumn)
    {
    bool flag = item->data(Qt::UserRole) == QVariant(true) ? true : false;
    // update the icon
    if (flag)
      {
      item->setData(Qt::DecorationRole, QPixmap(":/Icons/Small/CjyxLock.png"));
      }
    else
      {
      item->setData(Qt::DecorationRole, QPixmap(":/Icons/Small/CjyxUnlock.png"));
      }
    d->MarkupsNode->SetNthControlPointLocked(n, flag);
    }
  else if (column == qCjyxMarkupsModuleWidgetPrivate::VisibleColumn)
    {
    bool flag = item->data(Qt::UserRole) == QVariant(true) ? true : false;
    // update the eye icon
    if (flag)
      {
      item->setData(Qt::DecorationRole, QPixmap(":/Icons/Small/CjyxVisible.png"));
      }
    else
      {
      item->setData(Qt::DecorationRole, QPixmap(":/Icons/Small/CjyxInvisible.png"));
      }
    d->MarkupsNode->SetNthControlPointVisibility(n, flag);
    }
  else if (column == qCjyxMarkupsModuleWidgetPrivate::NameColumn)
    {
    std::string name = std::string(item->text().toUtf8());
    d->MarkupsNode->SetNthControlPointLabel(n, name);
    }
  else if (column ==  qCjyxMarkupsModuleWidgetPrivate::DescriptionColumn)
    {
    std::string description = std::string(item->text().toUtf8());
    d->MarkupsNode->SetNthControlPointDescription(n, description);
    }
  else if (column == qCjyxMarkupsModuleWidgetPrivate::RColumn ||
           column == qCjyxMarkupsModuleWidgetPrivate::AColumn ||
           column == qCjyxMarkupsModuleWidgetPrivate::SColumn)
    {
    // get the new value
    double newPoint[3] = {0.0, 0.0, 0.0};
    if (d->activeMarkupTableWidget->item(row, qCjyxMarkupsModuleWidgetPrivate::RColumn) == nullptr ||
        d->activeMarkupTableWidget->item(row, qCjyxMarkupsModuleWidgetPrivate::AColumn) == nullptr ||
        d->activeMarkupTableWidget->item(row, qCjyxMarkupsModuleWidgetPrivate::SColumn) == nullptr)
      {
      // init state, return
      return;
      }
    newPoint[0] = d->activeMarkupTableWidget->item(row, qCjyxMarkupsModuleWidgetPrivate::RColumn)->text().toDouble();
    newPoint[1] = d->activeMarkupTableWidget->item(row, qCjyxMarkupsModuleWidgetPrivate::AColumn)->text().toDouble();
    newPoint[2] = d->activeMarkupTableWidget->item(row, qCjyxMarkupsModuleWidgetPrivate::SColumn)->text().toDouble();

    // get the old value
    double point[3] = {0.0, 0.0, 0.0};
    if (d->coordinatesComboBox->currentIndex() == COORDINATE_COMBOBOX_INDEX_WORLD)
      {
      double worldPoint[4] = {0.0, 0.0, 0.0, 1.0};
      d->MarkupsNode->GetNthControlPointPositionWorld(n, worldPoint);
      for (int p = 0; p < 3; ++p)
        {
        point[p] = worldPoint[p];
        }
      }
    else
      {
      d->MarkupsNode->GetNthControlPointPosition(n, point);
      }

    // changed?
    double minChange = 0.001;
    if (fabs(newPoint[0] - point[0]) > minChange ||
        fabs(newPoint[1] - point[1]) > minChange ||
        fabs(newPoint[2] - point[2]) > minChange)
      {
      if (d->coordinatesComboBox->currentIndex() == COORDINATE_COMBOBOX_INDEX_WORLD)
        {
        d->MarkupsNode->SetNthControlPointPositionWorld(n, newPoint[0], newPoint[1], newPoint[2]);
        }
      else
        {
        d->MarkupsNode->SetNthControlPointPosition(n, newPoint);
        }
      }
    else
      {
      //qDebug() << QString("Cell changed: no change in location bigger than ") + QString::number(minChange);
      }
    }
  else if (column == qCjyxMarkupsModuleWidgetPrivate::PositionColumn)
    {
    bool persistenceModeEnabled = d->getPersistanceModeEnabled();
    if (item->data(Qt::UserRole) == QVariant(vtkDMMLMarkupsNode::PositionUndefined))
      {
      item->setData(Qt::DecorationRole, QPixmap(":/Icons/XSmall/MarkupsUndefined.png"));
      d->MarkupsNode->UnsetNthControlPointPosition(row);
      if (!persistenceModeEnabled)
        {
        d->setPlaceModeEnabled(false);
        }
      }
    else if (item->data(Qt::UserRole) == QVariant(vtkDMMLMarkupsNode::PositionPreview))
      {
      item->setData(Qt::DecorationRole, QPixmap(":/Icons/XSmall/MarkupsInProgress.png"));
      d->MarkupsNode->ResetNthControlPointPosition(row);
      d->setPlaceModeEnabled(true);
      }
    else if (item->data(Qt::UserRole) == QVariant(vtkDMMLMarkupsNode::PositionMissing))
      {
      item->setData(Qt::DecorationRole, QPixmap(":/Icons/XSmall/MarkupsMissing.png"));
      d->MarkupsNode->SetNthControlPointPositionMissing(row);
      if (!persistenceModeEnabled)
        {
        d->setPlaceModeEnabled(false);
        }
      }
    else if (item->data(Qt::UserRole) == QVariant(vtkDMMLMarkupsNode::PositionDefined))
      {
      item->setData(Qt::DecorationRole, QPixmap(":/Icons/XSmall/MarkupsDefined.png"));
      d->MarkupsNode->RestoreNthControlPointPosition(row);
      if (!persistenceModeEnabled)
        {
        d->setPlaceModeEnabled(false);
        }
      }
    }
  else
    {
    qDebug() << QString("Cell Changed: unknown column: ") + QString::number(column);
    }
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onActiveMarkupTableCellClicked(QTableWidgetItem* item)
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (item == nullptr)
    {
    return;
    }
  int column = item->column();
  int row = item->row();
  if (column == qCjyxMarkupsModuleWidgetPrivate::VisibleColumn ||
    column == qCjyxMarkupsModuleWidgetPrivate::LockedColumn)
    {
    // toggle the user role, the icon update is triggered by this change
    if (item->data(Qt::UserRole) == QVariant(false))
      {
      item->setData(Qt::UserRole, QVariant(true));
      }
    else
      {
      item->setData(Qt::UserRole, QVariant(false));
      }
    }
  else if (column == qCjyxMarkupsModuleWidgetPrivate::PositionColumn)
    {
      if (item->data(Qt::UserRole) == QVariant(vtkDMMLMarkupsNode::PositionDefined))
        {
        item->setData(Qt::UserRole, QVariant(vtkDMMLMarkupsNode::PositionUndefined));
        }
      else if (item->data(Qt::UserRole) == QVariant(vtkDMMLMarkupsNode::PositionUndefined))
        {
        item->setData(Qt::UserRole, QVariant(vtkDMMLMarkupsNode::PositionPreview));
        }
      else if (item->data(Qt::UserRole) == QVariant(vtkDMMLMarkupsNode::PositionPreview))
        {
        item->setData(Qt::UserRole, QVariant(vtkDMMLMarkupsNode::PositionMissing));
        }
      else if (item->data(Qt::UserRole) == QVariant(vtkDMMLMarkupsNode::PositionMissing))
        {
        item->setData(Qt::UserRole, QVariant(vtkDMMLMarkupsNode::PositionDefined));
        }
    }
  d->MarkupsNode->SetControlPointPlacementStartIndex(row);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onActiveMarkupTableCurrentCellChanged(
     int currentRow, int currentColumn, int previousRow, int previousColumn)
{
  Q_D(qCjyxMarkupsModuleWidget);
  Q_UNUSED(currentColumn);
  Q_UNUSED(previousRow);
  Q_UNUSED(previousColumn);
  if (!d->MarkupsNode || !this->markupsLogic())
    {
    return;
    }
  if ((d->jumpModeComboBox->currentIndex() == JUMP_MODE_COMBOBOX_INDEX_IGNORE))
    {
    // Jump slices is disabled
    return;
    }
  // Jump slices
  bool jumpCentered = (d->jumpModeComboBox->currentIndex() == JUMP_MODE_COMBOBOX_INDEX_CENTERED);
  this->markupsLogic()->JumpSlicesToNthPointInMarkup(d->MarkupsNode->GetID(), currentRow, jumpCentered);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onRightClickActiveMarkupTableWidget(QPoint pos)
{
  Q_D(qCjyxMarkupsModuleWidget);
  Q_UNUSED(pos);

  // qDebug() << "onRightClickActiveMarkupTableWidget: pos = " << pos;

  QMenu menu;

  // Delete
  QAction *deletePointAction =
    new QAction(QString("Delete highlighted control point(s)"), &menu);
  menu.addAction(deletePointAction);
  QObject::connect(deletePointAction, SIGNAL(triggered()),
                   this, SLOT(onDeleteControlPointPushButtonClicked()));

  // Jump slices
  QAction *jumpSlicesAction =
    new QAction(QString("Jump slices"), &menu);
  menu.addAction(jumpSlicesAction);
  QObject::connect(jumpSlicesAction, SIGNAL(triggered()),
                   this, SLOT(onJumpSlicesActionTriggered()));

  // Refocus 3D cameras
  QAction *refocusCamerasAction =
    new QAction(QString("Refocus all cameras"), &menu);
  menu.addAction(refocusCamerasAction);
  QObject::connect(refocusCamerasAction, SIGNAL(triggered()),
                   this, SLOT(onRefocusCamerasActionTriggered()));

  menu.addSeparator();
  // get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();
  if (!selectedItems.isEmpty())
    {
    menu.addAction(d->cutAction);
    menu.addAction(d->copyAction);
    }
  menu.addAction(d->pasteAction);

  menu.addSeparator();
  // Change position status
  QAction* resetPointAction =
      new QAction(QIcon(":/Icons/XSmall/MarkupsInProgress.png"), QString("Edit position of highlighted control point(s)"), &menu);
  menu.addAction(resetPointAction);
  QObject::connect(resetPointAction, SIGNAL(triggered()),
      this, SLOT(onResetControlPointPushButtonClicked()));

  QAction* missingPointAction =
      new QAction(QIcon(":/Icons/XSmall/MarkupsMissing.png"), QString("Skip placement of highlighted control point(s)"), &menu);
  menu.addAction(missingPointAction);
  QObject::connect(missingPointAction, SIGNAL(triggered()),
      this, SLOT(onMissingControlPointPushButtonClicked()));

  QAction* restorePointAction =
      new QAction(QIcon(":/Icons/XSmall/MarkupsDefined.png"), QString("Restore position of highlighted control point(s)"), &menu);
  menu.addAction(restorePointAction);
  QObject::connect(restorePointAction, SIGNAL(triggered()),
      this, SLOT(onRestoreControlPointPushButtonClicked()));

  QAction* unsetPointAction =
      new QAction(QIcon(":/Icons/XSmall/MarkupsUndefined.png"), QString("Clear position of highlighted control point(s)"), &menu);
  menu.addAction(unsetPointAction);
  QObject::connect(unsetPointAction, SIGNAL(triggered()),
      this, SLOT(onUnsetControlPointPushButtonClicked()));

  menu.addSeparator();
  this->addSelectedCoordinatesToMenu(&menu);

  menu.exec(QCursor::pos());
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::addSelectedCoordinatesToMenu(QMenu *menu)
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  // get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, check if nothing is selected
  if (selectedItems.isEmpty())
    {
    return;
    }

  // get the list of selected rows to sort them in index order
  QList<int> rows;
  // The selected items list contains an item for each column in each row that
  // has been selected. Don't make any assumptions about the order of the
  // selected items, iterate through all of them and collect unique rows
  for (int i = 0; i < selectedItems.size(); ++i)
    {
    // get the row
    int row = selectedItems.at(i)->row();
    if (!rows.contains(row))
      {
      rows << row;
      }
    }
  // sort the list
  std::sort(rows.begin(), rows.end());

  // keep track of point to point distance
  double distance = 0.0;
  double lastPoint[3] = {0.0, 0.0, 0.0};

  menu->addSeparator();

  // loop over the selected rows
  for (int i = 0; i < rows.size() ; i++)
    {
    int row = rows.at(i);
    // label this selected markup if more than one
    QString indexString;
    if (rows.size() > 1)
      {
      // if there's a label use it
      if (!(d->MarkupsNode->GetNthControlPointLabel(row).empty()))
        {
        indexString =  QString(d->MarkupsNode->GetNthControlPointLabel(row).c_str());
        }
      else
        {
        // use the row number as an index (row starts at 0, but GUI starts at 1)
        indexString = QString::number(row+1);
        }
      indexString +=  QString(" : ");
      }

    double point[3] = {0.0, 0.0, 0.0};
    if (d->coordinatesComboBox->currentIndex() == COORDINATE_COMBOBOX_INDEX_WORLD)
      {
      double worldPoint[4] = {0.0, 0.0, 0.0, 1.0};
      d->MarkupsNode->GetNthControlPointPositionWorld(row, worldPoint);
      for (int p = 0; p < 3; ++p)
        {
        point[p] = worldPoint[p];
        }
      }
    else
      {
      d->MarkupsNode->GetNthControlPointPosition(row, point);
      }
    // format the coordinates
    QString coordinate =
      QString::number(point[0]) + QString(",") +
      QString::number(point[1]) + QString(",") +
      QString::number(point[2]);
    QString menuString = indexString + coordinate;
    menu->addAction(menuString);

    // calculate the point to point accumulated distance for control points
    if (rows.size() > 1)
      {
      if (i > 0)
        {
        double distanceToLastPoint = vtkMath::Distance2BetweenPoints(lastPoint, point);
        if (distanceToLastPoint != 0.0)
          {
          distanceToLastPoint = sqrt(distanceToLastPoint);
          }
        distance += distanceToLastPoint;
        }
      lastPoint[0] = point[0];
      lastPoint[1] = point[1];
      lastPoint[2] = point[2];
      }
    }
  if (distance != 0.0)
    {
    menu->addAction(QString("Summed linear distance: %1").arg(distance));
    }
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onJumpSlicesActionTriggered()
{
 Q_D(qCjyxMarkupsModuleWidget);

  // get the active node
  if (!d->MarkupsNode)
    {
    return;
    }

  // get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, check if nothing is selected
  if (selectedItems.isEmpty())
    {
    return;
    }

  // offset or center?
  bool jumpCentered = false;
  if (d->jumpModeComboBox->currentIndex() == JUMP_MODE_COMBOBOX_INDEX_CENTERED)
    {
    jumpCentered = true;
    }

  // jump to it
  if (this->markupsLogic())
    {
    // use the first selected
    this->markupsLogic()->JumpSlicesToNthPointInMarkup(d->MarkupsNode->GetID(), selectedItems.at(0)->row(), jumpCentered);
    }
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onRefocusCamerasActionTriggered()
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }
  // get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();
  // first, check if nothing is selected
  if (selectedItems.isEmpty())
    {
    return;
    }
  // refocus on this point
  if (this->markupsLogic())
    {
    // use the first selected
    this->markupsLogic()->FocusCamerasOnNthPointInMarkup(d->MarkupsNode->GetID(), selectedItems.at(0)->row());
    }
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onCreateMarkupsPushButtons()
{
  Q_D(qCjyxMarkupsModuleWidget);

  d->createMarkupsPushButtons();
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onUpdateMarkupsOptionsWidgets()
{
  Q_D(qCjyxMarkupsModuleWidget);

  d->updateMarkupsOptionsWidgets();
  d->placeMarkupsOptionsWidgets();
}

//-----------------------------------------------------------------------------
QStringList qCjyxMarkupsModuleWidget::getOtherMarkupNames(vtkDMMLNode *thisMarkup)
{
  QStringList otherMarkups;

  // check for other markups nodes in the scene
  if (!this->dmmlScene())
    {
    return otherMarkups;
    }

  vtkCollection *col = this->dmmlScene()->GetNodesByClass(thisMarkup->GetClassName());
  int numNodes = col->GetNumberOfItems();
  if (numNodes < 2)
    {
    col->RemoveAllItems();
    col->Delete();
    return otherMarkups;
    }

  for (int n = 0; n < numNodes; n++)
    {
    vtkDMMLNode *markupsNodeN = vtkDMMLNode::SafeDownCast(col->GetItemAsObject(n));
    if (strcmp(markupsNodeN->GetID(), thisMarkup->GetID()) != 0)
      {
      otherMarkups.append(QString(markupsNodeN->GetName()));
      }
    }
  col->RemoveAllItems();
  col->Delete();

  return otherMarkups;
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::cutSelectedToClipboard()
{
  this->copySelectedToClipboard();
  this->onDeleteControlPointPushButtonClicked(false); // no confirmation message
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::copySelectedToClipboard()
{
  Q_D(qCjyxMarkupsModuleWidget);
  // get the active node
  if (!d->MarkupsNode)
    {
    qDebug() << Q_FUNC_INFO << ": no active list from which to cut";
    return;
    }

  // get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, check if nothing is selected
  if (selectedItems.isEmpty())
    {
    return;
    }

  // iterate over the selected items and save their row numbers (there are
  // selected indices for each column in a row, so jump by the number of
  // columns), so can delete without relying on the table
  QList<int> rows;
  for (int i = 0; i < selectedItems.size(); i += d->numberOfColumns())
    {
    // get the row
    int row = selectedItems.at(i)->row();
    // qDebug() << "Saving: i = " << i << ", row = " << row;
    rows << row;
    }
  // sort the list
  std::sort(rows.begin(), rows.end());

  vtkNew<vtkDMMLMarkupsFiducialStorageNode> storageNode;
  // Excel recognizes tab character as field separator,
  // therefore use that instead of comma.
  storageNode->SetFieldDelimiterCharacters("\t");

  QString markupsAsString;
  for (int i = 0; i < rows.size(); ++i)
    {
    int markupIndex = rows.at(i);
    markupsAsString += (storageNode->GetPointAsString(d->MarkupsNode, markupIndex).c_str() + QString("\n"));
    }

  QApplication::clipboard()->setText(markupsAsString);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::pasteSelectedFromClipboard()
{
  Q_D(qCjyxMarkupsModuleWidget);

  QString clipboardText = QApplication::clipboard()->text();
  QStringList lines = clipboardText.split("\n");
  if (lines.empty())
    {
    return;
    }

  if (!d->MarkupsNode)
    {
    // No point list is selected - create a new one
    // Assume a markups point list
    this->onCreateMarkupByClass("vtkDMMLMarkupsFiducialNode");
    if (!d->MarkupsNode)
      {
      return;
      }
    }

  vtkNew<vtkDMMLMarkupsFiducialStorageNode> storageNode;
  if (clipboardText.contains("\t"))
    {
    storageNode->SetFieldDelimiterCharacters("\t");
    }

  // SetPointFromString calls various events reporting the id of the point modified.
  // However, already for > 200 points, it gets bad performance. Therefore, we call a simply modified call at the end.
  DMMLNodeModifyBlocker blocker(d->MarkupsNode);
  foreach(QString line, lines)
    {
    line = line.trimmed();
    if (line.isEmpty() || line.startsWith('#'))
      {
      // empty line or comment line
      continue;
      }
    storageNode->SetPointFromString(d->MarkupsNode, d->MarkupsNode->GetNumberOfControlPoints(), line.toUtf8());
    }
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onActiveMarkupsNodeModifiedEvent()
{
  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::setDMMLMarkupsNode(vtkDMMLMarkupsNode* markupsNode)
{
  Q_D(qCjyxMarkupsModuleWidget);

  if (!this->dmmlScene())
    {
    d->MarkupsNode = nullptr;
    }
  if (markupsNode == d->MarkupsNode)
    {
    // no change
    return;
    }

  qvtkReconnect(d->MarkupsNode, markupsNode, vtkCommand::ModifiedEvent,
    this, SLOT(onActiveMarkupsNodeModifiedEvent()));

  // fixed point number
  qvtkReconnect(d->MarkupsNode, markupsNode, vtkDMMLMarkupsNode::FixedNumberOfControlPointsModifiedEvent,
    this, SLOT(onActiveMarkupsNodeModifiedEvent()));

  // points
  qvtkReconnect(d->MarkupsNode, markupsNode, vtkDMMLMarkupsNode::PointModifiedEvent,
    this, SLOT(onActiveMarkupsNodePointModifiedEvent(vtkObject*, void*)));
  qvtkReconnect(d->MarkupsNode, markupsNode, vtkDMMLMarkupsNode::PointAddedEvent,
    this, SLOT(onActiveMarkupsNodePointAddedEvent()));
  qvtkReconnect(d->MarkupsNode, markupsNode, vtkDMMLMarkupsNode::PointRemovedEvent,
    this, SLOT(onActiveMarkupsNodePointRemovedEvent(vtkObject*, void*)));

  // display
  qvtkReconnect(d->MarkupsNode, markupsNode, vtkDMMLDisplayableNode::DisplayModifiedEvent,
    this, SLOT(onActiveMarkupsNodeDisplayModifiedEvent()));

  // transforms
  qvtkReconnect(d->MarkupsNode, markupsNode, vtkDMMLTransformableNode::TransformModifiedEvent,
    this, SLOT(onActiveMarkupsNodeTransformModifiedEvent()));

  // measurements
  if (d->MarkupsNode)
    {
    qvtkDisconnect(d->MarkupsNode->Measurements, vtkCommand::ModifiedEvent,
      this, SLOT(onMeasurementsCollectionModified()));
    }
  if (markupsNode)
    {
    qvtkConnect(markupsNode->Measurements, vtkCommand::ModifiedEvent,
      this, SLOT(onMeasurementsCollectionModified()));
    }

  // Setting the internal Markups node
  d->MarkupsNode = markupsNode;

  foreach(const auto& widget, d->MarkupsOptionsWidgets)
    {
    widget->setDMMLMarkupsNode(markupsNode);
    widget->setVisible(widget->canManageDMMLMarkupsNode(markupsNode));
    }

  this->observeMeasurementsInCurrentMarkupsNode();
  this->updateMeasurementsDescriptionLabel();
  this->populateMeasurementSettingsTable();

  // Force update of control point table
  d->activeMarkupTableWidget->setRowCount(0);

  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
vtkDMMLMarkupsNode* qCjyxMarkupsModuleWidget::dmmlMarkupsNode()
{
  Q_D(qCjyxMarkupsModuleWidget);
  vtkDMMLNode* node = d->activeMarkupTreeView->currentNode();
  if (!node)
    {
    return nullptr;
    }
  if (!this->dmmlScene())
    {
    return nullptr;
    }
 // make sure the node is still in the scene and convert to markups
 vtkDMMLMarkupsNode* markupsNode = vtkDMMLMarkupsNode::SafeDownCast(this->dmmlScene()->GetNodeByID(node->GetID()));
 return markupsNode;
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onActiveMarkupsNodePointModifiedEvent(vtkObject *caller, void *callData)
{
  Q_D(qCjyxMarkupsModuleWidget);

  // the call data should be the index n
  if (caller == nullptr)
    {
    return;
    }

  int* nPtr = reinterpret_cast<int*>(callData);
  int n = (nPtr ? *nPtr : -1);

  if (n>=0)
    {
    this->updateRow(n);
    // Only scroll to the point that is currently being placed if jump to slice is enabled
    // (this could be controlled by a separate flag, but there are already too many options on the GUI).
    if (d->jumpModeComboBox->currentIndex() != JUMP_MODE_COMBOBOX_INDEX_IGNORE)
      {
      vtkDMMLMarkupsNode* markupsNode = this->dmmlMarkupsNode();
      if (markupsNode)
        {
        int mPositionStatus = markupsNode->GetNthControlPointPositionStatus(n);
        if (mPositionStatus == vtkDMMLMarkupsNode::PositionPreview
          && d->activeMarkupTableWidget->currentRow() != n)
          {
          d->activeMarkupTableWidget->setCurrentCell(n, 0);
          }
        }
      }
    }
  else
    {
    this->updateRows();
    }
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onActiveMarkupsNodePointAddedEvent()
{
  Q_D(qCjyxMarkupsModuleWidget);

  int newRow = d->activeMarkupTableWidget->rowCount();
  d->activeMarkupTableWidget->insertRow(newRow);

  if (newRow >= 0)
    {
    this->updateRow(newRow);
    }
  else
    {
    this->updateRows();
    }

  // scroll to the new row only if jump slices is not selected
  // (if jump slices on click in table is selected, selecting the new
  // row before the point coordinates are updated will cause the slices
  // to jump to 0,0,0)
  if (d->jumpModeComboBox->currentIndex() == JUMP_MODE_COMBOBOX_INDEX_IGNORE)
    {
    d->activeMarkupTableWidget->setCurrentCell(newRow, 0);
    }

  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onActiveMarkupsNodePointRemovedEvent(vtkObject *caller, void *callData)
{
  Q_D(qCjyxMarkupsModuleWidget);

  if (caller == nullptr)
    {
    return;
    }

  // the call data should be the index n
  int *nPtr = reinterpret_cast<int*>(callData);
  int n = (nPtr ? *nPtr : -1);
  if (n >= 0)
    {
    d->activeMarkupTableWidget->removeRow(n);
    }
  else
    {
    // batch update finished
    // If points are only removed then number of rows will not match the number of control points,
    // which will trigger a full update in updateWidgetFromDMML.
    // If points are removed and added then onActiveMarkupsNodePointAddedEvent is called, which
    // performs the full update.
    this->updateWidgetFromDMML();
    }
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onActiveMarkupsNodeDisplayModifiedEvent()
{
  // update the display properties
  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onActiveMarkupsNodeTransformModifiedEvent()
{
  // update the transform check box label
  // update the coordinates in the table
  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onSliceIntersectionsVisibilityToggled(bool flag)
{
  if (!this->appLogic())
    {
    qWarning() << "Unable to get application logic";
    return;
    }
  return this->appLogic()->SetIntersectingSlicesEnabled(vtkDMMLApplicationLogic::IntersectingSlicesVisibility, flag);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onNewMarkupWithCurrentDisplayPropertiesTriggered()
{
  Q_D(qCjyxMarkupsModuleWidget);

  // get the active list
  if (!d->MarkupsNode)
    {
    // if there's no currently active markups list, trigger the default add
    // node
    this->onCreateMarkupByClass("vtkDMMLMarkupsFiducialNode");
    return;
    }

  // get the display node
  vtkDMMLDisplayNode *displayNode = d->MarkupsNode->GetDisplayNode();
  if (!displayNode)
    {
    qWarning() << Q_FUNC_INFO << " failed: Unable to get the display node on the markups node";
    return;
    }

  // create a new one
  vtkSmartPointer<vtkDMMLNode> newDisplayNode = vtkSmartPointer<vtkDMMLNode>::Take(
    this->dmmlScene()->CreateNodeByClass(displayNode->GetClassName()));
  // copy the old one
  if (!newDisplayNode)
    {
    qWarning() << Q_FUNC_INFO << " failed: error creating display node";
    return;
    }
  newDisplayNode->Copy(displayNode);

  // now create the new markups node
  const char *className = d->MarkupsNode->GetClassName();
  vtkSmartPointer<vtkDMMLMarkupsNode> newDMMLNode = vtkSmartPointer<vtkDMMLMarkupsNode>::Take(
    vtkDMMLMarkupsNode::SafeDownCast(this->dmmlScene()->CreateNodeByClass(className)));
  if (!newDMMLNode)
    {
    qWarning() << Q_FUNC_INFO << " failed: error creating markups node";
    return;
    }
  // copy the name and let them rename it
  newDMMLNode->SetName(d->MarkupsNode->GetName());

  /// add to the scene
  this->dmmlScene()->AddNode(newDisplayNode);
  this->dmmlScene()->AddNode(newDMMLNode);
  newDMMLNode->SetAndObserveDisplayNodeID(newDisplayNode->GetID());

  // set it active
  d->setSelectionNodeActivePlaceNode(newDMMLNode);
  this->setDMMLMarkupsNode(newDMMLNode);
  // let the user rename it
  d->activeMarkupTreeView->renameCurrentItem();
}

//-----------------------------------------------------------------------------
bool qCjyxMarkupsModuleWidget::sliceIntersectionsVisible()
{
  if (!this->appLogic())
    {
    qWarning() << "Unable to get application logic";
    return false;
    }
  return this->appLogic()->GetIntersectingSlicesEnabled(vtkDMMLApplicationLogic::IntersectingSlicesVisibility);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onHideCoordinateColumnsToggled(int index)
{
  Q_D(qCjyxMarkupsModuleWidget);
  bool hide = bool(index == COORDINATE_COMBOBOX_INDEX_HIDE);

    // back to default column widths
  d->activeMarkupTableWidget->setColumnHidden(qCjyxMarkupsModuleWidgetPrivate::RColumn, hide);
  d->activeMarkupTableWidget->setColumnHidden(qCjyxMarkupsModuleWidgetPrivate::AColumn, hide);
  d->activeMarkupTableWidget->setColumnHidden(qCjyxMarkupsModuleWidgetPrivate::SColumn, hide);

  if(hide)
    {
    d->activeMarkupTableWidget->setColumnWidth(qCjyxMarkupsModuleWidgetPrivate::NameColumn, 60);
    d->activeMarkupTableWidget->setColumnWidth(qCjyxMarkupsModuleWidgetPrivate::DescriptionColumn, 120);
    }
  else
    {
    // expand the name and description columns
    d->activeMarkupTableWidget->setColumnWidth(qCjyxMarkupsModuleWidgetPrivate::NameColumn, 120);
    d->activeMarkupTableWidget->setColumnWidth(qCjyxMarkupsModuleWidgetPrivate::DescriptionColumn, 240);
    }
  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onTransformedCoordinatesToggled(bool checked)
{
  Q_UNUSED(checked);

  // update the GUI
  // tbd: only update the coordinates
  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------
bool qCjyxMarkupsModuleWidget::setEditedNode(vtkDMMLNode* node,
                                              QString role /*=QString()*/,
                                              QString context /*=QString()*/)
{
  Q_D(qCjyxMarkupsModuleWidget);

  int controlPointIndex = -1; // <0 means control point index is not specified
  if (role == "ControlPointIndex")
    {
    bool ok = false;
    controlPointIndex = context.toInt(&ok);
    if (!ok)
      {
      controlPointIndex = -1;
      }
    }

  if (vtkDMMLMarkupsNode::SafeDownCast(node))
    {
    d->setSelectionNodeActivePlaceNode(node);
    if (controlPointIndex>=0)
      {
      d->activeMarkupTableWidget->setCurrentCell(controlPointIndex, 0);
      }
    return true;
    }

  if (vtkDMMLMarkupsDisplayNode::SafeDownCast(node))
    {
    vtkDMMLMarkupsDisplayNode* displayNode = vtkDMMLMarkupsDisplayNode::SafeDownCast(node);
    vtkDMMLMarkupsNode* displayableNode = vtkDMMLMarkupsNode::SafeDownCast(displayNode->GetDisplayableNode());
    if (!displayableNode)
      {
      return false;
      }
    if (controlPointIndex>=0)
      {
      d->activeMarkupTableWidget->setCurrentCell(controlPointIndex, 0);
      }
    return true;
    }

  return false;
}

//-----------------------------------------------------------
double qCjyxMarkupsModuleWidget::nodeEditable(vtkDMMLNode* node)
{
  if (node != nullptr && node->GetHideFromEditors())
    {
    // we only allow editing of visible nodes in this module
    return 0.0;
    }
  if (vtkDMMLMarkupsNode::SafeDownCast(node)
    || vtkDMMLMarkupsDisplayNode::SafeDownCast(node))
    {
    return 0.5;
    }
  else if (node->IsA("vtkDMMLAnnotationFiducialNode"))
    {
    // The module cannot directly edit this type of node but can convert it
    return 0.1;
    }
  else
    {
    return 0.0;
    }
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onResetToDefaultDisplayPropertiesPushButtonClicked()
{
  Q_D(qCjyxMarkupsModuleWidget);
  vtkDMMLMarkupsDisplayNode *displayNode = d->markupsDisplayNode();
  if (!displayNode)
    {
    return;
    }
  // set the display node from the logic defaults
  if (!this->markupsLogic())
    {
    return;
    }
  this->markupsLogic()->SetDisplayNodeToDefaults(displayNode);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onSaveToDefaultDisplayPropertiesPushButtonClicked()
{
  Q_D(qCjyxMarkupsModuleWidget);
  vtkDMMLMarkupsDisplayNode *displayNode = d->markupsDisplayNode();
  if (!displayNode)
    {
    return;
    }
  // set the display node from the logic defaults
  if (!this->markupsLogic())
    {
    return;
    }
  this->markupsLogic()->SetDisplayDefaultsFromNode(displayNode);

  // also save the settings permanently
  qCjyxMarkupsModule::writeDefaultMarkupsDisplaySettings(this->markupsLogic()->GetDefaultMarkupsDisplayNode());
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onMeasurementsCollectionModified()
{
  // Make sure all measurements are observed
  this->observeMeasurementsInCurrentMarkupsNode();

  // Reconstruct measurement settings table
  this->populateMeasurementSettingsTable();
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::observeMeasurementsInCurrentMarkupsNode()
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  for (int i=0; i<d->MarkupsNode->Measurements->GetNumberOfItems(); ++i)
    {
    vtkDMMLMeasurement* currentMeasurement = vtkDMMLMeasurement::SafeDownCast(
      d->MarkupsNode->Measurements->GetItemAsObject(i) );
    if (!currentMeasurement)
      {
      continue;
      }

    if (!qvtkIsConnected(currentMeasurement, vtkCommand::ModifiedEvent, this, SLOT(onMeasurementModified(vtkObject*))))
      {
      qvtkConnect(currentMeasurement, vtkCommand::ModifiedEvent, this, SLOT(onMeasurementModified(vtkObject*)));
      }
    }
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onMeasurementModified(vtkObject* caller)
{
  Q_D(qCjyxMarkupsModuleWidget);

  // Update measurements description label
  this->updateMeasurementsDescriptionLabel();

  // Update settings for modified measurement
  vtkDMMLMeasurement* measurement = vtkDMMLMeasurement::SafeDownCast(caller);
  if (measurement)
    {
    QString measurementName = QString::fromStdString(measurement->GetName());
    if (measurementName.isEmpty())
      {
      qWarning() << Q_FUNC_INFO << ": Cannot update settings UI for modified measurement because it has an empty name";
      }
    else
      {
      QList<QTableWidgetItem*> nameItemsFound = d->measurementSettingsTableWidget->findItems(measurementName, Qt::MatchExactly);
      foreach (QTableWidgetItem* nameItem, nameItemsFound)
        {
        QCheckBox* checkbox = qobject_cast<QCheckBox*>(d->measurementSettingsTableWidget->cellWidget(nameItem->row(), 1));
        checkbox->setChecked(measurement->GetEnabled());
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::updateMeasurementsDescriptionLabel()
{
  Q_D(qCjyxMarkupsModuleWidget);

  if (!d->MarkupsNode)
    {
    d->measurementsLabel->setText("No measurement");
    return;
    }

  QString measurementsString;
  for (int i=0; i<d->MarkupsNode->Measurements->GetNumberOfItems(); ++i)
    {
    vtkDMMLMeasurement* currentMeasurement = vtkDMMLMeasurement::SafeDownCast(
      d->MarkupsNode->Measurements->GetItemAsObject(i) );
    if (!currentMeasurement || !currentMeasurement->GetEnabled() || !currentMeasurement->GetValueDefined())
      {
      continue;
      }
    measurementsString.append(QString::fromStdString(currentMeasurement->GetName()));
    measurementsString.append(": ");
    if (currentMeasurement->GetLastComputationResult() == vtkDMMLMeasurement::OK)
      {
      measurementsString.append(currentMeasurement->GetValueWithUnitsAsPrintableString().c_str());
      }
    else
      {
      measurementsString.append(currentMeasurement->GetLastComputationResultAsPrintableString());
      }
    if (i != d->MarkupsNode->Measurements->GetNumberOfItems() - 1)
      {
      measurementsString.append("\n");
      }
    }
  d->measurementsLabel->setText(measurementsString.isEmpty() ? tr("No measurement") : measurementsString);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::populateMeasurementSettingsTable()
{
  Q_D(qCjyxMarkupsModuleWidget);

  d->measurementSettingsTableWidget->clear();
  d->measurementSettingsTableWidget->setVisible(
    d->MarkupsNode != nullptr && d->MarkupsNode->Measurements->GetNumberOfItems() > 0 );

  if (!d->MarkupsNode)
    {
    return;
    }

  d->measurementSettingsTableWidget->setHorizontalHeaderLabels(QStringList() << "Name" << "Enabled");
  d->measurementSettingsTableWidget->setRowCount(d->MarkupsNode->Measurements->GetNumberOfItems());
  for (int i=0; i<d->MarkupsNode->Measurements->GetNumberOfItems(); ++i)
    {
    vtkDMMLMeasurement* currentMeasurement = vtkDMMLMeasurement::SafeDownCast(
      d->MarkupsNode->Measurements->GetItemAsObject(i) );
    if (!currentMeasurement)
      {
      continue;
      }

    QTableWidgetItem* nameItem = new QTableWidgetItem(QString::fromStdString(currentMeasurement->GetName()));
    d->measurementSettingsTableWidget->setItem(i, 0, nameItem);

    QCheckBox* enabledCheckbox = new QCheckBox();
    enabledCheckbox->setChecked(currentMeasurement->GetEnabled());
    enabledCheckbox->setProperty(NAME_PROPERTY, QString::fromStdString(currentMeasurement->GetName()));
    QObject::connect(enabledCheckbox, SIGNAL(toggled(bool)), this, SLOT(onMeasurementEnabledCheckboxToggled(bool)));
    d->measurementSettingsTableWidget->setCellWidget(i, 1, enabledCheckbox);
    d->measurementSettingsTableWidget->setRowHeight(i, enabledCheckbox->sizeHint().height());
    }
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onMeasurementEnabledCheckboxToggled(bool on)
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  // Get measurement name from checkbox
  QCheckBox* checkbox = qobject_cast<QCheckBox*>(this->sender());
  QString measurementName = checkbox->property(NAME_PROPERTY).toString();

  // Enable/disable measurement with name
  for (int i=0; i<d->MarkupsNode->Measurements->GetNumberOfItems(); ++i)
    {
    vtkDMMLMeasurement* currentMeasurement = vtkDMMLMeasurement::SafeDownCast(
      d->MarkupsNode->Measurements->GetItemAsObject(i) );
    if (!currentMeasurement)
      {
      continue;
      }

    if (!measurementName.compare(QString::fromStdString(currentMeasurement->GetName())))
      {
      currentMeasurement->SetEnabled(on);
      }
    }
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::setCreateMarkupsButtonsColumns(unsigned int columns)
{
  Q_D(qCjyxMarkupsModuleWidget);
  d->createMarkupsButtonsColumns = columns;
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::updateImportExportWidgets()
{
  Q_D(qCjyxMarkupsModuleWidget);
  bool isExport = d->tableExportRadioButton->isChecked();
  if (isExport)
    {
    // Export
    d->exportImportTableLabel->setText("Output table:");
    d->exportImportPushButton->setText("Export");
    d->exportImportPushButton->setToolTip(tr("Export control points coordinates and properties to table."));
    }
  else
    {
    // Import
    d->exportImportTableLabel->setText("Input table:");
    d->exportImportPushButton->setText("Import");
    d->exportImportPushButton->setToolTip(
      tr("Import control points coordinates and properties from table node.\n"
      "Table column names : label, r, a, s, (or l, p, s), defined, selected, visible, locked, description."));
    }
  d->lpsExportRadioButton->setEnabled(isExport);
  d->rasExportRadioButton->setEnabled(isExport);
  d->exportImportPushButton->setEnabled(d->exportedImportedNodeComboBox->currentNode() != nullptr);
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::onImportExportApply()
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!this->markupsLogic())
    {
    return;
    }
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  if (d->tableExportRadioButton->isChecked())
    {
    // export
    int coordinateSystem = d->rasExportRadioButton->isChecked() ? vtkDMMLStorageNode::CoordinateSystemRAS : vtkDMMLStorageNode::CoordinateSystemLPS;
    this->markupsLogic()->ExportControlPointsToTable(d->MarkupsNode,
      vtkDMMLTableNode::SafeDownCast(d->exportedImportedNodeComboBox->currentNode()), coordinateSystem);
    }
  else
    {
    this->markupsLogic()->ImportControlPointsFromTable(d->MarkupsNode,
      vtkDMMLTableNode::SafeDownCast(d->exportedImportedNodeComboBox->currentNode()));
    }
  QApplication::restoreOverrideCursor();
}

// --------------------------------------------------------------------------
void qCjyxMarkupsModuleWidget::updateToolBar(vtkDMMLMarkupsNode* node)
{
  Q_D(qCjyxMarkupsModuleWidget);
  if (!this->dmmlScene() || this->dmmlScene()->IsBatchProcessing())
    {
    return;
    }
  qDMMLMarkupsToolBar* toolBar = d->toolBar();
  if (toolBar)
    {
    toolBar->setActiveMarkupsNode(node);
    }

}
