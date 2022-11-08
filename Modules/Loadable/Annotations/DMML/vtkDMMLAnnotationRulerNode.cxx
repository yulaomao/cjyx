// DMML includes
#include "vtkDMMLAnnotationRulerNode.h"
#include "vtkDMMLAnnotationTextDisplayNode.h"
#include "vtkDMMLAnnotationPointDisplayNode.h"
#include "vtkDMMLAnnotationLineDisplayNode.h"
#include "vtkDMMLAnnotationRulerStorageNode.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkAbstractTransform.h>
#include <vtkObjectFactory.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLAnnotationRulerNode);


//----------------------------------------------------------------------------
vtkDMMLAnnotationRulerNode::vtkDMMLAnnotationRulerNode()
{
  this->HideFromEditors = false;
  this->DistanceAnnotationFormat = nullptr;
  this->SetDistanceAnnotationFormat("%.0f mm");
  this->ModelID1 = nullptr;
  this->ModelID2 = nullptr;
}

//----------------------------------------------------------------------------
vtkDMMLAnnotationRulerNode::~vtkDMMLAnnotationRulerNode()
{
  vtkDebugMacro("Destructing...." << (this->GetID() != nullptr ? this->GetID() : "null id"));
  if (this->DistanceAnnotationFormat)
    {
      delete [] this->DistanceAnnotationFormat;
      this->DistanceAnnotationFormat = nullptr;
    }
  if (this->ModelID1)
    {
    delete [] this->ModelID1;
    this->ModelID1 = nullptr;
    }
  if (this->ModelID2)
    {
    delete [] this->ModelID2;
    this->ModelID2 = nullptr;
    }
}

//----------------------------------------------------------------------------
void vtkDMMLAnnotationRulerNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  of << " rulerDistanceAnnotationFormat=\"";
  if (this->DistanceAnnotationFormat)
    {
    of << this->DistanceAnnotationFormat << "\"";
    }
  else
    {
    of << "\"";
    }

  if (this->ModelID1)
    {
    of << " modelID1=\"" << this->ModelID1 << "\"";
    }
  if (this->ModelID2)
    {
    of << " modelID2=\"" << this->ModelID2 << "\"";
    }
  of << " distanceMeasurement=\"" << this->GetDistanceMeasurement() << "\"";
}


//----------------------------------------------------------------------------
void vtkDMMLAnnotationRulerNode::ReadXMLAttributes(const char** atts)
{
  // cout << "vtkDMMLAnnotationRulerNode::ReadXMLAttributes start"<< endl;

  int disabledModify = this->StartModify();

  this->ResetAnnotations();

  Superclass::ReadXMLAttributes(atts);


  while (*atts != nullptr)
    {
    const char* attName = *(atts++);
    std::string attValue(*(atts++));


    if (!strcmp(attName, "rulerDistanceAnnotationFormat"))
      {
      this->SetDistanceAnnotationFormat(attValue.c_str());
      }

    else if (!strcmp(attName, "modelID1"))
      {
      this->SetModelID1(attValue.c_str());
      }
    else if (!strcmp(attName, "modelID2"))
      {
      this->SetModelID2(attValue.c_str());
      }
    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLAnnotationRulerNode::Copy(vtkDMMLNode *anode)
{

  Superclass::Copy(anode);
  //vtkDMMLAnnotationRulerNode *node = (vtkDMMLAnnotationRulerNode *) anode;

  //this->SetPosition1(node->GetPosition1());
  //this->SetPosition2(node->GetPosition2());
  //this->SetDistanceAnnotationFormat(node->GetDistanceAnnotationFormat());
  //this->SetModelID1(node->GetModelID1());
  //this->SetModelID2(node->GetModelID2());
}

//-----------------------------------------------------------
void vtkDMMLAnnotationRulerNode::UpdateScene(vtkDMMLScene *scene)
{
  Superclass::UpdateScene(scene);

  // Nothing to do at this point  bc vtkDMMLAnnotationDisplayNode is subclass of vtkDMMLModelDisplayNode
  // => will be taken care of by vtkDMMLModelDisplayNode

}

//---------------------------------------------------------------------------
void vtkDMMLAnnotationRulerNode::ProcessDMMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessDMMLEvents(caller, event, callData);

  // Not necessary bc vtkDMMLAnnotationDisplayNode is subclass of vtkDMMLModelDisplayNode
  // => will be taken care of  in vtkDMMLModelNode
}

//----------------------------------------------------------------------------
void vtkDMMLAnnotationRulerNode::PrintAnnotationInfo(ostream& os, vtkIndent indent, int titleFlag)
{
  //cout << "vtkDMMLAnnotationRulerNode::PrintAnnotationInfo" << endl;
  if (titleFlag)
    {

      os <<indent << "vtkDMMLAnnotationRulerNode: Annotation Summary";
      if (this->GetName())
    {
      os << " of " << this->GetName();
    }
      os << endl;
    }

  Superclass::PrintAnnotationInfo(os, indent, 0);

  os << indent << "rulerDistanceAnnotationFormat: ";
  if (this->DistanceAnnotationFormat)
    {
      os  << this->DistanceAnnotationFormat << "\n";
    }
  else
    {
      os  << "(None)" << "\n";
    }

  os << indent << "Model 1: " << (this->ModelID1 ? this->ModelID1 : "none") << "\n";
  os << indent << "Model 2: " << (this->ModelID2 ? this->ModelID2 : "none") << "\n";

}

//---------------------------------------------------------------------------
int vtkDMMLAnnotationRulerNode::AddControlPoint(double newControl[3],int selectedFlag, int visibleFlag)
{
  if (this->GetNumberOfControlPoints() > 1) {
    vtkErrorMacro("AnnotationRuler: "<< this->GetName() << " cannot have more than 3 control points !");
    return -1;
  }
  return Superclass::AddControlPoint(newControl,selectedFlag,visibleFlag);
}

//---------------------------------------------------------------------------
double vtkDMMLAnnotationRulerNode::GetDistanceAnnotationScale()
{
  vtkDMMLAnnotationTextDisplayNode *node = this->GetAnnotationTextDisplayNode();
  if (!node)
    {
      return 0;
    }
  return node->GetTextScale();
}

//---------------------------------------------------------------------------
void vtkDMMLAnnotationRulerNode::SetDistanceAnnotationScale(double init)
{
  vtkDMMLAnnotationTextDisplayNode *node = this->GetAnnotationTextDisplayNode();

  if (!node)
    {
      vtkErrorMacro("AnnotationRuler: "<< this->GetName() << " cannot get AnnotationTextDisplayNode");
      return;
    }
  node->SetTextScale(init);
}

//---------------------------------------------------------------------------
void vtkDMMLAnnotationRulerNode::SetDistanceAnnotationVisibility(int flag)
{
  this->SetAnnotationAttribute(0,vtkDMMLAnnotationNode::TEXT_VISIBLE,flag);

}

//---------------------------------------------------------------------------
int vtkDMMLAnnotationRulerNode::GetDistanceAnnotationVisibility()
{
  return this->GetAnnotationAttribute(0, vtkDMMLAnnotationNode::TEXT_VISIBLE);
}

//---------------------------------------------------------------------------
int vtkDMMLAnnotationRulerNode::SetRuler(vtkIdType line1Id, int sel, int vis)
{
  vtkIdType line1IDPoints[2];
  this->GetEndPointsId(line1Id,line1IDPoints);

  //Change this later
  if (line1IDPoints[0]!= 0 || line1IDPoints[1] != 1)
    {
      vtkErrorMacro("Not valid line definition!");
      return -1;
    }
  this->SetSelected(sel);
  this->SetDisplayVisibility(vis);

  return 1;
}

//---------------------------------------------------------------------------
int vtkDMMLAnnotationRulerNode::SetControlPoint(int id, double newControl[3])
{
  if (id < 0 || id > 1) {
    return 0;
  }

  int flag = Superclass::SetControlPoint(id, newControl,1,1);
  if (!flag)
    {
      return 0;
    }
  if (this->GetNumberOfControlPoints() < 2)
    {
      return 1;
    }

  this->AddLine(0,1,1,1);
  return 1;
}

//---------------------------------------------------------------------------
double* vtkDMMLAnnotationRulerNode::GetPointColor()
{
  vtkDMMLAnnotationPointDisplayNode *node = this->GetAnnotationPointDisplayNode();
  if (!node)
    {
      return nullptr;
    }
  return node->GetSelectedColor();
}

//---------------------------------------------------------------------------
void vtkDMMLAnnotationRulerNode::SetPointColor(double initColor[3])
{
  vtkDMMLAnnotationPointDisplayNode *node = this->GetAnnotationPointDisplayNode();
  if (!node)
    {
      vtkErrorMacro("AnnotationRuler: "<< this->GetName() << " cannot get AnnotationPointDisplayNode");
      return;
    }
  node->SetSelectedColor(initColor);
}

//---------------------------------------------------------------------------
double* vtkDMMLAnnotationRulerNode::GetDistanceAnnotationTextColor()
{
  vtkDMMLAnnotationTextDisplayNode *node = this->GetAnnotationTextDisplayNode();
  if (!node)
    {
      return nullptr;
    }
  return node->GetSelectedColor();
}

//---------------------------------------------------------------------------
void vtkDMMLAnnotationRulerNode::SetDistanceAnnotationTextColor(double initColor[3])
{
  vtkDMMLAnnotationTextDisplayNode *node = this->GetAnnotationTextDisplayNode();
  if (!node)
    {
      vtkErrorMacro("AnnotationRuler: "<< this->GetName() << " cannot get AnnotationPointDisplayNode");
      return;
    }
  node->SetSelectedColor(initColor);
}

//---------------------------------------------------------------------------
double* vtkDMMLAnnotationRulerNode::GetLineColor()
{
  vtkDMMLAnnotationLineDisplayNode *node = this->GetAnnotationLineDisplayNode();
  if (!node)
    {
      return nullptr;
    }
  return node->GetSelectedColor();
}

//---------------------------------------------------------------------------
void vtkDMMLAnnotationRulerNode::SetLineColor(double initColor[3])
{
  vtkDMMLAnnotationLineDisplayNode *node = this->GetAnnotationLineDisplayNode();
  if (!node)
    {
      vtkErrorMacro("AnnotationRuler: "<< this->GetName() << " cannot get AnnotationPointDisplayNode");
      return;
    }
  node->SetSelectedColor(initColor);
}

//---------------------------------------------------------------------------
void vtkDMMLAnnotationRulerNode::ApplyTransform(vtkAbstractTransform* transform)
{
  double xyzIn[3];
  double xyzOut[3];
  double *p;

  // first point
  p = this->GetPosition1();
  if (p)
    {
    xyzIn[0] = p[0];
    xyzIn[1] = p[1];
    xyzIn[2] = p[2];

    transform->TransformPoint(xyzIn,xyzOut);
    this->SetPosition1(xyzOut);
    }

  // second point
  p = this->GetPosition2();
  if (p)
    {
    xyzIn[0] = p[0];
    xyzIn[1] = p[1];
    xyzIn[2] = p[2];

    transform->TransformPoint(xyzIn,xyzOut);
    this->SetPosition2(xyzOut);
    }
}

//---------------------------------------------------------------------------
vtkDMMLStorageNode* vtkDMMLAnnotationRulerNode::CreateDefaultStorageNode()
{
  vtkDMMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkDMMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkDMMLAnnotationRulerStorageNode"));
}

//---------------------------------------------------------------------------
double vtkDMMLAnnotationRulerNode::GetDistanceMeasurement()
{
  double distanceMeasurement = 0.0;

  double p1[4]={0,0,0,1};
  double p2[4]={0,0,0,1};
  this->GetPositionWorldCoordinates1(p1);
  this->GetPositionWorldCoordinates2(p2);

  distanceMeasurement = sqrt(vtkMath::Distance2BetweenPoints(p1,p2));

  return distanceMeasurement;
}

//---------------------------------------------------------------------------
void vtkDMMLAnnotationRulerNode::GetPosition1(double position[3])
{
  double * tmp = this->GetControlPointCoordinates(0);
  if (!tmp)
    {
    vtkErrorMacro("AnnotationRuler - Failed to get first control point");
    return;
    }
  for(int i=0; i < 3; ++i)
    {
    position[i] = tmp[i];
    }
}

//---------------------------------------------------------------------------
void vtkDMMLAnnotationRulerNode::GetPosition2(double position[3])
{
  double * tmp = this->GetControlPointCoordinates(1);
  if (!tmp)
    {
    vtkErrorMacro("AnnotationRuler - Failed to get second control point");
    return;
    }
  for(int i=0; i < 3; ++i){ position[i] = tmp[i]; }
}
