/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLVolumeHeaderlessStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkDMMLVolumeHeaderlessStorageNode_h
#define __vtkDMMLVolumeHeaderlessStorageNode_h

#include "vtkDMMLStorageNode.h"

class vtkImageData;

/// \brief DMML node for representing a volume storage.
///
/// vtkDMMLVolumeHeaderlessStorageNode nodes describes how volume data sets is stored on disk.
class VTK_DMML_EXPORT vtkDMMLVolumeHeaderlessStorageNode
  : public vtkDMMLStorageNode
{
public:
  static vtkDMMLVolumeHeaderlessStorageNode *New();
  vtkTypeMacro(vtkDMMLVolumeHeaderlessStorageNode,vtkDMMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  ///
  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Copy the node's attributes to this object
  void Copy(vtkDMMLNode *node) override;

  ///
  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "VolumeHeaderlessStorage";}

  ///
  /// Scan order in the file
  vtkGetStringMacro(FileScanOrder);
  vtkSetStringMacro(FileScanOrder);

  ///
  /// Two numbers: the number of columns and rows of pixels in each image
  vtkGetVector3Macro(FileDimensions, int);
  vtkSetVector3Macro(FileDimensions, int);

  ///
  /// Three numbers for the dimensions of each voxel, in millimeters
  vtkGetVector3Macro(FileSpacing, double);
  vtkSetVector3Macro(FileSpacing, double);

  ///
  /// The type of data in the file. One of: Char, UnsignedChar, Short,
  /// UnsignedShort, Int, UnsignedInt, Long, UnsignedLong, Float, Double
  vtkSetMacro(FileScalarType, int);
  vtkGetMacro(FileScalarType, int);

  void SetFileScalarTypeToUnsignedChar()
    {this->SetFileScalarType(VTK_UNSIGNED_CHAR);};
  void SetFileScalarTypeToChar()
    {this->SetFileScalarType(VTK_CHAR);};
  void SetFileScalarTypeToShort() {
    this->SetFileScalarType(VTK_SHORT);};
  void SetFileScalarTypeToUnsignedShort()
    {this->SetFileScalarType(VTK_UNSIGNED_SHORT);};
  void SetFileScalarTypeToInt() {
    this->SetFileScalarType(VTK_INT);};
  void SetFileScalarTypeToUnsignedInt() {
    this->SetFileScalarType(VTK_UNSIGNED_INT);};
  void SetFileScalarTypeToLong() {
    this->SetFileScalarType(VTK_LONG);};
  void SetFileScalarTypeToUnsignedLong() {
    this->SetFileScalarType(VTK_UNSIGNED_LONG);};
  void SetFileScalarTypeToFloat() {
    this->SetFileScalarType(VTK_FLOAT);};
  void SetFileScalarTypeToDouble() {
    this->SetFileScalarType(VTK_DOUBLE);};

  const char* GetFileScalarTypeAsString();

  void SetFileScalarTypeAsString(const char* );

  ///
  /// The number of scalar components for each voxel.
  /// Gray-level data has 1. Color data has 3
  vtkGetMacro(FileNumberOfScalarComponents, int);
  vtkSetMacro(FileNumberOfScalarComponents, int);

  ///
  /// Describes the order of bytes for each voxel.  Little endian
  /// positions the least-significant byte on the rightmost end,
  /// and is true of data generated on a PC or SGI.
  vtkGetMacro(FileLittleEndian, int);
  vtkSetMacro(FileLittleEndian, int);
  vtkBooleanMacro(FileLittleEndian, int);

  ///
  /// Center image on read
  vtkGetMacro(CenterImage, int);
  vtkSetMacro(CenterImage, int);

  /// Return true if node can be read in
  bool CanReadInReferenceNode(vtkDMMLNode *refNode) override;
  bool CanWriteFromReferenceNode(vtkDMMLNode *refNode) override;

protected:

  vtkDMMLVolumeHeaderlessStorageNode();
  ~vtkDMMLVolumeHeaderlessStorageNode() override;
  vtkDMMLVolumeHeaderlessStorageNode(const vtkDMMLVolumeHeaderlessStorageNode&);
  void operator=(const vtkDMMLVolumeHeaderlessStorageNode&);

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  /// Read data and set it in the referenced node
  int ReadDataInternal(vtkDMMLNode *refNode) override;

  /// Write data from a  referenced node
  int WriteDataInternal(vtkDMMLNode *refNode) override;

  char *FileScanOrder;
  int FileScalarType;
  int FileNumberOfScalarComponents;
  int FileLittleEndian;
  double FileSpacing[3];
  int FileDimensions[3];

  int CenterImage;

  char* WriteFileFormat;
};

#endif
