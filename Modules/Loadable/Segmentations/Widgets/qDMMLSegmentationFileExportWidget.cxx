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

  This file was originally developed by Andras Lasso, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

// Segmentations includes
#include "qDMMLSegmentationFileExportWidget.h"

#include "ui_qDMMLSegmentationFileExportWidget.h"

#include "vtkCjyxSegmentationsModuleLogic.h"

#include "vtkDMMLSegmentationDisplayNode.h"
#include "vtkDMMLSegmentationNode.h"
#include "vtkDMMLVolumeNode.h"

// VTK includes
#include <vtkStringArray.h>
#include <vtkWeakPointer.h>

// Qt includes
#include <QDebug>
#include <QDesktopServices>
#include <QSettings>
#include <QUrl>

// CTK includes
#include <ctkMessageBox.h>

// Cjyx includes
#include "qCjyxCoreApplication.h"
#include <vtkDMMLSliceLogic.h>
#include <vtkOrientedImageData.h>
#include <vtkOrientedImageDataResample.h>
#include <vtkCjyxApplicationLogic.h>

//-----------------------------------------------------------------------------
class qDMMLSegmentationFileExportWidgetPrivate: public Ui_qDMMLSegmentationFileExportWidget
{
  Q_DECLARE_PUBLIC(qDMMLSegmentationFileExportWidget);

protected:
  qDMMLSegmentationFileExportWidget* const q_ptr;
public:
  qDMMLSegmentationFileExportWidgetPrivate(qDMMLSegmentationFileExportWidget& object);
  void init();

public:
  vtkWeakPointer<vtkDMMLSegmentationNode> SegmentationNode;
  QString SettingsKey;
};

//-----------------------------------------------------------------------------
qDMMLSegmentationFileExportWidgetPrivate::qDMMLSegmentationFileExportWidgetPrivate(qDMMLSegmentationFileExportWidget& object)
  : q_ptr(&object)
{
  this->SegmentationNode = nullptr;
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationFileExportWidgetPrivate::init()
{
  Q_Q(qDMMLSegmentationFileExportWidget);
  this->setupUi(q);
  q->setEnabled(false);
  QObject::connect(this->ExportToFilesButton, SIGNAL(clicked()),
    q, SLOT(exportToFiles()));
  QObject::connect(this->ShowDestinationFolderButton, SIGNAL(clicked()),
    q, SLOT(showDestinationFolder()));
  QObject::connect(this->FileFormatComboBox, SIGNAL(currentIndexChanged(const QString&)),
    q, SLOT(setFileFormat(const QString&)));
  QObject::connect(this->ColorTableNodeSelector, SIGNAL(currentNodeIDChanged(const QString&)),
    q, SLOT(setColorNodeID(const QString&)));
  QObject::connect(this->UseColorTableValuesCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setUseLabelsFromColorNode(bool)));
}

//-----------------------------------------------------------------------------
// qDMMLSegmentationFileExportWidget methods

//-----------------------------------------------------------------------------
qDMMLSegmentationFileExportWidget::qDMMLSegmentationFileExportWidget(QWidget* _parent)
  : qDMMLWidget(_parent)
  , d_ptr(new qDMMLSegmentationFileExportWidgetPrivate(*this))
{
  Q_D(qDMMLSegmentationFileExportWidget);
  d->init();
  this->updateWidgetFromSettings();
}

//-----------------------------------------------------------------------------
qDMMLSegmentationFileExportWidget::~qDMMLSegmentationFileExportWidget() = default;

//-----------------------------------------------------------------------------
void qDMMLSegmentationFileExportWidget::setDMMLScene(vtkDMMLScene* scene)
{
  qDMMLWidget::setDMMLScene(scene);
  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
vtkDMMLSegmentationNode* qDMMLSegmentationFileExportWidget::segmentationNode() const
{
  Q_D(const qDMMLSegmentationFileExportWidget);
  return d->SegmentationNode;
}

//-----------------------------------------------------------------------------
QString qDMMLSegmentationFileExportWidget::segmentationNodeID()
{
  Q_D(qDMMLSegmentationFileExportWidget);
  return (d->SegmentationNode.GetPointer() ? d->SegmentationNode->GetID() : QString());
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationFileExportWidget::setSegmentationNode(vtkDMMLSegmentationNode* node)
{
  Q_D(qDMMLSegmentationFileExportWidget);

  qvtkReconnect(d->SegmentationNode, node, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromDMML()));
  qvtkReconnect(d->SegmentationNode, node, vtkDMMLSegmentationNode::ReferenceImageGeometryChangedEvent, this,
    SLOT(onSegmentationReferenceImageGeometryChanged()));

  bool nodeChanged = d->SegmentationNode != node;

  d->SegmentationNode = node;
  this->setEnabled(d->SegmentationNode.GetPointer() != nullptr);
  this->updateWidgetFromDMML();

  // Get reference volume node
  if (nodeChanged)
    {
    this->onSegmentationReferenceImageGeometryChanged();
    }

}

//-----------------------------------------------------------------------------
void qDMMLSegmentationFileExportWidget::setSegmentationNode(vtkDMMLNode* node)
{
  this->setSegmentationNode(vtkDMMLSegmentationNode::SafeDownCast(node));
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationFileExportWidget::updateWidgetFromDMML()
{
  Q_D(qDMMLSegmentationFileExportWidget);
  vtkDMMLColorTableNode* exportColorTableNode = nullptr;
  if (d->SegmentationNode)
    {
    exportColorTableNode = d->SegmentationNode->GetLabelmapConversionColorTableNode();
    }
  d->ColorTableNodeSelector->setCurrentNode(exportColorTableNode);
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationFileExportWidget::updateWidgetFromSettings()
{
  Q_D(qDMMLSegmentationFileExportWidget);

  if (d->SettingsKey.isEmpty())
    {
    return;
    }

  QSettings settings;

  QString fileFormat = settings.value(d->SettingsKey + "/FileFormat", "STL").toString();
  d->FileFormatComboBox->setCurrentIndex(d->FileFormatComboBox->findText(fileFormat));
  this->setFileFormat(fileFormat);

  QString path = qCjyxCoreApplication::application()->toCjyxHomeAbsolutePath(settings.value(d->SettingsKey + "/DestinationFolder", ".").toString());
  d->DestinationFolderButton->setDirectory(path);
  d->VisibleSegmentsOnlyCheckBox->setChecked(settings.value(d->SettingsKey + "/VisibleSegmentsOnly", false).toBool());

  d->MergeIntoSingleSTLFileCheckBox->setChecked(settings.value(d->SettingsKey + "/MergeIntoSingleFile", false).toBool());
  d->MergeIntoSingleOBJFileCheckBox->setChecked(settings.value(d->SettingsKey + "/MergeIntoSingleFile", false).toBool());

  d->SizeScaleSpinBox->setValue(settings.value(d->SettingsKey + "/SizeScale", 1.0).toDouble());
  d->ShowDestinationFolderOnExportCompleteCheckBox->setChecked(settings.value(d->SettingsKey + "/ShowDestinationFolderOnExportComplete", true).toBool());

  QString coordinateSystem = settings.value(d->SettingsKey + "/CoordinateSystem", "LPS").toString();
  d->CoordinateSystemComboBox->setCurrentIndex(d->CoordinateSystemComboBox->findText(coordinateSystem));
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationFileExportWidget::updateSettingsFromWidget()
{
  Q_D(qDMMLSegmentationFileExportWidget);

  if (d->SettingsKey.isEmpty())
    {
    return;
    }

  QSettings settings;

  settings.setValue(d->SettingsKey + "/FileFormat", d->FileFormatComboBox->currentText());
  QString path = qCjyxCoreApplication::application()->toCjyxHomeRelativePath(d->DestinationFolderButton->directory());
  settings.setValue(d->SettingsKey + "/DestinationFolder", path);
  settings.setValue(d->SettingsKey + "/VisibleSegmentsOnly", d->VisibleSegmentsOnlyCheckBox->isChecked());
  settings.setValue(d->SettingsKey + "/MergeIntoSingleFile", d->MergeIntoSingleSTLFileCheckBox->isChecked());
  settings.setValue(d->SettingsKey + "/SizeScale", d->SizeScaleSpinBox->value());
  settings.setValue(d->SettingsKey + "/ShowDestinationFolderOnExportComplete", d->ShowDestinationFolderOnExportCompleteCheckBox->isChecked());
  settings.setValue(d->SettingsKey + "/CoordinateSystem", d->CoordinateSystemComboBox->currentText());
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationFileExportWidget::onSegmentationReferenceImageGeometryChanged()
{
  Q_D(qDMMLSegmentationFileExportWidget);
  if (!d->SegmentationNode)
    {
    return;
    }

  vtkDMMLNode* referenceVolumeNode = d->SegmentationNode->GetNodeReference(
    vtkDMMLSegmentationNode::GetReferenceImageGeometryReferenceRole().c_str());
  d->ReferenceVolumeComboBox->setCurrentNode(referenceVolumeNode);
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationFileExportWidget::exportToFiles()
{
  Q_D(qDMMLSegmentationFileExportWidget);

  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

  this->updateSettingsFromWidget();

  vtkSmartPointer<vtkStringArray> segmentIds;
  if (d->VisibleSegmentsOnlyCheckBox->isChecked()
    && d->SegmentationNode != nullptr
    && vtkDMMLSegmentationDisplayNode::SafeDownCast(d->SegmentationNode->GetDisplayNode()) != nullptr)
    {
    segmentIds = vtkSmartPointer<vtkStringArray>::New();
    vtkDMMLSegmentationDisplayNode* displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(d->SegmentationNode->GetDisplayNode());
    displayNode->GetVisibleSegmentIDs(segmentIds);
    }

  bool merge = d->MergeIntoSingleSTLFileCheckBox->isChecked(); // merge is only used for STL format

  QString fileFormat = d->FileFormatComboBox->currentText();
  if (fileFormat.toUpper() == "STL" || fileFormat.toUpper() == "OBJ")
    {
    vtkCjyxSegmentationsModuleLogic::ExportSegmentsClosedSurfaceRepresentationToFiles(
      d->DestinationFolderButton->directory().toUtf8().constData(),
      d->SegmentationNode.GetPointer(),
      segmentIds.GetPointer(),
      d->FileFormatComboBox->currentText().toUtf8().constData(),
      d->CoordinateSystemComboBox->currentText() == "LPS",
      d->SizeScaleSpinBox->value(),
      merge);
    }
  else if (fileFormat.toUpper() == "NRRD" || fileFormat.toUpper() == "NIFTI")
    {
    std::string extension = fileFormat.toUtf8().constData();
    if (fileFormat == "NIFTI")
      {
      extension = "nii";
      if (d->UseCompressionCheckBox->isChecked())
        {
        extension = "nii.gz";
        }
      }

    vtkDMMLColorTableNode* labelmapConversionColorTableNode = nullptr;
    if (d->UseColorTableValuesCheckBox->isChecked())
      {
      labelmapConversionColorTableNode = vtkDMMLColorTableNode::SafeDownCast(d->ColorTableNodeSelector->currentNode());
      }

    vtkDMMLVolumeNode* referenceVolumeNode = vtkDMMLVolumeNode::SafeDownCast(d->ReferenceVolumeComboBox->currentNode());
    if (referenceVolumeNode && d->SegmentationNode &&
      vtkCjyxSegmentationsModuleLogic::IsEffectiveExentOutsideReferenceVolume(
        referenceVolumeNode, d->SegmentationNode, segmentIds))
      {
      ctkMessageBox* exportWarningMesssgeBox = new ctkMessageBox(this);
      exportWarningMesssgeBox->setAttribute(Qt::WA_DeleteOnClose);
      exportWarningMesssgeBox->setWindowTitle("Exporting may erase data");
      exportWarningMesssgeBox->addButton(QMessageBox::StandardButton::Ok);
      exportWarningMesssgeBox->addButton(QMessageBox::StandardButton::Cancel);
      exportWarningMesssgeBox->setDontShowAgainVisible(true);
      exportWarningMesssgeBox->setIcon(QMessageBox::Warning);
      exportWarningMesssgeBox->setDontShowAgainSettingsKey("Segmentations/AlwaysCropDuringSegmentationFileExport");
      exportWarningMesssgeBox->setText("The current segmentation does not completely fit into the new geometry.\n"
                                       "Do you want to crop the segmentation?\n");
      if (exportWarningMesssgeBox->exec() != QMessageBox::StandardButton::Ok)
        {
        return;
        }
      }

    vtkCjyxSegmentationsModuleLogic::ExportSegmentsBinaryLabelmapRepresentationToFiles(
      d->DestinationFolderButton->directory().toUtf8().constData(),
      d->SegmentationNode.GetPointer(),
      segmentIds.GetPointer(),
      extension,
      d->UseCompressionCheckBox->isChecked(),
      referenceVolumeNode,
      vtkSegmentation::EXTENT_UNION_OF_EFFECTIVE_SEGMENTS_AND_REFERENCE_GEOMETRY,
      labelmapConversionColorTableNode);
    }

  QApplication::restoreOverrideCursor();

  if (d->ShowDestinationFolderOnExportCompleteCheckBox->isChecked())
    {
    this->showDestinationFolder();
    }

  emit exportToFilesDone();
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationFileExportWidget::showDestinationFolder()
{
  Q_D(qDMMLSegmentationFileExportWidget);
  QDesktopServices::openUrl(QUrl("file:///" + d->DestinationFolderButton->directory(), QUrl::TolerantMode));
}

//------------------------------------------------------------------------------
QString qDMMLSegmentationFileExportWidget::settingsKey()const
{
  Q_D(const qDMMLSegmentationFileExportWidget);
  return d->SettingsKey;
}

//------------------------------------------------------------------------------
void qDMMLSegmentationFileExportWidget::setSettingsKey(const QString& key)
{
  Q_D(qDMMLSegmentationFileExportWidget);
  d->SettingsKey = key;
  this->updateWidgetFromSettings();
}

//------------------------------------------------------------------------------
void qDMMLSegmentationFileExportWidget::setFileFormat(const QString& formatStr)
{
  Q_D(qDMMLSegmentationFileExportWidget);

  bool formatIsSTL = formatStr == "STL";
  bool formatIsModel = formatIsSTL || formatStr == "OBJ";

  d->MergeIntoSingleSTLFileCheckBox->setVisible(formatIsSTL);
  d->MergeIntoSingleSTLFileCheckBox->setEnabled(formatIsModel);

  d->MergeIntoSingleOBJFileCheckBox->setVisible(!formatIsSTL);
  d->MergeIntoSingleOBJFileCheckBox->setEnabled(formatIsModel);

  d->CoordinateSystemComboBox->setEnabled(formatIsModel);
  d->ReferenceVolumeComboBox->setEnabled(!formatIsModel);
  d->SizeScaleSpinBox->setEnabled(formatIsModel);
  d->UseCompressionCheckBox->setEnabled(!formatIsModel);
  d->UseColorTableValuesCheckBox->setEnabled(!formatIsModel);
  d->ColorTableNodeSelector->setEnabled(!formatIsModel && d->UseColorTableValuesCheckBox->isChecked());
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationFileExportWidget::setUseLabelsFromColorNode(bool useColorNode)
{
  Q_UNUSED(useColorNode);
  Q_D(qDMMLSegmentationFileExportWidget);
  d->ColorTableNodeSelector->setEnabled(d->UseColorTableValuesCheckBox->isEnabled() && d->UseColorTableValuesCheckBox->isChecked());
}

//------------------------------------------------------------------------------
void qDMMLSegmentationFileExportWidget::setColorNodeID(const QString& id)
{
  Q_D(qDMMLSegmentationFileExportWidget);
  if (!d->SegmentationNode)
  {
    return;
  }
  std::string nodeID = id.toStdString();
  d->SegmentationNode->SetLabelmapConversionColorTableNodeID(nodeID.c_str());
}
