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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care
  and CANARIE.

==============================================================================*/

// DMML includes
#include "vtkDMMLMultiVolumeRenderingDisplayNode.h"

// VTK includes
#include <vtkObjectFactory.h>

// STL includes
#include <sstream>

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLMultiVolumeRenderingDisplayNode);

//----------------------------------------------------------------------------
vtkDMMLMultiVolumeRenderingDisplayNode::vtkDMMLMultiVolumeRenderingDisplayNode() = default;

//----------------------------------------------------------------------------
vtkDMMLMultiVolumeRenderingDisplayNode::~vtkDMMLMultiVolumeRenderingDisplayNode() = default;

//----------------------------------------------------------------------------
void vtkDMMLMultiVolumeRenderingDisplayNode::ReadXMLAttributes(const char** atts)
{
  this->Superclass::ReadXMLAttributes(atts);

  //vtkDMMLReadXMLBeginMacro(atts);
  //vtkDMMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLMultiVolumeRenderingDisplayNode::WriteXML(ostream& of, int nIndent)
{
  this->Superclass::WriteXML(of, nIndent);

  //vtkDMMLWriteXMLBeginMacro(of);
  //vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLMultiVolumeRenderingDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  //vtkDMMLPrintBeginMacro(os, indent);
  //vtkDMMLPrintEndMacro();
}
