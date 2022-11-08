
// Qt includes
#include <QApplication>
#include <QDebug>
#include <QTimer>

// Cjyx includes
#include "vtkCjyxConfigure.h"

// Annotations includes
#include "qDMMLAnnotationROIWidget.h"
#include "vtkDMMLAnnotationROINode.h"

// DMML includes
#include <vtkDMMLScene.h>

#include "qDMMLWidget.h"

//-----------------------------------------------------------------------------
int qDMMLAnnotationROIWidgetTest1(int argc, char * argv[] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  vtkSmartPointer<vtkDMMLScene> scene =
    vtkSmartPointer<vtkDMMLScene>::New();
  vtkSmartPointer<vtkDMMLAnnotationROINode> roi =
    vtkSmartPointer<vtkDMMLAnnotationROINode>::New();
  scene->AddNode(roi);

  qDMMLAnnotationROIWidget widget;
  widget.setDMMLAnnotationROINode(roi);

  qDebug() << "start edit";

  roi->SetXYZ(1, 1, 1);

  qDebug() << "end edit";

  widget.show();

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(100, &app, SLOT(quit()));
    }

  return app.exec();
}


