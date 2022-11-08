/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Alex Yarmakovich, Isomics Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// qCjyxVolumeRendering includes
#include "qCjyxVolumeRenderingModuleWidget.h"
#include "ui_qCjyxVolumeRenderingModuleWidget.h"

#include "vtkDMMLVolumeRenderingDisplayNode.h"
#include "vtkCjyxVolumeRenderingLogic.h"

#include "qCjyxCPURayCastVolumeRenderingPropertiesWidget.h"
#include "qCjyxGPURayCastVolumeRenderingPropertiesWidget.h"
#include "qCjyxMultiVolumeRenderingPropertiesWidget.h"
#include "qCjyxVolumeRenderingPresetComboBox.h"
#include "qCjyxGPUMemoryComboBox.h"

// DMML includes
#include "vtkDMMLAnnotationROINode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLVolumeNode.h"
#include "vtkDMMLViewNode.h"
#include "vtkDMMLVolumePropertyNode.h"
#include "vtkDMMLMarkupsROINode.h"

// VTK includes
#include <vtkVolumeProperty.h>

// STD includes
#include <vector>

// Qt includes
#include <QDebug>
#include <QSettings>

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_VolumeRendering
class qCjyxVolumeRenderingModuleWidgetPrivate
  : public Ui_qCjyxVolumeRenderingModuleWidget
{
  Q_DECLARE_PUBLIC(qCjyxVolumeRenderingModuleWidget);
protected:
  qCjyxVolumeRenderingModuleWidget* const q_ptr;

public:
  qCjyxVolumeRenderingModuleWidgetPrivate(qCjyxVolumeRenderingModuleWidget& object);
  virtual ~qCjyxVolumeRenderingModuleWidgetPrivate();

  virtual void setupUi(qCjyxVolumeRenderingModuleWidget*);
  vtkDMMLVolumeRenderingDisplayNode* displayNodeForVolumeNode(vtkDMMLVolumeNode* volumeNode)const;
  vtkDMMLVolumeRenderingDisplayNode* createVolumeRenderingDisplayNode(vtkDMMLVolumeNode* volumeNode);

  QMap<int, int>                     LastTechniques;
  double                             OldPresetPosition;
  QMap<QString, QWidget*>            RenderingMethodWidgets;
  vtkWeakPointer<vtkDMMLDisplayableNode> CropROINode;
  vtkWeakPointer<vtkDMMLVolumeRenderingDisplayNode> VolumeRenderingDisplayNode;
  vtkWeakPointer<vtkDMMLVolumePropertyNode> VolumePropertyNode;
};

//-----------------------------------------------------------------------------
// qCjyxVolumeRenderingModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qCjyxVolumeRenderingModuleWidgetPrivate::qCjyxVolumeRenderingModuleWidgetPrivate(qCjyxVolumeRenderingModuleWidget& object)
  : q_ptr(&object)
  , OldPresetPosition(0.0)
{
}

//-----------------------------------------------------------------------------
qCjyxVolumeRenderingModuleWidgetPrivate::~qCjyxVolumeRenderingModuleWidgetPrivate() = default;

//-----------------------------------------------------------------------------
void qCjyxVolumeRenderingModuleWidgetPrivate::setupUi(qCjyxVolumeRenderingModuleWidget* q)
{
  this->Ui_qCjyxVolumeRenderingModuleWidget::setupUi(q);

  QObject::connect(this->VolumeNodeComboBox, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
                   q, SLOT(onCurrentDMMLVolumeNodeChanged(vtkDMMLNode*)));
  // Inputs
  QObject::connect(this->VisibilityCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(onVisibilityChanged(bool)));
  QObject::connect(this->ROINodeComboBox, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
                   q, SLOT(onCurrentDMMLROINodeChanged(vtkDMMLNode*)));
  QObject::connect(this->VolumePropertyNodeComboBox, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
                   q, SLOT(onCurrentDMMLVolumePropertyNodeChanged(vtkDMMLNode*)));

  // Rendering
  QObject::connect(this->ROICropCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(onCropToggled(bool)));
  QObject::connect(this->ROICropDisplayCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(onROICropDisplayCheckBoxToggled(bool)));
  QObject::connect(this->ROIFitPushButton, SIGNAL(clicked()),
                   q, SLOT(fitROIToVolume()));

  // Techniques
  vtkCjyxVolumeRenderingLogic* volumeRenderingLogic = vtkCjyxVolumeRenderingLogic::SafeDownCast(q->logic());
  std::map<std::string, std::string> methods = volumeRenderingLogic->GetRenderingMethods();
  std::map<std::string, std::string>::const_iterator it;
  for (it = methods.begin(); it != methods.end(); ++it)
    {
    this->RenderingMethodComboBox->addItem(QString::fromStdString(it->first), QString::fromStdString(it->second));
    }
  QObject::connect(this->RenderingMethodComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onCurrentRenderingMethodChanged(int)));
  // Add empty widget at index 0 for the volume rendering methods with no widget.
  this->RenderingMethodStackedWidget->addWidget(new QWidget());
  q->addRenderingMethodWidget("vtkDMMLCPURayCastVolumeRenderingDisplayNode",
                              new qCjyxCPURayCastVolumeRenderingPropertiesWidget);
  q->addRenderingMethodWidget("vtkDMMLGPURayCastVolumeRenderingDisplayNode",
                              new qCjyxGPURayCastVolumeRenderingPropertiesWidget);
  q->addRenderingMethodWidget("vtkDMMLMultiVolumeRenderingDisplayNode",
                              new qCjyxMultiVolumeRenderingPropertiesWidget);

  // Currently, VTK ignores GPU memory size request - hide it on the GUI to not confuse users
  this->MemorySizeLabel->hide();
  this->MemorySizeComboBox->hide();

  QObject::connect(this->MemorySizeComboBox, SIGNAL(editTextChanged(QString)),
                   q, SLOT(onCurrentMemorySizeChanged()));
  QObject::connect(this->MemorySizeComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onCurrentMemorySizeChanged()));

  for (int qualityIndex=0; qualityIndex<vtkDMMLViewNode::VolumeRenderingQuality_Last; qualityIndex++)
    {
    this->QualityControlComboBox->addItem(vtkDMMLViewNode::GetVolumeRenderingQualityAsString(qualityIndex));
    }
  QObject::connect(this->QualityControlComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onCurrentQualityControlChanged(int)));

  QObject::connect(this->FramerateSliderWidget, SIGNAL(valueChanged(double)),
                   q, SLOT(onCurrentFramerateChanged(double)));

  QObject::connect(this->AutoReleaseGraphicsResourcesCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(onAutoReleaseGraphicsResourcesCheckBoxToggled(bool)));

  void onAutoReleaseGraphicsResourcesChanged(bool autoRelease);

  // Volume Properties
  this->PresetComboBox->setDMMLScene(volumeRenderingLogic->GetPresetsScene());
  this->PresetComboBox->setCurrentNode(nullptr);

  QObject::connect(this->PresetComboBox, SIGNAL(presetOffsetChanged(double, double, bool)),
                   this->VolumePropertyNodeWidget, SLOT(moveAllPoints(double, double, bool)));

  this->VolumePropertyNodeWidget->setThreshold(!volumeRenderingLogic->GetUseLinearRamp());
  QObject::connect(this->VolumePropertyNodeWidget, SIGNAL(thresholdChanged(bool)),
                   q, SLOT(onThresholdChanged(bool)));
  QObject::connect(this->VolumePropertyNodeWidget, SIGNAL(chartsExtentChanged()),
                   q, SLOT(onChartsExtentChanged()));

  QObject::connect(this->VolumePropertyNodeComboBox, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
                   this->PresetComboBox, SLOT(setDMMLVolumePropertyNode(vtkDMMLNode*)));

  QObject::connect(this->SynchronizeScalarDisplayNodeButton, SIGNAL(clicked()),
                   q, SLOT(synchronizeScalarDisplayNode()));
  QObject::connect(this->SynchronizeScalarDisplayNodeButton, SIGNAL(toggled(bool)),
                   q, SLOT(setFollowVolumeDisplayNode(bool)));
  QObject::connect(this->IgnoreVolumesThresholdCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setIgnoreVolumesThreshold(bool)));

  // Disable markups ROI widget by default
  this->MarkupsROIWidget->setVisible(false);

  // Default values
  this->InputsCollapsibleButton->setCollapsed(true);
  this->InputsCollapsibleButton->setEnabled(false);;
  this->AdvancedCollapsibleButton->setCollapsed(true);
  this->AdvancedCollapsibleButton->setEnabled(false);

  this->ExpandSynchronizeWithVolumesButton->setChecked(false);

  this->AdvancedTabWidget->setCurrentWidget(this->VolumePropertyTab);

  // Ensure that the view node combo box only shows view nodes, not slice nodes or chart nodes
  this->ViewCheckableNodeComboBox->setNodeTypes(QStringList(QString("vtkDMMLViewNode")));
}

// --------------------------------------------------------------------------
vtkDMMLVolumeRenderingDisplayNode* qCjyxVolumeRenderingModuleWidgetPrivate::displayNodeForVolumeNode(vtkDMMLVolumeNode* volumeNode)const
{
  if (!volumeNode)
    {
    return nullptr;
    }

  Q_Q(const qCjyxVolumeRenderingModuleWidget);
  vtkCjyxVolumeRenderingLogic *logic = vtkCjyxVolumeRenderingLogic::SafeDownCast(q->logic());
  if (!logic)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access volume rendering logic";
    return nullptr;
    }

  // Get volume rendering display node for volume
  return logic->GetFirstVolumeRenderingDisplayNode(volumeNode);
}

// --------------------------------------------------------------------------
vtkDMMLVolumeRenderingDisplayNode* qCjyxVolumeRenderingModuleWidgetPrivate::createVolumeRenderingDisplayNode(
  vtkDMMLVolumeNode* volumeNode)
{
  Q_Q(qCjyxVolumeRenderingModuleWidget);

  vtkCjyxVolumeRenderingLogic *logic = vtkCjyxVolumeRenderingLogic::SafeDownCast(q->logic());
  if (!logic)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access volume rendering logic";
    return nullptr;
    }

  vtkSmartPointer<vtkDMMLVolumeRenderingDisplayNode> displayNode =
    vtkSmartPointer<vtkDMMLVolumeRenderingDisplayNode>::Take(logic->CreateVolumeRenderingDisplayNode());
  displayNode->SetVisibility(0);
  q->dmmlScene()->AddNode(displayNode);

  if (volumeNode)
    {
    volumeNode->AddAndObserveDisplayNodeID(displayNode->GetID());
    }

  int wasModifying = displayNode->StartModify();
  // Initialize volume rendering without the threshold info of the Volumes module
  displayNode->SetIgnoreVolumeDisplayNodeThreshold(1);
  logic->UpdateDisplayNodeFromVolumeNode(displayNode, volumeNode, nullptr, nullptr, false /*do not create ROI*/);
  // Apply previous selection to the newly selected volume
  displayNode->SetIgnoreVolumeDisplayNodeThreshold(this->IgnoreVolumesThresholdCheckBox->isChecked());
  // Do not show newly selected volume (because it would be triggered by simply selecting it in the combobox,
  // and it would not adhere to the customary Cjyx behavior)
  // Set selected views to the display node
  foreach (vtkDMMLAbstractViewNode* viewNode, this->ViewCheckableNodeComboBox->checkedViewNodes())
    {
    displayNode->AddViewNodeID(viewNode ? viewNode->GetID() : nullptr);
    }
  displayNode->EndModify(wasModifying);
  return displayNode;
}

//-----------------------------------------------------------------------------
// qCjyxVolumeRenderingModuleWidget methods

//-----------------------------------------------------------------------------
qCjyxVolumeRenderingModuleWidget::qCjyxVolumeRenderingModuleWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qCjyxVolumeRenderingModuleWidgetPrivate(*this) )
{
  // setup the UI only in setup where the logic is available
}

//-----------------------------------------------------------------------------
qCjyxVolumeRenderingModuleWidget::~qCjyxVolumeRenderingModuleWidget() = default;

//-----------------------------------------------------------------------------
void qCjyxVolumeRenderingModuleWidget::setup()
{
  Q_D(qCjyxVolumeRenderingModuleWidget);
  d->setupUi(this);
}

// --------------------------------------------------------------------------
vtkDMMLVolumeNode* qCjyxVolumeRenderingModuleWidget::dmmlVolumeNode()const
{
  Q_D(const qCjyxVolumeRenderingModuleWidget);
  return vtkDMMLVolumeNode::SafeDownCast(d->VolumeNodeComboBox->currentNode());
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingModuleWidget::setDMMLVolumeNode(vtkDMMLNode* volumeNode)
{
  Q_D(qCjyxVolumeRenderingModuleWidget);
  d->VolumeNodeComboBox->setCurrentNode(volumeNode);
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingModuleWidget::onCurrentDMMLVolumeNodeChanged(vtkDMMLNode* node)
{
  Q_D(qCjyxVolumeRenderingModuleWidget);

  vtkDMMLVolumeNode* volumeNode = vtkDMMLVolumeNode::SafeDownCast(node);

  vtkCjyxVolumeRenderingLogic* logic = vtkCjyxVolumeRenderingLogic::SafeDownCast(this->logic());
  if (!logic)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access volume rendering logic";
    return;
    }

  vtkDMMLVolumeRenderingDisplayNode* displayNode = d->displayNodeForVolumeNode(volumeNode);
  if (!displayNode && volumeNode)
    {
    displayNode = d->createVolumeRenderingDisplayNode(volumeNode);
    }

  qvtkReconnect(d->VolumeRenderingDisplayNode, displayNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromDMML()));
  d->VolumeRenderingDisplayNode = displayNode;

  d->ViewCheckableNodeComboBox->setDMMLDisplayNode(displayNode);

  // Select preset node that was previously selected for this volume
  vtkDMMLVolumePropertyNode* volumePropertyNode = this->dmmlVolumePropertyNode();
  if (volumePropertyNode)
    {
    vtkDMMLVolumePropertyNode* presetNode = logic->GetPresetByName(volumePropertyNode->GetName());
    bool wasBlocking = d->PresetComboBox->blockSignals(true);
    d->PresetComboBox->setCurrentNode(presetNode);
    d->PresetComboBox->blockSignals(wasBlocking);
    }

  // Update widget from display node of the volume node
  this->updateWidgetFromDMML();
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingModuleWidget::onVisibilityChanged(bool visible)
{
  Q_D(qCjyxVolumeRenderingModuleWidget);

  // Get volume rendering display node for volume. Create if absent.
  vtkDMMLVolumeNode* volumeNode = this->dmmlVolumeNode();
  vtkDMMLVolumeRenderingDisplayNode* displayNode = d->displayNodeForVolumeNode(volumeNode);
  if (!displayNode)
    {
    if (volumeNode)
      {
      qCritical() << Q_FUNC_INFO << ": No volume rendering display node for volume " << volumeNode->GetName();
      }
    return;
    }

  displayNode->SetVisibility(visible);

  // Update widget from display node of the volume node
  this->updateWidgetFromDMML();
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingModuleWidget::addRenderingMethodWidget(
  const QString& methodClassName, qCjyxVolumeRenderingPropertiesWidget* widget)
{
  Q_D(qCjyxVolumeRenderingModuleWidget);
  d->RenderingMethodStackedWidget->addWidget(widget);
  d->RenderingMethodWidgets[methodClassName] = widget;
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingModuleWidget::updateWidgetFromDMML()
{
  Q_D(qCjyxVolumeRenderingModuleWidget);

  // Get display node
  vtkDMMLVolumeRenderingDisplayNode* displayNode = this->dmmlDisplayNode();

  // Get first view node
  vtkDMMLViewNode* firstViewNode = nullptr;
  if (displayNode && displayNode->GetScene())
    {
    firstViewNode = displayNode->GetFirstViewNode();
    }

  // Visibility checkbox
  d->VisibilityCheckBox->setChecked(displayNode ? displayNode->GetVisibility() : false);

  // Input section

  // Volume property selector
  // Update shift slider range and set transfer function extents when volume property node is modified
  vtkDMMLVolumePropertyNode* volumePropertyNode = (displayNode ? displayNode->GetVolumePropertyNode() : nullptr);
  this->qvtkReconnect(d->VolumePropertyNode, volumePropertyNode, vtkDMMLVolumePropertyNode::EffectiveRangeModified, this, SLOT(onEffectiveRangeModified()));
  bool volumePropertyNodeChanged = (d->VolumePropertyNode != volumePropertyNode);
  d->VolumePropertyNode = volumePropertyNode;
  if (volumePropertyNodeChanged)
    {
    // Perform widget updates
    this->onEffectiveRangeModified();
    }
  d->VolumePropertyNodeComboBox->setCurrentNode(volumePropertyNode);

  // ROI selector
  vtkDMMLAnnotationROINode* annotationROINode = (displayNode ? displayNode->GetAnnotationROINode() : nullptr);
  vtkDMMLMarkupsROINode* markupsROINode = (displayNode ? displayNode->GetMarkupsROINode() : nullptr);
  vtkDMMLDisplayableNode* roiNode = markupsROINode;
  if (!roiNode)
    {
    roiNode = annotationROINode;
    }
  this->qvtkReconnect(d->CropROINode, roiNode, vtkDMMLDisplayableNode::DisplayModifiedEvent, this, SLOT(updateWidgetFromROINode()));
  d->CropROINode = roiNode;
  this->updateWidgetFromROINode();

  bool wasBlocking = d->ROINodeComboBox->blockSignals(true);
  d->ROINodeComboBox->setCurrentNode(roiNode);
  d->ROINodeComboBox->blockSignals(wasBlocking);

  // Disable UI if there is no display node yet (need to show it first to have a display node)
  d->DisplayCollapsibleButton->setEnabled(displayNode != nullptr);
  d->AdvancedCollapsibleButton->setEnabled(displayNode != nullptr);

  // Display section
  d->PresetComboBox->setEnabled(volumePropertyNode != nullptr);
  wasBlocking = d->PresetComboBox->blockSignals(true);
  d->PresetComboBox->setCurrentNode(
    volumePropertyNode ? vtkCjyxVolumeRenderingLogic::SafeDownCast(this->logic())->GetPresetByName(volumePropertyNode->GetName()) : nullptr );
  d->PresetComboBox->blockSignals(wasBlocking);
  d->ROICropCheckBox->setChecked(roiNode && displayNode ? displayNode->GetCroppingEnabled() : false);
  d->ROICropCheckBox->setEnabled(displayNode != nullptr); // ROI can be created on request if display node is set
  d->ROICropDisplayCheckBox->setEnabled(displayNode != nullptr); // ROI can be created on request if display node is set
  d->ROIFitPushButton->setEnabled(roiNode != nullptr);
  d->RenderingMethodComboBox->setEnabled(displayNode != nullptr);

  // Advanced section

  // Volume properties tab
  d->VolumePropertyNodeWidget->setEnabled(volumePropertyNode != nullptr);

  // ROI tab
  d->MarkupsROIWidget->setDMMLMarkupsNode(markupsROINode);
  d->AnnotationROIWidget->setDMMLAnnotationROINode(annotationROINode);
  if (markupsROINode && !annotationROINode)
    {
    d->MarkupsROIWidget->setVisible(true);
    d->AnnotationROIWidget->setVisible(false);
    }
  else if (annotationROINode && !markupsROINode)
    {
    d->AnnotationROIWidget->setVisible(true);
    d->MarkupsROIWidget->setVisible(false);
    }

  // Techniques tab
  QSettings settings;
  QString defaultRenderingMethod =
    settings.value("VolumeRendering/RenderingMethod", QString("vtkDMMLGPURayCastVolumeRenderingDisplayNode")).toString();
  QString currentRenderingMethod = displayNode ? QString(displayNode->GetClassName()) : defaultRenderingMethod;
  d->RenderingMethodComboBox->setCurrentIndex(d->RenderingMethodComboBox->findData(currentRenderingMethod) );
  d->MemorySizeComboBox->setCurrentGPUMemory(firstViewNode ? firstViewNode->GetGPUMemorySize() : 0);
  d->QualityControlComboBox->setCurrentIndex(firstViewNode ? firstViewNode->GetVolumeRenderingQuality() : -1);
  d->AutoReleaseGraphicsResourcesCheckBox->setChecked(firstViewNode ? firstViewNode->GetAutoReleaseGraphicsResources() : false);

  if (firstViewNode)
    {
    d->FramerateSliderWidget->setValue(firstViewNode->GetExpectedFPS());
    }
  d->FramerateSliderWidget->setEnabled(
    firstViewNode && firstViewNode->GetVolumeRenderingQuality() == vtkDMMLViewNode::Adaptive );
  // Advanced rendering properties
  if (d->RenderingMethodWidgets[currentRenderingMethod])
    {
    qCjyxVolumeRenderingPropertiesWidget* renderingMethodWidget =
      qobject_cast<qCjyxVolumeRenderingPropertiesWidget*>(d->RenderingMethodWidgets[currentRenderingMethod]);
    renderingMethodWidget->setDMMLNode(displayNode);
    d->RenderingMethodStackedWidget->setCurrentWidget(renderingMethodWidget);
    }
  else
    {
    qWarning() << Q_FUNC_INFO << ": Failed to find rendering properties widget for rendering method " << currentRenderingMethod;
    // Index 0 is an empty widget
    d->RenderingMethodStackedWidget->setCurrentIndex(0);
    }

  // Volume properties tab
  d->SynchronizeScalarDisplayNodeButton->setEnabled(displayNode != nullptr);
  bool follow = displayNode ? displayNode->GetFollowVolumeDisplayNode() != 0 : false;
  if (follow)
    {
    d->SynchronizeScalarDisplayNodeButton->setCheckState(Qt::Checked);
    }
  d->SynchronizeScalarDisplayNodeButton->setChecked(follow);
  d->IgnoreVolumesThresholdCheckBox->setChecked(
    displayNode ? displayNode->GetIgnoreVolumeDisplayNodeThreshold() != 0 : false );
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingModuleWidget::updateWidgetFromROINode()
{
  Q_D(qCjyxVolumeRenderingModuleWidget);
  bool roiVisible = false;
  if (d->CropROINode)
    {
    roiVisible = d->CropROINode->GetDisplayVisibility();
    }
  QSignalBlocker blocker(d->ROICropDisplayCheckBox);
  d->ROICropDisplayCheckBox->setChecked(roiVisible);
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingModuleWidget::onCropToggled(bool crop)
{
  vtkDMMLVolumeRenderingDisplayNode* displayNode = this->dmmlDisplayNode();
  if (!displayNode)
    {
    return;
    }

  if (crop)
    {
    // Create ROI node
    vtkCjyxVolumeRenderingLogic* logic = vtkCjyxVolumeRenderingLogic::SafeDownCast(this->logic());
    if (logic)
      {
      logic->CreateROINode(displayNode);
      }
    else
      {
      qCritical() << Q_FUNC_INFO << ": Failed to access volume rendering logic";
      }
    }

  displayNode->SetCroppingEnabled(crop);
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingModuleWidget::fitROIToVolume()
{
  Q_D(qCjyxVolumeRenderingModuleWidget);
  vtkDMMLVolumeRenderingDisplayNode* displayNode = this->dmmlDisplayNode();
  if (!displayNode)
    {
    return;
    }

  // Fit ROI to volume
  vtkCjyxVolumeRenderingLogic::SafeDownCast(this->logic())->FitROIToVolume(displayNode);

  vtkDMMLMarkupsROINode* markupsROINode = displayNode->GetMarkupsROINode();
  vtkDMMLAnnotationROINode* annotationROINode = displayNode->GetAnnotationROINode();
  if (annotationROINode &&
     (d->AnnotationROIWidget->dmmlROINode() != this->dmmlROINode()
     || d->AnnotationROIWidget->dmmlROINode() != annotationROINode))
    {
    qCritical() << Q_FUNC_INFO << ": ROI node mismatch";
    return;
    }
  if (markupsROINode &&
     (d->MarkupsROIWidget->dmmlROINode() != this->dmmlMarkupsROINode()
     || d->MarkupsROIWidget->dmmlROINode() != markupsROINode))
    {
    qCritical() << Q_FUNC_INFO << ": ROI node mismatch";
    return;
    }

  // Update ROI widget extent
  if (markupsROINode && d->MarkupsROIWidget->dmmlROINode())
    {
    double xyz[3] = { 0.0 };
    double rxyz[3] = { 0.0 };

    d->MarkupsROIWidget->dmmlROINode()->GetXYZ(xyz);
    d->MarkupsROIWidget->dmmlROINode()->GetRadiusXYZ(rxyz);

    double bounds[6] = { 0.0 };
    for (int i = 0; i < 3; ++i)
      {
      bounds[i] = xyz[i] - rxyz[i];
      bounds[3 + i] = xyz[i] + rxyz[i];
      }
    d->MarkupsROIWidget->setExtent(bounds[0], bounds[3],
      bounds[1], bounds[4],
      bounds[2], bounds[5]);
    }
  else if (annotationROINode && d->AnnotationROIWidget->dmmlROINode())
    {
    double xyz[3] = {0.0};
    double rxyz[3] = {0.0};

    d->AnnotationROIWidget->dmmlROINode()->GetXYZ(xyz);
    d->AnnotationROIWidget->dmmlROINode()->GetRadiusXYZ(rxyz);

    double bounds[6] = {0.0};
    for (int i=0; i < 3; ++i)
      {
      bounds[i]   = xyz[i]-rxyz[i];
      bounds[3+i] = xyz[i]+rxyz[i];
      }
    d->AnnotationROIWidget->setExtent(bounds[0], bounds[3],
                            bounds[1], bounds[4],
                            bounds[2], bounds[5]);
    }
}

// --------------------------------------------------------------------------
vtkDMMLVolumePropertyNode* qCjyxVolumeRenderingModuleWidget::dmmlVolumePropertyNode()const
{
  Q_D(const qCjyxVolumeRenderingModuleWidget);
  return vtkDMMLVolumePropertyNode::SafeDownCast(d->VolumePropertyNodeComboBox->currentNode());
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingModuleWidget::setDMMLVolumePropertyNode(vtkDMMLNode* volumePropertyNode)
{
  Q_D(qCjyxVolumeRenderingModuleWidget);
  // Set if not already set
  d->VolumePropertyNodeComboBox->setCurrentNode(volumePropertyNode);
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingModuleWidget::onCurrentDMMLVolumePropertyNodeChanged(vtkDMMLNode* node)
{
  Q_D(qCjyxVolumeRenderingModuleWidget);
  vtkDMMLVolumePropertyNode* volumePropertyNode = vtkDMMLVolumePropertyNode::SafeDownCast(node);

  // Set volume property node to display node
  vtkDMMLVolumeRenderingDisplayNode* displayNode = this->dmmlDisplayNode();
  if (displayNode)
    {
    displayNode->SetAndObserveVolumePropertyNodeID(volumePropertyNode ? volumePropertyNode->GetID() : nullptr);
    }
}

// --------------------------------------------------------------------------
vtkDMMLDisplayableNode* qCjyxVolumeRenderingModuleWidget::dmmlROINode()const
{
  Q_D(const qCjyxVolumeRenderingModuleWidget);
  return vtkDMMLDisplayableNode::SafeDownCast(d->ROINodeComboBox->currentNode());
}

// --------------------------------------------------------------------------
vtkDMMLAnnotationROINode* qCjyxVolumeRenderingModuleWidget::dmmlAnnotationROINode()const
{
  Q_D(const qCjyxVolumeRenderingModuleWidget);
  return vtkDMMLAnnotationROINode::SafeDownCast(d->ROINodeComboBox->currentNode());
}

// --------------------------------------------------------------------------
vtkDMMLMarkupsROINode* qCjyxVolumeRenderingModuleWidget::dmmlMarkupsROINode()const
{
  Q_D(const qCjyxVolumeRenderingModuleWidget);
  return vtkDMMLMarkupsROINode::SafeDownCast(d->ROINodeComboBox->currentNode());
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingModuleWidget::setDMMLROINode(vtkDMMLNode* roiNode)
{
  Q_D(qCjyxVolumeRenderingModuleWidget);
  d->ROINodeComboBox->setCurrentNode(roiNode);
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingModuleWidget::onCurrentDMMLROINodeChanged(vtkDMMLNode* node)
{
  Q_D(qCjyxVolumeRenderingModuleWidget);
  vtkDMMLVolumeRenderingDisplayNode* displayNode = this->dmmlDisplayNode();
  if (displayNode)
    {
    displayNode->SetAndObserveROINodeID(node ? node->GetID() : nullptr);
    }
}

// --------------------------------------------------------------------------
vtkDMMLVolumeRenderingDisplayNode* qCjyxVolumeRenderingModuleWidget::dmmlDisplayNode()const
{
  Q_D(const qCjyxVolumeRenderingModuleWidget);
  vtkDMMLVolumeNode* volumeNode = this->dmmlVolumeNode();
  vtkDMMLVolumeRenderingDisplayNode* displayNode = d->displayNodeForVolumeNode(volumeNode);
  return displayNode;
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingModuleWidget::onCurrentRenderingMethodChanged(int index)
{
  Q_D(qCjyxVolumeRenderingModuleWidget);
  vtkDMMLVolumeRenderingDisplayNode* displayNode = this->dmmlDisplayNode();
  QString renderingClassName = d->RenderingMethodComboBox->itemData(index).toString();
  // Display node is already the right type, don't change anything
  if ( !displayNode || renderingClassName.isEmpty()
    || renderingClassName == displayNode->GetClassName())
    {
    return;
    }

  // Replace display nodes with new display nodes of the type corresponding to the requested method
  vtkCjyxVolumeRenderingLogic* volumeRenderingLogic = vtkCjyxVolumeRenderingLogic::SafeDownCast(this->logic());
  volumeRenderingLogic->ChangeVolumeRenderingMethod(renderingClassName.toUtf8());

  // Perform necessary setup steps for the new display node for the current volume
  this->onCurrentDMMLVolumeNodeChanged(d->VolumeNodeComboBox->currentNode());
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingModuleWidget::onCurrentMemorySizeChanged()
{
  Q_D(qCjyxVolumeRenderingModuleWidget);
  vtkDMMLVolumeRenderingDisplayNode* displayNode = this->dmmlDisplayNode();
  if (!displayNode)
    {
    return;
    }
  int gpuMemorySize = d->MemorySizeComboBox->currentGPUMemoryInMB();

  std::vector<vtkDMMLNode*> viewNodes;
  displayNode->GetScene()->GetNodesByClass("vtkDMMLViewNode", viewNodes);
  for (std::vector<vtkDMMLNode*>::iterator it=viewNodes.begin(); it!=viewNodes.end(); ++it)
    {
    vtkDMMLViewNode* viewNode = vtkDMMLViewNode::SafeDownCast(*it);
    if (displayNode->IsDisplayableInView(viewNode->GetID()))
      {
      viewNode->SetGPUMemorySize(gpuMemorySize);
      }
    }
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingModuleWidget::onCurrentQualityControlChanged(int index)
{
  vtkDMMLVolumeRenderingDisplayNode* displayNode = this->dmmlDisplayNode();
  if (!displayNode)
    {
    return;
    }

  std::vector<vtkDMMLNode*> viewNodes;
  displayNode->GetScene()->GetNodesByClass("vtkDMMLViewNode", viewNodes);
  for (std::vector<vtkDMMLNode*>::iterator it=viewNodes.begin(); it!=viewNodes.end(); ++it)
    {
    vtkDMMLViewNode* viewNode = vtkDMMLViewNode::SafeDownCast(*it);
    if (displayNode->IsDisplayableInView(viewNode->GetID()))
      {
      viewNode->SetVolumeRenderingQuality(index);
      }
    }

  this->updateWidgetFromDMML();
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingModuleWidget::onAutoReleaseGraphicsResourcesCheckBoxToggled(bool autoRelease)
{
  vtkDMMLVolumeRenderingDisplayNode* displayNode = this->dmmlDisplayNode();
  if (!displayNode)
    {
    return;
    }

  std::vector<vtkDMMLNode*> viewNodes;
  displayNode->GetScene()->GetNodesByClass("vtkDMMLViewNode", viewNodes);
  for (std::vector<vtkDMMLNode*>::iterator it=viewNodes.begin(); it!=viewNodes.end(); ++it)
    {
    vtkDMMLViewNode* viewNode = vtkDMMLViewNode::SafeDownCast(*it);
    if (displayNode->IsDisplayableInView(viewNode->GetID()))
      {
      viewNode->SetAutoReleaseGraphicsResources(autoRelease);
      }
    }

  this->updateWidgetFromDMML();
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingModuleWidget::onCurrentFramerateChanged(double fps)
{
  vtkDMMLVolumeRenderingDisplayNode* displayNode = this->dmmlDisplayNode();
  if (!displayNode)
    {
    return;
    }

  std::vector<vtkDMMLNode*> viewNodes;
  displayNode->GetScene()->GetNodesByClass("vtkDMMLViewNode", viewNodes);
  for (std::vector<vtkDMMLNode*>::iterator it=viewNodes.begin(); it!=viewNodes.end(); ++it)
    {
    vtkDMMLViewNode* viewNode = vtkDMMLViewNode::SafeDownCast(*it);
    if (displayNode->IsDisplayableInView(viewNode->GetID()))
      {
      viewNode->SetExpectedFPS(fps);
      }
    }
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingModuleWidget::synchronizeScalarDisplayNode()
{
  vtkDMMLVolumeRenderingDisplayNode* displayNode = this->dmmlDisplayNode();
  if (!displayNode)
    {
    return;
    }
  vtkCjyxVolumeRenderingLogic* volumeRenderingLogic = vtkCjyxVolumeRenderingLogic::SafeDownCast(this->logic());
  volumeRenderingLogic->CopyDisplayToVolumeRenderingDisplayNode(displayNode);
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingModuleWidget::setFollowVolumeDisplayNode(bool follow)
{
  vtkDMMLVolumeRenderingDisplayNode* displayNode = this->dmmlDisplayNode();
  if (!displayNode)
    {
    return;
    }
  displayNode->SetFollowVolumeDisplayNode(follow ? 1 : 0);
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingModuleWidget::setIgnoreVolumesThreshold(bool ignore)
{
  vtkDMMLVolumeRenderingDisplayNode* displayNode = this->dmmlDisplayNode();
  if (!displayNode)
    {
    return;
    }
  displayNode->SetIgnoreVolumeDisplayNodeThreshold(ignore ? 1 : 0);
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingModuleWidget::onThresholdChanged(bool threshold)
{
  vtkCjyxVolumeRenderingLogic* volumeRenderingLogic = vtkCjyxVolumeRenderingLogic::SafeDownCast(this->logic());
  volumeRenderingLogic->SetUseLinearRamp(!threshold);
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingModuleWidget::onROICropDisplayCheckBoxToggled(bool toggle)
{
  Q_D(qCjyxVolumeRenderingModuleWidget);
  vtkDMMLVolumeRenderingDisplayNode* displayNode = this->dmmlDisplayNode();
  if (!displayNode)
    {
    return;
    }

  if (toggle)
    {
    // Create ROI node
    vtkCjyxVolumeRenderingLogic* logic = vtkCjyxVolumeRenderingLogic::SafeDownCast(this->logic());
    if (logic)
      {
      logic->CreateROINode(displayNode);
      }
    else
      {
      qCritical() << Q_FUNC_INFO << ": Failed to access volume rendering logic";
      }
    }


  vtkDMMLDisplayableNode* roiNode = d->MarkupsROIWidget->dmmlROINode();
  if (!roiNode)
    {
    roiNode = d->AnnotationROIWidget->dmmlROINode();
    }
  if (!roiNode)
    {
    return;
    }

  int numberOfDisplayNodes = roiNode->GetNumberOfDisplayNodes();

  std::vector<int> wasModifying(numberOfDisplayNodes);

  for(int index = 0; index < numberOfDisplayNodes; index++)
    {
    vtkDMMLNode* node = roiNode->GetNthDisplayNode(index);
    if (!node)
      {
      continue;
      }
    wasModifying[index] = node->StartModify();
    }

  roiNode->SetDisplayVisibility(toggle);

  // When the display box is visible, it activated the cropping
  // (to follow the "what you see is what you get" pattern).
  if (toggle)
    {
    displayNode->SetCroppingEnabled(toggle);
    }

  for(int index = 0; index < numberOfDisplayNodes; index++)
    {
    vtkDMMLNode* node = roiNode->GetNthDisplayNode(index);
    if (!node)
      {
      continue;
      }
    node->EndModify(wasModifying[index]);
    }
}

//-----------------------------------------------------------
bool qCjyxVolumeRenderingModuleWidget::setEditedNode(vtkDMMLNode* node,
                                                       QString role /* = QString()*/,
                                                       QString context /* = QString()*/)
{
  Q_D(qCjyxVolumeRenderingModuleWidget);
  Q_UNUSED(role);
  Q_UNUSED(context);

  if (vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(node))
    {
    vtkDMMLVolumeRenderingDisplayNode* displayNode = vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(node);

    vtkDMMLVolumeNode* displayableNode = vtkDMMLVolumeNode::SafeDownCast(displayNode->GetDisplayableNode());
    if (!displayableNode)
      {
      return false;
      }
    d->VolumeNodeComboBox->setCurrentNode(displayableNode);
    return true;
    }

  if (vtkDMMLVolumePropertyNode::SafeDownCast(node))
    {
    // Find first volume rendering display node corresponding to this property node
    vtkDMMLScene* scene = this->dmmlScene();
    if (!scene)
      {
      return false;
      }
    vtkDMMLVolumeRenderingDisplayNode* displayNode = nullptr;
    vtkObject* itNode = nullptr;
    vtkCollectionSimpleIterator it;
    for (scene->GetNodes()->InitTraversal(it); (itNode = scene->GetNodes()->GetNextItemAsObject(it));)
      {
      displayNode = vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(itNode);
      if (!displayNode)
        {
        continue;
        }
      if (displayNode->GetVolumePropertyNode() != node)
        {
        continue;
        }
      vtkDMMLVolumeNode* displayableNode = vtkDMMLVolumeNode::SafeDownCast(displayNode->GetDisplayableNode());
      if (!displayableNode)
        {
        return false;
        }
      d->VolumeNodeComboBox->setCurrentNode(displayableNode);
      return true;
      }
    }

  if (vtkDMMLMarkupsROINode::SafeDownCast(node))
    {
    vtkCjyxVolumeRenderingLogic* volumeRenderingLogic = vtkCjyxVolumeRenderingLogic::SafeDownCast(this->logic());
    if (!volumeRenderingLogic)
      {
      qWarning() << Q_FUNC_INFO << "failed: invalid logic";
      return false;
      }
    vtkDMMLVolumeRenderingDisplayNode* displayNode = volumeRenderingLogic->GetFirstVolumeRenderingDisplayNodeByROINode(
      vtkDMMLMarkupsROINode::SafeDownCast(node));
    if (!displayNode)
      {
      return false;
      }
    vtkDMMLVolumeNode* displayableNode = vtkDMMLVolumeNode::SafeDownCast(displayNode->GetDisplayableNode());
    if (!displayableNode)
      {
      return false;
      }
    d->VolumeNodeComboBox->setCurrentNode(displayableNode);
    return true;
    }

  return false;
}

//-----------------------------------------------------------
double qCjyxVolumeRenderingModuleWidget::nodeEditable(vtkDMMLNode* node)
{
  if (vtkDMMLVolumePropertyNode::SafeDownCast(node)
    || vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(node))
    {
    return 0.5;
    }
  else if (vtkDMMLMarkupsROINode::SafeDownCast(node))
    {
    vtkCjyxVolumeRenderingLogic* volumeRenderingLogic = vtkCjyxVolumeRenderingLogic::SafeDownCast(this->logic());
    if (!volumeRenderingLogic)
      {
      qWarning() << Q_FUNC_INFO << " failed: Invalid logic";
      return 0.0;
      }
    if (volumeRenderingLogic->GetFirstVolumeRenderingDisplayNodeByROINode(vtkDMMLMarkupsROINode::SafeDownCast(node)))
      {
      // This ROI node is a clipping ROI for volume rendering - claim it with higher confidence than the generic 0.5
      return 0.6;
      }
    return 0.0;
    }
  else
    {
    return 0.0;
    }
}

//-----------------------------------------------------------
void qCjyxVolumeRenderingModuleWidget::onChartsExtentChanged()
{
  vtkDMMLVolumePropertyNode* volumePropertyNode = this->dmmlVolumePropertyNode();
  if (!volumePropertyNode)
    {
    return;
    }

  Q_D(qCjyxVolumeRenderingModuleWidget);
  double effectiveRange[4] = { 0.0 };
  d->VolumePropertyNodeWidget->chartsExtent(effectiveRange);

  int wasDisabled = volumePropertyNode->GetDisableModifiedEvent();
  volumePropertyNode->DisableModifiedEventOn();
  volumePropertyNode->SetEffectiveRange(effectiveRange[0], effectiveRange[1]);
  volumePropertyNode->SetDisableModifiedEvent(wasDisabled);

  // Update presets slider range
  d->PresetComboBox->updatePresetSliderRange();
}

//-----------------------------------------------------------
void qCjyxVolumeRenderingModuleWidget::onEffectiveRangeModified()
{
  Q_D(qCjyxVolumeRenderingModuleWidget);

  vtkDMMLVolumePropertyNode* volumePropertyNode = this->dmmlVolumePropertyNode();
  if (!volumePropertyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid volume property node";
    return;
    }

  // Set charts extent to effective range defined in volume property node
  double effectiveRange[2] = {0.0};
  volumePropertyNode->GetEffectiveRange(effectiveRange);
  if (effectiveRange[0] > effectiveRange[1])
    {
    if (!volumePropertyNode->CalculateEffectiveRange())
      {
      return; // Do not set undefined effective range
      }
    volumePropertyNode->GetEffectiveRange(effectiveRange);
    }
  bool wasBlocking = d->VolumePropertyNodeWidget->blockSignals(true);
  d->VolumePropertyNodeWidget->setChartsExtent(effectiveRange[0], effectiveRange[1]);
  d->VolumePropertyNodeWidget->blockSignals(wasBlocking);

  // Update presets slider range
  d->PresetComboBox->updatePresetSliderRange();
}
