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

// qDMML includes
#include "qCjyxCoreApplication.h"
#include "qCjyxSimpleMarkupsWidget.h"

// DMML includes
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkNew.h>

int qDMMLSimpleMarkupsWidgetTest1( int argc, char * argv [] )
{
  qCjyxCoreApplication app(argc, argv);

  qCjyxSimpleMarkupsWidget markupsWidget;
  markupsWidget.show();
  vtkNew<vtkDMMLScene> scene;

  markupsWidget.setDMMLScene(scene.GetPointer());

  if (argc < 3 || QString(argv[2]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}
