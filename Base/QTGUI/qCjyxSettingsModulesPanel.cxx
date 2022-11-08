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
#include <QMainWindow>
#include <QSettings>

// QtGUI includes
#include "qCjyxApplication.h"
#include "qCjyxCoreCommandOptions.h"
#include "qCjyxModuleFactoryFilterModel.h"
#include "qCjyxModuleFactoryManager.h"
#include "qCjyxModuleManager.h"
#include "qCjyxModulesMenu.h"
#include "qCjyxModuleSelectorToolBar.h"
#include "qCjyxRelativePathMapper.h"
#include "qCjyxSettingsModulesPanel.h"
#include "ui_qCjyxSettingsModulesPanel.h"

// --------------------------------------------------------------------------
// qCjyxSettingsModulesPanelPrivate

//-----------------------------------------------------------------------------
class qCjyxSettingsModulesPanelPrivate: public Ui_qCjyxSettingsModulesPanel
{
  Q_DECLARE_PUBLIC(qCjyxSettingsModulesPanel);
protected:
  qCjyxSettingsModulesPanel* const q_ptr;

public:
  qCjyxSettingsModulesPanelPrivate(qCjyxSettingsModulesPanel& object);
  void init();

  qCjyxModulesMenu* ModulesMenu;
  QStringList ModulesToAlwaysIgnore;
};

// --------------------------------------------------------------------------
// qCjyxSettingsModulesPanelPrivate methods

// --------------------------------------------------------------------------
qCjyxSettingsModulesPanelPrivate::qCjyxSettingsModulesPanelPrivate(qCjyxSettingsModulesPanel& object)
  :q_ptr(&object)
{
  this->ModulesMenu = nullptr;
}

// --------------------------------------------------------------------------
void qCjyxSettingsModulesPanelPrivate::init()
{
  Q_Q(qCjyxSettingsModulesPanel);

  this->setupUi(q);

  qCjyxCoreApplication * coreApp = qCjyxCoreApplication::application();
  qCjyxAbstractModuleFactoryManager* factoryManager = coreApp->moduleManager()->factoryManager();

  // Show Hidden
  QObject::connect(this->ShowHiddenModulesCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(onShowHiddenModulesChanged(bool)));

  // Additional module paths
  this->AdditionalModulePathMoreButton->setChecked(false);

  // Modules
  qCjyxModuleFactoryFilterModel* filterModel =
    this->DisableModulesListView->filterModel();
  QObject::connect(this->FilterToLoadPushButton, SIGNAL(toggled(bool)),
                   filterModel, SLOT(setShowToLoad(bool)));
  QObject::connect(this->FilterToIgnorePushButton, SIGNAL(toggled(bool)),
                   filterModel, SLOT(setShowToIgnore(bool)));
  QObject::connect(this->FilterLoadedPushButton, SIGNAL(toggled(bool)),
                   filterModel, SLOT(setShowLoaded(bool)));
  QObject::connect(this->FilterIgnoredPushButton, SIGNAL(toggled(bool)),
                   filterModel, SLOT(setShowIgnored(bool)));
  QObject::connect(this->FilterFailedPushButton, SIGNAL(toggled(bool)),
                   filterModel, SLOT(setShowFailed(bool)));
  QObject::connect(this->FilterTitleSearchBox, SIGNAL(textChanged(QString)),
                   filterModel, SLOT(setFilterFixedString(QString)));

  this->FilterMoreButton->setChecked(false); // hide filters by default

  // Home
  this->ModulesMenu = new qCjyxModulesMenu(q);
  this->ModulesMenu->setDuplicateActions(true);
  this->HomeModuleButton->setMenu(this->ModulesMenu);
  QObject::connect(this->ModulesMenu, SIGNAL(currentModuleChanged(QString)),
                   q, SLOT(onHomeModuleChanged(QString)));
  this->ModulesMenu->setModuleManager(coreApp->moduleManager());

  // Favorites
  this->FavoritesModulesListView->filterModel()->setHideAllWhenShowModulesIsEmpty(true);
  this->FavoritesMoveLeftButton->setIcon(q->style()->standardIcon(QStyle::SP_ArrowLeft));
  this->FavoritesMoveRightButton->setIcon(q->style()->standardIcon(QStyle::SP_ArrowRight));
  QObject::connect(this->FavoritesRemoveButton, SIGNAL(clicked()),
                   this->FavoritesModulesListView, SLOT(hideSelectedModules()));
  QObject::connect(this->FavoritesMoveLeftButton, SIGNAL(clicked()),
                   this->FavoritesModulesListView, SLOT(moveLeftSelectedModules()));
  QObject::connect(this->FavoritesMoveRightButton, SIGNAL(clicked()),
                   this->FavoritesModulesListView, SLOT(moveRightSelectedModules()));
  QObject::connect(this->FavoritesMoreButton, SIGNAL(toggled(bool)),
                   this->FavoritesModulesListView, SLOT(scrollToSelectedModules()));
  this->FavoritesMoreButton->setChecked(false);

  // Default values
  this->TemporaryDirectoryButton->setDirectory(coreApp->defaultTemporaryPath());
  this->DisableModulesListView->setFactoryManager( factoryManager );
  this->FavoritesModulesListView->setFactoryManager( factoryManager );

  this->ModulesMenu->setCurrentModule(Cjyx_DEFAULT_HOME_MODULE);

  // Cjyx_DEFAULT_FAVORITE_MODULES contains module names in a comma-separated list
  // (chosen this format because the same format is used for storing the favorites list in the .ini file).
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
  QStringList favoritesRaw = QString(Cjyx_DEFAULT_FAVORITE_MODULES).split(",", Qt::SkipEmptyParts);
#else
  QStringList favoritesRaw = QString(Cjyx_DEFAULT_FAVORITE_MODULES).split(",", QString::SkipEmptyParts);
#endif
  // The separator commas have been removed, but we also need need to remove leading and trailing spaces from the retrieved names.
  QStringList favorites;
  foreach(QString s, favoritesRaw)
    {
    favorites << s.trimmed();
    }
  this->FavoritesModulesListView->filterModel()->setShowModules(favorites);

  // Register settings
  q->registerProperty("disable-loadable-modules", this->LoadLoadableModulesCheckBox,
                      "checked", SIGNAL(toggled(bool)));
  q->registerProperty("disable-scripted-loadable-modules", this->LoadScriptedLoadableModulesCheckBox,
                      "checked", SIGNAL(toggled(bool)));
  q->registerProperty("disable-cli-modules", this->LoadCommandLineModulesCheckBox,
                      "checked", SIGNAL(toggled(bool)));

  q->registerProperty("disable-builtin-loadable-modules", this->LoadBuiltInLoadableModulesCheckBox,
                      "checked", SIGNAL(toggled(bool)));
  q->registerProperty("disable-builtin-scripted-loadable-modules", this->LoadBuiltInScriptedLoadableModulesCheckBox,
                      "checked", SIGNAL(toggled(bool)));
  q->registerProperty("disable-builtin-cli-modules", this->LoadBuiltInCommandLineModulesCheckBox,
                      "checked", SIGNAL(toggled(bool)));

  q->registerProperty("Modules/HomeModule", this->ModulesMenu,
                      "currentModule", SIGNAL(currentModuleChanged(QString)));
  q->registerProperty("Modules/FavoriteModules", this->FavoritesModulesListView->filterModel(),
                      "showModules", SIGNAL(showModulesChanged(QStringList)));
  qCjyxRelativePathMapper* relativePathMapper = new qCjyxRelativePathMapper(
    this->TemporaryDirectoryButton, "directory", SIGNAL(directoryChanged(QString)));
  q->registerProperty("TemporaryPath", relativePathMapper,
                      "relativePath", SIGNAL(relativePathChanged(QString)));
  q->registerProperty("Modules/ShowHiddenModules", this->ShowHiddenModulesCheckBox,
                      "checked", SIGNAL(toggled(bool)));
  qCjyxRelativePathMapper* relativePathMapper2 = new qCjyxRelativePathMapper(
    this->AdditionalModulePathsView, "directoryList", SIGNAL(directoryListChanged()));
  q->registerProperty("Modules/AdditionalPaths", relativePathMapper2,
                      "relativePaths", SIGNAL(relativePathsChanged(QStringList)),
                      "Additional module paths", ctkSettingsPanel::OptionRequireRestart,
                      coreApp->revisionUserSettings());

  this->ModulesToAlwaysIgnore = coreApp->revisionUserSettings()->value("Modules/IgnoreModules").toStringList();
  emit q->modulesToAlwaysIgnoreChanged(this->ModulesToAlwaysIgnore);

  q->registerProperty("Modules/IgnoreModules", q,
                      "modulesToAlwaysIgnore", SIGNAL(modulesToAlwaysIgnoreChanged(QStringList)),
                      "Modules to ignore", ctkSettingsPanel::OptionRequireRestart,
                      coreApp->revisionUserSettings());
  QObject::connect(factoryManager, SIGNAL(modulesToIgnoreChanged(QStringList)),
                   q, SLOT(setModulesToAlwaysIgnore(QStringList)));

  // Actions to propagate to the application when settings are changed
  QObject::connect(this->TemporaryDirectoryButton, SIGNAL(directoryChanged(QString)),
                   q, SLOT(onTemporaryPathChanged(QString)));
  QObject::connect(this->AdditionalModulePathsView, SIGNAL(directoryListChanged()),
                   q, SLOT(onAdditionalModulePathsChanged()));

  // Connect AdditionalModulePaths buttons
  QObject::connect(this->AddAdditionalModulePathButton, SIGNAL(clicked()),
                   q, SLOT(onAddModulesAdditionalPathClicked()));
  QObject::connect(this->RemoveAdditionalModulePathButton, SIGNAL(clicked()),
                   q, SLOT(onRemoveModulesAdditionalPathClicked()));
}

// --------------------------------------------------------------------------
// qCjyxSettingsModulesPanel methods

// --------------------------------------------------------------------------
qCjyxSettingsModulesPanel::qCjyxSettingsModulesPanel(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qCjyxSettingsModulesPanelPrivate(*this))
{
  Q_D(qCjyxSettingsModulesPanel);
  d->init();
}

// --------------------------------------------------------------------------
qCjyxSettingsModulesPanel::~qCjyxSettingsModulesPanel() = default;

//-----------------------------------------------------------------------------
void qCjyxSettingsModulesPanel::setModulesToAlwaysIgnore(const QStringList& moduleNames)
{
  Q_D(qCjyxSettingsModulesPanel);

  // This slot is called in two cases:
  //
  // (1) each time the signal qCjyxAbstractModuleFactoryManager::modulesToIgnore
  // is invoked.
  //
  // (2) each time the default settings are restored.
  //
  // To ensure the module names specified using the "--modules-to-ignore"
  // command line arguments are not saved in the settings, this  slot
  // will emit the "onModulesToAlwaysIgnoreChanged()" with an updated
  // list.

  if (d->ModulesToAlwaysIgnore == moduleNames)
    {
    return;
    }

  // Ensure the ModulesListView observing the factoryManager is updated
  // when settings are restored.
  qCjyxCoreApplication * coreApp = qCjyxCoreApplication::application();
  coreApp->moduleManager()->factoryManager()->setModulesToIgnore(moduleNames);

  // Update the list of modules to ignore removing the one
  // specified from the command line.
  QStringList updatedModulesToAlwaysIgnore;
  foreach(const QString& moduleName, moduleNames)
    {
    if (!coreApp->coreCommandOptions()->modulesToIgnore().contains(moduleName))
      {
      updatedModulesToAlwaysIgnore.append(moduleName);
      }
    }

  if (d->ModulesToAlwaysIgnore == updatedModulesToAlwaysIgnore)
    {
    return;
    }

  d->ModulesToAlwaysIgnore = updatedModulesToAlwaysIgnore;

  emit modulesToAlwaysIgnoreChanged(updatedModulesToAlwaysIgnore);
}

//-----------------------------------------------------------------------------
QStringList qCjyxSettingsModulesPanel::modulesToAlwaysIgnore()const
{
  Q_D(const qCjyxSettingsModulesPanel);
  return d->ModulesToAlwaysIgnore;
}

// --------------------------------------------------------------------------
void qCjyxSettingsModulesPanel::onHomeModuleChanged(const QString& moduleName)
{
  Q_D(qCjyxSettingsModulesPanel);
  QAction* moduleAction = d->ModulesMenu->moduleAction(moduleName);
  Q_ASSERT(moduleAction);
  d->HomeModuleButton->setText(moduleAction->text());
  d->HomeModuleButton->setIcon(moduleAction->icon());
}

// --------------------------------------------------------------------------
void qCjyxSettingsModulesPanel::onTemporaryPathChanged(const QString& path)
{
  qCjyxCoreApplication::application()->setTemporaryPath(path);
}

// --------------------------------------------------------------------------
void qCjyxSettingsModulesPanel::onShowHiddenModulesChanged(bool show)
{
  QMainWindow* mainWindow = qCjyxApplication::application()->mainWindow();
  foreach (qCjyxModuleSelectorToolBar* toolBar,
           mainWindow->findChildren<qCjyxModuleSelectorToolBar*>())
    {
    toolBar->modulesMenu()->setShowHiddenModules(show);
    // refresh the list
    toolBar->modulesMenu()->setModuleManager(
      toolBar->modulesMenu()->moduleManager());
    }
}

// --------------------------------------------------------------------------
void qCjyxSettingsModulesPanel::onAdditionalModulePathsChanged()
{
  Q_D(qCjyxSettingsModulesPanel);
  d->RemoveAdditionalModulePathButton->setEnabled(
        d->AdditionalModulePathsView->directoryList().count() > 0);
}

// --------------------------------------------------------------------------
void qCjyxSettingsModulesPanel::onAddModulesAdditionalPathClicked()
{
  Q_D(qCjyxSettingsModulesPanel);
  qCjyxCoreApplication * coreApp = qCjyxCoreApplication::application();
  QString mostRecentPath = coreApp->toCjyxHomeAbsolutePath(
    coreApp->revisionUserSettings()->value("Modules/MostRecentlySelectedPath").toString());
  QString path = QFileDialog::getExistingDirectory(
        this, tr("Select folder"),
        mostRecentPath);
  // An empty directory means that the user cancelled the dialog.
  if (path.isEmpty())
    {
    return;
    }
  d->AdditionalModulePathsView->addDirectory(path);
  coreApp->revisionUserSettings()->setValue("Modules/MostRecentlySelectedPath",
    coreApp->toCjyxHomeRelativePath(path));
}

// --------------------------------------------------------------------------
void qCjyxSettingsModulesPanel::onRemoveModulesAdditionalPathClicked()
{
  Q_D(qCjyxSettingsModulesPanel);
  // Remove all selected
  d->AdditionalModulePathsView->removeSelectedDirectories();
}
