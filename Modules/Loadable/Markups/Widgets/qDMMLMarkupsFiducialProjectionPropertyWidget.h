/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Brigham and Women's Hospital

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Laurent Chauvin, Brigham and Women's
  Hospital. The project was supported by grants 5P01CA067165,
  5R01CA124377, 5R01CA138586, 2R44DE019322, 7R01CA124377,
  5R42CA137886, 5R42CA137886
  It was then updated for the Markups module by Nicole Aucoin, BWH.

==============================================================================*/

#ifndef __qDMMLMarkupsFiducialProjectionPropertyWidget_h
#define __qDMMLMarkupsFiducialProjectionPropertyWidget_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// Cjyx includes
#include "qDMMLWidget.h"

#include "qCjyxMarkupsModuleWidgetsExport.h"

class qDMMLMarkupsFiducialProjectionPropertyWidgetPrivate;
class vtkDMMLMarkupsDisplayNode;
class vtkDMMLMarkupsNode;

/// \ingroup Cjyx_QtModules_Markups
class Q_CJYX_MODULE_MARKUPS_WIDGETS_EXPORT
qDMMLMarkupsFiducialProjectionPropertyWidget
  : public qDMMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qDMMLWidget Superclass;
  qDMMLMarkupsFiducialProjectionPropertyWidget(QWidget *newParent = nullptr);
  ~qDMMLMarkupsFiducialProjectionPropertyWidget() override;

public slots:
  void setDMMLMarkupsNode(vtkDMMLMarkupsNode* markupsNode);
  void setDMMLMarkupsDisplayNode(vtkDMMLMarkupsDisplayNode* markupsDisplayNode);
  void setProjectionVisibility(bool showProjection);
  void setProjectionColor(QColor newColor);
  void setUseFiducialColor(bool useFiducialColor);
  void setOutlinedBehindSlicePlane(bool outlinedBehind);
  void setProjectionOpacity(double opacity);

protected slots:
  void updateWidgetFromDisplayNode();

protected:
  QScopedPointer<qDMMLMarkupsFiducialProjectionPropertyWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLMarkupsFiducialProjectionPropertyWidget);
  Q_DISABLE_COPY(qDMMLMarkupsFiducialProjectionPropertyWidget);

};

#endif
