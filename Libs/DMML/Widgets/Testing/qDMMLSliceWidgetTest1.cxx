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
#include "qDMMLSliceWidget.h"
#include "qDMMLNodeObject.h"

// DMML includes
#include <vtkDMMLApplicationLogic.h>
#include <vtkDMMLColorTableNode.h>
#include <vtkDMMLDisplayNode.h>
#include <vtkDMMLMessageCollection.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLSliceViewDisplayableManagerFactory.h>
#include <vtkDMMLVolumeNode.h>

// VTK includes
#include <vtkMultiThreader.h>
#include <vtkNew.h>
#include "qDMMLWidget.h"

int qDMMLSliceWidgetTest1(int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();
  vtkMultiThreader::SetGlobalMaximumNumberOfThreads(1);
  if( argc < 2 )
    {
    std::cerr << "Error: missing arguments" << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << "  inputURL_scene.dmml " << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkDMMLScene> scene;
  vtkNew<vtkDMMLApplicationLogic> applicationLogic;
  vtkDMMLSliceViewDisplayableManagerFactory::GetInstance()->SetDMMLApplicationLogic(applicationLogic);
  applicationLogic->SetDMMLScene(scene.GetPointer());
  scene->SetURL(argv[1]);
  vtkNew<vtkDMMLMessageCollection> userMessages;
  scene->Connect(userMessages);
  if (scene->GetNumberOfNodes() == 0)
    {
    std::cerr << "Can't load scene:" << argv[1] << " error: " <<userMessages->GetAllMessagesAsString() << std::endl;
    return EXIT_FAILURE;
    }
  vtkDMMLSliceNode* redSliceNode = nullptr;
  // search for a red slice node
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
  vtkDMMLNode* node = scene->GetFirstNodeByClass("vtkDMMLScalarVolumeNode");
  vtkDMMLVolumeNode* volumeNode = vtkDMMLVolumeNode::SafeDownCast(node);
  if (!volumeNode)
    {
    std::cerr << "Scene must contain a valid vtkDMMLVolumeNode:" << node << std::endl;
    return EXIT_FAILURE;
    }
  if (!volumeNode->GetDisplayNode()->GetColorNode())
    {
    // add a custom color node (grey)
    vtkDMMLColorTableNode* colorNode = vtkDMMLColorTableNode::New();
    colorNode->SetTypeToGrey();
    scene->AddNode(colorNode);
    colorNode->Delete();
    volumeNode->GetDisplayNode()->SetAndObserveColorNodeID(colorNode->GetID());
    }

  // "Red" slice by default
  qDMMLSliceWidget sliceWidget;
  sliceWidget.setDMMLScene(scene.GetPointer());
  sliceWidget.setDMMLSliceNode(redSliceNode);
  sliceWidget.show();

  qDMMLNodeObject nodeObject(volumeNode, &sliceWidget);
  QTimer modifyTimer;
  modifyTimer.setInterval(2000);
  QObject::connect(&modifyTimer, SIGNAL(timeout()),
                   &nodeObject, SLOT(modify()));
  //modifyTimer.start();
  if (argc < 3 || QString(argv[2]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}

