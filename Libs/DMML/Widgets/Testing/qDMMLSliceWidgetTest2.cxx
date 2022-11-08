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
#include "qDMMLSliceControllerWidget.h"
#include "qDMMLSliceView.h"
#include "qDMMLSliceWidget.h"
#include "qDMMLNodeObject.h"

// DMML includes
#include <vtkDMMLAbstractSliceViewDisplayableManager.h>
#include <vtkDMMLApplicationLogic.h>
#include <vtkDMMLColorTableNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceLogic.h>
#include <vtkDMMLSliceCompositeNode.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLScalarVolumeNode.h>
#include <vtkDMMLScalarVolumeDisplayNode.h>
#include <vtkDMMLVolumeArchetypeStorageNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>
#include "qDMMLWidget.h"

vtkDMMLScalarVolumeNode* loadVolume(const char* volume, vtkDMMLScene* scene)
{
  vtkNew<vtkDMMLScalarVolumeDisplayNode> displayNode;
  vtkNew<vtkDMMLScalarVolumeNode> scalarNode;
  vtkNew<vtkDMMLVolumeArchetypeStorageNode> storageNode;

  displayNode->SetAutoWindowLevel(false);
  displayNode->SetInterpolate(false);

  storageNode->SetFileName(volume);
  if (storageNode->SupportedFileType(volume) == 0)
    {
    return nullptr;
    }
  scalarNode->SetName("foo");
  scalarNode->SetScene(scene);
  displayNode->SetScene(scene);
  scene->AddNode(storageNode.GetPointer());
  scene->AddNode(displayNode.GetPointer());
  scalarNode->SetAndObserveStorageNodeID(storageNode->GetID());
  scalarNode->SetAndObserveDisplayNodeID(displayNode->GetID());
  scene->AddNode(scalarNode.GetPointer());
  storageNode->ReadData(scalarNode.GetPointer());

  // Default color tables are not present in the scene if there is no vtkDMMLColorLogic,
  // therefore we need to create and set the color node manually.
  vtkNew<vtkDMMLColorTableNode> colorNode;
  colorNode->SetTypeToGrey();
  scene->AddNode(colorNode.GetPointer());
  displayNode->SetAndObserveColorNodeID(colorNode->GetID());

  return scalarNode.GetPointer();
}

int qDMMLSliceWidgetTest2(int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();
  if( argc < 2 )
    {
    std::cerr << "Error: missing arguments" << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << "  input_image.nrrd " << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkDMMLScene> scene;
  vtkNew<vtkDMMLApplicationLogic> applicationLogic;
  applicationLogic->SetDMMLScene(scene.GetPointer());

  vtkDMMLScalarVolumeNode* scalarNode = loadVolume(argv[1], scene.GetPointer());
  if (scalarNode == nullptr)
    {
    std::cerr << "Not a valid volume: " << argv[1] << std::endl;
    return EXIT_FAILURE;
    }

  QSize viewSize(256, 256);
  qDMMLSliceWidget sliceWidget;
  sliceWidget.setDMMLScene(scene.GetPointer());
  sliceWidget.sliceLogic()->AddSliceNode("Red");

  sliceWidget.resize(viewSize.width(), sliceWidget.sliceController()->height() + viewSize.height() );

  vtkDMMLSliceCompositeNode* sliceCompositeNode = sliceWidget.sliceLogic()->GetSliceCompositeNode();
  sliceCompositeNode->SetBackgroundVolumeID(scalarNode->GetID());
  sliceWidget.show();

  qDMMLNodeObject nodeObject(scalarNode->GetDisplayNode(), &sliceWidget);
  nodeObject.setProcessEvents(false);
  nodeObject.setMessage("vtkDMMLDisplayNode");
  for (int i = 0; i < 30; ++i)
    {
    nodeObject.modify();
    }
  nodeObject.setProcessEvents(true);
  nodeObject.setMessage("vtkDMMLDisplayNode + render");
  for (int i = 0; i < 30; ++i)
    {
    nodeObject.modify();
    }

  // test the list of displayable managers
  QStringList expectedDisplayableManagerClassNames =
    QStringList() << "vtkDMMLVolumeGlyphSliceDisplayableManager"
                  << "vtkDMMLModelSliceDisplayableManager"
                  << "vtkDMMLCrosshairDisplayableManager"
                  << "vtkDMMLOrientationMarkerDisplayableManager"
                  << "vtkDMMLRulerDisplayableManager"
                  << "vtkDMMLScalarBarDisplayableManager";
  qDMMLSliceView *sliceView = const_cast<qDMMLSliceView*>(sliceWidget.sliceView());
  vtkNew<vtkCollection> collection;
  sliceView->getDisplayableManagers(collection.GetPointer());
  int numManagers = collection->GetNumberOfItems();
  std::cout << "Slice widget slice view has " << numManagers
            << " displayable managers." << std::endl;
  if (numManagers != expectedDisplayableManagerClassNames.size())
    {
    std::cerr << "Incorrect number of displayable managers, expected "
              << expectedDisplayableManagerClassNames.size()
              << " but got " << numManagers << std::endl;
    return EXIT_FAILURE;
    }
  for (int i = 0; i < numManagers; ++i)
    {
    vtkDMMLAbstractDisplayableManager *sliceViewDM =
      vtkDMMLAbstractDisplayableManager::SafeDownCast(collection->GetItemAsObject(i));
    if (sliceViewDM)
      {
      std::cout << "\tDisplayable manager " << i << " class name = " << sliceViewDM->GetClassName() << std::endl;
      if (!expectedDisplayableManagerClassNames.contains(sliceViewDM->GetClassName()))
        {
        std::cerr << "\t\tnot in expected list!" << std::endl;
        return EXIT_FAILURE;
        }
      }
    else
      {
      std::cerr << "\tDisplayable manager " << i << " is null." << std::endl;
      return EXIT_FAILURE;
      }
    }
  collection->RemoveAllItems();

/*
  QTimer modifyTimer;
  modifyTimer.setInterval(0);
  QObject::connect(&modifyTimer, SIGNAL(timeout()),
                   &nodeObject, SLOT(modify()));
  modifyTimer.start();
*/
  if (argc < 3 || QString(argv[2]) != "-I" )
    {
    QTimer::singleShot(1000, &app, SLOT(quit()));
    }

  return app.exec();
}
