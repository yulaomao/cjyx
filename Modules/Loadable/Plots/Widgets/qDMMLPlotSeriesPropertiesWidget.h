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

#ifndef __qDMMLPlotSeriesPropertiesWidget_h
#define __qDMMLPlotSeriesPropertiesWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>

// Plots Widgets includes
#include "qCjyxPlotsModuleWidgetsExport.h"
#include "ui_qDMMLPlotSeriesPropertiesWidget.h"

class qDMMLPlotSeriesPropertiesWidgetPrivate;
class vtkDMMLNode;
class vtkDMMLPlotSeriesNode;

class vtkDMMLPlotViewLogic;

class Q_CJYX_MODULE_PLOTS_WIDGETS_EXPORT qDMMLPlotSeriesPropertiesWidget : public qDMMLWidget
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef qDMMLWidget Superclass;

  /// Constructors
  explicit qDMMLPlotSeriesPropertiesWidget(QWidget* parent = nullptr);
  ~qDMMLPlotSeriesPropertiesWidget() override;

  /// Get \a PlotViewNode
  vtkDMMLPlotSeriesNode* dmmlPlotSeriesNode()const;

public slots:

  /// Select plot series node to edit.
  void setDMMLPlotSeriesNode(vtkDMMLNode* node);

  /// Select plot series node to edit.
  void setDMMLPlotSeriesNode(vtkDMMLPlotSeriesNode* plotSeriesNode);

protected:
  QScopedPointer<qDMMLPlotSeriesPropertiesWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLPlotSeriesPropertiesWidget);
  Q_DISABLE_COPY(qDMMLPlotSeriesPropertiesWidget);
};

#endif
