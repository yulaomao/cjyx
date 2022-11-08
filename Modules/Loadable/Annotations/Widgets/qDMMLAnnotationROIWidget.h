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

#ifndef __qDMMLAnnotationROIWidget_h
#define __qDMMLAnnotationROIWidget_h

// Qt includes
#include <QWidget>

// AnnotationWidgets includes
#include "qCjyxAnnotationsModuleWidgetsExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

class vtkDMMLNode;
class vtkDMMLAnnotationROINode;
class qDMMLAnnotationROIWidgetPrivate;

class Q_CJYX_MODULE_ANNOTATIONS_WIDGETS_EXPORT qDMMLAnnotationROIWidget : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  /// Constructors
  explicit qDMMLAnnotationROIWidget(QWidget* parent = nullptr);
  ~qDMMLAnnotationROIWidget() override;

  /// Returns the current DMML ROI node
  vtkDMMLAnnotationROINode* dmmlROINode()const;

  void setExtent(double min, double max);
  void setExtent(double minLR, double maxLR,
                 double minPA, double maxPA,
                 double minIS, double maxIS);
public slots:
  /// Set the DMML node of interest
  void setDMMLAnnotationROINode(vtkDMMLAnnotationROINode* node);

  /// Utility function that calls setDMMLAnnotationROINode(vtkDMMLAnnotationROINode*)
  /// It's useful to connect to vtkDMMLNode* signals when you are sure of
  /// the type
  void setDMMLAnnotationROINode(vtkDMMLNode* node);

  /// Turn on/off the visibility of the ROI node
  void setDisplayClippingBox(bool visible);

  /// Turn on/off the tracking mode of the sliders.
  /// The ROI node will be updated only when the slider handles are released.
  void setInteractiveMode(bool interactive);
signals:
  void displayClippingBoxChanged(bool);

protected slots:
  /// Internal function to update the widgets based on the ROI node
  void onDMMLNodeModified();
  /// Internal function to update the ROI node based on the sliders
  void updateROI();
  /// Internal function to update the ROIDisplay node
  void onDMMLDisplayNodeModified();

protected:
  QScopedPointer<qDMMLAnnotationROIWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLAnnotationROIWidget);
  Q_DISABLE_COPY(qDMMLAnnotationROIWidget);
};

#endif
