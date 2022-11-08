#include "qCjyxScalarVolumeDisplayWidget.h"
#include "ui_qCjyxScalarVolumeDisplayWidget.h"

// Qt includes
#include <QToolButton>

// CTK includes
#include <ctkVTKColorTransferFunction.h>
#include <ctkTransferFunctionGradientItem.h>
#include <ctkTransferFunctionScene.h>
#include <ctkTransferFunctionBarsItem.h>
#include <ctkVTKHistogram.h>

// DMML includes
#include "vtkDMMLColorNode.h"
#include "vtkDMMLScalarVolumeDisplayNode.h"
#include "vtkDMMLScalarVolumeNode.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkColorTransferFunction.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>

// Qt includes
#include <QDebug>

// Cjyx includes
#include "qCjyxAbstractCoreModule.h"
#include "qCjyxApplication.h"
#include "qCjyxModuleManager.h"
#include "vtkCjyxVolumesLogic.h"

// STD includes
#include <limits>

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_Volumes
class qCjyxScalarVolumeDisplayWidgetPrivate
  : public Ui_qCjyxScalarVolumeDisplayWidget
{
  Q_DECLARE_PUBLIC(qCjyxScalarVolumeDisplayWidget);
protected:
  qCjyxScalarVolumeDisplayWidget* const q_ptr;
public:
  qCjyxScalarVolumeDisplayWidgetPrivate(qCjyxScalarVolumeDisplayWidget& object);
  ~qCjyxScalarVolumeDisplayWidgetPrivate();
  void init();

  ctkVTKHistogram* Histogram;
  vtkSmartPointer<vtkColorTransferFunction> ColorTransferFunction;
};

//-----------------------------------------------------------------------------
qCjyxScalarVolumeDisplayWidgetPrivate::qCjyxScalarVolumeDisplayWidgetPrivate(
  qCjyxScalarVolumeDisplayWidget& object)
  : q_ptr(&object)
{
  this->Histogram = new ctkVTKHistogram();
  this->ColorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
}

//-----------------------------------------------------------------------------
qCjyxScalarVolumeDisplayWidgetPrivate::~qCjyxScalarVolumeDisplayWidgetPrivate()
{
  delete this->Histogram;
}

//-----------------------------------------------------------------------------
void qCjyxScalarVolumeDisplayWidgetPrivate::init()
{
  Q_Q(qCjyxScalarVolumeDisplayWidget);

  this->setupUi(q);

  ctkTransferFunctionScene* scene = qobject_cast<ctkTransferFunctionScene*>(
    this->TransferFunctionView->scene());
  // Transfer Function
  ctkVTKColorTransferFunction* transferFunction =
    new ctkVTKColorTransferFunction(this->ColorTransferFunction, q);

  ctkTransferFunctionGradientItem* gradientItem =
    new ctkTransferFunctionGradientItem(transferFunction);
  scene->addItem(gradientItem);
  // Histogram
  //scene->setTransferFunction(this->Histogram);
  ctkTransferFunctionBarsItem* barsItem =
    new ctkTransferFunctionBarsItem(this->Histogram);
  barsItem->setBarWidth(1.);
  scene->addItem(barsItem);

  // Add mapping from presets defined in the Volumes module logic (VolumeDisplayPresets.json)

    // read volume preset names from volumes logic
  vtkCjyxVolumesLogic* volumesModuleLogic = (qCjyxCoreApplication::application() ? vtkCjyxVolumesLogic::SafeDownCast(
    qCjyxCoreApplication::application()->moduleLogic("Volumes")) : nullptr);
  if (volumesModuleLogic)
  {
    QLayout* volumeDisplayPresetsLayout = this->PresetsWidget->layout();
    if (!volumeDisplayPresetsLayout)
      {
      volumeDisplayPresetsLayout = new QHBoxLayout;
      this->PresetsWidget->setLayout(volumeDisplayPresetsLayout);
      }
    std::vector<std::string> presetIds = volumesModuleLogic->GetVolumeDisplayPresetIDs();
    for (const auto& presetId : presetIds)
      {
      vtkCjyxVolumesLogic::VolumeDisplayPreset preset = volumesModuleLogic->GetVolumeDisplayPreset(presetId);
      QString presetIdStr = QString::fromStdString(presetId);
      QString presetName = q->tr(preset.name.c_str());
      QToolButton* presetButton = new QToolButton();
      presetButton->setObjectName(presetIdStr);
      presetButton->setToolTip(q->tr(preset.name.c_str()) + "\n" + q->tr(preset.description.c_str()));
      if (!preset.icon.empty())
        {
        presetButton->setIcon(QIcon(QString::fromStdString(preset.icon)));
        presetButton->setIconSize(QSize(45, 45));
        }
      volumeDisplayPresetsLayout->addWidget(presetButton);
      QObject::connect(presetButton, SIGNAL(clicked()),
        q, SLOT(onPresetButtonClicked()));
      }
    }
  else
    {
    qWarning() << Q_FUNC_INFO << " failed: Module logic 'Volumes' not found. No volume display presets will be added.";
    return;
    }

  QObject::connect(this->InterpolateCheckbox, SIGNAL(toggled(bool)),
                   q, SLOT(setInterpolate(bool)));
  QObject::connect(this->ColorTableComboBox, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
                   q, SLOT(setColorNode(vtkDMMLNode*)));
  QObject::connect(this->LockWindowLevelButton, SIGNAL(clicked()),
                   q, SLOT(onLockWindowLevelButtonClicked()));
  QObject::connect(this->HistogramGroupBox, SIGNAL(toggled(bool)),
                   q, SLOT(onHistogramSectionExpanded(bool)));
}

// --------------------------------------------------------------------------
qCjyxScalarVolumeDisplayWidget::qCjyxScalarVolumeDisplayWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qCjyxScalarVolumeDisplayWidgetPrivate(*this))
{
  Q_D(qCjyxScalarVolumeDisplayWidget);
  d->init();

  // disable as there is not DMML Node associated with the widget
  this->setEnabled(false);
}

// --------------------------------------------------------------------------
qCjyxScalarVolumeDisplayWidget::~qCjyxScalarVolumeDisplayWidget() = default;

// --------------------------------------------------------------------------
vtkDMMLScalarVolumeNode* qCjyxScalarVolumeDisplayWidget::volumeNode()const
{
  Q_D(const qCjyxScalarVolumeDisplayWidget);
  return vtkDMMLScalarVolumeNode::SafeDownCast(
    d->DMMLWindowLevelWidget->dmmlVolumeNode());
}

// --------------------------------------------------------------------------
bool qCjyxScalarVolumeDisplayWidget::isColorTableComboBoxEnabled()const
{
  Q_D(const qCjyxScalarVolumeDisplayWidget);
  return d->ColorTableComboBox->isEnabled();
}

// --------------------------------------------------------------------------
void qCjyxScalarVolumeDisplayWidget::setColorTableComboBoxEnabled(bool enable)
{
  Q_D(qCjyxScalarVolumeDisplayWidget);
  d->ColorTableComboBox->setEnabled(enable);
}

// --------------------------------------------------------------------------
bool qCjyxScalarVolumeDisplayWidget::isDMMLWindowLevelWidgetEnabled()const
{
  Q_D(const qCjyxScalarVolumeDisplayWidget);
  return d->DMMLWindowLevelWidget->isEnabled();
}

// --------------------------------------------------------------------------
void qCjyxScalarVolumeDisplayWidget::setDMMLWindowLevelWidgetEnabled(bool enable)
{
  Q_D(qCjyxScalarVolumeDisplayWidget);
  d->DMMLWindowLevelWidget->setEnabled(enable);
}

// --------------------------------------------------------------------------
vtkDMMLScalarVolumeDisplayNode* qCjyxScalarVolumeDisplayWidget::volumeDisplayNode()const
{
  vtkDMMLVolumeNode* volumeNode = this->volumeNode();
  return volumeNode ? vtkDMMLScalarVolumeDisplayNode::SafeDownCast(
    volumeNode->GetVolumeDisplayNode()) : nullptr;
}

// --------------------------------------------------------------------------
void qCjyxScalarVolumeDisplayWidget::setDMMLVolumeNode(vtkDMMLNode* node)
{
  this->setDMMLVolumeNode(vtkDMMLScalarVolumeNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qCjyxScalarVolumeDisplayWidget::setDMMLVolumeNode(vtkDMMLScalarVolumeNode* volumeNode)
{
  Q_D(qCjyxScalarVolumeDisplayWidget);

  vtkDMMLScalarVolumeDisplayNode* oldVolumeDisplayNode = this->volumeDisplayNode();

  qvtkReconnect(oldVolumeDisplayNode, volumeNode ? volumeNode->GetVolumeDisplayNode() :nullptr,
                vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromDMML()));

  d->DMMLWindowLevelWidget->setDMMLVolumeNode(volumeNode);
  d->DMMLVolumeThresholdWidget->setDMMLVolumeNode(volumeNode);

  this->setEnabled(volumeNode != nullptr);

  this->updateWidgetFromDMML();
}

// --------------------------------------------------------------------------
void qCjyxScalarVolumeDisplayWidget::updateWidgetFromDMML()
{
  Q_D(qCjyxScalarVolumeDisplayWidget);
  vtkDMMLScalarVolumeDisplayNode* displayNode =
    this->volumeDisplayNode();
  if (displayNode)
    {
    d->ColorTableComboBox->setCurrentNode(displayNode->GetColorNode());
    d->InterpolateCheckbox->setChecked(displayNode->GetInterpolate());
    bool lockedWindowLevel = displayNode->GetWindowLevelLocked();
    d->LockWindowLevelButton->setChecked(lockedWindowLevel);
    if (lockedWindowLevel)
      {
      d->LockWindowLevelButton->setIcon(QIcon(":Icons/Medium/CjyxLock.png"));
      d->LockWindowLevelButton->setToolTip(QString("Click to enable modification of Window/Level values"));
      }
    else
      {
      d->LockWindowLevelButton->setIcon(QIcon(":Icons/Medium/CjyxUnlock.png"));
      d->LockWindowLevelButton->setToolTip(QString("Click to prevent modification of Window/Level values"));
      }
    d->PresetsWidget->setEnabled(!lockedWindowLevel);
    d->DMMLWindowLevelWidget->setEnabled(!lockedWindowLevel);
    }
  this->updateHistogram();
}

//----------------------------------------------------------------------------
void qCjyxScalarVolumeDisplayWidget::updateHistogram()
{
  Q_D(qCjyxScalarVolumeDisplayWidget);

  // Get voxel array
  vtkDMMLScalarVolumeNode* volumeNode = this->volumeNode();
  vtkImageData* imageData = volumeNode ? volumeNode->GetImageData() : nullptr;
  vtkPointData* pointData = imageData ? imageData->GetPointData() : nullptr;
  vtkDataArray* voxelValues = pointData ? pointData->GetScalars() : nullptr;

  // If there are no voxel values then we completely hide the histogram section
  d->HistogramGroupBox->setVisible(voxelValues != nullptr);

  d->Histogram->setDataArray(voxelValues);
  // Calling histogram build() with an empty volume causes heap corruption
  // (reported by VS2013 in debug mode), therefore we only build
  // the histogram if there are voxels (otherwise histogram is hidden).

  if (!voxelValues || !this->isVisible() || d->HistogramGroupBox->collapsed())
    {
    d->ColorTransferFunction->RemoveAllPoints();
    return;
    }

  // Update histogram

  // Screen resolution is limited, therefore it does not make sense to compute
  // many bin counts.
  const int maxBinCount = 1000;
  if (voxelValues->GetDataType() == VTK_FLOAT || voxelValues->GetDataType() == VTK_DOUBLE)
    {
    d->Histogram->setNumberOfBins(maxBinCount);
    }
  else
    {
    double* range = voxelValues->GetRange();
    int binCount = static_cast<int>(range[1] - range[0] + 1);
    if (binCount > maxBinCount)
      {
      binCount = maxBinCount;
      }
    if (binCount < 1)
      {
      binCount = 1;
      }
    d->Histogram->setNumberOfBins(binCount);
    }
  d->Histogram->build();

  // Update histogram background

  // from vtkKWWindowLevelThresholdEditor::UpdateTransferFunction
  double range[2] = {0,255};
  vtkDMMLScalarVolumeDisplayNode* displayNode =
    this->volumeDisplayNode();
  if (displayNode)
    {
    displayNode->GetDisplayScalarRange(range);
    }
  else
    {
    imageData->GetScalarRange(range);
    }
  // AdjustRange call will take out points that are outside of the new
  // range, but it needs the points to be there in order to work, so call
  // RemoveAllPoints after it's done
  d->ColorTransferFunction->AdjustRange(range);
  d->ColorTransferFunction->RemoveAllPoints();

  double min = d->DMMLWindowLevelWidget->level() - 0.5 * d->DMMLWindowLevelWidget->window();
  double max = d->DMMLWindowLevelWidget->level() + 0.5 * d->DMMLWindowLevelWidget->window();
  double minVal = 0;
  double maxVal = 1;
  double low   = d->DMMLVolumeThresholdWidget->isOff() ? range[0] : d->DMMLVolumeThresholdWidget->lowerThreshold();
  double upper = d->DMMLVolumeThresholdWidget->isOff() ? range[1] : d->DMMLVolumeThresholdWidget->upperThreshold();

  d->ColorTransferFunction->SetColorSpaceToRGB();

  if (low >= max || upper <= min)
    {
    d->ColorTransferFunction->AddRGBPoint(range[0], 0, 0, 0);
    d->ColorTransferFunction->AddRGBPoint(range[1], 0, 0, 0);
    }
  else
    {
    max = qMax(min+0.001, max);
    low = qMax(range[0] + 0.001, low);
    min = qMax(range[0] + 0.001, min);
    upper = qMin(range[1] - 0.001, upper);

    if (min <= low)
      {
      minVal = (low - min)/(max - min);
      min = low + 0.001;
      }

    if (max >= upper)
      {
      maxVal = (upper - min)/(max-min);
      max = upper - 0.001;
      }

    d->ColorTransferFunction->AddRGBPoint(range[0], 0, 0, 0);
    d->ColorTransferFunction->AddRGBPoint(low, 0, 0, 0);
    d->ColorTransferFunction->AddRGBPoint(min, minVal, minVal, minVal);
    d->ColorTransferFunction->AddRGBPoint(max, maxVal, maxVal, maxVal);
    d->ColorTransferFunction->AddRGBPoint(upper, maxVal, maxVal, maxVal);
    if (upper+0.001 < range[1])
      {
      d->ColorTransferFunction->AddRGBPoint(upper+0.001, 0, 0, 0);
      d->ColorTransferFunction->AddRGBPoint(range[1], 0, 0, 0);
      }
    }

  d->ColorTransferFunction->SetAlpha(1.0);
  d->ColorTransferFunction->Build();
}

// -----------------------------------------------------------------------------
void qCjyxScalarVolumeDisplayWidget::showEvent( QShowEvent * event )
{
  this->updateHistogram();
  this->Superclass::showEvent(event);
}

// --------------------------------------------------------------------------
void qCjyxScalarVolumeDisplayWidget::onHistogramSectionExpanded(bool expanded)
{
  Q_UNUSED(expanded);
  this->updateHistogram();
}

// --------------------------------------------------------------------------
void qCjyxScalarVolumeDisplayWidget::setInterpolate(bool interpolate)
{
  vtkDMMLScalarVolumeDisplayNode* displayNode =
    this->volumeDisplayNode();
  if (!displayNode)
    {
    return;
    }
  displayNode->SetInterpolate(interpolate);
}

// --------------------------------------------------------------------------
void qCjyxScalarVolumeDisplayWidget::setColorNode(vtkDMMLNode* colorNode)
{
  vtkDMMLScalarVolumeDisplayNode* displayNode =
    this->volumeDisplayNode();
  if (!displayNode || !colorNode)
    {
    return;
    }
  Q_ASSERT(vtkDMMLColorNode::SafeDownCast(colorNode));
  displayNode->SetAndObserveColorNodeID(colorNode->GetID());
}

// --------------------------------------------------------------------------
void qCjyxScalarVolumeDisplayWidget::onLockWindowLevelButtonClicked()
{
  vtkDMMLScalarVolumeDisplayNode* displayNode = this->volumeDisplayNode();
  if (!displayNode)
    {
    return;
    }
  // toggle the lock
  int locked = displayNode->GetWindowLevelLocked();
  displayNode->SetWindowLevelLocked(!locked);
}

// --------------------------------------------------------------------------
void qCjyxScalarVolumeDisplayWidget::onPresetButtonClicked()
{
  QToolButton* preset = qobject_cast<QToolButton*>(this->sender());
  this->setPreset(preset->objectName());
}

// --------------------------------------------------------------------------
void qCjyxScalarVolumeDisplayWidget::setPreset(const QString& presetId)
{
  Q_D(qCjyxScalarVolumeDisplayWidget);
  vtkCjyxVolumesLogic* volumesModuleLogic = vtkCjyxVolumesLogic::SafeDownCast(qCjyxApplication::application()->moduleLogic("Volumes"));
  if (!volumesModuleLogic)
    {
    qCritical() << Q_FUNC_INFO << " failed: volumes module logic is not available";
    return;
    }
  volumesModuleLogic->ApplyVolumeDisplayPreset(this->volumeDisplayNode(), presetId.toStdString());
}
