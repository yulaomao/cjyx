/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLVectorVolumeDisplayNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLVectorVolumeDisplayNode.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkImageAppendComponents.h>
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkImageExtractComponents.h>
#include <vtkImageRGBToHSI.h>
#include <vtkImageShiftScale.h>
#include <vtkImageStencil.h>
#include <vtkImageThreshold.h>
#include <vtkVersion.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLVectorVolumeDisplayNode);

//----------------------------------------------------------------------------
vtkDMMLVectorVolumeDisplayNode::vtkDMMLVectorVolumeDisplayNode()
{
 this->ScalarMode = this->scalarModeMagnitude;
 this->GlyphMode = this->glyphModeLines;

 this->ShiftScale = vtkImageShiftScale::New();
 this->RGBToHSI = vtkImageRGBToHSI::New();
 this->ExtractIntensity = vtkImageExtractComponents::New();

 this->ShiftScale->SetOutputScalarTypeToUnsignedChar();
 this->ShiftScale->SetClampOverflow(1);

 this->ExtractIntensity->SetInputConnection( this->RGBToHSI->GetOutputPort() );
 this->ExtractIntensity->SetComponents( 2 );

 this->Threshold->SetInputConnection( this->ExtractIntensity->GetOutputPort() );

 this->AppendComponents->RemoveAllInputs();
 this->AppendComponents->AddInputConnection(0, this->ShiftScale->GetOutputPort());
 this->AppendComponents->AddInputConnection(0, this->MultiplyAlpha->GetOutputPort());

 this->MultiplyAlpha->RemoveAllInputs();
 this->MultiplyAlpha->SetInputConnection(0, this->Threshold->GetOutputPort() );
}

//----------------------------------------------------------------------------
vtkDMMLVectorVolumeDisplayNode::~vtkDMMLVectorVolumeDisplayNode()
{
  this->ShiftScale->Delete();
  this->RGBToHSI->Delete();
  this->ExtractIntensity->Delete();
}

//----------------------------------------------------------------------------
void vtkDMMLVectorVolumeDisplayNode::SetInputToImageDataPipeline(vtkAlgorithmOutput *imageDataConnection)
{
  this->ShiftScale->SetInputConnection(imageDataConnection);
  this->RGBToHSI->SetInputConnection(imageDataConnection);
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkDMMLVectorVolumeDisplayNode::GetInputImageDataConnection()
{
  return this->ShiftScale->GetNumberOfInputConnections(0) ?
    this->ShiftScale->GetInputConnection(0,0) : nullptr;
}

//---------------------------------------------------------------------------
vtkAlgorithmOutput* vtkDMMLVectorVolumeDisplayNode::GetScalarImageDataConnection()
{
  return this->GetInputImageDataConnection();
}

//----------------------------------------------------------------------------
void vtkDMMLVectorVolumeDisplayNode::UpdateImageDataPipeline()
{
  Superclass::UpdateImageDataPipeline();

  double halfWindow = (this->GetWindow() / 2.);
  double min = this->GetLevel() - halfWindow;
  this->ShiftScale->SetShift ( -min );
  this->ShiftScale->SetScale ( 255. / (this->GetWindow()) );
}

//----------------------------------------------------------------------------
void vtkDMMLVectorVolumeDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  std::stringstream ss;

  ss.clear();
  ss << this->ScalarMode;
  of << " scalarMode=\"" << ss.str() << "\"";

  ss.clear();
  ss << this->GlyphMode;
  of << " glyphMode=\"" << ss.str() << "\"";
}

//----------------------------------------------------------------------------
void vtkDMMLVectorVolumeDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "scalarMode"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->ScalarMode;
      }
    else if (!strcmp(attName, "glyphMode"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->GlyphMode;
      }
    }

  this->EndModify(disabledModify);

}

//----------------------------------------------------------------------------
void vtkDMMLVectorVolumeDisplayNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);
  vtkDMMLVectorVolumeDisplayNode *node = vtkDMMLVectorVolumeDisplayNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }
  this->SetScalarMode(node->ScalarMode);
  this->SetGlyphMode(node->GlyphMode);
}

//----------------------------------------------------------------------------
void vtkDMMLVectorVolumeDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "Scalar Mode:   " << this->ScalarMode << "\n";
  os << indent << "Glyph Mode:    " << this->GlyphMode << "\n";
}



//---------------------------------------------------------------------------
void vtkDMMLVectorVolumeDisplayNode::ProcessDMMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessDMMLEvents(caller, event, callData);
}
