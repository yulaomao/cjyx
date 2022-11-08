/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

#include "vtkDMMLScene.h"
#include "vtkDMMLTableNode.h"
#include "vtkDMMLTableViewNode.h"

#include "vtkDMMLCoreTestingMacros.h"

int vtkDMMLTableViewNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLTableViewNode> node1;
  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());

  // Check if modified eventes are only fired if and only if table node ID is changed

  vtkNew<vtkDMMLScene> scene;
  vtkNew<vtkDMMLTableNode> tableNode1;
  vtkNew<vtkDMMLTableNode> tableNode2;
  scene->AddNode(tableNode1.GetPointer() );
  scene->AddNode(tableNode2.GetPointer());

  vtkNew<vtkDMMLCoreTestingUtilities::vtkDMMLNodeCallback> callback;
  node1->AddObserver(vtkCommand::AnyEvent, callback.GetPointer());

  callback->ResetNumberOfEvents();
  node1->SetTableNodeID(tableNode1->GetID());
  CHECK_INT(callback->GetNumberOfModified(),1);

  callback->ResetNumberOfEvents();
  node1->SetTableNodeID(tableNode2->GetID());
  CHECK_INT(callback->GetNumberOfModified(),1);

  callback->ResetNumberOfEvents();
  node1->SetTableNodeID(tableNode2->GetID());
  CHECK_INT(callback->GetNumberOfModified(),0);

  callback->ResetNumberOfEvents();
  node1->SetTableNodeID(nullptr);
  CHECK_INT(callback->GetNumberOfModified(),1);

  callback->ResetNumberOfEvents();
  node1->SetTableNodeID(nullptr);
  CHECK_INT(callback->GetNumberOfModified(),0);

  std::cout << "vtkDMMLTableViewNodeTest1 completed successfully" << std::endl;
  return EXIT_SUCCESS;
}
