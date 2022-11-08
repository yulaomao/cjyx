/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// DMML includes
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLMarkupsAngleNode.h"
#include "vtkDMMLMarkupsClosedCurveNode.h"
#include "vtkDMMLMarkupsCurveNode.h"
#include "vtkDMMLMarkupsFiducialDisplayNode.h"
#include "vtkDMMLMarkupsFiducialStorageNode.h"
#include "vtkDMMLMarkupsFiducialNode.h"
#include "vtkDMMLMarkupsJsonStorageNode.h"
#include "vtkDMMLMarkupsLineNode.h"
#include "vtkDMMLMarkupsPlaneDisplayNode.h"
#include "vtkDMMLMarkupsPlaneNode.h"
#include "vtkDMMLMarkupsPlaneJsonStorageNode.h"
#include "vtkDMMLMarkupsROIDisplayNode.h"
#include "vtkDMMLMarkupsROINode.h"
#include "vtkDMMLMarkupsROIJsonStorageNode.h"
#include "vtkURIHandler.h"
#include "vtkDMMLScene.h"
#include "vtkPolyData.h"

// DMMLLogic includes
#include <vtkDMMLApplicationLogic.h>

// VTK includes
#include <vtkNew.h>
#include <vtkTestingOutputWindow.h>

// STD includes
#include <algorithm>

namespace
{
  // enable for more debugging output
  const bool verbose = false;
}

int TestStoragNode(vtkDMMLMarkupsNode* markupsNode, vtkDMMLMarkupsStorageNode* storageNode, const std::string& fileName)
{
  std::cout << "--------------------------------" << std::endl;
  std::cout << "TestStoragNode for " << markupsNode->GetClassName() << " with " << storageNode->GetClassName() << std::endl;

  // set up a scene
  vtkNew<vtkDMMLScene> scene;

  // Application logic - Handle creation of vtkDMMLSelectionNode and vtkDMMLInteractionNode
  vtkNew<vtkDMMLApplicationLogic> applicationLogic;
  applicationLogic->SetDMMLScene(scene);

  scene->AddNode(markupsNode);

  vtkNew<vtkDMMLMarkupsDisplayNode> dispNode;
  scene->AddNode(dispNode);
  markupsNode->SetAndObserveDisplayNodeID(dispNode->GetID());

  scene->AddNode(storageNode);
  markupsNode->SetAndObserveStorageNodeID(storageNode->GetID());

  // add a markup with one point with non default values
  int modifiedPointIndex =  markupsNode->AddNControlPoints(1);
  double orientation[4] = {0.2, 1.0, 0.0, 0.0};
  markupsNode->SetNthControlPointOrientation(modifiedPointIndex, orientation);
  markupsNode->ResetNthControlPointID(modifiedPointIndex);
  std::string associatedNodeID = std::string("testingAssociatedID");
  markupsNode->SetNthControlPointAssociatedNodeID(modifiedPointIndex,associatedNodeID);
  markupsNode->SetNthControlPointSelected(modifiedPointIndex, false);
  markupsNode->SetNthControlPointVisibility(modifiedPointIndex, false);
  markupsNode->SetNthControlPointLocked(modifiedPointIndex, true);

  std::string label = std::string("Testing label");
  markupsNode->SetNthControlPointLabel(modifiedPointIndex, label);
  std::string desc = std::string("description with spaces");
  markupsNode->SetNthControlPointDescription(modifiedPointIndex, desc);
  // NAN should not be present, but we test this case anyway
  // to make sure that having a NAN does not break reading or writing.
  double inputPoint[3] = {-9.9, 1.1, NAN};
  markupsNode->SetNthControlPointPosition(modifiedPointIndex, inputPoint);

  // and add a markup with 1 point, default values
  if (markupsNode->GetMaximumNumberOfControlPoints() != 1)
    {
    int defaultPointIndex = markupsNode->AddNControlPoints(1);
    CHECK_INT(defaultPointIndex, 1);
    }

  int emptyLabelIndex = -1;
  int commaIndex = -1;
  int quotesIndex = -1;
  bool testManyPoints = markupsNode->GetMaximumNumberOfControlPoints() < 0 || markupsNode->GetMaximumNumberOfControlPoints() > 5;
  if (testManyPoints)
    {
    // and another one unsetting the label
    emptyLabelIndex = markupsNode->AddNControlPoints(1);
    markupsNode->SetNthControlPointLabel(emptyLabelIndex, "");

    // add another one with a label and description that have commas in them
    commaIndex = markupsNode->AddNControlPoints(1);
    markupsNode->SetNthControlPointLabel(commaIndex, "Label, commas, two");
    markupsNode->SetNthControlPointDescription(commaIndex, "Description one comma \"and two quotes\", for more testing");

    // add another one with a label and description with complex combos of commas and quotes
    quotesIndex = markupsNode->AddNControlPoints(1);
    markupsNode->SetNthControlPointLabel(quotesIndex, "Label with end quotes \"around the last phrase\"");
    markupsNode->SetNthControlPointDescription(quotesIndex, "\"Description fully quoted\"");
    }

  //
  // test write
  //

  if (verbose)
    {
    std::cout << "\nWriting this markup to file:" << std::endl;
    vtkIndent indent;
    markupsNode->PrintSelf(std::cout, indent);
    std::cout << std::endl;
    }

  storageNode->SetFileName(fileName.c_str());
  std::cout << "Writing " << storageNode->GetFileName() << std::endl;
  CHECK_BOOL(storageNode->WriteData(markupsNode), true);

  //
  // test read
  //

  vtkSmartPointer<vtkDMMLMarkupsNode> markupsNode2 = vtkSmartPointer<vtkDMMLMarkupsNode>::Take(
    vtkDMMLMarkupsNode::SafeDownCast(markupsNode->CreateNodeInstance()));

  vtkNew<vtkDMMLScene> scene2;
  // Application logic - Handle creation of vtkDMMLSelectionNode and vtkDMMLInteractionNode
  vtkNew<vtkDMMLApplicationLogic> applicationLogic2;
  applicationLogic2->SetDMMLScene(scene2);

  vtkSmartPointer<vtkDMMLMarkupsStorageNode> snode2 = vtkSmartPointer<vtkDMMLMarkupsStorageNode>::Take(
    vtkDMMLMarkupsStorageNode::SafeDownCast(storageNode->CreateNodeInstance()));

  scene2->AddNode(snode2);
  scene2->AddNode(markupsNode2);
  markupsNode2->SetAndObserveStorageNodeID(snode2->GetID());
  snode2->SetFileName(storageNode->GetFileName());

  std::cout << "Reading from " << snode2->GetFileName() << std::endl;
  CHECK_BOOL(snode2->ReadData(markupsNode2), true);

  if (verbose)
    {
    std::cout << "\nMarkup read from file = " << std::endl;
    vtkIndent indent;
    markupsNode2->PrintSelf(std::cout, indent);
    std::cout << std::endl;
    }

  // test values on the first markup
  double newOrientation[4] = { -5, -5, -5, -5 };
  markupsNode2->GetNthControlPointOrientation(modifiedPointIndex, newOrientation);
  std::cout << "Orientation from read file: [" << newOrientation[0] << ", " << newOrientation[1] << ", "
    << newOrientation[2] << ", " << newOrientation[3] << "]" << std::endl;
  for (int r = 0; r < 4; r++)
    {
    CHECK_DOUBLE_TOLERANCE(newOrientation[r], orientation[r], 1e-3);
    }
  CHECK_STD_STRING(markupsNode2->GetNthControlPointAssociatedNodeID(modifiedPointIndex), associatedNodeID);
  CHECK_BOOL(markupsNode2->GetNthControlPointSelected(modifiedPointIndex), false);
  CHECK_BOOL(markupsNode2->GetNthControlPointVisibility(modifiedPointIndex), false);
  CHECK_BOOL(markupsNode2->GetNthControlPointLocked(modifiedPointIndex), true);
  CHECK_STD_STRING(markupsNode2->GetNthControlPointLabel(modifiedPointIndex), label);
  CHECK_STD_STRING(markupsNode2->GetNthControlPointDescription(modifiedPointIndex), desc);

  double outputPoint[3] = { -3, -3, -3 };
  markupsNode2->GetNthControlPointPosition(modifiedPointIndex, outputPoint);
  CHECK_DOUBLE_TOLERANCE(outputPoint[0], inputPoint[0], 0.1);
  CHECK_DOUBLE_TOLERANCE(outputPoint[1], inputPoint[1], 0.1);
  CHECK_DOUBLE_TOLERANCE(outputPoint[2], inputPoint[2], 0.1);

  if (testManyPoints)
    {
    // test the unset label on the third markup
    CHECK_STD_STRING(markupsNode2->GetNthControlPointLabel(emptyLabelIndex), "");
    }

  // now read it again with a display node defined
  vtkNew<vtkDMMLMarkupsDisplayNode> dispNode2;
  scene->AddNode(dispNode2);
  markupsNode2->SetAndObserveDisplayNodeID(dispNode2->GetID());
  std::cout << "Added display node, re-reading from " << snode2->GetFileName() << std::endl;
  CHECK_BOOL(snode2->ReadData(markupsNode2), true);

  //
  // test with RAS coordinate system
  storageNode->UseRASOn();
  std::cout << "Writing file in RAS coordinate system: " << storageNode->GetFileName() << std::endl;
  CHECK_BOOL(storageNode->WriteData(markupsNode), true);

  // read it in after clearing out the test data
  // Set to use LPS to verify that not this hint but the coordinate system that
  // is specified in the file is taken into account.
  snode2->UseLPSOn();
  markupsNode2->RemoveAllControlPoints();
  std::cout << "Reading file that uses RAS coordinate system from " << snode2->GetFileName() << std::endl;

  CHECK_BOOL(snode2->ReadData(markupsNode2), true);
  CHECK_BOOL(snode2->GetUseRAS(), true);

  if (verbose)
    {
    std::cout << "\nMarkups specified in RAS read from file: " << storageNode->GetFileName() << std::endl;
    vtkIndent indent;
    markupsNode2->PrintSelf(std::cout, indent);
    }

  // check the point coordinates are correct when stored in files in RAS coordinate system
  double outputPointLoadedFromRASFile[3];
  markupsNode2->GetNthControlPointPosition(modifiedPointIndex, outputPointLoadedFromRASFile);
  CHECK_DOUBLE_TOLERANCE(outputPointLoadedFromRASFile[0], inputPoint[0], 0.1);
  CHECK_DOUBLE_TOLERANCE(outputPointLoadedFromRASFile[1], inputPoint[1], 0.1);
  CHECK_DOUBLE_TOLERANCE(outputPointLoadedFromRASFile[2], inputPoint[2], 0.1);

  if (testManyPoints)
    {
    // check for commas in the markup label and description
    std::string labelWithCommas = markupsNode2->GetNthControlPointLabel(commaIndex);
    int numCommas = std::count(labelWithCommas.begin(), labelWithCommas.end(), ',');
    CHECK_INT(numCommas, 2);
    std::string descriptionWithCommasAndQuotes = markupsNode2->GetNthControlPointDescription(commaIndex);
    numCommas = std::count(descriptionWithCommasAndQuotes.begin(),
                           descriptionWithCommasAndQuotes.end(),
                           ',');
    CHECK_INT(numCommas, 1);
    int numQuotes = std::count(descriptionWithCommasAndQuotes.begin(),
                              descriptionWithCommasAndQuotes.end(),
                              '"');
    CHECK_INT(numQuotes, 2);

    // check ending quoted label
    std::string labelWithQuotes = markupsNode2->GetNthControlPointLabel(quotesIndex);
    numQuotes = std::count(labelWithQuotes.begin(), labelWithQuotes.end(), '"');
    CHECK_INT(numQuotes, 2);

    // check fully quoted description
    std::string descWithQuotes = markupsNode2->GetNthControlPointDescription(quotesIndex);
    numQuotes = std::count(descWithQuotes.begin(), descWithQuotes.end(), '"');
    CHECK_INT(numQuotes, 2);
    }

  return EXIT_SUCCESS;
}

int vtkDMMLMarkupsStorageNodeTest2(int argc, char* argv[])
{
  vtkNew<vtkDMMLMarkupsFiducialStorageNode> storageNodeFcsv;
  EXERCISE_ALL_BASIC_DMML_METHODS(storageNodeFcsv);

  vtkNew<vtkDMMLMarkupsJsonStorageNode> storageNodeJson;
  EXERCISE_ALL_BASIC_DMML_METHODS(storageNodeJson);

  // Test if information can be saved to file and retrieved
  std::string tempFolder = ".";
  if (argc > 1)
    {
    tempFolder = std::string(argv[1]);
    }
  CHECK_EXIT_SUCCESS(TestStoragNode(
    vtkSmartPointer<vtkDMMLMarkupsFiducialNode>::New(),
    vtkSmartPointer<vtkDMMLMarkupsFiducialStorageNode>::New(), tempFolder + "/vtkDMMLMarkupsStorageNodeTest2-fiducial-temp.fcsv"));
  CHECK_EXIT_SUCCESS(TestStoragNode(
    vtkSmartPointer<vtkDMMLMarkupsFiducialNode>::New(),
    vtkSmartPointer<vtkDMMLMarkupsJsonStorageNode>::New(), tempFolder + "/vtkDMMLMarkupsStorageNodeTest2-fiducial-temp.mrk.json"));
  CHECK_EXIT_SUCCESS(TestStoragNode(
    vtkSmartPointer<vtkDMMLMarkupsLineNode>::New(),
    vtkSmartPointer<vtkDMMLMarkupsJsonStorageNode>::New(), tempFolder + "/vtkDMMLMarkupsStorageNodeTest2-line-temp.mrk.json"));
  CHECK_EXIT_SUCCESS(TestStoragNode(
    vtkSmartPointer<vtkDMMLMarkupsAngleNode>::New(),
    vtkSmartPointer<vtkDMMLMarkupsJsonStorageNode>::New(), tempFolder + "/vtkDMMLMarkupsStorageNodeTest2-angle-temp.mrk.json"));
  CHECK_EXIT_SUCCESS(TestStoragNode(
    vtkSmartPointer<vtkDMMLMarkupsCurveNode>::New(),
    vtkSmartPointer<vtkDMMLMarkupsJsonStorageNode>::New(), tempFolder + "/vtkDMMLMarkupsStorageNodeTest2-opencurve-temp.mrk.json"));
  CHECK_EXIT_SUCCESS(TestStoragNode(
    vtkSmartPointer<vtkDMMLMarkupsClosedCurveNode>::New(),
    vtkSmartPointer<vtkDMMLMarkupsJsonStorageNode>::New(), tempFolder + "/vtkDMMLMarkupsStorageNodeTest2-closedcurve-temp.mrk.json"));
  CHECK_EXIT_SUCCESS(TestStoragNode(
    vtkSmartPointer<vtkDMMLMarkupsPlaneNode>::New(),
    vtkSmartPointer<vtkDMMLMarkupsPlaneJsonStorageNode>::New(), tempFolder + "/vtkDMMLMarkupsStorageNodeTest2-plane-temp.mrk.json"));
  CHECK_EXIT_SUCCESS(TestStoragNode(
    vtkSmartPointer<vtkDMMLMarkupsROINode>::New(),
    vtkSmartPointer<vtkDMMLMarkupsROIJsonStorageNode>::New(), tempFolder + "/vtkDMMLMarkupsStorageNodeTest2-roi-temp.mrk.json"));

  // Test if markups node can be instantiated correctly
  vtkNew<vtkDMMLScene> scene;
  // Application logic - Handle creation of vtkDMMLSelectionNode and vtkDMMLInteractionNode
  vtkNew<vtkDMMLApplicationLogic> applicationLogic;
  applicationLogic->SetDMMLScene(scene);
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLMarkupsFiducialNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLMarkupsLineNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLMarkupsAngleNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLMarkupsCurveNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLMarkupsClosedCurveNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLMarkupsDisplayNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLMarkupsPlaneDisplayNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLMarkupsPlaneNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLMarkupsPlaneJsonStorageNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLMarkupsFiducialDisplayNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLMarkupsROIDisplayNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLMarkupsROINode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLMarkupsROIJsonStorageNode>::New());

  scene->AddNode(storageNodeJson);

  vtkDMMLMarkupsNode* fiducialNode2 = storageNodeJson->AddNewMarkupsNodeFromFile(
    std::string(tempFolder + "/vtkDMMLMarkupsStorageNodeTest2-fiducial-temp.mrk.json").c_str());
  CHECK_NOT_NULL(fiducialNode2);
  CHECK_STRING(fiducialNode2->GetClassName(), "vtkDMMLMarkupsFiducialNode");

  vtkDMMLMarkupsNode* lineNode = storageNodeJson->AddNewMarkupsNodeFromFile(
    std::string(tempFolder + "/vtkDMMLMarkupsStorageNodeTest2-line-temp.mrk.json").c_str());
  CHECK_NOT_NULL(lineNode);
  CHECK_STRING(lineNode->GetClassName(), "vtkDMMLMarkupsLineNode");

  vtkDMMLMarkupsNode* angleNode = storageNodeJson->AddNewMarkupsNodeFromFile(
    std::string(tempFolder + "/vtkDMMLMarkupsStorageNodeTest2-angle-temp.mrk.json").c_str());
  CHECK_NOT_NULL(angleNode);
  CHECK_STRING(angleNode->GetClassName(), "vtkDMMLMarkupsAngleNode");

  vtkDMMLMarkupsNode* openCurveNode = storageNodeJson->AddNewMarkupsNodeFromFile(
    std::string(tempFolder + "/vtkDMMLMarkupsStorageNodeTest2-opencurve-temp.mrk.json").c_str());
  CHECK_NOT_NULL(openCurveNode);
  CHECK_STRING(openCurveNode->GetClassName(), "vtkDMMLMarkupsCurveNode");

  vtkDMMLMarkupsNode* closedurveNode = storageNodeJson->AddNewMarkupsNodeFromFile(
    std::string(tempFolder + "/vtkDMMLMarkupsStorageNodeTest2-closedcurve-temp.mrk.json").c_str());
  CHECK_NOT_NULL(closedurveNode);
  CHECK_STRING(closedurveNode->GetClassName(), "vtkDMMLMarkupsClosedCurveNode");

  vtkDMMLMarkupsNode* planeNode = storageNodeJson->AddNewMarkupsNodeFromFile(
    std::string(tempFolder + "/vtkDMMLMarkupsStorageNodeTest2-plane-temp.mrk.json").c_str());
  CHECK_NOT_NULL(planeNode);
  CHECK_STRING(planeNode->GetClassName(), "vtkDMMLMarkupsPlaneNode");

  vtkDMMLMarkupsNode* roiNode = storageNodeJson->AddNewMarkupsNodeFromFile(
    std::string(tempFolder + "/vtkDMMLMarkupsStorageNodeTest2-roi-temp.mrk.json").c_str());
  CHECK_NOT_NULL(roiNode);
  CHECK_STRING(roiNode->GetClassName(), "vtkDMMLMarkupsROINode");


  return EXIT_SUCCESS;
}
