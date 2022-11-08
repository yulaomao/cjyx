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
#include <QTimer>

// Cjyx includes
#include "vtkCjyxConfigure.h"

// Volumes includes
#include "qCjyxDiffusionTensorVolumeDisplayWidget.h"

// DMML includes
#include <vtkDMMLDiffusionTensorVolumeDisplayNode.h>
#include <vtkDMMLDiffusionTensorVolumeNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>
#include "qDMMLWidget.h"

//-----------------------------------------------------------------------------
int qCjyxDiffusionTensorVolumeDisplayWidgetTest1( int argc, char * argv[] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  vtkSmartPointer<vtkDMMLScene> scene = vtkSmartPointer<vtkDMMLScene>::New();
  vtkSmartPointer<vtkDMMLDiffusionTensorVolumeDisplayNode> displayNode =
    vtkSmartPointer<vtkDMMLDiffusionTensorVolumeDisplayNode>::New();
  scene->AddNode(displayNode);
  vtkSmartPointer<vtkDMMLDiffusionTensorVolumeNode> volumeNode =
    vtkSmartPointer<vtkDMMLDiffusionTensorVolumeNode>::New();
  volumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());
  scene->AddNode(volumeNode);

  qCjyxDiffusionTensorVolumeDisplayWidget widget;
  widget.setDMMLScene(scene);
  widget.setDMMLVolumeNode(volumeNode);

  widget.show();
  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}
