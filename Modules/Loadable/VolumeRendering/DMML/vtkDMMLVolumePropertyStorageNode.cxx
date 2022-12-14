/*=Auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLVolumePropertyStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include "vtkDMMLVolumePropertyNode.h"
#include "vtkDMMLVolumePropertyStorageNode.h"
#include "vtkDMMLScene.h"

#include <vtkColorTransferFunction.h>
#include <vtkObjectFactory.h>
#include <vtkPiecewiseFunction.h>
#include <vtkStringArray.h>
#include <vtkVolumeProperty.h>

#include <sstream>

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLVolumePropertyStorageNode);

//----------------------------------------------------------------------------
vtkDMMLVolumePropertyStorageNode::vtkDMMLVolumePropertyStorageNode()
{
  this->DefaultWriteFileExtension = "vp";
}

//----------------------------------------------------------------------------
vtkDMMLVolumePropertyStorageNode::~vtkDMMLVolumePropertyStorageNode() = default;

//----------------------------------------------------------------------------
void vtkDMMLVolumePropertyStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
bool vtkDMMLVolumePropertyStorageNode::CanReadInReferenceNode(vtkDMMLNode *refNode)
{
  return refNode->IsA("vtkDMMLVolumePropertyNode");
}

//----------------------------------------------------------------------------
int vtkDMMLVolumePropertyStorageNode::ReadDataInternal(vtkDMMLNode *refNode)
{
  vtkDMMLVolumePropertyNode *vpNode =
    vtkDMMLVolumePropertyNode::SafeDownCast(refNode);

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("ReadData: File name not specified");
    return 0;
    }

  int result = 1;

  std::ifstream ifs;
#ifdef _WIN32
  ifs.open(fullName.c_str(), ios::binary | ios::in);
#else
  ifs.open(fullName.c_str(), ios::in);
#endif
  if ( !ifs )
    {
    vtkErrorMacro("Cannot open volume property file: " << fullName);
    return 0;
    }
  char line[1024];
  std::string sline;

  ifs.getline(line, 1024);
  sline = line;
  if (!sline.empty())
    {
    int value;
    std::stringstream ss;
    ss << sline;
    ss >> value;
    vpNode->GetVolumeProperty()->SetInterpolationType(value);
    }
  ifs.getline(line, 1024);
  sline = line;
  if (!sline.empty())
    {
    int value;
    std::stringstream ss;
    ss << sline;
    ss >> value;
    vpNode->GetVolumeProperty()->SetShade(value);
    }
  ifs.getline(line, 1024);
  sline = line;
  if (!sline.empty())
    {
    double value;
    std::stringstream ss;
    ss << sline;
    ss >> value;
    vpNode->GetVolumeProperty()->SetDiffuse(value);
    }
  ifs.getline(line, 1024);
  sline = line;
  if (!sline.empty())
    {
    double value;
    std::stringstream ss;
    ss << sline;
    ss >> value;
    vpNode->GetVolumeProperty()->SetAmbient(value);
    }
  ifs.getline(line, 1024);
  sline = line;
  if (!sline.empty())
    {
    double value;
    std::stringstream ss;
    ss << sline;
    ss >> value;
    vpNode->GetVolumeProperty()->SetSpecular(value);
    }
  ifs.getline(line, 1024);
  sline = line;
  if (!sline.empty())
    {
    double value;
    std::stringstream ss;
    ss << sline;
    ss >> value;
    vpNode->GetVolumeProperty()->SetSpecularPower(value);
    }

  ifs.getline(line, 1024);
  sline = line;
  if (!sline.empty())
    {
    vtkPiecewiseFunction *scalarOpacity=vtkPiecewiseFunction::New();
    vpNode->GetPiecewiseFunctionFromString(sline, scalarOpacity),
    vpNode->SetScalarOpacity(scalarOpacity);
    scalarOpacity->Delete();
    }

  ifs.getline(line, 1024);
  sline = line;
  if (!sline.empty())
    {
    vtkPiecewiseFunction *gradientOpacity=vtkPiecewiseFunction::New();
    vpNode->GetPiecewiseFunctionFromString(sline, gradientOpacity);
    vpNode->SetGradientOpacity(gradientOpacity);
    gradientOpacity->Delete();
    }

  ifs.getline(line, 1024);
  sline = line;
  if (!sline.empty())
    {
    vtkColorTransferFunction *colorTransfer=vtkColorTransferFunction::New();
    vpNode->GetColorTransferFunctionFromString(sline, colorTransfer);
    vpNode->SetColor(colorTransfer);
    colorTransfer->Delete();
    }
  ifs.close();

  return result;
}

//----------------------------------------------------------------------------
int vtkDMMLVolumePropertyStorageNode::WriteDataInternal(vtkDMMLNode *refNode)
{
  vtkDMMLVolumePropertyNode *vpNode = vtkDMMLVolumePropertyNode::SafeDownCast(refNode);

  std::string fullName =  this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("vtkDMMLVolumePropertyStorageNode: File name not specified");
    return 0;
    }

  std::ofstream ofs;
#ifdef _WIN32
  ofs.open(fullName.c_str(), ios::binary | ios::out);
#else
  ofs.open(fullName.c_str(), ios::out);
#endif

  if ( !ofs )
    {
    vtkErrorMacro("Cannot open volume property file: " << fullName);
    return 0;
    }
  ofs << vpNode->GetVolumeProperty()->GetInterpolationType()  << std::endl;
  ofs << vpNode->GetVolumeProperty()->GetShade()  << std::endl;
  ofs << vpNode->GetVolumeProperty()->GetDiffuse()  << std::endl;
  ofs << vpNode->GetVolumeProperty()->GetAmbient()  << std::endl;
  ofs << vpNode->GetVolumeProperty()->GetSpecular()  << std::endl;
  ofs << vpNode->GetVolumeProperty()->GetSpecularPower()  << std::endl;
  ofs << vpNode->GetPiecewiseFunctionString(vpNode->GetVolumeProperty()->GetScalarOpacity())  << std::endl;
  ofs << vpNode->GetPiecewiseFunctionString(vpNode->GetVolumeProperty()->GetGradientOpacity())<< std::endl;
  ofs << vpNode->GetColorTransferFunctionString(vpNode->GetVolumeProperty()->GetRGBTransferFunction())<< std::endl;

  ofs.close();

  int result =1;
  return result;
}

//----------------------------------------------------------------------------
void vtkDMMLVolumePropertyStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("VolumeProperty (.vp)");
}

//----------------------------------------------------------------------------
void vtkDMMLVolumePropertyStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("VolumeProperty (.vp)");
  this->SupportedWriteFileTypes->InsertNextValue("Text (.txt)");
  this->SupportedWriteFileTypes->InsertNextValue("VolumeProperty (.*)");
}
