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
#include "qCjyxSubjectHierarchyVisibilityPlugin.h"

// SubjectHierarchy logic includes
#include "vtkCjyxSubjectHierarchyModuleLogic.h"

// Cjyx includes
#include "qCjyxCoreApplication.h"
#include "vtkCjyxApplicationLogic.h"

// DMML includes
#include "vtkDMMLDisplayableNode.h"
#include "vtkDMMLDisplayNode.h"
#include "vtkDMMLScalarVolumeNode.h"

// VTK includes
#include <vtkObjectFactory.h>

// Qt includes
#include <QDebug>
#include <QStandardItem>
#include <QAction>

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_SubjectHierarchy_Widgets
class qCjyxSubjectHierarchyVisibilityPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qCjyxSubjectHierarchyVisibilityPlugin);
protected:
  qCjyxSubjectHierarchyVisibilityPlugin* const q_ptr;
public:
  qCjyxSubjectHierarchyVisibilityPluginPrivate(qCjyxSubjectHierarchyVisibilityPlugin& object);
  ~qCjyxSubjectHierarchyVisibilityPluginPrivate() override;
  void init();
public:
  QAction* ToggleVisibility2DAction;
  QAction* ToggleVisibility3DAction;
  QAction* ShowInAllViewsAction;
};

//-----------------------------------------------------------------------------
// qCjyxSubjectHierarchyVisibilityPluginPrivate methods

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyVisibilityPluginPrivate::qCjyxSubjectHierarchyVisibilityPluginPrivate(qCjyxSubjectHierarchyVisibilityPlugin& object)
: q_ptr(&object)
{
  this->ToggleVisibility2DAction = nullptr;
  this->ToggleVisibility3DAction = nullptr;
  this->ShowInAllViewsAction = nullptr;
}

//------------------------------------------------------------------------------
void qCjyxSubjectHierarchyVisibilityPluginPrivate::init()
{
  Q_Q(qCjyxSubjectHierarchyVisibilityPlugin);

  this->ShowInAllViewsAction = new QAction("Show in all views", q);
  QObject::connect(this->ShowInAllViewsAction, SIGNAL(triggered()), q, SLOT(showInAllViews()));

  this->ToggleVisibility2DAction = new QAction("2D visibility",q);
  QObject::connect(this->ToggleVisibility2DAction, SIGNAL(toggled(bool)), q, SLOT(toggleCurrentItemVisibility2D(bool)));
  this->ToggleVisibility2DAction->setCheckable(true);
  this->ToggleVisibility2DAction->setChecked(false);

  this->ToggleVisibility3DAction = new QAction("3D visibility",q);
  QObject::connect(this->ToggleVisibility3DAction, SIGNAL(toggled(bool)), q, SLOT(toggleCurrentItemVisibility3D(bool)));
  this->ToggleVisibility3DAction->setCheckable(true);
  this->ToggleVisibility3DAction->setChecked(false);
}

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyVisibilityPluginPrivate::~qCjyxSubjectHierarchyVisibilityPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qCjyxSubjectHierarchyVisibilityPlugin methods

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyVisibilityPlugin::qCjyxSubjectHierarchyVisibilityPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qCjyxSubjectHierarchyVisibilityPluginPrivate(*this) )
{
  this->m_Name = QString("Visibility");

  Q_D(qCjyxSubjectHierarchyVisibilityPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyVisibilityPlugin::~qCjyxSubjectHierarchyVisibilityPlugin() = default;

//---------------------------------------------------------------------------
QList<QAction*> qCjyxSubjectHierarchyVisibilityPlugin::visibilityContextMenuActions()const
{
  Q_D(const qCjyxSubjectHierarchyVisibilityPlugin);

  QList<QAction*> actions;
  actions << d->ShowInAllViewsAction << d->ToggleVisibility2DAction << d->ToggleVisibility3DAction;
  return actions;
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchyVisibilityPlugin::showVisibilityContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qCjyxSubjectHierarchyVisibilityPlugin);

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

  // Determine 2D and 3D visibility of branch. Visible only if visibility is on for all items in branch
  bool visible2D = true;
  bool visible2DVisible = false;
  bool visible3D = true;
  bool visible3DVisible = false;
  bool visibleInAllViews = true;
  vtkSmartPointer<vtkCollection> childDisplayableNodes = vtkSmartPointer<vtkCollection>::New();
  shNode->GetDataNodesInBranch(itemID, childDisplayableNodes, "vtkDMMLDisplayableNode");
  childDisplayableNodes->InitTraversal();
  for (int i=0; i<childDisplayableNodes->GetNumberOfItems(); ++i)
    {
    vtkDMMLDisplayableNode* displayableNode = vtkDMMLDisplayableNode::SafeDownCast(childDisplayableNodes->GetItemAsObject(i));
    vtkDMMLScalarVolumeNode* volumeNode = vtkDMMLScalarVolumeNode::SafeDownCast(childDisplayableNodes->GetItemAsObject(i));
    if (!displayableNode || volumeNode)
      {
      // Disable it for volume nodes
      // (instead, it has a different visibility icon and a show in foreground action in the Volumes plugin)
      continue;
      }
    vtkDMMLDisplayNode* displayNode = vtkDMMLDisplayNode::SafeDownCast(displayableNode->GetDisplayNode());
    if (!displayNode)
      {
      displayableNode->CreateDefaultDisplayNodes();
      displayNode = displayableNode->GetDisplayNode();
      }
    if (!displayNode)
      {
      qCritical() << Q_FUNC_INFO << ": Failed to find display node for displayable node " << displayableNode->GetName();
      continue;
      }
    visible2D = visible2D && (displayNode->GetVisibility2D() > 0);
    visible2DVisible = true;
    visible3D = visible3D && (displayNode->GetVisibility3D() > 0);
    visible3DVisible = true;
    visibleInAllViews = visibleInAllViews && (displayNode->GetViewNodeIDs().empty());
    }

  bool wasBlocked = d->ToggleVisibility2DAction->blockSignals(true);
  d->ToggleVisibility2DAction->setChecked(visible2D);
  d->ToggleVisibility2DAction->blockSignals(wasBlocked);
  d->ToggleVisibility2DAction->setVisible(visible2DVisible);

  wasBlocked = d->ToggleVisibility3DAction->blockSignals(true);
  d->ToggleVisibility3DAction->setChecked(visible3D);
  d->ToggleVisibility3DAction->blockSignals(wasBlocked);
  d->ToggleVisibility3DAction->setVisible(visible3DVisible);

  d->ShowInAllViewsAction->setVisible(!visibleInAllViews);
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchyVisibilityPlugin::toggleCurrentItemVisibility2D(bool on)
{
  // Get currently selected node and scene
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkIdType currentItemID = qCjyxSubjectHierarchyPluginHandler::instance()->currentItem();
  if (!currentItemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current subject hierarchy item";
    return;
    }

  vtkSmartPointer<vtkCollection> childDisplayableNodes = vtkSmartPointer<vtkCollection>::New();
  shNode->GetDataNodesInBranch(currentItemID, childDisplayableNodes, "vtkDMMLDisplayableNode");
  childDisplayableNodes->InitTraversal();
  for (int i=0; i<childDisplayableNodes->GetNumberOfItems(); ++i)
    {
    vtkDMMLDisplayableNode* displayableNode = vtkDMMLDisplayableNode::SafeDownCast(childDisplayableNodes->GetItemAsObject(i));
    vtkDMMLDisplayNode* displayNode = displayableNode ? vtkDMMLDisplayNode::SafeDownCast(displayableNode->GetDisplayNode()) : nullptr;
    if (displayNode)
      {
      displayNode->SetVisibility2D(on);
      }
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchyVisibilityPlugin::toggleCurrentItemVisibility3D(bool on)
{
  // Get currently selected node and scene
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkIdType currentItemID = qCjyxSubjectHierarchyPluginHandler::instance()->currentItem();
  if (!currentItemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current subject hierarchy item";
    return;
    }

  vtkSmartPointer<vtkCollection> childDisplayableNodes = vtkSmartPointer<vtkCollection>::New();
  shNode->GetDataNodesInBranch(currentItemID, childDisplayableNodes, "vtkDMMLDisplayableNode");
  childDisplayableNodes->InitTraversal();
  for (int i=0; i<childDisplayableNodes->GetNumberOfItems(); ++i)
    {
    vtkDMMLDisplayableNode* displayableNode = vtkDMMLDisplayableNode::SafeDownCast(childDisplayableNodes->GetItemAsObject(i));
    vtkDMMLDisplayNode* displayNode = displayableNode ? vtkDMMLDisplayNode::SafeDownCast(displayableNode->GetDisplayNode()) : nullptr;
    if (displayNode)
      {
      displayNode->SetVisibility3D(on);
      }
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchyVisibilityPlugin::showInAllViews()
{
  // Get currently selected node and scene
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkIdType currentItemID = qCjyxSubjectHierarchyPluginHandler::instance()->currentItem();
  if (!currentItemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current subject hierarchy item";
    return;
    }

  vtkSmartPointer<vtkCollection> childDisplayableNodes = vtkSmartPointer<vtkCollection>::New();
  shNode->GetDataNodesInBranch(currentItemID, childDisplayableNodes, "vtkDMMLDisplayableNode");
  childDisplayableNodes->InitTraversal();
  for (int i=0; i<childDisplayableNodes->GetNumberOfItems(); ++i)
    {
    vtkDMMLDisplayableNode* displayableNode = vtkDMMLDisplayableNode::SafeDownCast(childDisplayableNodes->GetItemAsObject(i));
    vtkDMMLDisplayNode* displayNode = displayableNode ? vtkDMMLDisplayNode::SafeDownCast(displayableNode->GetDisplayNode()) : nullptr;
    if (displayNode && displayNode->IsShowModeDefault())
      {
      DMMLNodeModifyBlocker blocker(displayNode);
      displayNode->RemoveAllViewNodeIDs();
      displayNode->SetVisibility(true);
      }
    }
}
