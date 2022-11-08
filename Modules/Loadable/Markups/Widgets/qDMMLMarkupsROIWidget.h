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

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

#ifndef __qDMMLMarkupsROIWidget_h
#define __qDMMLMarkupsROIWidget_h

// Markups widgets includes
#include "qDMMLMarkupsAbstractOptionsWidget.h"
#include "qCjyxMarkupsModuleWidgetsExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// ------------------------------------------------------------------------------
class vtkDMMLNode;
class vtkDMMLMarkupsROINode;
class qDMMLMarkupsROIWidgetPrivate;

// ------------------------------------------------------------------------------
class Q_CJYX_MODULE_MARKUPS_WIDGETS_EXPORT qDMMLMarkupsROIWidget
: public qDMMLMarkupsAbstractOptionsWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qDMMLMarkupsAbstractOptionsWidget Superclass;
  qDMMLMarkupsROIWidget(QWidget* parent=nullptr);
  ~qDMMLMarkupsROIWidget() override;

  /// Returns the current DMML ROI node
  vtkDMMLMarkupsROINode* dmmlROINode()const;

  void setExtent(double min, double max);
  void setExtent(double minLR, double maxLR,
                 double minPA, double maxPA,
                 double minIS, double maxIS);

  /// Gets the name of the additional options widget type
  const QString className() const override {return "qDMMLMarkupsROIWidget";}

  /// Checks whether a given node can be handled by the widget
  bool canManageDMMLMarkupsNode(vtkDMMLMarkupsNode *markupsNode) const override;

  /// Get the inside out state.
  bool insideOut();

public slots:
  /// Turn on/off the visibility of the ROI node
  void setDisplayClippingBox(bool visible);

  /// Turn on/off the tracking mode of the sliders.
  /// The ROI node will be updated only when the slider handles are released.
  void setInteractiveMode(bool interactive);

  /// Turn on/off inside out state.
  void setInsideOut(bool insideOut);

  /// Updates the widget on DMML changes
  void updateWidgetFromDMML() override;

  /// Set the DMML node of interest
  void setDMMLMarkupsNode(vtkDMMLMarkupsNode* node) override;

  /// Returns an instance of the widget
  qDMMLMarkupsAbstractOptionsWidget* createInstance() const override
  { return new qDMMLMarkupsROIWidget(); }

signals:
  void displayClippingBoxChanged(bool);

protected slots:
  /// Internal function to update the ROI node based on the sliders
  void updateROI();
  /// Internal function to update the ROIDisplay node
  void onDMMLDisplayNodeModified();
  /// Internal function to update type of ROI
  void onROITypeParameterChanged();

protected:
  void setup();

protected:
  QScopedPointer<qDMMLMarkupsROIWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLMarkupsROIWidget);
  Q_DISABLE_COPY(qDMMLMarkupsROIWidget);
};

#endif
