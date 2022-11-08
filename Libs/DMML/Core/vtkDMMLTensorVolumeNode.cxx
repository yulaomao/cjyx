/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLVolumeNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLDiffusionTensorVolumeDisplayNode.h"
#include "vtkDMMLNRRDStorageNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLTensorVolumeNode.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkObjectFactory.h>
#include <vtkMatrix4x4.h>


//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLTensorVolumeNode);

//----------------------------------------------------------------------------
vtkDMMLTensorVolumeNode::vtkDMMLTensorVolumeNode()
{
  for(int i=0; i<3; i++)
    {
    for(int j=0; j<3; j++)
      {
      this->MeasurementFrameMatrix[i][j] = (i == j) ? 1.0 : 0.0;
      }
    }
  this->Order = -1; //Tensor order
}

//----------------------------------------------------------------------------
vtkDMMLTensorVolumeNode::~vtkDMMLTensorVolumeNode() = default;

//----------------------------------------------------------------------------
void vtkDMMLTensorVolumeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  std::stringstream ss;
  for(int i=0; i<3; i++)
    {
    for(int j=0; j<3; j++)
      {
      ss << this->MeasurementFrameMatrix[i][j] << " ";
      if ( i != 2 && j != 2 )
        {
        ss << "  ";
        }
      }
    }
  of << " measurementFrame=\"" << ss.str() << "\"";

  of << " order=\"" << Order << "\"";
}

//----------------------------------------------------------------------------
void vtkDMMLTensorVolumeNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "measurementFrame"))
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for(int i=0; i<3; i++)
        {
        for(int j=0; j<3; j++)
          {
          ss >> val;
          this->MeasurementFrameMatrix[i][j] = val;
          }
        }
      }

    if (!strcmp(attName, "order"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> Order;
      }
  }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLTensorVolumeNode::SetMeasurementFrameMatrix(const double mf[3][3])
{
  for (int i=0; i<3; i++)
    {
    for (int j=0; j<3; j++)
      {
      this->MeasurementFrameMatrix[i][j] = mf[i][j];
      }
    }
}

//----------------------------------------------------------------------------
void vtkDMMLTensorVolumeNode::GetMeasurementFrameMatrix(double mf[3][3])
{
  for (int i=0; i<3; i++)
    {
    for (int j=0; j<3; j++)
      {
      mf[i][j]= this->MeasurementFrameMatrix[i][j];
      }
    }
}

//----------------------------------------------------------------------------
void vtkDMMLTensorVolumeNode::SetMeasurementFrameMatrix(vtkMatrix4x4 *mf)
{
  for (int i=0; i<3; i++)
    {
    for (int j=0; j<3; j++)
      {
      this->MeasurementFrameMatrix[i][j]=mf->GetElement(i,j);
      }
    }
}

//----------------------------------------------------------------------------
void vtkDMMLTensorVolumeNode::GetMeasurementFrameMatrix(vtkMatrix4x4 *mf)
{

  mf->Identity();
  for (int i=0; i<3; i++)
    {
    for (int j=0; j<3; j++)
      {
      mf->SetElement(i,j,this->MeasurementFrameMatrix[i][j]);
      }
    }
}

//----------------------------------------------------------------------------
void vtkDMMLTensorVolumeNode::SetMeasurementFrameMatrix(const double xr, const double xa, const double xs,
                           const double yr, const double ya, const double ys,
                           const double zr, const double za, const double zs)
{
  MeasurementFrameMatrix[0][0] = xr;
  MeasurementFrameMatrix[0][1] = xa;
  MeasurementFrameMatrix[0][2] = xs;
  MeasurementFrameMatrix[1][0] = yr;
  MeasurementFrameMatrix[1][1] = ya;
  MeasurementFrameMatrix[1][2] = ys;
  MeasurementFrameMatrix[2][0] = zr;
  MeasurementFrameMatrix[2][1] = za;
  MeasurementFrameMatrix[2][2] = zs;
}

//----------------------------------------------------------------------------
void vtkDMMLTensorVolumeNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkDMMLTensorVolumeNode* node = vtkDMMLTensorVolumeNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  // Matrices
  for(int i=0; i<3; i++)
    {
    for(int j=0; j<3; j++)
      {
      this->MeasurementFrameMatrix[i][j] = node->MeasurementFrameMatrix[i][j];
      }
    }
  this->Order = node->Order;
}

//----------------------------------------------------------------------------
void vtkDMMLTensorVolumeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << "MeasurementFrameMatrix:\n";
  for(int i=0; i<3; i++)
    {
    for(int j=0; j<3; j++)
      {
      os << indent << " " << this->MeasurementFrameMatrix[i][j];
      }
      os << indent << "\n";
    }
  os << "\n";
  os << "Order: "<<this->Order;

}

//----------------------------------------------------------------------------
vtkDMMLStorageNode* vtkDMMLTensorVolumeNode::CreateDefaultStorageNode()
{
  vtkDMMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkDMMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkDMMLNRRDStorageNode"));
}
