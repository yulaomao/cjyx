/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLSliceLayerLogic.h,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

///  vtkDMMLSliceLayerLogic - cjyx logic class for slice manipulation
///
/// This class manages the logic associated with reslicing of volumes
/// (but not the GUI).  Features of the class include:
//
/// - Reslicing
/// -- uses the vtkImageData and IJKToRAS transform from a vtkDMMLVolumeNode
/// -- disp
/// -- uses a current slice view specification (typically set by vtkDMMLSliceLogic)
/// - Outputs
/// -- Colors vtkImageData for the given slice
/// -- image is mapped through current window/level and lookup table
//
/// This class can also be used for resampling volumes for further computation.
//

#ifndef __vtkDMMLSliceLayerLogic_h
#define __vtkDMMLSliceLayerLogic_h

// DMMLLogic includes
#include "vtkDMMLAbstractLogic.h"

// DMML includes
#include "vtkDMMLVolumeNode.h"
#include "vtkDMMLSliceNode.h"
#include "vtkDMMLScalarVolumeNode.h"
#include "vtkDMMLVectorVolumeNode.h"
#include "vtkDMMLDiffusionWeightedVolumeNode.h"
#include "vtkDMMLDiffusionTensorVolumeNode.h"

// VTK includes
#include <vtkImageLogic.h>
#include <vtkImageExtractComponents.h>
#include <vtkVersion.h>

class vtkAssignAttribute;
class vtkImageReslice;
class vtkGeneralTransform;

// STL includes
//#include <cstdlib>

class vtkImageLabelOutline;
class vtkTransform;

class VTK_DMML_LOGIC_EXPORT vtkDMMLSliceLayerLogic
  : public vtkDMMLAbstractLogic
{
public:

  /// The Usual vtk class functions
  static vtkDMMLSliceLayerLogic *New();
  vtkTypeMacro(vtkDMMLSliceLayerLogic,vtkDMMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///
  /// The volume node to operate on
  vtkGetObjectMacro (VolumeNode, vtkDMMLVolumeNode);
  void SetVolumeNode (vtkDMMLVolumeNode *VolumeNode);

  ///
  /// The volume display node has the render properties of the volume
  /// - this node is set implicitly when the volume is set
  ///   and it is observed by this logic
  vtkGetObjectMacro (VolumeDisplayNode, vtkDMMLVolumeDisplayNode);
  vtkGetObjectMacro (VolumeDisplayNodeUVW, vtkDMMLVolumeDisplayNode);

  ///
  /// The slice node that defines the view
  vtkGetObjectMacro (SliceNode, vtkDMMLSliceNode);
  void SetSliceNode (vtkDMMLSliceNode *SliceNode);

  ///
  /// The image reslice or slice being used
  vtkGetObjectMacro (Reslice, vtkImageReslice);
  vtkGetObjectMacro (ResliceUVW, vtkImageReslice);

  ///
  /// Select if this is a label layer or not (it currently determines if we use
  /// the label outline filter)
  vtkGetMacro (IsLabelLayer, int);
  vtkSetMacro (IsLabelLayer, int);
  vtkBooleanMacro (IsLabelLayer, int);

  ///
  /// The filter that turns the label map into an outline
  vtkGetObjectMacro (LabelOutline, vtkImageLabelOutline);

  ///
  /// Get the output of the pipeline for this layer
  vtkImageData *GetImageData();
  vtkAlgorithmOutput *GetImageDataConnection();

  ///
  /// Get the output of the texture UVW pipeline for this layer
  vtkImageData *GetImageDataUVW();
  vtkAlgorithmOutput *GetImageDataConnectionUVW();

  void UpdateImageDisplay();

  ///
  /// set the Reslice transforms to reflect the current state
  /// of the VolumeNode and the SliceNode
  void UpdateTransforms();

  void UpdateGlyphs();


  ///
  /// Check that we are observing the correct display node
  /// (correct means the same one that the volume node is referencing)
  void UpdateNodeReferences();

  ///
  /// The current reslice transform XYToIJK
  vtkGetObjectMacro (XYToIJKTransform, vtkGeneralTransform);

  ///
  /// Get/set interpolation mode used in image recjyx (when interpolation is enabled).
  /// By default it uses VTK_RESLICE_LINEAR and can be set to VTK_RESLICE_CUBIC for higher quality interpolation.
  vtkGetMacro(InterpolationMode, int);
  vtkSetMacro(InterpolationMode, int);

protected:
  vtkDMMLSliceLayerLogic();
  ~vtkDMMLSliceLayerLogic() override;
  vtkDMMLSliceLayerLogic(const vtkDMMLSliceLayerLogic&);
  void operator=(const vtkDMMLSliceLayerLogic&);

  // Initialize listening to DMML events
  void SetDMMLSceneInternal(vtkDMMLScene * newScene) override;

  ///
  /// provide the virtual method that updates this Logic based
  /// on dmml changes
  void ProcessDMMLSceneEvents(vtkObject* caller,
                                      unsigned long event,
                                      void* callData) override;
  void ProcessDMMLNodesEvents(vtkObject* caller,
                                      unsigned long event,
                                      void* callData) override;
  void UpdateLogic();
  void OnDMMLNodeModified(vtkDMMLNode* node) override;
  vtkAlgorithmOutput* GetSliceImageDataConnection();
  vtkAlgorithmOutput* GetSliceImageDataConnectionUVW();

  // Copy VolumeDisplayNodeObserved into VolumeDisplayNode
  void UpdateVolumeDisplayNode();

  ///
  /// the DMML Nodes that define this Logic's parameters
  vtkDMMLVolumeNode *VolumeNode;
  vtkDMMLVolumeDisplayNode *VolumeDisplayNode;
  vtkDMMLVolumeDisplayNode *VolumeDisplayNodeUVW;
  vtkDMMLVolumeDisplayNode *VolumeDisplayNodeObserved;
  vtkDMMLSliceNode *SliceNode;

  ///
  /// the VTK class instances that implement this Logic's operations
  vtkImageReslice *Reslice;
  vtkImageReslice *ResliceUVW;
  vtkImageLabelOutline *LabelOutline;
  vtkImageLabelOutline *LabelOutlineUVW;

  vtkAssignAttribute* AssignAttributeTensorsToScalars;
  vtkAssignAttribute* AssignAttributeScalarsToTensors;
  vtkAssignAttribute* AssignAttributeScalarsToTensorsUVW;

  /// TODO: make this a vtkAbstractTransform for non-linear
  vtkGeneralTransform *XYToIJKTransform;
  vtkGeneralTransform *UVWToIJKTransform;

  int IsLabelLayer;

  int UpdatingTransforms;

  int InterpolationMode;
};

#endif

