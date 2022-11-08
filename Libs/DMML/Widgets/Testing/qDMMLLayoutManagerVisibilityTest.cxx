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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Common test driver includes
#include "qDMMLWidgetCxxTests.h"
#include "qDMMLLayoutManagerTestHelper.cxx"

// Qt includes
#include <QApplication>
#include <QHash>
#include <QWidget>

// Cjyx includes
#include "qDMMLLayoutManager.h"
#include "vtkCjyxConfigure.h"

// DMML includes
#include <vtkDMMLAbstractViewNode.h>
#include <vtkDMMLApplicationLogic.h>
#include <vtkDMMLLayoutLogic.h>
#include <vtkDMMLLayoutNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLSliceViewDisplayableManagerFactory.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkNew.h>
#include "qDMMLWidget.h"

// --------------------------------------------------------------------------
bool checkNodeVisibility(int line,
                         vtkDMMLAbstractViewNode* viewNode,
                         bool expectedNodeVisibility)
{
  bool nodeVisibility = (viewNode->GetVisibility() != 0);
  if (nodeVisibility != expectedNodeVisibility)
    {
    std::cerr << "Line " << line << " - Problem with GetVisibility()"
              << " associated with node " << viewNode->GetID() << "\n"
              << "  visibility: " << nodeVisibility << "\n"
              << "  expectedVisibility: " << expectedNodeVisibility << std::endl;
    return false;
    }
  return true;
}

// --------------------------------------------------------------------------
bool checkNodeMappedInLayout(int line,
                             vtkDMMLAbstractViewNode* viewNode,
                             bool expectedNodeMappedInLayout)
{
  bool nodeMappedInLayout = (viewNode->IsMappedInLayout() != 0);
  if (nodeMappedInLayout != expectedNodeMappedInLayout)
    {
    std::cerr << "Line " << line << " - Problem with GetVisibility()"
              << " associated with node " << viewNode->GetID() << "\n"
              << "  mappedInLayout: " << nodeMappedInLayout << "\n"
              << "  expectedMappedInLayout: " << expectedNodeMappedInLayout << std::endl;
    return false;
    }
  return true;
}


// --------------------------------------------------------------------------
bool checkNodeVisibleInLayout(int line,
                             vtkDMMLAbstractViewNode* viewNode,
                             bool expectedNodeVisibleInLayout)
{
  bool nodeVisibleInLayout = (viewNode->IsViewVisibleInLayout() != 0);
  if (nodeVisibleInLayout != expectedNodeVisibleInLayout)
    {
    std::cerr << "Line " << line << " - Problem with GetVisibility()"
              << " associated with node " << viewNode->GetID() << "\n"
              << "  visibleInLayout: " << nodeVisibleInLayout << "\n"
              << "  expectedVisibleInLayout: " << expectedNodeVisibleInLayout << std::endl;
    return false;
    }
  return true;
}

// --------------------------------------------------------------------------
bool checkViews(int line,
                qDMMLLayoutManager* layoutManager,
                QHash<vtkDMMLAbstractViewNode*, QList<bool> > viewNodesToExpectedVisibility)
{
  foreach(vtkDMMLAbstractViewNode* viewNode, viewNodesToExpectedVisibility.keys())
    {

    bool expectedNodeVisibility = viewNodesToExpectedVisibility[viewNode].at(0);
    bool expectedNodeMappedInLayout = viewNodesToExpectedVisibility[viewNode].at(1);
    bool expectedNodeVisibleInLayout = viewNodesToExpectedVisibility[viewNode].at(2);

    if (!viewNode)
      {
      std::cerr << "Line " << line << " - Problem with to GetNodeByID()."
               << " 'viewNode' should NOT be null." << std::endl;
      return false;
      }

    if (!checkNodeVisibility(line, viewNode, expectedNodeVisibility))
      {
      return false;
      }

    if (!checkNodeMappedInLayout(line, viewNode, expectedNodeMappedInLayout))
      {
      return false;
      }

    if (!checkNodeVisibleInLayout(line, viewNode, expectedNodeVisibleInLayout))
      {
      return false;
      }

    QWidget* viewWidget = layoutManager->viewWidget(viewNode);
    if (!viewWidget)
      {
      std::cerr << "Line " << line << " - Failed to retrieve view widget"
               << "associated with view node " << viewNode->GetID() << std::endl;
      return false;
      }

    bool widgetVisibility = viewWidget->isVisible();
    bool expectedWidgetVisibility = expectedNodeVisibleInLayout;
    if (widgetVisibility != expectedWidgetVisibility)
      {
      std::cerr << "Line " << line << " - Problem with widget visibility"
                << " associated with node " << viewNode->GetID() << "\n"
                << "  widgetVisibility: " << widgetVisibility << "\n"
                << "  expectedWidgetVisibility: " << expectedWidgetVisibility << std::endl;
      return false;
      }
    }
  return true;
}

// --------------------------------------------------------------------------
enum
{
  NoVisibility = 0,
  Visibility = 1
};

enum
{
  NotMappedInLayout = 0,
  MappedInLayout = 1
};

enum
{
  NotVisibleInLayout = 0,
  VisibleInLayout = 1
};

// --------------------------------------------------------------------------
bool runTests(vtkDMMLScene* scene,
              vtkDMMLLayoutNode* layoutNode,
              qDMMLLayoutManager* layoutManager)
{
  layoutNode->SetViewArrangement(vtkDMMLLayoutNode::CjyxLayoutConventionalView);
  qApp->processEvents();

  vtkDMMLAbstractViewNode* redNode = vtkDMMLAbstractViewNode::SafeDownCast(
        scene->GetNodeByID("vtkDMMLSliceNodeRed"));
  vtkDMMLAbstractViewNode* yellowNode = vtkDMMLAbstractViewNode::SafeDownCast(
        scene->GetNodeByID("vtkDMMLSliceNodeYellow"));
  vtkDMMLAbstractViewNode* greenNode = vtkDMMLAbstractViewNode::SafeDownCast(
        scene->GetNodeByID("vtkDMMLSliceNodeGreen"));
  vtkDMMLAbstractViewNode* threeDNode = vtkDMMLAbstractViewNode::SafeDownCast(
        scene->GetNodeByID("vtkDMMLViewNode1"));


  // All view widgets are expected to be visible
  {
    QHash<vtkDMMLAbstractViewNode*, QList<bool> > viewNodesToExpectedVisibility;
    viewNodesToExpectedVisibility[redNode] =    QList<bool>()  << Visibility   << MappedInLayout    << VisibleInLayout;
    viewNodesToExpectedVisibility[yellowNode] = QList<bool>()  << Visibility   << MappedInLayout    << VisibleInLayout;
    viewNodesToExpectedVisibility[greenNode] =  QList<bool>()  << Visibility   << MappedInLayout    << VisibleInLayout;
    viewNodesToExpectedVisibility[threeDNode] = QList<bool>()  << Visibility   << MappedInLayout    << VisibleInLayout;

    if (!checkViews(__LINE__, layoutManager, viewNodesToExpectedVisibility))
      {
      return false;
      }
  }

  yellowNode->SetVisibility(0);
  qApp->processEvents();

  // Yellow widget is expected to be hidden
  {
    QHash<vtkDMMLAbstractViewNode*, QList<bool> > viewNodesToExpectedVisibility;
    viewNodesToExpectedVisibility[redNode] =    QList<bool>()  << Visibility   << MappedInLayout    << VisibleInLayout;
    viewNodesToExpectedVisibility[yellowNode] = QList<bool>()  << NoVisibility << MappedInLayout    << NotVisibleInLayout;
    viewNodesToExpectedVisibility[greenNode] =  QList<bool>()  << Visibility   << MappedInLayout    << VisibleInLayout;
    viewNodesToExpectedVisibility[threeDNode] = QList<bool>()  << Visibility   << MappedInLayout    << VisibleInLayout;

    if (!checkViews(__LINE__, layoutManager, viewNodesToExpectedVisibility))
      {
      return false;
      }
  }

  threeDNode->SetVisibility(0);
  qApp->processEvents();

  // Yellow and ThreeD widgets are expected to be hidden
  {
    QHash<vtkDMMLAbstractViewNode*, QList<bool> > viewNodesToExpectedVisibility;
    viewNodesToExpectedVisibility[redNode] =    QList<bool>()  << Visibility   << MappedInLayout    << VisibleInLayout;
    viewNodesToExpectedVisibility[yellowNode] = QList<bool>()  << NoVisibility << MappedInLayout    << NotVisibleInLayout;
    viewNodesToExpectedVisibility[greenNode] =  QList<bool>()  << Visibility   << MappedInLayout    << VisibleInLayout;
    viewNodesToExpectedVisibility[threeDNode] = QList<bool>()  << NoVisibility << MappedInLayout    << NotVisibleInLayout;

    if (!checkViews(__LINE__, layoutManager, viewNodesToExpectedVisibility))
      {
      return false;
      }
  }

  layoutNode->SetViewArrangement(vtkDMMLLayoutNode::CjyxLayoutFourUpPlotView);
  qApp->processEvents();

  vtkDMMLAbstractViewNode* plotNode =
      vtkDMMLAbstractViewNode::SafeDownCast(scene->GetNodeByID("vtkDMMLPlotViewNodePlotView1"));

  // Only yellow widgets is expected to be hidden
  {
    QHash<vtkDMMLAbstractViewNode*, QList<bool> > viewNodesToExpectedVisibility;
    viewNodesToExpectedVisibility[redNode] =    QList<bool>()  << Visibility   << MappedInLayout    << VisibleInLayout;
    viewNodesToExpectedVisibility[yellowNode] = QList<bool>()  << NoVisibility << MappedInLayout    << NotVisibleInLayout;
    viewNodesToExpectedVisibility[greenNode] =  QList<bool>()  << Visibility   << MappedInLayout    << VisibleInLayout;
    viewNodesToExpectedVisibility[plotNode] =  QList<bool>()  << Visibility   << MappedInLayout    << VisibleInLayout;

    if (!checkViews(__LINE__, layoutManager, viewNodesToExpectedVisibility))
      {
      return false;
      }
  }

  layoutNode->SetViewArrangement(vtkDMMLLayoutNode::CjyxLayoutFourUpTableView);
  qApp->processEvents();

  vtkDMMLAbstractViewNode* tableNode =
      vtkDMMLAbstractViewNode::SafeDownCast(scene->GetNodeByID("vtkDMMLTableViewNodeTableView1"));

  // Only yellow widgets is expected to be hidden
  {
    QHash<vtkDMMLAbstractViewNode*, QList<bool> > viewNodesToExpectedVisibility;
    viewNodesToExpectedVisibility[redNode] =    QList<bool>()  << Visibility   << MappedInLayout    << VisibleInLayout;
    viewNodesToExpectedVisibility[yellowNode] = QList<bool>()  << NoVisibility << MappedInLayout    << NotVisibleInLayout;
    viewNodesToExpectedVisibility[greenNode] =  QList<bool>()  << Visibility   << MappedInLayout    << VisibleInLayout;
    viewNodesToExpectedVisibility[tableNode] =  QList<bool>()  << Visibility   << MappedInLayout    << VisibleInLayout;

    if (!checkViews(__LINE__, layoutManager, viewNodesToExpectedVisibility))
      {
      return false;
      }
  }

  layoutNode->SetViewArrangement(vtkDMMLLayoutNode::CjyxLayoutConventionalView);
  qApp->processEvents();

  // Yellow and ThreeD widgets are expected to be hidden
  {
    QHash<vtkDMMLAbstractViewNode*, QList<bool> > viewNodesToExpectedVisibility;
    viewNodesToExpectedVisibility[redNode] =    QList<bool>()  << Visibility   << MappedInLayout    << VisibleInLayout;
    viewNodesToExpectedVisibility[yellowNode] = QList<bool>()  << NoVisibility << MappedInLayout    << NotVisibleInLayout;
    viewNodesToExpectedVisibility[greenNode] =  QList<bool>()  << Visibility   << MappedInLayout    << VisibleInLayout;
    viewNodesToExpectedVisibility[threeDNode] = QList<bool>()  << NoVisibility << MappedInLayout    << NotVisibleInLayout;
    viewNodesToExpectedVisibility[plotNode] =  QList<bool>()  << Visibility   << NotMappedInLayout << NotVisibleInLayout;
    viewNodesToExpectedVisibility[tableNode] =  QList<bool>()  << Visibility   << NotMappedInLayout << NotVisibleInLayout;

    if (!checkViews(__LINE__, layoutManager, viewNodesToExpectedVisibility))
      {
      return false;
      }
  }

  scene->Clear(/*removeSingletons = */ 0);
  qApp->processEvents();

  // All view widgets are expected to be visible
  {
    QHash<vtkDMMLAbstractViewNode*, QList<bool> > viewNodesToExpectedVisibility;
    viewNodesToExpectedVisibility[redNode] =    QList<bool>()  << Visibility   << MappedInLayout    << VisibleInLayout;
    viewNodesToExpectedVisibility[yellowNode] = QList<bool>()  << Visibility   << MappedInLayout    << VisibleInLayout;
    viewNodesToExpectedVisibility[greenNode] =  QList<bool>()  << Visibility   << MappedInLayout    << VisibleInLayout;
    viewNodesToExpectedVisibility[threeDNode] = QList<bool>()  << Visibility   << MappedInLayout    << VisibleInLayout;
    viewNodesToExpectedVisibility[plotNode] =  QList<bool>()  << Visibility   << NotMappedInLayout << NotVisibleInLayout;
    viewNodesToExpectedVisibility[tableNode] =  QList<bool>()  << Visibility   << NotMappedInLayout << NotVisibleInLayout;

    if (!checkViews(__LINE__, layoutManager, viewNodesToExpectedVisibility))
      {
      return false;
      }
  }

  {
    // The following test will check that the view node visibility attribute
    // is ignored when loading a scene.
    // For more detailed, see comment in vtkDMMLAbstractViewNode::ReadXMLAttributes

    yellowNode->SetVisibility(0);
    qApp->processEvents();

    if (!checkNodeVisibility(__LINE__,
                             yellowNode,
                             /* expectedNodeVisibility = */ false))
      {
      return false;
      }

    scene->SetSaveToXMLString(1);
    scene->Commit();

    // Serialized scene has yellow node with visibility set to 0
    std::string xmlScene = scene->GetSceneXMLString();

    // Clear current scene
    scene->Clear(/*removeSingletons = */ 0);
    qApp->processEvents();

    if (!checkNodeVisibility(__LINE__,
                             yellowNode,
                             /* expectedNodeVisibility = */ true))
      {
      return false;
      }

    scene->SetLoadFromXMLString(1);
    scene->SetSceneXMLString(xmlScene);
    scene->Import();
    qApp->processEvents();

    if (!checkNodeVisibility(__LINE__,
                             yellowNode,
                             /* expectedNodeVisibility = */ true))
      {
      return false;
      }
  }

  return true;
}

// --------------------------------------------------------------------------
int qDMMLLayoutManagerVisibilityTest(int argc, char * argv[] )
{
  (void)checkViewArrangement; // Fix -Wunused-function warning

  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  QWidget w;
  w.show();

  qDMMLLayoutManager layoutManager(&w, &w);

  vtkNew<vtkDMMLApplicationLogic> applicationLogic;
  vtkDMMLSliceViewDisplayableManagerFactory::GetInstance()->SetDMMLApplicationLogic(applicationLogic);

  vtkNew<vtkDMMLScene> scene;
  vtkNew<vtkDMMLLayoutNode> layoutNode;

  scene->AddNode(layoutNode.GetPointer());
  applicationLogic->SetDMMLScene(scene.GetPointer());
  layoutManager.setDMMLScene(scene.GetPointer());

  if (!runTests(scene.GetPointer(), layoutNode.GetPointer(), &layoutManager))
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
