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
#include "qCjyxSubjectHierarchyVolumeRenderingPlugin.h"
#include "qCjyxSubjectHierarchyVolumesPlugin.h"

// Cjyx includes
#include "qCjyxAbstractModuleWidget.h"
#include "qCjyxApplication.h"
#include "qCjyxLayoutManager.h"

// Volume Rendering includes
#include "vtkCjyxVolumeRenderingLogic.h"

// DMML includes
#include <vtkDMMLCameraNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLScalarVolumeNode.h>
#include <vtkDMMLViewLogic.h>
#include <vtkDMMLViewNode.h>
#include <vtkDMMLVolumeRenderingDisplayNode.h>
#include <vtkDMMLVolumePropertyNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// Qt includes
#include <QDebug>
#include <QAction>
#include <QSettings>

// DMML widgets includes
#include "qDMMLNodeComboBox.h"
#include "qDMMLThreeDView.h"
#include "qDMMLThreeDWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_SubjectHierarchy_Widgets
class qCjyxSubjectHierarchyVolumeRenderingPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qCjyxSubjectHierarchyVolumeRenderingPlugin);
protected:
  qCjyxSubjectHierarchyVolumeRenderingPlugin* const q_ptr;
public:
  qCjyxSubjectHierarchyVolumeRenderingPluginPrivate(qCjyxSubjectHierarchyVolumeRenderingPlugin& object);
  ~qCjyxSubjectHierarchyVolumeRenderingPluginPrivate() override;
  void init();
public:
  vtkWeakPointer<vtkCjyxVolumeRenderingLogic> VolumeRenderingLogic;

  QAction* ToggleVolumeRenderingAction;
  QAction* VolumeRenderingOptionsAction;
};

//-----------------------------------------------------------------------------
// qCjyxSubjectHierarchyVolumeRenderingPluginPrivate methods

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyVolumeRenderingPluginPrivate::qCjyxSubjectHierarchyVolumeRenderingPluginPrivate(qCjyxSubjectHierarchyVolumeRenderingPlugin& object)
: q_ptr(&object)
, ToggleVolumeRenderingAction(nullptr)
, VolumeRenderingOptionsAction(nullptr)
{
}

//------------------------------------------------------------------------------
void qCjyxSubjectHierarchyVolumeRenderingPluginPrivate::init()
{
  Q_Q(qCjyxSubjectHierarchyVolumeRenderingPlugin);

  this->ToggleVolumeRenderingAction = new QAction("Show in 3D views as volume rendering",q);
  QObject::connect(this->ToggleVolumeRenderingAction, SIGNAL(toggled(bool)), q, SLOT(toggleVolumeRenderingForCurrentItem(bool)));
  this->ToggleVolumeRenderingAction->setCheckable(true);
  this->ToggleVolumeRenderingAction->setChecked(false);

  this->VolumeRenderingOptionsAction = new QAction("Volume rendering options...",q);
  this->VolumeRenderingOptionsAction->setToolTip(tr("Switch to Volume Rendering module to manage display options"));
  QObject::connect(this->VolumeRenderingOptionsAction, SIGNAL(triggered()), q, SLOT(showVolumeRenderingOptionsForCurrentItem()));
}

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyVolumeRenderingPluginPrivate::~qCjyxSubjectHierarchyVolumeRenderingPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qCjyxSubjectHierarchyVolumeRenderingPlugin methods

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyVolumeRenderingPlugin::qCjyxSubjectHierarchyVolumeRenderingPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qCjyxSubjectHierarchyVolumeRenderingPluginPrivate(*this) )
{
  this->m_Name = QString("VolumeRendering");

  Q_D(qCjyxSubjectHierarchyVolumeRenderingPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyVolumeRenderingPlugin::~qCjyxSubjectHierarchyVolumeRenderingPlugin() = default;

//-----------------------------------------------------------------------------
void qCjyxSubjectHierarchyVolumeRenderingPlugin::setVolumeRenderingLogic(vtkCjyxVolumeRenderingLogic* volumeRenderingLogic)
{
  Q_D(qCjyxSubjectHierarchyVolumeRenderingPlugin);
  d->VolumeRenderingLogic = volumeRenderingLogic;
}

//---------------------------------------------------------------------------
QList<QAction*> qCjyxSubjectHierarchyVolumeRenderingPlugin::visibilityContextMenuActions()const
{
  Q_D(const qCjyxSubjectHierarchyVolumeRenderingPlugin);

  QList<QAction*> actions;
  actions << d->ToggleVolumeRenderingAction << d->VolumeRenderingOptionsAction;
  return actions;
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchyVolumeRenderingPlugin::showVisibilityContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qCjyxSubjectHierarchyVolumeRenderingPlugin);

  if (!itemID)
    {
    // There are no scene actions in this plugin
    return;
    }
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Volume
  if (qCjyxSubjectHierarchyPluginHandler::instance()->pluginByName("Volumes")->canOwnSubjectHierarchyItem(itemID))
    {
    vtkDMMLScalarVolumeNode* volumeNode = vtkDMMLScalarVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
    vtkDMMLVolumeRenderingDisplayNode* displayNode = nullptr;
    if (!volumeNode)
      {
      qCritical() << Q_FUNC_INFO << ": Failed to find scalar volume node associated to subject hierarchy item " << itemID;
      return;
      }
    if (d->VolumeRenderingLogic)
      {
      displayNode = d->VolumeRenderingLogic->GetFirstVolumeRenderingDisplayNode(volumeNode);
      }
    else
      {
      qWarning() << Q_FUNC_INFO << ": volume rendering logic is not set, cannot set up toggle volume rendering action";
      }

    d->ToggleVolumeRenderingAction->blockSignals(true);
    d->ToggleVolumeRenderingAction->setChecked(displayNode ? displayNode->GetVisibility() : false);
    d->ToggleVolumeRenderingAction->blockSignals(false);
    d->ToggleVolumeRenderingAction->setVisible(true);

    d->VolumeRenderingOptionsAction->setVisible(true);
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchyVolumeRenderingPlugin::toggleVolumeRenderingForCurrentItem(bool on)
{
  Q_D(qCjyxSubjectHierarchyVolumeRenderingPlugin);
  vtkIdType currentItemID = qCjyxSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
    }
  // Whenever the volume is shown/hidden, volume rendering should be shown as requested by this method.
  this->showVolumeRendering(on, currentItemID, nullptr);
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchyVolumeRenderingPlugin::resetFieldOfView(vtkDMMLDisplayNode* displayNode, vtkDMMLViewNode* viewNode/*=nullptr*/)
{
  Q_D(qCjyxSubjectHierarchyVolumeRenderingPlugin);
  vtkDMMLDisplayableNode* volumeNode = displayNode->GetDisplayableNode();
  double rasBounds[6] = { 0.0 };
  volumeNode->GetRASBounds(rasBounds);
  double cameraFocalPoint[3] =
    {
    (rasBounds[0] + rasBounds[1]) / 2.0,
    (rasBounds[2] + rasBounds[3]) / 2.0,
    (rasBounds[4] + rasBounds[5]) / 2.0,
    };

  // Get list of view nodes that will have their FOV reset
  QList<vtkDMMLViewNode*> viewNodes;
  if (viewNode)
    {
    // Specific view is provided - reset FOV in that single view
    viewNodes << viewNode;
    }
  else
    {
    // FOV reset in all views is requested - do it in all views where the volume is visible in
    qDMMLLayoutManager* layoutManager = qCjyxApplication::application()->layoutManager();
    if (!layoutManager)
      {
      qCritical() << Q_FUNC_INFO << " failed: invalid layout manager";
      return;
      }
    for (int i = 0; i < layoutManager->threeDViewCount(); i++)
      {
      qDMMLThreeDWidget* threeDWidget = layoutManager->threeDWidget(i);
      if (!threeDWidget)
        {
        continue;
        }
      vtkDMMLViewNode* currentViewNode = threeDWidget->dmmlViewNode();
      if (!currentViewNode)
        {
        continue;
        }
      if (!displayNode->IsDisplayableInView(currentViewNode->GetID()))
        {
        continue;
        }
      viewNodes << currentViewNode;
      }
    }

  vtkCjyxApplicationLogic* appLogic = qCjyxApplication::application()->applicationLogic();
  if (!appLogic)
    {
    qCritical() << Q_FUNC_INFO << " failed: cannot get application logic";
    return;
    }
  foreach(vtkDMMLViewNode* currentViewNode, viewNodes)
    {
    // Show the volume in slice view
    vtkDMMLViewLogic* viewLogic = appLogic->GetViewLogic(currentViewNode);
    if (!viewLogic)
      {
      qCritical() << Q_FUNC_INFO << " failed: cannot get slice logic";
      continue;
      }
    vtkDMMLCameraNode* cameraNode = viewLogic->GetCameraNode();
    if (!cameraNode)
      {
      continue;
      }
    cameraNode->SetFocalPoint(cameraFocalPoint);
    cameraNode->ResetClippingRange();
    }
}


//---------------------------------------------------------------------------
bool qCjyxSubjectHierarchyVolumeRenderingPlugin::showVolumeRendering(bool show, vtkIdType itemID, vtkDMMLViewNode* viewNode/*=nullptr*/)
{
  Q_D(qCjyxSubjectHierarchyVolumeRenderingPlugin);

  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return false;
    }
  vtkDMMLScalarVolumeNode* volumeNode = vtkDMMLScalarVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!volumeNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find scalar volume node associated to subject hierarchy item " << itemID;
    return false;
    }
  if (!d->VolumeRenderingLogic)
    {
    qWarning() << Q_FUNC_INFO << ": volume rendering logic is not set, cannot set up toggle volume rendering action";
    return false;
    }
  bool wasVisible = false;
  vtkDMMLVolumeRenderingDisplayNode* displayNode = d->VolumeRenderingLogic->GetFirstVolumeRenderingDisplayNode(volumeNode);
  if (displayNode)
    {
    wasVisible = displayNode->GetVisibility();
    }
  else
    {
    // there is no volume rendering display node
    if (!show)
      {
      // not visible and should not be visible, so we are done
      return true;
      }
    displayNode = d->VolumeRenderingLogic->CreateDefaultVolumeRenderingNodes(volumeNode);
    }
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to create volume rendering display node for scalar volume node " << volumeNode->GetName();
    return false;
    }

  // Prevent volume rendering from show up again in 3D view when eye icon is clicked,
  // after the volume rendering checkbox was unchecked.
  displayNode->SetShowMode(show ? vtkDMMLDisplayNode::ShowDefault : vtkDMMLDisplayNode::ShowIgnore);

  if (viewNode)
    {
    // Show/hide in specific view
    DMMLNodeModifyBlocker blocker(displayNode);
    if (show)
      {
      // show
      if (!wasVisible)
        {
        displayNode->SetVisibility(true);
        // This was hidden in all views, show it only in the currently selected view
        displayNode->RemoveAllViewNodeIDs();
        }
      displayNode->AddViewNodeID(viewNode->GetID());
      }
    else
      {
      // This hides the volume rendering in all views, which is a bit more than asked for,
      // but since drag-and-drop to view only requires selective showing (and not selective hiding),
      // this should be good enough. The behavior can be refined later if needed.
      displayNode->SetVisibility(false);
      }
    }
  else
    {
    // Show in all views
    DMMLNodeModifyBlocker blocker(displayNode);
    displayNode->RemoveAllViewNodeIDs();
    displayNode->SetVisibility(show);
    }

  if (show)
    {
    QSettings settings;
    bool resetFieldOfView = settings.value("SubjectHierarchy/ResetFieldOfViewOnShowVolume", true).toBool();
    if (resetFieldOfView)
      {
      this->resetFieldOfView(displayNode, viewNode);
      }
    }

  return true;
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchyVolumeRenderingPlugin::showVolumeRenderingOptionsForCurrentItem()
{
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  vtkIdType currentItemID = qCjyxSubjectHierarchyPluginHandler::instance()->currentItem();
  if (!currentItemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
    }

  qCjyxAbstractModuleWidget* moduleWidget = qCjyxSubjectHierarchyAbstractPlugin::switchToModule("VolumeRendering");
  if (moduleWidget)
    {
    // Get node selector combobox
    qDMMLNodeComboBox* nodeSelector = moduleWidget->findChild<qDMMLNodeComboBox*>("VolumeNodeComboBox");

    // Choose current data node
    if (nodeSelector)
      {
      nodeSelector->setCurrentNode(shNode->GetItemDataNode(currentItemID));
      }
    }
}

//-----------------------------------------------------------------------------
bool qCjyxSubjectHierarchyVolumeRenderingPlugin::showItemInView(vtkIdType itemID, vtkDMMLAbstractViewNode* viewNode, vtkIdList* allItemsToShow)
{
  vtkDMMLViewNode* threeDViewNode = vtkDMMLViewNode::SafeDownCast(viewNode);
  if (threeDViewNode)
    {
    return this->showVolumeRendering(true, itemID, threeDViewNode);
    }
  else
    {
    // Use volume's module implementation for displaying volume in slice views
    qCjyxSubjectHierarchyVolumesPlugin* volumesPlugin = qobject_cast<qCjyxSubjectHierarchyVolumesPlugin*>(
      qCjyxSubjectHierarchyPluginHandler::instance()->pluginByName("Volumes"));
    if (!volumesPlugin)
      {
      qCritical() << Q_FUNC_INFO << ": Failed to access Volumes subject hierarchy plugin";
      return false;
      }
    return volumesPlugin->showItemInView(itemID, viewNode, allItemsToShow);
    }
}
