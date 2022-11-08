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

// Qt includes
#include <QApplication>
#include <QTimer>

// Cjyx includes
#include "vtkCjyxConfigure.h"

// qDMML includes
#include "qDMMLNavigationView.h"
#include "qDMMLThreeDView.h"

// DMML includes
#include <vtkDMMLInteractionNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSelectionNode.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkNew.h>
#include "qDMMLWidget.h"

// STD includes

int qDMMLNavigationViewTest1(int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  qDMMLNavigationView navigationView;
  navigationView.setWindowTitle("Navigation view");

  qDMMLThreeDView threeDView;
  threeDView.setWindowTitle("ThreeDView");
  navigationView.setRendererToListen(threeDView.renderer());

  vtkNew<vtkDMMLScene> scene;
  navigationView.setDMMLScene(scene.GetPointer());
  threeDView.setDMMLScene(scene.GetPointer());

  // vtkDMMLAbstractDisplayableManager requires selection and interaction nodes
  vtkNew<vtkDMMLSelectionNode> selectionNode;
  scene->AddNode(selectionNode.GetPointer());
  vtkNew<vtkDMMLInteractionNode> interactionNode;
  scene->AddNode(interactionNode.GetPointer());

  vtkNew<vtkDMMLViewNode> viewNode;
  viewNode->SetBoxVisible(true);
  scene->AddNode(viewNode.GetPointer());

  threeDView.setDMMLViewNode(viewNode.GetPointer());
  navigationView.setDMMLViewNode(viewNode.GetPointer());

  navigationView.show();
  threeDView.show();

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }


  return app.exec();
}
