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

// .NAME vtkCjyxSuperLoadableModuleTemplateLogic - cjyx logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkCjyxSuperLoadableModuleTemplateLogic_h
#define __vtkCjyxSuperLoadableModuleTemplateLogic_h

// Cjyx includes
#include "vtkCjyxModuleLogic.h"

// DMML includes

// STD includes
#include <cstdlib>

#include "vtkCjyxSuperLoadableModuleTemplateModuleLogicExport.h"


/// \ingroup Cjyx_QtModules_ExtensionTemplate
class VTK_CJYX_SUPERLOADABLEMODULETEMPLATE_MODULE_LOGIC_EXPORT vtkCjyxSuperLoadableModuleTemplateLogic :
  public vtkCjyxModuleLogic
{
public:

  static vtkCjyxSuperLoadableModuleTemplateLogic *New();
  vtkTypeMacro(vtkCjyxSuperLoadableModuleTemplateLogic, vtkCjyxModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:
  vtkCjyxSuperLoadableModuleTemplateLogic();
  ~vtkCjyxSuperLoadableModuleTemplateLogic() override;

  void SetDMMLSceneInternal(vtkDMMLScene* newScene) override;
  /// Register DMML Node classes to Scene. Gets called automatically when the DMMLScene is attached to this logic class.
  void RegisterNodes() override;
  void UpdateFromDMMLScene() override;
  void OnDMMLSceneNodeAdded(vtkDMMLNode* node) override;
  void OnDMMLSceneNodeRemoved(vtkDMMLNode* node) override;
private:

  vtkCjyxSuperLoadableModuleTemplateLogic(const vtkCjyxSuperLoadableModuleTemplateLogic&); // Not implemented
  void operator=(const vtkCjyxSuperLoadableModuleTemplateLogic&); // Not implemented
};

#endif
