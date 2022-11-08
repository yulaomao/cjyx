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

// SubjectHierarchy Plugins includes
#include "qCjyxSubjectHierarchyPluginHandler.h"
#include "qCjyxSubjectHierarchyVolumesPlugin.h"
#include "qCjyxSubjectHierarchyDefaultPlugin.h"

// Cjyx includes
#include "qDMMLSliceWidget.h"
#include "qCjyxApplication.h"
#include "qCjyxLayoutManager.h"
#include "vtkCjyxApplicationLogic.h"
#include "qCjyxModuleManager.h"
#include "vtkCjyxVolumesLogic.h"
#include "vtkCjyxColorLogic.h"
#include "qCjyxAbstractCoreModule.h"

// DMML includes
#include <vtkDMMLLabelMapVolumeNode.h>
#include <vtkDMMLLabelMapVolumeDisplayNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLScalarVolumeDisplayNode.h>
#include <vtkDMMLScalarVolumeNode.h>
#include <vtkDMMLColorLegendDisplayNode.h>
#include <vtkDMMLSelectionNode.h>
#include <vtkDMMLSliceCompositeNode.h>
#include <vtkDMMLSliceLogic.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkCollection.h>
#include <vtkImageData.h>

// Qt includes
#include <QAction>
#include <QDebug>
#include <QSettings>
#include <QStandardItem>
#include <QTimer>
#include <QMenu>

// CTK includes
#include "ctkSignalMapper.h"

namespace
{
  const std::string PRESET_AUTO = "_AUTO";
  const std::string DISPLAY_NODE_PRESET_PREFIX = "_DISPLAY_NODE_";
  const int MAX_NUMBER_OF_DISPLAY_NODE_PRESETS = 4;
}

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_SubjectHierarchy_Plugins
class qCjyxSubjectHierarchyVolumesPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qCjyxSubjectHierarchyVolumesPlugin);
protected:
  qCjyxSubjectHierarchyVolumesPlugin* const q_ptr;
public:
  qCjyxSubjectHierarchyVolumesPluginPrivate(qCjyxSubjectHierarchyVolumesPlugin& object);
  ~qCjyxSubjectHierarchyVolumesPluginPrivate() override;
  void init();

  bool resetFieldOfViewOnShow();
  bool resetViewOrientationOnShow();

  qDMMLSliceWidget* sliceWidgetForSliceCompositeNode(vtkDMMLSliceCompositeNode* compositeNode);

public:
  QIcon VolumeIcon;
  QIcon VolumeVisibilityOffIcon;
  QIcon VolumeVisibilityOnIcon;

  QAction* ShowVolumesInBranchAction{nullptr};
  QAction* ShowVolumeInForegroundAction{nullptr};
  QAction* ResetFieldOfViewOnShowAction{nullptr};
  QAction* ResetViewOrientationOnShowAction{nullptr};

  QAction* VolumeDisplayPresetAction{nullptr};
  QMenu* PresetSubmenu{nullptr};
  ctkSignalMapper* PresetModeMapper{nullptr};

  QAction* ShowColorLegendAction{nullptr};

  vtkWeakPointer<vtkDMMLVolumeNode> SelectedVolumeNode;
};

//-----------------------------------------------------------------------------
// qCjyxSubjectHierarchyVolumesPluginPrivate methods

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyVolumesPluginPrivate::qCjyxSubjectHierarchyVolumesPluginPrivate(qCjyxSubjectHierarchyVolumesPlugin& object)
: q_ptr(&object)
{
  this->VolumeIcon = QIcon(":Icons/Volume.png");
  this->VolumeVisibilityOffIcon = QIcon(":Icons/VolumeVisibilityOff.png");
  this->VolumeVisibilityOnIcon = QIcon(":Icons/VolumeVisibilityOn.png");
}

//------------------------------------------------------------------------------
void qCjyxSubjectHierarchyVolumesPluginPrivate::init()
{
  Q_Q(qCjyxSubjectHierarchyVolumesPlugin);

  // Connect layout changes to slot so that volume visibility icons are correctly updated
  if (qCjyxApplication::application()->layoutManager())
    {
    QObject::connect(qCjyxApplication::application()->layoutManager(), SIGNAL(layoutChanged(int)), q, SLOT(onLayoutChanged(int)));
    // Make sure initial connections are made even before a layout change
    QTimer::singleShot(0, q, SLOT(onLayoutChanged()));
    }

  this->ShowVolumesInBranchAction = new QAction("Show volumes in folder",q);
  QObject::connect(this->ShowVolumesInBranchAction, SIGNAL(triggered()), q, SLOT(showVolumesInBranch()));

  this->ShowVolumeInForegroundAction = new QAction("Show in slice views as foreground", q);
  QObject::connect(this->ShowVolumeInForegroundAction, SIGNAL(triggered()), q, SLOT(showVolumeInForeground()));

  this->ResetFieldOfViewOnShowAction = new QAction("Reset field of view on show",q);
  QObject::connect(this->ResetFieldOfViewOnShowAction, SIGNAL(toggled(bool)), q, SLOT(toggleResetFieldOfViewOnShowAction(bool)));
  this->ResetFieldOfViewOnShowAction->setCheckable(true);
  this->ResetFieldOfViewOnShowAction->setChecked(false);

  this->ResetViewOrientationOnShowAction = new QAction("Reset view orientation on show",q);
  QObject::connect(this->ResetViewOrientationOnShowAction, SIGNAL(toggled(bool)), q, SLOT(toggleResetViewOrientationOnShowAction(bool)));
  this->ResetViewOrientationOnShowAction->setCheckable(true);
  this->ResetViewOrientationOnShowAction->setChecked(false);

  // Add color legend action

  this->ShowColorLegendAction = new QAction(tr("Show color legend"), q);
  this->ShowColorLegendAction->setObjectName("ShowColorLegendAction");
  q->setActionPosition(this->ShowColorLegendAction, qCjyxSubjectHierarchyAbstractPlugin::SectionBottom);
  QObject::connect(this->ShowColorLegendAction, SIGNAL(toggled(bool)), q, SLOT(toggleVisibilityForCurrentItem(bool)));
  this->ShowColorLegendAction->setCheckable(true);
  this->ShowColorLegendAction->setChecked(false);

  // Add volume preset actions

  this->VolumeDisplayPresetAction = new QAction("Window/level presets");
  this->VolumeDisplayPresetAction->setObjectName("VolumeDisplayPresetAction");
  q->setActionPosition(this->VolumeDisplayPresetAction, qCjyxSubjectHierarchyAbstractPlugin::SectionBottom);

  // read volume preset names from volumes logic
  vtkCjyxVolumesLogic* volumesModuleLogic = (qCjyxCoreApplication::application() ? vtkCjyxVolumesLogic::SafeDownCast(
    qCjyxCoreApplication::application()->moduleLogic("Volumes")) : nullptr);
  if (!volumesModuleLogic)
    {
    qWarning() << Q_FUNC_INFO << " failed: Module logic 'Volumes' not found.";
    return;
    }

  this->PresetSubmenu = new QMenu();
  this->PresetSubmenu->setToolTipsVisible(true);
  QActionGroup* presetModeActions = new QActionGroup(this);
  presetModeActions->setExclusive(true);
  this->PresetModeMapper = new ctkSignalMapper(this);

  for (int displayNodePresetIndex = 0; displayNodePresetIndex < MAX_NUMBER_OF_DISPLAY_NODE_PRESETS; displayNodePresetIndex++)
    {
    QString presetIdStr = QString("%1%2").arg(QString::fromStdString(DISPLAY_NODE_PRESET_PREFIX)).arg(displayNodePresetIndex);
    QAction* presetAction = new QAction();
    presetAction->setObjectName(presetIdStr);
    presetAction->setToolTip(tr("Default preset for the selected volume"));
    presetAction->setCheckable(true);
    this->PresetSubmenu->addAction(presetAction);
    presetModeActions->addAction(presetAction);
    this->PresetModeMapper->setMapping(presetAction, presetIdStr);
    }

  // Add Automatic preset
  QAction* autoAction = new QAction(tr("Automatic"));
  autoAction->setObjectName(QString::fromStdString(PRESET_AUTO));
  autoAction->setToolTip(tr("Display the full intensity range of the volume."));
  autoAction->setCheckable(true);
  this->PresetSubmenu->addAction(autoAction);
  presetModeActions->addAction(autoAction);
  this->PresetModeMapper->setMapping(autoAction, QString::fromStdString(PRESET_AUTO));

  std::vector<std::string> presetIds = volumesModuleLogic->GetVolumeDisplayPresetIDs();
  for (const auto& presetId : presetIds)
    {
    vtkCjyxVolumesLogic::VolumeDisplayPreset preset = volumesModuleLogic->GetVolumeDisplayPreset(presetId);
    QString presetName = tr(preset.name.c_str());
    QString presetIdStr = QString::fromStdString(presetId);
    QAction* presetAction = new QAction(presetName);
    presetAction->setObjectName(presetIdStr);
    presetAction->setToolTip(tr(preset.description.c_str()));
    if (!preset.icon.empty())
      {
      presetAction->setIcon(QIcon(QString::fromStdString(preset.icon)));
      }
    presetAction->setCheckable(true);
    this->PresetSubmenu->addAction(presetAction);
    presetModeActions->addAction(presetAction);
    this->PresetModeMapper->setMapping(presetAction, presetIdStr);
    }

  this->VolumeDisplayPresetAction->setMenu(this->PresetSubmenu);
  QObject::connect(presetModeActions, SIGNAL(triggered(QAction*)), this->PresetModeMapper, SLOT(map(QAction*)));
  QObject::connect(this->PresetModeMapper, SIGNAL(mapped(QString)), q, SLOT(setVolumePreset(QString)));
}

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyVolumesPluginPrivate::~qCjyxSubjectHierarchyVolumesPluginPrivate() = default;

//-----------------------------------------------------------------------------
bool qCjyxSubjectHierarchyVolumesPluginPrivate::resetFieldOfViewOnShow()
{
  QSettings settings;
  return settings.value("SubjectHierarchy/ResetFieldOfViewOnShowVolume", true).toBool();
}

//-----------------------------------------------------------------------------
bool qCjyxSubjectHierarchyVolumesPluginPrivate::resetViewOrientationOnShow()
{
  QSettings settings;
  return settings.value("SubjectHierarchy/ResetViewOrientationOnShowVolume", true).toBool();
}

//------------------------------------------------------------------------------
qDMMLSliceWidget* qCjyxSubjectHierarchyVolumesPluginPrivate::sliceWidgetForSliceCompositeNode(vtkDMMLSliceCompositeNode* compositeNode)
{
  qDMMLLayoutManager* layoutManager = qCjyxApplication::application()->layoutManager();
  if (!layoutManager)
    {
    return nullptr;
    }

  QStringList sliceViewNames = layoutManager->sliceViewNames();
  foreach (QString sliceName, sliceViewNames)
    {
    qDMMLSliceWidget* sliceWidget = layoutManager->sliceWidget(sliceName);
    if (sliceWidget->dmmlSliceCompositeNode() == compositeNode)
      {
      return sliceWidget;
      }
    }

  return nullptr;
}

//-----------------------------------------------------------------------------
// qCjyxSubjectHierarchyVolumesPlugin methods

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyVolumesPlugin::qCjyxSubjectHierarchyVolumesPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qCjyxSubjectHierarchyVolumesPluginPrivate(*this) )
{
  this->m_Name = QString("Volumes");

  Q_D(qCjyxSubjectHierarchyVolumesPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyVolumesPlugin::~qCjyxSubjectHierarchyVolumesPlugin() = default;

//---------------------------------------------------------------------------
QList<QAction*> qCjyxSubjectHierarchyVolumesPlugin::viewContextMenuActions()const
{
  Q_D(const qCjyxSubjectHierarchyVolumesPlugin);
  QList<QAction*> actions;
  actions << d->VolumeDisplayPresetAction << d->ShowColorLegendAction;
  return actions;
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchyVolumesPlugin::showViewContextMenuActionsForItem(vtkIdType itemID, QVariantMap eventData)
{
  Q_D(qCjyxSubjectHierarchyVolumesPlugin);

  d->VolumeDisplayPresetAction->setVisible(false);

  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode || !shNode->GetScene())
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  if (itemID != shNode->GetSceneItemID())
    {
    return;
    }
  if (!eventData.contains("ViewNodeID"))
    {
    return;
    }
  vtkDMMLSliceNode* sliceNode = vtkDMMLSliceNode::SafeDownCast(
    shNode->GetScene()->GetNodeByID(eventData["ViewNodeID"].toString().toStdString()));
  if (!sliceNode)
    {
    return;
    }
  vtkDMMLSliceLogic* sliceLogic = qCjyxApplication::application()->applicationLogic()->GetSliceLogic(sliceNode);
  if (!sliceLogic)
    {
    return;
    }

  QVariantList worldPosVector = eventData["WorldPosition"].toList();
  if (worldPosVector.size() != 3)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid world position";
    return;
    }
  double worldPos[3] = { worldPosVector[0].toDouble(), worldPosVector[1].toDouble(), worldPosVector[2].toDouble() };
  int volumeLayer = sliceLogic->GetEditableLayerAtWorldPosition(worldPos);

  // Cache nodes to have them available for the menu action execution.
  d->SelectedVolumeNode = sliceLogic->GetLayerVolumeNode(volumeLayer);

  bool hasPrimaryDisplayNode = false;
  bool colorLegendIsVisible = false;
  if (d->SelectedVolumeNode)
    {
    // Check the checkbox of the current display preset
    vtkCjyxVolumesLogic* volumesModuleLogic = vtkCjyxVolumesLogic::SafeDownCast(
      qCjyxApplication::application()->moduleLogic("Volumes"));
    if (volumesModuleLogic)
      {
      // For presets in display node
      vtkDMMLScalarVolumeDisplayNode* scalarVolumeDisplayNode = vtkDMMLScalarVolumeDisplayNode::SafeDownCast(d->SelectedVolumeNode->GetVolumeDisplayNode());
      double currentWindowWidth = scalarVolumeDisplayNode ? scalarVolumeDisplayNode->GetWindow() : 0;
      double currentWindowLevel = scalarVolumeDisplayNode ? scalarVolumeDisplayNode->GetLevel() : 0;
      int numberOfDisplayNodePresets = scalarVolumeDisplayNode->GetNumberOfWindowLevelPresets();
      // For presets in volumes module logic
      QString appliedPresetId = QString::fromStdString(volumesModuleLogic->GetAppliedVolumeDisplayPresetId(d->SelectedVolumeNode->GetVolumeDisplayNode()));
      for (QAction* presetAction : d->PresetSubmenu->actions())
        {
        QString presetId = presetAction->objectName();
        if (presetId.startsWith(QString::fromStdString(DISPLAY_NODE_PRESET_PREFIX)))
          {
          // Preset stored in display node
          int displayNodePresetIndex = presetId.right(presetId.length() - DISPLAY_NODE_PRESET_PREFIX.length()).toInt();
          if (scalarVolumeDisplayNode && displayNodePresetIndex < numberOfDisplayNodePresets)
            {
            // existing display node preset
            double presetWindowWidth = scalarVolumeDisplayNode->GetWindowPreset(displayNodePresetIndex);
            double presetWindowLevel = scalarVolumeDisplayNode->GetLevelPreset(displayNodePresetIndex);
            presetAction->setText(tr("Default (WW=%1, WL=%2)").arg(presetWindowWidth).arg(presetWindowLevel));
            presetAction->setChecked(!scalarVolumeDisplayNode->GetAutoWindowLevel()
                && currentWindowWidth == presetWindowWidth && currentWindowLevel == presetWindowLevel);
            presetAction->setVisible(true);
            }
          else
            {
            // don't display this action, no corresponding display node preset
            presetAction->setVisible(false);
            }
          }
        else if (presetId == QString::fromStdString(PRESET_AUTO))
          {
          presetAction->setChecked(scalarVolumeDisplayNode->GetAutoWindowLevel());
          }
        else
          {
          // Preset storedin volume logic
          presetAction->setChecked(presetAction->objectName() == appliedPresetId);
          }
        }
      }

    // Parameters for color legend checkbox
    vtkCjyxColorLogic* colorsModuleLogic = vtkCjyxColorLogic::SafeDownCast(
      qCjyxApplication::application()->moduleLogic("Colors"));
    if (colorsModuleLogic)
      {
        vtkDMMLVolumeDisplayNode* volumeDisplayNode = vtkDMMLVolumeDisplayNode::SafeDownCast(d->SelectedVolumeNode->GetVolumeDisplayNode());
        if (volumeDisplayNode)
          {
          hasPrimaryDisplayNode = true;
          }
        vtkDMMLColorLegendDisplayNode* colorLegendDisplayNode = vtkCjyxColorLogic::GetColorLegendDisplayNode(d->SelectedVolumeNode);
        if (colorLegendDisplayNode)
          {
          colorLegendIsVisible = colorLegendDisplayNode->GetVisibility(sliceNode->GetID());
          }
      }
    }
  d->ShowColorLegendAction->setVisible(hasPrimaryDisplayNode);
  QSignalBlocker Block(d->ShowColorLegendAction);
  d->ShowColorLegendAction->setChecked(colorLegendIsVisible);

  d->VolumeDisplayPresetAction->setVisible(d->SelectedVolumeNode.GetPointer() != nullptr);
}

//----------------------------------------------------------------------------
double qCjyxSubjectHierarchyVolumesPlugin::canAddNodeToSubjectHierarchy(
  vtkDMMLNode* node, vtkIdType parentItemID/*=vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID*/)const
{
  Q_UNUSED(parentItemID);
  if (!node)
    {
    qCritical() << Q_FUNC_INFO << ": Input node is nullptr";
    return 0.0;
    }
  else if (node->IsA("vtkDMMLScalarVolumeNode"))
    {
    // Node is a volume
    return 0.5;
    }
  return 0.0;
}

//---------------------------------------------------------------------------
double qCjyxSubjectHierarchyVolumesPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID)const
{
  if (itemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return 0.0;
    }
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return 0.0;
    }

  // Volume
  vtkDMMLNode* associatedNode = shNode->GetItemDataNode(itemID);
  if (associatedNode && associatedNode->IsA("vtkDMMLScalarVolumeNode"))
    {
    return 0.5; // There are other plugins that can handle special volume nodes better, thus the relatively low value
    }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qCjyxSubjectHierarchyVolumesPlugin::roleForPlugin()const
{
  return "Scalar volume";
}

//-----------------------------------------------------------------------------
QString qCjyxSubjectHierarchyVolumesPlugin::tooltip(vtkIdType itemID)const
{
  if (itemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QString("Invalid");
    }
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return QString("Error");
    }

  // Get basic tooltip from abstract plugin
  QString tooltipString = Superclass::tooltip(itemID);

  vtkDMMLScalarVolumeNode* volumeNode = vtkDMMLScalarVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  vtkImageData* imageData = (volumeNode ? volumeNode->GetImageData() : nullptr);
  if (volumeNode && imageData)
    {
    int dimensions[3] = {0,0,0};
    imageData->GetDimensions(dimensions);
    double spacing[3] = {0.0,0.0,0.0};
    volumeNode->GetSpacing(spacing);
    tooltipString.append( QString(" (Dimensions: %1x%2x%3  Spacing: %4mm x %5mm x %6mm)")
      .arg(dimensions[0]).arg(dimensions[1]).arg(dimensions[2])
      .arg(spacing[0],0,'g',3).arg(spacing[1],0,'g',3).arg(spacing[2],0,'g',3) );
    }
  else
    {
    tooltipString.append(" !Invalid volume");
    }

  return tooltipString;
}

//---------------------------------------------------------------------------
QIcon qCjyxSubjectHierarchyVolumesPlugin::icon(vtkIdType itemID)
{
  Q_D(qCjyxSubjectHierarchyVolumesPlugin);

  if (itemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QIcon();
    }

  // Volume
  if (this->canOwnSubjectHierarchyItem(itemID))
    {
    return d->VolumeIcon;
    }

  // Item unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qCjyxSubjectHierarchyVolumesPlugin::visibilityIcon(int visible)
{
  Q_D(qCjyxSubjectHierarchyVolumesPlugin);

  if (visible == 1)
    {
    return d->VolumeVisibilityOnIcon;
    }
  else
    {
    return d->VolumeVisibilityOffIcon;
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchyVolumesPlugin::setDisplayVisibility(vtkIdType itemID, int visible)
{
  if (itemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return;
    }
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Volume
  vtkDMMLScalarVolumeNode* associatedVolumeNode = vtkDMMLScalarVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (associatedVolumeNode)
    {
    if (visible)
      {
      // If visibility is on, then show the volume in the background of all slice views
      this->showVolumeInAllViews(associatedVolumeNode, vtkDMMLApplicationLogic::BackgroundLayer);
      }
    else
      {
      // If visibility is off, then hide the volume from all layers of all slice views
      this->hideVolumeFromAllViews(associatedVolumeNode);
      }
    }
  // Default
  else
    {
    qCjyxSubjectHierarchyPluginHandler::instance()->defaultPlugin()->setDisplayVisibility(itemID, visible);
    }
}

//-----------------------------------------------------------------------------
int qCjyxSubjectHierarchyVolumesPlugin::getDisplayVisibility(vtkIdType itemID)const
{
  if (itemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return -1;
    }
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return -1;
    }

  // Sanity checks for volume
  vtkDMMLVolumeNode* volumeNode = vtkDMMLVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!volumeNode)
    {
    return -1;
    }

  // Return with 1 if volume is shown using volume rendering
  int numberOfDisplayNodes = volumeNode->GetNumberOfDisplayNodes();
  for (int displayNodeIndex = 0; displayNodeIndex < numberOfDisplayNodes; displayNodeIndex++)
    {
    vtkDMMLDisplayNode* displayNode = volumeNode->GetNthDisplayNode(displayNodeIndex);
    if (!displayNode || !displayNode->IsShowModeDefault())
      {
      continue;
      }
    if (vtkDMMLScalarVolumeDisplayNode::SafeDownCast(displayNode))
      {
      // scalar volume display node does not control visibility, visibility in those
      // views will be collected from slice views below
      continue;
      }
    if (displayNode->GetVisibility())
      {
      return 1;
      }
    }

  // Collect all volumes that are shown in any slice views in any layers
  QSet<vtkIdType> shownVolumeItemIDs;
  this->collectShownVolumes(shownVolumeItemIDs);

  if (shownVolumeItemIDs.contains(itemID))
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchyVolumesPlugin::showVolumeInAllViews(
  vtkDMMLScalarVolumeNode* node, int layer/*=vtkDMMLApplicationLogic::BackgroundLayer*/ )
{
  Q_D(qCjyxSubjectHierarchyVolumesPlugin);

  if (!node)
    {
    qCritical() << Q_FUNC_INFO << ": nullptr node";
    return;
    }
  vtkDMMLScene* scene = qCjyxSubjectHierarchyPluginHandler::instance()->dmmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid DMML scene";
    return;
    }
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Collect currently shown volumes from the given layers so that their visibility icons can be updated after
  // showing the specified volume
  QSet<vtkIdType> subjectHierarchyItemsToUpdate;
  this->collectShownVolumes(subjectHierarchyItemsToUpdate, layer);
  vtkIdType shItemId = shNode->GetItemByDataNode(node);
  subjectHierarchyItemsToUpdate.insert(shItemId);

  bool resetOrientation = d->resetViewOrientationOnShow();
  bool resetFov = d->resetFieldOfViewOnShow();

  vtkDMMLSliceCompositeNode* compositeNode = nullptr;
  int numberOfCompositeNodes = scene->GetNumberOfNodesByClass("vtkDMMLSliceCompositeNode");
  for (int i=0; i<numberOfCompositeNodes; i++)
    {
    compositeNode = vtkDMMLSliceCompositeNode::SafeDownCast(scene->GetNthNodeByClass(i, "vtkDMMLSliceCompositeNode"));
    if (layer & vtkDMMLApplicationLogic::BackgroundLayer)
      {
      compositeNode->SetBackgroundVolumeID(node->GetID());
      }
    else if (layer & vtkDMMLApplicationLogic::ForegroundLayer)
      {
      compositeNode->SetForegroundVolumeID(node->GetID());
      }
    else if (layer & vtkDMMLApplicationLogic::LabelLayer)
      {
      compositeNode->SetLabelVolumeID(node->GetID());
      }
    if (resetOrientation || resetFov)
      {
      qDMMLSliceWidget* sliceWidget = d->sliceWidgetForSliceCompositeNode(compositeNode);
      vtkDMMLSliceLogic* sliceLogic = sliceWidget ? sliceWidget->sliceLogic() : nullptr;
      vtkDMMLSliceNode* sliceNode = sliceWidget ? sliceWidget->dmmlSliceNode() : nullptr;
      if (sliceLogic && sliceNode)
        {
        if (resetOrientation)
          {
          // Set to default orientation before rotation so that the view is snapped
          // closest to the default orientation of this slice view.
          sliceNode->SetOrientationToDefault();
          // If the volume is shown in only one view and the volume is a single slice then
          // make sure the view is aligned with that, otherwise just snap to closest view.
          bool forceSlicePlaneToSingleSlice = (numberOfCompositeNodes == 1);
          sliceWidget->sliceLogic()->RotateSliceToLowestVolumeAxes(forceSlicePlaneToSingleSlice);
          }
        if (resetFov)
          {
          sliceLogic->FitSliceToAll();
          }
        }
      }
    }

  // Volume rendering display
  qCjyxSubjectHierarchyAbstractPlugin* volumeRenderingPlugin = qCjyxSubjectHierarchyPluginHandler::instance()->pluginByName("VolumeRendering");
  vtkNew<vtkIdList> allShItemIds;
  allShItemIds->InsertNextId(shItemId);
  if (volumeRenderingPlugin)
    {
    std::vector<vtkDMMLNode*> allViewNodes;
    scene->GetNodesByClass("vtkDMMLViewNode", allViewNodes);
    int numberOfDisplayNodes = node->GetNumberOfDisplayNodes();
    for (int displayNodeIndex = 0; displayNodeIndex < numberOfDisplayNodes; displayNodeIndex++)
      {
      vtkDMMLDisplayNode* displayNode = node->GetNthDisplayNode(displayNodeIndex);
      // By default we do not show volume rendering, only if it is explicitly enabled.
      if (!displayNode || !displayNode->IsShowModeDefault())
        {
        continue;
        }
      if (!displayNode->IsA("vtkDMMLVolumeRenderingDisplayNode")) // ignore everything except VolumeRendering
        {
        // we only manage existing volume rendering display nodes here
        // (we don't want to show volume rendering until volume rendering has been explicitly enabled)
        continue;
        }
      for (vtkDMMLNode* node : allViewNodes)
        {
        vtkDMMLViewNode* viewNode = vtkDMMLViewNode::SafeDownCast(node);
        if (!viewNode)
          {
          continue;
          }
        if (!displayNode->IsDisplayableInView(viewNode->GetID()))
          {
          continue;
          }
        volumeRenderingPlugin->showItemInView(shItemId, viewNode, allShItemIds);
        }
      }
    }

  // Show color legend display node
  int numberOfDisplayNodes = node->GetNumberOfDisplayNodes();
  for (int displayNodeIndex = 0; displayNodeIndex < numberOfDisplayNodes; displayNodeIndex++)
    {
    vtkDMMLDisplayNode* displayNode = node->GetNthDisplayNode(displayNodeIndex);
    // ignore everything except vtkDMMLColorLegendDisplayNOde
    // we only manage existing color legend display nodes here
    // (we don't want to show color legend until color legend has been explicitly enabled)
    if (displayNode && displayNode->IsShowModeDefault()
      && displayNode->IsA("vtkDMMLColorLegendDisplayNode"))
      {
      displayNode->VisibilityOn();
      }
    }

  // Update scene model for subject hierarchy nodes that were just shown
  for ( QSet<vtkIdType>::iterator volumeItemIt = subjectHierarchyItemsToUpdate.begin();
       volumeItemIt != subjectHierarchyItemsToUpdate.end(); ++volumeItemIt )
   {
   shNode->ItemModified(*volumeItemIt);
   }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchyVolumesPlugin::hideVolumeFromAllViews(vtkDMMLScalarVolumeNode* node)
{
  if (!node || !node->GetID())
    {
    qCritical() << Q_FUNC_INFO << ": nullptr node";
    return;
    }
  vtkDMMLScene* scene = qCjyxSubjectHierarchyPluginHandler::instance()->dmmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid DMML scene";
    return;
    }

  char* volumeNodeID = node->GetID();
  vtkDMMLSliceCompositeNode* compositeNode = nullptr;
  int numberOfCompositeNodes = scene->GetNumberOfNodesByClass("vtkDMMLSliceCompositeNode");
  for (int i=0; i<numberOfCompositeNodes; i++)
    {
    compositeNode = vtkDMMLSliceCompositeNode::SafeDownCast(scene->GetNthNodeByClass(i, "vtkDMMLSliceCompositeNode"));
    const char* backgroundVolumeID = compositeNode->GetBackgroundVolumeID();
    const char* foregroundVolumeID = compositeNode->GetForegroundVolumeID();
    const char* labelVolumeID = compositeNode->GetLabelVolumeID();
    if (backgroundVolumeID && !strcmp(backgroundVolumeID, volumeNodeID))
      {
      compositeNode->SetBackgroundVolumeID(nullptr);
      }
    if (foregroundVolumeID && !strcmp(foregroundVolumeID, volumeNodeID))
      {
      compositeNode->SetForegroundVolumeID(nullptr);
      }
    if (labelVolumeID && !strcmp(labelVolumeID, volumeNodeID))
      {
      compositeNode->SetLabelVolumeID(nullptr);
      }
    }

  // Color legend display, volume rendering display node
  int numberOfDisplayNodes = node->GetNumberOfDisplayNodes();
  for (int displayNodeIndex = 0; displayNodeIndex < numberOfDisplayNodes; displayNodeIndex++)
    {
    vtkDMMLDisplayNode* displayNode = node->GetNthDisplayNode(displayNodeIndex);
    if (!displayNode || !displayNode->IsShowModeDefault())
      {
      continue;
      }
    if (vtkDMMLScalarVolumeDisplayNode::SafeDownCast(displayNode))
      {
      // visibility in slice views is managed separately
      continue;
      }
    displayNode->SetVisibility(false);
    }

  // Get subject hierarchy item for the volume node and have the scene model updated
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkIdType volumeItemID = shNode->GetItemByDataNode(node);
  if (volumeItemID)
    {
    shNode->ItemModified(volumeItemID);
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchyVolumesPlugin::collectShownVolumes( QSet<vtkIdType>& shownVolumeItemIDs,
  int layer/*=vtkDMMLApplicationLogic::BackgroundLayer | vtkDMMLApplicationLogic::ForegroundLayer | vtkDMMLApplicationLogic::LabelLayer*/ )const
{
  shownVolumeItemIDs.clear();

  vtkDMMLScene* scene = qCjyxSubjectHierarchyPluginHandler::instance()->dmmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid DMML scene";
    return;
    }
  if (scene->IsBatchProcessing())
    {
    return;
    }
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  vtkDMMLSliceCompositeNode* compositeNode = nullptr;
  const int numberOfCompositeNodes = scene->GetNumberOfNodesByClass("vtkDMMLSliceCompositeNode");
  for (int i=0; i<numberOfCompositeNodes; i++)
    {
    compositeNode = vtkDMMLSliceCompositeNode::SafeDownCast( scene->GetNthNodeByClass( i, "vtkDMMLSliceCompositeNode" ) );
    if ( layer & vtkDMMLApplicationLogic::BackgroundLayer
      && compositeNode->GetBackgroundVolumeID() && strcmp(compositeNode->GetBackgroundVolumeID(),"") )
      {
      shownVolumeItemIDs.insert(shNode->GetItemByDataNode( scene->GetNodeByID(compositeNode->GetBackgroundVolumeID())) );
      }
    if ( layer & vtkDMMLApplicationLogic::ForegroundLayer
      && compositeNode->GetForegroundVolumeID() && strcmp(compositeNode->GetForegroundVolumeID(),"") )
      {
      shownVolumeItemIDs.insert(shNode->GetItemByDataNode( scene->GetNodeByID(compositeNode->GetForegroundVolumeID())) );
      }
    if ( layer & vtkDMMLApplicationLogic::LabelLayer
      && compositeNode->GetLabelVolumeID() && strcmp(compositeNode->GetLabelVolumeID(),"") )
      {
      shownVolumeItemIDs.insert(shNode->GetItemByDataNode( scene->GetNodeByID(compositeNode->GetLabelVolumeID())) );
      }
    }
}

//---------------------------------------------------------------------------
QList<QAction*> qCjyxSubjectHierarchyVolumesPlugin::visibilityContextMenuActions()const
{
  Q_D(const qCjyxSubjectHierarchyVolumesPlugin);

  QList<QAction*> actions;
  actions << d->ShowVolumesInBranchAction << d->ShowVolumeInForegroundAction
    << d->ResetFieldOfViewOnShowAction << d->ResetViewOrientationOnShowAction << d->ShowColorLegendAction;
  return actions;
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchyVolumesPlugin::showVisibilityContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qCjyxSubjectHierarchyVolumesPlugin);

  if (itemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return;
    }
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Volume
  if (this->canOwnSubjectHierarchyItem(itemID))
    {
    d->ShowVolumeInForegroundAction->setVisible(true);

    d->ResetFieldOfViewOnShowAction->setChecked(d->resetFieldOfViewOnShow());
    d->ResetFieldOfViewOnShowAction->setVisible(true);

    d->ResetViewOrientationOnShowAction->setChecked(d->resetViewOrientationOnShow());
    d->ResetViewOrientationOnShowAction->setVisible(true);
    }

  // Folders (Patient, Study, Folder)
  if ( shNode->IsItemLevel(itemID, vtkDMMLSubjectHierarchyConstants::GetDICOMLevelPatient())
    || shNode->IsItemLevel(itemID, vtkDMMLSubjectHierarchyConstants::GetDICOMLevelStudy())
    || shNode->IsItemLevel(itemID, vtkDMMLSubjectHierarchyConstants::GetSubjectHierarchyLevelFolder()) )
    {
    d->ShowVolumesInBranchAction->setVisible(true);
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchyVolumesPlugin::showVolumesInBranch()
{
  vtkDMMLSelectionNode* selectionNode = qCjyxCoreApplication::application()->applicationLogic()->GetSelectionNode();
  if (!selectionNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to get selection node";
    return;
    }
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkDMMLScene* scene = qCjyxSubjectHierarchyPluginHandler::instance()->dmmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid DMML scene";
    return;
    }
  vtkIdType currentItemID = qCjyxSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
    }

  // First, hide all volumes from all views, so that only the volumes from the selected branch are shown
  QSet<vtkIdType> subjectHierarchyItemsToUpdate;
  this->collectShownVolumes(subjectHierarchyItemsToUpdate, vtkDMMLApplicationLogic::BackgroundLayer);
  for ( QSet<vtkIdType>::iterator volumeItemIt = subjectHierarchyItemsToUpdate.begin();
       volumeItemIt != subjectHierarchyItemsToUpdate.end(); ++volumeItemIt )
   {
   this->hideVolumeFromAllViews(vtkDMMLScalarVolumeNode::SafeDownCast(shNode->GetItemDataNode(*volumeItemIt)));
   }

  // Deselect all volumes before showing the ones from the branch
  selectionNode->SetActiveVolumeID(nullptr);
  selectionNode->SetSecondaryVolumeID(nullptr);

  // Show volumes in branch
  vtkSmartPointer<vtkCollection> childVolumeNodes = vtkSmartPointer<vtkCollection>::New();
  shNode->GetDataNodesInBranch(currentItemID, childVolumeNodes, "vtkDMMLScalarVolumeNode");
  childVolumeNodes->InitTraversal();
  for (int i=0; i<childVolumeNodes->GetNumberOfItems(); ++i)
    {
    vtkDMMLScalarVolumeNode* volumeNode = vtkDMMLScalarVolumeNode::SafeDownCast(childVolumeNodes->GetItemAsObject(i));
    if (volumeNode)
      {
      // Get subject hierarchy item for the volume node
      vtkIdType volumeShItemID = shNode->GetItemByDataNode(volumeNode);
      if (volumeShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
        {
        qCritical() << Q_FUNC_INFO << ": Unable to get subject hierarchy item";
        continue;
        }

      // Show first two volume in branch only
      if (!selectionNode->GetActiveVolumeID())
        {
        selectionNode->SetActiveVolumeID(volumeNode->GetID());
        this->showVolumeInAllViews(volumeNode, vtkDMMLApplicationLogic::BackgroundLayer);
        }
      else if (!selectionNode->GetSecondaryVolumeID())
        {
        selectionNode->SetSecondaryVolumeID(volumeNode->GetID());
        this->showVolumeInAllViews(volumeNode, vtkDMMLApplicationLogic::ForegroundLayer);

        // Make sure the secondary volume is shown in a semi-transparent way
        vtkDMMLSliceCompositeNode* compositeNode = nullptr;
        int numberOfCompositeNodes = scene->GetNumberOfNodesByClass("vtkDMMLSliceCompositeNode");
        for (int i=0; i<numberOfCompositeNodes; i++)
          {
          compositeNode = vtkDMMLSliceCompositeNode::SafeDownCast ( scene->GetNthNodeByClass( i, "vtkDMMLSliceCompositeNode" ) );
          if (compositeNode && compositeNode->GetForegroundOpacity() == 0.0)
            {
            compositeNode->SetForegroundOpacity(0.5);
            }
          }
        }
      }
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchyVolumesPlugin::showVolumeInForeground()
{
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
  }
  vtkDMMLScene* scene = qCjyxSubjectHierarchyPluginHandler::instance()->dmmlScene();
  if (!scene)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid DMML scene";
    return;
  }
  vtkIdType currentItemID = qCjyxSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
  }

  vtkDMMLScalarVolumeNode* volumeNode = vtkDMMLScalarVolumeNode::SafeDownCast(shNode->GetItemDataNode(currentItemID));
  if (!volumeNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to get current item as a volume node";
    return;
  }

  // Show volume in foreground
  this->showVolumeInAllViews(volumeNode, vtkDMMLApplicationLogic::ForegroundLayer);

  // Make sure the secondary volume is shown in a semi-transparent way
  vtkDMMLSliceCompositeNode* compositeNode = nullptr;
  int numberOfCompositeNodes = scene->GetNumberOfNodesByClass("vtkDMMLSliceCompositeNode");
  for (int i=0; i<numberOfCompositeNodes; i++)
    {
    compositeNode = vtkDMMLSliceCompositeNode::SafeDownCast ( scene->GetNthNodeByClass( i, "vtkDMMLSliceCompositeNode" ) );
    if (compositeNode && compositeNode->GetForegroundOpacity() == 0.0)
      {
      compositeNode->SetForegroundOpacity(0.5);
      }
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchyVolumesPlugin::onLayoutChanged(int layout)
{
  Q_UNUSED(layout);
  this->onLayoutChanged();
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchyVolumesPlugin::onLayoutChanged()
{
  vtkDMMLScene* scene = qCjyxSubjectHierarchyPluginHandler::instance()->dmmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid DMML scene";
    return;
    }

  // Connect Modified event of each composite node to the plugin, so that visibility icons are
  // updated when volumes are shown/hidden from outside subject hierarchy
  vtkDMMLSliceCompositeNode* compositeNode = nullptr;
  int numberOfCompositeNodes = scene->GetNumberOfNodesByClass("vtkDMMLSliceCompositeNode");
  for (int i=0; i<numberOfCompositeNodes; i++)
    {
    compositeNode = vtkDMMLSliceCompositeNode::SafeDownCast(scene->GetNthNodeByClass(i, "vtkDMMLSliceCompositeNode"));
    qvtkReconnect(compositeNode, vtkCommand::ModifiedEvent, this, SLOT(onSliceCompositeNodeModified()));
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchyVolumesPlugin::onSliceCompositeNodeModified()
{
  vtkDMMLScene* scene = qCjyxSubjectHierarchyPluginHandler::instance()->dmmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid DMML scene";
    return;
    }
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    if (!scene->IsClosing() && !scene->IsRestoring())
      {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      }
    return;
    }

  vtkDMMLScalarVolumeNode* volumeNode = nullptr;
  int numberOfVolumeNodes = scene->GetNumberOfNodesByClass("vtkDMMLScalarVolumeNode");
  for (int i=0; i<numberOfVolumeNodes; i++)
    {
    volumeNode = vtkDMMLScalarVolumeNode::SafeDownCast(scene->GetNthNodeByClass(i, "vtkDMMLScalarVolumeNode"));
    vtkIdType volumeItemID = shNode->GetItemByDataNode(volumeNode);
    if (volumeItemID)
      {
      shNode->ItemModified(volumeItemID);
      }
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchyVolumesPlugin::toggleResetFieldOfViewOnShowAction(bool on)
{
  QSettings settings;
  settings.setValue("SubjectHierarchy/ResetFieldOfViewOnShowVolume", on);
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchyVolumesPlugin::toggleResetViewOrientationOnShowAction(bool on)
{
  QSettings settings;
  settings.setValue("SubjectHierarchy/ResetViewOrientationOnShowVolume", on);
}

//-----------------------------------------------------------------------------
bool qCjyxSubjectHierarchyVolumesPlugin::showItemInView(vtkIdType itemID, vtkDMMLAbstractViewNode* viewNode, vtkIdList* allItemsToShow)
{
  Q_D(qCjyxSubjectHierarchyVolumesPlugin);
  vtkDMMLViewNode* threeDViewNode = vtkDMMLViewNode::SafeDownCast(viewNode);
  if (threeDViewNode)
    {
    qCjyxSubjectHierarchyAbstractPlugin* volumeRenderingPlugin = qCjyxSubjectHierarchyPluginHandler::instance()->pluginByName("VolumeRendering");
    if (!volumeRenderingPlugin)
      {
      qCritical() << Q_FUNC_INFO << ": Failed to access Volume rendering subject hierarchy plugin";
      return false;
      }
    return volumeRenderingPlugin->showItemInView(itemID, viewNode, allItemsToShow);
    }

  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return false;
    }

  vtkDMMLVolumeNode* volumeToShow = vtkDMMLVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!volumeToShow)
    {
    // This method can only handle volume nodes
    return false;
    }

  vtkDMMLSliceNode* sliceNode = vtkDMMLSliceNode::SafeDownCast(viewNode);
  if (!sliceNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: can only show items if a valid slice view is specified";
    return false;
    }

  // Get foreground, background, and label nodes
  vtkDMMLVolumeNode* backgroundNode = nullptr;
  vtkDMMLVolumeNode* foregroundNode = nullptr;
  vtkDMMLLabelMapVolumeNode* labelNode = nullptr;
  for (vtkIdType itemIndex = 0; itemIndex < allItemsToShow->GetNumberOfIds(); itemIndex++)
    {
    vtkDMMLVolumeNode* volumeNode = vtkDMMLVolumeNode::SafeDownCast(shNode->GetItemDataNode(allItemsToShow->GetId(itemIndex)));
    if (!volumeNode)
      {
      continue;
      }
    if (vtkDMMLLabelMapVolumeNode::SafeDownCast(volumeNode))
      {
      if (labelNode == nullptr)
        {
        labelNode = vtkDMMLLabelMapVolumeNode::SafeDownCast(volumeNode);
        }
      }
    else
      {
      if (!backgroundNode)
        {
        backgroundNode = volumeNode;
        }
      else if (!foregroundNode)
        {
        foregroundNode = volumeNode;
        }
      }
      if (backgroundNode && foregroundNode && labelNode)
        {
        // all volume layers are filled - we can ignore the rest of the selected volumes
        break;
        }
    }

  // Show the volume in slice view
  vtkDMMLSliceLogic* sliceLogic = nullptr;
  vtkCjyxApplicationLogic* appLogic = qCjyxApplication::application()->applicationLogic();
  if (appLogic)
    {
    sliceLogic = appLogic->GetSliceLogic(sliceNode);
    }
  if (!sliceLogic)
    {
    qCritical() << Q_FUNC_INFO << " failed: cannot get slice logic";
    return false;
    }
  vtkDMMLSliceCompositeNode* sliceCompositeNode = sliceLogic->GetSliceCompositeNode();
  if (!sliceCompositeNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: cannot get slice composite node";
    return false;
    }
  if (backgroundNode || foregroundNode)
    {
    sliceLogic->StartSliceCompositeNodeInteraction(
      vtkDMMLSliceCompositeNode::BackgroundVolumeFlag
      | vtkDMMLSliceCompositeNode::ForegroundVolumeFlag
      | vtkDMMLSliceCompositeNode::ForegroundOpacityFlag);
    if (volumeToShow == backgroundNode)
      {
      sliceCompositeNode->SetBackgroundVolumeID(backgroundNode->GetID());
      if (!foregroundNode)
        {
        sliceCompositeNode->SetForegroundVolumeID(nullptr);
        }
      sliceLogic->EndSliceCompositeNodeInteraction();
      }
    if (volumeToShow == foregroundNode)
      {
      sliceLogic->StartSliceCompositeNodeInteraction(vtkDMMLSliceCompositeNode::BackgroundVolumeFlag | vtkDMMLSliceCompositeNode::ForegroundVolumeFlag);
      if (!backgroundNode)
        {
        sliceCompositeNode->SetBackgroundVolumeID(nullptr);
        }
      sliceCompositeNode->SetForegroundVolumeID(foregroundNode->GetID());
      sliceCompositeNode->SetForegroundOpacity(0.5);
      sliceLogic->EndSliceCompositeNodeInteraction();
      }
    }
  if (volumeToShow == labelNode)
    {
    sliceLogic->StartSliceCompositeNodeInteraction(vtkDMMLSliceCompositeNode::LabelVolumeFlag);
    sliceCompositeNode->SetLabelVolumeID(labelNode ? labelNode->GetID() : nullptr);
    sliceLogic->EndSliceCompositeNodeInteraction();
    }

  // Align view orientation and field of view to background volume
  if (d->resetViewOrientationOnShow() || d->resetFieldOfViewOnShow())
    {
    int interactionFlags = 0;
    if (d->resetViewOrientationOnShow())
      {
      interactionFlags |= vtkDMMLSliceNode::ResetOrientationFlag
        | vtkDMMLSliceNode::RotateToBackgroundVolumePlaneFlag;
      }
    if (d->resetFieldOfViewOnShow())
      {
      interactionFlags |= vtkDMMLSliceNode::ResetFieldOfViewFlag;
      }
    sliceLogic->StartSliceNodeInteraction(interactionFlags);
    if (d->resetViewOrientationOnShow())
      {
      // reset orientation before snapping slice to closest volume axis
      sliceNode->SetOrientationToDefault();
      sliceLogic->RotateSliceToLowestVolumeAxes();
      }
    if (d->resetFieldOfViewOnShow())
      {
      sliceLogic->FitSliceToAll();
      }
    sliceLogic->EndSliceNodeInteraction();
    }

  shNode->ItemModified(itemID);

  return true;
}

// --------------------------------------------------------------------------
void qCjyxSubjectHierarchyVolumesPlugin::setVolumePreset(const QString& presetId)
{
  Q_D(qCjyxSubjectHierarchyVolumesPlugin);

  if (d->SelectedVolumeNode == nullptr)
    {
    qCritical() << Q_FUNC_INFO << " failed: invalid volume";
    return;
    }

  if (presetId.startsWith(QString::fromStdString(DISPLAY_NODE_PRESET_PREFIX)))
    {
    // Preset stored in display node
    int displayNodePresetIndex = presetId.right(presetId.length() - DISPLAY_NODE_PRESET_PREFIX.length()).toInt();
    vtkDMMLScalarVolumeDisplayNode* scalarVolumeDisplayNode = vtkDMMLScalarVolumeDisplayNode::SafeDownCast(d->SelectedVolumeNode->GetVolumeDisplayNode());
    if (!scalarVolumeDisplayNode)
      {
      qCritical() << Q_FUNC_INFO << " failed: volume display node is invalid";
      return;
      }
    scalarVolumeDisplayNode->SetWindowLevelFromPreset(displayNodePresetIndex);
    }
  else if (presetId == QString::fromStdString(PRESET_AUTO))
    {
    vtkDMMLScalarVolumeDisplayNode* scalarVolumeDisplayNode = vtkDMMLScalarVolumeDisplayNode::SafeDownCast(d->SelectedVolumeNode->GetVolumeDisplayNode());
    if (!scalarVolumeDisplayNode)
      {
      qCritical() << Q_FUNC_INFO << " failed: volume display node is invalid";
      return;
      }
    scalarVolumeDisplayNode->SetAutoWindowLevel(true);
    }
  else
    {
    // Preset stored in volumes logic
    vtkCjyxVolumesLogic* volumesModuleLogic = vtkCjyxVolumesLogic::SafeDownCast(qCjyxApplication::application()->moduleLogic("Volumes"));
    if (!volumesModuleLogic)
      {
      qCritical() << Q_FUNC_INFO << " failed: volumes module logic is not available";
      return;
      }
    volumesModuleLogic->ApplyVolumeDisplayPreset(d->SelectedVolumeNode->GetVolumeDisplayNode(), presetId.toStdString());
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchyVolumesPlugin::toggleVisibilityForCurrentItem(bool on)
{
  Q_D(qCjyxSubjectHierarchyVolumesPlugin);
  if (d->SelectedVolumeNode)
    {
    // Show color legend display node
    vtkDMMLDisplayNode* displayNode = d->SelectedVolumeNode->GetVolumeDisplayNode();
    if (!displayNode || !displayNode->GetColorNode())
      {
      // No color node for this node, color legend is not applicable
      return;
      }

    vtkDMMLColorLegendDisplayNode* colorLegendDisplayNode = nullptr;
    if (on)
      {
      colorLegendDisplayNode = vtkCjyxColorLogic::AddDefaultColorLegendDisplayNode(displayNode);
      }
    else
      {
      colorLegendDisplayNode = vtkCjyxColorLogic::GetColorLegendDisplayNode(displayNode);
      }

    if (colorLegendDisplayNode)
      {
      colorLegendDisplayNode->SetVisibility(on);
      // If visibility is set to false then prevent making the node visible again on show.
      colorLegendDisplayNode->SetShowMode(on ? vtkDMMLDisplayNode::ShowDefault : vtkDMMLDisplayNode::ShowIgnore);
      }
    }
}
