#include <QApplication>

// Cjyx includes
#include "vtkCjyxConfigure.h"

// VTK includes
#include "qDMMLWidget.h"

// STD includes
#include <cstdlib>

int qDMMLWidgetsExportTest1( int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  // qDMMLWidgetsExport   dmmlItem;

  return EXIT_SUCCESS;
}
