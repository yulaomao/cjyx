// DMML includes
#include "vtkDMMLAnnotationSnapshotNode.h"
#include "vtkDMMLAnnotationSnapshotStorageNode.h"
#include "vtkDMMLScene.h"

// VTKsys includes
#include <vtksys/SystemTools.hxx>

// VTK includes
#include <vtkImageData.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//------------------------------------------------------------------------------
vtkDMMLAnnotationSnapshotNode::vtkDMMLAnnotationSnapshotNode()
{
  this->ScreenShot = nullptr;
  this->ScaleFactor = 1.0;
}

//------------------------------------------------------------------------------
vtkDMMLAnnotationSnapshotNode::~vtkDMMLAnnotationSnapshotNode()
{
  if (this->ScreenShot)
    {
    this->ScreenShot->Delete();
    this->ScreenShot = nullptr;
    }
}

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLAnnotationSnapshotNode);


//----------------------------------------------------------------------------
void vtkDMMLAnnotationSnapshotNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  of << " screenshotType=\"" << this->GetScreenShotType() << "\"";

  vtkStdString description = this->GetSnapshotDescription();
  vtksys::SystemTools::ReplaceString(description,"\n","[br]");

  of << " snapshotDescription=\"" << description << "\"";

  of << " scaleFactor=\"" << this->GetScaleFactor() << "\"";
}


//----------------------------------------------------------------------------
void vtkDMMLAnnotationSnapshotNode::ReadXMLAttributes(const char** atts)
{

  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "screenshotType"))
      {
      std::stringstream ss;
      ss << attValue;
      int screenshotType;
      ss >> screenshotType;
      this->SetScreenShotType(screenshotType);
      }
    else if (!strcmp(attName, "scaleFactor"))
      {
      std::stringstream ss;
      ss << attValue;
      double scaleFactor;
      ss >> scaleFactor;
      this->SetScaleFactor(scaleFactor);
      }
    else if(!strcmp(attName, "snapshotDescription"))
      {
      std::stringstream ss;
      ss << attValue;
      vtkStdString sceneViewDescription;
      ss >> sceneViewDescription;

      vtksys::SystemTools::ReplaceString(sceneViewDescription,"[br]","\n");

      this->SetSnapshotDescription(sceneViewDescription);
      }
    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
vtkDMMLStorageNode* vtkDMMLAnnotationSnapshotNode::CreateDefaultStorageNode()
{
  vtkDMMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkDMMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkDMMLAnnotationSnapshotStorageNode"));
}

//----------------------------------------------------------------------------
void vtkDMMLAnnotationSnapshotNode::SetSnapshotDescription(const vtkStdString& newDescription)
{
  if (this->SnapshotDescription == newDescription)
    {
    return;
    }
  this->SnapshotDescription = newDescription;
  this->StorableModifiedTime.Modified();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkDMMLAnnotationSnapshotNode::SetScreenShot(vtkImageData* newScreenShot)
{
  this->StorableModifiedTime.Modified();
  vtkSetObjectBodyMacro(ScreenShot, vtkImageData, newScreenShot);
}

//----------------------------------------------------------------------------
void vtkDMMLAnnotationSnapshotNode::SetScreenShotType(int newScreenShotType)
{
  if (this->ScreenShotType == newScreenShotType)
    {
    return;
    }
  this->ScreenShotType = newScreenShotType;
  this->StorableModifiedTime.Modified();
  this->Modified();
}
