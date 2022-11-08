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

// Cjyx includes
#include "vtkCjyxConfigure.h"

// qDMML includes
#include "qDMMLThreeDView.h"

// DMML includes
#include "vtkDMMLAbstractThreeDViewDisplayableManager.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>
#include "qDMMLWidget.h"

// STD includes

int qDMMLThreeDViewTest1(int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();
  qDMMLThreeDView view;
  view.show();

  // test the list of displayable managers
  QStringList expectedDisplayableManagerClassNames =
    QStringList() << "vtkDMMLCameraDisplayableManager"
                  << "vtkDMMLCrosshairDisplayableManager3D"
                  << "vtkDMMLViewDisplayableManager"
                  << "vtkDMMLModelDisplayableManager"
                  << "vtkDMMLThreeDReformatDisplayableManager"
                  << "vtkDMMLOrientationMarkerDisplayableManager"
                  << "vtkDMMLRulerDisplayableManager";
  vtkNew<vtkCollection> collection;
  view.getDisplayableManagers(collection.GetPointer());
  int numManagers = collection->GetNumberOfItems();
  std::cout << "3D view has " << numManagers
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
    vtkDMMLAbstractDisplayableManager *threeDViewDM =
      vtkDMMLAbstractDisplayableManager::SafeDownCast(collection->GetItemAsObject(i));
    if (threeDViewDM)
      {
      std::cout << "\tDisplayable manager " << i << " class name = " << threeDViewDM->GetClassName() << std::endl;
      if (!expectedDisplayableManagerClassNames.contains(threeDViewDM->GetClassName()))
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

  return EXIT_SUCCESS;
}

