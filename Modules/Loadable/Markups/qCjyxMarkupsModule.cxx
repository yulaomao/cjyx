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
#include <QDebug>
#include <QSettings>
#include <QMainWindow>
#include <QMenu>

// DMMLDisplayableManager includes
#include <vtkDMMLSliceViewDisplayableManagerFactory.h>
#include <vtkDMMLThreeDViewDisplayableManagerFactory.h>

// QTGUI includes
#include <qCjyxApplication.h>
#include "qCjyxCoreApplication.h"
#include <qCjyxIOManager.h>
#include <qCjyxNodeWriter.h>
#include "qCjyxModuleManager.h"

#include "vtkDMMLScene.h"

// SubjectHierarchy Plugins includes
#include "qCjyxSubjectHierarchyPluginHandler.h"
#include "qCjyxSubjectHierarchyMarkupsPlugin.h"

// Markups module includes
#include "qCjyxMarkupsModule.h"
#include "qCjyxMarkupsModuleWidget.h"
#include "qCjyxMarkupsReader.h"
#include "qCjyxMarkupsWriter.h"

// Markups nodes includes
#include "vtkDMMLMarkupsAngleNode.h"
#include "vtkDMMLMarkupsDisplayNode.h"
#include "vtkDMMLMarkupsClosedCurveNode.h"
#include "vtkDMMLMarkupsCurveNode.h"
#include "vtkDMMLMarkupsFiducialNode.h"
#include "vtkDMMLMarkupsLineNode.h"
#include "vtkDMMLMarkupsPlaneNode.h"
#include "vtkDMMLMarkupsROINode.h"

// Markups logic includes
#include "vtkCjyxMarkupsLogic.h"

// Markups widgets
#include "qDMMLMarkupsAngleMeasurementsWidget.h"
#include "qDMMLMarkupsCurveSettingsWidget.h"
#include "qDMMLMarkupsPlaneWidget.h"
#include "qDMMLMarkupsROIWidget.h"
#include "qDMMLMarkupsToolBar.h"
#include "qDMMLMarkupsOptionsWidgetsFactory.h"
#include "qDMMLNodeComboBox.h"

// DisplayableManager initialization
#include <vtkAutoInit.h>

VTK_MODULE_INIT(vtkCjyxMarkupsModuleDMMLDisplayableManager);

static const double UPDATE_VIRTUAL_OUTPUT_NODES_PERIOD_SEC = 0.020; // refresh output with a maximum of 50FPS

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_Markups
class qCjyxMarkupsModulePrivate
{
QVTK_OBJECT
Q_DECLARE_PUBLIC(qCjyxMarkupsModule);
protected:
  qCjyxMarkupsModule* const q_ptr;
public:
  qCjyxMarkupsModulePrivate(qCjyxMarkupsModule& object);

  /// Adds Markups toolbar to the application GUI
  virtual void addToolBar();

  virtual ~qCjyxMarkupsModulePrivate();
  qDMMLMarkupsToolBar* ToolBar;
  bool MarkupsModuleOwnsToolBar{ true };
  bool AutoShowToolBar{ true };
  vtkWeakPointer<vtkDMMLMarkupsNode> MarkupsToShow;

};

//-----------------------------------------------------------------------------
// qCjyxMarkupsModulePrivate methods

//-----------------------------------------------------------------------------
qCjyxMarkupsModulePrivate::qCjyxMarkupsModulePrivate(qCjyxMarkupsModule& object)
  : q_ptr(&object)
{
  this->ToolBar = new qDMMLMarkupsToolBar;
  this->ToolBar->setWindowTitle(QObject::tr("Markups"));
  this->ToolBar->setObjectName("MarkupsToolbar");
  this->ToolBar->setVisible(false);
}

//-----------------------------------------------------------------------------
qCjyxMarkupsModulePrivate::~qCjyxMarkupsModulePrivate()
{
  if (this->MarkupsModuleOwnsToolBar)
    {
    // the toolbar has not been added to the main window
    // so it is still owned by this class, therefore
    // we are responsible for deleting it
    delete this->ToolBar;
    this->ToolBar = nullptr;
    }

  /// NOTE: This prevents deletion of QWidgets by the destructor of the factory,
  /// which produces a segmentation fault. Though I can't confirm, I believe
  /// this behaviour is due to deletion of QWidgets in global scope destructors
  /// and how this conflicts with the lifecycle of QWidgets.
  qDMMLMarkupsOptionsWidgetsFactory::instance()->unregisterAll();
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModulePrivate::addToolBar()
{
  Q_Q(qCjyxMarkupsModule);

  QMainWindow* mainWindow = qCjyxApplication::application()->mainWindow();
  if (mainWindow == nullptr)
    {
    qDebug("qCjyxMarkupsModulePrivate::addToolBar: no main window is available, toolbar is not added");
    return;
    }

  this->ToolBar->setWindowTitle("Markups");
  this->ToolBar->setObjectName("MarkupsToolBar");
  //// Add a toolbar break to make the sequence toolbar appear in a separate row
  //// (it is a long toolbar and would make many toolbar buttons disappear from
  //// all the standard toolbars if they are all displayed in a single row).
  mainWindow->addToolBarBreak();
  mainWindow->addToolBar(this->ToolBar);
  this->MarkupsModuleOwnsToolBar = false;
  foreach(QMenu * toolBarMenu, mainWindow->findChildren<QMenu*>())
    {
    if (toolBarMenu->objectName() == QString("WindowToolBarsMenu"))
      {
      toolBarMenu->addAction(this->ToolBar->toggleViewAction());
      break;
      }
    }

  //// Main window takes care of saving and restoring toolbar geometry and state.
  //// However, when state is restored the markups toolbar was not created yet.
  //// We need to restore the main window state again, now, that the Markups toolbar is available.
  QSettings settings;
  settings.beginGroup("MainWindow");
  bool restore = settings.value("RestoreGeometry", false).toBool();
  if (restore)
    {
    mainWindow->restoreState(settings.value("windowState").toByteArray());
    }
  this->ToolBar->initializeToolBarLayout();
}

//-----------------------------------------------------------------------------
// qCjyxMarkupsModule methods

//-----------------------------------------------------------------------------
qCjyxMarkupsModule::qCjyxMarkupsModule(QObject* _parent)
  : Superclass(_parent), d_ptr(new qCjyxMarkupsModulePrivate(*this))
{
  Q_D(qCjyxMarkupsModule);
  /*
  vtkDMMLScene* scene = qCjyxCoreApplication::application()->dmmlScene();
  if (scene)
    {
    // Need to listen for any new makrups nodes being added to show toolbar
    this->qvtkConnect(scene, vtkDMMLScene::NodeAddedEvent, this, SLOT(onNodeAddedEvent(vtkObject*, vtkObject*)));
    }
    */
}

//-----------------------------------------------------------------------------
QStringList qCjyxMarkupsModule::categories()const
{
  return QStringList() << "" << "Informatics";
}

//-----------------------------------------------------------------------------
qCjyxMarkupsModule::~qCjyxMarkupsModule() = default;

//-----------------------------------------------------------------------------
QString qCjyxMarkupsModule::helpText()const
{
  QString help =
    "A module to create and manage markups in 2D and 3D."
    " Replaces the Annotations module for fiducials.";
  help += this->defaultDocumentationLink();
  return help;
}

//-----------------------------------------------------------------------------
QString qCjyxMarkupsModule::acknowledgementText()const
{
  return "This work was supported by NAMIC, NAC, and the Cjyx Community.";
}

//-----------------------------------------------------------------------------
QStringList qCjyxMarkupsModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Nicole Aucoin (BWH)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qCjyxMarkupsModule::icon()const
{
  return QIcon(":/Icons/Markups.png");
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModule::setup()
{
  Q_D(qCjyxMarkupsModule);
  this->Superclass::setup();

  vtkCjyxMarkupsLogic *logic = vtkCjyxMarkupsLogic::SafeDownCast(this->logic());
  if (!logic)
    {
    qCritical() << Q_FUNC_INFO << ": cannot get Markups logic.";
    return;
    }

  // Register displayable managers (same displayable manager handles both slice and 3D views)
  vtkDMMLSliceViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager("vtkDMMLMarkupsDisplayableManager");
  vtkDMMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager("vtkDMMLMarkupsDisplayableManager");

  // Register IO
  qCjyxIOManager* ioManager = qCjyxApplication::application()->ioManager();
  qCjyxMarkupsReader *markupsReader = new qCjyxMarkupsReader(logic, this);
  ioManager->registerIO(markupsReader);
  ioManager->registerIO(new qCjyxMarkupsWriter(this));

  // Add toolbar
  d->addToolBar();

  // Register Subject Hierarchy core plugins
  qCjyxSubjectHierarchyPluginHandler::instance()->registerPlugin(new qCjyxSubjectHierarchyMarkupsPlugin());
}

//-----------------------------------------------------------------------------
qCjyxAbstractModuleRepresentation* qCjyxMarkupsModule::createWidgetRepresentation()
{
  // Create and configure the additional widgets
  auto optionsWidgetFactory = qDMMLMarkupsOptionsWidgetsFactory::instance();
  optionsWidgetFactory->registerOptionsWidget(new qDMMLMarkupsAngleMeasurementsWidget());
  optionsWidgetFactory->registerOptionsWidget(new qDMMLMarkupsCurveSettingsWidget());
  optionsWidgetFactory->registerOptionsWidget(new qDMMLMarkupsPlaneWidget());
  optionsWidgetFactory->registerOptionsWidget(new qDMMLMarkupsROIWidget());

  // Create and configure module widget.
  auto moduleWidget = new qCjyxMarkupsModuleWidget();
  // Set the number of columns for the grid of "add markups buttons" to the number of markups
  // registered in this module.

  moduleWidget->setCreateMarkupsButtonsColumns(4);

  return moduleWidget;
}

//-----------------------------------------------------------------------------
vtkDMMLAbstractLogic* qCjyxMarkupsModule::createLogic()
{
  return vtkCjyxMarkupsLogic::New();
}

//-----------------------------------------------------------------------------
QStringList qCjyxMarkupsModule::associatedNodeTypes() const
{
  // This module can edit properties
  return QStringList() << "vtkDMMLMarkupsNode";
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModule::setDMMLScene(vtkDMMLScene* scene)
{
  Q_D(qCjyxMarkupsModule);
  this->Superclass::setDMMLScene(scene);
  vtkCjyxMarkupsLogic* logic = vtkCjyxMarkupsLogic::SafeDownCast(this->logic());
  if (!logic)
    {
    qCritical() << Q_FUNC_INFO << " failed: logic is invalid";
    return;
    }
  // Update default view nodes from settings
  this->readDefaultMarkupsDisplaySettings(logic->GetDefaultMarkupsDisplayNode());
  this->writeDefaultMarkupsDisplaySettings(logic->GetDefaultMarkupsDisplayNode());
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModule::readDefaultMarkupsDisplaySettings(vtkDMMLMarkupsDisplayNode* markupsDisplayNode)
{
  if (!markupsDisplayNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: markupsDisplayNode is invalid";
    return;
    }
  QSettings settings;

  if (settings.contains("Markups/SnapMode"))
    {
    markupsDisplayNode->SetSnapMode(vtkDMMLMarkupsDisplayNode::GetSnapModeFromString(
      settings.value("Markups/SnapMode").toString().toUtf8()));
    }
  if (settings.contains("Markups/FillVisibility"))
    {
    markupsDisplayNode->SetFillVisibility(settings.value("Markups/FillVisibility").toBool());
    }
  if (settings.contains("Markups/OutlineVisibility"))
    {
    markupsDisplayNode->SetOutlineVisibility(settings.value("Markups/OutlineVisibility").toBool());
    }
  if (settings.contains("Markups/FillOpacity"))
    {
    markupsDisplayNode->SetFillOpacity(settings.value("Markups/FillOpacity").toDouble());
    }
  if (settings.contains("Markups/OutlineOpacity"))
    {
    markupsDisplayNode->SetOutlineOpacity(settings.value("Markups/OutlineOpacity").toDouble());
    }
  if (settings.contains("Markups/TextScale"))
    {
    markupsDisplayNode->SetTextScale(settings.value("Markups/TextScale").toDouble());
    }
  if (settings.contains("Markups/GlyphType"))
    {
    int glyphType = vtkDMMLMarkupsDisplayNode::GetGlyphTypeFromString(settings.value("Markups/GlyphType").toString().toUtf8());
    // If application settings is old then it may contain invalid GlyphType. In this case use Sphere3D glyph instead.
    if (glyphType == vtkDMMLMarkupsDisplayNode::GlyphTypeInvalid)
      {
      glyphType = vtkDMMLMarkupsDisplayNode::Sphere3D;
      }
    markupsDisplayNode->SetGlyphType(glyphType);
    }
  if (settings.contains("Markups/GlyphScale"))
    {
    markupsDisplayNode->SetGlyphScale(settings.value("Markups/GlyphScale").toDouble());
    }
  if (settings.contains("Markups/GlyphSize"))
    {
    markupsDisplayNode->SetGlyphSize(settings.value("Markups/GlyphSize").toDouble());
    }
  if (settings.contains("Markups/UseGlyphScale"))
    {
    markupsDisplayNode->SetUseGlyphScale(settings.value("Markups/UseGlyphScale").toBool());
    }

  if (settings.contains("Markups/SliceProjection"))
    {
    markupsDisplayNode->SetSliceProjection(settings.value("Markups/SliceProjection").toBool());
    }
  if (settings.contains("Markups/SliceProjectionUseFiducialColor"))
    {
    markupsDisplayNode->SetSliceProjectionUseFiducialColor(settings.value("Markups/SliceProjectionUseFiducialColor").toBool());
    }
  if (settings.contains("Markups/SliceProjectionOutlinedBehindSlicePlane"))
    {
    markupsDisplayNode->SetSliceProjectionOutlinedBehindSlicePlane(settings.value("Markups/SliceProjectionOutlinedBehindSlicePlane").toBool());
    }
  if (settings.contains("Markups/SliceProjectionColor"))
    {
    QVariant variant = settings.value("Markups/SliceProjectionColor");
    QColor qcolor = variant.value<QColor>();
    markupsDisplayNode->SetSliceProjectionColor(qcolor.redF(), qcolor.greenF(), qcolor.blueF());
    }
  if (settings.contains("Markups/SliceProjectionOpacity"))
    {
    markupsDisplayNode->SetSliceProjectionOpacity(settings.value("Markups/SliceProjectionOpacity").toDouble());
    }


  if (settings.contains("Markups/CurveLineSizeMode"))
    {
    markupsDisplayNode->SetCurveLineSizeMode(vtkDMMLMarkupsDisplayNode::GetCurveLineSizeModeFromString(
      settings.value("Markups/CurveLineSizeMode").toString().toUtf8()));
    }
  if (settings.contains("Markups/LineThickness"))
    {
    markupsDisplayNode->SetLineThickness(settings.value("Markups/LineThickness").toDouble());
    }
  if (settings.contains("Markups/LineDiameter"))
    {
    markupsDisplayNode->SetLineDiameter(settings.value("Markups/LineDiameter").toDouble());
    }

  if (settings.contains("Markups/LineColorFadingStart"))
    {
    markupsDisplayNode->SetLineColorFadingStart(settings.value("Markups/LineColorFadingStart").toDouble());
    }
  if (settings.contains("Markups/LineColorFadingEnd"))
    {
    markupsDisplayNode->SetLineColorFadingEnd(settings.value("Markups/LineColorFadingEnd").toDouble());
    }
  if (settings.contains("Markups/LineColorFadingSaturation"))
    {
    markupsDisplayNode->SetLineColorFadingSaturation(settings.value("Markups/LineColorFadingSaturation").toDouble());
    }
  if (settings.contains("Markups/LineColorFadingHueOffset"))
    {
    markupsDisplayNode->SetLineColorFadingHueOffset(settings.value("Markups/LineColorFadingHueOffset").toDouble());
    }

  if (settings.contains("Markups/OccludedVisibility"))
    {
    markupsDisplayNode->SetOccludedVisibility(settings.value("Markups/OccludedVisibility").toBool());
    }
  if (settings.contains("Markups/OccludedOpacity"))
    {
    markupsDisplayNode->SetOccludedOpacity(settings.value("Markups/OccludedOpacity").toDouble());
    }

  if (settings.contains("Markups/TextProperty"))
    {
    vtkDMMLMarkupsDisplayNode::UpdateTextPropertyFromString(
      settings.value("Markups/TextProperty").toString().toStdString(),
      markupsDisplayNode->GetTextProperty());
    }

  if (settings.contains("Markups/SelectedColor"))
    {
    QVariant variant = settings.value("Markups/SelectedColor");
    QColor qcolor = variant.value<QColor>();
    markupsDisplayNode->SetSelectedColor(qcolor.redF(), qcolor.greenF(), qcolor.blueF());
    }
  if (settings.contains("Markups/UnselectedColor"))
    {
    QVariant variant = settings.value("Markups/UnselectedColor");
    QColor qcolor = variant.value<QColor>();
    markupsDisplayNode->SetColor(qcolor.redF(), qcolor.greenF(), qcolor.blueF());
    }
  if (settings.contains("Markups/ActiveColor"))
    {
    QVariant variant = settings.value("Markups/ActiveColor");
    QColor qcolor = variant.value<QColor>();
    markupsDisplayNode->SetColor(qcolor.redF(), qcolor.greenF(), qcolor.blueF());
    }
  if (settings.contains("Markups/Opacity"))
    {
    markupsDisplayNode->SetOpacity(settings.value("Markups/Opacity").toDouble());
    }
  if (settings.contains("Markups/InteractionHandleScale"))
    {
    markupsDisplayNode->SetInteractionHandleScale(settings.value("Markups/InteractionHandleScale").toDouble());
    }

}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModule::writeDefaultMarkupsDisplaySettings(vtkDMMLMarkupsDisplayNode* markupsDisplayNode)
{
  if (!markupsDisplayNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: markupsDisplayNode is invalid";
    return;
    }
  QSettings settings;

  settings.setValue("Markups/SnapMode", vtkDMMLMarkupsDisplayNode::GetSnapModeAsString(
    markupsDisplayNode->GetSnapMode()));
  settings.setValue("Markups/FillVisibility", markupsDisplayNode->GetFillVisibility());
  settings.setValue("Markups/OutlineVisibility", markupsDisplayNode->GetOutlineVisibility());
  settings.setValue("Markups/FillOpacity", markupsDisplayNode->GetFillOpacity());
  settings.setValue("Markups/OutlineOpacity", markupsDisplayNode->GetOutlineOpacity());
  settings.setValue("Markups/TextScale", markupsDisplayNode->GetTextScale());

  settings.setValue("Markups/GlyphType", vtkDMMLMarkupsDisplayNode::GetGlyphTypeAsString(markupsDisplayNode->GetGlyphType()));
  settings.setValue("Markups/GlyphScale", markupsDisplayNode->GetGlyphScale());
  settings.setValue("Markups/GlyphSize", markupsDisplayNode->GetGlyphSize());
  settings.setValue("Markups/UseGlyphScale", markupsDisplayNode->GetUseGlyphScale());

  settings.setValue("Markups/SliceProjection", markupsDisplayNode->GetSliceProjection());
  settings.setValue("Markups/SliceProjectionUseFiducialColor", markupsDisplayNode->GetSliceProjectionUseFiducialColor());
  settings.setValue("Markups/SliceProjectionOutlinedBehindSlicePlane", markupsDisplayNode->GetSliceProjectionOutlinedBehindSlicePlane());
  double* color = markupsDisplayNode->GetSliceProjectionColor();
  settings.setValue("Markups/SliceProjectionColor", QColor::fromRgbF(color[0], color[1], color[2]));
  settings.setValue("Markups/SliceProjectionOpacity", markupsDisplayNode->GetSliceProjectionOpacity());

  settings.setValue("Markups/CurveLineSizeMode", vtkDMMLMarkupsDisplayNode::GetCurveLineSizeModeAsString(markupsDisplayNode->GetCurveLineSizeMode()));
  settings.setValue("Markups/LineThickness", markupsDisplayNode->GetLineThickness());
  settings.setValue("Markups/LineDiameter", markupsDisplayNode->GetLineDiameter());

  settings.setValue("Markups/LineColorFadingStart", markupsDisplayNode->GetLineColorFadingStart());
  settings.setValue("Markups/LineColorFadingEnd", markupsDisplayNode->GetLineColorFadingEnd());
  settings.setValue("Markups/LineColorFadingSaturation", markupsDisplayNode->GetLineColorFadingSaturation());
  settings.setValue("Markups/LineLineColorFadingHueOffset", markupsDisplayNode->GetLineColorFadingHueOffset());

  settings.setValue("Markups/OccludedVisibility", markupsDisplayNode->GetOccludedVisibility());
  settings.setValue("Markups/OccludedOpacity", markupsDisplayNode->GetOccludedOpacity());

  settings.setValue("Markups/TextProperty", QString::fromStdString(
    vtkDMMLDisplayNode::GetTextPropertyAsString(markupsDisplayNode->GetTextProperty())));

  color = markupsDisplayNode->GetSelectedColor();
  settings.setValue("Markups/SelectedColor", QColor::fromRgbF(color[0], color[1], color[2]));
  color = markupsDisplayNode->GetColor();
  settings.setValue("Markups/UnselectedColor", QColor::fromRgbF(color[0], color[1], color[2]));
  color = markupsDisplayNode->GetActiveColor();
  settings.setValue("Markups/ActiveColor", QColor::fromRgbF(color[0], color[1], color[2]));
  settings.setValue("Markups/Opacity", markupsDisplayNode->GetOpacity());

  settings.setValue("Markups/InteractionHandleScale", markupsDisplayNode->GetInteractionHandleScale());
}

//-----------------------------------------------------------------------------
qDMMLMarkupsToolBar* qCjyxMarkupsModule::toolBar()
{
  Q_D(qCjyxMarkupsModule);
  return d->ToolBar;
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModule::setToolBarVisible(bool visible)
{
  Q_D(qCjyxMarkupsModule);
  d->ToolBar->setVisible(visible);
}

//-----------------------------------------------------------------------------
bool qCjyxMarkupsModule::isToolBarVisible()
{
  Q_D(qCjyxMarkupsModule);
  return d->ToolBar->isVisible();
}

//-----------------------------------------------------------------------------
bool qCjyxMarkupsModule::autoShowToolBar()
{
  Q_D(qCjyxMarkupsModule);
  return d->AutoShowToolBar;
}

//-----------------------------------------------------------------------------
void qCjyxMarkupsModule::setAutoShowToolBar(bool autoShow)
{
  Q_D(qCjyxMarkupsModule);
  d->AutoShowToolBar = autoShow;
}
//-----------------------------------------------------------------------------
bool qCjyxMarkupsModule::showMarkups(vtkDMMLMarkupsNode* markupsNode)
{
  qCjyxCoreApplication* app = qCjyxCoreApplication::application();
  if (!app
      || !app->moduleManager()
      || !dynamic_cast<qCjyxMarkupsModule*>(app->moduleManager()->module("Markups")))
    {
    qCritical("Markups module is not available");
    return false;
    }
  qCjyxMarkupsModule* markupsModule = dynamic_cast<qCjyxMarkupsModule*>(app->moduleManager()->module("Markups"));
  if (markupsModule->autoShowToolBar())
    {
    markupsModule->setToolBarVisible(true);
    }
  return true;
}

/*
// --------------------------------------------------------------------------
void qCjyxMarkupsModule::onNodeAddedEvent(vtkObject*, vtkObject* node)
{
  Q_D(qCjyxMarkupsModule);

  vtkDMMLMarkupsNode* markupsNode = vtkDMMLMarkupsNode::SafeDownCast(node);
  if (!markupsNode)
    {
    return;
    }

  qCjyxMarkupsModule::showMarkups(markupsNode);
}
*/
