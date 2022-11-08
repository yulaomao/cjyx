/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLDiffusionWeightedVolumeDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkDMMLDiffusionWeightedVolumeDisplayNode_h
#define __vtkDMMLDiffusionWeightedVolumeDisplayNode_h

// DMML includes
#include "vtkDMMLScalarVolumeDisplayNode.h"

// VTK includes
class vtkAlgorithmOutput;
class vtkImageData;
class vtkImageExtractComponents;

/// \brief DMML node for representing a volume (image stack).
///
/// Volume nodes describe data sets that can be thought of as stacks of 2D
/// images that form a 3D volume.  Volume nodes describe where the images
/// are stored on disk, how to render the data (window and level), and how
/// to read the files.  This information is extracted from the image
/// headers (if they exist) at the time the DMML file is generated.
/// Consequently, DMML files isolate DMML browsers from understanding how
/// to read the myriad of file formats for medical data.
class VTK_DMML_EXPORT vtkDMMLDiffusionWeightedVolumeDisplayNode : public vtkDMMLScalarVolumeDisplayNode
{
  public:
  static vtkDMMLDiffusionWeightedVolumeDisplayNode *New();
  vtkTypeMacro(vtkDMMLDiffusionWeightedVolumeDisplayNode,vtkDMMLScalarVolumeDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  ///
  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Copy the node's attributes to this object
  void Copy(vtkDMMLNode *node) override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "DiffusionWeightedVolumeDisplay";}

  ///
  /// Get the pipeline input
  vtkAlgorithmOutput* GetInputImageDataConnection() override;


  void UpdateImageDataPipeline() override;

  //--------------------------------------------------------------------------
  /// Display Information
  //--------------------------------------------------------------------------

  ///
  /// Set/Get interpolate reformated slices
  vtkGetMacro(DiffusionComponent, int);
  vtkSetMacro(DiffusionComponent, int);

protected:
  vtkDMMLDiffusionWeightedVolumeDisplayNode();
  ~vtkDMMLDiffusionWeightedVolumeDisplayNode() override;
  vtkDMMLDiffusionWeightedVolumeDisplayNode(const vtkDMMLDiffusionWeightedVolumeDisplayNode&);
  void operator=(const vtkDMMLDiffusionWeightedVolumeDisplayNode&);

  ///
  /// Set the input of the pipeline
  void SetInputToImageDataPipeline(vtkAlgorithmOutput *imageDataConnection) override;

  vtkAlgorithmOutput* GetScalarImageDataConnection() override;

  /// This property holds the current diffusion component used for display.
  int DiffusionComponent;
  vtkImageExtractComponents *ExtractComponent;
};

#endif

