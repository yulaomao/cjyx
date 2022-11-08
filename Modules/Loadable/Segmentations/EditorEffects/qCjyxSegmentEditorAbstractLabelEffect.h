/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __qCjyxSegmentEditorAbstractLabelEffect_h
#define __qCjyxSegmentEditorAbstractLabelEffect_h

// Segmentations Editor Effects includes
#include "qCjyxSegmentationsEditorEffectsExport.h"

#include "qCjyxSegmentEditorAbstractEffect.h"

class qCjyxSegmentEditorAbstractLabelEffectPrivate;

class vtkMatrix4x4;
class vtkOrientedImageData;
class vtkPolyData;
class vtkDMMLVolumeNode;
class vtkDMMLSegmentationNode;

/// \ingroup CjyxRt_QtModules_Segmentations
/// \brief Base class for all "label" effects.
///
/// This base class provides common GUI and DMML for the options PaintOver and Threshold.
class Q_CJYX_SEGMENTATIONS_EFFECTS_EXPORT qCjyxSegmentEditorAbstractLabelEffect :
  public qCjyxSegmentEditorAbstractEffect
{
public:
  Q_OBJECT

public:
  typedef qCjyxSegmentEditorAbstractEffect Superclass;
  qCjyxSegmentEditorAbstractLabelEffect(QObject* parent = nullptr);
  ~qCjyxSegmentEditorAbstractLabelEffect() override;

public:
  /// Clone editor effect
  /// (redefinition of pure virtual function to allow python wrapper to identify this as abstract class)
  qCjyxSegmentEditorAbstractEffect* clone() override = 0;

  /// Create options frame widgets, make connections, and add them to the main options frame using \sa addOptionsWidget
  void setupOptionsFrame() override;

  /// Set default parameters in the parameter DMML node
  void setDMMLDefaults() override;

  /// Perform actions needed on reference geometry change
  void referenceGeometryChanged() override;

  /// Perform actions needed on master volume change
  void masterVolumeNodeChanged() override;

public slots:
  /// Update user interface from parameter set node
  void updateGUIFromDMML() override;

  /// Update parameter set node from user interface
  void updateDMMLFromGUI() override;

// Utility functions
public:

  /// Rasterize a poly data onto the input image into the slice view
  Q_INVOKABLE static void appendPolyMask(vtkOrientedImageData* input, vtkPolyData* polyData, qDMMLSliceWidget* sliceWidget, vtkDMMLSegmentationNode* segmentationNode=nullptr);

  /// Create a slice view screen space (2D) mask image for the given polydata
  Q_INVOKABLE static void createMaskImageFromPolyData(vtkPolyData* polyData, vtkOrientedImageData* outputMask, qDMMLSliceWidget* sliceWidget);

  /// Append image onto image. Resamples appended image and saves result in input image
  Q_INVOKABLE static void appendImage(vtkOrientedImageData* inputImage, vtkOrientedImageData* appendedImage);

  /// Return matrix for volume node that takes into account the IJKToRAS
  /// and any linear transforms that have been applied
  Q_INVOKABLE static void imageToWorldMatrix(vtkDMMLVolumeNode* node, vtkMatrix4x4* ijkToRas);

  /// Return matrix for oriented image data that takes into account the image to world
  /// and any linear transforms that have been applied on the given segmentation
  Q_INVOKABLE static void imageToWorldMatrix(vtkOrientedImageData* image, vtkDMMLSegmentationNode* node, vtkMatrix4x4* ijkToRas);

protected:
  QScopedPointer<qCjyxSegmentEditorAbstractLabelEffectPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qCjyxSegmentEditorAbstractLabelEffect);
  Q_DISABLE_COPY(qCjyxSegmentEditorAbstractLabelEffect);
};

#endif
