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

// Qt includes
#include <QDebug>

// CjyxQt includes
#include <qCjyxCoreApplication.h>
#include <qCjyxModuleManager.h>
#include <qCjyxAbstractCoreModule.h>

// CTK includes
//#include <ctkModelTester.h>

// DMML includes
#include <vtkDMMLApplicationLogic.h>
#include <vtkDMMLScalarVolumeNode.h>
#include <vtkDMMLLabelMapVolumeNode.h>
#include <vtkDMMLVolumeDisplayNode.h>

// Colors includes
#include <vtkCjyxColorLogic.h>
#include <vtkDMMLColorLegendDisplayNode.h>

// Volumes includes
#include "qCjyxVolumesModuleWidget.h"
#include "ui_qCjyxVolumesModuleWidget.h"

#include "vtkCjyxVolumesLogic.h"

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_Volumes
class qCjyxVolumesModuleWidgetPrivate: public Ui_qCjyxVolumesModuleWidget
{
public:
};

//-----------------------------------------------------------------------------
qCjyxVolumesModuleWidget::qCjyxVolumesModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qCjyxVolumesModuleWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qCjyxVolumesModuleWidget::~qCjyxVolumesModuleWidget() = default;

//-----------------------------------------------------------------------------
void qCjyxVolumesModuleWidget::setup()
{
  Q_D(qCjyxVolumesModuleWidget);
  d->setupUi(this);

  QObject::connect(d->ActiveVolumeNodeSelector, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
                   d->DMMLVolumeInfoWidget, SLOT(setVolumeNode(vtkDMMLNode*)));

  QObject::connect(d->ActiveVolumeNodeSelector, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
                   d->VolumeDisplayWidget, SLOT(setDMMLVolumeNode(vtkDMMLNode*)));

  QObject::connect(d->ActiveVolumeNodeSelector, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
                   this, SLOT(nodeSelectionChanged(vtkDMMLNode*)));

  QObject::connect(d->ColorLegendCollapsibleButton, SIGNAL(contentsCollapsed(bool)),
                   this, SLOT(colorLegendCollapsibleButtonCollapsed(bool)));

  // Set up labelmap conversion
  d->ConvertVolumeFrame->setVisible(false);
  QObject::connect(d->ConvertVolumeButton, SIGNAL(clicked()),
                   this, SLOT(convertVolume()));
}

//------------------------------------------------------------------------------
void qCjyxVolumesModuleWidget::nodeSelectionChanged(vtkDMMLNode* node)
{
  Q_UNUSED(node);
  this->updateWidgetFromDMML();
}

//------------------------------------------------------------------------------
void qCjyxVolumesModuleWidget::updateWidgetFromDMML()
{
  Q_D(qCjyxVolumesModuleWidget);

  vtkDMMLVolumeNode* currentVolumeNode = vtkDMMLVolumeNode::SafeDownCast(
    d->ActiveVolumeNodeSelector->currentNode() );

  // Color legend section
  vtkDMMLColorLegendDisplayNode* colorLegendNode = nullptr;
  if (currentVolumeNode)
    {
    colorLegendNode = vtkCjyxColorLogic::GetColorLegendDisplayNode(currentVolumeNode);
    }
  d->ColorLegendDisplayNodeWidget->setDMMLColorLegendDisplayNode(colorLegendNode);
  if (!colorLegendNode)
    {
    d->ColorLegendCollapsibleButton->setCollapsed(true);
    }

  d->InfoCollapsibleButton->setEnabled(currentVolumeNode);
  d->DisplayCollapsibleButton->setEnabled(currentVolumeNode);
  d->ColorLegendCollapsibleButton->setEnabled(currentVolumeNode);

  // Convert to volume section
  bool convertVolumeSectionVisible = false;
  if (currentVolumeNode)
    {

    // Show convert to labelmap frame only if the exact type is scalar volume
    // (not labelmap, vector, tensor, DTI, etc.)
    QStringList convertTargetNodeTypes;
    if (!strcmp(currentVolumeNode->GetClassName(), "vtkDMMLScalarVolumeNode"))
      {
      convertVolumeSectionVisible = true;
      d->ConvertVolumeLabel->setText(tr("Convert to label map:"));
      convertTargetNodeTypes << "vtkDMMLLabelMapVolumeNode";
      }
    else if (!strcmp(currentVolumeNode->GetClassName(), "vtkDMMLLabelMapVolumeNode"))
      {
      convertVolumeSectionVisible = true;
      d->ConvertVolumeLabel->setText(tr("Convert to scalar volume:"));
      convertTargetNodeTypes << "vtkDMMLScalarVolumeNode";
      }
    if (convertVolumeSectionVisible)
      {
      // Set base name of target labelmap node
      d->ConvertVolumeTargetSelector->setBaseName(QString("%1_Label").arg(currentVolumeNode->GetName()));
      d->ConvertVolumeTargetSelector->setNodeTypes(convertTargetNodeTypes);
      }
    }
  d->ConvertVolumeFrame->setVisible(convertVolumeSectionVisible);
}

//------------------------------------------------------------------------------
void qCjyxVolumesModuleWidget::convertVolume()
{
  Q_D(qCjyxVolumesModuleWidget);

  vtkDMMLVolumeNode* currentVolume = vtkDMMLVolumeNode::SafeDownCast(d->ActiveVolumeNodeSelector->currentNode());
  if (!vtkDMMLScalarVolumeNode::SafeDownCast(currentVolume))
    {
    qWarning() << Q_FUNC_INFO << " failed: Cannot convert this volume type";
    return;
    }
  vtkCjyxVolumesLogic* logic = vtkCjyxVolumesLogic::SafeDownCast(this->logic());
  if (!logic)
    {
    qWarning() << Q_FUNC_INFO << " failed: Invalid volumes logic";
    return;
    }
  vtkDMMLLabelMapVolumeNode* currentLabelMapVolumeNode = vtkDMMLLabelMapVolumeNode::SafeDownCast(currentVolume);

  // If there is no target labelmap node selected, then perform in-place conversion
  vtkDMMLVolumeNode* targetVolumeNode = vtkDMMLVolumeNode::SafeDownCast(
    d->ConvertVolumeTargetSelector->currentNode());
  bool inPlaceConversion = (targetVolumeNode == nullptr);
  if (inPlaceConversion)
    {
    if (currentLabelMapVolumeNode)
      {
      targetVolumeNode = vtkDMMLScalarVolumeNode::New();
      }
    else
      {
      targetVolumeNode = vtkDMMLLabelMapVolumeNode::New();
      }
    targetVolumeNode->SetName(currentVolume->GetName());
    targetVolumeNode->SetHideFromEditors(currentVolume->GetHideFromEditors());
    targetVolumeNode->SetSaveWithScene(currentVolume->GetSaveWithScene());
    targetVolumeNode->SetSelectable(currentVolume->GetSelectable());
    targetVolumeNode->SetSingletonTag(currentVolume->GetSingletonTag());
    targetVolumeNode->SetDescription(currentVolume->GetDescription());
    std::vector< std::string > attributeNames = targetVolumeNode->GetAttributeNames();
    for (std::vector< std::string >::iterator attributeNameIt = attributeNames.begin();
      attributeNameIt != attributeNames.end(); ++attributeNameIt)
      {
      targetVolumeNode->SetAttribute(attributeNameIt->c_str(), currentVolume->GetAttribute(attributeNameIt->c_str()));
      }
    targetVolumeNode->SetDescription(currentVolume->GetDescription());
    this->dmmlScene()->AddNode(targetVolumeNode);
    targetVolumeNode->Delete(); // node is now solely owned by the scene
    }
  if (currentLabelMapVolumeNode)
    {
    logic->CreateScalarVolumeFromVolume(this->dmmlScene(),
      vtkDMMLScalarVolumeNode::SafeDownCast(targetVolumeNode), currentVolume);
    }
  else
    {
    logic->CreateLabelVolumeFromVolume(this->dmmlScene(),
      vtkDMMLLabelMapVolumeNode::SafeDownCast(targetVolumeNode), currentVolume);
    }

  // In case of in-place conversion select the new labelmap node and delete the scalar volume node
  if (inPlaceConversion)
    {
    d->ActiveVolumeNodeSelector->setCurrentNode(targetVolumeNode);
    this->dmmlScene()->RemoveNode(currentVolume);
    }
}

//-----------------------------------------------------------
bool qCjyxVolumesModuleWidget::setEditedNode(vtkDMMLNode* node,
                                               QString role /* = QString()*/,
                                               QString context /* = QString()*/)
{
  Q_D(qCjyxVolumesModuleWidget);
  Q_UNUSED(role);
  Q_UNUSED(context);
  if (vtkDMMLVolumeNode::SafeDownCast(node))
    {
    d->ActiveVolumeNodeSelector->setCurrentNode(node);
    return true;
    }

  if (vtkDMMLVolumeDisplayNode::SafeDownCast(node))
    {
    vtkDMMLVolumeDisplayNode* displayNode = vtkDMMLVolumeDisplayNode::SafeDownCast(node);
    vtkDMMLVolumeNode* displayableNode = vtkDMMLVolumeNode::SafeDownCast(displayNode->GetDisplayableNode());
    if (!displayableNode)
      {
      return false;
      }
    d->ActiveVolumeNodeSelector->setCurrentNode(displayableNode);
    return true;
    }

  return false;
}

//------------------------------------------------------------------------------
void qCjyxVolumesModuleWidget::colorLegendCollapsibleButtonCollapsed(bool collapsed)
{
  Q_D(qCjyxVolumesModuleWidget);
  if (collapsed)
    {
    return;
    }

  vtkDMMLVolumeNode* currentVolume = vtkDMMLVolumeNode::SafeDownCast(d->ActiveVolumeNodeSelector->currentNode());
  vtkDMMLColorLegendDisplayNode* colorLegendNode = vtkCjyxColorLogic::GetColorLegendDisplayNode(currentVolume);
  if (!colorLegendNode && currentVolume)
    {
    // color legend node does not exist, we need to create it now

    // Pause render to prevent the new Color legend displayed for a moment before it is hidden.
    vtkDMMLApplicationLogic* dmmlAppLogic = this->logic()->GetDMMLApplicationLogic();
    if (dmmlAppLogic)
      {
      dmmlAppLogic->PauseRender();
      }
    colorLegendNode = vtkCjyxColorLogic::AddDefaultColorLegendDisplayNode(currentVolume);
    if (colorLegendNode)
      {
      colorLegendNode->SetVisibility(false); // just because the groupbox is opened, don't show color legend yet
      }
    if (dmmlAppLogic)
      {
      dmmlAppLogic->ResumeRender();
      }
    }
  d->ColorLegendDisplayNodeWidget->setDMMLColorLegendDisplayNode(colorLegendNode);
}
