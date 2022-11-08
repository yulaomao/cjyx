/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  This file was partly developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

=========================================================================auto=*/

///  vtkCjyxTransformLogic - cjyx logic class for volumes manipulation
///
/// This class manages the logic associated with reading, saving,
/// and changing propertied of the volumes

#ifndef __vtkCjyxTransformLogic_h
#define __vtkCjyxTransformLogic_h

// CjyxLogic includes
#include "vtkCjyxBaseLogic.h"
#include "vtkCjyxTransformsModuleLogicExport.h"

// DMMLLogic includes
#include <vtkDMMLAbstractLogic.h>

// STD includes
#include <vector>

// DMML includes
class vtkDMMLDisplayableNode;
class vtkDMMLMarkupsNode;
class vtkDMMLScalarVolumeNode;
class vtkDMMLScene;
class vtkDMMLSliceNode;
class vtkDMMLTransformableNode;
class vtkDMMLTransformDisplayNode;
class vtkDMMLTransformNode;
class vtkDMMLVolumeNode;

// VTK includes
class vtkImageData;
class vtkMatrix4x4;
class vtkPoints;
class vtkPointSet;
class vtkPolyData;
class vtkUnstructuredGrid;

class VTK_CJYX_TRANSFORMS_MODULE_LOGIC_EXPORT vtkCjyxTransformLogic : public vtkDMMLAbstractLogic
{
  public:

  /// The Usual vtk class functions
  static vtkCjyxTransformLogic *New();
  vtkTypeMacro(vtkCjyxTransformLogic,vtkDMMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override { Superclass::PrintSelf(os, indent); }

  /// Apply the associated transform to the transformable node. Return true
  /// on success, false otherwise.
  /// This method is kept for backward compatibility only, it is recommended to use
  /// vtkDMMLTransformableNode::HardenTransform() method instead.
  static bool hardenTransform(vtkDMMLTransformableNode* node);

  ///
  /// Read transform from file
  vtkDMMLTransformNode* AddTransform (const char* filename, vtkDMMLScene *scene);

  ///
  /// Write transform's data to a specified file
  int SaveTransform (const char* filename, vtkDMMLTransformNode *transformNode);

  /// Generate polydata for 2D transform visualization
  /// Return true on success.
  static bool GetVisualization2d(vtkPolyData* output_RAS, vtkDMMLTransformDisplayNode* displayNode,
    vtkDMMLSliceNode* sliceNode, vtkDMMLMarkupsNode* glyphPointsNode = nullptr);

  /// Generate polydata for 2D transform visualization
  /// Return true on success.
  static bool GetVisualization2d(vtkPolyData* output_RAS, vtkDMMLTransformDisplayNode* displayNode,
    vtkMatrix4x4* sliceToRAS, double* fieldOfViewOrigin, double* fieldOfViewSize, vtkPoints* samplePositions_RAS = nullptr);

  /// Generate polydata for 3D transform visualization
  /// roiToRAS defines the ROI origin and direction.
  /// roiSize defines the ROI size (in the ROI coordinate system spacing)  .
  /// Return true on success.
  static bool GetVisualization3d(vtkPolyData* output_RAS, vtkDMMLTransformDisplayNode* displayNode,
    vtkMatrix4x4* roiToRAS, int* roiSize, vtkPoints* samplePositions_RAS = nullptr);

  /// Generate polydata for 3D transform visualization
  /// Region node can be slice (vtkDMMLSliceNode), volume (vtkDMMLVolumeNode), region of interest (vtkDMMLAnnotationROINode), or model (vtkDMMLModelNode).
  /// Return true on success.
  static bool GetVisualization3d(vtkPolyData* output_RAS, vtkDMMLTransformDisplayNode* displayNode, vtkDMMLNode* regionNode);

  /// Name of the scalar array that stores the displacement magnitude values
  /// in polydata returned by GetVisualization2d and GetVisualization3d.
  static const char* GetVisualizationDisplacementMagnitudeScalarName();

  /// Create a volume node that contains the transform displacement in each voxel.
  /// If magnitude is true then a scalar volume is created, each voxel containing the magnitude of the displacement.
  /// If magnitude is false then a 3-component scalar volume is created, each voxel containing the displacement vector.
  /// referenceVolumeNode specifies the volume origin, spacing, extent, and orientation.
  /// If existingOutputVolumeNode is specified then instead of creating a new volume node, that existing node will be updated.
  vtkDMMLVolumeNode* CreateDisplacementVolumeFromTransform(vtkDMMLTransformNode* inputTransformNode, vtkDMMLVolumeNode* referenceVolumeNode = nullptr,
    bool magnitude = true, vtkDMMLVolumeNode* existingOutputVolumeNode = nullptr);

  /// Convert the input transform to a grid transform.
  /// If referenceVolumeNode is specified then it will determine the origin, spacing, extent, and orientation of the displacement field.
  /// If existingOutputTransformNode is specified then instead of creating a new transform node, that existing node will be updated.
  vtkDMMLTransformNode* ConvertToGridTransform(vtkDMMLTransformNode* inputTransformNode, vtkDMMLVolumeNode* referenceVolumeNode = nullptr,
    vtkDMMLTransformNode* existingOutputTransformNode = nullptr);

  /// Take samples from the displacement field and store the magnitude in an image volume
  /// The extents of the output image must be set before calling this method.
  /// The origin and spacing attributes of the output image are ignored (origin, spacing, and axis directions
  /// are all specified by ijkToRAS).
  /// If transformToWorld is true then transform to world is returned, otherwise transform from world is returned.
  /// Returns true on success.
  static bool GetTransformedPointSamplesAsMagnitudeImage(vtkImageData* outputMagnitudeImage, vtkDMMLTransformNode* inputTransformNode,
    vtkMatrix4x4* ijkToRAS, bool transformToWorld = true);

  /// Take samples from the displacement field and store the vector components in an image volume
  /// The extents of the output image must be set before calling this method.
  /// The origin and spacing attributes of the output image are ignored (origin, spacing, and axis directions
  /// are all specified by ijkToRAS).
  /// If transformToWorld is true then transform to world is returned, otherwise transform from world is returned.
  /// Returns true on success.
  static bool GetTransformedPointSamplesAsVectorImage(vtkImageData* outputVectorImage, vtkDMMLTransformNode* inputTransformNode,
    vtkMatrix4x4* ijkToRAS, bool transformToWorld = true);

  /// Return the list of nodes that are transformed by the given node.
  /// If recursive is True, this be recursively called on any transform node
  /// that might be transformed by the given node. Otherwise, only the
  /// nodes immediately transformed by the given transform are returned.
  static void GetTransformedNodes(
    vtkDMMLScene* scene, vtkDMMLTransformNode* transformNode,
    std::vector<vtkDMMLDisplayableNode*>& transformedNodes,
    bool recursive=true);

  /// Return the RAS bounding box around the list of given nodes
  /// using GetRASBounds. Only the nodes with a valid bounding box are taken
  /// into account.
  /// \sa GetNodesBounds()
  static void GetNodesRASBounds(
    const std::vector<vtkDMMLDisplayableNode*>& nodes,
    double bounds[6]);

  /// Return the bounding box around the list of given nodes
  /// using GetBounds. Only the nodes with a valid bounding box
  /// are taken into account.
  /// \sa GetNodesRASBounds()
  static void GetNodesBounds(
    const std::vector<vtkDMMLDisplayableNode*>& nodes,
    double bounds[6]);

  enum TransformKind
  {
    TRANSFORM_OTHER,
    TRANSFORM_LINEAR,
    TRANSFORM_BSPLINE,
    TRANSFORM_GRID,
    TRANSFORM_THINPLATESPLINE
  };
  /// Returns TRANSFORM_LINEAR if the node contains a simple linear transform.
  /// Returns TRANSFORM_BSPLINE if the node contains a bspline transform with an
  ///   optional additive or composite bulk component.
  /// Returns TRANSFORM_GRID if the node contains a simple grid transform.
  /// Returns TRANSFORM_THINPLATESPLINE if the node contains a simple thin plate spline transform.
  /// Returns TRANSFORM_OTHER in all other cases.
  static TransformKind GetTransformKind(vtkDMMLTransformNode *transformNode);

protected:
  vtkCjyxTransformLogic();
  ~vtkCjyxTransformLogic() override;
  vtkCjyxTransformLogic(const vtkCjyxTransformLogic&);
  void operator=(const vtkCjyxTransformLogic&);

  /// Generate glyph for 2D transform visualization
  /// If samplePositions_RAS is specified then those samples will be used as glyph starting points instead of a regular grid.
  /// \sa GetVisualization2d
  static void GetGlyphVisualization2d(vtkPolyData* output_RAS, vtkDMMLTransformDisplayNode* displayNode, vtkMatrix4x4* sliceToRAS,
    double* fieldOfViewOrigin, double* fieldOfViewSize, vtkPoints* samplePositions_RAS = nullptr);
  /// Generate glyph for 3D transform visualization
  /// If samplePositions_RAS is specified then those samples will be used as glyph starting points instead of a regular grid.
  /// \sa GetVisualization3d
  static void GetGlyphVisualization3d(vtkPolyData* output_RAS, vtkDMMLTransformDisplayNode* displayNode, vtkMatrix4x4* roiToRAS,
    int* roiSize, vtkPoints* samplePositions_RAS = nullptr);

  /// Generate grid for 2D transform visualization
  /// \sa GetVisualization2d
  static void GetGridVisualization2d(vtkPolyData* output_RAS, vtkDMMLTransformDisplayNode* displayNode, vtkMatrix4x4* sliceToRAS,
    double* fieldOfViewOrigin, double* fieldOfViewSize);
  /// Generate grid for 3D transform visualization
  /// \sa GetVisualization3d
  static void GetGridVisualization3d(vtkPolyData* output_RAS, vtkDMMLTransformDisplayNode* displayNode, vtkMatrix4x4* roiToRAS, int* roiSize);

  /// Generate contours for 2D transform visualization
  /// \sa GetVisualization2d
  static void GetContourVisualization2d(vtkPolyData* output_RAS, vtkDMMLTransformDisplayNode* displayNode, vtkMatrix4x4* sliceToRAS,
    double* fieldOfViewOrigin, double* fieldOfViewSize);
  /// Generate contours for 3D transform visualization
  /// \sa GetVisualization3d
  static void GetContourVisualization3d(vtkPolyData* output_RAS, vtkDMMLTransformDisplayNode* displayNode, vtkMatrix4x4* roiToRAS, int* roiSize);

  /// Return the number of samples in each grid
  static int GetGridSubdivision(vtkDMMLTransformDisplayNode* displayNode);

  /// Add lines to the gridPolyData to make it a grid. If warpedGrid is specified then a warped grid is generated, too.
  static void CreateGrid(vtkPolyData* outputGrid_RAS, vtkDMMLTransformDisplayNode* displayNode, int numGridPoints[3], vtkPolyData* outputWarpedGrid_RAS=nullptr);

  /// Takes samples from the displacement field specified by a point set
  /// and stores it in an unstructured grid.
  /// If transformToWorld is true then transform to world is returned, otherwise transform from world is returned.
  static void GetTransformedPointSamples(vtkPointSet* outputPointSet,
    vtkDMMLTransformNode* inputTransformNode, vtkPoints* samplePositions_RAS,
    bool transformToWorld = true);

  /// Takes samples from the displacement field specified by the transformation on a uniform grid
  /// and stores it in an unstructured grid.
  /// gridToRAS specifies the grid origin, direction, and spacing
  /// gridSize is a 3-component int array specifying the dimension of the grid
  /// If transformToWorld is true then transform to world is returned, otherwise transform from world is returned.
  static void GetTransformedPointSamples(vtkPointSet* outputPointSet_RAS, vtkDMMLTransformNode* inputTransformNode,
    vtkMatrix4x4* gridToRAS, int* gridSize, bool transformToWorld = true);

  /// Takes samples from the displacement field specified by the transformation on a slice
  /// and stores it in an unstructured grid.
  /// pointGroupSize: the number of points will be N*pointGroupSize (the actual number will be returned in numGridPoints[3])
  /// samplePositions_RAS: if specified then instead of a regular grid, sample points on the slice will be used
  static void GetTransformedPointSamplesOnSlice(vtkPointSet* outputPointSet_RAS, vtkDMMLTransformNode* inputTransformNode,
    vtkMatrix4x4* sliceToRAS, double* fieldOfViewOrigin, double* fieldOfViewSize, double pointSpacing, int pointGroupSize = 1, int* numGridPoints = nullptr,
    vtkPoints* samplePositions_RAS = nullptr);

  /// Takes samples from the displacement field specified by the transformation on a 3D ROI
  /// and stores it in an unstructured grid.
  /// pointGroupSize: the number of points will be N*pointGroupSize (the actual number will be returned in numGridPoints[3])
  static void GetTransformedPointSamplesOnRoi(vtkPointSet* outputPointSet_RAS, vtkDMMLTransformNode* inputTransformNode,
    vtkMatrix4x4* roiToRAS, int* roiSize, double pointSpacingMm, int pointGroupSize=1, int* numGridPoints=nullptr);

  /// Get markup points as vtkPoints in RAS coordinate system.
  static void  GetMarkupsAsPoints(vtkDMMLMarkupsNode* markupsNode, vtkPoints* samplePoints_RAS);

};

#endif
