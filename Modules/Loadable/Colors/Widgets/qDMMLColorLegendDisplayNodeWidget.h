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

==============================================================================*/

#ifndef __qDMMLColorLegendDisplayNodeWidget_h
#define __qDMMLColorLegendDisplayNodeWidget_h

// Cjyx includes
#include <qDMMLWidget.h>

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// Colors Widgets includes
#include "qCjyxColorsModuleWidgetsExport.h"

class vtkDMMLNode;
class vtkDMMLColorLegendDisplayNode;
class qDMMLColorLegendDisplayNodeWidgetPrivate;
class QAbstractButton;

/// \ingroup Cjyx_QtModules_Colors
class Q_CJYX_MODULE_COLORS_WIDGETS_EXPORT qDMMLColorLegendDisplayNodeWidget
  : public qDMMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qDMMLWidget Superclass;
  explicit qDMMLColorLegendDisplayNodeWidget(QWidget *parent=0);
  ~qDMMLColorLegendDisplayNodeWidget() override;

public slots:
  void setDMMLScene(vtkDMMLScene*) override;
  /// Set ColorLegendDisplay DMML node (Parameter node)
  /// Set color legend display node
  void setDMMLColorLegendDisplayNode(vtkDMMLColorLegendDisplayNode* node);
  /// Get color legend display node
  vtkDMMLColorLegendDisplayNode* dmmlColorLegendDisplayNode();
  /// Utility function to be connected with generic signals
  void setDMMLColorLegendDisplayNode(vtkDMMLNode* node);

  void onColorLegendVisibilityToggled(bool);

  void onTitleTextChanged(const QString&);

  void onLabelFormatChanged(const QString&);

  void onMaximumNumberOfColorsChanged(int);
  void onNumberOfLabelsChanged(int);

protected slots:
  /// Update widget GUI from color legend parameters node
  void updateWidgetFromDMML();

  void onColorLegendOrientationButtonClicked(QAbstractButton*);
  void onLabelTextButtonClicked(QAbstractButton*);
  void onPositionChanged();
  void onSizeChanged();


protected:
  QScopedPointer<qDMMLColorLegendDisplayNodeWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLColorLegendDisplayNodeWidget);
  Q_DISABLE_COPY(qDMMLColorLegendDisplayNodeWidget);
};

#endif
