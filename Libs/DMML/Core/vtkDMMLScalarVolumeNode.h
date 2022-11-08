/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLVolumeNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkDMMLScalarVolumeNode_h
#define __vtkDMMLScalarVolumeNode_h

// DMML includes
#include "vtkDMMLVolumeNode.h"
class vtkDMMLScalarVolumeDisplayNode;
class vtkCodedEntry;

/// \brief DMML node for representing a volume (image stack).
///
/// Volume nodes describe data sets that can be thought of as stacks of 2D
/// images that form a 3D volume. Volume nodes contain only the image data,
/// where it is store on disk and how to read the files is controlled by
/// the volume storage node, how to render the data (window and level) is
/// controlled by the volume display nodes. Image information is extracted
/// from the image headers (if they exist) at the time the DMML file is
/// generated.
/// Consequently, DMML files isolate DMML browsers from understanding how
/// to read the myriad of file formats for medical data.
class VTK_DMML_EXPORT vtkDMMLScalarVolumeNode : public vtkDMMLVolumeNode
{
  public:
  static vtkDMMLScalarVolumeNode *New();
  vtkTypeMacro(vtkDMMLScalarVolumeNode,vtkDMMLVolumeNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  ///
  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkDMMLNode::CopyContent
  vtkDMMLCopyContentDefaultMacro(vtkDMMLScalarVolumeNode);

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "Volume";}

  ///
  /// Make a 'None' volume node with blank image data
  static void CreateNoneNode(vtkDMMLScene *scene);

  ///
  /// Associated display DMML node
  virtual vtkDMMLScalarVolumeDisplayNode* GetScalarVolumeDisplayNode();

  ///
  /// Create default storage node or nullptr if does not have one
  vtkDMMLStorageNode* CreateDefaultStorageNode() override;

  ///
  /// Create and observe default display node
  void CreateDefaultDisplayNodes() override;

  /// Measured quantity of voxel values, specified as a standard coded entry.
  /// For example: (DCM, 112031, "Attenuation Coefficient")
  void SetVoxelValueQuantity(vtkCodedEntry*);
  vtkGetObjectMacro(VoxelValueQuantity, vtkCodedEntry);

  /// Measurement unit of voxel value quantity, specified as a standard coded entry.
  /// For example: (UCUM, [hnsf'U], "Hounsfield unit")
  /// It stores a single unit. Plural (units) name is chosen to be consistent
  /// with nomenclature in the DICOM standard.
  void SetVoxelValueUnits(vtkCodedEntry*);
  vtkGetObjectMacro(VoxelValueUnits, vtkCodedEntry);

protected:
  vtkDMMLScalarVolumeNode();
  ~vtkDMMLScalarVolumeNode() override;
  vtkDMMLScalarVolumeNode(const vtkDMMLScalarVolumeNode&);
  void operator=(const vtkDMMLScalarVolumeNode&);

  vtkCodedEntry* VoxelValueQuantity{nullptr};
  vtkCodedEntry* VoxelValueUnits{nullptr};
};

#endif
