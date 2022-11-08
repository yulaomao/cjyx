/*==============================================================================

  Copyright (c) The Intervention Centre
  Oslo University Hospital, Oslo, Norway. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Rafael Palomar (The Intervention Centre,
  Oslo University Hospital) and was supported by The Research Council of Norway
  through the ALive project (grant nr. 311393).

==============================================================================*/

// This tests functionality related to the registration of markup items in
// relation to the pluggable markups architecture.

// DMML Markups nodes includes
#include "vtkDMMLMarkupsAngleNode.h"
#include "vtkDMMLMarkupsFiducialNode.h"

// VTK Widgets Markups includes
#include "vtkCjyxAngleWidget.h"
#include "vtkCjyxPointsWidget.h"

// Markups logic includes
#include "vtkCjyxMarkupsLogic.h"

// DMML includes
#include <vtkDMMLApplicationLogic.h>
#include <vtkDMMLCoreTestingMacros.h>
#include <vtkDMMLScene.h>

// VTK includes
#include<vtkCallbackCommand.h>
#include<vtkNew.h>
#include<vtkSmartPointer.h>

//------------------------------------------------------------------------------
void RegisteredEventDetectionCallback(vtkObject *caller, unsigned long, void*, void*);
void UnregisteredEventDetectionCallback(vtkObject *caller, unsigned long, void*, void*);

//------------------------------------------------------------------------------
bool registeredEventReceived = false;
bool unregisteredEventReceived = false;

//------------------------------------------------------------------------------
int vtkCjyxMarkupsLogicTest4(int , char*[])
{
  vtkNew<vtkDMMLScene> scene;

  // Application logic - Creates vtkDMMLSelectionNode and vtkDMMLInteractionNode
  vtkNew<vtkDMMLApplicationLogic> applicationLogic;
  applicationLogic->SetDMMLScene(scene);

  vtkNew<vtkCjyxMarkupsLogic> logic4;
  logic4->SetDMMLScene(scene);

  // Set a callback for detecting registration events
  vtkNew<vtkCallbackCommand> registeredCallbackCommand;
  registeredCallbackCommand->SetCallback(RegisteredEventDetectionCallback);
  logic4->AddObserver(vtkCjyxMarkupsLogic::MarkupRegistered, registeredCallbackCommand);

  // Set a callback for detecting unregistration events
  vtkNew<vtkCallbackCommand> unregisteredCallbackCommand;
  unregisteredCallbackCommand->SetCallback(UnregisteredEventDetectionCallback);
  logic4->AddObserver(vtkCjyxMarkupsLogic::MarkupUnregistered, unregisteredCallbackCommand);

  TESTING_OUTPUT_ASSERT_WARNINGS_BEGIN();
  // Test registration of a Markups Node with correct node and widget
  logic4->RegisterMarkupsNode(vtkSmartPointer<vtkDMMLMarkupsFiducialNode>::New(),
                              vtkSmartPointer<vtkCjyxPointsWidget>::New());
  TESTING_OUTPUT_ASSERT_WARNINGS_END();
  CHECK_BOOL(registeredEventReceived, false);  // already registered, should not re-register

  // Test registration of a Markups Node with nullptr node
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  registeredEventReceived = false;
  logic4->RegisterMarkupsNode(nullptr, vtkSmartPointer<vtkCjyxPointsWidget>::New(), false);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  CHECK_BOOL(registeredEventReceived, false);

  // Test registration of a Markups Node with nullptr widget
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  registeredEventReceived = false;
  logic4->RegisterMarkupsNode(vtkSmartPointer<vtkDMMLMarkupsFiducialNode>::New(), nullptr, false);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  CHECK_BOOL(registeredEventReceived, false);

  // Try to register the same markup node. It should trigger a warning macro
  TESTING_OUTPUT_ASSERT_WARNINGS_BEGIN();
  registeredEventReceived = false;
  logic4->RegisterMarkupsNode(vtkSmartPointer<vtkDMMLMarkupsFiducialNode>::New(),
                              vtkSmartPointer<vtkCjyxPointsWidget>::New(), false);
  TESTING_OUTPUT_ASSERT_WARNINGS_END();
  CHECK_BOOL(registeredEventReceived, false);

  // Try to unregister a nullptr node
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  unregisteredEventReceived = false;
  logic4->UnregisterMarkupsNode(nullptr);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  CHECK_BOOL(unregisteredEventReceived, false);

  // Try to unregister a valid node
  unregisteredEventReceived = false;
  logic4->UnregisterMarkupsNode(vtkSmartPointer<vtkDMMLMarkupsFiducialNode>::New());
  CHECK_BOOL(unregisteredEventReceived, true);

  // Try to unregister a non-registered markup
  TESTING_OUTPUT_ASSERT_WARNINGS_BEGIN();
  unregisteredEventReceived = false;
  logic4->UnregisterMarkupsNode(vtkSmartPointer<vtkDMMLMarkupsFiducialNode>::New());
  TESTING_OUTPUT_ASSERT_WARNINGS_END();
  CHECK_BOOL(unregisteredEventReceived, false);

  logic4->UnregisterMarkupsNode(vtkSmartPointer<vtkDMMLMarkupsAngleNode>::New());

  vtkNew<vtkDMMLMarkupsFiducialNode> markupsFiducialNode;
  vtkNew<vtkDMMLMarkupsAngleNode> markupsAngleNode;
  vtkNew<vtkCjyxPointsWidget> markupsPointsWidget;
  vtkNew<vtkCjyxAngleWidget> markupsAngleWidget;

  // Register non-registered valid nodes and retrieve the values
  registeredEventReceived = false;
  logic4->RegisterMarkupsNode(markupsFiducialNode, markupsPointsWidget, false);
  CHECK_BOOL(registeredEventReceived, true);
  registeredEventReceived = false;
  logic4->RegisterMarkupsNode(markupsAngleNode, markupsAngleWidget, true);
  CHECK_BOOL(registeredEventReceived, true);
  if (logic4->GetWidgetByMarkupsType(markupsFiducialNode->GetMarkupType()) != markupsPointsWidget)
    {
    std::cerr << "Error in GetWidgetByMarkupsType: obtained widget does not match the expected one." << std::endl;
    return EXIT_FAILURE;
    }
  if (logic4->GetNodeByMarkupsType(markupsFiducialNode->GetMarkupType()) != markupsFiducialNode)
    {
    std::cerr << "Error in GetNodeByMarkupsType: obtained node does not match the expected one." << std::endl;
    return EXIT_FAILURE;
    }
  if (logic4->GetCreateMarkupsPushButton(markupsFiducialNode->GetMarkupType()) != false)
    {
    std::cerr << "Error in GetCreateMarkupsPushButton: obtained value does not match the expected one." << std::endl;
    return EXIT_FAILURE;
    }
  if (logic4->GetWidgetByMarkupsType(markupsAngleNode->GetMarkupType()) != markupsAngleWidget)
    {
    std::cerr << "Error in GetWidgetByMarkupsType: obtained widget does not match the expected one." << std::endl;
    return EXIT_FAILURE;
    }
  if (logic4->GetNodeByMarkupsType(markupsAngleNode->GetMarkupType()) != markupsAngleNode)
    {
    std::cerr << "Error in GetNodeByMarkupsType: obtained widget does not match the expected one." << std::endl;
    return EXIT_FAILURE;
    }
  if (logic4->GetCreateMarkupsPushButton(markupsAngleNode->GetMarkupType()) != true)
    {
    std::cerr << "Error in GetCreateMarkupsPushButton: obtained value does not match the expected one." << std::endl;
    return EXIT_FAILURE;
    }

  // Get registered markups with a nullptr
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  logic4->GetWidgetByMarkupsType(nullptr);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  logic4->GetNodeByMarkupsType(nullptr);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  logic4->GetCreateMarkupsPushButton(nullptr);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  // Get registered markups with an invalid name
  vtkDMMLMarkupsNode* node = logic4->GetNodeByMarkupsType("invalid name");
  CHECK_NULL(node);

  vtkCjyxMarkupsWidget* widget = logic4->GetWidgetByMarkupsType("invalid name");
  CHECK_NULL(widget);

  return EXIT_SUCCESS;
}

//------------------------------------------------------------------------------
void RegisteredEventDetectionCallback(vtkObject*, unsigned long, void*, void*)
{
  registeredEventReceived = true;
}

//------------------------------------------------------------------------------
void UnregisteredEventDetectionCallback(vtkObject*, unsigned long, void*, void*)
{
  unregisteredEventReceived = true;
}
