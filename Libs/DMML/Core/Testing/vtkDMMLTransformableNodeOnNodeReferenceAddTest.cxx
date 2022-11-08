/*=auto=========================================================================

  Program: 3D Cjyx

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Yuzheng Zhou, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLLinearTransformNode.h"
#include "vtkDMMLTransformableNode.h"
#include "vtkDMMLScalarVolumeNode.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkGeneralTransform.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include "vtkDMMLCoreTestingMacros.h"

namespace
{

void populateScene_nodeRefAfterAddNode(vtkDMMLScene* scene);
void populateScene_nodeRefBeforeAddNode(vtkDMMLScene* scene);
int testScene(vtkDMMLScene* scene);

} // end of anonymous namespace

//---------------------------------------------------------------------------
int vtkDMMLTransformableNodeOnNodeReferenceAddTest(int , char * [] )
{
  // Test with a freshly created scene (node reference added _after_ node added to the scene)
  vtkNew<vtkDMMLScene> scene;
  populateScene_nodeRefAfterAddNode(scene.GetPointer());
  CHECK_EXIT_SUCCESS(testScene(scene.GetPointer()));

  // Test with a freshly created scene (node reference added _before_ node added to the scene)
  vtkNew<vtkDMMLScene> scene1;
  populateScene_nodeRefBeforeAddNode(scene1.GetPointer());
  CHECK_EXIT_SUCCESS(testScene(scene1.GetPointer()));

  // Test with a loaded scene
  // Write scene to XML
  scene->SetSaveToXMLString(1);
  scene->Commit();
  std::string xmlScene = scene->GetSceneXMLString();
  std::cout << xmlScene << std::endl;
  // Load scene from XML
  vtkNew<vtkDMMLScene> scene2;
  scene2->SetLoadFromXMLString(1);
  scene2->SetSceneXMLString(xmlScene);
  scene2->Import();
  CHECK_EXIT_SUCCESS(testScene(scene2.GetPointer()));

  return EXIT_SUCCESS;
}

namespace
{

//---------------------------------------------------------------------------
void populateScene_nodeRefAfterAddNode(vtkDMMLScene* scene)
{
  vtkNew<vtkDMMLLinearTransformNode> transformNode;
  scene->AddNode(transformNode.GetPointer());

  // Test add reference after add node
  vtkNew<vtkDMMLScalarVolumeNode> transformableableNode;
  scene->AddNode(transformableableNode.GetPointer());
  transformableableNode->SetAndObserveTransformNodeID(transformNode->GetID());
}

//---------------------------------------------------------------------------
void populateScene_nodeRefBeforeAddNode(vtkDMMLScene* scene)
{
  vtkNew<vtkDMMLLinearTransformNode> transformNode;
  scene->AddNode(transformNode.GetPointer());

  // Test add reference before add node
  // (Node references should be fully functional, even if the node reference is set
  // before the node was added to the scene.)
  vtkNew<vtkDMMLScalarVolumeNode> transformableableNode;
  transformableableNode->SetAndObserveTransformNodeID(transformNode->GetID());
  scene->AddNode(transformableableNode.GetPointer());
}

int testScene(vtkDMMLScene* scene)
{
  // Check transform node IDs
  vtkDMMLNode* transformNode =
    scene->GetFirstNodeByClass("vtkDMMLLinearTransformNode");
  CHECK_NOT_NULL(transformNode);
  CHECK_STRING(transformNode->GetID(), "vtkDMMLLinearTransformNode1");

  // Check references
  vtkDMMLTransformableNode* transformableNode =
    vtkDMMLTransformableNode::SafeDownCast(scene->GetFirstNodeByClass("vtkDMMLScalarVolumeNode"));

  CHECK_NOT_NULL(transformableNode);
  CHECK_STRING(transformableNode->GetTransformNodeID(), transformNode->GetID());

  // Test vtkDMMLTransformableNode::OnNodeReferenceAdded()
  vtkDMMLLinearTransformNode* linearTransformNode = vtkDMMLLinearTransformNode::SafeDownCast(scene->GetFirstNodeByClass("vtkDMMLLinearTransformNode"));
  vtkDMMLScalarVolumeNode* volumeNode = vtkDMMLScalarVolumeNode::SafeDownCast(scene->GetFirstNodeByClass("vtkDMMLScalarVolumeNode"));
  vtkNew<vtkDMMLCoreTestingUtilities::vtkDMMLNodeCallback> callback;
  volumeNode->AddObserver(vtkCommand::AnyEvent, callback.GetPointer());

  linearTransformNode->GetTransformToParent()->Modified();
  CHECK_STD_STRING(callback->GetErrorString(), "");
  CHECK_INT(callback->GetNumberOfModified(), 0);
  CHECK_INT(callback->GetNumberOfEvents(vtkDMMLTransformNode::TransformModifiedEvent), 1);
  callback->ResetNumberOfEvents();

  return EXIT_SUCCESS;
}

}
