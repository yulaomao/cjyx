

#include "qCjyxMainWindow_p.h"

// Qt includes
#include <QAction>
#include <QCloseEvent>
#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QKeySequence>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QQueue>
#include <QSettings>
#include <QShowEvent>
#include <QSignalMapper>
#include <QStyle>
#include <QTextEdit>
#include <QTimer>
#include <QToolButton>
#include <QLabel>
#include <QDir>
#include<QCoreApplication>

// CTK includes
#include <ctkErrorLogWidget.h>
#include <ctkMessageBox.h>
#ifdef Cjyx_USE_PYTHONQT
# include <ctkPythonConsole.h>
#endif
#ifdef Cjyx_USE_QtTesting
#include <ctkQtTestingUtility.h>
#endif
#include <ctkVTKSliceView.h>
#include <ctkVTKWidgetsUtils.h>

// CjyxApp includes
#include "qCjyxAboutDialog.h"
#include "qCjyxActionsDialog.h"
#include "qCjyxApplication.h"
#include "qCjyxAbstractModule.h"
#include "qCjyxAbstractModuleRepresentation.h"
#if defined Cjyx_USE_QtTesting && defined Cjyx_BUILD_CLI_SUPPORT
#include "qCjyxCLIModuleWidgetEventPlayer.h"
#endif
#include "qCjyxCommandOptions.h"
#include "qCjyxCoreCommandOptions.h"
#include "qCjyxErrorReportDialog.h"
#include "qCjyxAbstractModuleWidget.h"
#ifdef Cjyx_BUILD_EXTENSIONMANAGER_SUPPORT
#include "qCjyxExtensionsManagerModel.h"
#endif
#include "qCjyxLayoutManager.h"
#include "qCjyxModuleManager.h"
#include "qCjyxModulesMenu.h"
#include "qCjyxModuleSelectorToolBar.h"
#include "qCjyxIOManager.h"
#include "qCjyxAbstractModuleWidget.h"
#include <PythonQt.h>

// qDMML includes
#include <qDMMLSliceWidget.h>
#include <qDMMLThreeDWidget.h>
#include <qDMMLThreeDViewControllerWidget.h>
#include <qDMMLNodeComboBox.h>

// DMMLLogic includes
#include <vtkDMMLSliceLogic.h>
#include <vtkDMMLSliceLayerLogic.h>
#include <vtkDMMLMessageCollection.h>

// DMML includes
#include <vtkDMMLLayoutNode.h>
#include <vtkDMMLMessageCollection.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceCompositeNode.h>

// VTK includes
#include <vtkCollection.h>

#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif //_MSC_VER

namespace
{

//-----------------------------------------------------------------------------
void setThemeIcon(QAction* action, const QString& name)
{
  action->setIcon(QIcon::fromTheme(name, action->icon()));
}

} // end of anonymous namespace

//-----------------------------------------------------------------------------
// qCjyxMainWindowPrivate methods

qCjyxMainWindowPrivate::qCjyxMainWindowPrivate(qCjyxMainWindow& object)
  : q_ptr(&object)
{
#ifdef Cjyx_USE_PYTHONQT
  this->PythonConsoleDockWidget = nullptr;
  this->PythonConsoleToggleViewAction = nullptr;
#endif
  this->ErrorLogWidget = nullptr;
  this->ErrorLogToolButton = nullptr;
  this->ModuleSelectorToolBar = nullptr;
  this->LayoutManager = nullptr;
  this->WindowInitialShowCompleted = false;
  this->IsClosing = false;
}

//-----------------------------------------------------------------------------
qCjyxMainWindowPrivate::~qCjyxMainWindowPrivate()
{
  delete this->ErrorLogWidget;
}

//-----------------------------------------------------------------------------
void qCjyxMainWindowPrivate::init()
{
  Q_Q(qCjyxMainWindow);
  this->setupUi(q);

  this->setupStatusBar();
  q->setupMenuActions();
  this->StartupState = q->saveState();
  q->restoreGUIState();
}

//-----------------------------------------------------------------------------
void qCjyxMainWindowPrivate::setupUi(QMainWindow * mainWindow)
{
  Q_Q(qCjyxMainWindow);

  this->Ui_qCjyxMainWindow::setupUi(mainWindow);

  qCjyxApplication * app = qCjyxApplication::application();

  //----------------------------------------------------------------------------
  // Recently loaded files
  //----------------------------------------------------------------------------
  QObject::connect(app->coreIOManager(), SIGNAL(newFileLoaded(qCjyxIO::IOProperties)),
                   q, SLOT(onNewFileLoaded(qCjyxIO::IOProperties)));
  QObject::connect(app->coreIOManager(), SIGNAL(fileSaved(qCjyxIO::IOProperties)),
                   q, SLOT(onFileSaved(qCjyxIO::IOProperties)));

  //----------------------------------------------------------------------------
  // Load DICOM
  //----------------------------------------------------------------------------
#ifndef Cjyx_BUILD_DICOM_SUPPORT
  this->LoadDICOMAction->setVisible(false);
#endif

  //----------------------------------------------------------------------------
  // ModulePanel
  //----------------------------------------------------------------------------
  this->PanelDockWidget->toggleViewAction()->setText(qCjyxMainWindow::tr("Show &Module Panel"));
  this->PanelDockWidget->toggleViewAction()->setToolTip(
    qCjyxMainWindow::tr("Collapse/Expand the GUI panel and allows Cjyx's viewers to occupy "
          "the entire application window"));
  this->PanelDockWidget->toggleViewAction()->setShortcut(QKeySequence("Ctrl+5"));
  //this->AppearanceMenu->insertAction(this->ShowStatusBarAction,
    //                                 this->PanelDockWidget->toggleViewAction());

  //----------------------------------------------------------------------------
  // ModuleManager
  //----------------------------------------------------------------------------
  // Update the list of modules when they are loaded
  qCjyxModuleManager * moduleManager = qCjyxApplication::application()->moduleManager();
  if (!moduleManager)
    {
    qWarning() << "No module manager is created.";
    }

  QObject::connect(moduleManager,SIGNAL(moduleLoaded(QString)),
                   q, SLOT(onModuleLoaded(QString)));

  QObject::connect(moduleManager, SIGNAL(moduleAboutToBeUnloaded(QString)),
                   q, SLOT(onModuleAboutToBeUnloaded(QString)));

  //----------------------------------------------------------------------------
  // ModuleSelector ToolBar
  //----------------------------------------------------------------------------
  // Create a Module selector
  this->ModuleSelectorToolBar = new qCjyxModuleSelectorToolBar("Module Selection",q);
  this->ModuleSelectorToolBar->setObjectName(QString::fromUtf8("ModuleSelectorToolBar"));
  this->ModuleSelectorToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
  this->ModuleSelectorToolBar->setModuleManager(moduleManager);
  q->insertToolBar(this->ModuleToolBar, this->ModuleSelectorToolBar);
  this->ModuleSelectorToolBar->stackUnder(this->ModuleToolBar);

  // Connect the selector with the module panel
  this->ModulePanel->setModuleManager(moduleManager);
  QObject::connect(this->ModuleSelectorToolBar, SIGNAL(moduleSelected(QString)),
                   this->ModulePanel, SLOT(setModule(QString)));

  // Ensure the panel dock widget is visible
  QObject::connect(this->ModuleSelectorToolBar, SIGNAL(moduleSelected(QString)),
                   this->PanelDockWidget, SLOT(show()));

  //----------------------------------------------------------------------------
  // MouseMode ToolBar
  //----------------------------------------------------------------------------
  // MouseMode toolBar should listen the DMML scene
  this->MouseModeToolBar->setApplicationLogic(
    qCjyxApplication::application()->applicationLogic());
  this->MouseModeToolBar->setDMMLScene(qCjyxApplication::application()->dmmlScene());
  QObject::connect(qCjyxApplication::application(),
                   SIGNAL(dmmlSceneChanged(vtkDMMLScene*)),
                   this->MouseModeToolBar,
                   SLOT(setDMMLScene(vtkDMMLScene*)));
  //----------------------------------------------------------------------------
  // Capture tool bar
  //----------------------------------------------------------------------------
  // Capture bar needs to listen to the DMML scene and the layout
  this->CaptureToolBar->setDMMLScene(qCjyxApplication::application()->dmmlScene());
  QObject::connect(qCjyxApplication::application(),
                   SIGNAL(dmmlSceneChanged(vtkDMMLScene*)),
                   this->CaptureToolBar,
                   SLOT(setDMMLScene(vtkDMMLScene*)));
  this->CaptureToolBar->setDMMLScene(
      qCjyxApplication::application()->dmmlScene());

  QObject::connect(this->CaptureToolBar,
                   SIGNAL(screenshotButtonClicked()),
                   qCjyxApplication::application()->ioManager(),
                   SLOT(openScreenshotDialog()));

  // to get the scene views module dialog to pop up when a button is pressed
  // in the capture tool bar, we emit a signal, and the
  // io manager will deal with the sceneviews module
  QObject::connect(this->CaptureToolBar,
                   SIGNAL(sceneViewButtonClicked()),
                   qCjyxApplication::application()->ioManager(),
                   SLOT(openSceneViewsDialog()));

  // if testing is enabled on the application level, add a time out to the pop ups
  if (qCjyxApplication::application()->testAttribute(qCjyxCoreApplication::AA_EnableTesting))
    {
    this->CaptureToolBar->setPopupsTimeOut(true);
    }

  QList<QAction*> toolBarActions;
  toolBarActions << this->MainToolBar->toggleViewAction();
  //toolBarActions << this->UndoRedoToolBar->toggleViewAction();
  toolBarActions << this->ModuleSelectorToolBar->toggleViewAction();
  toolBarActions << this->ModuleToolBar->toggleViewAction();
  toolBarActions << this->ViewToolBar->toggleViewAction();
  //toolBarActions << this->LayoutToolBar->toggleViewAction();
  toolBarActions << this->MouseModeToolBar->toggleViewAction();
  toolBarActions << this->CaptureToolBar->toggleViewAction();
  toolBarActions << this->ViewersToolBar->toggleViewAction();
  toolBarActions << this->DialogToolBar->toggleViewAction();
  //this->WindowToolBarsMenu->addActions(toolBarActions);

  //----------------------------------------------------------------------------
  // Hide toolbars by default
  //----------------------------------------------------------------------------
  // Hide the Layout toolbar by default
  // The setVisibility slot of the toolbar is connected to the
  // QAction::triggered signal.
  // It's done for a long list of reasons. If you change this behavior, make sure
  // minimizing the application and restore it doesn't hide the module panel. check
  // also the geometry and the state of the menu qactions are correctly restored when
  // loading cjyx.
  this->UndoRedoToolBar->toggleViewAction()->trigger();
  this->LayoutToolBar->toggleViewAction()->trigger();
  //q->removeToolBar(this->UndoRedoToolBar);
  //q->removeToolBar(this->LayoutToolBar);
  delete this->UndoRedoToolBar;
  this->UndoRedoToolBar = nullptr;
  delete this->LayoutToolBar;
  this->LayoutToolBar = nullptr;

  // Color of the spacing between views:
  //QFrame* layoutFrame = new QFrame(this->CentralWidget);
  //layoutFrame->setObjectName("CentralWidgetLayoutFrame");
  //QHBoxLayout* centralLayout = new QHBoxLayout(this->CentralWidget);
  //QFrame* layoutFrame = new QFrame(this->CentralWidget);
  QFrame* layoutFrame = new QFrame(this->FourViewWidget);
  layoutFrame->setObjectName("CentralWidgetLayoutFrame");
  QHBoxLayout* centralLayout = new QHBoxLayout(this->FourViewWidget);
  centralLayout->setContentsMargins(0, 0, 0, 0);
  centralLayout->addWidget(layoutFrame);
  this->PatientWidget->setVisible(false);
  this->FourViewAction->setChecked(true);

  //----------------------------------------------------------------------------
  // Layout Manager
  //----------------------------------------------------------------------------
  // Instantiate and assign the layout manager to the cjyx application
  this->LayoutManager = new qCjyxLayoutManager(layoutFrame);
  this->LayoutManager->setScriptedDisplayableManagerDirectory(
      qCjyxApplication::application()->cjyxHome() + "/bin/Python/dmmlDisplayableManager");
  qCjyxApplication::application()->setLayoutManager(this->LayoutManager);
#ifdef Cjyx_USE_QtTesting
  // we store this layout manager to the Object state property for QtTesting
  qCjyxApplication::application()->testingUtility()->addObjectStateProperty(
      qCjyxApplication::application()->layoutManager(), QString("layout"));
  qCjyxApplication::application()->testingUtility()->addObjectStateProperty(
      this->ModuleSelectorToolBar->modulesMenu(), QString("currentModule"));
#endif
  // Layout manager should also listen the DMML scene
  // Note: This creates the OpenGL context for each view, so things like
  // multisampling should probably be configured before this line is executed.
  this->LayoutManager->setDMMLScene(qCjyxApplication::application()->dmmlScene());
  QObject::connect(qCjyxApplication::application(),
                   SIGNAL(dmmlSceneChanged(vtkDMMLScene*)),
                   this->LayoutManager,
                   SLOT(setDMMLScene(vtkDMMLScene*)));
  QObject::connect(this->LayoutManager, SIGNAL(layoutChanged(int)),
                   q, SLOT(onLayoutChanged(int)));

  // TODO: When module will be managed by the layoutManager, this should be
  //       revisited.
  QObject::connect(this->LayoutManager, SIGNAL(selectModule(QString)),
                   this->ModuleSelectorToolBar, SLOT(selectModule(QString)));

  // Add menus for configuring compare view
  QMenu *compareMenu = new QMenu(qCjyxMainWindow::tr("Select number of viewers..."), mainWindow);
  compareMenu->setObjectName("CompareMenuView");
  compareMenu->addAction(this->ViewLayoutCompare_2_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompare_3_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompare_4_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompare_5_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompare_6_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompare_7_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompare_8_viewersAction);
  this->ViewLayoutCompareAction->setMenu(compareMenu);
  QObject::connect(compareMenu, SIGNAL(triggered(QAction*)),
                   q, SLOT(onLayoutCompareActionTriggered(QAction*)));

  // ... and for widescreen version of compare view as well
  compareMenu = new QMenu(qCjyxMainWindow::tr("Select number of viewers..."), mainWindow);
  compareMenu->setObjectName("CompareMenuWideScreen");
  compareMenu->addAction(this->ViewLayoutCompareWidescreen_2_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompareWidescreen_3_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompareWidescreen_4_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompareWidescreen_5_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompareWidescreen_6_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompareWidescreen_7_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompareWidescreen_8_viewersAction);
  this->ViewLayoutCompareWidescreenAction->setMenu(compareMenu);
  QObject::connect(compareMenu, SIGNAL(triggered(QAction*)),
                   q, SLOT(onLayoutCompareWidescreenActionTriggered(QAction*)));

  // ... and for the grid version of the compare views
  compareMenu = new QMenu(qCjyxMainWindow::tr("Select number of viewers..."), mainWindow);
  compareMenu->setObjectName("CompareMenuGrid");
  compareMenu->addAction(this->ViewLayoutCompareGrid_2x2_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompareGrid_3x3_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompareGrid_4x4_viewersAction);
  this->ViewLayoutCompareGridAction->setMenu(compareMenu);
  QObject::connect(compareMenu, SIGNAL(triggered(QAction*)),
                   q, SLOT(onLayoutCompareGridActionTriggered(QAction*)));


  // Capture tool bar needs to listen to the layout manager
  QObject::connect(this->LayoutManager,
                   SIGNAL(activeDMMLThreeDViewNodeChanged(vtkDMMLViewNode*)),
                   this->CaptureToolBar,
                   SLOT(setActiveDMMLThreeDViewNode(vtkDMMLViewNode*)));
  this->CaptureToolBar->setActiveDMMLThreeDViewNode(
      this->LayoutManager->activeDMMLThreeDViewNode());

  // Authorize Drops action from outside
  q->setAcceptDrops(true);

  //----------------------------------------------------------------------------
  // View Toolbar
  //----------------------------------------------------------------------------
  // Populate the View ToolBar with all the layouts of the layout manager
  this->LayoutButton = new QToolButton(q);
  this->LayoutButton->setText(qCjyxMainWindow::tr("Layout"));
  QMenu* LayoutMenu = new QMenu();
  LayoutMenu->addAction(this->ViewLayoutCompareWidescreenAction);
  LayoutMenu->addAction(this->ViewLayoutConventionalAction);
  LayoutMenu->addAction(this->ViewLayoutFourUpAction);
  LayoutMenu->addAction(this->ViewLayoutOneUp3DAction);
  LayoutMenu->addAction(this->ViewLayoutOneUpRedSliceAction);
  LayoutMenu->addAction(this->ViewLayoutOneUpGreenSliceAction);
  LayoutMenu->addAction(this->ViewLayoutOneUpYellowSliceAction);


  this->LayoutButton->setMenu(LayoutMenu);
  
  this->LayoutButton->setPopupMode(QToolButton::InstantPopup);

  this->LayoutButton->setDefaultAction(this->ViewLayoutFourUpAction);

  QObject::connect(LayoutMenu, SIGNAL(triggered(QAction*)),
                   q, SLOT(onLayoutActionTriggered(QAction*)));

  this->ViewToolBar->addWidget(this->LayoutButton);

  QObject::connect(this->ViewToolBar,
                   SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
                   this->LayoutButton, SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));

  //----------------------------------------------------------------------------
  // Viewers Toolbar
  //----------------------------------------------------------------------------
  // Viewers toolBar should listen the DMML scene
  this->ViewersToolBar->setApplicationLogic(
    qCjyxApplication::application()->applicationLogic());
  this->ViewersToolBar->setDMMLScene(qCjyxApplication::application()->dmmlScene());
  QObject::connect(qCjyxApplication::application(),
                   SIGNAL(dmmlSceneChanged(vtkDMMLScene*)),
                   this->ViewersToolBar,
                   SLOT(setDMMLScene(vtkDMMLScene*)));
  //----------------------------------------------------------------------------
  // CurrentModuleToolBar
  // ---------------------------------------------------------------------------
  QActionGroup* ModuleActionGroup = new QActionGroup(this->CurrentModuleToolBar);
  ModuleActionGroup->addAction(this->DataManageAction);
  ModuleActionGroup->addAction(this->ImageRebuildAction);
  ModuleActionGroup->addAction(this->VolumeRenderingAction);
  ModuleActionGroup->addAction(this->VolumeCropAction);
  ModuleActionGroup->addAction(this->MeasureAction);
  ModuleActionGroup->addAction(this->ModuleToolAction);
  //this->DataManageAction->setChecked(true);
  ModuleActionGroup->setExclusive(true);

  //----------------------------------------------------------------------------
  // Undo/Redo Toolbar
  //----------------------------------------------------------------------------
  // Listen to the scene to enable/disable the undo/redo toolbuttons
  //q->qvtkConnect(qCjyxApplication::application()->dmmlScene(), vtkCommand::ModifiedEvent,
  //               q, SLOT(onDMMLSceneModified(vtkObject*)));
  //q->onDMMLSceneModified(qCjyxApplication::application()->dmmlScene());

  //----------------------------------------------------------------------------
  // Icons in the menu
  //----------------------------------------------------------------------------
  // Customize QAction icons with standard pixmaps

  this->CutAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  this->CopyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  this->PasteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);

  setThemeIcon(this->FileExitAction, "application-exit");
  setThemeIcon(this->EditUndoAction, "edit-undo");
  setThemeIcon(this->EditRedoAction, "edit-redo");
  setThemeIcon(this->CutAction, "edit-cut");
  setThemeIcon(this->CopyAction, "edit-copy");
  setThemeIcon(this->PasteAction, "edit-paste");
  setThemeIcon(this->EditApplicationSettingsAction, "preferences-system");
  setThemeIcon(this->ModuleHomeAction, "go-home");
  this->TwoLayoutDockWidget->hide();
  //代码添加QDockWidget控件的情况
  QDockWidget* dockWidget = this->TwoLayoutDockWidget;
  delete dockWidget->titleBarWidget();  //删掉标题栏
  QWidget* widget = new QWidget;
  dockWidget->setTitleBarWidget(widget); //在标题栏加一个空的widget

  //----------------------------------------------------------------------------
  // Error log widget
  //----------------------------------------------------------------------------
  this->ErrorLogWidget = new ctkErrorLogWidget;
  this->ErrorLogWidget->setErrorLogModel(
    qCjyxApplication::application()->errorLogModel());

  //----------------------------------------------------------------------------
  // Python console
  //----------------------------------------------------------------------------
#ifdef Cjyx_USE_PYTHONQT
  if (q->pythonConsole())
    {
    if (QSettings().value("Python/DockableWindow").toBool())
      {
      this->PythonConsoleDockWidget = new QDockWidget(qCjyxMainWindow::tr("Python Interactor"));
      this->PythonConsoleDockWidget->setObjectName("PythonConsoleDockWidget");
      this->PythonConsoleDockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
      this->PythonConsoleDockWidget->setWidget(q->pythonConsole());
      this->PythonConsoleToggleViewAction = this->PythonConsoleDockWidget->toggleViewAction();
      // Set default state
      q->addDockWidget(Qt::BottomDockWidgetArea, this->PythonConsoleDockWidget);
      this->PythonConsoleDockWidget->hide();
      }
    else
      {
      ctkPythonConsole* pythonConsole = q->pythonConsole();
      pythonConsole->setWindowTitle("Cjyx Python Interactor");
      pythonConsole->resize(600, 280);
      pythonConsole->hide();
      this->PythonConsoleToggleViewAction = new QAction("");
      this->PythonConsoleToggleViewAction->setCheckable(true);
      }
    q->pythonConsole()->setScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->updatePythonConsolePalette();
    QObject::connect(q->pythonConsole(), SIGNAL(aboutToExecute(const QString&)),
      q, SLOT(onPythonConsoleUserInput(const QString&)));
    // Set up show/hide action
    this->PythonConsoleToggleViewAction->setText(qCjyxMainWindow::tr("&Python Interactor"));
    this->PythonConsoleToggleViewAction->setToolTip(qCjyxMainWindow::tr(
      "Show Python Interactor window for controlling the application's data, user interface, and internals"));
    this->PythonConsoleToggleViewAction->setShortcuts({qCjyxMainWindow::tr("Ctrl+3"), qCjyxMainWindow::tr("Ctrl+`")});
    QObject::connect(this->PythonConsoleToggleViewAction, SIGNAL(toggled(bool)),
      q, SLOT(onPythonConsoleToggled(bool)));
    //this->ViewMenu->insertAction(this->ModuleHomeAction, this->PythonConsoleToggleViewAction);
    this->PythonConsoleToggleViewAction->setIcon(QIcon(":/python-icon.png"));
    //this->DialogToolBar->addAction(this->PythonConsoleToggleViewAction);
    }
  else
    {
    qWarning("qCjyxMainWindowPrivate::setupUi: Failed to create Python console");
    }
#endif

  //----------------------------------------------------------------------------
  // Dockable Widget Area Definitions
  //----------------------------------------------------------------------------
  // Setting the left and right dock widget area to occupy the bottom corners
  // means the module panel is able to have more vertical space since it is the
  // usual left/right dockable widget. Since the module panel is typically not a
  // majority of the width dimension, this means the python interactor in the
  // bottom widget area still has a wide aspect ratio.
  // If application window is narrow then the Python interactor can be docked to the top
  // to use the full width of the application window.
  q->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
  q->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

  //--------------------------------------------------------------------------
  //拖拽实现弹窗
  //--------------------------------------------------------------------------
  PythonQt::init();
  PythonQtObjectPtr context = PythonQt::self()->getMainModule();
  context.evalScript(QString(
      "class MyClass(qt.QWidget) :\n"
      "    def eventFilter(self, object, event) :\n"
      "        if event.type() == qt.QEvent.DragEnter :\n"
      "           self.dragEnterEvent(event)\n"
      "           return True\n"
      "        if event.type() == qt.QEvent.Drop :\n"
      "           self.dropEvent(event)\n"
      "           return True\n"
      "        return False\n"
      "    def dragEnterEvent(self, event) :\n"
      "        if event.mimeData().hasUrls :\n"
      "           event.acceptProposedAction()  # allows drop event to proceed\n"
      "        else :\n"
      "           event.ignore()\n"
      "    def dropEvent(self, event) :\n"
      "       path = qt.QUrl()\n"
      "       path = event.mimeData().urls()[0]\n"
      "       widget = cjyx.modules.inputdataloadable.widgetRepresentation() \n"
      "       widget.onDragEvent(path.toLocalFile()) \n"


      "my_class = MyClass()\n"
      "cjyx.util.mainWindow().installEventFilter(my_class)\n"
  ));

  //---------------------------------------------------------------------------
  //connect ImageRebuild
  //---------------------------------------------------------------------------

  QObject::connect(this->ImageRebuildMenu, SIGNAL(triggered(QAction*)), q, SLOT(onImageRebuildAction1(QAction*)));
  QObject::connect(this->MduleToolMenu, SIGNAL(triggered(QAction*)), q, SLOT(onModuleToolMenu(QAction*)));
  QObject::connect(this->AddPointAction, SIGNAL(triggered(bool)), q, SLOT(onMarkupToolBar(bool)));
  QObject::connect(this->LengthAction, SIGNAL(triggered(bool)), q, SLOT(onMarkupToolBar(bool)));
  QObject::connect(this->AngleAction, SIGNAL(triggered(bool)), q, SLOT(onMarkupToolBar(bool)));
  QObject::connect(this->NoteAction, SIGNAL(triggered(bool)), q, SLOT(onMarkupToolBar(bool)));
  //QObject::connect(this->ManagerMenu, SIGNAL(triggered(QAction*)), q, SLOT(onImageRebuildAction(QAction*)));
  //QObject::connect(this->ManagerMenu, SIGNAL(triggered(QAction*)), q, SLOT(onImageRebuildAction(QAction*)));
  //q->setRightLayout();
}

//-----------------------------------------------------------------------------
void qCjyxMainWindowPrivate::updatePythonConsolePalette()
{
  Q_Q(qCjyxMainWindow);
#ifdef Cjyx_USE_PYTHONQT
  ctkPythonConsole* pythonConsole = q->pythonConsole();
  if (!pythonConsole)
    {
    return;
    }
  QPalette palette = qCjyxApplication::application()->palette();

  // pythonConsole->setBackgroundColor is not called, because by default
  // the background color of the current palette is used, which is good.

  // Color of the >> prompt. Blue in both bright and dark styles (brighter in dark style).
  pythonConsole->setPromptColor(palette.color(QPalette::Highlight));

  // Color of text that user types in the console. Blue in both bright and dark styles (brighter in dark style).
  pythonConsole->setCommandTextColor(palette.color(QPalette::Link));

  // Color of output of commands. Black in bright style, white in dark style.
  pythonConsole->setOutputTextColor(palette.color(QPalette::WindowText));

  // Color of error messages. Red in both bright and dark styles, but slightly brighter in dark style.
  QColor textColor = q->palette().color(QPalette::Normal, QPalette::Text);
  if (textColor.lightnessF() < 0.5)
    {
    // Light theme
    pythonConsole->setErrorTextColor(QColor::fromRgb(240, 0, 0)); // darker than Qt::red
    }
  else
    {
    // Dark theme
    pythonConsole->setErrorTextColor(QColor::fromRgb(255, 68, 68)); // lighter than Qt::red
    }

  // Color of welcome message (printed when the terminal is reset)
  // and "user input" (this does not seem to be used in Cjyx).
  // Gray in both bright and dark styles.
  pythonConsole->setStdinTextColor(palette.color(QPalette::Disabled, QPalette::WindowText));   // gray
  pythonConsole->setWelcomeTextColor(palette.color(QPalette::Disabled, QPalette::WindowText)); // gray
#endif
}


//-----------------------------------------------------------------------------
void qCjyxMainWindowPrivate::setupRecentlyLoadedMenu(const QList<qCjyxIO::IOProperties>& fileProperties)
{
  Q_Q(qCjyxMainWindow);

  //this->RecentlyLoadedMenu->setEnabled(fileProperties.size() > 0);
  //this->RecentlyLoadedMenu->clear();

  QListIterator<qCjyxIO::IOProperties> iterator(fileProperties);
  iterator.toBack();
  while (iterator.hasPrevious())
    {
    qCjyxIO::IOProperties filePropertie = iterator.previous();
    QString fileName = filePropertie.value("fileName").toString();
    if (fileName.isEmpty())
      {
      continue;
      }
    //QAction * action = this->RecentlyLoadedMenu->addAction(
    //  fileName, q, SLOT(onFileRecentLoadedActionTriggered()));
    //action->setProperty("fileParameters", filePropertie);
    //action->setEnabled(QFile::exists(fileName));
    }

  // Add separator and clear action
  //this->RecentlyLoadedMenu->addSeparator();
  //QAction * clearAction = this->RecentlyLoadedMenu->addAction(
  //  "清除历史", q, SLOT(onFileRecentLoadedActionTriggered()));
  //clearAction->setProperty("clearMenu", QVariant(true));
}

//-----------------------------------------------------------------------------
void qCjyxMainWindowPrivate::filterRecentlyLoadedFileProperties()
{
  int numberOfFilesToKeep = QSettings().value("RecentlyLoadedFiles/NumberToKeep").toInt();

  // Remove extra element
  while (this->RecentlyLoadedFileProperties.size() > numberOfFilesToKeep)
    {
    this->RecentlyLoadedFileProperties.dequeue();
    }
}

//-----------------------------------------------------------------------------
QList<qCjyxIO::IOProperties> qCjyxMainWindowPrivate::readRecentlyLoadedFiles()
{
  QList<qCjyxIO::IOProperties> fileProperties;

  QSettings settings;
  int size = settings.beginReadArray("RecentlyLoadedFiles/RecentFiles");
  for(int i = 0; i < size; ++i)
    {
    settings.setArrayIndex(i);
    QVariant file = settings.value("file");
    qCjyxIO::IOProperties properties = file.toMap();
    properties["fileName"] = qCjyxApplication::application()->toCjyxHomeAbsolutePath(properties["fileName"].toString());
    fileProperties << properties;
    }
  settings.endArray();

  return fileProperties;
}

//-----------------------------------------------------------------------------
void qCjyxMainWindowPrivate::writeRecentlyLoadedFiles(const QList<qCjyxIO::IOProperties>& fileProperties)
{
  QSettings settings;
  settings.beginWriteArray("RecentlyLoadedFiles/RecentFiles", fileProperties.size());
  for (int i = 0; i < fileProperties.size(); ++i)
    {
    settings.setArrayIndex(i);
    qCjyxIO::IOProperties properties = fileProperties.at(i);
    properties["fileName"] = qCjyxApplication::application()->toCjyxHomeRelativePath(properties["fileName"].toString());
    settings.setValue("file", properties);
    }
  settings.endArray();
}

//-----------------------------------------------------------------------------
bool qCjyxMainWindowPrivate::confirmCloseApplication()
{
  Q_Q(qCjyxMainWindow);
  vtkDMMLScene* scene = qCjyxApplication::application()->dmmlScene();
  QString question;
  if (scene->GetStorableNodesModifiedSinceRead())
    {
    question = qCjyxMainWindow::tr("一些数据已被修改。退出前是否保存它们?");
    }
  else if (scene->GetModifiedSinceRead())
    {
    question = qCjyxMainWindow::tr("场景已修改。退出前是否保存?");
    }
  bool close = false;
  if (!question.isEmpty())
    {
    QMessageBox* messageBox = new QMessageBox(QMessageBox::Warning, qCjyxMainWindow::tr("退出之前是否保存?"), question, QMessageBox::NoButton, q);
    QAbstractButton* saveButton =
       messageBox->addButton(qCjyxMainWindow::tr("保存"), QMessageBox::ActionRole);
    QAbstractButton* exitButton =
       messageBox->addButton(qCjyxMainWindow::tr("退出 (不保存)"), QMessageBox::ActionRole);
    QAbstractButton* cancelButton =
       messageBox->addButton(qCjyxMainWindow::tr("取消退出"), QMessageBox::ActionRole);
    Q_UNUSED(cancelButton);
    messageBox->exec();
    if (messageBox->clickedButton() == saveButton)
      {
      // \todo Check if the save data dialog was "applied" and close the
      // app in that case
      this->SaveSceneAction->trigger();
      }
    else if (messageBox->clickedButton() == exitButton)
      {
      close = true;
      }
    messageBox->deleteLater();
    }
  else
    {
    close = ctkMessageBox::confirmExit("MainWindow/DontConfirmExit", q);
    }
  return close;
}

//-----------------------------------------------------------------------------
bool qCjyxMainWindowPrivate::confirmCloseScene()
{
  Q_Q(qCjyxMainWindow);
  vtkDMMLScene* scene = qCjyxApplication::application()->dmmlScene();
  QString question;
  if (scene->GetStorableNodesModifiedSinceRead())
    {
    question = qCjyxMainWindow::tr("一些数据已被修改。是否要在关闭场景之前保存它们?");
    }
  else if (scene->GetModifiedSinceRead())
    {
    question = qCjyxMainWindow::tr("场景已修改。是否要在关闭场景之前保存它?");
    }
  else
    {
    // no unsaved changes, no need to ask confirmation
    return true;
    }

  ctkMessageBox* confirmCloseMsgBox = new ctkMessageBox(q);
  confirmCloseMsgBox->setAttribute(Qt::WA_DeleteOnClose);
  confirmCloseMsgBox->setWindowTitle(qCjyxMainWindow::tr("关闭场景前保存?"));
  confirmCloseMsgBox->setText(question);

  // Use AcceptRole&RejectRole instead of Save&Discard because we would
  // like discard changes to be the default behavior.
  confirmCloseMsgBox->addButton(qCjyxMainWindow::tr("关闭场景（不保存）"), QMessageBox::AcceptRole);
  confirmCloseMsgBox->addButton(qCjyxMainWindow::tr("保存场景"), QMessageBox::RejectRole);
  confirmCloseMsgBox->addButton(QMessageBox::Cancel);
  confirmCloseMsgBox->setButtonText(QMessageBox::Cancel,"取消");

  confirmCloseMsgBox->setDontShowAgainVisible(true);
  confirmCloseMsgBox->setDontShowAgainSettingsKey("MainWindow/DontConfirmSceneClose");
  confirmCloseMsgBox->setIcon(QMessageBox::Question);
  int resultCode = confirmCloseMsgBox->exec();
  if (resultCode == QMessageBox::Cancel)
    {
    return false;
    }
  if (resultCode != QMessageBox::AcceptRole)
    {
    if (!qCjyxApplication::application()->ioManager()->openSaveDataDialog())
      {
      return false;
      }
    }
  return true;
}

//-----------------------------------------------------------------------------
void qCjyxMainWindowPrivate::setupStatusBar()
{
  Q_Q(qCjyxMainWindow);
  this->ErrorLogToolButton = new QToolButton();
  this->ErrorLogToolButton->setDefaultAction(this->WindowErrorLogAction);
  q->statusBar()->addPermanentWidget(this->ErrorLogToolButton);

  QObject::connect(qCjyxApplication::application()->errorLogModel(),
                   SIGNAL(entryAdded(ctkErrorLogLevel::LogLevel)),
                   q, SLOT(onWarningsOrErrorsOccurred(ctkErrorLogLevel::LogLevel)));
}

//-----------------------------------------------------------------------------
void qCjyxMainWindowPrivate::setErrorLogIconHighlighted(bool highlighted)
{
  Q_Q(qCjyxMainWindow);
  QIcon defaultIcon = q->style()->standardIcon(QStyle::SP_MessageBoxCritical);
  QIcon icon = defaultIcon;
  if(!highlighted)
    {
    QIcon disabledIcon;
    disabledIcon.addPixmap(
          defaultIcon.pixmap(QSize(32, 32), QIcon::Disabled, QIcon::On), QIcon::Active, QIcon::On);
    icon = disabledIcon;
    }
  this->WindowErrorLogAction->setIcon(icon);
}

//-----------------------------------------------------------------------------
// qCjyxMainWindow methods

//-----------------------------------------------------------------------------
qCjyxMainWindow::qCjyxMainWindow(QWidget *_parent)
  : Superclass(_parent)
  , d_ptr(new qCjyxMainWindowPrivate(*this))
{
  Q_D(qCjyxMainWindow);
  d->init();
}

//-----------------------------------------------------------------------------
qCjyxMainWindow::qCjyxMainWindow(qCjyxMainWindowPrivate* pimpl,
                                     QWidget* windowParent)
  : Superclass(windowParent)
  , d_ptr(pimpl)
{
  // init() is called by derived class.
}

//-----------------------------------------------------------------------------
qCjyxMainWindow::~qCjyxMainWindow()
{
  Q_D(qCjyxMainWindow);
  // When quitting the application, the modules are unloaded (~qCjyxCoreApplication)
  // in particular the Colors module which deletes vtkDMMLColorLogic and removes
  // all the color nodes from the scene. If a volume was loaded in the views,
  // it would then try to render it with no color node and generate warnings.
  // There is no need to render anything so remove the volumes from the views.
  // It is maybe not the best place to do that but I couldn't think of anywhere
  // else.
  vtkCollection* sliceLogics = d->LayoutManager ? d->LayoutManager->dmmlSliceLogics() : nullptr;
  for (int i = 0; i < sliceLogics->GetNumberOfItems(); ++i)
    {
    vtkDMMLSliceLogic* sliceLogic = vtkDMMLSliceLogic::SafeDownCast(sliceLogics->GetItemAsObject(i));
    if (!sliceLogic)
      {
      continue;
      }
    sliceLogic->GetSliceCompositeNode()->SetReferenceBackgroundVolumeID(nullptr);
    sliceLogic->GetSliceCompositeNode()->SetReferenceForegroundVolumeID(nullptr);
    sliceLogic->GetSliceCompositeNode()->SetReferenceLabelVolumeID(nullptr);
    }
}

//-----------------------------------------------------------------------------
qCjyxModuleSelectorToolBar* qCjyxMainWindow::moduleSelector()const
{
  Q_D(const qCjyxMainWindow);
  return d->ModuleSelectorToolBar;
}

#ifdef Cjyx_USE_PYTHONQT
//---------------------------------------------------------------------------
ctkPythonConsole* qCjyxMainWindow::pythonConsole()const
{
  return qCjyxCoreApplication::application()->pythonConsole();
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::onPythonConsoleUserInput(const QString& cmd)
{
  if (!cmd.isEmpty())
    {
    qDebug("Python console user input: %s", qPrintable(cmd));
    }
}
#endif

//---------------------------------------------------------------------------
ctkErrorLogWidget* qCjyxMainWindow::errorLogWidget()const
{
  Q_D(const qCjyxMainWindow);
  return d->ErrorLogWidget;
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::on_ShowStatusBarAction_triggered(bool toggled)
{
  this->statusBar()->setVisible(toggled);
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::on_FileFavoriteModulesAction_triggered()
{
  qCjyxApplication::application()->openSettingsDialog("Modules");
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::on_FileAddDataAction_triggered()
{
  qCjyxApplication::application()->ioManager()->openAddDataDialog();
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::on_FileLoadDataAction_triggered()
{
  //qCjyxApplication::application()->ioManager()->openAddDataDialog();
    QString type = "PLY模型文件(*.ply);;OBJ模型文件(*.obj)";
    QString fileName = QFileDialog::getOpenFileName(nullptr, "导入数据文件",
        "D:/",
       type );
    std::cout << fileName.toStdString() << std::endl;
    qCjyxApplication::application()->coreIOManager()->loadFile(fileName);
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::on_FileImportSceneAction_triggered()
{
  qCjyxApplication::application()->ioManager()->openAddSceneDialog();
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::on_FileLoadSceneAction_triggered()
{
  //qCjyxApplication::application()->ioManager()->openLoadSceneDialog();
    Q_D(qCjyxMainWindow);
    QString fileName = QFileDialog::getOpenFileName(nullptr, "打开项目文件",
        "D:/",
        "项目文件(*.cjyx)");
    if (fileName.isEmpty())
        return;
    PythonQt::init();
    PythonQtObjectPtr context = PythonQt::self()->getMainModule();
    //PyRun_SimpleString("cjyx.modules.datamanager.widgetRepresentation().self().isLoadDMML = True");
    context.addVariable("filename", fileName);
    //PyRun_SimpleString("cjyx.util.loadScene(filename)");
    qCjyxApplication::application()->coreIOManager()->loadScene(fileName);
    

    //PyRun_SimpleString("cjyx.modules.celiang.widgetRepresentation().self().ReadFromCJYX(filename)");
    int num = qCjyxApplication::application()->dmmlScene()->GetNodesByClass("vtkDMMLScalarVolumeNode")->GetNumberOfItems();
    qDebug() << num;
    if (num >0)
    {
        PyRun_SimpleString("cjyx.modules.patient.widgetRepresentation().ViewSet()");
    }
    d->ResetViewAction->trigger();
    QStringList list = fileName.split("/");
    this->setWindowTitle(list[list.size() - 1]);
    this->CurrentPath = fileName;
    //PyRun_SimpleString("cjyx.modules.datamanager.widgetRepresentation().self().isLoadDMML = False");
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::on_FileAddVolumeAction_triggered()
{
  qCjyxApplication::application()->ioManager()->openAddVolumesDialog();
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::on_FileAddTransformAction_triggered()
{
  qCjyxApplication::application()->ioManager()->openAddTransformDialog();
}
//---------------------------------------------------------------------------
void qCjyxMainWindow::on_SaveSceneAction_triggered()
{
    PythonQt::init();
    PythonQtObjectPtr context = PythonQt::self()->getMainModule();
    QString fileName;
    if (this->CurrentPath == nullptr)
    {

        PyRun_SimpleString("path = cjyx.modules.setting.widgetRepresentation().self().path");
        QVariant defaultfile = context.getVariable("path");
        QString name = this->windowTitle().left(this->windowTitle().size() - 1);
        QString DefaultFileName = defaultfile.toString()+"/"+ name;
        QString Type = "项目文件(*.cjyx)";
        fileName = QFileDialog::getSaveFileName(nullptr, "保存项目文件", DefaultFileName, Type);
    }
    else
    {
        fileName = this->CurrentPath;
    }
    if (fileName.isEmpty())
        return;
    //qCjyxIO::IOProperties properties;
    //properties["fileName"] = fileName;
    //qCjyxCoreApplication::application()->coreIOManager()->saveNodes(QString("SceneFile"), properties);
    context.addVariable("filename", fileName);

    PyRun_SimpleString("cjyx.util.saveScene(filename)");
    PyRun_SimpleString("cjyx.modules.celiang.widgetRepresentation().self().Zip2CJYX(filename)");
    QStringList list = fileName.split("/");
    this->setWindowTitle(list[list.size() - 1]);
    this->CurrentPath = fileName;
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::on_SaveAsSceneAction_triggered()
{
    QString fileName;
    if (this->CurrentPath == nullptr)
    {
        PythonQt::init();
        PythonQtObjectPtr context = PythonQt::self()->getMainModule();
        PyRun_SimpleString("path = cjyx.modules.setting.widgetRepresentation().self().path");
        QVariant defaultfile = context.getVariable("path");
        QString name = this->windowTitle().left(this->windowTitle().size() - 1);
        QString DefaultFileName = defaultfile.toString() + "/" + name;
        QString Type = "项目文件(*.cjyx)";
        fileName = QFileDialog::getSaveFileName(nullptr, "保存项目文件", DefaultFileName, Type);
    }
    else
    {
        QString Type = "项目文件(*.cjyx)";
        QString DefaultFileName = this->CurrentPath;
        QString fileName = QFileDialog::getSaveFileName(nullptr, "另存项目文件", DefaultFileName, Type);
    }
    if (fileName.isEmpty())
        return;
    qCjyxIO::IOProperties properties;
    properties["fileName"] = fileName;
    qCjyxCoreApplication::application()->coreIOManager()->saveNodes(QString("SceneFile"), properties);

    QStringList list = fileName.split("/");
    QString windowName = list[list.size() - 1];
    this->setWindowTitle(windowName);
    this->CurrentPath = fileName;
}
//---------------------------------------------------------------------------
void qCjyxMainWindow::on_FileSaveSceneAction_triggered()
{
  //qCjyxApplication::application()->ioManager()->openSaveDataDialog();

    CreateSaveDataWidget();
    SaveWidget->show();
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::CreateSaveDataWidget()
{
    Q_D(qCjyxMainWindow);
    SaveWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(SaveWidget);
    QHBoxLayout* layout1 = new QHBoxLayout();

    QLabel* label = new QLabel();
    label->setText("选择节点：");
    nodeSelect = new qDMMLNodeComboBox();
    QStringList nodeTypes;
    nodeTypes << QString("vtkDMMLModelNode");
    nodeTypes << QString("vtkDMMLVolumeNode");
    nodeSelect->setNodeTypes(nodeTypes);
    nodeSelect->setAddEnabled(0);
    nodeSelect->setRemoveEnabled(0);
    nodeSelect->setRemoveEnabled(0);
    nodeSelect->setDMMLScene(qCjyxApplication::application()->dmmlScene());

    layout1->addWidget(label);
    layout1->addWidget(nodeSelect);
    nodeSelect->setEnabled(true);
    QObject::connect(qCjyxApplication::application(),SIGNAL(dmmlSceneChanged(vtkDMMLScene*)),nodeSelect,SLOT(setDMMLScene(vtkDMMLScene*)));
    QPushButton* save = new QPushButton();
    save->setText("保存数据"); 
    save->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    layout->addLayout(layout1);
    layout->setSpacing(10);
    layout->addWidget(save);
    SaveWidget->setWindowTitle("导出数据文件");
    SaveWidget->setWindowIcon(QIcon(":/3DCjyx-DesktopIcon.png"));
    SaveWidget->setLayout(layout);
    qDebug() << QDir::currentPath();
   QObject::connect(save, SIGNAL(clicked()), this, SLOT(onSaveData()));
}
//---------------------------------------------------------------------------
void qCjyxMainWindow::onSaveData()
{
    QString type1 = "DICOM(*.dcm)";
    QString type2 = "PLY(*.ply);;OBJ(*.obj)";
    auto node = nodeSelect->currentNode();
    QString DefaultFileName = "D:/" + QString::fromStdString(node->GetName());
    QString dialogTitle = "导出\" " + QString::fromStdString(node->GetName()) + "\"";

    if (node == nullptr)
    {
        std::cout << "node null" << std::endl;
        return;
    }
    std::string tagname = node->GetNodeTagName();
    cout << tagname << endl;
    if (tagname == "体数据")
    {
        std::cout << "node volume" << std::endl;
        //QString fileName = QFileDialog::getSaveFileName(nullptr, dialogTitle, DefaultFileName,type1);
        this->findChild<QDockWidget*>()->show();
        qCjyxLayoutManager* layoutManager = qCjyxApplication::application()->layoutManager();

        if (!layoutManager)
        {
            return;
        }
        layoutManager->setCurrentModule("DicomExporter");
        SaveWidget->hide();
    }
    else if (tagname == "模型")
    {
        std::cout << "node model" << std::endl;
        QString fileName = QFileDialog::getSaveFileName(nullptr, dialogTitle, DefaultFileName,type2);
        qCjyxIO::IOProperties properties;
        properties["fileName"] = fileName;
        properties["nodeID"] = node->GetID();
        auto filetype = qCjyxCoreApplication::application()->coreIOManager()->fileWriterFileType(node);
        qCjyxCoreApplication::application()->coreIOManager()->saveNodes(filetype, properties);
        SaveWidget->hide();
    }


    
}


//---------------------------------------------------------------------------
void qCjyxMainWindow::on_FileExitAction_triggered()
{
  this->close();
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::on_SDBSaveToDirectoryAction_triggered()
{
  // Q_D(qCjyxMainWindow);
  // open a file dialog to let the user choose where to save
  QString tempDir = qCjyxCoreApplication::application()->temporaryPath();
  QString saveDirName = QFileDialog::getExistingDirectory(
    this, tr("Cjyx Data Bundle Directory (Select Empty Directory)"),
    tempDir, QFileDialog::ShowDirsOnly);
  if (saveDirName.isEmpty())
    {
    std::cout << "No directory name chosen!" << std::endl;
    return;
    }
  // pass in a screen shot
  QWidget* widget = qCjyxApplication::application()->layoutManager()->viewport();
  QImage screenShot = ctk::grabVTKWidget(widget);
  qCjyxIO::IOProperties properties;
  properties["fileName"] = saveDirName;
  properties["screenShot"] = screenShot;
  qCjyxCoreApplication::application()->coreIOManager()
    ->saveNodes(QString("SceneFile"), properties);
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::on_SDBSaveToMRBAction_triggered()
{
  //
  // open a file dialog to let the user choose where to save
  // make sure it was selected and add a .cjyx to it if needed
  //
  QString fileName = QFileDialog::getSaveFileName(
    this, tr("Save Data Bundle File"),
    "", tr("Medical Reality Bundle (*.cjyx)"));

  if (fileName.isEmpty())
    {
    std::cout << "No directory name chosen!" << std::endl;
    return;
    }

  if ( !fileName.endsWith(".cjyx") )
    {
    fileName += QString(".cjyx");
    }
  qCjyxIO::IOProperties properties;
  properties["fileName"] = fileName;
  qCjyxCoreApplication::application()->coreIOManager()
    ->saveNodes(QString("SceneFile"), properties);
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::on_FileCloseSceneAction_triggered()
{
  Q_D(qCjyxMainWindow);
  if (d->confirmCloseScene())
    {
    qDebug("Close main DMML scene");
    qCjyxCoreApplication::application()->dmmlScene()->Clear(false);
    // Make sure we don't remember the last scene's filename to prevent
    // accidentally overwriting the scene.
    qCjyxCoreApplication::application()->dmmlScene()->SetURL("");
    // Set default scene file format to .dmml
    qCjyxCoreIOManager* coreIOManager = qCjyxCoreApplication::application()->coreIOManager();
    coreIOManager->setDefaultSceneFileType("DMML Scene (.dmml)");
    }
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::on_EditRecordMacroAction_triggered()
{
#ifdef Cjyx_USE_QtTesting
  qCjyxApplication::application()->testingUtility()->recordTestsBySuffix(QString("xml"));
#endif
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::on_EditPlayMacroAction_triggered()
{
#ifdef Cjyx_USE_QtTesting
  qCjyxApplication::application()->testingUtility()->openPlayerDialog();
#endif
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::on_EditUndoAction_triggered()
{
  qCjyxApplication::application()->dmmlScene()->Undo();
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::on_EditRedoAction_triggered()
{
  qCjyxApplication::application()->dmmlScene()->Redo();
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::on_ModuleHomeAction_triggered()
{
  this->setHomeModuleCurrent();
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::setLayout(int layout)
{
  qCjyxApplication::application()->layoutManager()->setLayout(layout);
}

//---------------------------------------------------------------------------
vtkDMMLAbstractViewNode* qCjyxMainWindow::layoutMaximizedViewNode()
{
  return qCjyxApplication::application()->layoutManager()->maximizedViewNode();
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::setLayoutMaximizedViewNode(vtkDMMLAbstractViewNode* viewNode)
{
  qCjyxApplication::application()->layoutManager()->setMaximizedViewNode(viewNode);
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::setLayoutNumberOfCompareViewRows(int num)
{
  qCjyxApplication::application()->layoutManager()->setLayoutNumberOfCompareViewRows(num);
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::setLayoutNumberOfCompareViewColumns(int num)
{
  qCjyxApplication::application()->layoutManager()->setLayoutNumberOfCompareViewColumns(num);
}

//-----------------------------------------------------------------------------
void qCjyxMainWindow::on_WindowErrorLogAction_triggered()
{
  Q_D(qCjyxMainWindow);
  d->ErrorLogWidget->show();
  d->ErrorLogWidget->activateWindow();
  d->ErrorLogWidget->raise();
}

//-----------------------------------------------------------------------------
void qCjyxMainWindow::onPythonConsoleToggled(bool toggled)
{
  Q_D(qCjyxMainWindow);
#ifdef Cjyx_USE_PYTHONQT
  ctkPythonConsole* pythonConsole = this->pythonConsole();
  if (!pythonConsole)
    {
    qCritical() << Q_FUNC_INFO << " failed: python console is not available";
    return;
    }
  if (d->PythonConsoleDockWidget)
    {
    // Dockable Python console
    if (toggled)
      {
      d->PythonConsoleDockWidget->activateWindow();
      QTextEdit* textEditWidget = pythonConsole->findChild<QTextEdit*>();
      if (textEditWidget)
        {
        textEditWidget->setFocus();
        }
      }
    }
  else
    {
    // Independent Python console
    if (toggled)
      {
      pythonConsole->show();
      pythonConsole->activateWindow();
      pythonConsole->raise();
      }
    else
      {
      pythonConsole->hide();
      }
    }
#else
  Q_UNUSED(toggled);
#endif
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::on_WindowToolbarsResetToDefaultAction_triggered()
{
  Q_D(qCjyxMainWindow);
  this->restoreState(d->StartupState);
}

//-----------------------------------------------------------------------------
void qCjyxMainWindow::onFileRecentLoadedActionTriggered()
{
  Q_D(qCjyxMainWindow);

  QAction* loadRecentFileAction = qobject_cast<QAction*>(this->sender());
  Q_ASSERT(loadRecentFileAction);

  // Clear menu if it applies
  if (loadRecentFileAction->property("clearMenu").isValid())
    {
    d->RecentlyLoadedFileProperties.clear();
    d->setupRecentlyLoadedMenu(d->RecentlyLoadedFileProperties);
    return;
    }

  QVariant fileParameters = loadRecentFileAction->property("fileParameters");
  Q_ASSERT(fileParameters.isValid());

  qCjyxIO::IOProperties fileProperties = fileParameters.toMap();
  qCjyxIO::IOFileType fileType =
      static_cast<qCjyxIO::IOFileType>(
        fileProperties.find("fileType").value().toString());

  qCjyxApplication* app = qCjyxApplication::application();

  vtkNew<vtkDMMLMessageCollection> userMessages;
  bool success = app->coreIOManager()->loadNodes(fileType, fileProperties, nullptr, userMessages);
  qCjyxIOManager::showLoadNodesResultDialog(success, userMessages);
}

//-----------------------------------------------------------------------------
void qCjyxMainWindow::closeEvent(QCloseEvent *event)
{
  Q_D(qCjyxMainWindow);

  // This is necessary because of a Qt bug on MacOS.
  // (https://bugreports.qt.io/browse/QTBUG-43344).
  // This flag prevents a second close event to be handled.
  if (d->IsClosing)
    {
    return;
    }
  d->IsClosing = true;

  if (d->confirmCloseApplication())
    {
    // Proceed with closing the application

    // Exit current module to leave it a chance to change the UI (e.g. layout)
    // before writing settings.
    d->ModuleSelectorToolBar->selectModule("");

    this->saveGUIState();
    event->accept();

    QTimer::singleShot(0, qApp, SLOT(closeAllWindows()));
    }
  else
    {
    // Request is cancelled, application will not be closed
    event->ignore();
    d->IsClosing = false;
    }
}

//-----------------------------------------------------------------------------
void qCjyxMainWindow::showEvent(QShowEvent *event)
{
  Q_D(qCjyxMainWindow);
  this->Superclass::showEvent(event);
  if (!d->WindowInitialShowCompleted)
    {
    d->WindowInitialShowCompleted = true;
    this->disclaimer();
    this->pythonConsoleInitialDisplay();

#ifdef Cjyx_BUILD_EXTENSIONMANAGER_SUPPORT
    qCjyxApplication* app = qCjyxApplication::application();
    if (app && app->extensionsManagerModel())
      {
      connect(app->extensionsManagerModel(), SIGNAL(extensionUpdatesAvailable(bool)),
        this, SLOT(setExtensionUpdatesAvailable(bool)));
      this->setExtensionUpdatesAvailable(!app->extensionsManagerModel()->availableUpdateExtensions().empty());
      }
#endif

    emit initialWindowShown();
    }
}

//-----------------------------------------------------------------------------
void qCjyxMainWindow::pythonConsoleInitialDisplay()
{
  Q_D(qCjyxMainWindow);
#ifdef Cjyx_USE_PYTHONQT
  qCjyxApplication * app = qCjyxApplication::application();
  if (qCjyxCoreApplication::testAttribute(qCjyxCoreApplication::AA_DisablePython))
    {
    return;
    }
  if (app->commandOptions()->showPythonInteractor() && d->PythonConsoleDockWidget)
    {
    d->PythonConsoleDockWidget->show();
    }
#endif
}

//-----------------------------------------------------------------------------
void qCjyxMainWindow::disclaimer()
{
  qCjyxCoreApplication * app = qCjyxCoreApplication::application();
  if (app->testAttribute(qCjyxCoreApplication::AA_EnableTesting) ||
      !app->coreCommandOptions()->pythonCode().isEmpty() ||
      !app->coreCommandOptions()->pythonScript().isEmpty())
    {
    return;
    }

  QString message = QString(Cjyx_DISCLAIMER_AT_STARTUP);
  if (message.isEmpty())
    {
    // No disclaimer message to show, skip the popup
    return;
    }

  // Replace "%1" in the text by the name and version of the application
  message = message.arg(app->applicationName() + " " + app->applicationVersion());

  ctkMessageBox* disclaimerMessage = new ctkMessageBox(this);
  disclaimerMessage->setAttribute( Qt::WA_DeleteOnClose, true );
  disclaimerMessage->setText(message);
  disclaimerMessage->setIcon(QMessageBox::Information);
  disclaimerMessage->setDontShowAgainSettingsKey("MainWindow/DontShowDisclaimerMessage");
  QTimer::singleShot(0, disclaimerMessage, SLOT(exec()));
}

//-----------------------------------------------------------------------------
void qCjyxMainWindow::setupMenuActions()
{
  Q_D(qCjyxMainWindow);

  d->ViewLayoutConventionalAction->setData(vtkDMMLLayoutNode::CjyxLayoutConventionalView);
  d->ViewLayoutConventionalWidescreenAction->setData(vtkDMMLLayoutNode::CjyxLayoutConventionalWidescreenView);
  d->ViewLayoutConventionalPlotAction->setData(vtkDMMLLayoutNode::CjyxLayoutConventionalPlotView);
  d->ViewLayoutFourUpAction->setData(vtkDMMLLayoutNode::CjyxLayoutFourUpView);
  d->ViewLayoutFourUpPlotAction->setData(vtkDMMLLayoutNode::CjyxLayoutFourUpPlotView);
  d->ViewLayoutFourUpPlotTableAction->setData(vtkDMMLLayoutNode::CjyxLayoutFourUpPlotTableView);
  d->ViewLayoutFourUpTableAction->setData(vtkDMMLLayoutNode::CjyxLayoutFourUpTableView);
  d->ViewLayoutDual3DAction->setData(vtkDMMLLayoutNode::CjyxLayoutDual3DView);
  d->ViewLayoutTriple3DAction->setData(vtkDMMLLayoutNode::CjyxLayoutTriple3DEndoscopyView);
  d->ViewLayoutOneUp3DAction->setData(vtkDMMLLayoutNode::CjyxLayoutOneUp3DView);
  d->ViewLayout3DTableAction->setData(vtkDMMLLayoutNode::CjyxLayout3DTableView);
  d->ViewLayoutOneUpPlotAction->setData(vtkDMMLLayoutNode::CjyxLayoutOneUpPlotView);
  d->ViewLayoutOneUpRedSliceAction->setData(vtkDMMLLayoutNode::CjyxLayoutOneUpRedSliceView);
  d->ViewLayoutOneUpYellowSliceAction->setData(vtkDMMLLayoutNode::CjyxLayoutOneUpYellowSliceView);
  d->ViewLayoutOneUpGreenSliceAction->setData(vtkDMMLLayoutNode::CjyxLayoutOneUpGreenSliceView);
  d->ViewLayoutTabbed3DAction->setData(vtkDMMLLayoutNode::CjyxLayoutTabbed3DView);
  d->ViewLayoutTabbedSliceAction->setData(vtkDMMLLayoutNode::CjyxLayoutTabbedSliceView);
  d->ViewLayoutCompareAction->setData(vtkDMMLLayoutNode::CjyxLayoutCompareView);
  d->ViewLayoutCompareWidescreenAction->setData(vtkDMMLLayoutNode::CjyxLayoutCompareWidescreenView);
  d->ViewLayoutCompareGridAction->setData(vtkDMMLLayoutNode::CjyxLayoutCompareGridView);
  d->ViewLayoutThreeOverThreeAction->setData(vtkDMMLLayoutNode::CjyxLayoutThreeOverThreeView);
  d->ViewLayoutThreeOverThreePlotAction->setData(vtkDMMLLayoutNode::CjyxLayoutThreeOverThreePlotView);
  d->ViewLayoutFourOverFourAction->setData(vtkDMMLLayoutNode::CjyxLayoutFourOverFourView);
  d->ViewLayoutTwoOverTwoAction->setData(vtkDMMLLayoutNode::CjyxLayoutTwoOverTwoView);
  d->ViewLayoutSideBySideAction->setData(vtkDMMLLayoutNode::CjyxLayoutSideBySideView);
  d->ViewLayoutFourByThreeSliceAction->setData(vtkDMMLLayoutNode::CjyxLayoutFourByThreeSliceView);
  d->ViewLayoutFourByTwoSliceAction->setData(vtkDMMLLayoutNode::CjyxLayoutFourByTwoSliceView);
  d->ViewLayoutFiveByTwoSliceAction->setData(vtkDMMLLayoutNode::CjyxLayoutFiveByTwoSliceView);
  d->ViewLayoutThreeByThreeSliceAction->setData(vtkDMMLLayoutNode::CjyxLayoutThreeByThreeSliceView);

  d->ViewLayoutCompare_2_viewersAction->setData(2);
  d->ViewLayoutCompare_3_viewersAction->setData(3);
  d->ViewLayoutCompare_4_viewersAction->setData(4);
  d->ViewLayoutCompare_5_viewersAction->setData(5);
  d->ViewLayoutCompare_6_viewersAction->setData(6);
  d->ViewLayoutCompare_7_viewersAction->setData(7);
  d->ViewLayoutCompare_8_viewersAction->setData(8);

  d->ViewLayoutCompareWidescreen_2_viewersAction->setData(2);
  d->ViewLayoutCompareWidescreen_3_viewersAction->setData(3);
  d->ViewLayoutCompareWidescreen_4_viewersAction->setData(4);
  d->ViewLayoutCompareWidescreen_5_viewersAction->setData(5);
  d->ViewLayoutCompareWidescreen_6_viewersAction->setData(6);
  d->ViewLayoutCompareWidescreen_7_viewersAction->setData(7);
  d->ViewLayoutCompareWidescreen_8_viewersAction->setData(8);

  d->ViewLayoutCompareGrid_2x2_viewersAction->setData(2);
  d->ViewLayoutCompareGrid_3x3_viewersAction->setData(3);
  d->ViewLayoutCompareGrid_4x4_viewersAction->setData(4);

  d->WindowErrorLogAction->setIcon(
    this->style()->standardIcon(QStyle::SP_MessageBoxCritical));

  if (this->errorLogWidget())
    {
    d->setErrorLogIconHighlighted(false);
    this->errorLogWidget()->installEventFilter(this);
    }
#ifdef Cjyx_USE_PYTHONQT
  if (this->pythonConsole())
    {
    this->pythonConsole()->installEventFilter(this);
    }
#endif

  qCjyxApplication * app = qCjyxApplication::application();

#ifdef Cjyx_BUILD_EXTENSIONMANAGER_SUPPORT
  d->ViewExtensionsManagerAction->setVisible(
    app->revisionUserSettings()->value("Extensions/ManagerEnabled").toBool());
#else
  d->ViewExtensionsManagerAction->setVisible(false);
  //d->WindowToolBarsMenu->removeAction(d->ViewExtensionsManagerAction);
#endif

#if defined Cjyx_USE_QtTesting && defined Cjyx_BUILD_CLI_SUPPORT
  if (app->commandOptions()->enableQtTesting() ||
      app->userSettings()->value("QtTesting/Enabled").toBool())
    {
    d->EditPlayMacroAction->setVisible(true);
    d->EditRecordMacroAction->setVisible(true);
    app->testingUtility()->addPlayer(new qCjyxCLIModuleWidgetEventPlayer());
    }
#endif
  Q_UNUSED(app);
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::on_LoadDICOMAction_triggered()
{
    Q_D(qCjyxMainWindow);
    int num = qCjyxApplication::application()->dmmlScene()->GetNodesByClass("vtkDMMLScalarVolumeNode")->GetNumberOfItems();
    if (num > 0)
    {
        QMessageBox* message = new QMessageBox(QMessageBox::Information, QString(tr("提示")), QString::fromStdString("当前场景中体数据已经修改，是否保存？新建项目文件将清空当前场景。"), QMessageBox::NoButton, this);
        QAbstractButton* saveButton =
            message->addButton(qCjyxMainWindow::tr("保存项目文件"), QMessageBox::ActionRole);
        QAbstractButton* exitButton =
            message->addButton(qCjyxMainWindow::tr("不保存（直接清空项目文件）"), QMessageBox::ActionRole);
        QAbstractButton* cancelButton =
            message->addButton(qCjyxMainWindow::tr("取消新建项目"), QMessageBox::ActionRole);
        message->exec();
        if (message->clickedButton() == saveButton)
        {
            // \todo Check if the save data dialog was "applied" and close the
            // app in that case
            d->SaveSceneAction->trigger();

        }
        else if (message->clickedButton() == exitButton)
        {
            d->FileCloseSceneAction->trigger();
        }
        else
        {
            return;
        }

    }
    PythonQt::init();
    PythonQtObjectPtr context = PythonQt::self()->getMainModule();
    QString fileName = QFileDialog::getOpenFileName(nullptr, "新建项目文件",
        "D:/",
        "DICOM(*.dcm)");
    std::cout << fileName.toStdString() << std::endl;
    if (fileName == "")
        return;
    //qCjyxApplication::application()->coreIOManager()->loadFile(fileName);
    d->FileCloseSceneAction->trigger();
    context.addVariable("filename", fileName);
    PyRun_SimpleString("cjyx.util.loadVolume(filename)");

    qDebug() << num;
    if (num == 1)
    {

        PyRun_SimpleString("cjyx.modules.patient.widgetRepresentation().ViewSet()");
    }

}

//---------------------------------------------------------------------------
void qCjyxMainWindow::on_DataManageAction_triggered()
{
    Q_D(qCjyxMainWindow);
    PythonQt::init();
    PythonQtObjectPtr context = PythonQt::self()->getMainModule();
    PyRun_SimpleString("cjyx.modules.setting.widgetRepresentation()");
    PyRun_SimpleString("index = cjyx.modules.setting.widgetRepresentation().self().windowlayout");
    QVariant index = context.getVariable("index");
    if (index == 0 || index == 1)
    {
        d->PanelDockWidget->show();
        qCjyxLayoutManager* layoutManager = qCjyxApplication::application()->layoutManager();

        if (!layoutManager)
        {
            return;
        }
        layoutManager->setCurrentModule("DataManager");
        
    }
    else
    {
        d->TwoLayoutDockWidget->show();
        context.evalScript(QString(
            "widget1 = cjyx.util.findChild(cjyx.util.mainWindow(), 'DataManagerWidget') \n"
            "dataManagerWidget = cjyx.modules.datamanager.widgetRepresentation() \n"
            "a = widget1.layout() \n"
            "a.addWidget(dataManagerWidget) \n"
            "dataManagerWidget.show() \n"
            
        ));
        
    }
}
//---------------------------------------------------------------------------
void qCjyxMainWindow::on_VolumeRenderingAction_triggered()
{

    this->findChild<QDockWidget*>()->show();
    qCjyxLayoutManager* layoutManager = qCjyxApplication::application()->layoutManager();

    if (!layoutManager)
    {
        return;
    }
    layoutManager->setCurrentModule("VolumeRendering");
}
//---------------------------------------------------------------------------
void qCjyxMainWindow::on_VolumeCropAction_triggered()
{
    this->findChild<QDockWidget*>()->show();
    qCjyxLayoutManager* layoutManager = qCjyxApplication::application()->layoutManager();

    if (!layoutManager)
    {
        return;
    }
    layoutManager->setCurrentModule("CropVolume");
}
//---------------------------------------------------------------------------
void qCjyxMainWindow::on_MeasureAction_triggered()
{
    this->findChild<QDockWidget*>()->show();
    qCjyxLayoutManager* layoutManager = qCjyxApplication::application()->layoutManager();

    if (!layoutManager)
    {
        return;
    }
    layoutManager->setCurrentModule("celiang");
}
//---------------------------------------------------------------------------
void qCjyxMainWindow::on_ModuleToolAction_triggered()
{
    this->findChild<QDockWidget*>()->show();
    qCjyxLayoutManager* layoutManager = qCjyxApplication::application()->layoutManager();

    if (!layoutManager)
    {
        return;
    }
    layoutManager->setCurrentModule("SurfaceUtils");
}
//---------------------------------------------------------------------------
void qCjyxMainWindow::on_ImageRebuildAction_triggered()
{
    this->findChild<QDockWidget*>()->show();
    qCjyxLayoutManager* layoutManager = qCjyxApplication::application()->layoutManager();

    if (!layoutManager)
    {
        return;
    }
    layoutManager->setCurrentModule("NewRebuild");
}
//---------------------------------------------------------------------------
void qCjyxMainWindow::on_DICOMBrowerAction_triggered()
{
    Q_D(qCjyxMainWindow);
    if (d->DICOMBrowerAction->isChecked())
    {
        d->ModulePanel->show();
        d->FourViewWidget->setVisible(false);
        d->PatientWidget->setVisible(true);

        qCjyxLayoutManager* layoutManager = qCjyxApplication::application()->layoutManager();
        qCjyxAbstractModuleWidget* PatientModule =
            dynamic_cast<qCjyxAbstractModuleWidget*>(qCjyxApplication::application()->moduleManager()->module("Patient")->widgetRepresentation());

        PatientModule->setParent(d->PatientWidget);
        d->PatientWidget->layout()->addWidget(PatientModule);
        PatientModule->show();
        d->FourViewAction->setChecked(false);

    }
    else
    {
        d->DICOMBrowerAction->setChecked(true);
    }
}
//--------------------------------------------------------------------------
void qCjyxMainWindow::on_FourViewAction_triggered()
{
    Q_D(qCjyxMainWindow);
    d->FourViewWidget->setVisible(true);
    d->PatientWidget->setVisible(false);
    d->DICOMBrowerAction->setChecked(false);
    
}
//---------------------------------------------------------------------------

void qCjyxMainWindow::on_ResetViewAction_triggered()
{
    qCjyxLayoutManager* layoutManager = qCjyxApplication::application()->layoutManager();
    layoutManager->sliceWidget("Red")->fitSliceToBackground();
    layoutManager->sliceWidget("Green")->fitSliceToBackground();
    layoutManager->sliceWidget("Yellow")->fitSliceToBackground();
    layoutManager->threeDWidget("View1")->threeDController()->resetFocalPoint();
}

//----------------------------------------------------------------------------
void qCjyxMainWindow::onWarningsOrErrorsOccurred(ctkErrorLogLevel::LogLevel logLevel)
{
  Q_D(qCjyxMainWindow);
  if(logLevel > ctkErrorLogLevel::Info)
    {
    d->setErrorLogIconHighlighted(true);
    }
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::on_EditApplicationSettingsAction_triggered()
{
  //qCjyxApplication::application()->openSettingsDialog();
    PythonQt::init();
    PythonQtObjectPtr context = PythonQt::self()->getMainModule();
    PyRun_SimpleString("cjyx.modules.setting.widgetRepresentation().show()");

}

//---------------------------------------------------------------------------
void qCjyxMainWindow::addFileToRecentFiles(const qCjyxIO::IOProperties& fileProperties)
{
  Q_D(qCjyxMainWindow);

  // Remove previous instance of the same file name. Since the file name can be slightly different
  // (different directory separator, etc.) we don't do a binary compare but compare QFileInfo.
  QString fileName = fileProperties.value("fileName").toString();
  if (fileName.isEmpty())
    {
    return;
    }
  QFileInfo newFileInfo(fileName);
  for (auto propertiesIt = d->RecentlyLoadedFileProperties.begin(); propertiesIt != d->RecentlyLoadedFileProperties.end() ;)
    {
    QFileInfo existingFileInfo(propertiesIt->value("fileName").toString());
    if (newFileInfo == existingFileInfo)
      {
      // remove previous instance
      propertiesIt = d->RecentlyLoadedFileProperties.erase(propertiesIt);
      }
    else
      {
      propertiesIt++;
      }
    }

  d->RecentlyLoadedFileProperties.enqueue(fileProperties);
  d->filterRecentlyLoadedFileProperties();
  d->setupRecentlyLoadedMenu(d->RecentlyLoadedFileProperties);
  // Keep the settings up-to-date
  qCjyxMainWindowPrivate::writeRecentlyLoadedFiles(d->RecentlyLoadedFileProperties);
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::onNewFileLoaded(const qCjyxIO::IOProperties& fileProperties)
{
  this->addFileToRecentFiles(fileProperties);
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::onFileSaved(const qCjyxIO::IOProperties& fileProperties)
{
  Q_D(qCjyxMainWindow);
  QString fileName = fileProperties["fileName"].toString();
  if (fileName.isEmpty())
    {
    return;
    }
  // Adding every saved file to the recent files list could quickly overwrite the entire list,
  // therefore we only add the scene file.
  if (fileName.endsWith(".dmml", Qt::CaseInsensitive)
    || fileName.endsWith(".cjyx", Qt::CaseInsensitive))
    {
    // Scene file properties do not contain fileType and it contains screenshot,
    // which can cause complication when attempted to be stored,
    // therefore we create a new clean property set.
    qCjyxIO::IOProperties properties;
    properties["fileName"] = fileName;
    properties["fileType"] = QString("SceneFile");
    this->addFileToRecentFiles(properties);
    }
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::on_CopyAction_triggered()
{
  QWidget* focused = QApplication::focusWidget();
  if (focused != nullptr)
    {
    QApplication::postEvent(focused,
                            new QKeyEvent( QEvent::KeyPress,
                                           Qt::Key_C,
                                           Qt::ControlModifier));
    QApplication::postEvent(focused,
                            new QKeyEvent( QEvent::KeyRelease,
                                           Qt::Key_C,
                                           Qt::ControlModifier));
    }
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::on_PasteAction_triggered()
{
  QWidget* focused = QApplication::focusWidget();
  if (focused != nullptr)
    {
    QApplication::postEvent(focused,
                            new QKeyEvent( QEvent::KeyPress,
                                           Qt::Key_V,
                                           Qt::ControlModifier));
    QApplication::postEvent(focused,
                            new QKeyEvent( QEvent::KeyRelease,
                                           Qt::Key_V,
                                           Qt::ControlModifier));
    }
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::on_CutAction_triggered()
{
  QWidget* focused = QApplication::focusWidget();
  if (focused != nullptr)
    {
    QApplication::postEvent(focused,
                            new QKeyEvent( QEvent::KeyPress,
                                           Qt::Key_X,
                                           Qt::ControlModifier));
    QApplication::postEvent(focused,
                            new QKeyEvent( QEvent::KeyRelease,
                                           Qt::Key_X,
                                           Qt::ControlModifier));
    }
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::on_ViewExtensionsManagerAction_triggered()
{
#ifdef Cjyx_BUILD_EXTENSIONMANAGER_SUPPORT
  qCjyxApplication * app = qCjyxApplication::application();
  app->openExtensionsManagerDialog();
#endif
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::onModuleLoaded(const QString& moduleName)
{
  Q_D(qCjyxMainWindow);

  qCjyxAbstractCoreModule* coreModule =
    qCjyxApplication::application()->moduleManager()->module(moduleName);
  qCjyxAbstractModule* module = qobject_cast<qCjyxAbstractModule*>(coreModule);
  if (!module)
    {
    return;
    }

  // Module ToolBar
  QAction * action = module->action();
  if (!action || action->icon().isNull())
    {
    return;
    }

  Q_ASSERT(action->data().toString() == module->name());
  Q_ASSERT(action->text() == module->title());

  // Add action to ToolBar if it's an "allowed" action
  int index = d->FavoriteModules.indexOf(module->name());
  if (index != -1)
    {
    // find the location of where to add the action.
    // Note: FavoriteModules is sorted
    QAction* beforeAction = nullptr; // 0 means insert at end
    foreach(QAction* toolBarAction, d->ModuleToolBar->actions())
      {
      bool isActionAFavoriteModule =
        (d->FavoriteModules.indexOf(toolBarAction->data().toString()) != -1);
      if ( isActionAFavoriteModule &&
          d->FavoriteModules.indexOf(toolBarAction->data().toString()) > index)
        {
        beforeAction = toolBarAction;
        break;
        }
      }
    d->ModuleToolBar->insertAction(beforeAction, action);
    action->setParent(d->ModuleToolBar);
    }
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::onModuleAboutToBeUnloaded(const QString& moduleName)
{
  Q_D(qCjyxMainWindow);

  if (d->ModuleSelectorToolBar->selectedModule() == moduleName)
    {
    d->ModuleSelectorToolBar->selectModule("");
    }

  foreach(QAction* action, d->ModuleToolBar->actions())
    {
    if (action->data().toString() == moduleName)
      {
      d->ModuleToolBar->removeAction(action);
      return;
      }
    }
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::onDMMLSceneModified(vtkObject* sender)
{
  Q_UNUSED(sender);
  //Q_D(qCjyxMainWindow);
  //
  //vtkDMMLScene* scene = vtkDMMLScene::SafeDownCast(sender);
  //if (scene && scene->IsBatchProcessing())
  //  {
  //  return;
  //  }
  //d->EditUndoAction->setEnabled(scene && scene->GetNumberOfUndoLevels());
  //d->EditRedoAction->setEnabled(scene && scene->GetNumberOfRedoLevels());
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::onLayoutActionTriggered(QAction* action)
{
  Q_D(qCjyxMainWindow);
  bool found = true;
  // std::cerr << "onLayoutActionTriggered: " << action->text().toStdString() << std::endl;
  //foreach(QAction* maction, d->LayoutMenu->actions())
  //  {
  //  if (action->text() == maction->text())
  //    {
  //    found = true;
  //    break;
  //    }
  //  }
  cout << action->data().toInt() << endl;
  d->LayoutButton->setDefaultAction(action);
  if (found && !action->data().isNull())
    {
      cout << action->data().toInt() << endl;
    this->setLayout(action->data().toInt());
    this->setLayoutMaximizedViewNode(nullptr);
    }
}


void qCjyxMainWindow::onImageRebuildAction1(QAction* action)
{
    Q_D(qCjyxMainWindow);
    d->ImageRebuildAction->trigger();
    QString name = action->objectName();
    if (name != "")
    {
        PythonQt::init();
        PythonQtObjectPtr context = PythonQt::self()->getMainModule();
        context.addVariable("name", name);
        PyRun_SimpleString("cjyx.modules.newrebuild.widgetRepresentation().self().OnMeunChanged(name)");
    }
}

void qCjyxMainWindow::onModuleToolMenu(QAction* action)
{
    Q_D(qCjyxMainWindow);
    d->ModuleToolAction->trigger();
    QString name = action->objectName();
    if (name != "")
    {
        PythonQt::init();
        PythonQtObjectPtr context = PythonQt::self()->getMainModule();
        context.addVariable("name", name);
        if(name == "ActionSmoothing")
            PyRun_SimpleString("cjyx.modules.surfaceutils.widgetRepresentation().self().clickSmoothing(True)");
        else if (name == "ActionFill")
            PyRun_SimpleString("cjyx.modules.surfaceutils.widgetRepresentation().self().clickFill(True)");
        else if (name == "ActionChouQu")
            PyRun_SimpleString("cjyx.modules.surfaceutils.widgetRepresentation().self().clickDecimation(True)");
        else if (name == "ActionFaXian")
            PyRun_SimpleString("cjyx.modules.surfaceutils.widgetRepresentation().self().clickNormal(True)");
        else if (name == "ActionBool")
            PyRun_SimpleString("cjyx.modules.surfaceutils.widgetRepresentation().self().clickBoolean(True)");
        else if (name == "ActionCrop")
            PyRun_SimpleString("cjyx.modules.surfaceutils.widgetRepresentation().self().clickPlaneCrop(True)");
        else if (name == "ActionRCrop")
            PyRun_SimpleString("cjyx.modules.surfaceutils.widgetRepresentation().self().clickRoiCrop(True)");


    }

}

void qCjyxMainWindow::onMarkupToolBar(bool a)
{
    QAction* action = dynamic_cast<QAction*>(sender());
    if (action->objectName() != "")
    {
        PythonQt::init();
        PythonQtObjectPtr context = PythonQt::self()->getMainModule();
        if(action->objectName() == "AddPointAction")
            PyRun_SimpleString("cjyx.modules.celiang.widgetRepresentation().self().OnAddPointList()");
        else if (action->objectName() == "LengthAction")
            PyRun_SimpleString("cjyx.modules.celiang.widgetRepresentation().self().OnAddLine()");
        else if (action->objectName() == "AngleAction")
            PyRun_SimpleString("cjyx.modules.celiang.widgetRepresentation().self().OnAddAngle()");
        else if (action->objectName() == "NoteAction")
            PyRun_SimpleString("cjyx.modules.celiang.widgetRepresentation().self().OnAddNote()");

    }
}
//---------------------------------------------------------------------------
void qCjyxMainWindow::onLayoutCompareActionTriggered(QAction* action)
{
  Q_D(qCjyxMainWindow);

  // std::cerr << "onLayoutCompareActionTriggered: " << action->text().toStdString() << std::endl;

  // we need to communicate both the layout change and the number of viewers.
  this->setLayout(d->ViewLayoutCompareAction->data().toInt());
  this->setLayoutMaximizedViewNode(nullptr);
  this->setLayoutNumberOfCompareViewRows(action->data().toInt());
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::onLayoutCompareWidescreenActionTriggered(QAction* action)
{
  Q_D(qCjyxMainWindow);

  // std::cerr << "onLayoutCompareWidescreenActionTriggered: " << action->text().toStdString() << std::endl;

  // we need to communicate both the layout change and the number of viewers.
  this->setLayout(d->ViewLayoutCompareWidescreenAction->data().toInt());
  this->setLayoutMaximizedViewNode(nullptr);
  this->setLayoutNumberOfCompareViewColumns(action->data().toInt());
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::onLayoutCompareGridActionTriggered(QAction* action)
{
  Q_D(qCjyxMainWindow);

  // std::cerr << "onLayoutCompareGridActionTriggered: " << action->text().toStdString() << std::endl;

  // we need to communicate both the layout change and the number of viewers.
  this->setLayout(d->ViewLayoutCompareGridAction->data().toInt());
  this->setLayoutMaximizedViewNode(nullptr);
  this->setLayoutNumberOfCompareViewRows(action->data().toInt());
  this->setLayoutNumberOfCompareViewColumns(action->data().toInt());
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::onLayoutChanged(int layout)
{
  Q_D(qCjyxMainWindow);

  // Update the layout button icon with the current view layout.

  // Actions with a menu are ignored, because they are just submenus without
  // data assigned, so they should never be triggered (they could be triggered
  // at startup, when layout is set to CjyxLayoutInitialView = 0).

  //foreach(QAction* action, d->LayoutMenu->actions())
  //  {
  //  if (!action->menu() && action->data().toInt() == layout)
  //    {
  //    d->LayoutButton->setDefaultAction(action);
  //    }
  //  }
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::dragEnterEvent(QDragEnterEvent *event)
{
  qCjyxApplication::application()->ioManager()->dragEnterEvent(event);
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::dropEvent(QDropEvent *event)
{
  qCjyxApplication::application()->ioManager()->dropEvent(event);
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::restoreGUIState(bool force/*=false*/)
{
  Q_D(qCjyxMainWindow);
  QSettings settings;
  settings.beginGroup("MainWindow");
  this->setToolButtonStyle(settings.value("ShowToolButtonText").toBool()
                        ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly);
  bool restore = settings.value("RestoreGeometry", false).toBool();
  if (restore || force)
    {
    this->restoreGeometry(settings.value("geometry").toByteArray());
    this->restoreState(settings.value("windowState").toByteArray());
    d->LayoutManager->setLayout(settings.value("layout").toInt());
    }
  settings.endGroup();
  d->FavoriteModules << settings.value("Modules/FavoriteModules").toStringList();

  foreach(const qCjyxIO::IOProperties& fileProperty, qCjyxMainWindowPrivate::readRecentlyLoadedFiles())
    {
    d->RecentlyLoadedFileProperties.enqueue(fileProperty);
    }
  d->filterRecentlyLoadedFileProperties();
  d->setupRecentlyLoadedMenu(d->RecentlyLoadedFileProperties);
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::saveGUIState(bool force/*=false*/)
{
  Q_D(qCjyxMainWindow);
  QSettings settings;
  settings.beginGroup("MainWindow");
  bool restore = settings.value("RestoreGeometry", false).toBool();
  if (restore || force)
    {
    settings.setValue("geometry", this->saveGeometry());
    settings.setValue("windowState", this->saveState());
    settings.setValue("layout", d->LayoutManager->layout());
    }
  settings.endGroup();
  qCjyxMainWindowPrivate::writeRecentlyLoadedFiles(d->RecentlyLoadedFileProperties);
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::setHomeModuleCurrent()
{
  Q_D(qCjyxMainWindow);
  QSettings settings;
  QString homeModule = settings.value("Modules/HomeModule").toString();
  d->ModuleSelectorToolBar->selectModule(homeModule);
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::restoreToolbars()
{
  Q_D(qCjyxMainWindow);
  this->restoreState(d->StartupState);
}

//---------------------------------------------------------------------------
bool qCjyxMainWindow::eventFilter(QObject* object, QEvent* event)
{
  Q_D(qCjyxMainWindow);
  if (object == this->errorLogWidget())
    {
    if (event->type() == QEvent::ActivationChange
        && this->errorLogWidget()->isActiveWindow())
      {
      d->setErrorLogIconHighlighted(false);
      }
    }
#ifdef Cjyx_USE_PYTHONQT
  if (object == this->pythonConsole())
    {
    if (event->type() == QEvent::Hide)
      {
      bool wasBlocked = d->PythonConsoleToggleViewAction->blockSignals(true);
      d->PythonConsoleToggleViewAction->setChecked(false);
      d->PythonConsoleToggleViewAction->blockSignals(wasBlocked);
      }
    }
#endif
  return this->Superclass::eventFilter(object, event);
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::changeEvent(QEvent* event)
{
  Q_D(qCjyxMainWindow);
  switch (event->type())
    {
    case QEvent::PaletteChange:
      {
      d->updatePythonConsolePalette();
      break;
      }
    default:
      break;
    }
}

//---------------------------------------------------------------------------
void qCjyxMainWindow::setExtensionUpdatesAvailable(bool updateAvailable)
{
#ifdef Cjyx_BUILD_EXTENSIONMANAGER_SUPPORT
  Q_D(qCjyxMainWindow);
  qCjyxApplication* app = qCjyxApplication::application();
  if (!app || !app->revisionUserSettings()->value("Extensions/ManagerEnabled").toBool())
    {
    return;
    }

  // Check if there was a change
  const char extensionUpdateAvailablePropertyName[] = "extensionUpdateAvailable";
  if (d->ViewExtensionsManagerAction->property(extensionUpdateAvailablePropertyName).toBool() == updateAvailable)
    {
    // no change
    return;
    }
  d->ViewExtensionsManagerAction->setProperty(extensionUpdateAvailablePropertyName, updateAvailable);

  if (updateAvailable)
    {
    d->ViewExtensionsManagerAction->setIcon(QIcon(":/Icons/ExtensionNotificationIcon.png"));
    }
  else
    {
    d->ViewExtensionsManagerAction->setIcon(QIcon(":/Icons/ExtensionDefaultIcon.png"));
    }
#endif
}

void qCjyxMainWindow::setTwoLayout()
{

}
void qCjyxMainWindow::setLeftLayout()
{
    this->addDockWidget(Qt::LeftDockWidgetArea, this->findChild<QDockWidget*>());
    Layoutnumber = 0;
}
void qCjyxMainWindow::setRightLayout()
{
    this->addDockWidget(Qt::RightDockWidgetArea, this->findChild<QDockWidget*>());
    Layoutnumber = 1;
}