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
#include <QHBoxLayout>
#include <QTimer>

// Cjyx includes
#include "vtkCjyxConfigure.h"

// qDMML includes
#include "qDMMLColorListView.h"

// DMML includes
#include <vtkDMMLColorTableNode.h>
#include <vtkDMMLPETProceduralColorNode.h>

// VTK includes
#include <vtkNew.h>
#include "qDMMLWidget.h"

// STD includes

int qDMMLColorListViewTest1(int argc, char * argv [])
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  QWidget topLevel;
  qDMMLColorListView ColorListView;
  qDMMLColorListView ColorListView1;
  qDMMLColorListView ColorListView2;

  QHBoxLayout* hboxLayout = new QHBoxLayout;
  hboxLayout->addWidget(&ColorListView);
  hboxLayout->addWidget(&ColorListView1);
  hboxLayout->addWidget(&ColorListView2);
  topLevel.setLayout(hboxLayout);

  vtkNew<vtkDMMLColorTableNode> colorTableNode;
  colorTableNode->SetType(vtkDMMLColorTableNode::Labels);

  ColorListView.setDMMLColorNode(colorTableNode.GetPointer());
  if (ColorListView.dmmlColorNode() != colorTableNode.GetPointer())
    {
    std::cerr << "qDMMLColorListView::setDMMLColorNode() failed" << std::endl;
    return EXIT_FAILURE;
    }
  // for some reasons it generate a warning if the type is changed.
  colorTableNode->NamesInitialisedOff();
  colorTableNode->SetTypeToCool1();

  vtkNew<vtkDMMLPETProceduralColorNode> colorPETNode;
  colorPETNode->SetTypeToRainbow();
  ColorListView2.setDMMLColorNode(colorPETNode.GetPointer());
  if (ColorListView2.dmmlColorNode() != colorPETNode.GetPointer())
    {
    std::cerr << "qDMMLColorListView::setDMMLColorNode() failed" << std::endl;
    return EXIT_FAILURE;
    }
  colorPETNode->SetTypeToMIP();

  topLevel.show();

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}

