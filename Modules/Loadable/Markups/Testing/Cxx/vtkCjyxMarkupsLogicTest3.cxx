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

// Annotations includes
#include "vtkDMMLAnnotationFiducialNode.h"
#include "vtkDMMLAnnotationHierarchyNode.h"
#include "vtkDMMLAnnotationPointDisplayNode.h"
#include "vtkCjyxAnnotationModuleLogic.h"

// Markups includes
#include "vtkDMMLMarkupsNode.h"
#include "vtkDMMLMarkupsFiducialNode.h"
#include "vtkCjyxMarkupsLogic.h"

// DMML includes
#include "vtkDMMLApplicationLogic.h"
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLScene.h"


// VTK includes
#include <vtkNew.h>


int vtkCjyxMarkupsLogicTest3(int , char * [] )
{
  vtkNew<vtkCjyxMarkupsLogic> logic1;
  vtkNew<vtkDMMLScene> scene;
  // add a selection node
  vtkDMMLApplicationLogic* applicationLogic = vtkDMMLApplicationLogic::New();
  applicationLogic->SetDMMLScene(scene.GetPointer());

  // Test converting annotations in the scene to markups

  // no scene
  logic1->ConvertAnnotationFiducialsToMarkups();

  // empty scene
  logic1->SetDMMLScene(scene.GetPointer());
  logic1->ConvertAnnotationFiducialsToMarkups();

  // set up the annotation module logic
  vtkNew<vtkCjyxAnnotationModuleLogic> annotLogic;
  annotLogic->SetDMMLScene(scene.GetPointer());

  // add some annotations
  for (int n = 0; n < 10; n++)
    {
    vtkNew<vtkDMMLAnnotationFiducialNode> annotFid;
    double p1[3] = {1.1, -2.2, 3.3};
    p1[0] = static_cast<double>(n);
    annotFid->SetFiducialCoordinates(p1);
    annotFid->Initialize(scene.GetPointer());
    }
  std::cout << "After one list, active hierarchy = " << annotLogic->GetActiveHierarchyNode()->GetID() << std::endl;
  // and another hierarchy and make it active
  annotLogic->AddHierarchy();
  std::cout << "After adding a new hierarchy, active hierarchy = " << annotLogic->GetActiveHierarchyNode()->GetID() << std::endl;
  // add some more annotations
  for (int n = 0; n < 5; n++)
    {
    vtkNew<vtkDMMLAnnotationFiducialNode> annotFid;
    double p1[3] = {5.5, -6.6, 0.0};
    p1[2] = static_cast<double>(n);
    annotFid->SetFiducialCoordinates(p1);
    annotFid->Initialize(scene.GetPointer());
    if (n == 3)
      {
      annotFid->SetDescription("testing description");
      }
    if (n == 4)
      {
      annotFid->SetAttribute("AssociatedNodeID", "vtkDMMLScalarVolumeNode4");
      }
    }

  // convert and test
  logic1->ConvertAnnotationFiducialsToMarkups();

  int numAnnotationFiducials = scene->GetNumberOfNodesByClass("vtkDMMLAnnotationFiducialNode");
  int numMarkupsFiducials = scene->GetNumberOfNodesByClass("vtkDMMLMarkupsFiducialNode");
  if (numAnnotationFiducials != 0 ||
      numMarkupsFiducials != 2)
    {
    std::cerr << "Failed to convert 15 annotation fiducials in two hierarchies "
    << " to 2 markup lists, have " << numAnnotationFiducials
    << " annotation fiduicals and " << numMarkupsFiducials
    << " markups fiducial lists" << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Converted annotation fiducials to " << numMarkupsFiducials
              << " markups fiducial lists" << std::endl;
    }
//  vtkIndent indent;
//  for (int n = 0; n < numMarkupsFiducials; ++n)
//    {
//    vtkDMMLNode *dmmlNode = scene->GetNthNodeByClass(n, "vtkDMMLMarkupsFiducialNode");
//    std::cout << "\nConverted Markups node " << n << ":" << std::endl;
//    dmmlNode->PrintSelf(std::cout, indent);
//    }

  // clean up before testing
  applicationLogic->SetDMMLScene(nullptr);
  logic1->SetDMMLScene(nullptr);
  annotLogic->SetDMMLScene(nullptr);
  applicationLogic->Delete();

  // check the second list
  vtkDMMLNode *dmmlNode = scene->GetNthNodeByClass(1, "vtkDMMLMarkupsFiducialNode");
  if (dmmlNode)
    {
    vtkDMMLMarkupsFiducialNode *markupsFid = vtkDMMLMarkupsFiducialNode::SafeDownCast(dmmlNode);
    if (markupsFid)
      {
      std::string desc = markupsFid->GetNthControlPointDescription(3);
      if (desc.compare("testing description") != 0)
        {
        std::cerr << "Failed to get the expected description on markup 3, got: "
                  << desc.c_str() << std::endl;
        return EXIT_FAILURE;
        }
      std::string assocNodeID = markupsFid->GetNthControlPointAssociatedNodeID(4);
      if (assocNodeID.compare("vtkDMMLScalarVolumeNode4") != 0)
        {
        std::cerr << "Failed to get the expected associated node id on markup 4, got: "
                  << assocNodeID.c_str() << std::endl;
        return EXIT_FAILURE;
        }
      vtkVector3d posVector = markupsFid->GetNthControlPointPositionVector(0);
      double* pos = posVector.GetData();
      double expectedPos[3] = {5.5, -6.6, 0.0};
      if (vtkMath::Distance2BetweenPoints(pos, expectedPos) > 0.01)
        {
        std::cerr << "Expected 0th position of 5.5, -6.6, 0.0, but got: "
                  << pos[0] << "," << pos[1] << "," << pos[2] << std::endl;
        return EXIT_FAILURE;
        }
      vtkDMMLMarkupsDisplayNode *dispNode = markupsFid->GetMarkupsDisplayNode();
      if (dispNode)
        {
        double col[3];
        dispNode->GetColor(col);
        double annotCol[3];
        vtkNew<vtkDMMLAnnotationPointDisplayNode> pointDispNode;
        pointDispNode->GetColor(annotCol);
        if (vtkMath::Distance2BetweenPoints(col, annotCol) > 0.01)
          {
          std::cerr << "Failed to set color, expected "
                    << annotCol[0] << ","
                    << annotCol[1] << ","
                    << annotCol[2] << ", but got: "
                    << col[0] << ","
                    << col[1] << ","
                    << col[2] << std::endl;
          return EXIT_FAILURE;
          }
        }
      else
        {
        std::cerr << "Second markups node doesn't have a display node!"
                  << std::endl;
        return EXIT_FAILURE;
        }
      }
    else
      {
      std::cerr << "Unable to get second markups fiducial node for testing!" << std::endl;
      return EXIT_FAILURE;
      }
    }
  // cleanup
  scene->Clear(1);

  return EXIT_SUCCESS;
}
