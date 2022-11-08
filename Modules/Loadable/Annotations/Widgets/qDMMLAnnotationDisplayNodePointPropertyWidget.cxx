// Qt includes
#include <QObject>

// qDMMLWidget includes
#include <qDMMLNodeComboBox.h>

#include "qDMMLAnnotationDisplayNodePointPropertyWidget.h"
#include "qDMMLAnnotationDisplayNodePointPropertyWidget_p.h"

#include "ui_qDMMLAnnotationDisplayNodePointPropertyWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_annotations


//-----------------------------------------------------------------------------
// qDMMLAnnotationDisplayNodePointPropertyWidgetPrivate methods

//-----------------------------------------------------------------------------
qDMMLAnnotationDisplayNodePointPropertyWidgetPrivate::qDMMLAnnotationDisplayNodePointPropertyWidgetPrivate(qDMMLAnnotationDisplayNodePointPropertyWidget& object) :
    q_ptr(&object)
{

}

//-----------------------------------------------------------------------------
void qDMMLAnnotationDisplayNodePointPropertyWidgetPrivate::setupUi(qDMMLAnnotationDisplayNodePropertyWidget * widget)
{
}

//-----------------------------------------------------------------------------
// qDMMLAnnotationDisplayNodePointPropertyWidget methods

//-----------------------------------------------------------------------------
qDMMLAnnotationDisplayNodePointPropertyWidget::qDMMLAnnotationDisplayNodePointPropertyWidget(QWidget *newParent) :
    Superclass(newParent), d_ptr(
        new qDMMLAnnotationDisplayNodePointPropertyWidgetPrivate(*this))
{
  Q_D(qDMMLAnnotationDisplayNodePointPropertyWidget);


}

//-----------------------------------------------------------------------------
qDMMLAnnotationDisplayNodePointPropertyWidget::~qDMMLAnnotationDisplayNodePointPropertyWidget()
{
}


//-----------------------------------------------------------------------------
void qDMMLAnnotationDisplayNodePointPropertyWidget::updateDMMLFromWidget()
{
  Q_D(qDMMLAnnotationDisplayNodePointPropertyWidget);

}

//-----------------------------------------------------------------------------
void qDMMLAnnotationDisplayNodePointPropertyWidget::updateWidgetFromDMML()
{
  Q_D(qDMMLAnnotationDisplayNodePointPropertyWidget);


}

