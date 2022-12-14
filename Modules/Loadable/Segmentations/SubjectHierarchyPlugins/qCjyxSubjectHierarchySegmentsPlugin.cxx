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
#include "qCjyxSubjectHierarchySegmentsPlugin.h"
#include "qCjyxSubjectHierarchySegmentationsPlugin.h"
#include "vtkCjyxSegmentationsModuleLogic.h"

// SubjectHierarchy Plugins includes
#include "qCjyxSubjectHierarchyPluginHandler.h"
#include "qCjyxSubjectHierarchyDefaultPlugin.h"

// Terminologies includes
#include "qCjyxTerminologyItemDelegate.h"

// DMML includes
#include "vtkDMMLScene.h"
#include "vtkDMMLSegmentationNode.h"
#include "vtkDMMLSegmentationDisplayNode.h"
#include <vtkDMMLSliceNode.h>
#include "vtkDMMLSubjectHierarchyConstants.h"
#include "vtkDMMLSubjectHierarchyNode.h"

// Qt includes
#include <QDebug>
#include <QIcon>
#include <QMessageBox>
#include <QAction>
#include <QMenu>

// Cjyx includes
#include "qCjyxApplication.h"
#include "qCjyxLayoutManager.h"
#include "qDMMLSliceWidget.h"

// DMML widgets includes
#include "qDMMLNodeComboBox.h"

//-----------------------------------------------------------------------------
/// \ingroup CjyxRt_QtModules_Segmentations
class qCjyxSubjectHierarchySegmentsPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qCjyxSubjectHierarchySegmentsPlugin);
protected:
  qCjyxSubjectHierarchySegmentsPlugin* const q_ptr;
public:
  qCjyxSubjectHierarchySegmentsPluginPrivate(qCjyxSubjectHierarchySegmentsPlugin& object);
  ~qCjyxSubjectHierarchySegmentsPluginPrivate() override;
  void init();
public:
  QIcon SegmentIcon;

  QAction* ShowOnlyCurrentSegmentAction{nullptr};
  QAction* ShowAllSegmentsAction{nullptr};
  QAction* JumpSlicesAction{nullptr};
  QAction* CloneSegmentAction{nullptr};
};

//-----------------------------------------------------------------------------
// qCjyxSubjectHierarchySegmentsPluginPrivate methods

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchySegmentsPluginPrivate::qCjyxSubjectHierarchySegmentsPluginPrivate(qCjyxSubjectHierarchySegmentsPlugin& object)
: q_ptr(&object)
, SegmentIcon(QIcon(":Icons/Segment.png"))
{
}

//------------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentsPluginPrivate::init()
{
  Q_Q(qCjyxSubjectHierarchySegmentsPlugin);

  // Show only current segment action
  this->ShowOnlyCurrentSegmentAction = new QAction("Show only this segment",q);
  QObject::connect(this->ShowOnlyCurrentSegmentAction, SIGNAL(triggered()), q, SLOT(showOnlyCurrentSegment()));

  // Show all segments action
  this->ShowAllSegmentsAction = new QAction("Show all segments",q);
  QObject::connect(this->ShowAllSegmentsAction, SIGNAL(triggered()), q, SLOT(showAllSegments()));

  // Jump slices action
  this->JumpSlicesAction = new QAction("Jump slices",q);
  QObject::connect(this->JumpSlicesAction, SIGNAL(triggered()), q, SLOT(jumpSlices()));

  // Clone segment action
  this->CloneSegmentAction = new QAction("Clone", q);
  qCjyxSubjectHierarchyAbstractPlugin::setActionPosition(this->CloneSegmentAction,
    qCjyxSubjectHierarchyAbstractPlugin::SectionNode, 0.5); // put it right after "Rename" action
  QObject::connect(this->CloneSegmentAction, SIGNAL(triggered()), q, SLOT(cloneSegment()));
}

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchySegmentsPluginPrivate::~qCjyxSubjectHierarchySegmentsPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qCjyxSubjectHierarchySegmentsPlugin methods

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchySegmentsPlugin::qCjyxSubjectHierarchySegmentsPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qCjyxSubjectHierarchySegmentsPluginPrivate(*this) )
{
  this->m_Name = QString("Segments");

  Q_D(qCjyxSubjectHierarchySegmentsPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchySegmentsPlugin::~qCjyxSubjectHierarchySegmentsPlugin() = default;

//----------------------------------------------------------------------------
double qCjyxSubjectHierarchySegmentsPlugin::canReparentItemInsideSubjectHierarchy(vtkIdType itemID, vtkIdType parentItemID)const
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
  if ( parentItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID
    || shNode->GetItemAttribute(itemID, vtkDMMLSegmentationNode::GetSegmentIDAttributeName()).empty() )
    {
    // Cannot reparent if item is not a segment or there is no parent
    return 0.0;
    }

  vtkDMMLSegmentationNode* segmentationNode = vtkDMMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(parentItemID));
  if (segmentationNode)
    {
    // If item is segment and parent is segmentation then can reparent
    return 1.0;
    }

  return 0.0;
}

//---------------------------------------------------------------------------
bool qCjyxSubjectHierarchySegmentsPlugin::reparentItemInsideSubjectHierarchy(vtkIdType itemID, vtkIdType parentItemID)
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
  if ( parentItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID
    || shNode->GetItemAttribute(itemID, vtkDMMLSegmentationNode::GetSegmentIDAttributeName()).empty()
    || shNode->GetItemParent(itemID) == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID )
    {
    // Cannot reparent if item is not a segment or there is no parent
    return false;
    }

  // Get source and target segmentation items
  vtkDMMLSegmentationNode* fromSegmentationNode = vtkDMMLSegmentationNode::SafeDownCast(
    shNode->GetItemDataNode( shNode->GetItemParent(itemID) ) );
  vtkDMMLSegmentationNode* toSegmentationNode = vtkDMMLSegmentationNode::SafeDownCast(
    shNode->GetItemDataNode(parentItemID) );
  if (!fromSegmentationNode || !toSegmentationNode)
    {
    return false;
    }

  // Get segment ID
  std::string segmentId(shNode->GetItemAttribute(itemID, vtkDMMLSegmentationNode::GetSegmentIDAttributeName()));

  // Perform reparenting
  // Note: No actual subject hierarchy reparenting is done, because the function call below triggers
  //   removal and addition of segment subject hierarchy nodes as the segment is removed from source
  //   segmentation and added to target segmentation
  bool success = toSegmentationNode->GetSegmentation()->CopySegmentFromSegmentation(
    fromSegmentationNode->GetSegmentation(), segmentId, true );

  // Notify user if failed to reparent
  if (!success)
    {
    // If the two master representations are the same, then probably the segment IDs were duplicate
    if (fromSegmentationNode->GetSegmentation()->GetMasterRepresentationName() == toSegmentationNode->GetSegmentation()->GetMasterRepresentationName())
      {
      QString message = QString("Segment ID of the moved segment (%1) might exist in the target segmentation.\n"
        "Please check the error window for details.").arg(segmentId.c_str());
      QMessageBox::warning(nullptr, tr("Failed to move segment between segmentations"), message);
      return false;
      }

    // Otherwise master representation has to be changed
    QString message = QString("Cannot convert source master representation '%1' into target master '%2',"
      "thus unable to move segment '%3' from segmentation '%4' to '%5'.\n\n"
      "Would you like to change the master representation of '%5' to '%1'?\n\n"
      "Note: This may result in unwanted data loss in %5.")
      .arg(fromSegmentationNode->GetSegmentation()->GetMasterRepresentationName().c_str())
      .arg(toSegmentationNode->GetSegmentation()->GetMasterRepresentationName().c_str())
      .arg(segmentId.c_str()).arg(fromSegmentationNode->GetName()).arg(toSegmentationNode->GetName());
    QMessageBox::StandardButton answer =
      QMessageBox::question(nullptr, tr("Failed to move segment between segmentations"), message,
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer == QMessageBox::Yes)
      {
      // Convert target segmentation to master representation of source segmentation
      bool successfulConversion = toSegmentationNode->GetSegmentation()->CreateRepresentation(
        fromSegmentationNode->GetSegmentation()->GetMasterRepresentationName() );
      if (!successfulConversion)
        {
        QString message = QString("Failed to convert %1 to %2").arg(toSegmentationNode->GetName())
          .arg(fromSegmentationNode->GetSegmentation()->GetMasterRepresentationName().c_str());
        QMessageBox::warning(nullptr, tr("Conversion failed"), message);
        return false;
        }

      // Change master representation of target to that of source
      toSegmentationNode->GetSegmentation()->SetMasterRepresentationName(
        fromSegmentationNode->GetSegmentation()->GetMasterRepresentationName() );

      // Retry reparenting
      return this->reparentItemInsideSubjectHierarchy(itemID, parentItemID);
      }
    return false;
    }

  return true;
}

//---------------------------------------------------------------------------
double qCjyxSubjectHierarchySegmentsPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID)const
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

  // Segment
  if (!shNode->GetItemAttribute(itemID, vtkDMMLSegmentationNode::GetSegmentIDAttributeName()).empty())
    {
    return 1.0;
    }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qCjyxSubjectHierarchySegmentsPlugin::roleForPlugin()const
{
  return "Segment";
}

//---------------------------------------------------------------------------
const QString qCjyxSubjectHierarchySegmentsPlugin::helpText()const
{
  //return QString("<p style=\" margin-top:4px; margin-bottom:1px; margin-left:0px; margin-right:0px; "
  //  "-qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'sans-serif'; font-size:9pt; "
  //  "font-weight:600; color:#000000;\">Create new Contour set from scratch</span></p>"
  //  "<p style=\" margin-top:0px; margin-bottom:11px; margin-left:26px; margin-right:0px; "
  //  "-qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'sans-serif'; "
  //  "font-size:9pt; color:#000000;\">Right-click on an existing Study node and select 'Create child contour set'. "
  //  "This menu item is only available for Study level nodes</span></p>");
  //TODO:
  return QString();
}

//-----------------------------------------------------------------------------
QString qCjyxSubjectHierarchySegmentsPlugin::tooltip(vtkIdType itemID)const
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
  vtkDMMLScene* scene = qCjyxSubjectHierarchyPluginHandler::instance()->dmmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid DMML scene";
    return QString("Invalid");
    }

  // Get basic tooltip from abstract plugin
  QString tooltipString = Superclass::tooltip(itemID);

  if (scene->IsImporting())
    {
    // During import SH node may be created before the segmentation is read into the scene,
    // so don't attempt to access the segment yet
    return tooltipString;
    }

  vtkSegment* segment = vtkCjyxSegmentationsModuleLogic::GetSegmentForSegmentSubjectHierarchyItem(itemID, scene);
  if (!segment)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to get segment for segment subject hierarchy item " << shNode->GetItemName(itemID).c_str();
    return tooltipString;
    }

  // Representations
  std::vector<std::string> containedRepresentationNames;
  segment->GetContainedRepresentationNames(containedRepresentationNames);
  tooltipString.append( QString("Segment (Representations: ") );
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

  // Color
  double color[3] = {0.0,0.0,0.0};
  segment->GetColor(color);
  tooltipString.append( QString(" (Color: %1,%2,%3)").arg(
    (int)(color[0]*255)).arg((int)(color[1]*255)).arg((int)(color[2]*255)) );

  // Tags
  std::map<std::string,std::string> tags;
  segment->GetTags(tags);
  tooltipString.append( QString(" (Tags: ") );
  if (tags.empty())
    {
    tooltipString.append( QString("None)") );
    }
  else
    {
    for (std::map<std::string,std::string>::iterator tagIt=tags.begin(); tagIt!=tags.end(); ++tagIt)
      {
      std::string tagString = tagIt->first + ": " + tagIt->second + ", ";
      tooltipString.append( tagString.c_str() );
      }
    tooltipString = tooltipString.left(tooltipString.length()-2).append(")");
    }

  return tooltipString;
}

//---------------------------------------------------------------------------
QIcon qCjyxSubjectHierarchySegmentsPlugin::icon(vtkIdType itemID)
{
  if (itemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QIcon();
    }

  Q_D(qCjyxSubjectHierarchySegmentsPlugin);

  // Contour set
  if (this->canOwnSubjectHierarchyItem(itemID))
    {
    return d->SegmentIcon;
    }

  // Item unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qCjyxSubjectHierarchySegmentsPlugin::visibilityIcon(int visible)
{
  // Have the default plugin (which is not registered) take care of this
  return qCjyxSubjectHierarchyPluginHandler::instance()->defaultPlugin()->visibilityIcon(visible);
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentsPlugin::setDisplayVisibility(vtkIdType itemID, int visible)
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

  // Get segmentation node and display node
  vtkDMMLSegmentationNode* segmentationNode =
    vtkCjyxSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem(itemID, shNode->GetScene());
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to find segmentation node for segment subject hierarchy item " << shNode->GetItemName(itemID).c_str();
    return;
    }
  vtkDMMLSegmentationDisplayNode* displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(
    segmentationNode->GetDisplayNode() );
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": No display node for segmentation";
    return;
    }

  // Get segment ID
  std::string segmentId = shNode->GetItemAttribute(itemID, vtkDMMLSegmentationNode::GetSegmentIDAttributeName());

  // Set visibility
  displayNode->SetSegmentVisibility(segmentId, (bool)visible);

  // Trigger update of visibility icon
  shNode->ItemModified(itemID);
}

//-----------------------------------------------------------------------------
int qCjyxSubjectHierarchySegmentsPlugin::getDisplayVisibility(vtkIdType itemID)const
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
  vtkDMMLScene* scene = qCjyxSubjectHierarchyPluginHandler::instance()->dmmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid DMML scene";
    return -1;
    }

  if (scene->IsImporting())
    {
    // during import SH node may be created before the segmentation is read into the scene,
    // so don't attempt to access the segment yet
    return -1;
    }

  // Get segmentation node and display node
  vtkDMMLSegmentationNode* segmentationNode =
    vtkCjyxSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem(itemID, scene);
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to find segmentation node for segment subject hierarchy item " << shNode->GetItemName(itemID).c_str();
    return -1;
    }
  vtkDMMLSegmentationDisplayNode* displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(
    segmentationNode->GetDisplayNode() );
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": No display node for segmentation";
    return -1;
    }

  // Get segment ID
  std::string segmentId = shNode->GetItemAttribute(itemID, vtkDMMLSegmentationNode::GetSegmentIDAttributeName());

  // Get visibility
  return (displayNode->GetSegmentVisibility(segmentId) ? 1 : 0);
}

//-----------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentsPlugin::setDisplayColor(vtkIdType itemID, QColor color, QMap<int, QVariant> terminologyMetaData)
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
  vtkDMMLScene* scene = qCjyxSubjectHierarchyPluginHandler::instance()->dmmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid DMML scene";
    return;
    }

  // Get segment
  vtkSegment* segment = vtkCjyxSegmentationsModuleLogic::GetSegmentForSegmentSubjectHierarchyItem(itemID, scene);
  if (!segment)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to get segment for segment subject hierarchy item " << shNode->GetItemName(itemID).c_str();
    return;
    }

  // Set terminology metadata
  if (terminologyMetaData.contains(qCjyxTerminologyItemDelegate::TerminologyRole))
    {
    segment->SetTag(vtkSegment::GetTerminologyEntryTagName(),
      terminologyMetaData[qCjyxTerminologyItemDelegate::TerminologyRole].toString().toUtf8().constData() );
    }
  if (terminologyMetaData.contains(qCjyxTerminologyItemDelegate::NameRole))
    {
    segment->SetName(
      terminologyMetaData[qCjyxTerminologyItemDelegate::NameRole].toString().toUtf8().constData() );
    }
  if (terminologyMetaData.contains(qCjyxTerminologyItemDelegate::NameAutoGeneratedRole))
    {
    segment->SetNameAutoGenerated(
      terminologyMetaData[qCjyxTerminologyItemDelegate::NameAutoGeneratedRole].toBool() );
    }
  if (terminologyMetaData.contains(qCjyxTerminologyItemDelegate::ColorAutoGeneratedRole))
    {
    segment->SetColorAutoGenerated(
      terminologyMetaData[qCjyxTerminologyItemDelegate::ColorAutoGeneratedRole].toBool() );
    }

  // Set color.
  // We can end up in an infinite loop if the color cannot be set (by constantly triggering an update
  // due to detected change), therefore we compare the color carefully (clamp to valid range and use
  // a small tolerance for comparison).
  double oldColor[3];
  segment->GetColor(oldColor);
  double newColor[3] = { color.redF(), color.greenF(), color.blueF() };
  bool colorChanged = false;
  for (int i=0; i<3; i++)
    {
    if (!(oldColor[i] >= 0.0))  // use ! >= instead of < to include NaN values
      {
      oldColor[i] = 0.0;
      }
    else if (oldColor[i] > 1.0)
      {
      oldColor[i] = 1.0;
      }
    if (!(newColor[i] >= 0.0))  // use ! >= instead of < to include NaN values
      {
      newColor[i] = 0.0;
      }
    else if (newColor[i] > 1.0)
      {
      newColor[i] = 1.0;
      }
    if (fabs(oldColor[i] - newColor[i]) > 1e-6)
      {
      colorChanged = true;
      }
    }
  if (colorChanged)
    {
    segment->SetColor(newColor);

    // Trigger update of color swatch
    shNode->ItemModified(itemID);
    }
}

//-----------------------------------------------------------------------------
QColor qCjyxSubjectHierarchySegmentsPlugin::getDisplayColor(vtkIdType itemID, QMap<int, QVariant> &terminologyMetaData)const
{
  if (itemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QColor(0,0,0,0);
    }
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return QColor(0,0,0,0);
    }
  vtkDMMLScene* scene = qCjyxSubjectHierarchyPluginHandler::instance()->dmmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid DMML scene";
    return QColor(0,0,0,0);
    }

  if (scene->IsImporting())
    {
    // During import SH node may be created before the segmentation is read into the scene,
    // so don't attempt to access the segment yet
    return QColor(0,0,0,0);
    }

  // Get segment
  vtkSegment* segment = vtkCjyxSegmentationsModuleLogic::GetSegmentForSegmentSubjectHierarchyItem(itemID, scene);
  if (!segment)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to get segment for segment subject hierarchy item " << shNode->GetItemName(itemID).c_str();
    return QColor(0,0,0,0);
    }

  // Get terminology metadata
  terminologyMetaData.clear();
  std::string tagValue;
  terminologyMetaData[qCjyxTerminologyItemDelegate::TerminologyRole] =
    QVariant( segment->GetTag(vtkSegment::GetTerminologyEntryTagName(), tagValue) ? QString(tagValue.c_str()) : QString() );
  terminologyMetaData[qCjyxTerminologyItemDelegate::NameRole] = segment->GetName();
  terminologyMetaData[qCjyxTerminologyItemDelegate::NameAutoGeneratedRole] = segment->GetNameAutoGenerated();
  terminologyMetaData[qCjyxTerminologyItemDelegate::ColorAutoGeneratedRole] = segment->GetColorAutoGenerated();

  // Get and return color
  double* colorArray = segment->GetColor();
  return QColor::fromRgbF(colorArray[0], colorArray[1], colorArray[2]);
}

//---------------------------------------------------------------------------
QList<QAction*> qCjyxSubjectHierarchySegmentsPlugin::itemContextMenuActions()const
{
  Q_D(const qCjyxSubjectHierarchySegmentsPlugin);
  QList<QAction*> actions;
  actions << d->CloneSegmentAction;
  return actions;
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentsPlugin::showContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(const qCjyxSubjectHierarchySegmentsPlugin);

  qCjyxSubjectHierarchySegmentationsPlugin* segmentationsPlugin = qobject_cast<qCjyxSubjectHierarchySegmentationsPlugin*>(
    qCjyxSubjectHierarchyPluginHandler::instance()->pluginByName("Segmentations"));

  // Segments plugin shows all segmentations plugin functions in segment context menu
  segmentationsPlugin->showContextMenuActionsForItem(itemID);

  // Owned Segment
  if (this->canOwnSubjectHierarchyItem(itemID) && this->isThisPluginOwnerOfItem(itemID))
    {
    d->CloneSegmentAction->setVisible(true);
    }
}

//---------------------------------------------------------------------------
QList<QAction*> qCjyxSubjectHierarchySegmentsPlugin::visibilityContextMenuActions()const
{
  Q_D(const qCjyxSubjectHierarchySegmentsPlugin);

  QList<QAction*> actions;
  actions << d->ShowOnlyCurrentSegmentAction << d->ShowAllSegmentsAction << d->JumpSlicesAction;
  return actions;
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentsPlugin::showVisibilityContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(const qCjyxSubjectHierarchySegmentsPlugin);

  qCjyxSubjectHierarchySegmentationsPlugin* segmentationsPlugin = qobject_cast<qCjyxSubjectHierarchySegmentationsPlugin*>(
    qCjyxSubjectHierarchyPluginHandler::instance()->pluginByName("Segmentations") );

  // Segments plugin shows all segmentations plugin functions in segment context menu
  segmentationsPlugin->showContextMenuActionsForItem(itemID);

  // Owned Segment
  if (this->canOwnSubjectHierarchyItem(itemID) && this->isThisPluginOwnerOfItem(itemID))
    {
    d->ShowOnlyCurrentSegmentAction->setVisible(true);
    d->ShowAllSegmentsAction->setVisible(true);
    d->JumpSlicesAction->setVisible(true);
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentsPlugin::editProperties(vtkIdType itemID)
{
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Switch to segmentations module and select parent segmentation node
  vtkDMMLSegmentationNode* segmentationNode =
    vtkCjyxSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem(itemID, shNode->GetScene());
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to find segmentation node for segment subject hierarchy item " << shNode->GetItemName(itemID).c_str();
    return;
    }

  qCjyxApplication::application()->openNodeModule(segmentationNode);
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentsPlugin::showOnlyCurrentSegment()
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

  // Get segmentation node and display node
  vtkIdType segmentationShItemID = shNode->GetItemParent(currentItemID);
  if (segmentationShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find segmentation subject hierarchy item for segment item " << shNode->GetItemName(currentItemID).c_str();
    return;
    }
  vtkDMMLSegmentationNode* segmentationNode = vtkDMMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(segmentationShItemID));
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to find segmentation node for segment subject hierarchy item " << shNode->GetItemName(currentItemID).c_str();
    return;
    }
  vtkDMMLSegmentationDisplayNode* displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(
    segmentationNode->GetDisplayNode() );
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": No display node for segmentation";
    return;
    }

  // Hide all segments except the current one
  std::vector<vtkIdType> segmentShItemIDs;
  shNode->GetItemChildren(segmentationShItemID, segmentShItemIDs);
  std::vector<vtkIdType>::iterator segmentIt;
  for (segmentIt = segmentShItemIDs.begin(); segmentIt != segmentShItemIDs.end(); ++segmentIt)
    {
    vtkIdType segmentItemID = (*segmentIt);
    bool visible = false;
    if (segmentItemID == currentItemID)
      {
      visible = true;
      }

    // Get segment ID
    std::string segmentId = shNode->GetItemAttribute(segmentItemID, vtkDMMLSegmentationNode::GetSegmentIDAttributeName());
    // Set visibility
    displayNode->SetSegmentVisibility(segmentId, visible);
    // Trigger update of visibility icon
    shNode->ItemModified(segmentItemID);
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentsPlugin::showAllSegments()
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

  // Get segmentation node and display node
  vtkIdType segmentationShItemID = shNode->GetItemParent(currentItemID);
  if (segmentationShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find segmentation subject hierarchy item for segment item " << shNode->GetItemName(currentItemID).c_str();
    return;
    }
  vtkDMMLSegmentationNode* segmentationNode = vtkDMMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(segmentationShItemID));
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to find segmentation node for segment subject hierarchy item " << shNode->GetItemName(currentItemID).c_str();
    return;
    }
  vtkDMMLSegmentationDisplayNode* displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(
    segmentationNode->GetDisplayNode() );
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": No display node for segmentation";
    return;
    }

  // Show all segments
  std::vector<vtkIdType> segmentShItemIDs;
  shNode->GetItemChildren(segmentationShItemID, segmentShItemIDs);
  std::vector<vtkIdType>::iterator segmentIt;
  for (segmentIt = segmentShItemIDs.begin(); segmentIt != segmentShItemIDs.end(); ++segmentIt)
    {
    vtkIdType segmentItemID = (*segmentIt);
    // Get segment ID
    std::string segmentId = shNode->GetItemAttribute(segmentItemID, vtkDMMLSegmentationNode::GetSegmentIDAttributeName());
    // Set visibility
    displayNode->SetSegmentVisibility(segmentId, true);
    // Trigger update of visibility icon
    shNode->ItemModified(segmentItemID);
    }
}

//------------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentsPlugin::jumpSlices()
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

  // Get segmentation node
  vtkIdType segmentationShItemID = shNode->GetItemParent(currentItemID);
  if (segmentationShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find segmentation subject hierarchy item for segment item " << shNode->GetItemName(currentItemID).c_str();
    return;
    }
  vtkDMMLSegmentationNode* segmentationNode = vtkDMMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(segmentationShItemID));
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to find segmentation node for segment subject hierarchy item " << shNode->GetItemName(currentItemID).c_str();
    return;
    }

  // Get segment ID
  std::string segmentId = shNode->GetItemAttribute(currentItemID, vtkDMMLSegmentationNode::GetSegmentIDAttributeName());

  // Get center position of segment
  double* segmentCenterPosition = segmentationNode->GetSegmentCenterRAS(segmentId);
  if (!segmentCenterPosition)
    {
    return;
    }

  qCjyxLayoutManager* layoutManager = qCjyxApplication::application()->layoutManager();
  if (!layoutManager)
    {
    // Application is closing
    return;
    }
  foreach(QString sliceViewName, layoutManager->sliceViewNames())
    {
    // Check if segmentation is visible in this view
    qDMMLSliceWidget* sliceWidget = layoutManager->sliceWidget(sliceViewName);
    vtkDMMLSliceNode* sliceNode = sliceWidget->dmmlSliceNode();
    if (!sliceNode || !sliceNode->GetID())
      {
      continue;
      }
    bool visibleInView = false;
    int numberOfDisplayNodes = segmentationNode->GetNumberOfDisplayNodes();
    for (int displayNodeIndex = 0; displayNodeIndex < numberOfDisplayNodes; displayNodeIndex++)
      {
      vtkDMMLDisplayNode* segmentationDisplayNode = segmentationNode->GetNthDisplayNode(displayNodeIndex);
      if (segmentationDisplayNode && segmentationDisplayNode->IsDisplayableInView(sliceNode->GetID()))
        {
        visibleInView = true;
        break;
        }
      }
    if (!visibleInView)
      {
      continue;
      }
    sliceNode->JumpSliceByCentering(segmentCenterPosition[0], segmentCenterPosition[1], segmentCenterPosition[2]);
    }
}

//------------------------------------------------------------------------------
void qCjyxSubjectHierarchySegmentsPlugin::cloneSegment()
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

  // Get segmentation node
  vtkIdType segmentationShItemID = shNode->GetItemParent(currentItemID);
  if (segmentationShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find segmentation subject hierarchy item for segment item " << shNode->GetItemName(currentItemID).c_str();
    return;
    }
  vtkDMMLSegmentationNode* segmentationNode = vtkDMMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(segmentationShItemID));
  if (!segmentationNode || !segmentationNode->GetSegmentation())
    {
    qCritical() << Q_FUNC_INFO << ": Unable to find segmentation node for segment subject hierarchy item " << shNode->GetItemName(currentItemID).c_str();
    return;
    }

  // Get segment ID
  std::string segmentId = shNode->GetItemAttribute(currentItemID, vtkDMMLSegmentationNode::GetSegmentIDAttributeName());

  // Get segment and segmentation object
  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  vtkSegment* segment = segmentation->GetSegment(segmentId);
  if (!segment)
    {
    qCritical() << Q_FUNC_INFO << " failed: error getting the segment object";
    return;
    }

  // Copy
  vtkNew<vtkSegment> segmentCopy;
  segmentCopy->DeepCopy(segment);

  // Find the next segment's ID, because we want to insert the copied segment right below the current segment
  int segmentIndex = segmentation->GetSegmentIndex(segmentId);
  std::string insertBeforeSegmentId;
  if (segmentIndex + 1 < segmentation->GetNumberOfSegments())
    {
    insertBeforeSegmentId = segmentation->GetNthSegmentID(segmentIndex + 1);
    }

  std::string targetSegmentId = segmentation->GenerateUniqueSegmentID(segmentId);
  if (!segmentation->AddSegment(segmentCopy, targetSegmentId, insertBeforeSegmentId))
    {
    qCritical() << Q_FUNC_INFO << " failed: error adding cloned segment '" << segmentId.c_str() << "' to segmentation";
    return;
    }
}
//---------------------------------------------------------------------------
bool qCjyxSubjectHierarchySegmentsPlugin::showItemInView(
  vtkIdType itemID, vtkDMMLAbstractViewNode* viewNode, vtkIdList* allItemsToShow)
{
  Q_UNUSED(itemID);
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return false;
    }
  vtkIdType currentItemID = qCjyxSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return false;
    }

  // Get the segmentation node's itemID and use the segmentation plugin to show it.
  vtkIdType segmentationItemId = shNode->GetItemParent(currentItemID);
  qCjyxSubjectHierarchySegmentationsPlugin* segmentationsPlugin = qobject_cast<qCjyxSubjectHierarchySegmentationsPlugin*>(
    qCjyxSubjectHierarchyPluginHandler::instance()->pluginByName("Segmentations") );
  return segmentationsPlugin->showItemInView(segmentationItemId, viewNode, allItemsToShow);
}
