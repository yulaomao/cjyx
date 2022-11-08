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

// SuperLoadableModuleTemplate Logic includes
#include "vtkCjyxSuperLoadableModuleTemplateLogic.h"

// DMML includes
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkCjyxSuperLoadableModuleTemplateLogic);

//----------------------------------------------------------------------------
vtkCjyxSuperLoadableModuleTemplateLogic::vtkCjyxSuperLoadableModuleTemplateLogic()
{
}

//----------------------------------------------------------------------------
vtkCjyxSuperLoadableModuleTemplateLogic::~vtkCjyxSuperLoadableModuleTemplateLogic()
{
}

//----------------------------------------------------------------------------
void vtkCjyxSuperLoadableModuleTemplateLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkCjyxSuperLoadableModuleTemplateLogic::SetDMMLSceneInternal(vtkDMMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkDMMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkDMMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkDMMLScene::EndBatchProcessEvent);
  this->SetAndObserveDMMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkCjyxSuperLoadableModuleTemplateLogic::RegisterNodes()
{
  assert(this->GetDMMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkCjyxSuperLoadableModuleTemplateLogic::UpdateFromDMMLScene()
{
  assert(this->GetDMMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkCjyxSuperLoadableModuleTemplateLogic
::OnDMMLSceneNodeAdded(vtkDMMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkCjyxSuperLoadableModuleTemplateLogic
::OnDMMLSceneNodeRemoved(vtkDMMLNode* vtkNotUsed(node))
{
}
