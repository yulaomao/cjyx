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

#ifndef __qDMMLPlotView_h
#define __qDMMLPlotView_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKChartView.h>

#include "qDMMLWidgetsExport.h"

class qDMMLPlotViewPrivate;

// DMML includes
class vtkDMMLPlotViewNode;
class vtkDMMLScene;

// VTK includes
class vtkCollection;
class vtkStringArray;

/// \brief qDMMLPlotView is the display canvas for a Plot.
///
/// qDMMLPlotView supports only 2D plots.
/// For extending this class to 3DPlots it is needed to expand the mother class
/// cktVTKChartView to use also vtkChartXYZ (currently exploiting only vtkChartXY).

class QDMML_WIDGETS_EXPORT qDMMLPlotView : public ctkVTKChartView
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef ctkVTKChartView Superclass;

  /// Constructors
  explicit qDMMLPlotView(QWidget* parent = nullptr);
  ~qDMMLPlotView() override;

  /// Return a pointer on the current DMML scene.
  vtkDMMLScene* dmmlScene() const;

  /// Get the PlotView node observed by view.
  vtkDMMLPlotViewNode* dmmlPlotViewNode()const;

  /// Redefine the sizeHint so layouts work properly.
  QSize sizeHint() const override;

public slots:

  /// Set the DMML \a scene that should be listened for events.
  void setDMMLScene(vtkDMMLScene* newScene);

  /// Set the current \a viewNode to observe.
  void setDMMLPlotViewNode(vtkDMMLPlotViewNode* newPlotViewNode);

  /// Change axis limits to show all content.
  void fitToContent();

  /// Unselect all the points
  void RemovePlotSelections();

  /// Save the current plot as vector graphics, in svg file format.
  /// Note that regardless of the file extension in the input fileName,
  /// the extension of the created file will always be ".svg".
  void saveAsSVG(const QString &fileName);

signals:

  /// When designing custom qDMMLWidget in the designer, you can connect the
  /// dmmlSceneChanged signal directly to the aggregated DMML widgets that
  /// have a setDMMLScene slot.
  void dmmlSceneChanged(vtkDMMLScene*);

  /// Signal emitted when a data point or more has been selected. Returns
  /// the DMMLPlotSeriesNodes IDs and the correspective arrays with
  /// the data points ids (vtkIdTypeArray).
  void dataSelected(vtkStringArray* dmmlPlotSeriesIDs, vtkCollection* selectionCol);

protected slots:

  void updateDMMLChartAxisRangeFromWidget();

protected:
  QScopedPointer<qDMMLPlotViewPrivate> d_ptr;

  /// Handle keyboard events
  void keyPressEvent(QKeyEvent* event) override;

  void keyReleaseEvent(QKeyEvent* event) override;

private:
  Q_DECLARE_PRIVATE(qDMMLPlotView);
  Q_DISABLE_COPY(qDMMLPlotView);
};

#endif
