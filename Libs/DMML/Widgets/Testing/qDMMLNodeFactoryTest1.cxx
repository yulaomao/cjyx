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

  This file was originally developed by Luis Ibanez, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QApplication>

// Cjyx includes
#include "vtkCjyxConfigure.h"

// DMMLWidgets includes
#include <qDMMLNodeFactory.h>

// DMML includes
#include <vtkDMMLNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkNew.h>
#include "qDMMLWidget.h"

// STD includes
#include <cstdlib>

int qDMMLNodeFactoryTest1( int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  qDMMLNodeFactory nodeFactory;

  // Check default values
  if (nodeFactory.dmmlScene() != nullptr ||
      nodeFactory.createNode("vtkDMMLCameraNode") != nullptr)
    {
    std::cerr << "Line " << __LINE__ << " - qDMMLNodeFactory wrong default values" << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkDMMLScene> scene;

  {
    nodeFactory.setDMMLScene(scene.GetPointer());

    if (nodeFactory.dmmlScene() != scene.GetPointer())
      {
      std::cerr << "Line " << __LINE__ << " - qDMMLNodeFactory::setDMMLScene() failed" << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    vtkDMMLNode* createdEmptyNode = nodeFactory.createNode("");
    if (createdEmptyNode != nullptr)
      {
      std::cerr << "Line " << __LINE__ << " - qDMMLNodeFactory::createNode() created a bad node" << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    // Test a simple node
    vtkDMMLNode* createdNode = nodeFactory.createNode("vtkDMMLCameraNode");
    if (createdNode == nullptr ||
        createdNode->IsA("vtkDMMLCameraNode") != 1 ||
        createdNode->GetReferenceCount() != 2 ||
        scene->IsNodePresent(createdNode) == 0)
      {
      std::cerr << "Line " << __LINE__ << " - qDMMLNodeFactory::createNode() failed.\n"
                << " node: " << createdNode << "\n"
                << " class: " << createdNode->IsA("vtkDMMLCameraNode") << "\n"
                << " refCount: " << createdNode->GetReferenceCount() << "\n"
                << " present: " << scene->IsNodePresent(createdNode) << "\n"
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    // Test a singleton node
    vtkDMMLNode* createdSingletonNode =
      nodeFactory.createNode("vtkDMMLInteractionNode");

    if (createdSingletonNode == nullptr ||
        createdSingletonNode->IsA("vtkDMMLInteractionNode") != 1 ||
        createdSingletonNode->GetReferenceCount() != 2 ||
        scene->IsNodePresent(createdSingletonNode) == 0)
      {
      std::cerr << "Line " << __LINE__ << " - qDMMLNodeFactory::createNode() failed with singleton.\n"
                << " node: " << createdSingletonNode << "\n"
                << " class: " << createdSingletonNode->IsA("vtkDMMLInteractionNode") << "\n"
                << " refCount: " << createdSingletonNode->GetReferenceCount() << "\n"
                << " present: " << scene->IsNodePresent(createdSingletonNode) << "\n"
                << std::endl;
      return EXIT_FAILURE;
      }

    // Test another singleton
    vtkDMMLNode* createdSingletonNode2 =
      nodeFactory.createNode("vtkDMMLInteractionNode");

    // Adding the same singleton in the scene should copy the properties of the
    // node to add in the existing node. \sa vtkDMMLScene::AddNode
    if (createdSingletonNode2 == nullptr ||
        createdSingletonNode2 != createdSingletonNode ||
        createdSingletonNode2->IsA("vtkDMMLInteractionNode") != 1 ||
        createdSingletonNode2->GetReferenceCount() != 2 ||
        scene->IsNodePresent(createdSingletonNode2) == 0)
      {
      std::cerr << "Line " << __LINE__ << " - qDMMLNodeFactory::createNode() failed with singleton2.\n"
                << " node: " << createdSingletonNode << " / " << createdSingletonNode2 << "\n"
                << " class: " << createdSingletonNode2->IsA("vtkDMMLInteractionNode") << "\n"
                << " refCount: " << createdSingletonNode2->GetReferenceCount() << "\n"
                << " present: " << scene->IsNodePresent(createdSingletonNode2) << "\n"
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    // Test static utility method
    vtkDMMLNode* createdNodeStatic =
      qDMMLNodeFactory::createNode(scene.GetPointer(), "vtkDMMLCameraNode");
    if (createdNodeStatic == nullptr ||
        createdNodeStatic->IsA("vtkDMMLCameraNode") != 1 ||
        createdNodeStatic->GetReferenceCount() != 2 ||
        scene->IsNodePresent(createdNodeStatic) == 0)
      {
      std::cerr << "Line " << __LINE__ << " - qDMMLNodeFactory::createNode() failed.\n"
                << " node: " << createdNodeStatic << "\n"
                << " class: " << createdNodeStatic->IsA("vtkDMMLCameraNode") << "\n"
                << " refCount: " << createdNodeStatic->GetReferenceCount() << "\n"
                << " present: " << scene->IsNodePresent(createdNodeStatic) << "\n"
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    // Test attributes
    nodeFactory.addAttribute("attribute1", "value1");
    nodeFactory.addAttribute("attribute2", "value2");
    nodeFactory.removeAttribute("attribute2");
    nodeFactory.removeAttribute("attribute0");

    if (nodeFactory.attribute("attribute1") != "value1" ||
        (nodeFactory.attribute("attribute2").isNull() != true) ||
        (nodeFactory.attribute("attribute0").isNull() != true))
      {
      std::cerr << "Line " << __LINE__ << " - qDMMLNodeFactory::addAttribute failed:"
                << " attribute1: " << qPrintable(nodeFactory.attribute("attribute1"))
                << " attribute2: " << qPrintable(nodeFactory.attribute("attribute2"))
                << " attribute0: " << qPrintable(nodeFactory.attribute("attribute0"))
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    // Test createNode with attribute
    vtkDMMLNode* createdNodeWithAttribute1 =
      nodeFactory.createNode("vtkDMMLCameraNode");

    if (createdNodeWithAttribute1 == nullptr ||
        strcmp(createdNodeWithAttribute1->GetAttribute("attribute1"), "value1") != 0 ||
        createdNodeWithAttribute1->GetAttribute("attribute2") != nullptr)
      {
      std::cerr << "Line " << __LINE__ << " - qDMMLNodeFactory::createNode() with attribute failed." << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    // Test basename
    nodeFactory.setBaseName("vtkDMMLCameraNode", "MyBaseName");
    vtkDMMLNode* createdNodeWithBaseName =
      nodeFactory.createNode("vtkDMMLCameraNode");
    vtkDMMLNode* createdNodeWithoutBaseName =
      nodeFactory.createNode("vtkDMMLColorTableNode");
    if (nodeFactory.baseName("vtkDMMLCameraNode") != "MyBaseName" ||
        nodeFactory.baseName("vtkDMMLColorTableNode").isNull() != true ||
        strcmp(createdNodeWithBaseName->GetName(), "MyBaseName") != 0 ||
        strcmp(createdNodeWithoutBaseName->GetName(), "MyBaseName") == 0)
      {
      std::cerr << "Line " << __LINE__ << " - qDMMLFactory::setBaseName failed." << std::endl;
      return EXIT_FAILURE;
      }
  }
  return EXIT_SUCCESS;
}
