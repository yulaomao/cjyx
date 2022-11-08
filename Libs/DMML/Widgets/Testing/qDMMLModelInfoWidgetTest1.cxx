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

// qDMML includes
#include "qDMMLModelInfoWidget.h"

// DMML includes
#include <vtkDMMLModelNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include "qDMMLWidget.h"

// STD includes

int qDMMLModelInfoWidgetTest1(int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  vtkSmartPointer< vtkDMMLModelNode > modelNode = vtkSmartPointer< vtkDMMLModelNode >::New();

  vtkSmartPointer< vtkPolyData > polyData = vtkSmartPointer< vtkPolyData >::New();
  modelNode->SetAndObservePolyData(polyData);

  qDMMLModelInfoWidget modelInfo;
  modelInfo.setDMMLModelNode(modelNode);
  modelInfo.show();

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}

