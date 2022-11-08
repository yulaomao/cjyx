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

// qDMML includes
#include "qDMMLVolumeInfoWidget.h"

// DMML includes
#include <vtkDMMLColorTableNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLScalarVolumeDisplayNode.h>
#include <vtkDMMLScalarVolumeNode.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkVersion.h>
#include "qDMMLWidget.h"

int qDMMLVolumeInfoWidgetTest1(int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  vtkNew< vtkDMMLScalarVolumeNode > volumeNode;

  vtkNew< vtkImageData > imageData;
  imageData->SetDimensions(256, 256, 1);
  imageData->AllocateScalars(VTK_UNSIGNED_SHORT, 1); // allocate storage for image data
  volumeNode->SetAndObserveImageData(imageData.GetPointer());
  volumeNode->SetSpacing(2., 2., 512.);
  volumeNode->SetOrigin(0, 0, 0);

  vtkNew<vtkDMMLScalarVolumeDisplayNode> displayNode;
  vtkNew<vtkDMMLScene> scene;
  scene->AddNode(volumeNode.GetPointer());
  scene->AddNode(displayNode.GetPointer());

  vtkNew<vtkDMMLColorTableNode> colorNode;
  colorNode->SetTypeToGrey();
  scene->AddNode(colorNode.GetPointer());
  displayNode->SetAndObserveColorNodeID(colorNode->GetID());

  volumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());

  qDMMLVolumeInfoWidget volumeInfo;
  volumeInfo.setVolumeNode(volumeNode.GetPointer());
  volumeInfo.show();

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}

