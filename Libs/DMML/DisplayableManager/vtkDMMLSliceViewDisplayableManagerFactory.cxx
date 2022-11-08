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
#include "vtkDMMLSliceViewDisplayableManagerFactory.h"

// VTK includes
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// vtkDMMLSliceViewDisplayableManagerFactory methods

//----------------------------------------------------------------------------
// Up the reference count so it behaves like New
vtkDMMLSliceViewDisplayableManagerFactory* vtkDMMLSliceViewDisplayableManagerFactory::New()
{
  vtkDMMLSliceViewDisplayableManagerFactory* instance = Self::GetInstance();
  instance->Register(nullptr);
  return instance;
}

//----------------------------------------------------------------------------
vtkDMMLSliceViewDisplayableManagerFactory* vtkDMMLSliceViewDisplayableManagerFactory::GetInstance()
{
  if(!Self::Instance)
    {
    // Try the factory first
    Self::Instance = (vtkDMMLSliceViewDisplayableManagerFactory*)
                     vtkObjectFactory::CreateInstance("vtkDMMLSliceViewDisplayableManagerFactory");

    // if the factory did not provide one, then create it here
    if(!Self::Instance)
      {
      Self::Instance = new vtkDMMLSliceViewDisplayableManagerFactory;
#ifdef VTK_HAS_INITIALIZE_OBJECT_BASE
      Self::Instance->InitializeObjectBase();
#endif
      }
    }
  // return the instance
  return Self::Instance;
}

//----------------------------------------------------------------------------
vtkDMMLSliceViewDisplayableManagerFactory::
    vtkDMMLSliceViewDisplayableManagerFactory():Superclass()
{
}

//----------------------------------------------------------------------------
vtkDMMLSliceViewDisplayableManagerFactory::~vtkDMMLSliceViewDisplayableManagerFactory() = default;

//----------------------------------------------------------------------------
void vtkDMMLSliceViewDisplayableManagerFactory::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

VTK_SINGLETON_CXX(vtkDMMLSliceViewDisplayableManagerFactory);

