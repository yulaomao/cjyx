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

#ifndef __qDMMLPlotWidget_h
#define __qDMMLPlotWidget_h

// Qt includes
#include <QWidget>
class QResizeEvent;

// qDMMLWidget includes
#include "qDMMLWidget.h"
class qDMMLPlotViewControllerWidget;
class qDMMLPlotView;
class qDMMLPlotWidgetPrivate;

// DMML includes
class vtkDMMLPlotViewNode;
class vtkDMMLScene;

/// \brief qDMMLPlotWidget is the toplevel Plotting widget that can be
/// packed in a layout.
///
/// qDMMLPlotWidget provides plotting capabilities with a display
/// canvas for the plot and a controller widget to control the
/// content and properties of the plot.
class QDMML_WIDGETS_EXPORT qDMMLPlotWidget : public qDMMLWidget
{
  Q_OBJECT
  Q_PROPERTY(QString viewLabel READ viewLabel WRITE setViewLabel)
public:
  /// Superclass typedef
  typedef qDMMLWidget Superclass;

  /// Constructors
  explicit qDMMLPlotWidget(QWidget* parent = nullptr);
  ~qDMMLPlotWidget() override;

  /// Get the Plot node observed by view.
  Q_INVOKABLE vtkDMMLPlotViewNode* dmmlPlotViewNode()const;

  /// Get a reference to the underlying Plot View
  /// Becareful if you change the PlotView, you might
  /// unsynchronize the view from the nodes/logics.
  Q_INVOKABLE qDMMLPlotView* plotView()const;

    /// Get plot view controller widget
  Q_INVOKABLE qDMMLPlotViewControllerWidget* plotController()const;

  /// Get the view label for the Plot.
  /// \sa qDMMLPlotControllerWidget::PlotViewLabel()
  /// \sa setPlotViewLabel()
  QString viewLabel()const;

  /// Set the view label for the Plot.
  /// \sa qDMMLPlotControllerWidget::PlotViewLabel()
  /// \sa PlotViewLabel()
  void setViewLabel(const QString& newPlotViewLabel);

public slots:
  /// Set the current \a viewNode to observe
  void setDMMLPlotViewNode(vtkDMMLPlotViewNode* newPlotViewNode);

protected:
  QScopedPointer<qDMMLPlotWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLPlotWidget);
  Q_DISABLE_COPY(qDMMLPlotWidget);
};

#endif
