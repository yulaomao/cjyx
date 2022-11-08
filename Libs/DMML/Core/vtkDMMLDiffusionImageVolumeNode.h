/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLVolumeNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkDMMLDiffusionImageVolumeNode_h
#define __vtkDMMLDiffusionImageVolumeNode_h

#include "vtkDMMLTensorVolumeNode.h"
class vtkDMMLDiffusionWeightedVolumeNode;

/// \brief DMML node for representing diffusion weighted MRI volume
///
/// Diffusion Weighted Volume nodes describe data sets that encode diffusion weighted
/// images. These images are the basis for computing the diffusion tensor.
/// The node is a container for the necessary information to interpert DW images:
/// 1. Gradient information.
/// 2. B value for each gradient.
/// 3. Measurement frame that relates the coordinate system where the gradients are given
///  to RAS.
class VTK_DMML_EXPORT vtkDMMLDiffusionImageVolumeNode : public vtkDMMLTensorVolumeNode
{
  public:
  static vtkDMMLDiffusionImageVolumeNode *New();
  vtkTypeMacro(vtkDMMLDiffusionImageVolumeNode,vtkDMMLTensorVolumeNode);
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
  vtkDMMLCopyContentDefaultMacro(vtkDMMLDiffusionImageVolumeNode);

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "DiffusionImageVolume";}

 /// Description:
  /// String ID of the storage DMML node
  void SetBaselineNodeID(const char* id);
  vtkGetStringMacro(BaselineNodeID);

  ///
  /// String ID of the display DMML node
  void SetMaskNodeID(const char* id);
  vtkGetStringMacro(MaskNodeID);

 /// Description:
  /// String ID of the display DMML node
  void SetDiffusionWeightedNodeID(const char* id);
  vtkGetStringMacro(DiffusionWeightedNodeID);

  ///
  /// Associated volume DMML node
  vtkDMMLVolumeNode* GetBaselineNode();

  ///
  /// Associated volume DMML node
  vtkDMMLVolumeNode* GetMaskNode();

  ///
  /// Associated volume DMML node
  vtkDMMLDiffusionWeightedVolumeNode* GetDiffusionWeightedNode();

  ///
  /// Associated volume DMML node
  //vtkDMMLDiffusionImageVolumeDisplayNode* GetDisplayNode();

  ///
  /// Update the stored reference to another node in the scene
  void UpdateReferenceID(const char *oldID, const char *newID) override;

   ///
  /// Finds the storage node and read the data
  //void UpdateScene(vtkDMMLScene *scene);

  ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  void UpdateReferences() override;


  ///
  /// alternative method to propagate events generated in Display nodes
  void ProcessDMMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;

  ///
  /// Create default storage node or nullptr if does not have one
  vtkDMMLStorageNode* CreateDefaultStorageNode() override
    {
    return Superclass::CreateDefaultStorageNode();
    }

protected:
  vtkDMMLDiffusionImageVolumeNode();
  ~vtkDMMLDiffusionImageVolumeNode() override;
  vtkDMMLDiffusionImageVolumeNode(const vtkDMMLDiffusionImageVolumeNode&);
  void operator=(const vtkDMMLDiffusionImageVolumeNode&);

  char *BaselineNodeID;
  char *MaskNodeID;
  char *DiffusionWeightedNodeID;

};

#endif
