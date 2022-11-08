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
#include "qDMMLLayoutManagerTestHelper.cxx"

int qDMMLLayoutManagerTest4(int argc, char * argv[] )
{
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

  layoutNode->SetViewArrangement(vtkDMMLLayoutNode::CjyxLayoutOneUpRedSliceView);

  for (int i = vtkDMMLLayoutNode::CjyxLayoutInitialView;
    i < vtkDMMLLayoutNode::CjyxLayoutFinalView-1; ++i)
    {
    layoutManager.setLayout(i);
    if (!checkViewArrangement(__LINE__, &layoutManager, layoutNode.GetPointer(), i))
      {
      return EXIT_FAILURE;
      }
    scene->Clear(false);
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
