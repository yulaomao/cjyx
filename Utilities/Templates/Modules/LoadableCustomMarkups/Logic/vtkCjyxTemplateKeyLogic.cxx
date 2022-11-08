/*==============================================================================

  Copyright (c) The Intervention Centre
  Oslo University Hospital, Oslo, Norway. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Rafael Palomar (The Intervention Centre,
  Oslo University Hospital) and was supported by The Research Council of Norway
  through the ALive project (grant nr. 311393).

==============================================================================*/

#include "vtkCjyxTemplateKeyLogic.h"

// TemplateKey DMML includes
#include "vtkDMMLMarkupsTestLineNode.h"

// TemplateKey VTKWidgets includes
#include "vtkCjyxTestLineWidget.h"

// DMML includes
#include <vtkDMMLScene.h>

// Markups logic includes
#include <vtkCjyxMarkupsLogic.h>

// Markups DMML includes
#include <vtkDMMLMarkupsDisplayNode.h>

// VTK includes
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkCjyxTemplateKeyLogic);

//---------------------------------------------------------------------------
vtkCjyxTemplateKeyLogic::vtkCjyxTemplateKeyLogic()
{
}

//---------------------------------------------------------------------------
vtkCjyxTemplateKeyLogic::~vtkCjyxTemplateKeyLogic() = default;

//---------------------------------------------------------------------------
void vtkCjyxTemplateKeyLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkCjyxTemplateKeyLogic::RegisterNodes()
{
  vtkDMMLScene *scene = this->GetDMMLScene();
  if (!scene)
    {
    vtkErrorMacro("RegisterNodes failed: invalid scene");
    return;
    }

  vtkCjyxMarkupsLogic* markupsLogic = vtkCjyxMarkupsLogic::SafeDownCast(this->GetModuleLogic("Markups"));
  if (!markupsLogic)
    {
    vtkErrorMacro("RegisterNodes failed: invalid markups module logic");
    return;
    }

  vtkNew<vtkDMMLMarkupsTestLineNode> markupsTestLineNode;
  vtkNew<vtkCjyxTestLineWidget> testLineWidget;
  markupsLogic->RegisterMarkupsNode(markupsTestLineNode, testLineWidget);
}
