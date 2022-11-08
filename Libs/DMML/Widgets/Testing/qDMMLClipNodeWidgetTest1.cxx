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
#include "qDMMLClipNodeWidget.h"

// DMML includes
#include <vtkDMMLClipModelsNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include "qDMMLWidget.h"

// STD includes

int qDMMLClipNodeWidgetTest1(int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  vtkSmartPointer< vtkDMMLClipModelsNode > clipNode =
    vtkSmartPointer< vtkDMMLClipModelsNode >::New();

  qDMMLClipNodeWidget clipNodeWidget;

  if (clipNodeWidget.isEnabled())
    {
    std::cerr << "No vtkMRLMClipModelsNode provided, should be disabled."
              << std::endl;
    return EXIT_FAILURE;
    }

  int clipType = clipNode->GetClipType();
  int redSliceClipState = clipNode->GetRedSliceClipState();
  int yellowSliceClipState = clipNode->GetYellowSliceClipState();
  int greenSliceClipState = clipNode->GetGreenSliceClipState();

  clipNodeWidget.setDMMLClipNode(clipNode);

  if (clipNodeWidget.dmmlClipNode() != clipNode.GetPointer())
    {
    std::cerr << "qDMMLClipNodeWidget::setDMMLClipNode() failed."
              << clipNodeWidget.dmmlClipNode() << std::endl;
    return EXIT_FAILURE;
    }

  if (clipNodeWidget.clipType() != clipType)
    {
    std::cerr << "Wrong clipType: " << clipNodeWidget.clipType() << std::endl;
    return EXIT_FAILURE;
    }

  if (clipNodeWidget.redSliceClipState() != redSliceClipState)
    {
    std::cerr << "Wrong red slice clip state: " << clipNodeWidget.redSliceClipState() << std::endl;
    return EXIT_FAILURE;
    }

  if (clipNodeWidget.yellowSliceClipState() != yellowSliceClipState)
    {
    std::cerr << "Wrong yellow slice clip state: " << clipNodeWidget.yellowSliceClipState() << std::endl;
    return EXIT_FAILURE;
    }

  if (clipNodeWidget.greenSliceClipState() != greenSliceClipState)
    {
    std::cerr << "Wrong green slice clip state: " << clipNodeWidget.greenSliceClipState() << std::endl;
    return EXIT_FAILURE;
    }

  clipNode->SetClipType(vtkDMMLClipModelsNode::ClipIntersection);

  if (clipNodeWidget.clipType() != vtkDMMLClipModelsNode::ClipIntersection)
    {
    std::cerr << "vtkDMMLClipModelsNode::SetClipType() failed: " << clipNodeWidget.clipType() << std::endl;
    return EXIT_FAILURE;
    }

  clipNodeWidget.setClipType(vtkDMMLClipModelsNode::ClipUnion);

  if (clipNode->GetClipType() != vtkDMMLClipModelsNode::ClipUnion)
    {
    std::cerr << "qDMMLClipNodeWidget::setClipType() failed: "
              << clipNode->GetClipType() << std::endl;
    return EXIT_FAILURE;
    }

  // Red slice Clip state
  clipNode->SetRedSliceClipState(vtkDMMLClipModelsNode::ClipNegativeSpace);

  if (clipNodeWidget.redSliceClipState() != vtkDMMLClipModelsNode::ClipNegativeSpace)
    {
    std::cerr << "vtkDMMLClipModelsNode::SetRedSliceClipState() failed: " << clipNodeWidget.redSliceClipState() << std::endl;
    return EXIT_FAILURE;
    }

  clipNodeWidget.setRedSliceClipState(vtkDMMLClipModelsNode::ClipOff);

  if (clipNode->GetRedSliceClipState() != vtkDMMLClipModelsNode::ClipOff)
    {
    std::cerr << "qDMMLClipNodeWidget::setRedSliceClipState() failed: "
              << clipNode->GetRedSliceClipState() << std::endl;
    return EXIT_FAILURE;
    }

  // Yellow slice Clip state
  clipNode->SetYellowSliceClipState(vtkDMMLClipModelsNode::ClipOff);

  if (clipNodeWidget.yellowSliceClipState() != vtkDMMLClipModelsNode::ClipOff)
    {
    std::cerr << "vtkDMMLClipModelsNode::SetYellowSliceClipState() failed: " << clipNodeWidget.yellowSliceClipState() << std::endl;
    return EXIT_FAILURE;
    }

  clipNodeWidget.setYellowSliceClipState(vtkDMMLClipModelsNode::ClipPositiveSpace);

  if (clipNode->GetYellowSliceClipState() != vtkDMMLClipModelsNode::ClipPositiveSpace)
    {
    std::cerr << "qDMMLClipNodeWidget::setYellowSliceClipState() failed: "
              << clipNode->GetYellowSliceClipState() << std::endl;
    return EXIT_FAILURE;
    }

  // Green slice Clip state
  clipNode->SetGreenSliceClipState(vtkDMMLClipModelsNode::ClipPositiveSpace);

  if (clipNodeWidget.greenSliceClipState() != vtkDMMLClipModelsNode::ClipPositiveSpace)
    {
    std::cerr << "vtkDMMLClipModelsNode::SetGreenSliceClipState() failed: " << clipNodeWidget.greenSliceClipState() << std::endl;
    return EXIT_FAILURE;
    }

  clipNodeWidget.setGreenSliceClipState(vtkDMMLClipModelsNode::ClipNegativeSpace);

  if (clipNode->GetGreenSliceClipState() != vtkDMMLClipModelsNode::ClipNegativeSpace)
    {
    std::cerr << "qDMMLClipNodeWidget::setGreenSliceClipState() failed: "
              << clipNode->GetGreenSliceClipState() << std::endl;
    return EXIT_FAILURE;
    }

  clipNodeWidget.show();

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}

