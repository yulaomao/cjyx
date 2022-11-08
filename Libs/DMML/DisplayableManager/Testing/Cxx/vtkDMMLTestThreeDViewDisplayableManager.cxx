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
#include "vtkDMMLTestThreeDViewDisplayableManager.h"

// DMML includes
#include <vtkDMMLCameraNode.h>

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLTestThreeDViewDisplayableManager );

//---------------------------------------------------------------------------
int vtkDMMLTestThreeDViewDisplayableManager::NodeAddedCount = 0;

//---------------------------------------------------------------------------
class vtkDMMLTestThreeDViewDisplayableManager::vtkInternal
{
public:
  vtkInternal(vtkDMMLTestThreeDViewDisplayableManager * external);
  ~vtkInternal();

  vtkDMMLTestThreeDViewDisplayableManager*             External;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkDMMLTestThreeDViewDisplayableManager::vtkInternal::vtkInternal(vtkDMMLTestThreeDViewDisplayableManager * external)
{
  this->External = external;
}

//---------------------------------------------------------------------------
vtkDMMLTestThreeDViewDisplayableManager::vtkInternal::~vtkInternal() = default;

//---------------------------------------------------------------------------
// vtkDMMLTestThreeDViewDisplayableManager methods

//---------------------------------------------------------------------------
vtkDMMLTestThreeDViewDisplayableManager::vtkDMMLTestThreeDViewDisplayableManager()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkDMMLTestThreeDViewDisplayableManager::~vtkDMMLTestThreeDViewDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkDMMLTestThreeDViewDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkDMMLTestThreeDViewDisplayableManager::AdditionnalInitializeStep()
{
}

//---------------------------------------------------------------------------
void vtkDMMLTestThreeDViewDisplayableManager::Create()
{
  assert(this->GetRenderer());
  assert(this->GetDMMLViewNode());
}

//---------------------------------------------------------------------------
void vtkDMMLTestThreeDViewDisplayableManager::OnDMMLSceneNodeAdded(vtkDMMLNode* node)
{
  vtkDMMLCameraNode * cameraNode = vtkDMMLCameraNode::SafeDownCast(node);
  if (!cameraNode)
    {
    return;
    }
  vtkDMMLTestThreeDViewDisplayableManager::NodeAddedCount++;
  //std::cout << "vtkDMMLTestThreeDViewDisplayableManager - NodeAdded - "
  //          << (node ? node->GetName() : "None")<< std::endl;
}

