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
  and CANARIE.

==============================================================================*/

#ifndef __qDMMLSegmentationGeometryWidget_h
#define __qDMMLSegmentationGeometryWidget_h

// Segmentations includes
#include "qCjyxSegmentationsModuleWidgetsExport.h"

// DMMLWidgets includes
#include "qDMMLWidget.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

class vtkOrientedImageData;
class vtkDMMLNode;
class vtkDMMLSegmentationNode;
class qDMMLSegmentationGeometryWidgetPrivate;

/// \ingroup Cjyx_QtModules_Segmentations_Widgets
class Q_CJYX_MODULE_SEGMENTATIONS_WIDGETS_EXPORT qDMMLSegmentationGeometryWidget : public qDMMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

  Q_PROPERTY(bool editEnabled READ editEnabled WRITE setEditEnabled)
  Q_PROPERTY(double oversamplingFactor READ oversamplingFactor WRITE setOversamplingFactor)
  Q_PROPERTY(bool isotropicSpacing READ isotropicSpacing WRITE setIsotropicSpacing)
  Q_PROPERTY(bool padSegmentation READ padSegmentation WRITE setPadSegmentation)

public:
  typedef qDMMLWidget Superclass;
  /// Constructor
  explicit qDMMLSegmentationGeometryWidget(QWidget* parent = nullptr);
  /// Destructor
  ~qDMMLSegmentationGeometryWidget() override;

  /// Get segmentation DMML node
  Q_INVOKABLE vtkDMMLSegmentationNode* segmentationNode()const;
  Q_INVOKABLE QString segmentationNodeID()const;

  bool editEnabled()const;
  vtkDMMLNode* sourceNode()const;
  double oversamplingFactor()const;
  bool isotropicSpacing()const;
  bool padSegmentation()const;

  void setSpacing(double aSpacing[3]);

  /// Get calculated geometry image data
  Q_INVOKABLE void geometryImageData(vtkOrientedImageData* outputGeometry);

public slots:
  /// Set segmentation DMML node
  void setSegmentationNode(vtkDMMLSegmentationNode* node);

  void setEditEnabled(bool aEditEnabled);
  void setSourceNode(vtkDMMLNode* sourceNode);
  void setOversamplingFactor(double aOversamplingFactor);
  void setIsotropicSpacing(bool aIsotropicSpacing);
  void setPadSegmentation(bool aPadSegmentation);

  /// Set reference geometry conversion parameter to the one specified
  void setReferenceImageGeometryForSegmentationNode();

  /// Resample existing labelmaps in segmentation node with specified geometry
  void resampleLabelmapsInSegmentationNode();

protected slots:
  /// Calculate output geometry from input segmentation and source node and update UI
  void updateWidgetFromDMML();

  /// Calculate source axis permutation and then output geometry.
  void onSourceNodeChanged(vtkDMMLNode*);

  void onOversamplingFactorChanged(double);
  void onIsotropicSpacingChanged(bool);
  void onUserSpacingChanged(double*);
  void onPadSegmentationChanged(bool);

protected:
  QScopedPointer<qDMMLSegmentationGeometryWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLSegmentationGeometryWidget);
  Q_DISABLE_COPY(qDMMLSegmentationGeometryWidget);
};

#endif
