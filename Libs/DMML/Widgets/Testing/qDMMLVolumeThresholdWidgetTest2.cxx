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
#include <QVBoxLayout>

// Cjyx includes
#include "vtkCjyxConfigure.h"

// qDMML includes
#include "qDMMLSliceWidget.h"
#include "qDMMLVolumeThresholdWidget.h"

// DMML includes
#include <vtkDMMLApplicationLogic.h>
#include <vtkDMMLMessageCollection.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLSliceViewDisplayableManagerFactory.h>
#include <vtkDMMLVolumeNode.h>

// DMMLLogic includes
#include <vtkDMMLColorLogic.h>

// VTK includes
#include <vtkNew.h>
#include "qDMMLWidget.h"

// STD includes

int qDMMLVolumeThresholdWidgetTest2(int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  if( argc < 2 )
    {
    std::cerr << "Error: missing arguments" << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << "  inputURL_scene.dmml " << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkDMMLScene> scene;
  vtkNew<vtkDMMLApplicationLogic> applicationLogic;
  applicationLogic->SetDMMLScene(scene.GetPointer());
  vtkDMMLSliceViewDisplayableManagerFactory::GetInstance()->SetDMMLApplicationLogic(applicationLogic);

  // Add default color nodes
  vtkNew<vtkDMMLColorLogic> colorLogic;
  colorLogic->SetDMMLScene(scene.GetPointer());

  scene->SetURL(argv[1]);
  vtkNew<vtkDMMLMessageCollection> userMessages;
  scene->Connect(userMessages);
  if (scene->GetNumberOfNodes() == 0)
    {
    std::cerr << "Can't load scene:" << argv[1] << " error: " << userMessages->GetAllMessagesAsString() << std::endl;
    return EXIT_FAILURE;
    }
  vtkDMMLNode* node = scene->GetFirstNodeByClass("vtkDMMLScalarVolumeNode");
  vtkDMMLVolumeNode* volumeNode = vtkDMMLVolumeNode::SafeDownCast(node);
  if (!volumeNode)
    {
    std::cerr << "Scene must contain a valid vtkDMMLVolumeNode:" << node << std::endl;
    return EXIT_FAILURE;
    }
  vtkDMMLSliceNode* redSliceNode = nullptr;
  std::vector<vtkDMMLNode*> sliceNodes;
  scene->GetNodesByClass("vtkDMMLSliceNode", sliceNodes);
  for (unsigned int i = 0; i < sliceNodes.size(); ++i)
    {
    vtkDMMLSliceNode* sliceNode = vtkDMMLSliceNode::SafeDownCast(sliceNodes[i]);
    if (!strcmp(sliceNode->GetLayoutName(), "Red") )
      {
      redSliceNode = sliceNode;
      break;
      }
    }
  if (!redSliceNode)
    {
    std::cerr << "Scene must contain a valid vtkDMMLSliceNode:" << redSliceNode << std::endl;
    return EXIT_FAILURE;
    }

  QWidget topLevel;
  qDMMLVolumeThresholdWidget volumeThreshold;
  qDMMLSliceWidget sliceWidget;
  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(&volumeThreshold);
  layout->addWidget(&sliceWidget);
  topLevel.setLayout(layout);

  volumeThreshold.setDMMLVolumeNode(volumeNode);
  sliceWidget.setDMMLScene(scene.GetPointer());
  sliceWidget.setDMMLSliceNode(redSliceNode);
  topLevel.show();

  if (argc < 3 || QString(argv[2]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}

