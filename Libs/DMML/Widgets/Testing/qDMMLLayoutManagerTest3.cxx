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

// Common test driver includes
#include "qDMMLWidgetCxxTests.h"
#include "qDMMLLayoutManagerTestHelper.cxx"

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
#include <vtkCollection.h>
#include <vtkNew.h>
#include "qDMMLWidget.h"

// Common test driver includes
#include "qDMMLWidgetCxxTests.h"

namespace
{

// --------------------------------------------------------------------------
bool checkNumberOfItems(int line, qDMMLLayoutManager* layoutManager, int expected)
{
  vtkDMMLLayoutLogic * layoutLogic = layoutManager->layoutLogic();
  vtkCollection* viewNodes = layoutLogic->GetViewNodes();
  if (viewNodes->GetNumberOfItems() != expected)
    {
    std::cerr << "Line " << line << " - Problem with vtkDMMLLayoutLogic::GetViewNodes()\n"
              << " expected NumberOfItems: " << expected << "\n"
              << " current NumberOfItems: " << viewNodes->GetNumberOfItems() << std::endl;
    return false;
    }
  return true;
}

} // end of anonymous namespace

// --------------------------------------------------------------------------
int qDMMLLayoutManagerTest3(int argc, char * argv[] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  QWidget w;
  w.show();
  qDMMLLayoutManager* layoutManager = new qDMMLLayoutManager(&w, &w);

  vtkNew<vtkDMMLScene> scene;

  vtkNew<vtkDMMLApplicationLogic> applicationLogic;
  vtkDMMLSliceViewDisplayableManagerFactory::GetInstance()->SetDMMLApplicationLogic(applicationLogic);

  vtkDMMLLayoutNode * layoutNode = nullptr;
  {
    vtkNew<vtkDMMLLayoutNode> newLayoutNode;

    // The view arrangement can be set before the view descriptions are registered
    // into the layout node. Setting the scene to the layout manager will set the
    // the scene to the layout logic which will register the layout descriptions.
    TESTING_OUTPUT_ASSERT_WARNINGS_BEGIN();
    newLayoutNode->SetViewArrangement(vtkDMMLLayoutNode::CjyxLayoutOneUpRedSliceView);
    TESTING_OUTPUT_ASSERT_WARNINGS_END();

    layoutNode = vtkDMMLLayoutNode::SafeDownCast(scene->AddNode(newLayoutNode.GetPointer()));
  }

  applicationLogic->SetDMMLScene(scene.GetPointer());
  layoutManager->setDMMLScene(scene.GetPointer());

  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkDMMLLayoutNode::CjyxLayoutOneUpRedSliceView))
    {
    return EXIT_FAILURE;
    }
  if (!checkNumberOfItems(__LINE__, layoutManager, /* expected = */ 1))
    {
    return EXIT_FAILURE;
    }

  layoutNode->SetViewArrangement(vtkDMMLLayoutNode::CjyxLayoutOneUpGreenSliceView);
  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkDMMLLayoutNode::CjyxLayoutOneUpGreenSliceView))
    {
    return EXIT_FAILURE;
    }
  if (!checkNumberOfItems(__LINE__, layoutManager, /* expected = */ 1))
    {
    return EXIT_FAILURE;
    }

  layoutNode->SetViewArrangement(vtkDMMLLayoutNode::CjyxLayoutConventionalView);
  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkDMMLLayoutNode::CjyxLayoutConventionalView))
    {
    return EXIT_FAILURE;
    }
  if (!checkNumberOfItems(__LINE__, layoutManager, /* expected = */ 4))
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
