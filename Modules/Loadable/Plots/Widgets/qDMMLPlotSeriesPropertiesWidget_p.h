/*==============================================================================

  Copyright (c) Kapteyn Astronomical Institute
  University of Groningen, Groningen, Netherlands. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Davide Punzo, Kapteyn Astronomical Institute,
  and was supported through the European Research Council grant nr. 291531.

==============================================================================*/

#ifndef __qDMMLPlotSeriesPropertiesWidget_p_h
#define __qDMMLPlotSeriesPropertiesWidget_p_h

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

/// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qDMML includes
#include "qDMMLPlotSeriesPropertiesWidget.h"
#include "ui_qDMMLPlotSeriesPropertiesWidget.h"

/// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>
#include <vtkImageData.h>

class QAction;
class ctkVTKPlotViewView;
class vtkDMMLPlotChartNode;
class vtkDMMLPlotSeriesNode;
class vtkDMMLPlotViewNode;
class vtkObject;

//-----------------------------------------------------------------------------
class qDMMLPlotSeriesPropertiesWidgetPrivate
 : public QObject
 , public Ui_qDMMLPlotSeriesPropertiesWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qDMMLPlotSeriesPropertiesWidget);
protected:
  qDMMLPlotSeriesPropertiesWidget* const q_ptr;
public:
  qDMMLPlotSeriesPropertiesWidgetPrivate(qDMMLPlotSeriesPropertiesWidget& object);
  ~qDMMLPlotSeriesPropertiesWidgetPrivate() override;

  void setupUi(qDMMLWidget* widget);

  vtkDMMLPlotChartNode* GetPlotChartNodeFromView();

public slots:

  /// Update widget state using the associated DMML PlotSeries node.
  void updateWidgetFromDMML();

  /// Called after a PlotSeriesNode is selected
  /// using the associated qDMMLNodeComboBox.
  void onPlotSeriesNodeChanged(vtkDMMLNode* node);

  /// Called after a TableNode is selected
  /// using the associated qDMMLNodeComboBox.
  void onInputTableNodeChanged(vtkDMMLNode* node);

  /// Change the x-Axis of the plot.
  void onXAxisChanged(int index);

  /// Change the labels of the plot.
  void onLabelsChanged(int index);

  /// Change the y-Axis of the plot.
  void onYAxisChanged(int index);

  /// Change the type of plot (scatter, line, bar).
  void onPlotTypeChanged(int index);

  /// Change markers style for Line and Scatter plots.
  void onMarkersStyleChanged(const QString& style);

  /// Change markers size for Line and Scatter plots.
  void onMarkersSizeChanged(double size);

  /// Change line style for Line and Scatter plots.
  void onLineStyleChanged(const QString &style);

  /// Change line width for Line and Scatter plots.
  void onLineWidthChanged(double width);

  /// Change the color of a single PlotSeriesNode.
  void onPlotSeriesColorChanged(const QColor& color);

public:
  vtkWeakPointer<vtkDMMLPlotViewNode>    PlotViewNode;
  vtkWeakPointer<vtkDMMLPlotSeriesNode>    PlotSeriesNode;

};

#endif
