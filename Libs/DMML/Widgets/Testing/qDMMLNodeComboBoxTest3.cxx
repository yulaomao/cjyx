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
#include "qDMMLNodeComboBox.h"

// DMML includes
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkNew.h>
#include "qDMMLWidget.h"


// Common test driver includes
#include "qDMMLWidgetCxxTests.h"

int qDMMLNodeComboBoxTest3( int argc, char * argv [] )
{
  if (argc < 2)
    {
    std::cerr<< "Wrong number of arguments." << std::endl;
    return EXIT_FAILURE;
    }

  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  qDMMLNodeComboBox nodeSelector;
  nodeSelector.show();
  nodeSelector.setNodeTypes(QStringList("vtkDMMLViewNode"));
  vtkNew<vtkDMMLScene> scene;

  nodeSelector.setDMMLScene(scene.GetPointer());
  scene->SetURL(argv[1]);

  // The scene may contain markups, which are not supported by Cjyx core
  // (it is implemented in Markups module), but it does not affect the test.
  // There is also problem with parsing DiffusionTensorDisplayProperties
  // (type=-1 is not expected), which is probably not relevant anymore,
  // as it seems to be an obsolete node type.
  // So just suppress the errors that are logged during scene loading.
  TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_BEGIN();
  scene->Connect();
  TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_END();

  if (argc < 3 || QString(argv[2]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}
