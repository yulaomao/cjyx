
// Qt includes
#include <QApplication>

// Cjyx includes
#include "vtkCjyxConfigure.h"

// DMML includes
#include <qDMMLMatrixWidget.h>
#include <vtkDMMLTransformNode.h>

// VTK includes
#include <vtkNew.h>
#include "qDMMLWidget.h"

int qDMMLMatrixWidgetTest1( int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  QWidget widget;

  qDMMLMatrixWidget   dmmlItem( &widget );

  vtkDMMLTransformNode* nullTransformNode = nullptr;

  {
    vtkDMMLTransformNode* expectedTransformNode = nullTransformNode;
    vtkDMMLTransformNode* currentTransformNode = dmmlItem.dmmlTransformNode();
    if (expectedTransformNode != currentTransformNode)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with dmmlTransformNode()\n"
                << "  expectedTransformNode:" << expectedTransformNode << "\n"
                << "  currentTransformNode:" << currentTransformNode << std::endl;
      return EXIT_FAILURE;
      }
  }

  dmmlItem.setDMMLTransformNode(nullTransformNode);

  {
    vtkDMMLTransformNode* expectedTransformNode = nullTransformNode;
    vtkDMMLTransformNode* currentTransformNode = dmmlItem.dmmlTransformNode();
    if (expectedTransformNode != currentTransformNode)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with dmmlTransformNode()\n"
                << "  expectedTransformNode:" << expectedTransformNode << "\n"
                << "  currentTransformNode:" << currentTransformNode << std::endl;
      return EXIT_FAILURE;
      }
  }

  vtkNew<vtkDMMLTransformNode> transformNode;
  dmmlItem.setDMMLTransformNode(transformNode.GetPointer());

  {
    vtkDMMLTransformNode* expectedTransformNode = transformNode.GetPointer();
    vtkDMMLTransformNode* currentTransformNode = dmmlItem.dmmlTransformNode();
    if (expectedTransformNode != currentTransformNode)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with dmmlTransformNode()\n"
                << "  expectedTransformNode:" << expectedTransformNode << "\n"
                << "  currentTransformNode:" << currentTransformNode << std::endl;
      return EXIT_FAILURE;
      }
  }

  dmmlItem.setDMMLTransformNode(nullTransformNode);

  {
    vtkDMMLTransformNode* expectedTransformNode = nullTransformNode;
    vtkDMMLTransformNode* currentTransformNode = dmmlItem.dmmlTransformNode();
    if (expectedTransformNode != currentTransformNode)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with dmmlTransformNode()\n"
                << "  expectedTransformNode:" << expectedTransformNode << "\n"
                << "  currentTransformNode:" << currentTransformNode << std::endl;
      return EXIT_FAILURE;
      }
  }

  return EXIT_SUCCESS;
}
