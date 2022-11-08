/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLVolumeNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkDMMLDiffusionTensorVolumeNode_h
#define __vtkDMMLDiffusionTensorVolumeNode_h

#include "vtkDMMLDiffusionImageVolumeNode.h"

class vtkDMMLDiffusionTensorVolumeDisplayNode;

/// \brief DMML node for representing diffusion weighted MRI volume.
///
/// Diffusion Weighted Volume nodes describe data sets that encode diffusion weighted
/// images. These images are the basis for computing the diffusion tensor.
/// The node is a container for the necessary information to interpert DW images:
/// 1. Gradient information.
/// 2. B value for each gradient.
/// 3. Measurement frame that relates the coordinate system where the gradients are given
///  to RAS.
class VTK_DMML_EXPORT vtkDMMLDiffusionTensorVolumeNode : public vtkDMMLDiffusionImageVolumeNode
{
  public:
  static vtkDMMLDiffusionTensorVolumeNode *New();
  vtkTypeMacro(vtkDMMLDiffusionTensorVolumeNode,vtkDMMLDiffusionImageVolumeNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override { return "DiffusionTensorVolume"; }

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkDMMLNode::CopyContent
  vtkDMMLCopyContentDefaultMacro(vtkDMMLDiffusionTensorVolumeNode);

  /// Associated volume display DMML node
  virtual void SetAndObserveDisplayNodeID(const char *DisplayNodeID);

  /// Associated display DMML node
  virtual vtkDMMLDiffusionTensorVolumeDisplayNode* GetDiffusionTensorVolumeDisplayNode();

  /// Create default storage node or nullptr if does not have one
  vtkDMMLStorageNode* CreateDefaultStorageNode() override;

  /// Create and observe default display node
  void CreateDefaultDisplayNodes() override;

protected:
  vtkDMMLDiffusionTensorVolumeNode();
  ~vtkDMMLDiffusionTensorVolumeNode() override;

  vtkDMMLDiffusionTensorVolumeNode(const vtkDMMLDiffusionTensorVolumeNode&);
  void operator=(const vtkDMMLDiffusionTensorVolumeNode&);

};

#endif
