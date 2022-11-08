/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLDiffusionWeightedVolumeDisplayNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLDiffusionWeightedVolumeDisplayNode.h"

// VTK includes
#include <vtkImageAppendComponents.h>
#include <vtkImageData.h>
#include <vtkImageExtractComponents.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkImageThreshold.h>
#include <vtkObjectFactory.h>
#include <vtkVersion.h>

// STD includes
#include <sstream>

//------------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLDiffusionWeightedVolumeDisplayNode);

//----------------------------------------------------------------------------
vtkDMMLDiffusionWeightedVolumeDisplayNode::vtkDMMLDiffusionWeightedVolumeDisplayNode()
{
  this->DiffusionComponent = 0;
  this->ExtractComponent = vtkImageExtractComponents::New();
  this->Threshold->SetInputConnection( this->ExtractComponent->GetOutputPort());
  this->MapToWindowLevelColors->SetInputConnection(
    this->ExtractComponent->GetOutputPort());
}

//----------------------------------------------------------------------------
vtkDMMLDiffusionWeightedVolumeDisplayNode::~vtkDMMLDiffusionWeightedVolumeDisplayNode()
{
  this->ExtractComponent->Delete();
}

//----------------------------------------------------------------------------
void vtkDMMLDiffusionWeightedVolumeDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  std::stringstream ss;
  ss << this->DiffusionComponent;
  of << " diffusionComponent=\"" << ss.str() << "\"";
}

//----------------------------------------------------------------------------
void vtkDMMLDiffusionWeightedVolumeDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "diffusionComponent"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->DiffusionComponent;
      }
    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkDMMLDiffusionWeightedVolumeDisplayNode::Copy(vtkDMMLNode *anode)
{
  int disabledModify = this->StartModify();

  vtkDMMLDiffusionWeightedVolumeDisplayNode *node = (vtkDMMLDiffusionWeightedVolumeDisplayNode *) anode;
  this->SetDiffusionComponent(node->DiffusionComponent);
  this->Superclass::Copy(anode);


  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLDiffusionWeightedVolumeDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{

  Superclass::PrintSelf(os,indent);

  os << indent << "Diffusion Component:   " << this->DiffusionComponent << "\n";

}

//----------------------------------------------------------------------------
void vtkDMMLDiffusionWeightedVolumeDisplayNode
::SetInputToImageDataPipeline(vtkAlgorithmOutput *imageDataConnection)
{
  this->ExtractComponent->SetInputConnection(imageDataConnection);
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkDMMLDiffusionWeightedVolumeDisplayNode::GetInputImageDataConnection()
{
  return this->ExtractComponent->GetNumberOfInputConnections(0) ?
    this->ExtractComponent->GetInputConnection(0,0) : nullptr;;
}



//---------------------------------------------------------------------------
vtkAlgorithmOutput* vtkDMMLDiffusionWeightedVolumeDisplayNode::GetScalarImageDataConnection()
{
  return this->ExtractComponent->GetOutputPort();
}

//----------------------------------------------------------------------------
void vtkDMMLDiffusionWeightedVolumeDisplayNode::UpdateImageDataPipeline()
{
  this->ExtractComponent->SetComponents(this->GetDiffusionComponent());
  this->Superclass::UpdateImageDataPipeline();
}
