
// DMMLAnnotation includes
#include "vtkDMMLAnnotationPointDisplayNode.h"

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

const char *vtkDMMLAnnotationPointDisplayNode::GlyphTypesNames[GlyphMax+2] =
{
  "GlyphMin",
  "Vertex2D",
  "Dash2D",
  "Cross2D",
  "ThickCross2D",
  "Triangle2D",
  "Square2D",
  "Circle2D",
  "Diamond2D",
  "Arrow2D",
  "ThickArrow2D",
  "HookedArrow2D",
  "StarBurst2D",
  "Sphere3D"
};
//  "Diamond3D"

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLAnnotationPointDisplayNode);

//----------------------------------------------------------------------------
vtkDMMLAnnotationPointDisplayNode::vtkDMMLAnnotationPointDisplayNode()
{
  this->GlyphType = vtkDMMLAnnotationPointDisplayNode::Sphere3D;
  this->GlyphScale = 5.0;
  this->SliceProjection = (vtkDMMLAnnotationDisplayNode::ProjectionOff |
                           vtkDMMLAnnotationPointDisplayNode::ProjectionUseFiducialColor |
                           vtkDMMLAnnotationPointDisplayNode::ProjectionOutlinedBehindSlicePlane);
}

//----------------------------------------------------------------------------
void vtkDMMLAnnotationPointDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  of << " glyphScale=\"" << this->GlyphScale << "\"";
  of << " glyphType=\"" << this->GlyphType << "\"";
  of << " sliceProjection=\"" << this->SliceProjection << "\"";

  of << " projectedColor=\"" << this->ProjectedColor[0] << " "
     << this->ProjectedColor[1] << " "
     << this->ProjectedColor[2] << "\"";

  of << " projectedOpacity=\"" << this->ProjectedOpacity << "\"";
 }

//----------------------------------------------------------------------------
void vtkDMMLAnnotationPointDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);

      if (!strcmp(attName, "glyphType"))
        {
        std::stringstream ss;
        ss << attValue;
        ss >> this->GlyphType;
        }
      else if (!strcmp(attName, "glyphScale"))
        {
        std::stringstream ss;
        ss << attValue;
        ss >> this->GlyphScale;
        }
      else if (!strcmp(attName, "sliceProjection"))
        {
        std::stringstream ss;
        ss << attValue;
        ss >> this->SliceProjection;
        }
      else if (!strcmp(attName, "projectedColor"))
        {
        std::stringstream ss;
        ss << attValue;
        ss >> this->ProjectedColor[0];
        ss >> this->ProjectedColor[1];
        ss >> this->ProjectedColor[2];
        }
      else if (!strcmp(attName, "projectedOpacity"))
        {
        std::stringstream ss;
        ss << attValue;
        ss >> this->ProjectedOpacity;
        }
    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkDMMLAnnotationPointDisplayNode::Copy(vtkDMMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkDMMLAnnotationPointDisplayNode *node = (vtkDMMLAnnotationPointDisplayNode *) anode;
  this->SetGlyphType(node->GlyphType);
  this->SetGlyphScale(node->GlyphScale);
  this->SetSliceProjection(node->SliceProjection);
  this->SetProjectedColor(node->GetProjectedColor());
  this->SetProjectedOpacity(node->GetProjectedOpacity());

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
const char* vtkDMMLAnnotationPointDisplayNode::GetGlyphTypeAsString()
{
  return this->GetGlyphTypeAsString(this->GlyphType);
}

//----------------------------------------------------------------------------
const char* vtkDMMLAnnotationPointDisplayNode::GetGlyphTypeAsString(int glyphType)
{
  if (glyphType < GlyphMin || (glyphType > GlyphMax))
    {
      return "UNKNOWN";
    }
    return this->GlyphTypesNames[glyphType];
}

//----------------------------------------------------------------------------
void vtkDMMLAnnotationPointDisplayNode::SetGlyphTypeFromString(const char *glyphString)
{
  for (int ID = GlyphMin; ID <= GlyphMax; ID++)
    {
      if (!strcmp(glyphString,GlyphTypesNames[ID]))
      {
      this->SetGlyphType(ID);
      return;
      }
    }
  vtkErrorMacro("Invalid glyph type string: " << glyphString);
}

//----------------------------------------------------------------------------
void vtkDMMLAnnotationPointDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << indent << "Glyph scale: (";
  os << this->GlyphScale << ")\n";
  os << indent << "Glyph type: ";
  os << this->GetGlyphTypeAsString() << " (" << this->GlyphType << ")\n";
  os << indent << "Slice projection: (";
  os << this->SliceProjection << ")\n";
  os << indent << "Projected Color: (";
  os << this->ProjectedColor[0] << ","
     << this->ProjectedColor[1] << ","
     << this->ProjectedColor[2] << ")" << "\n";
  os << indent << "Projected Opacity: " << this->ProjectedOpacity << "\n";
}

//---------------------------------------------------------------------------
void vtkDMMLAnnotationPointDisplayNode::ProcessDMMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessDMMLEvents(caller, event, callData);
  return;
}

//-----------------------------------------------------------
void vtkDMMLAnnotationPointDisplayNode::UpdateScene(vtkDMMLScene *scene)
{
   Superclass::UpdateScene(scene);
}

//---------------------------------------------------------------------------
int  vtkDMMLAnnotationPointDisplayNode::GlyphTypeIs3D(int glyphType)
{
  if (glyphType >= vtkDMMLAnnotationPointDisplayNode::Sphere3D)
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

//---------------------------------------------------------------------------
void  vtkDMMLAnnotationPointDisplayNode::SetGlyphType(int type)
{
  if (this->GlyphType == type)
    {
    return;
    }
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting GlyphType to " << type);
  this->GlyphType = type;

  this->Modified();
}

//---------------------------------------------------------------------------
void vtkDMMLAnnotationPointDisplayNode::SetGlyphScale(double scale)
{
  if (this->GlyphScale == scale)
    {
    return;
    }
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting GlyphScale to " << scale);
  this->GlyphScale = scale;
  this->Modified();
}

//----------------------------------------------------------------------------
// Create a backup of this node and store it with the node.
void vtkDMMLAnnotationPointDisplayNode::CreateBackup()
{

  vtkDMMLAnnotationPointDisplayNode * backupNode = vtkDMMLAnnotationPointDisplayNode::New();

  int oldMode = backupNode->GetDisableModifiedEvent();
  backupNode->DisableModifiedEventOn();
  backupNode->Copy(this);
  backupNode->SetDisableModifiedEvent(oldMode);

  this->m_Backup = backupNode;

}

//----------------------------------------------------------------------------
// Restores the backup of this node.
void vtkDMMLAnnotationPointDisplayNode::RestoreBackup()
{

  if (this->m_Backup)
    {
    DMMLNodeModifyBlocker blocker(this);
    this->Copy(this->m_Backup);
    }
  else
    {
    vtkErrorMacro("RestoreBackup - could not get the attached backup");
    }

}





