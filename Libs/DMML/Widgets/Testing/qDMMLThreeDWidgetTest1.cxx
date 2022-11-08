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
#include "qDMMLThreeDWidget.h"
#include "qDMMLThreeDViewControllerWidget.h"

// DMML includes
#include <vtkDMMLInteractionNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSelectionNode.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkNew.h>
#include "qDMMLWidget.h"

// STD includes
#include <cstdlib>
#include <iostream>

int qDMMLThreeDWidgetTest1(int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();
  qDMMLThreeDWidget widget;
  widget.show();

  vtkNew<vtkDMMLScene> scene;

  // vtkDMMLAbstractDisplayableManager requires selection and interaction nodes
  vtkNew<vtkDMMLSelectionNode> selectionNode;
  scene->AddNode(selectionNode.GetPointer());
  vtkNew<vtkDMMLInteractionNode> interactionNode;
  scene->AddNode(interactionNode.GetPointer());

  vtkNew<vtkDMMLViewNode> viewNode;
  scene->AddNode(viewNode.GetPointer());

  widget.setViewLabel("View1");
  widget.setDMMLScene(scene.GetPointer());
  widget.setDMMLViewNode(viewNode.GetPointer());

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}

