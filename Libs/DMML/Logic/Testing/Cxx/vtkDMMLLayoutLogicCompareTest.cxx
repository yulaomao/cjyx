// DMMLLogic includes
#include "vtkDMMLLayoutLogic.h"

// DMML includes
#include "vtkDMMLCoreTestingMacros.h"
#include <vtkDMMLLayoutNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>

// STD includes

bool TestSetCjyxLayoutCompareGridView();
bool TestSetCjyxLayoutCompareGridViewEvents();

//----------------------------------------------------------------------------
int vtkDMMLLayoutLogicCompareTest(int , char * [] )
{
  bool res = true;
  res = TestSetCjyxLayoutCompareGridView() && res;
  res = TestSetCjyxLayoutCompareGridViewEvents() && res;
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//----------------------------------------------------------------------------
bool TestSetCjyxLayoutCompareGridView()
{
  vtkNew<vtkDMMLScene> scene;

  // Add default slice orientation presets
  vtkDMMLSliceNode::AddDefaultSliceOrientationPresets(scene.GetPointer());

  vtkNew<vtkDMMLLayoutLogic> layoutLogic;
  layoutLogic->SetDMMLScene(scene.GetPointer());

  vtkDMMLLayoutNode* layoutNode = layoutLogic->GetLayoutNode();

  layoutNode->SetViewArrangement(vtkDMMLLayoutNode::CjyxLayoutCompareGridView);
  if (layoutNode->GetViewArrangement() != vtkDMMLLayoutNode::CjyxLayoutCompareGridView)
    {
    std::cout << __LINE__ << ": SetViewArrangement failed." << std::endl;
    return false;
    }

  if (layoutLogic->GetViewNodes()->GetNumberOfItems() != 3)
    {
    std::cout << __LINE__ << ": SetViewArrangement(Grid) failed. "
              << layoutLogic->GetViewNodes()->GetNumberOfItems() << " views."
              << std::endl;
    return false;
    }

  layoutNode->SetNumberOfCompareViewRows(2);
  if (layoutLogic->GetViewNodes()->GetNumberOfItems() != 4)
    {
    std::cout << __LINE__ << ": SetNumberOfCompareViewRows(Grid) failed. "
              << layoutLogic->GetViewNodes()->GetNumberOfItems() << " views."
              << std::endl;
    return false;
    }

  layoutNode->SetNumberOfCompareViewColumns(2);
  if (layoutLogic->GetViewNodes()->GetNumberOfItems() != 6)
    {
    std::cout << __LINE__ << ": SetNumberOfCompareViewRows(Grid) failed. "
              << layoutLogic->GetViewNodes()->GetNumberOfItems() << " views."
              << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool TestSetCjyxLayoutCompareGridViewEvents()
{
  vtkNew<vtkDMMLScene> scene;

  // Add default slice orientation presets
  vtkDMMLSliceNode::AddDefaultSliceOrientationPresets(scene.GetPointer());

  vtkNew<vtkDMMLLayoutLogic> layoutLogic;
  layoutLogic->SetDMMLScene(scene.GetPointer());

  vtkDMMLLayoutNode* layoutNode = layoutLogic->GetLayoutNode();

  vtkNew<vtkDMMLCoreTestingUtilities::vtkDMMLNodeCallback> spy;
  layoutNode->AddObserver(vtkCommand::AnyEvent, spy.GetPointer());

  layoutNode->SetViewArrangement(vtkDMMLLayoutNode::CjyxLayoutCompareGridView);
  if (spy->GetTotalNumberOfEvents() != 1 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1)
    {
    std::cout << __LINE__ << ": SetViewArrangement failed. "
              << spy->GetTotalNumberOfEvents() << " events, "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " modified events"
              << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();

  // Fires 2 modified events:
  //  once when changing the compareviewrows,
  //  once when updating the layout description by the logic
  // Ideally, it should be 1 event.
  layoutNode->SetNumberOfCompareViewRows(2);
  if (spy->GetTotalNumberOfEvents() != 2 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 2)
    {
    std::cout << __LINE__ << ": SetViewArrangement failed. "
              << spy->GetTotalNumberOfEvents() << " events, "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " modified events"
              << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();


  // Fires 2 modified events:
  //  once when changing the compareviewrows,
  //  once when updating the layout description by the logic
  // Ideally, it should be 1 event.
  layoutNode->SetNumberOfCompareViewColumns(2);
  if (spy->GetTotalNumberOfEvents() != 2 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 2)
    {
    std::cout << __LINE__ << ": SetViewArrangement failed. "
              << spy->GetTotalNumberOfEvents() << " events, "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " modified events"
              << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();

  layoutNode->SetViewArrangement(vtkDMMLLayoutNode::CjyxLayoutFourUpView);
  if (spy->GetTotalNumberOfEvents() != 1 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1)
    {
    std::cout << __LINE__ << ": SetViewArrangement failed. "
              << spy->GetTotalNumberOfEvents() << " events, "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " modified events"
              << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();

  // Fires 2 modified events:
  //  once when changing the compareviewrows,
  //  once when updating the layout description by the logic
  // Ideally, it should be 1 event.
  layoutNode->SetNumberOfCompareViewRows(3);
  if (spy->GetTotalNumberOfEvents() != 2 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 2)
    {
    std::cout << __LINE__ << ": SetViewArrangement failed. "
              << spy->GetTotalNumberOfEvents() << " events, "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " modified events"
              << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();
  return true;
}
