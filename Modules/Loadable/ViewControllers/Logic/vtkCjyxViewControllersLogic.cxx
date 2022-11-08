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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// SliceViewControllers Logic includes
#include "vtkCjyxViewControllersLogic.h"

// DMML includes
#include <vtkDMMLPlotViewNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkCjyxViewControllersLogic);

//----------------------------------------------------------------------------
vtkCjyxViewControllersLogic::vtkCjyxViewControllersLogic() = default;

//----------------------------------------------------------------------------
vtkCjyxViewControllersLogic::~vtkCjyxViewControllersLogic() = default;

//----------------------------------------------------------------------------
void vtkCjyxViewControllersLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
 }

//-----------------------------------------------------------------------------
void vtkCjyxViewControllersLogic::RegisterNodes()
{
  vtkDMMLScene *scene = this->GetDMMLScene();
  if (!scene)
    {
    vtkErrorMacro("vtkCjyxViewControllersLogic::RegisterNodes failed: invalid scene");
    return;
    }
}

//-----------------------------------------------------------------------------
vtkDMMLSliceNode* vtkCjyxViewControllersLogic::GetDefaultSliceViewNode()
{
  vtkDMMLScene *scene = this->GetDMMLScene();
  if (!scene)
    {
    vtkErrorMacro("vtkCjyxViewControllersLogic::GetDefaultSliceViewNode failed: invalid scene");
    return nullptr;
    }
  vtkDMMLNode* defaultNode = scene->GetDefaultNodeByClass("vtkDMMLSliceNode");
  if (!defaultNode)
    {
    defaultNode = scene->CreateNodeByClass("vtkDMMLSliceNode");
    scene->AddDefaultNode(defaultNode);
    defaultNode->Delete(); // scene owns it now
    }
  return vtkDMMLSliceNode::SafeDownCast(defaultNode);
}

//-----------------------------------------------------------------------------
vtkDMMLViewNode* vtkCjyxViewControllersLogic::GetDefaultThreeDViewNode()
{
  vtkDMMLScene *scene = this->GetDMMLScene();
  if (!scene)
    {
    vtkErrorMacro("vtkCjyxViewControllersLogic::GetDefaultThreeDViewNode failed: invalid scene");
    return nullptr;
    }
  vtkDMMLNode* defaultNode = scene->GetDefaultNodeByClass("vtkDMMLViewNode");
  if (!defaultNode)
    {
    defaultNode = scene->CreateNodeByClass("vtkDMMLViewNode");
    scene->AddDefaultNode(defaultNode);
    defaultNode->Delete(); // scene owns it now
    }
  return vtkDMMLViewNode::SafeDownCast(defaultNode);
}

//-----------------------------------------------------------------------------
vtkDMMLPlotViewNode *vtkCjyxViewControllersLogic::GetDefaultPlotViewNode()
{
  vtkDMMLScene *scene = this->GetDMMLScene();
  if (!scene)
    {
    vtkErrorMacro("vtkCjyxViewControllersLogic::GetDefaultPlotViewNode failed: invalid scene");
    return nullptr;
    }
  vtkDMMLNode* defaultNode = scene->GetDefaultNodeByClass("vtkDMMLPlotViewNode");
  if (!defaultNode)
    {
    defaultNode = scene->CreateNodeByClass("vtkDMMLPlotViewNode");
    scene->AddDefaultNode(defaultNode);
    defaultNode->Delete(); // scene owns it now
    }
  return vtkDMMLPlotViewNode::SafeDownCast(defaultNode);
}

//-----------------------------------------------------------------------------
void vtkCjyxViewControllersLogic::ResetAllViewNodesToDefault()
{
  vtkDMMLScene *scene = this->GetDMMLScene();
  if (!scene)
    {
    vtkErrorMacro("vtkCjyxViewControllersLogic::ResetAllViewNodesToDefault failed: invalid scene");
    return;
    }
  scene->StartState(vtkDMMLScene::BatchProcessState);
  vtkDMMLSliceNode* defaultSliceViewNode = GetDefaultSliceViewNode();
  std::vector< vtkDMMLNode* > viewNodes;
  scene->GetNodesByClass("vtkDMMLSliceNode", viewNodes);
  for (std::vector< vtkDMMLNode* >::iterator it = viewNodes.begin(); it != viewNodes.end(); ++it)
    {
    vtkDMMLSliceNode* sliceNode = vtkDMMLSliceNode::SafeDownCast(*it);
    if (!sliceNode)
      {
      continue;
      }
    sliceNode->Reset(defaultSliceViewNode);
    sliceNode->SetOrientationToDefault();
    }
  viewNodes.clear();
  vtkDMMLViewNode* defaultThreeDViewNode = GetDefaultThreeDViewNode();
  scene->GetNodesByClass("vtkDMMLViewNode", viewNodes);
  for (std::vector< vtkDMMLNode* >::iterator it = viewNodes.begin(); it != viewNodes.end(); ++it)
    {
    (*it)->Reset(defaultThreeDViewNode);
    }
  viewNodes.clear();
  vtkDMMLPlotViewNode* defaultPlotViewNode = GetDefaultPlotViewNode();
  scene->GetNodesByClass("vtkDMMLPlotViewNode", viewNodes);
  for (std::vector< vtkDMMLNode* >::iterator it = viewNodes.begin(); it != viewNodes.end(); ++it)
    {
    (*it)->Reset(defaultPlotViewNode);
    }
  scene->EndState(vtkDMMLScene::BatchProcessState);
}
