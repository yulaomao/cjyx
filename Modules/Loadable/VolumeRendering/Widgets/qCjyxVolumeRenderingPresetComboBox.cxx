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
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

// qCjyxVolumeRendering includes
#include "qCjyxVolumeRenderingPresetComboBox.h"
#include "ui_qCjyxVolumeRenderingPresetComboBox.h"
#include "qCjyxPresetComboBox.h"

// Qt includes
#include <QDebug>

// CTK includes
#include <ctkUtils.h>

// DMMLWidgets includes
#include <qDMMLSceneModel.h>

// DMML includes
#include <vtkDMMLScalarVolumeNode.h>
#include <vtkDMMLVolumePropertyNode.h>

// VTK includes
#include <vtkWeakPointer.h>
#include <vtkImageData.h>
#include <vtkColorTransferFunction.h>
#include <vtkVolumeProperty.h>

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_VolumeRendering
class qCjyxVolumeRenderingPresetComboBoxPrivate : public Ui_qCjyxVolumeRenderingPresetComboBox
{
  Q_DECLARE_PUBLIC(qCjyxVolumeRenderingPresetComboBox);
protected:
  qCjyxVolumeRenderingPresetComboBox* const q_ptr;

public:
  qCjyxVolumeRenderingPresetComboBoxPrivate(qCjyxVolumeRenderingPresetComboBox& object);
  virtual ~qCjyxVolumeRenderingPresetComboBoxPrivate();

  void init();
  void populatePresetsIcons();

  double OldPresetPosition;

  /// Volume property node controlling volume rendering transfer functions.
  /// Its content mirrors the currently selected preset node in the combobox.
  vtkWeakPointer<vtkDMMLVolumePropertyNode> VolumePropertyNode;
};

//-----------------------------------------------------------------------------
// qCjyxVolumeRenderingPresetComboBoxPrivate methods

//-----------------------------------------------------------------------------
qCjyxVolumeRenderingPresetComboBoxPrivate::qCjyxVolumeRenderingPresetComboBoxPrivate(
  qCjyxVolumeRenderingPresetComboBox& object)
  : q_ptr(&object)
  , OldPresetPosition(0.0)
  , VolumePropertyNode(nullptr)
{
}

//-----------------------------------------------------------------------------
qCjyxVolumeRenderingPresetComboBoxPrivate::~qCjyxVolumeRenderingPresetComboBoxPrivate() = default;

//-----------------------------------------------------------------------------
void qCjyxVolumeRenderingPresetComboBoxPrivate::init()
{
  Q_Q(qCjyxVolumeRenderingPresetComboBox);

  this->Ui_qCjyxVolumeRenderingPresetComboBox::setupUi(q);

  QObject::connect(this->PresetComboBox, SIGNAL(nodeActivated(vtkDMMLNode*)), q, SLOT(applyPreset(vtkDMMLNode*)));
  QObject::connect(this->PresetComboBox, SIGNAL(currentNodeChanged(vtkDMMLNode*)), q, SIGNAL(currentNodeChanged(vtkDMMLNode*)));
  QObject::connect(this->PresetComboBox, SIGNAL(currentNodeIDChanged(QString)), q, SIGNAL(currentNodeIDChanged(QString)));

  QObject::connect(this->PresetOffsetSlider, SIGNAL(valueChanged(double)),
    q, SLOT(offsetPreset(double)));
  QObject::connect(this->PresetOffsetSlider, SIGNAL(sliderPressed()),
    q, SLOT(startInteraction()));
  QObject::connect(this->PresetOffsetSlider, SIGNAL(valueChanged(double)),
    q, SLOT(interaction()));
  QObject::connect(this->PresetOffsetSlider, SIGNAL(sliderReleased()),
    q, SLOT(endInteraction()));

  this->PresetComboBox->setDMMLScene(nullptr);
  this->PresetComboBox->setCurrentNode(nullptr);
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingPresetComboBoxPrivate::populatePresetsIcons()
{
  Q_Q(qCjyxVolumeRenderingPresetComboBox);

  // This is a hack and doesn't work yet
  for (int i = 0; i < this->PresetComboBox->nodeCount(); ++i)
    {
    vtkDMMLNode* presetNode = this->PresetComboBox->nodeFromIndex(i);
    QIcon presetIcon(QString(":/presets/") + presetNode->GetName());
    if (!presetIcon.isNull())
      {
      qDMMLSceneModel* sceneModel = qobject_cast<qDMMLSceneModel*>(
        this->PresetComboBox->sortFilterProxyModel()->sourceModel() );
      sceneModel->setData(sceneModel->indexFromNode(presetNode), presetIcon, Qt::DecorationRole);
      }
    }
}


//-----------------------------------------------------------------------------
// qCjyxVolumeRenderingPresetComboBox methods

// --------------------------------------------------------------------------
qCjyxVolumeRenderingPresetComboBox::qCjyxVolumeRenderingPresetComboBox(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qCjyxVolumeRenderingPresetComboBoxPrivate(*this))
{
  Q_D(qCjyxVolumeRenderingPresetComboBox);
  d->init();
}

// --------------------------------------------------------------------------
qCjyxVolumeRenderingPresetComboBox::~qCjyxVolumeRenderingPresetComboBox() = default;

// --------------------------------------------------------------------------
vtkDMMLNode* qCjyxVolumeRenderingPresetComboBox::currentNode()const
{
  Q_D(const qCjyxVolumeRenderingPresetComboBox);
  return d->PresetComboBox->currentNode();
}

// --------------------------------------------------------------------------
QString qCjyxVolumeRenderingPresetComboBox::currentNodeID()const
{
  Q_D(const qCjyxVolumeRenderingPresetComboBox);
  return d->PresetComboBox->currentNodeID();
}

// --------------------------------------------------------------------------
vtkDMMLVolumePropertyNode* qCjyxVolumeRenderingPresetComboBox::dmmlVolumePropertyNode()const
{
  Q_D(const qCjyxVolumeRenderingPresetComboBox);
  return d->VolumePropertyNode;
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingPresetComboBox::setCurrentNode(vtkDMMLNode* node)
{
  Q_D(qCjyxVolumeRenderingPresetComboBox);
  d->PresetComboBox->setCurrentNode(node);
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingPresetComboBox::setCurrentNodeID(const QString& nodeID)
{
  Q_D(qCjyxVolumeRenderingPresetComboBox);
  d->PresetComboBox->setCurrentNodeID(nodeID);
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingPresetComboBox::setDMMLVolumePropertyNode(vtkDMMLNode* node)
{
  Q_D(qCjyxVolumeRenderingPresetComboBox);
  d->VolumePropertyNode = vtkDMMLVolumePropertyNode::SafeDownCast(node);

  this->resetOffset();
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingPresetComboBox::startInteraction()
{
  Q_D(qCjyxVolumeRenderingPresetComboBox);
  if (!d->VolumePropertyNode)
    {
    return;
    }

  vtkVolumeProperty* volumeProperty = d->VolumePropertyNode->GetVolumeProperty();
  if (volumeProperty)
    {
    volumeProperty->InvokeEvent(vtkCommand::StartInteractionEvent);
    }
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingPresetComboBox::endInteraction()
{
  Q_D(qCjyxVolumeRenderingPresetComboBox);
  if (!d->VolumePropertyNode)
    {
    return;
    }

  vtkVolumeProperty* volumeProperty = d->VolumePropertyNode->GetVolumeProperty();
  if (volumeProperty)
    {
    volumeProperty->InvokeEvent(vtkCommand::EndInteractionEvent);
    }
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingPresetComboBox::interaction()
{
  Q_D(qCjyxVolumeRenderingPresetComboBox);
  if (!d->VolumePropertyNode)
    {
    return;
    }

  vtkVolumeProperty* volumeProperty = d->VolumePropertyNode->GetVolumeProperty();
  if (volumeProperty)
    {
    volumeProperty->InvokeEvent(vtkCommand::InteractionEvent);
    }
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingPresetComboBox::offsetPreset(double newPosition)
{
  Q_D(qCjyxVolumeRenderingPresetComboBox);
  if (!d->VolumePropertyNode)
    {
    return;
    }

  emit presetOffsetChanged(newPosition - d->OldPresetPosition, 0., false);
  d->OldPresetPosition = newPosition;
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingPresetComboBox::resetOffset()
{
  Q_D(qCjyxVolumeRenderingPresetComboBox);

  // Reset the slider position to the center.
  d->OldPresetPosition = 0.0;
  d->PresetOffsetSlider->setValue(0.0);
  this->updatePresetSliderRange();
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingPresetComboBox::updatePresetSliderRange()
{
  Q_D(qCjyxVolumeRenderingPresetComboBox);
  if (!d->VolumePropertyNode)
    {
    return;
    }

  if (!d->VolumePropertyNode->GetVolumeProperty())
    {
    return;
    }

  if (d->PresetOffsetSlider->slider()->isSliderDown())
    {
    // Do not change slider range while moving the slider
    return;
    }

  double effectiveRange[2] = { 0.0 };
  d->VolumePropertyNode->GetEffectiveRange(effectiveRange);
  if (effectiveRange[0] > effectiveRange[1])
    {
    if (!d->VolumePropertyNode->CalculateEffectiveRange())
      {
      return; // Do not use undefined effective range
      }
    d->VolumePropertyNode->GetEffectiveRange(effectiveRange);
    }
  double transferFunctionWidth = effectiveRange[1] - effectiveRange[0];

  bool wasBlocking = d->PresetOffsetSlider->blockSignals(true);
  d->PresetOffsetSlider->setRange(-transferFunctionWidth, transferFunctionWidth);
  d->PresetOffsetSlider->setSingleStep(ctk::closestPowerOfTen(transferFunctionWidth)/500.0);
  d->PresetOffsetSlider->setPageStep(d->PresetOffsetSlider->singleStep());
  d->PresetOffsetSlider->blockSignals(wasBlocking);
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingPresetComboBox::applyPreset(vtkDMMLNode* node)
{
  Q_D(qCjyxVolumeRenderingPresetComboBox);

  if (this->signalsBlocked())
    {
    // Prevent the preset node from overwriting the active volume property node (thus reverting
    // changes in the transfer functions) when the widget's signals are blocked.
    // Needed to handle here, because if the inner combobox's signals are blocked, then the icon
    // is not updated.
    return;
    }

  vtkDMMLVolumePropertyNode* presetNode = vtkDMMLVolumePropertyNode::SafeDownCast(node);
  if (!presetNode || !d->VolumePropertyNode)
    {
    return;
    }

  if ( !presetNode->GetVolumeProperty()
    || !presetNode->GetVolumeProperty()->GetRGBTransferFunction()
    || presetNode->GetVolumeProperty()->GetRGBTransferFunction()->GetRange()[0] >
       presetNode->GetVolumeProperty()->GetRGBTransferFunction()->GetRange()[1] )
    {
    qCritical() << Q_FUNC_INFO << ": Invalid volume property preset node";
    return;
    }

  d->VolumePropertyNode->Copy(presetNode);

  this->resetOffset();
}

// --------------------------------------------------------------------------
bool qCjyxVolumeRenderingPresetComboBox::showIcons()const
{
  Q_D(const qCjyxVolumeRenderingPresetComboBox);
  return d->PresetComboBox->showIcons();
}

// --------------------------------------------------------------------------
void qCjyxVolumeRenderingPresetComboBox::setShowIcons(bool show)
{
  Q_D(qCjyxVolumeRenderingPresetComboBox);
  d->PresetComboBox->setShowIcons(show);
}
