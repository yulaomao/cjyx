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
#include "qDMMLNodeComboBox.h"

// DMML includes
#include <vtkDMMLNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkNew.h>
#include "qDMMLWidget.h"


int qDMMLNodeComboBoxTest4( int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  qDMMLNodeComboBox nodeSelector;
  nodeSelector.setNodeTypes(QStringList() << "vtkDMMLScalarVolumeNode" << "vtkDMMLLabelMapVolumeNode");
  nodeSelector.setBaseName("SomeBaseName");

  vtkNew<vtkDMMLScene> scene;
  nodeSelector.setDMMLScene(scene.GetPointer());

  vtkDMMLNode* node = nodeSelector.addNode();
  if (nodeSelector.nodeCount() != 1)
    {
    std::cerr << __LINE__ << "qDMMLNodeComboBox::addNode is broken" << std::endl;
    return EXIT_FAILURE;
    }

  node->SetName("foo");

  node = nodeSelector.addNode("vtkDMMLScalarVolumeNode");
  if (nodeSelector.nodeCount() != 2)
    {
    std::cerr << __LINE__ << "qDMMLNodeComboBox::addNode is broken" << std::endl;
    return EXIT_FAILURE;
    }
  if (strcmp(node->GetName(), "SomeBaseName_1")!=0)
    {
    std::cerr << __LINE__ << "qDMMLNodeComboBox::addNode is broken" << std::endl;
    return EXIT_FAILURE;
    }

  node = nodeSelector.addNode("vtkDMMLLabelMapVolumeNode");
  if (nodeSelector.nodeCount() != 3)
    {
    std::cerr << __LINE__ << "qDMMLNodeComboBox::addNode is broken" << std::endl;
    return EXIT_FAILURE;
    }
  if (strcmp(node->GetName(), "SomeBaseName_2")!=0)
    {
    std::cerr << __LINE__ << "qDMMLNodeComboBox::addNode is broken" << std::endl;
    return EXIT_FAILURE;
    }

  // Test that we cannot add a node type that is not among the list of allowed node types
  node = nodeSelector.addNode("vtkDMMLModelNode");
  if (nodeSelector.nodeCount() != 3)
    {
    std::cerr << __LINE__ << "qDMMLNodeComboBox::addNode is broken" << std::endl;
    return EXIT_FAILURE;
    }

  // Check if base name of a particular class can be changed
  nodeSelector.setBaseName("DifferentBaseName", "vtkDMMLLabelMapVolumeNode");
  node = nodeSelector.addNode("vtkDMMLLabelMapVolumeNode");
  if (strcmp(node->GetName(), "DifferentBaseName")!=0)
    {
    std::cerr << __LINE__ << "qDMMLNodeComboBox::addNode is broken" << std::endl;
    return EXIT_FAILURE;
    }

  nodeSelector.show();

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}
