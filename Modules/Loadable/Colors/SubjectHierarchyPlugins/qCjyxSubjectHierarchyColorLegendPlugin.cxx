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

==============================================================================*/

// SubjectHierarchy Plugins includes
#include "qCjyxSubjectHierarchyPluginHandler.h"
#include "qCjyxSubjectHierarchyColorLegendPlugin.h"

// Cjyx includes
#include "qCjyxAbstractModuleWidget.h"
#include "qCjyxApplication.h"
#include "qCjyxLayoutManager.h"

// Colors includes
#include "vtkCjyxColorLogic.h"

// DMML includes
#include <vtkDMMLCameraNode.h>
#include <vtkDMMLColorLegendDisplayNode.h>
#include <vtkDMMLScalarVolumeNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLViewLogic.h>
#include <vtkDMMLViewNode.h>
#include <vtkDMMLVolumeDisplayNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// Qt includes
#include <QDebug>
#include <QAction>
#include <QMenu>
#include <QSettings>

// DMML widgets includes
#include "qDMMLNodeComboBox.h"
#include "qDMMLThreeDView.h"
#include "qDMMLThreeDWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_SubjectHierarchy_Widgets
class qCjyxSubjectHierarchyColorLegendPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qCjyxSubjectHierarchyColorLegendPlugin);
protected:
  qCjyxSubjectHierarchyColorLegendPlugin* const q_ptr;
public:
  qCjyxSubjectHierarchyColorLegendPluginPrivate(qCjyxSubjectHierarchyColorLegendPlugin& object);
  ~qCjyxSubjectHierarchyColorLegendPluginPrivate() override;
  void init();
public:

  QAction* ShowColorLegendAction;
};

//-----------------------------------------------------------------------------
// qCjyxSubjectHierarchyColorLegendPluginPrivate methods

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyColorLegendPluginPrivate::qCjyxSubjectHierarchyColorLegendPluginPrivate(qCjyxSubjectHierarchyColorLegendPlugin& object)
  : q_ptr(&object)
  , ShowColorLegendAction(nullptr)
{
}

//------------------------------------------------------------------------------
void qCjyxSubjectHierarchyColorLegendPluginPrivate::init()
{
  Q_Q(qCjyxSubjectHierarchyColorLegendPlugin);

  this->ShowColorLegendAction = new QAction("Show color legend",q);
  qCjyxSubjectHierarchyAbstractPlugin::setActionPosition(this->ShowColorLegendAction,
    qCjyxSubjectHierarchyAbstractPlugin::SectionDefault, 10);
  QObject::connect(this->ShowColorLegendAction, SIGNAL(toggled(bool)), q, SLOT(toggleVisibilityForCurrentItem(bool)));
  this->ShowColorLegendAction->setCheckable(true);
  this->ShowColorLegendAction->setChecked(false);
}

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyColorLegendPluginPrivate::~qCjyxSubjectHierarchyColorLegendPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qCjyxSubjectHierarchyColorLegendPlugin methods

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyColorLegendPlugin::qCjyxSubjectHierarchyColorLegendPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qCjyxSubjectHierarchyColorLegendPluginPrivate(*this) )
{
  this->m_Name = QString("ColorLegend");

  Q_D(qCjyxSubjectHierarchyColorLegendPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyColorLegendPlugin::~qCjyxSubjectHierarchyColorLegendPlugin() = default;

//---------------------------------------------------------------------------
QList<QAction*> qCjyxSubjectHierarchyColorLegendPlugin::visibilityContextMenuActions()const
{
  Q_D(const qCjyxSubjectHierarchyColorLegendPlugin);

  QList<QAction*> actions;
  actions << d->ShowColorLegendAction;
  return actions;
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchyColorLegendPlugin::showVisibilityContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qCjyxSubjectHierarchyColorLegendPlugin);

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
  vtkDMMLDisplayableNode* displayableNode = vtkDMMLDisplayableNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!displayableNode)
    {
    // Not a displayable node color legend is not applicable
    return;
    }
  vtkDMMLDisplayNode* displayNode = displayableNode->GetDisplayNode();
  if (!displayNode || !displayNode->GetColorNode())
    {
    // No color node for this node, color legend is not applicable
    return;
    }

  vtkDMMLColorLegendDisplayNode* colorLegendDisplayNode = vtkCjyxColorLogic::GetColorLegendDisplayNode(displayNode);
  QSignalBlocker blocker(d->ShowColorLegendAction);
  d->ShowColorLegendAction->setChecked(colorLegendDisplayNode && colorLegendDisplayNode->GetVisibility());
  d->ShowColorLegendAction->setVisible(true);
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchyColorLegendPlugin::toggleVisibilityForCurrentItem(bool on)
{
  Q_D(qCjyxSubjectHierarchyColorLegendPlugin);
  vtkIdType itemID = qCjyxSubjectHierarchyPluginHandler::instance()->currentItem();
  if (itemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    return;
    }
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkDMMLDisplayableNode* displayableNode = vtkDMMLDisplayableNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!displayableNode)
    {
    // Not a displayable node color legend is not applicable
    return;
    }
  vtkDMMLDisplayNode* displayNode = displayableNode->GetDisplayNode();
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

//-----------------------------------------------------------------------------
bool qCjyxSubjectHierarchyColorLegendPlugin::showItemInView(vtkIdType itemID, vtkDMMLAbstractViewNode* viewNode, vtkIdList* allItemsToShow)
{
  Q_D(qCjyxSubjectHierarchyColorLegendPlugin);

  vtkDMMLViewNode* threeDViewNode = vtkDMMLViewNode::SafeDownCast(viewNode);
  vtkDMMLSliceNode* sliceNode = vtkDMMLSliceNode::SafeDownCast(viewNode);

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

  bool wasVisible = false;
  vtkDMMLColorLegendDisplayNode* displayNode = vtkCjyxColorLogic::GetColorLegendDisplayNode(volumeNode);
  if (displayNode)
    {
    wasVisible = displayNode->GetVisibility();
    }
  else
    {
    // if there is no color legend node => create it, get first color legend node otherwise
    displayNode = vtkCjyxColorLogic::AddDefaultColorLegendDisplayNode(volumeNode);
    }
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to create color display node for scalar volume node " << volumeNode->GetName();
    return false;
    }

  if (viewNode)
    {
    // Show in specific view
    DMMLNodeModifyBlocker blocker(displayNode);
    // show
    if (!wasVisible)
      {
      displayNode->SetVisibility(true);
      }
    displayNode->AddViewNodeID(viewNode->GetID());
    }
  else if (sliceNode)
    {
    // Show in specific view
    DMMLNodeModifyBlocker blocker(displayNode);
    // show
    if (!wasVisible)
      {
      displayNode->SetVisibility(true);
      }
    displayNode->AddViewNodeID(sliceNode->GetID());
    }
  else
    {
    // Show in all views
    DMMLNodeModifyBlocker blocker(displayNode);
    displayNode->RemoveAllViewNodeIDs();
    displayNode->SetVisibility(true);
    }

  return true;
}

//---------------------------------------------------------------------------
bool qCjyxSubjectHierarchyColorLegendPlugin::showColorLegendInView( bool show, vtkIdType itemID, vtkDMMLViewNode* viewNode/*=nullptr*/)
{
  Q_D(qCjyxSubjectHierarchyColorLegendPlugin);

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

  bool wasVisible = false;
  vtkDMMLColorLegendDisplayNode* displayNode = vtkCjyxColorLogic::GetColorLegendDisplayNode(volumeNode);
  if (displayNode)
    {
    wasVisible = displayNode->GetVisibility();
    }
  else
    {
    // there is no color legend display node
    if (!show)
      {
      // not visible and should not be visible, so we are done
      return true;
      }
    // if there is no color legend node => create it, get first color legend node otherwise
    displayNode = vtkCjyxColorLogic::AddDefaultColorLegendDisplayNode(volumeNode);
    }
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to create color display node for scalar volume node " << volumeNode->GetName();
    return false;
    }

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

  return true;
}

//---------------------------------------------------------------------------
bool qCjyxSubjectHierarchyColorLegendPlugin::showColorLegendInSlice( bool show, vtkIdType itemID, vtkDMMLSliceNode* sliceNode/*=nullptr*/)
{
  Q_D(qCjyxSubjectHierarchyColorLegendPlugin);

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

  bool wasVisible = false;
  vtkDMMLColorLegendDisplayNode* displayNode = vtkCjyxColorLogic::GetColorLegendDisplayNode(volumeNode);
  if (displayNode)
    {
    wasVisible = displayNode->GetVisibility();
    }
  else
    {
    // there is no color legend display node
    if (!show)
      {
      // not visible and should not be visible, so we are done
      return true;
      }
    // if there is no color legend node => create it, get first color legend node otherwise
    displayNode = vtkCjyxColorLogic::AddDefaultColorLegendDisplayNode(volumeNode);
    }
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to create color display node for scalar volume node " << volumeNode->GetName();
    return false;
    }

  if (sliceNode)
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
      displayNode->AddViewNodeID(sliceNode->GetID());
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

  return true;
}
