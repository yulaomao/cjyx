# Transforms

## Related DMML nodes
- [vtkDMMLTransformableNode](https://slicer.org/doc/html/classvtkDMMLTransformableNode.html): any node that can be transformed
- [vtkDMMLTransformNode](https://slicer.org/doc/html/classvtkDMMLTransformNode.html): it can store any linear or deformable transform or composite of multiple transforms
  - [vtkDMMLLinearTransformNode](https://slicer.org/doc/html/classvtkDMMLLinearTransformNode.html): Deprecated. The transform does exactly the same as vtkDMMLTransformNode but has a different class name, which are still used for showing only certain transform types in node selectors. In the future this class will be removed. A vtkDMMLLinearTransformNode may contain non-linear components after a non-linear transform is hardened on it. Therefore, to check linearity of a transform the vtkDMMLTransformNode::IsLinear() and vtkDMMLTransformNode::IsTransformToWorldLinear() and vtkDMMLTransformNode::IsTransformToNodeLinear() methods must be used instead of using vtkDMMLLinearTransformNode::SafeDownCast(transform)!=NULL.
  - [vtkDMMLBSplineTransformNode](https://slicer.org/doc/html/classvtkDMMLBSplineTransformNode.html): Deprecated. The transform does exactly the same as vtkDMMLTransformNode but has a different class name, which are still used for showing only certain transform types in node selectors. In the future this class will be removed.
  - [vtkDMMLGridTransformNode](https://slicer.org/doc/html/classvtkDMMLGridTransformNode.html): Deprecated. The transform does exactly the same as vtkDMMLTransformNode but has a different class name, which are still used for showing only certain transform types in node selectors. In the future this class will be removed.

## Transform files

- Cjyx stores transforms in VTK classes in memory, but uses ITK transform IO classes to read/write transforms to files. ITK's convention is to use LPS coordinate system as opposed to RAS coordinate system in Cjyx (see Coordinate systems page for details). Conversion between VTK and ITK transform classes are implemented in vtkITKTransformConverter.
- ITK stores the transform in resampling (a.k.a., image processing) convention, i.e., that transforms points from fixed to moving coordinate system. This transform is usable as is for resampling a moving image in the coordinate system of a fixed image. For transforming points and surface models to the fixed coordinate system, one needs the transform in the modeling (a.k.a. computer graphics) convention, i.e., transform from moving to fixed coordinate system (which is the inverse of the "image processing" convention).
- Transform nodes in Cjyx can store transforms in both modeling (when "to parent" transform is set) and resampling way (when "from parent" transform is set). When writing transform to ITK files, linear transforms are inverted as needed and written as an AffineTransform. Non-linear transforms cannot be inverted without losing information (in general), therefore if a non-linear transform is defined in resampling convention in Cjyx then it is written to ITK file using special "Inverse" transform types (e.g., InverseDisplacementFieldTransform instead of DisplacementFieldTransform). Definition of the inverse classes are available in [vtkITKTransformInverse](https://github.com/Slicer/Slicer/blob/master/Libs/DMML/Core/vtkITKTransformInverse.h). The inverse classes are only usable for file IO, because currently ITK does not provide a generic inverse transform computation method. Options to manage inverse transforms in applications:
  - Create VTK transforms from ITK transforms: VTK transforms can compute their inverse, transform can be changed dynamically, the inverse will be always updated automatically in real-time (this approach is used by Cjyx)
  - Invert transform in ITK statically: by converting to displacement field and inverting the displacement field; whenever the forward transform changes, the complete inverse transform has to be computed again (which is typically very time consuming)
  - Avoid inverse non-linear transforms: make sure that non-linear transforms are only set as FromParent
- Transforms module in Cjyx shows linear transform matrix values "to parent" (modeling convention) in RAS coordinate system. Therefore to retrieve the same values from an ITK transforms as shown in Cjyx GUI, one has switch between RAS/LPS and modeling/resampling. See example [here](../script_repository.md#convert-between-itk-and-cjyx-linear-transforms).

## Events

When a transform node is observed by a transformable node, [vtkDMMLTransformableNode::TransformModifiedEvent](https://slicer.org/doc/html/classvtkDMMLTransformableNode.html#ace1c30fc9df552543f00d51a20c038a6a4993bf6e23a6dfc138cb2efc1b9ce43b) is fired on the transformable node at observation time. Anytime a transform is modified, vtkCommand::ModifiedEvent is fired on the transform node and [vtkDMMLTransformableNode::TransformModifiedEvent](https://slicer.org/doc/html/classvtkDMMLTransformableNode.html#ace1c30fc9df552543f00d51a20c038a6a4993bf6e23a6dfc138cb2efc1b9ce43b) is fired on the transformable node.

## Examples

Examples for common operations on transform are provided in the [script repository](../script_repository.md#transforms).