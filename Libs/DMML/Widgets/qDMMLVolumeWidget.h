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

#ifndef __qDMMLVolumeWidget_h
#define __qDMMLVolumeWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>
#include "qDMMLWidgetsExport.h"

class vtkDMMLNode;
class vtkDMMLScalarVolumeDisplayNode;
class vtkDMMLScalarVolumeNode;
class qDMMLVolumeWidgetPrivate;

/// \brief Abstract widget to represent and control the properties of a scalar
/// volume node.
class QDMML_WIDGETS_EXPORT qDMMLVolumeWidget : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  /// Constructors
  typedef QWidget Superclass;
  explicit qDMMLVolumeWidget(QWidget* parentWidget = nullptr);
  ~qDMMLVolumeWidget() override;

  ///
  /// Return the current DMML node of interest
  Q_INVOKABLE vtkDMMLScalarVolumeNode* dmmlVolumeNode()const;

public slots:
  /// Set the volume to observe
  void setDMMLVolumeNode(vtkDMMLScalarVolumeNode* displayNode);

  /// Utility slot to conveniently connect a SIGNAL(vtkDMMLNode*) with the
  /// widget.
  void setDMMLVolumeNode(vtkDMMLNode* node);

protected slots:
  /// Update the widget from volume node properties.
  /// The slot gets called when a volume node is set or when
  /// it has been modified.
  /// To be reimplemented in subclasses.
  /// \sa updateWidgetFromDMMLDisplayNode(), setDMMLVolumeNode()
  virtual void updateWidgetFromDMMLVolumeNode();

  /// Update the widget from volume display node properties.
  /// The slot gets called when a volume display node is set or when
  /// it has been modified.
  /// \sa updateWidgetFromDMMLDisplayNode(), setDMMLVolumeDisplayNode()
  virtual void updateWidgetFromDMMLDisplayNode();

protected:
  QScopedPointer<qDMMLVolumeWidgetPrivate> d_ptr;
  qDMMLVolumeWidget(qDMMLVolumeWidgetPrivate* ptr,
                    QWidget* parentWidget = nullptr);

  /// Return the volume display node.
  vtkDMMLScalarVolumeDisplayNode* dmmlDisplayNode()const;

private:
  Q_DECLARE_PRIVATE(qDMMLVolumeWidget);
  Q_DISABLE_COPY(qDMMLVolumeWidget);

  /// Observe volume display node
  void setDMMLVolumeDisplayNode(vtkDMMLScalarVolumeDisplayNode* displayNode);
};

#endif
