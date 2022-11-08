#ifndef __qDMMLAnnotationDisplayNodePointPropertyWidget_p_h
#define __qDMMLAnnotationDisplayNodePointPropertyWidget_p_h

// Qt includes
#include <QObject>

// CTK includes
#include <ctkPimpl.h>

// Annotations includes
#include "qDMMLAnnotationDisplayNodePointPropertyWidget.h"
#include "qDMMLAnnotationDisplayNodePropertyWidget.h"


#include "ui_qDMMLAnnotationDisplayNodePointPropertyWidget.h"

#include "qCjyxAnnotationsModuleWidgetsExport.h"

class qDMMLAnnotationDisplayNodePropertyWidgetPrivate;

class qDMMLAnnotationDisplayNodePointPropertyWidgetPrivate: public QObject,
    public Ui_qDMMLAnnotationDisplayNodePointPropertyWidget
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(qDMMLAnnotationDisplayNodePointPropertyWidget);
protected:
  qDMMLAnnotationDisplayNodePointPropertyWidget* const q_ptr;

public:

  qDMMLAnnotationDisplayNodePointPropertyWidgetPrivate(qDMMLAnnotationDisplayNodePointPropertyWidget& object);
  typedef qDMMLAnnotationDisplayNodePointPropertyWidgetPrivate Self;
  void setupUi(qDMMLAnnotationDisplayNodePropertyWidget * widget);

};

#endif
