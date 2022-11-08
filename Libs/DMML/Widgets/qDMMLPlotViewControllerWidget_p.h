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

#ifndef __qDMMLPlotViewControllerWidget_p_h
#define __qDMMLPlotViewControllerWidget_p_h

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

// qDMML includes
#include "qDMMLPlotViewControllerWidget.h"
#include "qDMMLViewControllerBar_p.h"
#include "ui_qDMMLPlotViewControllerWidget.h"

// VTK includes
#include <vtkWeakPointer.h>

class QAction;
class qDMMLSceneViewMenu;
class vtkDMMLPlotViewNode;
class vtkDMMLPlotChartNode;
class QString;

//-----------------------------------------------------------------------------
class qDMMLPlotViewControllerWidgetPrivate
  : public qDMMLViewControllerBarPrivate
  , public Ui_qDMMLPlotViewControllerWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qDMMLPlotViewControllerWidget);

public:
  typedef qDMMLViewControllerBarPrivate Superclass;
  qDMMLPlotViewControllerWidgetPrivate(qDMMLPlotViewControllerWidget& object);
  ~qDMMLPlotViewControllerWidgetPrivate() override;

  void init() override;

  vtkWeakPointer<vtkDMMLPlotChartNode>   PlotChartNode;
  qDMMLPlotView*                         PlotView;

  QToolButton*                           FitToWindowToolButton;

  vtkDMMLPlotChartNode* GetPlotChartNodeFromView();

public slots:
  /// Called after a PlotChartNode is selected
  /// using the associated qDMMLNodeComboBox.
  void onPlotChartNodeSelected(vtkDMMLNode* node);

  /// Called after an PlotSeriesNode is selected
  /// using the associated qDMMLNodeComboBox.
  void onPlotSeriesNodesSelected();

  /// Called after a PlotSeriesNode is added
  /// using the associated qDMMLNodeComboBox.
  void onPlotSeriesNodeAdded(vtkDMMLNode* node);

  /// Called after a Plot type is selected using the qComboBox.
  /// Changes type of all associated plots.
  void onPlotTypeChanged(int);

  /// Called after interactino mode is changed by using the qComboBox.
  void onInteractionModeChanged(int);

protected:
  void setupPopupUi() override;

public:

};

#endif
