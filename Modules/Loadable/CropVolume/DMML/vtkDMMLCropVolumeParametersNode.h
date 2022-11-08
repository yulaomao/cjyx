/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/
// .NAME vtkDMMLVolumeRenderingParametersNode - DMML node for storing a slice through RAS space
// .SECTION Description
// This node stores the information about the currently selected volume
//
//

#ifndef __vtkDMMLCropVolumeParametersNode_h
#define __vtkDMMLCropVolumeParametersNode_h

#include "vtkDMML.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLNode.h"
#include "vtkCjyxCropVolumeModuleDMMLExport.h"

class vtkDMMLDisplayableNode;
class vtkDMMLTransformNode;
class vtkDMMLVolumeNode;

/// \ingroup Cjyx_QtModules_CropVolume
class VTK_CJYX_CROPVOLUME_MODULE_DMML_EXPORT vtkDMMLCropVolumeParametersNode : public vtkDMMLNode
{
public:
  enum
    {
    InterpolationNearestNeighbor = 1,
    InterpolationLinear = 2,
    InterpolationWindowedSinc = 3,
    InterpolationBSpline = 4
    };

  static vtkDMMLCropVolumeParametersNode *New();
  vtkTypeMacro(vtkDMMLCropVolumeParametersNode,vtkDMMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  /// Set node attributes from XML attributes
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkDMMLNode::CopyContent
  vtkDMMLCopyContentMacro(vtkDMMLCropVolumeParametersNode);

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "CropVolumeParameters";}

  /// Set volume node to be cropped
  void SetInputVolumeNodeID(const char *nodeID);
  /// Get volume node to be cropped
  const char *GetInputVolumeNodeID();
  vtkDMMLVolumeNode* GetInputVolumeNode();

  /// Set resulting cropped volume node
  void SetOutputVolumeNodeID(const char *nodeID);
  /// Get resulting cropped volume node
  const char* GetOutputVolumeNodeID();
  vtkDMMLVolumeNode* GetOutputVolumeNode();

  /// Set cropping region of interest.
  /// It can be either vtkDMMLAnnotationROINode or vtkDMMLMarkupsROINode.
  void SetROINodeID(const char *nodeID);
  /// Get cropping region of interest
  const char* GetROINodeID();
  vtkDMMLDisplayableNode* GetROINode();

  /// Set transform node that may be used for aligning
  /// the ROI with the input volume.
  void SetROIAlignmentTransformNodeID(const char *nodeID);
  const char* GetROIAlignmentTransformNodeID();
  vtkDMMLTransformNode* GetROIAlignmentTransformNode();
  void DeleteROIAlignmentTransformNode();

  vtkSetMacro(IsotropicResampling,bool);
  vtkGetMacro(IsotropicResampling,bool);
  vtkBooleanMacro(IsotropicResampling,bool);

  vtkSetMacro(VoxelBased,bool);
  vtkGetMacro(VoxelBased,bool);
  vtkBooleanMacro(VoxelBased,bool);

  vtkSetMacro(InterpolationMode, int);
  vtkGetMacro(InterpolationMode, int);

  vtkSetMacro(SpacingScalingConst, double);
  vtkGetMacro(SpacingScalingConst, double);

  vtkSetMacro(FillValue, double);
  vtkGetMacro(FillValue, double);

protected:
  vtkDMMLCropVolumeParametersNode();
  ~vtkDMMLCropVolumeParametersNode() override;

  vtkDMMLCropVolumeParametersNode(const vtkDMMLCropVolumeParametersNode&);
  void operator=(const vtkDMMLCropVolumeParametersNode&);

  bool VoxelBased;
  int InterpolationMode;
  bool IsotropicResampling;
  double SpacingScalingConst;
  double FillValue;
};

#endif

