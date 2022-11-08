/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QTimer>

// Cjyx includes
#include "vtkCjyxConfigure.h"

// Models includes
#include <ctkVTKDataSetArrayComboBox.h>

// DMML includes
#include <vtkDMMLModelDisplayNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include "qDMMLWidget.h"

// STD includes

int qDMMLModelDisplayNodeWidgetTest2( int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  if (argc < 2)
    {
    std::cerr << "Usage: qDMMLModelDisplayNodeWidgetTest2 <dmmlScenePath>" << std::endl;
    return EXIT_FAILURE;
    }

  vtkSmartPointer<vtkDMMLScene> scene = vtkSmartPointer<vtkDMMLScene>::New();
  scene->SetURL(argv[1]);
  scene->Connect();

  vtkDMMLModelDisplayNode* modelDisplayNode = vtkDMMLModelDisplayNode::SafeDownCast(
    scene->GetFirstNodeByClass("vtkDMMLModelDisplayNode"));

  if (!modelDisplayNode)
    {
    std::cerr << "Scene: " << argv[1] << " must contain a"
              << " vtkDMMLModelDisplayNode" << std::endl;
    return EXIT_FAILURE;
    }

  //qDMMLModelDisplayNodeWidget modelDisplayNodeWidget;
  //modelDisplayNodeWidget.setDMMLModelDisplayNode(modelDisplayNode);
  //modelDisplayNodeWidget.show();
  ctkVTKDataSetArrayComboBox dataSetModel;
  dataSetModel.setDataSet(modelDisplayNode->GetInputMesh());

  if (argc < 3 || QString(argv[2]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}
