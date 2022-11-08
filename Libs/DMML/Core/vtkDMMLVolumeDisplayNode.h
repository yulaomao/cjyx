/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLVolumeDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkDMMLVolumeDisplayNode_h
#define __vtkDMMLVolumeDisplayNode_h

// DMML includes
#include "vtkDMMLDisplayNode.h"
class vtkDMMLScene;
class vtkDMMLVolumeNode;

// VTK includes
class vtkAlgorithmOutput;
class vtkImageData;
class vtkImageStencilData;

/// \brief DMML node for representing a volume display attributes.
///
/// vtkDMMLVolumeDisplayNode nodes describe how volume is displayed.
class VTK_DMML_EXPORT vtkDMMLVolumeDisplayNode : public vtkDMMLDisplayNode
{
public:
  vtkTypeMacro(vtkDMMLVolumeDisplayNode,vtkDMMLDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///
  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkDMMLNode::CopyContent
  vtkDMMLCopyContentMacro(vtkDMMLVolumeDisplayNode);

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override = 0;

  ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  void UpdateReferences() override;

  ///
  /// Finds the storage node and read the data
  void UpdateScene(vtkDMMLScene *scene) override;

  ///
  /// Sets ImageData for background mask
  /// Must be reimplemented in deriving class if they need it.
  /// GetBackgroundImageStencilDataConnection() returns 0 if the background image data
  /// is not used.
  virtual void SetBackgroundImageStencilDataConnection(vtkAlgorithmOutput * imageDataConnection);
  virtual vtkAlgorithmOutput* GetBackgroundImageStencilDataConnection();
  virtual vtkImageStencilData* GetBackgroundImageStencilData();

  /// Returns the output of the pipeline if there is a not a null input.
  /// Gets ImageData converted from the real data in the node
  /// The image is the direct output of the pipeline, it might not be
  /// up-to-date. You can call Update() on the returned vtkImageData or use
  /// GetUpToDateImageData() instead.
  /// \sa GetUpToDateImageData()
  virtual vtkAlgorithmOutput* GetImageDataConnection();

  /// Gets ImageData and ensure it's up-to-date by calling Update() on the
  /// pipeline.
  /// Please note that it can be slow, depending on the filters in
  /// the pipeline and the dimension of the input data.

  /// Set the pipeline input.
  /// Filters can be applied to the input image data. The output image data
  /// is the one used by the mappers.
  /// It internally calls SetInputImageDataPipeline that can be reimplemented.
  virtual void SetInputImageDataConnection(vtkAlgorithmOutput *imageDataConnection);
  virtual vtkAlgorithmOutput* GetInputImageDataConnection();

  /// Gets the pipeline input. To be reimplemented in subclasses.
  virtual vtkImageData* GetInputImageData();

  /// Gets the pipeline output. To be reimplemented in subclasses.
  virtual vtkImageData* GetOutputImageData();
  virtual vtkAlgorithmOutput* GetOutputImageDataConnection();

  ///
  /// Update the pipeline based on this node attributes
  virtual void UpdateImageDataPipeline();

  ///
  /// alternative method to propagate events generated in Display nodes
  void ProcessDMMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;
  ///
  /// set gray colormap or override in subclass
  virtual void SetDefaultColorMap();

  /// Search in the scene the volume node vtkDMMLVolumeDisplayNode is associated
  /// to
  vtkDMMLVolumeNode* GetVolumeNode();

protected:
  vtkDMMLVolumeDisplayNode();
  ~vtkDMMLVolumeDisplayNode() override;
  vtkDMMLVolumeDisplayNode(const vtkDMMLVolumeDisplayNode&);
  void operator=(const vtkDMMLVolumeDisplayNode&);

  virtual void SetInputToImageDataPipeline(vtkAlgorithmOutput *imageDataConnection);
};

#endif
