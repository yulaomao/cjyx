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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// DMML includes
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLUnitNode.h"

// VTK includes
#include <vtkNew.h>

// STD includes
#include <vector>

const size_t NUMBER_OF_UNITS = 5;
const char* UNITS[NUMBER_OF_UNITS][2] = {{"length", "m"},
                                         {"length", "km"},
                                         {"energy", "J"},
                                         {"luminous_intensity", "cd"},
                                         {"energy", "eV"}
                                        };

//---------------------------------------------------------------------------
vtkDMMLScene* CreatePopulatedScene();
bool TestScenesUnitNodeID(vtkDMMLScene* scene);
bool TestUnitNodeAttribute(vtkDMMLScene* scene);

//---------------------------------------------------------------------------
int vtkDMMLUnitNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLUnitNode> node1;
  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());

  bool res = true;
  vtkDMMLScene* scene = CreatePopulatedScene();
  res = res && TestScenesUnitNodeID(scene);
  res = res && TestUnitNodeAttribute(scene);

  scene->Delete();
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//---------------------------------------------------------------------------
vtkDMMLScene* CreatePopulatedScene()
{
  vtkDMMLScene* scene = vtkDMMLScene::New();
  for (size_t i = 0; i < NUMBER_OF_UNITS; ++i)
    {
    vtkNew<vtkDMMLUnitNode> unit;
    unit->SetQuantity(UNITS[i][0]);
    unit->SetName(UNITS[i][1]);

    scene->AddNode(unit.GetPointer());
    }
  return scene;
}

//---------------------------------------------------------------------------
bool TestScenesUnitNodeID(vtkDMMLScene* scene)
{
  assert(scene);

  std::vector<vtkDMMLNode*> units;
  scene->GetNodesByClass("vtkDMMLUnitNode", units);

  if (units.size() != NUMBER_OF_UNITS)
    {
    std::cerr<<"Did not find the right number of nodes in the scene!"<<std::endl;
    return false;
    }

  for (size_t i = 0; i < units.size(); ++i)
    {
    std::string id = "vtkDMMLUnitNode";
    id += UNITS[i][1];
    if (id.compare(units[i]->GetID()) != 0)
      {
      std::cerr<<"Did not find the right ID !\n"
        "expected: "<<id<<" got: "<<units[i]->GetID()<<std::endl;
      return false;
      }
    }

  return true;
}

//---------------------------------------------------------------------------
bool TestUnitNodeAttribute(vtkDMMLScene* scene)
{
  assert(scene);
  int testedNode = 3;
  vtkDMMLNode* unit = scene->GetNthNodeByClass(testedNode, "vtkDMMLUnitNode");
  if (!unit)
    {
    std::cerr<<"Did not find correct node in the scene!"<<std::endl;
    return false;
    }

  bool foundQuantityAttribute = false;
  std::vector<std::string> attributes = unit->GetAttributeNames();
  for (size_t i = 0; i < attributes.size(); ++i)
    {
    if (attributes[i].compare("Quantity") == 0)
      {
      foundQuantityAttribute = true;
      break;
      }
    }

  if (!foundQuantityAttribute)
    {
    std::cerr<<"Did not find attribute Quantity in the node!"<<std::endl;
    return false;
    }

  return foundQuantityAttribute;
}
