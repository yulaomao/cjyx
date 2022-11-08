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
#include "qCjyxSubjectHierarchyDefaultPlugin.h"
#include "qCjyxSubjectHierarchyFolderPlugin.h"
#include "qCjyxSubjectHierarchyModelsPlugin.h"
#include "qCjyxSubjectHierarchyPluginHandler.h"

// Terminologies includes
#include "qCjyxTerminologyItemDelegate.h"
#include "vtkCjyxTerminologiesModuleLogic.h"

// DMML includes
#include <vtkDMMLScene.h>
#include <vtkDMMLModelNode.h>
#include <vtkDMMLModelDisplayNode.h>

// vtkSegmentationCore includes
#include <vtkSegment.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

// Qt includes
#include <QDebug>
#include <QStandardItem>
#include <QAction>

// Cjyx includes
#include "qCjyxAbstractModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_SubjectHierarchy_Plugins
class qCjyxSubjectHierarchyModelsPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qCjyxSubjectHierarchyModelsPlugin);
protected:
  qCjyxSubjectHierarchyModelsPlugin* const q_ptr;
public:
  qCjyxSubjectHierarchyModelsPluginPrivate(qCjyxSubjectHierarchyModelsPlugin& object);
  ~qCjyxSubjectHierarchyModelsPluginPrivate() override;
  void init();
public:
  QIcon ModelIcon;
};

//-----------------------------------------------------------------------------
// qCjyxSubjectHierarchyModelsPluginPrivate methods

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyModelsPluginPrivate::qCjyxSubjectHierarchyModelsPluginPrivate(qCjyxSubjectHierarchyModelsPlugin& object)
: q_ptr(&object)
, ModelIcon(QIcon(":Icons/Model.png"))
{
}

//------------------------------------------------------------------------------
void qCjyxSubjectHierarchyModelsPluginPrivate::init()
{
}

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyModelsPluginPrivate::~qCjyxSubjectHierarchyModelsPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qCjyxSubjectHierarchyModelsPlugin methods

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyModelsPlugin::qCjyxSubjectHierarchyModelsPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qCjyxSubjectHierarchyModelsPluginPrivate(*this) )
{
  this->m_Name = QString("Models");

  Q_D(qCjyxSubjectHierarchyModelsPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyModelsPlugin::~qCjyxSubjectHierarchyModelsPlugin() = default;

//----------------------------------------------------------------------------
double qCjyxSubjectHierarchyModelsPlugin::canAddNodeToSubjectHierarchy(
  vtkDMMLNode* node, vtkIdType parentItemID/*=vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID*/)const
{
  Q_UNUSED(parentItemID);
  if (!node)
    {
    qCritical() << Q_FUNC_INFO << ": Input node is NULL";
    return 0.0;
    }
  else if (node->IsA("vtkDMMLModelNode"))
    {
    // Node is a model
    return 0.5;
    }
  return 0.0;
}

//---------------------------------------------------------------------------
double qCjyxSubjectHierarchyModelsPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID)const
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

  // Model
  vtkDMMLNode* associatedNode = shNode->GetItemDataNode(itemID);
  if (associatedNode && associatedNode->IsA("vtkDMMLModelNode"))
    {
    return 0.5; // There may be other plugins that can handle special models better
    }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qCjyxSubjectHierarchyModelsPlugin::roleForPlugin()const
{
  return "Model";
}

//---------------------------------------------------------------------------
QIcon qCjyxSubjectHierarchyModelsPlugin::icon(vtkIdType itemID)
{
  Q_D(qCjyxSubjectHierarchyModelsPlugin);

  if (itemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QIcon();
    }

  if (this->canOwnSubjectHierarchyItem(itemID))
    {
    return d->ModelIcon;
    }

  // Item unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qCjyxSubjectHierarchyModelsPlugin::visibilityIcon(int visible)
{
  // Have the default plugin (which is not registered) take care of this
  return qCjyxSubjectHierarchyPluginHandler::instance()->defaultPlugin()->visibilityIcon(visible);
}

//-----------------------------------------------------------------------------
QString qCjyxSubjectHierarchyModelsPlugin::tooltip(vtkIdType itemID)const
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

  vtkDMMLModelNode* modelNode = vtkDMMLModelNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  vtkPolyData* polyData = modelNode ? modelNode->GetPolyData() : nullptr;
  vtkDMMLModelDisplayNode* displayNode = modelNode ? vtkDMMLModelDisplayNode::SafeDownCast(modelNode->GetDisplayNode()) : nullptr;
  if (modelNode && displayNode && polyData)
    {
    bool visible = (displayNode->GetVisibility() > 0);
    tooltipString.append( QString(" (Points: %1  Cells: %2  Visible: %3")
      .arg(polyData->GetNumberOfPoints()).arg(polyData->GetNumberOfCells())
      .arg(visible ? "YES" : "NO") );
    if (visible)
      {
        double color[3] = {0.0,0.0,0.0};
        displayNode->GetColor(color);
      tooltipString.append( QString("  Color: %4,%5,%6  Opacity: %7%")
        .arg(int(color[0]*255.0)).arg(int(color[1]*255.0)).arg(int(color[2]*255.0))
        .arg(int(displayNode->GetOpacity()*100.0)) );
      }
    tooltipString.append(QString(")"));
    }
  else
    {
    tooltipString.append(" !Invalid model");
    }

  return tooltipString;
}

//-----------------------------------------------------------------------------
void qCjyxSubjectHierarchyModelsPlugin::setDisplayColor(vtkIdType itemID, QColor color, QMap<int, QVariant> terminologyMetaData)
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

  // Get model node and display node
  vtkDMMLModelNode* modelNode = vtkDMMLModelNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!modelNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to find model node for subject hierarchy item " << shNode->GetItemName(itemID).c_str();
    return;
    }
  vtkDMMLModelDisplayNode* displayNode = vtkDMMLModelDisplayNode::SafeDownCast(modelNode->GetDisplayNode());
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": No display node for model";
    return;
    }

  // Set terminology metadata
  if (terminologyMetaData.contains(qCjyxTerminologyItemDelegate::TerminologyRole))
    {
    modelNode->SetAttribute(vtkSegment::GetTerminologyEntryTagName(),
      terminologyMetaData[qCjyxTerminologyItemDelegate::TerminologyRole].toString().toUtf8().constData() );
    }
  if (terminologyMetaData.contains(qCjyxTerminologyItemDelegate::NameRole))
    {
    modelNode->SetName(
      terminologyMetaData[qCjyxTerminologyItemDelegate::NameRole].toString().toUtf8().constData() );
    }
  if (terminologyMetaData.contains(qCjyxTerminologyItemDelegate::NameAutoGeneratedRole))
    {
    modelNode->SetAttribute( vtkCjyxTerminologiesModuleLogic::GetNameAutoGeneratedAttributeName(),
      terminologyMetaData[qCjyxTerminologyItemDelegate::NameAutoGeneratedRole].toString().toUtf8().constData() );
    }
  if (terminologyMetaData.contains(qCjyxTerminologyItemDelegate::ColorAutoGeneratedRole))
    {
    modelNode->SetAttribute( vtkCjyxTerminologiesModuleLogic::GetColorAutoGeneratedAttributeName(),
      terminologyMetaData[qCjyxTerminologyItemDelegate::ColorAutoGeneratedRole].toString().toUtf8().constData() );
    }

  // Set color
  double* oldColorArray = displayNode->GetColor();
  QColor oldColor = QColor::fromRgbF(oldColorArray[0], oldColorArray[1], oldColorArray[2]);
  if (oldColor != color)
    {
    displayNode->SetColor(color.redF(), color.greenF(), color.blueF());

    // Solid color is set, therefore disable scalar visibility
    // (otherwise color would come from the scalar value and colormap).
    displayNode->SetScalarVisibility(false);

    // Trigger update of color swatch
    shNode->ItemModified(itemID);
    }
}

//-----------------------------------------------------------------------------
QColor qCjyxSubjectHierarchyModelsPlugin::getDisplayColor(vtkIdType itemID, QMap<int, QVariant> &terminologyMetaData)const
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

  // Get model node and display node
  vtkDMMLModelNode* modelNode = vtkDMMLModelNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!modelNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to find model node for subject hierarchy item " << shNode->GetItemName(itemID).c_str();
    return QColor(0,0,0,0);
    }
  vtkDMMLModelDisplayNode* displayNode = vtkDMMLModelDisplayNode::SafeDownCast(modelNode->GetDisplayNode());
  if (!displayNode)
    {
    return QColor(0,0,0,0);
    }

  // Get terminology metadata
  terminologyMetaData.clear();
  terminologyMetaData[qCjyxTerminologyItemDelegate::TerminologyRole] =
    modelNode->GetAttribute(vtkSegment::GetTerminologyEntryTagName());
  terminologyMetaData[qCjyxTerminologyItemDelegate::NameRole] = modelNode->GetName();
  // If auto generated flags are not initialized, then set them to the default
  // (color: on, name: off - this way color will be set from the selector but name will not)
  bool nameAutoGenerated = false;
  if (modelNode->GetAttribute(vtkCjyxTerminologiesModuleLogic::GetNameAutoGeneratedAttributeName()))
    {
    nameAutoGenerated = QVariant(modelNode->GetAttribute(vtkCjyxTerminologiesModuleLogic::GetNameAutoGeneratedAttributeName())).toBool();
    }
  terminologyMetaData[qCjyxTerminologyItemDelegate::NameAutoGeneratedRole] = nameAutoGenerated;
  bool colorAutoGenerated = true;
  if (modelNode->GetAttribute(vtkCjyxTerminologiesModuleLogic::GetColorAutoGeneratedAttributeName()))
    {
    colorAutoGenerated = QVariant(modelNode->GetAttribute(vtkCjyxTerminologiesModuleLogic::GetColorAutoGeneratedAttributeName())).toBool();
    }
  terminologyMetaData[qCjyxTerminologyItemDelegate::ColorAutoGeneratedRole] = colorAutoGenerated;

  // Get and return color
  double* colorArray = displayNode->GetColor();
  return QColor::fromRgbF(colorArray[0], colorArray[1], colorArray[2]);
}
