/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Subject Hierarchy includes
#include "vtkCjyxSubjectHierarchyModuleLogic.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

// DMML includes
#include "vtkDMMLScene.h"
#include <vtkDMMLSubjectHierarchyNode.h>
#include "vtkDMMLModelNode.h"
#include "vtkDMMLModelDisplayNode.h"
#include "vtkDMMLScalarVolumeNode.h"
#include "vtkDMMLScalarVolumeDisplayNode.h"
#include "vtkDMMLLinearTransformNode.h"

// STD includes
#include <cassert>
#include <sstream>

namespace
{
  bool PopulateScene(vtkDMMLScene* scene);

  bool TestExpand();
  bool TestAccess();
  bool TestAssociations();
  bool TestTreeOperations();
  bool TestInsertDicomSeriesEmptyScene();
  bool TestInsertDicomSeriesPopulatedScene();
  bool TestVisibilityOperations();
  bool TestTransformBranch();

  const char* STUDY_ATTRIBUTE_NAME = "TestStudyAttribute";
  const char* STUDY_ATTRIBUTE_VALUE = "1";
  const char* UID_NAME = vtkDMMLSubjectHierarchyConstants::GetDICOMUIDName();
  const char* PATIENT_UID_VALUE = "PATIENT";
  const char* STUDY1_UID_VALUE = "STUDY1";
  const char* STUDY2_UID_VALUE = "STUDY2";
  const char* VOLUME1_UID_VALUE = "VOLUME1";
  const char* MODEL1_UID_VALUE = "MODEL1";
  const char* VOLUME2_UID_VALUE = "VOLUME2";
  const char* MODEL21_UID_VALUE = "MODEL21";
  const char* MODEL22_UID_VALUE = "MODEL22";

} // end of anonymous namespace

//---------------------------------------------------------------------------
int vtkCjyxSubjectHierarchyModuleLogicTest( int vtkNotUsed(argc),
                                              char * vtkNotUsed(argv)[] )
{
  if (!TestExpand())
    {
    std::cerr << "'TestExpand' call not successful." << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

namespace
{
  //---------------------------------------------------------------------------
  // Populate a sample subject hierarchy scene
  // Scene
  //  + SubjectHierarchyItem
  //     |    (Patient)
  //     +- SubjectHierarchyItem
  //     |   |    (Study)
  //     |   +- SubjectHierarchyItem -- ScalarVolumeNode (volume1)
  //     |   |        (Series)            > DisplayNode
  //     |   +- SubjectHierarchyItem -- ModelNode (model1)
  //     |            (Series)            > DisplayNode
  //     +- SubjectHierarchyItem
  //         |    (Study)
  //         +- SubjectHierarchyItem -- ScalarVolumeNode (volume2)
  //         |        (Series)                  > DisplayNode
  //         +- SubjectHierarchyItem -- ModelNode (model21) <--  ModelHierarchyNode
  //         |       (Series)               > DisplayNode
  //         +- SubjectHierarchyItem -- ModelNode (model22) <--  ModelHierarchyNode
  //                 (Series)               > DisplayNode
  //
  bool PopulateScene(vtkDMMLScene* scene)
    {
    // Get subject hierarchy node from the scene
    vtkDMMLSubjectHierarchyNode* shNode = vtkDMMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene);
    if (!shNode)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with PopulateScene\n"
                << "\tFailed to access subject hierarchy node"
                << std::endl;
      return false;
      }
    vtkIdType sceneItemID = shNode->GetSceneItemID();
    if (sceneItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with PopulateScene\n"
                << "\tInvalid scene item"
                << std::endl;
      return false;
      }

    // Create subject and studies
    vtkIdType patientItemID = shNode->CreateSubjectItem(sceneItemID, "Patient");
    shNode->SetItemUID(patientItemID, UID_NAME, PATIENT_UID_VALUE);

    vtkIdType study1ItemID = shNode->CreateStudyItem(patientItemID, "Study1");
    shNode->SetItemUID(study1ItemID, UID_NAME, STUDY1_UID_VALUE);

    vtkIdType study2ItemID = shNode->CreateStudyItem(patientItemID, "Study2");
    shNode->SetItemUID(study2ItemID, UID_NAME, STUDY2_UID_VALUE);
    shNode->SetItemAttribute(study2ItemID, STUDY_ATTRIBUTE_NAME, STUDY_ATTRIBUTE_VALUE);

    // Create volume series in study 1
    vtkNew<vtkDMMLScalarVolumeNode> volume1Node;
    volume1Node->SetName("Volume1");
    scene->AddNode(volume1Node.GetPointer());

    vtkNew<vtkDMMLScalarVolumeDisplayNode> volume1DisplayNode;
    scene->AddNode(volume1DisplayNode.GetPointer());
    volume1Node->SetAndObserveDisplayNodeID(volume1DisplayNode->GetID());

    vtkIdType volume1SeriesItemID = shNode->CreateItem(study1ItemID, volume1Node.GetPointer());
    shNode->SetItemUID(volume1SeriesItemID, UID_NAME, VOLUME1_UID_VALUE);

    // Create model1 series in study 1
    vtkNew<vtkDMMLModelNode> model1Node;
    model1Node->SetName("Model1");
    scene->AddNode(model1Node.GetPointer());

    vtkNew<vtkDMMLModelDisplayNode> model1DisplayNode;
    scene->AddNode(model1DisplayNode.GetPointer());
    model1Node->SetAndObserveDisplayNodeID(model1DisplayNode->GetID());

    vtkIdType model1SeriesItemID = shNode->CreateItem(study1ItemID, model1Node.GetPointer());
    shNode->SetItemUID(model1SeriesItemID, UID_NAME, MODEL1_UID_VALUE);

    // Create volume series in study 2
    vtkNew<vtkDMMLScalarVolumeNode> volume2Node;
    volume2Node->SetName("Volume2");
    scene->AddNode(volume2Node.GetPointer());

    vtkNew<vtkDMMLScalarVolumeDisplayNode> volume2DisplayNode;
    scene->AddNode(volume2DisplayNode.GetPointer());
    volume2Node->SetAndObserveDisplayNodeID(volume2DisplayNode->GetID());

    vtkIdType volume2SeriesItemID = shNode->CreateItem(study2ItemID, volume2Node.GetPointer());
    shNode->SetItemUID(volume2SeriesItemID, UID_NAME, VOLUME2_UID_VALUE);

    // Create model21 series in study 2
    vtkNew<vtkDMMLModelNode> model21Node;
    model21Node->SetName("Model21");
    scene->AddNode(model21Node.GetPointer());

    vtkNew<vtkDMMLModelDisplayNode> model21DisplayNode;
    scene->AddNode(model21DisplayNode.GetPointer());
    model21Node->SetAndObserveDisplayNodeID(model21DisplayNode->GetID());

    vtkIdType model21SeriesItemID = shNode->CreateItem(study2ItemID, model21Node.GetPointer());
    shNode->SetItemUID(model21SeriesItemID, UID_NAME, MODEL21_UID_VALUE);

    // Create model22 series in study 2
    vtkNew<vtkDMMLModelNode> model22Node;
    model22Node->SetName("Model22");
    scene->AddNode(model22Node.GetPointer());

    vtkNew<vtkDMMLModelDisplayNode> model22DisplayNode;
    scene->AddNode(model22DisplayNode.GetPointer());
    model22Node->SetAndObserveDisplayNodeID(model22DisplayNode->GetID());

    vtkIdType model22SeriesItemID = shNode->CreateItem(study2ItemID, model22Node.GetPointer());
    shNode->SetItemUID(model22SeriesItemID, UID_NAME, MODEL22_UID_VALUE);

    int currentNodeCount = scene->GetNumberOfNodes();
    int expectedNodeCount = 11;
    if (expectedNodeCount != currentNodeCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with PopulateScene\n"
                << "\tcurrentNodeCount: " << currentNodeCount << "\n"
                << "\texpectedNodeCount: " << expectedNodeCount
                << std::endl;
      return false;
      }

    currentNodeCount = scene->GetNumberOfNodesByClass("vtkDMMLScalarVolumeNode");
    expectedNodeCount = 2;
    if (expectedNodeCount != currentNodeCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with PopulateScene\n"
                << "\tcurrentNodeCount: " << currentNodeCount << "\n"
                << "\texpectedNodeCount: " << expectedNodeCount
                << std::endl;
      return false;
      }

    currentNodeCount = scene->GetNumberOfNodesByClass("vtkDMMLModelNode");
    expectedNodeCount = 3;
    if (expectedNodeCount != currentNodeCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with PopulateScene\n"
                << "\tcurrentNodeCount: " << currentNodeCount << "\n"
                << "\texpectedNodeCount: " << expectedNodeCount
                << std::endl;
      return false;
      }

    return true;
    }

  //---------------------------------------------------------------------------
  bool TestExpand()
    {
    if (!TestAccess())
      {
      std::cerr << "'TestAccess' call not successful." << std::endl;
      return false;
      }
    if (!TestAssociations())
      {
      std::cerr << "'TestAssociations' call not successful." << std::endl;
      return false;
      }
    if (!TestTreeOperations())
      {
      std::cerr << "'TestTreeOperations' call not successful." << std::endl;
      return false;
      }
    if (!TestInsertDicomSeriesEmptyScene())
      {
      std::cerr << "'TestInsertDicomSeriesEmptyScene' call not successful." << std::endl;
      return false;
      }
    if (!TestInsertDicomSeriesPopulatedScene())
      {
      std::cerr << "'TestInsertDicomSeriesPopulatedScene' call not successful." << std::endl;
      return false;
      }
    if (!TestVisibilityOperations())
      {
      std::cerr << "'TestVisibilityOperations' call not successful." << std::endl;
      return false;
      }
    if (!TestTransformBranch())
      {
      std::cerr << "'TestTransformBranch' call not successful." << std::endl;
      return false;
      }
    return true;
    }

  //---------------------------------------------------------------------------
  bool TestAccess()
    {
    vtkNew<vtkDMMLScene> scene;
    if (!PopulateScene(scene.GetPointer()))
      {
      return false;
      }

    vtkDMMLSubjectHierarchyNode* shNode = vtkDMMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene.GetPointer());
    if (!shNode)
      {
      return false;
      }

    // Get item by UID
    vtkIdType patientItemID = shNode->GetItemByUID(UID_NAME, PATIENT_UID_VALUE);
    if (patientItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
      {
      std::cerr << "Failed to get subject by UID" << std::endl;
      return false;
      }
    // Check level
    if (shNode->GetItemLevel(patientItemID).compare(vtkDMMLSubjectHierarchyConstants::GetDICOMLevelPatient()))
      {
      std::cerr << "Wrong level of found subject item" << std::endl;
      return false;
      }

    return true;
    }

  //---------------------------------------------------------------------------
  bool TestAssociations()
    {
    vtkSmartPointer<vtkDMMLScene> scene = vtkSmartPointer<vtkDMMLScene>::New();
    if (!PopulateScene(scene))
      {
      return false;
      }

    vtkDMMLSubjectHierarchyNode* shNode = vtkDMMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene);
    if (!shNode)
      {
      return false;
      }

    // Get volume item for testing association
    vtkIdType volume1ShItemID = shNode->GetItemByUID(UID_NAME, VOLUME1_UID_VALUE);
    if (volume1ShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
      {
      std::cerr << "Failed to get volume by UID" << std::endl;
      return false;
      }

    // Get associated data node
    vtkDMMLScalarVolumeNode* volume1Node = vtkDMMLScalarVolumeNode::SafeDownCast(
      shNode->GetItemDataNode(volume1ShItemID) );
    if (!volume1Node)
      {
      std::cerr << "Failed to get associated volume node" << std::endl;
      return false;
      }

    // Get associated subject hierarchy item
    vtkIdType volume1ShItemIdTest = shNode->GetItemByDataNode(volume1Node);
    if (volume1ShItemIdTest != volume1ShItemID)
      {
      std::cerr << "Failed to get associated subject hierarchy item for volume" << std::endl;
      return false;
      }

    return true;
    }

  //---------------------------------------------------------------------------
  bool TestTreeOperations()
    {
    vtkSmartPointer<vtkDMMLScene> scene = vtkSmartPointer<vtkDMMLScene>::New();
    if (!PopulateScene(scene))
      {
      return false;
      }

    vtkDMMLSubjectHierarchyNode* shNode = vtkDMMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene);
    if (!shNode)
      {
      return false;
      }

    // Get nodes used in this test case
    vtkIdType patientShItemID = shNode->GetItemByUID(UID_NAME, PATIENT_UID_VALUE);
    vtkIdType study1ShItemID = shNode->GetItemByUID(UID_NAME, STUDY1_UID_VALUE);
    vtkIdType volume1ShItemID = shNode->GetItemByUID(UID_NAME, VOLUME1_UID_VALUE);
    vtkIdType volume2ShItemID = shNode->GetItemByUID(UID_NAME, VOLUME2_UID_VALUE);
    vtkIdType model21ShItemID = shNode->GetItemByUID(UID_NAME, MODEL21_UID_VALUE);
    if ( patientShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID || study1ShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID
      || volume1ShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID || volume2ShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID
      || model21ShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID )
      {
      std::cerr << "Failed to get items by UID" << std::endl;
      return false;
      }

    // AreItemsInSameBranch utility method in logic class
    vtkIdType commonAncestorItemID = vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID;

    commonAncestorItemID = vtkCjyxSubjectHierarchyModuleLogic::AreItemsInSameBranch(
      shNode, volume1ShItemID, volume2ShItemID, vtkDMMLSubjectHierarchyConstants::GetDICOMLevelStudy());
    if (commonAncestorItemID != vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
      {
      std::cerr << "Common parent check failed (volume1 and volume2 are not in the same study)" << std::endl;
      return false;
      }

    commonAncestorItemID = vtkCjyxSubjectHierarchyModuleLogic::AreItemsInSameBranch(
      shNode, volume2ShItemID, model21ShItemID, vtkDMMLSubjectHierarchyConstants::GetDICOMLevelStudy());
    if ( commonAncestorItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID
      || shNode->GetItemLevel(commonAncestorItemID).compare(vtkDMMLSubjectHierarchyConstants::GetDICOMLevelStudy()) )
      {
      std::cerr << "Common parent check failed (volume2 and model21 are in the same study)" << std::endl;
      return false;
      }

    commonAncestorItemID = vtkCjyxSubjectHierarchyModuleLogic::AreItemsInSameBranch(
      shNode, volume1ShItemID, volume2ShItemID, vtkDMMLSubjectHierarchyConstants::GetDICOMLevelPatient());
    if ( commonAncestorItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID
      || shNode->GetItemLevel(commonAncestorItemID).compare(vtkDMMLSubjectHierarchyConstants::GetDICOMLevelPatient()) )
      {
      std::cerr << "Common parent check failed (volume1 and volume2 are in the same patient)" << std::endl;
      return false;
      }

    // GetItemChildWithName utility method in node class
    vtkIdType foundChildItemID = vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID;

    foundChildItemID = shNode->GetItemChildWithName(shNode->GetSceneItemID(), shNode->GetItemName(patientShItemID));
    if ( foundChildItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID
      || shNode->GetItemLevel(foundChildItemID).compare(vtkDMMLSubjectHierarchyConstants::GetDICOMLevelPatient()) )
      {
      std::cerr << "Failed to find top-level subject hierarchy item by name" << std::endl;
      return false;
      }

    foundChildItemID = shNode->GetItemChildWithName(patientShItemID, shNode->GetItemName(study1ShItemID));
    if ( foundChildItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID
      || shNode->GetItemLevel(foundChildItemID).compare(vtkDMMLSubjectHierarchyConstants::GetDICOMLevelStudy()) )
      {
      std::cerr << "Failed to find child subject hierarchy item by name" << std::endl;
      return false;
      }

    // GetDataNodesInBranch utility method in node class
    vtkNew<vtkCollection> childDataNodes;

    shNode->GetDataNodesInBranch(patientShItemID, childDataNodes.GetPointer());
    int expectedChildCount = 5;
    int currentChildCount = childDataNodes->GetNumberOfItems();
    if (currentChildCount != expectedChildCount)
      {
      std::cerr << "Line " << __LINE__ << " - Failed to find associated children nodes:\n"
                << "\tcurrent count: " << currentChildCount << "\n"
                << "\texpected count: " << expectedChildCount
                << std::endl;
      return false;
      }

    childDataNodes->RemoveAllItems();
    shNode->GetDataNodesInBranch(patientShItemID, childDataNodes.GetPointer(), "vtkDMMLScalarVolumeNode");
    expectedChildCount = 2;
    currentChildCount = childDataNodes->GetNumberOfItems();
    if (currentChildCount != expectedChildCount)
      {
      std::cerr << "Line " << __LINE__ << " - Failed to find associated children volume nodes:\n"
                << "\tcurrent count: " << currentChildCount << "\n"
                << "\texpected count: " << expectedChildCount
                << std::endl;
      return false;
      }

    // GetAttributeFromItemAncestor utility method in node class
    std::string attributeValueAnyLevel =
      shNode->GetAttributeFromItemAncestor(volume2ShItemID, STUDY_ATTRIBUTE_NAME);
    if (attributeValueAnyLevel.compare(STUDY_ATTRIBUTE_VALUE))
      {
      std::cerr << "Failed to find attribute from ancestor in any level" << std::endl;
      return false;
      }

    std::string attributeValueStudyLevel =
      shNode->GetAttributeFromItemAncestor(volume2ShItemID, STUDY_ATTRIBUTE_NAME, vtkDMMLSubjectHierarchyConstants::GetDICOMLevelStudy());
    if (attributeValueStudyLevel.compare(STUDY_ATTRIBUTE_VALUE))
      {
      std::cerr << "Failed to find attribute from ancestor in study level" << std::endl;
      return false;
      }

    std::string attributeValueWrongLevel =
      shNode->GetAttributeFromItemAncestor(volume2ShItemID, STUDY_ATTRIBUTE_NAME, vtkDMMLSubjectHierarchyConstants::GetDICOMLevelPatient());
    if (!attributeValueWrongLevel.empty())
      {
      std::cerr << "Found attribute from ancestor in wrong level" << std::endl;
      return false;
      }

    // GetItemAncestorAtLevel utility method in node class
    vtkIdType ancestorAtStudyLevel =
      shNode->GetItemAncestorAtLevel(volume1ShItemID, vtkDMMLSubjectHierarchyConstants::GetDICOMLevelStudy());
    if ( ancestorAtStudyLevel == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID
      || shNode->GetItemLevel(ancestorAtStudyLevel).compare(vtkDMMLSubjectHierarchyConstants::GetDICOMLevelStudy()) )
      {
      std::cerr << "Failed to find ancestor at study level" << std::endl;
      return false;
      }

    vtkIdType ancestorAtPatientLevel =
      shNode->GetItemAncestorAtLevel(volume1ShItemID, vtkDMMLSubjectHierarchyConstants::GetDICOMLevelPatient());
    if ( ancestorAtPatientLevel == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID
      || shNode->GetItemLevel(ancestorAtPatientLevel).compare(vtkDMMLSubjectHierarchyConstants::GetDICOMLevelPatient()) )
      {
      std::cerr << "Failed to find ancestor at patient level" << std::endl;
      return false;
      }

    return true;
    }

  //---------------------------------------------------------------------------
  bool TestInsertDicomSeriesEmptyScene()
    {
    vtkSmartPointer<vtkDMMLScene> scene = vtkSmartPointer<vtkDMMLScene>::New();

    vtkDMMLSubjectHierarchyNode* shNode = vtkDMMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene);
    if (!shNode)
      {
      return false;
      }

    // Create series item to insert
    const char* seriesUid = "NEW_SERIES";
    vtkIdType seriesItemID = shNode->CreateHierarchyItem(shNode->GetSceneItemID(), "Series", "Series");
    shNode->SetItemUID(seriesItemID, UID_NAME, seriesUid);

    vtkIdType insertedSeriesItemID = vtkCjyxSubjectHierarchyModuleLogic::InsertDicomSeriesInHierarchy(
      shNode, PATIENT_UID_VALUE, STUDY1_UID_VALUE, seriesUid );
    if (insertedSeriesItemID != seriesItemID)
      {
      std::cerr << "Failed to insert DICOM series item" << std::endl;
      return false;
      }

    // Check newly created study and patient items
    vtkIdType patientShItemID = shNode->GetItemByUID(UID_NAME, PATIENT_UID_VALUE);
    vtkIdType studyShItemID = shNode->GetItemByUID(UID_NAME, STUDY1_UID_VALUE);
    if ( patientShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID
      || studyShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID )
      {
      std::cerr << "Patient and study items not created for DICOM series" << std::endl;
      return false;
      }

    // Check if series is indeed the child of the newly created study
    if (shNode->GetItemParent(seriesItemID) != studyShItemID)
      {
      std::cerr << "DICOM series item not correctly inserted under study item" << std::endl;
      return false;
      }

    return true;
    }

  //---------------------------------------------------------------------------
  bool TestInsertDicomSeriesPopulatedScene()
    {
    vtkSmartPointer<vtkDMMLScene> scene = vtkSmartPointer<vtkDMMLScene>::New();
    if (!PopulateScene(scene))
      {
      return false;
      }

    vtkDMMLSubjectHierarchyNode* shNode = vtkDMMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene);
    if (!shNode)
      {
      return false;
      }

    // Create series item to insert
    const char* seriesUid = "NEW_SERIES";
    vtkIdType seriesItemID = shNode->CreateHierarchyItem(shNode->GetSceneItemID(), "Series", "Series");
    shNode->SetItemUID(seriesItemID, UID_NAME, seriesUid);

    vtkIdType insertedSeriesItemID =
      vtkCjyxSubjectHierarchyModuleLogic::InsertDicomSeriesInHierarchy(
        shNode, PATIENT_UID_VALUE, STUDY1_UID_VALUE, seriesUid );

    // Check if series was inserted under the desired study
    vtkIdType studyShItemID = shNode->GetItemByUID(UID_NAME, STUDY1_UID_VALUE);
    if (studyShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
      {
      std::cerr << "Study item not found" << std::endl;
      return false;
      }
    if (shNode->GetItemParent(seriesItemID) != studyShItemID)
      {
      std::cerr << "DICOM series item not correctly inserted under study item" << std::endl;
      return false;
      }
    if (insertedSeriesItemID != seriesItemID)
      {
      std::cerr << "Inserted series does not match retrieved series" << std::endl;
      return false;
      }

    return true;
    }

  //---------------------------------------------------------------------------
  bool TestVisibilityOperations()
    {
    vtkSmartPointer<vtkDMMLScene> scene = vtkSmartPointer<vtkDMMLScene>::New();
    if (!PopulateScene(scene))
      {
      return false;
      }

    vtkDMMLSubjectHierarchyNode* shNode = vtkDMMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene);
    if (!shNode)
      {
      return false;
      }

    // Get items used in this test case
    vtkIdType patientShItemID = shNode->GetItemByUID(UID_NAME, PATIENT_UID_VALUE);
    vtkIdType study1ShItemID = shNode->GetItemByUID(UID_NAME, STUDY1_UID_VALUE);
    vtkIdType study2ShItemID = shNode->GetItemByUID(UID_NAME, STUDY2_UID_VALUE);
    vtkIdType volume1ShItemID = shNode->GetItemByUID(UID_NAME, VOLUME1_UID_VALUE);
    vtkIdType model1ShItemID = shNode->GetItemByUID(UID_NAME, MODEL1_UID_VALUE);
    vtkIdType volume2ShItemID = shNode->GetItemByUID(UID_NAME, VOLUME2_UID_VALUE);
    vtkIdType model21ShItemID = shNode->GetItemByUID(UID_NAME, MODEL21_UID_VALUE);
    vtkIdType model22ShItemID = shNode->GetItemByUID(UID_NAME, MODEL22_UID_VALUE);
    if ( patientShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID || study1ShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID
      || study2ShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID || volume1ShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID
      || model1ShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID || volume2ShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID
      || model21ShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID || model22ShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID )
      {
      std::cerr << "Failed to get items by UID" << std::endl;
      return false;
      }
    vtkDMMLScalarVolumeNode* volume1Node = vtkDMMLScalarVolumeNode::SafeDownCast(shNode->GetItemDataNode(volume1ShItemID));
    vtkDMMLModelNode* model1Node = vtkDMMLModelNode::SafeDownCast(shNode->GetItemDataNode(model1ShItemID));
    vtkDMMLScalarVolumeNode* volume2Node = vtkDMMLScalarVolumeNode::SafeDownCast(shNode->GetItemDataNode(volume2ShItemID));
    vtkDMMLModelNode* model21Node = vtkDMMLModelNode::SafeDownCast(shNode->GetItemDataNode(model21ShItemID));
    vtkDMMLModelNode* model22Node = vtkDMMLModelNode::SafeDownCast(shNode->GetItemDataNode(model22ShItemID));
    if (!volume1Node || !model1Node || !volume2Node || !model21Node || !model22Node)
      {
      std::cerr << "Unable to get data nodes" << std::endl;
      return false;
      }
    vtkDMMLScalarVolumeDisplayNode* volume1DisplayNode = vtkDMMLScalarVolumeDisplayNode::SafeDownCast(volume1Node->GetDisplayNode());
    vtkDMMLModelDisplayNode* model1DisplayNode = vtkDMMLModelDisplayNode::SafeDownCast(model1Node->GetDisplayNode());
    vtkDMMLScalarVolumeDisplayNode* volume2DisplayNode = vtkDMMLScalarVolumeDisplayNode::SafeDownCast(volume2Node->GetDisplayNode());
    vtkDMMLModelDisplayNode* model21DisplayNode = vtkDMMLModelDisplayNode::SafeDownCast(model21Node->GetDisplayNode());
    vtkDMMLModelDisplayNode* model22DisplayNode = vtkDMMLModelDisplayNode::SafeDownCast(model22Node->GetDisplayNode());
    if (!volume1DisplayNode || !model1DisplayNode || !volume2DisplayNode || !model21DisplayNode || !model22DisplayNode)
      {
      std::cerr << "Unable to get display nodes" << std::endl;
      return false;
      }

    // By default Visibility is on, SliceIntersectionVisibility is off. Check this
    if (model1DisplayNode->GetVisibility() != 1 || model1DisplayNode->GetVisibility2D() != 0)
      {
      std::cerr << "Default visibility settings changed! Test application needs to be reviewed" << std::endl;
      return false;
      }

    if (shNode->GetItemDisplayVisibility(patientShItemID) != 1)
      {
      std::cerr << "Wrong display visibility value for patient" << std::endl;
      return false;
      }

    return true;
    }

  //---------------------------------------------------------------------------
  bool TestTransformBranch()
    {
    vtkSmartPointer<vtkDMMLScene> scene = vtkSmartPointer<vtkDMMLScene>::New();
    if (!PopulateScene(scene))
      {
      return false;
      }

    vtkDMMLSubjectHierarchyNode* shNode = vtkDMMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene);
    if (!shNode)
      {
      return false;
      }

    // Get items used in this test case
    vtkIdType study2ShItemID = shNode->GetItemByUID(UID_NAME, STUDY2_UID_VALUE);
    vtkIdType model21ShItemID = shNode->GetItemByUID(UID_NAME, MODEL21_UID_VALUE);
    vtkIdType model22ShItemID = shNode->GetItemByUID(UID_NAME, MODEL22_UID_VALUE);
    if ( study2ShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID
      || model21ShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID || model22ShItemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID )
      {
      std::cerr << "Failed to get items by UID" << std::endl;
      return false;
      }

    // Set non-empty poly data to model21 and model22 to be able to check transform hardening
    vtkNew<vtkPoints> model21Points;
    model21Points->InsertNextPoint(0.0, 0.0, 0.0);
    vtkNew<vtkPolyData> model21PolyData;
    model21PolyData->SetPoints(model21Points.GetPointer());

    vtkNew<vtkPoints> model22Points;
    model22Points->InsertNextPoint(0.0, 0.0, 0.0);
    vtkNew<vtkPolyData> model22PolyData;
    model22PolyData->SetPoints(model22Points.GetPointer());

    vtkDMMLModelNode* model21Node = vtkDMMLModelNode::SafeDownCast(shNode->GetItemDataNode(model21ShItemID));
    vtkDMMLModelNode* model22Node = vtkDMMLModelNode::SafeDownCast(shNode->GetItemDataNode(model22ShItemID));
    if (!model21Node || !model22Node)
      {
      std::cerr << "Failed to get model data nodes" << std::endl;
      return false;
      }
    model21Node->SetAndObservePolyData(model21PolyData.GetPointer());
    model22Node->SetAndObservePolyData(model22PolyData.GetPointer());

    // Create transforms to use in this test case
    vtkNew<vtkMatrix4x4> transformMatrix1;
    transformMatrix1->SetElement(0,3,100.0); // Translation along X by 100 mm
    vtkNew<vtkDMMLLinearTransformNode> transformNode1;
    transformNode1->SetMatrixTransformToParent(transformMatrix1.GetPointer());
    scene->AddNode(transformNode1.GetPointer());

    vtkNew<vtkMatrix4x4> transformMatrix2;
    transformMatrix2->SetElement(1,3,50.0); // Translation along Y by 50 mm
    vtkNew<vtkDMMLLinearTransformNode> transformNode2; // Second transform is identity
    transformNode2->SetMatrixTransformToParent(transformMatrix2.GetPointer());
    scene->AddNode(transformNode2.GetPointer());

    // Check transformed items
    if (shNode->IsAnyNodeInBranchTransformed(study2ShItemID))
      {
      std::cerr << "Wrong inquiry about transformed items in study2" << std::endl;
      return false;
      }

    // Transform model21 using transform1
    vtkCjyxSubjectHierarchyModuleLogic::TransformBranch(shNode, model21ShItemID, transformNode1.GetPointer());
    vtkDMMLTransformNode* currentTransformNode = model21Node->GetParentTransformNode();
    vtkDMMLTransformNode* expectedTransformNode = transformNode1.GetPointer();
    if (currentTransformNode != expectedTransformNode)
      {
      std::cerr << "Line " << __LINE__ << " - Failed to apply transform1 on model1:\n"
                << "\tcurrentTransformNode: "
                << (currentTransformNode ? currentTransformNode->GetID() : "<null>") << "\n"
                << "\texpectedTransformNode: "
                << (expectedTransformNode ? expectedTransformNode->GetID() : "<null>")
                << std::endl;
      return false;
      }
    if (!shNode->IsAnyNodeInBranchTransformed(study2ShItemID))
      {
      std::cerr << "Wrong inquiry about transformed items in study2" << std::endl;
      return false;
      }
    if (shNode->IsAnyNodeInBranchTransformed(study2ShItemID, true, transformNode1.GetPointer()))
      {
      std::cerr << "Wrong inquiry about transformed items in study2 with exception transform set" << std::endl;
      return false;
      }

    // Transform study2 using transform2, harden transform on model21
    vtkCjyxSubjectHierarchyModuleLogic::TransformBranch(shNode, study2ShItemID, transformNode2.GetPointer());
    if ( model21Node->GetParentTransformNode() != transformNode2.GetPointer()
      || model22Node->GetParentTransformNode() != transformNode2.GetPointer() )
      {
      std::cerr << "Failed to apply transform2 on study2" << std::endl;
      return false;
      }

    // Check whether transform1 was hardened on model21
    double model21PointsArray[3] = {0.0, 0.0, 0.0};
    model21PolyData->GetPoint(0, model21PointsArray);
    if (model21PointsArray[0] != 100.0 || model21PointsArray[1] != 0.0 || model21PointsArray[2] != 0.0)
      {
      std::cerr << transformNode2->GetName()
                << " was not hardened on model21 when applying "
                << transformNode2->GetName() << " on " << shNode->GetItemName(study2ShItemID) << std::endl;
      return false;
      }

    // Set transform1 on study2 without hardening
    vtkCjyxSubjectHierarchyModuleLogic::TransformBranch(shNode, study2ShItemID, transformNode1.GetPointer(), false);
    if ( model21Node->GetParentTransformNode() != transformNode1.GetPointer()
      || model22Node->GetParentTransformNode() != transformNode1.GetPointer() )
      {
      std::cerr << "Failed to apply transform1 on study2" << std::endl;
      return false;
      }
    model21PolyData->GetPoint(0, model21PointsArray);
    if (model21PointsArray[0] != 100.0 || model21PointsArray[1] != 0.0 || model21PointsArray[2] != 0.0)
      {
      std::cerr << "Transform2 was hardened on model21 when applying transform1 on study2 without hardening" << std::endl;
      return false;
      }

    // Harden transform2 on model22
    vtkCjyxSubjectHierarchyModuleLogic::TransformBranch(shNode, study2ShItemID, transformNode2.GetPointer(), false);
    vtkCjyxSubjectHierarchyModuleLogic::HardenTransformOnBranch(shNode, study2ShItemID);

    model21PolyData->GetPoint(0, model21PointsArray);
    if (model21PointsArray[0] != 100.0 || model21PointsArray[1] != 50.0 || model21PointsArray[2] != 0.0)
      {
      std::cerr << "Failed to harden transform on study2" << std::endl;
      return false;
      }

    double model22PointsArray[3] =   {0.0, 0.0, 0.0};
    model22PolyData->GetPoint(0, model22PointsArray);
    if (model22PointsArray[0] != 0.0 || model22PointsArray[1] != 50.0 || model22PointsArray[2] != 0.0)
      {
      std::cerr << "Failed to harden transform on study2" << std::endl;
      return false;
      }

    // Remove transforms from study
    vtkCjyxSubjectHierarchyModuleLogic::TransformBranch(shNode, study2ShItemID, transformNode1.GetPointer());
    vtkCjyxSubjectHierarchyModuleLogic::TransformBranch(shNode, study2ShItemID, nullptr);
    if (shNode->IsAnyNodeInBranchTransformed(study2ShItemID))
      {
      std::cerr << "Wrong inquiry about transformed items in study2" << std::endl;
      return false;
      }

    return true;
    }

} // end of anonymous namespace
