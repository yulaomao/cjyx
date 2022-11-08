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

#ifndef __qDMMLSliceControllerWidget_p_h
#define __qDMMLSliceControllerWidget_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Cjyx API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qDMML includes
#include "qDMMLSliceControllerWidget.h"
#include "qDMMLViewControllerBar_p.h"
#include "ui_qDMMLSliceControllerWidget.h"

// DMMLLogic includes
#include <vtkDMMLSliceLogic.h>

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkCollection.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

class ctkSignalMapper;
class ctkDoubleSpinBox;
class ctkVTKSliceView;
class QSpinBox;
class qDMMLSliderWidget;
class vtkDMMLSliceNode;
class vtkObject;
class vtkDMMLSegmentationDisplayNode;
class vtkDMMLSelectionNode;

//-----------------------------------------------------------------------------
struct QDMML_WIDGETS_EXPORT qDMMLOrientation
{
  QString Prefix;
  QString ToolTip;
};

//-----------------------------------------------------------------------------
class QDMML_WIDGETS_EXPORT qDMMLSliceControllerWidgetPrivate
  : public qDMMLViewControllerBarPrivate
  , public Ui_qDMMLSliceControllerWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qDMMLSliceControllerWidget);

public:
  typedef qDMMLSliceControllerWidgetPrivate Self;
  typedef qDMMLViewControllerBarPrivate Superclass;
  qDMMLSliceControllerWidgetPrivate(qDMMLSliceControllerWidget& object);
  ~qDMMLSliceControllerWidgetPrivate() override;

  void init() override;
  void setColor(QColor color) override;

  void setupLinkedOptionsMenu();
  void setupReformatOptionsMenu();
  void setupLightboxMenu();
  void setupCompositingMenu();
  void setupSliceSpacingMenu();
  void setupSliceModelMenu();
  void setupSegmentationMenu();
  void setupLabelMapMenu();
  void setupMoreOptionsMenu();
  void setupOrientationMarkerMenu();
  void setupRulerMenu();

  qDMMLOrientation dmmlOrientation(const QString& name);

  vtkSmartPointer<vtkCollection> saveNodesForUndo(const QString& nodeTypes);

  void enableLayerWidgets();

  vtkDMMLSliceLogic* compositeNodeLogic(vtkDMMLSliceCompositeNode* node);
  vtkDMMLSliceLogic* sliceNodeLogic(vtkDMMLSliceNode* node);

  void setForegroundInterpolation(vtkDMMLSliceLogic* logic, bool interpolate);
  void setBackgroundInterpolation(vtkDMMLSliceLogic* logic, bool interpolate);

  /// Create a list of orientation containing the regular presets and also
  /// the "Reformat" string if sliceToRAS is different one of the preset.
  static void updateSliceOrientationSelector(
      vtkDMMLSliceNode* sliceNode, QComboBox *sliceOrientationSelector);

public slots:
  /// Update widget state when the scene is modified
  void updateFromDMMLScene();

  /// Update widget state using the associated DMML slice node
  void updateWidgetFromDMMLSliceNode();

  /// Update widget state using the associated DMML slice composite node
  void updateWidgetFromDMMLSliceCompositeNode();

  /// Update step size when unit changes
  void updateWidgetFromUnitNode();

  /// Called after a foreground layer volume node is selected
  /// using the associated qDMMLNodeComboBox
  void onForegroundLayerNodeSelected(vtkDMMLNode* node);

  /// Called after a background layer volume node is selected
  /// using the associated qDMMLNodeComboBox
  void onBackgroundLayerNodeSelected(vtkDMMLNode* node);

  /// Called after a label layer volume node is selected
  /// using the associated qDMMLNodeComboBox
  void onLabelMapNodeSelected(vtkDMMLNode* node);

  /// Called after a segmentation node is selected in the combobox
  void onSegmentationNodeSelected(vtkDMMLNode* node);

  /// Called after the currently selected segmentation node's display
  /// option is modified
  void onSegmentationNodeDisplayModifiedEvent(vtkObject* nodeObject);
  /// Called when segment visibility is changed from the segment combobox
  void onSegmentVisibilitySelectionChanged(QStringList selectedSegmentIDs);
  /// Update segmentation outline/fill button
  void updateSegmentationOutlineFillButton();
  /// Utility function to get the display node of the current segmentation
  vtkDMMLSegmentationDisplayNode* currentSegmentationDisplayNode();

  void updateFromForegroundDisplayNode(vtkObject* displayNode);
  void updateFromBackgroundDisplayNode(vtkObject* displayNode);

  void updateFromForegroundVolumeNode(vtkObject* volumeNode);
  void updateFromBackgroundVolumeNode(vtkObject* volumeNode);

  /// Called after the SliceLogic is modified
  void onSliceLogicModifiedEvent();

  void applyCustomLightbox();

protected:
  void setupPopupUi() override;
  void setDMMLSliceCompositeNodeInternal(vtkDMMLSliceCompositeNode* sliceComposite);
  void setAndObserveSelectionNode();

public:
  vtkDMMLSliceCompositeNode*          DMMLSliceCompositeNode;
  vtkDMMLSelectionNode*               SelectionNode{nullptr};
  vtkSmartPointer<vtkDMMLSliceLogic>  SliceLogic;
  vtkCollection*                      SliceLogics;
  vtkWeakPointer<vtkAlgorithmOutput>  ImageDataConnection;
  QHash<QString, qDMMLOrientation>    SliceOrientationToDescription;
  QButtonGroup*                       ControllerButtonGroup;

  QToolButton*                        FitToWindowToolButton;
  qDMMLSliderWidget*                  SliceOffsetSlider;
  /// Cjyx offset resolution without applying display scaling.
  double                              SliceOffsetResolution{1.0};
  double                              LastLabelMapOpacity;
  double                              LastForegroundOpacity;
  double                              LastBackgroundOpacity;

  QMenu*                              LightboxMenu;
  QMenu*                              CompositingMenu;
  QMenu*                              SliceSpacingMenu;
  QMenu*                              SliceModelMenu;
  QMenu*                              SegmentationMenu;
  QMenu*                              LabelMapMenu;
  QMenu*                              OrientationMarkerMenu;
  QMenu*                              RulerMenu;

  ctkDoubleSpinBox*                   SliceSpacingSpinBox;
  ctkDoubleSpinBox*                   SliceFOVSpinBox;
  QSpinBox*                           LightBoxRowsSpinBox;
  QSpinBox*                           LightBoxColumnsSpinBox;

  ctkDoubleSpinBox*                   SliceModelFOVXSpinBox;
  ctkDoubleSpinBox*                   SliceModelFOVYSpinBox;

  ctkDoubleSpinBox*                   SliceModelOriginXSpinBox;
  ctkDoubleSpinBox*                   SliceModelOriginYSpinBox;

  QSpinBox*                           SliceModelDimensionXSpinBox;
  QSpinBox*                           SliceModelDimensionYSpinBox;

  QSize                               ViewSize;

  ctkSignalMapper*                    OrientationMarkerTypesMapper;
  ctkSignalMapper*                    OrientationMarkerSizesMapper;

  ctkSignalMapper*                    RulerTypesMapper;
  ctkSignalMapper*                    RulerColorMapper;
};

#endif
