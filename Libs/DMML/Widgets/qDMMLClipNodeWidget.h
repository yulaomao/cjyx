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

#ifndef __qDMMLClipNodeWidget_h
#define __qDMMLClipNodeWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// qDMML includes
#include "qDMMLWidgetsExport.h"
#include "vtkDMMLClipModelsNode.h"

class qDMMLClipNodeWidgetPrivate;
class vtkDMMLNode;
class vtkDMMLClipModelsNode;

class QDMML_WIDGETS_EXPORT qDMMLClipNodeWidget : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  qDMMLClipNodeWidget(QWidget *parent=nullptr);
  ~qDMMLClipNodeWidget() override;

  vtkDMMLClipModelsNode* dmmlClipNode()const;

  int clipType()const;
  int redSliceClipState()const;
  int yellowSliceClipState()const;
  int greenSliceClipState()const;
  vtkDMMLClipModelsNode::ClippingMethodType clippingMethod()const;

  void setClipType(int);
  void setRedSliceClipState(int);
  void setYellowSliceClipState(int);
  void setGreenSliceClipState(int);
  void setClippingMethod(vtkDMMLClipModelsNode::ClippingMethodType);

public slots:
  /// Set the clip node to represent
  void setDMMLClipNode(vtkDMMLClipModelsNode *node);
  /// Utility function to be connected to signals/slots
  void setDMMLClipNode(vtkDMMLNode *node);

protected slots:
  void updateWidgetFromDMML();

  void updateNodeClipType();
  void updateNodeRedClipState();
  void updateNodeYellowClipState();
  void updateNodeGreenClipState();
  void updateNodeClippingMethod();

protected:
  QScopedPointer<qDMMLClipNodeWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLClipNodeWidget);
  Q_DISABLE_COPY(qDMMLClipNodeWidget);
};

#endif
