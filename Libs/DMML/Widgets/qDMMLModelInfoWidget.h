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

#ifndef __qDMMLModelInfoWidget_h
#define __qDMMLModelInfoWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// qDMML includes
#include "qDMMLWidgetsExport.h"

class qDMMLModelInfoWidgetPrivate;
class vtkDMMLNode;
class vtkDMMLModelNode;

class QDMML_WIDGETS_EXPORT qDMMLModelInfoWidget
  : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  qDMMLModelInfoWidget(QWidget *parent=nullptr);
  ~qDMMLModelInfoWidget() override;

  vtkDMMLModelNode* dmmlModelNode()const;

public slots:
  /// Utility function to be connected with generic signals
  void setDMMLModelNode(vtkDMMLNode *node);
  /// Set the Model node to display
  void setDMMLModelNode(vtkDMMLModelNode *node);

protected slots:
  void updateWidgetFromDMML();

protected:
  void showEvent(QShowEvent *) override;
  QScopedPointer<qDMMLModelInfoWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLModelInfoWidget);
  Q_DISABLE_COPY(qDMMLModelInfoWidget);
};

#endif
