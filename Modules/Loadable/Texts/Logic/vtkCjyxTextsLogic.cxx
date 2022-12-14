/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

#include "vtkCjyxTextsLogic.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// DMML includes
#include <vtkDMMLScene.h>
#include <vtkDMMLTextNode.h>
#include <vtkDMMLTextStorageNode.h>

vtkStandardNewMacro(vtkCjyxTextsLogic);

//----------------------------------------------------------------------------
vtkCjyxTextsLogic::vtkCjyxTextsLogic() = default;

//----------------------------------------------------------------------------
vtkCjyxTextsLogic::~vtkCjyxTextsLogic() = default;
