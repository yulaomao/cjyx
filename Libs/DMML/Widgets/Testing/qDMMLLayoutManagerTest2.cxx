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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QWidget>

// Cjyx includes
#include "qDMMLLayoutManager.h"
#include "vtkCjyxConfigure.h"

// DMML includes
#include <vtkDMMLApplicationLogic.h>
#include <vtkDMMLLayoutLogic.h>
#include <vtkDMMLLayoutNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceViewDisplayableManagerFactory.h>

// VTK includes
#include <vtkNew.h>
#include "qDMMLWidget.h"

// Common test driver includes
#include "qDMMLWidgetCxxTests.h"
#include "qDMMLLayoutManagerTestHelper.cxx"

// --------------------------------------------------------------------------
int qDMMLLayoutManagerTest2(int argc, char * argv[] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  QWidget w;
  w.show();
  qDMMLLayoutManager* layoutManager = new qDMMLLayoutManager(&w, &w);

  vtkNew<vtkDMMLApplicationLogic> applicationLogic;
  vtkDMMLSliceViewDisplayableManagerFactory::GetInstance()->SetDMMLApplicationLogic(applicationLogic);

  {
    vtkNew<vtkDMMLScene> scene;
    applicationLogic->SetDMMLScene(scene.GetPointer());
    layoutManager->setDMMLScene(scene.GetPointer());
    if (layoutManager->dmmlScene() != scene.GetPointer())
      {
      std::cerr << __LINE__ << " Problem with setDMMLScene()" << std::endl;
      return EXIT_FAILURE;
      }

    vtkDMMLLayoutNode* layoutNode = layoutManager->layoutLogic()->GetLayoutNode();

    if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkDMMLLayoutNode::CjyxLayoutInitialView))
      {
      return EXIT_FAILURE;
      }

    int expectedThreeDViewCout = 1;
    int currentThreeDViewCount = layoutManager->threeDViewCount();
    if (expectedThreeDViewCout != currentThreeDViewCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with qDMMLLayoutManager\n"
                << "  expectedThreeDViewCout:" << expectedThreeDViewCout << "\n"
                << "  currentThreeDViewCount:" << currentThreeDViewCount << std::endl;
      return EXIT_FAILURE;
      }

    int expectedSliceViewCout = 3;
    int currentSliceViewCount = layoutManager->sliceViewNames().count();
    if (expectedSliceViewCout != currentSliceViewCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with qDMMLLayoutManager\n"
                << "  expectedSliceViewCout:" << expectedSliceViewCout << "\n"
                << "  currentSliceViewCount:" << currentSliceViewCount << std::endl;
      return EXIT_FAILURE;
      }

    int expectedTableViewCout = 0;
    int currentTableViewCount = layoutManager->tableViewCount();
    if (expectedTableViewCout != currentTableViewCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with qDMMLLayoutManager\n"
                << "  expectedTableViewCout:" << expectedTableViewCout << "\n"
                << "  currentTableViewCount:" << currentTableViewCount << std::endl;
      return EXIT_FAILURE;
      }

    layoutManager->setDMMLScene(nullptr);
    applicationLogic->SetDMMLScene(nullptr);

    int current = scene->GetReferenceCount();
    int expected = 1;
    if (current != expected)
      {
      std::cerr << __LINE__ << " Problem with DMMLScene reference count !\n"
                << "  current: " << current << "\n"
                << "  expected: " << expected << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    // Setting a new scene is expected to reset the factories
    vtkNew<vtkDMMLScene> scene;
    applicationLogic->SetDMMLScene(scene.GetPointer());
    layoutManager->setDMMLScene(scene.GetPointer());
    vtkDMMLLayoutNode* layoutNode = layoutManager->layoutLogic()->GetLayoutNode();

    if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkDMMLLayoutNode::CjyxLayoutInitialView))
      {
      return EXIT_FAILURE;
      }

    int expectedThreeDViewCout = 1;
    int currentThreeDViewCount = layoutManager->threeDViewCount();
    if (expectedThreeDViewCout != currentThreeDViewCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with qDMMLLayoutManager\n"
                << "  expectedThreeDViewCout:" << expectedThreeDViewCout << "\n"
                << "  currentThreeDViewCount:" << currentThreeDViewCount << std::endl;
      return EXIT_FAILURE;
      }

    int expectedSliceViewCout = 3;
    int currentSliceViewCount = layoutManager->sliceViewNames().count();
    if (expectedSliceViewCout != currentSliceViewCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with qDMMLLayoutManager\n"
                << "  expectedSliceViewCout:" << expectedSliceViewCout << "\n"
                << "  currentSliceViewCount:" << currentSliceViewCount << std::endl;
      return EXIT_FAILURE;
      }

    int expectedTableViewCout = 0;
    int currentTableViewCount = layoutManager->tableViewCount();
    if (expectedTableViewCout != currentTableViewCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with qDMMLLayoutManager\n"
                << "  expectedTableViewCout:" << expectedTableViewCout << "\n"
                << "  currentTableViewCount:" << currentTableViewCount << std::endl;
      return EXIT_FAILURE;
      }

  }
  vtkDMMLLayoutNode* layoutNode = nullptr;
  {
    vtkNew<vtkDMMLScene> scene;
    vtkNew<vtkDMMLLayoutNode> newLayoutNode;

    // The view arrangement can be set before the view descriptions are registered, but it will log warning
    TESTING_OUTPUT_ASSERT_WARNINGS_BEGIN();
    newLayoutNode->SetViewArrangement(vtkDMMLLayoutNode::CjyxLayoutOneUpRedSliceView);
    TESTING_OUTPUT_ASSERT_WARNINGS_END();

    layoutNode = vtkDMMLLayoutNode::SafeDownCast(scene->AddNode(newLayoutNode.GetPointer()));
    applicationLogic->SetDMMLScene(scene.GetPointer());
    layoutManager->setDMMLScene(scene.GetPointer());
  }

  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkDMMLLayoutNode::CjyxLayoutOneUpRedSliceView))
    {
    return EXIT_FAILURE;
    }

  layoutNode->SetViewArrangement(vtkDMMLLayoutNode::CjyxLayoutOneUpGreenSliceView);
  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkDMMLLayoutNode::CjyxLayoutOneUpGreenSliceView))
    {
    return EXIT_FAILURE;
    }

  layoutManager->setLayout(vtkDMMLLayoutNode::CjyxLayoutCompareView);
  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkDMMLLayoutNode::CjyxLayoutCompareView))
    {
    return EXIT_FAILURE;
    }

  vtkDMMLScene * scene = layoutManager->dmmlScene();

  scene->StartState(vtkDMMLScene::ImportState);
  scene->EndState(vtkDMMLScene::ImportState);

  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkDMMLLayoutNode::CjyxLayoutCompareView))
    {
    return EXIT_FAILURE;
    }

  scene->StartState(vtkDMMLScene::ImportState);
  layoutNode->SetViewArrangement(vtkDMMLLayoutNode::CjyxLayoutOneUpGreenSliceView);
  scene->EndState(vtkDMMLScene::ImportState);

  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkDMMLLayoutNode::CjyxLayoutOneUpGreenSliceView))
    {
    return EXIT_FAILURE;
    }

  scene->StartState(vtkDMMLScene::CloseState);
  scene->EndState(vtkDMMLScene::CloseState);

  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkDMMLLayoutNode::CjyxLayoutOneUpGreenSliceView))
    {
    return EXIT_FAILURE;
    }

  scene->StartState(vtkDMMLScene::CloseState);
  layoutNode->SetViewArrangement(vtkDMMLLayoutNode::CjyxLayoutOneUpRedSliceView);
  scene->EndState(vtkDMMLScene::CloseState);

  if (!checkViewArrangement(__LINE__, layoutManager,layoutNode,  vtkDMMLLayoutNode::CjyxLayoutOneUpRedSliceView))
    {
    return EXIT_FAILURE;
    }

  // The layout is changed to none only if vtkDMMLScene::Clear() is called
  scene->StartState(vtkDMMLScene::CloseState);

  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkDMMLLayoutNode::CjyxLayoutOneUpRedSliceView))
    {
    return EXIT_FAILURE;
    }

  // Imitates what vtkDMMLScene::Clear() would have done:
  layoutNode->SetViewArrangement(vtkDMMLLayoutNode::CjyxLayoutNone);

  // and restore it back
  scene->EndState(vtkDMMLScene::CloseState);

  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkDMMLLayoutNode::CjyxLayoutOneUpRedSliceView))
    {
    return EXIT_FAILURE;
    }

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    return safeApplicationQuit(&app);
    }
  else
    {
    return app.exec();
    }
}

