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
#include "qDMMLCheckableNodeComboBox.h"
#include "qDMMLSceneFactoryWidget.h"

// DMML includes

// VTK includes
#include "qDMMLWidget.h"

// STD includes

int qDMMLCheckableNodeComboBoxTest1( int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  qDMMLCheckableNodeComboBox nodeSelector;

  // Check default state
  bool addEnabled = nodeSelector.addEnabled();
  bool removeEnabled = nodeSelector.removeEnabled();
  bool editEnabled = nodeSelector.editEnabled();
  // Add button should be enabled
  if (addEnabled || removeEnabled || editEnabled)
    {
    std::cerr << __LINE__ << " - Incorrect state" << std::endl
                          << " Expected => Add[0], Remove[0], Edit[0]" << std::endl
                          << " Current => Add[" << addEnabled << "], "
                          << "Remove[" << removeEnabled << "], "
                          << "Edit[" << editEnabled << "]" << std::endl;
    return EXIT_FAILURE;
    }
  nodeSelector.setNodeTypes(QStringList("vtkDMMLViewNode"));

  qDMMLSceneFactoryWidget sceneFactory;
  sceneFactory.generateScene();
  sceneFactory.generateNode("vtkDMMLViewNode");
  sceneFactory.generateNode("vtkDMMLViewNode");

  nodeSelector.setDMMLScene(sceneFactory.dmmlScene());

  sceneFactory.generateNode("vtkDMMLViewNode");
  sceneFactory.generateNode("vtkDMMLViewNode");
  sceneFactory.generateNode("vtkDMMLViewNode");

  nodeSelector.show();

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}
