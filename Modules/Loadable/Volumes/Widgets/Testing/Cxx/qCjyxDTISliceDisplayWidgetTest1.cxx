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
#include "qCjyxDTISliceDisplayWidget.h"

// DMML includes
#include <vtkDMMLDiffusionTensorVolumeSliceDisplayNode.h>
#include <vtkDMMLDiffusionTensorDisplayPropertiesNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>
#include "qDMMLWidget.h"

//-----------------------------------------------------------------------------
int qCjyxDTISliceDisplayWidgetTest1( int argc, char * argv[] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  vtkSmartPointer<vtkDMMLScene> scene = vtkSmartPointer<vtkDMMLScene>::New();
  vtkSmartPointer<vtkDMMLDiffusionTensorDisplayPropertiesNode> propertiesNode =
    vtkSmartPointer<vtkDMMLDiffusionTensorDisplayPropertiesNode>::New();
  scene->AddNode(propertiesNode);
  vtkSmartPointer<vtkDMMLDiffusionTensorVolumeSliceDisplayNode> displayNode =
    vtkSmartPointer<vtkDMMLDiffusionTensorVolumeSliceDisplayNode>::New();
  displayNode->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(propertiesNode->GetID());
  scene->AddNode(displayNode);

  qCjyxDTISliceDisplayWidget widget;
  widget.setDMMLScene(scene);
  widget.setDMMLDTISliceDisplayNode(displayNode);

  for (int i = 0;
       i < vtkDMMLDiffusionTensorVolumeSliceDisplayNode::GetNumberOfScalarInvariants();
       ++i)
    {
    widget.setColorGlyphBy(vtkDMMLDiffusionTensorVolumeSliceDisplayNode::GetNthScalarInvariant(i));
    }

  widget.show();
  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}
