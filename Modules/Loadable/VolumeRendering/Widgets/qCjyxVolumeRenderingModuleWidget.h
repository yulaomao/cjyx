/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Alex Yarmakovich, Isomics Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qCjyxVolumeRenderingModuleWidget_h
#define __qCjyxVolumeRenderingModuleWidget_h

// CTK includes
#include <ctkVTKObject.h>

// Cjyx includes
#include "qCjyxAbstractModuleWidget.h"
#include "qCjyxVolumeRenderingModuleWidgetsExport.h"

class qCjyxVolumeRenderingPropertiesWidget;
class qCjyxVolumeRenderingModuleWidgetPrivate;
class vtkDMMLAnnotationROINode;
class vtkDMMLDisplayableNode;
class vtkDMMLNode;
class vtkDMMLMarkupsROINode;
class vtkDMMLViewNode;
class vtkDMMLVolumeNode;
class vtkDMMLVolumePropertyNode;
class vtkDMMLVolumeRenderingDisplayNode;

/// \ingroup Cjyx_QtModules_VolumeRendering
class Q_CJYX_MODULE_VOLUMERENDERING_WIDGETS_EXPORT qCjyxVolumeRenderingModuleWidget :
  public qCjyxAbstractModuleWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qCjyxAbstractModuleWidget Superclass;
  qCjyxVolumeRenderingModuleWidget(QWidget *parent=nullptr);
  ~qCjyxVolumeRenderingModuleWidget() override;

  Q_INVOKABLE vtkDMMLVolumeNode* dmmlVolumeNode()const;
  Q_INVOKABLE vtkDMMLDisplayableNode* dmmlROINode()const;
  Q_INVOKABLE vtkDMMLAnnotationROINode* dmmlAnnotationROINode()const;
  Q_INVOKABLE vtkDMMLMarkupsROINode* dmmlMarkupsROINode()const;
  Q_INVOKABLE vtkDMMLVolumePropertyNode* dmmlVolumePropertyNode()const;
  Q_INVOKABLE vtkDMMLVolumeRenderingDisplayNode* dmmlDisplayNode()const;

  void addRenderingMethodWidget(const QString& methodClassName, qCjyxVolumeRenderingPropertiesWidget* widget);

  bool setEditedNode(vtkDMMLNode* node, QString role = QString(), QString context = QString()) override;
  double nodeEditable(vtkDMMLNode* node) override;

public slots:
  void setDMMLVolumeNode(vtkDMMLNode* node);
  void setDMMLROINode(vtkDMMLNode* node);
  void setDMMLVolumePropertyNode(vtkDMMLNode* node);

  void fitROIToVolume();

protected slots:
  void onCurrentDMMLVolumeNodeChanged(vtkDMMLNode* node);
  void onVisibilityChanged(bool);
  void onCropToggled(bool);

  void onCurrentDMMLROINodeChanged(vtkDMMLNode* node);
  void onCurrentDMMLVolumePropertyNodeChanged(vtkDMMLNode* node);

  void onCurrentRenderingMethodChanged(int index);
  void onCurrentMemorySizeChanged();
  void onCurrentQualityControlChanged(int index);
  void onCurrentFramerateChanged(double fps);
  void onAutoReleaseGraphicsResourcesCheckBoxToggled(bool autoRelease);

  void updateWidgetFromDMML();
  void updateWidgetFromROINode();

  void synchronizeScalarDisplayNode();
  void setFollowVolumeDisplayNode(bool);
  void setIgnoreVolumesThreshold(bool ignore);

  void onThresholdChanged(bool threshold);
  void onROICropDisplayCheckBoxToggled(bool toggle);
  void onChartsExtentChanged();
  void onEffectiveRangeModified();

protected:
  void setup() override;

protected:
  QScopedPointer<qCjyxVolumeRenderingModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qCjyxVolumeRenderingModuleWidget);
  Q_DISABLE_COPY(qCjyxVolumeRenderingModuleWidget);
};

#endif
