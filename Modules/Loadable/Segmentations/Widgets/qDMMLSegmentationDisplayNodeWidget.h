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

#ifndef __qDMMLSegmentationDisplayNodeWidget_h
#define __qDMMLSegmentationDisplayNodeWidget_h

// DMMLWidgets includes
#include "qDMMLWidget.h"

#include "qCjyxSegmentationsModuleWidgetsExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

class qDMMLSegmentationDisplayNodeWidgetPrivate;

class vtkDMMLSegmentationNode;
class vtkDMMLSegmentationDisplayNode;
class QItemSelection;

/// \brief Qt widget for selecting a single segment from a segmentation.
///   If multiple segments are needed, then use \sa qDMMLSegmentsTableView instead in SimpleListMode
/// \ingroup Cjyx_QtModules_Segmentations_Widgets
class Q_CJYX_MODULE_SEGMENTATIONS_WIDGETS_EXPORT qDMMLSegmentationDisplayNodeWidget : public qDMMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  /// Constructor
  explicit qDMMLSegmentationDisplayNodeWidget(QWidget* parent = nullptr);
  /// Destructor
  ~qDMMLSegmentationDisplayNodeWidget() override;

  /// Get current segmentation display node
  Q_INVOKABLE vtkDMMLSegmentationDisplayNode* segmentationDisplayNode() const;
  /// Get current segmentation display node's ID
  Q_INVOKABLE QString segmentationDisplayNodeID();

  /// Get segment ID of selected segment
  Q_INVOKABLE QString currentSegmentID();

public slots:
  /// Set segmentation display DMML node
  Q_INVOKABLE void setSegmentationDisplayNode(vtkDMMLSegmentationDisplayNode* node);
  /// Utility function to set segmentation display node by segmentation node
  Q_INVOKABLE void setSegmentationNode(vtkDMMLSegmentationNode* node);

  /// Get segment ID of selected segment
  Q_INVOKABLE void setCurrentSegmentID(QString segmentID);

  /// Update widget from DMML
  void updateWidgetFromDMML();

  /// Populate combobox that contain possible 3D representation names
  void populate3DRepresentationsCombobox();
  /// Populate combobox that contain possible 2D representation names
  void populate2DRepresentationsCombobox();

  void onVisibilityChanged(int);
  void onOpacityChanged(double);
  void onVisibilitySliceFillChanged(int);
  void onVisibilitySliceOutlineChanged(int);
  void onVisibility3DChanged(int);
  void onOpacitySliceFillChanged(double);
  void onOpacitySliceOutlineChanged(double);
  void onOpacity3DChanged(double);
  void onSliceIntersectionThicknessChanged(int);
  void onRepresentation3DChanged(int);
  void onRepresentation2DChanged(int);
  void onSegmentVisibilitySliceFillChanged(int);
  void onSegmentVisibilitySliceOutlineChanged(int);
  void onSegmentVisibility3DChanged(int);
  void onSegmentOpacitySliceFillChanged(double);
  void onSegmentOpacitySliceOutlineChanged(double);
  void onSegmentOpacity3DChanged(double);

  /// Handles segment selection changes when connecting directly to a \sa qDMMLSegmentsTableView
  void onSegmentSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

protected:
  /// Update visibility and opacity section for selected segment
  void updateSelectedSegmentSection();

protected:
  QScopedPointer<qDMMLSegmentationDisplayNodeWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLSegmentationDisplayNodeWidget);
  Q_DISABLE_COPY(qDMMLSegmentationDisplayNodeWidget);
};

#endif
