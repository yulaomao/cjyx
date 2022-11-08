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

#ifndef __qDMMLPlotViewControllerWidget_h
#define __qDMMLPlotViewControllerWidget_h

// CTK includes
#include <ctkVTKObject.h>

// qDMMLWidget includes
#include "qDMMLViewControllerBar.h"
class qDMMLPlotViewControllerWidgetPrivate;
class qDMMLPlotView;

// DMML includes
class vtkDMMLPlotViewNode;

///
/// qDMMLPlotViewControllerWidget offers controls to a Plot view
/// (vtkDMMLPlotChartNode and vtkDMMLPlotSeriesNode). This controller
/// allows for the content (data) and style (properties) of a plot to
/// be defined.
class QDMML_WIDGETS_EXPORT qDMMLPlotViewControllerWidget
  : public qDMMLViewControllerBar
{
  Q_OBJECT
  QVTK_OBJECT

public:
  /// Superclass typedef
  typedef qDMMLViewControllerBar Superclass;

  /// Constructors
  explicit qDMMLPlotViewControllerWidget(QWidget* parent = nullptr);
  ~qDMMLPlotViewControllerWidget() override;

  /// Set the label for the Plot view (abbreviation for the view name).
  void setViewLabel(const QString& newViewLabel);

  /// Get the label for the view (abbreviation for the view name).
  QString viewLabel()const;

  /// Get PlotViewNode associated with this PlotViewController.
  Q_INVOKABLE vtkDMMLPlotViewNode* dmmlPlotViewNode() const;

public slots:
  /// Set the scene.
  void setDMMLScene(vtkDMMLScene* newScene) override;

  /// Set the PlotView with which this controller interacts.
  void setPlotView(qDMMLPlotView* PlotView);

  /// Set the PlotViewNode associated with this PlotViewController.
  /// PlotViewNodes are 1-to-1 with PlotViews
  void setDMMLPlotViewNode(vtkDMMLPlotViewNode* PlotViewNode);

  /// Adjust the chart viewer's field of view to match
  /// the extent of the chart axes.
  void fitPlotToAxes();

  /// Save the selected plot to a file
  void onExportButton();

protected slots:
  void updateWidgetFromDMMLView() override;
  void updateWidgetFromDMML();

private:
  Q_DECLARE_PRIVATE(qDMMLPlotViewControllerWidget);
  Q_DISABLE_COPY(qDMMLPlotViewControllerWidget);
};

#endif
