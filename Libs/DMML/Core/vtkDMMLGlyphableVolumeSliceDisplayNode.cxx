/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLGlyphableVolumeSliceDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

#include "vtkAlgorithmOutput.h"
#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include "vtkTransform.h"
#include "vtkImageData.h"
#include "vtkPolyData.h"

#include "vtkTransformPolyDataFilter.h"

#include "vtkDMMLGlyphableVolumeSliceDisplayNode.h"
#include <sstream>

//------------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLGlyphableVolumeSliceDisplayNode);

//----------------------------------------------------------------------------
vtkDMMLGlyphableVolumeSliceDisplayNode::vtkDMMLGlyphableVolumeSliceDisplayNode()
{
  this->ColorMode = this->colorModeScalar;

  this->SliceImagePort = nullptr;

  this->SliceToXYTransformer = vtkTransformPolyDataFilter::New();

  this->SliceToXYTransform = vtkTransform::New();

  this->SliceToXYMatrix = vtkMatrix4x4::New();
  this->SliceToXYMatrix->Identity();
  this->SliceToXYTransform->PreMultiply();
  this->SliceToXYTransform->SetMatrix(this->SliceToXYMatrix);

  //this->SliceToXYTransformer->SetInput(this->GlyphGlyphFilter->GetOutput());
  this->SliceToXYTransformer->SetTransform(this->SliceToXYTransform);

  // don't backface cull the glyphs - they may not be geometrically consistent
  // since they have been transformed in ways that may have flipped them.
  // See issue 1368
  this->BackfaceCulling = 0;
}


//----------------------------------------------------------------------------
vtkDMMLGlyphableVolumeSliceDisplayNode::~vtkDMMLGlyphableVolumeSliceDisplayNode()
{
  this->RemoveObservers ( vtkCommand::ModifiedEvent, this->DMMLCallbackCommand );
  this->SetSliceImagePort(nullptr);
  this->SliceToXYMatrix->Delete();
  this->SliceToXYTransform->Delete();
  this->SliceToXYTransformer->Delete();
}

//----------------------------------------------------------------------------
void vtkDMMLGlyphableVolumeSliceDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  of << " colorMode =\"" << this->ColorMode << "\"";
}


//----------------------------------------------------------------------------
void vtkDMMLGlyphableVolumeSliceDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "colorMode"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> ColorMode;
      }

    }

  this->EndModify(disabledModify);

}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkDMMLGlyphableVolumeSliceDisplayNode::Copy(vtkDMMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkDMMLGlyphableVolumeSliceDisplayNode *node = (vtkDMMLGlyphableVolumeSliceDisplayNode *) anode;

  this->SetColorMode(node->ColorMode);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLGlyphableVolumeSliceDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
 //int idx;

  Superclass::PrintSelf(os,indent);
  os << indent << "ColorMode:             " << this->ColorMode << "\n";
}
//----------------------------------------------------------------------------
void vtkDMMLGlyphableVolumeSliceDisplayNode::SetSliceGlyphRotationMatrix(vtkMatrix4x4 *vtkNotUsed(matrix))
{
}

//----------------------------------------------------------------------------
void vtkDMMLGlyphableVolumeSliceDisplayNode::SetSlicePositionMatrix(vtkMatrix4x4 *matrix)
{
//  if (this->GlyphGlyphFilter)
//    {
//    this->GlyphGlyphFilter->SetVolumePositionMatrix(matrix);
//    }
  this->SliceToXYMatrix->DeepCopy(matrix);
  this->SliceToXYMatrix->Invert();
  if (this->SliceToXYTransform)
    {
    this->SliceToXYTransform->SetMatrix(this->SliceToXYMatrix);
    }
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkDMMLGlyphableVolumeSliceDisplayNode::SetSliceImagePort(vtkAlgorithmOutput *imagePort)
{
   vtkSetObjectBodyMacro(SliceImagePort,vtkAlgorithmOutput,imagePort);
}

//----------------------------------------------------------------------------
void vtkDMMLGlyphableVolumeSliceDisplayNode
::SetInputToPolyDataPipeline(vtkAlgorithmOutput *vtkNotUsed(glyphPolyData))
{
  vtkErrorMacro(<< this->GetClassName() <<" ("<<this
                    <<"): SetInputPolyData method should not be used");
}

//---------------------------------------------------------------------------
vtkPolyData* vtkDMMLGlyphableVolumeSliceDisplayNode::GetOutputMesh()
{
  // Don't check input polydata as it is not used, but the image data instead.
  if (!this->GetOutputMeshConnection())
    {
    return nullptr;
    }
  return vtkPolyData::SafeDownCast(
    this->GetOutputMeshConnection()->GetProducer()->GetOutputDataObject(
      this->GetOutputMeshConnection()->GetIndex()));
}
//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkDMMLGlyphableVolumeSliceDisplayNode
::GetOutputMeshConnection()
{
  return nullptr;
}

//----------------------------------------------------------------------------
void vtkDMMLGlyphableVolumeSliceDisplayNode::UpdateAssignedAttribute()
{
  this->SliceToXYTransformer->SetInputConnection(
    this->GetOutputMeshConnection());
}

//---------------------------------------------------------------------------
vtkPolyData* vtkDMMLGlyphableVolumeSliceDisplayNode::GetSliceOutputPolyData()
{
  // Don't check input polydata as it is not used, but the image data instead.
  if (!this->GetSliceOutputPort())
    {
    return nullptr;
    }
  return vtkPolyData::SafeDownCast(
    this->GetSliceOutputPort()->GetProducer()->GetOutputDataObject(
      this->GetSliceOutputPort()->GetIndex()));
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkDMMLGlyphableVolumeSliceDisplayNode::GetSliceOutputPort()
{
  return this->SliceToXYTransformer->GetOutputPort();
}

//---------------------------------------------------------------------------
void vtkDMMLGlyphableVolumeSliceDisplayNode::ProcessDMMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessDMMLEvents(caller, event, callData);
  return;
}

//-----------------------------------------------------------
void vtkDMMLGlyphableVolumeSliceDisplayNode::UpdateScene(vtkDMMLScene *scene)
{
   Superclass::UpdateScene(scene);
}

//-----------------------------------------------------------
void vtkDMMLGlyphableVolumeSliceDisplayNode::UpdateReferences()
{
  Superclass::UpdateReferences();
}
