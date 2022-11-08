// DMMLLogic includes
#include "vtkDMMLLayoutLogic.h"

// DMML includes
#include <vtkDMMLLayoutNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkCollection.h>

#include "vtkDMMLCoreTestingMacros.h"

int vtkDMMLLayoutLogicTest1(int , char * [] )
{
  vtkNew<vtkDMMLScene> scene;

  // Add default slice orientation presets
  vtkDMMLSliceNode::AddDefaultSliceOrientationPresets(scene.GetPointer());

  vtkNew<vtkDMMLLayoutLogic> layoutLogic;
  layoutLogic->SetDMMLScene(scene.GetPointer());
  vtkDMMLLayoutNode* layoutNode = layoutLogic->GetLayoutNode();
  if (!layoutNode)
    {
    std::cerr << __LINE__ << " vtkDMMLLayoutNode::SetDMMLScene failed"
              << ", no layout node:" << layoutNode << std::endl;
    return EXIT_FAILURE;
    }
  layoutNode->SetViewArrangement(
    vtkDMMLLayoutNode::CjyxLayoutConventionalView);
  vtkCollection* views = layoutLogic->GetViewNodes();
  if (views->GetNumberOfItems() != 4)
    {
    std::cerr << __LINE__ << " Wrong number of views returned:"
              << layoutLogic->GetViewNodes()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
    }
  vtkDMMLViewNode* viewNode = vtkDMMLViewNode::SafeDownCast(
    views->GetItemAsObject(0));
  vtkDMMLSliceNode* redNode = vtkDMMLSliceNode::SafeDownCast(
    views->GetItemAsObject(1));
  vtkDMMLSliceNode* yellowNode = vtkDMMLSliceNode::SafeDownCast(
    views->GetItemAsObject(2));
  vtkDMMLSliceNode* greenNode = vtkDMMLSliceNode::SafeDownCast(
    views->GetItemAsObject(3));

  if (!viewNode || !redNode || !yellowNode || !greenNode)
    {
    std::cerr << __LINE__ << " Wrong nodes returned:"
              << viewNode << " " << redNode << " "
              << yellowNode << " " << greenNode << std::endl;
    return EXIT_FAILURE;
    }

  layoutLogic->Print(std::cout);

  return EXIT_SUCCESS;
}

