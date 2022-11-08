// Qt includes
#include <QVBoxLayout>

// Cjyx includes
#include "qCjyxDiffusionTensorVolumeDisplayWidget.h"
#include "qCjyxDiffusionWeightedVolumeDisplayWidget.h"
#include "qCjyxLabelMapVolumeDisplayWidget.h"
#include "qCjyxScalarVolumeDisplayWidget.h"
#include "qCjyxVolumeDisplayWidget.h"

// DMML includes
#include <vtkDMMLDiffusionTensorVolumeNode.h>
#include <vtkDMMLDiffusionWeightedVolumeNode.h>
#include <vtkDMMLLabelMapVolumeNode.h>

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_Volumes
class qCjyxVolumeDisplayWidgetPrivate
{
  Q_DECLARE_PUBLIC(qCjyxVolumeDisplayWidget);

protected:
  qCjyxVolumeDisplayWidget* const q_ptr;

public:
  qCjyxVolumeDisplayWidgetPrivate(qCjyxVolumeDisplayWidget& object);
  void init();
  qCjyxWidget* widgetForVolume(vtkDMMLNode* volumeNode);
  void setVolumeInWidget(qCjyxWidget* displayWidget, vtkDMMLVolumeNode* volumeNode);
  vtkDMMLVolumeNode* volumeInWidget(qCjyxWidget* displayWidget);

  // show the selected widget, hide all others
  void setCurrentDisplayWidget(qCjyxWidget* displayWidget);

  qCjyxWidget* CurrentWidget{ nullptr };

  qCjyxScalarVolumeDisplayWidget*            ScalarVolumeDisplayWidget{ nullptr };
  qCjyxLabelMapVolumeDisplayWidget*          LabelMapVolumeDisplayWidget{ nullptr };
  qCjyxDiffusionWeightedVolumeDisplayWidget* DWVolumeDisplayWidget{ nullptr };
  qCjyxDiffusionTensorVolumeDisplayWidget*   DTVolumeDisplayWidget{ nullptr };
};

// --------------------------------------------------------------------------
qCjyxVolumeDisplayWidgetPrivate::qCjyxVolumeDisplayWidgetPrivate(
  qCjyxVolumeDisplayWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qCjyxVolumeDisplayWidgetPrivate::init()
{
  Q_Q(qCjyxVolumeDisplayWidget);

  QVBoxLayout* layout = new QVBoxLayout(q);
  layout->setSpacing(4);
  layout->setContentsMargins(0, 0, 0, 0);

  this->ScalarVolumeDisplayWidget = new qCjyxScalarVolumeDisplayWidget(q);
  this->ScalarVolumeDisplayWidget->hide();
  layout->addWidget(this->ScalarVolumeDisplayWidget);

  this->LabelMapVolumeDisplayWidget = new qCjyxLabelMapVolumeDisplayWidget(q);
  this->LabelMapVolumeDisplayWidget->hide();
  layout->addWidget(this->LabelMapVolumeDisplayWidget);

  this->DWVolumeDisplayWidget = new qCjyxDiffusionWeightedVolumeDisplayWidget(q);
  this->DWVolumeDisplayWidget->hide();
  layout->addWidget(this->DWVolumeDisplayWidget);

  this->DTVolumeDisplayWidget = new qCjyxDiffusionTensorVolumeDisplayWidget(q);
  this->DTVolumeDisplayWidget->hide();
  layout->addWidget(this->DTVolumeDisplayWidget);
}

// --------------------------------------------------------------------------
void qCjyxVolumeDisplayWidgetPrivate::setCurrentDisplayWidget(qCjyxWidget* displayWidget)
{
  if (this->CurrentWidget == displayWidget)
    {
    return;
    }
  if (this->CurrentWidget)
    {
    this->CurrentWidget->hide();
    }
  this->CurrentWidget = displayWidget;
  if (this->CurrentWidget)
    {
    this->CurrentWidget->show();
    }
}

// --------------------------------------------------------------------------
qCjyxWidget* qCjyxVolumeDisplayWidgetPrivate::widgetForVolume(vtkDMMLNode* volumeNode)
{
  // We must check first the most specific volume type and if there is no match
  // then try scalar volume.
  if (vtkDMMLDiffusionTensorVolumeNode::SafeDownCast(volumeNode))
    {
    return this->DTVolumeDisplayWidget;
    }
  else if (vtkDMMLDiffusionWeightedVolumeNode::SafeDownCast(volumeNode))
    {
    return this->DWVolumeDisplayWidget;
    }
  else if (vtkDMMLLabelMapVolumeNode::SafeDownCast(volumeNode))
    {
    return this->LabelMapVolumeDisplayWidget;
    }
  else if (vtkDMMLScalarVolumeNode::SafeDownCast(volumeNode))
    {
    return this->ScalarVolumeDisplayWidget;
    }
  return nullptr;
}

// --------------------------------------------------------------------------
void qCjyxVolumeDisplayWidgetPrivate::setVolumeInWidget(qCjyxWidget* displayWidget, vtkDMMLVolumeNode* volumeNode)
{
  Q_Q(qCjyxVolumeDisplayWidget);
  if (!displayWidget)
    {
    return;
    }
  vtkDMMLScene* scene = volumeNode ? volumeNode->GetScene() : nullptr;
  // We must remove the node "before" the setting the scene to nullptr.
  // Because removing the scene could modify the observed node (e.g setting
  // the scene to 0 on a colortable combobox will set the color node of the
  // observed node to 0.
  if (scene && displayWidget->dmmlScene() != scene)
    {
    // set non-null scene
    displayWidget->setDMMLScene(scene);
    }
  if (displayWidget == this->ScalarVolumeDisplayWidget)
    {
    this->ScalarVolumeDisplayWidget->setDMMLVolumeNode(volumeNode);
    }
  if (displayWidget == this->LabelMapVolumeDisplayWidget)
    {
    this->LabelMapVolumeDisplayWidget->setDMMLVolumeNode(volumeNode);
    }
  if (displayWidget == this->DWVolumeDisplayWidget)
    {
    this->DWVolumeDisplayWidget->setDMMLVolumeNode(volumeNode);
    }
  if (displayWidget == this->DTVolumeDisplayWidget)
    {
    this->DTVolumeDisplayWidget->setDMMLVolumeNode(volumeNode);
    }
  if (!scene && displayWidget->dmmlScene() != scene)
    {
    // remove scene after the node has been removed
    displayWidget->setDMMLScene(scene);
    }
}

// --------------------------------------------------------------------------
vtkDMMLVolumeNode* qCjyxVolumeDisplayWidgetPrivate::volumeInWidget(qCjyxWidget* displayWidget)
{
  Q_Q(qCjyxVolumeDisplayWidget);
  if (displayWidget == this->ScalarVolumeDisplayWidget)
    {
    return this->ScalarVolumeDisplayWidget->volumeNode();
    }
  if (displayWidget == this->LabelMapVolumeDisplayWidget)
    {
    return this->LabelMapVolumeDisplayWidget->volumeNode();
    }
  if (displayWidget == this->DWVolumeDisplayWidget)
    {
    return this->DWVolumeDisplayWidget->volumeNode();
    }
  if (displayWidget == this->DTVolumeDisplayWidget)
    {
    return this->DTVolumeDisplayWidget->volumeNode();
    }
  return nullptr;
}

// --------------------------------------------------------------------------
// qCjyxVolumeDisplayWidget
// --------------------------------------------------------------------------
qCjyxVolumeDisplayWidget::qCjyxVolumeDisplayWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qCjyxVolumeDisplayWidgetPrivate(*this))
{
  Q_D(qCjyxVolumeDisplayWidget);
  d->init();
}

// --------------------------------------------------------------------------
qCjyxVolumeDisplayWidget::~qCjyxVolumeDisplayWidget() = default;

// --------------------------------------------------------------------------
void qCjyxVolumeDisplayWidget::setDMMLVolumeNode(vtkDMMLNode* aVolumeNode)
{
  Q_D(qCjyxVolumeDisplayWidget);
  vtkDMMLVolumeNode* volumeNode = vtkDMMLVolumeNode::SafeDownCast(aVolumeNode);
  qCjyxWidget* newWidget = d->widgetForVolume(volumeNode);

  if (newWidget == d->CurrentWidget)
    {
    d->setVolumeInWidget(d->CurrentWidget, volumeNode);
    return;
    }

  if (d->CurrentWidget)
    {
    d->setVolumeInWidget(d->CurrentWidget, nullptr);
    }
  d->setVolumeInWidget(newWidget, volumeNode);
  d->setCurrentDisplayWidget(newWidget);
}

// --------------------------------------------------------------------------
void qCjyxVolumeDisplayWidget::updateFromDMML(vtkObject* volume)
{
  vtkDMMLVolumeNode* volumeNode = vtkDMMLVolumeNode::SafeDownCast(volume);
  this->setDMMLVolumeNode(volumeNode);
}
