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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qCjyxExtensionsManagerWidget_h
#define __qCjyxExtensionsManagerWidget_h

// CTK includes
#include <ctkErrorLogLevel.h>

// Qt includes
#include <QUrl>
#include <QWidget>

// QtGUI includes
#include "qCjyxBaseQTGUIExport.h"

// CTK includes
#include <ctkErrorLogLevel.h>

class qCjyxExtensionsManagerWidgetPrivate;
class qCjyxExtensionsManagerModel;

class Q_CJYX_BASE_QTGUI_EXPORT qCjyxExtensionsManagerWidget
  : public QWidget
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef QWidget Superclass;

  /// Constructor
  explicit qCjyxExtensionsManagerWidget(QWidget* parent = nullptr);

  /// Destructor
  ~qCjyxExtensionsManagerWidget() override;

  Q_INVOKABLE qCjyxExtensionsManagerModel* extensionsManagerModel()const;
  Q_INVOKABLE void setExtensionsManagerModel(qCjyxExtensionsManagerModel* model);

  /// Shows a popup if operations are still in progress, asking if the user wants to stop them.
  bool confirmClose();

  bool isInBatchProcessing();

signals:

  /// If this signal is emitted when entering/exiting batch processing mode.
  /// In batch mode the user should not be able to allowed to quite the extensions manager.
  void inBatchProcessing(bool batch);

public slots:
  void refreshInstallWidget();

  // Request update state of automatic extension update check and install checkbox states
  void updateAutoUpdateWidgetsFromModel();

protected slots:
  void onModelUpdated();

  void onCurrentTabChanged(int index);

  void onManageLinkActivated(const QUrl& link);
  void onManageUrlChanged(const QUrl& newUrl);
  void onInstallUrlChanged(const QUrl& newUrl);
  void onSearchTextChanged(const QString& newText);

  void onCheckForUpdatesTriggered();
  void onEditBookmarksTriggered();
  void onInstallUpdatesTriggered();
  void onInstallBookmarkedTriggered();
  void onInstallFromFileTriggered();

  void setAutoUpdateCheck(bool toggle);
  void setAutoUpdateInstall(bool toggle);
  void setAutoInstallDependencies(bool toggle);

  void onMessageLogged(const QString& text, ctkErrorLogLevel::LogLevels level);
  void onMessagesAcknowledged();

protected:
  void timerEvent(QTimerEvent*) override;

  void processSearchTextChange();

  QScopedPointer<qCjyxExtensionsManagerWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qCjyxExtensionsManagerWidget);
  Q_DISABLE_COPY(qCjyxExtensionsManagerWidget);
};

#endif
