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

#include "qCjyxApplicationHelper.h"

// Qt includes
#include <QFont>
#include <QLabel>
#include <QSettings>
#include <QSysInfo>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>

// Cjyx includes
#include "qCjyxApplication.h"
#ifdef Cjyx_BUILD_CLI_SUPPORT
# include "qCjyxCLIExecutableModuleFactory.h"
# include "qCjyxCLILoadableModuleFactory.h"
#endif
#include "qCjyxCommandOptions.h"
#include "qCjyxCoreModuleFactory.h"
#include "qCjyxLoadableModuleFactory.h"
#include "qCjyxModuleFactoryManager.h"
#include "qCjyxModuleManager.h"

#ifdef Cjyx_USE_PYTHONQT
# include "qCjyxScriptedLoadableModuleFactory.h"
#endif

#include <vtkSystemInformation.h>

// CTK includes
#include <ctkMessageBox.h>
#include <ctkProxyStyle.h>
#ifdef Cjyx_USE_PYTHONQT
# include <ctkPythonConsole.h>
#endif

// DMMLWidgets includes
#include <qDMMLEventLoggerWidget.h>
#include <qDMMLWidget.h>

// ITK includes
#include <itkFactoryRegistration.h>

// VTK includes
#include <vtksys/SystemTools.hxx>
#include <vtkNew.h>
#include <vtkLogger.h>

// PythonQt includes
#ifdef Cjyx_USE_PYTHONQT
# include <PythonQtObjectPtr.h>
# include <PythonQtPythonInclude.h>
#endif

#ifdef _WIN32
#include <Windows.h> //for SHELLEXECUTEINFO
#endif

//----------------------------------------------------------------------------
qCjyxApplicationHelper::qCjyxApplicationHelper(QObject * parent) : Superclass(parent)
{
}

//----------------------------------------------------------------------------
qCjyxApplicationHelper::~qCjyxApplicationHelper() = default;

//----------------------------------------------------------------------------
void qCjyxApplicationHelper::preInitializeApplication(
    const char* argv0, ctkProxyStyle* style)
{
  vtkLogger::SetStderrVerbosity(vtkLogger::VERBOSITY_OFF);
  itk::itkFactoryRegistration();
  qDMMLWidget::preInitializeApplication();

  // Allow a custom application name so that the settings
  // can be distinct for differently named applications
  QString applicationName("Cjyx");
  if (argv0)
    {
    std::string name = vtksys::SystemTools::GetFilenameWithoutExtension(argv0);
    applicationName = QString::fromLocal8Bit(name.c_str());
    applicationName.remove(QString("App-real"));
    }
  QCoreApplication::setApplicationName(applicationName);

  QCoreApplication::setApplicationVersion(Cjyx_MAIN_PROJECT_VERSION_FULL);
  //vtkObject::SetGlobalWarningDisplay(false);
  QApplication::setDesktopSettingsAware(false);
  if (style)
    {
    QApplication::setStyle(style);
    }

  qDMMLWidget::postInitializeApplication();
}

//----------------------------------------------------------------------------
void qCjyxApplicationHelper::setupModuleFactoryManager(qCjyxModuleFactoryManager * moduleFactoryManager)
{
  qCjyxApplication* app = qCjyxApplication::application();
  // Register module factories
  moduleFactoryManager->registerFactory(new qCjyxCoreModuleFactory);

  qCjyxCommandOptions* options = qCjyxApplication::application()->commandOptions();

  if(options->disableModules())
    {
    return;
    }

  if (!options->disableLoadableModules())
    {
    moduleFactoryManager->registerFactory(new qCjyxLoadableModuleFactory);
    if (!options->disableBuiltInModules() &&
        !options->disableBuiltInLoadableModules() &&
        !options->runPythonAndExit())
      {
      QString loadablePath = app->cjyxHome() + "/" + Cjyx_QTLOADABLEMODULES_LIB_DIR + "/";
      moduleFactoryManager->addSearchPath(loadablePath);
      // On Win32, *both* paths have to be there, since scripts are installed
      // in the install location, and exec/libs are *automatically* installed
      // in intDir.
      moduleFactoryManager->addSearchPath(loadablePath + app->intDir());
      }
    }

#ifdef Cjyx_USE_PYTHONQT
  if (!options->disableScriptedLoadableModules())
    {
    moduleFactoryManager->registerFactory(
      new qCjyxScriptedLoadableModuleFactory);
    if (!options->disableBuiltInModules() &&
        !options->disableBuiltInScriptedLoadableModules() &&
        !qCjyxApplication::testAttribute(qCjyxApplication::AA_DisablePython) &&
        !options->runPythonAndExit())
      {
      QString scriptedPath = app->cjyxHome() + "/" + Cjyx_QTSCRIPTEDMODULES_LIB_DIR + "/";
      moduleFactoryManager->addSearchPath(scriptedPath);
      // On Win32, *both* paths have to be there, since scripts are installed
      // in the install location, and exec/libs are *automatically* installed
      // in intDir.
      moduleFactoryManager->addSearchPath(scriptedPath + app->intDir());
      }
    }
#endif

#ifdef Cjyx_BUILD_CLI_SUPPORT
  if (!options->disableCLIModules())
    {
    QString tempDirectory =
      qCjyxCoreApplication::application()->temporaryPath();

    // Always prefer executable CLIs. While launching a new process and transfer data via files may take slightly
    // longer, the file transfer is more robust, the CLI module can be stopped at any time (while a thread may be
    // requested to stop, but there is no way to force it to stop cleanly), errors in the CLI module cannot crash
    // the application, and startup time and memory usage is reduced by avoiding loading all CLI modules into the
    // main process. See more information in https://github.com/Slicer/Slicer/issues/4893.
    const bool preferExecutableCLIs = true;

    qCjyxCLILoadableModuleFactory* cliLoadableFactory = new qCjyxCLILoadableModuleFactory();
    cliLoadableFactory->setTempDirectory(tempDirectory);
    moduleFactoryManager->registerFactory(cliLoadableFactory, preferExecutableCLIs ? 0 : 1);

    qCjyxCLIExecutableModuleFactory* cliExecutableFactory = new qCjyxCLIExecutableModuleFactory();
    cliExecutableFactory->setTempDirectory(tempDirectory);
    moduleFactoryManager->registerFactory(cliExecutableFactory, preferExecutableCLIs ? 1 : 0);

    if (!options->disableBuiltInModules() &&
        !options->disableBuiltInCLIModules() &&
        !options->runPythonAndExit())
      {
      QString cliPath = app->cjyxHome() + "/" + Cjyx_CLIMODULES_LIB_DIR + "/";
      moduleFactoryManager->addSearchPath(cliPath);
      // On Win32, *both* paths have to be there, since scripts are installed
      // in the install location, and exec/libs are *automatically* installed
      // in intDir.
      moduleFactoryManager->addSearchPath(cliPath + app->intDir());
#ifdef Q_OS_MAC
      moduleFactoryManager->addSearchPath(app->cjyxHome() + "/" + Cjyx_CLIMODULES_SUBDIR);
#endif
      }
    }
#endif
  moduleFactoryManager->addSearchPaths(
    app->toCjyxHomeAbsolutePaths(app->revisionUserSettings()->value("Modules/AdditionalPaths").toStringList()));

  QStringList modulesToAlwaysIgnore =
    app->revisionUserSettings()->value("Modules/IgnoreModules").toStringList();
  QStringList modulesToTemporarlyIgnore = options->modulesToIgnore();
  // Discard modules already listed in the settings
  foreach(const QString& moduleToAlwaysIgnore, modulesToAlwaysIgnore)
    {
    modulesToTemporarlyIgnore.removeAll(moduleToAlwaysIgnore);
    }
  QStringList modulesToIgnore = modulesToAlwaysIgnore << modulesToTemporarlyIgnore;
  moduleFactoryManager->setModulesToIgnore(modulesToIgnore);

  moduleFactoryManager->setVerboseModuleDiscovery(app->commandOptions()->verboseModuleDiscovery());
}

//----------------------------------------------------------------------------
void qCjyxApplicationHelper::showDMMLEventLoggerWidget()
{
  qDMMLEventLoggerWidget* logger = new qDMMLEventLoggerWidget(nullptr);
  logger->setAttribute(Qt::WA_DeleteOnClose);
  logger->setConsoleOutputEnabled(false);
  logger->setDMMLScene(qCjyxApplication::application()->dmmlScene());

  QObject::connect(qCjyxApplication::application(),
                   SIGNAL(dmmlSceneChanged(vtkDMMLScene*)),
                   logger,
                   SLOT(setDMMLScene(vtkDMMLScene*)));

  logger->show();
}

//----------------------------------------------------------------------------
bool qCjyxApplicationHelper::checkRenderingCapabilities()
{
  vtkNew<vtkSystemInformation> systemInfo;
  systemInfo->RunRenderingCheck();
  if (systemInfo->GetRenderingCapabilities() & vtkSystemInformation::OPENGL)
    {
    return true;
    }

  qWarning("Graphics capability of this computer is not sufficient to run this application");

  QString message = tr("Graphics capability of this computer is not sufficient to "
    "run this application. The application most likely will not function properly.");

  QString details = tr(
    "See more information and help at:\n%1/user_guide/get_help.html#cjyx-application-does-not-start\n\n"
    "Graphics capabilities of this computer:\n\n")
    .arg(qCjyxApplication::application()->documentationBaseUrl());
  details += systemInfo->GetRenderingCapabilitiesDetails().c_str();

  ctkMessageBox *messageBox = new ctkMessageBox(nullptr);
  messageBox->setAttribute(Qt::WA_DeleteOnClose, true);
  messageBox->setIcon(QMessageBox::Warning);
  messageBox->setWindowTitle(tr("Insufficient graphics capability"));
  messageBox->setText(message);
  messageBox->setDetailedText(details);
#if defined(_WIN32)
  // Older versions of Windows Remote Desktop protocol (RDP) makes the system report lower
  // OpenGL capability than the actual capability is (when the system is used locally).
  // On these systems, Cjyx cannot be started while an RDP connection is active,
  // but an already started Cjyx can be operated without problems.
  // Retry option allows delayed restart of Cjyx through remote connection.
  // There is no need to offer "retry" option on other operating systems.
  messageBox->setStandardButtons(QMessageBox::Close | QMessageBox::Ignore | QMessageBox::Retry);
#else
  messageBox->setStandardButtons(QMessageBox::Close | QMessageBox::Ignore);
#endif
  messageBox->setDefaultButton(QMessageBox::Close);
  int result = messageBox->exec();

#if defined(_WIN32)
  if (result == QMessageBox::Retry)
    {
    // This option is for restarting the application outside of a
    // remote desktop session (during remote desktop sessions, system
    // may report lower OpenGL capabilities).

    // Run tscon system tool to create a new session, which terminates
    // the existing session (closes remote desktop connection).
    qCjyxApplicationHelper::runAsAdmin("tscon.exe", "1 /dest:console");

    QApplication::processEvents();

    // By now the remote desktop session is terminated, we restart
    // the application in a normal local desktop session.
    qCjyxApplication::restart();
    }
#endif

  return (result == QMessageBox::Ignore);
}

//----------------------------------------------------------------------------
int qCjyxApplicationHelper::runAsAdmin(QString executable, QString parameters/*=QString()*/, QString workingDir/*=QString()*/)
{
#if defined(_WIN32)
  // Run tscon system tool to create a new session, which terminates
  // the existing session (closes remote desktop connection).
  SHELLEXECUTEINFO shExecInfo;
  shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
  shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
  shExecInfo.hwnd = nullptr;
  // tscon requires administrator access, therefore "runas" verb is needed.
  // UAC popup will be displayed.
  shExecInfo.lpVerb = "runas";
  shExecInfo.lpFile = executable.toUtf8().constData();
  shExecInfo.lpParameters = nullptr;
  if (!parameters.isEmpty())
    {
    shExecInfo.lpParameters = parameters.toUtf8().constData();
    }
  shExecInfo.lpDirectory = nullptr;
  if (!workingDir.isEmpty())
    {
    shExecInfo.lpDirectory = workingDir.toUtf8().constData();
    }
  shExecInfo.nShow = SW_MAXIMIZE;
  shExecInfo.hInstApp = nullptr;
  ShellExecuteEx(&shExecInfo);
  WaitForSingleObject(shExecInfo.hProcess, INFINITE);
  DWORD exitCode = 0;
  GetExitCodeProcess(shExecInfo.hProcess, &exitCode);
  CloseHandle(shExecInfo.hProcess);
  return exitCode;
#else
  Q_UNUSED(executable);
  Q_UNUSED(parameters);
  Q_UNUSED(workingDir);
  qFatal("%s: not implemented for Linux and macOS.", Q_FUNC_INFO);
  return -1;
#endif
}
