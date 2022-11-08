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

#ifndef __qDMMLPlotView_p_h
#define __qDMMLPlotView_p_h

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

// Qt includes
class QToolButton;
#include <QMap>

// VTK includes
#include <vtkWeakPointer.h>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>
//class ctkPopupWidget;

// qDMML includes
#include "qDMMLPlotView.h"

// vtk includes
#include <vtkSmartPointer.h>
class vtkPlot;

class vtkDMMLPlotSeriesNode;
class vtkDMMLPlotViewNode;
class vtkDMMLPlotChartNode;
class vtkObject;
class vtkPlot;
class vtkStringArray;

//-----------------------------------------------------------------------------
class qDMMLPlotViewPrivate: public QObject
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qDMMLPlotView);
protected:
  qDMMLPlotView* const q_ptr;
public:
  qDMMLPlotViewPrivate(qDMMLPlotView& object);
  ~qDMMLPlotViewPrivate() override;

  virtual void init();

  void setDMMLScene(vtkDMMLScene* scene);
  vtkDMMLScene *dmmlScene();

  vtkDMMLPlotSeriesNode* plotSeriesNodeFromPlot(vtkPlot* plot);

  // Tries to update the existing plot. If returns nullptr then it means the existing plot must be deleted.
  // If returned plot differs from the existin plot, then existing plot must be replaced by the returned one.
  vtkSmartPointer<vtkPlot> updatePlotFromPlotSeriesNode(vtkDMMLPlotSeriesNode* plotSeriesNode, vtkPlot* existingPlot);

  // Adjust range to make it displayable with logarithmic scale
  void adjustRangeForLogScale(double range[2], double computedLimit[2]);

public slots:
  /// Handle DMML scene event
  void startProcessing();
  void endProcessing();

  void updateWidgetFromDMML();
  void onPlotChartNodeChanged();

  void RecalculateBounds();
  void switchInteractionMode();

  void emitSelection();

protected:

  vtkWeakPointer<vtkDMMLScene>         DMMLScene;
  vtkWeakPointer<vtkDMMLPlotViewNode>  DMMLPlotViewNode;
  vtkWeakPointer<vtkDMMLPlotChartNode> DMMLPlotChartNode;

  //QToolButton*                       PinButton;
//  ctkPopupWidget*                    PopupWidget;

  bool                               UpdatingWidgetFromDMML;

  QMap< vtkPlot*, QString > MapPlotToPlotSeriesNodeID;
};

#endif
