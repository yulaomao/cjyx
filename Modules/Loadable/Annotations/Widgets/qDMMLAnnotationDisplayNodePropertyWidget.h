
#ifndef __qDMMLAnnotationDisplayNodePropertyWidget_h
#define __qDMMLAnnotationDisplayNodePropertyWidget_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// Cjyx includes
#include "qDMMLWidget.h"

#include "qCjyxAnnotationsModuleWidgetsExport.h"

class qDMMLAnnotationDisplayNodePropertyWidgetPrivate;

/// \ingroup Cjyx_QtModules_Annotations
class Q_CJYX_MODULE_ANNOTATIONS_WIDGETS_EXPORT qDMMLAnnotationDisplayNodePropertyWidget : public qDMMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qDMMLWidget Superclass;
  qDMMLAnnotationDisplayNodePropertyWidget(QWidget *newParent = 0);
  virtual ~qDMMLAnnotationDisplayNodePropertyWidget();

protected:
  QScopedPointer<qDMMLAnnotationDisplayNodePropertyWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLAnnotationDisplayNodePropertyWidget);
  Q_DISABLE_COPY(qDMMLAnnotationDisplayNodePropertyWidget);

};

#endif
