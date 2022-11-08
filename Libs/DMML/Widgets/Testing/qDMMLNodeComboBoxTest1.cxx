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
#include "qDMMLNodeComboBox.h"
#include "qDMMLSceneFactoryWidget.h"

// DMML includes
#include <vtkDMMLNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include "qDMMLWidget.h"

// STD includes

int qDMMLNodeComboBoxTest1( int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  qDMMLNodeComboBox nodeSelector;

  // default state
  bool addEnabled = nodeSelector.addEnabled();
  bool removeEnabled = nodeSelector.removeEnabled();
  bool editEnabled = nodeSelector.editEnabled();
  // Add button should be enabled
  if (!(addEnabled && removeEnabled && !editEnabled))
    {
    std::cerr << __LINE__ << " - Incorrect state" << std::endl
                          << " Expected => Add[1], Remove[1], Edit[0]" << std::endl
                          << " Current => Add[" << addEnabled << "], "
                          << "Remove[" << removeEnabled << "], "
                          << "Edit[" << editEnabled << "]" << std::endl;
    return EXIT_FAILURE;
    }

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
  if (nodeSelector.nodeCount() != 0)
    {
    std::cerr << __LINE__ << " - qDMMLNodeSelector::count() failed: "
              << nodeSelector.nodeCount() << " nodes." <<std::endl;
    return EXIT_FAILURE;
    }

  sceneFactory.generateNode();
  sceneFactory.generateNode();
  sceneFactory.generateNode();
  sceneFactory.generateNode();
  sceneFactory.generateNode();

  // All the types are accepted when no type has been given.
  // the nodeselector may or may not contain nodes (some are hidden)
  /*if (nodeSelector.nodeCount())
    {
    std::cerr << __LINE__ << " - qDMMLNodeSelector::count() failed:"
              << nodeSelector.nodeCount() << " nodes instead of "
              << sceneFactory.dmmlScene()->GetNumberOfNodes() << std::endl;
    return EXIT_FAILURE;
    }
  */

  nodeSelector.setDMMLScene(nullptr);
  if (nodeSelector.dmmlScene() != nullptr)
    {
    std::cerr << __LINE__ << " - qDMMLNodeSelector::setDMMLScene() failed." << std::endl;
    return EXIT_FAILURE;
    }

  // test nodeType
  sceneFactory.generateScene();
  sceneFactory.generateNode("vtkDMMLViewNode");

  nodeSelector.setNodeTypes(QStringList("vtkDMMLViewNode"));
  nodeSelector.setNodeTypeLabel("3D view", "vtkDMMLViewNode");
  nodeSelector.setDMMLScene(sceneFactory.dmmlScene());
  if (nodeSelector.nodeCount() != 1)
    {
    std::cerr << __LINE__ << " - qDMMLNodeSelector: NodeType failed."
              << std::endl;
    return EXIT_FAILURE;
    }
  sceneFactory.generateNode("vtkDMMLViewNode");
  if (nodeSelector.nodeCount() != 2)
    {
    std::cerr << __LINE__ << " - qDMMLNodeSelector: NodeType failed."
              << std::endl;
    return EXIT_FAILURE;
    }
  sceneFactory.dmmlScene()->RemoveNode(sceneFactory.dmmlScene()->GetNthNode(0));
  if (nodeSelector.nodeCount() != 1)
    {
    std::cerr << __LINE__ << " - qDMMLNodeSelector: NodeType failed."
              << std::endl;
    return EXIT_FAILURE;
    }
  sceneFactory.dmmlScene()->RemoveNode(sceneFactory.dmmlScene()->GetNthNode(0));
  if (nodeSelector.nodeCount() != 0)
    {
    std::cerr << __LINE__ << " - qDMMLNodeSelector: NodeType failed."
              << std::endl;
    return EXIT_FAILURE;
    }
  sceneFactory.generateNode("vtkDMMLViewNode");
  if (nodeSelector.nodeCount() != 1)
    {
    std::cerr << __LINE__ << " - qDMMLNodeSelector: NodeType failed."
              << std::endl;
    return EXIT_FAILURE;
    }
  sceneFactory.deleteScene();
  if (nodeSelector.nodeCount() != 0)
    {
    std::cerr << __LINE__ << " - qDMMLNodeSelector: dmml scene events failed."
              << std::endl;
    return EXIT_FAILURE;
    }
  if (nodeSelector.currentNode() != nullptr)
    {
    std::cerr << __LINE__ << " - qDMMLNodeSelector: currentNode failed."
              << std::endl;
    return EXIT_FAILURE;
    }
  // FIXME: add more basic tests here

  // Check Attributes
  sceneFactory.generateScene();
  sceneFactory.generateNode("vtkDMMLViewNode");
  sceneFactory.generateNode("vtkDMMLViewNode");
  sceneFactory.generateNode("vtkDMMLViewNode");
  sceneFactory.generateNode("vtkDMMLViewNode");
  sceneFactory.generateNode("vtkDMMLViewNode");

  vtkDMMLNode* node = sceneFactory.dmmlScene()->GetNthNode(0);
  node->SetAttribute("foo", "bar");
  node = sceneFactory.dmmlScene()->GetNthNode(1);
  node->SetAttribute("foo", "bar2");

  nodeSelector.addAttribute("vtkDMMLViewNode", "foo", QString("bar2"));
  nodeSelector.setDMMLScene(sceneFactory.dmmlScene());
  if (nodeSelector.nodeCount() != 1)
    {
    std::cerr << __LINE__ << " - qDMMLNodeSelector: attribute filtering failed, expected 1 node with attribute foo set to bar2. nodeCount = "
              << nodeSelector.nodeCount() << std::endl;
    return EXIT_FAILURE;
    }

  // Check hide child node type
  sceneFactory.generateNode("vtkDMMLTransformNode");
  sceneFactory.generateNode("vtkDMMLTransformNode");
  sceneFactory.generateNode("vtkDMMLGridTransformNode");
  nodeSelector.setNodeTypes(QStringList("vtkDMMLTransformNode"));
  if (nodeSelector.nodeCount() != 3)
    {
    std::cerr << __LINE__ << " - qDMMLNodeSelector: node type filtering failed." << nodeSelector.nodeCount() << std::endl;
    return EXIT_FAILURE;
    }
  nodeSelector.setShowChildNodeTypes(false);
  if (nodeSelector.nodeCount() != 2)
    {
    std::cerr << __LINE__ << " - qDMMLNodeSelector: show child node types failed." << std::endl;
    return EXIT_FAILURE;
    }
  nodeSelector.setShowChildNodeTypes(true);
  if (nodeSelector.nodeCount() != 3)
    {
    std::cerr << __LINE__ << " - qDMMLNodeSelector: show child node types failed." << std::endl;
    return EXIT_FAILURE;
    }
  nodeSelector.setHideChildNodeTypes(QStringList("vtkDMMLGridTransformNode"));
  if (nodeSelector.nodeCount() != 2)
    {
    std::cerr << __LINE__ << " - qDMMLNodeSelector: show child node types failed." << std::endl;
    return EXIT_FAILURE;
    }

  // Checks with more than 1 type
  QStringList types;
  //types << "vtkDMMLViewNode" << "vtkDMMLCameraNode";
  // don't use the view node as that has an attribute filter on it
  types << "vtkDMMLModelNode" << "vtkDMMLCameraNode";
  //test setNodeTypes()/nodeTypes()
  nodeSelector.setNodeTypes(types);

  if (nodeSelector.nodeTypes() != types)
    {
    std::cerr << __LINE__ << " - qDMMLNodeSelector::setNodeTypes() failed." << std::endl;
    return EXIT_FAILURE;
    }

  sceneFactory.generateScene();
  sceneFactory.generateNode(types[0]);
  sceneFactory.generateNode(types[0]);
  sceneFactory.generateNode(types[0]);
  sceneFactory.generateNode("vtkDMMLLinearTransformNode");
  sceneFactory.generateNode(types[1]);
  sceneFactory.generateNode(types[1]);

  nodeSelector.setDMMLScene(sceneFactory.dmmlScene());
  // had added 5 nodes of the right type, and one extra
  if (nodeSelector.nodeCount() != 5)
    {
    std::cerr << __LINE__ << " - qDMMLNodeSelector:NodeTypeS: setDMMLScene fails, expected node count of 5, got " << nodeSelector.nodeCount() << std::endl;
    return EXIT_FAILURE;
    }
  // add another node of the right type, expect 6 now
  sceneFactory.generateNode(types[1]);
  if (nodeSelector.nodeCount() != 6)
    {
    std::cerr << __LINE__ << " - qDMMLNodeSelector:NodeTypeS: node added to the scene fails, expected nodeCount of 6, got " << nodeSelector.nodeCount() << std::endl;
    return EXIT_FAILURE;
    }

  nodeSelector.setDMMLScene(nullptr);

  currentCount = nodeSelector.nodeCount();
  if (currentCount != 0)
    {
    std::cerr << __LINE__ << " - Error in count() - Expected: 0, current:" << currentCount << std::endl;
    return EXIT_FAILURE;
    }

  //
  // Let's connect the sceneFactory with the widget
  //

  QObject::connect(&sceneFactory, SIGNAL(dmmlSceneChanged(vtkDMMLScene*)),
                   &nodeSelector, SLOT(setDMMLScene(vtkDMMLScene*)));


  // Let's check the state of the buttons

  sceneFactory.generateScene();

  // Test: setDMMLScene()/dmmlScene()
  nodeSelector.setDMMLScene(sceneFactory.dmmlScene());
  if (nodeSelector.dmmlScene() != sceneFactory.dmmlScene())
    {
    std::cerr << __LINE__ << " - qDMMLNodeSelector::setDMMLScene() failed." << std::endl;
    return EXIT_FAILURE;
    }

  sceneFactory.deleteScene();

  return EXIT_SUCCESS;
}
