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

/// Qt includes
#include <QApplication>
#include <QComboBox>
#include <QDate>
#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegExp>
#include <QRegExpValidator>
#include <QSettings>

/// CTK includes
#include <ctkCheckableHeaderView.h>
#include <ctkCheckableModelHelper.h>
#include <ctkPathLineEdit.h>
#include <ctkVTKWidgetsUtils.h>

/// Cjyx includes
#include "qCjyxApplication.h"
#include "qCjyxCoreIOManager.h"
#include "qCjyxFileWriterOptionsWidget.h"
#include "qCjyxSaveDataDialog_p.h"
#include "qCjyxLayoutManager.h"
#include "qDMMLUtils.h"

/// Logic includes
#include "vtkDMMLLogic.h"

/// DMML includes
#include <vtkCacheManager.h>
#include <vtkCollection.h>
#include <vtkDataFileFormatHelper.h> // for GetFileExtensionFromFormatString()
//#include <vtkDMMLHierarchyNode.h>
#include <vtkDMMLMessageCollection.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLStorageNode.h>
#include <vtkDMMLSceneViewNode.h>

/// VTK includes
#include <vtkStringArray.h>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>

// STD includes
#include <cstring> // for strlen

namespace
{
  const char SHOW_OPTIONS_SETTINGS_KEY[]="ioManager/SaveDataDialogShowOptions";
}

//-----------------------------------------------------------------------------
qCjyxFileNameItemDelegate::qCjyxFileNameItemDelegate( QObject * parent )
  : Superclass(parent)
{
  this->DMMLScene = nullptr;
}

//-----------------------------------------------------------------------------
QString qCjyxFileNameItemDelegate::forceFileNameExtension(const QString& fileName, const QString& extension,
                                                   vtkDMMLScene* dmmlScene, const QString& nodeID)
{
  QString strippedFileName = qCjyxCoreIOManager::forceFileNameValidCharacters(fileName);
  if(!dmmlScene)
    {
    // no scene is set, cannot check extension
    return strippedFileName;
    }
  vtkObject* object = dmmlScene;
  if (!nodeID.isEmpty())
    {
    object = qCjyxSaveDataDialogPrivate::getNodeByID(nodeID.toUtf8().data(), dmmlScene);
    }
  if (!object)
    {
    qCritical() << Q_FUNC_INFO << " failed: node not found by ID " << qPrintable(nodeID);
    return QString();
    }
  qCjyxCoreIOManager* coreIOManager =
    qCjyxCoreApplication::application()->coreIOManager();
  strippedFileName = coreIOManager->stripKnownExtension(strippedFileName, object) + extension;
  return strippedFileName;
}

//-----------------------------------------------------------------------------
qCjyxSaveDataDialogPrivate::qCjyxSaveDataDialogPrivate(QWidget* parentWidget)
  : QDialog(parentWidget)
{
  this->DMMLScene = nullptr;

  this->setupUi(this);

  this->ErrorLabel->setVisible(false);
  this->ConfirmOverwriteAnswer = QMessageBox::Ignore;
  this->CancelRequested = false;

  this->WarningIcon.addPixmap(qApp->style()->standardPixmap(QStyle::SP_MessageBoxWarning));
  this->ErrorIcon.addPixmap(qApp->style()->standardPixmap(QStyle::SP_MessageBoxCritical));

  this->FileWidget->setItemDelegateForColumn(
    FileNameColumn, new qCjyxFileNameItemDelegate(this));
  this->FileWidget->verticalHeader()->setVisible(false);

  // Checkable headers.
  // We replace the current FileWidget header view with a checkable header view.
  // Checked files (rows) will be saved, unchecked files will be discarded.
  // In order to have a column checkable, we need to manually set a checkstate
  // to a column. No checkstate (null QVariant) means uncheckable.
  this->FileWidget->model()->setHeaderData(SelectColumn, Qt::Horizontal,
                                           Qt::Unchecked, Qt::CheckStateRole);
  QHeaderView* previousHeaderView = this->FileWidget->horizontalHeader();
  ctkCheckableHeaderView* headerView = new ctkCheckableHeaderView(Qt::Horizontal, this->FileWidget);
  // Copy the previous behavior of the header into the new checkable header view
  headerView->setSectionsClickable(previousHeaderView->sectionsClickable());
  headerView->setSectionsMovable(previousHeaderView->sectionsMovable());
  headerView->setHighlightSections(previousHeaderView->highlightSections());
  headerView->setStretchLastSection(previousHeaderView->stretchLastSection());
  // Propagate to top-level items only (depth = 1),no need to go deeper
  // (depth = -1 or 2, 3...) as it is a flat list.
  headerView->checkableModelHelper()->setPropagateDepth(1);
  // Finally assign the new header to the view
  this->FileWidget->setHorizontalHeader(headerView);

  // Connect push buttons to associated actions
  connect(this->DirectoryButton, SIGNAL(directorySelected(QString)),
          this, SLOT(setDirectory(QString)));
  connect(this->SelectSceneDataButton, SIGNAL(clicked()),
          this, SLOT(selectModifiedSceneData()));
  connect(this->SelectDataButton, SIGNAL(clicked()),
          this, SLOT(selectModifiedData()));
  connect(this->DataBundleButton, SIGNAL(clicked()),
          this, SLOT(saveSceneAsDataBundle()));
  connect(this->ShowMoreCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(showMoreColumns(bool)));

  connect(this->FileWidget, SIGNAL(itemChanged(QTableWidgetItem*)),
          this, SLOT(onItemChanged(QTableWidgetItem*)));

  if (!qCjyxApplication::application()->userSettings()->contains(SHOW_OPTIONS_SETTINGS_KEY))
    {
    qCjyxApplication::application()->userSettings()->setValue(SHOW_OPTIONS_SETTINGS_KEY, false);
    }
  this->ShowMoreCheckBox->setChecked(qCjyxApplication::application()->userSettings()->value(SHOW_OPTIONS_SETTINGS_KEY).toBool());
  this->showMoreColumns(this->ShowMoreCheckBox->isChecked());

  this->PopulatingItems = false;
}

//-----------------------------------------------------------------------------
qCjyxSaveDataDialogPrivate::~qCjyxSaveDataDialogPrivate() = default;

//-----------------------------------------------------------------------------
void qCjyxSaveDataDialogPrivate::setDMMLScene(vtkDMMLScene* scene)
{
  this->DMMLScene = scene;

  qCjyxFileNameItemDelegate * fileNameItemDelegate =
      dynamic_cast<qCjyxFileNameItemDelegate*>(this->FileWidget->itemDelegateForColumn(Self::FileNameColumn));
  Q_ASSERT(fileNameItemDelegate);
  fileNameItemDelegate->DMMLScene = scene;

  this->populateItems();
}

//-----------------------------------------------------------------------------
vtkDMMLScene* qCjyxSaveDataDialogPrivate::dmmlScene()const
{
  return this->DMMLScene;
}

//-----------------------------------------------------------------------------
void qCjyxSaveDataDialogPrivate::setDirectory(const QString& newDirectory)
{
  QDir newDir(newDirectory);
  Q_ASSERT(newDir.exists());

  const int rowCount = this->FileWidget->rowCount();
  for (int row = 0; row < rowCount; ++row)
    {
    QTableWidgetItem* selectItem = this->FileWidget->item(row, SelectColumn);
    Q_ASSERT(selectItem);
    if (selectItem->checkState() == Qt::Unchecked)
      {
      continue;
      }
    ctkPathLineEdit* directoryItemEdit = qobject_cast<ctkPathLineEdit*>(
      this->FileWidget->cellWidget(row, FileDirectoryColumn));
    Q_ASSERT(directoryItemEdit);
    directoryItemEdit->setCurrentPath(newDir.path());
    }
}

//-----------------------------------------------------------------------------
void qCjyxSaveDataDialogPrivate::populateItems()
{
  // clear the list
  this->FileWidget->setRowCount(0);
  this->ErrorLabel->setVisible(false);
  if (this->DMMLScene == nullptr)
    {
    return;
    }

  this->PopulatingItems = true;

  QDir newDir(this->DMMLScene->GetRootDirectory());
  if (!newDir.exists())
    {
    this->DMMLScene->SetRootDirectory(".");
    }

  // sorting the table while doing insertions is dangerous
  // Moreover, we want to have the DMML scene to be the first item.
  this->FileWidget->setSortingEnabled(false);

  this->DirectoryButton->setDirectory(this->DMMLScene->GetRootDirectory());

  this->populateScene();

  // get all storable nodes in the main scene
  // and store them in the map by ID to avoid duplicates for the scene views
  std::map<std::string, vtkDMMLNode *> storableNodes;
  std::vector<vtkDMMLNode *> nodes;
  this->DMMLScene->GetNodesByClass("vtkDMMLStorableNode", nodes);
  std::vector<vtkDMMLNode *>::iterator it;

  for (it = nodes.begin(); it != nodes.end(); it++)
    {
    vtkDMMLNode* node = (*it);
    this->populateNode(node);
    storableNodes[std::string(node->GetID())] = node;
    }

  // get all additional storable nodes for all scene views except "Master Scene View"
  nodes.clear();
  this->DMMLScene->GetNodesByClass("vtkDMMLSceneViewNode", nodes);
  for (it = nodes.begin(); it != nodes.end(); it++)
    {
    vtkDMMLSceneViewNode *svNode = vtkDMMLSceneViewNode::SafeDownCast(*it);
    // skip "Master Scene View" since it contains the same nodes as the scene
    if (svNode->GetName() && std::string("Master Scene View") == std::string(svNode->GetName()))
      {
      continue;
      }
    std::vector<vtkDMMLNode *> snodes;
    svNode->GetNodesByClass("vtkDMMLStorableNode", snodes);
    std::vector<vtkDMMLNode *>::iterator sit;
    for (sit = snodes.begin(); sit != snodes.end(); sit++)
      {
      vtkDMMLNode* node = (*sit);
      if (storableNodes.find(std::string(node->GetID())) == storableNodes.end())
        {
        // process only new storable nodes
        this->populateNode(node);
        storableNodes[std::string(node->GetID())] = node;
        }
      }
    }


  // Here we could have restore the sorting property but we want to keep the
  // DMML scene the first item of the list so we don't do restore the sorting.
  // this->FileWidget->setSortingEnabled(oldSortingEnabled);

  // Enable/disable nodes depending on the scene file format
  this->onSceneFormatChanged();

  this->PopulatingItems = false;

  this->updateSize();
}

//-----------------------------------------------------------------------------
void qCjyxSaveDataDialogPrivate::populateScene()
{
  // Create a new entry
  int row = this->FileWidget->rowCount();
  this->FileWidget->insertRow(row);

  // Get absolute filename
  QFileInfo sceneFileInfo;
  if (this->DMMLScene->GetURL() != nullptr &&
      strlen(this->DMMLScene->GetURL()) > 0)
    {
    sceneFileInfo = QFileInfo( QDir(this->DMMLScene->GetRootDirectory()),
                               this->DMMLScene->GetURL());
    }
  else
    {
    sceneFileInfo = QFileInfo( QDir(this->DMMLScene->GetRootDirectory()),
                               QDate::currentDate().toString(
                                 "yyyy-MM-dd") + "-Scene.dmml");
    }

  // Scene Name
  QTableWidgetItem* sceneNameItem = new QTableWidgetItem("");
  sceneNameItem->setFlags(sceneNameItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsEnabled);
  this->FileWidget->setItem(row, NodeNameColumn, sceneNameItem);

  // Scene Type
  QTableWidgetItem* sceneTypeItem = new QTableWidgetItem("");
  sceneTypeItem->setData(Self::SceneTypeRole, QString("Scene"));
  sceneTypeItem->setFlags(sceneTypeItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsEnabled);
  this->FileWidget->setItem(row, NodeTypeColumn, sceneTypeItem);

  // Scene Status
  QTableWidgetItem* sceneModifiedItem = new QTableWidgetItem(
    this->DMMLScene->GetModifiedSinceRead() ? "Modified" : "Not Modified");
  sceneModifiedItem->setFlags(sceneModifiedItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsEnabled);
  this->FileWidget->setItem(row, NodeStatusColumn, sceneModifiedItem);

  qCjyxCoreIOManager* coreIOManager =
    qCjyxCoreApplication::application()->coreIOManager();

  // Scene Format
  QComboBox* sceneComboBoxWidget = new QComboBox(this->FileWidget);
  int currentFormat = -1;
  QString currentExtension = coreIOManager->extractKnownExtension(sceneFileInfo.fileName(), this->DMMLScene);
  foreach(const QString& nameFilter,
          coreIOManager->fileWriterExtensions(this->DMMLScene))
    {
    QString extension = QString::fromStdString(
      vtkDataFileFormatHelper::GetFileExtensionFromFormatString(nameFilter.toUtf8()));
    sceneComboBoxWidget->addItem(nameFilter, extension);
    if (extension == currentExtension)
      {
      currentFormat = sceneComboBoxWidget->count() - 1;
      }
    }
  sceneComboBoxWidget->setCurrentIndex(currentFormat);

  this->FileWidget->setCellWidget(row, FileFormatColumn, sceneComboBoxWidget);
  QObject::connect(sceneComboBoxWidget, SIGNAL(currentIndexChanged(int)),
                   this, SLOT(formatChanged()));
  QObject::connect(sceneComboBoxWidget, SIGNAL(currentIndexChanged(int)),
                   this, SLOT(onSceneFormatChanged()));

  // Scene FileName
  QTableWidgetItem* fileNameItem = this->createFileNameItem(sceneFileInfo, currentExtension, /* nodeID = */ QString());
  this->FileWidget->setItem( row, FileNameColumn, fileNameItem);

  // Scene Directory
  ctkPathLineEdit* sceneDirectoryEdit =
      this->createFileDirectoryWidget(sceneFileInfo);

  this->FileWidget->setCellWidget(row, FileDirectoryColumn, sceneDirectoryEdit);

  // Scene Selected
  QTableWidgetItem* selectItem = this->FileWidget->item(row, SelectColumn);
  selectItem->setFlags(selectItem->flags() | Qt::ItemIsUserCheckable);
  selectItem->setCheckState(
    this->DMMLScene->GetModifiedSinceRead() ? Qt::Checked : Qt::Unchecked);

  // Options
  this->updateOptionsWidget(row);

  // Set current scene file format based on last successful scene save
  int lastFormatIndex = sceneComboBoxWidget->findText(coreIOManager->defaultSceneFileType());
  if (lastFormatIndex != -1)
    {
    sceneComboBoxWidget->setCurrentIndex(lastFormatIndex);
    }
}

//-----------------------------------------------------------------------------
void qCjyxSaveDataDialogPrivate::populateNode(vtkDMMLNode* node)
{
  vtkDMMLStorableNode* storableNode = vtkDMMLStorableNode::SafeDownCast(node);
  // Don't show if the node doesn't want to (internal node)
  if (!storableNode ||
    storableNode->GetHideFromEditors() || !storableNode->GetSaveWithScene())
    {
    return;
    }

/*
  // if the node is an annotation node and in a hierarchy, the hierarchy will
  // take care of writing it out
  if (node->IsA("vtkDMMLAnnotationNode"))
    {
    // not supporting rulers just yet
    if (!node->IsA("vtkDMMLAnnotationRulerNode"))
      {
      vtkDMMLHierarchyNode *hnode = vtkDMMLHierarchyNode::GetAssociatedHierarchyNode(node->GetScene(), node->GetID());
      if (hnode &&
          hnode->GetParentNodeID())
        {
        // std::cout << "Skipping node in a hierarchy: " << node->GetName() << std::endl;
        return;
        }
      }
    }
*/
  // Get absolute filename and create storage node if needed
  QFileInfo fileInfo = this->nodeFileInfo(storableNode);
  if (fileInfo.filePath().isEmpty())
    {
    return;
    }

  qCjyxCoreIOManager* coreIOManager =
    qCjyxCoreApplication::application()->coreIOManager();
  Q_ASSERT(coreIOManager);
  // Must be called after nodeFileInfo() as it creates a storage node
  // that is mandatory for fileWriterFileType()
  if (coreIOManager->fileWriterFileType(node) == QString("NoFile"))
    {
    return;
    }

  if (!storableNode->GetStorageNode())
    {
    qCritical() << Q_FUNC_INFO << " failed: storage node not found for node "
      << (storableNode->GetID() ? storableNode->GetID() : "(unknown)")
      << ". The node will not be shown in the save data dialog.";
    return;
    }

  // Create a new entry
  int row = this->FileWidget->rowCount();
  this->FileWidget->insertRow(row);

  // Node name
  QTableWidgetItem *nodeNameItem = this->createNodeNameItem(storableNode);
  this->FileWidget->setItem(row, NodeNameColumn, nodeNameItem);

  // Node type
  QTableWidgetItem *nodeTypeItem = this->createNodeTypeItem(storableNode);
  this->FileWidget->setItem(row, NodeTypeColumn, nodeTypeItem);

  // Node status
  QTableWidgetItem *nodeModifiedItem = this->createNodeStatusItem(storableNode, fileInfo);
  this->FileWidget->setItem(row, NodeStatusColumn, nodeModifiedItem);

  // File format
  QComboBox* fileFormatsWidget = qobject_cast<QComboBox*>(
    this->createFileFormatsWidget(storableNode, fileInfo));
  this->FileWidget->setCellWidget(row, FileFormatColumn, fileFormatsWidget);
  QString extension = fileFormatsWidget->itemData(
    fileFormatsWidget->currentIndex()).toString();

  // File name
  QTableWidgetItem *fileNameItem = this->createFileNameItem(fileInfo, extension, QString(node->GetID()));
  this->FileWidget->setItem(row, FileNameColumn, fileNameItem);

  // File Directory
  QWidget* directoryWidget = this->createFileDirectoryWidget(fileInfo);
  this->FileWidget->setCellWidget(row, FileDirectoryColumn, directoryWidget);

  // Select modified nodes by default
  QTableWidgetItem* selectItem = this->FileWidget->item(row, SelectColumn);
  selectItem->setCheckState(
    storableNode->GetModifiedSinceRead() ? Qt::Checked : Qt::Unchecked);

  // Options
  this->updateOptionsWidget(row);
}

//-----------------------------------------------------------------------------
QFileInfo qCjyxSaveDataDialogPrivate::nodeFileInfo(vtkDMMLStorableNode* node)
{
  // Remove characters from node name that cannot be used in file names
  // (same method as in qCjyxFileNameItemDelegate::forceFileNameExtension)
  QString inputNodeName(node->GetName() ? node->GetName() : "");
  QString safeNodeName = qCjyxCoreIOManager::forceFileNameValidCharacters(inputNodeName);

  vtkDMMLStorageNode* snode = node->GetStorageNode();
  if (snode == nullptr)
    {
    bool success = node->AddDefaultStorageNode();
    if (!success)
      {
      qCritical() << Q_FUNC_INFO << " failed: error while trying to add storage node";
      return QFileInfo();
      }
    snode = node->GetStorageNode();
    if (!snode)
      {
      // no error and no storage node means that
      // there is no need for storage node, the node can be stored in the scene
      return QFileInfo();
      }
    }
  else
    {
    // a file name exists, but we want to update the filename to match the current
    // node name
    if (snode->GetFileName() && node->GetName())
      {
      std::string filenameWithoutExtension = snode->GetFileNameWithoutExtension();
      // Only reset the file name if the user has set the name explicitly (that is,
      // if the name isn't the default created by qCjyxVolumesIOOptionsWidget::setFileNames
      // TODO: this logic relies on the GUI so we should consider moving it into DMML proper
      // with a way for storage nodes to generate their default node names from a given filename
      if (QString(filenameWithoutExtension.c_str()) != safeNodeName)
        {
        QFileInfo existingInfo(snode->GetFileName());
        std::string extension = snode->GetSupportedFileExtension();
        QFileInfo newInfo(existingInfo.path(), safeNodeName + QString(extension.c_str()));
        snode->SetFileName(newInfo.filePath().toUtf8());
        node->StorableModified();
        }
      }
    }
  if (snode->GetFileName() == nullptr && !this->DirectoryButton->directory().isEmpty())
    {
    QString fileExtension = snode->GetDefaultWriteFileExtension();
    if (!fileExtension.isEmpty())
      {
      fileExtension = QString(".") + fileExtension;
      }

    QFileInfo fileName(QDir(this->DirectoryButton->directory()),
                       safeNodeName + fileExtension);
    snode->SetFileName(fileName.absoluteFilePath().toUtf8());
    }

  QFileInfo fileInfo;
  if (this->DMMLScene->IsFilePathRelative(snode->GetFileName()))
    {
    fileInfo = QFileInfo(QDir(this->DMMLScene->GetRootDirectory()),
                         snode->GetFileName());
    }
  else
    {
    fileInfo = QFileInfo(snode->GetFileName());
    }
  return fileInfo;
}

//-----------------------------------------------------------------------------
QTableWidgetItem* qCjyxSaveDataDialogPrivate::createNodeNameItem(vtkDMMLStorableNode* node)
{
  QTableWidgetItem* nodeNameItem = new QTableWidgetItem(node->GetName());
  nodeNameItem->setFlags( nodeNameItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsEnabled);
  // the tooltip is used to store the id of the nodes
  Q_ASSERT(node->GetStorageNode());
  nodeNameItem->setData(Qt::ToolTipRole, QString(node->GetID()) + " " + node->GetStorageNode()->GetID() );
  return nodeNameItem;
}

//-----------------------------------------------------------------------------
QTableWidgetItem* qCjyxSaveDataDialogPrivate::createNodeTypeItem(vtkDMMLStorableNode* node)
{
  QTableWidgetItem* nodeTypeItem = new QTableWidgetItem(node->GetNodeTagName());
  nodeTypeItem->setFlags( nodeTypeItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsEnabled);
  // TODO: add icon based on the type
  return nodeTypeItem;
}

//-----------------------------------------------------------------------------
QTableWidgetItem* qCjyxSaveDataDialogPrivate
::createNodeStatusItem(vtkDMMLStorableNode* node, const QFileInfo& fileInfo)
{
  Q_UNUSED(fileInfo);
  // Node status (modified / not modified)
  // As a safety measure:
  // If the data is sitting in cache, it's vulnerable to overwriting or deleting.
  // Mark the node as modified since read so that a user will be more likely
  // to save it to a reliable location on local (or remote) disk.
  /*
  if ( this->DMMLScene->GetCacheManager() )
    {
    if ( this->DMMLScene->GetCacheManager()->GetRemoteCacheDirectory() )
      {
      QString cacheDir = this->DMMLScene->GetCacheManager()->GetRemoteCacheDirectory();
      int pos = fileInfo.absoluteFilePath().indexOf(cacheDir);
      if ( pos != -1)
        {
        int disableModify = node->GetDisableModifiedEvent();
        node->SetDisableModifiedEvent(1);
        node->ModifiedSinceReadOn();
        node->SetDisableModifiedEvent(disableModify);
        }
      }
    else
      {
      qWarning() << "Warning saving data: cannot get a default cache "
        "directory, so not able to check whether any datafiles are residing in cache "
        "and should be marked for save by default. Please take care when saving data.";
      }
    }
  else
    {
    qWarning() << "Warning saving data: cannot get a default cache "
      "directory, so not able to check whether any datafiles are residing in cache "
      "and should be marked for save by default. Please take care when saving data.";
    }
  */
  QTableWidgetItem *nodeModifiedItem =
    new QTableWidgetItem(node->GetModifiedSinceRead() ?
                         tr("Modified") : tr("Not Modified"));
  nodeModifiedItem->setFlags( nodeModifiedItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsEnabled);
  return nodeModifiedItem;
}

//-----------------------------------------------------------------------------
QWidget* qCjyxSaveDataDialogPrivate::createFileFormatsWidget(vtkDMMLStorableNode* node, QFileInfo& fileInfo)
{
  vtkDMMLStorageNode* snode = node->GetStorageNode();
  Q_ASSERT(snode);
  QComboBox* fileFormats = new QComboBox(this->FileWidget);
  // Add custom qCjyxSaveFile
  qCjyxCoreIOManager* coreIOManager =
    qCjyxCoreApplication::application()->coreIOManager();
  int currentFormat = -1;
  QString currentExtension = coreIOManager->completeCjyxWritableFileNameSuffix(node);
  foreach(QString nameFilter, coreIOManager->fileWriterExtensions(node))
    {
    QString extension = QString::fromStdString(
      vtkDataFileFormatHelper::GetFileExtensionFromFormatString(nameFilter.toUtf8()));
    fileFormats->addItem(nameFilter, extension);
    if (extension == currentExtension)
      {
      currentFormat = fileFormats->count() - 1;
      }
    }
  // The existing file name doesn't contain an existing extension, pick the
  // default extension if any
  if (currentFormat == -1 &&
      snode->GetDefaultWriteFileExtension() != nullptr)
    {
    for (int i = 0; i < fileFormats->count(); ++i)
      {
      if (fileFormats->itemData(i).toString() ==
          QString('.') + QString(snode->GetDefaultWriteFileExtension()))
        {
        currentFormat = i;
        fileInfo = QFileInfo(fileInfo.dir(),
                             fileInfo.completeBaseName() +
                               fileFormats->itemData(i).toString());
        break;
        }
      }
    }

  fileFormats->setCurrentIndex(currentFormat);
  if (currentFormat == -1)
    {
    // In order to set a custom text (different than the combobox items),
    // we need to make the combobox editable, but we don't want the user to
    // edit the combobox line.
    fileFormats->setEditable(true);
    fileFormats->lineEdit()->setReadOnly(true);
    fileFormats->setEditText("Select a format");
    }

  // TODO: use QSignalMapper
  QObject::connect(fileFormats, SIGNAL(currentIndexChanged(int)),
                   this, SLOT(formatChanged()));
  return fileFormats;
}

//-----------------------------------------------------------------------------
QTableWidgetItem* qCjyxSaveDataDialogPrivate
::createFileNameItem(const QFileInfo& fileInfo, const QString& extension, const QString& nodeID)
{
  QTableWidgetItem* fileNameItem = new QTableWidgetItem(
    qCjyxFileNameItemDelegate::forceFileNameExtension(fileInfo.fileName(), extension, this->dmmlScene(), nodeID));
  if (!extension.isEmpty())
    {
    fileNameItem->setData(Self::FileExtensionRole, extension);
    }
  fileNameItem->setData(Self::UIDRole, nodeID);
  return fileNameItem;
}

//-----------------------------------------------------------------------------
ctkPathLineEdit* qCjyxSaveDataDialogPrivate::createFileDirectoryWidget(const QFileInfo& fileInfo)
{
  ctkPathLineEdit* directoryEdit = new ctkPathLineEdit("Output folder", QStringList(), ctkPathLineEdit::Dirs, this->FileWidget);
  directoryEdit->setCurrentPath(fileInfo.absolutePath());
  directoryEdit->setShowHistoryButton(false);
  return directoryEdit;
}

//-----------------------------------------------------------------------------
void qCjyxSaveDataDialogPrivate
::clearUserMessagesInStorageNodes()
{
  for (int row = 0; row < this->FileWidget->rowCount(); ++row)
    {
    vtkDMMLStorableNode* storableNode;
    vtkDMMLStorageNode* snode;
    if ((storableNode = vtkDMMLStorableNode::SafeDownCast(this->object(row))) && (snode = storableNode->GetStorageNode()))
      {
      snode->GetUserMessages()->ClearMessages();
      }
    }
}

//-----------------------------------------------------------------------------
void qCjyxSaveDataDialogPrivate::accept()
{
  if (!this->save())
    {
    return;
    }
  this->done(QDialog::Accepted);
}

//-----------------------------------------------------------------------------
bool qCjyxSaveDataDialogPrivate::save()
{
  bool success = true;

  this->ErrorLabel->setVisible(false);
  this->ConfirmOverwriteAnswer = QMessageBox::Ignore;
  this->CancelRequested = false;

  QFileInfo sceneFile = this->sceneFile();
  const int sceneRow = this->findSceneRow();
  bool saveSceneFile = this->mustSceneBeSaved();
  this->setStatusIcon(sceneRow, QIcon(), QString());
  if (saveSceneFile)
    {
    if (sceneFile.exists())
      {
      saveSceneFile = this->confirmOverwrite(sceneFile.absoluteFilePath());
      if (!saveSceneFile)
        {
        this->setStatusIcon(sceneRow, this->ErrorIcon, tr("Scene file was not saved because user chose not to overwrite existing file: %1.")
          .arg(sceneFile.absoluteFilePath()));
        success = false;
        }
      }
    }

  // Set the root directory to the scene so that the nodes paths are points
  // to the new scene path. It's important to have the right node paths
  // relatively to the scene in the saved dmml scene file.
  this->DMMLSceneRootDirectoryBeforeSaving = this->DMMLScene->GetRootDirectory();
  this->setSceneRootDirectory(sceneFile.absoluteDir().absolutePath().toUtf8());

  // Save the nodes first then the scene
  // Saving the nodes first ensures that the saved scene will points to the
  // potentially new path of the nodes.
  if (!this->saveNodes())
    {
    success = false;
    }

  if (saveSceneFile && !this->CancelRequested)
    {
    if (!this->saveScene())
      {
      success = false;
      }
    }
  else
    {
    // restore the root directory only if the scene is not saved
    this->setSceneRootDirectory(this->DMMLSceneRootDirectoryBeforeSaving);
    }

  this->ErrorLabel->setVisible(!success);
  return success;
}

//-----------------------------------------------------------------------------
bool qCjyxSaveDataDialogPrivate::saveNodes()
{
  bool doneWithSaveDataDialog = true;
  QList<qCjyxIO::IOProperties> files;
  const int sceneRow = this->findSceneRow();
  for (int row = 0; row < this->FileWidget->rowCount(); ++row)
    {
    // only save nodes here
    if (row == sceneRow || this->FileWidget->isRowHidden(row))
      {
      continue;
      }

    QTableWidgetItem* selectItem = this->FileWidget->item(row, SelectColumn);
    QTableWidgetItem* nodeNameItem = this->FileWidget->item(row, NodeNameColumn);
    QTableWidgetItem* nodeStatusItem = this->FileWidget->item(row, NodeStatusColumn);

    Q_ASSERT(selectItem);
    Q_ASSERT(nodeNameItem);

    this->setStatusIcon(row, QIcon(), QString());

    if (this->CancelRequested)
      {
      continue;
      }

    // don't save unchecked nodes
    if (selectItem->checkState() != Qt::Checked ||
        this->FileWidget->isRowHidden(row))
      {
      continue;
      }

    // file properties
    QFileInfo file = this->file(row);
    QString format = this->format(row);
    qCjyxIOOptions* options = this->options(row);
    vtkDMMLNode* node = vtkDMMLNode::SafeDownCast(this->object(row));
    vtkDMMLStorableNode* const storableNode = vtkDMMLStorableNode::SafeDownCast(this->object(row));
    vtkDMMLStorageNode* snode = storableNode ? storableNode->GetStorageNode() : nullptr;

    if (snode)
      {
      snode->GetUserMessages()->ClearMessages();
      }

    if (file.fileName().isEmpty())
      {
      this->setStatusIcon(row, this->ErrorIcon, tr("Node %1 not saved, file name is empty.").arg(nodeNameItem->text()));
      doneWithSaveDataDialog = false;
      continue;
      }

    // check if the file already exists
    if (file.exists())
      {
      if (!this->confirmOverwrite(file.absoluteFilePath()))
        {
        this->setStatusIcon(row, this->ErrorIcon, tr("Node %1 was not saved because user chose not to overwrite existing file: %2.")
          .arg(nodeNameItem->text())
          .arg(file.absoluteFilePath()) );
        doneWithSaveDataDialog = false;
        continue;
        }
      }

    // save the node
    qCjyxCoreIOManager* coreIOManager =
      qCjyxCoreApplication::application()->coreIOManager();
    Q_ASSERT(coreIOManager);

    qCjyxIO::IOFileType fileType = coreIOManager->fileWriterFileType(node, format);
    qCjyxIO::IOProperties savingParameters;
    if (options)
      {
      // options properties nodeID and fileName will be overwritten
      // \todo fileName is wrong as it contains an obsolete directory
      savingParameters = options->properties();
      }
    savingParameters["nodeID"] = QString(node->GetID());
    savingParameters["fileName"] = file.absoluteFilePath();
    savingParameters["fileFormat"] = format;

    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
    bool success = coreIOManager->saveNodes(fileType, savingParameters);
    QApplication::restoreOverrideCursor();

    // node has failed to be written
    // get storage node again because the writer plugin may replace the storage node with a different class
    snode = storableNode->GetStorageNode();
    if (!success)
      {
      if (snode)
        {
        // Make sure an error message is added if saving returns with error
        snode->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
          (tr("Cannot write data file: %1.").arg(file.absoluteFilePath())).toStdString());
        this->updateStatusIconFromStorageNode(row, success);
        }
      else
        {
        this->setStatusIcon(row, this->ErrorIcon, tr("Failed to save node %1 to file %2.")
          .arg(nodeNameItem->text())
          .arg(file.absoluteFilePath()));
        }
      doneWithSaveDataDialog = false;
      continue;
      }

    // Display any warning or error messages from the storage node
    if (snode)
      {
      if (snode->GetUserMessages()->GetNumberOfMessages() > 0)
        {
        doneWithSaveDataDialog = false;
        }
      this->updateStatusIconFromStorageNode(row, success);
      }

    selectItem->setCheckState(
      storableNode->GetModifiedSinceRead() ? Qt::Checked : Qt::Unchecked);

    selectItem->setCheckState(Qt::Unchecked);
    nodeStatusItem->setText("Not Modified");
    }
  this->updateSize();

  return doneWithSaveDataDialog;
}

//-----------------------------------------------------------------------------
QFileInfo qCjyxSaveDataDialogPrivate::file(int row)const
{
  QTableWidgetItem* fileNameItem = this->FileWidget->item(row, FileNameColumn);
  ctkPathLineEdit* fileDirectoryEdit = qobject_cast<ctkPathLineEdit*>(
    this->FileWidget->cellWidget(row, FileDirectoryColumn));

  if (!fileNameItem || !fileDirectoryEdit)
    {
    qCritical() << Q_FUNC_INFO << " failed: filename or directory button not found";
    return QFileInfo();
    }

  QDir directory = fileDirectoryEdit->currentPath();
  return QFileInfo(directory, fileNameItem->text());
}

//-----------------------------------------------------------------------------
vtkObject* qCjyxSaveDataDialogPrivate::object(int row)const
{
  if (this->type(row) == tr("Scene"))
    {
    return this->DMMLScene;
    }
  QTableWidgetItem* nodeNameItem = this->FileWidget->item(row, NodeNameColumn);
  Q_ASSERT(nodeNameItem);

  /// \todo support dmmlScene row
  QStringList nodeIDs = nodeNameItem->data(Qt::ToolTipRole).toString().split(" ");
  vtkDMMLNode *node = this->getNodeByID(nodeIDs[0].toUtf8().data());
  return node;
}

//-----------------------------------------------------------------------------
vtkDMMLNode* qCjyxSaveDataDialogPrivate::getNodeByID(char *id)const
{
  return qCjyxSaveDataDialogPrivate::getNodeByID(id, this->DMMLScene);
}

//-----------------------------------------------------------------------------
vtkDMMLNode* qCjyxSaveDataDialogPrivate::getNodeByID(char *id, vtkDMMLScene* scene)
{
  vtkDMMLNode *node = scene->GetNodeByID(id);
  if (node == nullptr)
    {
    // search in SceneView nodes
    std::string sID(id);
    std::vector<vtkDMMLNode *> nodes;
    scene->GetNodesByClass("vtkDMMLSceneViewNode", nodes);
    std::vector<vtkDMMLNode *>::iterator it;

    for (it = nodes.begin(); it != nodes.end(); it++)
      {
      vtkDMMLSceneViewNode *svNode = vtkDMMLSceneViewNode::SafeDownCast(*it);
      // skip "Master Scene View" since it contains the same nodes as the scene
      if (svNode->GetName() && std::string("Master Scene View") == std::string(svNode->GetName()))
        {
        continue;
        }
      std::vector<vtkDMMLNode *> snodes;
      svNode->GetNodesByClass("vtkDMMLStorableNode", snodes);
      std::vector<vtkDMMLNode *>::iterator sit;
      for (sit = snodes.begin(); sit != snodes.end(); sit++)
        {
        vtkDMMLNode* snode = (*sit);
        if (std::string(snode->GetID()) == sID)
          {
          return snode;
          }
        }
      }
    }
  return node;
}

//-----------------------------------------------------------------------------
QString qCjyxSaveDataDialogPrivate::format(int row)const
{
  QComboBox* fileFormatComboBox = qobject_cast<QComboBox*>(
    this->FileWidget->cellWidget(row, FileFormatColumn));
  Q_ASSERT(fileFormatComboBox);
  return fileFormatComboBox->currentText();
}

//-----------------------------------------------------------------------------
QString qCjyxSaveDataDialogPrivate::type(int row)const
{
  QTableWidgetItem* typeItem = this->FileWidget->item(row, NodeTypeColumn);
  Q_ASSERT(typeItem);
  return typeItem->data(Self::SceneTypeRole).toString();
}

//-----------------------------------------------------------------------------
qCjyxIOOptions* qCjyxSaveDataDialogPrivate::options(int row)const
{
  qCjyxIOOptionsWidget* optionsWidget = qobject_cast<qCjyxIOOptionsWidget*>(
    this->FileWidget->cellWidget(row, OptionsColumn));
  return optionsWidget;
}

//-----------------------------------------------------------------------------
bool qCjyxSaveDataDialogPrivate::mustSceneBeSaved()const
{
  QAbstractItemModel* model = this->FileWidget->model();
  QModelIndexList found = model->match(
    model->index(0, NodeTypeColumn),
    Self::SceneTypeRole, QString("Scene"), 1, Qt::MatchExactly);
  if (found.count() == 0 || !found[0].isValid())
    {
    return false;
    }
  QTableWidgetItem* selectItem =
    this->FileWidget->item(found[0].row(), SelectColumn);

  return selectItem->checkState() == Qt::Checked;
}

//-----------------------------------------------------------------------------
int qCjyxSaveDataDialogPrivate::findSceneRow()const
{
  QAbstractItemModel* model = this->FileWidget->model();
  QModelIndexList found = model->match(
    model->index(0, NodeTypeColumn),
    Self::SceneTypeRole, QString("Scene"), 1, Qt::MatchExactly);
  return found[0].row();
}

//-----------------------------------------------------------------------------
QFileInfo qCjyxSaveDataDialogPrivate::sceneFile()const
{
  // search the scene
  int sceneRow = this->findSceneRow();

  QTableWidgetItem* fileNameItem = this->FileWidget->item(sceneRow, FileNameColumn);
  ctkPathLineEdit* fileDirectoryEdit = qobject_cast<ctkPathLineEdit*>(
    this->FileWidget->cellWidget(sceneRow, FileDirectoryColumn));

  QDir directory = fileDirectoryEdit->currentPath();
  QFileInfo file = QFileInfo(directory, fileNameItem->text());
  if (file.fileName().isEmpty())
    {
    file = QFileInfo(QString(this->DMMLScene->GetURL()));
    }
  return file;
}

//-----------------------------------------------------------------------------
QString qCjyxSaveDataDialogPrivate::sceneFileFormat()const
{
  return this->format(this->findSceneRow());
}

//-----------------------------------------------------------------------------
void qCjyxSaveDataDialogPrivate::setSceneRootDirectory(const QString& dir)
{
  this->DMMLScene->SetRootDirectory(dir.toUtf8());

  // update the root directory of scene snapshot nodes (not sure why)
  const int nnodes = this->DMMLScene->GetNumberOfNodesByClass("vtkDMMLSceneViewNode");
  for (int n=0; n<nnodes; n++)
    {
    vtkDMMLNode* node = this->DMMLScene->GetNthNodeByClass(n, "vtkDMMLSceneViewNode");
    vtkDMMLSceneViewNode *snode = vtkDMMLSceneViewNode::SafeDownCast(node);
    if (snode && snode->GetStoredScene())
      {
      snode->GetStoredScene()->SetRootDirectory(this->DMMLScene->GetRootDirectory());
      }
    }
}

//-----------------------------------------------------------------------------
bool qCjyxSaveDataDialogPrivate::saveScene()
{
  qCjyxIO::IOProperties properties;
  QFileInfo file = this->sceneFile();
  properties["fileName"] = file.absoluteFilePath();

  // create a screenShot of the full layout
  if (qCjyxApplication::application()->layoutManager())
    {
    QWidget* widget = qCjyxApplication::application()->layoutManager()->viewport();
    QImage screenShot = ctk::grabVTKWidget(widget);
    properties["screenShot"] = screenShot;
    }

  int row = this->findSceneRow();
  qCjyxIOOptions* options = this->options(row);
  if (options)
    {
    properties.unite(options->properties());
    }

  vtkNew<vtkDMMLMessageCollection> userMessages;
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  bool success = qCjyxApplication::application()->coreIOManager()->saveNodes(QString("SceneFile"), properties, userMessages);
  QApplication::restoreOverrideCursor();
  this->updateStatusIconFromMessageCollection(row, userMessages, success);
  if (success)
    {
    qCjyxCoreIOManager* coreIOManager = qCjyxCoreApplication::application()->coreIOManager();
    Q_ASSERT(coreIOManager);
    if (coreIOManager)
      {
      coreIOManager->setDefaultSceneFileType(this->sceneFileFormat());
      }
    }

  return success;
}

//-----------------------------------------------------------------------------
void qCjyxSaveDataDialogPrivate::selectModifiedData()
{
  const int rowCount = this->FileWidget->rowCount();
  for (int row = 0; row < rowCount; ++row)
    {
    QTableWidgetItem* statusItem = this->FileWidget->item(row, NodeStatusColumn);
    QTableWidgetItem* typeItem = this->FileWidget->item(row, NodeTypeColumn);
    Q_ASSERT(statusItem);
    Q_ASSERT(typeItem);
    QTableWidgetItem* selectItem = this->FileWidget->item(row, SelectColumn);
    Q_ASSERT(selectItem);
    selectItem->setCheckState(
      statusItem->text() == tr("Modified") &&
      typeItem->data(Self::SceneTypeRole).toString() != tr("Scene") ? Qt::Checked : Qt::Unchecked);
    }
}

//-----------------------------------------------------------------------------
void qCjyxSaveDataDialogPrivate::selectModifiedSceneData()
{
  // Select all the "modified" nodes.
  const int rowCount = this->FileWidget->rowCount();
  for (int row = 0; row < rowCount; ++row)
    {
    QTableWidgetItem* statusItem = this->FileWidget->item(row, NodeStatusColumn);
    Q_ASSERT(statusItem);
    if (statusItem->text() != tr("Modified"))
      {
      continue;
      }
    QTableWidgetItem* selectItem = this->FileWidget->item(row, SelectColumn);
    Q_ASSERT(selectItem);
    selectItem->setCheckState(
      statusItem->text() == tr("Modified") ? Qt::Checked : Qt::Unchecked);
    }
}

//-----------------------------------------------------------------------------
void qCjyxSaveDataDialogPrivate::saveSceneAsDataBundle()
{
  int sceneRow = this->findSceneRow();
  QComboBox* box = qobject_cast<QComboBox*>(
    this->FileWidget->cellWidget(sceneRow, FileFormatColumn));
  int mrbIndex = box->findText("mrb", Qt::MatchContains);
  int dmmlIndex = box->findText("dmml", Qt::MatchContains);
  // Toggle between scene data bundle entry and dmml entry
  if (mrbIndex != -1)
    {
    if (box->currentIndex() != mrbIndex)
      {
      box->setCurrentIndex(mrbIndex);
      }
    else
      {
      if (dmmlIndex != -1)
        {
        box->setCurrentIndex(dmmlIndex);
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qCjyxSaveDataDialogPrivate::formatChanged()
{
  // Search for the item whose format has changed
  QComboBox* formatComboBox = qobject_cast<QComboBox*>(this->sender());
  Q_ASSERT(formatComboBox);
  const int rowCount = this->FileWidget->rowCount();
  int row = 0;
  for (; row < rowCount; ++row)
    {
    if (formatComboBox == this->FileWidget->cellWidget(row, FileFormatColumn))
      {
      break;
      }
    }
  Q_ASSERT(row < rowCount);

  this->formatChanged(row);
}

//-----------------------------------------------------------------------------
void qCjyxSaveDataDialogPrivate::formatChanged(int row)
{
  QComboBox* formatComboBox = qobject_cast<QComboBox*>(
    this->FileWidget->cellWidget(row, FileFormatColumn));

  // In case the combobox was editable (hack to display custom text), we now
  // don't need this property anymore.
  formatComboBox->setEditable(false);

  QTableWidgetItem* fileNameItem = this->FileWidget->item(row, FileNameColumn);
  Q_ASSERT(fileNameItem);

  // Set the new selected extension to the file name
  QString extension = QString::fromStdString(vtkDataFileFormatHelper::GetFileExtensionFromFormatString(
    formatComboBox->currentText().toUtf8()));
  if (extension == "*")
    {
    extension = QString();
    }
  fileNameItem->setData(Self::FileExtensionRole, extension);

  // Update fileName based on new selected extension
  QString nodeID = fileNameItem->data(Self::UIDRole).toString();
  fileNameItem->setText(
        qCjyxFileNameItemDelegate::forceFileNameExtension(fileNameItem->text(), extension, this->DMMLScene, nodeID));

  // If the user changed the format, that means he wants to save the node
  // Select the row to mark the node to be saved.
  QTableWidgetItem* selectItem = this->FileWidget->item(row, SelectColumn);
  Q_ASSERT(selectItem);
  selectItem->setCheckState(Qt::Checked);

  this->updateOptionsWidget(row);
}

//-----------------------------------------------------------------------------
void qCjyxSaveDataDialogPrivate::updateOptionsWidget(int row)
{
  qCjyxCoreIOManager* coreIOManager =
    qCjyxCoreApplication::application()->coreIOManager();
  qCjyxIOOptions* options =
    coreIOManager->fileWriterOptions(this->object(row), this->format(row));
  qCjyxFileWriterOptionsWidget* optionsWidget =
    dynamic_cast<qCjyxFileWriterOptionsWidget*>(options);
  if (optionsWidget)
    {
    // The optionsWidget can use the filename to initialize some options.
    optionsWidget->setFileName(this->file(row).absoluteFilePath());
    optionsWidget->setObject(this->object(row));
    // TODO: support uneven rows. Until that day, we want to make sure the whole
    // widget is visible
    optionsWidget->setMinimumWidth(optionsWidget->sizeHint().width());
    // TODO: connect signal validChanged(bool) with the accept button
    }
  else
    {
    // we can't use options that are not widgets
    delete options;
    }
  this->FileWidget->setCellWidget(row, OptionsColumn, optionsWidget);
  this->FileWidget->resizeColumnToContents(OptionsColumn);
}

//-----------------------------------------------------------------------------
void qCjyxSaveDataDialogPrivate::updateStatusIconFromStorageNode(int row, bool success)
{
  vtkDMMLStorableNode * const node = vtkDMMLStorableNode::SafeDownCast(this->object(row));
  vtkDMMLMessageCollection* userMessages = nullptr;
  if (node && node->GetStorageNode())
    {
    userMessages = node->GetStorageNode()->GetUserMessages();
    }
  this->updateStatusIconFromMessageCollection(row, userMessages, success);
}

//-----------------------------------------------------------------------------
void qCjyxSaveDataDialogPrivate::updateStatusIconFromMessageCollection(int row, vtkDMMLMessageCollection* userMessages, bool success)
{
  QString messagesStr;
  bool warningFound = false;
  bool errorFound = false;
  if (userMessages)
    {
    messagesStr = QString::fromStdString(userMessages->GetAllMessagesAsString(&errorFound, &warningFound));
    if (errorFound)
      {
      qCritical() << Q_FUNC_INFO << "Data save error:" << messagesStr;
      }
    else if (warningFound)
      {
      qWarning() << Q_FUNC_INFO << "Data save warning:" << messagesStr;
      }
    else
      {
      qDebug() << Q_FUNC_INFO << "Data save information:" << messagesStr;
      }
    }
  this->setStatusIcon(row, (!success || errorFound) ? this->ErrorIcon : (warningFound ? this->WarningIcon : QIcon()), messagesStr);
}

//-----------------------------------------------------------------------------
void qCjyxSaveDataDialogPrivate::setStatusIcon(int row, const QIcon& icon, const QString& message)
{
  QTableWidgetItem* fileNameItem = this->FileWidget->item(row, FileNameColumn);
  fileNameItem->setIcon(icon);
  fileNameItem->setToolTip(message);
}

//-----------------------------------------------------------------------------
void qCjyxSaveDataDialogPrivate::showMoreColumns(bool show)
{
  this->FileWidget->setColumnHidden(OptionsColumn, !show);
  this->FileWidget->setColumnHidden(NodeNameColumn, !show);
  this->FileWidget->setColumnHidden(NodeTypeColumn, !show);
  this->FileWidget->setColumnHidden(NodeStatusColumn, !show);
  this->updateSize();

  qCjyxApplication::application()->userSettings()->setValue(SHOW_OPTIONS_SETTINGS_KEY, show);
}

//-----------------------------------------------------------------------------
void qCjyxSaveDataDialogPrivate::onItemChanged(QTableWidgetItem* widgetItem)
{
  if (widgetItem->column() != FileNameColumn)
    {
    return;
    }
  vtkDMMLScene* dmmlScene = this->DMMLScene;
  if (!dmmlScene)
    {
    return;
    }
  if (this->PopulatingItems)
    {
    return;
    }

  /// If filename is changed then we need to validate if it matches any of the supported
  /// file extension. If it does then update the file format selector.
  /// If it does not match any of the file extensions then the current file extension will
  /// be added (this way when the user just enters filename, the extension is added automatically).
  QTableWidgetItem* fileNameItem = this->FileWidget->item(widgetItem->row(), FileNameColumn);
  QString strippedFileName = qCjyxCoreIOManager::forceFileNameValidCharacters(fileNameItem->text());

  // Determine current file extension
  vtkObject* objectToSave = dmmlScene;
  QString nodeID = fileNameItem->data(Self::UIDRole).toString();
  if (!nodeID.isEmpty())
    {
    objectToSave = qCjyxSaveDataDialogPrivate::getNodeByID(nodeID.toUtf8().data(), dmmlScene);
    if (!objectToSave)
      {
      qCritical() << Q_FUNC_INFO << " failed: node not found by ID " << qPrintable(nodeID);
      return;
      }
    }
  qCjyxCoreIOManager* coreIOManager =
    qCjyxCoreApplication::application()->coreIOManager();
  QString currentExtension = coreIOManager->extractKnownExtension(strippedFileName, objectToSave);

  // Update file format selector according to current extension
  QComboBox* fileFormatsWidget = qobject_cast<QComboBox*>(this->FileWidget->cellWidget(widgetItem->row(), FileFormatColumn));
  int newFormat = fileFormatsWidget->findData(currentExtension);
  if (newFormat >= 0)
    {
    // current extension matches a supported format, update the format selector widget accordingly
    fileFormatsWidget->setCurrentIndex(newFormat);
    }
  else
    {
    // does not match any supported format, reset the filename
    this->formatChanged(widgetItem->row());
    }
}

//-----------------------------------------------------------------------------
void qCjyxSaveDataDialogPrivate::updateSize()
{
  // let's try to resize the columns according to their new contents
  this->FileWidget->resizeColumnsToContents();

  // Prevent collapsing the directory column
  this->FileWidget->horizontalHeader()->setMinimumSectionSize(40);

  this->FileWidget->horizontalHeader()->setSectionResizeMode(FileDirectoryColumn, QHeaderView::Stretch);
}

//-----------------------------------------------------------------------------
void qCjyxSaveDataDialogPrivate::onSceneFormatChanged()
{
  int sceneRow = this->findSceneRow();
  QComboBox* box = qobject_cast<QComboBox*>(
    this->FileWidget->cellWidget(sceneRow, FileFormatColumn));
  // Gray out all the nodes when saving scene as bundle
  this->enableNodes(box->currentIndex() == 0);
}

//-----------------------------------------------------------------------------
void qCjyxSaveDataDialogPrivate::enableNodes(bool enable)
{
  int sceneRow = this->findSceneRow();
  for (int i = 0; i < this->FileWidget->rowCount(); ++i)
    {
    if (i == sceneRow)
      {
      continue;
      }
    // When scene is saved as .mrb then item selection state,
    // file format, and options may be ignored. To indicate
    // this clearly to the users, we only show the scene in the table
    // and hide all the other storable nodes.
    if (enable)
      {
      this->FileWidget->showRow(i);
      }
    else
      {
      this->FileWidget->hideRow(i);
      }
    }
}

//-----------------------------------------------------------------------------
qCjyxSaveDataDialog::qCjyxSaveDataDialog(QObject* parentObject)
  : qCjyxFileDialog(parentObject)
  , d_ptr(new qCjyxSaveDataDialogPrivate(nullptr))
{
}

//-----------------------------------------------------------------------------
qCjyxSaveDataDialog::~qCjyxSaveDataDialog() = default;

//-----------------------------------------------------------------------------
qCjyxIO::IOFileType qCjyxSaveDataDialog::fileType()const
{
  // FIXME: not really a nofile, but more a collection of files
  return QString("NoFile");
}

//-----------------------------------------------------------------------------
QString qCjyxSaveDataDialog::description()const
{
  return tr("Any Data");
}

//-----------------------------------------------------------------------------
qCjyxFileDialog::IOAction qCjyxSaveDataDialog::action()const
{
  return qCjyxFileDialog::Write;
}

//-----------------------------------------------------------------------------
bool qCjyxSaveDataDialog::exec(const qCjyxIO::IOProperties& readerProperties)
{
  Q_D(qCjyxSaveDataDialog);
  Q_UNUSED(readerProperties);
  Q_ASSERT(!readerProperties.contains("fileName"));

  d->setDMMLScene(qCjyxCoreApplication::application()->dmmlScene());
  if (d->exec() != QDialog::Accepted)
    {
    return false;
    }
  return true;
}

//-----------------------------------------------------------------------------
bool qCjyxSaveDataDialogPrivate::confirmOverwrite(const QString& filepath)
{
  QMessageBox::StandardButton answer = this->ConfirmOverwriteAnswer;
  if (answer != QMessageBox::NoToAll && answer != QMessageBox::YesToAll)
  {
    answer = QMessageBox::question(this, tr("Saving file..."),
      tr("The file: %1 already exists. Do you want to replace it ?").arg(filepath),
      QMessageBox::Yes | QMessageBox::No |
      QMessageBox::YesToAll | QMessageBox::NoToAll | QMessageBox::Cancel,
      QMessageBox::Yes);
    if (answer == QMessageBox::YesToAll || answer == QMessageBox::NoToAll)
      {
      this->ConfirmOverwriteAnswer = answer;
      }
    if (answer == QMessageBox::Cancel)
      {
      this->CancelRequested = true;
      }
    }
  return (answer == QMessageBox::Yes || answer == QMessageBox::YesToAll);
}
