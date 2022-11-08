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

#ifndef __qDMMLSegmentationFileExportWidget_h
#define __qDMMLSegmentationFileExportWidget_h

// DMMLWidgets includes
#include "qDMMLWidget.h"

#include "qCjyxSegmentationsModuleWidgetsExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

class qDMMLSegmentationFileExportWidgetPrivate;

class vtkDMMLNode;
class vtkDMMLSegmentationNode;
class vtkDMMLSegmentationDisplayNode;
class QItemSelection;

/// \brief Qt widget for selecting a single segment from a segmentation.
///   If multiple segments are needed, then use \sa qDMMLSegmentsTableView instead in SimpleListMode
/// \ingroup Cjyx_QtModules_Segmentations_Widgets
class Q_CJYX_MODULE_SEGMENTATIONS_WIDGETS_EXPORT qDMMLSegmentationFileExportWidget : public qDMMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

    /// Key for storing selected options in application settings.
    /// If an empty key string is given, then selections are not saved or loaded in settings.
    /// Empty by default.
    Q_PROPERTY(QString settingsKey READ settingsKey WRITE setSettingsKey)

public:
  /// Constructor
  explicit qDMMLSegmentationFileExportWidget(QWidget* parent = nullptr);
  /// Destructor
  ~qDMMLSegmentationFileExportWidget() override;

  QString settingsKey()const;
  void setSettingsKey(const QString& key);

  /// Get current segmentation node
  Q_INVOKABLE vtkDMMLSegmentationNode* segmentationNode() const;
  /// Get current segmentation node's ID
  Q_INVOKABLE QString segmentationNodeID();

signals:
  /// Emitted when conversion is done
  void exportToFilesDone();

public slots:
  void setDMMLScene(vtkDMMLScene* dmmlScene) override;

  /// Set segmentation DMML node
  void setSegmentationNode(vtkDMMLSegmentationNode* node);
  void setSegmentationNode(vtkDMMLNode* node);

  void exportToFiles();

  void showDestinationFolder();

  void updateWidgetFromSettings();
  void updateSettingsFromWidget();

  void updateWidgetFromDMML();

  void onSegmentationReferenceImageGeometryChanged();

protected slots:

  void setFileFormat(const QString&);
  void setColorNodeID(const QString&);
  void setUseLabelsFromColorNode(bool useColorNode);

protected:
  QScopedPointer<qDMMLSegmentationFileExportWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLSegmentationFileExportWidget);
  Q_DISABLE_COPY(qDMMLSegmentationFileExportWidget);
};

#endif
