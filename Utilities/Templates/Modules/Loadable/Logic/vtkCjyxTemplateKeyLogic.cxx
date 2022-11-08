/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// TemplateKey Logic includes
#include "vtkCjyxTemplateKeyLogic.h"

// DMML includes
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkCjyxTemplateKeyLogic);

//----------------------------------------------------------------------------
vtkCjyxTemplateKeyLogic::vtkCjyxTemplateKeyLogic()
{
}

//----------------------------------------------------------------------------
vtkCjyxTemplateKeyLogic::~vtkCjyxTemplateKeyLogic()
{
}

//----------------------------------------------------------------------------
void vtkCjyxTemplateKeyLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkCjyxTemplateKeyLogic::SetDMMLSceneInternal(vtkDMMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkDMMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkDMMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkDMMLScene::EndBatchProcessEvent);
  this->SetAndObserveDMMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkCjyxTemplateKeyLogic::RegisterNodes()
{
  assert(this->GetDMMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkCjyxTemplateKeyLogic::UpdateFromDMMLScene()
{
  assert(this->GetDMMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkCjyxTemplateKeyLogic
::OnDMMLSceneNodeAdded(vtkDMMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkCjyxTemplateKeyLogic
::OnDMMLSceneNodeRemoved(vtkDMMLNode* vtkNotUsed(node))
{
}
