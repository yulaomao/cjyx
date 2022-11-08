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

// .NAME vtkDMMLMarkupsStorageNode - Abstract base class for markups storage nodes.
// .SECTION Description
// This class is the base of all markups storage nodes and it allows specifying the
// coordinate system that is used in files.
//
// This interface class is defined in DMML core to allow CLI modules to read/write markups
// in a specific coordinate system.

#ifndef __vtkDMMLMarkupsStorageNode_h
#define __vtkDMMLMarkupsStorageNode_h

// DMML includes
#include "vtkDMMLStorageNode.h"

class VTK_DMML_EXPORT vtkDMMLMarkupsStorageNode : public vtkDMMLStorageNode
{
public:
  vtkTypeMacro(vtkDMMLMarkupsStorageNode,vtkDMMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object
  void Copy(vtkDMMLNode *node) override;

  bool CanReadInReferenceNode(vtkDMMLNode *refNode) override;

  /// Get/Set flag that controls if points are to be written in various coordinate systems
  vtkSetClampMacro(CoordinateSystem, int, 0, vtkDMMLStorageNode::CoordinateSystemType_Last-1);
  vtkGetMacro(CoordinateSystem, int);
  std::string GetCoordinateSystemAsString();
  static const char* GetCoordinateSystemAsString(int id);
  static int GetCoordinateSystemFromString(const char* name);
  /// Convenience methods to get/set various coordinate system values
  /// \sa SetCoordinateSystem, GetCoordinateSystem
  void UseRASOn();
  bool GetUseRAS();
  void UseLPSOn();
  bool GetUseLPS();

protected:
  vtkDMMLMarkupsStorageNode();
  ~vtkDMMLMarkupsStorageNode() override;
  vtkDMMLMarkupsStorageNode(const vtkDMMLMarkupsStorageNode&);
  void operator=(const vtkDMMLMarkupsStorageNode&);

private:

  /// Flag set to enum RAS if the points are to be written out/read in using
  /// the RAS coordinate system, enum LPS if the points are to be written
  /// out/read in using LPS coordinate system, enum IJK if the points are
  /// to be written out in the IJK coordinates for the associated volume node.
  int CoordinateSystem;
};

#endif
