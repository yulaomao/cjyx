#include <qDMMLLinearTransformSlider.h>
#include <QApplication>

// Cjyx includes
#include "vtkCjyxConfigure.h"

// VTK includes
#include "qDMMLWidget.h"

// STD includes

int qDMMLLinearTransformSliderTest1( int argc , char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  QWidget widget;

  qDMMLLinearTransformSlider   dmmlItem( &widget );

  return EXIT_SUCCESS;
}
