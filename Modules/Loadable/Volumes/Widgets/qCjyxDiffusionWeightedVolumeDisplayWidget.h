/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qCjyxDiffusionWeightedVolumeDisplayWidget_h
#define __qCjyxDiffusionWeightedVolumeDisplayWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// Cjyx includes
#include <qCjyxWidget.h>

#include "qCjyxVolumesModuleWidgetsExport.h"

class vtkDMMLNode;
class vtkDMMLDiffusionWeightedVolumeDisplayNode;
class vtkDMMLDiffusionWeightedVolumeNode;
class vtkDMMLDiffusionWeightedSliceDisplayNode;
class vtkDMMLGlyphableVolumeSliceDisplayNode;
class qCjyxDiffusionWeightedVolumeDisplayWidgetPrivate;

/// \ingroup Cjyx_QtModules_Volumes
class Q_CJYX_QTMODULES_VOLUMES_WIDGETS_EXPORT qCjyxDiffusionWeightedVolumeDisplayWidget : public qCjyxWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  /// Constructors
  typedef qCjyxWidget Superclass;
  explicit qCjyxDiffusionWeightedVolumeDisplayWidget(QWidget* parent = nullptr);
  ~qCjyxDiffusionWeightedVolumeDisplayWidget() override;

  vtkDMMLDiffusionWeightedVolumeNode* volumeNode()const;
  vtkDMMLDiffusionWeightedVolumeDisplayNode* volumeDisplayNode()const;
  QList<vtkDMMLGlyphableVolumeSliceDisplayNode*> sliceDisplayNodes()const;
public slots:

  /// Set the DMML node of interest
  void setDMMLVolumeNode(vtkDMMLDiffusionWeightedVolumeNode* volumeNode);
  void setDMMLVolumeNode(vtkDMMLNode* node);

  void setDWIComponent(int component);
protected slots:
  void updateWidgetFromVolumeNode();
  void updateWidgetFromDisplayNode();

protected:
  QScopedPointer<qCjyxDiffusionWeightedVolumeDisplayWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qCjyxDiffusionWeightedVolumeDisplayWidget);
  Q_DISABLE_COPY(qCjyxDiffusionWeightedVolumeDisplayWidget);
};

#endif
