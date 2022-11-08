/*==============================================================================

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

// Segmentations includes
#include "qCjyxSubjectHierarchySegmentationsPlugin.h"

#include "qCjyxSubjectHierarchySegmentsPlugin.h"
#include "vtkSegmentation.h"

// SubjectHierarchy includes
#include "vtkCjyxSegmentationsModuleLogic.h"
#include "qCjyxSubjectHierarchyPluginHandler.h"
#include "qCjyxSubjectHierarchyDefaultPlugin.h"

// Qt includes
#include <QDebug>
#include <QAction>
#include <QMenu>
#include <QIcon>
#include <QMessageBox>
#include <QApplication>

// DMML includes
#include <vtkDMMLScene.h>
#include <vtkDMMLSegmentationNode.h>
#include <vtkDMMLSegmentationDisplayNode.h>
#include <vtkDMMLLabelMapVolumeNode.h>
#include <vtkDMMLModelNode.h>
#include <vtkDMMLModelDisplayNode.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_SubjectHierarchy_Widgets
class qCjyxSubjectHierarchySegmentationsPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qCjyxSubjectHierarchySegmentationsPlugin);
protected:
  qCjyxSubjectHierarchySegmentationsPlugin* const q_ptr;
public:
  qCjyxSubjectHierarchySegmentationsPluginPrivate(qCjyxSubjectHierarchySegmentationsPlugin& object);
  ~qCjyxSubjectHierarchySegmentationsPluginPrivate() override;
  void init();
public:
  QIcon SegmentationIcon;

  QAction* ExportBinaryLabelmapAction;
  QAction* ExportClosedSurfaceAction;
  QAction* ConvertLabelmapToSegmentationAction;
  QAction* ConvertModelToSegmentationAction;
  QAction* ConvertModelsToSegmentationAction;
  QAction* CreateBinaryLabelmapRepresentationAction;
  QAction* CreateClosedSurfaceRepresentationAction;
  QAction* RemoveBinaryLabelmapRepresentationAction;
  QAction* RemoveClosedSurfaceRepresentationAction;

  QAction* Toggle2DFillVisibilityAction;
  QAction* Toggle2DOutlineVisibilityAction;

  bool SegmentSubjectHierarchyItemRemovalInProgress;
};

//-----------------------------------------------------------------------------
// qCjyxSubjectHierarchySegmentationsPluginPrivate methods

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchySegmentationsPluginPrivate::qCjyxSubjectHierarchySegmentationsPluginPrivate(qCjyxSubjectHierarchySegmentationsPlugin& object)
: q_ptr(&object)
, SegmentationIcon(QIcon(":Icons/Segmentation.png"))
, ExportBinaryLabelmapAction(nullptr)
, ExportClosedSurfaceAction(nullptr)
, ConvertLabelmapToSegmentationAction(nullptr)
, ConvertModelToSegmentationAction(nullptr)
, ConvertModelsToSegmentationAction(nullptr)
, CreateBinaryLabelmapRepresentationAction(nullptr)
, CreateClosedSurfaceRepresentationAction(nullptr)
, RemoveBinaryLabelmapRepresentationAction(nullptr)
, RemoveClosedSurfaceRepresentationAction(nullptr)
, Toggle2DFillVisibilityAction(nullptr)
, Toggle2DOutlineVisibilityAction(nullptr)
, SegmentSubjectHierarchyItemRemovalInProgress(false)
{
}

//------------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentationsPluginPrivate::init()
{
  Q_Q(qCjyxSubjectHierarchySegmentationsPlugin);

  this->ExportBinaryLabelmapAction = new QAction("Export visible segments to binary labelmap",q);
  QObject::connect(this->ExportBinaryLabelmapAction, SIGNAL(triggered()), q, SLOT(exportToBinaryLabelmap()));

  this->ExportClosedSurfaceAction = new QAction("Export visible segments to models",q);
  QObject::connect(this->ExportClosedSurfaceAction, SIGNAL(triggered()), q, SLOT(exportToClosedSurface()));

  this->ConvertLabelmapToSegmentationAction = new QAction("Convert labelmap to segmentation node",q);
  QObject::connect(this->ConvertLabelmapToSegmentationAction, SIGNAL(triggered()), q, SLOT(convertLabelmapToSegmentation()));

  this->ConvertModelToSegmentationAction = new QAction("Convert model to segmentation node",q);
  QObject::connect(this->ConvertModelToSegmentationAction, SIGNAL(triggered()), q, SLOT(convertModelToSegmentation()));

  this->ConvertModelsToSegmentationAction = new QAction("Convert models to segmentation node",q);
  QObject::connect(this->ConvertModelsToSegmentationAction, SIGNAL(triggered()), q, SLOT(convertModelsToSegmentation()));

  this->CreateBinaryLabelmapRepresentationAction = new QAction("Create binary labelmap representation", q);
  QObject::connect(this->CreateBinaryLabelmapRepresentationAction, SIGNAL(triggered()), q, SLOT(createBinaryLabelmapRepresentation()));
  this->CreateClosedSurfaceRepresentationAction = new QAction("Create closed surface representation", q);
  QObject::connect(this->CreateClosedSurfaceRepresentationAction, SIGNAL(triggered()), q, SLOT(createClosedSurfaceRepresentation()));

  this->RemoveBinaryLabelmapRepresentationAction = new QAction("Remove binary labelmap representation", q);
  QObject::connect(this->RemoveBinaryLabelmapRepresentationAction, SIGNAL(triggered()), q, SLOT(removeBinaryLabelmapRepresentation()));
  this->RemoveClosedSurfaceRepresentationAction = new QAction("Remove closed surface representation", q);
  QObject::connect(this->RemoveClosedSurfaceRepresentationAction, SIGNAL(triggered()), q, SLOT(removeClosedSurfaceRepresentation()));

  this->Toggle2DFillVisibilityAction = new QAction("2D fill visibility",q);
  QObject::connect(this->Toggle2DFillVisibilityAction, SIGNAL(toggled(bool)), q, SLOT(toggle2DFillVisibility(bool)));
  this->Toggle2DFillVisibilityAction->setCheckable(true);
  this->Toggle2DFillVisibilityAction->setChecked(false);

  this->Toggle2DOutlineVisibilityAction = new QAction("2D outline visibility",q);
  QObject::connect(this->Toggle2DOutlineVisibilityAction, SIGNAL(toggled(bool)), q, SLOT(toggle2DOutlineVisibility(bool)));
  this->Toggle2DOutlineVisibilityAction->setCheckable(true);
  this->Toggle2DOutlineVisibilityAction->setChecked(false);
}

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchySegmentationsPluginPrivate::~qCjyxSubjectHierarchySegmentationsPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qCjyxSubjectHierarchySegmentationsPlugin methods

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchySegmentationsPlugin::qCjyxSubjectHierarchySegmentationsPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qCjyxSubjectHierarchySegmentationsPluginPrivate(*this) )
{
  this->m_Name = QString("Segmentations");

  Q_D(qCjyxSubjectHierarchySegmentationsPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchySegmentationsPlugin::~qCjyxSubjectHierarchySegmentationsPlugin() = default;

//----------------------------------------------------------------------------
double qCjyxSubjectHierarchySegmentationsPlugin::canAddNodeToSubjectHierarchy(
  vtkDMMLNode* node, vtkIdType parentItemID/*=vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID*/)const
{
  Q_UNUSED(parentItemID);
  if (!node)
    {
    qCritical() << Q_FUNC_INFO << ": Input node is NULL";
    return 0.0;
    }
  else if (node->IsA("vtkDMMLSegmentationNode"))
    {
    // Node is a segmentation
    return 0.9;
    }
  return 0.0;
}

//----------------------------------------------------------------------------
bool qCjyxSubjectHierarchySegmentationsPlugin::addNodeToSubjectHierarchy(vtkDMMLNode* nodeToAdd, vtkIdType parentItemID)
{
  if (!qCjyxSubjectHierarchyAbstractPlugin::addNodeToSubjectHierarchy(nodeToAdd, parentItemID))
    {
    return false;
    }
  vtkDMMLSegmentationNode* addedSegmentationNode = vtkDMMLSegmentationNode::SafeDownCast(nodeToAdd);
  if (!addedSegmentationNode)
    {
    qWarning() << Q_FUNC_INFO << ": segmentation node was expected";
    return false;
    }
  this->updateAllSegmentsFromDMML(addedSegmentationNode);
  return true;
}

//----------------------------------------------------------------------------
double qCjyxSubjectHierarchySegmentationsPlugin::canReparentItemInsideSubjectHierarchy(vtkIdType itemID, vtkIdType parentItemID)const
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
  if (parentItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    // Cannot reparent if there is no parent
    return 0.0;
    }

  vtkDMMLSegmentationNode* segmentationNode = vtkDMMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(parentItemID));
  if (segmentationNode)
    {
    // If item is labelmap or model and parent is segmentation then can reparent
    vtkDMMLLabelMapVolumeNode* labelmapNode = vtkDMMLLabelMapVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
    vtkDMMLModelNode* modelNode = vtkDMMLModelNode::SafeDownCast(shNode->GetItemDataNode(itemID));
    if (labelmapNode || modelNode)
      {
      return 1.0;
      }
    }

  return 0.0;
}

//---------------------------------------------------------------------------
bool qCjyxSubjectHierarchySegmentationsPlugin::reparentItemInsideSubjectHierarchy(vtkIdType itemID, vtkIdType parentItemID)
{
  if (itemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return false;
    }
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return false;
    }
  if (parentItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    // Cannot reparent if there is no parent
    return false;
    }

  vtkDMMLSegmentationNode* segmentationNode = vtkDMMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(parentItemID));
  vtkDMMLLabelMapVolumeNode* labelmapNode = vtkDMMLLabelMapVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  vtkDMMLModelNode* modelNode = vtkDMMLModelNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!segmentationNode || (!labelmapNode && !modelNode))
    {
    // Invalid inputs
    return false;
    }

  bool success = false;
  std::string importedRepresentationName("");
  if (labelmapNode)
    {
    importedRepresentationName = std::string(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName());
    success = vtkCjyxSegmentationsModuleLogic::ImportLabelmapToSegmentationNode(labelmapNode, segmentationNode);
    }
  else
    {
    importedRepresentationName = std::string(vtkSegmentationConverter::GetClosedSurfaceRepresentationName());
    success = vtkCjyxSegmentationsModuleLogic::ImportModelToSegmentationNode(modelNode, segmentationNode);
    }

  // Notify user if failed to import
  if (!success)
    {
    // Probably master representation has to be changed
    QString message = QString("Cannot convert source master representation '%1' into target master '%2',"
      "thus unable to import node '%3' to segmentation '%4'.\n\n"
      "Would you like to change the master representation of '%4' to '%1'?\n\n"
      "Note: This may result in unwanted data loss in %4.")
      .arg(importedRepresentationName.c_str())
      .arg(segmentationNode->GetSegmentation()->GetMasterRepresentationName().c_str())
      .arg(labelmapNode ? labelmapNode->GetName() : modelNode->GetName()).arg(segmentationNode->GetName());
    QMessageBox::StandardButton answer =
      QMessageBox::question(nullptr, tr("Failed to import data to segmentation"), message,
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer == QMessageBox::Yes)
      {
      // Convert target segmentation to master representation of source segmentation
      bool successfulConversion = segmentationNode->GetSegmentation()->CreateRepresentation(importedRepresentationName);
      if (!successfulConversion)
        {
        QString message = QString("Failed to convert %1 to %2").arg(segmentationNode->GetName()).arg(importedRepresentationName.c_str());
        QMessageBox::warning(nullptr, tr("Conversion failed"), message);
        return false;
        }

      // Change master representation of target to that of source
      segmentationNode->GetSegmentation()->SetMasterRepresentationName(importedRepresentationName);

      // Retry reparenting
      return this->reparentItemInsideSubjectHierarchy(itemID, parentItemID);
      }
    }

  // Real reparenting does not happen, the dragged node will remain where it was
  return false;
}

//---------------------------------------------------------------------------
double qCjyxSubjectHierarchySegmentationsPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID)const
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

  // Segmentation
  vtkDMMLNode* associatedNode = shNode->GetItemDataNode(itemID);
  if (associatedNode && associatedNode->IsA("vtkDMMLSegmentationNode"))
    {
    // Make sure the segmentation subject hierarchy item indicates its virtual branch
    shNode->SetItemAttribute(itemID,
      vtkDMMLSubjectHierarchyConstants::GetSubjectHierarchyVirtualBranchAttributeName().c_str(), "1");
    return 0.9;
    }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qCjyxSubjectHierarchySegmentationsPlugin::roleForPlugin()const
{
  return "Segmentation";
}

//-----------------------------------------------------------------------------
QString qCjyxSubjectHierarchySegmentationsPlugin::tooltip(vtkIdType itemID)const
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
    return QString("Invalid");
    }

  // Get basic tooltip from abstract plugin
  QString tooltipString = Superclass::tooltip(itemID);

  vtkDMMLSegmentationNode* segmentationNode = vtkDMMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Subject hierarchy item not associated to valid segmentation node";
    return tooltipString;
    }

  // Representations
  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  std::vector<std::string> containedRepresentationNames;
  segmentation->GetContainedRepresentationNames(containedRepresentationNames);
  tooltipString.append( QString(" (Representations: ") );
  if (containedRepresentationNames.empty())
    {
    tooltipString.append( QString("None!)") );
    }
  else
    {
    for (std::vector<std::string>::iterator reprIt = containedRepresentationNames.begin();
      reprIt != containedRepresentationNames.end(); ++reprIt)
      {
      tooltipString.append( reprIt->c_str() );
      tooltipString.append( ", " );
      }
    tooltipString = tooltipString.left(tooltipString.length()-2).append(")");
    }

  // Master representation
  tooltipString.append(QString(" (Master representation: %1)").arg(segmentation->GetMasterRepresentationName().c_str()));

  // Number of segments
  tooltipString.append(QString(" (Number of segments: %1)").arg(segmentation->GetNumberOfSegments()));

  return tooltipString;
}

//---------------------------------------------------------------------------
const QString qCjyxSubjectHierarchySegmentationsPlugin::helpText()const
{
  //TODO:
  //return QString("<p style=\" margin-top:4px; margin-bottom:1px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'sans-serif'; font-size:9pt; font-weight:600; color:#000000;\">Create new Contour set from scratch</span></p>"
  //  "<p style=\" margin-top:0px; margin-bottom:11px; margin-left:26px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'sans-serif'; font-size:9pt; color:#000000;\">Right-click on an existing Study node and select 'Create child contour set'. This menu item is only available for Study level nodes</span></p>");
  return QString();
}

//---------------------------------------------------------------------------
QIcon qCjyxSubjectHierarchySegmentationsPlugin::icon(vtkIdType itemID)
{
  if (itemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QIcon();
    }

  Q_D(qCjyxSubjectHierarchySegmentationsPlugin);

  // Segmentation
  if (this->canOwnSubjectHierarchyItem(itemID))
    {
    return d->SegmentationIcon;
    }

  // Item unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qCjyxSubjectHierarchySegmentationsPlugin::visibilityIcon(int visible)
{
  // Have the default plugin (which is not registered) take care of this
  return qCjyxSubjectHierarchyPluginHandler::instance()->defaultPlugin()->visibilityIcon(visible);
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentationsPlugin::setDisplayVisibility(vtkIdType itemID, int visible)
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

  vtkDMMLSegmentationNode* segmentationNode = vtkDMMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Subject hierarchy item not associated to valid segmentation node";
    return;
    }

  segmentationNode->SetDisplayVisibility(visible);

  // Trigger updating subject hierarchy visibility icon by calling modified on the segmentation SH node and all its parents
  std::set<vtkIdType> parentItems;
  vtkIdType parentItem = shNode->GetItemByDataNode(segmentationNode);
  do
    {
    parentItems.insert(parentItem);
    }
  while ( (parentItem = shNode->GetItemParent(parentItem) ) != shNode->GetSceneItemID() ); // The double parentheses avoids a Linux build warning

  std::set<vtkIdType>::iterator parentIt;
  for (parentIt=parentItems.begin(); parentIt!=parentItems.end(); ++parentIt)
    {
    shNode->ItemModified(*parentIt);
    }
}

//-----------------------------------------------------------------------------
int qCjyxSubjectHierarchySegmentationsPlugin::getDisplayVisibility(vtkIdType itemID)const
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

  vtkDMMLSegmentationNode* segmentationNode = vtkDMMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find segmentation node associated to subject hierarchy item " << itemID;
    return -1;
    }

  return segmentationNode->GetDisplayVisibility();
}

//---------------------------------------------------------------------------
QList<QAction*> qCjyxSubjectHierarchySegmentationsPlugin::itemContextMenuActions()const
{
  Q_D(const qCjyxSubjectHierarchySegmentationsPlugin);

  QList<QAction*> actions;
  actions
    << d->ExportBinaryLabelmapAction << d->ExportClosedSurfaceAction
    << d->CreateBinaryLabelmapRepresentationAction << d->RemoveBinaryLabelmapRepresentationAction
    << d->CreateClosedSurfaceRepresentationAction << d->RemoveClosedSurfaceRepresentationAction
    << d->ConvertLabelmapToSegmentationAction << d->ConvertModelToSegmentationAction << d->ConvertModelsToSegmentationAction;
  return actions;
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentationsPlugin::showContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qCjyxSubjectHierarchySegmentationsPlugin);

  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  if (itemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    // There are no scene actions in this plugin
    return;
    }

  // Owned Segmentation or Segment (segments plugin shows all segmentations plugin functions in segment context menu)
  qCjyxSubjectHierarchySegmentsPlugin* segmentsPlugin = qobject_cast<qCjyxSubjectHierarchySegmentsPlugin*>(
    qCjyxSubjectHierarchyPluginHandler::instance()->pluginByName("Segments") );
  if ( (this->canOwnSubjectHierarchyItem(itemID) && this->isThisPluginOwnerOfItem(itemID))
    || (segmentsPlugin->canOwnSubjectHierarchyItem(itemID) && segmentsPlugin->isThisPluginOwnerOfItem(itemID)) )
    {
    d->ExportBinaryLabelmapAction->setVisible(true);
    d->ExportClosedSurfaceAction->setVisible(true);

    // Get segmentation node
    vtkDMMLSegmentationNode* segmentationNode = vtkDMMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(itemID));
    if (!segmentationNode)
      {
      // The selected item is not a segmentation node, so the parent should be the segmentation node
      vtkIdType parentItemID = shNode->GetItemParent(itemID);
      if (parentItemID != vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
        {
        segmentationNode = vtkDMMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(parentItemID));
        }
      }

    // Add/remove representation actions
    if (segmentationNode && segmentationNode->GetSegmentation())
      {
      vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
      if (segmentation->GetMasterRepresentationName() != vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName())
        {
        if (segmentation->ContainsRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()))
          {
          d->RemoveBinaryLabelmapRepresentationAction->setVisible(true);
          }
        else
          {
        d->CreateBinaryLabelmapRepresentationAction->setVisible(true);
          }
        }
      if (segmentation->GetMasterRepresentationName() != vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName())
        {
        if (segmentation->ContainsRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName()))
          {
          d->RemoveClosedSurfaceRepresentationAction->setVisible(true);
          }
        else
          {
        d->CreateClosedSurfaceRepresentationAction->setVisible(true);
          }
        }
      }
    }
  else if (!shNode->GetItemOwnerPluginName(itemID).compare("LabelMaps"))
    {
    d->ConvertLabelmapToSegmentationAction->setVisible(true);
    }
  else if (!shNode->GetItemOwnerPluginName(itemID).compare("Models"))
    {
    d->ConvertModelToSegmentationAction->setVisible(true);
    }
  else if (!shNode->GetItemOwnerPluginName(itemID).compare("Folder"))
    {
    d->ConvertModelsToSegmentationAction->setVisible(true);
    }
}

//---------------------------------------------------------------------------
QList<QAction*> qCjyxSubjectHierarchySegmentationsPlugin::visibilityContextMenuActions()const
{
  Q_D(const qCjyxSubjectHierarchySegmentationsPlugin);

  QList<QAction*> actions;
  actions << d->Toggle2DFillVisibilityAction << d->Toggle2DOutlineVisibilityAction;
  return actions;
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentationsPlugin::showVisibilityContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qCjyxSubjectHierarchySegmentationsPlugin);

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

  // Segmentation
  if (this->canOwnSubjectHierarchyItem(itemID))
    {
    vtkDMMLSegmentationNode* segmentationNode = vtkDMMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(itemID));
    if (!segmentationNode)
      {
      qCritical() << Q_FUNC_INFO << ": Failed to find segmentation node associated to subject hierarchy item " << itemID;
      return;
      }
    vtkDMMLSegmentationDisplayNode* displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());
    if (!displayNode)
      {
      qCritical() << Q_FUNC_INFO << ": Failed to find display node for segmentation node " << segmentationNode->GetName();
      return;
      }

    d->Toggle2DFillVisibilityAction->blockSignals(true);
    d->Toggle2DFillVisibilityAction->setChecked(displayNode->GetVisibility2DFill());
    d->Toggle2DFillVisibilityAction->blockSignals(false);
    d->Toggle2DFillVisibilityAction->setVisible(true);

    d->Toggle2DOutlineVisibilityAction->blockSignals(true);
    d->Toggle2DOutlineVisibilityAction->setChecked(displayNode->GetVisibility2DOutline());
    d->Toggle2DOutlineVisibilityAction->blockSignals(false);
    d->Toggle2DOutlineVisibilityAction->setVisible(true);
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentationsPlugin::onSegmentAdded(vtkObject* caller, void* callData)
{
  // Get segmentation node
  vtkDMMLSegmentationNode* segmentationNode = reinterpret_cast<vtkDMMLSegmentationNode*>(caller);
  if (!segmentationNode)
    {
    return;
    }
  if (segmentationNode->GetScene() && segmentationNode->GetScene()->IsImporting())
    {
    // During scene import SH may not exist yet (if the scene was created without automatic SH creation)
    return;
    }
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Get associated subject hierarchy node
  vtkIdType segmentationShItemID = shNode->GetItemByDataNode(segmentationNode);
  if (segmentationShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy item for segmentation node " << segmentationNode->GetName();
    return;
    }

  // Get segment ID and segment
  char* segmentId = reinterpret_cast<char*>(callData);
  if (!segmentId)
    {
    // Calling InvokePendingModifiedEvent loses event parameters, so in this case segment IDs are empty
    this->updateAllSegmentsFromDMML(segmentationNode);
    return;
    }
  vtkSegment* segment = segmentationNode->GetSegmentation()->GetSegment(segmentId);
  if (!segment)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get added segment with ID '" << segmentId << "'";
    return;
    }

  // Find the current SegmentID if it already exists
  vtkIdType segmentShItemID = vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID;
  std::vector<vtkIdType> segmentShItemIDs;
  shNode->GetItemChildren(segmentationShItemID, segmentShItemIDs);
  std::vector<vtkIdType>::iterator segmentIt;
  for (segmentIt = segmentShItemIDs.begin(); segmentIt != segmentShItemIDs.end(); ++segmentIt)
    {
    std::string currentSegmentId = shNode->GetItemAttribute(*segmentIt, vtkDMMLSegmentationNode::GetSegmentIDAttributeName());
    if (!currentSegmentId.compare(segmentId))
      {
      segmentShItemID = (*segmentIt);
      break;
      }
    }

  if (segmentShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    // Get position of segment under parent
    int positionUnderParent = -1;
    vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
    if (segmentation)
      {
      positionUnderParent = segmentation->GetSegmentIndex(segmentId);
      }

    // Add the segment in subject hierarchy to allow individual handling (e.g. visibility)
    vtkIdType segmentShItemID = shNode->CreateHierarchyItem(
      segmentationShItemID, (segment->GetName() ? segment->GetName() : ""),
      vtkDMMLSubjectHierarchyConstants::GetSubjectHierarchyVirtualBranchAttributeName(),
      positionUnderParent);
    shNode->SetItemAttribute(segmentShItemID, vtkDMMLSegmentationNode::GetSegmentIDAttributeName(), segmentId);
    // Set plugin for the new item (automatically selects the segment plugin based on confidence values)
    qCjyxSubjectHierarchyPluginHandler::instance()->findAndSetOwnerPluginForSubjectHierarchyItem(segmentShItemID);
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentationsPlugin::onSegmentRemoved(vtkObject* caller, void* callData)
{
  // Do nothing if subject hierarchy item removal is already in progress
  Q_D(qCjyxSubjectHierarchySegmentationsPlugin);
  if (d->SegmentSubjectHierarchyItemRemovalInProgress)
    {
    return;
    }

  // Get segmentation node
  vtkDMMLSegmentationNode* segmentationNode = reinterpret_cast<vtkDMMLSegmentationNode*>(caller);
  if (!segmentationNode)
    {
    return;
    }
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Get associated subject hierarchy item
  vtkIdType segmentationShItemID = shNode->GetItemByDataNode(segmentationNode);
  if (segmentationShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Subject hierarchy item cannot be found for segmentation node "
      << segmentationNode->GetName() << " so per-segment subject hierarchy node cannot be removed.";
    return;
    }

  // Get segment ID
  char* segmentId = reinterpret_cast<char*>(callData);
  if (!segmentId)
    {
    // Calling InvokePendingModifiedEvent loses event parameters, so in this case segment IDs are empty
    this->updateAllSegmentsFromDMML(segmentationNode);
    return;
    }

  // Find subject hierarchy item for segment
  std::vector<vtkIdType> segmentShItemIDs;
  shNode->GetItemChildren(segmentationShItemID, segmentShItemIDs);
  std::vector<vtkIdType>::iterator segmentIt;
  for (segmentIt = segmentShItemIDs.begin(); segmentIt != segmentShItemIDs.end(); ++segmentIt)
    {
    std::string currentSegmentId = shNode->GetItemAttribute(*segmentIt, vtkDMMLSegmentationNode::GetSegmentIDAttributeName());
    if (currentSegmentId.empty())
      {
      // this child item is not a segment - ignore it
      continue;
      }
    if (!currentSegmentId.compare(segmentId))
      {
      shNode->RemoveItem(*segmentIt);
      return;
      }
    }

  // Log message if segment subject hierarchy item was not found
  qDebug() << Q_FUNC_INFO << ": Unable to find subject hierarchy item for segment" << segmentId << " in segmentation " << segmentationNode->GetName();
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentationsPlugin::onSegmentModified(vtkObject* caller, void* callData)
{
  // Get segmentation node
  vtkDMMLSegmentationNode* segmentationNode = reinterpret_cast<vtkDMMLSegmentationNode*>(caller);
  if (!segmentationNode)
    {
    return;
    }
  if (segmentationNode->GetScene() && segmentationNode->GetScene()->IsImporting())
    {
    // During scene import SH may not exist yet (if the scene was created without automatic SH creation)
    return;
    }
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Get associated subject hierarchy item
  vtkIdType segmentationShItemID = shNode->GetItemByDataNode(segmentationNode);
  if (segmentationShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to find subject hierarchy item for segmentation node "
      << segmentationNode->GetName() << " so per-segment subject hierarchy node cannot be created";
    return;
    }

  // Get segment ID and segment
  char* segmentId = reinterpret_cast<char*>(callData);
  if (!segmentId)
    {
    // no segmentId is specified - it means that any and all may have been changed
    this->updateAllSegmentsFromDMML(segmentationNode);
    return;
    }

  vtkSegment* segment = segmentationNode->GetSegmentation()->GetSegment(segmentId);
  if (!segment)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get added segment with ID '" << segmentId << "'";
    return;
    }

  // Find subject hierarchy item for segment
  vtkIdType segmentShItemID = vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID;
  std::vector<vtkIdType> segmentShItemIDs;
  shNode->GetItemChildren(segmentationShItemID, segmentShItemIDs);
  std::vector<vtkIdType>::iterator segmentIt;
  for (segmentIt = segmentShItemIDs.begin(); segmentIt != segmentShItemIDs.end(); ++segmentIt)
    {
    std::string currentSegmentId = shNode->GetItemAttribute(*segmentIt, vtkDMMLSegmentationNode::GetSegmentIDAttributeName());
    if (!currentSegmentId.compare(segmentId))
      {
      segmentShItemID = (*segmentIt);
      break;
      }
    }

  if (segmentShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    // Segment name and color is set for new segments before adding them to the segmentation.
    // In that case the subject hierarchy item does not exist yet for the segment
    return;
    }

  // Rename segment subject hierarchy item if segment name is different (i.e. has just been renamed)
  if (shNode->GetItemName(segmentShItemID).compare(segment->GetName() ? segment->GetName() : ""))
    {
    shNode->SetItemName(segmentShItemID, (segment->GetName() ? segment->GetName() : ""));
    // modified event is triggered by the name change, so there is no need for invoking modified event
    }
  else
    {
    shNode->InvokeCustomModifiedEvent(
      vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemModifiedEvent, (void*)&segmentShItemID);
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentationsPlugin::onDisplayNodeModified(vtkObject* caller)
{
  // Get segmentation node
  vtkDMMLSegmentationNode* segmentationNode = reinterpret_cast<vtkDMMLSegmentationNode*>(caller);
  if (!segmentationNode)
    {
    return;
    }
  this->updateAllSegmentsFromDMML(segmentationNode);
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentationsPlugin::onSubjectHierarchyItemModified(vtkObject* caller, void* callData)
{
  vtkDMMLSubjectHierarchyNode* shNode = reinterpret_cast<vtkDMMLSubjectHierarchyNode*>(caller);
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Get item ID
  vtkIdType itemID = vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID;
  if (callData)
    {
    vtkIdType* itemIdPtr = reinterpret_cast<vtkIdType*>(callData);
    if (itemIdPtr)
      {
      itemID = *itemIdPtr;
      }
    }
  if (!itemID)
    {
    return;
    }

  if (!shNode->HasItemAttribute(itemID, vtkDMMLSegmentationNode::GetSegmentIDAttributeName()))
    {
    return;
    }
  // If segment name is different than subject hierarchy item name then rename segment
  vtkSegment* segment = vtkCjyxSegmentationsModuleLogic::GetSegmentForSegmentSubjectHierarchyItem(itemID, shNode->GetScene());
  if (!segment)
    {
    return;
    }
  if (segment->GetName() && strcmp(segment->GetName(), shNode->GetItemName(itemID).c_str())==0)
    {
    // no change
    return;
    }
  segment->SetName(shNode->GetItemName(itemID).c_str());
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentationsPlugin::onSubjectHierarchyItemAboutToBeRemoved(vtkObject* caller, void* callData)
{
  Q_D(qCjyxSubjectHierarchySegmentationsPlugin);

  vtkDMMLSubjectHierarchyNode* shNode = reinterpret_cast<vtkDMMLSubjectHierarchyNode*>(caller);
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  if (!shNode->GetScene() || shNode->GetScene()->IsClosing())
    {
    return;
    }

  // Get item ID
  vtkIdType itemID = vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID;
  if (callData)
    {
    vtkIdType* itemIdPtr = reinterpret_cast<vtkIdType*>(callData);
    if (itemIdPtr)
      {
      itemID = *itemIdPtr;
      }
    }

  // If a segment subject hierarchy item was removed then remove segment from its segmentation
  // Note: No need to handle removal of segmentation item, because the virtual branch is
  //       automatically removed in case the parent node is removed (in vtkDMMLSubjectHierarchyNode::RemoveItem)
  if (shNode->HasItemAttribute(itemID, vtkDMMLSegmentationNode::GetSegmentIDAttributeName()))
    {
    std::string segmentId = shNode->GetItemAttribute(itemID, vtkDMMLSegmentationNode::GetSegmentIDAttributeName());

    // Rely only on ID because the removed node is not in the scene any more
    vtkIdType parentItemID = shNode->GetItemParent(itemID);
    if (parentItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
      {
      return;
      }
    vtkDMMLSegmentationNode* segmentationNode = vtkDMMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(parentItemID));
    if (segmentationNode && segmentationNode->GetSegmentation()->GetSegment(segmentId))
      {
      d->SegmentSubjectHierarchyItemRemovalInProgress = true;
      segmentationNode->GetSegmentation()->RemoveSegment(segmentId);
      d->SegmentSubjectHierarchyItemRemovalInProgress = false;
      }
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentationsPlugin::exportToBinaryLabelmap()
{
  vtkDMMLSegmentationNode* segmentationNode = vtkCjyxSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem(
    qCjyxSubjectHierarchyPluginHandler::instance()->currentItem(), qCjyxSubjectHierarchyPluginHandler::instance()->dmmlScene());
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: current subject hierarchy item is invalid.";
    return;
    }

  // Create binary labelmap representation using default parameters
  bool success = segmentationNode->GetSegmentation()->CreateRepresentation(
    vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
  if (!success)
    {
    QString message = QString( "Failed to create binary labelmap representation for segmentation %1 using default"
      "conversion parameters!\n\nPlease visit the Segmentation module and try the advanced create representation function.").
      arg(segmentationNode->GetName() );
    qCritical() << Q_FUNC_INFO << ": " << message;
    QMessageBox::warning(nullptr, tr("Failed to export segmentation to labelmap node"), message);
    return;
    }

  // Get exported (visible) segment IDs
  std::vector<std::string> segmentIDs;
  vtkDMMLSegmentationDisplayNode* displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());
  displayNode->GetVisibleSegmentIDs(segmentIDs);

  // Create new labelmap node
  vtkSmartPointer<vtkDMMLNode> newNode = vtkSmartPointer<vtkDMMLNode>::Take(
    segmentationNode->GetScene()->CreateNodeByClass("vtkDMMLLabelMapVolumeNode"));
  vtkDMMLLabelMapVolumeNode* newLabelmapNode = vtkDMMLLabelMapVolumeNode::SafeDownCast(
    segmentationNode->GetScene()->AddNode(newNode));
  newLabelmapNode->CreateDefaultDisplayNodes();
  std::string exportedNodeName = std::string(segmentationNode->GetName());
  if (segmentIDs.size() == 1)
    {
    exportedNodeName += "-" + std::string(segmentationNode->GetSegmentation()->GetSegment(segmentIDs[0])->GetName());
    }
  exportedNodeName += "-label";
  exportedNodeName = segmentationNode->GetScene()->GetUniqueNameByString(exportedNodeName.c_str());
  newLabelmapNode->SetName(exportedNodeName.c_str());

  // Get reference volume
  vtkDMMLVolumeNode* referenceVolumeNode = vtkDMMLVolumeNode::SafeDownCast(
    segmentationNode->GetNodeReference(vtkDMMLSegmentationNode::GetReferenceImageGeometryReferenceRole().c_str()) );

  // Export visible segments into a multi-label labelmap volume
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  success = vtkCjyxSegmentationsModuleLogic::ExportVisibleSegmentsToLabelmapNode(
    segmentationNode, newLabelmapNode, referenceVolumeNode, vtkSegmentation::EXTENT_REFERENCE_GEOMETRY);
  QApplication::restoreOverrideCursor();
  if (!success)
    {
    QString message = QString("Failed to export segments from segmentation %1 to labelmap node!\n\n"
      "Most probably the segment cannot be converted into binary labelmap representation").
      arg(segmentationNode->GetName());
    qCritical() << Q_FUNC_INFO << ": " << message;
    QMessageBox::warning(nullptr, tr("Failed to export segments"), message);
    return;
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentationsPlugin::exportToClosedSurface()
{
  vtkDMMLSegmentationNode* segmentationNode = vtkCjyxSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem(
    qCjyxSubjectHierarchyPluginHandler::instance()->currentItem(), qCjyxSubjectHierarchyPluginHandler::instance()->dmmlScene());
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: current subject hierarchy item is invalid.";
    return;
    }

  // Create closed surface representation using default parameters
  bool success = segmentationNode->GetSegmentation()->CreateRepresentation(
    vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
  if (!success)
    {
    QString message = QString( "Failed to create closed surface representation for segmentation %1 using default"
      "conversion parameters!\n\nPlease visit the Segmentation module and try the advanced create representation function.").
      arg(segmentationNode->GetName() );
    qCritical() << Q_FUNC_INFO << ": " << message;
    QMessageBox::warning(nullptr, tr("Failed to export segmentation to models"), message);
    return;
    }

  // Create new folder item
  std::string newFolderName = std::string(segmentationNode->GetName()) + "-models";
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  // Since segmentationNode is not nullptr, we can be sure that shNode is valid.
  vtkIdType currentItemID = qCjyxSubjectHierarchyPluginHandler::instance()->currentItem();
  vtkIdType folderItemID = shNode->CreateFolderItem(
    shNode->GetItemParent(currentItemID),
    shNode->GenerateUniqueItemName(newFolderName) );

  // Export visible segments into a models
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  success = vtkCjyxSegmentationsModuleLogic::ExportVisibleSegmentsToModels(
    segmentationNode, folderItemID );
  QApplication::restoreOverrideCursor();
  if (!success)
    {
    QString message = QString("Failed to export segments from segmentation %1 to models!\n\n"
      "Most probably the segment cannot be converted into closed surface representation.").
      arg(segmentationNode->GetName());
    qCritical() << Q_FUNC_INFO << ": " << message;
    QMessageBox::warning(nullptr, tr("Failed to export segments"), message);
    return;
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentationsPlugin::convertLabelmapToSegmentation()
{
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkIdType currentItemID = qCjyxSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
    }
  vtkDMMLLabelMapVolumeNode* labelmapNode = vtkDMMLLabelMapVolumeNode::SafeDownCast(shNode->GetItemDataNode(currentItemID));
  if (!labelmapNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access labelmap node";
    return;
    }

  // Create new segmentation node
  vtkSmartPointer<vtkDMMLNode> newNode = vtkSmartPointer<vtkDMMLNode>::Take(
    labelmapNode->GetScene()->CreateNodeByClass("vtkDMMLSegmentationNode"));
  vtkDMMLSegmentationNode* newSegmentationNode = vtkDMMLSegmentationNode::SafeDownCast(
    labelmapNode->GetScene()->AddNode(newNode));
  std::string newSegmentationNodeName = std::string(labelmapNode->GetName()) + "-segmentation";
  newSegmentationNode->SetName(newSegmentationNodeName.c_str());

  if (!vtkCjyxSegmentationsModuleLogic::ImportLabelmapToSegmentationNode(labelmapNode, newSegmentationNode))
    {
    qCritical() << Q_FUNC_INFO << ": Failed to import labelmap '" << labelmapNode->GetName() << "' to segmentation '" << newSegmentationNode->GetName() << "'";
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentationsPlugin::convertModelToSegmentation()
{
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkIdType currentItemID = qCjyxSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
    }
  vtkDMMLModelNode* modelNode = vtkDMMLModelNode::SafeDownCast(shNode->GetItemDataNode(currentItemID));
  if (!modelNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access model node";
    return;
    }

  // Create new segmentation node
  vtkSmartPointer<vtkDMMLNode> newNode = vtkSmartPointer<vtkDMMLNode>::Take(
    modelNode->GetScene()->CreateNodeByClass("vtkDMMLSegmentationNode"));
  vtkDMMLSegmentationNode* newSegmentationNode = vtkDMMLSegmentationNode::SafeDownCast(
    modelNode->GetScene()->AddNode(newNode));
  std::string newSegmentationNodeName = std::string(modelNode->GetName()) + "-segmentation";
  newSegmentationNode->SetName(newSegmentationNodeName.c_str());

  if (!vtkCjyxSegmentationsModuleLogic::ImportModelToSegmentationNode(modelNode, newSegmentationNode))
    {
    qCritical() << Q_FUNC_INFO << ": Failed to import model '" << modelNode->GetName() << "' to segmentation '" << newSegmentationNode->GetName() << "'";
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentationsPlugin::convertModelsToSegmentation()
{
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkIdType currentItemID = qCjyxSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
    }

  // Create new segmentation node
  vtkSmartPointer<vtkDMMLNode> newNode = vtkSmartPointer<vtkDMMLNode>::Take(
    shNode->GetScene()->CreateNodeByClass("vtkDMMLSegmentationNode"));
  vtkDMMLSegmentationNode* newSegmentationNode = vtkDMMLSegmentationNode::SafeDownCast(
    shNode->GetScene()->AddNode(newNode));
  std::string newSegmentationNodeName = shNode->GetItemName(currentItemID) + "-segmentation";
  newSegmentationNode->SetName(newSegmentationNodeName.c_str());
  newSegmentationNode->SetMasterRepresentationToClosedSurface();

  if (!vtkCjyxSegmentationsModuleLogic::ImportModelsToSegmentationNode(currentItemID, newSegmentationNode))
    {
    qCritical() << Q_FUNC_INFO << ": Failed to import models from folder '" << shNode->GetItemName(currentItemID).c_str()
      << "' to segmentation '" << newSegmentationNode->GetName() << "'";
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentationsPlugin::updateAllSegmentsFromDMML(vtkDMMLSegmentationNode* segmentationNode)
{
  // Get segmentation node
  if (!segmentationNode)
    {
    qWarning() << Q_FUNC_INFO << ": invalid segmentation node";
    return;
    }
  if (segmentationNode->GetScene() && segmentationNode->GetScene()->IsImporting())
    {
    // During scene import SH may not exist yet (if the scene was created without automatic SH creation)
    return;
    }
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Get associated subject hierarchy item
  vtkIdType segmentationShItemID = shNode->GetItemByDataNode(segmentationNode);
  if (segmentationShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to find subject hierarchy item for segmentation node "
      << segmentationNode->GetName() << " so per-segment subject hierarchy node cannot be created";
    return;
    }
  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  if (!segmentation)
    {
    qWarning() << Q_FUNC_INFO << ": invalid segmentation";
    return;
    }

  // List of segment IDs that have to be added to the segment list
  std::vector<std::string> segmentIDsToBeAddedToSh;
  segmentationNode->GetSegmentation()->GetSegmentIDs(segmentIDsToBeAddedToSh);

  // Segment modify/remove
  std::vector<vtkIdType> segmentShItemIDs;
  shNode->GetItemChildren(segmentationShItemID, segmentShItemIDs);
  std::vector<vtkIdType>::iterator segmentIt;
  for (segmentIt = segmentShItemIDs.begin(); segmentIt != segmentShItemIDs.end(); ++segmentIt)
    {
    std::string segmentId = shNode->GetItemAttribute(*segmentIt, vtkDMMLSegmentationNode::GetSegmentIDAttributeName());
    vtkSegment* segment = segmentation->GetSegment(segmentId);
    if (!segment)
      {
      // Segment has been removed
      this->onSegmentRemoved(segmentationNode, (void*)(segmentId.c_str()));
      continue;
      }
    this->onSegmentModified(segmentationNode, (void*)(segmentId.c_str()));

    // Remove segment ID from the list of segments to be added (it's already added)
    segmentIDsToBeAddedToSh.erase(std::remove(segmentIDsToBeAddedToSh.begin(), segmentIDsToBeAddedToSh.end(), segmentId), segmentIDsToBeAddedToSh.end());
    }

  // Segment add
  for (std::vector<std::string>::iterator segmentIdIt = segmentIDsToBeAddedToSh.begin(); segmentIdIt != segmentIDsToBeAddedToSh.end(); ++segmentIdIt)
    {
    this->onSegmentAdded(segmentationNode, (void*)(segmentIdIt->c_str()));
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentationsPlugin::toggle2DFillVisibility(bool on)
{
  vtkDMMLSegmentationNode* segmentationNode = vtkCjyxSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem(
    qCjyxSubjectHierarchyPluginHandler::instance()->currentItem(), qCjyxSubjectHierarchyPluginHandler::instance()->dmmlScene());
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: current subject hierarchy item is invalid.";
    return;
    }
  vtkDMMLSegmentationDisplayNode* displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find display node for segmentation node " << segmentationNode->GetName();
    return;
    }

  // Set 2D fill visibility
  displayNode->SetVisibility2DFill(on);
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentationsPlugin::toggle2DOutlineVisibility(bool on)
{
  vtkDMMLSegmentationNode* segmentationNode = vtkCjyxSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem(
    qCjyxSubjectHierarchyPluginHandler::instance()->currentItem(), qCjyxSubjectHierarchyPluginHandler::instance()->dmmlScene());
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: current subject hierarchy item is invalid.";
    return;
    }
  vtkDMMLSegmentationDisplayNode* displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find display node for segmentation node " << segmentationNode->GetName();
    return;
    }

  // Set 2D outline visibility
  displayNode->SetVisibility2DOutline(on);
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentationsPlugin::updateRepresentation(const QString& representationName, bool create)
{
  vtkDMMLSegmentationNode* segmentationNode = vtkCjyxSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem(
    qCjyxSubjectHierarchyPluginHandler::instance()->currentItem(), qCjyxSubjectHierarchyPluginHandler::instance()->dmmlScene());
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: current subject hierarchy item is invalid.";
    return;
    }

  if (create)
    {
    // Create representation using default parameters
    bool success = segmentationNode->GetSegmentation()->CreateRepresentation(representationName.toStdString());
    if (!success)
      {
      QString message = QString("Failed to create %1 representation for segmentation %2 using default"
        "conversion parameters!\n\nPlease visit the Segmentation module and try the advanced create representation function.")
       .arg(representationName)
       .arg(segmentationNode->GetName());
      qCritical() << Q_FUNC_INFO << ": " << message;
      QMessageBox::warning(nullptr, tr("Failed to export segmentation to models"), message);
      return;
      }
    }
  else
    {
    // Remove representation using default parameters
    segmentationNode->GetSegmentation()->RemoveRepresentation(representationName.toStdString());
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentationsPlugin::createBinaryLabelmapRepresentation()
{
  this->updateRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName(), true);
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentationsPlugin::createClosedSurfaceRepresentation()
{
  this->updateRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName(), true);
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentationsPlugin::removeBinaryLabelmapRepresentation()
{
  this->updateRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName(), false);
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentationsPlugin::removeClosedSurfaceRepresentation()
{
  this->updateRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName(), false);
}

//-----------------------------------------------------------------------------
bool qCjyxSubjectHierarchySegmentationsPlugin::showItemInView(vtkIdType itemID, vtkDMMLAbstractViewNode* viewNode, vtkIdList* allItemsToShow)
{
  vtkDMMLViewNode* threeDViewNode = vtkDMMLViewNode::SafeDownCast(viewNode);
  if (threeDViewNode)
    {
    // Display in a 3D view is requested - make sure closed surface representation is created.
    // Otherwise drag-and-drop of a segmentation into a 3D view could just make the segmentation
    // disappear (as segmentation only shows up in a 3D view if closed surface representation is available).
    vtkDMMLSegmentationNode* segmentationNode = vtkCjyxSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem(
      qCjyxSubjectHierarchyPluginHandler::instance()->currentItem(), qCjyxSubjectHierarchyPluginHandler::instance()->dmmlScene());
    if (segmentationNode)
      {
      segmentationNode->CreateClosedSurfaceRepresentation();
      }
    else
      {
      qWarning() << Q_FUNC_INFO << ": failed to get segmentation node";
      }
    }

  return qCjyxSubjectHierarchyAbstractPlugin::showItemInView(itemID, viewNode, allItemsToShow);
}
