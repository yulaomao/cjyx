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

#ifndef __qDMMLDisplayNodeWidget_h
#define __qDMMLDisplayNodeWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// qDMML includes
#include "qDMMLWidgetsExport.h"

class qDMMLDisplayNodeWidgetPrivate;
class vtkDMMLNode;
class vtkDMMLDisplayNode;

class QDMML_WIDGETS_EXPORT qDMMLDisplayNodeWidget : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  qDMMLDisplayNodeWidget(QWidget *parent=nullptr);
  ~qDMMLDisplayNodeWidget() override;

  vtkDMMLDisplayNode* dmmlDisplayNode()const;

  bool visibility()const;
  bool selected()const;
  bool clipping()const;
  bool threeDVisible()const;
  bool sliceIntersectionVisible()const;
  int sliceIntersectionThickness()const;
  double sliceIntersectionOpacity()const;

public slots:
  /// Set the volume node to display
  void setDMMLDisplayNode(vtkDMMLDisplayNode *node);
  /// Utility function to be connected with generic signals
  void setDMMLDisplayNode(vtkDMMLNode *node);
  /// Utility function to be connected with generic signals,
  /// it internally shows the 1st display node.
  void setDMMLDisplayableNode(vtkDMMLNode* displayableNode);

  void setVisibility(bool);
  void setSelected(bool);
  void setClipping(bool);
  void setThreeDVisible(bool);
  void setSliceIntersectionVisible(bool);
  void setSliceIntersectionThickness(int);
  void setSliceIntersectionOpacity(double);

  /// Make checkboxes visible or invisible, for use with display nodes that don't
  /// need to control visibility, selected, clipping, slice intersection
  /// visibility, slice intersection thickness
  void setVisibilityVisible(bool);
  void setSelectedVisible(bool);
  void setClippingVisible(bool);
  void setThreeDVisibleVisible(bool);
  void setSliceIntersectionVisibleVisible(bool);
  void setSliceIntersectionThicknessVisible(bool);
  void setSliceIntersectionOpacityVisible(bool);

protected slots:
  void updateWidgetFromDMML();
  void updateNodeFromProperty();

protected:
  QScopedPointer<qDMMLDisplayNodeWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLDisplayNodeWidget);
  Q_DISABLE_COPY(qDMMLDisplayNodeWidget);
};

#endif
