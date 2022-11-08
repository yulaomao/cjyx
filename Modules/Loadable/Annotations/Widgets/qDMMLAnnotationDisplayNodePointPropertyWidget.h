
#ifndef __qDMMLAnnotationDisplayNodePointPropertyWidget_h
#define __qDMMLAnnotationDisplayNodePointPropertyWidget_h

// CTK includes
#include <ctkPimpl.h>

// Annotations includes
#include "qDMMLAnnotationDisplayNodePropertyWidget.h"


#include "qCjyxAnnotationsModuleWidgetsExport.h"

class qDMMLAnnotationDisplayNodePointPropertyWidgetPrivate;
class vtkDMMLVolumeNode;

/// \ingroup Cjyx_QtModules_Annotations
class Q_CJYX_MODULE_ANNOTATIONS_WIDGETS_EXPORT qDMMLAnnotationDisplayNodePointPropertyWidget :
    public qDMMLAnnotationDisplayNodePropertyWidget
{
  Q_OBJECT

public:

  typedef qDMMLAnnotationDisplayNodePropertyWidget Superclass;
  explicit qDMMLAnnotationDisplayNodePointPropertyWidget(QWidget *newParent = 0);
  virtual ~qDMMLAnnotationDisplayNodePointPropertyWidget();

public slots:

  void updateWidgetFromDMML();

  void updateDMMLFromWidget();

private slots:



protected:
  QScopedPointer<qDMMLAnnotationDisplayNodePointPropertyWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLAnnotationDisplayNodePointPropertyWidget);
  Q_DISABLE_COPY(qDMMLAnnotationDisplayNodePointPropertyWidget);
  typedef qDMMLAnnotationDisplayNodePointPropertyWidgetPrivate ctkPimpl;
  bool updating;

};

#endif // __qDMMLAnnotationDisplayNodePointPropertyWidget_h
