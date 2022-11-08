/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLVolumeDisplayNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLVolumeDisplayNode.h"
#include "vtkDMMLVolumeNode.h"

// VTK includes
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkCommand.h>
#include <vtkImageData.h>
#include <vtkImageStencilData.h>
#include <vtkTrivialProducer.h>

// Initialize static member that controls resampling --
// old comment: "This offset will be changed to 0.5 from 0.0 per 2/8/2002 Cjyx
// development meeting, to move ijk coordinates to voxel centers."

//----------------------------------------------------------------------------
vtkDMMLVolumeDisplayNode::vtkDMMLVolumeDisplayNode()
{
  // try setting a default greyscale color map
  //this->SetDefaultColorMap(0);
}

//----------------------------------------------------------------------------
vtkDMMLVolumeDisplayNode::~vtkDMMLVolumeDisplayNode() = default;

//----------------------------------------------------------------------------
void vtkDMMLVolumeDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkDMMLVolumeDisplayNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
void vtkDMMLVolumeDisplayNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);
  vtkDMMLVolumeDisplayNode *node = vtkDMMLVolumeDisplayNode::SafeDownCast(anode);
  if (node)
    {
    this->SetInputImageDataConnection(node->GetInputImageDataConnection());
    }
  this->UpdateImageDataPipeline();
}

//---------------------------------------------------------------------------
void vtkDMMLVolumeDisplayNode::ProcessDMMLEvents(vtkObject *caller,
                                                 unsigned long event,
                                                 void *callData)
{
  if (event ==  vtkCommand::ModifiedEvent)
    {
    this->UpdateImageDataPipeline();
    }
  this->Superclass::ProcessDMMLEvents(caller, event, callData);
}

//----------------------------------------------------------------------------
void vtkDMMLVolumeDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//-----------------------------------------------------------
void vtkDMMLVolumeDisplayNode::UpdateScene(vtkDMMLScene *scene)
{
  this->Superclass::UpdateScene(scene);
}

//-----------------------------------------------------------
void vtkDMMLVolumeDisplayNode::UpdateReferences()
{
  this->Superclass::UpdateReferences();
}

//---------------------------------------------------------------------------
vtkAlgorithmOutput* vtkDMMLVolumeDisplayNode::GetImageDataConnection()
{
/*
  if (!this->GetInputImageData())
    {
    return 0;
    }
  this->UpdateImageDataPipeline();
*/
  return this->GetOutputImageDataConnection();
}

//----------------------------------------------------------------------------
void vtkDMMLVolumeDisplayNode
::SetInputImageDataConnection(vtkAlgorithmOutput *imageDataConnection)
{
  if (this->GetInputImageDataConnection() == imageDataConnection)
    {
    return;
    }
  this->SetInputToImageDataPipeline(imageDataConnection);
  this->Modified();
}
//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkDMMLVolumeDisplayNode
::GetInputImageDataConnection()
{
  return nullptr;
}

//----------------------------------------------------------------------------
void vtkDMMLVolumeDisplayNode::SetInputToImageDataPipeline(vtkAlgorithmOutput *vtkNotUsed(imageDataConnection))
{
}

//----------------------------------------------------------------------------
vtkImageData* vtkDMMLVolumeDisplayNode::GetInputImageData()
{
  vtkAlgorithmOutput* imageConnection = this->GetInputImageDataConnection();
  vtkAlgorithm* producer = imageConnection ? imageConnection->GetProducer() : nullptr;
  return vtkImageData::SafeDownCast(
    producer ? producer->GetOutputDataObject(0) : nullptr);
}

//----------------------------------------------------------------------------
void vtkDMMLVolumeDisplayNode::SetBackgroundImageStencilDataConnection(vtkAlgorithmOutput* vtkNotUsed(imageDataConnection))
{
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkDMMLVolumeDisplayNode::GetBackgroundImageStencilDataConnection()
{
  return nullptr;
}

//----------------------------------------------------------------------------
vtkImageStencilData* vtkDMMLVolumeDisplayNode::GetBackgroundImageStencilData()
{
  vtkAlgorithmOutput* imageConnection = this->GetBackgroundImageStencilDataConnection();
  vtkAlgorithm* producer = imageConnection ? imageConnection->GetProducer() : nullptr;
  return vtkImageStencilData::SafeDownCast(
    producer ? producer->GetOutputDataObject(0) : nullptr);
}

//----------------------------------------------------------------------------
vtkImageData* vtkDMMLVolumeDisplayNode::GetOutputImageData()
{
  vtkAlgorithmOutput* imageConnection = this->GetOutputImageDataConnection();
  vtkAlgorithm* producer = imageConnection ? imageConnection->GetProducer() : nullptr;
  return vtkImageData::SafeDownCast(
    producer ? producer->GetOutputDataObject(0) : nullptr);
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkDMMLVolumeDisplayNode::GetOutputImageDataConnection()
{
  return nullptr;
}

//----------------------------------------------------------------------------
void vtkDMMLVolumeDisplayNode::UpdateImageDataPipeline()
{
}

//----------------------------------------------------------------------------
void vtkDMMLVolumeDisplayNode::SetDefaultColorMap()
{
  this->SetAndObserveColorNodeID("vtkDMMLColorTableNodeGrey");
}

//----------------------------------------------------------------------------
vtkDMMLVolumeNode* vtkDMMLVolumeDisplayNode::GetVolumeNode()
{
  return vtkDMMLVolumeNode::SafeDownCast(this->GetDisplayableNode());
}

//----------------------------------------------------------------------------
