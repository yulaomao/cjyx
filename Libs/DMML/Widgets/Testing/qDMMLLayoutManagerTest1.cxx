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
#include <qDMMLWidgetsConfigure.h> // For DMML_WIDGETS_HAVE_WEBENGINE_SUPPORT
#include "qDMMLLayoutManager.h"
#include "qDMMLSliceWidget.h"
#include "qDMMLTableWidget.h"
#include "qDMMLThreeDWidget.h"
#include "vtkCjyxConfigure.h"

// DMML includes
#include <vtkDMMLApplicationLogic.h>
#include <vtkDMMLLayoutNode.h>
#include <vtkDMMLLayoutLogic.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLSliceViewDisplayableManagerFactory.h>
#include <vtkDMMLTableViewNode.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkNew.h>
#include "qDMMLWidget.h"

namespace
{
//------------------------------------------------------------------------------
bool testLayoutManagerViewWidgetForTable(int line, qDMMLLayoutManager* layoutManager, int viewId)
{
  qDMMLTableWidget* widget = layoutManager->tableWidget(viewId);
  vtkDMMLTableViewNode* node = widget ? widget->dmmlTableViewNode() : nullptr;
  if (!widget || !node)
    {
    std::cerr << "Line " << line << " - Problem with qDMMLLayoutManager::tableWidget()" << std::endl;
    return false;
    }
  if (layoutManager->viewWidget(node) != widget)
    {
    std::cerr << "Line " << line << " - Problem with qDMMLLayoutManager::viewWidget()" << std::endl;
    return false;
    }
  return true;
}
//------------------------------------------------------------------------------
bool testLayoutManagerViewWidgetForSlice(int line, qDMMLLayoutManager* layoutManager, const char* viewName)
{
  qDMMLSliceWidget* widget = layoutManager->sliceWidget(viewName);
  vtkDMMLSliceNode* node = widget ? widget->dmmlSliceNode() : nullptr;
  if (!widget || !node)
    {
    std::cerr << "Line " << line << " - Problem with qDMMLLayoutManager::sliceWidget()" << std::endl;
    return false;
    }
  if (layoutManager->viewWidget(node) != widget)
    {
    std::cerr << "Line " << line << " - Problem with qDMMLLayoutManager::viewWidget()" << std::endl;
    return false;
    }
  return true;
}
//------------------------------------------------------------------------------
bool testLayoutManagerViewWidgetForThreeD(int line, qDMMLLayoutManager* layoutManager, int viewId)
{
  qDMMLThreeDWidget* widget = layoutManager->threeDWidget(viewId);
  vtkDMMLViewNode* node = widget ? widget->dmmlViewNode() : nullptr;
  if (!widget || !node)
    {
    std::cerr << "Line " << line << " - Problem with qDMMLLayoutManager::threeDWidget()" << std::endl;
    return false;
    }
  if (layoutManager->viewWidget(node) != widget)
    {
    std::cerr << "Line " << line << " - Problem with qDMMLLayoutManager::viewWidget()" << std::endl;
    return false;
    }
  return true;
}
}

//------------------------------------------------------------------------------
int qDMMLLayoutManagerTest1(int argc, char * argv[] )
{
  (void)checkViewArrangement; // Fix -Wunused-function warning

  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();
  qDMMLLayoutManager * layoutManager = new qDMMLLayoutManager();

  vtkNew<vtkDMMLApplicationLogic> applicationLogic;
  vtkDMMLSliceViewDisplayableManagerFactory::GetInstance()->SetDMMLApplicationLogic(applicationLogic);
  {
    vtkNew<vtkDMMLScene> scene;
    applicationLogic->SetDMMLScene(scene.GetPointer());
    layoutManager->setDMMLScene(scene.GetPointer());
    if (layoutManager->dmmlScene() != scene.GetPointer())
      {
      std::cerr << "Line " << __LINE__ << " - Problem with qDMMLLayoutManager::setDMMLScene()" << std::endl;
      return EXIT_FAILURE;
      }
    layoutManager->setDMMLScene(nullptr);
    applicationLogic->SetDMMLScene(nullptr);
    if (layoutManager->dmmlScene() != nullptr)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with qDMMLLayoutManager::setDMMLScene()" << std::endl;
      return EXIT_FAILURE;
      }
  }

  vtkNew<vtkDMMLScene> scene;
  applicationLogic->SetDMMLScene(scene.GetPointer());
  layoutManager->setDMMLScene(scene.GetPointer());

  QWidget * viewport = new QWidget;
  viewport->setWindowTitle("Old widget");
  layoutManager->setViewport(viewport);
  viewport->show();
  layoutManager->setViewport(nullptr);
  layoutManager->setViewport(viewport);

  QWidget * viewport2 = new QWidget;
  viewport2->setWindowTitle("New widget");
  layoutManager->setViewport(viewport2);
  viewport2->show();


  layoutManager->setLayout(vtkDMMLLayoutNode::CjyxLayoutConventionalView);
  if (!testLayoutManagerViewWidgetForSlice(__LINE__, layoutManager, "Green"))
    {
    return EXIT_FAILURE;
    }
  if (!testLayoutManagerViewWidgetForSlice(__LINE__, layoutManager, "Red"))
    {
    return EXIT_FAILURE;
    }
  if (!testLayoutManagerViewWidgetForSlice(__LINE__, layoutManager, "Yellow"))
    {
    return EXIT_FAILURE;
    }
  if (!testLayoutManagerViewWidgetForThreeD(__LINE__, layoutManager, 0))
    {
    return EXIT_FAILURE;
    }

  layoutManager->setLayout(vtkDMMLLayoutNode::CjyxLayoutFourUpTableView);
  if (!testLayoutManagerViewWidgetForSlice(__LINE__, layoutManager, "Green"))
    {
    return EXIT_FAILURE;
    }
  if (!testLayoutManagerViewWidgetForSlice(__LINE__, layoutManager, "Red"))
    {
    return EXIT_FAILURE;
    }
  if (!testLayoutManagerViewWidgetForSlice(__LINE__, layoutManager, "Yellow"))
    {
    return EXIT_FAILURE;
    }
  if (!testLayoutManagerViewWidgetForTable(__LINE__, layoutManager, 0))
    {
    return EXIT_FAILURE;
    }

  int res = 0;
  if (argc < 2 || QString(argv[1]) != "-I")
    {
    res = safeApplicationQuit(&app);
    }
  else
    {
    res = app.exec();
    }

  delete layoutManager;
  delete viewport;
  delete viewport2;
  return res;
}

