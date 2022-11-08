/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLProceduralColorStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.6 $

=========================================================================auto=*/

// DMML include
#include "vtkDMMLProceduralColorStorageNode.h"
#include "vtkDMMLProceduralColorNode.h"
#include "vtkDMMLScene.h"

// VTK include
#include <vtkColorTransferFunction.h>
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>

// STD include
#include <sstream>

//------------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLProceduralColorStorageNode);

//----------------------------------------------------------------------------
vtkDMMLProceduralColorStorageNode::vtkDMMLProceduralColorStorageNode()
{
  this->DefaultWriteFileExtension = "txt";
}

//----------------------------------------------------------------------------
vtkDMMLProceduralColorStorageNode::~vtkDMMLProceduralColorStorageNode() = default;

//----------------------------------------------------------------------------
void vtkDMMLProceduralColorStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDMMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
bool vtkDMMLProceduralColorStorageNode::CanReadInReferenceNode(vtkDMMLNode* refNode)
{
  // FreeSurfer color nodes are special cases and are treated like
  // color table nodes
  return (refNode->IsA("vtkDMMLProceduralColorNode") &&
          !refNode->IsA("vtkDMMLFreeSurferProceduralColorNode"));
}

//----------------------------------------------------------------------------
int vtkDMMLProceduralColorStorageNode::ReadDataInternal(vtkDMMLNode *refNode)
{
  std::string fullName = this->GetFullNameFromFileName();

  // cast the input node
  vtkDMMLProceduralColorNode *colorNode =
    vtkDMMLProceduralColorNode::SafeDownCast(refNode);

  if (colorNode == nullptr)
    {
    vtkErrorMacro("ReadData: unable to cast input node " << refNode->GetID()
                  << " to a known procedural color node");
    return 0;
    }

  vtkColorTransferFunction *ctf = colorNode->GetColorTransferFunction();
  if (!ctf)
    {
    vtkErrorMacro("ReadDataInternal: no color transfer function!");
    return 0;
    }

  std::string extension = vtkDMMLStorageNode::GetLowercaseExtensionFromFileName(fullName);
  if (extension == std::string(".txt"))
    {
    // open the file for reading input
    fstream fstr;

    fstr.open(fullName.c_str(), fstream::in);

    if (!fstr.is_open())
      {
      vtkErrorMacro("ERROR opening procedural color file " << this->FileName << endl);
      return 0;
      }

    // clear out the node
    int wasModifying = colorNode->StartModify();
    colorNode->SetTypeToFile();
    colorNode->NamesInitialisedOff();
    ctf->RemoveAllPoints();

    char line[1024];

    while (fstr.good())
      {
      fstr.getline(line, 1024);

      // does it start with a #?
      if (line[0] == '#')
        {
        vtkDebugMacro("Comment line, skipping:\n\"" << line << "\"");
        }
      else
        {
        // is it empty?
        if (line[0] == '\0')
          {
          vtkDebugMacro("Empty line, skipping:\n\"" << line << "\"");
          }
        else
          {
          vtkDebugMacro("got a line: \n\"" << line << "\"");
          std::stringstream ss;
          ss << line;
          double x = 0.0, r = 0.0, g = 0.0, b = 0.0;
          // TBD: check that it's not a color table file!
          ss >> x;
          ss >> r;
          ss >> g;
          ss >> b;
          ctf->AddRGBPoint(x, r, g, b);
          }
        }
      }
    fstr.close();
    colorNode->EndModify(wasModifying);
    }
  else
    {
    vtkErrorMacro("ReadDataInternal: other extensions than .txt not supported yet! Can't read " << extension);
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkDMMLProceduralColorStorageNode::WriteDataInternal(vtkDMMLNode *refNode)
{
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("vtkDMMLProceduralColorStorageNode: File name not specified");
    return 0;
    }

  // cast the input node
  vtkDMMLProceduralColorNode *colorNode = nullptr;
  if ( refNode->IsA("vtkDMMLProceduralColorNode") )
    {
    colorNode = dynamic_cast <vtkDMMLProceduralColorNode *> (refNode);
    }

  if (colorNode == nullptr)
    {
    vtkErrorMacro("WriteData: unable to cast input node " << refNode->GetID() << " to a known color table node");
    return 0;
    }

  vtkColorTransferFunction *ctf = colorNode->GetColorTransferFunction();
  if (!ctf)
    {
    vtkErrorMacro("WriteDataInternal: no color transfer function!");
    return 0;
    }

  std::string extension = vtkDMMLStorageNode::GetLowercaseExtensionFromFileName(fullName);
  if (extension == std::string(".txt"))
    {
    // open the file for writing
    fstream of;

    of.open(fullName.c_str(), fstream::out);

    if (!of.is_open())
      {
      vtkErrorMacro("WriteDataInternal: unable to open file " << fullName.c_str() << " for writing");
      return 0;
      }

    // put down a header
    of << "# Color procedural file " << (this->GetFileName() != nullptr ? this->GetFileName() : "null") << endl;
    int numPoints = ctf->GetSize();
    of << "# " << numPoints << " points" << endl;
    of << "# position R G B" << endl;
    for (int i = 0; i < numPoints; ++i)
      {
      double val[6];
      ctf->GetNodeValue(i, val);
      // val holds location, r, g, b, midpoint, sharpness
      of << val[0];
      of << " ";
      of << val[1];
      of << " ";
      of << val[2];
      of << " ";
      of << val[3];
      of << endl;
      }
    of.close();
    }
  else
    {
    vtkErrorMacro("WriteDataInternal: only .txt supported");
    return 0;
    }


  return 1;
}

//----------------------------------------------------------------------------
void vtkDMMLProceduralColorStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("Color Function (.cxml)");
  this->SupportedReadFileTypes->InsertNextValue("Text (.txt)");
}

//----------------------------------------------------------------------------
void vtkDMMLProceduralColorStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Color Function (.cxml)");
  this->SupportedWriteFileTypes->InsertNextValue("Text (.txt)");
}
