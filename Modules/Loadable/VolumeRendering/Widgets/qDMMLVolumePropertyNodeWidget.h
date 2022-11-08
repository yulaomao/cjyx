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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qDMMLVolumePropertyNodeWidget_h
#define __qDMMLVolumePropertyNodeWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// VolumeRendering includes
#include "qCjyxVolumeRenderingModuleWidgetsExport.h"
class qDMMLVolumePropertyNodeWidgetPrivate;

// DMML includes
class vtkDMMLNode;
class vtkDMMLVolumePropertyNode;

// VTK includes
class vtkControlPointsItem;
class vtkVolumeProperty;

class Q_CJYX_MODULE_VOLUMERENDERING_WIDGETS_EXPORT qDMMLVolumePropertyNodeWidget
  : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(bool threshold READ hasThreshold WRITE setThreshold)

public:
  /// Constructors
  typedef QWidget Superclass;
  explicit qDMMLVolumePropertyNodeWidget(QWidget* parent=nullptr);
  ~qDMMLVolumePropertyNodeWidget() override;

  /// Return the VTK volume property associated with the DMML volume property
  /// node.
  vtkVolumeProperty* volumeProperty()const;

  ///
  /// Return the outer bounds of the opacity and color transfer function
  /// views.
  void chartsBounds(double bounds[4])const;
  void chartsExtent(double extent[4])const;

  void setThreshold(bool enable);
  bool hasThreshold()const;

public slots:
  /// Convenient function to connect with signal/slots
  void setDMMLVolumePropertyNode(vtkDMMLNode* node);
  void setDMMLVolumePropertyNode(vtkDMMLVolumePropertyNode* volumePropertyNode);

  void moveAllPoints(double x, double y = 0.,
                     bool dontMoveFirstAndLast = false);
  void spreadAllPoints(double factor,
                       bool dontSpreadFirstAndLast = false);

  /// Set chart extent
  void setChartsExtent(double extent[2]);
  void setChartsExtent(double min, double max);

signals:
  void thresholdChanged(bool enabled);
  void volumePropertyChanged();
  void chartsExtentChanged();

protected slots:
  void updateFromVolumePropertyNode();

protected:
  QScopedPointer<qDMMLVolumePropertyNodeWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLVolumePropertyNodeWidget);
  Q_DISABLE_COPY(qDMMLVolumePropertyNodeWidget);
};

#endif
