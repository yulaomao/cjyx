/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under this License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qDMMLModelDisplayNodeWidget_h
#define __qDMMLModelDisplayNodeWidget_h

// DMMLWidgets includes
#include "qDMMLWidget.h"
#include "qDMMLScalarsDisplayWidget.h"

// CTK includes
#include <ctkVTKObject.h>

// qDMML includes
#include "qCjyxModelsModuleWidgetsExport.h"

class qDMMLModelDisplayNodeWidgetPrivate;
class vtkDMMLColorNode;
class vtkDMMLDisplayNode;
class vtkDMMLModelDisplayNode;
class vtkDMMLNode;

class Q_CJYX_QTMODULES_MODELS_WIDGETS_EXPORT qDMMLModelDisplayNodeWidget : public qDMMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

  Q_PROPERTY(bool clippingConfigurationButtonVisible READ clippingConfigurationButtonVisible WRITE setClippingConfigurationButtonVisible)

public:
  typedef qDMMLWidget Superclass;
  qDMMLModelDisplayNodeWidget(QWidget* parent = nullptr);
  ~qDMMLModelDisplayNodeWidget() override;

  /// Get model display node (if model was selected not folder)
  vtkDMMLModelDisplayNode* dmmlModelDisplayNode()const;
  /// Get current display node (may be model or folder display node)
  vtkDMMLDisplayNode* dmmlDisplayNode()const;
  /// Get current item (if single selection)
  vtkIdType currentSubjectHierarchyItemID()const;
  /// Get current items (if multi selection)
  QList<vtkIdType> currentSubjectHierarchyItemIDs()const;

  bool visibility()const;
  bool clipping()const;
  bool sliceIntersectionVisible()const;
  int sliceIntersectionThickness()const;
  double sliceIntersectionOpacity()const;
  bool clippingConfigurationButtonVisible()const;

signals:
  /// Signal sent if the auto/manual value is updated
  void scalarRangeModeValueChanged(vtkDMMLDisplayNode::ScalarRangeFlagType value);
  /// Signal sent if the any property in the display node is changed
  void displayNodeChanged();
  /// Signal sent if user toggles clipping checkbox on the GUI
  void clippingToggled(bool);
  /// Signal sent if clipping configuration button is clicked
  void clippingConfigurationButtonClicked();

public slots:
  /// Set the scene.
  void setDMMLScene(vtkDMMLScene* newScene) override;
  /// Set the model display node
  void setDMMLModelDisplayNode(vtkDMMLModelDisplayNode* node);
  /// Utility function to be connected with generic signals
  void setDMMLModelDisplayNode(vtkDMMLNode* node);
  /// Set display node (may be model or folder display node)
  void setDMMLDisplayNode(vtkDMMLDisplayNode* displayNode);
  /// Utility function to be connected with generic signals,
  /// it internally shows the 1st display node.
  /// can be set from the item of a model node or a folder.
  void setCurrentSubjectHierarchyItemID(vtkIdType currentItemID);
  /// Set the current subject hierarchy items.
  /// Both model and folder items are supported. In case of multi
  /// selection, the first item's display properties are displayed
  /// in the widget, but the changed settings are applied on all
  /// selected items if applicable.
  void setCurrentSubjectHierarchyItemIDs(QList<vtkIdType> currentItemIDs);

  void setVisibility(bool);
  void setClipping(bool);

  void setSliceIntersectionVisible(bool);
  void setSliceDisplayMode(int);
  void setSliceIntersectionThickness(int);
  void setSliceIntersectionOpacity(double);
  void setDistanceToColorNode(vtkDMMLNode*);

  void setRepresentation(int);
  void setPointSize(double);
  void setLineWidth(double);
  void setShowFaces(int);
  void setColor(const QColor&);
  void setBackfaceHueOffset(double newOffset);
  void setBackfaceSaturationOffset(double newOffset);
  void setBackfaceBrightnessOffset(double newOffset);

  void setOpacity(double);
  void setEdgeVisibility(bool);
  void setEdgeColor(const QColor&);
  void setLighting(bool);
  void setInterpolation(int);

  /// Show/hide "Configure..." button for clipping
  void setClippingConfigurationButtonVisible(bool);

protected slots:
  void updateWidgetFromDMML();
  void updateDisplayNodesFromProperty();

protected:
  QScopedPointer<qDMMLModelDisplayNodeWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLModelDisplayNodeWidget);
  Q_DISABLE_COPY(qDMMLModelDisplayNodeWidget);
};

#endif
