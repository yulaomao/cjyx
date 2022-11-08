/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLSliceLogic.h,v $
  Date:      $Date$
  Version:   $Revision: 18866

=========================================================================auto=*/

#ifndef __vtkDMMLSliceLogic_h
#define __vtkDMMLSliceLogic_h

// DMMLLogic includes
#include "vtkDMMLAbstractLogic.h"

// STD includes
#include <vector>
#include <deque>

class vtkDMMLDisplayNode;
class vtkDMMLLinearTransformNode;
class vtkDMMLModelDisplayNode;
class vtkDMMLModelNode;
class vtkDMMLSliceCompositeNode;
class vtkDMMLSliceDisplayNode;
class vtkDMMLSliceLayerLogic;
class vtkDMMLSliceNode;
class vtkDMMLVolumeNode;

class vtkAlgorithmOutput;
class vtkCollection;
class vtkImageBlend;
class vtkTransform;
class vtkImageData;
class vtkImageReslice;
class vtkTransform;

struct SliceLayerInfo;
struct BlendPipeline;

/// \brief Cjyx logic class for slice manipulation.
///
/// This class manages the logic associated with display of slice windows
/// (but not the GUI).  Features of the class include:
///  -- a back-to-front list of DmmlVolumes to be displayed
///  -- a compositing mode for each volume layer (opacity, outline, glyph, checkerboard, etc)
///  -- each layer is required to provide an RGBA image in the space defined by the vtkDMMLSliceNode
///
/// This class manages internal vtk pipelines that create an output vtkImageData
/// which can be used by the vtkCjyxSliceGUI class to display the resulting
/// composite image or it can be used as a texture map in a vtkCjyxView.
/// This class can also be used for resampling volumes for further computation.
class VTK_DMML_LOGIC_EXPORT vtkDMMLSliceLogic : public vtkDMMLAbstractLogic
{
public:
  /// The Usual VTK class functions
  static vtkDMMLSliceLogic *New();
  vtkTypeMacro(vtkDMMLSliceLogic,vtkDMMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// CompositeModifiedEvent is generated when slice composite node is modified
  enum
    {
    CompositeModifiedEvent = 18000
    };

  enum
    {
    LayerNone = -1,
    LayerBackground = 0,
    LayerForeground = 1,
    LayerLabel = 2
    };

  ///
  /// The DMML slice node for this slice logic
  vtkGetObjectMacro (SliceNode, vtkDMMLSliceNode);
  void SetSliceNode (vtkDMMLSliceNode * newSliceNode);

  /// Convenience function for adding a slice node and setting it in this logic
  vtkDMMLSliceNode* AddSliceNode(const char* layoutName);

  ///
  /// The DMML slice node for this slice logic
  vtkGetObjectMacro (SliceCompositeNode, vtkDMMLSliceCompositeNode);
  void SetSliceCompositeNode (vtkDMMLSliceCompositeNode *SliceCompositeNode);

  ///
  /// The background slice layer
  /// TODO: this will eventually be generalized to a list of layers
  vtkGetObjectMacro (BackgroundLayer, vtkDMMLSliceLayerLogic);
  void SetBackgroundLayer (vtkDMMLSliceLayerLogic *BackgroundLayer);

  ///
  /// The foreground slice layer
  /// TODO: this will eventually be generalized to a list of layers
  vtkGetObjectMacro (ForegroundLayer, vtkDMMLSliceLayerLogic);
  void SetForegroundLayer (vtkDMMLSliceLayerLogic *ForegroundLayer);

  ///
  /// The Label slice layer
  /// TODO: this will eventually be generalized to a list of layers
  vtkGetObjectMacro (LabelLayer, vtkDMMLSliceLayerLogic);
  void SetLabelLayer (vtkDMMLSliceLayerLogic *LabelLayer);

  ///
  /// Helper to set the background layer Window/Level
  void SetBackgroundWindowLevel(double window, double level);

  ///
  /// Helper to get the background layer Window/Level, intensity range and
  /// status of automatic Window/Level setting
  void GetBackgroundWindowLevelAndRange(double& window, double& level,
                                      double& rangeLow, double& rangeHigh, bool& autoWindowLevel);

  ///
  /// Helper to get the background layer Window/Level and intensity range
  void GetBackgroundWindowLevelAndRange(double& window, double& level,
                                      double& rangeLow, double& rangeHigh);

  ///
  /// Helper to set the foreground layer Window/Level
  void SetForegroundWindowLevel(double window, double level);

  ///
  /// Helper to get the foreground layer Window/Level, intensity range and
  /// status of automatic Window/Level setting
  void GetForegroundWindowLevelAndRange(double& window, double& level,
                                      double& rangeLow, double& rangeHigh, bool& autoWindowLevel);

  ///
  /// Helper to get the foreground layer Window/Level and intensity range
  void GetForegroundWindowLevelAndRange(double& window, double& level,
                                      double& rangeLow, double& rangeHigh);
  ///
  /// Model slice plane
  vtkGetObjectMacro(SliceModelNode, vtkDMMLModelNode);

  ///
  /// Model slice plane display properties.
  /// The method is deprecated, use SliceDisplayNode instead.
  vtkGetObjectMacro(SliceModelDisplayNode, vtkDMMLModelDisplayNode);

  ///
  /// Slice plane display properties
  vtkDMMLSliceDisplayNode* GetSliceDisplayNode();

  ///
  /// Model slice plane transform from xy to RAS
  vtkGetObjectMacro(SliceModelTransformNode, vtkDMMLLinearTransformNode);

  ///
  /// The compositing filter
  /// TODO: this will eventually be generalized to a per-layer compositing function
  vtkImageBlend* GetBlend();
  vtkImageBlend* GetBlendUVW();

  ///
  /// An image reslice instance to pull a single slice from the volume that
  /// represents the filmsheet display output
  vtkGetObjectMacro(ExtractModelTexture, vtkImageReslice);

  ///
  /// the tail of the pipeline
  /// -- returns nullptr if none of the inputs exist
  vtkAlgorithmOutput *GetImageDataConnection();

  ///
  /// update the pipeline to reflect the current state of the nodes
  void UpdatePipeline();

  /// Internally used by UpdatePipeline
  void UpdateImageData();

  /// Reimplemented to avoir calling ProcessDMMLSceneEvents when we are added the
  /// DMMLModelNode into the scene
  virtual bool EnterDMMLCallback()const;

  ///
  /// Manage and synchronise the SliceNode
  void UpdateSliceNode();

  ///
  /// Update cjyx node given a layout name
  void UpdateSliceNodeFromLayout();

  ///
  /// Manage and synchronise the SliceCompositeNode
  void UpdateSliceCompositeNode();

  ///
  /// Get the volume node corresponding to layer
  /// (0=background, 1=foreground, 2=label)
  vtkDMMLVolumeNode *GetLayerVolumeNode(int layer);

  ///
  /// Get the size of the volume, transformed to RAS space
  static void GetVolumeRASBox(vtkDMMLVolumeNode *volumeNode, double rasDimensions[3], double rasCenter[3]);

  ///
  /// Get the size of the volume, transformed to slice space
  void GetVolumeSliceDimensions(vtkDMMLVolumeNode *volumeNode, double sliceDimensions[3], double sliceCenter[3]);

  ///
  /// Get the spacing of the volume, transformed to slice space
  /// - to be used, for example, to set the slice increment for stepping a single
  ///   voxel relative to the current slice view
  double *GetVolumeSliceSpacing(vtkDMMLVolumeNode *volumeNode);

  ///
  /// Get the min/max bounds of the volume
  /// - note these are not translated by the current slice offset so they can
  ///   be used to calculate the range (e.g. of a slider) that operates in slice space
  /// If useVoxelCenter is set to false (default) then bounds of voxel sides are returned
  /// (otherwise then bounds of voxels centers are returned).
  void GetVolumeSliceBounds(vtkDMMLVolumeNode *volumeNode, double sliceBounds[6], bool useVoxelCenter=false);

  ///
  /// adjust the node's field of view to match the extent of current background volume
  void FitSliceToVolume(vtkDMMLVolumeNode *volumeNode, int width, int height);

  ///
  /// Get the size of the volume, transformed to RAS space
  void GetBackgroundRASBox(double rasDimensions[3], double rasCenter[3]);

  ///
  /// Get the size of the volume, transformed to slice space
  void GetBackgroundSliceDimensions(double sliceDimensions[3], double sliceCenter[3]);

  ///
  /// Get the spacing of the volume, transformed to slice space
  /// - to be used, for example, to set the slice increment for stepping a single
  ///   voxel relative to the current slice view
  double *GetBackgroundSliceSpacing();

  ///
  /// Get the min/max bounds of the volume
  /// - note these are not translated by the current slice offset so they can
  ///   be used to calculate the range (e.g. of a slider) that operates in slice space
  void GetBackgroundSliceBounds(double sliceBounds[6]);

  /// Rotate slice view to match axes of the lowest volume layer (background, foreground, label).
  /// \param forceSlicePlaneToSingleSlice If the volume is single-slice and forceSlicePlaneToSingleSlice
  /// is enabled then slice view will be aligned with the volume's slice plane. If the flag is disabled
  /// of the volume has more than one slice then the slice view will be rotated to the closest orthogonal axis.
  void RotateSliceToLowestVolumeAxes(bool forceSlicePlaneToSingleSlice = true);

  ///
  /// adjust the node's field of view to match the extent of current background volume
  void FitSliceToBackground(int width, int height);

  ///
  /// adjust the node's field of view to match the extent of all volume layers
  ///  (fits to first non-null layer)
  void FitSliceToAll(int width = -1, int height = -1);

  /// adjust the node's field of view to match the FOV
  /// the value fov will be applied to the smallest slice window dimension
  void FitFOVToBackground(double fov);

  /// Adjust dimensions and fov based on the new viewport size.
  /// The size should be the viewport size (typically vtkRenderWindow), not the
  /// size of the renderers (important if it's in a lightbox mode).
  /// It must be called each time the renderwindow size is modified and each
  /// time the lightbox configuration is changed.
  void ResizeSliceNode(double newWidth, double newHeight);

  ///
  /// Get the spacing of the lowest volume layer (background, foreground, label),
  /// transformed to slice space
  /// - to be used, for example, to set the slice increment for stepping a single
  ///   voxel relative to the current slice view
  /// - returns first non-null layer
  double *GetLowestVolumeSliceSpacing();

  ///
  /// Get the min/max bounds of the lowest volume layer (background, foreground, label)
  /// - note these are not translated by the current slice offset so they can
  ///   be used to calculate the range (e.g. of a slider) that operates in slice space
  /// - returns first non-null layer
  /// If useVoxelCenter is set to false (default) then bounds of voxel sides are returned
  /// (otherwise then bounds of voxels centers are returned).
  void GetLowestVolumeSliceBounds(double sliceBounds[6], bool useVoxelCenter=false);

  ///
  /// Get/Set the current distance from the origin to the slice plane
  double GetSliceOffset();
  void SetSliceOffset(double offset);

  ///
  /// Get the largest slice bounding box for all volumes in layers
  void GetSliceBounds(double sliceBounds[6]);

  ///
  /// Set slice extents to all layers
  void SetSliceExtentsToSliceNode();

  /// Indicate an interaction with the slice node is beginning. The
  /// parameters of the slice node being manipulated are passed as a
  /// bitmask. See vtkDMMLSliceNode::InteractionFlagType.
  void StartSliceNodeInteraction(unsigned int parameters);

  /// Indicate an interaction with the slice node has been completed
  void EndSliceNodeInteraction();

  /// Indicate an interaction with the slice composite node is
  /// beginning. The parameters of the slice node being manipulated
  /// are passed as a bitmask. See vtkDMMLSliceNode::InteractionFlagType.
  void StartSliceCompositeNodeInteraction(unsigned int parameters);

  /// Indicate an interaction with the slice composite node has been completed
  void EndSliceCompositeNodeInteraction();

  /// Indicate the slice offset value is starting to change
  void StartSliceOffsetInteraction();

  /// Indicate the slice offset value has completed its change
  void EndSliceOffsetInteraction();

  ///
  /// Set the current distance so that it corresponds to the closest center of
  /// a voxel in IJK space (integer value)
  void SnapSliceOffsetToIJK();

  static const int SLICE_INDEX_ROTATED;
  static const int SLICE_INDEX_OUT_OF_VOLUME;
  static const int SLICE_INDEX_NO_VOLUME;

  /// Get the DICOM slice index (1-based) from slice offset (distance from the origin to the slice plane).
  /// If the return value is negative then then no slice index can be determined:
  /// SLICE_INDEX_ROTATED=the slice is rotated compared to the volume planes,
  /// SLICE_INDEX_OUT_OF_VOLUME=the slice plane is out of the volume
  /// SLICE_INDEX_NO_VOLUME=the specified volume is not available
  int GetSliceIndexFromOffset(double sliceOffset, vtkDMMLVolumeNode *volumeNode);

  /// Get the DICOM slice index (1-based) from slice offset (distance from the origin to the slice plane).
  /// Slice index is computed for the first available volume (the search order is
  /// background, foreground, label volume).
  /// If the return value is negative then then no slice index can be determined for the
  /// first available volume:
  /// SLICE_INDEX_ROTATED=the slice is rotated compared to the volume planes,
  /// SLICE_INDEX_OUT_OF_VOLUME=the slice plane is out of the volume
  /// SLICE_INDEX_NO_VOLUME=no volume is available
  int GetSliceIndexFromOffset(double sliceOffset);

  ///
  /// Make a slice model with the current configuration
  void CreateSliceModel();
  void DeleteSliceModel();

  ///
  /// Get  all slice displaynodes creating PolyData models like glyphs etc.
  std::vector< vtkDMMLDisplayNode*> GetPolyDataDisplayNodes();
  /// Return the associated cjyxlayer nodes
  static vtkDMMLSliceCompositeNode* GetSliceCompositeNode(vtkDMMLSliceNode* node);
  /// Return the associated slice node
  static vtkDMMLSliceNode* GetSliceNode(vtkDMMLSliceCompositeNode* node);

  /// Default node name suffix for use with volume slice models to distinguish them
  /// as built in models rather than user accessible.
  /// \sa IsSliceModelNode
  static const std::string SLICE_MODEL_NODE_NAME_SUFFIX;

  /// Return true if the node is a model node that has the default volume slice
  /// node name suffix, false otherwise
  /// \sa SLICE_MODEL_NODE_NAME_SUFFIX
  static bool IsSliceModelNode(vtkDMMLNode *dmmlNode);
  /// Return true if the display node is a volume slice node display node
  /// by checking the attribute SliceLogic.IsSliceModelDiplayNode
  /// Returns false if the attribute is not present, true if the attribute
  /// is present and not equal to zero
  static bool IsSliceModelDisplayNode(vtkDMMLDisplayNode *dmmlDisplayNode);

  /// Get volume at the specified world position that should be used
  /// for interactions, such as window/level adjustments.
  /// backgroundVolumeEditable and foregroundVolumeEditable can be used specify that
  /// a volume is not editable (even if it is visible at the given position).
  int GetEditableLayerAtWorldPosition(double worldPos[3], bool backgroundVolumeEditable = true, bool foregroundVolumeEditable = true);

protected:

  vtkDMMLSliceLogic();
  ~vtkDMMLSliceLogic() override;

  void SetDMMLSceneInternal(vtkDMMLScene * newScene) override;

  ///
  /// process logic events
  void ProcessDMMLLogicsEvents(vtkObject * caller,
                                       unsigned long event,
                                       void * callData) override;
  void ProcessDMMLLogicsEvents();

  void OnDMMLSceneNodeAdded(vtkDMMLNode* node) override;
  void OnDMMLSceneNodeRemoved(vtkDMMLNode* node) override;
  void UpdateFromDMMLScene() override;
  void OnDMMLSceneStartClose() override;
  void OnDMMLSceneEndImport() override;
  void OnDMMLSceneEndRestore() override;

  void UpdateSliceNodes();
  void SetupCrosshairNode();

  void OnDMMLNodeModified(vtkDMMLNode* node) override;
  static vtkDMMLSliceCompositeNode* GetSliceCompositeNode(vtkDMMLScene* scene,
                                                          const char* layoutName);
  static vtkDMMLSliceNode* GetSliceNode(vtkDMMLScene* scene,
    const char* layoutName);

  ///
  /// Helper to set Window/Level in any layer
  void SetWindowLevel(double window, double level, int layer);

  /// Helper to update input of blend filter from a set of layers.
  /// It minimizes changes to the imaging pipeline (does not remove and
  /// re-add an input if it is not changed) because rebuilding of the pipeline
  /// is a relatively expensive operation.
  bool UpdateBlendLayers(vtkImageBlend* blend, const std::deque<SliceLayerInfo> &layers);

  /// Returns true if position is inside the selected layer volume.
  /// Use background flag to choose between foreground/background layer.
  bool IsEventInsideVolume(bool background, double worldPos[3]);

  /// Returns true if the volume's window/level values are editable on the GUI.
  bool VolumeWindowLevelEditable(const char* volumeNodeID);

  bool                        AddingSliceModelNodes;

  vtkDMMLSliceNode *          SliceNode;
  vtkDMMLSliceCompositeNode * SliceCompositeNode;
  vtkDMMLSliceLayerLogic *    BackgroundLayer;
  vtkDMMLSliceLayerLogic *    ForegroundLayer;
  vtkDMMLSliceLayerLogic *    LabelLayer;

  BlendPipeline* Pipeline;
  BlendPipeline* PipelineUVW;
  vtkImageReslice * ExtractModelTexture;
  vtkAlgorithmOutput *    ImageDataConnection;

  vtkDMMLModelNode *            SliceModelNode;
  vtkDMMLModelDisplayNode *     SliceModelDisplayNode;
  vtkDMMLLinearTransformNode *  SliceModelTransformNode;
  double                        SliceSpacing[3];

private:

  vtkDMMLSliceLogic(const vtkDMMLSliceLogic&) = delete;
  void operator=(const vtkDMMLSliceLogic&) = delete;

};

#endif
