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

// .NAME vtkDMMLMarkupsFiducialDisplayNode - DMML node to represent display properties for markups fiducials
// .SECTION Description
// Currently, the only difference compared to the generic markups display node is that point labels
// are displayed by default.
//

#ifndef __vtkDMMLMarkupsFiducialDisplayNode_h
#define __vtkDMMLMarkupsFiducialDisplayNode_h

#include "vtkCjyxMarkupsModuleDMMLExport.h"

#include "vtkDMMLMarkupsDisplayNode.h"

class vtkDMMLProceduralColorNode;

/// \ingroup Cjyx_QtModules_Markups
class  VTK_CJYX_MARKUPS_MODULE_DMML_EXPORT vtkDMMLMarkupsFiducialDisplayNode : public vtkDMMLMarkupsDisplayNode
{
public:
  static vtkDMMLMarkupsFiducialDisplayNode *New();
  vtkTypeMacro ( vtkDMMLMarkupsFiducialDisplayNode,vtkDMMLMarkupsDisplayNode );

  //--------------------------------------------------------------------------
  // DMMLNode methods
  //--------------------------------------------------------------------------

  vtkDMMLNode* CreateNodeInstance (  ) override;

  // Get node XML tag name (like Volume, Markups)
  const char* GetNodeTagName() override {return "MarkupsFiducialDisplay";};

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkDMMLNode::CopyContent
  vtkDMMLCopyContentDefaultMacro(vtkDMMLMarkupsFiducialDisplayNode);

protected:
  vtkDMMLMarkupsFiducialDisplayNode();
  ~vtkDMMLMarkupsFiducialDisplayNode() override;
  vtkDMMLMarkupsFiducialDisplayNode( const vtkDMMLMarkupsFiducialDisplayNode& );
  void operator= ( const vtkDMMLMarkupsFiducialDisplayNode& );
};
#endif
