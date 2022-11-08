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

#ifndef __qCjyxLabelMapVolumeDisplayWidget_h
#define __qCjyxLabelMapVolumeDisplayWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// Cjyx includes
#include <qCjyxWidget.h>

#include "qCjyxVolumesModuleWidgetsExport.h"

class vtkDMMLNode;
class vtkDMMLLabelMapVolumeDisplayNode;
class vtkDMMLScalarVolumeNode;
class qCjyxLabelMapVolumeDisplayWidgetPrivate;

/// \ingroup Cjyx_QtModules_Volumes
class Q_CJYX_QTMODULES_VOLUMES_WIDGETS_EXPORT qCjyxLabelMapVolumeDisplayWidget : public qCjyxWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  /// Constructors
  typedef qCjyxWidget Superclass;
  explicit qCjyxLabelMapVolumeDisplayWidget(QWidget* parent);
  ~qCjyxLabelMapVolumeDisplayWidget() override;

  vtkDMMLScalarVolumeNode* volumeNode()const;
  vtkDMMLLabelMapVolumeDisplayNode* volumeDisplayNode()const;

  int sliceIntersectionThickness()const;

public slots:

  /// Set the DMML node of interest
  void setDMMLVolumeNode(vtkDMMLScalarVolumeNode* volumeNode);
  void setDMMLVolumeNode(vtkDMMLNode* node);

  void setColorNode(vtkDMMLNode* colorNode);

  void setSliceIntersectionThickness(int);

protected slots:
  void updateWidgetFromDMML();

protected:
  QScopedPointer<qCjyxLabelMapVolumeDisplayWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qCjyxLabelMapVolumeDisplayWidget);
  Q_DISABLE_COPY(qCjyxLabelMapVolumeDisplayWidget);
};

#endif
