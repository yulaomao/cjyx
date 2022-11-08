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
#include <QApplication>

#include <qDMMLTreeView.h>
#include <qDMMLSceneTransformModel.h>

// Cjyx includes
#include "vtkCjyxConfigure.h"

// VTK includes
#include <vtkDMMLApplicationLogic.h>
#include <vtkDMMLScene.h>
#include "qDMMLWidget.h"

#include <vtkTimerLog.h>

// STD includes

int qDMMLTreeViewTest1( int argc, char * argv [] )
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
  std::cout << std::endl<< "***************************************" << std::endl;
  vtkTimerLog* timer = vtkTimerLog::New();
  vtkDMMLScene* scene = vtkDMMLScene::New();
  vtkDMMLApplicationLogic* applicationLogic = vtkDMMLApplicationLogic::New();
  applicationLogic->SetDMMLScene(scene);
  scene->SetURL(argv[1]);
  timer->StartTimer();
  scene->Import();
  timer->StopTimer();
  std::cout << std::endl << "Loaded: " << timer->GetElapsedTime() << std::endl;
  timer->StartTimer();
  applicationLogic->Delete();
  scene->Delete();
  timer->StopTimer();
  std::cout << std::endl << "Deleted: " << timer->GetElapsedTime() << std::endl;

  std::cout << std::endl<< "***************************************" << std::endl;
  scene = vtkDMMLScene::New();
  applicationLogic = vtkDMMLApplicationLogic::New();
  applicationLogic->SetDMMLScene(scene);
  qDMMLSceneModel   sceneModel;
  sceneModel.setDMMLScene(scene);
  scene->SetURL(argv[1]);
  timer->StartTimer();
  scene->Import();
  timer->StopTimer();
  std::cout << "qDMMLSceneModel Loaded: " << timer->GetElapsedTime() << std::endl;
  timer->StartTimer();
  applicationLogic->Delete();
  scene->Delete();
  timer->StopTimer();
  std::cout << "qDMMLSceneModel Deleted: " << timer->GetElapsedTime() << std::endl;

  std::cout << std::endl<< "***************************************" << std::endl;
  scene = vtkDMMLScene::New();
  applicationLogic = vtkDMMLApplicationLogic::New();
  applicationLogic->SetDMMLScene(scene);
  qDMMLSceneTransformModel   transformModel;
  transformModel.setDMMLScene(scene);
  scene->SetURL(argv[1]);
   timer->StartTimer();
  scene->Import();
  timer->StopTimer();
  std::cout << "qDMMLSceneTransformModel Loaded: " << timer->GetElapsedTime() << std::endl;
  timer->StartTimer();
  applicationLogic->Delete();
  scene->Delete();
  timer->StopTimer();
  std::cout << "qDMMLSceneTransformModel Deleted: " << timer->GetElapsedTime() << std::endl;

  std::cout << std::endl<< "***************************************" << std::endl;
  scene = vtkDMMLScene::New();
  applicationLogic = vtkDMMLApplicationLogic::New();
  applicationLogic->SetDMMLScene(scene);
  qDMMLSceneTransformModel   transformModel2;
  transformModel2.setDMMLScene(scene);
  qDMMLSortFilterProxyModel  sortModel;
  sortModel.setSourceModel(&transformModel2);
  scene->SetURL(argv[1]);
   timer->StartTimer();
  scene->Import();
  timer->StopTimer();
  std::cout << "qDMMLSceneTransformModel(+qDMMLSortFilterProxyModel) Loaded: " << timer->GetElapsedTime() << std::endl;
  timer->StartTimer();
  applicationLogic->Delete();
  scene->Delete();
  timer->StopTimer();
  std::cout << "qDMMLSceneTransformModel(+qDMMLSortFilterProxyModel) Deleted: " << timer->GetElapsedTime() << std::endl;


  std::cout << std::endl<< "***************************************" << std::endl;
  scene = vtkDMMLScene::New();
  applicationLogic = vtkDMMLApplicationLogic::New();
  applicationLogic->SetDMMLScene(scene);
  qDMMLTreeView   dmmlItem;
  dmmlItem.setDMMLScene(scene);
  scene->SetURL(argv[1]);
   timer->StartTimer();
  scene->Import();
  timer->StopTimer();
  std::cout << "qDMMLTreeView Loaded: " << timer->GetElapsedTime() << std::endl;
  timer->StartTimer();
  applicationLogic->Delete();
  scene->Delete();
  timer->StopTimer();
  std::cout << "qDMMLTreeView Deleted: " << timer->GetElapsedTime() << std::endl;

  std::cout << std::endl<< "***************************************" << std::endl;
  scene = vtkDMMLScene::New();
  applicationLogic = vtkDMMLApplicationLogic::New();
  applicationLogic->SetDMMLScene(scene);
  qDMMLTreeView   treeWidget;
  treeWidget.show();
  treeWidget.setDMMLScene(scene);
  scene->SetURL(argv[1]);
  timer->StartTimer();
  scene->Import();
  timer->StopTimer();
  std::cout << "qDMMLTreeView visible Loaded: " << timer->GetElapsedTime() << std::endl;
  timer->StartTimer();
  applicationLogic->Delete();
  scene->Delete();
  timer->StopTimer();
  std::cout << "qDMMLTreeView visible Deleted: " << timer->GetElapsedTime() << std::endl;

  timer->Delete();
  return EXIT_SUCCESS;
}
