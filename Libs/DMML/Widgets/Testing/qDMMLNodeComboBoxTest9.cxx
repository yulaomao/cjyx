/*==============================================================================

  Program: 3D Cjyx

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Johan Andruejol, Kitware Inc.

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QTimer>

// Cjyx includes
#include "vtkCjyxConfigure.h"

// CTK includes
#include <ctkCoreTestingMacros.h>

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
int qDMMLNodeComboBoxTest9( int argc, char * argv [] )
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
  CHECK_NULL(testingAttribute);

  vtkNew<vtkDMMLScalarVolumeNode> emptyStringAttributeNode;
  emptyStringAttributeNode->SetAttribute(testingAttributeName, "");
  scene->AddNode(emptyStringAttributeNode.GetPointer());

  testingAttribute = emptyStringAttributeNode->GetAttribute(testingAttributeName);
  CHECK_STRING(testingAttribute, "");

  vtkNew<vtkDMMLScalarVolumeNode> validAttributeNode;
  validAttributeNode->SetAttribute(testingAttributeName, "a");
  scene->AddNode(validAttributeNode.GetPointer());

  testingAttribute = validAttributeNode->GetAttribute(testingAttributeName);
  CHECK_STRING(testingAttribute, "a");

  // a node selector with no filtering attribute, three volumes should be
  // counted
  qDMMLNodeComboBox nodeSelector;
  nodeSelector.setNodeTypes(QStringList("vtkDMMLScalarVolumeNode"));
  nodeSelector.setDMMLScene(scene.GetPointer());

  CHECK_INT(nodeSelector.nodeCount(), 3);
  QVariant filter = nodeSelector.sortFilterProxyModel()->attributeFilter("vtkDMMLScalarVolumeNode", testingAttributeName);
  CHECK_QVARIANT(filter, QVariant());
  std::cout << "Passed with no filtering\n" << std::endl;
  nodeSelector.show();

  // a node selector, remove any attribute
  qDMMLNodeComboBox nodeSelectorA;
  nodeSelectorA.setNodeTypes(QStringList("vtkDMMLScalarVolumeNode"));
  nodeSelectorA.removeAttribute("vtkDMMLScalarVolumeNode", testingAttributeName);
  nodeSelectorA.setDMMLScene(scene.GetPointer());

  CHECK_INT(nodeSelectorA.nodeCount(), 3);
  filter = nodeSelectorA.sortFilterProxyModel()->attributeFilter("vtkDMMLScalarVolumeNode", testingAttributeName);
  CHECK_QVARIANT(filter, QVariant());
  std::cout << "Passed with removing attribute before anything\n" << std::endl;
  nodeSelectorA.show();

  // a node selector with a defined filtering attribute that doesn't match any
  // volumes, count should be zero
  qDMMLNodeComboBox nodeSelectorB;
  nodeSelectorB.setNodeTypes(QStringList("vtkDMMLScalarVolumeNode"));
  nodeSelectorB.addAttribute("vtkDMMLScalarVolumeNode", testingAttributeName, "a");
  nodeSelectorB.setDMMLScene(scene.GetPointer());

  CHECK_INT(nodeSelectorB.nodeCount(), 1);

  filter = nodeSelectorB.sortFilterProxyModel()->attributeFilter("vtkDMMLScalarVolumeNode", testingAttributeName);
  CHECK_QVARIANT(filter, QVariant("a"));

  nodeSelectorB.removeAttribute("vtkDMMLScalarVolumeNode", testingAttributeName);
  CHECK_INT(nodeSelectorB.nodeCount(), 3);

  filter = nodeSelectorB.sortFilterProxyModel()->attributeFilter("vtkDMMLScalarVolumeNode", testingAttributeName);
  CHECK_QVARIANT(filter, QVariant());
  std::cout << "Passed with removing attribute after stuff happened\n" << std::endl;
  nodeSelectorB.show();

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}
