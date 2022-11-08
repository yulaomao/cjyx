/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// DMMLDisplayableManager includes
#include "vtkDMMLDisplayableManagerFactory.h"
#include "vtkDMMLDisplayableManagerGroup.h"
#ifdef DMMLDisplayableManager_USE_PYTHON
#include "vtkDMMLScriptedDisplayableManager.h"
#endif

// DMMLLogic includes
#include "vtkDMMLApplicationLogic.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// STD includes
#include <algorithm>
#include <string>
#include <vector>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLDisplayableManagerFactory);

//----------------------------------------------------------------------------
class vtkDMMLDisplayableManagerFactory::vtkInternal
{
public:
  vtkInternal();

  // Collection of Displayable Manager classNames
  std::vector<std::string> DisplayableManagerClassNames;

  // .. and its associated convenient typedef
  typedef std::vector<std::string>::iterator DisplayableManagerClassNamesIt;

  // The application logic (can be a vtkCjyxApplicationLogic
  vtkSmartPointer<vtkDMMLApplicationLogic> ApplicationLogic;
};

//----------------------------------------------------------------------------
// vtkInternal methods

//----------------------------------------------------------------------------
vtkDMMLDisplayableManagerFactory::vtkInternal::vtkInternal() = default;

//----------------------------------------------------------------------------
// vtkDMMLDisplayableManagerFactory methods

//----------------------------------------------------------------------------
vtkDMMLDisplayableManagerFactory::vtkDMMLDisplayableManagerFactory()
{
  this->Internal = new vtkInternal;
}

//----------------------------------------------------------------------------
vtkDMMLDisplayableManagerFactory::~vtkDMMLDisplayableManagerFactory()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableManagerFactory::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
bool vtkDMMLDisplayableManagerFactory::IsDisplayableManagerRegistered(const char* vtkClassName)
{
  // Sanity checks
  if (!vtkClassName)
    {
    vtkWarningMacro(<<"IsDisplayableManagerRegistered - vtkClassName is NULL");
    return false;
    }

  // Check if the DisplayableManager has already been registered
  vtkInternal::DisplayableManagerClassNamesIt it = std::find(
      this->Internal->DisplayableManagerClassNames.begin(),
      this->Internal->DisplayableManagerClassNames.end(),
      vtkClassName);

  if ( it == this->Internal->DisplayableManagerClassNames.end())
    {
    return false;
    }
  else
    {
    return true;
    }
}

//----------------------------------------------------------------------------
bool vtkDMMLDisplayableManagerFactory::RegisterDisplayableManager(const char* vtkClassOrScriptName)
{
  // Sanity checks
  if (!vtkClassOrScriptName)
    {
    vtkWarningMacro(<<"RegisterDisplayableManager - vtkClassOrScriptName is NULL");
    return false;
    }

  // Check if the DisplayableManager has already been registered
  vtkInternal::DisplayableManagerClassNamesIt it = std::find(
      this->Internal->DisplayableManagerClassNames.begin(),
      this->Internal->DisplayableManagerClassNames.end(),
      vtkClassOrScriptName);

  if ( it != this->Internal->DisplayableManagerClassNames.end())
    {
    vtkWarningMacro(<<"RegisterDisplayableManager - " << vtkClassOrScriptName << " already registered");
    return false;
    }

  if (!vtkDMMLDisplayableManagerGroup::IsADisplayableManager(vtkClassOrScriptName))
    {
    vtkWarningMacro(<<"RegisterDisplayableManager - " << vtkClassOrScriptName
                    << " is not a displayable manager. Failed to register");
    return false;
    }
  // Register it
  this->Internal->DisplayableManagerClassNames.emplace_back(vtkClassOrScriptName);

  this->InvokeEvent(Self::DisplayableManagerFactoryRegisteredEvent,
                    const_cast<char*>(vtkClassOrScriptName));

  return true;
}

//----------------------------------------------------------------------------
bool vtkDMMLDisplayableManagerFactory::UnRegisterDisplayableManager(const char* vtkClassOrScriptName)
{
  // Sanity checks
  if (!vtkClassOrScriptName)
    {
    vtkWarningMacro(<<"UnRegisterDisplayableManager - vtkClassOrScriptName is NULL");
    return false;
    }

  // Check if the DisplayableManager is registered
  vtkInternal::DisplayableManagerClassNamesIt it = std::find(
      this->Internal->DisplayableManagerClassNames.begin(),
      this->Internal->DisplayableManagerClassNames.end(),
      vtkClassOrScriptName);

  if ( it == this->Internal->DisplayableManagerClassNames.end())
    {
    vtkWarningMacro(<<"UnRegisterDisplayableManager - " << vtkClassOrScriptName << " is NOT registered");
    return false;
    }

  this->Internal->DisplayableManagerClassNames.erase(it);

  this->InvokeEvent(Self::DisplayableManagerFactoryUnRegisteredEvent,
                    const_cast<char*>(vtkClassOrScriptName));

  return true;
}

//----------------------------------------------------------------------------
int vtkDMMLDisplayableManagerFactory::GetRegisteredDisplayableManagerCount()
{
  return static_cast<int>(this->Internal->DisplayableManagerClassNames.size());
}

//----------------------------------------------------------------------------
std::string vtkDMMLDisplayableManagerFactory::GetRegisteredDisplayableManagerName(int n)
{
  if (n < 0 || n >= this->GetRegisteredDisplayableManagerCount())
    {
    vtkWarningMacro(<<"GetNthRegisteredDisplayableManagerName - "
                    "n " << n << " is invalid. A valid value for n should be >= 0 and < " <<
                    this->GetRegisteredDisplayableManagerCount());
    return std::string();
    }
  return this->Internal->DisplayableManagerClassNames.at(n);
}

//----------------------------------------------------------------------------
vtkDMMLDisplayableManagerGroup* vtkDMMLDisplayableManagerFactory::InstantiateDisplayableManagers(
    vtkRenderer * newRenderer)
{
  // Sanity checks
  if (!newRenderer)
    {
    vtkWarningMacro(<<"InstanciateDisplayableManagers - newRenderer is NULL");
    return nullptr;
    }

  vtkDMMLDisplayableManagerGroup * displayableManagerGroup = vtkDMMLDisplayableManagerGroup::New();
  displayableManagerGroup->Initialize(this, newRenderer);
  return displayableManagerGroup;
}

//----------------------------------------------------------------------------
vtkDMMLApplicationLogic* vtkDMMLDisplayableManagerFactory
::GetDMMLApplicationLogic()const
{
  return this->Internal->ApplicationLogic.GetPointer();
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayableManagerFactory
::SetDMMLApplicationLogic(vtkDMMLApplicationLogic* logic)
{
  this->Internal->ApplicationLogic = logic;
}
