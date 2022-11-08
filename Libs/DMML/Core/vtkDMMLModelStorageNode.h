/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLModelStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkDMMLModelStorageNode_h
#define __vtkDMMLModelStorageNode_h

#include "vtkDMMLStorageNode.h"

class vtkDMMLModelNode;
class vtkPointSet;

/// \brief DMML node for model storage on disk.
///
/// Storage nodes has methods to read/write vtkPolyData to/from disk.
class VTK_DMML_EXPORT vtkDMMLModelStorageNode : public vtkDMMLStorageNode
{
public:
  static vtkDMMLModelStorageNode *New();
  vtkTypeMacro(vtkDMMLModelStorageNode,vtkDMMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  ///
  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override  {return "ModelStorage";}

  /// Read node attributes from XML file
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Return true if the reference node can be read in
  bool CanReadInReferenceNode(vtkDMMLNode *refNode) override;

  /// Get/Set flag that controls if points are to be written in various coordinate systems
  vtkSetClampMacro(CoordinateSystem, int, 0, vtkDMMLStorageNode::CoordinateSystemType_Last-1);
  vtkGetMacro(CoordinateSystem, int);
  static const char* GetCoordinateSystemAsString(int id);
  static int GetCoordinateSystemFromString(const char* name);

  /// Helper function that can convert a mesh (polydata, unstructured grid, or even just a point cloud)
  /// between RAS and LPS coordinate system.
  static void ConvertBetweenRASAndLPS(vtkPointSet* inputMesh, vtkPointSet* outputMesh);

protected:
  vtkDMMLModelStorageNode();
  ~vtkDMMLModelStorageNode() override;
  vtkDMMLModelStorageNode(const vtkDMMLModelStorageNode&);
  void operator=(const vtkDMMLModelStorageNode&);

  /// Initialize all the supported read file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  /// Get data node that is associated with this storage node
  vtkDMMLModelNode* GetAssociatedDataNode();

  /// Read data and set it in the referenced node
  int ReadDataInternal(vtkDMMLNode *refNode) override;

  /// Write data from a  referenced node
  int WriteDataInternal(vtkDMMLNode *refNode) override;

  static int GetCoordinateSystemFromFileHeader(const char* header);

  static int GetCoordinateSystemFromFieldData(vtkPointSet* mesh);

  int CoordinateSystem;
};

#endif
