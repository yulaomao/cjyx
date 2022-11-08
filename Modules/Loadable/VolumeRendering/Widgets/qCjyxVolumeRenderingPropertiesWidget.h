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

#ifndef __qCjyxVolumeRenderingPropertiesWidget_h
#define __qCjyxVolumeRenderingPropertiesWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// Cjyx includes
#include "qCjyxVolumeRenderingModuleWidgetsExport.h"

class qCjyxVolumeRenderingPropertiesWidgetPrivate;
class vtkDMMLNode;
class vtkDMMLVolumeNode;
class vtkDMMLVolumeRenderingDisplayNode;

/// \ingroup Cjyx_QtModules_VolumeRendering
class Q_CJYX_MODULE_VOLUMERENDERING_WIDGETS_EXPORT qCjyxVolumeRenderingPropertiesWidget
  : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  typedef QWidget Superclass;
  qCjyxVolumeRenderingPropertiesWidget(QWidget *parent=nullptr);
  ~qCjyxVolumeRenderingPropertiesWidget() override;

  vtkDMMLNode* dmmlNode()const;
  vtkDMMLVolumeRenderingDisplayNode* dmmlVolumeRenderingDisplayNode()const;
  vtkDMMLVolumeNode* dmmlVolumeNode()const;

public slots:
  void setDMMLVolumeRenderingDisplayNode(vtkDMMLVolumeRenderingDisplayNode* node);
  /// Utility slot to set the volume rendering display node.
  void setDMMLNode(vtkDMMLNode* node);

protected slots:
  virtual void updateWidgetFromDMML();
  virtual void updateWidgetFromDMMLVolumeNode();

protected:
  QScopedPointer<qCjyxVolumeRenderingPropertiesWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qCjyxVolumeRenderingPropertiesWidget);
  Q_DISABLE_COPY(qCjyxVolumeRenderingPropertiesWidget);
};

#endif
