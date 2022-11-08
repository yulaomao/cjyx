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

==============================================================================*/

// DMML includes
#include "vtkDMMLInteractionNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSelectionNode.h"

// Annotations includes
#include "vtkDMMLAnnotationRulerNode.h"
#include "vtkDMMLAnnotationHierarchyNode.h"
#include "vtkCjyxAnnotationModuleLogic.h"

// STD includes
#include <vtkNew.h>

bool ImportTwiceTest(bool verbose);

//---------------------------------------------------------------------------
int vtkCjyxAnnotationModuleLogicImportSceneTest(int vtkNotUsed(argc), char * vtkNotUsed(argv) [])
{
  bool verbose = true;
  bool res = true;
  res = ImportTwiceTest(verbose) && res;
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//---------------------------------------------------------------------------
// The tests makes sure the annotation hierarchy nodes can be loaded twice.
// a) populates a scene with an annotation
// b) and imports a similar scene into the existing scene.
bool ImportTwiceTest(bool verbose)
{
  vtkNew<vtkDMMLScene> scene;

  vtkNew<vtkDMMLSelectionNode> selectionNode;
  scene->AddNode(selectionNode.GetPointer());

  vtkNew<vtkDMMLInteractionNode> interactionNode;
  scene->AddNode(interactionNode.GetPointer());

  vtkNew<vtkCjyxAnnotationModuleLogic > logic;
  logic->SetDMMLScene(scene.GetPointer());

  logic->AddHierarchy();
  vtkNew<vtkDMMLAnnotationRulerNode> rnode;
  rnode->Initialize(scene.GetPointer());

  std::cout << "Starting scene has " << scene->GetNumberOfNodes() << " nodes" << std::endl;
  /// At this point the following node id should be in the scene:
  ///   - vtkDMMLAnnotationHierarchyNode
  ///       - vtkDMMLAnnotationHierarchyNode -> vtkDMMLFiducialAnnotationNode -> vtkDMMLFiducialAnnotationDisplayNode
  ///            -
  ///   - vtkDMMLModelHierarchyNode1 (pointing to vtkDMMLModelDisplayNode1 and vtkDMMLModelNode1)
  ///   - vtkDMMLModelHierarchyNode2 (parent of vtkDMMLModelHierarchyNode1)
  vtkDMMLAnnotationRulerNode* ruler1 = vtkDMMLAnnotationRulerNode::SafeDownCast(
    scene->GetNodeByID("vtkDMMLAnnotationRulerNode1"));
  double controlPoint1[3]={34,56,78};
  double controlPoint2[3]={134,156,178};
  ruler1->SetControlPointWorldCoordinates(0,controlPoint1, 0, 1);
  ruler1->SetControlPointWorldCoordinates(1,controlPoint2, 0, 1);
  vtkDMMLAnnotationHierarchyNode* rulerHierarchy1 = vtkDMMLAnnotationHierarchyNode::SafeDownCast(
    vtkDMMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(
      scene.GetPointer(),
      ruler1->GetID()));
  vtkDMMLAnnotationHierarchyNode* listNode1 = vtkDMMLAnnotationHierarchyNode::SafeDownCast(
    scene->GetNodeByID("vtkDMMLAnnotationHierarchyNode2"));
  vtkDMMLAnnotationHierarchyNode* allAnnotationsNode1 = vtkDMMLAnnotationHierarchyNode::SafeDownCast(
    scene->GetNodeByID("vtkDMMLAnnotationHierarchyNode1"));
  if (rulerHierarchy1->GetParentNode() != listNode1)
    {
    std::cerr << __LINE__ << ": Import failed." << std::endl;
    return false;
    }
  if (listNode1->GetParentNode() != allAnnotationsNode1)
    {
    std::cerr << __LINE__ << ": Import failed." << std::endl;
    return false;
    }

  // Save
  scene->SetSaveToXMLString(1);
  scene->Commit();
  std::string xmlScene = scene->GetSceneXMLString();
  if (verbose)
    {
    std::cout << xmlScene << std::endl;
    }

  // Load same scene into scene
  scene->SetSceneXMLString(xmlScene);
  scene->SetLoadFromXMLString(1);
  scene->Import();

  if (verbose)
    {
    scene->SetSaveToXMLString(1);
    scene->Commit();
    xmlScene = scene->GetSceneXMLString();
    std::cout << "Scene after import: " << std::endl;
    std::cout << xmlScene << std::endl;
    }

  vtkDMMLAnnotationRulerNode* ruler2 = vtkDMMLAnnotationRulerNode::SafeDownCast(
    scene->GetNodeByID("vtkDMMLAnnotationRulerNode2"));
  vtkDMMLAnnotationHierarchyNode* rulerHierarchy2 = vtkDMMLAnnotationHierarchyNode::SafeDownCast(
    vtkDMMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(
      scene.GetPointer(),
      ruler2->GetID()));
  vtkDMMLAnnotationHierarchyNode* listNode2 = vtkDMMLAnnotationHierarchyNode::SafeDownCast(
    scene->GetNodeByID("vtkDMMLAnnotationHierarchyNode5"));
  vtkDMMLAnnotationHierarchyNode* allAnnotationsNode2 = vtkDMMLAnnotationHierarchyNode::SafeDownCast(
    scene->GetNodeByID("vtkDMMLAnnotationHierarchyNode4"));

  if (rulerHierarchy2->GetParentNode() != listNode2)
    {
    std::cerr << __LINE__ << ": Import failed." << std::endl;
    return false;
    }
  if (listNode2->GetParentNode() != allAnnotationsNode2)
    {
    std::cerr << __LINE__ << ": Import failed." << std::endl;
    return false;
    }
  return true;
}
