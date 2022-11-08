
// DMMLAnnotation includes
#include "vtkDMMLAnnotationTextDisplayNode.h"

// VTK includes
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLAnnotationDisplayNode);

//----------------------------------------------------------------------------
vtkDMMLAnnotationDisplayNode::vtkDMMLAnnotationDisplayNode()
{
  this->SetVisibility(1);
  this->SetVectorVisibility(0);
  this->SetScalarVisibility(0);
  this->SetTensorVisibility(0);

  this->Color[0] = 0.7373;
  this->Color[1] = 0.2549;
  this->Color[2] = 0.1098;

  this->SelectedColor[0] = 0.2667;
  this->SelectedColor[1] = 0.6745;
  this->SelectedColor[2] = 0.3922;

  this->SetName("");
  this->Opacity = 1.0;
  this->Ambient = 0;
  this->Diffuse = 1.0;
  this->Specular = 0;
  this->Power = 1;

  this->SliceProjection = vtkDMMLAnnotationDisplayNode::ProjectionOff;
  this->ProjectedColor[0] = 1.0;
  this->ProjectedColor[1] = 1.0;
  this->ProjectedColor[2] = 1.0;
  this->ProjectedOpacity = 1.0;

  this->m_Backup = nullptr;

}

//----------------------------------------------------------------------------
vtkDMMLAnnotationDisplayNode::~vtkDMMLAnnotationDisplayNode()
{

  if (this->m_Backup)
    {
    this->m_Backup->Delete();
    this->m_Backup = nullptr;
    }

}


//----------------------------------------------------------------------------
void vtkDMMLAnnotationDisplayNode::WriteXML(ostream& of, int nIndent)
{

  Superclass::WriteXML(of, nIndent);
}



//----------------------------------------------------------------------------
void vtkDMMLAnnotationDisplayNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkDMMLAnnotationDisplayNode::Copy(vtkDMMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
void vtkDMMLAnnotationDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//---------------------------------------------------------------------------
void vtkDMMLAnnotationDisplayNode::ProcessDMMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessDMMLEvents(caller, event, callData);
  return;
}

//-----------------------------------------------------------
void vtkDMMLAnnotationDisplayNode::UpdateScene(vtkDMMLScene *scene)
{
   Superclass::UpdateScene(scene);
}

//----------------------------------------------------------------------------
// Clears the backup of this node.
void vtkDMMLAnnotationDisplayNode::ClearBackup()
{
  if (this->m_Backup)
    {
    this->m_Backup->Delete();
    this->m_Backup = nullptr;
    }
}

//----------------------------------------------------------------------------
// Returns the backup of this node.
vtkDMMLAnnotationDisplayNode * vtkDMMLAnnotationDisplayNode::GetBackup()
{

  return this->m_Backup;

}





