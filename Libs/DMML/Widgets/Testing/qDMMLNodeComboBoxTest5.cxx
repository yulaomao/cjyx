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
#include <QSignalSpy>
#include <QTimer>

// Cjyx includes
#include "vtkCjyxConfigure.h"

// qDMML includes
#include "qDMMLNodeComboBox.h"

// DMML includes
#include <vtkDMMLCameraNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkNew.h>
#include "qDMMLWidget.h"

int qDMMLNodeComboBoxTest5( int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  qDMMLNodeComboBox nodeSelector;
  nodeSelector.setNodeTypes(QStringList("vtkDMMLCameraNode"));
  nodeSelector.setNoneEnabled(true);

  vtkNew<vtkDMMLScene> scene;

  vtkNew<vtkDMMLCameraNode> camNode;
  scene->AddNode(camNode.GetPointer());

  nodeSelector.setDMMLScene(scene.GetPointer());

  if (nodeSelector.currentNode() != nullptr)
    {
    std::cerr << "qDMMLNodeComboBox::setDMMLScene() failed: " << std::endl;
    return EXIT_FAILURE;
    }

  QSignalSpy spy(&nodeSelector, SIGNAL(currentNodeChanged(bool)));
  nodeSelector.setCurrentNode(camNode.GetPointer());
  if (spy.count() != 1)
    {
    std::cerr << "qDMMLNodeComboBox::setCurrentNode() failed: "
              << spy.count() << std::endl;
    return EXIT_FAILURE;
    }
  spy.clear();
  nodeSelector.setCurrentNode(nullptr);
  if (spy.count() != 1)
    {
    std::cerr << "qDMMLNodeComboBox::setCurrentNode() failed: "
              << spy.count() << std::endl;
    return EXIT_FAILURE;
    }
  spy.clear();
  nodeSelector.setCurrentNode(camNode.GetPointer());
  if (spy.count() != 1)
    {
    std::cerr << "qDMMLNodeComboBox::setCurrentNode() failed: "
              << spy.count() << std::endl;
    return EXIT_FAILURE;
    }

  nodeSelector.show();

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}
