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

#ifndef __qDMMLVolumeInfoWidget_h
#define __qDMMLVolumeInfoWidget_h


// Qt includes
#include <QListWidget>

// CTK includes
#include <ctkVTKObject.h>

// qDMML includes
#include "qDMMLWidget.h"
#include "qDMMLWidgetsExport.h"

class qDMMLVolumeInfoWidgetPrivate;
class vtkDMMLNode;
class vtkDMMLVolumeNode;

class QDMML_WIDGETS_EXPORT qDMMLVolumeInfoWidget : public qDMMLWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(bool dataTypeEditable READ isDataTypeEditable WRITE setDataTypeEditable)
public:
  typedef qDMMLWidget Superclass;

  qDMMLVolumeInfoWidget(QWidget *parent=nullptr);
  ~qDMMLVolumeInfoWidget() override;

  vtkDMMLVolumeNode* volumeNode()const;
  // Depends on the dimension, spacing and origin of the volume
  bool isCentered()const;

  // Disabled by default
  bool isDataTypeEditable()const;

public slots:
  /// Utility function to be connected with generic signals
  void setVolumeNode(vtkDMMLNode *node);
  /// Set the volume node to display
  void setVolumeNode(vtkDMMLVolumeNode *node);
  void setDataTypeEditable(bool enable);

  void setImageSpacing(double*);
  void setImageOrigin(double*);
  void center();
  void setScanOrder(int);
  void setNumberOfScalars(int);
  void setScalarType(int);
  void setWindowLevelFromPreset(QListWidgetItem*);

protected slots:
  void updateWidgetFromDMML();

protected:
  QScopedPointer<qDMMLVolumeInfoWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLVolumeInfoWidget);
  Q_DISABLE_COPY(qDMMLVolumeInfoWidget);
};

#endif
