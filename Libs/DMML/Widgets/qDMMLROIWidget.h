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

#ifndef __qDMMLROIWidget_h
#define __qDMMLROIWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

#include "qDMMLWidgetsExport.h"

class vtkDMMLNode;
class vtkDMMLROINode;
class qDMMLROIWidgetPrivate;

class QDMML_WIDGETS_EXPORT qDMMLROIWidget : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  /// Constructors
  explicit qDMMLROIWidget(QWidget* parent);
  ~qDMMLROIWidget() override;

  /// Returns the current DMML ROI node
  vtkDMMLROINode* dmmlROINode()const;

  void setExtent(double min, double max);
public slots:
  /// Set the DMML node of interest
  void setDMMLROINode(vtkDMMLROINode* node);

  /// Utility function that calls setDMMLROINode(vtkDMMLROINode*)
  /// It's useful to connect to vtkDMMLNode* signals when you are sure of
  /// the type
  void setDMMLROINode(vtkDMMLNode* node);

  /// Turn on/off the visibility of the ROI node
  void setDisplayClippingBox(bool visible);

  /// Turn on/off the tracking mode of the sliders.
  /// The ROI node will be updated only when the slider handles are released.
  void setInteractiveMode(bool interactive);

protected slots:
  /// Internal function to update the widgets based on the ROI node
  void onDMMLNodeModified();
  /// Internal function to update the ROI node based on the sliders
  void updateROI();

protected:
  QScopedPointer<qDMMLROIWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLROIWidget);
  Q_DISABLE_COPY(qDMMLROIWidget);
};

#endif
