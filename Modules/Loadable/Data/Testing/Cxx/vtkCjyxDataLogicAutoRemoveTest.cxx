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

// Models logic
#include "vtkCjyxDataModuleLogic.h"

// DMML includes
#include <vtkDMMLModelDisplayNode.h>
#include <vtkDMMLModelNode.h>
#include <vtkDMMLModelStorageNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkNew.h>

bool testAutoRemoveModelFirst();
bool testAutoRemoveDisplayFirst();

//-----------------------------------------------------------------------------
int vtkCjyxDataLogicAutoRemoveTest( int , char * [] )
{
  bool res = true;
  res = testAutoRemoveModelFirst() && res;
  res = testAutoRemoveDisplayFirst() && res;
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//-----------------------------------------------------------------------------
bool testAutoRemoveModelFirst()
{
  vtkNew<vtkCjyxDataModuleLogic> modelsLogic;

  vtkNew<vtkDMMLScene> scene;
  modelsLogic->SetDMMLScene(scene.GetPointer());

  vtkNew<vtkDMMLModelDisplayNode> displayNode1;
  vtkNew<vtkDMMLModelDisplayNode> displayNode2;
  vtkNew<vtkDMMLModelStorageNode> storageNode;
  vtkNew<vtkDMMLModelNode> modelNode;

  scene->AddNode(displayNode1.GetPointer());
  scene->AddNode(displayNode2.GetPointer());
  scene->AddNode(storageNode.GetPointer());
  scene->AddNode(modelNode.GetPointer());

  modelNode->SetAndObserveDisplayNodeID(displayNode1->GetID());
  modelNode->AddAndObserveDisplayNodeID(displayNode2->GetID());
  modelNode->SetAndObserveStorageNodeID(storageNode->GetID());

  scene->RemoveNode(modelNode.GetPointer());

  // vtkCjyxDataModuleLogic should have remove the storage and display
  // nodes from the scene: no zombie.
  if (scene->IsNodePresent(displayNode1.GetPointer()) ||
      scene->IsNodePresent(displayNode2.GetPointer()) ||
      scene->IsNodePresent(storageNode.GetPointer()) ||
      displayNode1->GetScene() != nullptr ||
      displayNode2->GetScene() != nullptr ||
      storageNode->GetScene() != nullptr)
    {
    std::cout << __LINE__ << ": testAutoRemove() failed"
              << "Display 1: " << scene->IsNodePresent(
                displayNode1.GetPointer())
              << " " << displayNode1->GetScene() << std::endl
              << "Display 2: " << scene->IsNodePresent(
                displayNode2.GetPointer())
              << " " << displayNode2->GetScene() << std::endl
              << "Storage: " << scene->IsNodePresent(
                storageNode.GetPointer())
              << " " << storageNode->GetScene() << std::endl;
    return false;
    }

  return true;
}

//-----------------------------------------------------------------------------
bool testAutoRemoveDisplayFirst()
{
  std::cout << "Start RemoveModel display first" << std::endl;

  vtkNew<vtkCjyxDataModuleLogic> modelsLogic;

  vtkNew<vtkDMMLScene> scene;
  modelsLogic->SetDMMLScene(scene.GetPointer());

  vtkNew<vtkDMMLModelDisplayNode> displayNode1;
  vtkNew<vtkDMMLModelDisplayNode> displayNode2;
  vtkNew<vtkDMMLModelStorageNode> storageNode;
  vtkNew<vtkDMMLModelNode> modelNode;

  scene->AddNode(displayNode1.GetPointer());
  scene->AddNode(displayNode2.GetPointer());
  scene->AddNode(storageNode.GetPointer());
  scene->AddNode(modelNode.GetPointer());

  modelNode->SetAndObserveStorageNodeID(storageNode->GetID());
  modelNode->SetAndObserveDisplayNodeID(displayNode1->GetID());
  modelNode->AddAndObserveDisplayNodeID(displayNode2->GetID());
  std::cout << "Setup done" << std::endl;

  scene->RemoveNode(displayNode1.GetPointer());
  std::cout << "removed done" << std::endl;
  scene->RemoveNode(modelNode.GetPointer());

  // vtkCjyxDataModuleLogic should have remove the storage and display
  // nodes from the scene: no zombie.
  if (scene->IsNodePresent(displayNode1.GetPointer()) ||
      scene->IsNodePresent(displayNode2.GetPointer()) ||
      scene->IsNodePresent(storageNode.GetPointer()) ||
      displayNode1->GetScene() != nullptr ||
      displayNode2->GetScene() != nullptr ||
      storageNode->GetScene() != nullptr)
    {
    std::cout << __LINE__ << ": testAutoRemove() failed"
              << "Display 1: " << scene->IsNodePresent(
                displayNode1.GetPointer())
              << " " << displayNode1->GetScene() << std::endl
              << "Display 2: " << scene->IsNodePresent(
                displayNode2.GetPointer())
              << " " << displayNode2->GetScene() << std::endl
              << "Storage: " << scene->IsNodePresent(
                storageNode.GetPointer())
              << " " << storageNode->GetScene() << std::endl;
    return false;
    }

  return true;
}
