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

// DMML includes
#include "vtkDMMLMarkupsFiducialDisplayNode.h"
#include "vtkDMMLMarkupsFiducialNode.h"
#include "vtkDMMLMarkupsFiducialStorageNode.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkBoundingBox.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLMarkupsFiducialNode);


//----------------------------------------------------------------------------
vtkDMMLMarkupsFiducialNode::vtkDMMLMarkupsFiducialNode() = default;

//----------------------------------------------------------------------------
vtkDMMLMarkupsFiducialNode::~vtkDMMLMarkupsFiducialNode() = default;

//----------------------------------------------------------------------------
void vtkDMMLMarkupsFiducialNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);

  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLIntMacro(maximumNumberOfControlPoints, MaximumNumberOfControlPoints);
  vtkDMMLWriteXMLIntMacro(requiredNumberOfControlPoints, RequiredNumberOfControlPoints);
  vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsFiducialNode::ReadXMLAttributes(const char** atts)
{
  DMMLNodeModifyBlocker blocker(this);

  Superclass::ReadXMLAttributes(atts);

  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLIntMacro(maximumNumberOfControlPoints, MaximumNumberOfControlPoints);
  vtkDMMLReadXMLIntMacro(requiredNumberOfControlPoints, RequiredNumberOfControlPoints);
  vtkDMMLReadXMLEndMacro();

  // In scenes created by Cjyx version version 4.13.0 revision 30287 (built 2021-10-05).
  // The value used to represent unlimited control points has been changed to -1.
  if (this->MaximumNumberOfControlPoints == 0)
    {
    this->MaximumNumberOfControlPoints = -1;
    }
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsFiducialNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkDMMLCopyBeginMacro(anode);
  vtkDMMLCopyIntMacro(MaximumNumberOfControlPoints);
  vtkDMMLCopyIntMacro(RequiredNumberOfControlPoints);
  vtkDMMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLMarkupsFiducialNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}


//-------------------------------------------------------------------------
void vtkDMMLMarkupsFiducialNode::CreateDefaultDisplayNodes()
{
  if (this->GetDisplayNode() != nullptr &&
    vtkDMMLMarkupsDisplayNode::SafeDownCast(this->GetDisplayNode()) != nullptr)
    {
    // display node already exists
    return;
    }
  if (this->GetScene() == nullptr)
    {
    vtkErrorMacro("vtkDMMLMarkupsFiducialNode::CreateDefaultDisplayNodes failed: scene is invalid");
    return;
    }
  vtkDMMLMarkupsFiducialDisplayNode* dispNode = vtkDMMLMarkupsFiducialDisplayNode::SafeDownCast(
    this->GetScene()->AddNewNodeByClass("vtkDMMLMarkupsFiducialDisplayNode"));
  if (!dispNode)
    {
    vtkErrorMacro("vtkDMMLMarkupsFiducialNode::CreateDefaultDisplayNodes failed: unable to create vtkDMMLMarkupsFiducialDisplayNode");
    return;
    }
  this->SetAndObserveDisplayNodeID(dispNode->GetID());
}
