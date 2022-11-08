#ifndef __qCjyxSaveDataDialog_p_h
#define __qCjyxSaveDataDialog_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Cjyx API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// Qt includes
#include <QDialog>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QStyledItemDelegate>

// Cjyx includes
#include "qCjyxIOOptions.h"
#include "qCjyxSaveDataDialog.h"
#include "ui_qCjyxSaveDataDialog.h"

class ctkPathLineEdit;
class vtkDMMLNode;
class vtkDMMLStorableNode;
class vtkObject;

//-----------------------------------------------------------------------------
class qCjyxSaveDataDialogPrivate
  : public QDialog
  , public Ui_qCjyxSaveDataDialog
{
  Q_OBJECT
public:
  typedef qCjyxSaveDataDialogPrivate Self;
  explicit qCjyxSaveDataDialogPrivate(QWidget* _parent=nullptr);
  ~qCjyxSaveDataDialogPrivate() override;

  void populateItems();

  void setDMMLScene(vtkDMMLScene* scene);
  vtkDMMLScene* dmmlScene()const;

  /// Helper function for finding a node in the main scene and all scene view scenes.
  /// This method differs from vtkDMMLScene::GetNodeByID in that this method looks for
  /// node IDs in the internal scene view scenes as well.
  static vtkDMMLNode* getNodeByID(char *id, vtkDMMLScene* scene);

  void formatChanged(int row);

public slots:
  void setDirectory(const QString& newDirectory);
  void selectModifiedSceneData();
  void selectModifiedData();
  bool save();
  /// Reimplemented from QDialog::accept(), only accept the dialog if
  /// save() is successful.
  void accept() override;

protected slots:
  void formatChanged();
  bool saveScene();
  bool saveNodes();
  QFileInfo sceneFile()const; // ### Cjyx 4.4: Move as protected
  void showMoreColumns(bool);
  void updateSize();
  void onSceneFormatChanged();
  void enableNodes(bool);
  void saveSceneAsDataBundle();
  void onItemChanged(QTableWidgetItem*);

protected:
  enum ColumnType
  {
    SelectColumn = 0,
    FileNameColumn = 0,
    FileFormatColumn = 1,
    FileDirectoryColumn = 2,
    OptionsColumn = 3,
    NodeNameColumn = 4,
    NodeTypeColumn = 5,
    NodeStatusColumn = 6
  };

  enum CustomRole
  {
    SceneTypeRole = Qt::UserRole,
    FileExtensionRole,
    UIDRole
  };

  int               findSceneRow()const;
  bool              mustSceneBeSaved()const;
  void              setSceneRootDirectory(const QString& rootDirectory);
  void              updateOptionsWidget(int row);
  void              updateStatusIconFromStorageNode(int row, bool success);
  void              updateStatusIconFromMessageCollection(int row, vtkDMMLMessageCollection* userMessages, bool success);
  void              setStatusIcon(int row, const QIcon& icon, const QString& message);

  QString           sceneFileFormat()const;

  void              populateScene();
  void              populateNode(vtkDMMLNode* node);

  QFileInfo         nodeFileInfo(vtkDMMLStorableNode* node);
  QTableWidgetItem* createNodeNameItem(vtkDMMLStorableNode* node);
  QTableWidgetItem* createNodeTypeItem(vtkDMMLStorableNode* node);
  QTableWidgetItem* createNodeStatusItem(vtkDMMLStorableNode* node, const QFileInfo& fileInfo);
  QWidget*          createFileFormatsWidget(vtkDMMLStorableNode* node, QFileInfo& fileInfo);
  QTableWidgetItem* createFileNameItem(const QFileInfo& fileInfo, const QString& extension, const QString& nodeID);
  ctkPathLineEdit*  createFileDirectoryWidget(const QFileInfo& fileInfo);
  void              clearUserMessagesInStorageNodes();

  QFileInfo         file(int row)const;
  vtkObject*        object(int row)const;
  QString           format(int row)const;
  QString           type(int row)const;
  qCjyxIOOptions* options(int row)const;

  bool confirmOverwrite(const QString& filepath);

  /// Helper function for finding a node in the main scene and all scene view scenes
  vtkDMMLNode*      getNodeByID(char *id)const;

  vtkDMMLScene* DMMLScene;
  QString DMMLSceneRootDirectoryBeforeSaving;

  // Items are currently being added to the scene, indicates that no GUI updates should be performed.
  bool PopulatingItems;

  QMessageBox::StandardButton ConfirmOverwriteAnswer;
  bool CancelRequested;
  QIcon WarningIcon;
  QIcon ErrorIcon;

  friend class qCjyxFileNameItemDelegate;
};

//-----------------------------------------------------------------------------
class qCjyxFileNameItemDelegate : public QStyledItemDelegate
{
public:
  typedef QStyledItemDelegate Superclass;
  qCjyxFileNameItemDelegate( QObject * parent = nullptr );
  static QString forceFileNameExtension(const QString& fileName, const QString& extension,
                               vtkDMMLScene *dmmlScene, const QString &nodeID);

  vtkDMMLScene* DMMLScene;
};

#endif
