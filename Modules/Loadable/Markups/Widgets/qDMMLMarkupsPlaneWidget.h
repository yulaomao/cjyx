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

#ifndef __qDMMLMarkupsPlaneWidget_h
#define __qDMMLMarkupsPlaneWidget_h

// Qt includes
#include <QWidget>

// Markups widgets includes
#include "qDMMLMarkupsAbstractOptionsWidget.h"
#include "qCjyxMarkupsModuleWidgetsExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

class vtkDMMLAnnotationPlaneNode;
class vtkDMMLNode;
class vtkDMMLMarkupsPlaneNode;
class qDMMLMarkupsPlaneWidgetPrivate;

class Q_CJYX_MODULE_MARKUPS_WIDGETS_EXPORT qDMMLMarkupsPlaneWidget : public qDMMLMarkupsAbstractOptionsWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qDMMLMarkupsAbstractOptionsWidget Superclass;
  qDMMLMarkupsPlaneWidget(QWidget* parent=nullptr);
  ~qDMMLMarkupsPlaneWidget() override;

  /// Returns the current DMML Plane node
  vtkDMMLMarkupsPlaneNode* dmmlPlaneNode()const;

  /// Gets the name of the additional options widget type
  const QString className() const override { return "qDMMLMarkupsPlaneWidget"; }

  /// Checks whether a given node can be handled by the widget
  bool canManageDMMLMarkupsNode(vtkDMMLMarkupsNode *markupsNode) const override;

public slots:
  /// Updates the widget based on information from DMML.
  void updateWidgetFromDMML() override;

  /// Set the DMML node of interest
  void setDMMLMarkupsNode(vtkDMMLMarkupsNode* node) override;

  /// Returns an instance of the widget
  qDMMLMarkupsAbstractOptionsWidget* createInstance() const override
  {
    return new qDMMLMarkupsPlaneWidget();
  }

protected slots:
  /// Internal function to update type of Plane
  void onPlaneTypeIndexChanged();
  void onPlaneSizeModeIndexChanged();
  void onPlaneSizeSpinBoxChanged();
  void onPlaneBoundsSpinBoxChanged();
  void onNormalVisibilityCheckBoxChanged();
  void onNormalOpacitySliderChanged();

protected:
  void setup();

protected:
  QScopedPointer<qDMMLMarkupsPlaneWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLMarkupsPlaneWidget);
  Q_DISABLE_COPY(qDMMLMarkupsPlaneWidget);

};

#endif
