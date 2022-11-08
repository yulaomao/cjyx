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

#ifndef __qDMMLPlotChartPropertiesWidget_h
#define __qDMMLPlotChartPropertiesWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>

// Plots Widgets includes
#include "qCjyxPlotsModuleWidgetsExport.h"
#include "ui_qDMMLPlotChartPropertiesWidget.h"

class qDMMLPlotChartPropertiesWidgetPrivate;
class vtkDMMLNode;
class vtkDMMLPlotChartNode;

class Q_CJYX_MODULE_PLOTS_WIDGETS_EXPORT qDMMLPlotChartPropertiesWidget : public qDMMLWidget
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef qDMMLWidget Superclass;

  /// Constructors
  explicit qDMMLPlotChartPropertiesWidget(QWidget* parent = nullptr);
  ~qDMMLPlotChartPropertiesWidget() override;

  /// Get \a PlotViewNode
  vtkDMMLPlotChartNode* dmmlPlotChartNode()const;

public slots:

  /// Set the scene.
  void setDMMLScene(vtkDMMLScene* newScene) override;

  /// Set a new PlotViewNode.
  void setDMMLPlotChartNode(vtkDMMLNode* node);

  /// Set a new PlotViewNode.
  void setDMMLPlotChartNode(vtkDMMLPlotChartNode* plotChartNode);

  /// Control the display of a grid in the chart.
  void setGridVisibility(bool show);

  /// Control the display of the legend in the chart.
  void setLegendVisibility(bool show);

  /// Set the title.
  /// \sa TitleVisibility
  void setTitle(const QString& str);

  /// Set the label along the X-Axis.
  /// \sa showXAxisLabel
  void setXAxisLabel(const QString& str);

  /// Set the label along the Y-Axis.
  /// \sa showYAxisLabel
  void setYAxisLabel(const QString& str);

  /// Change the type of font used in the plot.
  void setFontType(const QString& type);

  /// Change the font size of the title of the plot.
  void setTitleFontSize(double size);

  /// Change the font size of the legend of the plot.
  void setLegendFontSize(double size);

  /// Change the font size of the title of the axes.
  void setAxisTitleFontSize(double size);

  /// Change the font size of the labels of the axes.
  void setAxisLabelFontSize(double size);

  void setXAxisManualRangeEnabled(bool);
  void setXAxisRangeMin(double);
  void setXAxisRangeMax(double);

  void setYAxisManualRangeEnabled(bool);
  void setYAxisRangeMin(double);
  void setYAxisRangeMax(double);

  void setXAxisLogScale(bool);
  void setYAxisLogScale(bool);

signals:

  /// Signal fired when the user adds a new plot series node to the scene
  /// using the widget's plot series selector.
  void seriesNodeAddedByUser(vtkDMMLNode*);

protected:
  QScopedPointer<qDMMLPlotChartPropertiesWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLPlotChartPropertiesWidget);
  Q_DISABLE_COPY(qDMMLPlotChartPropertiesWidget);
};

#endif
