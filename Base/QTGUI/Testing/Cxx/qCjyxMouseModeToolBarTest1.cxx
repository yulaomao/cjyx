// Qt includes
//#include <QApplication>
//
// Cjyx includes
#include "vtkCjyxConfigure.h"

// Cjyx includes
#include "qCjyxApplication.h"
#include "qCjyxMouseModeToolBar.h"

// DMML includes
#include <vtkDMMLCoreTestingMacros.h>
#include <vtkDMMLInteractionNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSelectionNode.h>

// Logic includes
#include <vtkCjyxApplicationLogic.h>

// VTK includes

// STD includes

#define CHECK_PLACE_ACTION_TEXT(expected, mouseToolBar) \
  { \
  QString activeActionText; \
  activeActionText = activePlaceActionText(mouseToolBar); \
  std::cout << "Line " << __LINE__ << " Active place action text = " << qPrintable(activeActionText) << std::endl; \
  if (activeActionText.compare(QString(expected)) != 0) \
    { \
    std::cerr << "Line " << __LINE__ << " Error: Expected active action text of '" << #expected << "', got '" \
      << qPrintable(activeActionText) << "'" << std::endl; \
    return EXIT_FAILURE; \
    } \
  };

QString activePlaceActionText(qCjyxMouseModeToolBar& mouseModeToolBar)
{
  QAction* placeAction = mouseModeToolBar.actions()[2];
  if (!placeAction->isEnabled())
    {
    return QString();
    }
  return placeAction->text();
  /*
  return mouseModeToolBar.actions()[2]->text();
  foreach(QAction* action, mouseModeToolBar.actions())
    {
    if (action->objectName() == QString("ToolBarAction"))
      {
      return action->text();
      break;
      }
    }
  return QString();
  */
}

QString getActiveActionText(qCjyxMouseModeToolBar& mouseModeToolBar)
  {
  foreach(QAction * action, mouseModeToolBar.actions())
    {
    std::cout << "action name: " << qPrintable(action->objectName()) << std::endl;;
    if (action->isChecked())
      {
      return action->text();
      break;
      }

    }
  return QString();
  }

int qCjyxMouseModeToolBarTest1(int argc, char * argv[] )
{
  qCjyxApplication app(argc, argv);
  qCjyxMouseModeToolBar mouseToolBar;

  // set the scene without the app logic
  vtkDMMLScene* scene = vtkDMMLScene::New();

  // Check that setting a scene without interaction node does not cause any problem
  // (it may log messages that interaction node is not found)
  mouseToolBar.setDMMLScene(scene);

  // Now reset scene in the toolbar to null and set the scene again now after app logic
  // adds interaction and selection nodes.
  mouseToolBar.setDMMLScene(nullptr);
  vtkCjyxApplicationLogic *appLogic = vtkCjyxApplicationLogic::New();
  appLogic->SetDMMLScene(scene);
  mouseToolBar.setApplicationLogic(appLogic);
  mouseToolBar.setDMMLScene(scene);

  std::cout << "Done set up, starting testing..." << std::endl;

  // exercise public slots
  mouseToolBar.switchToViewTransformMode();
  mouseToolBar.switchPlaceMode();
  mouseToolBar.setPersistence(true);
  // without a qCjyxApplication, setting the cursor is a noop
  mouseToolBar.changeCursorTo(QCursor(Qt::BusyCursor));

  CHECK_PLACE_ACTION_TEXT("Toggle Markups Toolbar", mouseToolBar);

  // get the selection and interaction nodes that the mouse mode tool bar
  // listens to
  vtkDMMLSelectionNode *selectionNode = vtkDMMLSelectionNode::SafeDownCast(
    scene->GetNodeByID("vtkDMMLSelectionNodeSingleton"));
  CHECK_NOT_NULL(selectionNode);

  // add markups/annotation
  selectionNode->AddNewPlaceNodeClassNameToList("vtkDMMLMarkupsFiducialNode", ":/Icons/MarkupsFiducialMouseModePlace.png", "Point List");
  selectionNode->AddNewPlaceNodeClassNameToList("vtkDMMLMarkupsCurveNode", ":/Icons/MarkupsCurveMouseModePlace.png", "Curve");
  selectionNode->AddNewPlaceNodeClassNameToList("vtkDMMLAnnotationROINode", ":/Icons/AnnotationROIWithArrow.png", "ROI");
  selectionNode->AddNewPlaceNodeClassNameToList("vtkDMMLAnnotationRulerNode", ":/Icons/AnnotationDistanceWithArrow.png", "Ruler");

  selectionNode->SetReferenceActivePlaceNodeClassName("vtkDMMLMarkupsFiducialNode");
  selectionNode->SetActivePlaceNodeID("vtkDMMLMarkupsFiducialNode1");
  selectionNode->SetActivePlaceNodePlacementValid(true);
  CHECK_PLACE_ACTION_TEXT("Point List", mouseToolBar);

  selectionNode->SetReferenceActivePlaceNodeClassName("vtkDMMLMarkupsCurveNode");
  selectionNode->SetActivePlaceNodeID("vtkDMMLMarkupsCurveNode1");
  selectionNode->SetActivePlaceNodePlacementValid(true);
  CHECK_PLACE_ACTION_TEXT("Curve", mouseToolBar);

  selectionNode->SetReferenceActivePlaceNodeClassName("vtkDMMLAnnotationROINode");
  selectionNode->SetActivePlaceNodeID("vtkDMMLAnnotationROINode1");
  selectionNode->SetActivePlaceNodePlacementValid(true);
  CHECK_PLACE_ACTION_TEXT("ROI", mouseToolBar);

  selectionNode->SetReferenceActivePlaceNodeClassName("vtkDMMLAnnotationRulerNode");
  selectionNode->SetActivePlaceNodeID("vtkDMMLAnnotationRulerNode1");
  selectionNode->SetActivePlaceNodePlacementValid(true);
  CHECK_PLACE_ACTION_TEXT("Ruler", mouseToolBar);

  selectionNode->SetReferenceActivePlaceNodeClassName("vtkDMMLMarkupsFiducialNode");
  selectionNode->SetActivePlaceNodeID("vtkDMMLMarkupsFiducialNode1");
  selectionNode->SetActivePlaceNodePlacementValid(true);
  CHECK_PLACE_ACTION_TEXT("Point List", mouseToolBar);

  vtkDMMLInteractionNode *interactionNode = vtkDMMLInteractionNode::SafeDownCast(
    scene->GetNodeByID("vtkDMMLInteractionNodeSingleton"));
  CHECK_NOT_NULL(interactionNode);

  interactionNode->SetPlaceModePersistence(1);
  interactionNode->SetPlaceModePersistence(0);
  interactionNode->SwitchToSinglePlaceMode();
  CHECK_PLACE_ACTION_TEXT("Point List", mouseToolBar);

  interactionNode->SwitchToViewTransformMode();
  CHECK_PLACE_ACTION_TEXT("Point List", mouseToolBar);

  // clean up
  appLogic->Delete();
  scene->Delete();

  return EXIT_SUCCESS;
}

