
// Annotation includes
#include "qDMMLAnnotationDisplayNodePropertyWidget.h"


//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_Annotation
class qDMMLAnnotationDisplayNodePropertyWidgetPrivate
{
public:
  qDMMLAnnotationDisplayNodePropertyWidgetPrivate();

};

//-----------------------------------------------------------------------------
// qDMMLAnnotationDisplayNodePropertyWidgetPrivate methods

//-----------------------------------------------------------------------------
qDMMLAnnotationDisplayNodePropertyWidgetPrivate::qDMMLAnnotationDisplayNodePropertyWidgetPrivate()
{

}

//-----------------------------------------------------------------------------
// qDMMLAnnotationDisplayNodePropertyWidget methods

//-----------------------------------------------------------------------------
qDMMLAnnotationDisplayNodePropertyWidget::qDMMLAnnotationDisplayNodePropertyWidget(QWidget *newParent) :
    Superclass(newParent)
  , d_ptr(new qDMMLAnnotationDisplayNodePropertyWidgetPrivate())
{
}

//-----------------------------------------------------------------------------
qDMMLAnnotationDisplayNodePropertyWidget::~qDMMLAnnotationDisplayNodePropertyWidget()
{
}
