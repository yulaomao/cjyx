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
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/
// .NAME vtkDMMLMarkupsPlaneDisplayNode - DMML node to represent display properties for markups Plane
// .SECTION Description
// Adjusts default display parameters for Plane such as fill opacity.
//

#ifndef __vtkDMMLMarkupsPlaneDisplayNode_h
#define __vtkDMMLMarkupsPlaneDisplayNode_h

// Markups DMML includes
#include "vtkDMMLMarkupsDisplayNode.h"
#include "vtkCjyxMarkupsModuleDMMLExport.h"

/// \ingroup Cjyx_QtModules_Markups
class  VTK_CJYX_MARKUPS_MODULE_DMML_EXPORT vtkDMMLMarkupsPlaneDisplayNode : public vtkDMMLMarkupsDisplayNode
{
public:
  static vtkDMMLMarkupsPlaneDisplayNode* New();
  vtkTypeMacro(vtkDMMLMarkupsPlaneDisplayNode, vtkDMMLMarkupsDisplayNode);

  //--------------------------------------------------------------------------
  // DMMLNode methods
  //--------------------------------------------------------------------------

  vtkDMMLNode* CreateNodeInstance() override;

  // Get node XML tag name (like Volume, Markups)
  const char* GetNodeTagName() override { return "MarkupsPlaneDisplay"; };

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkDMMLNode::CopyContent
  vtkDMMLCopyContentDefaultMacro(vtkDMMLMarkupsPlaneDisplayNode);

  enum
  {
    ComponentPlane = vtkDMMLMarkupsDisplayNode::Component_Last,
    ComponentPlane_Last
  };

  /// Indexes of the scale handles
  enum
  {
    HandleLEdge,
    HandleREdge,
    HandlePEdge,
    HandleAEdge,

    HandleLPCorner,
    HandleRPCorner,
    HandleLACorner,
    HandleRACorner,

    HandlePlane_Last
  };

  //@{
  /// Get/Set the visibility of the plane normal arrow.
  vtkSetMacro(NormalVisibility, bool);
  vtkGetMacro(NormalVisibility, bool);
  vtkBooleanMacro(NormalVisibility, bool);
  //@}

  //@{
  /// Get/Set the opacity of the plane normal arrow.
  vtkSetMacro(NormalOpacity, double);
  vtkGetMacro(NormalOpacity, double);
  //@}

protected:

  bool NormalVisibility{ true };
  double NormalOpacity{ 1.0 };

  vtkDMMLMarkupsPlaneDisplayNode();
  ~vtkDMMLMarkupsPlaneDisplayNode() override;
  vtkDMMLMarkupsPlaneDisplayNode(const vtkDMMLMarkupsPlaneDisplayNode&);
  void operator= (const vtkDMMLMarkupsPlaneDisplayNode&);
};
#endif
