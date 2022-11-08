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
#include "vtkDMMLTestSliceViewDisplayableManager.h"

// DMML includes
#include <vtkDMMLCameraNode.h>

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLTestSliceViewDisplayableManager );

//---------------------------------------------------------------------------
int vtkDMMLTestSliceViewDisplayableManager::NodeAddedCount = 0;

//---------------------------------------------------------------------------
class vtkDMMLTestSliceViewDisplayableManager::vtkInternal
{
public:
  vtkInternal(vtkDMMLTestSliceViewDisplayableManager * external);
  ~vtkInternal();

  vtkDMMLTestSliceViewDisplayableManager*             External;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkDMMLTestSliceViewDisplayableManager::vtkInternal::vtkInternal(vtkDMMLTestSliceViewDisplayableManager * external)
{
  this->External = external;
}

//---------------------------------------------------------------------------
vtkDMMLTestSliceViewDisplayableManager::vtkInternal::~vtkInternal() = default;

//---------------------------------------------------------------------------
// vtkDMMLTestSliceViewDisplayableManager methods

//---------------------------------------------------------------------------
vtkDMMLTestSliceViewDisplayableManager::vtkDMMLTestSliceViewDisplayableManager()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkDMMLTestSliceViewDisplayableManager::~vtkDMMLTestSliceViewDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkDMMLTestSliceViewDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkDMMLTestSliceViewDisplayableManager::AdditionnalInitializeStep()
{
}

//---------------------------------------------------------------------------
void vtkDMMLTestSliceViewDisplayableManager::Create()
{
  assert(this->GetRenderer());
  assert(this->GetDMMLSliceNode());
}

//---------------------------------------------------------------------------
void vtkDMMLTestSliceViewDisplayableManager::OnDMMLSceneNodeAdded(vtkDMMLNode* node)
{
  vtkDMMLCameraNode * cameraNode = vtkDMMLCameraNode::SafeDownCast(node);
  if (!cameraNode)
    {
    return;
    }
  vtkDMMLTestSliceViewDisplayableManager::NodeAddedCount++;
  //std::cout << "vtkDMMLTestSliceViewDisplayableManager - NodeAdded - "
  //          << (node ? node->GetName() : "None")<< std::endl;
}
