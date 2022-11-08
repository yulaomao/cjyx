#include "GUI/qCjyxSceneViewsModuleWidget.h"
#include "ui_qCjyxSceneViewsModuleWidget.h"

// CTK includes
#include "ctkMessageBox.h"
#include "ctkFittedTextBrowser.h"

// Qt includes
#include <QDebug>
#include <QLabel>
#include <QMainWindow>
#include <QPointer>
#include <QPushButton>
#include <QStatusBar>
#include <QToolButton>
#include <QVBoxLayout>

// DMML includes
#include "qDMMLUtils.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSceneViewNode.h"

// VTK includes
#include "vtkCollection.h"
#include "vtkImageData.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

// GUI includes
#include "GUI/qCjyxSceneViewsModuleDialog.h"
#include "qCjyxApplication.h"

enum
{
  SCENE_VIEW_THUMBNAIL_COLUMN = 0,
  SCENE_VIEW_DESCRIPTION_COLUMN,
  SCENE_VIEW_ACTIONS_COLUMN,
  // Add columns above this line
  SCENE_VIEW_NUMBER_OF_COLUMNS
};

static const char ROW_INDEX_PROPERTY[] = "RowIndex";

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_SceneViews
class qCjyxSceneViewsModuleWidgetPrivate: public Ui_qCjyxSceneViewsModuleWidget
{
  Q_DECLARE_PUBLIC(qCjyxSceneViewsModuleWidget);
protected:
  qCjyxSceneViewsModuleWidget* const q_ptr;
public:

  qCjyxSceneViewsModuleWidgetPrivate(qCjyxSceneViewsModuleWidget& object);
  ~qCjyxSceneViewsModuleWidgetPrivate();
  void setupUi(qCjyxWidget* widget);

  vtkCjyxSceneViewsModuleLogic* logic() const;
  qCjyxSceneViewsModuleDialog* sceneViewDialog();
  void updateTableRowFromSceneView(int row, vtkDMMLSceneViewNode *sceneView);

  QPointer<qCjyxSceneViewsModuleDialog> SceneViewDialog;
};

//-----------------------------------------------------------------------------
// qCjyxSceneViewsModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
vtkCjyxSceneViewsModuleLogic*
qCjyxSceneViewsModuleWidgetPrivate::logic() const
{
  Q_Q(const qCjyxSceneViewsModuleWidget);
  return vtkCjyxSceneViewsModuleLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
qCjyxSceneViewsModuleDialog* qCjyxSceneViewsModuleWidgetPrivate::sceneViewDialog()
{
  if (!this->SceneViewDialog)
    {
    qCjyxApplication* app = qCjyxApplication::application();
    QWidget* mainWindow = app ? app->mainWindow() : nullptr;
    this->SceneViewDialog = new qCjyxSceneViewsModuleDialog(mainWindow);

    // pass a pointer to the logic class
    this->SceneViewDialog->setLogic(this->logic());
    }
  return this->SceneViewDialog;
}

//-----------------------------------------------------------------------------
qCjyxSceneViewsModuleWidgetPrivate::qCjyxSceneViewsModuleWidgetPrivate(qCjyxSceneViewsModuleWidget& object)
  : q_ptr(&object)
{
  this->SceneViewDialog = nullptr;
}

//-----------------------------------------------------------------------------
qCjyxSceneViewsModuleWidgetPrivate::~qCjyxSceneViewsModuleWidgetPrivate()
{
  if (this->SceneViewDialog)
    {
    this->SceneViewDialog->close();
    delete this->SceneViewDialog.data();
    }
}

//-----------------------------------------------------------------------------
void qCjyxSceneViewsModuleWidgetPrivate::setupUi(qCjyxWidget* widget)
{
  Q_Q(qCjyxSceneViewsModuleWidget);
  this->Ui_qCjyxSceneViewsModuleWidget::setupUi(widget);

  this->SceneViewTableWidget->setColumnCount(SCENE_VIEW_NUMBER_OF_COLUMNS);
  this->SceneViewTableWidget->setHorizontalHeaderLabels(QStringList() << "Thumbnail" << "Description" << "Actions");
  this->SceneViewTableWidget->horizontalHeader()->hide();

  this->SceneViewTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  this->SceneViewTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  this->SceneViewTableWidget->horizontalHeader()->setSectionResizeMode(SCENE_VIEW_DESCRIPTION_COLUMN, QHeaderView::Stretch);

  // background of text browser widget is painted by the widget, and images has no background
  // either, so it is easier to just disable selection
  this->SceneViewTableWidget->setSelectionMode(QAbstractItemView::NoSelection);

  // listen for click on a markup
  QObject::connect(this->SceneViewTableWidget, SIGNAL(cellDoubleClicked(int, int)), q, SLOT(onSceneViewDoubleClicked(int, int)));
}

//-----------------------------------------------------------------------------
void qCjyxSceneViewsModuleWidgetPrivate::updateTableRowFromSceneView(int row, vtkDMMLSceneViewNode *sceneView)
{
  Q_Q(qCjyxSceneViewsModuleWidget);
  if (row >= this->SceneViewTableWidget->rowCount())
    {
    return;
    }

  // Thumbnail
  vtkImageData* thumbnailImage = sceneView->GetScreenShot();
  QLabel* thumbnailWidget = dynamic_cast<QLabel*>(this->SceneViewTableWidget->cellWidget(row, SCENE_VIEW_THUMBNAIL_COLUMN));
  if (thumbnailWidget == nullptr)
    {
    thumbnailWidget = new QLabel;
    this->SceneViewTableWidget->setCellWidget(row, SCENE_VIEW_THUMBNAIL_COLUMN, thumbnailWidget);
    }
  if (thumbnailImage)
    {
    QImage qimage;
    qDMMLUtils::vtkImageDataToQImage(thumbnailImage, qimage);
    thumbnailWidget->setPixmap(QPixmap::fromImage(qimage).scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
  else
    {
    thumbnailWidget->setPixmap(QPixmap(":/Icons/Extension.png"));
    }

  // Description
  QString name = sceneView->GetName();
  QString description = sceneView->GetSceneViewDescription().c_str();
  // replace any carriage returns with html line breaks
  description.replace(QString("\n"), QString("<br>"));
  ctkFittedTextBrowser* descriptionWidget = dynamic_cast<ctkFittedTextBrowser*>(this->SceneViewTableWidget->cellWidget(row, SCENE_VIEW_DESCRIPTION_COLUMN));
  if (descriptionWidget == nullptr)
    {
    descriptionWidget = new ctkFittedTextBrowser;
    descriptionWidget->setOpenExternalLinks(true);
    descriptionWidget->setAutoFillBackground(false);
    this->SceneViewTableWidget->setCellWidget(row, SCENE_VIEW_DESCRIPTION_COLUMN, descriptionWidget);
    }
  descriptionWidget->setHtml("<h3>" + name + "</h3>\n" + description);

  QFrame* actionsWidget = dynamic_cast<QFrame*>(this->SceneViewTableWidget->cellWidget(row, SCENE_VIEW_ACTIONS_COLUMN));
  if (actionsWidget == nullptr)
    {
    actionsWidget = new QFrame;
    QVBoxLayout* actionsLayout = new QVBoxLayout;
    actionsWidget->setLayout(actionsLayout);
    QToolButton* restoreButton = new QToolButton;
    restoreButton->setText(qCjyxSceneViewsModuleWidget::tr("Restore"));
    restoreButton->setToolTip(qCjyxSceneViewsModuleWidget::tr("Restore"));
    restoreButton->setIcon(QIcon(":/Icons/Restore.png"));
    restoreButton->setProperty(ROW_INDEX_PROPERTY, row);
    QObject::connect(restoreButton, SIGNAL(clicked()), q, SLOT(onRestoreButtonClicked()));
    QToolButton* editButton = new QToolButton;
    editButton->setText(qCjyxSceneViewsModuleWidget::tr("Edit"));
    editButton->setToolTip(qCjyxSceneViewsModuleWidget::tr("Edit"));
    editButton->setIcon(QIcon(":/Icons/Medium/CjyxConfigure.png"));
    editButton->setProperty(ROW_INDEX_PROPERTY, row);
    QObject::connect(editButton, SIGNAL(clicked()), q, SLOT(onEditButtonClicked()));
    QToolButton* deleteButton = new QToolButton;
    deleteButton->setText(qCjyxSceneViewsModuleWidget::tr("Delete"));
    deleteButton->setToolTip(qCjyxSceneViewsModuleWidget::tr("Delete"));
    deleteButton->setIcon(QIcon(":/Icons/Delete.png"));
    deleteButton->setProperty(ROW_INDEX_PROPERTY, row);
    QObject::connect(deleteButton, SIGNAL(clicked()), q, SLOT(onDeleteButtonClicked()));
    actionsLayout->addWidget(restoreButton);
    actionsLayout->addWidget(editButton);
    actionsLayout->addWidget(deleteButton);
    this->SceneViewTableWidget->setCellWidget(row, SCENE_VIEW_ACTIONS_COLUMN, actionsWidget);
    }
}

//-----------------------------------------------------------------------------
// qCjyxSceneViewsModuleWidget methods

//-----------------------------------------------------------------------------
qCjyxSceneViewsModuleWidget::qCjyxSceneViewsModuleWidget(QWidget* parent) :
  qCjyxAbstractModuleWidget(parent)
  , d_ptr(new qCjyxSceneViewsModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qCjyxSceneViewsModuleWidget::~qCjyxSceneViewsModuleWidget() = default;

//-----------------------------------------------------------------------------
void qCjyxSceneViewsModuleWidget::setup()
{
  Q_D(qCjyxSceneViewsModuleWidget);
  this->Superclass::setup();
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qCjyxSceneViewsModuleWidget::moveDownSelected(QString dmmlId)
{
  Q_D(qCjyxSceneViewsModuleWidget);

  const char* id = d->logic()->MoveSceneViewDown(dmmlId.toUtf8());

  if (id != nullptr &&
      strcmp(id, "") != 0)
    {
    this->updateFromDMMLScene();
    }
}

//-----------------------------------------------------------------------------
void qCjyxSceneViewsModuleWidget::moveUpSelected(QString dmmlId)
{
  Q_D(qCjyxSceneViewsModuleWidget);

  const char* id = d->logic()->MoveSceneViewUp(dmmlId.toUtf8());

  if (id != nullptr &&
      strcmp(id, "") != 0)
    {
    this->updateFromDMMLScene();
    }
}

//-----------------------------------------------------------------------------
void qCjyxSceneViewsModuleWidget::restoreSceneView(const QString& dmmlId)
{
  Q_D(qCjyxSceneViewsModuleWidget);

  // by default, make sure no nodes from the current scene are lost, adding them to
  // the scene view about to be restored
  if (!d->logic()->RestoreSceneView(dmmlId.toUtf8(), false))
    {
    // ask if the user wishes to save the current scene nodes, restore and delete them or cancel
    qCjyxApplication* app = qCjyxApplication::application();
    QWidget* mainWindow = app ? app->mainWindow() : nullptr;
    ctkMessageBox* missingNodesMsgBox = new ctkMessageBox(mainWindow);
    missingNodesMsgBox->setWindowTitle("Data missing from Scene View");
    vtkDMMLSceneViewNode* viewNode = vtkDMMLSceneViewNode::SafeDownCast(this->dmmlScene()->GetNodeByID(dmmlId.toUtf8()));
    QString sceneViewName;
    if (viewNode)
      {
      sceneViewName = QString(viewNode->GetName());
      }
    QString labelText = QString("Add data to scene view \"")
      + sceneViewName
      + QString("\" before restoring?\n"
                "\n");
    QString infoText = QString(
      "Data is present in the current scene but not in the scene view.\n"
      "\n"
      "If you don't add and restore, data not already saved to disk"
      ", or saved in another scene view,"
      " will be permanently lost!\n");
    missingNodesMsgBox->setText(labelText + infoText);
    // until CTK bug is fixed, informative text will overlap the don't show
    // again message so put it all in the label text
    // missingNodesMsgBox->setInformativeText(infoText);
    QPushButton *continueButton = missingNodesMsgBox->addButton(QMessageBox::Discard);
    continueButton->setText("Restore without saving");
    QPushButton *addButton = missingNodesMsgBox->addButton(QMessageBox::Save);
    addButton->setText("Add and Restore");
    missingNodesMsgBox->addButton(QMessageBox::Cancel);

    missingNodesMsgBox->setIcon(QMessageBox::Warning);
    missingNodesMsgBox->setDontShowAgainVisible(true);
    missingNodesMsgBox->setDontShowAgainSettingsKey("SceneViewsModule/AlwaysRemoveNodes");
    int ret = missingNodesMsgBox->exec();
    switch (ret)
      {
      case QMessageBox::Discard:
        d->logic()->RestoreSceneView(dmmlId.toUtf8(), true);
        break;
      case QMessageBox::Save:
        if (viewNode)
          {
          viewNode->AddMissingNodes();

          // and restore again
          d->logic()->RestoreSceneView(dmmlId.toUtf8(), false);
          }
        break;
      case QMessageBox::Cancel:
      default:
        break;
      }
    missingNodesMsgBox->deleteLater();
    }

  qCjyxApplication::application()->mainWindow()->statusBar()->showMessage("The SceneView was restored including the attached scene.", 2000);
}

//-----------------------------------------------------------------------------
void qCjyxSceneViewsModuleWidget::editSceneView(const QString& dmmlId)
{
  Q_D(qCjyxSceneViewsModuleWidget);
  d->sceneViewDialog()->loadNode(dmmlId);
  d->sceneViewDialog()->exec();
  this->updateFromDMMLScene();
}

//-----------------------------------------------------------------------------
void qCjyxSceneViewsModuleWidget::updateFromDMMLScene()
{
  Q_D(qCjyxSceneViewsModuleWidget);

  if (this->dmmlScene() == nullptr)
    {
    d->SceneViewTableWidget->setRowCount(0);
    return;
    }
  int numSceneViews = this->dmmlScene()->GetNumberOfNodesByClass("vtkDMMLSceneViewNode");

  // don't recreate the table if the number of items is not changed to preserve selection state
  d->SceneViewTableWidget->setRowCount(numSceneViews);

  std::vector<vtkDMMLNode*> sceneViewNodes;
  this->dmmlScene()->GetNodesByClass("vtkDMMLSceneViewNode", sceneViewNodes);
  int rowIndex = 0;
  for (std::vector<vtkDMMLNode*>::iterator it = sceneViewNodes.begin(); it != sceneViewNodes.end(); ++it)
    {
    vtkDMMLSceneViewNode* sceneViewNode = vtkDMMLSceneViewNode::SafeDownCast(*it);
    if (!sceneViewNode)
      {
      continue;
      }
    d->updateTableRowFromSceneView(rowIndex, sceneViewNode);
    ++rowIndex;
    }

  d->SceneViewTableWidget->resizeRowsToContents();
}

//-----------------------------------------------------------------------------
void qCjyxSceneViewsModuleWidget::enter()
{
  this->Superclass::enter();

  // set up dmml scene observations so that the GUI gets updated
  this->qvtkConnect(this->dmmlScene(), vtkDMMLScene::NodeAddedEvent,
                    this, SLOT(onDMMLSceneEvent(vtkObject*, vtkObject*)));
  this->qvtkConnect(this->dmmlScene(), vtkDMMLScene::NodeRemovedEvent,
                    this, SLOT(onDMMLSceneEvent(vtkObject*, vtkObject*)));
  this->qvtkConnect(this->dmmlScene(), vtkDMMLScene::EndCloseEvent,
                    this, SLOT(onDMMLSceneReset()));
  this->qvtkConnect(this->dmmlScene(), vtkDMMLScene::EndImportEvent,
                    this, SLOT(onDMMLSceneReset()));
  this->qvtkConnect(this->dmmlScene(), vtkDMMLScene::EndRestoreEvent,
                    this, SLOT(onDMMLSceneReset()));
  this->qvtkConnect(this->dmmlScene(), vtkDMMLScene::EndBatchProcessEvent,
                    this, SLOT(onDMMLSceneReset()));

  this->updateFromDMMLScene();
}

//-----------------------------------------------------------------------------
void qCjyxSceneViewsModuleWidget::exit()
{
  this->Superclass::exit();

  // remove dmml scene observations, don't need to update the GUI while the
  // module is not showing
  this->qvtkDisconnectAll();
}

//-----------------------------------------------------------------------------
void qCjyxSceneViewsModuleWidget::onDMMLSceneEvent(vtkObject*, vtkObject* node)
{
  if (!this->dmmlScene() || this->dmmlScene()->IsBatchProcessing())
    {
    return;
    }
  vtkDMMLSceneViewNode* sceneViewNode = vtkDMMLSceneViewNode::SafeDownCast(node);
  if (sceneViewNode)
    {
    this->updateFromDMMLScene();
    }
}

//-----------------------------------------------------------------------------
void qCjyxSceneViewsModuleWidget::onDMMLSceneReset()
{
  if (!this->dmmlScene() || this->dmmlScene()->IsBatchProcessing())
    {
    return;
    }
  this->updateFromDMMLScene();
}

//-----------------------------------------------------------------------------
// SceneView functionality
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void qCjyxSceneViewsModuleWidget::showSceneViewDialog()
{
  Q_D(qCjyxSceneViewsModuleWidget);
  // show the dialog
  d->sceneViewDialog()->reset();
  d->sceneViewDialog()->exec();
}

//-----------------------------------------------------------
bool qCjyxSceneViewsModuleWidget::setEditedNode(vtkDMMLNode* node,
                                                  QString role /* = QString()*/,
                                                  QString context /* = QString()*/)
{
  Q_UNUSED(role);
  Q_UNUSED(context);
  Q_D(qCjyxSceneViewsModuleWidget);
  if (!vtkDMMLSceneViewNode::SafeDownCast(node))
    {
    return false;
    }
  std::vector<vtkDMMLNode*> sceneViewNodes;
  this->dmmlScene()->GetNodesByClass("vtkDMMLSceneViewNode", sceneViewNodes);
  int rowIndex = 0;
  for (std::vector<vtkDMMLNode*>::iterator it = sceneViewNodes.begin(); it != sceneViewNodes.end(); ++it)
    {
    vtkDMMLSceneViewNode* sceneViewNode = vtkDMMLSceneViewNode::SafeDownCast(*it);
    if (!sceneViewNode)
      {
      continue;
      }
    if (node == sceneViewNode)
      {
      // scene view node found
      this->updateFromDMMLScene();
      QModelIndex newIndex = d->SceneViewTableWidget->model()->index(rowIndex, SCENE_VIEW_ACTIONS_COLUMN);
      d->SceneViewTableWidget->setCurrentIndex(newIndex);
      return true;
      }
    ++rowIndex;
    }
  // scene view node not found
  return false;
}

//-----------------------------------------------------------------------------
void qCjyxSceneViewsModuleWidget::onSceneViewDoubleClicked(int row, int column)
{
  Q_UNUSED(column);
  Q_D(qCjyxSceneViewsModuleWidget);
  vtkDMMLNode* sceneViewNode = this->dmmlScene()->GetNthNodeByClass(row, "vtkDMMLSceneViewNode");
  if (!sceneViewNode || !sceneViewNode->GetID())
    {
    return;
    }
  this->restoreSceneView(QString(sceneViewNode->GetID()));
}

//-----------------------------------------------------------------------------
void qCjyxSceneViewsModuleWidget::onRestoreButtonClicked()
{
  Q_D(qCjyxSceneViewsModuleWidget);
  QToolButton* button = qobject_cast<QToolButton*>(this->sender());
  if (!button)
    {
    return;
    }
  int rowIndex = button->property(ROW_INDEX_PROPERTY).toInt();
  vtkDMMLNode* sceneViewNode = this->dmmlScene()->GetNthNodeByClass(rowIndex, "vtkDMMLSceneViewNode");
  if (!sceneViewNode || !sceneViewNode->GetID())
    {
    return;
    }
  this->restoreSceneView(QString(sceneViewNode->GetID()));
}

//-----------------------------------------------------------------------------
void qCjyxSceneViewsModuleWidget::onEditButtonClicked()
{
  Q_D(qCjyxSceneViewsModuleWidget);
  QToolButton* button = qobject_cast<QToolButton*>(this->sender());
  if (!button)
    {
    return;
    }
  int rowIndex = button->property(ROW_INDEX_PROPERTY).toInt();
  vtkDMMLNode* sceneViewNode = this->dmmlScene()->GetNthNodeByClass(rowIndex, "vtkDMMLSceneViewNode");
  if (!sceneViewNode || !sceneViewNode->GetID())
    {
    return;
    }
  this->editSceneView(QString(sceneViewNode->GetID()));
}

//-----------------------------------------------------------------------------
void qCjyxSceneViewsModuleWidget::onDeleteButtonClicked()
{
  Q_D(qCjyxSceneViewsModuleWidget);
  QToolButton* button = qobject_cast<QToolButton*>(this->sender());
  if (!button)
    {
    return;
    }
  int rowIndex = button->property(ROW_INDEX_PROPERTY).toInt();
  vtkDMMLNode* sceneViewNode = this->dmmlScene()->GetNthNodeByClass(rowIndex, "vtkDMMLSceneViewNode");
  if (!sceneViewNode)
    {
    return;
    }
  this->dmmlScene()->RemoveNode(sceneViewNode);
}
