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

==============================================================================*/

#ifndef __qDMMLAnnotationRulerProjectionPropertyWidget_h
#define __qDMMLAnnotationRulerProjectionPropertyWidget_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// Cjyx includes
#include "qDMMLWidget.h"

#include "qCjyxAnnotationsModuleWidgetsExport.h"

class qDMMLAnnotationRulerProjectionPropertyWidgetPrivate;
class vtkDMMLAnnotationRulerNode;

/// \ingroup Cjyx_QtModules_Annotations
class Q_CJYX_MODULE_ANNOTATIONS_WIDGETS_EXPORT
qDMMLAnnotationRulerProjectionPropertyWidget
  : public qDMMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qDMMLWidget Superclass;
  qDMMLAnnotationRulerProjectionPropertyWidget(QWidget *newParent = nullptr);
  ~qDMMLAnnotationRulerProjectionPropertyWidget() override;

public slots:
  void setDMMLRulerNode(vtkDMMLAnnotationRulerNode* rulerNode);
  void setProjectionVisibility(bool showProjection);
  void setUseRulerColor(bool useRulerColor);
  void setProjectionColor(QColor newColor);
  void setOverlineThickness(int thickness);
  void setUnderlineThickness(int thickness);
  void setColoredWhenParallel(bool coloredWhenParallel);
  void setThickerOnTop(bool thickerOnTop);
  void setDashed(bool dashed);

protected slots:
  void updateWidgetFromDisplayNode();

protected:
  QScopedPointer<qDMMLAnnotationRulerProjectionPropertyWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLAnnotationRulerProjectionPropertyWidget);
  Q_DISABLE_COPY(qDMMLAnnotationRulerProjectionPropertyWidget);

};

#endif
