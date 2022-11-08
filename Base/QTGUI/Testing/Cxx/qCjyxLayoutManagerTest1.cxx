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

// Cjyx includes
#include "vtkCjyxConfigure.h"

// Cjyx includes
#include "qCjyxLayoutManager.h"

// DMML includes

// VTK includes
#include "qDMMLWidget.h"

// STD includes
#include <cstdlib>

int qCjyxLayoutManagerTest1(int argc, char * argv[] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();
  qCjyxLayoutManager* layoutManager = new qCjyxLayoutManager(nullptr);
  delete layoutManager;
  return EXIT_SUCCESS;
}

