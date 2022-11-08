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

#include "vtkCjyxConfigure.h" // Cjyx_VTK_RENDERING_USE_{OpenGL|OpenGL2}_BACKEND

// Qt includes
#include <QApplication>
#include <QTimer>

// qDMML includes
#include "qDMMLVolumePropertyNodeWidget.h"

// DMML includes
#include <vtkDMMLVolumePropertyNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include "qDMMLWidget.h"

// this test only works on VTKv6 and later

#include <vtkAutoInit.h>
#if defined(Cjyx_VTK_RENDERING_USE_OpenGL2_BACKEND)
VTK_MODULE_INIT(vtkRenderingContextOpenGL2);
#else
VTK_MODULE_INIT(vtkRenderingContextOpenGL);
#endif

// STD includes

int qDMMLVolumePropertyNodeWidgetTest1(int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  vtkSmartPointer<vtkDMMLVolumePropertyNode> volumePropertyNode =
    vtkSmartPointer<vtkDMMLVolumePropertyNode>::New();

  qDMMLVolumePropertyNodeWidget widget;
  widget.setDMMLVolumePropertyNode(volumePropertyNode);

  widget.show();

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}
