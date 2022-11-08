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

// CTK includes
#include <ctkModelTester.h>

// qDMML includes
#include "qDMMLSceneFactoryWidget.h"
#include "qDMMLSceneModel.h"

// VTK includes
#include "qDMMLWidget.h"


// STD includes

int qDMMLModelTest1(int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  try
    {
    qDMMLSceneModel model;
    ctkModelTester tester(&model);
    tester.setTestDataEnabled(false);

    qDMMLSceneFactoryWidget sceneFactory(nullptr);
    sceneFactory.generateScene();

    model.setDMMLScene(sceneFactory.dmmlScene());
    model.setDMMLScene(sceneFactory.dmmlScene());

    sceneFactory.generateNode();
    sceneFactory.deleteNode();

    sceneFactory.generateNode();
    sceneFactory.deleteNode();

    sceneFactory.generateNode();
    sceneFactory.generateNode();

    sceneFactory.deleteNode();
    sceneFactory.deleteNode();

    sceneFactory.generateNode();
    sceneFactory.deleteNode();

    sceneFactory.generateNode();
    sceneFactory.deleteNode();

    sceneFactory.generateNode();
    sceneFactory.generateNode();

    sceneFactory.deleteNode();
    sceneFactory.deleteNode();

    for( int i = 0; i < 100; ++i)
      {
      sceneFactory.deleteNode();
      }
    for( int i = 0; i < 100; ++i)
      {
      sceneFactory.generateNode();
      }
    for( int i = 0; i < 99; ++i)
      {
      sceneFactory.deleteNode();
      }
    }
  catch (const char* error)
    {
    std::cerr << error << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

