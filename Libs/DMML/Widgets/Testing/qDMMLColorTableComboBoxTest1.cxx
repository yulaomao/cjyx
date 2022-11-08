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
#include "qDMMLColorTableComboBox.h"
#include "qDMMLSceneFactoryWidget.h"

// DMML includes

// VTK includes
#include "qDMMLWidget.h"

// STD includes

int qDMMLColorTableComboBoxTest1( int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  qDMMLColorTableComboBox nodeSelector;
  qDMMLSceneFactoryWidget sceneFactory;

  sceneFactory.generateScene();

  int currentCount = nodeSelector.nodeCount();
  if (currentCount != 0)
    {
    std::cerr << __LINE__ << " - Error in count() - Expected: 0, current:" << currentCount << std::endl;
    return EXIT_FAILURE;
    }

  // Test: setDMMLScene()/dmmlScene()
  nodeSelector.setDMMLScene(sceneFactory.dmmlScene());
  if (nodeSelector.dmmlScene() != sceneFactory.dmmlScene())
    {
    std::cerr << __LINE__ << " - qDMMLNodeSelector::setDMMLScene() failed." << std::endl;
    return EXIT_FAILURE;
    }
  // test a second time, just to make sure methods are well reinit.
  sceneFactory.generateScene();
  nodeSelector.setDMMLScene(sceneFactory.dmmlScene());
  if (nodeSelector.dmmlScene() != sceneFactory.dmmlScene())
    {
    std::cerr << __LINE__ << " - qDMMLNodeSelector::setDMMLScene() failed." << std::endl;
    return EXIT_FAILURE;
    }

  // no type has been given yet -> no item should be listed
  if (nodeSelector.nodeCount())
    {
    std::cerr << __LINE__ << " - qDMMLNodeSelector::count() failed: " << nodeSelector.nodeCount() << std::endl;
    return EXIT_FAILURE;
    }

  sceneFactory.deleteScene();

  return EXIT_SUCCESS;
}
