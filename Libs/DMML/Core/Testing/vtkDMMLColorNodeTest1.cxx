/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLColorNode.h"
#include "vtkDMMLColorTableStorageNode.h"
#include "vtkDMMLCoreTestingMacros.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

/// \brief Concrete implementation of vtkDMMLColorNode
class vtkDMMLColorNodeTestHelper1 : public vtkDMMLColorNode
{
public:
  // Provide a concrete New.
  static vtkDMMLColorNodeTestHelper1 *New();

  vtkTypeMacro(vtkDMMLColorNodeTestHelper1,vtkDMMLColorNode);

  vtkDMMLNode* CreateNodeInstance() override
    {
    return vtkDMMLColorNodeTestHelper1::New();
    }

  const char * GetTypeAsString() override
    {
    return "vtkDMMLColorNodeTestHelper1";
    }

  virtual int ReadFile()
    {
    std::cout << "vtkDMMLColorNodeTestHelper1 pretending to read a file " << std::endl;
    return EXIT_SUCCESS;
    }
  int GetNumberOfColors() override {return 1;}
  bool GetColor(int vtkNotUsed(ind), double color[4]) override
    {
    color[0] = 10;
    color[1] = 100;
    color[2] = 200;
    return true;
    }

  vtkDMMLStorageNode* CreateDefaultStorageNode() override
    {
    // just some random storage node to pass the storage node test of basic DMML node tests
    return vtkDMMLColorTableStorageNode::New();
    }
};
vtkStandardNewMacro(vtkDMMLColorNodeTestHelper1);

//---------------------------------------------------------------------------
int TestGetColorNameAsFileName();
int TestGetColorNameAsFileName(const char* colorName,
                                const char* expectedColorFileName,
                                const char* substr = "_");

//---------------------------------------------------------------------------
int vtkDMMLColorNodeTest1(int , char * [] )
{
  vtkNew<vtkDMMLColorNodeTestHelper1> node1;
  EXERCISE_ALL_BASIC_DMML_METHODS(node1.GetPointer());

  CHECK_EXIT_SUCCESS(TestGetColorNameAsFileName());

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestGetColorNameAsFileName()
{
  CHECK_EXIT_SUCCESS(TestGetColorNameAsFileName("validName", "validName"));
  CHECK_EXIT_SUCCESS(TestGetColorNameAsFileName("name with space", "name_with_space"));
  CHECK_EXIT_SUCCESS(TestGetColorNameAsFileName("n^a$m(e!w)i-t_h~v{a}l.iD#c%h'ars", "n^a$m(e!w)i-t_h~v{a}l.iD#c%h'ars"));
  CHECK_EXIT_SUCCESS(TestGetColorNameAsFileName("n`a@m&e*w+i=t[h]i;n:v\\a|l\"i<D>c,h/a?rs", "n_a_m_e_w_i_t_h_i_n_v_a_l_i_D_c_h_a_rs"));
  CHECK_EXIT_SUCCESS(TestGetColorNameAsFileName("name with \nreturn", "name_with__return"));
  CHECK_EXIT_SUCCESS(TestGetColorNameAsFileName("ÑÂme wïth àçÇénts", "____me_w__th_________nts"));
  CHECK_EXIT_SUCCESS(TestGetColorNameAsFileName(
    "very very very very very very very very very very "
    "very very very very very very very very very very "
    "very very very very very very very very very very "
    "very very very very very very very very very very "
    "very very very very very very very very very very "
    "very very very long name",
    "very_very_very_very_very_very_very_very_very_very_"
    "very_very_very_very_very_very_very_very_very_very_"
    "very_very_very_very_very_very_very_very_very_very_"
    "very_very_very_very_very_very_very_very_very_very_"
    "very_very_very_very_very_very_very_very_very_very_"
    "very_v"));
  CHECK_EXIT_SUCCESS(TestGetColorNameAsFileName("name with space", "name with space", " "));
  CHECK_EXIT_SUCCESS(TestGetColorNameAsFileName("name with space", "name__with__space", "__"));
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestGetColorNameAsFileName(const char* colorName, const char * expectedColorFileName, const char* substr)
{
  vtkNew<vtkDMMLColorNodeTestHelper1> colorNode;
  colorNode->SetNamesFromColors();
  colorNode->SetColorName(0, colorName);
  std::string fileName = colorNode->GetColorNameAsFileName(0, substr);
  if (fileName != std::string(expectedColorFileName))
    {
    std::cerr << "Input: " << colorName
              << "\nOutput: " << fileName.c_str()
              << "\n Expected: " << expectedColorFileName <<std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
