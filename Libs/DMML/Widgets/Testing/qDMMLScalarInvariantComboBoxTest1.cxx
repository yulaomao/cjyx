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
#include "qDMMLScalarInvariantComboBox.h"

// DMML includes
#include <vtkDMMLDiffusionTensorDisplayPropertiesNode.h>

// VTK includes
#include <vtkNew.h>
#include "qDMMLWidget.h"

// STD includes

int qDMMLScalarInvariantComboBoxTest1(int argc, char * argv [] )
{
  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  vtkNew<vtkDMMLDiffusionTensorDisplayPropertiesNode> displayPropertiesNode;

  qDMMLScalarInvariantComboBox scalarComboBox;
  scalarComboBox.setDisplayPropertiesNode(displayPropertiesNode.GetPointer());

  displayPropertiesNode->SetColorGlyphBy(
    vtkDMMLDiffusionTensorDisplayPropertiesNode::ParallelDiffusivity);
  if (scalarComboBox.scalarInvariant() != vtkDMMLDiffusionTensorDisplayPropertiesNode::ParallelDiffusivity)
    {
    std::cerr << "qDMMLScalarInvariantComboBox failed: "
              << scalarComboBox.scalarInvariant() << " instead of "
              << displayPropertiesNode->GetColorGlyphBy() << std::endl;
    return EXIT_FAILURE;
    }

  scalarComboBox.setScalarInvariant(
    vtkDMMLDiffusionTensorDisplayPropertiesNode::FractionalAnisotropy);
  if (displayPropertiesNode->GetColorGlyphBy() !=
      vtkDMMLDiffusionTensorDisplayPropertiesNode::FractionalAnisotropy)
    {
    std::cerr << "qDMMLScalarInvariantComboBox::setScalarInvariant() failed: "
              << displayPropertiesNode->GetColorGlyphBy() << " instead of "
              << scalarComboBox.scalarInvariant() << std::endl;
    return EXIT_FAILURE;
    }

  scalarComboBox.show();

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}

