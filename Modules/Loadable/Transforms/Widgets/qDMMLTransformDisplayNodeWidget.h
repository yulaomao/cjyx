/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

==============================================================================*/


#ifndef __qDMMLTransformDisplayNodeWidget_h
#define __qDMMLTransformDisplayNodeWidget_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// Cjyx includes
#include "qDMMLWidget.h"

#include "qCjyxTransformsModuleWidgetsExport.h"

class qDMMLTransformDisplayNodeWidgetPrivate;
class vtkDMMLTransformNode;
class vtkDMMLNode;

/// \ingroup Cjyx_QtModules_Transforms
class Q_CJYX_MODULE_TRANSFORMS_WIDGETS_EXPORT
qDMMLTransformDisplayNodeWidget
  : public qDMMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qDMMLWidget Superclass;
  qDMMLTransformDisplayNodeWidget(QWidget *newParent = nullptr);
  ~qDMMLTransformDisplayNodeWidget() override;

public slots:

  /// Set the DMML node of interest
  /// Note that setting transformNode to 0 will disable the widget
  void setDMMLTransformNode(vtkDMMLTransformNode* transformNode);

  /// Utility function that calls setDMMLTransformNode(vtkDMMLTransformNode* transformNode)
  /// It's useful to connect to vtkDMMLNode* signals
  void setDMMLTransformNode(vtkDMMLNode* node);

  void setVisibility(bool);
  void setVisibility2d(bool);
  void setVisibility3d(bool);

  void setGlyphVisualizationMode(bool);
  void setGridVisualizationMode(bool);
  void setContourVisualizationMode(bool);

  void updateGlyphSourceOptions(int sourceOption);
  void regionNodeChanged(vtkDMMLNode* node);
  void glyphPointsNodeChanged(vtkDMMLNode* node);
  void setGlyphSpacingMm(double spacing);
  void setGlyphScalePercent(double scale);
  void setGlyphDisplayRangeMm(double min, double max);
  void setGlyphType(int glyphType);
  void setGlyphTipLengthPercent(double length);
  void setGlyphDiameterMm(double diameterMm);
  void setGlyphShaftDiameterPercent(double diameterPercent);
  void setGlyphResolution(double resolution);
  void setGridScalePercent(double scale);
  void setGridSpacingMm(double spacing);
  void setGridLineDiameterMm(double diameterMm);
  void setGridResolutionMm(double resolutionMm);
  void setGridShowNonWarped(bool show);
  void setContourLevelsMm(QString values_str);
  void setContourResolutionMm(double resolutionMm);
  void setContourOpacityPercent(double opacity);

  void setEditorVisibility(bool enabled);
  void setEditorTranslationEnabled(bool enabled);
  void setEditorRotationEnabled(bool enabled);
  void setEditorScalingEnabled(bool enabled);
  void updateEditorBounds();

  void setColorTableNode(vtkDMMLNode* colorTableNode);

  void colorUpdateRange();
  void onColorInteractionEvent();
  void onColorModifiedEvent();

protected slots:
  void updateWidgetFromDisplayNode();

protected:
  QScopedPointer<qDMMLTransformDisplayNodeWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLTransformDisplayNodeWidget);
  Q_DISABLE_COPY(qDMMLTransformDisplayNodeWidget);

};

#endif
