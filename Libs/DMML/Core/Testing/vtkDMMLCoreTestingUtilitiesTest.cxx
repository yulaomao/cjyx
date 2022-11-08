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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 1U24CA194354-01

==============================================================================*/

// DMML includes
#include "vtkDMMLCoreTestingUtilities.h"
#include "vtkDMMLNode.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

using namespace vtkDMMLCoreTestingUtilities;

//----------------------------------------------------------------------------
bool TestCheckNodeInSceneByID();
bool TestCheckNodeIdAndName();

//----------------------------------------------------------------------------
int vtkDMMLCoreTestingUtilitiesTest(int , char * [] )
{
  bool res = true;
  res = res && TestCheckNodeInSceneByID();
  res = res && TestCheckNodeIdAndName();
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

namespace
{

//---------------------------------------------------------------------------
class vtkDMMLCoreTestingUtilitiesNode : public vtkDMMLNode
{
public:
  static vtkDMMLCoreTestingUtilitiesNode *New();
  vtkTypeMacro(vtkDMMLCoreTestingUtilitiesNode, vtkDMMLNode);
  vtkDMMLNode* CreateNodeInstance() override
    {
    return vtkDMMLCoreTestingUtilitiesNode::New();
    }
  const char* GetNodeTagName() override
    {
    return "Testing";
    }
private:
  vtkDMMLCoreTestingUtilitiesNode() = default;
};
vtkStandardNewMacro(vtkDMMLCoreTestingUtilitiesNode);

}

//----------------------------------------------------------------------------
bool TestCheckNodeInSceneByID()
{
  if (CheckNodeInSceneByID(
        __LINE__, nullptr, nullptr, nullptr))
    {
    return false;
    }

  vtkNew<vtkDMMLScene> scene;
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLCoreTestingUtilitiesNode>::New());

  if (CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(), nullptr, nullptr))
    {
    return false;
    }

  if (CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        "vtkDMMLCoreTestingUtilitiesNode", nullptr))
    {
    return false;
    }

  vtkNew<vtkDMMLCoreTestingUtilitiesNode> node1;

  if (CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        "vtkDMMLCoreTestingUtilitiesNode", node1.GetPointer()))
    {
    return false;
    }

  scene->AddNode(node1.GetPointer());

  if (!CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        "vtkDMMLCoreTestingUtilitiesNode1", node1.GetPointer()))
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestCheckNodeIdAndName()
{
  if (CheckNodeIdAndName(
        __LINE__, nullptr, nullptr, nullptr))
    {
    return false;
    }

  vtkNew<vtkDMMLCoreTestingUtilitiesNode> node1;

  if (!CheckNodeIdAndName(
        __LINE__, node1.GetPointer(), nullptr, nullptr))
    {
    return false;
    }

  if (CheckNodeIdAndName(
        __LINE__, node1.GetPointer(), "vtkDMMLCoreTestingUtilitiesNode1", "Testing"))
    {
    return false;
    }

  vtkNew<vtkDMMLScene> scene;
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLCoreTestingUtilitiesNode>::New());

  scene->AddNode(node1.GetPointer());

  if (!CheckNodeIdAndName(
        __LINE__, node1.GetPointer(), "vtkDMMLCoreTestingUtilitiesNode1", "Testing"))
    {
    return false;
    }

  return true;
}
