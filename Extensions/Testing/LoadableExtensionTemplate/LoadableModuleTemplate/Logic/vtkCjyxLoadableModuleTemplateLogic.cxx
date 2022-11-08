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

// LoadableModuleTemplate Logic includes
#include "vtkCjyxLoadableModuleTemplateLogic.h"

// DMML includes
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkCjyxLoadableModuleTemplateLogic);

//----------------------------------------------------------------------------
vtkCjyxLoadableModuleTemplateLogic::vtkCjyxLoadableModuleTemplateLogic()
{
}

//----------------------------------------------------------------------------
vtkCjyxLoadableModuleTemplateLogic::~vtkCjyxLoadableModuleTemplateLogic()
{
}

//----------------------------------------------------------------------------
void vtkCjyxLoadableModuleTemplateLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkCjyxLoadableModuleTemplateLogic::SetDMMLSceneInternal(vtkDMMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkDMMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkDMMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkDMMLScene::EndBatchProcessEvent);
  this->SetAndObserveDMMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkCjyxLoadableModuleTemplateLogic::RegisterNodes()
{
  assert(this->GetDMMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkCjyxLoadableModuleTemplateLogic::UpdateFromDMMLScene()
{
  assert(this->GetDMMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkCjyxLoadableModuleTemplateLogic
::OnDMMLSceneNodeAdded(vtkDMMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkCjyxLoadableModuleTemplateLogic
::OnDMMLSceneNodeRemoved(vtkDMMLNode* vtkNotUsed(node))
{
}
