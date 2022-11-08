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
#include "vtkDMMLTestCustomDisplayableManager.h"

// DMML includes
#include <vtkDMMLCameraNode.h>
#include <vtkDMMLViewNode.h>
#include <vtkDMMLSliceNode.h>

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLTestCustomDisplayableManager );

//---------------------------------------------------------------------------
int vtkDMMLTestCustomDisplayableManager::NodeAddedCountThreeDView = 0;
int vtkDMMLTestCustomDisplayableManager::NodeAddedCountSliceView = 0;

//---------------------------------------------------------------------------
class vtkDMMLTestCustomDisplayableManager::vtkInternal
{
public:
  vtkInternal(vtkDMMLTestCustomDisplayableManager * external);
  ~vtkInternal();

  vtkDMMLTestCustomDisplayableManager*             External;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkDMMLTestCustomDisplayableManager::vtkInternal::vtkInternal(vtkDMMLTestCustomDisplayableManager * external)
{
  this->External = external;
}

//---------------------------------------------------------------------------
vtkDMMLTestCustomDisplayableManager::vtkInternal::~vtkInternal() = default;

//---------------------------------------------------------------------------
// vtkDMMLTestCustomDisplayableManager methods

//---------------------------------------------------------------------------
vtkDMMLTestCustomDisplayableManager::vtkDMMLTestCustomDisplayableManager()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkDMMLTestCustomDisplayableManager::~vtkDMMLTestCustomDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkDMMLTestCustomDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkDMMLTestCustomDisplayableManager::AdditionnalInitializeStep()
{
  this->AddInteractorStyleObservableEvent(vtkCommand::KeyPressEvent);
}

//---------------------------------------------------------------------------
void vtkDMMLTestCustomDisplayableManager::OnInteractorStyleEvent(int eventid)
{
  std::cout << "OnInteractorStyleEvent: event id = " << eventid << std::endl;
}

//---------------------------------------------------------------------------
void vtkDMMLTestCustomDisplayableManager::Create()
{
  assert(this->GetRenderer());
  assert(this->GetDMMLDisplayableNode());
}

//---------------------------------------------------------------------------
void vtkDMMLTestCustomDisplayableManager::OnDMMLSceneNodeAdded(vtkDMMLNode* node)
{
  vtkDMMLCameraNode * cameraNode = vtkDMMLCameraNode::SafeDownCast(node);
  if (!cameraNode)
    {
    return;
    }
  if (vtkDMMLViewNode::SafeDownCast(this->GetDMMLDisplayableNode()))
    {
    vtkDMMLTestCustomDisplayableManager::NodeAddedCountThreeDView++;
    //std::cout << "vtkDMMLTestCustomDisplayableManager[vtkDMMLViewNode] - NodeAdded - "
    //          << (node ? node->GetName() : "None")<< std::endl;
    }
  if (vtkDMMLSliceNode::SafeDownCast(this->GetDMMLDisplayableNode()))
    {
    vtkDMMLTestCustomDisplayableManager::NodeAddedCountSliceView++;
    //std::cout << "vtkDMMLTestCustomDisplayableManager[vtkDMMLSliceNode] - NodeAdded - "
    //          << (node ? node->GetName() : "None")<< std::endl;
    }
}
