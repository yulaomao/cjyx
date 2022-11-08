/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLVolumeNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkDMMLTensorVolumeNode_h
#define __vtkDMMLTensorVolumeNode_h

#include "vtkDMMLScalarVolumeNode.h"
class vtkDMMLStorageNode;

class vtkDoubleArray;
class vtkImageData;
class vtkMatrix4x4;

/// \brief DMML node for representing diffusion weighted MRI volume.
///
/// Diffusion Weighted Volume nodes describe data sets that encode diffusion weighted
/// images. These images are the basis for computing the diffusion tensor.
/// The node is a container for the necessary information to interpert DW images:
/// 1. Gradient information.
/// 2. B value for each gradient.
/// 3. Measurement frame that relates the coordinate system where the gradients are given
///  to RAS.
class VTK_DMML_EXPORT vtkDMMLTensorVolumeNode : public vtkDMMLScalarVolumeNode
{
  public:
  static vtkDMMLTensorVolumeNode *New();
  vtkTypeMacro(vtkDMMLTensorVolumeNode,vtkDMMLScalarVolumeNode);
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
  vtkDMMLCopyContentMacro(vtkDMMLTensorVolumeNode);

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "TensorVolume";}

  ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  void UpdateReferences() override
    { Superclass::UpdateReferences(); }

 /// Description:
 /// Update the stored reference to another node in the scene
  void UpdateReferenceID(const char *oldID, const char *newID) override
    { Superclass::UpdateReferenceID(oldID, newID); }

  ///
  vtkGetMacro(Order,int);
  vtkSetMacro(Order,int);

  ///
  /// Set the Measurement frame matrix from 3x3 array
  void SetMeasurementFrameMatrix(const double mf[3][3]);
  /// Description
  /// Set the measurement frame matrix from doubles
  void SetMeasurementFrameMatrix(const double xr, const double xa, const double xs,
                           const double yr, const double ya, const double ys,
                           const double zr, const double za, const double zs);

  void GetMeasurementFrameMatrix(double mf[3][3]);

  ///
  /// Set/Get the measurement frame matrix from a vtk 4x4 matrix
  void SetMeasurementFrameMatrix(vtkMatrix4x4 *mat);
  void GetMeasurementFrameMatrix(vtkMatrix4x4 *mat);

  ///
  /// Create default storage node or nullptr if does not have one
  vtkDMMLStorageNode* CreateDefaultStorageNode() override;

protected:
  vtkDMMLTensorVolumeNode();
  ~vtkDMMLTensorVolumeNode() override;
  vtkDMMLTensorVolumeNode(const vtkDMMLTensorVolumeNode&);
  void operator=(const vtkDMMLTensorVolumeNode&);

  double MeasurementFrameMatrix[3][3];
  int Order;
};

#endif
