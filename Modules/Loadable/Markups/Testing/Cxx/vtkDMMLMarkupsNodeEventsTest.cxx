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
#include "vtkDMMLMarkupsFiducialNode.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkIndent.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkTestingOutputWindow.h>

// STL includes
#include <vector>

#include "vtkDMMLCoreTestingMacros.h"

namespace
{

class vtkDMMLMarkupNodeObserver : public vtkCommand
{
public:
  static vtkDMMLMarkupNodeObserver *New()
    {
    return new vtkDMMLMarkupNodeObserver;
    }

  vtkDMMLMarkupNodeObserver()
    {
    }

  void Execute(vtkObject *caller, unsigned long event, void*) override
    {
    vtkDMMLDisplayableNode* dispNode = vtkDMMLDisplayableNode::SafeDownCast(caller);
    if (!dispNode)
      {
      return;
      }
    invokedEvents.push_back(event);
    }

  std::vector<int> invokedEvents;
};

void addEventsToObserver(vtkDMMLMarkupsNode* node, vtkDMMLMarkupNodeObserver* observer)
{
  node->AddObserver(vtkDMMLMarkupsNode::LockModifiedEvent, observer);
  node->AddObserver(vtkDMMLMarkupsNode::LabelFormatModifiedEvent, observer);
  node->AddObserver(vtkDMMLMarkupsNode::PointAddedEvent, observer);
  node->AddObserver(vtkDMMLMarkupsNode::PointRemovedEvent, observer);
  node->AddObserver(vtkDMMLMarkupsNode::PointPositionDefinedEvent, observer);
  node->AddObserver(vtkDMMLMarkupsNode::PointPositionUndefinedEvent, observer);
  node->AddObserver(vtkDMMLMarkupsNode::PointPositionMissingEvent, observer);
  node->AddObserver(vtkDMMLMarkupsNode::PointPositionNonMissingEvent, observer);
  node->AddObserver(vtkDMMLMarkupsNode::PointModifiedEvent, observer);
  node->AddObserver(vtkDMMLMarkupsNode::CenterOfRotationModifiedEvent, observer);
  node->AddObserver(vtkDMMLMarkupsNode::FixedNumberOfControlPointsModifiedEvent, observer);
  node->AddObserver(vtkDMMLMarkupsNode::PointAboutToBeRemovedEvent, observer);
}

bool containsEvent(vtkDMMLMarkupNodeObserver* observer, int eventId)
{
  bool found = std::find(observer->invokedEvents.begin(), observer->invokedEvents.end(),
    eventId) != observer->invokedEvents.end();
  observer->invokedEvents.clear();
  return found;
}

}

int vtkDMMLMarkupsNodeEventsTest(int, char* [])
{
  vtkNew<vtkDMMLMarkupsFiducialNode> node;
  vtkNew<vtkDMMLScene> scene;
  vtkNew<vtkDMMLMarkupNodeObserver> observer;

  addEventsToObserver(node, observer);

  scene->AddNode(node.GetPointer());

  // Default settings
  vtkVector3d point1, point2;
  point1[0] = 10;
  point1[1] = 20;
  point1[2] = 30;

  // Test 1: PointAddedEvent
  node->AddControlPoint(point1, "first point");
  CHECK_BOOL(containsEvent(observer, vtkDMMLMarkupsNode::PointAddedEvent), true);

  // Test 2: LockModifiedEvent
  node->SetLocked(true);
  CHECK_BOOL(containsEvent(observer, vtkDMMLMarkupsNode::LockModifiedEvent), true);

  // Test 3: LabelFormatModifiedEvent
  node->SetMarkupLabelFormat("the value is: %");
  CHECK_BOOL(containsEvent(observer, vtkDMMLMarkupsNode::LabelFormatModifiedEvent), true);

  // Test 4: PointRemovedEvent
  node->RemoveNthControlPoint(0);
  CHECK_BOOL(containsEvent(observer, vtkDMMLMarkupsNode::PointRemovedEvent), true);

  // Test 5: PointPositionDefinedEvent
  node->AddControlPoint(point1, "first point");
  node->SetNthControlPointPosition(0, point2[0], point2[1], point2[2]);
  CHECK_BOOL(containsEvent(observer, vtkDMMLMarkupsNode::PointPositionDefinedEvent), true);

  // Test 6: PointPositionUndefinedEvent
  node->UnsetNthControlPointPosition(0);
  CHECK_BOOL(containsEvent(observer, vtkDMMLMarkupsNode::PointPositionUndefinedEvent), true);

  // Test 7: PointPositionMissingEvent
  node->SetNthControlPointPositionMissing(0);
  CHECK_BOOL(containsEvent(observer, vtkDMMLMarkupsNode::PointPositionMissingEvent), true);

  // Test 8: PointPositionNonMissingEvent
  node->SetNthControlPointPosition(0, point2[0], point2[1], point2[2]);
  CHECK_BOOL(containsEvent(observer, vtkDMMLMarkupsNode::PointPositionNonMissingEvent), true);

  // Test 9: PointModifiedEvent
  node->SetNthControlPointLabel(0, "new first point");
  CHECK_BOOL(containsEvent(observer, vtkDMMLMarkupsNode::PointModifiedEvent), true);

  // Test 10: CenterPointModifiedEvent
  node->SetCenterOfRotation(point2.GetData());
  CHECK_BOOL(containsEvent(observer, vtkDMMLMarkupsNode::CenterOfRotationModifiedEvent), true);

  // Test 11: FixedNumberOfControlPointsModifiedEvent
  node->SetFixedNumberOfControlPoints(true);
  node->SetFixedNumberOfControlPoints(false);
  CHECK_BOOL(containsEvent(observer, vtkDMMLMarkupsNode::FixedNumberOfControlPointsModifiedEvent), true);

  // Test 12: PointAboutToBeRemovedEvent
  node->RemoveNthControlPoint(0);
  CHECK_BOOL(containsEvent(observer, vtkDMMLMarkupsNode::PointAboutToBeRemovedEvent), true);

  return EXIT_SUCCESS;
}
