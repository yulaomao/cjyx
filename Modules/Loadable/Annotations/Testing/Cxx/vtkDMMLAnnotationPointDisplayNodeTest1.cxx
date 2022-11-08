#include "vtkDMMLAnnotationPointDisplayNode.h"


#include "vtkDMMLCoreTestingMacros.h"

int vtkDMMLAnnotationPointDisplayNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLAnnotationPointDisplayNode> node1;

  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());

  TEST_SET_GET_INT_RANGE(node1, GlyphType, -1, 10);

  // min glyph type
  if (node1->GetMinimumGlyphType() != vtkDMMLAnnotationPointDisplayNode::GlyphMin)
    {
    std::cerr << "Error: GetMinimumGlyphType " << node1->GetMinimumGlyphType() << " != vtkDMMLAnnotationPointDisplayNode::GlyphMin " << vtkDMMLAnnotationPointDisplayNode::GlyphMin << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "GlyphMin = " << node1->GetMinimumGlyphType() << ", as string = " << node1->GetGlyphTypeAsString(node1->GetMinimumGlyphType()) << std::endl;
    }

  // max glyph type
  if (node1->GetMaximumGlyphType() != vtkDMMLAnnotationPointDisplayNode::GlyphMax)
    {
    std::cerr << "Error: GetMaximumGlyphType " << node1->GetMaximumGlyphType() << " != vtkDMMLAnnotationPointDisplayNode::GlyphMax " << vtkDMMLAnnotationPointDisplayNode::GlyphMax << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "GlyphMax = " << node1->GetMaximumGlyphType() << ", as string = " << node1->GetGlyphTypeAsString(node1->GetMaximumGlyphType()) << std::endl;
    }
  for (int i = vtkDMMLAnnotationPointDisplayNode::GlyphMin;
       i < vtkDMMLAnnotationPointDisplayNode::GlyphMax;
       i++)
    {
    node1->SetGlyphType(i);
    std::cout << i << " GetGlyphType = " << node1->GetGlyphType() << ", as string = " << node1->GetGlyphTypeAsString() << ", GetGlyphTypeAsString(" << i << ") = " << node1->GetGlyphTypeAsString(i) << std::endl;
    }

  // print out the enums
  std::cout << "Enum GlyphShapes:" << std::endl;
  std::cout << "    GlyphMin = " << vtkDMMLAnnotationPointDisplayNode::GlyphMin << ", as string = " << node1->GetGlyphTypeAsString(vtkDMMLAnnotationPointDisplayNode::GlyphMin) << std::endl;
  std::cout << "    Vertex2D = " << vtkDMMLAnnotationPointDisplayNode::Vertex2D << ", as string = " << node1->GetGlyphTypeAsString(vtkDMMLAnnotationPointDisplayNode::Vertex2D) << std::endl;
  std::cout << "    Dash2D = " << vtkDMMLAnnotationPointDisplayNode::Dash2D << ", as string = " << node1->GetGlyphTypeAsString(vtkDMMLAnnotationPointDisplayNode::Dash2D) << std::endl;
  std::cout << "    Cross2D = " << vtkDMMLAnnotationPointDisplayNode::Cross2D << ", as string = " << node1->GetGlyphTypeAsString(vtkDMMLAnnotationPointDisplayNode::Cross2D) << std::endl;
  std::cout << "    ThickCross2D = " << vtkDMMLAnnotationPointDisplayNode::ThickCross2D << ", as string = " << node1->GetGlyphTypeAsString(vtkDMMLAnnotationPointDisplayNode::ThickCross2D) << std::endl;
  std::cout << "    Triangle2D = " << vtkDMMLAnnotationPointDisplayNode::Triangle2D << ", as string = " << node1->GetGlyphTypeAsString(vtkDMMLAnnotationPointDisplayNode::Triangle2D) << std::endl;
  std::cout << "    Square2D = " << vtkDMMLAnnotationPointDisplayNode::Square2D << ", as string = " << node1->GetGlyphTypeAsString(vtkDMMLAnnotationPointDisplayNode::Square2D) << std::endl;
  std::cout << "    Circle2D = " << vtkDMMLAnnotationPointDisplayNode::Circle2D << ", as string = " << node1->GetGlyphTypeAsString(vtkDMMLAnnotationPointDisplayNode::Circle2D) << std::endl;
  std::cout << "    Diamond2D = " << vtkDMMLAnnotationPointDisplayNode::Diamond2D << ", as string = " << node1->GetGlyphTypeAsString(vtkDMMLAnnotationPointDisplayNode::Diamond2D) << std::endl;
  std::cout << "    Arrow2D = " << vtkDMMLAnnotationPointDisplayNode::Arrow2D << ", as string = " << node1->GetGlyphTypeAsString(vtkDMMLAnnotationPointDisplayNode::Arrow2D) << std::endl;
  std::cout << "    ThickArrow2D = " << vtkDMMLAnnotationPointDisplayNode::ThickArrow2D << ", as string = " << node1->GetGlyphTypeAsString(vtkDMMLAnnotationPointDisplayNode::ThickArrow2D) << std::endl;
  std::cout << "    HookedArrow2D = " << vtkDMMLAnnotationPointDisplayNode::HookedArrow2D << ", as string = " << node1->GetGlyphTypeAsString(vtkDMMLAnnotationPointDisplayNode::HookedArrow2D) << std::endl;
  std::cout << "    StarBurst2D = " << vtkDMMLAnnotationPointDisplayNode::StarBurst2D << ", as string = " << node1->GetGlyphTypeAsString(vtkDMMLAnnotationPointDisplayNode::StarBurst2D) << std::endl;
  std::cout << "    Sphere3D = " << vtkDMMLAnnotationPointDisplayNode::Sphere3D << ", as string = " << node1->GetGlyphTypeAsString(vtkDMMLAnnotationPointDisplayNode::Sphere3D) << std::endl;
  std::cout << "    Diamond3D = " << vtkDMMLAnnotationPointDisplayNode::Diamond3D << ", as string = " << node1->GetGlyphTypeAsString(vtkDMMLAnnotationPointDisplayNode::Diamond3D) << std::endl;
  std::cout << "    GlyphMax = " << vtkDMMLAnnotationPointDisplayNode::GlyphMax << ", as string = " << node1->GetGlyphTypeAsString(vtkDMMLAnnotationPointDisplayNode::GlyphMax) << std::endl;

  // semantic assumptions
  if (node1->GetMinimumGlyphType() != vtkDMMLAnnotationPointDisplayNode::Vertex2D)
    {
    std::cerr << "Error: minimum glyph type " << node1->GetMinimumGlyphType() << " != Vertex2d: " << vtkDMMLAnnotationPointDisplayNode::Vertex2D << std::endl;
    return EXIT_FAILURE;
    }
  if (vtkDMMLAnnotationPointDisplayNode::Vertex2D != 1)
    {
    std::cerr << "ERROR: Vertex2D (" << vtkDMMLAnnotationPointDisplayNode::Vertex2D << ") is not defined as 1! Setting glyph types on the source class won't work!" << std::endl;
    return EXIT_FAILURE;
    }

  if (node1->GetMaximumGlyphType() != vtkDMMLAnnotationPointDisplayNode::Sphere3D)
    {
    std::cerr << "Error: maximum glyph type " << node1->GetMaximumGlyphType() << " != Sphere3D: " << vtkDMMLAnnotationPointDisplayNode::Sphere3D << std::endl;
    return EXIT_FAILURE;
    }

  // spot test int to string mapping
  node1->SetGlyphType(vtkDMMLAnnotationPointDisplayNode::Sphere3D);
  if (strcmp(node1->GetGlyphTypeAsString(), "Sphere3D") != 0)
    {
    std::cerr << "ERROR: set the glyph type to " << vtkDMMLAnnotationPointDisplayNode::Sphere3D << ", but get glyph type as string returned " << node1->GetGlyphTypeAsString() << " instead of Sphere3D" << std::endl;
    return EXIT_FAILURE;
    }

  // test GlyphTypeIs3D
  node1->SetGlyphTypeFromString("Triangle2D");
  if (node1->GlyphTypeIs3D() == 1)
    {
    std::cerr << "ERROR: triangle 2d not recognised as a 2d glyph" << std::endl;
    return EXIT_FAILURE;
    }
  node1->SetGlyphTypeFromString("Sphere3D");
  if (node1->GlyphTypeIs3D() != 1)
    {
    std::cerr << "ERROR: sphere 3d not recognised as a 3d glyph" << std::endl;
    return EXIT_FAILURE;
    }

  TEST_SET_GET_DOUBLE_RANGE(node1, GlyphScale, -1.0, 25.6);

  return EXIT_SUCCESS;
}
