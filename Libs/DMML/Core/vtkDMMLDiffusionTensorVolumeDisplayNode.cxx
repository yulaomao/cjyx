/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLDiffusionTensorVolumeDisplayNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLDiffusionTensorVolumeDisplayNode.h"
#include "vtkDMMLDiffusionTensorVolumeSliceDisplayNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLVolumeNode.h"

// Teem includes
#include <vtkDiffusionTensorGlyph.h>
#include <vtkDiffusionTensorMathematics.h>

// VTK includes
#include <vtkImageAppendComponents.h>
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkImageExtractComponents.h>
#include <vtkImageLogic.h>
#include <vtkImageMathematics.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkImageShiftScale.h>
#include <vtkImageThreshold.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkSphereSource.h>
#include <vtkVersion.h>

// STD includes

//------------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLDiffusionTensorVolumeDisplayNode);

//----------------------------------------------------------------------------
vtkDMMLDiffusionTensorVolumeDisplayNode
::vtkDMMLDiffusionTensorVolumeDisplayNode()
{
 this->ScalarInvariant = vtkDMMLDiffusionTensorDisplayPropertiesNode::ColorOrientation;
 this->DTIMathematics = vtkDiffusionTensorMathematics::New();
 this->DTIMathematicsAlpha = vtkDiffusionTensorMathematics::New();
 this->Threshold->SetInputConnection( this->DTIMathematics->GetOutputPort());
 this->MapToWindowLevelColors->SetInputConnection( this->DTIMathematics->GetOutputPort());

 this->ShiftScale = vtkImageShiftScale::New();
 this->ShiftScale->SetOutputScalarTypeToUnsignedChar();
 this->ShiftScale->SetClampOverflow(1);

 this->ImageCast = vtkImageCast::New();
 this->ImageCast->SetOutputScalarTypeToUnsignedChar();

 this->ImageMath  = vtkImageMathematics::New();
 this->ImageMath->SetOperationToMultiplyByK();
 this->ImageMath->SetConstantK(255);

 this->DiffusionTensorGlyphFilter = vtkDiffusionTensorGlyph::New();
 vtkSphereSource *sphere = vtkSphereSource::New();
 this->DiffusionTensorGlyphFilter->SetSourceConnection( sphere->GetOutputPort() );
 sphere->Delete();

 this->ScalarRangeFlag = vtkDMMLDisplayNode::UseDataScalarRange;
}

//----------------------------------------------------------------------------
vtkDMMLDiffusionTensorVolumeDisplayNode
::~vtkDMMLDiffusionTensorVolumeDisplayNode()
{
  this->DTIMathematics->Delete();
  this->DTIMathematicsAlpha->Delete();

  this->DiffusionTensorGlyphFilter->Delete();
  this->ShiftScale->Delete();
  this->ImageMath->Delete();
  this->ImageCast->Delete();
}


//----------------------------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeDisplayNode::WriteXML(ostream& of, int nIndent)
{
  this->Superclass::WriteXML(of, nIndent);

  of << " scalarInvariant=\"" << this->ScalarInvariant << "\"";
}

//----------------------------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeDisplayNode
::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "scalarInvariant"))
      {
      int scalarInvariant;
      std::stringstream ss;
      ss << attValue;
      ss >> scalarInvariant;
      this->SetScalarInvariant(scalarInvariant);
      }

    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkDMMLDiffusionTensorVolumeDisplayNode::Copy(vtkDMMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkDMMLDiffusionTensorVolumeDisplayNode *node =
    (vtkDMMLDiffusionTensorVolumeDisplayNode *) anode;
  this->SetScalarInvariant(node->ScalarInvariant);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeDisplayNode
::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "ScalarInvariant:             " << this->ScalarInvariant << "\n";
}

//---------------------------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeDisplayNode
::ProcessDMMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  this->Superclass::ProcessDMMLEvents(caller, event, callData);
}

//-----------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeDisplayNode::UpdateScene(vtkDMMLScene *scene)
{
  this->Superclass::UpdateScene(scene);
}

//-----------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeDisplayNode::UpdateReferences()
{
  this->Superclass::UpdateReferences();
}


//----------------------------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeDisplayNode
::UpdateReferenceID(const char *oldID, const char *newID)
{
  this->Superclass::UpdateReferenceID(oldID, newID);
}

//----------------------------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeDisplayNode::UpdateImageDataPipeline()
{
  int scalarInvariant = this->GetScalarInvariant();
  this->DTIMathematics->SetOperation(scalarInvariant);
  switch (scalarInvariant)
    {
    case vtkDMMLDiffusionTensorDisplayPropertiesNode::ColorOrientation:
    case vtkDMMLDiffusionTensorDisplayPropertiesNode::ColorMode:
    case vtkDMMLDiffusionTensorDisplayPropertiesNode::ColorOrientationMiddleEigenvector:
    case vtkDMMLDiffusionTensorDisplayPropertiesNode::ColorOrientationMinEigenvector:
      {
      // alpha
      this->DTIMathematics->SetScaleFactor(1000.0);
      this->DTIMathematicsAlpha->SetOperation(
        vtkDMMLDiffusionTensorDisplayPropertiesNode::FractionalAnisotropy);
      this->ImageMath->SetInputConnection( this->DTIMathematicsAlpha->GetOutputPort());
      this->ImageCast->SetInputConnection( this->ImageMath->GetOutputPort());
      this->Threshold->SetInputConnection( this->ImageCast->GetOutputPort());

      // window/level
      this->ShiftScale->SetInputConnection(this->DTIMathematics->GetOutputPort());
      double halfWindow = (this->GetWindow() / 2.);
      double min = this->GetLevel() - halfWindow;
      this->ShiftScale->SetShift ( -min );
      this->ShiftScale->SetScale ( 255. / (this->GetWindow()) );

      this->ExtractRGB->SetInputConnection(this->ShiftScale->GetOutputPort());
      if (this->AppendComponents->GetInputConnection(0, 0) != this->ExtractRGB->GetOutputPort() ||
          this->AppendComponents->GetInputConnection(0, 1) != this->Threshold->GetOutputPort())
        {
        this->AppendComponents->RemoveAllInputs();
        this->AppendComponents->SetInputConnection(0, this->ExtractRGB->GetOutputPort());
        this->AppendComponents->AddInputConnection(0, this->Threshold->GetOutputPort() );
        }
      break;
      }
    default:
      this->DTIMathematics->SetScaleFactor(1.0);
      this->Threshold->SetInputConnection( this->DTIMathematics->GetOutputPort());
      this->MapToWindowLevelColors->SetInputConnection( this->DTIMathematics->GetOutputPort());
      this->ExtractRGB->SetInputConnection(this->MapToColors->GetOutputPort());
      if (this->AppendComponents->GetInputConnection(0, 0) != this->ExtractRGB->GetOutputPort() ||
          this->AppendComponents->GetInputConnection(0, 1) != this->AlphaLogic->GetOutputPort())
        {
        this->AppendComponents->RemoveAllInputs();
        this->AppendComponents->SetInputConnection(0, this->ExtractRGB->GetOutputPort() );
        this->AppendComponents->AddInputConnection(0, this->AlphaLogic->GetOutputPort() );
        }
      break;
    }

  Superclass::UpdateImageDataPipeline();

}

//----------------------------------------------------------------------------
void
vtkDMMLDiffusionTensorVolumeDisplayNode::SetTensorRotationMatrix(vtkMatrix4x4 *rotationMatrix)
{
  this->DTIMathematics->SetTensorRotationMatrix(rotationMatrix);
  this->DTIMathematicsAlpha->SetTensorRotationMatrix(rotationMatrix);
}

//----------------------------------------------------------------------------
std::vector< vtkDMMLGlyphableVolumeSliceDisplayNode*>
vtkDMMLDiffusionTensorVolumeDisplayNode::GetSliceGlyphDisplayNodes(
  vtkDMMLVolumeNode* volumeNode )
{
  std::vector< vtkDMMLGlyphableVolumeSliceDisplayNode*> nodes;
  int nnodes = volumeNode->GetNumberOfDisplayNodes();
  vtkDMMLDiffusionTensorVolumeSliceDisplayNode *node = nullptr;
  for (int n=0; n<nnodes; n++)
    {
    node = vtkDMMLDiffusionTensorVolumeSliceDisplayNode::SafeDownCast(
      volumeNode->GetNthDisplayNode(n));
    if (node)
      {
      nodes.push_back(node);
      }
    }
  return nodes;
}

//----------------------------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeDisplayNode
::AddSliceGlyphDisplayNodes( vtkDMMLVolumeNode* volumeNode )
{
  std::vector< vtkDMMLGlyphableVolumeSliceDisplayNode*> nodes =
    this->GetSliceGlyphDisplayNodes( volumeNode );
  if (nodes.size() == 0)
    {
    vtkDMMLDiffusionTensorDisplayPropertiesNode *glyphDTDPN =
      vtkDMMLDiffusionTensorDisplayPropertiesNode::New();
    this->GetScene()->AddNode(glyphDTDPN);
    int modifyState = glyphDTDPN->StartModify();
    glyphDTDPN->SetLineGlyphResolution(5);
    glyphDTDPN->EndModify(modifyState);
    glyphDTDPN->Delete();

    for (int i=0; i<3; i++)
      {
      if (this->GetScene())
        {
        vtkDMMLDiffusionTensorVolumeSliceDisplayNode *node =
          vtkDMMLDiffusionTensorVolumeSliceDisplayNode::New();
        if (i == 0)
          {
          node->SetName("Red");
          }
        else if (i == 1)
          {
          node->SetName("Yellow");
          }
        else if (i == 2)
          {
          node->SetName("Green");
          }

        this->GetScene()->AddNode(node);
        node->Delete();

        int modifyState2 = node->StartModify();
        node->SetVisibility(0);

        node->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(glyphDTDPN->GetID());

        node->SetAndObserveColorNodeID("vtkDMMLColorTableNodeRainbow");

        node->EndModify(modifyState2);

        volumeNode->AddAndObserveDisplayNodeID(node->GetID());

        }
      }
   }
}

//----------------------------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeDisplayNode
::SetInputToImageDataPipeline(vtkAlgorithmOutput *imageDataConnection)
{
  this->DTIMathematics->SetInputConnection(imageDataConnection);
  this->DTIMathematicsAlpha->SetInputConnection(imageDataConnection);
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkDMMLDiffusionTensorVolumeDisplayNode
::GetInputImageDataConnection()
{
  return this->DTIMathematics->GetNumberOfInputConnections(0) ?
    this->DTIMathematics->GetInputConnection(0, 0) : nullptr;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkDMMLDiffusionTensorVolumeDisplayNode::GetBackgroundImageStencilDataConnection()
{
  switch (this->GetScalarInvariant())
    {
    case vtkDMMLDiffusionTensorDisplayPropertiesNode::ColorOrientation:
    case vtkDMMLDiffusionTensorDisplayPropertiesNode::ColorMode:
    case vtkDMMLDiffusionTensorDisplayPropertiesNode::ColorOrientationMiddleEigenvector:
    case vtkDMMLDiffusionTensorDisplayPropertiesNode::ColorOrientationMinEigenvector:
      {
      return nullptr;
      }
    default:
      return this->Superclass::GetBackgroundImageStencilDataConnection();
    }
}

//---------------------------------------------------------------------------
vtkAlgorithmOutput* vtkDMMLDiffusionTensorVolumeDisplayNode
::GetScalarImageDataConnection()
{
  return this->DTIMathematics->GetOutputPort();
}

//---------------------------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeDisplayNode
::GetDisplayScalarRange(double range[2])
{
  const int ScalarInvariant = this->GetScalarInvariant();
  if (vtkDMMLDiffusionTensorDisplayPropertiesNode::
      ScalarInvariantHasKnownScalarRange(ScalarInvariant))
    {
    vtkDMMLDiffusionTensorDisplayPropertiesNode
      ::ScalarInvariantKnownScalarRange(ScalarInvariant, range);
    }
  else
    {
    this->DTIMathematics->Update();
    this->GetScalarImageData()->GetScalarRange(range);
    }
}

//----------------------------------------------------------------------------
std::vector<int> vtkDMMLDiffusionTensorVolumeDisplayNode::GetSupportedColorModes()
{
  std::vector<int> modes;
  modes.push_back(vtkDMMLDiffusionTensorDisplayPropertiesNode::FractionalAnisotropy);
  modes.push_back(vtkDMMLDiffusionTensorDisplayPropertiesNode::ColorOrientation);
  modes.push_back(vtkDMMLDiffusionTensorDisplayPropertiesNode::Trace);
  modes.push_back(vtkDMMLDiffusionTensorDisplayPropertiesNode::LinearMeasure);
  modes.push_back(vtkDMMLDiffusionTensorDisplayPropertiesNode::PlanarMeasure);
  modes.push_back(vtkDMMLDiffusionTensorDisplayPropertiesNode::SphericalMeasure);
  modes.push_back(vtkDMMLDiffusionTensorDisplayPropertiesNode::RelativeAnisotropy);
  modes.push_back(vtkDMMLDiffusionTensorDisplayPropertiesNode::ParallelDiffusivity);
  modes.push_back(vtkDMMLDiffusionTensorDisplayPropertiesNode::PerpendicularDiffusivity);
  modes.push_back(vtkDMMLDiffusionTensorDisplayPropertiesNode::MaxEigenvalue);
  modes.push_back(vtkDMMLDiffusionTensorDisplayPropertiesNode::MidEigenvalue);
  modes.push_back(vtkDMMLDiffusionTensorDisplayPropertiesNode::MinEigenvalue);
  return modes;
}

//----------------------------------------------------------------------------
int vtkDMMLDiffusionTensorVolumeDisplayNode::GetNumberOfScalarInvariants()
{
  static std::vector<int> modes =
    vtkDMMLDiffusionTensorVolumeDisplayNode::GetSupportedColorModes();
  return modes.size();
}

//----------------------------------------------------------------------------
int vtkDMMLDiffusionTensorVolumeDisplayNode::GetNthScalarInvariant(int i)
{
  static std::vector<int> modes =
    vtkDMMLDiffusionTensorVolumeDisplayNode::GetSupportedColorModes();
  return modes[i];
}
