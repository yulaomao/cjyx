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

// Cjyx includes
#include "vtkCjyxConfigure.h"

// qDMML includes
#include <qDMMLSceneFactoryWidget.h>

// DMML includes
#include <vtkDMMLScene.h>

// VTK includes
#include "qDMMLWidget.h"

// STD includes

int qDMMLSceneFactoryWidgetTest1( int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  qDMMLSceneFactoryWidget   sceneFactory;
  sceneFactory.generateScene();
  if (sceneFactory.dmmlScene() == nullptr)
    {
    std::cerr << "qDMMLSceneFactoryWidget::generateScene() failed" << std::endl;
    return EXIT_FAILURE;
    }
  if (sceneFactory.dmmlScene()->GetNumberOfNodes() != 0)
    {
    std::cerr << "qDMMLSceneFactoryWidget::generateScene() failed" << std::endl;
    return EXIT_FAILURE;
    }
  sceneFactory.generateScene();
  if (sceneFactory.dmmlScene() == nullptr)
    {
    std::cerr << "qDMMLSceneFactoryWidget::generateScene() failed" << std::endl;
    return EXIT_FAILURE;
    }
  if (sceneFactory.dmmlScene()->GetNumberOfNodes() != 0)
    {
    std::cerr << "qDMMLSceneFactoryWidget::generateScene() failed" << std::endl;
    return EXIT_FAILURE;
    }
  // delete a node that doesn't exist: should not crash
  sceneFactory.deleteNode();
  sceneFactory.generateNode();
  if (sceneFactory.dmmlScene()->GetNumberOfNodes() != 1)
    {
    std::cerr << "qDMMLSceneFactoryWidget::generateScene() failed" << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
