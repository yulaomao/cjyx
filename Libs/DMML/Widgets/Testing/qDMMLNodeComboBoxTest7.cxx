/*==============================================================================

  Program: 3D Cjyx

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Nicole Aucoin, BWH

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
#include <vtkDMMLScalarVolumeNode.h>

// VTK includes
#include <vtkNew.h>
#include "qDMMLWidget.h"


// test the filtering with many cases
int qDMMLNodeComboBoxTest7( int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  vtkNew<vtkDMMLScene> scene;

  vtkNew<vtkDMMLScalarVolumeNode> noAttributeNode;
  scene->AddNode(noAttributeNode.GetPointer());

  const char *testingAttributeName = "testingAttribute";
  const char *testingAttribute = noAttributeNode->GetAttribute(testingAttributeName);
  std::cout << "Volume node with no call to SetAttribute, GetAttribute returns " << (testingAttribute ? testingAttribute : "0") << "." << std::endl;

  vtkNew<vtkDMMLScalarVolumeNode> emptyStringAttributeNode;
  emptyStringAttributeNode->SetAttribute(testingAttributeName, "");
  scene->AddNode(emptyStringAttributeNode.GetPointer());

  testingAttribute = emptyStringAttributeNode->GetAttribute(testingAttributeName);
  std::cout << "Volume node with SetAttribute called with an empty string, GetAttribute returns " << (testingAttribute ? testingAttribute : "0") << "." << std::endl;

  vtkNew<vtkDMMLScalarVolumeNode> validAttributeNode;
  validAttributeNode->SetAttribute(testingAttributeName, "a");
  scene->AddNode(validAttributeNode.GetPointer());

  testingAttribute = validAttributeNode->GetAttribute(testingAttributeName);
  std::cout << "Volume node with SetAttribute called with 'a', GetAttribute returns " << (testingAttribute ? testingAttribute : "0") << "." << std::endl;

  // a node selector with no filtering attribute, three volumes should be
  // counted
  qDMMLNodeComboBox nodeSelector;
  nodeSelector.setNodeTypes(QStringList("vtkDMMLScalarVolumeNode"));
  nodeSelector.setDMMLScene(scene.GetPointer());

  int nodeCount = nodeSelector.nodeCount();
  if (nodeCount != 3)
    {
    std::cerr << "qDMMLNodeComboBox:: no filtering on attribute string doesn't yield 3 nodes, got nodeCount = " << nodeCount << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Passed with no filtering\n" << std::endl;
    }
  nodeSelector.show();

  // a node selector with a defined filtering attribute, only one volume
  // should be counted
  qDMMLNodeComboBox nodeSelectorA;
  nodeSelectorA.setNodeTypes(QStringList("vtkDMMLScalarVolumeNode"));
  nodeSelectorA.addAttribute("vtkDMMLScalarVolumeNode", testingAttributeName, "a");
  nodeSelectorA.setDMMLScene(scene.GetPointer());

  nodeCount = nodeSelectorA.nodeCount();
  if (nodeCount != 1)
    {
    std::cerr << "qDMMLNodeComboBox:: filtering on attribute string 'a' doesn't yield a single node, got nodeCount = " << nodeCount << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Passed filtering on 'a'\n" << std::endl;
    }
  nodeSelectorA.show();

  // a node selector with a defined filtering attribute that doesn't match any
  // volumes, count should be zero
  qDMMLNodeComboBox nodeSelectorB;
  nodeSelectorB.setNodeTypes(QStringList("vtkDMMLScalarVolumeNode"));
  nodeSelectorB.addAttribute("vtkDMMLScalarVolumeNode", testingAttributeName, "b");
  nodeSelectorB.setDMMLScene(scene.GetPointer());

  nodeCount = nodeSelectorB.nodeCount();
  if (nodeCount != 0)
    {
    std::cerr << "qDMMLNodeComboBox:: filtering on attribute string 'b' doesn't yield no nodes, got nodeCount = " << nodeCount << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Passed filtering on 'b'\n" << std::endl;
    }
  nodeSelectorB.show();

  // a node selector with an empty string as the filtering attribute, only one
  // volume should be counted
  qDMMLNodeComboBox nodeSelectorEmpty;
  nodeSelectorEmpty.setNodeTypes(QStringList("vtkDMMLScalarVolumeNode"));
  nodeSelectorEmpty.addAttribute("vtkDMMLScalarVolumeNode", testingAttributeName, "");
  nodeSelectorEmpty.setDMMLScene(scene.GetPointer());

  nodeCount = nodeSelectorEmpty.nodeCount();
  if (nodeCount != 1)
    {
    std::cerr << "qDMMLNodeComboBox:: filtering on attribute string '' doesn't yield a single node, got nodeCount = " << nodeCount << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Passed filtering on ''\n" << std::endl;
    }
  nodeSelectorEmpty.show();

  // a node selctor without a value for the filtering attribute, two volumes
  // should match (empty string and valid string)
  qDMMLNodeComboBox nodeSelectorNull;
  nodeSelectorNull.setNodeTypes(QStringList("vtkDMMLScalarVolumeNode"));
  nodeSelectorNull.addAttribute("vtkDMMLScalarVolumeNode", testingAttributeName);
  nodeSelectorNull.setDMMLScene(scene.GetPointer());

  nodeCount = nodeSelectorNull.nodeCount();
  if (nodeCount != 2)
    {
    std::cerr << "qDMMLNodeComboBox:: filtering on null attribute string doesn't yield two nodes, got nodeCount = " << nodeCount << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Passed filtering on null\n" << std::endl;
    }
  nodeSelectorNull.show();

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}
