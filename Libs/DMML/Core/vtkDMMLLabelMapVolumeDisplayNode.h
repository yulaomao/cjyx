/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLLabelMapVolumeDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkDMMLLabelMapVolumeDisplayNode_h
#define __vtkDMMLLabelMapVolumeDisplayNode_h

#include "vtkDMMLVolumeDisplayNode.h"

class vtkImageAlgorithm;
class vtkImageMapToColors;

/// \brief DMML node for representing a volume display attributes.
///
/// vtkDMMLLabelMapVolumeDisplayNode nodes describe how volume is displayed.
class VTK_DMML_EXPORT vtkDMMLLabelMapVolumeDisplayNode : public vtkDMMLVolumeDisplayNode
{
  public:
  static vtkDMMLLabelMapVolumeDisplayNode *New();
  vtkTypeMacro(vtkDMMLLabelMapVolumeDisplayNode,vtkDMMLVolumeDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "LabelMapVolumeDisplay";}

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkDMMLNode::CopyContent
  vtkDMMLCopyContentDefaultMacro(vtkDMMLLabelMapVolumeDisplayNode);

  ///
  /// alternative method to propagate events generated in Display nodes
  void ProcessDMMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;
  ///
  /// set default labels colormap
  void SetDefaultColorMap() override;

  /// Set the pipeline input
  void SetInputImageDataConnection(vtkAlgorithmOutput *imageDataConnection) override;

  /// Get the pipeline input
  vtkImageData* GetInputImageData() override;

  /// Gets the pipeline output
  vtkAlgorithmOutput* GetOutputImageDataConnection() override;

  void UpdateImageDataPipeline() override;

protected:
  vtkDMMLLabelMapVolumeDisplayNode();
  ~vtkDMMLLabelMapVolumeDisplayNode() override;
  vtkDMMLLabelMapVolumeDisplayNode(const vtkDMMLLabelMapVolumeDisplayNode&);
  void operator=(const vtkDMMLLabelMapVolumeDisplayNode&);

  vtkImageMapToColors *MapToColors;

};

#endif
