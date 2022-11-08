/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLDiffusionTensorVolumeSliceDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include <vtkImageData.h>
#include <vtkVersion.h>

#include "vtkDiffusionTensorGlyph.h"

#include "vtkTransformPolyDataFilter.h"

#include "vtkDMMLDiffusionTensorDisplayPropertiesNode.h"
#include "vtkDMMLDiffusionTensorVolumeSliceDisplayNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLDisplayableNode.h"

vtkCxxSetReferenceStringMacro(vtkDMMLDiffusionTensorVolumeSliceDisplayNode, DiffusionTensorDisplayPropertiesNodeID);

//------------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLDiffusionTensorVolumeSliceDisplayNode);

//----------------------------------------------------------------------------
vtkDMMLDiffusionTensorVolumeSliceDisplayNode::vtkDMMLDiffusionTensorVolumeSliceDisplayNode()
  :vtkDMMLGlyphableVolumeSliceDisplayNode()
{

  // Enumerated
  this->DiffusionTensorDisplayPropertiesNode = nullptr;
  this->DiffusionTensorDisplayPropertiesNodeID = nullptr;


  this->DiffusionTensorGlyphFilter = vtkDiffusionTensorGlyph::New();
  this->DiffusionTensorGlyphFilter->SetInputConnection(this->SliceImagePort);
  this->DiffusionTensorGlyphFilter->SetResolution (1);

  this->ColorMode = this->colorModeScalar;

  this->UpdateAssignedAttribute();
}


//----------------------------------------------------------------------------
vtkDMMLDiffusionTensorVolumeSliceDisplayNode::~vtkDMMLDiffusionTensorVolumeSliceDisplayNode()
{
  this->RemoveObservers ( vtkCommand::ModifiedEvent, this->DMMLCallbackCommand );
  this->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(nullptr);
  this->DiffusionTensorGlyphFilter->Delete();
}

//----------------------------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeSliceDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  if (this->DiffusionTensorDisplayPropertiesNodeID != nullptr)
    {
    of << " DiffusionTensorDisplayPropertiesNodeRef=\"" << this->DiffusionTensorDisplayPropertiesNodeID << "\"";
    }
}


//----------------------------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeSliceDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "DiffusionTensorDisplayPropertiesNodeRef"))
      {
      this->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(attValue);
      }
    }

  this->EndModify(disabledModify);
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkDMMLDiffusionTensorVolumeSliceDisplayNode::Copy(vtkDMMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkDMMLDiffusionTensorVolumeSliceDisplayNode *node = (vtkDMMLDiffusionTensorVolumeSliceDisplayNode *) anode;

  this->SetDiffusionTensorDisplayPropertiesNodeID(node->DiffusionTensorDisplayPropertiesNodeID);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeSliceDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
 //int idx;

  Superclass::PrintSelf(os,indent);
//  os << indent << "ColorMode:             " << this->ColorMode << "\n";
}
//----------------------------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeSliceDisplayNode::SetSliceGlyphRotationMatrix(vtkMatrix4x4 *matrix)
{
  this->DiffusionTensorGlyphFilter->SetTensorRotationMatrix(matrix);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeSliceDisplayNode::SetSlicePositionMatrix(vtkMatrix4x4 *matrix)
{
  // We need to call vtkDiffusionTensorGlyph::SetVolumePositionMatrix BEFORE
  // calling Superclass::SetSlicePositionMatrix(matrix)
  // because the later fire the even Modified() which will update the pipeline
  // and execute the filter that needs to be up-to-date.
  this->DiffusionTensorGlyphFilter->SetVolumePositionMatrix(matrix);
  Superclass::SetSlicePositionMatrix(matrix);
}

//----------------------------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeSliceDisplayNode::SetSliceImagePort(vtkAlgorithmOutput *imagePort)
{
  this->DiffusionTensorGlyphFilter->SetInputConnection(imagePort);
  this->Superclass::SetSliceImagePort(imagePort);
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkDMMLDiffusionTensorVolumeSliceDisplayNode
::GetOutputMeshConnection()
{
  return this->DiffusionTensorGlyphFilter->GetOutputPort();
}

//----------------------------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeSliceDisplayNode::UpdateAssignedAttribute()
{
  this->Superclass::UpdateAssignedAttribute();

  // set display properties according to the tensor-specific display properties node for glyphs
  vtkDMMLDiffusionTensorDisplayPropertiesNode * dtDPN =
    this->GetDiffusionTensorDisplayPropertiesNode( );

  this->DiffusionTensorGlyphFilter->SetSourceConnection(
    dtDPN ?
    dtDPN->GetGlyphConnection() : nullptr );

  if (dtDPN == nullptr ||
      this->SliceImagePort == nullptr ||
      dtDPN->GetGlyphGeometry( ) == vtkDMMLDiffusionTensorDisplayPropertiesNode::Superquadrics)
    {
    this->ScalarVisibilityOff();
    return;
    }

  // TO DO: need filter to calculate FA, average FA, etc. as requested

  // get tensors from the fiber bundle node and glyph them
  // TO DO: include superquadrics
  // if glyph type is other than superquadrics, get glyph source
  this->DiffusionTensorGlyphFilter->ClampScalingOff();

  // TO DO: implement max # ellipsoids, random sampling features
  this->DiffusionTensorGlyphFilter->SetResolution(1);
  this->DiffusionTensorGlyphFilter->SetDimensionResolution( dtDPN->GetLineGlyphResolution(), dtDPN->GetLineGlyphResolution());
  this->DiffusionTensorGlyphFilter->SetScaleFactor( dtDPN->GetGlyphScaleFactor( ) );

  vtkDebugMacro("setting glyph geometry" << dtDPN->GetGlyphGeometry( ) );

  // set glyph coloring
  if (this->GetColorMode ( ) == colorModeSolid)
    {
    this->ScalarVisibilityOff( );
    }
  else if (this->GetColorMode ( ) == colorModeScalar)
    {
    switch ( dtDPN->GetColorGlyphBy( ))
      {
      case vtkDMMLDiffusionTensorDisplayPropertiesNode::FractionalAnisotropy:
        {
        vtkDebugMacro("coloring with FA==============================");
        this->ScalarVisibilityOn( );
        this->DiffusionTensorGlyphFilter->ColorGlyphsByFractionalAnisotropy( );
        }
        break;
      case vtkDMMLDiffusionTensorDisplayPropertiesNode::LinearMeasure:
        {
        vtkDebugMacro("coloring with Cl=============================");
        this->ScalarVisibilityOn( );
        this->DiffusionTensorGlyphFilter->ColorGlyphsByLinearMeasure( );
        }
        break;
      case vtkDMMLDiffusionTensorDisplayPropertiesNode::Trace:
        {
        vtkDebugMacro("coloring with trace =================");
        this->ScalarVisibilityOn( );
        this->DiffusionTensorGlyphFilter->ColorGlyphsByTrace( );
        }
        break;
      case vtkDMMLDiffusionTensorDisplayPropertiesNode::ColorOrientation:
        {
        vtkDebugMacro("coloring with direction (re-implement)");
        this->ScalarVisibilityOn( );
        this->DiffusionTensorGlyphFilter->ColorGlyphsByOrientation( );
        vtkDMMLNode* colorNode = this->GetScene()->GetNodeByID("vtkDMMLColorTableNodeFullRainbow");
        if (colorNode)
          {
          this->SetAndObserveColorNodeID(colorNode->GetID());
          }
        }
        break;
      case vtkDMMLDiffusionTensorDisplayPropertiesNode::PlanarMeasure:
        {
        vtkDebugMacro("coloring with planar");
        this->ScalarVisibilityOn( );
        this->DiffusionTensorGlyphFilter->ColorGlyphsByPlanarMeasure( );
        }
        break;
      case vtkDMMLDiffusionTensorDisplayPropertiesNode::MaxEigenvalue:
        {
        vtkDebugMacro("coloring with max eigenval");
        this->ScalarVisibilityOn( );
        this->DiffusionTensorGlyphFilter->ColorGlyphsByMaxEigenvalue( );
        }
        break;
      case vtkDMMLDiffusionTensorDisplayPropertiesNode::MidEigenvalue:
        {
        vtkDebugMacro("coloring with mid eigenval");
        this->ScalarVisibilityOn( );
        this->DiffusionTensorGlyphFilter->ColorGlyphsByMidEigenvalue( );
        }
        break;
      case vtkDMMLDiffusionTensorDisplayPropertiesNode::MinEigenvalue:
        {
        vtkDebugMacro("coloring with min eigenval");
        this->ScalarVisibilityOn( );
        this->DiffusionTensorGlyphFilter->ColorGlyphsByMinEigenvalue( );
        }
        break;
      case vtkDMMLDiffusionTensorDisplayPropertiesNode::RelativeAnisotropy:
        {
        vtkDebugMacro("coloring with relative anisotropy");
        this->ScalarVisibilityOn( );
        this->DiffusionTensorGlyphFilter->ColorGlyphsByRelativeAnisotropy( );
        }
        break;
      default:
        {
        vtkDebugMacro("coloring with relative anisotropy");
        this->ScalarVisibilityOff( );
        }
        break;
      }
    }

  // Updating the filter can be time consuming, we want to refrain from updating
  // as much as possible. Not updating the filter may result into an out-of-date
  // scalar range if AutoScalarRange is true. We infer here that the user doesn't
  // care of an unsync scalar range value if the display node is invisible, or if
  // the glyphs are invisible (Visibility vs ScalarVisibility).
  // Moreover, if the input is null the filter would generate an error.
  if (this->GetVisibility() &&
      this->GetScalarVisibility() &&
      this->GetAutoScalarRange() &&
      this->SliceImagePort != nullptr)
    {
          int ScalarInvariant =  0;
          if ( DiffusionTensorDisplayPropertiesNode )
          {
            ScalarInvariant = DiffusionTensorDisplayPropertiesNode->GetColorGlyphBy();
          }

          double range[2];
          if (DiffusionTensorDisplayPropertiesNode && vtkDMMLDiffusionTensorDisplayPropertiesNode::ScalarInvariantHasKnownScalarRange(ScalarInvariant))
          {
            vtkDMMLDiffusionTensorDisplayPropertiesNode::ScalarInvariantKnownScalarRange(ScalarInvariant, range);
          } else {
            this->DiffusionTensorGlyphFilter->Update();
            this->DiffusionTensorGlyphFilter->GetOutput()->GetScalarRange(range);
          }
          this->ScalarRange[0] = range[0];
          this->ScalarRange[1] = range[1];
    }
}

//----------------------------------------------------------------------------
vtkDMMLDiffusionTensorDisplayPropertiesNode* vtkDMMLDiffusionTensorVolumeSliceDisplayNode::GetDiffusionTensorDisplayPropertiesNode ( )
{
  vtkDMMLDiffusionTensorDisplayPropertiesNode* node = nullptr;

  // Find the node corresponding to the ID we have saved.
  if  ( this->GetScene() && this->GetDiffusionTensorDisplayPropertiesNodeID() )
    {
    vtkDMMLNode* cnode = this->GetScene()->GetNodeByID( this->DiffusionTensorDisplayPropertiesNodeID );
    node = vtkDMMLDiffusionTensorDisplayPropertiesNode::SafeDownCast ( cnode );
    }

  return node;
}

//----------------------------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeSliceDisplayNode::SetAndObserveDiffusionTensorDisplayPropertiesNodeID ( const char *id )
{
  vtkDebugMacro(<< this->GetClassName() << ": Setting and Observing Diffusion Tensor Display Properties ID: " << id  );

  if (
      (id != this->GetDiffusionTensorDisplayPropertiesNodeID())
      && id != nullptr && this->GetDiffusionTensorDisplayPropertiesNodeID() != nullptr
      && (strcmp(id, this->GetDiffusionTensorDisplayPropertiesNodeID()) == 0)
      )
    {
    return;
    }

  // Stop observing any old node
  vtkSetAndObserveDMMLObjectMacro ( this->DiffusionTensorDisplayPropertiesNode, nullptr );

  // Set the ID. This is the "ground truth" reference to the node.
  this->SetDiffusionTensorDisplayPropertiesNodeID ( id );

  // Get the node corresponding to the ID. This pointer is only to observe the object.
  vtkDMMLNode *cnode = this->GetDiffusionTensorDisplayPropertiesNode ( );

  // Observe the node using the pointer.
  vtkSetAndObserveDMMLObjectMacro ( this->DiffusionTensorDisplayPropertiesNode , cnode );

  //The new DiffusionTensorDisplayPropertiesNode can have a different setting on the properties
  //so we emit the event that the polydata has been modified
  if (cnode && this->SliceImagePort)
    {
    this->Modified();
    }

}
//---------------------------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeSliceDisplayNode::ProcessDMMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  this->Superclass::ProcessDMMLEvents(caller, event, callData);
  this->UpdateAssignedAttribute();

  // Let everyone know that the "display" has changed.
  vtkDMMLDiffusionTensorDisplayPropertiesNode* propertiesNode =
    vtkDMMLDiffusionTensorDisplayPropertiesNode::SafeDownCast(caller);
  if (propertiesNode != nullptr &&
      this->DiffusionTensorDisplayPropertiesNodeID != nullptr &&
      propertiesNode->GetID() != nullptr &&
      strcmp(this->DiffusionTensorDisplayPropertiesNodeID,
             propertiesNode->GetID()) == 0 &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->Modified();
    }
}

//-----------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeSliceDisplayNode::UpdateScene(vtkDMMLScene *scene)
{
   Superclass::UpdateScene(scene);

   this->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(this->GetDiffusionTensorDisplayPropertiesNodeID());
}

//-----------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeSliceDisplayNode::UpdateReferences()
{
  Superclass::UpdateReferences();

  if (this->DiffusionTensorDisplayPropertiesNodeID != nullptr && this->Scene->GetNodeByID(this->DiffusionTensorDisplayPropertiesNodeID) == nullptr)
    {
    this->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(nullptr);
    }
}


//----------------------------------------------------------------------------
void vtkDMMLDiffusionTensorVolumeSliceDisplayNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID, newID);
  if (this->DiffusionTensorDisplayPropertiesNodeID && !strcmp(oldID, this->DiffusionTensorDisplayPropertiesNodeID))
    {
    this->SetDiffusionTensorDisplayPropertiesNodeID(newID);
    }
}

//----------------------------------------------------------------------------
std::vector<int> vtkDMMLDiffusionTensorVolumeSliceDisplayNode::GetSupportedColorModes()
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
int vtkDMMLDiffusionTensorVolumeSliceDisplayNode::GetNumberOfScalarInvariants()
{
  static std::vector<int> modes = vtkDMMLDiffusionTensorVolumeSliceDisplayNode::GetSupportedColorModes();
  return modes.size();
}

//----------------------------------------------------------------------------
int vtkDMMLDiffusionTensorVolumeSliceDisplayNode::GetNthScalarInvariant(int i)
{
  static std::vector<int> modes = vtkDMMLDiffusionTensorVolumeSliceDisplayNode::GetSupportedColorModes();
  return modes[i];
}



