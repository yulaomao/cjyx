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

int vtkDMMLLayoutLogicTest2(int , char * [] )
{
  vtkNew<vtkDMMLLayoutLogic> logic;
  EXERCISE_BASIC_OBJECT_METHODS(logic.GetPointer());

  vtkNew<vtkDMMLScene> scene;

  // populate scene with nodes
  vtkNew<vtkDMMLViewNode> viewNode;
  viewNode->SetLayoutName("1");
  scene->AddNode(viewNode.GetPointer());

  vtkNew<vtkDMMLSliceNode> redSliceNode;
  redSliceNode->SetLayoutName("Red");
  scene->AddNode(redSliceNode.GetPointer());

  vtkNew<vtkDMMLSliceNode> yellowSliceNode;
  yellowSliceNode->SetLayoutName("Yellow");
  scene->AddNode(yellowSliceNode.GetPointer());

  vtkNew<vtkDMMLSliceNode> greenSliceNode;
  greenSliceNode->SetLayoutName("Green");
  scene->AddNode(greenSliceNode.GetPointer());

  // populate scene with layout
  vtkNew<vtkDMMLLayoutNode> layout;
  scene->AddNode(layout.GetPointer());

  logic->SetDMMLScene(scene.GetPointer());
  logic->SetDMMLScene(nullptr);
  logic->SetDMMLScene(scene.GetPointer());

  // ConventionalView
  layout->SetViewArrangement(vtkDMMLLayoutNode::CjyxLayoutConventionalView);
  if (logic->GetViewNodes()->GetNumberOfItems() != 4)
    {
    std::cerr << __LINE__ << " Wrong number of views returned:"
              << logic->GetViewNodes()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
    }
  if (logic->GetViewNodes()->GetItemAsObject(0) != viewNode.GetPointer() ||
      logic->GetViewNodes()->GetItemAsObject(1) != redSliceNode.GetPointer() ||
      logic->GetViewNodes()->GetItemAsObject(2) != greenSliceNode.GetPointer() ||
      logic->GetViewNodes()->GetItemAsObject(3) != yellowSliceNode.GetPointer())
    {
    std::cerr << __LINE__ << " Wrong nodes returned: "
              << vtkDMMLNode::SafeDownCast(logic->GetViewNodes()->GetItemAsObject(0))->GetID() << " "
              << vtkDMMLNode::SafeDownCast(logic->GetViewNodes()->GetItemAsObject(1))->GetID() << " "
              << vtkDMMLNode::SafeDownCast(logic->GetViewNodes()->GetItemAsObject(2))->GetID() << " "
              << vtkDMMLNode::SafeDownCast(logic->GetViewNodes()->GetItemAsObject(3))->GetID() << " "
              << std::endl << "  instead of :"
              << viewNode->GetID() << " " << redSliceNode->GetID() << " "
              << greenSliceNode->GetID() << " " << yellowSliceNode->GetID() << " "
              << "Pointers: "
              << logic->GetViewNodes()->GetItemAsObject(0) << " "
              << logic->GetViewNodes()->GetItemAsObject(1) << " "
              << logic->GetViewNodes()->GetItemAsObject(2) << " "
              << logic->GetViewNodes()->GetItemAsObject(3) << " "
              << std::endl << "  instead of :"
              << viewNode.GetPointer() << " " << redSliceNode.GetPointer() << " "
              << yellowSliceNode.GetPointer() << " " << greenSliceNode.GetPointer() << " "
              << std::endl;
    return EXIT_FAILURE;
    }

  // 3D layout
  layout->SetViewArrangement(vtkDMMLLayoutNode::CjyxLayoutOneUp3DView);
  if (logic->GetViewNodes()->GetNumberOfItems() != 1)
    {
    std::cerr << __LINE__ << " Wrong number of views returned:"
              << logic->GetViewNodes()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
    }
  if (logic->GetViewNodes()->GetItemAsObject(0) != viewNode.GetPointer())
    {
    std::cerr << __LINE__ << " Wrong node returned:"
              << logic->GetViewNodes()->GetItemAsObject(0) << std::endl;
    return EXIT_FAILURE;
    }

  // RED layout
  layout->SetViewArrangement(vtkDMMLLayoutNode::CjyxLayoutOneUpRedSliceView);
  if (logic->GetViewNodes()->GetNumberOfItems() != 1)
    {
    std::cerr << __LINE__ << " Wrong number of views returned:"
              << logic->GetViewNodes()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
    }
  if (logic->GetViewNodes()->GetItemAsObject(0) != redSliceNode.GetPointer())
    {
    std::cerr << __LINE__ << " Wrong node returned:"
              << logic->GetViewNodes()->GetItemAsObject(0) << std::endl;
    return EXIT_FAILURE;
    }
  // YELLOW layout
  layout->SetViewArrangement(vtkDMMLLayoutNode::CjyxLayoutOneUpYellowSliceView);
  if (logic->GetViewNodes()->GetNumberOfItems() != 1)
    {
    std::cerr << __LINE__ <<  " Wrong number of views returned:"
              << logic->GetViewNodes()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
    }
  if (logic->GetViewNodes()->GetItemAsObject(0) != yellowSliceNode.GetPointer())
    {
    std::cerr << __LINE__ << " Wrong node returned:"
              << logic->GetViewNodes()->GetItemAsObject(0) << std::endl;
    return EXIT_FAILURE;
    }
  // GREEN layout
  layout->SetViewArrangement(vtkDMMLLayoutNode::CjyxLayoutOneUpGreenSliceView);
  if (logic->GetViewNodes()->GetNumberOfItems() != 1)
    {
    std::cerr << __LINE__ << " Wrong number of views returned:"
              << logic->GetViewNodes()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
    }
  if (logic->GetViewNodes()->GetItemAsObject(0) != greenSliceNode.GetPointer())
    {
    std::cerr << __LINE__ << " Wrong node returned:"
              << logic->GetViewNodes()->GetItemAsObject(0) << std::endl;
    return EXIT_FAILURE;
    }
  logic->Print(std::cout);

  return EXIT_SUCCESS;
}

