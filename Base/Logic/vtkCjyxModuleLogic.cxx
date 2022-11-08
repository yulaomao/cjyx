/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

// CjyxLogic includes
#include "vtkCjyxModuleLogic.h"

// VTK includes
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkCjyxModuleLogic);

//----------------------------------------------------------------------------
vtkCjyxModuleLogic::vtkCjyxModuleLogic() = default;

//----------------------------------------------------------------------------
vtkCjyxModuleLogic::~vtkCjyxModuleLogic()
{
  this->SetDMMLApplicationLogic(nullptr);
}

//----------------------------------------------------------------------------
vtkCjyxApplicationLogic* vtkCjyxModuleLogic::GetApplicationLogic()
{
  return vtkCjyxApplicationLogic::SafeDownCast(this->GetDMMLApplicationLogic());
}

//----------------------------------------------------------------------------
vtkDMMLAbstractLogic* vtkCjyxModuleLogic::GetModuleLogic(const char* moduleName)
{
  vtkDMMLApplicationLogic* appLogic =
    vtkDMMLApplicationLogic::SafeDownCast(this->GetDMMLApplicationLogic());
  if (!appLogic)
    {
    return nullptr;
    }
  return appLogic->GetModuleLogic(moduleName);
}

//----------------------------------------------------------------------------
std::string vtkCjyxModuleLogic::GetModuleShareDirectory()const
{
  return this->ModuleShareDirectory;
}

//----------------------------------------------------------------------------
void vtkCjyxModuleLogic::SetModuleShareDirectory(const std::string& shareDirectory)
{
  if (this->ModuleShareDirectory != shareDirectory)
    {
    this->ModuleShareDirectory = shareDirectory;
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkCjyxModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
