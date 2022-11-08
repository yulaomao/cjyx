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

  This file was originally developed by Andras Lasso, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// DMML includes
#include "vtkDMMLLabelMapVolumeNode.h"
#include "vtkDMMLLabelMapVolumeDisplayNode.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkDataArray.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLLabelMapVolumeNode);

//----------------------------------------------------------------------------
vtkDMMLLabelMapVolumeNode::vtkDMMLLabelMapVolumeNode() = default;

//----------------------------------------------------------------------------
vtkDMMLLabelMapVolumeNode::~vtkDMMLLabelMapVolumeNode() = default;

//-----------------------------------------------------------
void vtkDMMLLabelMapVolumeNode::CreateNoneNode(vtkDMMLScene *scene)
{
  vtkNew<vtkImageData> id;
  id->SetDimensions(1, 1, 1);
  id->AllocateScalars(VTK_SHORT, 1);
  id->GetPointData()->GetScalars()->FillComponent(0, 0);

  vtkNew<vtkDMMLLabelMapVolumeNode> n;
  n->SetName("None");
  // the scene will set the id
  n->SetAndObserveImageData(id.GetPointer());
  scene->AddNode(n.GetPointer());
}

//----------------------------------------------------------------------------
void vtkDMMLLabelMapVolumeNode::CreateDefaultDisplayNodes()
{
  if (vtkDMMLLabelMapVolumeDisplayNode::SafeDownCast(this->GetDisplayNode())!=nullptr)
    {
    // display node already exists
    return;
    }
  if (this->GetScene()==nullptr)
    {
    vtkErrorMacro("vtkDMMLLabelMapVolumeNode::CreateDefaultDisplayNodes failed: scene is invalid");
    return;
    }
  vtkDMMLLabelMapVolumeDisplayNode* dispNode = vtkDMMLLabelMapVolumeDisplayNode::SafeDownCast(
    this->GetScene()->AddNewNodeByClass("vtkDMMLLabelMapVolumeDisplayNode") );
  dispNode->SetDefaultColorMap();
  this->SetAndObserveDisplayNodeID(dispNode->GetID());
}

//----------------------------------------------------------------------------
int vtkDMMLLabelMapVolumeNode::GetResamplingInterpolationMode()
{
  // Labelmap volumes must be resampled using nearest neighbor method to avoid
  // introducing new label values at boundaries.
  return VTK_NEAREST_INTERPOLATION;
}
