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

// CTK includes
#include <ctkTest.h>

// Models includes
#include <qCjyxAbstractModuleRepresentation.h>
#include "qCjyxModelsModule.h"
#include "vtkCjyxModelsLogic.h"

// DMML includes
#include <vtkDMMLModelDisplayNode.h>
#include <vtkDMMLModelHierarchyNode.h>
#include <vtkDMMLModelNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLViewNode.h>

// Subject hierarchy widgets
#include <qDMMLSubjectHierarchyTreeView.h>

// VTK includes
#include <vtkNew.h>
#include <vtkTestingOutputWindow.h>

// --------------------------------------------------------------------------
class qCjyxModelsModuleWidgetTester: public QObject
{
  Q_OBJECT
private:

private slots:
  void testClearCurrentNode();
};

// ----------------------------------------------------------------------------
void qCjyxModelsModuleWidgetTester::testClearCurrentNode()
{
  // Create a scene with a model under hierarchy:
  // Scene
  //   + View
  //   + Hierarchy
  //       + Hierarchy -> ModelNode
  vtkNew<vtkDMMLScene> scene;

  scene->SetLoadFromXMLString(1);
  scene->SetSceneXMLString(
"<DMML  version=\"Cjyx4.4.0\" userTags=\"\">"
" <View id=\"vtkDMMLViewNode1\"  name=\"View1\" ></View>"
" <ModelDisplay id=\"vtkDMMLModelDisplayNode4\"  name=\"ModelDisplay\"  ></ModelDisplay>"
" <Model id=\"vtkDMMLModelNode4\"  name=\"left\"  displayNodeRef=\"vtkDMMLModelDisplayNode4\"  references=\"display:vtkDMMLModelDisplayNode4;\"  ></Model>"
" <ModelDisplay id=\"vtkDMMLModelDisplayNode5\"  name=\"ModelDisplay_1\" ></ModelDisplay>"
" <ModelHierarchy id=\"vtkDMMLModelHierarchyNode1\"  name=\"Model Hierarchy\" ></ModelHierarchy>"
" <ModelHierarchy id=\"vtkDMMLModelHierarchyNode2\"  name=\"ModelHierarchy\" parentNodeRef=\"vtkDMMLModelHierarchyNode1\"  associatedNodeRef=\"vtkDMMLModelNode4\"  expanded=\"true\" ></ModelHierarchy>"
" </DMML>");
  scene->Connect();
  vtkDMMLModelNode* modelNode = vtkDMMLModelNode::SafeDownCast(scene->GetFirstNode(nullptr, "vtkDMMLModelNode"));

  // Instantiate Models module panel
  qCjyxModelsModule module;
  TESTING_OUTPUT_ASSERT_WARNINGS_BEGIN();
  module.initialize(nullptr);
  TESTING_OUTPUT_ASSERT_WARNINGS_END(); // warning due to using 0 as application logic
  module.setDMMLScene(scene.GetPointer());

  QWidget* moduleWidget = dynamic_cast<QWidget*>(module.widgetRepresentation());
  moduleWidget->show();

  // Make the model item current
  qDMMLSubjectHierarchyTreeView* view = moduleWidget->findChild<qDMMLSubjectHierarchyTreeView*>();
  view->setCurrentNode(modelNode);

  // When the scene is cleared (EndCloseEvent), only the top-level nodes are
  // removed in the scene model of the Models tree view. The model node row is
  // not explicitly removed and therefore the QItemSelectionModel::currentRowChanged
  // signal is not emitted as it happens later when the event loop is executed.
  // If the tree view were to observe that signal it would fail to be notified
  // on time and would not stop observing the model display node (by the display
  // node widget and more specifically qDMMLDisplayNodeViewComboBox).
  // This would lead to some inconsistent state (observing a node with a null
  // scene because it has been removed by the scene) and a crash.
  scene->Clear(0);
  //qApp->exec();
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qCjyxModelsModuleWidgetTest)
#include "moc_qCjyxModelsModuleWidgetTest.cxx"
